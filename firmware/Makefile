CXX = clang++
CXXFLAGS = $(CXXFLAGS1)

CXXFLAGS0 = -Wall -Werror -std=c++20
CXXFLAGS1 = -Wall -Werror -std=c++20 -g -fsanitize=address,leak,undefined

SRC = source
BIN = build/bin
OBJ = build/objects
INCLUDE = include

.PHONY: all
all: firmware

firmware: $(OBJ)/main.o $(OBJ)/dbc.o
	$(CXX) $(CXXFLAGS) -o $(BIN)/firmware $(OBJ)/main.o $(OBJ)/dbc.o

$(OBJ)/main.o: $(SRC)/main.cpp build
	$(CXX) -o $(OBJ)/main.o -c $(SRC)/main.cpp -I $(INCLUDE)

$(OBJ)/dbc.o: $(SRC)/dbc.cpp build $(INCLUDE)/dbc.h
	$(CXX) -o $(OBJ)/dbc.o -c $(SRC)/dbc.cpp -I $(INCLUDE)

build:
	mkdir -p build/bin build/objects

.PHONY: clean
clean:
	rm -rf build
