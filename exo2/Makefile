CC=mpicc

LDFLAGS=-lm
CFLAGS=-g
DEP=obj/utils.o obj/main.o
TARGET=bin/exo2

all: $(TARGET)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@ 

bin/exo2: $(DEP)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm $(TARGET) $(DEP)
