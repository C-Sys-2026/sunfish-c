# UCI walkthrough

## What happens to one command

```text
GUI or user
  -> uci_main() in src/uci.c reads a line
  -> tokenize() separates the words
  -> dispatch_command() selects a command handler
     -> handle_position() calls uci_position_parse()
        -> src/position_handler.c validates FEN and legal coordinate moves
     -> handle_go() calls go_handler_start()
        -> src/go_handler.c parses SearchLimits and starts the worker
        -> search_position(Position *, SearchLimits *) searches iteratively
        -> format_search_info() prints completed-depth information
        -> format_bestmove() prints the final coordinate move
     -> handle_stop() calls go_handler_stop()
        -> the search sees the atomic stop flag and returns its last completed move
```

`uci_main()` owns the input loop. The engine position is held as a `UciPosition`,
which includes the engine's `Position`, side to move, and FEN move counters.
`SearchLimits` carries the depth and computed time budget. `SearchResult`
contains the best legal move, score, node count, completed depth, and principal
variation.

## Commands to demonstrate

Start from the initial position and apply two moves:

```text
position startpos moves e2e4 e7e5
```

Load a FEN position:

```text
position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
```

Ask for a bounded search:

```text
go depth 4
```

For a time-limited GUI search, the engine also accepts `movetime`, `wtime`,
`btime`, `winc`, `binc`, and `movestogo`. To interrupt an active search, send:

```text
stop
```

The engine emits only UCI responses and `info` lines on stdout. Input and
validation errors go to stderr. `position` is parsed into a temporary value;
invalid FEN or moves leave the last valid position unchanged.

## Module ownership

| Module | Responsibility |
|---|---|
| `src/uci.c` | Input loop, tokenizer, command dispatch, handshake, and position ownership |
| `src/position_handler.c` | Start position, FEN, and legal coordinate-move parsing |
| `src/go_handler.c` | Go limits, worker lifecycle, info output, and bestmove output |
| `src/search.c` | Iterative deepening, alpha-beta search, node counting, and stop checks |
| `include/uci_api.h` | Shared position, limits, and result types |
| `tests/test_search_checks.c` | Position parsing, legality, special-move, and stop checks |

Build on Windows with a C11 MinGW compiler:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude -Isrc src/*.c -o build/sunfish-c.exe
```

The Makefile builds the same source set with `make release`.
