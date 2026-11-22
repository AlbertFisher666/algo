# REPL golden tests

Each `caseNN.in` is fed to `./build/repl --script caseNN.in`,
output is compared to `caseNN.out` by `tests/run_golden.sh`.
Normalization: `^> ` stripped, file paths → `<FILE>`.

| Case | Covers |
|------|--------|
| 01 | tree: insert/draw/validate/min/max/height/size |
| 02 | tree: balance on degenerate 1..7 → draw |
| 03 | list: push_back/push_front/draw/reverse/draw/sort/draw |
| 04 | hash: create/insert/stats/draw/remove |
| 05 | save/load tree round-trip |
| 06 | save/load list round-trip |
| 07 | save/load hash round-trip |
| 08 | errors: `insert abc`, `height` without tree, `hash 0`, `load nosuchfile` |
| 09 | duplicates: tree/hash → error, list → ok |
| 10 | free + recreate |

To add a case: create `caseNN.in`, run
`./build/repl --script tests/repl_golden/caseNN.in > /tmp/got`,
verify by hand, save as `caseNN.out`, run `make test-repl`.
