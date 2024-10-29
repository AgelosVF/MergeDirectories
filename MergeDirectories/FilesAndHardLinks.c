#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "cmpcats.h"

void copyFile(const char* sourcePath, const char* destinationPath) {

	//open the file to read
	FILE* sourceFile = fopen(sourcePath, "rb");
	if (sourceFile == NULL) {
		perror("Error opening source file");
		exit(-1);
	}

	//fopen creates the file since it doesnt exist
	FILE* destinationFile = fopen(destinationPath, "wb");
	if (destinationFile == NULL) {
		perror("Error opening destination file");
		fclose(sourceFile);
		exit(-1);
	}

	//used to copy the permisions
	struct stat sourceStat;
	if (stat(sourcePath, &sourceStat) != 0) {
		perror("Error getting source file stat");
		fclose(sourceFile);
		fclose(destinationFile);
		exit(-1);
	}

	char buffer[1024];
	size_t bytesRead;
	while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) {
		fwrite(buffer, 1, bytesRead, destinationFile);
	}

	fclose(sourceFile);
	fclose(destinationFile);

	//change the permisions
	if (chmod(destinationPath, sourceStat.st_mode) != 0) {
		perror("Error setting destination file permissions");
	}
}

int copyOrHardLink(unsigned long int inode,hash_table *hashTable,char *createNewFile,char *FileName){

    file_struct *filePTR=hashtable_search(hashTable, inode);
    if(filePTR==NULL){
            //first time to copy the file
            copyFile(FileName, createNewFile);
            filePTR=malloc(sizeof(file_struct));
            filePTR->original_inode=inode;
            filePTR->path=createNewFile;
            hashtable_insert(hashTable, filePTR);
        return 1;
    }
    else{
            //have already made a copy of the file so we create a hard link between them
            if(link(filePTR->path,createNewFile)==0){
            }
            else {
                    perror("Failed to create hard link: ");
            }
            //free the name since we dont have to save it to the hashtable
            free(createNewFile);
        return 2;
    }


}
