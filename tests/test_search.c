

#include "../third_party/munit/munit.h"
#include "../src/movegen.h"
#include "../src/board.h"
#include "../src/position.h"
#include "../src/eval.h"
#include "../src/search.h"
#define assert(expr) munit_assert(expr)
#include <stdio.h>
#include <string.h>
#include "../src/hash.h"
#include "../src/tt.h"


void test_quiescence_integration(void) {
    Position pos = {0};
    strcpy(pos.board, initial);
    pos.score = 42;

    // At depth zero, alpha-beta should use quiescence.
    int result = negamax_ab(pos, 0, -1000000, 1000000);

    assert(result == quiescence(pos, -1000000, 1000000));

    printf("PASS: Alpha-beta calls quiescence at depth zero\n");
}


void test_quiescence_capture(void) {
    Position pos = {0};

    strcpy(pos.board, initial);

    // Clear the playable squares.
    for (int row = 2; row <= 9; row++) {
        for (int col = 1; col <= 8; col++) {
            pos.board[row * 10 + col] = '.';
        }
    }

    // White rook on c2, black queen on c7.
    pos.board[83] = 'R';
    pos.board[33] = 'q';

    pos.score = 0;

    // TODO 1: Call quiescence with a wide alpha-beta window.
    int result = quiescence(pos, -1000000, 1000000);

    // TODO 2: Assert that capturing the queen improves the score.
    assert(result > pos.score);

    printf("PASS: Quiescence finds a valuable capture\n");
}


void test_quiescence_quiet_position(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    pos.score = 42;

    // No captures are available initially, so return the current score.
    int score = quiescence(pos, -1000000, 1000000);

    assert(score == 42);

    printf("PASS: Quiescence returns stand-pat in quiet positions\n");
}


void test_move_ordering(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    Move moves[256];
    int count = gen_moves(&pos, moves);

    order_moves(&pos, moves, count);

    // Each move must score at least as highly as the next move.
    for (int i = 1; i < count; i++) {
        assert(value(&pos, moves[i - 1]) >= value(&pos, moves[i]));
    }

    printf("PASS: Moves are ordered by evaluation score\n");
}



void test_alpha_beta(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    // Both searches should agree in a quiet position at depth zero.
    int normal = negamax(pos, 0);
    int pruned = negamax_ab(pos, 0, -1000000, 1000000);

    assert(normal == pruned);

    // Alpha-beta should produce a valid score at deeper depths.
    for (int depth = 1; depth <= 3; depth++) {
        int score = negamax_ab(pos, depth, -1000000, 1000000);

        assert(score > -1000000);
        assert(score < 1000000);
    }

    printf("PASS: Alpha-beta search with quiescence\n");
}



void test_depth_zero(void) {
    Position pos = {0};
    strcpy(pos.board, initial);
    pos.score = 123;

    // At depth zero, search should return the current evaluation.
    assert(negamax(pos, 0) == 123);

    printf("PASS: Depth zero returns current score\n");
}

void test_depth_one(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    Move moves[256];
    int count = gen_moves(&pos, moves);

    int expected = -1000000;

    // At depth one, the best score is the largest immediate move score.
    for (int i = 0; i < count; i++) {
        int score = value(&pos, moves[i]);

        if (score > expected)
            expected = score;
    }

    assert(negamax(pos, 1) == expected);

    printf("PASS: Depth one selects best immediate score\n");
}

void test_depth_two(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    // Searching two plies should produce a finite score.
    int score = negamax(pos, 2);

    assert(score > -1000000 && score < 1000000);

    printf("PASS: Depth two completes successfully\n");
}

void test_negamax_checkmate(void){
    Position pos = {0};
    pos.board[98] = 'K';
    pos.board[87] = 'q';
    pos.board[76] = 'k';

    assert(negamax(pos, 0) == -MATE_UPPER);
    printf("PASS: Negamax detects checkmate\n");
}

void test_negamax_stalemate(void){
    Position pos = {0};
    pos.board[98] = 'K';
    pos.board[57] = 'r';
    pos.board[28] = 'k';
    pos.board[84] = 'r';

    assert(negamax(pos, 0) == 0);
    printf("PASS: Negamax detects stalemate\n");
}
void test_alpha_beta_checkmate(void){
    Position pos = {0};
    pos.board[98] = 'K';
    pos.board[87] = 'q';
    pos.board[76] = 'k';

    assert(negamax_ab(pos, 0, -1000000, 1000000) == -MATE_UPPER);
    printf("PASS: Alpha-beta detects checkmate\n");
}
void test_alpha_beta_stalemate(void){
    Position pos = {0};
    pos.board[98] = 'K';
    pos.board[57] = 'r';
    pos.board[28] = 'k';
    pos.board[84] = 'r';

    assert(negamax_ab(pos, 0, -1000000, 1000000) == 0);
    printf("PASS: Alpha-beta detects stalemate\n");
}

void test_quiescence_checkmate(void){
    Position pos = {0};
    pos.board[98] = 'K';
    pos.board[87] = 'q';
    pos.board[76] = 'k';

    assert(quiescence(pos, -1000000, 1000000) == -MATE_UPPER);
    printf("PASS: Quiescence detects checkmate\n");
}
void test_quiescence_stalemate(void){
    Position pos = {0};
    pos.board[98] = 'K';
    pos.board[57] = 'r';
    pos.board[28] = 'k';
    pos.board[84] = 'r';

    assert(quiescence(pos, -1000000, 1000000) == 0);
    printf("PASS: Quiescence detects stalemate\n");
}
void test_quiescence_depth_limit(void){
    Position pos = {0};
    pos.board[98] = 'K';
    pos.board[57] = 'r';
    pos.board[28] = 'k';
    pos.board[84] = 'r';

    assert(quiescence(pos, -1000000, 1000000) == 0);
    printf("PASS: Quiescence respects depth limit\n");
}

void test_search_stores_tt_entry(void){
    Position pos = {0};
    strcpy(pos.board, initial);

    int depth = 2;
    int alpha = -1000000;
    int beta = 1000000;
    tt_clear();
    // Perform a search to populate the transposition table.
    int search_score = negamax_ab(pos, depth, alpha, beta);
    printf("Search score: %d\n", search_score);

    uint64_t hash = hash_position(&pos);
    int score;
    int found = tt_probe(hash, depth, alpha, beta, &score);
    printf("Hash: %llu\n", (unsigned long long)hash);
    printf("Found: %d\n", found);
    fflush(stdout);
    assert(found);
    printf("PASS: Search stores entry in transposition table\n");
}

void test_search_uses_tt_entry(void){
    Position pos = {0};
    strcpy(pos.board, initial);
    int depth = 2;
    int alpha = -1000000;
    int beta = 1000000;
    tt_clear();
    uint64_t hash = hash_position(&pos);
    int score = 12345;
    tt_store(hash, depth, score, TT_EXACT, (Move){-1, -1, '\0'});
    int rescore = negamax_ab(pos, depth, alpha, beta);   
    assert(score == rescore);
    printf("PASS: Search uses entry from transposition table\n");
}

void test_iterative_deepening(void){
    Position pos = {0};
    strcpy(pos.board, initial);
    tt_clear();
    int max_depth = 3;
    int score = iterative_deepening(pos, max_depth);
    int expected = negamax_ab(pos, max_depth, -1000000, 1000000);
    assert(score == expected);
}

void test_iterative_deepening_zero_depth(void){
    Position pos = {0};
    strcpy(pos.board, initial);
    tt_clear();
    int max_depth = 0;
    int score = iterative_deepening(pos, max_depth);
    assert(score == pos.score);
}

void test_best_move(void){
    Position pos = {0};
    strcpy(pos.board, empty);
    pos.board[28] = 'k';  // h8
    pos.board[32] = 'R';  // b7
    pos.board[43] = 'R';  // c6
    pos.board[91] = 'K';  // a1
    
    // best move should find checkmate
    Move best_move = find_best_move(pos, 1);
    if (best_move.i == -1 || best_move.j == -1) {
        printf("No best move found\n");
        fflush(stdout);
        assert(0);
    }
    printf("Best move: %c%d -> %c%d\n",
        'a' + (best_move.i % 10) - 1,
        10 - (best_move.i / 10),
        'a' + (best_move.j % 10) - 1,
        10 - (best_move.j / 10));
    Position next = make_move(pos, best_move);
    assert(is_checkmate(&next));
    printf("PASS: find_best_move finds checkmate\n");
}

void test_best_move_checkmate(void){
    Position pos = {0};
    strcpy(pos.board, empty);
    pos.board[28] = 'k';  // h8
    pos.board[32] = 'R';  // b7
    pos.board[43] = 'R';  // c6
    pos.board[91] = 'K';  // a1
    
    // best move should find checkmate
    Move best_move = find_best_move(pos, 2);
    if (best_move.i == -1 || best_move.j == -1) {
        printf("No best move found\n");
        fflush(stdout);
        assert(0);
    }
    printf("Best move: %c%d -> %c%d\n",
        'a' + (best_move.i % 10) - 1,
        10 - (best_move.i / 10),
        'a' + (best_move.j % 10) - 1,
        10 - (best_move.j / 10));
    Position next = make_move(pos, best_move);
    assert(is_checkmate(&next));
    printf("PASS: find_best_move finds checkmate\n");    
}

void test_best_move_stalemate(void){
    Position pos = {0};
    strcpy(pos.board, empty);
    pos.board[28] = 'k';  // h8
    pos.board[32] = 'R';  // b7
    pos.board[47] = 'R';  // g6
    pos.board[91] = 'K';  // a1
    pos = rotate(pos, 0);
    
    Move best_move = find_best_move(pos, 2);
    if (best_move.i == -1 || best_move.j == -1) {
        printf("No best move found\n");
        fflush(stdout);
        assert(1);
        printf("PASS: find_best_move finds stalemate\n");
    }
}

void test_search_options_default(void){
    SearchOptions options = search_options_default();
    assert(options.enable_null_move == 0);
    assert(options.enable_lmr == 0);
    assert(options.enable_futility == 0);
    printf("PASS: search_options_default returns default options\n");
}

void test_search_set_get_options(void){
    SearchOptions options = {0,0,0};
    search_set_options(options);
    //enable only null move
    options.enable_null_move = 1;
    search_set_options(options);
    SearchOptions retrieved = search_get_options();
    assert(retrieved.enable_null_move == 1);
    assert(retrieved.enable_lmr == 0);
    assert(retrieved.enable_futility == 0);
    //repeat for lmr and futility
    options.enable_null_move = 0;
    options.enable_lmr = 1;
    search_set_options(options);
    retrieved = search_get_options();
    assert(retrieved.enable_null_move == 0);
    assert(retrieved.enable_lmr == 1);
    assert(retrieved.enable_futility == 0);
    options.enable_lmr = 0;
    options.enable_futility = 1;
    search_set_options(options);
    retrieved = search_get_options();
    assert(retrieved.enable_null_move == 0);
    assert(retrieved.enable_lmr == 0);
    assert(retrieved.enable_futility == 1);
    printf("PASS: search_set_options and search_get_options work correctly\n");
}
#if 0
int main(void) {
    pad_tables();
    init_king_tables();

    test_depth_zero();
    test_depth_one();
    test_depth_two();
    test_alpha_beta();
    test_move_ordering();
    test_quiescence_quiet_position();
    test_quiescence_integration();
    test_quiescence_capture();
    printf("All search tests passed!\n");
    return 0;
}
#endif
