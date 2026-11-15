

#include "../third_party/munit/munit.h"
#define assert(expr) munit_assert(expr)
#include <stdio.h>
#include <string.h>

#include "../src/board.h"
#include "../src/position.h"
#include "../src/movegen.h"


void test_empty_board(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    for (int i = 0; i < 120; i++) {
        if (strchr("PNBRQKpnbrqk", pos.board[i]))
            pos.board[i] = '.';
    }

    Move moves[256];
    int count = gen_moves(&pos, moves);

    assert(count == 0);
    printf("PASS: Empty board generates 0 moves\n");
}

void test_center_knight(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    for (int i = 0; i < 120; i++) {
        if (strchr("PNBRQKpnbrqk", pos.board[i]))
            pos.board[i] = '.';
    }

    pos.board[55] = 'N';

    Move moves[256];
    int count = gen_moves(&pos, moves);

    assert(count == 8);
    printf("PASS: Center knight generates 8 moves\n");
}


#if 0
int main(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    pos.wc[0] = 1;
    pos.wc[1] = 1;
    pos.bc[0] = 1;
    pos.bc[1] = 1;

    Move moves[256];

    int count = gen_moves(&pos, moves);

    assert(count == 20);

    printf("PASS: Starting position generates 20 moves\n");
    /*
    20 moves becuase in the starting position, each pawn can move one or two squares forward (16 moves), 
    and each knight can move to two different squares (4 moves). Therefore, the total number of legal moves in the starting position is 16 + 4 = 20.
    */
    test_empty_board();
    test_center_knight();
    return 0;
}
#endif
