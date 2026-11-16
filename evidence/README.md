# UCI acceptance evidence

## Scope and reproducibility

These logs were captured during the earlier local integration run, before the
work was restricted to Krishna-owned files. They are historical evidence, not
validation of the `krishna-uci` branch. That branch intentionally excludes the
shared UCI API and position/search implementations assigned to other owners.
Re-run the acceptance sequence after those dependencies are merged.

## Run metadata

- Compiler: `gcc.exe (MinGW.org GCC-6.3.0-1) 6.3.0`
- Build commands: `mingw32-make release`; also verified directly with
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

## Evidence not available

- Perft results are not recorded: this repository does not yet contain the
  fixed-fixture perft harness from KH7. No expected or observed counts are
  invented; see `perft.csv`.
- Benchmark comparisons are not recorded: no benchmark command, benchmark
  suite, or independent TT/pruning feature switches are present. No speedup or
  Elo claim is made; see `benchmark.csv`.

Implemented behavior is described in the source and walkthrough. Measured
behavior is limited to the captured runs above. Proposed comparisons are not
presented as measured results.
