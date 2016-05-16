CC=gcc
CFLAGS= -O3 
RM=rm -f
OUT=bitweaving
INCLUDES=-I .
LIBS=-l m -lpthread 

all: build
build: bitweaving.o
	$(CC)  $(INCLUDES) $(CFLAGS) -o $(OUT) bitweaving.o $(LIBS)
	$(RM) *.o

bitweaving.o: bitweaving.c
	$(CC) $(INCLUDES) $(CFLAGS) -c bitweaving.c
clean:
	$(RM) *.o $(OUT)

