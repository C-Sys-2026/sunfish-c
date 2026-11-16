#include "chess.h"
#include "uci_api.h"

int chess_run(void)
{
    return uci_main();
}

int main(void)
{
    return chess_run();
}
