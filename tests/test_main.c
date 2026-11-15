#include "../third_party/munit/munit.h"

void test_castling_changes_hash(void);
void test_en_passant_changes_hash(void);
void test_king_passant_changes_hash(void);
void test_same_position_same_hash(void);
void test_different_positions_different_hashes(void);
void test_san_history(void);
void test_pawn_attack(void);
void test_pawn_direction(void);
void test_knight_attack(void);
void test_rook_attack(void);
void test_bishop_attack(void);
void test_queen_attack(void);
void test_king_attack(void);
void test_blocked_attack(void);
void test_in_check(void);
void test_not_in_check(void);
void test_missing_king(void);
void test_board_boundary(void);
void test_initial_legal_moves(void);
void test_pinned_rook(void);
void test_castling_through_check(void);
void test_checkmate(void);
void test_stalemate(void);
void test_empty_board(void);
void test_center_knight(void);
void test_promotion(void);
void test_position_castling(void);
void test_en_passant(void);
void test_position_pawn_move(void);
void test_move_score(void);
void test_double_pawn_push(void);
void test_san_pawn_move(void);
void test_knight_move(void);
void test_pawn_capture(void);
void test_san_castling(void);
void test_small_buffer(void);
void test_no_ambiguity(void);
void test_file_disambiguation(void);
void test_rank_disambiguation(void);
void test_both_file_and_rank(void);
void test_quiescence_integration(void);
void test_quiescence_capture(void);
void test_quiescence_quiet_position(void);
void test_move_ordering(void);
void test_alpha_beta(void);
void test_depth_zero(void);
void test_depth_one(void);
void test_depth_two(void);
void test_negamax_checkmate(void);
void test_negamax_stalemate(void);
void test_alpha_beta_checkmate(void);
void test_alpha_beta_stalemate(void);
void test_quiescence_checkmate(void);
void test_quiescence_stalemate(void);
void test_quiescence_depth_limit(void);
void test_store_and_probe(void);
void test_insufficient_depth(void);
void test_clear(void);
void test_index_collision(void);
void test_tt_bounds(void);

#define WRAP(fn) static MunitResult wrap_##fn(const MunitParameter* p, void* d) { (void)p; (void)d; fn(); return MUNIT_OK; }
#define CASE(name, fn) { name, wrap_##fn, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }

WRAP(test_castling_changes_hash)
WRAP(test_en_passant_changes_hash)
WRAP(test_king_passant_changes_hash)
WRAP(test_same_position_same_hash)
WRAP(test_different_positions_different_hashes)
WRAP(test_san_history)
WRAP(test_pawn_attack)
WRAP(test_pawn_direction)
WRAP(test_knight_attack)
WRAP(test_rook_attack)
WRAP(test_bishop_attack)
WRAP(test_queen_attack)
WRAP(test_king_attack)
WRAP(test_blocked_attack)
WRAP(test_in_check)
WRAP(test_not_in_check)
WRAP(test_missing_king)
WRAP(test_board_boundary)
WRAP(test_initial_legal_moves)
WRAP(test_pinned_rook)
WRAP(test_castling_through_check)
WRAP(test_checkmate)
WRAP(test_stalemate)
WRAP(test_empty_board)
WRAP(test_center_knight)
WRAP(test_promotion)
WRAP(test_position_castling)
WRAP(test_en_passant)
WRAP(test_position_pawn_move)
WRAP(test_move_score)
WRAP(test_double_pawn_push)
WRAP(test_san_pawn_move)
WRAP(test_knight_move)
WRAP(test_pawn_capture)
WRAP(test_san_castling)
WRAP(test_small_buffer)
WRAP(test_no_ambiguity)
WRAP(test_file_disambiguation)
WRAP(test_rank_disambiguation)
WRAP(test_both_file_and_rank)
WRAP(test_quiescence_integration)
WRAP(test_quiescence_capture)
WRAP(test_quiescence_quiet_position)
WRAP(test_move_ordering)
WRAP(test_alpha_beta)
WRAP(test_depth_zero)
WRAP(test_depth_one)
WRAP(test_depth_two)
WRAP(test_negamax_checkmate)
WRAP(test_negamax_stalemate)
WRAP(test_alpha_beta_checkmate)
WRAP(test_alpha_beta_stalemate)
WRAP(test_quiescence_checkmate)
WRAP(test_quiescence_stalemate)
WRAP(test_quiescence_depth_limit)
WRAP(test_store_and_probe)
WRAP(test_insufficient_depth)
WRAP(test_clear)
WRAP(test_index_collision)
WRAP(test_tt_bounds)

static MunitTest tests[] = {
    CASE("/hash/castling_changes_hash", test_castling_changes_hash),
    CASE("/hash/en_passant_changes_hash", test_en_passant_changes_hash),
    CASE("/hash/king_passant_changes_hash", test_king_passant_changes_hash),
    CASE("/hash/same_position_same_hash", test_same_position_same_hash),
    CASE("/hash/different_positions_different_hashes", test_different_positions_different_hashes),
    CASE("/history/san_history", test_san_history),
    CASE("/legality/pawn_attack", test_pawn_attack),
    CASE("/legality/pawn_direction", test_pawn_direction),
    CASE("/legality/knight_attack", test_knight_attack),
    CASE("/legality/rook_attack", test_rook_attack),
    CASE("/legality/bishop_attack", test_bishop_attack),
    CASE("/legality/queen_attack", test_queen_attack),
    CASE("/legality/king_attack", test_king_attack),
    CASE("/legality/blocked_attack", test_blocked_attack),
    CASE("/legality/in_check", test_in_check),
    CASE("/legality/not_in_check", test_not_in_check),
    CASE("/legality/missing_king", test_missing_king),
    CASE("/legality/board_boundary", test_board_boundary),
    CASE("/legality/initial_legal_moves", test_initial_legal_moves),
    CASE("/legality/pinned_rook", test_pinned_rook),
    CASE("/legality/castling_through_check", test_castling_through_check),
    CASE("/legality/checkmate", test_checkmate),
    CASE("/legality/stalemate", test_stalemate),
    CASE("/movegen/empty_board", test_empty_board),
    CASE("/movegen/center_knight", test_center_knight),
    CASE("/position/promotion", test_promotion),
    CASE("/position/position_castling", test_position_castling),
    CASE("/position/en_passant", test_en_passant),
    CASE("/position/position_pawn_move", test_position_pawn_move),
    CASE("/position/move_score", test_move_score),
    CASE("/position/double_pawn_push", test_double_pawn_push),
    CASE("/san/san_pawn_move", test_san_pawn_move),
    CASE("/san/knight_move", test_knight_move),
    CASE("/san/pawn_capture", test_pawn_capture),
    CASE("/san/san_castling", test_san_castling),
    CASE("/san/small_buffer", test_small_buffer),
    CASE("/san_disambiguation/no_ambiguity", test_no_ambiguity),
    CASE("/san_disambiguation/file_disambiguation", test_file_disambiguation),
    CASE("/san_disambiguation/rank_disambiguation", test_rank_disambiguation),
    CASE("/san_disambiguation/both_file_and_rank", test_both_file_and_rank),
    CASE("/search/quiescence_integration", test_quiescence_integration),
    CASE("/search/quiescence_capture", test_quiescence_capture),
    CASE("/search/quiescence_quiet_position", test_quiescence_quiet_position),
    CASE("/search/move_ordering", test_move_ordering),
    CASE("/search/alpha_beta", test_alpha_beta),
    CASE("/search/depth_zero", test_depth_zero),
    CASE("/search/depth_one", test_depth_one),
    CASE("/search/depth_two", test_depth_two),
    CASE("/search/negamax_checkmate", test_negamax_checkmate),
    CASE("/search/negamax_stalemate", test_negamax_stalemate),
    CASE("/search/alpha_beta_checkmate", test_alpha_beta_checkmate),
    CASE("/search/alpha_beta_stalemate", test_alpha_beta_stalemate),
    CASE("/search/quiescence_checkmate", test_quiescence_checkmate),
    CASE("/search/quiescence_stalemate", test_quiescence_stalemate),
    CASE("/search/quiescence_depth_limit", test_quiescence_depth_limit),
    CASE("/tt/store_and_probe", test_store_and_probe),
    CASE("/tt/insufficient_depth", test_insufficient_depth),
    CASE("/tt/clear", test_clear),
    CASE("/tt/index_collision", test_index_collision),
    CASE("/tt/tt_bounds", test_tt_bounds),
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
    "/sunfish-c", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char* argv[]) {
    return munit_suite_main(&suite, NULL, argc, argv);
}
