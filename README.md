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
The executable is written to `build/sunfish-c`.

Remove generated output with:

```sh
make clean
```

The current M0 skeleton only verifies the project layout and build setup.
Chess representation, move generation, evaluation, search, and UCI support
will be added in later milestones.
"# sunfish-c" 
