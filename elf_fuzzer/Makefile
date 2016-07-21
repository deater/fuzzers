CC = gcc
CFLAGS = -O2 -Wall
LFLAGS =

all:	elf_fuzzer random_file

elf_fuzzer:	elf_fuzzer.o
	$(CC) $(LFLAGS) -o elf_fuzzer elf_fuzzer.o

elf_fuzzer.o:	elf_fuzzer.c
	$(CC) $(CFLAGS) -c elf_fuzzer.c

random_file:	random_file.o
	$(CC) $(LFLAGS) -o random_file random_file.o

random_file.o:	random_file.c
	$(CC) $(CFLAGS) -c random_file.c

clean:	
	rm -f *~ *.o elf_fuzzer random_file test.elf
