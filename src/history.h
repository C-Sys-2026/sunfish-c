
#ifndef HISTORY_H
#define HISTORY_H

int history_clear(const char *filename);

int history_save_move(const char *filename,
                      const char *san,
                      int move_number,
                      int is_white);

#endif
