#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <sys/wait.h>

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

		fd=open("test.elf",O_CREAT,S_IRWXU);
		if (fd<0) {
			fprintf(stderr,"Error creating!\n");
			return -1;
		}

		close(fd);

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
