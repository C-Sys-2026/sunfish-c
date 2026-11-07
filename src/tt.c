
#include "tt.h"

static TTEntry table[TT_SIZE];

void tt_clear(void) {
    // Invalidate all cached search results.
    for (int i = 0; i < TT_SIZE; i++) {
        table[i].valid = 0;
    }
}

void tt_store(uint64_t key, int depth, int score) {
    // Map the 64-bit hash to a table slot.
    int index = (int)(key % TT_SIZE);

    table[index] = (TTEntry){
        key,
        depth,
        score,
        1
    };
}

int tt_probe(uint64_t key, int depth, int *score) {
    int index = (int)(key % TT_SIZE);

    TTEntry *entry = &table[index];

    // Only reuse results for the same position at sufficient depth.
    if (entry->valid &&
        entry->key == key &&
        entry->depth >= depth) {

        *score = entry->score;
        return 1;
    }

    return 0;
}
