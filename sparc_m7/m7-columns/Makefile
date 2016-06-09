CC=/opt/solarisstudio12.5/bin/cc
CFLAGS=-g -m64
RM=rm -f
OUT=m7bench
INCLUDES=-I /opt/dax
LIBS=-L /opt/dax -R /opt/dax -ldax -lthread 

all: build
build: m7bench.o
	$(CC) $(INCLUDES) $(CFLAGS) -o $(OUT) $(LIBS) m7bench.o
	$(RM) *.o 

dax_m7bench.o: m7bench.c
	$(CC) $(INCLUDES) $(CFLAGS) -c m7bench.c $(LIBS)

clean:
	$(RM) *.o $(OUT)
