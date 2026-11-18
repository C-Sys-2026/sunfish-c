
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "position.h"

extern const int directions[128][8];
extern const int direction_count[128];

int gen_moves(const Position *pos, Move moves[]);

#endif
