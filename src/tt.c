
#include "tt.h"

static TTEntry table[TT_SIZE];

void tt_clear(void) {
    // Invalidate all cached search results.
    for (int i = 0; i < TT_SIZE; i++) {
        table[i].valid = 0;
    }
}



int tt_probe(uint64_t key, int depth,
             int alpha, int beta, int *score) {

    int index = (int)(key % TT_SIZE);
    TTEntry *entry = &table[index];

    // Reject invalid entries, hash mismatches, or insufficient depth.
    if (!entry->valid ||
        entry->key != key ||
        entry->depth < depth) {
        return 0;
    }

    // Exact scores can always be reused.
    if (entry->flag == TT_EXACT) {
        *score = entry->score;
        return 1;
    }

    // Lower bounds can produce a beta cutoff.
    if (entry->flag == TT_LOWER && entry->score >= beta) {
        *score = entry->score;
        return 1;
    }

    // Upper bounds can produce an alpha cutoff.
    if (entry->flag == TT_UPPER && entry->score <= alpha) {
        *score = entry->score;
        return 1;
    }

    // The stored bound cannot resolve this search window.
    return 0;
}

void tt_store(uint64_t key, int depth, int score, TTFlag flag, Move best_move){
    int index = (int)(key % TT_SIZE);

    // TODO 1: Store all five fields, including the flag.
    // Hint: Use a TTEntry compound literal.
    table[index] = (TTEntry) {
        .key = key,
        .depth = depth,
        .score = score,
        .flag = flag,
        .valid = 1,
        .best_move = best_move
    };
}

Move tt_get_best_move(uint64_t key){
    int index = (int)(key % TT_SIZE);
    TTEntry *entry = &table[index];

    // Reject invalid entries or hash mismatches.
    if (!entry->valid || entry->key != key) {
        return (Move){-1, -1, '\0'}; // Return an invalid move.
    }

    return entry->best_move;
}