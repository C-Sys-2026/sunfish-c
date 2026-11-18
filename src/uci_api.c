#include "uci_api.h"

#include "board.h"
#include "eval.h"
#include "legality.h"
#include "movegen.h"
#include "search.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static volatile int stop_requested;

static int square_from_coordinate(const char *text)
{
    if (text == NULL || text[0] < 'a' || text[0] > 'h'
        || text[1] < '1' || text[1] > '8') {
        return -1;
    }
    return A1 - (text[1] - '1') * 10 + (text[0] - 'a');
}

int uci_move_from_uci(const char *text, int black_to_move, Move *move)
{
    int from;
    int to;

    if (text == NULL || move == NULL
        || (text[4] != '\0' && text[4] != 'n' && text[4] != 'b'
            && text[4] != 'r' && text[4] != 'q')
        || (text[4] != '\0' && text[5] != '\0')) {
        return 0;
    }
    from = square_from_coordinate(text);
    to = square_from_coordinate(text + 2);
    if (from < 0 || to < 0 || text[2] == '\0' || text[3] == '\0') {
        return 0;
    }
    if (black_to_move) {
        from = 119 - from;
        to = 119 - to;
    }
    move->i = from;
    move->j = to;
    move->prom = text[4] == '\0' ? '\0' : (char)toupper((unsigned char)text[4]);
    return 1;
}

int uci_move_to_uci(Move move, int black_to_move, char output[6])
{
    int from = black_to_move ? 119 - move.i : move.i;
    int to = black_to_move ? 119 - move.j : move.j;

    if (output == NULL || from < 0 || from >= BOARD_SIZE || to < 0
        || to >= BOARD_SIZE || from / 10 < 2 || from / 10 > 9
        || to / 10 < 2 || to / 10 > 9) {
        return 0;
    }
    output[0] = (char)('a' + from % 10 - 1);
    output[1] = (char)('8' - from / 10 + 2);
    output[2] = (char)('a' + to % 10 - 1);
    output[3] = (char)('8' - to / 10 + 2);
    output[4] = move.prom == '\0' ? '\0' : (char)tolower((unsigned char)move.prom);
    output[5] = '\0';
    return 1;
}

void uci_position_start(UciPosition *position)
{
    if (position == NULL) {
        return;
    }
    memcpy(position->position.board, initial, sizeof(initial));
    position->position.score = 0;
    position->position.wc[0] = 1;
    position->position.wc[1] = 1;
    position->position.bc[0] = 1;
    position->position.bc[1] = 1;
    position->position.ep = 0;
    position->position.kp = 0;
    position->black_to_move = 0;
    position->halfmove_clock = 0;
    position->fullmove_number = 1;
}

static int parse_fen(UciPosition *result, int argc, char *const argv[])
{
    Position parsed = {0};
    int rank = 0;
    int file = 0;
    const char *board_text = argv[1];

    memcpy(parsed.board, empty, sizeof(empty));
    for (const char *cursor = board_text; *cursor != '\0'; cursor++) {
        if (*cursor == '/') {
            if (file != 8) return 0;
            rank++;
            file = 0;
        } else if (isdigit((unsigned char)*cursor)) {
            file += *cursor - '0';
        } else if (strchr("PNBRQKpnbrqk", *cursor) != NULL) {
            if (rank > 7 || file > 7) return 0;
            parsed.board[20 + rank * 10 + file + 1] = *cursor;
            file++;
        } else {
            return 0;
        }
        if (file > 8) return 0;
    }
    if (rank != 7 || file != 8 || argc != 7
        || (strcmp(argv[2], "w") != 0 && strcmp(argv[2], "b") != 0)
        || strlen(argv[3]) > 4) {
        return 0;
    }
    if (strchr(parsed.board, 'K') == NULL || strchr(parsed.board, 'k') == NULL) {
        return 0;
    }
    parsed.wc[1] = strchr(argv[3], 'K') != NULL;
    parsed.wc[0] = strchr(argv[3], 'Q') != NULL;
    parsed.bc[1] = strchr(argv[3], 'k') != NULL;
    parsed.bc[0] = strchr(argv[3], 'q') != NULL;
    if (strcmp(argv[4], "-") != 0) {
        int ep = square_from_coordinate(argv[4]);
        if (ep < 0) return 0;
        parsed.ep = ep;
    }
    parsed.score = 0;
    if (strcmp(argv[5], "0") != 0 && strtol(argv[5], NULL, 10) < 0) return 0;
    if (strtol(argv[6], NULL, 10) < 1) return 0;
    result->halfmove_clock = atoi(argv[5]);
    result->fullmove_number = atoi(argv[6]);
    if (strcmp(argv[2], "b") == 0) {
        parsed = rotate(parsed, 0);
    }
    result->position = parsed;
    result->black_to_move = strcmp(argv[2], "b") == 0;
    return 1;
}

int uci_position_parse(UciPosition *position, int argc, char *const argv[])
{
    UciPosition parsed;
    int index;

    if (position == NULL || argv == NULL || argc < 1) return 0;
    if (strcmp(argv[0], "startpos") == 0) {
        if (argc != 1 && (argc < 2 || strcmp(argv[1], "moves") != 0)) return 0;
        uci_position_start(&parsed);
        index = argc == 1 ? 1 : 2;
    } else if (strcmp(argv[0], "fen") == 0) {
        if (!parse_fen(&parsed, argc, argv)) return 0;
        index = argc;
    } else {
        return 0;
    }
    for (; index < argc; index++) {
        Move move;
        Move legal_moves[256];
        int legal_count;
        int legal = 0;
        if (!uci_move_from_uci(argv[index], parsed.black_to_move, &move)) return 0;
        legal_count = gen_legal_moves(&parsed.position, legal_moves);
        for (int legal_index = 0; legal_index < legal_count; legal_index++) {
            if (legal_moves[legal_index].i == move.i
                && legal_moves[legal_index].j == move.j
                && legal_moves[legal_index].prom == move.prom) {
                legal = 1;
                break;
            }
        }
        if (!legal) return 0;
        if (parsed.position.board[move.i] == 'P'
            || parsed.position.board[move.j] != '.') {
            parsed.halfmove_clock = 0;
        } else {
            parsed.halfmove_clock++;
        }
        parsed.position = make_move(parsed.position, move);
        if (parsed.black_to_move) parsed.fullmove_number++;
        parsed.black_to_move = !parsed.black_to_move;
    }
    *position = parsed;
    return 1;
}

void search_reset_stop(void) { stop_requested = 0; }
void search_request_stop(void) { stop_requested = 1; }

SearchResult search_position(const Position *position, const SearchLimits *limits)
{
    SearchResult result = {0};
    Move moves[256];
    int count;
    int max_depth;
    clock_t started = clock();

    if (position == NULL || limits == NULL) return result;
    count = gen_legal_moves(position, moves);
    if (count == 0) return result;
    result.best_move = moves[0];
    result.has_best_move = 1;
    max_depth = limits->depth > 0 ? limits->depth : 1;
    if (max_depth > 8) max_depth = 8;
    for (int depth = 1; depth <= max_depth && !stop_requested; depth++) {
        Move best = find_best_move(*position, depth);
        if (best.i >= 0) result.best_move = best;
        result.depth = depth;
        result.score = iterative_deepening(*position, depth);
        result.nodes++;
        if (limits->info_callback != NULL) limits->info_callback(&result, limits->info_context);
        if (limits->time_budget_ms >= 0
            && (clock() - started) * 1000 / CLOCKS_PER_SEC >= limits->time_budget_ms) break;
    }
    result.pv[0] = result.best_move;
    result.pv_length = 1;
    result.nps = result.nodes * CLOCKS_PER_SEC
        / (unsigned long long)(clock() - started + 1);
    return result;
}
