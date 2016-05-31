CC=/opt/solarisstudio12.5/bin/cc
CFLAGS=   -xarch=native -m64 -xvis -I /opt/solarisstudio12.5/lib/compilers/include/cc/ -lm -lpthread
TARGETS = canary 4bits 8bits 9bits 12bits 16bits 32bits test 8bits_mt 32bits_mt 4bits_mt 9bits_mt 12bits_mt 16bits_mt 15bits 15bits_mt

.PHONY: all clean cleannall

all: $(TARGETS)

#8bits

%:%.c util.o clock.il
	$(CC) -g -O   -o $@ $^ $(CFLAGS)
	#$(CC) -S   -o $@.asm $^ $(CFLAGS)

util.o:util.h

%.o:%.c
	$(CC) -g -c -O   -o $@ $^ $(CFLAGS)


clean:
	rm -f *.asm *.o
	rm -f $(TARGETS)

cleanall: clean
	rm -f *~
