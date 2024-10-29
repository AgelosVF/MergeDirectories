#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "cmpcats.h"


void initializeQueue(Queue* q) {
	q->front = NULL;
	q->rear = NULL;
}

//puts object in the queue. Used only in createSymLink
void enqueue(Queue* q, const char* symlinkPath, const char* targetFileName) {
	//malloc space for the QueueItem and the strings in it
	QueueItem* newItem = (QueueItem*)malloc(sizeof(QueueItem));
	char *newLinkPath=(char*)malloc(strlen(symlinkPath)+1);
	sprintf(newLinkPath,"%s", symlinkPath);
	char *newLinkTarget=(char*)malloc(strlen(targetFileName)+1);
	sprintf(newLinkTarget,"%s", targetFileName);
	//make the pointers in the item to point to the strings
	newItem->symlinkPath = newLinkPath;
	newItem->targetFileName = newLinkTarget;
	newItem->next = NULL;

	//update the queue struct pointers
	if (q->rear == NULL) {
		q->front = newItem;
		q->rear = newItem;
	} else {
		q->rear->next = newItem;
		q->rear = newItem;
	}
}

//creates the link or if the file it points to hasnt been created yet it puts its information to a queue in order to be created later
void createSymlinkOrEnqueue(Queue* q, const char* symlinkPath, const char* targetFileName){
	struct stat fileStat;
	//REMOVE: printf("Got symlink:%s filename:%s\n",symlinkPath,targetFileName);
	
	int exists=fileExists(targetFileName);
	if (exists==1) {
		//the file we want the link to point to exists
		lstat(targetFileName, &fileStat);
		switch (fileStat.st_mode & S_IFMT) {
			case S_IFDIR:
				if (symlink(targetFileName, symlinkPath) != 0) {
					perror("Error creating symlink");
					exit(-1);
				}
				else {
					//REMOVE: printf("Symlink created successfully\n");
				}
				break;
			case S_IFREG:
				if (symlink(targetFileName, symlinkPath) != 0) {
					perror("Error creating symlink");
					exit(-1);
				}
				else {

					//REMOVE: printf("Symlink created successfully\n");
				}
				break;
			default:
				printf("%s points to a file that isnt a directory or link will skip it\n",symlinkPath);
				return;
		}
	}
	else {
		//havent created the file yet so put the link in a queue in order to create it after all the directories and files have been created
		if (errno == ENOENT) {
			enqueue(q, symlinkPath, targetFileName);
		} 
		else {
			perror("Error checking file existence");
		}
	}
}

//empty the queue creating each link that its file exists and skiping the ones whoses files dont exist
void processQueue(Queue* q) {
	struct stat info;
	int exists;
	while (q->front != NULL) {
		QueueItem* current = q->front;
		q->front = q->front->next;
		//REMOVE: printf("proccess queue: %s %s\n",current->targetFileName ,current->symlinkPath);
		
		exists=fileExists(current->targetFileName);
		if(exists==1){

			lstat(current->targetFileName, &info);
			if (symlink(current->targetFileName, current->symlinkPath) != 0) {
				perror("Error creating symlink");
			}
			else {
				//REMOVE: printf("Symlink created successfully for %s\n", current->targetFileName);
			}
		} 
		else {
			printf("File %s does not exist\n", current->targetFileName);
		}

		free(current->symlinkPath);
		free(current->targetFileName);
		free(current);
	}
	q->rear = NULL;
}

//used for debuging. processQueue also frees the list.
void freeQueue(Queue* q) {
    QueueItem* current = q->front;

    while (current != NULL) {
	
        QueueItem* next = current->next;
        free(current->symlinkPath);
        free(current->targetFileName);
        free(current);
        current = next;
    }
    q->front = NULL;
    q->rear = NULL;
}

