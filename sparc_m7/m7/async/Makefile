CC=/opt/solarisstudio12.5/bin/cc
CFLAGS=-g -m64
RM=rm -rf
OUT=async
INCLUDES=-I /opt/dax
LIBS=-L /opt/dax -R /opt/dax -ldax -lthread 

all: build
build: async.o
	$(CC) $(INCLUDES) $(CFLAGS) -o $(OUT) $(LIBS) async.o
	$(RM) *.o 

dax_async.o: async.c
	$(CC) $(INCLUDES) $(CFLAGS) -c async.c

clean:
	$(RM) *.o $(OUT)
