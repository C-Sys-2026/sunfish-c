
#include <assert.h>
#include "../third_party/munit/munit.h"
#define assert(expr) munit_assert(expr)
#include <stdio.h>
#include <string.h>

#include "../src/board.h"
#include "../src/position.h"
#include "../src/eval.h"


void test_promotion(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    // Put a pawn on a7, one move away from promotion.
    pos.board[31] = 'P';
    pos.board[81] = '.';

    Move move = {31, 21, 'Q'};
    Position next = make_move(pos, move);

    // After rotation, the promoted queen belongs to the opponent.
    assert(next.board[98] == 'q');

    printf("PASS: Pawn promotion creates a queen\n");
}

void test_position_castling(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    // Clear the squares between the king and kingside rook.
    pos.board[96] = '.';
    pos.board[97] = '.';
    pos.wc[1] = 1;

    Move move = {95, 97, '\0'};
    Position next = make_move(pos, move);

    // Both king and rook must move during castling.
    assert(next.board[22] == 'k');
    assert(next.board[23] == 'r');

    // Moving the king removes both castling rights.
    assert(next.bc[0] == 0);
    assert(next.bc[1] == 0);

    printf("PASS: Castling moves king and rook\n");
}

void test_en_passant(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    // White pawn on e5, black pawn on d5.
    pos.board[65] = '.';
    pos.board[55] = 'P';
    pos.board[54] = 'p';
    pos.board[85] = '.';
    pos.board[34] = '.';
    pos.ep = 44;

    Move move = {55, 44, '\0'};
    Position next = make_move(pos, move);

    // The capturing pawn moves diagonally onto an empty square.
    assert(next.board[75] == 'p');

    // The captured pawn must disappear.
    assert(next.board[65] == '.');

    printf("PASS: En passant removes the captured pawn\n");
}


void test_position_pawn_move(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    Move move = {85, 65, '\0'}; // e2 to e4
    Position next = make_move(pos, move);

    // The original board must remain unchanged.
    assert(pos.board[85] == 'P');

    // After rotation, Black is the side to move.
    assert(next.board[54] == 'p');
    assert(next.board[34] == '.');

    printf("PASS: Pawn move updates the board\n");
}

void test_move_score(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    Move move = {85, 65, '\0'};

    int expected = pst['P'][65] - pst['P'][85];

    assert(value(&pos, move) == expected);

    printf("PASS: Pawn move evaluation is correct\n");
}

void test_double_pawn_push(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    Move move = {85, 65, '\0'};
    Position next = make_move(pos, move);

    // The en passant square is mirrored after rotation.
    assert(next.ep == 44);

    printf("PASS: Double pawn push sets en passant square\n");
}

#if 0
int main(void) {
    pad_tables();
    init_king_tables();

    test_position_pawn_move();
    test_move_score();
    test_double_pawn_push();
    test_promotion();
    test_position_castling();
    test_en_passant();
    printf("All position tests passed!\n");
    return 0;
}
#endif
