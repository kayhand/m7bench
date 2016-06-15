CC=gcc
#CC=/opt/solarisstudio12.5/bin/cc
CFLAGS= -O3 -g -Wall
RM=rm -f
OUT=bitweaving
INCLUDES=-I .
LIBS=-l m -lpthread 
#LIBS=-l m -lpthread -llgrp
all: build
build: bitweaving.o
	$(CC)  $(INCLUDES) $(CFLAGS) -o $(OUT) bitweaving.o $(LIBS)
	$(RM) *.o

bitweaving.o: bitweaving.c
	$(CC) $(INCLUDES) $(CFLAGS) -c bitweaving.c
clean:
	$(RM) *.o $(OUT)

