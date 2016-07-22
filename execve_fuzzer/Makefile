CC = gcc
CFLAGS = -O2 -Wall -g
LFLAGS = -lbsd

all:	execve_fuzzer

execve_fuzzer:	execve_fuzzer.o random_file.o
	$(CC) $(LFLAGS) -o execve_fuzzer execve_fuzzer.o random_file.o

execve_fuzzer.o:	execve_fuzzer.c
	$(CC) $(CFLAGS) -c execve_fuzzer.c

random_file.o:	random_file.c
	$(CC) $(CFLAGS) -c random_file.c

clean:	
	rm -f *~ *.o execve_fuzzer test.elf
