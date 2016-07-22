int get_random_shell(char *string, int size);

#define RANDOM_FILE_RANDOM	1
#define RANDOM_FILE_SYSTEM	2
#define RANDOM_FILE_EXECUTABLE	3

int get_random_file(char *string, int size, int type);
