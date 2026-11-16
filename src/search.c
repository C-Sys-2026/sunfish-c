
#include "search.h"
#include "movegen.h"
#include "legality.h"
#include "position.h"
#include "eval.h"
#include "tt.h"
#include "hash.h"
#include "board.h"
#include <ctype.h>

int negamax(Position pos, int depth) {
    int score;
    if (is_checkmate(&pos)) {
        return -MATE_UPPER; // prefer faster checkmates
    }

    if (is_stalemate(&pos)) {
        return 0; // stalemate is a draw
    }
    // Stop searching when we reach the requested depth.
    if (depth <= 0) {
        return pos.score;
    }

    Move moves[256];
    int count = gen_legal_moves(&pos, moves);

    int best = -1000000;

    // Explore moves from the opponent's perspective.
    for (int i = 0; i < count; i++) {
        Position next = make_move(pos, moves[i]);
        score = -negamax(next, depth - 1);

        if (score > best) {
            best = score;
        }
    }

    return best;
}


int negamax_ab(Position pos, int depth, int alpha, int beta) {
    int score;
    int original_alpha = alpha;
    
    if (is_checkmate(&pos)) {
        return -MATE_UPPER; // prefer faster checkmates
    }

    if (is_stalemate(&pos)) {
        return 0; // stalemate is a draw
    }
    // Continue tactical exchanges instead of stopping immediately.
    if (depth <= 0) {
        return quiescence(pos, alpha, beta);
    }

    // Check the transposition table for a previously computed score.
    uint64_t hash = hash_position(&pos);
    if (tt_probe(hash, depth, alpha, beta, &score)) {
        return score;
    }
    

    Move moves[256];
    int count = gen_legal_moves(&pos, moves);

    // Good move ordering helps alpha-beta prune more branches.
    order_moves(&pos, moves, count);

    int best = -1000000;

    for (int i = 0; i < count; i++) {
        Position next = make_move(pos, moves[i]);

        // Reverse the search bounds for the opponent.
        score = -negamax_ab(next, depth - 1, -beta, -alpha);
        
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

    TTFlag flag;

    if (best <= original_alpha) {
        flag = TT_UPPER;
    } else if (best >= beta) {
        flag = TT_LOWER;
    } else {
        flag = TT_EXACT;
    }

    // 4. STORE: Save the result for this position.
    tt_store(hash, depth, best, flag);

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
    return quiescence_search(pos, alpha, beta, 8);
}

int quiescence_search(Position pos, int alpha, int beta, int depth){
    int in_check = 0;
    int has_king = 0;

    // The current evaluation is our starting point.

    if (is_checkmate(&pos)) {
        return -MATE_UPPER; // prefer faster checkmates
    }

    if (is_stalemate(&pos)) {
        return 0; // stalemate is a draw
    }

    if (depth <= 0) {
        return pos.score;
    }

    for (int square = 0; square < BOARD_SIZE; square++) {
        if (pos.board[square] == 'K') {
            has_king = 1;
            in_check = is_in_check(&pos);
            break;
        }
    }

    int stand_pat = pos.score;

    if (stand_pat >= beta && !in_check) {
        return beta;
    }

    if (stand_pat > alpha && !in_check) {
        alpha = stand_pat;
    }

    Move moves[256];
    int count = has_king ? gen_legal_moves(&pos, moves) : gen_moves(&pos, moves);

    // Searching promising captures first improves pruning.
    order_moves(&pos, moves, count);

    for (int i = 0; i < count; i++) {
        Move move = moves[i];
        Position next = make_move(pos, move);

        // if we are not in check, only consider captures en passat and promotions
        if (!in_check) {
            int accept = 0;
            if (islower((unsigned char)pos.board[move.j])) {
                accept = 1;
            } else if (move.prom != '\0') {
                accept = 1;
            } else if (move.j == pos.ep && (pos.board[move.i] == 'P')) {
                accept = 1;
            }
            if (!accept) {
                continue;
            }
        }

        // The opponent evaluates the position from the opposite perspective.

        int score = -quiescence_search(next, -beta, -alpha, depth-1);
        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }

    }

    return alpha;
}