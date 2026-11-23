**ALGO_EXPLAINED.md**
Полный разбор библиотеки algo: от C++-минимума до REPL, слоя алгоритмов
и batch-режимов.
Стиль: 80s C-flavoured C++.
Это не исходник, а объяснение по README.md и .cpp/.hpp.

# algo - как устроена система

**Статус:** v1.0.2 - experiment, not a product. Private repo:
https://github.com/AlbertFisher666/algo

Проект `algo` - маленькая библиотека алгоритмов и структур данных:
BST, двусвязный список и хеш-таблица с цепочками. Сверху - слой
алгоритмов над каждой структурой (`*_algo`) и интерактивный REPL
с ASCII-диаграммами, сохранением/загрузкой в текстовый `.repl`
и batch-режимами (`--exec`, `--script`).

Главная особенность: это C++ только по факту компиляции. Внутри -
стиль «80s C-flavoured C++»: `struct`, свободные функции, `new/delete`,
`NULL`, `printf`, `FILE*`, фиксированные буферы. Нет `iostream`,
`std::string`, STL-контейнеров, классов с конструкторами/деструкторами,
шаблонов, исключений и RAII.

---

## 1. C++-минимум, без которого не понять проект

### 1.1. Единицы трансляции, заголовки и исходники

Каждый `.cpp` - отдельная **единица трансляции**. Компилятор обрабатывает
её независимо. Заголовки `.hpp` - это «контракты»: объявления структур и
функций. Исходники `.cpp` - определения функций.

Схема:

```text
include/algo/tree.hpp   -> объявляет struct TreeNode, struct Tree, API
src/tree.cpp            -> включает tree.hpp и реализует API
tests/tree_test.cpp     -> включает tree.hpp, вызывает API, имеет main()
src/repl.cpp            -> включает все заголовки и использует API
```

`#include "algo/tree.hpp"` ищется относительно `-I include`. Поэтому
заголовки лежат в `include/algo/`, а не рядом с `.cpp`.

В каждом заголовке есть **include guard**:

```cpp
#ifndef ALGO_TREE_HPP
#define ALGO_TREE_HPP
...
#endif
```

Это защита от повторного включения одного и того же заголовка в одну
единицу трансляции. Без неё структуры и прототипы могли бы определиться
дважды.

### 1.2. `struct` - это просто набор полей

В проекте нет классов. `struct TreeNode` - это описание layout:

```cpp
struct TreeNode {
    int key;
    TreeNode* left;
    TreeNode* right;
};
```

`TreeNode` хранит ключ и два указателя на детей. `Tree` хранит указатель
на корень и размер:

```cpp
struct Tree {
    TreeNode* root;
    int size;
};
```

Аналогично `ListNode/List` и `HashNode/Hash`.

Важно: у этих структур **нет конструкторов**. `Tree t;` создаёт
неинициализированную переменную. Пока не вызван `tree_init(&t)`, поля
`root` и `size` содержат мусор. Поэтому API всегда начинается с init.

### 1.3. Указатели, стек и куча

Указатель хранит адрес. `TreeNode*` - адрес узла в памяти. `NULL` -
нулевой указатель, «никуда не указывает».

Локальные переменные вроде `Tree t;` в тестах и `Tree g_tree;` в REPL
живут в статической/стековой памяти. А узлы создаются через `new`:

```cpp
TreeNode* n = new TreeNode;
```

`new` выделяет память в куче и возвращает указатель. Освобождает память
только `delete`:

```cpp
delete n;
```

Это не `malloc/free`. Проект использует C++-механизм `new/delete`, но
без умных указателей. Вся ответственность за освобождение - на коде.

### 1.4. `const`

`const Tree*` означает «указатель на Tree, который эта функция не должна
менять». Например:

```cpp
int tree_find(const Tree* t, int key);
int tree_height(const Tree* t);
void tree_print(const Tree* t);
```

Это не защита от всех ошибок, но контракт: find/height/print не мутируют
дерево. `hash_find` тоже принимает `const Hash*`. Функции `*_algo`,
которые только читают структуру (`tree_validate`, `tree_min/max`,
`tree_to_array_inorder`, `tree_bfs`, `hash_stats`), тоже принимают
`const*`.

### 1.5. Рекурсия и итерация

В проекте есть и то, и другое:

- `tree_insert` - итеративный, через `TreeNode**`.
- `tree_remove` - итеративный поиск + итеративный `tree_detach_min`.
- `tree_free`, `tree_height`, `tree_print` - рекурсивные.
- `list_free`, `list_find`, `list_remove` - итеративные.
- `hash_*` - итеративные.
- `tree_algo`: `validate`, `to_array_inorder`, `build_balanced` -
  рекурсивные; `min/max`, `bfs` - итеративные.
- `list_algo`: `reverse`, `middle` - итеративные; `sort` - рекурсивный
  merge sort.
- `hash_algo`: `hash_stats` - итеративный, один проход по бакетам.

Рекурсия удобна для дерева, но на очень глубоком дереве может переполнить
стек. README отдельно подчёркивает: `tree_insert` итеративный, поэтому
на глубоких деревьях не падает по стеку. А вот `tree_free` рекурсивный.

### 1.6. `static`, `enum`, глобальное состояние

В `repl.cpp` есть глобальное состояние:

```cpp
enum Kind { KIND_NONE, KIND_TREE, KIND_LIST, KIND_HASH };

static Kind g_kind = KIND_NONE;
static Tree g_tree;
static List g_list;
static Hash g_hash;
```

`static` на файловом уровне даёт внутреннюю линковку: эти переменные
видны только внутри `repl.cpp`. `enum Kind` - тег, какая структура сейчас
жива. Одновременно активна ровно одна.

`teardown()` - главная точка уничтожения живой структуры:

```cpp
static void teardown(void) {
    if (g_kind == KIND_TREE) tree_free(&g_tree);
    if (g_kind == KIND_LIST) list_free(&g_list);
    if (g_kind == KIND_HASH) hash_free(&g_hash);
    g_kind = KIND_NONE;
}
```

Команды `tree`, `list`, `hash`, `free`, загрузка `load` и выход из REPL
так или иначе приводят к `teardown()`.

### 1.7. C-ввод/вывод и разбор строк

REPL не использует `std::cin`/`std::cout`. Используются:

- `printf` - вывод;
- `fgets` - чтение строки в фиксированный буфер;
- `FILE*`, `fopen`, `fclose`, `fprintf` - save/load;
- `strcmp`, `strncmp`, `strlen`, `memmove` - строки;
- `strtol` - разбор целых;
- `errno`, `strerror` - ошибки.

Буферы фиксированные: `char line[256]`, `char tok[64]`. Это стиль проекта,
но у него есть цена: очень длинная строка будет прочитана частями, а
размер токена ограничен.

---

## 2. Карта проекта

```text
include/algo/tree.hpp            BST - контракт
include/algo/list.hpp            DLL - контракт
include/algo/hash.hpp            hash - контракт
include/algo/tree_algo.hpp       BST algorithms - контракт
include/algo/list_algo.hpp       DLL algorithms - контракт
include/algo/hash_algo.hpp       hash introspection - контракт
src/tree.cpp                     BST - реализация
src/list.cpp                     DLL - реализация
src/hash.cpp                     hash - реализация
src/tree_algo.cpp                BST algorithms - реализация
src/list_algo.cpp                DLL algorithms - реализация
src/hash_algo.cpp                hash introspection - реализация
src/repl.cpp                     интерактивный REPL + batch
tests/tree_test.cpp              тесты BST
tests/list_test.cpp              тесты DLL
tests/hash_test.cpp              тесты hash
tests/tree_algo_test.cpp         тесты tree_algo
tests/tree_algo_bounds_test.cpp  границы int для tree_algo
tests/list_algo_test.cpp         тесты list_algo
tests/hash_algo_test.cpp         тесты hash_algo
tests/run_golden.sh              REPL golden - runner
tests/repl_golden/               REPL golden - 10 кейсов (.in/.out)
Makefile                         правила сборки
```

Поток данных:

```text
tree_test.cpp / list_test.cpp / hash_test.cpp
tree_algo_test.cpp / list_algo_test.cpp / hash_algo_test.cpp
        |
        v
   заголовки API
        |
        v
   src/*.cpp  --->  new/delete, указатели, printf
        ^
        |
src/repl.cpp ---> execute_line ---> cmd_* ---> API
        ^
        |
tests/run_golden.sh ---> repl --script ---> tests/repl_golden/*.in
```

Тесты, golden и REPL - три разные «верхушки» над одной библиотекой.

---

## 3. Сборка и запуск

Требования из README:

- `g++` с C++17;
- `make`;
- `bash` (для golden).

Сборка тестов (unit + golden):

```sh
make test
```

Ожидаемый вывод заканчивается на:

```text
all unit tests passed
all golden tests passed
```

Только golden:

```sh
make test-repl
```

Сборка REPL:

```sh
make repl
```

Или напрямую:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -I include \
    src/repl.cpp src/tree.cpp src/list.cpp src/hash.cpp \
    src/tree_algo.cpp src/list_algo.cpp src/hash_algo.cpp \
    -o build/repl
```

Запуск:

```sh
./build/repl
```

Флаги:

- `-std=c++17` - стандарт;
- `-O2` - оптимизация;
- `-Wall -Wextra` - предупреждения;
- `-I include` - путь к заголовкам;
- несколько `.cpp` - потому что это отдельные единицы трансляции, их
  надо скомпоновать линковщиком в один исполняемый файл.

CI (`.github/workflows/ci.yml`) гоняет `make` и `make test` на Ubuntu
при каждом push и pull request.

---

## 4. Контракты заголовков

### 4.1. `tree.hpp`

```cpp
struct TreeNode { int key; TreeNode* left; TreeNode* right; };
struct Tree     { TreeNode* root; int size; };

void tree_init(Tree* t);
void tree_free(Tree* t);
int  tree_insert(Tree* t, int key);
int  tree_find(const Tree* t, int key);
int  tree_remove(Tree* t, int key);
int  tree_height(const Tree* t);
void tree_print(const Tree* t);
```

Смысл кодов:

- `tree_insert`: `0` - вставлено, `-1` - дубликат/ошибка.
- `tree_find`: `1` - есть, `0` - нет.
- `tree_remove`: `0` - удалено, `-1` - не найдено.
- `tree_height`: `0` для пустого дерева.
- `tree_print`: in-order, то есть по возрастанию.

### 4.2. `list.hpp`

```cpp
struct ListNode { int key; ListNode* prev; ListNode* next; };
struct List     { ListNode* head; ListNode* tail; int size; };

void list_init(List* l);
void list_free(List* l);
int  list_push_back(List* l, int key);
int  list_push_front(List* l, int key);
ListNode* list_find(List* l, int key);
int  list_remove(List* l, int key);
void list_print(const List* l);
```

`list_find` возвращает указатель на первый узел с ключом или `NULL`.
`list_remove` удаляет первый такой узел.

### 4.3. `hash.hpp`

```cpp
struct HashNode { int key; HashNode* next; };
struct Hash     { HashNode** buckets; int capacity; int size; };

int  hash_init(Hash* h, int capacity);
void hash_free(Hash* h);
int  hash_insert(Hash* h, int key);
int  hash_find(const Hash* h, int key);
int  hash_remove(Hash* h, int key);
void hash_print(const Hash* h);
```

`buckets` - массив указателей на головы цепочек. `capacity` фиксируется
при `hash_init`. `size` - число ключей. Ресайза нет.

### 4.4. `tree_algo.hpp`

```cpp
int tree_validate(const Tree* t);
int tree_min(const Tree* t, int* out);
int tree_max(const Tree* t, int* out);
int tree_to_array_inorder(const Tree* t, int* buf, int cap);
int tree_from_sorted_array(Tree* t, const int* keys, int n);
int tree_balance(Tree* t);

typedef void (*TreeVisitFn)(int key, void* user);
int tree_bfs(const Tree* t, TreeVisitFn cb, void* user);
```

Коды:

- `tree_validate`: `1` - BST-инвариант выполнен, `0` - нарушен или `t == NULL`.
- `tree_min/max`: `1` - записано в `*out`, `0` - пусто или `out == NULL`.
- `tree_to_array_inorder`: число записанных ключей (`== t->size`) или `-1`.
- `tree_from_sorted_array`: `0` - OK, `-1` - ошибка. Требует пустое дерево.
- `tree_balance`: `0` - OK, `-1` - ошибка.
- `tree_bfs`: число посещённых узлов.

### 4.5. `list_algo.hpp`

```cpp
int list_reverse(List* l);
ListNode* list_middle(List* l);
int list_sort(List* l);
```

Коды:

- `list_reverse`: `0` - OK, `-1` - `l == NULL`.
- `list_middle`: узел или `NULL` для пустого списка; для чётного размера
  возвращает **нижнюю** середину (`1..4` → `2`).
- `list_sort`: `0` - OK, `-1` - `l == NULL`. Stable merge sort.

### 4.6. `hash_algo.hpp`

```cpp
struct HashStats {
    int capacity;
    int size;
    int min_chain;     /* 0 if table is empty */
    int max_chain;
    int empty_buckets;
    double load_factor; /* size / capacity */
};

int hash_stats(const Hash* h, HashStats* out);
```

`hash_stats`: `0` - OK, `-1` - `h == NULL`, `out == NULL` или
`h->capacity <= 0`. `min_chain` считается только по непустым бакетам
и равен `0` для пустой таблицы.

---

## 5. BST: `tree.cpp`

### 5.1. Инициализация и освобождение

```cpp
void tree_init(Tree* t) {
    t->root = NULL;
    t->size = 0;
}
```

Пустое дерево: корень `NULL`, размер `0`.

Освобождение рекурсивное, post-order:

```cpp
static void tree_free_node(TreeNode* n) {
    if (n == NULL) return;
    tree_free_node(n->left);
    tree_free_node(n->right);
    delete n;
}
```

Сначала дети, потом сам узел. Это важно: нельзя удалить узел раньше, чем
сохранены/обработаны указатели на детей. После `tree_free` поля
сбрасываются.

### 5.2. Поиск

```cpp
int tree_find(const Tree* t, int key) {
    const TreeNode* cur = t->root;
    while (cur != NULL) {
        if (key == cur->key) return 1;
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return 0;
}
```

Итеративный спуск по BST-свойству: меньше - налево, больше - направо.
`const` подчёркивает, что поиск не меняет дерево.

### 5.3. Высота

```cpp
static int tree_height_node(const TreeNode* n) {
    if (n == NULL) return 0;
    int l = tree_height_node(n->left);
    int r = tree_height_node(n->right);
    return 1 + (l > r ? l : r);
}
```

Высота пустого дерева - `0`, листа - `1`. Рекурсивно берётся максимум
из высот поддеревьев плюс единица.

### 5.4. Вставка через `TreeNode**`

Ключевой приём:

```cpp
int tree_insert(Tree* t, int key) {
    TreeNode** slot = &t->root;
    while (*slot != NULL) {
        if (key == (*slot)->key) return -1;
        slot = (key < (*slot)->key) ? &(*slot)->left : &(*slot)->right;
    }
    *slot = new TreeNode;
    (*slot)->key   = key;
    (*slot)->left  = NULL;
    (*slot)->right = NULL;
    t->size++;
    return 0;
}
```

`slot` - это указатель на указатель. Он хранит адрес того поля-указателя,
куда нужно положить новый узел. Если дерево пустое, `slot` указывает на
`t->root`. Если идём налево, `slot` становится адресом `(*slot)->left`.
Когда нашли `NULL`, запись `*slot = new TreeNode` сразу привязывает новый
узел к родителю. Отдельный `parent` не нужен.

Дубликаты запрещены: при равенстве возвращается `-1`, размер не меняется.

### 5.5. Удаление: три случая

Сначала итеративный поиск слота:

```cpp
TreeNode** slot = &t->root;
while (*slot != NULL && (*slot)->key != key) {
    slot = (key < (*slot)->key) ? &(*slot)->left : &(*slot)->right;
}
if (*slot == NULL) return -1;
```

Дальше три случая:

1. **Лист** - просто заменить указатель на `NULL` и удалить узел.
2. **Один ребёнок** - заменить указатель на этого ребёнка и удалить узел.
3. **Два ребёнка** - заменить ключ узла на минимум из правого поддерева,
   затем отсоединить этот минимум.

Минимум ищется итеративно:

```cpp
static int tree_detach_min(TreeNode** slot) {
    while ((*slot)->left != NULL) {
        slot = &(*slot)->left;
    }
    TreeNode* victim = *slot;
    int key = victim->key;
    *slot = victim->right;
    delete victim;
    return key;
}
```

У минимума нет левого ребёнка. Его правое поддерево (если есть)
подставляется на его место. Ключ возвращается, чтобы записать его в
удаляемый узел. Размер уменьшается только при успешном удалении.

### 5.6. Печать

```cpp
static void tree_print_node(const TreeNode* n) {
    if (n == NULL) return;
    tree_print_node(n->left);
    printf("%d\n", n->key);
    tree_print_node(n->right);
}
```

In-order обход BST даёт отсортированный по возрастанию вывод.

---

## 6. Двусвязный список: `list.cpp`

### 6.1. Инициализация и освобождение

```cpp
void list_init(List* l) {
    l->head = NULL;
    l->tail = NULL;
    l->size = 0;
}
```

Освобождение итеративное:

```cpp
ListNode* cur = l->head;
while (cur != NULL) {
    ListNode* next = cur->next;
    delete cur;
    cur = next;
}
```

Сначала сохраняется `next`, потом удаляется текущий. Иначе после `delete`
нельзя было бы прочитать `cur->next`.

### 6.2. `push_back`

```cpp
ListNode* n = new ListNode;
n->key  = key;
n->prev = l->tail;
n->next = NULL;
if (l->tail != NULL) l->tail->next = n;
else                 l->head = n;
l->tail = n;
l->size++;
```

Если список пуст, новый узел становится и головой, и хвостом. Иначе
старый хвост ссылается на новый. `prev` нового - старый хвост.

### 6.3. `push_front`

Симметрично:

```cpp
n->prev = NULL;
n->next = l->head;
if (l->head != NULL) l->head->prev = n;
else                 l->tail = n;
l->head = n;
l->size++;
```

### 6.4. Поиск и удаление

`list_find` идёт от головы по `next` и возвращает первый узел с ключом.

`list_remove` ищет первый такой узел, затем перецепляет соседей:

```cpp
if (cur->prev != NULL) cur->prev->next = cur->next;
else                   l->head = cur->next;

if (cur->next != NULL) cur->next->prev = cur->prev;
else                   l->tail = cur->prev;

delete cur;
l->size--;
```

Отдельно обрабатываются голова, хвост и середина. Если узел не найден,
возвращается `-1`.

---

## 7. Хеш-таблица: `hash.cpp`

### 7.1. Инициализация

```cpp
int hash_init(Hash* h, int capacity) {
    if (capacity <= 0) return -1;
    h->buckets = new HashNode*[capacity];
    for (int i = 0; i < capacity; i++) h->buckets[i] = NULL;
    h->capacity = capacity;
    h->size = 0;
    return 0;
}
```

`buckets` - массив указателей. Каждый бакет изначально пуст. Ёмкость
фиксируется и не меняется.

### 7.2. FNV-1a

```cpp
static unsigned int hash_fn(int key, int cap) {
    unsigned int h = 2166136261u;
    unsigned char* p = (unsigned char*)&key;
    for (int i = 0; i < (int)sizeof(int); i++) {
        h ^= p[i];
        h *= 16777619u;
    }
    return h % (unsigned int)cap;
}
```

Берутся байты `int` по адресу `&key`. Для каждого байта: XOR с текущим
хешем, затем умножение на FNV-prime. Результат сворачивается по модулю
`capacity`. Хеш зависит от размера `int` и порядка байтов, но для учебной
структуры этого достаточно.

### 7.3. Вставка, поиск, удаление

`hash_insert`:

1. вычисляет индекс;
2. идёт по цепочке и ищет дубликат;
3. если дубликата нет, создаёт узел и вставляет его **в голову** цепочки;
4. увеличивает `size`.

Поэтому внутри бакета новые элементы идут раньше старых.

`hash_find` идёт по цепочке и возвращает `1`/`0`.

`hash_remove` использует указатель на указатель:

```cpp
HashNode** slot = &h->buckets[idx];
while (*slot != NULL && (*slot)->key != key) {
    slot = &(*slot)->next;
}
if (*slot == NULL) return -1;
HashNode* victim = *slot;
*slot = victim->next;
delete victim;
h->size--;
```

Это позволяет удалить узел из головы или из середины цепочки без
отдельного `prev`.

`hash_print` печатает по бакетам, внутри бакета - в порядке цепочки.
Это не отсортированный вывод.

---

## 8. Слой алгоритмов: `*_algo.cpp`

### 8.1. `tree_algo.cpp`

**`tree_validate`** - проверка BST-инварианта. Рекурсивный
`validate_node(n, lo, hi)` принимает открытые границы `long long`:

```cpp
static int validate_node(const TreeNode* n, long long lo, long long hi) {
    if (n == NULL) return 1;
    if ((long long)n->key <= lo || (long long)n->key >= hi) return 0;
    if (!validate_node(n->left,  lo, (long long)n->key)) return 0;
    if (!validate_node(n->right, (long long)n->key, hi)) return 0;
    return 1;
}
```

Стартовые границы - `-2147483648LL - 1` и `2147483647LL + 1`, то есть
чуть шире `int`. Строгие `<`/`>` означают, что дубликаты недопустимы:
повтор ключа не может быть `> lo` и `< hi` одновременно на одной стороне
родителя. Это согласовано с `tree_insert` (он отвергает duplicate).

**`tree_min` / `tree_max`** - итеративный спуск влево/вправо до упора.
`const TreeNode*`, не мутируют.

**`tree_to_array_inorder`** - рекурсивный in-order, пишет ключи в `buf`,
возвращает `pos`. Требует `cap >= t->size`.

**`tree_from_sorted_array`** - рекурсивный `build_balanced(a, lo, hi)`:
`mid = lo + (hi - lo) / 2`, левое и правое поддеревья рекурсивно. Даёт
идеально сбалансированное дерево, глубина рекурсии `O(log n)`.
Требует пустое дерево (`root == NULL && size == 0`).

**`tree_balance`** - in-order → массив → `tree_free` → `from_sorted_array`.
O(n) время, O(n) память.

**`tree_bfs`** - очередь на массиве `const TreeNode*[t->size]`,
`head`/`tail` индексы, без `std::queue`. Колбэк `cb(key, user)` на каждый
узел, возвращает число посещённых.

### 8.2. `list_algo.cpp`

**`list_reverse`** - итеративно, swap `prev`/`next` по узлам, затем swap
`head`/`tail`.

**`list_middle`** - two-pointer. Для чётного размера возвращает **нижнюю**
середину: `slow = head`, `fast = head->next` в старте. Защита от
`l->head == NULL`, потому что `head->next` разыменовывает `head`.

**`list_sort`** - stable merge sort по `next`. `list_merge_sorted(left,
right, tail)` использует `<=` (stable), восстанавливает `prev` и пишет
последний узел в `*tail`. `list_sort_chain(head, tail)` рекурсивно
делит пополам (нижняя середина), сортирует половины, сливает. Глубина
рекурсии `O(log n)`. После сортировки `tail` восстановлен, `size` не
меняется (то же число узлов).

### 8.3. `hash_algo.cpp`

**`hash_stats`** - один проход по бакетам. Для каждого бакета считается
длина цепочки. `min_chain` - минимум по непустым (`-1` до первого
непустого, потом `0` для пустой таблицы). `max_chain` - максимум.
`empty_buckets` - число пустых. `load_factor = (double)size / capacity`.

---

## 9. REPL: `src/repl.cpp`

### 9.1. Одна живая структура

`g_kind` говорит, что сейчас живо. `teardown()` уничтожает ровно её.
Команды создания сначала вызывают `teardown()`, поэтому старая структура
не утекает.

```text
tree      -> teardown(); tree_init(&g_tree); g_kind = KIND_TREE;
list      -> teardown(); list_init(&g_list); g_kind = KIND_LIST;
hash [c]  -> teardown(); hash_init(&g_hash, c); g_kind = KIND_HASH;
free      -> teardown();
quit/exit -> teardown(); выход.
```

### 9.2. `execute_line` - единый диспетчер

Диспетчеризация вынесена из `main` в `execute_line(char* line)`:

- `0` - команда выполнена (OK);
- `1` - ошибка команды (неизвестная команда, плохой аргумент,
  `need(...)` не прошёл, `save`/`load` вернули `-1`);
- `2` - `quit`/`exit`.

`execute_line` обрезает `\n`/`\r`, вызывает `next_token`, затем плоскую
`strcmp`-цепочку. Для команд с аргументом - `parse_whole_int`:

```cpp
static int parse_whole_int(const char* s, int* out) {
    if (s == NULL || *s == '\0') return 0;
    errno = 0;
    char* end = NULL;
    long v = strtol(s, &end, 10);
    if (errno == ERANGE) return 0;
    if (*end != '\0') return 0;
    if (v < -2147483647L - 1 || v > 2147483647L) return 0;
    *out = (int)v;
    return 1;
}
```

Он требует, чтобы строка была целиком одним целым: без «хвостов»,
без пустоты, без переполнения `int`.

`execute_line` **модифицирует** строку (обрезает `\n`, `next_token`
двигает указатели). Поэтому в batch-режимах `--exec` копируется в
буфер перед разбором.

### 9.3. Команды

- `tree`, `list`, `hash [cap]` - создать структуру.
- `insert <k>` - для tree/hash вставка, для list - `push_back`.
- `find <k>` - поиск во всех.
- `remove <k>` - удаление во всех.
- `push_back <k>`, `push_front <k>` - только list.
- `print` - печать ключей.
- `draw | schema | ascii` - ASCII-диаграмма.
- `size` - текущий размер.
- `height` - высота, только tree.
- `validate` - BST-инвариант, только tree.
- `min | max` - крайние ключи, только tree.
- `balance` - перестроить сбалансированно, только tree.
- `reverse` - развернуть in place, только list.
- `sort` - stable merge sort, только list.
- `stats` - статистика цепочек, только hash.
- `save <file.repl>`, `load <file.repl>` - сохранение/загрузка.
- `free` - уничтожить текущую структуру.
- `help`, `quit | exit`.

`need(Kind, what)` проверяет, что команда применима к текущей структуре,
и печатает ошибку с ожидаемым и текущим типом.

### 9.4. ASCII-диаграммы

Все draw-функции принимают `const`-указатели и не мутируют структуры.

**tree.** Рекурсивный обход с префиксами:

```text
5
|-- 3
|   |-- 1
|   `-- 4
`-- 8
    |-- 7
    `-- 9
size=7 height=3
```

Ветка `|--` - не последний ребёнок, `` `-- `` - последний. Префикс
накапливает `|   ` или `    ` для отступов.

**list.** Печатается как:

```text
head [1] <-> [2] <-> [3] tail
size=3
```

Если элементов больше `LIST_DRAW_MAX` (10), середина сворачивается в
`... (N more) ...`.

**hash.** Один бакет на строку:

```text
hash (capacity=8, size=3)
[ 0] (empty)
[ 1] 20 -> 10
...
load factor: 0.37
```

Load factor в `draw` считается целочисленно:
`lf100 = size * 100 / capacity`, затем печатается `lf100 / 100` и
`lf100 % 100`. В `stats` - `double` (`3/8 → 0.38`). Это разные формулы,
и это нормально.

### 9.5. Save: `.repl`

`save_current`:

1. проверяет, что структура есть;
2. предупреждает, если путь не заканчивается на `.repl` (но не падает);
3. открывает файл на запись;
4. пишет комментарии:

```text
# algo repl save
# structure: tree
```

5. пишет тег и данные:
   - tree: `tree`, затем ключи in-order;
   - list: `list`, затем ключи head-to-tail;
   - hash: `hash <capacity>`, затем ключи по бакетам.

Для дерева in-order ключи складываются в динамический `int[]` размера
`t->size`, затем построчно печатаются. Для списка и хеша можно писать
сразу в `FILE*`.

### 9.6. Load: `.repl`

`load_from` устроен осторожно: **сначала строит временную структуру**,
и только при полном успехе заменяет живую. Если файл битый, живая
структура не теряется.

Алгоритм:

1. Проверить расширение, предупредить при отсутствии `.repl`.
2. Открыть файл.
3. Прочитать первую непустую некомментированную строку - это тег.
4. `parse_tag` понимает:
   - `tree`;
   - `list`;
   - `hash` или `hash <capacity>`; по умолчанию capacity = 16.
5. Создать временные `Tree t`, `List l`; `Hash h` инициализируется
   лениво при первом ключе.
6. Читать остальные строки: каждая - один целый ключ.
7. Для tree/hash дубликаты предупреждаются и пропускаются.
   Для list дубликаты разрешены.
8. При ошибке разбора ключа освободить временные структуры и вернуть `-1`.
9. При успехе вызвать `teardown()`, затем присвоить временную структуру
   глобальной и выставить `g_kind`.

Формат:

```text
# algo repl save
# structure: tree
tree
5
3
8
```

Строки с `#` и пустые строки пропускаются. Пробелы и `\r\n` обрезаются
функцией `trim`.

Важное следствие: save/load сохраняет **множество ключей**, но не всегда
точную форму:

- **tree**: in-order save + insert при load даёт вырожденное вправо
  дерево, а не исходную форму. Высота после load может стать `n`.
- **list**: порядок head-to-tail сохраняется.
- **hash**: capacity сохраняется, но при вставке в голову цепочки порядок
  элементов внутри бакета может развернуться.

### 9.7. Batch mode

`main(argc, argv)` разбирает флаги:

- без аргументов - интерактив (приветствие, `> `, `execute_line` в цикле,
  выход на `2`);
- `--exec "<commands>"` - копирует строку в буфер, `run_exec_string`
  разбивает по `;`, `execute_line` каждую, стоп на `1` (если нет
  `--keep-going`), на `2` - вернуть `0`;
- `--script <file>` - `run_script` читает файл через `fgets`,
  `execute_line` каждую строку, те же правила остановки;
- `--keep-going` - не останавливаться на ошибках (оба batch-режима);
- `--help` - `usage`, код `0`;
- неизвестный флаг - `usage`, код `1`.

В batch-режимах **не печатается** `> ` и приветствие. Код возврата:
`0` при успехе, `1` при ошибке команды. `quit`/`exit` внутри
`--exec`/`--script` останавливает batch и возвращает `0`.

`run_exec_string` не использует `strtok`, потому что `strtok` мутирует
строку и «съедает» пустые куски; свой цикл по `strchr(start, ';')`
проще и предсказуемее.

---

## 10. Тесты

### 10.1. Unit-тесты

Отдельные программы с `main` и `assert` из `<assert.h>`.
Никакого фреймворка.

`tree_test.cpp`, `list_test.cpp`, `hash_test.cpp` - базовые операции
структур (insert/find/remove/height/print, push/find/remove,
init/insert/collision/remove).

`tree_algo_test.cpp`:

- validate: пустое, хорошее, битое;
- min/max: пустое, крайние ключи;
- to_array_inorder: отсортировано, длина == size;
- from_sorted_array: сбалансировано, validate;
- balance: вырожденное 1..7 → высота 3, validate;
- bfs: уровне-порядок через колбэк.

`tree_algo_bounds_test.cpp`:

- проверяет границы `int`: `INT_MIN` / `INT_MAX`
- При построеннии дерева из отсортированного массива
- При проверки `tree_validate`
- Поиск минимума и максимума и преобразование в in-order массив.

`list_algo_test.cpp`:

- reverse: пусто, один, чётное, нечётное - head/tail/prev/next;
- middle: пусто, один, нечётное, чётное (нижняя);
- sort: пусто, один, обратный вход, отсортированный вход, дубликаты;
- `check_forward` проверяет `prev`-цепочку и `tail`.

`hash_algo_test.cpp`:

- пустая: capacity 8, всё по нулям, `empty_buckets == 8`;
- все в одном бакете: capacity 1, три ключа, `min == max == 3`;
- spread: capacity 8, восемь ключей, поля согласованы;
- bad args: `NULL` h / `NULL` out → `-1`.

`assert` при провале просто abort-ит программу. `main` печатает
`>>> test_name`, затем запускает тест. В конце - `all <struct> tests passed`.

### 10.2. Golden REPL-тесты

`tests/repl_golden/` - 10 кейсов: `case01.in` … `case10.in` и
соответствующие `.out`. Каждый `.in` подаётся через `--script`,
вывод сравнивается с `.out` через `diff`.

`tests/run_golden.sh`:

- `set -e`, `export LC_ALL=C` (чтобы `strerror` был стабилен);
- `normalize()`: `sed` убирает `^> ` (на случай, если попадёт),
  заменяет `/tmp/...` и `tests/repl_golden/...repl` на `<FILE>`;
- цикл по `$DIR/*.in`, для каждого - `--script`, `diff -u`
  нормализованных `.out` и `.got`;
- при расхождении - `FAIL`, `cat diff`, `exit 1`;
- в конце - `all golden tests passed`.

Кейсы:

1. tree: insert/draw/validate/min/max/height/size
2. tree: balance на вырожденном (1..7 → draw)
3. list: push_back/push_front/draw/reverse/draw/sort/draw
4. hash: create/insert/stats/draw/remove
5. save/load tree round-trip
6. save/load list round-trip
7. save/load hash round-trip
8. ошибки: `insert abc`, `height` без tree, `hash 0`, `load nosuchfile`
9. дубликаты: tree/hash → `error: insert N failed (duplicate?)`,
   list → `ok`
10. `free` + повторное создание

`make test` = unit + golden. `make test-repl` = только golden.

---

## 11. Как всё работает вместе: пример

```text
> tree
tree created
> insert 5
ok
> insert 3
ok
> insert 8
ok
> draw
5
|-- 3
`-- 8
size=3 height=2
> validate
valid
> balance
ok
> save demo.repl
saved demo.repl (3 keys)
> free
freed
> load demo.repl
loaded demo.repl (3 keys)
> draw
5
|-- 3
`-- 8
size=3 height=2
> quit
bye
```

При `load` дерево строится заново из in-order ключей. Для трёх ключей
`3,5,8` вставка даст цепочку `3 -> 5 -> 8` справа, поэтому draw после
load может отличаться от исходного, если исходное дерево было
сбалансировано. В примере выше исходное дерево тоже было `5/3/8`, но
в общем случае форма не гарантируется.

---

## 12. Важные детали, ограничения и подводные камни

1. **Нет RAII.** `Tree`, `List`, `Hash` не освобождают себя сами.
   Нужно явно вызывать `tree_free`, `list_free`, `hash_free`.
2. **Нет конструкторов.** После `Tree t;` обязательно `tree_init(&t)`.
   Иначе поля содержат мусор.
3. **Поверхностное копирование структур.** `g_tree = t` копирует
   указатель на корень. Если после этого освободить и `t`, и `g_tree`,
   будет double free. В `load_from` так не делается: временная структура
   передаётся живой, а старая уничтожается.
4. **`new` может бросить `std::bad_alloc`.** Проект это не обрабатывает:
   это эксперимент, а не продакшн.
5. **Рекурсивные `tree_free`, `tree_height`, `tree_print`.** На очень
   глубоком дереве возможен stack overflow. Итеративная только вставка
   (и удаление). В `tree_algo` рекурсивны `validate`, `to_array_inorder`,
   `build_balanced`; в `list_algo` - `sort`.
6. **Хеш без ресайза.** `capacity` фиксирована. При большом `size`
   цепочки растут, load factor растёт.
7. **`hash_fn` зависит от endianness и `sizeof(int)`.** Для учебной
   задачи это нормально, но не переносимая криптография.
8. **`hash_print` не сортирует.** Порядок - по бакетам и цепочкам.
9. **`list_find` принимает `List*`, а не `const List*`.** Хотя не мутирует;
   это API-особенность.
10. **REPL-буферы фиксированные.** `fgets` в `char line[256]`; очень
    длинная строка будет обработана частями как несколько команд.
11. **Save/load не сохраняет точную форму tree/hash.** Сохраняется
    множество ключей и capacity; форма дерева и порядок цепочек могут
    измениться.
12. **Расширение `.repl` только предупреждение.** `save`/`load` не
    отказываются работать с другим расширением.
13. **Дубликаты при load.** Для tree/hash предупреждаются и
    пропускаются; для list разрешены.
14. **`teardown()` - единственная точка уничтожения живой структуры.**
    Временные структуры при ошибке load освобождаются отдельно, но
    живая не трогается до успеха.
15. **`*_algo` не меняют старые API.** `tree.hpp`, `list.hpp`,
    `hash.hpp` не тронуты; `*_algo` - отдельные модули (ROADMAP §0.6).
16. **`tree_algo` использует `const TreeNode**` в BFS-очереди.**
    Это строже, чем `TreeNode**`, и соответствует контракту `const Tree*`.
17. **`list_middle` для чётного размера возвращает нижнюю середину.**
    `fast = head->next` в старте; это зафиксировано в тесте
    `test_middle_even` (`1..4` → `2`).
18. **`hash_stats.min_chain` - только по непустым бакетам.**
    `0` для пустой таблицы; это зафиксировано в комментарии в
    `hash_algo.hpp` и в тесте `test_empty`.
19. **`load_factor` в `draw` и `stats` считается по-разному.**
    `draw`: целочисленно, `lf100 = size * 100 / capacity`, печать
    `lf100 / 100` и `lf100 % 100` (`3/8 → 0.37`).
    `stats`: `double`, `(double)size / capacity` (`3/8 → 0.38`).
    Это не ошибка - разные формулы для разных целей. Если нужно
    единообразие - привести `draw` к `double` (но тогда golden-кейс
    `case04` поменяется).
20. **`execute_line` модифицирует строку.** `--exec` копирует `argv`
    в буфер перед разбором; `--script` читает в `char line[256]`,
    которую можно модифицировать. Интерактив - тоже `char line[256]`.

---

## 13. Итоговая схема

```text
                 +-------------------+
                 |      repl.cpp     |
                 |  g_kind, teardown |
                 |  execute_line     |
                 |  cmd_*, draw      |
                 |  save/load        |
                 |  --exec/--script  |
                 +---------+---------+
                           |
          +----------------+----------------+
          |                |                |
          v                v                v
     tree.cpp          list.cpp         hash.cpp
     BST               DLL              chained hash
     TreeNode**        prev/next        buckets + FNV-1a
     new/delete        new/delete       new/delete
          |                |                |
          +----------------+----------------+
                           |
          +----------------+----------------+
          |                |                |
          v                v                v
     tree_algo.cpp    list_algo.cpp    hash_algo.cpp
     validate         reverse          hash_stats
     min/max          middle
     inorder/from     sort (stable
     balance          merge)
     bfs
          |
          +----------------+----------------+
                           |
          +----------------+----------------+
          |                                 |
          v                                 v
   tests/*_test.cpp                  tests/run_golden.sh
   assert-based                      repl --script
   make test (unit)                  tests/repl_golden/*.in/.out
                                     make test-repl (golden)
```

Это и есть вся система: три ручные структуры данных с C-подобным API,
слой алгоритмов над каждой, REPL с одной живой структурой, ASCII-
визуализация, текстовый save/load, интерактив и batch-режимы,
unit-тесты и golden-тесты.

/* ============================================================
 * Original code and README:
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * Copyright (c) 2026 Albert Ghosting Frangle (Albert Fisher)
 *
 * This explanation is generated from the provided sources.
 * ============================================================ */
