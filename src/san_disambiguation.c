
#include "san_disambiguation.h"
#include "movegen.h"
#include <stddef.h>

int san_disambiguation(const Position *pos, Move move,
                       char output[3]) {

    if (pos == NULL || output == NULL) {
        return 0;
    }

    output[0] = '\0';

    char piece = pos->board[move.i];

    if (piece == 'P' || piece == 'K') {
        return 1;
    }

    Move moves[256];
    int count = gen_moves(pos, moves);

    int same_file = 0;
    int same_rank = 0;
    int ambiguous = 0;

    for (int k = 0; k < count; k++) {

        Move other = moves[k];

        if (other.j != move.j ||
            other.i == move.i ||
            pos->board[other.i] != piece) {
            continue;
        }

        ambiguous = 1;

        if (other.i % 10 == move.i % 10) {
            same_file = 1;
        }

        if (other.i / 10 == move.i / 10) {
            same_rank = 1;
        }
    }

    if (!ambiguous) {
        return 1;
    }

    char file = 'a' + (move.i % 10) - 1;
    char rank = '0' + 10 - (move.i / 10);

    if (!same_file) {
        output[0] = file;
        output[1] = '\0';
    } else if (!same_rank) {
        output[0] = rank;
        output[1] = '\0';
    } else {
        output[0] = file;
        output[1] = rank;
        output[2] = '\0';
    }

    return 1;
}
