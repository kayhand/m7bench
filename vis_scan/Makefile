CC=/opt/solarisstudio12.4/bin/cc
CFLAGS=    -xarch=native -m64 -xvis -I /opt/solarisstudio12.4/lib/compilers/include/cc/

.PHONY: all clean cleannall

all: canary load_unsigned_word 8bits

#8bits

%:%.c
	$(CC) -gHV -O   -o $@ $^ $(CFLAGS)
	$(CC) -S   -o $@.asm $^ $(CFLAGS)

clean:
	rm -f *.asm
	rm -f load_unsigned_word 8bits canary

cleanall: clean
	rm -f *~
