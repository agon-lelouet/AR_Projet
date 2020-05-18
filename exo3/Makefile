CC=mpicc

LDFLAGS=-lm
CFLAGS=-g
DEP=obj/utils.o obj/main.o
TARGET=bin/exo3

all: $(TARGET)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@ 

bin/exo3: $(DEP)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm $(TARGET) $(DEP)
