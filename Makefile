CC=g++
CFLAGS=-Wall -std=c++17
OBJ=obj
BIN=bin
SRC=src
INC=include

all: Lexer Parser
	$(CC) $(SRC)/main.cpp -o $(BIN)/main $(OBJ)/*.o $(CFLAGS)

Lexer: $(SRC)/lexer.cpp $(INC)/lexer.h
	$(CC) -c $(SRC)/lexer.cpp -o $(OBJ)/lexer.o $(CFLAGS)

Parser: $(SRC)/parser.cpp $(INC)/parser.h
	$(CC) -c $(SRC)/parser.cpp -o $(OBJ)/parser.o $(CFLAGS)

clean: 
	rm -rf $(BIN)/ $(OBJ)
	mkdir $(BIN)/ $(OBJ)
