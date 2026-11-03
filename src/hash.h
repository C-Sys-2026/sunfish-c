
#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include "position.h"

void init_zobrist(void);
uint64_t hash_position(const Position *pos);

#endif
