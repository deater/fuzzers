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

static int randomize_shebang(int fd) {

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
	int wstatus;
	int success=0,fail=0;
	int result;
	int run=0;
	int fail_type[MAX_FAILS],i;
	int random_seed=0,seed_specified=0;
	struct utsname uname_info;
	int type=0;

	char *newargv[] = { NULL, "hello", "world", NULL };
	char *newenviron[] = { NULL };

	/* Init variables */
	for(i=0;i<MAX_FAILS;i++) fail_type[i]=0;

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

		}
		run++;
		if (run%2000==0) {

			printf("After %d: %d success, %d fail\n",
				run,success,fail);
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
