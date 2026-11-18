#include "chess.h"
#include "uci_api.h"
#include <string.h>

int chess_run(void)
{
    return uci_main();
}

int bench_main(void);

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "bench") == 0) {
        return bench_main();
    }
    return chess_run();
}
