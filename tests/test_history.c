
#include <assert.h>
#include "../third_party/munit/munit.h"
#define assert(expr) munit_assert(expr)
#include <stdio.h>
#include <string.h>

#include "../src/history.h"

void test_san_history(void) {
    const char *filename = "test_history.txt";

    assert(history_clear(filename) == 1);

    // Save a short opening in SAN.
    assert(history_save_move(filename, "e4", 1, 1) == 1);
    assert(history_save_move(filename, "e5", 1, 0) == 1);

    assert(history_save_move(filename, "Nf3", 2, 1) == 1);
    assert(history_save_move(filename, "Nc6", 2, 0) == 1);

    FILE *file = fopen(filename, "r");
    assert(file != NULL);

    char line[128];

    assert(fgets(line, sizeof(line), file) != NULL);
    assert(strcmp(line, "1. e4 e5\n") == 0);

    assert(fgets(line, sizeof(line), file) != NULL);
    assert(strcmp(line, "2. Nf3 Nc6\n") == 0);

    fclose(file);
    remove(filename);

    printf("PASS: SAN move history saved correctly\n");
}

#if 0
int main(void) {
    test_san_history();

    printf("All history tests passed!\n");

    return 0;
}
#endif
