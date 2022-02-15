CC = gcc
FLAGS = -pedantic -Wall -Wextra -Werror
SRC = $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJ = $(SRC:.c=.o)
BIN = solitare

%.o: %.c
	$(CC) $(FLAGS) -g -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) -lncurses

.PHONY: clean
clean:
	@rm $(OBJ)
	@rm $(BIN)
