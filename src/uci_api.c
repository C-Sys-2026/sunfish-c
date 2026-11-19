#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#else
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#endif

#include "uci_api.h"

#include "board.h"
#include "eval.h"
#include "legality.h"
#include "movegen.h"
#include "search.h"
#include "tt.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static volatile int stop_requested;
static unsigned long long search_started_ms;
static int search_time_budget_ms = -1;

static unsigned long long monotonic_time_ms(void)
{
#ifdef _WIN32
    return (unsigned long long)GetTickCount64();
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        return 0;
    }
    return (unsigned long long)now.tv_sec * 1000ULL
        + (unsigned long long)now.tv_nsec / 1000000ULL;
#endif
}

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

static int parse_nonnegative_field(const char *text, int minimum, int *value)
{
    char *end = NULL;
    long parsed;

    if (text == NULL || *text == '\0') return 0;
    for (const char *cursor = text; *cursor != '\0'; cursor++) {
        if (!isdigit((unsigned char)*cursor)) return 0;
    }
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0'
        || parsed < minimum || parsed > INT_MAX) {
        return 0;
    }
    *value = (int)parsed;
    return 1;
}

static int parse_fen(UciPosition *result, int argc, char *const argv[])
{
    Position parsed = {0};
    int rank = 0;
    int file = 0;
    const char *board_text = argv[1];
    int halfmove;
    int fullmove;

    memcpy(parsed.board, empty, sizeof(empty));
    for (const char *cursor = board_text; *cursor != '\0'; cursor++) {
        if (*cursor == '/') {
            if (file != 8) return 0;
            rank++;
            file = 0;
        } else if (*cursor >= '1' && *cursor <= '8') {
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
        || (strcmp(argv[3], "-") != 0 && strlen(argv[3]) > 4)) {
        return 0;
    }
    if (strchr(parsed.board, 'K') == NULL || strchr(parsed.board, 'k') == NULL) {
        return 0;
    }
    for (const char *cursor = argv[3]; *cursor != '\0' && *cursor != '-';
         cursor++) {
        if (*cursor != 'K' && *cursor != 'Q'
            && *cursor != 'k' && *cursor != 'q') {
            return 0;
        }
        if ((cursor != argv[3] && strchr(argv[3], *cursor) != cursor)
            || (cursor == argv[3] && strchr(cursor + 1, *cursor) != NULL)) {
            return 0;
        }
    }
    if (strchr(argv[3], '-') != NULL && strcmp(argv[3], "-") != 0) {
        return 0;
    }
    parsed.wc[1] = strchr(argv[3], 'K') != NULL;
    parsed.wc[0] = strchr(argv[3], 'Q') != NULL;
    parsed.bc[1] = strchr(argv[3], 'k') != NULL;
    parsed.bc[0] = strchr(argv[3], 'q') != NULL;
    if (strcmp(argv[4], "-") != 0) {
        int ep = square_from_coordinate(argv[4]);
        if (ep < 0 || (argv[4][1] != '3' && argv[4][1] != '6')) return 0;
        parsed.ep = ep;
    }
    parsed.score = 0;
    if (!parse_nonnegative_field(argv[5], 0, &halfmove)
        || !parse_nonnegative_field(argv[6], 1, &fullmove)) {
        return 0;
    }
    result->halfmove_clock = halfmove;
    result->fullmove_number = fullmove;
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
        if (argc < 7 || !parse_fen(&parsed, 7, argv)) return 0;
        index = 7;
        if (argc > index) {
            if (strcmp(argv[index], "moves") != 0) return 0;
            index++;
        }
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

void search_reset_stop(void)
{
    stop_requested = 0;
    search_time_budget_ms = -1;
}

void search_request_stop(void) { stop_requested = 1; }

int search_stop_requested(void)
{
    unsigned long long now = monotonic_time_ms();
    if (!stop_requested && search_time_budget_ms >= 0
        && now >= search_started_ms
        && now - search_started_ms >= (unsigned long long)search_time_budget_ms) {
        stop_requested = 1;
    }
    return stop_requested != 0;
}

/* Search one complete root iteration. If a stop arrives mid-iteration, leave
 * the caller's last fully completed result untouched. */
static int search_root_iteration(const Position *position, int depth,
                                 Move *best_move, int *best_score)
{
    Move moves[256];
    int count = gen_legal_moves(position, moves);
    int alpha = -1000000;
    const int beta = 1000000;
    int best = -1000000;
    Move selected = {-1, -1, '\0'};

    if (count == 0) {
        return 0;
    }
    order_moves(position, moves, count);
    for (int index = 0; index < count; index++) {
        Position next;
        int score;
        if (search_stop_requested()) {
            return 0;
        }
        next = make_move(*position, moves[index]);
        score = -negamax_ab(next, depth - 1, -beta, -alpha);
        if (search_stop_requested()) {
            return 0;
        }
        if (score > best) {
            best = score;
            selected = moves[index];
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    *best_move = selected;
    *best_score = best;
    return 1;
}

SearchResult search_position(const Position *position, const SearchLimits *limits)
{
    SearchResult result = {0};
    Move moves[256];
    int count;
    int max_depth;
    unsigned long long started_ms = monotonic_time_ms();

    if (position == NULL || limits == NULL) return result;
    search_started_ms = started_ms;
    search_time_budget_ms = limits->time_budget_ms;
    count = gen_legal_moves(position, moves);
    if (count == 0) {
        search_time_budget_ms = -1;
        return result;
    }
    result.best_move = moves[0];
    result.has_best_move = 1;
    search_reset_node_count();
    max_depth = limits->depth > 0 ? limits->depth : 1;
    if (max_depth > UCI_MAX_PV) max_depth = UCI_MAX_PV;
    for (int depth = 1; depth <= max_depth && !search_stop_requested(); depth++) {
        Move best;
        int score;
        if (!search_root_iteration(position, depth, &best, &score)) {
            break;
        }
        result.best_move = best;
        result.score = score;
        result.depth = depth;
        result.nodes = search_get_node_count();
        result.hashfull = tt_hashfull();
        {
            unsigned long long elapsed_ms = monotonic_time_ms() - started_ms;
            result.nps = result.nodes * 1000ULL / (elapsed_ms == 0 ? 1 : elapsed_ms);
        }
        result.pv[0] = result.best_move;
        result.pv_length = 1;
        if (limits->info_callback != NULL) limits->info_callback(&result, limits->info_context);
    }
    result.nodes = search_get_node_count();
    result.hashfull = tt_hashfull();
    {
        unsigned long long elapsed_ms = monotonic_time_ms() - started_ms;
        result.nps = result.nodes * 1000ULL / (elapsed_ms == 0 ? 1 : elapsed_ms);
    }
    search_time_budget_ms = -1;
    return result;
}
