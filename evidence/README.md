# UCI acceptance evidence

## Scope and reproducibility

`test-run-*`, `uci-*`, `perft.csv`, and `benchmark.csv` are snapshot evidence
from the submitted implementation. The prior 73-test log is retained as
historical evidence; the current clean run is 82 of 82 tests passing.

## Run metadata

- Compiler: see `test-run-command.txt`
- Build commands: `mingw32-make clean && mingw32-make test && mingw32-make release`;
  benchmark command: `mingw32-make bench`
- Also verified directly with
  `gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude -Isrc src/*.c -o <temporary-path>\sunfish-uci.exe`
- Test command: `mingw32-make test`
- Machine: Windows 11 Education, version `10.0.26200.0`; computer model not captured.

## Evidence files

- `uci-commands.txt`, `uci-stdout.txt`, and `uci-stderr.txt` are the captured
  clean UCI acceptance run. The stderr file is empty.
- `parser-error-commands.txt` and the paired stdout/stderr files show that blank,
  unknown, malformed-number, and invalid-position input does not add protocol
  text to stdout.
- `test-run-stdout.txt` and `test-run-stderr.txt` contain the full earlier
  integration test run, which completed with 73 of 73 tests passing.
- `search-legality.csv` summarizes the search-legality checks and points to the
  raw test log.

## Fresh evidence

- `perft.csv` records fixed start-position and endgame counts verified against
  the current test suite.
- `benchmark.csv` and `bench/raw.txt` record the fixed three-position,
  four-configuration benchmark run. TT and pruning are compared independently;
  no Elo or unsupported speedup claim is made.

Implemented behavior is described in the source and walkthrough. Measured
behavior is limited to the captured runs above. Proposed comparisons are not
presented as measured results.
