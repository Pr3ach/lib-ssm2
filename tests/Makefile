BIN=test
CC=gcc
LFLAGS=-m32
CFLAGS=-s -m32
SRC=../src/ssm2.c test.c
OBJ=$(SRC:.c=.o)

all: $(BIN) _clean

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(LFLAGS)

../src/%.o: ../src/%.c
	$(CC) $^ -o $@ $(CFLAGS) -c

_clean:
	rm -f $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)
