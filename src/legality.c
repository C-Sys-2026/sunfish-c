
#include "legality.h"
#include "board.h"

int is_square_attacked(const Position *pos, int square) {

    int pawn_left = square + N + W;
    int pawn_right = square + N + E;

    if (pawn_left >= 0 && pawn_left < BOARD_SIZE) {
        if (pos->board[pawn_left] == 'p') {
            return 1;
        }
    }

    if (pawn_right >= 0 && pawn_right < BOARD_SIZE) {
        if (pos->board[pawn_right] == 'p') {
            return 1;
        }
    }

    int knight_dirs[] = {
        N + N + E,
        N + N + W,
        S + S + E,
        S + S + W,
        E + E + N,
        E + E + S,
        W + W + N,
        W + W + S
    };

    for (int k = 0; k < 8; k++) {

        int knight_square = square + knight_dirs[k];

        if (knight_square >= 0 &&
            knight_square < BOARD_SIZE) {

            if (pos->board[knight_square] == 'n') {
                return 1;
            }
        }
    }

    int sliding_dirs[] = {
        N, S, E, W,
        N + E, N + W,
        S + E, S + W
    };

    for (int k = 0; k < 8; k++) {

        int target = square + sliding_dirs[k];

        while (target >= 0 && target < BOARD_SIZE) {

            char piece = pos->board[target];

            if (piece == ' ' || piece == '\n') {
                break;
            }

            if (piece == '.') {
                target += sliding_dirs[k];
                continue;
            }

            if (k < 4 && (piece == 'r' || piece == 'q')) {
                return 1;
            }

            if (k >= 4 && (piece == 'b' || piece == 'q')) {
                return 1;
            }

            break;
        }
    }

    for (int k = 0; k < 8; k++) {

        int adjacent = square + sliding_dirs[k];

        if (adjacent < 0 || adjacent >= BOARD_SIZE) {
            continue;
        }

        if (pos->board[adjacent] == 'k') {
            return 1;
        }
    }

    return 0;
}


int is_in_check(const Position *pos) {

    for (int i = 0; i < BOARD_SIZE; i++) {

        // TODO 1: Check if board[i] contains
        // the current player's king.

        if (pos->board[i] == 'K') {
            // TODO 2: If it does, return the result
            // of is_square_attacked() for that square.
            return is_square_attacked(pos, i);
        }

    }

    // If no king exists, treat the position
    // as unsafe.
    return 1;
}
