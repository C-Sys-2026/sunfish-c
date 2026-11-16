#include "../third_party/munit/munit.h"

#include "../src/legality.h"
#include "../include/uci_api.h"

#include <stdio.h>
#include <string.h>

#define assert(expression) munit_assert(expression)

static int contains_move(const Move moves[], int count, Move wanted)
{
    for (int index = 0; index < count; index++) {
        if (moves[index].i == wanted.i
            && moves[index].j == wanted.j
            && moves[index].prom == wanted.prom) {
            return 1;
        }
    }
    return 0;
}

static UciPosition parse_position(int argc, char *argv[])
{
    UciPosition position;
    assert(uci_position_parse(&position, argc, argv));
    return position;
}

static SearchResult search_at_depth(UciPosition *position, int depth)
{
    SearchLimits limits = {0};
    limits.depth = depth;
    limits.time_budget_ms = -1;
    limits.black_to_move = position->black_to_move;
    search_reset_stop();
    return search_position(&position->position, &limits);
}

static void assert_search_result_is_legal_and_restores(UciPosition *position,
                                                       SearchResult result)
{
    Move legal_moves[256];
    int count = gen_legal_moves(&position->position, legal_moves);

    if (count == 0) {
        assert(!result.has_best_move);
        return;
    }

    assert(result.has_best_move);
    assert(contains_move(legal_moves, count, result.best_move));
    {
        Position before = position->position;
        Position applied = make_move(position->position, result.best_move);
        assert(memcmp(&position->position, &before, sizeof(before)) == 0);
        position->position = applied;
        position->position = before;
        assert(memcmp(&position->position, &before, sizeof(before)) == 0);
    }
}

void test_uci_startpos_and_move_sequence(void)
{
    char *args[] = {"startpos", "moves", "e2e4", "e7e5", "g1f3"};
    UciPosition position = parse_position(5, args);
    char coordinate[6];
    Move move;

    assert(position.black_to_move == 1);
    assert(position.fullmove_number == 2);
    assert(position.halfmove_clock == 1);
    assert(uci_move_from_uci("b8c6", 1, &move));
    assert(uci_move_to_uci(move, 1, coordinate));
    assert(strcmp(coordinate, "b8c6") == 0);
    printf("PASS: UCI start position and move sequence\n");
}

void test_uci_fen_rejects_bad_position_without_changing_state(void)
{
    char *start_args[] = {"startpos"};
    char *bad_args[] = {"fen", "8/8/8/8/8/8/8/8", "w", "-", "-", "0", "1"};
    char *bad_moves[] = {"startpos", "moves", "e2e5"};
    UciPosition position = parse_position(1, start_args);
    UciPosition before = position;

    assert(!uci_position_parse(&position, 7, bad_args));
    assert(memcmp(&position, &before, sizeof(position)) == 0);
    assert(!uci_position_parse(&position, 3, bad_moves));
    assert(memcmp(&position, &before, sizeof(position)) == 0);
    printf("PASS: Invalid FEN preserves the current position\n");
}

void test_search_returns_a_legal_startpos_move(void)
{
    char *args[] = {"startpos"};
    UciPosition position = parse_position(1, args);
    Position before = position.position;
    SearchResult result = search_at_depth(&position, 2);

    assert_search_result_is_legal_and_restores(&position, result);
    assert(result.depth >= 1);
    assert(memcmp(&position.position, &before, sizeof(before)) == 0);
    printf("PASS: Search returns a legal start-position move\n");
}

void test_search_returns_no_move_for_checkmate(void)
{
    char *args[] = {
        "fen", "7k/6Q1/6K1/8/8/8/8/8", "b", "-", "-", "0", "1"
    };
    UciPosition position = parse_position(7, args);
    SearchResult result = search_at_depth(&position, 2);

    assert_search_result_is_legal_and_restores(&position, result);
    printf("PASS: Checkmate returns no best move\n");
}

void test_search_returns_no_move_for_stalemate(void)
{
    char *args[] = {
        "fen", "7k/5Q2/6K1/8/8/8/8/8", "b", "-", "-", "0", "1"
    };
    UciPosition position = parse_position(7, args);
    SearchResult result = search_at_depth(&position, 2);

    assert_search_result_is_legal_and_restores(&position, result);
    printf("PASS: Stalemate returns no best move\n");
}

void test_search_returns_legal_promotion_move(void)
{
    char *args[] = {
        "fen", "7k/P7/8/8/8/8/8/7K", "w", "-", "-", "0", "1"
    };
    UciPosition position = parse_position(7, args);
    SearchResult result = search_at_depth(&position, 1);

    assert_search_result_is_legal_and_restores(&position, result);
    assert(result.best_move.prom != '\0');
    printf("PASS: Promotion search result is legal\n");
}

void test_castling_and_en_passant_are_legal_search_candidates(void)
{
    char *castle_args[] = {
        "fen", "r3k2r/8/8/8/8/8/8/R3K2R", "w", "KQkq", "-", "0", "1"
    };
    char *ep_args[] = {
        "fen", "4k3/8/8/3pP3/8/8/8/4K3", "w", "-", "d6", "0", "1"
    };
    UciPosition castle_position = parse_position(7, castle_args);
    UciPosition ep_position = parse_position(7, ep_args);
    Move castle_move;
    Move ep_move;
    Move moves[256];
    int count;
    SearchResult castle_result;
    SearchResult ep_result;

    assert(uci_move_from_uci("e1g1", 0, &castle_move));
    count = gen_legal_moves(&castle_position.position, moves);
    assert(contains_move(moves, count, castle_move));

    assert(uci_move_from_uci("e5d6", 0, &ep_move));
    count = gen_legal_moves(&ep_position.position, moves);
    assert(contains_move(moves, count, ep_move));

    castle_result = search_at_depth(&castle_position, 1);
    ep_result = search_at_depth(&ep_position, 1);
    assert_search_result_is_legal_and_restores(&castle_position, castle_result);
    assert_search_result_is_legal_and_restores(&ep_position, ep_result);
    printf("PASS: Castling and en-passant candidates are legal\n");
}

void test_black_to_move_castling_coordinates_are_legal(void)
{
    char *args[] = {
        "fen", "r3k2r/8/8/8/8/8/8/R3K2R", "b", "KQkq", "-", "0", "1"
    };
    UciPosition position = parse_position(7, args);
    Move king_side_castle;
    Move moves[256];
    int count;

    assert(uci_move_from_uci("e8g8", 1, &king_side_castle));
    count = gen_legal_moves(&position.position, moves);
    assert(contains_move(moves, count, king_side_castle));
    assert_search_result_is_legal_and_restores(&position,
                                                search_at_depth(&position, 1));
    printf("PASS: Black castling coordinates map to a legal move\n");
}

void test_search_stop_keeps_a_legal_fallback_move(void)
{
    char *args[] = {"startpos"};
    UciPosition position = parse_position(1, args);
    SearchLimits limits = {0};
    SearchResult result;
    Move moves[256];
    int count;

    limits.depth = UCI_MAX_PV;
    limits.time_budget_ms = -1;
    search_request_stop();
    result = search_position(&position.position, &limits);
    search_reset_stop();

    count = gen_legal_moves(&position.position, moves);
    assert_search_result_is_legal_and_restores(&position, result);
    assert(result.has_best_move && contains_move(moves, count, result.best_move));
    printf("PASS: Stop preserves a legal fallback move\n");
}

void test_go_handler_stop_joins_search(void)
{
    char *position_args[] = {"startpos"};
    char *go_args[] = {"infinite"};
    UciPosition position = parse_position(1, position_args);

    assert(go_handler_start(&position, 1, go_args));
    go_handler_stop();
    go_handler_wait();
    printf("PASS: Stop joins the active search\n");
}

void test_go_handler_short_movetime_finishes(void)
{
    char *position_args[] = {"startpos"};
    char *go_args[] = {"movetime", "5"};
    UciPosition position = parse_position(1, position_args);
    SearchLimits limits = {0};
    SearchResult result;

    assert(go_handler_start(&position, 2, go_args));
    go_handler_wait();
    limits.depth = UCI_MAX_PV;
    limits.time_budget_ms = 5;
    result = search_position(&position.position, &limits);
    assert_search_result_is_legal_and_restores(&position, result);
    printf("PASS: Short movetime search finishes\n");
}

void test_go_handler_accepts_clocks_increments_and_movestogo(void)
{
    char *position_args[] = {"startpos"};
    char *go_args[] = {
        "wtime", "1000", "btime", "1000", "winc", "50",
        "binc", "50", "movestogo", "20", "depth", "1"
    };
    UciPosition position = parse_position(1, position_args);

    assert(go_handler_start(&position, 12, go_args));
    go_handler_wait();
    printf("PASS: Clock and increment limits are accepted\n");
}

void test_make_move_and_restore_preserve_all_position_fields(void)
{
    char *args[] = {"startpos"};
    UciPosition position = parse_position(1, args);
    UciPosition before = position;
    SearchResult result = search_at_depth(&position, 1);
    Position applied;

    assert(result.has_best_move);
    applied = make_move(position.position, result.best_move);
    assert(memcmp(&position.position, &before.position, sizeof(Position)) == 0);
    position.position = applied;
    position = before;
    assert(memcmp(&position, &before, sizeof(position)) == 0);
    printf("PASS: Applying and restoring a move preserves position state\n");
}
