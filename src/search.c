#include <stdio.h>
#include <string.h>
#include "search.h"
#include "movegen.h"
#include "legality.h"
#include "position.h"
#include "eval.h"
#include "tt.h"
#include "hash.h"
#include "board.h"
#include <ctype.h>
#include "uci_api.h"

static int tt_enabled = 1;
static int pruning_enabled = 1;
static unsigned long long search_nodes;

static SearchOptions search_options = {0, 0, 0};

void search_set_features(int tt, int pruning)
{
    tt_enabled = tt;
    pruning_enabled = pruning;
}

void search_set_options(SearchOptions options){
    search_options = options;
}

SearchOptions search_get_options(void){
    return search_options;
}

void search_reset_node_count(void) { search_nodes = 0; }
unsigned long long search_get_node_count(void) { return search_nodes; }

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
        if (search_stop_requested()) {
            break;
        }
        Position next = make_move(pos, moves[i]);
        score = -negamax(next, depth - 1);

        if (score > best) {
            best = score;
        }
    }

    return best;
}


static int negamax_ab_int(Position pos, int depth, int alpha, int beta, int allow_null) {
    int score;
    int original_alpha = alpha;
    search_nodes++;
    if (search_stop_requested()) {
        return alpha;
    }
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
    if (tt_enabled) {
        if (tt_probe(hash, depth, alpha, beta, &score)) {
            return score;
        }
    }
    
    if (allow_null && depth >= 3 && !is_in_check(&pos)) {
        // avoid applying for king and pawn endgames, as they are often tactical and require precise calculation
        // if it has N B R or Q then its not a king and pawn endgame, so we can apply null move pruning
        int piece_count = 0;
        for (int square = 0; square < BOARD_SIZE; square++) {
            char piece = pos.board[square];
            if (strchr("NBRQ", piece) != NULL) {
                piece_count++;
            }
        }
        if (piece_count > 0) {
            
            Position null_pos = make_null_move(pos);
            int null_score = -negamax_ab_int(null_pos, depth - 3, -beta, -beta + 1, 0); // null move search with reduced depth and no further null moves
            // If the null move score is greater than or equal to beta, we can prune this branch.
            if (null_score >= beta) {
                return beta; // Fail-hard beta cutoff
            }
        }
    }

    // en passant captures and promotions are considered tactical, so we don't apply LMR for them
    int lmr = search_options.enable_lmr &&
          depth >= 3 &&
          !is_in_check(&pos);

    Move moves[256];
    int count = gen_legal_moves(&pos, moves);

    // Good move ordering helps alpha-beta prune more branches.
    order_moves(&pos, moves, count);

    int best = -1000000;
    int bestidx = -1;

    for (int i = 0; i < count; i++) {
        if (search_stop_requested()) {
            break;
        }
        Move move = moves[i];
        Position next = make_move(pos, move);

        int full_depth = depth - 1;
        int new_depth = full_depth;

        // Detect captures, including en passant.
        int is_capture =
            islower((unsigned char)pos.board[move.j]) ||
            (pos.board[move.i] == 'P' &&
            pos.ep != 0 &&
            move.j == pos.ep);

        int is_promotion = move.prom != '\0';

        // Only reduce late, quiet moves.
        int reduced =
            lmr &&
            i >= 4 &&
            !is_capture &&
            !is_promotion;

        if (reduced) {
            new_depth--;
        }

        //futility pruning: if the score is less than alpha and we are not in check, we can skip further search
        if (search_options.enable_futility &&
            depth == 1 &&
            bestidx >= 0 &&
            !is_in_check(&pos) &&
            !is_capture &&
            !is_promotion &&
            !is_in_check(&next) &&
            pos.score + 200 <= alpha) {
            continue;
        }

        // Initial search.
        score = -negamax_ab_int(
            next,
            new_depth,
            -beta,
            -alpha,
            search_options.enable_null_move
        );

        

        // Verify promising reduced moves at full depth.
        if (reduced && score > alpha) {
            score = -negamax_ab_int(
                next,
                full_depth,
                -beta,
                -alpha,
                search_options.enable_null_move
            );
        }

        // Update best using the final score.
        if (score > best) {
            best = score;
            bestidx = i;
        }

        // Update alpha for ALL moves, whether reduced or not.
        if (score > alpha) {
            alpha = score;
        }

        // Ordinary alpha-beta cutoff.
        if (pruning_enabled && alpha >= beta) {
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
    if (tt_enabled && bestidx >= 0) {
        tt_store(hash, depth, best, flag, moves[bestidx]);
    }

    return best;
}

int negamax_ab(Position pos, int depth, int alpha, int beta) {
    return negamax_ab_int(pos, depth, alpha, beta, search_options.enable_null_move);
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

    if (search_stop_requested()) {
        return alpha;
    }

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
        if (search_stop_requested()) {
            break;
        }
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

int iterative_deepening(Position pos, int max_depth){
    if (max_depth <= 0) {
        return pos.score;
    }
    int score = pos.score;
    for (int depth = 1; depth <= max_depth; depth++) {
        if (search_stop_requested()) {
            break;
        }
        score = negamax_ab(pos, depth, -1000000, 1000000);
    }
    return score;
}

Move find_best_move(Position pos, int depth){
    Move moves[256];
    int count = gen_legal_moves(&pos, moves);
    int best_score = -1000000;
    Move best_move = {-1, -1, '\0'};
    if (depth < 1) {
        return best_move;
    }
    for (int i = 0; i < count; i++) {
        if (search_stop_requested()) {
            break;
        }
        Position next = make_move(pos, moves[i]);
        int score = -negamax_ab(next, depth - 1, -1000000, 1000000);
        if (score > best_score) {
            best_score = score;
            best_move = moves[i];
        }
    }
    return best_move;
}

SearchOptions search_options_default(void){
    return (SearchOptions){0, 0, 0};
}
