CC=gcc
CFLAGS=-Wall -g

all: Main.o CatFunctions.o HelpingFunctions.o SymLinkQueue.o HashTable.o FilesAndHardLinks.o
	$(CC) $(CFLAGS) Main.o CatFunctions.o HelpingFunctions.o SymLinkQueue.o HashTable.o FilesAndHardLinks.o cmpcats.h -o cmpcats

Main.o: Main.c cmpcats.h
	$(CC) $(CFLAGS) -c Main.c 
CatFunctions.o: CatFunctions.c cmpcats.h
	$(CC) $(CFLAGS) -c CatFunctions.c 

SymLinkQueue.o: SymLinkQueue.c cmpcats.h
	$(CC) $(CFLAGS) -c SymLinkQueue.c

HelpingFunctions.o: HelpingFunctions.c
	$(CC) $(CFLAGS) -c HelpingFunctions.c 

FilesAndHardLinks.o: FilesAndHardLinks.c cmpcats.h
	$(CC) $(CFLAGS) -c FilesAndHardLinks.c 

HashTable.o:HashTable.c cmpcats.h
	$(CC) $(CFLAGS) -c HashTable.c 

clean:
	rm cmpcats CatFunctions.o HelpingFunctions.o Main.o SymLinkQueue.o HashTable.o FilesAndHardLinks.o

	


