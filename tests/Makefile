CC=gcc
LFLAGS=-m32
CFLAGS=-s -m32
BIN1=test_read
BIN2=test_readblock
SRC1=../src/ssm2.c test_read.c
SRC2=../src/ssm2.c test_readblock.c
OBJ1=$(SRC1:.c=.o)
OBJ2=$(SRC2:.c=.o)

all: $(BIN1) $(BIN2) _clean

$(BIN1): $(OBJ1)
	$(CC) $^ -o $@ $(LFLAGS)

$(BIN2): $(OBJ2)
	$(CC) $^ -o $@ $(LFLAGS)

../src/%.o: ../src/%.c
	$(CC) $^ -o $@ $(CFLAGS) -c

_clean:
	rm -f $(OBJ1) $(OBJ2)

clean:
	rm -f $(OBJ1) $(BIN1) $(BIN2)
