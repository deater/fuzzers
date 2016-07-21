#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/types.h>

#define NUM_ROOTS 7
char random_roots[NUM_ROOTS][10]={
	"/",
	"//",
	"///",
	".",
	"..",
	"../..",
	"../../.."
};

#define MAX_DEPTH 16

int get_file_random(char *string, int size) {

	int root_num,depth,current_depth=0,which_one;
	int result,num_entries;
	struct stat stat_buf;
	DIR *directory;
	struct dirent *entry;

	root_num=rand()%NUM_ROOTS;
	depth=rand()%MAX_DEPTH;

	strncpy(string,random_roots[root_num],size);

	while(1) {
		result=stat(string,&stat_buf);
		if (result<0) {
                	//printf("Cannot access %s: no such file or directory!\n",path);
	                return -1;
        	}

		/* handle if it's not a directory */
		if ( (stat_buf.st_mode&S_IFMT)!=S_IFDIR) {
			return 0;
		}

		directory=opendir(string);
		if (directory==NULL) {
			printf("Error! %s\n",strerror(errno));
			return 0;
		}

		num_entries=0;
		while(1) {
			entry=readdir(directory);
			if (entry==NULL) break;
			num_entries++;
		}
		//printf("Found %d entries\n",num_entries);

		rewinddir(directory);

		which_one=rand()%num_entries;
		num_entries=0;
		while(1) {
			entry=readdir(directory);
			if (num_entries==which_one) {
				strncat(string,"/",size);
				strncat(string,entry->d_name,size);
				break;
			}
			num_entries++;
		}

		closedir(directory);

		current_depth++;
		if (current_depth>depth) break;
	}

	return 0;
}

char filename[BUFSIZ];

int main(int argc, char **argv) {

	int i;

	/* proc, sys, dev, /bin,/usr/bin,/usr/sbin, completely random */

	srand(time(NULL));

	for(i=0;i<10;i++) {
		get_file_random(filename, BUFSIZ);

		printf("Found: %s\n",filename);
	}

	return 0;
}
