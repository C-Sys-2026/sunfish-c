
#ifndef SAN_H
#define SAN_H

#include <stddef.h>
#include "position.h"

int move_to_san(const Position *pos, Move move,
                char *output, size_t size);
int move_to_san_for_side(const Position *pos, Move move, int black_to_move,
                         char *output, size_t size);

#endif
