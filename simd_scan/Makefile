CC=
CFLAGS= -g -Wall -std=c99  -O3 -march=core-avx-i -mtune=core-avx-i -mavx2
RM=rm -f
INCLUDES=-I .
LIBS=-l m
OUT= 4bits 8bits 12bits 16bits 17bits
# 3bits

all: $(OUT)

% :%.c 

% : %.o
	gcc -o $@ $(CFLAGS) $(INCLUDES) $^ $(LIBS)

%.o : %.c
	gcc -c $(CFLAGS) $(INCLUDES) $^ $(LIBS)


clean:
	$(RM) *.o $(OUT) 

cleanall: clean
	$(RM) *~

