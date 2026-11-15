

#include "../third_party/munit/munit.h"
#define assert(expr) munit_assert(expr)
#include <stdio.h>
#include <string.h>

#include "../src/hash.h"
#include "../src/board.h"


void test_castling_changes_hash(void) {
    Position a = {0};
    Position b = {0};

    strcpy(a.board, initial);
    strcpy(b.board, initial);

    a.wc[0] = 1;

    assert(hash_position(&a) != hash_position(&b));

    printf("PASS: Castling rights affect the hash\n");
}

void test_en_passant_changes_hash(void) {
    Position a = {0};
    Position b = {0};

    strcpy(a.board, initial);
    strcpy(b.board, initial);

    a.ep = 44;

    assert(hash_position(&a) != hash_position(&b));

    printf("PASS: En passant affects the hash\n");
}

void test_king_passant_changes_hash(void) {
    Position a = {0};
    Position b = {0};

    strcpy(a.board, initial);
    strcpy(b.board, initial);

    a.kp = 95;

    assert(hash_position(&a) != hash_position(&b));

    printf("PASS: King-passant affects the hash\n");
}


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

#if 0
int main(void) {
    // Initialize the Zobrist keys before hashing positions.
    init_zobrist();

    test_same_position_same_hash();
    test_different_positions_different_hashes();
    test_en_passant_changes_hash();
    test_king_passant_changes_hash();

    printf("All Zobrist hashing tests passed!\n");

    return 0;
}
#endif
