#include "bench.h"

#include "search.h"
#include "tt.h"
#include "uci_api.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct {
    const char *name;
    const char *fen;
} BenchFixture;

int bench_main(void)
{
    static const BenchFixture fixtures[] = {
        {"startpos",
         "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
        {"middlegame",
         "r1bq1rk1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPP2PPP/R1BQ1RK1 w - - 0 8"},
        {"tactical",
         "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 9"}
    };
    static const char *configs[] = {
        "TT_off_pruning_off", "TT_on_pruning_off",
        "TT_off_pruning_on", "TT_on_pruning_on"
    };
    const int tt_values[] = {0, 1, 0, 1};
    const int pruning_values[] = {0, 0, 1, 1};
    FILE *csv = fopen("evidence/benchmark.csv", "w");

    printf("suite,configuration,time_ms,nodes,nps,depth,score,pv,status,raw_evidence\n");
    if (csv != NULL) {
        fprintf(csv,
                "suite,configuration,time_ms,nodes,nps,depth,score,pv,status,raw_evidence\n");
    }

    for (size_t fixture = 0;
         fixture < sizeof(fixtures) / sizeof(fixtures[0]); fixture++) {
        UciPosition position;
        int parsed;
        if (fixture == 0) {
            uci_position_start(&position);
            parsed = 1;
        } else {
            char *moves1[] = {"startpos", "moves", "e2e4", "e7e5",
                              "g1f3", "b8c6", "f1b5", "g8f6"};
            char *moves2[] = {"startpos", "moves", "d2d4", "d7d5",
                              "c1f4", "c7c6", "e2e3", "e7e6",
                              "g1f3", "g8f6"};
            parsed = uci_position_parse(&position,
                                        fixture == 1 ? 8 : 10,
                                        fixture == 1 ? moves1 : moves2);
        }
        if (!parsed) {
            fprintf(stderr, "bench: invalid fixture '%s'\n",
                    fixtures[fixture].name);
            continue;
        }
        for (size_t config = 0;
             config < sizeof(configs) / sizeof(configs[0]); config++) {
            SearchLimits limits = {0};
            SearchResult result;
            clock_t started;
            long elapsed_ms;
            char pv[512] = "";

            tt_clear();
            search_set_features(tt_values[config], pruning_values[config]);
            search_reset_stop();
            limits.depth = 3;
            limits.time_budget_ms = -1;
            limits.black_to_move = position.black_to_move;
            started = clock();
            result = search_position(&position.position, &limits);
            elapsed_ms = (long)((clock() - started) * 1000 / CLOCKS_PER_SEC);

            for (int i = 0; i < result.pv_length; i++) {
                char move[6];
                if (uci_move_to_uci(result.pv[i],
                                     position.black_to_move ^ (i & 1), move)) {
                    if (pv[0] != '\0') strcat(pv, " ");
                    strcat(pv, move);
                }
            }
            printf("%s,%s,%ld,%llu,%llu,%d,%d,%s,measured,bench/raw.txt\n",
                   fixtures[fixture].name, configs[config], elapsed_ms,
                   result.nodes, result.nps, result.depth, result.score, pv);
            if (csv != NULL) {
                fprintf(csv,
                        "%s,%s,%ld,%llu,%llu,%d,%d,%s,measured,bench/raw.txt\n",
                        fixtures[fixture].name, configs[config], elapsed_ms,
                        result.nodes, result.nps, result.depth, result.score, pv);
            }
        }
    }
    if (csv != NULL) fclose(csv);
    search_set_features(1, 1);
    return 0;
}
