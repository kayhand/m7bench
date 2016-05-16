ifndef case
${error case is not set -- example run =>  "make case=12bits"}
endif

CC=gcc
CFLAGS=-std=c99 -O3 -march=core-avx-i -mtune=core-avx-i -mavx2
RM=rm -rf
OUT=${case}
INCLUDES=-I .
LIBS=-l m

all: build
build: ${case}.o
	$(CC) $(INCLUDES) $(CFLAGS) -o $(OUT) $(LIBS) ${case}.o
	$(RM) *.o

${case}.o: ${case}.c
	$(CC) $(INCLUDES) $(CFLAGS) -c ${case}.c
clean:
	$(RM) *.o $(OUT)
