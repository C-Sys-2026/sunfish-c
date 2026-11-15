

#include "../third_party/munit/munit.h"
#define assert(expr) munit_assert(expr)
#include <stdint.h>
#include <stdio.h>

#include "../src/tt.h"

void test_store_and_probe(void) {
    tt_clear();

    uint64_t key = UINT64_C(123456789);
    int score = 0;

    tt_store(key, 4, 250, TT_EXACT);

    assert(tt_probe(key, 4, -1000, 1000, &score) == 1);
    assert(score == 250);

    printf("PASS: Stored entry can be retrieved\n");
}

void test_insufficient_depth(void) {
    tt_clear();

    uint64_t key = UINT64_C(987654321);
    int score = 0;

    tt_store(key, 2, 100, TT_EXACT);

    assert(tt_probe(key, 5, -1000, 1000, &score) == 0);

    printf("PASS: Insufficient-depth entry is rejected\n");
}

void test_clear(void) {
    tt_clear();

    uint64_t key = UINT64_C(55555);
    int score = 0;

    tt_store(key, 3, 75, TT_EXACT);
    tt_clear();

    assert(tt_probe(key, 3, -1000, 1000, &score) == 0);

    printf("PASS: Clearing invalidates stored entries\n");
}

void test_index_collision(void) {
    tt_clear();

    uint64_t first = UINT64_C(42);
    uint64_t second = first + TT_SIZE;
    int score = 0;

    // Both keys map to the same table index.
    tt_store(first, 3, 100, TT_EXACT);
    tt_store(second, 3, 200, TT_EXACT);

    assert(tt_probe(first, 3, -1000, 1000, &score) == 0);
    assert(tt_probe(second, 3, -1000, 1000, &score) == 1);
    assert(score == 200);

    printf("PASS: Index collisions are handled safely\n");
}

void test_tt_bounds(void) {
    int score = 0;
    uint64_t key = UINT64_C(12345);

    tt_clear();

    // Exact scores can be reused at sufficient depth.
    tt_store(key, 4, 150, TT_EXACT);

    assert(tt_probe(key, 4, 0, 200, &score) == 1);
    assert(score == 150);

    tt_clear();

    // A lower bound can establish a beta cutoff.
    tt_store(key, 4, 150, TT_LOWER);

    // TODO 1: Probe with alpha = 0, beta = 100.
    // Assert that it succeeds.
    assert(tt_probe(key, 4, 0, 100, &score) == 1);

    // TODO 2: Probe with alpha = 0, beta = 200.
    // Assert that it fails.
    assert(tt_probe(key, 4, 0, 200, &score) == 0);

    tt_clear();

    // An upper bound can establish an alpha cutoff.
    tt_store(key, 4, -50, TT_UPPER);

    // TODO 3: Probe with alpha = 0, beta = 200.
    // Assert that it succeeds.
    assert(tt_probe(key, 4, 0, 200, &score) == 1);
    // TODO 4: Probe with alpha = -100, beta = 200.
    // Assert that it fails.
    assert(tt_probe(key, 4, -100, 200, &score) == 0);

    printf("PASS: Transposition table respects score bounds\n");
}

#if 0
int main(void) {
    test_store_and_probe();
    test_insufficient_depth();
    test_clear();
    test_index_collision();
    test_tt_bounds();

    printf("\nAll transposition table tests passed!\n");

    return 0;
}
#endif
