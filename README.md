# Sunfish-C

Sunfish-C is a small, explainable chess engine written in C, inspired by
[Sunfish](https://github.com/thomasahle/sunfish).

## Repository layout

- `src/` - C source files
- `include/` - public headers
- `tests/` - rule and engine tests
- `docs/` - project documentation
- `bench/` - reproducible benchmark inputs and results
- `build/` - generated build output (not committed)

## Requirements

- A C11 compiler
- GNU Make or a compatible `make` implementation

## Build

The default target creates a debug build:

```sh
make
```

A release build is available with:

```sh
make release
```

Both configurations compile with `-Wall -Wextra -Wpedantic -std=c11`.

Run the fixed benchmark suite and regenerate `evidence/benchmark.csv`:

```sh
make bench
```
The executable is written to `build/sunfish-c`.

## PGN export

The UCI loop records the complete move list from each accepted `position`
command. Checkmate and stalemate are exported automatically when supplied as
the final position; external resignations and agreed draws are not part of UCI,
so send the explicit extension `gameover 1-0`, `gameover 0-1`, or
`gameover 1/2-1/2` to finish and export those games. PGNs are written as
timestamp-based `.txt` files in `games/`. `go`, `stop`, EOF, and `quit` do not
finish or export a game, and `ucinewgame` clears the current history.

Remove generated output with:

```sh
make clean
```

"# sunfish-c" 
