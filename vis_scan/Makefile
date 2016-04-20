CC=/opt/solarisstudio12.4/bin/cc
CFLAGS=    -xarch=native -m64 -xvis -I /opt/solarisstudio12.4/lib/compilers/include/cc/
TARGETS = canary load_unsigned_word 8bits  16bits 32bits

.PHONY: all clean cleannall

all: $(TARGETS)

#8bits

%:%.c 
	$(CC) -gHV -O   -o $@ $^ $(CFLAGS)
	$(CC) -S   -o $@.asm $^ $(CFLAGS)

clean:
	rm -f *.asm
	rm -f $(TARGETS)

cleanall: clean
	rm -f *~
