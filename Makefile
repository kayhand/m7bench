CC=gcc
CFLAGS=-g -std=c99 -O3
RM=rm -f
OUT=m7bench
INCLUDES=-I .
LIBS=-l m

all: build
build: m7bench.o
	$(CC)  $(INCLUDES) $(CFLAGS) -o $(OUT) m7bench.o $(LIBS)
	$(RM) *.o

m7bench.o: m7bench.c
	$(CC) $(INCLUDES) $(CFLAGS) -c m7bench.c
clean:
	$(RM) *.o $(OUT)

