#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static HANDLE search_thread;
#else
#include <pthread.h>
static pthread_t search_thread;
#endif

#include "uci_api.h"

#include "search.h"
#include "tt.h"

typedef struct {
    UciPosition position;
    SearchLimits limits;
} SearchRequest;

static SearchRequest search_request;
static int search_thread_started;

static int parse_nonnegative(const char *text, int allow_zero, int *value)
{
    char *end = NULL;
    long parsed;

    if (text == NULL || *text == '\0') {
        return 0;
    }
    for (const char *cursor = text; *cursor != '\0'; cursor++) {
        if (*cursor < '0' || *cursor > '9') {
            return 0;
        }
    }
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed > INT_MAX
        || (!allow_zero && parsed == 0)) {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

static int set_limit(int *target, int *seen, int bit, const char *value,
                     int allow_zero, const char *option)
{
    if ((*seen & bit) != 0) {
        fprintf(stderr, "uci: duplicate go option '%s'\n", option);
        return 0;
    }
    if (!parse_nonnegative(value, allow_zero, target)) {
        fprintf(stderr, "uci: invalid number for go option '%s'\n", option);
        return 0;
    }
    *seen |= bit;
    return 1;
}

static int parse_limits(int argc, char *const argv[], int black_to_move,
                        SearchLimits *limits)
{
    enum {
        SEEN_DEPTH = 1 << 0,
        SEEN_MOVETIME = 1 << 1,
        SEEN_WTIME = 1 << 2,
        SEEN_BTIME = 1 << 3,
        SEEN_WINC = 1 << 4,
        SEEN_BINC = 1 << 5,
        SEEN_MOVESTOGO = 1 << 6
    };
    int white_time = 0;
    int black_time = 0;
    int white_increment = 0;
    int black_increment = 0;
    int movestogo = 30;
    int movetime = -1;
    int seen = 0;
    int active_clock_seen;
    int depth_seen = 0;
    int index = 0;

    memset(limits, 0, sizeof(*limits));
    limits->depth = 64;
    limits->time_budget_ms = -1;
    limits->black_to_move = black_to_move;

    while (index < argc) {
        const char *option = argv[index++];
        int *target = NULL;
        int bit = 0;
        int allow_zero = 1;

        if (strcmp(option, "infinite") == 0) {
            limits->infinite = 1;
            continue;
        }
        if (index >= argc) {
            fprintf(stderr, "uci: missing value for go option '%s'\n", option);
            return 0;
        }

        if (strcmp(option, "depth") == 0) {
            target = &limits->depth;
            bit = SEEN_DEPTH;
            allow_zero = 0;
        } else if (strcmp(option, "movetime") == 0) {
            target = &movetime;
            bit = SEEN_MOVETIME;
        } else if (strcmp(option, "wtime") == 0) {
            target = &white_time;
            bit = SEEN_WTIME;
        } else if (strcmp(option, "btime") == 0) {
            target = &black_time;
            bit = SEEN_BTIME;
        } else if (strcmp(option, "winc") == 0) {
            target = &white_increment;
            bit = SEEN_WINC;
        } else if (strcmp(option, "binc") == 0) {
            target = &black_increment;
            bit = SEEN_BINC;
        } else if (strcmp(option, "movestogo") == 0) {
            target = &movestogo;
            bit = SEEN_MOVESTOGO;
            allow_zero = 0;
        } else {
            fprintf(stderr, "uci: unsupported go option '%s'\n", option);
            return 0;
        }

        if (!set_limit(target, &seen, bit, argv[index++], allow_zero, option)) {
            return 0;
        }
        if (bit == SEEN_DEPTH) {
            depth_seen = 1;
            if (limits->depth > UCI_MAX_PV) {
                fprintf(stderr, "uci: depth exceeds supported maximum %d\n",
                        UCI_MAX_PV);
                return 0;
            }
        }
    }

    active_clock_seen = black_to_move
        ? (seen & SEEN_BTIME) != 0 : (seen & SEEN_WTIME) != 0;
    if (active_clock_seen && !limits->infinite) {
        int remaining = black_to_move ? black_time : white_time;
        int increment = black_to_move ? black_increment : white_increment;
        long long allocation;

        allocation = (long long)remaining / movestogo
            + ((long long)increment * 3) / 4;
        if (remaining > 0 && allocation >= remaining) {
            allocation = remaining > 5 ? remaining - 5 : 1;
        }
        if (allocation < 1) {
            allocation = 1;
        }
        limits->time_budget_ms = (int)allocation;
    }

    if (!limits->infinite && movetime >= 0
        && (limits->time_budget_ms < 0 || movetime < limits->time_budget_ms)) {
        limits->time_budget_ms = movetime > 0 ? movetime : 1;
    }

    if (!depth_seen && !active_clock_seen && movetime < 0
        && !limits->infinite) {
        limits->depth = 6;
    }
    if (limits->infinite && !depth_seen) {
        limits->depth = UCI_MAX_PV;
    }
    return 1;
}

static void report_info(const SearchResult *result, void *context)
{
    const SearchRequest *request = (const SearchRequest *)context;
    format_search_info(result, request->position.black_to_move);
}

static void run_search(void)
{
    SearchResult result;

    search_request.limits.info_callback = report_info;
    search_request.limits.info_context = &search_request;
    result = search_position(&search_request.position.position,
                            &search_request.limits);
    format_bestmove(&result, search_request.position.black_to_move);
}

#ifdef _WIN32
static DWORD WINAPI search_thread_entry(LPVOID context)
{
    (void)context;
    run_search();
    return 0;
}
#else
static void *search_thread_entry(void *context)
{
    (void)context;
    run_search();
    return NULL;
}
#endif

void go_handler_stop(void)
{
    search_request_stop();
}

void go_handler_wait(void)
{
    if (!search_thread_started) {
        return;
    }
#ifdef _WIN32
    WaitForSingleObject(search_thread, INFINITE);
    CloseHandle(search_thread);
    search_thread = NULL;
#else
    pthread_join(search_thread, NULL);
#endif
    search_thread_started = 0;
}

int go_handler_start(const UciPosition *position, int argc,
                     char *const argv[])
{
    SearchLimits limits;

    if (position == NULL) {
        fprintf(stderr, "uci: cannot search a null position\n");
        return 0;
    }
    if (argc < 0 || (argc > 0 && argv == NULL)) {
        fprintf(stderr, "uci: invalid go command arguments\n");
        return 0;
    }
    if (!parse_limits(argc, argv, position->black_to_move, &limits)) {
        return 0;
    }

    go_handler_stop();
    go_handler_wait();
    search_reset_stop();
    search_request.position = *position;
    search_request.limits = limits;

#ifdef _WIN32
    search_thread = CreateThread(NULL, 0, search_thread_entry, NULL, 0, NULL);
    if (search_thread == NULL) {
        fprintf(stderr, "uci: could not start search thread (error %lu)\n",
                (unsigned long)GetLastError());
        return 0;
    }
#else
    {
        int error = pthread_create(&search_thread, NULL, search_thread_entry,
                                   NULL);
        if (error != 0) {
            fprintf(stderr, "uci: could not start search thread (%d)\n", error);
            return 0;
        }
    }
#endif
    search_thread_started = 1;
    return 1;
}

static int mate_in_moves(int score)
{
    if (score > 0) {
        return (MATE_UPPER - score + 1) / 2;
    }
    return -((MATE_UPPER + score + 1) / 2);
}

static void format_uint64(uint64_t value, char output[21])
{
    char reversed[20];
    size_t length = 0;

    do {
        reversed[length++] = (char)('0' + value % 10);
        value /= 10;
    } while (value != 0 && length < sizeof(reversed));

    for (size_t index = 0; index < length; index++) {
        output[index] = reversed[length - index - 1];
    }
    output[length] = '\0';
}

void format_search_info(const SearchResult *result, int black_to_move)
{
    char line[2048];
    char nodes[21];
    char nps[21];
    size_t used;
    int written;
    int is_mate;

    if (result == NULL) {
        return;
    }

    format_uint64(result->nodes, nodes);
    format_uint64(result->nps, nps);
    is_mate = result->score >= MATE_LOWER || result->score <= -MATE_LOWER;
    if (is_mate) {
        written = snprintf(line, sizeof(line),
                           "info depth %d score mate %d nodes %s nps %s hashfull %d",
                           result->depth, mate_in_moves(result->score),
                           nodes, nps, result->hashfull);
    } else {
        written = snprintf(line, sizeof(line),
                           "info depth %d score cp %d nodes %s nps %s hashfull %d",
                           result->depth, result->score,
                           nodes, nps, result->hashfull);
    }
    if (written < 0 || (size_t)written >= sizeof(line)) {
        return;
    }
    used = (size_t)written;

    if (result->pv_length > 0) {
        written = snprintf(line + used, sizeof(line) - used, " pv");
        if (written < 0 || (size_t)written >= sizeof(line) - used) {
            return;
        }
        used += (size_t)written;
    }

    for (int index = 0; index < result->pv_length; index++) {
        char move_text[6];
        if (!uci_move_to_uci(result->pv[index],
                             black_to_move ^ (int)(index & 1), move_text)) {
            break;
        }
        written = snprintf(line + used, sizeof(line) - used, " %s", move_text);
        if (written < 0 || (size_t)written >= sizeof(line) - used) {
            break;
        }
        used += (size_t)written;
    }

    fprintf(stdout, "%s\n", line);
    fflush(stdout);
}

void format_bestmove(const SearchResult *result, int black_to_move)
{
    char move_text[6];

    if (result == NULL || !result->has_best_move
        || !uci_move_to_uci(result->best_move, black_to_move, move_text)) {
        puts("bestmove 0000");
    } else {
        printf("bestmove %s\n", move_text);
    }
    fflush(stdout);
}
