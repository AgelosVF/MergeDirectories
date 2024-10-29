#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include "cmpcats.h"


void get_arguments3(char **argv,int *firstDirIndex, int *secondDirIndex,int * newDirIndex);
void get_arguments2(char **argv,int *firstDirIndex, int *secondDirIndex);

int main(int argc, char *argv[])
{
	int firstDirIndex,secondDirIndex,newDirIndex;
	if(argc==4){
		get_arguments2(argv, &firstDirIndex, &secondDirIndex);
	}
	else if(argc==6){
		get_arguments3(argv, &firstDirIndex, &secondDirIndex, &newDirIndex);
	}
	else{
		printf("Expected call is \"-d <DirName1> <DirName2>\" or\"%s -d <DirName1> <DirName2> -s <newDir>\".\nThe program prints the files in each directory that arent in the other directory and if the -s flag is used it merges both directories in a new directory with name newDir",argv[0]);
		exit(0);
	}

	char* path1=realpath(argv[firstDirIndex], NULL);
	if(path1==NULL){
		perror("Couldnt find filename1\n");
		exit(-1);
	}
	char* path2=realpath(argv[secondDirIndex], NULL);
	if(path2==NULL){
		perror("Couldnt find filename2\n");
		exit(-1);
	}

	if (argc==4){
		//if given -d DirName DirName just print the diffrences without making a new directory
		print_dif(path1,path2);

	}
	if(argc==6){
		//create the new directory and call merge_dir to fill it and print the diffrences.
		if(mkdir(argv[newDirIndex],0777)!=0){
			perror("Failed to make the first new directory: ");
			free(path1);
			free(path2);
			exit(-1);
		}
		char * newPath=realpath(argv[newDirIndex], NULL);
		if (newPath==NULL){
			perror("Couldnt create the full path for the new directory\n");
			exit(-1);
		}
		merge_dir(path1, path2, newPath);
		free(newPath);
	}
	free(path1);
	free(path2);

	return 1;
}

void get_arguments2(char **argv,int *firstDirIndex, int *secondDirIndex){
	if((strcmp("-d", argv[1])!=0)){
		printf("Invalid flag\n");
		exit(0);
	}
	//since we have 3 arguments and the flag is before the directory names the only way it can be arranged is "-d directory1 directory2"
	*firstDirIndex=2;
	*secondDirIndex=3;
}


void get_arguments3(char **argv,int *firstDirIndex, int *secondDirIndex,int * newDirIndex){

	//if -d dirName1 dirName2 -s newDir
	if((strcmp("-d", argv[1])==0)){
		*firstDirIndex=2;
		*secondDirIndex=3;
		if( (strcmp("-s", argv[4]))==0){
			*newDirIndex=5;
		}
		else{
			printf("Error invalid flag was expecting -s.\n");
			exit(-1);
		}
	}
	else if((strcmp("-s",argv[1]))==0)//else if -s newDir -d dirName1 dirName2
	{
		*newDirIndex=2;
		if( (strcmp("-d", argv[3]))==0){
			*firstDirIndex=4;
			*secondDirIndex=5;
			}
		else{
			printf("Error invalid flag was expecting -d.\n");
			exit(-1);
		}
	}
	else{
		printf("Error invalid arguments.\n");
		exit(-1);
	}


}














