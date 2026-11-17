# ROADMAP — algo v1.0

Цель: превратить `algo` из набора трёх структур в **библиотеку структур + алгоритмов**
с проверяемым REPL. Направление: **A (Algo Core) + C (REPL 2.0)**.

- A: слой алгоритмов над tree/list/hash (`*_algo.{hpp,cpp}`).
- C: REPL получает batch-режимы `--exec` / `--script` и golden-тесты.

Стиль сохраняется: 80s C-flavoured C++, `struct`, свободные функции,
`new/delete`, `NULL`, `printf`, `FILE*`, фиксированные буферы. Без STL,
без исключений, без RAII.

Git — вне scope этого документа (ведётся отдельно).

---

## 0. Принципы

1. Каждый этап заканчивается **зелёными тестами** и рабочим `make`.
2. Один этап = один логический слой. Не смешивать tree_algo и list_algo в одном коммите.
3. Новый модуль = `<struct>_algo.{hpp,cpp}` + `<struct>_algo_test.cpp`.
4. API-конвенции:
   - мутирующие: `0` — OK, `-1` — ошибка;
   - предикаты: `1` — да, `0` — нет;
   - обходы с колбэком: возвращают число посещённых узлов;
   - `const*` там, где функция не мутирует.
5. `-Wall -Wextra` — ноль предупреждений в новом коде.
6. Старые API (`tree.h`, `list.h`, `hash.h`) **не меняются**. Только добавляются новые модули.

---

## 1. Карта этапов

| # | Этап | Выход | Оценка |
|---|------|-------|--------|
| 0 | Baseline | тег/точка отката | 0.5д |
| 1 | Каркас сборки | Makefile + конвенции + пустой модуль | 0.5–1д |
| 2 | Tree Algo | `tree_algo` + тесты | 1–2д |
| 3 | List Algo | `list_algo` + тесты | 1–2д |
| 4 | Hash Algo | `hash_algo` + тесты | 0.5–1д |
| 5 | REPL: команды | новые команды в REPL | 1–2д |
| 6 | REPL: batch | `--exec` / `--script` | 1–2д |
| 7 | Golden-тесты | `tests/repl_golden/` + `run_golden.sh` | 1д |
| 8 | Docs + v1.0 | README, EXPLAINED, тег | 1д |

Итого: ~7–11 дней спокойного темпа. Каждый этап — отдельная точка остановки.

---

## 2. Этап 0 — Baseline

**Цель:** зафиксировать «v0.1 — done», чтобы было куда откатиться.

Шаги:
1. Убедиться, что `make test` заканчивается `all tests passed`.
2. Собрать REPL по README вручную, прогнать сценарий:
   `tree` → `insert 5` → `insert 3` → `draw` → `save demo.repl` → `free` → `load demo.repl` → `draw` → `quit`.
3. Положить этот `ROADMAP.md` в корень.
4. В README в статусе добавить строку `v0.1 — baseline`.

**DoD:** проект собирается, тесты зелёные, ROADMAP на месте.

---

## 3. Этап 1 — Каркас сборки и конвенции

**Цель:** новые модули добавляются автоматически, без ручного `g++`.

Шаги:
1. Заменить/дополнить Makefile (готовый — в приложении A).
   Ключевое: `TEST_SRCS := $(wildcard tests/*_test.cpp)`,
   `TEST_BINS := $(patsubst tests/%.cpp,build/%,$(TEST_SRCS))`.
2. Добавить цели: `all`, `test`, `repl`, `test-repl` (заглушка до этапа 7), `clean`.
3. Создать пустые `include/algo/tree_algo.hpp` и `src/tree_algo.cpp`
   (только guard + header comment + пустое тело). Убедиться, что `make` проходит.
4. Записать конвенции (раздел 0 этого файла) в `CONTRIBUTING.md` или оставить здесь.

**DoD:** `make clean && make && make test` зелёный; пустой `tree_algo` компилируется и линкуется.

---

## 4. Этап 2 — Tree Algo

**Цель:** алгоритмический слой над BST.

### 4.1 API (`include/algo/tree_algo.hpp`)

- `int tree_validate(const Tree* t)` — проверка BST-инварианта с границами `long long`.
- `int tree_min(const Tree* t, int* out)` / `int tree_max(...)` — `1` при успехе, `0` если пусто.
- `int tree_to_array_inorder(const Tree* t, int* buf, int cap)` — заполняет буфер, возвращает `t->size`.
- `int tree_from_sorted_array(Tree* t, const int* keys, int n)` — строит идеально сбалансированное дерево напрямую через `new TreeNode` (не через `tree_insert`).
- `int tree_balance(Tree* t)` — in-order → массив → `tree_free` → `from_sorted_array`.
- `typedef void (*TreeVisitFn)(int key, void* user);`
  `int tree_bfs(const Tree* t, TreeVisitFn cb, void* user)` — очередь на массиве `TreeNode*[t->size]`, уровень-порядок.

### 4.2 Реализация (`src/tree_algo.cpp`)

Порядок: `validate` → `min/max` → `to_array_inorder` → `from_sorted_array` → `balance` → `bfs`.
Внутренние хелперы — `static`. Никакого `std::`.

Ключевые приёмы:
- `validate_node` принимает `long long lo, hi`; при спуске влево `hi = n->key - 1`, вправо `lo = n->key + 1` (или просто строгие `<`/`>` — тогда `lo = n->key`, `hi = n->key` и проверка `n->key > lo && n->key < hi`; выбрать один вариант и задокументировать).
- `build_balanced(a, lo, hi)` — рекурсивный, `mid = lo + (hi - lo) / 2`.
- `bfs` — массив-очередь, `head/tail` индексы, без `std::queue`.

### 4.3 Тесты (`tests/tree_algo_test.cpp`)

| Тест | Ожидание |
|---|---|
| `test_validate_empty` | `1` |
| `test_validate_good` | `1` на 5/3/8/1/4/7/9 |
| `test_validate_broken` | собрать вручную битое (правый ребёнок < родителя) → `0` |
| `test_min_max_empty` | оба `0` |
| `test_min_max` | крайние ключи |
| `test_to_array_inorder` | отсортировано, длина == size |
| `test_from_sorted_array` | height ≤ ⌈log2(n+1)⌉, size == n, validate == 1 |
| `test_balance` | на вырожденном (1..7) высота падает, validate == 1 |
| `test_bfs_order` | колбэк собирает уровне-порядок, сверить с ожидаемым массивом |

**DoD:** `make test` включает `tree_algo_test`, зелёный, `-Wall -Wextra` чистый.

---

## 5. Этап 3 — List Algo

### 5.1 API (`include/algo/list_algo.hpp`)

- `int list_reverse(List* l)` — in place, swap(prev,next) по узлам, swap(head,tail).
- `ListNode* list_middle(List* l)` — two-pointer, для чётного возвращает нижнюю середину.
- `int list_sort(List* l)` — stable merge sort; merge на `next`, потом один проход восстанавливает `prev`, `head`, `tail`.

### 5.2 Реализация (`src/list_algo.cpp`)

- `list_reverse` — итеративно.
- `list_middle` — `slow=head`, `fast=head`; `while (fast && fast->next) { slow=slow->next; fast=fast->next->next; }`.
- `list_sort`:
  - `static ListNode* merge(ListNode* a, ListNode* b)` — стабильное (`<=`), работает по `next`.
  - `static ListNode* split_half(ListNode* head)` — отрезает первую половину, возвращает вторую.
  - `static ListNode* sort_rec(ListNode* head)` — рекурсия `O(log n)` по глубине.
  - после merge — `fix_links`: пройти, выставить `prev`, найти `tail`, выставить `size`.

### 5.3 Тесты (`tests/list_algo_test.cpp`)

- `reverse`: пусто, один, чётное, нечётное — проверить head/tail/prev/next.
- `middle`: пусто → `NULL`, один, чётное (нижняя), нечётное.
- `sort`: пусто, один, случайные 20, уже отсортированные, обратно отсортированные, дубликаты.
- `prev`-цепочка после sort — отдельный тест: пройти tail→head и сверить с head→tail.

**DoD:** аналогично этапу 2.

---

## 6. Этап 4 — Hash Algo

### 6.1 API (`include/algo/hash_algo.hpp`)

- `struct HashStats { int capacity; int size; int min_chain; int max_chain; int empty_buckets; double load_factor; };`
- `int hash_stats(const Hash* h, HashStats* out)` — `0`/`-1`.

### 6.2 Реализация
Один проход по бакетам; `min_chain` считается только по непустым (или `0` для пустой таблицы — зафиксировать в комментарии).

### 6.3 Тесты
Пустая (capacity=8), capacity=1 с тремя ключами (`max_chain==3`, `empty_buckets==0`), равномерная.

**DoD:** аналогично.

---

## 7. Этап 5 — REPL: новые команды

Новые команды (диспетчер в `main` — плоская `strcmp`-цепочка, как сейчас):

| Команда | Структура | Вывод |
|---|---|---|
| `validate` | tree | `valid` / `BROKEN` |
| `min` / `max` | tree | число / `(empty)` |
| `balance` | tree | `ok` / ошибка |
| `reverse` | list | `ok` |
| `sort` | list | `ok` |
| `stats` | hash | многострочно: capacity/size/min_chain/max_chain/empty/load |

Реализация — `static void cmd_*` в стиле существующих, с `need(...)`.
Обновить `cmd_help`. Ручной прогон всех новых + старых команд.

**DoD:** старые команды не сломаны, новые работают, `make test` зелёный (REPL не в unit-тестах, но собирается).

---

## 8. Этап 6 — REPL: `--exec` и `--script`

### 8.1 Рефакторинг
Вынести диспетчеризацию из `main` в:
```cpp
static int execute_line(char* line);  /* 0 ok, 1 err, 2 quit */
```
Интерактивный `main` использует её в цикле.

### 8.2 Режимы (`int main(int argc, char** argv)`)

- без аргументов — интерактив (как сейчас);
- `--exec "<commands>"` — разбить по `;` (свой цикл, не `strtok`, т.к. он мутирует строку — а нам ок, но лучше явно), `execute_line` каждую; стоп на первой ошибке (или `--keep-going`);
- `--script <file>` — читать файл построчно через `fgets`, `execute_line`; стоп на ошибке (или `--keep-going`);
- `--keep-going` — флаг, не останавливаться на ошибках;
- `--help` — краткая справка.

В batch-режимах **не печатать** `> ` и приветствие `algo repl. type 'help'...`.
Код возврата: `0` при успехе, `1` при ошибке команды.

### 8.3 DoD
```sh
./build/repl --exec "tree; insert 5; insert 3; draw"
./build/repl --script tests/repl_golden/case01.in
./build/repl --exec "tree; insert abc"   # rc=1
```
Интерактив не изменился.

---

## 9. Этап 7 — Golden-тесты REPL

### 9.1 Структура
```
tests/repl_golden/
    case01.in   case01.out
    ...
tests/run_golden.sh
```

### 9.2 Нормализация (`run_golden.sh`)
- убрать `> ` (на случай, если попадёт);
- убрать приветствие (в batch его нет — проверить);
- пути файлов → `<FILE>` (для save/load кейсов);
- ASCII-диаграммы **не** нормализовать — они контракт.

### 9.3 Кейсы (10)
1. tree: insert/draw/validate/min/max/height/size
2. tree: balance на вырожденном (1..7 → draw)
3. list: push_back/push_front/draw/reverse/draw/sort/draw
4. hash: create/insert/stats/draw/remove
5. save/load tree round-trip
6. save/load list round-trip
7. save/load hash round-trip
8. ошибки: `insert abc`, `height` без tree, `hash 0`, `load nosuchfile`
9. дубликаты: tree/hash → warning, list → ok
10. `free` + повторное создание

### 9.4 Makefile
`test-repl` зависит от `repl`, запускает `run_golden.sh`.
`make test` = unit + golden.

**DoD:** `make test` зелёный, включая golden.

---

## 10. Этап 8 — Документация и v1.0

1. README: разделы API (tree_algo/list_algo/hash_algo), REPL (новые команды, batch-режимы), Tests (`make test-repl`), статус `v1.0`.
2. `ALGO_EXPLAINED.md`: раздел «Algo layer» (validate/balance/merge sort/BFS) и «Batch mode» (`execute_line`, `--exec`, `--script`).
3. `ROADMAP.md`: v1.0 done, generic layer (void\*+компаратор) — в «next».
4. `make clean && make && make test` с нуля, `-Wall -Wextra` чисто.
5. Тег `v1.0`.

**DoD:** всё собирается с нуля, тесты зелёные, документация соответствует коду.

---

## 11. Next (после v1.0, не входит)

- Generic layer: `void*` ключи + компаратор, `_g`-версии API, int-обёртки поверх.
- `hash_resize` (динамическая capacity).
- Визуализация в DOT/SVG.
- Автотесты на `list_sort` стабильность (нужны ключи с метками — потребует generic layer).

---

# Приложение A — Makefile

```make
# Makefile for algo v1.0
# 80s C-flavoured C++ — build rules.

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I include
BUILD    := build

LIB_SRCS := src/tree.cpp src/list.cpp src/hash.cpp \
            src/tree_algo.cpp src/list_algo.cpp src/hash_algo.cpp
LIB_OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(LIB_SRCS))

TEST_SRCS := $(wildcard tests/*_test.cpp)
TEST_BINS := $(patsubst tests/%.cpp,$(BUILD)/%,$(TEST_SRCS))

REPL_SRC := src/repl.cpp
REPL_BIN := $(BUILD)/repl

.PHONY: all test test-repl repl clean

all: $(REPL_BIN) $(TEST_BINS)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(REPL_BIN): $(REPL_SRC) $(LIB_OBJS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/%_test: tests/%_test.cpp $(LIB_OBJS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

repl: $(REPL_BIN)

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "=== $$t ==="; ./$$t || exit 1; done
	@echo "all tests passed"

test-repl: $(REPL_BIN)
	@sh tests/run_golden.sh

clean:
	rm -rf $(BUILD)
```

Замечания:
- `-I include` уже учтён.
- `test` автоматически подхватывает новые `tests/*_test.cpp`.
- `test-repl` — отдельная цель, но её можно добавить в `test` после этапа 7 (см. ниже).

---

# Приложение B — `include/algo/tree_algo.hpp`

```cpp
#ifndef ALGO_TREE_ALGO_HPP
#define ALGO_TREE_ALGO_HPP

/* tree_algo.hpp - algorithms over the BST. 80s C-flavoured C++. */

#include "algo/tree.hpp"

/* BST invariant check. 1 = valid, 0 = broken. */
int tree_validate(const Tree* t);

/* Min / max key. Returns 1 and writes *out on success,
 * 0 if the tree is empty or out is NULL. */
int tree_min(const Tree* t, int* out);
int tree_max(const Tree* t, int* out);

/* Fill buf (cap >= t->size) with in-order keys.
 * Returns number written (== t->size), or -1 on bad args. */
int tree_to_array_inorder(const Tree* t, int* buf, int cap);

/* Build a perfectly balanced tree from a sorted array.
 * t must be init'd and empty. 0 on OK, -1 on bad args. */
int tree_from_sorted_array(Tree* t, const int* keys, int n);

/* Rebuild t in place, balanced, from its own keys.
 * 0 on OK, -1 on bad args / alloc error. */
int tree_balance(Tree* t);

/* BFS visit, level order. cb(key, user) per node.
 * Returns number of nodes visited. */
typedef void (*TreeVisitFn)(int key, void* user);
int tree_bfs(const Tree* t, TreeVisitFn cb, void* user);

#endif /* ALGO_TREE_ALGO_HPP */
```

---

# Приложение C — `src/tree_algo.cpp`

```cpp
/* ============================================================
 * tree_algo.cpp
 * Algorithms over the BST. 80s C-flavoured C++.
 * ============================================================ */

#include "algo/tree_algo.hpp"

#include <stdio.h>

/* ---------- validate ---------- */

static int validate_node(const TreeNode* n, long long lo, long long hi) {
    if (n == NULL) return 1;
    if ((long long)n->key <= lo || (long long)n->key >= hi) return 0;
    if (!validate_node(n->left,  lo, (long long)n->key)) return 0;
    if (!validate_node(n->right, (long long)n->key, hi)) return 0;
    return 1;
}

int tree_validate(const Tree* t) {
    if (t == NULL) return 0;
    /* Use values just outside int range as open bounds. */
    long long lo = -2147483648LL - 1;
    long long hi =  2147483647LL + 1;
    return validate_node(t->root, lo, hi);
}

/* ---------- min / max ---------- */

int tree_min(const Tree* t, int* out) {
    if (t == NULL || out == NULL || t->root == NULL) return 0;
    const TreeNode* cur = t->root;
    while (cur->left != NULL) cur = cur->left;
    *out = cur->key;
    return 1;
}

int tree_max(const Tree* t, int* out) {
    if (t == NULL || out == NULL || t->root == NULL) return 0;
    const TreeNode* cur = t->root;
    while (cur->right != NULL) cur = cur->right;
    *out = cur->key;
    return 1;
}

/* ---------- to_array_inorder ---------- */

static void fill_inorder(const TreeNode* n, int* buf, int* pos) {
    if (n == NULL) return;
    fill_inorder(n->left, buf, pos);
    buf[(*pos)++] = n->key;
    fill_inorder(n->right, buf, pos);
}

int tree_to_array_inorder(const Tree* t, int* buf, int cap) {
    if (t == NULL || (t->size > 0 && buf == NULL)) return -1;
    if (cap < t->size) return -1;
    int pos = 0;
    fill_inorder(t->root, buf, &pos);
    return pos;
}

/* ---------- from_sorted_array ---------- */

static TreeNode* build_balanced(const int* a, int lo, int hi) {
    if (lo > hi) return NULL;
    int mid = lo + (hi - lo) / 2;
    TreeNode* n = new TreeNode;
    n->key   = a[mid];
    n->left  = build_balanced(a, lo, mid - 1);
    n->right = build_balanced(a, mid + 1, hi);
    return n;
}

int tree_from_sorted_array(Tree* t, const int* keys, int n) {
    if (t == NULL) return -1;
    if (n < 0) return -1;
    if (n > 0 && keys == NULL) return -1;
    if (t->root != NULL || t->size != 0) return -1;  /* must be empty */
    t->root = build_balanced(keys, 0, n - 1);
    t->size = n;
    return 0;
}

/* ---------- balance ---------- */

int tree_balance(Tree* t) {
    if (t == NULL) return -1;
    if (t->size == 0) return 0;

    int* buf = new int[t->size];
    int n = tree_to_array_inorder(t, buf, t->size);
    if (n != t->size) { delete[] buf; return -1; }

    tree_free(t);
    int rc = tree_from_sorted_array(t, buf, n);
    delete[] buf;
    return rc;
}

/* ---------- bfs ---------- */

int tree_bfs(const Tree* t, TreeVisitFn cb, void* user) {
    if (t == NULL || cb == NULL || t->root == NULL) return 0;

    TreeNode** q = new TreeNode*[t->size];
    int head = 0, tail = 0;
    q[tail++] = t->root;
    int visited = 0;

    while (head < tail) {
        TreeNode* n = q[head++];
        cb(n->key, user);
        visited++;
        if (n->left  != NULL) q[tail++] = n->left;
        if (n->right != NULL) q[tail++] = n->right;
    }

    delete[] q;
    return visited;
}
```

---

# Приложение D — `tests/tree_algo_test.cpp`

```cpp
/* ============================================================
 * tree_algo_test.cpp
 * Tests for tree_algo. assert-based, no frameworks.
 * ============================================================ */

#include "algo/tree_algo.hpp"

#include <assert.h>
#include <stdio.h>

/* ---------- validate ---------- */

static void test_validate_empty(void) {
    Tree t; tree_init(&t);
    assert(tree_validate(&t) == 1);
    tree_free(&t);
}

static void test_validate_good(void) {
    Tree t; tree_init(&t);
    int ks[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) tree_insert(&t, ks[i]);
    assert(tree_validate(&t) == 1);
    tree_free(&t);
}

static void test_validate_broken(void) {
    Tree t; tree_init(&t);
    TreeNode* r = new TreeNode; r->key = 5;
    TreeNode* l = new TreeNode; l->key = 9;  /* wrong: > parent */
    r->left = l; r->right = NULL;
    l->left = l->right = NULL;
    t.root = r; t.size = 2;
    assert(tree_validate(&t) == 0);
    tree_free(&t);
}

/* ---------- min / max ---------- */

static void test_min_max_empty(void) {
    Tree t; tree_init(&t);
    int v = 0;
    assert(tree_min(&t, &v) == 0);
    assert(tree_max(&t, &v) == 0);
    tree_free(&t);
}

static void test_min_max(void) {
    Tree t; tree_init(&t);
    int ks[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) tree_insert(&t, ks[i]);
    int v = 0;
    assert(tree_min(&t, &v) == 1 && v == 1);
    assert(tree_max(&t, &v) == 1 && v == 9);
    tree_free(&t);
}

/* ---------- to_array_inorder ---------- */

static void test_to_array_inorder(void) {
    Tree t; tree_init(&t);
    int ks[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) tree_insert(&t, ks[i]);
    int buf[7];
    int n = tree_to_array_inorder(&t, buf, 7);
    assert(n == 7);
    for (int i = 0; i < 7; i++) assert(buf[i] == i + 1);
    tree_free(&t);
}

/* ---------- from_sorted_array ---------- */

static void test_from_sorted_array(void) {
    int ks[] = {1,2,3,4,5,6,7};
    Tree t; tree_init(&t);
    assert(tree_from_sorted_array(&t, ks, 7) == 0);
    assert(t.size == 7);
    assert(tree_validate(&t) == 1);
    /* perfect balance for 7 nodes: height 3 */
    assert(tree_height(&t) == 3);
    tree_free(&t);
}

/* ---------- balance ---------- */

static void test_balance(void) {
    Tree t; tree_init(&t);
    for (int i = 1; i <= 7; i++) tree_insert(&t, i);  /* degenerate */
    assert(tree_height(&t) == 7);
    assert(tree_balance(&t) == 0);
    assert(tree_validate(&t) == 1);
    assert(tree_height(&t) == 3);
    assert(t.size == 7);
    tree_free(&t);
}

/* ---------- bfs ---------- */

typedef struct { int keys[16]; int n; } Acc;

static void acc_cb(int key, void* user) {
    Acc* a = (Acc*)user;
    a->keys[a->n++] = key;
}

static void test_bfs_order(void) {
    Tree t; tree_init(&t);
    int ks[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) tree_insert(&t, ks[i]);
    Acc a; a.n = 0;
    int visited = tree_bfs(&t, acc_cb, &a);
    assert(visited == 7);
    int expect[] = {5,3,8,1,4,7,9};
    for (int i = 0; i < 7; i++) assert(a.keys[i] == expect[i]);
    tree_free(&t);
}

int main(void) {
    printf(">>> test_validate_empty\n");     test_validate_empty();
    printf(">>> test_validate_good\n");      test_validate_good();
    printf(">>> test_validate_broken\n");    test_validate_broken();
    printf(">>> test_min_max_empty\n");      test_min_max_empty();
    printf(">>> test_min_max\n");            test_min_max();
    printf(">>> test_to_array_inorder\n");   test_to_array_inorder();
    printf(">>> test_from_sorted_array\n");  test_from_sorted_array();
    printf(">>> test_balance\n");            test_balance();
    printf(">>> test_bfs_order\n");          test_bfs_order();
    printf("all tree_algo tests passed\n");
    return 0;
}
```

---

# Приложение E — `include/algo/list_algo.hpp`

```cpp
#ifndef ALGO_LIST_ALGO_HPP
#define ALGO_LIST_ALGO_HPP

/* list_algo.hpp - algorithms over the doubly linked list. */

#include "algo/list.hpp"

/* Reverse in place. 0 on OK. */
int list_reverse(List* l);

/* Middle node (two-pointer). NULL on empty list.
 * For even size, returns the lower middle. */
ListNode* list_middle(List* l);

/* Stable merge sort by key, in place. 0 on OK. */
int list_sort(List* l);

#endif /* ALGO_LIST_ALGO_HPP */
```

---

# Приложение F — `src/list_algo.cpp`

```cpp
/* ============================================================
 * list_algo.cpp
 * Algorithms over the DLL. 80s C-flavoured C++.
 * ============================================================ */

#include "algo/list_algo.hpp"

#include <stdio.h>

int list_reverse(List* l) {
    if (l == NULL) return -1;
    ListNode* cur = l->head;
    while (cur != NULL) {
        ListNode* nxt = cur->next;
        cur->next = cur->prev;
        cur->prev = nxt;
        cur = nxt;
    }
    ListNode* tmp = l->head;
    l->head = l->tail;
    l->tail = tmp;
    return 0;
}

ListNode* list_middle(List* l) {
    if (l == NULL || l->head == NULL) return NULL;
    ListNode* slow = l->head;
    ListNode* fast = l->head;
    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;
}

/* ---------- merge sort ---------- */

static ListNode* merge(ListNode* a, ListNode* b) {
    /* Dummy head on stack: we only use its .next. */
    ListNode dummy;
    dummy.key = 0; dummy.prev = NULL; dummy.next = NULL;
    ListNode* tail = &dummy;

    while (a != NULL && b != NULL) {
        if (a->key <= b->key) { tail->next = a; a = a->next; }
        else                  { tail->next = b; b = b->next; }
        tail = tail->next;
    }
    tail->next = (a != NULL) ? a : b;
    return dummy.next;
}

static ListNode* split_half(ListNode* head) {
    /* Returns the head of the second half. */
    ListNode* slow = head;
    ListNode* fast = head->next;
    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }
    ListNode* second = slow->next;
    slow->next = NULL;
    return second;
}

static ListNode* sort_rec(ListNode* head) {
    if (head == NULL || head->next == NULL) return head;
    ListNode* second = split_half(head);
    ListNode* a = sort_rec(head);
    ListNode* b = sort_rec(second);
    return merge(a, b);
}

static void fix_links(List* l) {
    ListNode* prev = NULL;
    ListNode* cur  = l->head;
    int n = 0;
    while (cur != NULL) {
        cur->prev = prev;
        prev = cur;
        cur = cur->next;
        n++;
    }
    l->tail = prev;
    l->size = n;
}

int list_sort(List* l) {
    if (l == NULL) return -1;
    if (l->head == NULL || l->head->next == NULL) return 0;
    l->head = sort_rec(l->head);
    fix_links(l);
    return 0;
}
```

---

# Приложение G — `tests/list_algo_test.cpp`

```cpp
/* ============================================================
 * list_algo_test.cpp
 * Tests for list_algo. assert-based, no frameworks.
 * ============================================================ */

#include "algo/list_algo.hpp"

#include <assert.h>
#include <stdio.h>

/* Build 1..n, return list. */
static void build(List* l, int n) {
    list_init(l);
    for (int i = 1; i <= n; i++) list_push_back(l, i);
}

/* Walk forward, assert keys and prev-chain consistency. */
static void check_forward(List* l, const int* expect, int n) {
    assert(l->size == n);
    ListNode* cur = l->head;
    ListNode* prev = NULL;
    for (int i = 0; i < n; i++) {
        assert(cur != NULL);
        assert(cur->key == expect[i]);
        assert(cur->prev == prev);
        prev = cur;
        cur = cur->next;
    }
    assert(cur == NULL);
    assert(l->tail == prev);
}

static void test_reverse_empty(void) {
    List l; list_init(&l);
    assert(list_reverse(&l) == 0);
    assert(l.head == NULL && l.tail == NULL && l.size == 0);
    list_free(&l);
}

static void test_reverse_one(void) {
    List l; build(&l, 1);
    assert(list_reverse(&l) == 0);
    int e[] = {1}; check_forward(&l, e, 1);
    list_free(&l);
}

static void test_reverse_even(void) {
    List l; build(&l, 4);
    assert(list_reverse(&l) == 0);
    int e[] = {4,3,2,1}; check_forward(&l, e, 4);
    list_free(&l);
}

static void test_reverse_odd(void) {
    List l; build(&l, 5);
    assert(list_reverse(&l) == 0);
    int e[] = {5,4,3,2,1}; check_forward(&l, e, 5);
    list_free(&l);
}

static void test_middle_empty(void) {
    List l; list_init(&l);
    assert(list_middle(&l) == NULL);
    list_free(&l);
}

static void test_middle_one(void) {
    List l; build(&l, 1);
    assert(list_middle(&l)->key == 1);
    list_free(&l);
}

static void test_middle_odd(void) {
    List l; build(&l, 5);
    assert(list_middle(&l)->key == 3);  /* lower middle for 1..5 */
    list_free(&l);
}

static void test_middle_even(void) {
    List l; build(&l, 4);
    assert(list_middle(&l)->key == 2);  /* lower middle for 1..4 */
    list_free(&l);
}

static void test_sort_empty(void) {
    List l; list_init(&l);
    assert(list_sort(&l) == 0);
    assert(l.head == NULL && l.size == 0);
    list_free(&l);
}

static void test_sort_one(void) {
    List l; build(&l, 1);
    assert(list_sort(&l) == 0);
    int e[] = {1}; check_forward(&l, e, 1);
    list_free(&l);
}

static void test_sort_reverse_input(void) {
    List l; list_init(&l);
    for (int i = 10; i >= 1; i--) list_push_back(&l, i);
    assert(list_sort(&l) == 0);
    int e[10]; for (int i = 0; i < 10; i++) e[i] = i + 1;
    check_forward(&l, e, 10);
    list_free(&l);
}

static void test_sort_sorted_input(void) {
    List l; build(&l, 6);
    assert(list_sort(&l) == 0);
    int e[] = {1,2,3,4,5,6}; check_forward(&l, e, 6);
    list_free(&l);
}

static void test_sort_duplicates(void) {
    List l; list_init(&l);
    int in[] = {3,1,3,2,1,3};
    for (int i = 0; i < 6; i++) list_push_back(&l, in[i]);
    assert(list_sort(&l) == 0);
    int e[] = {1,1,2,3,3,3}; check_forward(&l, e, 6);
    list_free(&l);
}

int main(void) {
    printf(">>> test_reverse_empty\n");      test_reverse_empty();
    printf(">>> test_reverse_one\n");        test_reverse_one();
    printf(">>> test_reverse_even\n");       test_reverse_even();
    printf(">>> test_reverse_odd\n");        test_reverse_odd();
    printf(">>> test_middle_empty\n");       test_middle_empty();
    printf(">>> test_middle_one\n");         test_middle_one();
    printf(">>> test_middle_odd\n");         test_middle_odd();
    printf(">>> test_middle_even\n");        test_middle_even();
    printf(">>> test_sort_empty\n");         test_sort_empty();
    printf(">>> test_sort_one\n");           test_sort_one();
    printf(">>> test_sort_reverse_input\n"); test_sort_reverse_input();
    printf(">>> test_sort_sorted_input\n");  test_sort_sorted_input();
    printf(">>> test_sort_duplicates\n");    test_sort_duplicates();
    printf("all list_algo tests passed\n");
    return 0;
}
```

---

# Приложение H — `include/algo/hash_algo.hpp`

```cpp
#ifndef ALGO_HASH_ALGO_HPP
#define ALGO_HASH_ALGO_HPP

/* hash_algo.hpp - introspection over the chained hash table. */

#include "algo/hash.hpp"

struct HashStats {
    int capacity;
    int size;
    int min_chain;     /* 0 if table is empty */
    int max_chain;
    int empty_buckets;
    double load_factor; /* size / capacity */
};

/* Fill *out. 0 on OK, -1 on bad args. */
int hash_stats(const Hash* h, HashStats* out);

#endif /* ALGO_HASH_ALGO_HPP */
```

---

# Приложение I — `src/hash_algo.cpp`

```cpp
/* ============================================================
 * hash_algo.cpp
 * Introspection over the chained hash. 80s C-flavoured C++.
 * ============================================================ */

#include "algo/hash_algo.hpp"

int hash_stats(const Hash* h, HashStats* out) {
    if (h == NULL || out == NULL) return -1;
    if (h->capacity <= 0) return -1;

    int min_chain = -1;
    int max_chain = 0;
    int empty = 0;

    for (int i = 0; i < h->capacity; i++) {
        int len = 0;
        for (HashNode* cur = h->buckets[i]; cur != NULL; cur = cur->next) len++;
        if (len == 0) empty++;
        else if (min_chain < 0 || len < min_chain) min_chain = len;
        if (len > max_chain) max_chain = len;
    }

    out->capacity      = h->capacity;
    out->size          = h->size;
    out->min_chain     = (min_chain < 0) ? 0 : min_chain;
    out->max_chain     = max_chain;
    out->empty_buckets = empty;
    out->load_factor   = (double)h->size / (double)h->capacity;
    return 0;
}
```

---

# Приложение J — `tests/hash_algo_test.cpp`

```cpp
/* ============================================================
 * hash_algo_test.cpp
 * Tests for hash_algo. assert-based, no frameworks.
 * ============================================================ */

#include "algo/hash_algo.hpp"

#include <assert.h>
#include <stdio.h>

static void test_empty(void) {
    Hash h; assert(hash_init(&h, 8) == 0);
    HashStats s;
    assert(hash_stats(&h, &s) == 0);
    assert(s.capacity == 8);
    assert(s.size == 0);
    assert(s.min_chain == 0);
    assert(s.max_chain == 0);
    assert(s.empty_buckets == 8);
    assert(s.load_factor == 0.0);
    hash_free(&h);
}

static void test_all_in_one_bucket(void) {
    Hash h; assert(hash_init(&h, 1) == 0);
    hash_insert(&h, 10); hash_insert(&h, 20); hash_insert(&h, 30);
    HashStats s;
    assert(hash_stats(&h, &s) == 0);
    assert(s.size == 3);
    assert(s.max_chain == 3);
    assert(s.min_chain == 3);
    assert(s.empty_buckets == 0);
    hash_free(&h);
}

static void test_spread(void) {
    Hash h; assert(hash_init(&h, 8) == 0);
    for (int i = 0; i < 8; i++) hash_insert(&h, i);
    HashStats s;
    assert(hash_stats(&h, &s) == 0);
    assert(s.size == 8);
    assert(s.min_chain >= 1);
    assert(s.max_chain <= 8);
    assert(s.empty_buckets + (s.size > 0 ? 1 : 0) <= 8);
    hash_free(&h);
}

int main(void) {
    printf(">>> test_empty\n");           test_empty();
    printf(">>> test_all_in_one_bucket\n"); test_all_in_one_bucket();
    printf(">>> test_spread\n");          test_spread();
    printf("all hash_algo tests passed\n");
    return 0;
}
```

---

# Приложение K — REPL: изменения (фрагменты)

Это фрагменты для `src/repl.cpp`. Вставлять в соответствующие секции.

### K.1. Новые include (вверху)

```cpp
#include "algo/hash_algo.hpp"
#include "algo/list_algo.hpp"
#include "algo/tree_algo.hpp"
```

### K.2. Новые команды (после существующих `cmd_*`)

```cpp
static void cmd_validate(void) {
    if (!need(KIND_TREE, "validate")) return;
    printf(tree_validate(&g_tree) ? "valid\n" : "BROKEN\n");
}

static void cmd_min(void) {
    if (!need(KIND_TREE, "min")) return;
    int v;
    if (tree_min(&g_tree, &v)) printf("%d\n", v);
    else                       printf("(empty)\n");
}

static void cmd_max(void) {
    if (!need(KIND_TREE, "max")) return;
    int v;
    if (tree_max(&g_tree, &v)) printf("%d\n", v);
    else                       printf("(empty)\n");
}

static void cmd_balance(void) {
    if (!need(KIND_TREE, "balance")) return;
    if (tree_balance(&g_tree) == 0) printf("ok\n");
    else                            printf("error: balance failed\n");
}

static void cmd_reverse(void) {
    if (!need(KIND_LIST, "reverse")) return;
    list_reverse(&g_list);
    printf("ok\n");
}

static void cmd_sort(void) {
    if (!need(KIND_LIST, "sort")) return;
    list_sort(&g_list);
    printf("ok\n");
}

static void cmd_stats(void) {
    if (!need(KIND_HASH, "stats")) return;
    HashStats s;
    if (hash_stats(&g_hash, &s) != 0) { printf("error: stats failed\n"); return; }
    printf("capacity=%d size=%d\n", s.capacity, s.size);
    printf("min_chain=%d max_chain=%d empty_buckets=%d\n",
           s.min_chain, s.max_chain, s.empty_buckets);
    printf("load_factor=%.2f\n", s.load_factor);
}
```

### K.3. Обновить `cmd_help` (добавить строки)

```cpp
"  validate                 tree: BST invariant check\n"
"  min | max                tree: min / max key\n"
"  balance                  tree: rebuild balanced\n"
"  reverse                  list: reverse in place\n"
"  sort                     list: stable merge sort\n"
"  stats                    hash: chain statistics\n"
```

### K.4. Рефакторинг `main` → `execute_line`

```cpp
/* Returns 0 on OK, 1 on command error, 2 on quit. */
static int execute_line(char* line) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }

    char tok[64];
    char* rest = NULL;
    char* p = line;
    if (!next_token(&p, tok, sizeof tok, &rest)) return 0;

    if (strcmp(tok, "quit") == 0 || strcmp(tok, "exit") == 0) return 2;
    if (strcmp(tok, "help") == 0)    { cmd_help(); return 0; }
    if (strcmp(tok, "tree") == 0)    { cmd_tree(); return 0; }
    if (strcmp(tok, "list") == 0)    { cmd_list(); return 0; }
    if (strcmp(tok, "hash") == 0) {
        int cap = 16;
        if (*rest != '\0' && !parse_whole_int(rest, &cap)) {
            printf("error: bad capacity\n"); return 1;
        }
        cmd_hash(cap); return 0;
    }
    if (strcmp(tok, "insert") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: insert needs an integer\n"); return 1; }
        cmd_insert(k); return 0;
    }
    if (strcmp(tok, "find") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: find needs an integer\n"); return 1; }
        cmd_find(k); return 0;
    }
    if (strcmp(tok, "remove") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: remove needs an integer\n"); return 1; }
        cmd_remove(k); return 0;
    }
    if (strcmp(tok, "push_back") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: push_back needs an integer\n"); return 1; }
        cmd_push_back(k); return 0;
    }
    if (strcmp(tok, "push_front") == 0) {
        int k;
        if (!parse_whole_int(rest, &k)) { printf("error: push_front needs an integer\n"); return 1; }
        cmd_push_front(k); return 0;
    }
    if (strcmp(tok, "print") == 0) { cmd_print(); return 0; }
    if (strcmp(tok, "draw") == 0 ||
        strcmp(tok, "schema") == 0 ||
        strcmp(tok, "ascii") == 0) { cmd_draw(); return 0; }
    if (strcmp(tok, "size") == 0)   { cmd_size(); return 0; }
    if (strcmp(tok, "height") == 0) { cmd_height(); return 0; }
    if (strcmp(tok, "save") == 0) {
        if (*rest == '\0') { printf("error: save needs a file path\n"); return 1; }
        return save_current(rest) == 0 ? 0 : 1;
    }
    if (strcmp(tok, "load") == 0) {
        if (*rest == '\0') { printf("error: load needs a file path\n"); return 1; }
        return load_from(rest) == 0 ? 0 : 1;
    }
    if (strcmp(tok, "free") == 0) { cmd_free(); return 0; }
    if (strcmp(tok, "validate") == 0) { cmd_validate(); return 0; }
    if (strcmp(tok, "min") == 0)      { cmd_min(); return 0; }
    if (strcmp(tok, "max") == 0)      { cmd_max(); return 0; }
    if (strcmp(tok, "balance") == 0)  { cmd_balance(); return 0; }
    if (strcmp(tok, "reverse") == 0)  { cmd_reverse(); return 0; }
    if (strcmp(tok, "sort") == 0)     { cmd_sort(); return 0; }
    if (strcmp(tok, "stats") == 0)    { cmd_stats(); return 0; }

    printf("error: unknown command '%s' (try 'help')\n", tok);
    return 1;
}
```

### K.5. Batch-режимы и новый `main`

```cpp
/* Split a string by ';' and execute each piece. */
static int run_exec_string(char* s, int keep_going) {
    int rc = 0;
    char* start = s;
    for (;;) {
        char* semi = strchr(start, ';');
        if (semi != NULL) *semi = '\0';
        int r = execute_line(start);
        if (r == 2) return 0;          /* quit */
        if (r != 0) { rc = 1; if (!keep_going) return rc; }
        if (semi == NULL) break;
        start = semi + 1;
    }
    return rc;
}

static int run_script(const char* path, int keep_going) {
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        printf("error: cannot open '%s': %s\n", path, strerror(errno));
        return 1;
    }
    char line[256];
    int rc = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        int r = execute_line(line);
        if (r == 2) break;
        if (r != 0) { rc = 1; if (!keep_going) break; }
    }
    fclose(f);
    return rc;
}

static void usage(const char* prog) {
    printf("usage: %s [--exec \"cmd; cmd; ...\"] [--script FILE] [--keep-going]\n", prog);
    printf("       (no args: interactive mode)\n");
}

int main(int argc, char** argv) {
    const char* exec_str = NULL;
    const char* script   = NULL;
    int keep_going = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--exec") == 0 && i + 1 < argc) {
            exec_str = argv[++i];
        } else if (strcmp(argv[i], "--script") == 0 && i + 1 < argc) {
            script = argv[++i];
        } else if (strcmp(argv[i], "--keep-going") == 0) {
            keep_going = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            usage(argv[0]); return 0;
        } else {
            usage(argv[0]); return 1;
        }
    }

    if (exec_str != NULL) {
        char buf[1024];
        snprintf(buf, sizeof buf, "%s", exec_str);
        return run_exec_string(buf, keep_going);
    }
    if (script != NULL) {
        return run_script(script, keep_going);
    }

    /* interactive */
    char line[256];
    printf("algo repl. type 'help' for commands, 'quit' to exit.\n");
    for (;;) {
        printf("> ");
        fflush(stdout);
        if (fgets(line, sizeof line, stdin) == NULL) break;
        int r = execute_line(line);
        if (r == 2) break;
    }
    teardown();
    printf("bye\n");
    return 0;
}
```

Замечание: `execute_line` модифицирует строку (обрезает `\n`, `next_token` двигает указатели, но не пишет). `run_exec_string` копирует `--exec` в буфер, чтобы не портить `argv`.

---

# Приложение L — `tests/run_golden.sh`

```sh
#!/bin/sh
# Golden-output tests for the REPL.
# Each caseNN.in is fed via --script; stdout is compared to caseNN.out.
# Normalization: strip prompts, replace file paths with <FILE>.

set -e
REPL=./build/repl
DIR=tests/repl_golden
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

normalize() {
    sed -e 's/^> //' \
        -e 's|/tmp/[^ ]*|<FILE>|g' \
        -e 's|tests/repl_golden/[^ ]*\.repl|<FILE>|g'
}

fail=0
for in_file in "$DIR"/*.in; do
    base=$(basename "$in_file" .in)
    out_file="$DIR/$base.out"
    got="$TMP/$base.got"
    "$REPL" --script "$in_file" > "$got" 2>&1 || true
    if ! diff -u <(normalize < "$out_file") <(normalize < "$got") > "$TMP/$base.diff"; then
        echo "FAIL: $base"
        cat "$TMP/$base.diff"
        fail=1
    else
        echo "ok:   $base"
    fi
done

if [ "$fail" -ne 0 ]; then
    echo "golden tests FAILED"
    exit 1
fi
echo "all golden tests passed"
```

Замечание: `<(...)` — process substitution, требует bash. Заменить `#!/bin/sh` на `#!/usr/bin/env bash` (или на `#!/bin/bash`). Makefile вызывает `sh tests/run_golden.sh` — поправить на `bash tests/run_golden.sh`.

---

# Приложение M — Golden-кейсы (фрагменты)

### `tests/repl_golden/case01.in`
```
tree
insert 5
insert 3
insert 8
insert 1
insert 4
insert 7
insert 9
draw
validate
min
max
height
size
```

### `tests/repl_golden/case01.out`
```
tree created
ok
ok
ok
ok
ok
ok
ok
5
|-- 3
|   |-- 1
|   `-- 4
`-- 8
    |-- 7
    `-- 9
size=7 height=3
valid
1
9
3
7
```

### `tests/repl_golden/case02.in`
```
tree
insert 1
insert 2
insert 3
insert 4
insert 5
insert 6
insert 7
draw
balance
draw
```

### `tests/repl_golden/case02.out`
```
tree created
ok
ok
ok
ok
ok
ok
ok
1
`-- 2
    `-- 3
        `-- 4
            `-- 5
                `-- 6
                    `-- 7
size=7 height=7
ok
4
|-- 2
|   |-- 1
|   `-- 3
`-- 6
    |-- 5
    `-- 7
size=7 height=3
```

### `tests/repl_golden/case03.in`
```
list
push_back 1
push_back 2
push_back 3
draw
reverse
draw
push_front 0
draw
sort
draw
```

### `tests/repl_golden/case03.out`
```
list created
ok
ok
ok
head [1] <-> [2] <-> [3] tail
size=3
ok
head [3] <-> [2] <-> [1] tail
size=3
ok
head [0] <-> [3] <-> [2] <-> [1] tail
size=4
ok
head [0] <-> [1] <-> [2] <-> [3] tail
size=4
```

### `tests/repl_golden/case04.in`
```
hash 8
insert 10
insert 20
insert 30
stats
draw
remove 20
stats
```

Ожидаемый `.out` зависит от FNV-1a раскладки — **сгенерировать один раз** прогоном `./build/repl --script case04.in > case04.out` после реализации и вручную проверить, что числа корректны. Это нормально для golden-тестов: эталон создаётся из проверенного вручную вывода.

### `tests/repl_golden/case08.in`
```
insert abc
height
hash 0
load nosuchfile.repl
```

### `tests/repl_golden/case08.out`
```
error: insert needs an integer
error: no structure
error: height requires a tree (current: none)
error: bad capacity 0
warning: 'nosuchfile.repl' does not end with .repl
error: cannot open 'nosuchfile.repl': No such file or directory
```

(Точные строки сверить с реальным выводом REPL — `strerror` может отличаться по локали; при необходимости нормализовать в `run_golden.sh`.)

### Остальные кейсы
- `case05` — save/load tree (путь нормализуется);
- `case06` — save/load list;
- `case07` — save/load hash;
- `case09` — дубликаты: `tree; insert 5; insert 5; insert 5` → два `warning: duplicate...` или `error: insert 5 failed` (в зависимости от того, что печатает REPL — сверить);
- `case10` — `free` + повторное создание.

---

# Приложение N — итоговый `make test`

После этапа 7 в Makefile:

```make
test: $(TEST_BINS) test-repl
	@for t in $(TEST_BINS); do echo "=== $$t ==="; ./$$t || exit 1; done
	@echo "all unit tests passed"
```

И `run_golden.sh` вызывать через `bash`:

```make
test-repl: $(REPL_BIN)
	@bash tests/run_golden.sh
```

---

# Итоговое состояние v1.0

```
include/algo/
    tree.hpp          (без изменений)
    list.hpp          (без изменений)
    hash.hpp          (без изменений)
    tree_algo.hpp     (новый)
    list_algo.hpp     (новый)
    hash_algo.hpp     (новый)
src/
    tree.cpp          (без изменений)
    list.cpp          (без изменений)
    hash.cpp          (без изменений)
    tree_algo.cpp     (новый)
    list_algo.cpp     (новый)
    hash_algo.cpp     (новый)
    repl.cpp          (execute_line + batch + новые команды)
tests/
    tree_test.cpp     (без изменений)
    list_test.cpp     (без изменений)
    hash_test.cpp     (без изменений)
    tree_algo_test.cpp (новый)
    list_algo_test.cpp (новый)
    hash_algo_test.cpp (новый)
    run_golden.sh      (новый)
    repl_golden/
        case01.in/.out
        ... (10 кейсов)
Makefile              (новые цели: repl, test-repl, авто-сборка тестов)
ROADMAP.md            (этот файл)
CONTRIBUTING.md       (опционально: конвенции из раздела 0)
README.md             (обновить: API, REPL, Tests, v1.0)
ALGO_EXPLAINED.md     (обновить: Algo layer, Batch mode)
```

Проверка готовности:
```sh
make clean
make
make test          # unit + golden, всё зелёное
./build/repl --exec "tree; insert 5; balance; draw"
./build/repl       # интерактив по-прежнему работает
```
