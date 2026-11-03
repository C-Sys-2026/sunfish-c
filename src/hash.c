
#include "hash.h"

#include <stdint.h>

static uint64_t zobrist[128][120];

// Generate deterministic pseudo-random 64-bit numbers.
static uint64_t next_random(void) {
    static uint64_t state = UINT64_C(0x9E3779B97F4A7C15);

    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;

    return state * UINT64_C(2685821657736338717);
}

void init_zobrist(void) {
    // Assign a unique pseudo-random key to each piece/square combination.
    for (int p = 0; p < 128; p++) {
        for (int sq = 0; sq < 120; sq++) {
            zobrist[p][sq] = next_random();
        }
    }
}

uint64_t hash_position(const Position *pos) {
    uint64_t hash = 0;

    for (int sq = 0; sq < 120; sq++) {
        char piece = pos->board[sq];

        // Padding and empty squares do not contribute to the hash.
        if (piece == '.' || piece == ' ' || piece == '\n') {
            continue;
        }

        hash ^= zobrist[(unsigned char)piece][sq];
    }

    return hash;
}
