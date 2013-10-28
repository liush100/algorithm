/*
	Student Name: Shuhao Liu. Assignment2
	gcc -Wall -pedantic-errors rw.c -o rw -lpthread
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
int main(int argc, char *argv[]){
	char *buffer = NULL;
	FILE *infile = NULL;
	FILE *outfile = NULL;
	size_t len = 0;
	ssize_t read;

	if(argc !=3){
		printf("Sorry, you need one input file, one output file for this program\n");
		exit(EXIT_FAILURE);
	}

	if((infile = fopen(argv[1],"r")) == NULL){
		printf("Error in opening file %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	if((outfile = fopen(argv[2],"w")) == NULL){
		printf("Error in opening file %s\n", argv[2]);
		exit(EXIT_FAILURE);
	}
	while((read = getline(&buffer, &len, infile))!=-1){
		fwrite(buffer, 1, strlen(buffer), outfile);
		if(ferror(outfile)){
			printf("error occurred when write to file");
			fclose(infile);
			fclose(outfile);
			exit(EXIT_FAILURE);
		}
	}

	fclose(infile);
	fclose(outfile);
	free(buffer);
	printf("Finished\n");
	exit(EXIT_SUCCESS);
}

