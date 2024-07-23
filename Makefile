CC=g++
CFLAGS=-Wall -std=c++17
OBJ=obj
BIN=bin
SRC=src
INC=include

all: Lexer
	# make Lexer
	# make Parser
	$(CC) $(SRC)/main.cpp -o $(BIN)/main $(OBJ)/*.o $(CFLAGS)

Lexer: $(SRC)/lexer.cpp $(INC)/lexer.h
	$(CC) -c $(SRC)/lexer.cpp -o $(OBJ)/lexer.o $(CFLAGS)

Parser: $(SRC)/Parser.cpp $(INC)/Parser.h
	$(CC) -c $(SRC)/Parser.cpp -o $(OBJ)/Parser.o $(CFLAGS)

clean: 
	rm -rf $(BIN)/ $(OBJ)
	mkdir $(BIN)/ $(OBJ)
