ifndef case
${error case is not set -- example run =>  "make case=12bits"}
endif

CC=gcc
CFLAGS=-std=gnu99 -O3 -march=core-avx-i -mtune=core-avx-i -mavx2
RM=rm -rf
OUT=${case}
INCLUDES=-I .
LIBS=-l m 

all: build
build: ${case}.o
	$(CC) $(INCLUDES) $(CFLAGS) $(LIBS) -o $(OUT) ${case}.o
	$(RM) *.o

${case}.o: ${case}.c
	$(CC) $(INCLUDES) $(CFLAGS) -c ${case}.c $(LIBS)
clean:
	$(RM) *.o $(OUT)
