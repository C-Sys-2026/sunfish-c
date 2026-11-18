#ifndef PERFT_H
#define PERFT_H

#include <stdint.h>

#include "position.h"

/* Counts legal leaf positions without modifying the caller's Position. */
uint64_t perft(Position position, int depth);

#endif
