
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
int quiescence_search(Position pos, int alpha, int beta, int depth);

int iterative_deepening(Position pos, int max_depth);

Move find_best_move(Position pos, int depth);
void search_set_features(int tt_enabled, int pruning_enabled);
void search_reset_node_count(void);
unsigned long long search_get_node_count(void);

typedef struct {
    int enable_null_move;
    int enable_lmr;
    int enable_futility;
} SearchOptions;

SearchOptions search_options_default(void);
SearchOptions search_get_options(void);
void search_set_options(SearchOptions options);
#endif
