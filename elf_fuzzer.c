#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/wait.h>


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

#define MAX_FAILS 255

int main(int argc, char **argv) {

	int fd;
	pid_t child;
	int wstatus;
	int success=0,fail=0;
	int result;
	int run=0;
	int fail_type[MAX_FAILS],i;

	char *newargv[] = { NULL, "hello", "world", NULL };
	char *newenviron[] = { NULL };

	for(i=0;i<MAX_FAILS;i++) fail_type[i]=0;

	while(1) {

		fd=open("test.elf",O_CREAT|O_WRONLY,S_IRWXU);
		if (fd<0) {
			fprintf(stderr,"Error creating!\n");
			return -1;
		}

		/* Randomize magic */
		randomize_magic(fd);



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
		if (run>=1024) break;
	}
	printf("%d success, %d fail\n",success,fail);
	for(i=0;i<MAX_FAILS;i++) {
		if (fail_type[i]!=0) {
			print_error_name(i);
			printf(":\t%d (%s)\n",
				fail_type[i],strerror(i));
		}
	}
	return 0;
}
