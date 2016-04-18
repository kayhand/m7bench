CC=/opt/solarisstudio12.4/bin/cc

.PHONY: all clean cleannall

all: load_unsigned_word

%:%.c
	$(CC) -g  -o $@ $^ -m64  -xarch=sparcvis -xvis
	$(CC) -S   -o $@.asm $^ -m64  -xarch=sparcvis -xvis

clean:
	rm -f *.asm
	rm -f load_unsigned_word

cleanall: clean
	rm -f *~
