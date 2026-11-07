
#ifndef SAN_H
#define SAN_H

#include <stddef.h>
#include "position.h"

int move_to_san(const Position *pos, Move move,
                char *output, size_t size);

#endif
