#include "perft.h"

#include "legality.h"

uint64_t perft(Position position, int depth)
{
    Move moves[256];
    int count;
    uint64_t leaves = 0;

    if (depth <= 0) {
        return 1;
    }

    count = gen_legal_moves(&position, moves);
    for (int index = 0; index < count; index++) {
        leaves += perft(make_move(position, moves[index]), depth - 1);
    }
    return leaves;
}
