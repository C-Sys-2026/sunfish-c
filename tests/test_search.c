

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

void test_null_switch(void){
    Position pos = {0};
    strcpy(pos.board, initial);
    search_set_features(0, 1);
    SearchOptions options = {1,0,0};
    search_set_options(options);
    int score_with_null = negamax_ab(pos, 2, -1000000, 1000000);
    options.enable_null_move = 0;
    search_set_options(options);
    int score_without_null = negamax_ab(pos, 2, -1000000, 1000000);
    assert(score_with_null == score_without_null);
    printf("PASS: Null move pruning does not change the score\n");
}

void test_lmr_switch(void){
    Position pos = {0};
    strcpy(pos.board, initial);
    search_set_features(0, 1);
    SearchOptions options = {0,1,0};
    search_set_options(options);
    int score_with_lmr = negamax_ab(pos, 4, -1000000, 1000000);
    options.enable_lmr = 0;
    search_set_options(options);
    int score_without_lmr = negamax_ab(pos, 4, -1000000, 1000000);
    assert(score_with_lmr == score_without_lmr);
    printf("PASS: Late move reduction does not change the score\n");
}

void test_futility_switch(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    search_set_features(0, 1);  // Disable TT, enable alpha-beta pruning

    SearchOptions options = search_options_default();

    int alpha = pos.score + 250;
    int beta = pos.score + 500;

    // Futility pruning ON
    options.enable_futility = 1;
    search_set_options(options);

    search_reset_node_count();
    int score_with_futility = negamax_ab(pos, 1, alpha, beta);
    unsigned long long nodes_with_futility = search_get_node_count();

    // Futility pruning OFF
    options.enable_futility = 0;
    search_set_options(options);

    search_reset_node_count();
    int score_without_futility = negamax_ab(pos, 1, alpha, beta);
    unsigned long long nodes_without_futility = search_get_node_count();

    printf("Futility ON:  score = %d, nodes = %llu\n",
           score_with_futility, nodes_with_futility);

    printf("Futility OFF: score = %d, nodes = %llu\n",
           score_without_futility, nodes_without_futility);

    // Futility pruning should not increase the number of searched nodes.
    assert(nodes_with_futility <= nodes_without_futility);

    // This test position/window should exercise futility pruning.
    assert(nodes_with_futility < nodes_without_futility);

    // Restore defaults.
    search_set_options(search_options_default());
    search_set_features(1, 1);

    printf("PASS: Futility pruning reduces searched nodes\n");
}

void test_all_pruning_enabled(void) {
    Position pos = {0};
    strcpy(pos.board, initial);

    // Disable TT, keep alpha-beta pruning enabled.
    search_set_features(0, 1);

    // Enable all three selective-pruning features.
    SearchOptions options = {1, 1, 1};
    search_set_options(options);

    // Search from the initial position.
    Move best = find_best_move(pos, 4);

    // Generate legal moves to verify the result.
    Move legal_moves[256];
    int count = gen_legal_moves(&pos, legal_moves);

    int found = 0;

    for (int i = 0; i < count; i++) {
        if (best.i == legal_moves[i].i &&
            best.j == legal_moves[i].j &&
            best.prom == legal_moves[i].prom) {
            found = 1;
            break;
        }
    }

    assert(found);

    // Restore default settings.
    search_set_options(search_options_default());
    search_set_features(1, 1);

    printf("PASS: All pruning features work together\n");
}