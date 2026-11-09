#include "../third_party/munit/munit.h"

void test_castling_changes_hash(void);
void test_en_passant_changes_hash(void);
void test_king_passant_changes_hash(void);
void test_same_position_same_hash(void);
void test_different_positions_different_hashes(void);
void test_san_history(void);
void test_empty_board(void);
void test_center_knight(void);
void test_position_pawn_move(void);
void test_move_score(void);
void test_double_pawn_push(void);
void test_promotion(void);
void test_position_castling(void);
void test_en_passant(void);
void test_no_ambiguity(void);
void test_file_disambiguation(void);
void test_rank_disambiguation(void);
void test_both_file_and_rank(void);
void test_san_pawn_move(void);
void test_knight_move(void);
void test_pawn_capture(void);
void test_san_castling(void);
void test_small_buffer(void);
void test_quiescence_integration(void);
void test_quiescence_capture(void);
void test_quiescence_quiet_position(void);
void test_move_ordering(void);
void test_alpha_beta(void);
void test_depth_zero(void);
void test_depth_one(void);
void test_depth_two(void);
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
WRAP(test_empty_board)
WRAP(test_center_knight)
WRAP(test_position_pawn_move)
WRAP(test_move_score)
WRAP(test_double_pawn_push)
WRAP(test_promotion)
WRAP(test_position_castling)
WRAP(test_en_passant)
WRAP(test_no_ambiguity)
WRAP(test_file_disambiguation)
WRAP(test_rank_disambiguation)
WRAP(test_both_file_and_rank)
WRAP(test_san_pawn_move)
WRAP(test_knight_move)
WRAP(test_pawn_capture)
WRAP(test_san_castling)
WRAP(test_small_buffer)
WRAP(test_quiescence_integration)
WRAP(test_quiescence_capture)
WRAP(test_quiescence_quiet_position)
WRAP(test_move_ordering)
WRAP(test_alpha_beta)
WRAP(test_depth_zero)
WRAP(test_depth_one)
WRAP(test_depth_two)
WRAP(test_store_and_probe)
WRAP(test_insufficient_depth)
WRAP(test_clear)
WRAP(test_index_collision)
WRAP(test_tt_bounds)

static MunitTest tests[] = {
    CASE("/hash/castling", test_castling_changes_hash),
    CASE("/hash/en-passant", test_en_passant_changes_hash),
    CASE("/hash/king-passant", test_king_passant_changes_hash),
    CASE("/hash/same-position", test_same_position_same_hash),
    CASE("/hash/different-position", test_different_positions_different_hashes),
    CASE("/history/san", test_san_history),
    CASE("/movegen/empty-board", test_empty_board),
    CASE("/movegen/center-knight", test_center_knight),
    CASE("/position/pawn-move", test_position_pawn_move),
    CASE("/position/move-score", test_move_score),
    CASE("/position/double-pawn-push", test_double_pawn_push),
    CASE("/position/promotion", test_promotion),
    CASE("/position/castling", test_position_castling),
    CASE("/position/en-passant", test_en_passant),
    CASE("/san/no-ambiguity", test_no_ambiguity),
    CASE("/san/file-disambiguation", test_file_disambiguation),
    CASE("/san/rank-disambiguation", test_rank_disambiguation),
    CASE("/san/both-file-and-rank", test_both_file_and_rank),
    CASE("/san/pawn-move", test_san_pawn_move),
    CASE("/san/knight-move", test_knight_move),
    CASE("/san/pawn-capture", test_pawn_capture),
    CASE("/san/castling", test_san_castling),
    CASE("/san/small-buffer", test_small_buffer),
    CASE("/search/quiescence-integration", test_quiescence_integration),
    CASE("/search/quiescence-capture", test_quiescence_capture),
    CASE("/search/quiescence-quiet", test_quiescence_quiet_position),
    CASE("/search/move-ordering", test_move_ordering),
    CASE("/search/alpha-beta", test_alpha_beta),
    CASE("/search/depth-zero", test_depth_zero),
    CASE("/search/depth-one", test_depth_one),
    CASE("/search/depth-two", test_depth_two),
    CASE("/tt/store-and-probe", test_store_and_probe),
    CASE("/tt/insufficient-depth", test_insufficient_depth),
    CASE("/tt/clear", test_clear),
    CASE("/tt/index-collision", test_index_collision),
    CASE("/tt/bounds", test_tt_bounds),
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
    "/sunfish-c", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char* argv[]) {
    return munit_suite_main(&suite, NULL, argc, argv);
}
