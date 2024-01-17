.POSIX:

CXX=g++
WARNINGS=-Werror -Wall -Wextra
CXXFLAGS=-std=c++2b $(WARNINGS)
SRC=src/*.cpp
HEADERS=src/*.hpp
BUILD=build

build: src/main.cpp
	$(CXX) $(SRC) -g -o $(BUILD)/a.out $(CXXFLAGS)

run: build
	./$(BUILD)/a.out $(ARGS)

test:
	./$(BUILD)/a.out $(ARGS)

format:
	clang-format -i $(SRC) $(HEADERS)

.PHONY: build run format
