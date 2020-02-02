CC=gcc
LFLAGS=-m32
CFLAGS=-s -m32
BIN1=test_read
BIN2=test_readblock
BIN3=dump_ecu_read
BIN4=dump_ecu_readblock
SRC1=../src/ssm2.c test_read.c
SRC2=../src/ssm2.c test_readblock.c
SRC3=../src/ssm2.c dump_ecu_read.c
SRC4=../src/ssm2.c dump_ecu_readblock.c
OBJ1=$(SRC1:.c=.o)
OBJ2=$(SRC2:.c=.o)
OBJ3=$(SRC3:.c=.o)
OBJ4=$(SRC4:.c=.o)

all: $(BIN1) $(BIN2) $(BIN3) $(BIN4) _clean

$(BIN1): $(OBJ1)
	$(CC) $^ -o $@ $(LFLAGS)

$(BIN2): $(OBJ2)
	$(CC) $^ -o $@ $(LFLAGS)

$(BIN3): $(OBJ3)
	$(CC) $^ -o $@ $(LFLAGS)

$(BIN4): $(OBJ4)
	$(CC) $^ -o $@ $(LFLAGS)

../src/%.o: ../src/%.c
	$(CC) $^ -o $@ $(CFLAGS) -c

_clean:
	rm -f $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)

clean: _clean
	rm -f $(BIN1) $(BIN2) $(BIN3) $(BIN4)
