// This is a file for using file IO written in C
#include <stdio.h>
#include <string.h>

#define MAX_STRING_LENGTH 100

char *assign_string(char *src) {
	size_t src_len = strlen(src);
	char *dest = malloc(src_len + 1);
	strcpy(dest, src);
	
	return dest;
}

int main(int argc, char* argv[]) {
	const int num_args = 2;
	if (argc != (num_args+1) ) {
		printf("ERROR! Must have two arguements.");
		return 1;
	}

	char *writefile = assign_string(argv[1]);
	char *writestr = assign_string(argv[2]);

	// OPen the file 
	FILE *file = fopen(writefile, "w");
	if (file == NULL) {
		printf("Failed to open file %s for writing %s.\n", writefile, writestr);
		return 1;
	}
	// Write the string to the file
	fprintf(file, "%s", writestr);

	// Close the file
	fclose(file);	
	printf("Done writing to file: %s, with content %s.\n", writefile, writestr);	
	return 0;
}
