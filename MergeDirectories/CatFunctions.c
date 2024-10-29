#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include "cmpcats.h"
#include <time.h>
#include <errno.h>

int fileExists(const char* path) {
    struct stat buffer;
    int result = lstat(path, &buffer);
    if (result == 0) {
        return 1; // File exists since lstat succeded
    } else {
        if (errno == ENOENT) {
            return -1; // File does not exist
        } else {
            perror("Error HERE checking file existence");
            exit(-1); // Unexpected error occurred
        }
    }
}


//compare firstDir and secondDir and create the files they have at common (common==1) or not (common==-1) at the location of newDir

int compareAndCreate(const char *firstDir,const char *firstDirStart,const char *secondDir,const char *secondDirStart,const char *newDir,Queue *linkQueue,hash_table *hashTable,int create,int common){

	DIR *firstDirPTR=opendir(firstDir);
	if(firstDirPTR==NULL){
		perror("Couldn' opent the directory\n");
		exit(-1);
	}

	struct dirent *firstEntry;
	struct stat firstInfo;
	struct stat secondInfo;
	char *createNewDir;
	//flags used to check if we should create files and if we should create the files that the 2 directories have in common
	int createFlag;
	int commonFlag;
	
        time_t now = time(NULL);
	double firstLastMod,secondLastMod;


	//now that we opened the first catalog we read every file in it 1 by 1
	while ( (firstEntry=readdir(firstDirPTR))!=NULL) {
		createFlag=create;
		commonFlag=common;
		//we skip . and ..
		if ( (strcmp(firstEntry->d_name,".")==0 ) || (strcmp(firstEntry->d_name, "..")==0 ) ) {
			continue;
		}

		//create the absolute paths of the file in the directory be appending its name
		//to the directory it is in 
		char *firstFileName=malloc(strlen(firstDir)+strlen(firstEntry->d_name)+ 2); // 1 for '/' and 1 for '\0'
		if (firstFileName==NULL) {
			perror(("Failed to allocate space for absolute path\n"));
			exit(-1);
		}
		sprintf(firstFileName,"%s/%s",firstDir,firstEntry->d_name);

		//do the same for the second directory
		char *secondFileName=malloc(strlen(secondDir)+strlen(firstEntry->d_name)+ 2); // 1 for '/' and 1 for '\0'
		if (secondFileName==NULL) {
			perror(("Failed to allocate space for absolute path\n"));
			exit(-1);
		}
		sprintf(secondFileName,"%s/%s",secondDir,firstEntry->d_name);

		//use lstat to find what type of file firstFileName is 
		if(lstat(firstFileName, &firstInfo) !=0){
			perror("Failed to get file status\n");
			exit(-1);
		}

		//now we have 4 cases file,directory,link,other
		switch(firstInfo.st_mode & S_IFMT){
			case S_IFDIR: //Directory
				if(same_direct(firstFileName,secondFileName)==-1){
					printf("Directory: %s/%s\n",remove_substring(firstDir,firstDirStart),firstEntry->d_name);
					if(createFlag==1){
						//2 cases in which we create the directory
						//case 1: there doesnt exist another file with the same name in the secondDir
						if( fileExists(secondFileName)==-1){
							createNewDir=change_starting_path(firstFileName, firstDirStart, newDir);

							mkdir(createNewDir,firstInfo.st_mode);
							free(createNewDir);
						}
						else{
							//case 2: another file exists with the same name but the one from the current directory was modified more recently
							if(lstat(secondFileName, &secondInfo)!=0){
								perror("Error lstat case 2 failed\n");
								exit(-1);
							}
							firstLastMod=difftime(now, firstInfo.st_mtime);
							secondLastMod=difftime(now, secondInfo.st_mtime);

							if(firstLastMod<secondLastMod){
								createNewDir=change_starting_path(firstFileName, firstDirStart, newDir);
								mkdir(createNewDir, firstInfo.st_mode);
								free(createNewDir);
							}
							else{
								//since we didnt create the directory we wont create any of it's childs so set the create and common flags to 0
								//to avoid trying to create files in the recursive calls
								createFlag=0;
								commonFlag=0;
							}
						}

					}


				}
				else if(commonFlag==1 && createFlag==1){
					//create the new directory
					createNewDir=change_starting_path(firstFileName, firstDirStart, newDir);
					mkdir(createNewDir, firstInfo.st_mode);
					free(createNewDir);
				}

				//recursively check every file in that directory

				 compareAndCreate(firstFileName,firstDirStart,secondFileName,secondDirStart,newDir,linkQueue,hashTable,createFlag,commonFlag);
				break;
			case S_IFREG: //REGULAR FILE
				if(same_file(firstFileName,secondFileName)==-1){
					printf("Regular file: %s/%s\n",remove_substring(firstDir,firstDirStart),firstEntry->d_name);
					if(createFlag==1){
						//same as in directories we create the unique file in 2 cases 
						//case 1: there doesnt exist another file with the same name
						if(fileExists(secondFileName)==-1){
							
							char *createNewFile=change_starting_path(firstFileName, firstDirStart, newDir);
							unsigned long int inode=firstInfo.st_ino;
							copyOrHardLink(inode,hashTable,createNewFile,firstFileName);

						}
						else{
							//case 2: another file exists with the same name but the one from the current directory was modified more recently
							if(lstat(secondFileName, &secondInfo)!=0){
								perror("Error lstat case 2 failed\n");
								exit(-1);
							}
							firstLastMod=difftime(now, firstInfo.st_mtime);
							secondLastMod=difftime(now, secondInfo.st_mtime);

							if(firstLastMod<secondLastMod){
								//since this was the most recent file we create it
								char *createNewFile=change_starting_path(firstFileName, firstDirStart, newDir);
								
								unsigned long int inode=firstInfo.st_ino;
								copyOrHardLink(inode,hashTable,createNewFile,firstFileName);

							}
						}
					}
				}
				else if((createFlag==1)&&(commonFlag==1)){
					//create the file that both directories have in common
					char *createNewFile=change_starting_path(firstFileName, firstDirStart, newDir);

					unsigned long int inode=firstInfo.st_ino;
					copyOrHardLink(inode,hashTable,createNewFile,firstFileName);
					//we also need to add the info of the file in the secondDir to the hashtable (if it doesnt exist) to avoid dublicates
					if(lstat(secondFileName, &secondInfo)!=0){
						perror("Error lstat case 2 failed\n");
						exit(-1);
					}
					unsigned long int secondInode=secondInfo.st_ino;

					file_struct *filePTR=hashtable_search(hashTable, secondInode);
					if(filePTR==NULL){
						//we malloc a second time the same name to avoid double free when we destroy the table
						char *secondCopyNewFile=change_starting_path(firstFileName, firstDirStart, newDir);
						filePTR=malloc(sizeof(file_struct));
						filePTR->original_inode=secondInode;
						filePTR->path=secondCopyNewFile;

						hashtable_insert(hashTable, filePTR);
					}
				}
				
				break;
			case S_IFLNK: //SYM LINK
				if(same_link(firstFileName, firstDirStart, secondFileName, secondDirStart)==-1){

					printf("Link: %s/%s\n",remove_substring(firstDir,firstDirStart),firstEntry->d_name);
					if(createFlag==1){
						//same as in directories we create the unique file in 2 cases 
						//case 1: there doesnt exist another file with the same name
						if(fileExists(secondFileName)==-1){
							
							//call createSymlinkOrEnqueue to try to create the link now or later
							//first create the name of the link and its target in the new file

							char *createNewLink=change_starting_path(firstFileName, firstDirStart, newDir);
							char *oldTarget=realpath(firstFileName, NULL);
							char *newTarget=change_starting_path(oldTarget, firstDirStart, newDir);

							createSymlinkOrEnqueue(linkQueue, createNewLink, newTarget);
							free(createNewLink);
							free(oldTarget);
							free(newTarget);

						}
						else{
							//case 2: another file exists with the same name but the one from the current directory was modified more recently
							if(lstat(secondFileName, &secondInfo)!=0){
								perror("Error lstat case 2 failed\n");
								exit(-1);
							}
							firstLastMod=difftime(now, firstInfo.st_mtime);
							secondLastMod=difftime(now, secondInfo.st_mtime);

							if(firstLastMod<secondLastMod){

								char *createNewLink=change_starting_path(firstFileName, firstDirStart, newDir);
								char *oldTarget=realpath(firstFileName, NULL);
								char *newTarget=change_starting_path(oldTarget, firstDirStart, newDir);

								createSymlinkOrEnqueue(linkQueue, createNewLink, newTarget);
								free(createNewLink);
								free(oldTarget);
								free(newTarget);

							}
						}
					}
				}
				else if (createFlag==1 && commonFlag==1) {
					//call createSymlinkOrEnqueue to try to create the link now or later
					//first create the name of the link and its target in the new file

					char *createNewLink=change_starting_path(firstFileName, firstDirStart, newDir);
					char *oldTarget=realpath(firstFileName, NULL);
					char *newTarget=change_starting_path(oldTarget, firstDirStart, newDir);

					createSymlinkOrEnqueue(linkQueue, createNewLink, newTarget);
					free(createNewLink);
					free(oldTarget);
					free(newTarget);


				
				}
				break;
			default:
				printf("%s isnt a file,link or directory so i will ignore it\n",firstEntry->d_name);
				break;
		}
		free(firstFileName);
		free(secondFileName);

	
	}
	
	closedir(firstDirPTR);

return 1;
}
int same_file(const char* firstFile, const char *secondFile){
	struct stat firstInfo,secondInfo;
	
	//check same path and name
	//-------------------------------------------------------------------
	if( (lstat(secondFile, &secondInfo))!=0 ){
		//file with that path doesnt exist
		//printf("-Diffrent Name\n");
		return -1;
	}
	//--------------------------------------------------------------------

	//check same type of file (regular file)
	//--------------------------------------------------------------------
	if( (secondInfo.st_mode & S_IFMT) != S_IFREG ){
		//same name and path but diffrent type of file
		//printf("-Diffrent type\n");
		return -1;
	}
	//--------------------------------------------------------------------
	
	//check that they have the same size
	//--------------------------------------------------------------------
	if( (lstat(firstFile,&firstInfo)!=0) ){
		perror("Failed to get stat for file\n");
		exit(-1);
	}
	
	if( (firstInfo.st_size != secondInfo.st_size) ){
		//printf("Diffrent size\n");
		return -1;
	}
	//--------------------------------------------------------------------
	
	//compare data
	//--------------------------------------------------------------------
	FILE *firstFilePTR,*secondFilePTR;
	if( (firstFilePTR=fopen(firstFile, "r"))==NULL){
		perror("Couldnt open file to read\n");
		exit(-1);
	}
	if( (secondFilePTR=fopen(secondFile, "r"))==NULL){
		perror("Couldnt open file to read\n");
		fclose(firstFilePTR);
		exit(-1);
	}

	int char1,char2;//getc returns unsinged int that you can turn to char to display.
	//read bytes 1 by 1 and if they are diffrent return -1
	//same size->same number of characters
	while( (char1=getc(firstFilePTR))!=EOF && (char2=getc(secondFilePTR))!=EOF ){
		//printf("char1=%c char2=%c\n",char1,char2);
		if( char1!=char2){
			//difrent character->difrent data
			fclose(firstFilePTR);
			fclose(secondFilePTR);
			//printf("Difrent data\n");
			return -1;
		}
	}
	fclose(firstFilePTR);
	fclose(secondFilePTR);
	//--------------------------------------------------------------------
	//if we reached this point means the files have the same path,name,type,size and data
	
	return 1;
}
int same_direct(const char* firstDir, const char* secondDir){

	struct stat secondInfo;
	//check same path and name
	//-------------------------------------------------------------------
	if( (lstat(secondDir, &secondInfo))!=0 ){
		//file with that path doesnt exist
		return -1;
	}
	//--------------------------------------------------------------------

	//check same type of file (directory)
	//--------------------------------------------------------------------
	if( (secondInfo.st_mode & S_IFMT)!=S_IFDIR ){
		//same name and path but diffrent type of file
		return -1;
	}
	return 1;
}
int same_link(const char* firstLink,const char* firstPathStart, const char* secondLink, const char* secondPathStart){
	struct stat firstInfo,secondInfo;
	//check same path and name
	//-------------------------------------------------------------------
	if( (lstat(secondLink, &secondInfo))!=0 ){
		//file with that path doesnt exist
		return -1;
	}
	//--------------------------------------------------------------------

	//check same type of file (link)
	//--------------------------------------------------------------------
	if( (secondInfo.st_mode & S_IFMT)!=S_IFLNK ){
		//same name and path but diffrent type of file
		return -1;
	}

	//get the paths to the files that the links point to
			
	char* firstLinkTarget=realpath(firstLink, NULL);
	char* secondLinkTarget=realpath(secondLink, NULL);

	//create the shorten paths by removing the start up to the containts of the starting folder of the targets to check that they are the same.
	//For example if the firstPathStart is home/aggelos/Documents/Start and the firstLinkTarget is home/aggelos/Documents/Start/DirB/TargetFile we get /DirB/TargetFile
	const char* firstLinkTargetShort=remove_substring(firstLinkTarget, firstPathStart);
	if(firstLinkTargetShort==NULL){
		printf("Error link points outside of starting directory \n\tLink:%s\n\tTarget:%s\n",firstLink,firstLinkTargetShort);
		exit(-1);
	}
	const char* secondLinkTargetShort=remove_substring(secondLinkTarget, secondPathStart);
	if(secondLinkTargetShort==NULL){
		printf("Error link points outside of starting directory \n\tLink:%s\n\tTarget:%s\n",secondLink,secondLinkTargetShort);
		exit(-1);
	}

	if( ( strcmp(firstLinkTargetShort,secondLinkTargetShort) ) !=0 ){
		printf("DIFFRENT TARGETS %s %s\n",firstLinkTarget,secondLinkTarget);
		free(firstLinkTarget);
		free(secondLinkTarget);
		return -1;
	}

	//find what type of file the first link points to 
	if( stat(firstLinkTarget,&firstInfo)!=0){
		perror("Failed to get stat for file link points to\n");
		exit(-1);
	}
	
	//since both links seem the same call the same_file or same_dir to check if the file they point to is the same
	int ret;
	switch (firstInfo.st_mode & S_IFMT) {
		case S_IFDIR:
			ret=same_direct(firstLinkTarget, secondLinkTarget);
			free(firstLinkTarget);
			free(secondLinkTarget);
			return ret;
		case S_IFREG:
			ret=same_file(firstLinkTarget, secondLinkTarget);
			free(firstLinkTarget);
			free(secondLinkTarget);
			return ret;
		case S_IFLNK:
			printf("Shouldnt be here same_link gave me link in the switch\nog link:%s points to:%s\n",firstLink,firstLinkTarget);
			exit(-1);
		default:
			perror("Link points to unknown type of file \n");
			exit(-1);
	}
}




int merge_dir(const char *firstDir,const char *secondDir,const char *newDir){

	printf("Will print the files that are unique in each directory and merge them to one\n");
	Queue *LinkQueue=malloc(sizeof(Queue));
	initializeQueue(LinkQueue);
	//linear hash table with 2 buckets that hold 8 items without overflowing
	hash_table *HTable=hashtable_create(2, 4);
	//we scan and create the files/link/directories they have at common while saving the others to do later
	printf("Files that exist only in %s.\n------------------------------------------------------------------------\n",firstDir);
	compareAndCreate(firstDir,firstDir,secondDir,secondDir,newDir,LinkQueue,HTable,1,1);
	printf("\n------------------------------------------------------------------------\n");
	//we scan the second directory and create the files that it doesnt have in common with the first
	printf("Files that exist only in %s.\n------------------------------------------------------------------------\n",secondDir);
	compareAndCreate(secondDir,secondDir,firstDir,firstDir,newDir,LinkQueue,HTable,1,0);
	printf("\n------------------------------------------------------------------------\n");
	//hashtable_print(HTable);
	//Create all the links save in the queue that we couldnt create before because the targets werent created yet
	processQueue(LinkQueue);
	free(LinkQueue);
	hashtable_destroy(HTable);
	return 1;
}


int print_dif(const char *firstDir,const char *secondDir){
	
	printf("Will print the files that are unique to each directory\n");
	printf("Files that exist only in %s.\n------------------------------------------------------------------------\n",firstDir);
	compareAndCreate(firstDir,firstDir,secondDir,secondDir,NULL,NULL,NULL,0,0);
	printf("\n------------------------------------------------------------------------\n");
	//we scan the second directory and create the files that it doesnt have in common with the first
	printf("Files that exist only in %s.\n------------------------------------------------------------------------\n",secondDir);
	compareAndCreate(secondDir,secondDir,firstDir,firstDir,NULL,NULL,NULL,0,0);
	printf("\n------------------------------------------------------------------------\n");
	return 1;

}











