CC=/opt/solarisstudio12.5/bin/cc
CFLAGS=-g -m64 -xpagesize=4M -DDEBUG=1
RM=rm -rf
OUT=datagen
INCLUDES=-I .
LIBS=-L /usr/lib/sparcv9 -lm 

all: build
build: datagen.o
	$(CC) $(INCLUDES) $(CFLAGS) -o $(OUT) $(LIBS) datagen.o
	$(RM) *.o 

datagen.o: datagen.c
	$(CC) $(INCLUDES) $(CFLAGS) -c datagen.c

clean:
	$(RM) *.o $(OUT)
