#include "eval.h"
#include "position.h"
#include "board.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const int piece[128] = {
    ['P'] = 100,
    ['N'] = 280,
    ['B'] = 320,
    ['R'] = 479,
    ['Q'] = 929,
    ['K'] = 60000
};

const int original_pst[128][64] = {
    ['P'] = {0,   0,   0,   0,   0,   0,   0,   0,
            78,  83,  86,  73, 102,  82,  85,  90,
             7,  29,  21,  44,  40,  31,  44,   7,
           -17,  16,  -2,  15,  14,   0,  15, -13,
           -26,   3,  10,   9,   6,   1,   0, -23,
           -22,   9,   5, -11, -10,  -2,   3, -19,
           -31,   8,  -7, -37, -36, -14,   3, -31,
             0,   0,   0,   0,   0,   0,   0,   0},
    ['N'] = {-66, -53, -75, -75, -10, -55, -58, -70,
            -3,  -6, 100, -36,   4,  62,  -4, -14,
            10,  67,   1,  74,  73,  27,  62,  -2,
            24,  24,  45,  37,  33,  41,  25,  17,
            -1,   5,  31,  21,  22,  35,   2,   0,
           -18,  10,  13,  22,  18,  15,  11, -14,
           -23, -15,   2,   0,   2,   0, -23, -20,
           -74, -23, -26, -24, -19, -35, -22, -69},
    ['B'] = {-59, -78, -82, -76, -23,-107, -37, -50,
           -11,  20,  35, -42, -39,  31,   2, -22,
            -9,  39, -32,  41,  52, -10,  28, -14,
            25,  17,  20,  34,  26,  25,  15,  10,
            13,  10,  17,  23,  17,  16,   0,   7,
            14,  25,  24,  15,   8,  25,  20,  15,
            19,  20,  11,   6,   7,   6,  20,  16,
            -7,   2, -15, -12, -14, -15, -10, -10},
    ['R'] = { 35,  29,  33,   4,  37,  33,  56,  50,
            55,  29,  56,  67,  55,  62,  34,  60,
            19,  35,  28,  33,  45,  27,  25,  15,
             0,   5,  16,  13,  18,  -4,  -9,  -6,
           -28, -35, -16, -21, -13, -29, -46, -30,
           -42, -28, -42, -25, -25, -35, -26, -46,
           -53, -38, -31, -26, -29, -43, -44, -53,
           -30, -24, -18,   5,  -2, -18, -31, -32},
    ['Q'] = {6,   1,  -8,-104,  69,  24,  88,  26,
            14,  32,  60, -10,  20,  76,  57,  24,
            -2,  43,  32,  60,  72,  63,  43,   2,
             1, -16,  22,  17,  25,  20, -13,  -6,
           -14, -15,  -2,  -5,  -1, -10, -20, -22,
           -30,  -6, -13, -11, -16, -11, -16, -27,
           -36, -18,   0, -19, -15, -15, -21, -38,
           -39, -30, -31, -13, -31, -36, -34, -42},
    ['K'] = { 4,  54,  47, -99, -99,  60,  83, -62,
           -32,  10,  55,  56,  56,  55,  10,   3,
           -62,  12, -57,  44, -67,  28,  37, -31,
           -55,  50,  11,  -4, -19,  13,   0, -49,
           -55, -43, -52, -28, -51, -47,  -8, -50,
           -47, -42, -43, -79, -64, -32, -29, -32,
            -4,   3, -14, -50, -57, -18,  13,   4,
            17,  30,  -3, -14,   6,  -1,  40,  18}
};

int pst[128][120];

int K_MID[120];
int K_END[120];


void pad_tables(void) {
    char pieces[] = "PNBRQK";

    for (int p = 0; p < 6; p++) {
        char k = pieces[p];

        for (int i = 0; i < 64; i++) {
            int row = i / 8;
            int col = i % 8;

            int new_index = 21 + row * 10 + col;

            pst[(int)k][new_index] =
                original_pst[(int)k][i] + piece[(int)k];
        }
    }
}


void init_king_tables(void) {
    // Save the original middlegame king table
    memcpy(K_MID, pst['K'], sizeof(K_MID)); // we cant directly assign arrays in C, so we use memcpy

    // Generate the endgame king table
    for (int i = 0; i < 120; i++) {
        K_END[i] = piece['K'] + 70
            - 10 * (
                abs(2 * (i / 10) - 11)
                + abs(2 * (i % 10) - 9)
            );
    }

    /*
    * ENDGAME KING PIECE-SQUARE TABLE (8x8)
    *
    *       a    b    c    d    e    f    g    h
    *   +-----------------------------------------+
    * 8 | -70  -50  -30  -10  -10  -30  -50  -70 |
    * 7 | -50  -30  -10   10   10  -10  -30  -50 |
    * 6 | -30  -10   10   30   30   10  -10  -30 |
    * 5 | -10   10   30   50   50   30   10  -10 |
    * 4 | -10   10   30   50   50   30   10  -10 |
    * 3 | -30  -10   10   30   30   10  -10  -30 |
    * 2 | -50  -30  -10   10   10  -10  -30  -50 |
    * 1 | -70  -50  -30  -10  -10  -30  -50  -70 |
    *   +-----------------------------------------+
    *
    * Actual value = 60000 + positional bonus
    *
    * The king is encouraged to move toward the center
    * during the endgame.
    *
    * Center squares (d4, e4, d5, e5): +50
    * Corner squares (a1, h1, a8, h8): -70
    *
    * Generated using Manhattan distance from the board center.
    */
}


int value(const Position *pos, Move move) {
    int i = move.i;
    int j = move.j;
    char prom = move.prom;

    char p = pos->board[i];
    char q = pos->board[j];

    // Moving a piece changes its positional value.
    int score = pst[(int)p][j] - pst[(int)p][i];

    // Capturing an enemy piece gains its material and positional value.
    if (islower((unsigned char)q)) {
        score += pst[toupper((unsigned char)q)][119 - j];
    }

    // Account for a king exposed during castling.
    if (abs(j - pos->kp) < 2) {
        score += pst['K'][119 - j];
    }

    // Castling also changes the rook's position.
    if (p == 'K' && abs(j - i) == 2) {
        score += pst['R'][(i + j) / 2];
        score -= pst['R'][j < i ? A1 : H1];
    }

    if (p == 'P') {
        // Promotion replaces the pawn with the promoted piece.
        if (A8 <= j && j <= H8) {
            score += pst[(int)prom][j] - pst['P'][j];
        }

        // En passant captures a pawn on a different square.
        if (j == pos->ep) {
            score += pst['P'][119 - (j + S)];
        }
    }

    return score;
}
