CC=mpicc

LDFLAGS=-lm
CFLAGS=-g
DEP=obj/utils.o obj/main.o

all: bin/exo1

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@ 

bin/exo1: $(DEP)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm bin/* obj/*
