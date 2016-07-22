#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/utsname.h>
#include <sys/wait.h>


#include "random_file.h"
#include "version.h"

/* /usr/include/asm-generic/errno-base.h */
static int print_error_name(int which) {

	switch(which) {
		case 2:		printf("ENOENT"); break;
		case 8:		printf("ENOXEC"); break;
		case 13:	printf("EACCES"); break;
		case 20:	printf("ENOTDIR"); break;
		default:	printf("UNKNOWN(%d)",which);
				break;
	}
	return 0;

}

static int randomize_magic(int fd) {

#define MAGIC_SIZE 8192

	char magic[MAGIC_SIZE];
	int which,i;

	which=rand()%32;

	/* No magic at all */
	if (which==0) {
		return 0;
	}
	/* Completely random magic */
	else if (which==1) {
		for(i=0;i<MAGIC_SIZE;i++) {
			magic[i]=rand();
		}
		write(fd,magic,rand()%MAGIC_SIZE);
		return 0;
	/* Size 4 random magic */
	} else if (which==2) {
		for(i=0;i<4;i++) {
			magic[i]=rand();
		}
		write(fd,magic,4);
		return 0;
	}
	/* #! magic */
	else if (which==3) {
		for(i=0;i<MAGIC_SIZE;i++) {
			magic[i]=rand();
		}
		magic[0]='#';
		magic[1]='!';
		write(fd,magic,rand()%MAGIC_SIZE);
		return 0;
	}
	else {
		magic[0]=0x7f;
		magic[1]='E';
		magic[2]='L';
		magic[3]='F';
		write(fd,magic,4);
	}
	return 0;
}


static int randomize_elf(int fd) {

	/* Randomize magic */
	randomize_magic(fd);

	return 0;
}

#define WHITESPACE_SIZE	5000
static char whitespace[WHITESPACE_SIZE];

static int insert_whitespace(int fd) {

	int length=0,i;

	whitespace[0]=0;

	switch(rand()%4) {
		case 0:	length=0;	break;
		case 1:	length=1;	break;
		case 2:	length=rand()%16;	break;
		case 3:	length=rand()%WHITESPACE_SIZE;	break;
		default:	length=0;	break;

	}

	for(i=0;i<length;i++) {
		switch(rand()%5) {
			case 0:	whitespace[i]=' '; break;
			case 1: whitespace[i]='\t'; break;
			case 2: whitespace[i]='\r'; break;
			case 3: whitespace[i]='\n'; break;
			default: whitespace[i]=' '; break;
		}
	}
	whitespace[i]=0;

	write(fd,whitespace,strlen(whitespace));

	return 0;

}

int string_corrupt(char *string) {

	int number=0,i,which,len;

	len=strlen(string);

	if (len==0) return 0;

	switch(rand()%4) {
		case 0:	number=1; break;
		case 1: number=rand()%4; break;
		case 2: number=rand()%len; break;
		default:	number=0; break;
	}

	for(i=0;i<number;i++) {
		which=rand()%len;
		switch(rand()%4) {
			case 0:	string[which]|=(1<<rand()%8);
			case 1: string[which]&=~(1<<rand()%8);
			case 2:	string[which]=rand();
			default:	break;
		}
	}

	return 0;
}


#define FILENAME_SIZE 20000
static char filename[FILENAME_SIZE];

static int randomize_shebang(int fd) {

	int num_args,i,rand_length;

	write(fd,"#",1);

	if (rand()%2) insert_whitespace(fd);

	write(fd,"!",1);

	if (rand()%2) insert_whitespace(fd);

	switch(rand()%4) {
		case 0:	get_random_shell(filename, FILENAME_SIZE);
			break;
		case 1:	get_random_file(filename, FILENAME_SIZE, RANDOM_FILE_RANDOM);
			break;
		case 2:	get_random_file(filename, FILENAME_SIZE, RANDOM_FILE_SYSTEM);
			break;
		case 3:	get_random_file(filename, FILENAME_SIZE, RANDOM_FILE_EXECUTABLE);
			break;
	}

	/* Possibly corrupt filename */
	if (rand()%10==1) string_corrupt(filename);

	/* Write out filename */
	write(fd,filename,strlen(filename));

	if (rand()%2) insert_whitespace(fd);

	/* random number of command line args */
	switch(rand()%3) {
		case 0:	num_args=0; break;
		case 1:	num_args=rand()%4; break;
		case 2: num_args=rand()%20000; break;
		default:	num_args=0; break;
	}

	for(i=0;i<num_args;i++) {
		get_random_file(filename, FILENAME_SIZE, RANDOM_FILE_RANDOM);
		write(fd,filename,strlen(filename));

		insert_whitespace(fd);
	}

	/* write random data */
	if (rand()%2) {
		rand_length=rand()%FILENAME_SIZE;
		for(i=0;i<rand_length;i++) {
			filename[i]=rand();
		}
		write(fd,filename,rand_length);
	}

	return 0;
}


static void usage(char *name,int help) {

	printf("\nELF Fuzzer version %s\n\n",VERSION);

	if (help) {
		printf("%s [-h] [-v] [-r num]\n\n",name);
		printf("\t-h\tdisplay help\n");
		printf("\t-v\tdisplay version\n");
		printf("\t-r num\tseed random number generator with num\n");
		printf("\n");
	}
}


#define MAX_FAILS 255

int main(int argc, char **argv) {

	int fd;
	pid_t child;
	//int wstatus;
	int success=0,fail=0,hung=0;
	int result;
	int run=0;
	int fail_type[MAX_FAILS],i;
	int random_seed=0,seed_specified=0;
	struct utsname uname_info;
	int type=0;

	struct timespec wait_timeout;
	sigset_t wait_set,block_set;
	siginfo_t wait_info;
	int signal_result;
	int timeout=0;

	char *newargv[] = { NULL, "hello", "world", NULL };
	char *newenviron[] = { NULL };

	/* Init variables */
	for(i=0;i<MAX_FAILS;i++) fail_type[i]=0;
	wait_timeout.tv_sec=5;
	wait_timeout.tv_nsec=0;

	sigemptyset(&block_set);
	sigaddset(&block_set, SIGCHLD);
	sigprocmask(SIG_BLOCK, &block_set, NULL);

	sigemptyset(&wait_set);
	sigaddset(&wait_set, SIGCHLD);
	sigprocmask(SIG_BLOCK, &block_set, NULL);


	/* Parse command line args */
	if (argc>1) {
		i=1;
		while(1) {
			if (i>=argc) break;

			if(argv[i][0]=='-') {
				switch(argv[i][1]) {
				/* help */
				case 'h':	usage(argv[0],1);
						exit(0);
						break;
				/* seed */
				case 'r':	if (i+1<argc) {
							random_seed=atoi(argv[i+1]);
						}
						seed_specified=1;
						printf("Using user-specified random seed of %d\n",random_seed);
						i+=2;
						break;
				/* version */
				case 'v':	usage(argv[0],0);
						exit(0);
						break;

				default:	fprintf(stderr,"Unknown parameter %s\n",argv[1]);
						usage(argv[0],1);
						exit(1);
						break;
				}

			}
			else {
				fprintf(stderr,"Unknown parameter %s\n",argv[1]);
				usage(argv[0],1);
				exit(1);
			}

		}

	} else {
		/* Use defaults */
	}


	/* Randomize timer */
	if (!seed_specified) {
		random_seed=time(NULL);
	}
	srand(random_seed);

	/* Print banner */
	printf("\n*** elf_fuzzer %s *** by Vince Weaver\n\n",VERSION);
	uname(&uname_info);

	printf("\t%s version %s %s\n",
		uname_info.sysname,uname_info.release,uname_info.machine);
	printf("\tRandom seed=%d\n",random_seed);

	while(1) {

		fd=open("test.elf",O_CREAT|O_WRONLY,S_IRWXU);
		if (fd<0) {
			fprintf(stderr,"Error creating!\n");
			return -1;
		}

		type=rand()%2;

		if (type==0) {
			randomize_elf(fd);
		}
		else if (type==1) {
			randomize_shebang(fd);
		}

		/* Done, close the file */
		close(fd);


		/*************************************/
		/* Executable created, let's run it  */
		/*************************************/

		child=fork();
		/* error */
		if (child==-1) {
			fprintf(stderr,"Error forking!\n");
			return -1;
		}
		/* in the child */
		if (child==0) {
			result=execve("test.elf",newargv,newenviron);
			return errno;
		}
		else {

			signal_result=sigtimedwait(&wait_set,
					&wait_info,
					&wait_timeout);

			if (signal_result == -1) {
				if (errno==EAGAIN) {
					timeout=1;
				}
				printf("ERROR! sigtimedwait: %s\n", strerror(errno));
				exit(1);
			} else {
//				printf("received signal %i from %i with status %i\n", 
//					signal_result, wait_info.si_pid, wait_info.si_status);
			}

			if (timeout) {
				timeout=0;
				printf("Timeout!\n");
				kill(child,SIGKILL);
				hung++;
			}
			else {
				result=wait_info.si_status;
				if (result==0) {
					success++;
				}
				else {
					if (result<255) fail_type[result]++;
					else {
						printf("Error! Result too big!\n");
						}
					fail++;
				}
			}
#if 0
			waitpid(child,&wstatus,0);

			result=WEXITSTATUS(wstatus);
			if (WIFEXITED(wstatus) && (result==0)) {
				success++;
			}
			else {
//				printf("%s\n",strerror(result));
				fail_type[result]++;
				fail++;
			}
#endif

		}
		run++;
		if (run%50==0) {

			printf("After %d: %d success, %d fail, %d hung\n",
				run,success,fail,hung);
			for(i=0;i<MAX_FAILS;i++) {
				if (fail_type[i]!=0) {
					printf("\t");
					print_error_name(i);
					printf(":\t%d (%s)\n",
						fail_type[i],strerror(i));
				}
				fail_type[i]=0;
			}
		}
	}


	return 0;
}
