
#ifndef TT_H
#define TT_H

#include <stdint.h>
#include "position.h"

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
    Move best_move;
} TTEntry;

void tt_clear(void);

int tt_probe(uint64_t key, int depth,
             int alpha, int beta, int *score);

void tt_store(uint64_t key, int depth, int score,
              TTFlag flag, Move best_move);

Move tt_get_best_move(uint64_t key);
#endif
