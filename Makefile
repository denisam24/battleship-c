CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
SRC = battleship.c
EXE = battleship

all:
	$(CC) $(CFLAGS) -o $(EXE) $(SRC)

run: all
	./$(EXE)


.PHONY: clean
clean:
	rm -f $(EXE) *~
