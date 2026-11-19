#ifndef PGN_H
#define PGN_H

#include "uci_api.h"

#define PGN_MAX_MOVES 1024
#define PGN_SAN_SIZE 32

typedef struct {
    UciPosition root;
    UciPosition final_position;
    char san[PGN_MAX_MOVES][PGN_SAN_SIZE];
    int move_count;
    int active;
    unsigned long long last_export_fingerprint;
} PgnGame;

void pgn_game_clear(PgnGame *game);
int pgn_game_set_position(PgnGame *game, const UciPosition *root,
                          int move_count, char *const moves[]);
int pgn_game_terminal_result(const PgnGame *game, char result[8]);
int pgn_game_export(PgnGame *game, const char *result);
int pgn_game_export_to_file(PgnGame *game, const char *result,
                            const char *path);

#endif
