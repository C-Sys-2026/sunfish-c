
#ifndef TT_H
#define TT_H

#include <stdint.h>

#define TT_SIZE 100000

typedef struct {
    uint64_t key;
    int depth;
    int score;
    int valid;
} TTEntry;

void tt_clear(void);
void tt_store(uint64_t key, int depth, int score);
int tt_probe(uint64_t key, int depth, int *score);

#endif
