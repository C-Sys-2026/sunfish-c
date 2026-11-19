#include "uci_api.h"

#include "eval.h"
#include "hash.h"
#include "pgn.h"
#include "tt.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define UCI_LINE_SIZE 4096
#define UCI_MAX_TOKENS 1024

typedef struct {
    char *items[UCI_MAX_TOKENS];
    int count;
} UciTokens;

// types of uci commands
typedef enum {
    UCI_COMMAND_INVALID,
    UCI_COMMAND_UCI,
    UCI_COMMAND_ISREADY,
    UCI_COMMAND_NEWGAME,
    UCI_COMMAND_POSITION,
    UCI_COMMAND_GO,
    UCI_COMMAND_STOP,
    UCI_COMMAND_GAMEOVER,
    UCI_COMMAND_QUIT
} UciCommand;


static UciPosition current_position;
static PgnGame current_game;

// function for an error
static void report_error(const char *message)
{
    // pritning erros
    fprintf(stderr, "uci: %s\n", message);
}

static int tokenize(char *line, UciTokens *tokens)
{
    char *cursor = line;

    tokens->count = 0;
    while (*cursor != '\0') {
        while (isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor == '\0') {
            break;
        }
        if (tokens->count == UCI_MAX_TOKENS) {
            report_error("too many command tokens");
            return 0;
        }

        tokens->items[tokens->count++] = cursor;
        while (*cursor != '\0' && !isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor != '\0') {
            *cursor++ = '\0';
        }
    }
    return 1;
}

static UciCommand command_from_token(const char *token)
{
    if (strcmp(token, "uci") == 0) return UCI_COMMAND_UCI;
    if (strcmp(token, "isready") == 0) return UCI_COMMAND_ISREADY;
    if (strcmp(token, "ucinewgame") == 0) return UCI_COMMAND_NEWGAME;
    if (strcmp(token, "position") == 0) return UCI_COMMAND_POSITION;
    if (strcmp(token, "go") == 0) return UCI_COMMAND_GO;
    if (strcmp(token, "stop") == 0) return UCI_COMMAND_STOP;
    if (strcmp(token, "gameover") == 0) return UCI_COMMAND_GAMEOVER;
    if (strcmp(token, "quit") == 0) return UCI_COMMAND_QUIT;
    return UCI_COMMAND_INVALID;
}

static int handle_uci(const UciTokens *tokens)
{
    if (tokens->count != 1) {
        report_error("'uci' takes no arguments");
        return 1;
    }
    puts("id name Sunfish-C");
    puts("id author Sunfish-C contributors");
    puts("uciok");
    fflush(stdout);
    return 1;
}

static int handle_isready(const UciTokens *tokens)
{
    if (tokens->count != 1) {
        report_error("'isready' takes no arguments");
        return 1;
    }
    puts("readyok");
    fflush(stdout);
    return 1;
}

static int handle_newgame(const UciTokens *tokens)
{
    if (tokens->count != 1) {
        report_error("'ucinewgame' takes no arguments");
        return 1;
    }

    go_handler_stop();
    go_handler_wait();
    uci_position_start(&current_position);
    pgn_game_clear(&current_game);
    tt_clear();
    return 1;
}

static int handle_position(const UciTokens *tokens)
{
    UciPosition next_position;
    UciPosition root_position;
    int root_token_count;
    int move_count;
    char *const *move_tokens;
    char terminal_result[8];

    if (!uci_position_parse(&next_position, tokens->count - 1,
                            &tokens->items[1])) {
        report_error("invalid position command; previous position retained");
        return 1;
    }

    if (strcmp(tokens->items[1], "startpos") == 0) {
        root_token_count = 1;
    } else {
        root_token_count = 7;
    }
    if (tokens->count - 1 < root_token_count
        || !uci_position_parse(&root_position, root_token_count,
                               &tokens->items[1])) {
        report_error("could not reconstruct accepted position root");
        return 1;
    }
    if (tokens->count > root_token_count + 1
        && strcmp(tokens->items[root_token_count + 1], "moves") == 0) {
        move_count = tokens->count - root_token_count - 2;
        move_tokens = move_count > 0
            ? &tokens->items[root_token_count + 2] : NULL;
    } else {
        move_count = 0;
        move_tokens = NULL;
    }
    if (!pgn_game_set_position(&current_game, &root_position, move_count,
                               move_tokens)) {
        report_error("could not record accepted position history");
        return 1;
    }

    go_handler_stop();
    go_handler_wait();
    current_position = next_position;
    if (pgn_game_terminal_result(&current_game, terminal_result)
        && !pgn_game_export(&current_game, terminal_result)) {
        report_error("could not export completed PGN game");
    }
    return 1;
}

static int handle_go(const UciTokens *tokens)
{
    if (!go_handler_start(&current_position, tokens->count - 1,
                          &tokens->items[1])) {
        return 1;
    }
    return 1;
}

static int handle_stop(const UciTokens *tokens)
{
    if (tokens->count != 1) {
        report_error("'stop' takes no arguments");
        return 1;
    }
    go_handler_stop();
    return 1;
}

static int handle_gameover(const UciTokens *tokens)
{
    if (tokens->count != 2
        || (strcmp(tokens->items[1], "1-0") != 0
            && strcmp(tokens->items[1], "0-1") != 0
            && strcmp(tokens->items[1], "1/2-1/2") != 0)) {
        report_error("'gameover' requires 1-0, 0-1, or 1/2-1/2");
        return 1;
    }
    go_handler_stop();
    go_handler_wait();
    if (!pgn_game_export(&current_game, tokens->items[1])) {
        report_error("no active game or PGN export failed");
    }
    return 1;
}

static int handle_quit(const UciTokens *tokens)
{
    if (tokens->count != 1) {
        report_error("'quit' takes no arguments");
        return 1;
    }
    go_handler_stop();
    go_handler_wait();
    return 0;
}

static int dispatch_command(const UciTokens *tokens)
{
    switch (command_from_token(tokens->items[0])) {
    case UCI_COMMAND_UCI:
        return handle_uci(tokens);
    case UCI_COMMAND_ISREADY:
        return handle_isready(tokens);
    case UCI_COMMAND_NEWGAME:
        return handle_newgame(tokens);
    case UCI_COMMAND_POSITION:
        return handle_position(tokens);
    case UCI_COMMAND_GO:
        return handle_go(tokens);
    case UCI_COMMAND_STOP:
        return handle_stop(tokens);
    case UCI_COMMAND_GAMEOVER:
        return handle_gameover(tokens);
    case UCI_COMMAND_QUIT:
        return handle_quit(tokens);
    case UCI_COMMAND_INVALID:
    default:
        fprintf(stderr, "uci: unknown command '%s'\n", tokens->items[0]);
        return 1;
    }
}

int uci_run(FILE *input)
{
    char line[UCI_LINE_SIZE];

    if (input == NULL) {
        return 1;
    }
    pad_tables();
    init_king_tables();
    init_zobrist();
    tt_clear();
    uci_position_start(&current_position);
    pgn_game_clear(&current_game);

    while (fgets(line, sizeof(line), input) != NULL) {
        size_t length = strlen(line);
        UciTokens tokens;

        if (length > 0 && line[length - 1] != '\n' && !feof(input)) {
            int character;
            while ((character = fgetc(input)) != '\n' && character != EOF) {
            }
            report_error("command line is too long");
            continue;
        }
        while (length > 0
               && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
            line[--length] = '\0';
        }
        if (!tokenize(line, &tokens) || tokens.count == 0) {
            continue;
        }
        if (!dispatch_command(&tokens)) {
            break;
        }
    }

    go_handler_stop();
    go_handler_wait();
    search_reset_stop();
    return ferror(input) ? 1 : 0;
}

int uci_main(void)
{
    return uci_run(stdin);
}
