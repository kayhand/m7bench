CC=gcc
CFLAGS=-std=c99 -O3  -Wall
RM=rm -f
OUT=datagen
INCLUDES=-I .
LIBS=-l m

all: $(OUT)

%:%.o
	$(CC)  $(CFLAGS) $(INCLUDES)  -o $@ $^   $(LIBS)

%.o:%.c
	$(CC)  $(CFLAGS) $(INCLUDES) -c $^  $(LIBS)

clean:
	$(RM) *.o $(OUT) *~

