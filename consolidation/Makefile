CC=gcc
CFLAGS=-Wall -g

.PHONY: clean cleanall

all: main

main:main.o util.o
	gcc $(CFLAGS) -o $@ $^ -lm

%.o:%.c
	gcc $(CFLAGS) -c $< -o $@ -lm

%:%.o util.o


clean:
	rm -f main *.o
 
cleanall: clean
	rm -f *~
