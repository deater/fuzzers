#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/wait.h>

int randomize_magic(int fd) {

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

int main(int argc, char **argv) {

	int fd;
	pid_t child;
	int wstatus;
	int success=0,fail=0;
	int result;
	int run=0;

	char *newargv[] = { NULL, "hello", "world", NULL };
	char *newenviron[] = { NULL };


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
				printf("%s\n",strerror(result));
				fail++;
			}

		}
		run++;
		if (run>10) break;
	}
	printf("%d success, %d fail\n",success,fail);

	return 0;
}
