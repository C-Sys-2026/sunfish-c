
#include "san.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int move_to_san(const Position *pos, Move move,
                char *output, size_t size) {

    if (pos == NULL || output == NULL || size == 0) {
        return 0;
    }

    if (move.i < 0 || move.i >= 120 ||
        move.j < 0 || move.j >= 120) {
        return 0;
    }

    char piece = pos->board[move.i];

    char from_file = 'a' + (move.i % 10) - 1;
    char to_file = 'a' + (move.j % 10) - 1;
    int to_rank = 10 - (move.j / 10);

    int capture = 0;

    if (islower((unsigned char)pos->board[move.j])) {
        capture = 1;
    } else if (piece == 'P' &&
               move.j == pos->ep &&
               pos->ep != 0) {
        capture = 1;
    }

    int written = -1;

    // Castling
    if (piece == 'K' && abs(move.j - move.i) == 2) {

        written = snprintf(output, size, "%s",
                           move.j > move.i ? "O-O" : "O-O-O");

    } else if (piece == 'P') {

        char promotion = '\0';

        if (move.prom != '\0' && move.prom != ' ') {
            promotion = (char)toupper((unsigned char)move.prom);

            if (promotion != 'Q' && promotion != 'R' &&
                promotion != 'B' && promotion != 'N') {
                return 0;
            }
        }

        if (promotion != '\0') {

            if (capture) {
                written = snprintf(output, size,
                                   "%cx%c%d=%c",
                                   from_file, to_file,
                                   to_rank, promotion);
            } else {
                written = snprintf(output, size,
                                   "%c%d=%c",
                                   to_file, to_rank, promotion);
            }

        } else if (capture) {

            written = snprintf(output, size,
                               "%cx%c%d",
                               from_file, to_file, to_rank);

        } else {

            written = snprintf(output, size,
                               "%c%d", to_file, to_rank);
        }

    } else {

        // Other pieces: Nf3, Bxe6, etc.
        if (capture) {
            written = snprintf(output, size,
                               "%cx%c%d",
                               piece, to_file, to_rank);
        } else {
            written = snprintf(output, size,
                               "%c%c%d",
                               piece, to_file, to_rank);
        }
    }

    return written >= 0 && (size_t)written < size;
}
