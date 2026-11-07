
#include "history.h"

#include <stdio.h>

int history_clear(const char *filename) {
    // TODO 1: Open the file in write mode.
    FILE *file = fopen(filename, "w");

    // TODO 2: Return 0 if opening fails.
    if (!file) {
        return 0;
    }

    // TODO 3: Close the file and return 1.
    fclose(file);
    return 1;
}

int history_save_move(const char *filename,
                      const char *san,
                      int move_number,
                      int is_white) {

    // TODO 4: Open the file in append mode.
    FILE *file = fopen(filename, "a");

    // TODO 5: Return 0 if opening fails.
    if (!file) {
        return 0;
    }

    // TODO 6: Write the move in SAN.
    //
    // White: "1. e4 "
    // Black: "e5\n"
    //
    // Hint: Use fprintf() with different formats.
    if (is_white) {
        fprintf(file, "%d. %s ", move_number, san);
    } else {
        fprintf(file, "%s\n", san);
    }

    // TODO 7: Close the file and return 1.
    fclose(file);
    return 1;
}
