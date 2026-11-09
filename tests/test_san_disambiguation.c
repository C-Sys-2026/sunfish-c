
#include <assert.h>
#include "../third_party/munit/munit.h"
#define assert(expr) munit_assert(expr)
#include <stdio.h>
#include <string.h>

#include "../src/san_disambiguation.h"
#include "../src/board.h"

void test_no_ambiguity(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    char output[3];

    Move move = {97, 76, '\0'}; // g1 -> f3

    assert(san_disambiguation(&pos, move, output) == 1);
    assert(strcmp(output, "") == 0);

    printf("PASS: No ambiguity\n");
}

void test_file_disambiguation(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    for (int i = 0; i < 120; i++) {
        if (pos.board[i] != ' ' &&
            pos.board[i] != '\n') {
            pos.board[i] = '.';
        }
    }

    pos.board[92] = 'N'; // b1
    pos.board[96] = 'N'; // f1

    char output[3];

    // Both knights can reach d2.
    Move move = {92, 84, '\0'};

    assert(san_disambiguation(&pos, move, output) == 1);
    assert(strcmp(output, "b") == 0);

    printf("PASS: File disambiguation\n");
}

void test_rank_disambiguation(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    for (int i = 0; i < 120; i++) {
        if (pos.board[i] != ' ' &&
            pos.board[i] != '\n') {
            pos.board[i] = '.';
        }
    }

    pos.board[91] = 'R'; // a1
    pos.board[71] = 'R'; // a3

    char output[3];

    Move move = {91, 81, '\0'}; // a1 -> a2

    assert(san_disambiguation(&pos, move, output) == 1);
    assert(strcmp(output, "1") == 0);

    printf("PASS: Rank disambiguation\n");
}

void test_both_file_and_rank(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    for (int i = 0; i < 120; i++) {
        if (pos.board[i] != ' ' &&
            pos.board[i] != '\n') {
            pos.board[i] = '.';
        }
    }

    // Three knights can reach d2.
    pos.board[92] = 'N'; // b1
    pos.board[96] = 'N'; // f1
    pos.board[72] = 'N'; // b3

    char output[3];

    Move move = {92, 84, '\0'}; // b1 -> d2

    assert(san_disambiguation(&pos, move, output) == 1);
    assert(strcmp(output, "b1") == 0);

    printf("PASS: Both file and rank disambiguation\n");
}

#if 0
int main(void) {
    test_no_ambiguity();
    test_file_disambiguation();
    test_rank_disambiguation();
    test_both_file_and_rank();

    printf("\nAll SAN disambiguation tests passed!\n");

    return 0;
}
#endif
