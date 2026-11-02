
#include "position.h"

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
