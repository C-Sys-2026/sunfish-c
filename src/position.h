
#ifndef POSITION_H
#define POSITION_H

#include "board.h"

typedef struct {
    int i;
    int j;
    char prom;
} Move;

typedef struct {
    char board[121];
    int score;
    int wc[2];
    int bc[2];
    int ep;
    int kp;
} Position;

Position rotate(Position pos, int nullmove);

#endif
