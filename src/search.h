
#ifndef SEARCH_H
#define SEARCH_H

// Mate score boundaries
#define MATE_LOWER (60000 - 13 * 929)
#define MATE_UPPER (60000 + 10 * 929)

// Search tuning constants
#define QS 36
#define QS_A 180
#define LMR 70
#define EVAL_ROUGHNESS 15
#define NULL_MARGIN -200

// Time management
#define DELAY 200

// Maximum transposition table entries
#define TABLE_SIZE 1000000
#include "position.h"

int negamax(Position pos, int depth);

int negamax_ab(Position pos, int depth, int alpha, int beta);

void order_moves(const Position *pos, Move moves[], int count);

int quiescence(Position pos, int alpha, int beta);

int iterative_deepening(Position pos, int max_depth);

Move find_best_move(Position pos, int depth);
#endif
