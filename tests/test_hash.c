
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/hash.h"
#include "../src/board.h"

void test_same_position_same_hash(void) {
    Position a = {0};
    Position b = {0};

    strcpy(a.board, initial);
    strcpy(b.board, initial);

    uint64_t hash_a = hash_position(&a);
    uint64_t hash_b = hash_position(&b);

    // Identical positions must produce identical hashes.
    assert(hash_a == hash_b);

    printf("PASS: Identical positions have identical hashes\n");
}

void test_different_positions_different_hashes(void) {
    Position a = {0};
    Position b = {0};

    strcpy(a.board, initial);
    strcpy(b.board, initial);

    // Remove the pawn on e2 from the second position.
    b.board[85] = '.';

    uint64_t hash_a = hash_position(&a);
    uint64_t hash_b = hash_position(&b);

    // Different piece placements should produce different hashes.
    assert(hash_a != hash_b);

    printf("PASS: Different positions have different hashes\n");
}

int main(void) {
    // Initialize the Zobrist keys before hashing positions.
    init_zobrist();

    test_same_position_same_hash();
    test_different_positions_different_hashes();

    printf("All Zobrist hashing tests passed!\n");

    return 0;
}
