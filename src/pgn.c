#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "pgn.h"

#include "legality.h"
#include "san.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>
#define PGN_MKDIR(path) _mkdir(path)
static FILE *open_exclusive_file(const char *path)
{
    int descriptor = _sopen(path, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY,
                            _SH_DENYRW, _S_IREAD | _S_IWRITE);
    FILE *file;
    if (descriptor == -1) return NULL;
    file = _fdopen(descriptor, "w");
    if (file == NULL) {
        int saved_errno = errno;
        _close(descriptor);
        remove(path);
        errno = saved_errno;
    }
    return file;
}
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#define PGN_MKDIR(path) mkdir(path, 0777)
static FILE *open_exclusive_file(const char *path)
{
    int descriptor = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
    FILE *file;
    if (descriptor == -1) return NULL;
    file = fdopen(descriptor, "w");
    if (file == NULL) {
        int saved_errno = errno;
        close(descriptor);
        remove(path);
        errno = saved_errno;
    }
    return file;
}
#endif

static int valid_result(const char *result)
{
    return result != NULL
        && (strcmp(result, "1-0") == 0 || strcmp(result, "0-1") == 0
            || strcmp(result, "1/2-1/2") == 0);
}

static int is_start_position(const UciPosition *root);

static int get_local_time(time_t timestamp, struct tm *output)
{
    struct tm *value = localtime(&timestamp);
    if (value == NULL) return 0;
    *output = *value;
    return 1;
}

void pgn_game_clear(PgnGame *game)
{
    if (game != NULL) {
        memset(game, 0, sizeof(*game));
    }
}

int pgn_game_set_position(PgnGame *game, const UciPosition *root,
                          int move_count, char *const moves[])
{
    PgnGame next;

    if (game == NULL || root == NULL || move_count < 0
        || move_count > PGN_MAX_MOVES || (move_count > 0 && moves == NULL)) {
        return 0;
    }
    memset(&next, 0, sizeof(next));
    next.root = *root;
    next.final_position = *root;
    next.move_count = move_count;
    next.active = 1;

    for (int index = 0; index < move_count; index++) {
        Move move;
        Move legal_moves[256];
        char piece;
        int capture;
        int legal_count;
        int legal = 0;

        if (!uci_move_from_uci(moves[index],
                               next.final_position.black_to_move, &move)) {
            return 0;
        }
        legal_count = gen_legal_moves(&next.final_position.position, legal_moves);
        for (int legal_index = 0; legal_index < legal_count; legal_index++) {
            if (legal_moves[legal_index].i == move.i
                && legal_moves[legal_index].j == move.j
                && legal_moves[legal_index].prom == move.prom) {
                legal = 1;
                break;
            }
        }
        if (!legal
            || !move_to_san_for_side(&next.final_position.position, move,
                                      next.final_position.black_to_move,
                                      next.san[index],
                                      sizeof(next.san[index]))) {
            return 0;
        }
        piece = next.final_position.position.board[move.i];
        capture = next.final_position.position.board[move.j] != '.'
            || (piece == 'P' && move.j == next.final_position.position.ep
                && next.final_position.position.ep != 0);
        next.final_position.position =
            make_move(next.final_position.position, move);
        if (next.final_position.black_to_move) {
            next.final_position.fullmove_number++;
        }
        next.final_position.black_to_move = !next.final_position.black_to_move;
        if (piece == 'P' || capture) {
            next.final_position.halfmove_clock = 0;
        } else {
            next.final_position.halfmove_clock++;
        }
    }

    next.last_export_fingerprint = game->last_export_fingerprint;
    *game = next;
    return 1;
}

int pgn_game_terminal_result(const PgnGame *game, char result[8])
{
    const Position *position;
    int legal_count;

    if (game == NULL || result == NULL || !game->active) {
        return 0;
    }
    position = &game->final_position.position;
    legal_count = gen_legal_moves(position, NULL);
    if (legal_count != 0) {
        return 0;
    }
    if (!is_in_check(position)) {
        strcpy(result, "1/2-1/2");
    } else {
        strcpy(result, game->final_position.black_to_move ? "1-0" : "0-1");
    }
    return 1;
}

static Position fen_position(const UciPosition *root)
{
    return root->black_to_move ? rotate(root->position, 0) : root->position;
}

static int append_text(char *output, size_t size, size_t *used,
                       const char *format, ...)
{
    va_list args;
    int written;

    if (*used >= size) {
        return 0;
    }
    va_start(args, format);
    written = vsnprintf(output + *used, size - *used, format, args);
    va_end(args);
    if (written < 0 || (size_t)written >= size - *used) {
        return 0;
    }
    *used += (size_t)written;
    return 1;
}

static int make_fen(const UciPosition *root, char output[128])
{
    Position position = fen_position(root);
    size_t used = 0;
    int empty_count;
    int written;
    char castling[5];
    int castle_count = 0;

    for (int rank = 0; rank < 8; rank++) {
        empty_count = 0;
        for (int file = 0; file < 8; file++) {
            char piece = position.board[21 + rank * 10 + file];
            if (piece == '.') {
                empty_count++;
            } else {
                if (empty_count > 0
                    && !append_text(output, 128, &used, "%d", empty_count)) {
                    return 0;
                }
                empty_count = 0;
                if (!append_text(output, 128, &used, "%c", piece)) return 0;
            }
        }
        if (empty_count > 0
            && !append_text(output, 128, &used, "%d", empty_count)) {
            return 0;
        }
        if (rank != 7 && !append_text(output, 128, &used, "/")) return 0;
    }

    if (position.wc[1]) castling[castle_count++] = 'K';
    if (position.wc[0]) castling[castle_count++] = 'Q';
    if (position.bc[1]) castling[castle_count++] = 'k';
    if (position.bc[0]) castling[castle_count++] = 'q';
    castling[castle_count] = '\0';

    if (!append_text(output, 128, &used, " %c %s ",
                     root->black_to_move ? 'b' : 'w',
                     castle_count == 0 ? "-" : castling)) {
        return 0;
    }
    if (position.ep == 0) {
        if (!append_text(output, 128, &used, "-")) return 0;
    } else {
        int square = position.ep;
        if (!append_text(output, 128, &used, "%c%d",
                         (char)('a' + square % 10 - 1),
                         10 - square / 10)) {
            return 0;
        }
    }
    written = snprintf(output + used, 128 - used, " %d %d",
                       root->halfmove_clock, root->fullmove_number);
    return written >= 0 && (size_t)written < 128 - used;
}

static unsigned long long fingerprint(const PgnGame *game, const char *result)
{
    unsigned long long hash = 1469598103934665603ULL;
#define HASH_INT(value) \
    do { \
        unsigned int hash_value = (unsigned int)(value); \
        for (int hash_byte = 0; hash_byte < 4; hash_byte++) { \
            hash = (hash ^ (unsigned char)(hash_value >> (hash_byte * 8))) \
                * 1099511628211ULL; \
        } \
    } while (0)
    for (size_t index = 0; index < sizeof(game->root.position.board); index++) {
        hash = (hash ^ (unsigned char)game->root.position.board[index])
            * 1099511628211ULL;
    }
    HASH_INT(game->root.position.wc[0]);
    HASH_INT(game->root.position.wc[1]);
    HASH_INT(game->root.position.bc[0]);
    HASH_INT(game->root.position.bc[1]);
    HASH_INT(game->root.position.ep);
    HASH_INT(game->root.position.kp);
    HASH_INT(game->root.black_to_move);
    HASH_INT(game->root.halfmove_clock);
    HASH_INT(game->root.fullmove_number);
    for (int index = 0; index < game->move_count; index++) {
        for (const unsigned char *cursor =
                 (const unsigned char *)game->san[index];
             *cursor != '\0'; cursor++) {
            hash = (hash ^ *cursor) * 1099511628211ULL;
        }
        hash = (hash ^ 0xffU) * 1099511628211ULL;
    }
    for (const unsigned char *cursor = (const unsigned char *)result;
         *cursor; cursor++) {
        hash = (hash ^ *cursor) * 1099511628211ULL;
    }
#undef HASH_INT
    return hash == 0 ? 1 : hash;
}

static int write_game(FILE *file, const PgnGame *game, const char *result)
{
    time_t now = time(NULL);
    struct tm time_value;
    struct tm *local = NULL;
    char date[16];
    char fen[128];

    if (!get_local_time(now, &time_value)) return 0;
    local = &time_value;
    if (strftime(date, sizeof(date), "%Y.%m.%d", local) == 0) return 0;

    fprintf(file, "[Event \"Sunfish-C game\"]\n"
                  "[Site \"?\"]\n"
                  "[Date \"%s\"]\n"
                  "[Round \"?\"]\n"
                  "[White \"White\"]\n"
                  "[Black \"Black\"]\n"
                  "[Result \"%s\"]\n", date, result);
    if (!is_start_position(&game->root)) {
        if (!make_fen(&game->root, fen)) return 0;
        fprintf(file, "[SetUp \"1\"]\n[FEN \"%s\"]\n", fen);
    }
    fputc('\n', file);

    for (int index = 0; index < game->move_count; index++) {
        int black = game->root.black_to_move ^ (index & 1);
        int move_number = game->root.fullmove_number + (index
            + game->root.black_to_move) / 2;
        if (!black) {
            if (index > 0) fputc(' ', file);
            fprintf(file, "%d. ", move_number);
        } else if (index == 0) {
            fprintf(file, "%d... ", move_number);
        } else {
            fputc(' ', file);
        }
        fputs(game->san[index], file);
    }
    if (game->move_count > 0) fputc(' ', file);
    fprintf(file, "%s\n", result);
    return !ferror(file);
}

static int is_start_position(const UciPosition *root)
{
    UciPosition start;
    uci_position_start(&start);
    return !root->black_to_move && root->halfmove_clock == 0
        && root->fullmove_number == 1
        && memcmp(root->position.board, start.position.board,
                  sizeof(root->position.board)) == 0
        && root->position.wc[0] == start.position.wc[0]
        && root->position.wc[1] == start.position.wc[1]
        && root->position.bc[0] == start.position.bc[0]
        && root->position.bc[1] == start.position.bc[1]
        && root->position.ep == 0;
}

int pgn_game_export_to_file(PgnGame *game, const char *result,
                            const char *path)
{
    unsigned long long current_fingerprint;
    FILE *file;

    if (game == NULL || !game->active || path == NULL
        || !valid_result(result)) {
        return 0;
    }
    current_fingerprint = fingerprint(game, result);
    if (current_fingerprint == game->last_export_fingerprint) {
        return 1;
    }
    file = fopen(path, "w");
    if (file == NULL) {
        return 0;
    }
    {
        int written = write_game(file, game, result);
        int closed = fclose(file) == 0;
        if (!written || !closed) {
            remove(path);
            return 0;
        }
    }
    game->last_export_fingerprint = current_fingerprint;
    return 1;
}

int pgn_game_export(PgnGame *game, const char *result)
{
    unsigned long long current_fingerprint;
    time_t now;
    struct tm local_time;
    char timestamp[32];
    char path[128];
    int suffix = 0;

    if (game == NULL || !game->active || !valid_result(result)) {
        return 0;
    }
    current_fingerprint = fingerprint(game, result);
    if (current_fingerprint == game->last_export_fingerprint) {
        return 1;
    }
    if (PGN_MKDIR("games") != 0 && errno != EEXIST) {
        return 0;
    }
    now = time(NULL);
    if (!get_local_time(now, &local_time)) return 0;
    if (strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S",
                 &local_time) == 0) {
        return 0;
    }
    for (;;) {
        FILE *file;
        int written = suffix == 0
            ? snprintf(path, sizeof(path), "games/%s.txt", timestamp)
            : snprintf(path, sizeof(path), "games/%s_%02d.txt",
                       timestamp, suffix);
        if (written < 0 || (size_t)written >= sizeof(path)) return 0;

        file = open_exclusive_file(path);
        if (file == NULL) {
            if (errno == EEXIST) {
                suffix++;
                continue;
            }
            return 0;
        }
        {
            int wrote_game = write_game(file, game, result);
            int closed = fclose(file) == 0;
            if (!wrote_game || !closed) {
                remove(path);
                return 0;
            }
        }
        game->last_export_fingerprint = current_fingerprint;
        return 1;
    }
}