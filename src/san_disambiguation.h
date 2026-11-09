
#ifndef SAN_DISAMBIGUATION_H
#define SAN_DISAMBIGUATION_H

#include "position.h"

// Writes "", "b", "1", or "b1" into output.
// Returns 1 on success, 0 on failure.
int san_disambiguation(const Position *pos, Move move,
                       char output[3]);

#endif
