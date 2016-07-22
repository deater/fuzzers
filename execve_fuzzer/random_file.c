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

#include <bsd/bsd.h>

#include "random_file.h"

#define NUM_SHELLS 12
static char shell_names[NUM_SHELLS][20]={
	"sh",
	"bash",
	"dash",
	"zsh",
	"csh",
	"tcsh",
	"ksh",
	"pdksh",
	"python",
	"python2",
	"perl",
	"perl6",
};

#define NUM_EXE_PREFIXES 9
static char exe_prefixes[NUM_EXE_PREFIXES][20]={
	"/bin/",
	"/sbin/",
	"/usr/bin/",
	"/usr/sbin/",
	"/usr/local/bin/",
	"/usr/local/sbin/",
	"/opt/bin/",
	"/",
	"./",
};

int get_random_shell(char *string, int size) {

	int which_prefix,which_shell;

	which_prefix=rand()%NUM_EXE_PREFIXES;

	strlcpy(string,exe_prefixes[which_prefix],size);


	which_shell=rand()%NUM_SHELLS;

	strlcat(string,shell_names[which_shell],size);

	return 0;

}

#define NUM_ROOTS 7
static char random_roots[NUM_ROOTS][10]={
	"/",
	"//",
	"///",
	".",
	"..",
	"../..",
	"../../.."
};

#define NUM_SYSTEM_ROOTS 5
static char system_roots[NUM_SYSTEM_ROOTS][10]={
	"/proc/",
	"/dev/",
	"/sys/",
	"/debug/",
	"/",
};

#define MAX_DEPTH 16


int get_random_file(char *string, int size, int type) {

	int root_num,depth,current_depth=0,which_one;
	int result,num_entries;
	struct stat stat_buf;
	DIR *directory;
	struct dirent *entry;

	if (type==RANDOM_FILE_SYSTEM) {
		root_num=rand()%NUM_SYSTEM_ROOTS;
		strlcpy(string,system_roots[root_num],size);
	}
	if (type==RANDOM_FILE_EXECUTABLE) {
		root_num=rand()%NUM_EXE_PREFIXES;
		strlcpy(string,exe_prefixes[root_num],size);
	}
	if (type==RANDOM_FILE_RANDOM) {
		root_num=rand()%NUM_ROOTS;
		strlcpy(string,random_roots[root_num],size);
	}



	depth=rand()%MAX_DEPTH;

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
			//printf("Error! %s\n",strerror(errno));
			return 0;
		}

		num_entries=0;
		while(1) {
			entry=readdir(directory);
			if (entry==NULL) break;
			num_entries++;
		}
		//printf("Found %d entries\n",num_entries);

		if (num_entries==0) return 0;

		rewinddir(directory);

		which_one=rand()%num_entries;
		num_entries=0;
		while(1) {
			entry=readdir(directory);
			if (num_entries==which_one) {
				strlcat(string,"/",size);
				strlcat(string,entry->d_name,size);
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

#if 0


static char filename[BUFSIZ];

int main(int argc, char **argv) {

	int i;

	/* proc, sys, dev, /bin,/usr/bin,/usr/sbin, completely random */

	srand(time(NULL));

	for(i=0;i<10;i++) {
		get_random_file(filename, BUFSIZ, RANDOM_FILE_EXECUTABLE);

		printf("Found: %s\n",filename);
	}

	return 0;
}

#endif
