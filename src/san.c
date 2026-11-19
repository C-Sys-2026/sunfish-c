#include "san.h"

#include "legality.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int same_move(Move left, Move right)
{
    return left.i == right.i && left.j == right.j && left.prom == right.prom;
}

int move_to_san_for_side(const Position *pos, Move move, int black_to_move,
                         char *output, size_t size)
{
    char piece;
    char from_file;
    char to_file;
    int to_rank;
    int capture;
    int written;
    char disambiguation[3] = "";

    if (pos == NULL || output == NULL || size == 0
        || move.i < 0 || move.i >= 120 || move.j < 0 || move.j >= 120) {
        return 0;
    }

    piece = pos->board[move.i];
    if (strchr("PNBRQK", piece) == NULL) {
        return 0;
    }
    {
        int from_square = black_to_move ? 119 - move.i : move.i;
        int to_square = black_to_move ? 119 - move.j : move.j;
        from_file = (char)('a' + from_square % 10 - 1);
        to_file = (char)('a' + to_square % 10 - 1);
        to_rank = 10 - to_square / 10;
    }
    capture = strchr("pnbrqk", pos->board[move.j]) != NULL
        || (piece == 'P' && move.j == pos->ep && pos->ep != 0);

    if (piece == 'K' && abs(move.j - move.i) == 2) {
        written = snprintf(output, size, "%s",
                           (move.j > move.i) != black_to_move ? "O-O" : "O-O-O");
    } else if (piece != 'P') {
        Move legal_moves[256];
        int legal_count = gen_legal_moves(pos, legal_moves);
        int ambiguous = 0;
        int same_file = 0;
        int same_rank = 0;

        for (int index = 0; index < legal_count; index++) {
            Move other = legal_moves[index];
            if (other.i == move.i || other.j != move.j
                || pos->board[other.i] != piece
                || same_move(other, move)) {
                continue;
            }
            ambiguous = 1;
            if (other.i % 10 == move.i % 10) same_file = 1;
            if (other.i / 10 == move.i / 10) same_rank = 1;
        }
        if (ambiguous) {
            if (!same_file) {
                disambiguation[0] = from_file;
                disambiguation[1] = '\0';
            } else if (!same_rank) {
                int from_square = black_to_move ? 119 - move.i : move.i;
                disambiguation[0] = (char)('0' + 10 - from_square / 10);
                disambiguation[1] = '\0';
            } else {
                disambiguation[0] = from_file;
                {
                    int from_square = black_to_move ? 119 - move.i : move.i;
                    disambiguation[1] =
                        (char)('0' + 10 - from_square / 10);
                }
                disambiguation[2] = '\0';
            }
        }
        written = snprintf(output, size, "%c%s%s%c%d", piece, disambiguation,
                           capture ? "x" : "", to_file, to_rank);
    } else if (move.prom != '\0' && move.prom != ' ') {
        char promotion = (char)toupper((unsigned char)move.prom);
        if (promotion != 'Q' && promotion != 'R'
            && promotion != 'B' && promotion != 'N') {
            return 0;
        }
        written = capture
            ? snprintf(output, size, "%cx%c%d=%c", from_file, to_file,
                       to_rank, promotion)
            : snprintf(output, size, "%c%d=%c", to_file, to_rank, promotion);
    } else if (capture) {
        written = snprintf(output, size, "%cx%c%d", from_file, to_file,
                           to_rank);
    } else {
        written = snprintf(output, size, "%c%d", to_file, to_rank);
    }

    if (written < 0 || (size_t)written >= size) {
        return 0;
    }

    {
        Position next = make_move(*pos, move);
        if (is_in_check(&next)) {
            size_t used = (size_t)written;
            const char *suffix = gen_legal_moves(&next, NULL) == 0 ? "#" : "+";
            int suffix_written = snprintf(output + used, size - used, "%s",
                                          suffix);
            if (suffix_written < 0 || (size_t)suffix_written >= size - used) {
                return 0;
            }
            written += suffix_written;
        }
    }
    return written >= 0 && (size_t)written < size;
}

int move_to_san(const Position *pos, Move move, char *output, size_t size)
{
    return move_to_san_for_side(pos, move, 0, output, size);
}
