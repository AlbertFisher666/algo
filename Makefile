# Makefile for algo v1.1
# 80s C-flavoured C++ - build rules.

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
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJS) -o $@

repl: $(REPL_BIN)

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "=== $$t ==="; ./$$t || exit 1; done
	@echo "all tests passed"

test-repl: $(REPL_BIN)
	@bash tests/run_golden.sh

clean:
	rm -rf $(BUILD)
