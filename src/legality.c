
#include "legality.h"
#include "board.h"
#include "movegen.h"
#include <stdlib.h>

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


int gen_legal_moves(const Position *pos, Move moves[]) {

    Move candidates[256];

    int count = gen_moves(pos, candidates);

    int legal_count = 0;

    for (int k = 0; k < count; k++) {

        Move current = candidates[k];

        // TODO 1:
        // If the destination contains the enemy king,
        // skip this candidate using continue.
        if (pos->board[current.j] == 'k') {
            continue;
        }

        // TODO 2:
        // Check if the moving piece is a king
        // AND it is moving two squares.
        if (pos->board[current.i] == 'K' && abs(current.j - current.i) == 2) {

            // TODO 3:
            // If it is a castling move, call
            // is_castling_safe().
            // Skip the candidate if it is unsafe.
            if (!is_castling_safe(pos, current)) {
                continue;
            }

        }
        // Existing king-safety filtering.

        Position new_pos = make_move(*pos, current);

        new_pos = rotate(new_pos, 0);

        if (!is_in_check(&new_pos)) {
            if (moves != NULL) {
                moves[legal_count] = current;
            }
            legal_count++;
        }
    }

    return legal_count;
}


int is_castling_safe(const Position *pos, Move move) {

    if (is_in_check(pos)) {
        return 0;
    }

    int direction;

    if (move.j > move.i) {
        direction = E;
    } else {
        direction = W;
    }

    int middle = move.i + direction;

    Position temp = *pos;

    temp.board[middle] = 'K';
    temp.board[move.i] = '.';

    if (is_square_attacked(&temp, middle)) {
        return 0;
    }

    return 1;
}

int is_checkmate(const Position *pos) {
    int has_king = 0;

    for (int square = 0; square < BOARD_SIZE; square++) {
        if (pos->board[square] == 'K') {
            has_king = 1;
            break;
        }
    }

    if (has_king && is_in_check(pos) && gen_legal_moves(pos, NULL) == 0) {
        return 1;
    }
    return 0;
}

int is_stalemate(const Position *pos) {
    int has_king = 0;

    for (int square = 0; square < BOARD_SIZE; square++) {
        if (pos->board[square] == 'K') {
            has_king = 1;
            break;
        }
    }

    if (has_king && !is_in_check(pos) && gen_legal_moves(pos, NULL) == 0) {
        return 1;
    }
    return 0;
}