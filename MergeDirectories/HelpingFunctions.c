#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char* remove_substring(const char* str, const char* subStr) {
	int strLen = strlen(str);
	int subStrLen = strlen(subStr);


	if (strLen < subStrLen) {
		return NULL;  // subString is longer than the string
	}

	if (strncmp(str, subStr, subStrLen) == 0) {
		return (char*)(str + subStrLen);
	} 
	else {
		return NULL;  // subString doesn't match the beginning of the string
	}
}

char* change_starting_path(const char* path,const char* oldStart, const char* newStart)
{
	//carefull with the '/' either oldStart not end with one or newStart end with one but not both. xor
	//new_path should be freed by the function that called change_starting_path after it is used.
	const char* end=remove_substring(path, oldStart);
	if(end!=NULL){
		char *new_path=malloc(strlen(path) + strlen(newStart) +1);
		sprintf(new_path,"%s%s",newStart,end);
		return new_path;
	}
	else {
		char *new_path=malloc(strlen(newStart)+1);
		strcpy(new_path, newStart);
		return new_path;
	}
}

