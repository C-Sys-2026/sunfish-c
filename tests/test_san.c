
#include <assert.h>
#include "../third_party/munit/munit.h"
#define assert(expr) munit_assert(expr)
#include <stdio.h>
#include <string.h>

#include "../src/san.h"
#include "../src/board.h"

// Test 1: Basic pawn move (e2 -> e4)
void test_san_pawn_move(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    char san[32];
    Move move = {85, 65, '\0'};

    assert(move_to_san(&pos, move, san, sizeof(san)) == 1);
    assert(strcmp(san, "e4") == 0);

    printf("PASS: Pawn move\n");
}

// Test 2: Knight move (g1 -> f3)
void test_knight_move(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    char san[32];
    Move move = {97, 76, '\0'};

    assert(move_to_san(&pos, move, san, sizeof(san)) == 1);
    assert(strcmp(san, "Nf3") == 0);

    printf("PASS: Knight move\n");
}

// Test 3: Pawn capture (e4xd5)
void test_pawn_capture(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    pos.board[65] = 'P'; // White pawn on e4
    pos.board[54] = 'p'; // Black pawn on d5

    char san[32];
    Move move = {65, 54, '\0'};

    assert(move_to_san(&pos, move, san, sizeof(san)) == 1);
    assert(strcmp(san, "exd5") == 0);

    printf("PASS: Pawn capture\n");
}

// Test 4: Castling notation (e1 -> g1)
void test_san_castling(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    char san[32];
    Move move = {95, 97, '\0'};

    assert(move_to_san(&pos, move, san, sizeof(san)) == 1);
    assert(strcmp(san, "O-O") == 0);

    printf("PASS: Castling notation\n");
}

// Test 5: Reject insufficient output buffer
void test_small_buffer(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    char san[2];
    Move move = {97, 76, '\0'};

    assert(move_to_san(&pos, move, san, sizeof(san)) == 0);

    printf("PASS: Small buffer rejected\n");
}

#if 0
int main(void) {
    test_san_pawn_move();
    test_knight_move();
    test_pawn_capture();
    test_san_castling();
    test_small_buffer();

    printf("\nAll 5 basic SAN tests passed!\n");

    return 0;
}
#endif
