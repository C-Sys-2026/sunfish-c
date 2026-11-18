#ifndef UCI_API_H
#define UCI_API_H

#include "position.h"

#define UCI_MAX_PV 64

typedef struct {
    Position position;
    int black_to_move;
    int halfmove_clock;
    int fullmove_number;
} UciPosition;

typedef struct SearchResult SearchResult;
typedef void (*SearchInfoCallback)(const SearchResult *result, void *context);

typedef struct {
    int depth;
    int time_budget_ms;
    int black_to_move;
    int infinite;
    SearchInfoCallback info_callback;
    void *info_context;
} SearchLimits;

struct SearchResult {
    int depth;
    int score;
    int has_best_move;
    Move best_move;
    int pv_length;
    Move pv[UCI_MAX_PV];
    unsigned long long nodes;
    unsigned long long nps;
};

int uci_main(void);
void uci_position_start(UciPosition *position);
int uci_position_parse(UciPosition *position, int argc, char *const argv[]);
int uci_move_from_uci(const char *text, int black_to_move, Move *move);
int uci_move_to_uci(Move move, int black_to_move, char output[6]);

void search_reset_stop(void);
void search_request_stop(void);
SearchResult search_position(const Position *position,
                             const SearchLimits *limits);

int go_handler_start(const UciPosition *position, int argc,
                     char *const argv[]);
void go_handler_stop(void);
void go_handler_wait(void);
void format_search_info(const SearchResult *result, int black_to_move);
void format_bestmove(const SearchResult *result, int black_to_move);

#endif
