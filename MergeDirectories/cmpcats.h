//
//in CatFunctions.c
//compares openDir with lstatDir and print the files that are in openDir and not in lstatDir
int compare_dir(const char* openDir, const char* openDirStart, const char* lstatDir, const char*lstatDirStart);

//print every file in dirname. Used in compare_dir when we find a directory that exists only in one of the two directories.
int print_dir(const char* dirname,const char * firstDir);

//check if firstFile and secondFile are the "same" if yes return 1 else -1
int same_file(const char* firstFile, const char *secondFile);

//check if firstDir and secondDir are the "same" if yes return 1 else -1
int same_direct(const char* firstDir, const char* secondDir);

//check if firstLink and secondLink are the "same" if yes return 1 else -1
int same_link(const char* firstLink,const char* firstPathStart, const char* secondLink, const char* secondPathStart);

//merge the directories in the new directory and prints the diffrences
int merge_dir(const char *firstDir,const char *secondDir,const char *newDir);

//just prints the unique files in each directory
int print_dif(const char *firstDir,const char *secondDir);
//returns 1 if file exists 0 if not
int fileExists(const char* path);
//-----------------------------------------------------------------------------------------------------------//
//in HelplingFunctions.c 

//return return str without the subStr at the begining. If str=home/aggelos/DirA/DirB/DirC and subStr=home/aggelos/DirA it returns DirB/DirC
char* remove_substring(const char* str, const char* subStr);

//changes the path to start with newStart instead of oldStart
char* change_starting_path(const char* path,const char* oldStart, const char* newStart);

//-----------------------------------------------------------------------------------------------------------//
//in SymLinkQueue.c
//Queue in the form of a linked list used to store and handle symlinks

typedef struct QueueItem {
    char* symlinkPath;
    char* targetFileName;
    struct QueueItem* next;
} QueueItem;

typedef struct {
    QueueItem* front;
    QueueItem* rear;
} Queue;

//initializes the pointers in the queue to NULL
void initializeQueue(Queue* q);

//creates the link or if the file it points to hasnt been created yet it puts its information to a queue in order to be created later
void createSymlinkOrEnqueue(Queue* q, const char* symlinkPath, const char* fileName);

//empty the queue creating each link that its file exists and skiping the ones whoses files dont exist
void processQueue(Queue* q);


//------------------------------------------------------------------------------------------------------//
//in HashTable.c
//Hash table for processing files and hard links
//before creating any file we check the hash table if the inode exists it means we have already copied the file
//so we create a hardlink to it in order to avoid dublicates.

typedef struct{
	char*path;
	unsigned long int original_inode;
}file_struct;

typedef struct hash_object{
	unsigned long int pin;
	file_struct *file;
	struct hash_object *next;
}hash_object;

typedef struct hash_table{
	unsigned int init_buckets;// the number of starting buckets
	unsigned int current_buckets;// the number of buckets that the table currently holds
	int bucket_size; // the number of voters a bucket can hold without overflowing
	unsigned int round; // the current round/level of hashing 
	int p;//Could also be an int to for indexing instead of a pointer
	int num_keys;// to keep track of the number of elements in the table so I can check if i need to split a bucket
	hash_object **bucket_list;// an array of pointers to voters.Each bucket will have a linked list voters. 
}hash_table;



//create a table with 'size' buckets that each can hold 'cells' objects without overflowing
hash_table *hashtable_create(unsigned int size,int cells);


int hashtable_insert(hash_table *ht,file_struct *nFile);

//search and return the file with inode==s_pin returns NULL if it didn't find the file.
file_struct * hashtable_search(hash_table *ht,unsigned long int s_pin);

//delete the hashtable and free the memory
int hashtable_destroy(hash_table *ht);
//prints the hashtable used to find memory leaks not in the final program
void hashtable_print(hash_table *ht);
//-------------------------------------------------------------------------------------------------------//
//in FilesAndHardlinks

//checks hashtable to see if i have already created the fileName with inode=inode .If yes create a hard link to it if not copy the file
//returns 1 for creating a hardlink and 2 for copying the file
int copyOrHardLink(unsigned long int inode,hash_table *hashTable,char *createNewFile,char *FileName);

//create a copy of the file:sourcePath to the destinationPath
void copyFile(const char* sourcePath, const char* destinationPath); 

