#include "board.h"
#include "position.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const int directions[128][8] = {
    ['P'] = {N, N+N, N+W, N+E},
    ['N'] = {N+N+E, E+N+E, E+S+E, S+S+E, S+S+W, W+S+W, W+N+W, N+N+W},
    ['B'] = {N+W, N+E, S+W, S+E},
    ['R'] = {N, E, S, W},
    ['Q'] = {N, E, S, W, N+E, S+E, S+W, N+W},
    ['K'] = {N, E, S, W, N+E, S+E, S+W, N+W}
};


const int direction_count[128] = {
    ['P'] = 4,
    ['N'] = 8,
    ['B'] = 4,
    ['R'] = 4,
    ['Q'] = 8,
    ['K'] = 8
};


int gen_moves(const Position *pos, Move moves[]) {
    int count = 0;

    const char *board = pos->board; //pointer to existing board array in position struct

    for (int i = 0; i < 120; i++) {
        char p = board[i];

        if (strchr("PNBRQK", p) == NULL)
            continue;

        for (int k = 0; k < direction_count[(int)p]; k++) {
            int d = directions[(int)p][k];

            for (int j = i + d; ; j += d) {
                char q = board[j];

                // Stop at padding or friendly pieces
                if (strchr(" \nPNBRQK", q) != NULL)
                    break;

                if (p == 'P') {
                    if ((d == N || d == N + N) && q != '.')
                        break;

                    if (d == N + N &&
                        (i < A1 + N || board[i + N] != '.'))
                        break;

                    if ((d == N + W || d == N + E) &&
                        q == '.' &&
                        j != pos->ep &&
                        abs(j - pos->kp) > 1)
                        break;

                    // Pawn promotion
                    if (A8 <= j && j <= H8) {
                        moves[count++] = (Move){i, j, 'N'};
                        moves[count++] = (Move){i, j, 'B'};
                        moves[count++] = (Move){i, j, 'R'};
                        moves[count++] = (Move){i, j, 'Q'};
                        break;
                    }
                }

                moves[count++] = (Move){i, j, '\0'};

                // Non-sliding pieces and captures
                if (p == 'N' || p == 'K' || p == 'P' ||
                    strchr("pnbrqk", q) != NULL)
                    break;

                // Queenside castling
                if (i == A1 &&
                    board[j + E] == 'K' &&
                    pos->wc[0]) {
                    moves[count++] = (Move){j + E, j + W, '\0'};
                }

                // Kingside castling
                if (i == H1 &&
                    board[j + W] == 'K' &&
                    pos->wc[1]) {
                    moves[count++] = (Move){j + W, j + E, '\0'};
                }
            }
        }
    }

    return count;
}