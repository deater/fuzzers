CC = gcc
CFLAGS = -O2 -Wall
LFLAGS =

all:	elf_fuzzer

elf_fuzzer:	elf_fuzzer.o
	$(CC) $(LFLAGS) -o elf_fuzzer elf_fuzzer.o

elf_fuzzer.o:	elf_fuzzer.c
	$(CC) $(CFLAGS) -c elf_fuzzer.c

clean:	
	rm -f *~ *.o elf_fuzzer test.elf
