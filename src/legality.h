
#ifndef LEGALITY_H
#define LEGALITY_H

#include "position.h"

int is_square_attacked(const Position *pos, int square);
int is_in_check(const Position *pos);
int gen_legal_moves(const Position *pos, Move moves[]);
int is_castling_safe(const Position *pos, Move move);
int is_checkmate(const Position *pos);

#endif
