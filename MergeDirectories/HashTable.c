#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmpcats.h"


void hashtable_print(hash_table *ht){
	printf("Start Table.\n");
	//REPLACED WITH NUM_KEYS
	int count=0;
	//go throught each bucket
	for(int i=0; i<ht->current_buckets; i++){
		//in case bucket is empty
		if(ht->bucket_list[i]==NULL){
			printf("\t\%d\t---\n",i);
		}else {
			printf("\t%d\t",i);
			hash_object *tmp=ht->bucket_list[i];
			//go through each linked voter
			while(tmp != NULL){
				printf("%s %ld  -> ", tmp->file->path, tmp->file->original_inode);
				count++;
				//print the next voter in the linked list
				tmp=tmp->next;
			}
			printf("\n");
		}
	}
	printf("End Table.\n");
	printf("%d total voters stored in table. (num_keys= %d)\n",count,ht->num_keys);
}


// find where to which bucket of the hashtable the object with pin belongs. If h_plus==1 use the hash function of the next round (used in hash_split and hashtable_search)
int hash_function(hash_table *ht,unsigned long int pin,int h_plus){
	
	//calculate the 2^round .Since its a positive power of 2 I can calculate it using bitwise operator << because it will always be 0x100...0 where the number of 0 is equal to the round. 
	int round_op=0x1;
	round_op=round_op<<ht->round;
	
	//if we didnt find the file in the first hash value we check if he exists in second half of the split bucket
	if (h_plus == 1){ 
		round_op=round_op<<1;
	}

	int i=pin %( round_op * (ht->init_buckets));
	return i;
}
// check and if needed reset p to 0 and increase the round
int hash_checkupdate_round(hash_table *ht){

	//d = 2 ^ (times the table has doubled)
	int d=0b10;
	d=d << (ht->round) ;	
	if ( ht->current_buckets == (ht->init_buckets *d)){
		//the table has doubled in size so i should reset the index of where the next bucket will split and update round and init_buckets
		ht->p=0;
		ht->round=(ht->round +1);
		return 1;
	}
	return 0;
	
}
//check if the table has too many objects and should split a bucket. Returns 0 for no 1 for yes.
int hash_split_check(hash_table *ht){
	double l;
	l= (double)(ht->num_keys)/( (ht->current_buckets) * (ht->bucket_size) );
	if ( l <= 0.75){
		return 0;
	}else{
		return 1;
	}
}

//create a new bucket and split the objects in bucket_list[p] between the 2 of them
int hash_split(hash_table *ht){

	hash_object **tmp;
	//realloc to add space for a new bucket
	tmp=realloc(ht->bucket_list, sizeof(hash_object *)*(ht->current_buckets+1));
	if(tmp == NULL ){
		//couldnt alocate memory in that case realloc leaves the list as it was and returns null
		return -1;
	}
	//point to the new memory
	ht->bucket_list=tmp;

	int new_buck_index=ht->current_buckets;
	//make the new bucket NULL
	ht->bucket_list[new_buck_index]=NULL;
	//update current buckets 
	ht->current_buckets=ht->current_buckets +1;
	// get the index of the bucket that will split
	int split_bucket_index=ht->p;
	//update the index of the next bucket to split
	ht->p = (ht->p) +1;

	hash_object *list_to_distribute=ht->bucket_list[split_bucket_index];
	if(list_to_distribute==NULL){
		//the bucket was empty so no need to do anything
		return 1;
	}
	//detach the bucket from the list
	ht->bucket_list[split_bucket_index]=NULL;
	
	
	hash_object *tmp2;
	//go throught each object in the linked list and put it in the apropriate bucket using the hash function for round=round+1
	//the objects are added at the start of the bucket for speed since the order doesnt matter in performance
	while( list_to_distribute !=NULL){

		//get the new hash
		int i= hash_function(ht,list_to_distribute->pin,1);
		//point to object we are changing
		tmp2=list_to_distribute;
		//go to the next objet
		list_to_distribute=list_to_distribute->next;

		//make the first object of the apropriate bucket its next 
		tmp2->next=ht->bucket_list[i];
		//make the object the first in the bucket
		ht->bucket_list[i]=tmp2;


	}


	return 1;
}


//create a table with 'size' buckets that each can hold 'cells' objects without overflowing
hash_table *hashtable_create(unsigned int size,int cells){
	// pointer to new dynamicly created hash table
	hash_table *ht= malloc(sizeof(*ht));
	// initialize the values
	ht->init_buckets=size;
	ht->current_buckets=size;
	ht->bucket_size=cells;
	ht->round=0;
	ht->p=0;
	//ADDED
	ht->num_keys=0;
	//set up the buckets with calloc because it zeros out the memory so we dont have to set them to NULL
	ht->bucket_list=calloc(ht->init_buckets, sizeof(hash_object *));
	return ht;
}

int hashtable_total_files(hash_table *ht){
	return ht->num_keys;
}
//delete the hash table
int hashtable_destroy(hash_table *ht){

	int size=0;
	//go thought each bucket and clear every element of the linked list
	for(int i=0;i< ht->current_buckets;i++){
		while(ht->bucket_list[i]){
			hash_object *tmp=ht->bucket_list[i];
			//point to the next element
			ht->bucket_list[i]=ht->bucket_list[i]->next;
			//WRONG I DOUBLE FREE THIS WAY
				//free the voter that he points to and then the voter
				//free(tmp->next);
			//free the object it is pointing to
			free(tmp->file->path);
			free(tmp->file);
			free(tmp);
		}
	}
	free(ht->bucket_list);
	free(ht);
	return size;
}


//add to the hashtable pointed by ht the voter person. Will return 1 for success and 0 for faillure. Insert will call the hashfunction to find the corresponding bucket.
int hashtable_insert(hash_table *ht,file_struct *nFile) {
	if(nFile == NULL || ht ==NULL){
		printf("You can't add a null file or to a null table\n");
		return -1;
	}
	unsigned long int key=nFile->original_inode;
	//if the person is already in the table ignore the insert and return 0
	if((hashtable_search(ht, key))!=NULL) return 0;

	unsigned int index=hash_function(ht,key,0);
	
	//if the hash is less than the current p we call the hash function again i check if it is in the bucket that split
	if ( index < (ht->p) ){

		index=hash_function(ht, key, 1);
	}
	//create a new entry
	hash_object *new=malloc(sizeof(*new));
	if ( new== NULL )
	{
		perror("Error: couldnt allocate space for hashtable_insert\n");
		exit(-1);
	}
	new->pin=nFile->original_inode;
	new->file=nFile;
	new->next=NULL;
	//insert entry
	
	//add at the start of the linked list its faster.
	//the objects are added at the start of the bucket for speed since the order doesnt matter in performance
	new->next=ht->bucket_list[index];
	ht->bucket_list[index]=new;
	
	//update the number of keys in the table
	ht->num_keys++;

	//after adding the object to the table check if we need to split a bucket

	int check=hash_split_check(ht);
	if ( check == 1){
		hash_split(ht);
		//check if we need to go to the next round
		hash_checkupdate_round(ht);
	}
	
	return 1;
}



//search and return the voter with pin returns NULL if it didn't find the voter.
file_struct * hashtable_search(hash_table *ht, unsigned long int s_pin){
	if (s_pin<0 || ht==NULL){
		printf("Error tried to add %ld to NULL table\n",s_pin);
		exit(-1);
	}
	//get the index that the voter should be at
	int index= hash_function(ht, s_pin, 0);

	hash_object *tmp=ht->bucket_list[index];
	//go thought the linked list in the index to find the voter
	while ((tmp !=NULL) && (s_pin != tmp->pin)){
		tmp=tmp->next;
	}
	//check if there is a chance the bucket has split
	if( (tmp==NULL) && (ht->p > index)){
		index= hash_function(ht, s_pin, 1);
		if ( index < (ht->current_buckets)){
			//repeat the previous search in the new index
			tmp=ht->bucket_list[index];
			//go thought the linked list in the index to find the voter
			while ((tmp !=NULL) && (s_pin != tmp->pin)){
				tmp=tmp->next;
			}
		}
	}
	//reached the end of the linked list and didnt find object

	if (tmp==NULL){
		return NULL;
	}
	return tmp->file;

}

//search and remove the voter with pin. Made it for testing, not used in main.
file_struct* hashtable_remove(hash_table *ht, long int s_pin){

	if (s_pin<0 || ht==NULL){
		printf("Error tried to add %ld to NULL table\n",s_pin);
		exit(-1);
	}
	//get the index that the voter should be at
	int index= hash_function(ht, s_pin, 0);

	hash_object *tmp=ht->bucket_list[index];
	//Used to keep the list linked
	hash_object *prev=NULL;
	//go thought the linked list in the index to find the voter
	while ((tmp !=NULL) && (s_pin != tmp->pin)){

		prev=tmp;
		tmp=tmp->next;
	}

	//if i didnt find the object check if the bucket is before the index p 
	//if it is there is a chance the item is in another bucket using the hash
	//function for round=round+1
	if( (tmp==NULL) && (ht->p > index)){
		index= hash_function(ht, s_pin, 1);

		//make sure that the index isnt out of range 
		if ( index < ht->current_buckets){ 
			//repeat the previous search in the new index
			hash_object *tmp=ht->bucket_list[index];
			//go thought the linked list in the index to find the voter
			while ((tmp !=NULL) && (s_pin != tmp->pin)){
				prev=tmp;
				tmp=tmp->next;

			}
		}
	}
	if (tmp==NULL) return NULL;
	//remove the object from the linked list by making the previous object point to its next
	if (prev == NULL){
		//deleting the head of the linked list
		ht->bucket_list[index]=tmp->next;
	}else{
		//deleting from anywhere else
		prev->next=tmp->next;
	}
	//return file to the user
	file_struct *result=tmp->file;

	free(tmp);
	ht->num_keys--;
	return result;
}


