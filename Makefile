# Makefile — algo

SRC      = src/tree.cpp src/list.cpp src/hash.cpp
REPL_SRC = src/repl.cpp $(SRC)

CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I include
BIN      = build

test: $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/tree_test tests/tree_test.cpp $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN)/list_test tests/list_test.cpp $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN)/hash_test tests/hash_test.cpp $(SRC)
	./$(BIN)/tree_test
	./$(BIN)/list_test
	./$(BIN)/hash_test

repl: $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/repl $(REPL_SRC)

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -rf $(BIN)

.PHONY: test repl clean
