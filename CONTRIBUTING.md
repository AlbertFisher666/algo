# CONTRIBUTING - algo

Conventions for this repository. Adapted from `~/Documents/self/PROTOCOL.md`
and `af.md` section 5, but **not** a copy: `algo` is a modular library, CP+ is a
monolith. The differences below are deliberate.

## Style

80s C-flavoured C++:

- `struct` + free functions, no classes with methods
- `new` / `delete`, no RAII, no smart pointers
- `NULL`, not `nullptr`
- `printf`, `FILE*`, fixed buffers - no iostream, no `std::string`
- no STL containers, no exceptions
- `-Wall -Wextra` clean, zero warnings

Same as `cpvm.cpp`, same as PROTOCOL section 6.

## Modularity (differs from CP+)

CP+ is a monolith: one file, sections with numbers, never split
(PROTOCOL section 11, af.md section 5). **`algo` is the opposite.** Each structure
gets its own header + source + test:

    include/algo/<name>.hpp
    src/<name>.cpp
    tests/<name>_test.cpp

Do not merge `tree.cpp` and `tree_algo.cpp`. Do not put everything in
one file. The modular split is the point of this project (ROADMAP section 0.2).

## API conventions (ROADMAP §0.4)

- mutating: `0` = OK, `-1` = error
- predicates: `1` = yes, `0` = no
- callbacks: return number of nodes visited
- `const*` where the function does not mutate

## One stage = one layer (ROADMAP §0.2)

One stage ends with green tests, working `make`, clean `-Wall -Wextra`,
and a tag (`stageN-...`). Do not mix `tree_algo` and `list_algo` in one
commit.

## Per-session format (PROTOCOL §6)

One session - one small artefact: one function, one test, one section
in README, one commit. If it does not fit in 60 minutes - split it. If
it does not split - it is a direction, not a task. Write it to
`~/notes_tmp/` and come back later.

## Edit protocol (PROTOCOL §6)

1. Backup: `cp file file.bak_YYYYMMDD`
2. Edit one place
3. Save
4. `make` (or `make build/<test>`)
5. `make test`
6. If clean - commit. If error - roll back.

For refactoring: checklist -> script -> run **without** `-i` ->
`diff` -> `mv` -> compile -> test. Same as PROTOCOL section 6.

## What NOT to do (adapted from PROTOCOL §11)

- Do not split `cpvm.cpp` - it is not here, but the principle stands
  in reverse: **do** split `algo`. Do not merge modules.
- Do not add STL, iostream, exceptions, RAII - against style.
- Do not change old APIs (`tree.h`, `list.h`, `hash.h`) - ROADMAP section 0.6.
  Only add new modules.
- Do not commit with `build/`, `*.o`, `*.repl`, `*.bak` - `.gitignore`
  covers it, but check `git status` before `git add`.
- Do not tag before `make test` is green on a clean tree.

## Git (ROADMAP §0.1)

Each stage ends with a tag: `stage0-baseline`, `stage2-tree-algo`, ...
Current: `v1.0`.

Remote: `git@github.com:AlbertFisher666/algo.git`, branch `main`.
`project-scan.sh` section 7 checks upstream - keep it `OK up to date`.

## Docs

- `README.md` - user-facing: build, API, REPL, tests, CI, license.
- `ALGO_EXPLAINED.md` - deep dive: C++ minimum, data structures,
  algo layer, REPL, batch modes, tests, pitfalls.
- `ROADMAP.md` - plan and status: stages, DoD, Next.
- `CONTRIBUTING.md` - this file: conventions, modularity, edit protocol.

When code changes, update the doc that describes it. `README` is the
contract, `ALGO_EXPLAINED` is the explanation, `ROADMAP` is the plan.
