
#include "hash.h"

#include <stdint.h>

static uint64_t zobrist[128][120];
static uint64_t castling_keys[4];
static uint64_t ep_keys[120];
static uint64_t kp_keys[120];
static int zobrist_initialized;

// Generate deterministic pseudo-random 64-bit numbers.
static uint64_t next_random(void) {
    static uint64_t state = UINT64_C(0x9E3779B97F4A7C15);

    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;

    return state * UINT64_C(2685821657736338717);
}

void init_zobrist(void) {
    for (int p = 0; p < 128; p++) {
        for (int sq = 0; sq < 120; sq++) {
            zobrist[p][sq] = next_random();
        }
    }

    for (int i = 0; i < 4; i++) {
        castling_keys[i] = next_random();
    }

    for (int sq = 0; sq < 120; sq++) {
        ep_keys[sq] = next_random();
        kp_keys[sq] = next_random();
    }

    zobrist_initialized = 1;
}

uint64_t hash_position(const Position *pos) {
    if (!zobrist_initialized) {
        return 0;
    }

    uint64_t hash = 0;

    for (int sq = 0; sq < 120; sq++) {
        char piece = pos->board[sq];

        if (piece == '.' || piece == ' ' || piece == '\n') {
            continue;
        }

        hash ^= zobrist[(unsigned char)piece][sq];
    }

    // Castling rights are stored from the current player's perspective.
    if (pos->wc[0]) {
        hash ^= castling_keys[0];
    }

    if (pos->wc[1]) {
        hash ^= castling_keys[1];
    }

    if (pos->bc[0]) {
        hash ^= castling_keys[2];
    }

    if (pos->bc[1]) {
        hash ^= castling_keys[3];
    }

    if (pos->ep != 0) {
        hash ^= ep_keys[pos->ep];
    }

    if (pos->kp != 0) {
        hash ^= kp_keys[pos->kp];
    }

    return hash;
}
