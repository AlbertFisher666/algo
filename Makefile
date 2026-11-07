# Makefile — algo
# Style: minimal, explicit.

CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I include

tree_test: tests/tree_test.cpp src/tree.cpp
	$(CXX) $(CXXFLAGS) -o tree_test tests/tree_test.cpp src/tree.cpp

test: tree_test
	./tree_test

clean:
	rm -f tree_test

.PHONY: test clean
