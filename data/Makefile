CC=gcc
CFLAGS=-std=c99 -O3 
RM=rm -rf
OUT=datagen
INCLUDES=-I .
LIBS=-l m

all: build
build: datagen.o
	$(CC) $(INCLUDES) $(CFLAGS) -o $(OUT) datagen.o $(LIBS)
	$(RM) *.o

datagen.o: datagen.c
	$(CC) $(INCLUDES) $(CFLAGS) -c datagen.c
clean:
	$(RM) *.o $(OUT)

