
#ifndef TT_H
#define TT_H

#include <stdint.h>

#define TT_SIZE 100000

typedef enum {
    TT_EXACT,
    TT_LOWER,
    TT_UPPER
} TTFlag;

typedef struct {
    uint64_t key;
    int depth;
    int score;
    TTFlag flag;
    int valid;
} TTEntry;

void tt_clear(void);

void tt_store(uint64_t key, int depth, int score, TTFlag flag);

int tt_probe(uint64_t key, int depth,
             int alpha, int beta, int *score);

#endif
