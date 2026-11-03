
#include "search.h"
#include "movegen.h"
#include "position.h"
#include "eval.h"
#include <ctype.h>

int negamax(Position pos, int depth) {
    // Stop searching when we reach the requested depth.
    if (depth <= 0) {
        return pos.score;
    }

    Move moves[256];
    int count = gen_moves(&pos, moves);

    int best = -1000000;

    // Explore moves from the opponent's perspective.
    for (int i = 0; i < count; i++) {
        Position next = make_move(pos, moves[i]);
        int score = -negamax(next, depth - 1);

        if (score > best) {
            best = score;
        }
    }

    return best;
}


int negamax_ab(Position pos, int depth, int alpha, int beta) {
    // Continue tactical exchanges instead of stopping immediately.
    if (depth <= 0) {
        return quiescence(pos, alpha, beta);
    }

    Move moves[256];
    int count = gen_moves(&pos, moves);

    // Good move ordering helps alpha-beta prune more branches.
    order_moves(&pos, moves, count);

    int best = -1000000;

    for (int i = 0; i < count; i++) {
        Position next = make_move(pos, moves[i]);

        // Reverse the search bounds for the opponent.
        int score = -negamax_ab(next, depth - 1, -beta, -alpha);

        if (score > best) {
            best = score;
        }

        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            break;
        }
    }

    return best;
}


void order_moves(const Position *pos, Move moves[], int count) {
    // Sort moves by immediate evaluation gain, highest first.
    for (int i = 1; i < count; i++) {
        Move current = moves[i];
        int current_score = value(pos, current);

        int j = i - 1;

        // Shift lower-scoring moves to make room for current.
        while (j >= 0 && value(pos, moves[j]) < current_score) {
            moves[j + 1] = moves[j];
            j--;
        }

        moves[j + 1] = current;
    }
}


int quiescence(Position pos, int alpha, int beta) {
    // The current evaluation is our starting point.
    int stand_pat = pos.score;

    if (stand_pat >= beta) {
        return beta;
    }

    if (stand_pat > alpha) {
        alpha = stand_pat;
    }

    Move moves[256];
    int count = gen_moves(&pos, moves);

    // Searching promising captures first improves pruning.
    order_moves(&pos, moves, count);

    for (int i = 0; i < count; i++) {
        Move move = moves[i];

        // Only continue searching captures of enemy pieces.
        if (!islower((unsigned char)pos.board[move.j])) {
            continue;
        }

        Position next = make_move(pos, move);

        // The opponent evaluates the position from the opposite perspective.
        int score = -quiescence(next, -beta, -alpha);

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

