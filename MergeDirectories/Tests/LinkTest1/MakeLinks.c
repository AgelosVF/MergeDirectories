#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include<fcntl.h>
#include <sys/stat.h>

int main(void){
	mkdir("NewDir", 0777);
	mkdir("NewDir/SecondDir", 0777);
	char* path2=realpath("MakeLinks.c", NULL);
	char* path3=realpath("NewDir/SecondDir/", NULL);

	char *secPath=malloc( strlen(path3)+ strlen("SymLinkUp")+1);
	sprintf(secPath, "%s%s",path3,"SymLinkUp");
	printf("%s\n",secPath);


	symlink(path2,secPath);
	
	symlink(path3,"SymLinkDown");
	free(path2);
	free(path3);


	return 1;
}
