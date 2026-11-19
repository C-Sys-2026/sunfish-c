#include "../third_party/munit/munit.h"
#define assert(expr) munit_assert(expr)

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "../src/pgn.h"
#include "../src/uci_api.h"

#ifdef _WIN32
#include <io.h>
#else
#include <dirent.h>
#endif

#define TEST_GAMES_MAX 256
#define TEST_FILENAME_SIZE 128

typedef struct {
    char names[TEST_GAMES_MAX][TEST_FILENAME_SIZE];
    int count;
} TestGames;

static int read_file(const char *path, char *contents, size_t size);

static void list_games(TestGames *games)
{
    memset(games, 0, sizeof(*games));
#ifdef _WIN32
    {
        struct _finddata_t entry;
        intptr_t handle = _findfirst("games\\*.txt", &entry);
        if (handle == -1) return;
        do {
            if (games->count < TEST_GAMES_MAX) {
                snprintf(games->names[games->count++], TEST_FILENAME_SIZE,
                         "%s", entry.name);
            }
        } while (_findnext(handle, &entry) == 0);
        _findclose(handle);
    }
#else
    {
        DIR *directory = opendir("games");
        struct dirent *entry;
        if (directory == NULL) return;
        while ((entry = readdir(directory)) != NULL) {
            size_t length = strlen(entry->d_name);
            if (length >= 4 && strcmp(entry->d_name + length - 4, ".txt") == 0
                && games->count < TEST_GAMES_MAX) {
                snprintf(games->names[games->count++], TEST_FILENAME_SIZE,
                         "%s", entry->d_name);
            }
        }
        closedir(directory);
    }
#endif
}

static int has_game(const TestGames *games, const char *name)
{
    for (int index = 0; index < games->count; index++) {
        if (strcmp(games->names[index], name) == 0) return 1;
    }
    return 0;
}

static void run_uci_commands(const char *commands)
{
    FILE *input = tmpfile();
    assert(input != NULL);
    assert(fputs(commands, input) >= 0);
    assert(fseek(input, 0, SEEK_SET) == 0);
    assert(uci_run(input) == 0);
    assert(fclose(input) == 0);
}

static void assert_new_export(const TestGames *before, const char *expected)
{
    TestGames after;
    int new_count = 0;
    list_games(&after);
    for (int index = 0; index < after.count; index++) {
        if (!has_game(before, after.names[index])) {
            char path[TEST_FILENAME_SIZE + 8];
            char contents[4096];
            snprintf(path, sizeof(path), "games/%s", after.names[index]);
            assert(read_file(path, contents, sizeof(contents)));
            assert(strstr(contents, expected) != NULL);
            assert(remove(path) == 0);
            new_count++;
        }
    }
    assert(new_count == 1);
}
static void make_test_path(char path[128])
{
    unsigned long stamp = (unsigned long)time(NULL);
    int suffix = 0;
    FILE *file;

    do {
        snprintf(path, 128, "build/pgn-test-%lu-%d.txt", stamp, suffix++);
        file = fopen(path, "r");
        if (file != NULL) fclose(file);
    } while (file != NULL);
}

static int read_file(const char *path, char *contents, size_t size)
{
    FILE *file = fopen(path, "rb");
    size_t read_count;
    if (file == NULL) return 0;
    read_count = fread(contents, 1, size - 1, file);
    contents[read_count] = '\0';
    return fclose(file) == 0;
}

static int make_fen_game(PgnGame *game, const char *fen, const char *side,
                         const char *castling, const char *ep,
                         const char *halfmove, const char *fullmove,
                         int move_count, char *moves[])
{
    char *root_args[] = {
        "fen", (char *)fen, (char *)side, (char *)castling, (char *)ep,
        (char *)halfmove, (char *)fullmove
    };
    UciPosition root;
    return uci_position_parse(&root, 7, root_args)
        && pgn_game_set_position(game, &root, move_count, moves);
}

void test_pgn_history_export_and_duplicate_prevention(void)
{
    PgnGame game = {0};
    UciPosition root;
    char *moves[] = {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5"};
    char path[128];
    char contents[4096];
    long before;
    long after;
    FILE *file;

    uci_position_start(&root);
    assert(pgn_game_set_position(&game, &root, 5, moves));
    assert(strcmp(game.san[0], "e4") == 0);
    assert(strcmp(game.san[4], "Bb5") == 0);

    make_test_path(path);
    assert(pgn_game_export_to_file(&game, "1/2-1/2", path));
    file = fopen(path, "ab");
    assert(file != NULL);
    fputs("sentinel", file);
    assert(fclose(file) == 0);
    file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    before = ftell(file);
    assert(fclose(file) == 0);
    assert(pgn_game_export_to_file(&game, "1/2-1/2", path));
    file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    after = ftell(file);
    assert(fclose(file) == 0);
    assert(before == after);

    assert(read_file(path, contents, sizeof(contents)));
    assert(strstr(contents, "Sunfish-C game") != NULL);
    assert(strstr(contents, "1/2-1/2") != NULL);
    assert(strstr(contents, "1. e4 e5 2. Nf3 Nc6 3. Bb5 1/2-1/2") != NULL);
    assert(pgn_game_export_to_file(&game, "0-1", path));
    assert(read_file(path, contents, sizeof(contents)));
    assert(strstr(contents, "0-1") != NULL);
    assert(strstr(contents, "sentinel") == NULL);
    assert(remove(path) == 0);
}
void test_pgn_san_disambiguation_and_special_moves(void)
{
    PgnGame game = {0};
    char *knight_move[] = {"b1d2"};
    char *castle[] = {"e1g1"};
    char *black_castle[] = {"e8g8"};
    char *en_passant[] = {"e5d6"};
    char *promotion[] = {"a7a8q"};

    assert(make_fen_game(&game, "4k3/8/8/8/8/8/8/1N3N1K", "w", "-", "-",
                         "0", "1", 1, knight_move));
    assert(strcmp(game.san[0], "Nbd2") == 0);

    assert(make_fen_game(&game, "4k3/8/8/8/8/8/8/4K2R", "w", "K", "-", "0",
                         "1", 1, castle));
    assert(strcmp(game.san[0], "O-O") == 0);

    assert(make_fen_game(&game, "4k2r/8/8/8/8/8/8/4K3", "b", "k", "-", "0",
                         "1", 1, black_castle));
    assert(strcmp(game.san[0], "O-O") == 0);

    assert(make_fen_game(&game, "4k3/8/8/3pP3/8/8/8/4K3", "w", "-", "d6",
                         "0", "1", 1, en_passant));
    assert(strcmp(game.san[0], "exd6") == 0);

    assert(make_fen_game(&game, "4k3/P7/8/8/8/8/8/4K3", "w", "-", "-", "0",
                         "1", 1, promotion));
    assert(strcmp(game.san[0], "a8=Q+") == 0);
}

void test_pgn_terminal_checkmate_and_stalemate_results(void)
{
    PgnGame game = {0};
    UciPosition root;
    char *fools_mate[] = {"f2f3", "e7e5", "g2g4", "d8h4"};
    char result[8];
    char path[128];
    char contents[2048];

    uci_position_start(&root);
    assert(pgn_game_set_position(&game, &root, 4, fools_mate));
    assert(strcmp(game.san[3], "Qh4#") == 0);
    assert(pgn_game_terminal_result(&game, result));
    assert(strcmp(result, "0-1") == 0);
    make_test_path(path);
    assert(pgn_game_export_to_file(&game, result, path));
    assert(read_file(path, contents, sizeof(contents)));
    assert(strstr(contents, "0-1") != NULL);
    assert(strstr(contents, "2. g4 Qh4# 0-1") != NULL);
    assert(remove(path) == 0);

    assert(make_fen_game(&game, "7k/5Q2/6K1/8/8/8/8/8", "b", "-", "-", "0",
                         "1", 0, NULL));
    assert(pgn_game_terminal_result(&game, result));
    assert(strcmp(result, "1/2-1/2") == 0);
}

void test_pgn_fen_root_and_black_move_number(void)
{
    PgnGame game = {0};
    char *moves[] = {"e7e5"};
    char path[128];
    char contents[2048];

    assert(make_fen_game(&game, "4k3/4p3/8/8/8/8/8/4K3", "b", "-", "-", "0",
                         "7", 1, moves));
    assert(strcmp(game.san[0], "e5") == 0);
    make_test_path(path);
    assert(pgn_game_export_to_file(&game, "1-0", path));
    assert(read_file(path, contents, sizeof(contents)));
    assert(strstr(contents,
                  "[FEN \"4k3/4p3/8/8/8/8/8/4K3 b - - 0 7\"]") != NULL);
    assert(strstr(contents, "7... e5 1-0") != NULL);
    assert(remove(path) == 0);
}

void test_pgn_incomplete_position_and_reset_do_not_export(void)
{
    PgnGame game = {0};
    UciPosition root;
    char *moves[] = {"e2e4"};
    char result[8];

    uci_position_start(&root);
    assert(pgn_game_set_position(&game, &root, 1, moves));
    assert(!pgn_game_terminal_result(&game, result));
    assert(!pgn_game_export_to_file(&game, "1-0", ""));
    pgn_game_clear(&game);
    assert(!pgn_game_terminal_result(&game, result));
    assert(!pgn_game_export_to_file(&game, "1-0", "build/should-not-exist.txt"));
}

void test_uci_go_stop_quit_leave_game_incomplete(void)
{
    TestGames before;
    TestGames after;

    list_games(&before);
    run_uci_commands("position startpos moves e2e4\n"
                     "go depth 1\n"
                     "stop\n"
                     "ucinewgame\n"
                     "position startpos moves d2d4\n"
                     "quit\n");
    list_games(&after);
    assert(after.count == before.count);
    for (int index = 0; index < before.count; index++) {
        assert(has_game(&after, before.names[index]));
    }
    assert(!search_stop_requested());
}

void test_uci_gameover_and_terminal_position_export(void)
{
    TestGames before;

    list_games(&before);
    run_uci_commands("position startpos moves e2e4 e7e5\n"
                     "gameover 1/2-1/2\n"
                     "gameover 1/2-1/2\n"
                     "quit\n");
    assert_new_export(&before, "1. e4 e5 1/2-1/2");

    list_games(&before);
    run_uci_commands("position startpos moves f2f3 e7e5 g2g4 d8h4\n"
                     "position startpos moves f2f3 e7e5 g2g4 d8h4\n"
                     "quit\n");
    assert_new_export(&before, "2. g4 Qh4# 0-1");
}
