
#include "../third_party/munit/munit.h"
#define assert(expr) munit_assert(expr)

#include <stdio.h>
#include <string.h>

#include "../src/legality.h"
#include "../src/board.h"

static Position empty_position(void) {
    Position pos = {0};

    strcpy(pos.board, initial);

    for (int i = 0; i < BOARD_SIZE; i++) {
        if (pos.board[i] != ' ' &&
            pos.board[i] != '\n') {
            pos.board[i] = '.';
        }
    }

    return pos;
}


static int contains_move(Move moves[], int count, int from, int to){
    for (size_t i = 0; i < count; i++)
    {
        if(moves[i].i==from && moves[i].j ==to){
            return 1;
        }
    }
    return 0;
}

void test_pawn_attack(void) {
    Position pos = empty_position();

    pos.board[54] = 'p';

    assert(is_square_attacked(&pos, 65) == 1);

    pos.board[54] = '.';
    pos.board[56] = 'p';

    assert(is_square_attacked(&pos, 65) == 1);

    printf("PASS: Pawn attacks detected\n");
}

void test_pawn_direction(void) {
    Position pos = empty_position();

    pos.board[74] = 'p';

    assert(is_square_attacked(&pos, 65) == 0);

    pos.board[74] = '.';
    pos.board[55] = 'p';

    assert(is_square_attacked(&pos, 65) == 0);

    printf("PASS: Pawn attack direction is correct\n");
}

void test_knight_attack(void) {
    Position pos = empty_position();

    pos.board[46] = 'n';

    assert(is_square_attacked(&pos, 65) == 1);

    printf("PASS: Knight attack detected\n");
}

void test_rook_attack(void) {
    Position pos = empty_position();

    pos.board[25] = 'r';

    assert(is_square_attacked(&pos, 65) == 1);

    printf("PASS: Rook attack detected\n");
}

void test_bishop_attack(void) {
    Position pos = empty_position();

    pos.board[32] = 'b';

    assert(is_square_attacked(&pos, 65) == 1);

    printf("PASS: Bishop attack detected\n");
}

void test_queen_attack(void) {
    Position pos = empty_position();

    pos.board[25] = 'q';

    assert(is_square_attacked(&pos, 65) == 1);

    pos.board[25] = '.';
    pos.board[32] = 'q';

    assert(is_square_attacked(&pos, 65) == 1);

    printf("PASS: Queen attacks detected\n");
}

void test_king_attack(void) {
    Position pos = empty_position();

    pos.board[55] = 'k';

    assert(is_square_attacked(&pos, 65) == 1);

    pos.board[55] = '.';
    pos.board[45] = 'k';

    assert(is_square_attacked(&pos, 65) == 0);

    printf("PASS: King attack distance is correct\n");
}

void test_blocked_attack(void) {
    Position pos = empty_position();

    pos.board[25] = 'r';
    pos.board[45] = 'P';

    assert(is_square_attacked(&pos, 65) == 0);

    pos.board[45] = '.';

    assert(is_square_attacked(&pos, 65) == 1);

    printf("PASS: Sliding attacks respect blockers\n");
}

void test_in_check(void) {
    Position pos = empty_position();

    pos.board[65] = 'K';
    pos.board[25] = 'r';

    assert(is_in_check(&pos) == 1);

    printf("PASS: King in check detected\n");
}

void test_not_in_check(void) {
    Position pos = {0};

    strcpy(pos.board, initial);

    assert(is_in_check(&pos) == 0);

    printf("PASS: Initial position is not in check\n");
}

void test_missing_king(void) {
    Position pos = empty_position();

    assert(is_in_check(&pos) == 1);

    printf("PASS: Missing king treated as unsafe\n");
}

void test_board_boundary(void) {
    Position pos = empty_position();

    assert(is_square_attacked(&pos, A8) == 0);
    assert(is_square_attacked(&pos, H1) == 0);

    printf("PASS: Board boundary handled correctly\n");
}

void test_initial_legal_moves(void){
    // init a position
    Position pos = empty_position();
    strcpy(pos.board, initial);
    pos.wc[0] = 1;
    pos.wc[1] = 1;
    pos.bc[0] = 1;
    pos.bc[1] = 1;
    Move moves[256];
    int count = gen_legal_moves(&pos, moves);
    assert(count == 20);
    printf("PASS: Initial position has 20 legal moves\n");
}

void test_pinned_rook(void){
    Position pos = empty_position();
    pos.board[95] = 'K';
    pos.board[85] = 'R';
    pos.board[25] = 'r';
    pos.board[21] = 'k';

    Move moves[256];
    int count = gen_legal_moves(&pos, moves);

    assert(contains_move(moves, count, 85, 84) == 0);
    assert(contains_move(moves, count, 85, 75) == 1);
    printf("PASS: Pinned rook cannot move\n");
}

#if 0
int main(void) {
    test_pawn_attack();
    test_pawn_direction();
    test_knight_attack();
    test_rook_attack();
    test_bishop_attack();
    test_queen_attack();
    test_king_attack();
    test_blocked_attack();
    test_in_check();
    test_not_in_check();
    test_missing_king();
    test_board_boundary();
    test_initial_legal_moves();
    test_pinned_rook();
    printf("All legality tests passed!\n");

    return 0;
}
#endif
