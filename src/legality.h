
#ifndef LEGALITY_H
#define LEGALITY_H

#include "position.h"

int is_square_attacked(const Position *pos, int square);
int is_in_check(const Position *pos);

#endif
