
#ifndef EVAL_H // if not defined then define EVAL_H
#define EVAL_H

extern const int piece[128];
extern int pst[128][120];

extern int K_MID[120];
extern int K_END[120];

void pad_tables(void);
void init_king_tables(void);

#endif
