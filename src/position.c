
#include "position.h"
#include "eval.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Position rotate(Position pos, int nullmove) {
    Position rotated = pos;

    // Reverse the board and swap piece colors
    for (int i = 0; i < 120; i++) {
        char p = pos.board[119 - i];

        if (isupper((unsigned char)p))
            rotated.board[i] = tolower((unsigned char)p);
        else if (islower((unsigned char)p))
            rotated.board[i] = toupper((unsigned char)p);
        else
            rotated.board[i] = p;
    }

    rotated.board[120] = '\0';

    // Reverse the evaluation perspective
    rotated.score = -pos.score;

    // Swap castling rights
    rotated.wc[0] = pos.bc[0];
    rotated.wc[1] = pos.bc[1];

    rotated.bc[0] = pos.wc[0];
    rotated.bc[1] = pos.wc[1];

    // Update special squares
    rotated.ep = (pos.ep && !nullmove) ? 119 - pos.ep : 0;
    rotated.kp = (pos.kp && !nullmove) ? 119 - pos.kp : 0;

    return rotated;
}



Position make_move(Position pos, Move move) {
    int i = move.i;
    int j = move.j;
    char prom = move.prom;

    char p = pos.board[i];

    Position next = pos;

    // A move resets the special squares unless it creates new ones.
    next.ep = 0;
    next.kp = 0;

    // Move the piece without changing the original position.
    next.board[j] = p;
    next.board[i] = '.';

    // Moving a rook from its original square loses that castling right.
    next.wc[0] = pos.wc[0] && (i != A1);
    next.wc[1] = pos.wc[1] && (i != H1);
    next.bc[0] = pos.bc[0] && (j != A8);
    next.bc[1] = pos.bc[1] && (j != H8);

    if (p == 'K') {
        // Moving the king permanently removes both castling rights.
        next.wc[0] = 0;
        next.wc[1] = 0;

        if (abs(j - i) == 2) {
            // Castling moves the rook as well as the king.
            next.kp = (i + j) / 2;
            next.board[j < i ? A1 : H1] = '.';
            next.board[next.kp] = 'R';
        }
    }

    if (p == 'P') {
        if (A8 <= j && j <= H8)
            next.board[j] = prom;

        if (j - i == 2 * N)
            next.ep = i + N;

        // En passant captures a pawn behind the destination square.
        if (j == pos.ep)
            next.board[j + S] = '.';
    }

    // TODO: Update score using value(pos, move).
    next.score = pos.score + value(&pos, move);

    return rotate(next, 0); // because we use capital letters for white and lowercase for black, we need to rotate the position after making a move to maintain the correct perspective.
}

Position make_null_move(Position pos){
    Position next = pos;
    next.ep = 0;
    next.kp = 0;
    return rotate(next, 1);
}