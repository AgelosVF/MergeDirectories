Calling make compiles the code to the executable cmpcats that if you call without any arguments it will give you instructions about how to use it.
Has two uses: 
1. Print the files that exist only in each directory and sub directory by calling it as ./cmpcats -d dirname1 dirname2
2. Create a new directory that has all the files of both directories but doesnt have duplicates of files that exist in both directories in the exact
by calling as ./cmpcats -d dirname1 dirname2 -s newdirname
Στον φάκελο Tests υπάρχουν φάκελοι με συνδυασμό των οποίων έλεγξα τον κωδικά μου. Συγκεκριμένα στον φάκελο LinkTest1
υπάρχει κώδικας για ένα πρόχειρο πρόγραμμα σε C που αν γίνει compile και το τρέξετε θα φτιάξει μέσα του φακέλους
και soft links.

Ο κώδικας μου είναι χωρισμένος σε 7 αρχεία:
1. cmpcats.h:  
	Είναι το header file που περιέχει τους ορισμούς των διαμιραζόμενων μεταξύ αρχείων συναρτήσεων.
2. Main.c:  
	Είναι η main μου που διαβάζει τα ορίσματα βρίσκει το absolute path τον καταλόγων και καλεί την 
	merge_dir() ή την print_dir() (που βρίσκονται στο αρχειο CatFunctions.c ) ανάλογος τον ορισμάτων.
3. HelpingFunctions.c: 
	Περιέχει 2 συναρτήσεις:
		1. char* remove_substring(const char* str, const char* subStr); 
		2.char* change_starting_path(const char* path,const char* oldStart, const char* newStart); 
	τις οποίες χρησιμοποιώ για να αλλάζω τις αρχές των absolute paths έτσι ώστε να τεστάρω αν ένα 
	αρχείο ενός καταλόγου υπάρχει και στον άλλο και για να φτιάχνω τα αρχεία στον τρίτο κατάλογο.
4. HashTable.c: 
	Περιέχει τον κώδικα του hashtable που είχα παραδώσει στην πρώτη εργασία. Στο hashtable αποθηκεύω τα
	inode number και paths των καινούργιων αρχείων που δημιουργώ. Κάθε φορά που πάω να φτιάξω ένα
	καινούργιο αρχείο κοιτάω αν υπάρχει στο hashtable μου το inode του. Αν ναι τότε δημιουργώ ένα
	hardlink σε αυτό. Αλλιώς δημιουργώ το αρχείο και προσθέτω τα στοιχειά του στο table. Έτσι χειρίζομαι
	τα hardlinks και αποφεύγω να δημιουργώ διπλά αρχεία.
	Στο hashtable αποθηκεύουμε structs που έχουν το inode του αρχείου που αντίγραψαμε και το path στο οποίο
	βρίσκετε το καινούργιο.
5. FilesAndHardLinks.c:
	Περιέχει της συναρτήσεις για την αντιγραφή "regular file" και hardlink.
	Η int copyOrHardLink(unsigned long int inode,hash_table *hashTable,char *createNewFile,char *FileName);
	ελέγχει το hashtable να δει αν το αρχείο έχει ήδη δημιουργηθεί. Αν ναι τότε φτιάχνει ένα link στο path
	που πίρε από το hashtable μέσω link(). Αν δεν υπάρχει το αρχείο στο hashtable τότε καλεί την 
	void copyFile(const char* sourcePath, const char* destinationPath); για να αντιγράψει το αρχείο και το
	προσθέτει στο hashtable.
	H void copyFile(const char* sourcePath, const char* destinationPath); δημιουργεί ένα αντίγραφο του 
	sourcePath με fopen και fread. Μετά κλείνει τα 2 αρχεία και χρησιμοποιεί 
	chmod(destinationPath, sourceStat.st_mode)  για να θέσει τα permission του καινούργιου αρχείου ίδια με
	το αρχικού. 
6. SymLinkQueue.c:
	Περιέχει τον κώδικα που χειρίζεται την δημιουργία link.
	H συνάρτηση void createSymlinkOrEnqueue(Queue* q, const char* symlinkPath, const char* targetFileName)
	είναι υπεύθυνη για την δημιουργία των links. Επειδή υπάρχει περίπτωση το αρχείο που δείχνει το link να
	μην δημιουργηθεί ακόμα στην νέα ιεραρχία  πρέπει να ελέγξει αν υπάρχει. Αν ναι τότε φτιάχνει το 
	καινούργιο link μέσω symlink(). Αλλιώς βάζει της πληροφορίες του link σε ένα linked list μέσω της 
	enqueue() έτσι ώστε να δημιουργηθε αργότερα.
	Η συνάρτηση που θα δημιουργήσει αργότερα τα links που δεν φτιαχτήκαν πριν είναι η void processQueue(Queue* q).
	Αυτό που κάνει είναι ξεκινάει από την αρχή τις λίστας και ελέγχει ξανά αν το target του link έχει δημιουργηθεί.
	Αν ναι τότε δημιουργεί το link. Υπάρχει η περίπτωση το target να μην υπάρχει για ακόμα μια φορά αν ήταν εντός
	κάποιου directory που δεν αντιγράφτηκε. Σε αυτή την περίπτωση προσπέρνα αυτό το link έτσι ώστε να μην έχουμε 
	links που δείχνουν εκτός της καινούργια ιεραρχίας. Μετά κάνει free() τα δεδομένα και περνάει στο επόμενο στοιχειό
	της λίστας μέχρι να φτάσει στο τέλος της.
7. CatFunctions.c:
	Περιέχει τις συναρτήσεις που ενώνουν όλες τις προηγούμενες για να τρέξει το πρόγραμμα.
	
	int fileExists(const char* path) : Ελέγχει αν το αρχείο που της δόθηκε υπάρχει μέσω lstat().

	int same_file(const char* firstFile, const char *secondFile):
		Ελέγχει αν 2 "regular files" είναι τα ίδια. Πρωτα κάνει lstat(secondFile,&secondInfo) για να δει ότι 
		το secondFile εχει το ίδιο όνομα και σχετικό path με το firstFile. Αν υπάρχει και είναι και αυτό απλό αρχείο 
		(secondInfo.st_mode & S_IFMT) != S_IFREG )  συγκρίνει το μέγεθος τους (firstInfo.st_size != secondInfo.st_size).
		Aν είναι και αυτό το ίδιο ανοίγει και τα δυο αρχεία για να  διαβάσει και να συγκρίνει έναν έναν τους χαρακτήρες
		τους μέχρι να βρει διαφορετικό. Αν φτάσει στο τέλος των αρχείων σημαίνει ότι είναι ίδια.
	
	int same_direct(const char* firstDir, const char* secondDir):
		 Ελέγχει αν το secondDir είναι "ίδιος" φάκελος με το firstDir. Καλεί lstat() για να δει ότι υπάρχει αρχείο με 
		 αυτό το path και μετά ελέγχει αν είναι φάκελος ( secondInfo.st_mode & S_IFMT)!=S_IFDIR ).

	int same_link(const char* firstLink,const char* firstPathStart, const char* secondLink, const char* secondPathStart):
		Ελέγχει αν το secondLink είναι ίδιο με το πρώτο Αφού ελέγξει ότι υπάρχει και είναι link με τον ίδιο τρόπο με της
		από πάνω συναρτήσεις χρησιμοποιεί την realpath() για να βρει τα αρχεία στα οποία δείχνουν. Μετά συγκρίνει τα 
		relative path των στόχων για να δει ότι είναι τα ίδια Αν είναι τότε βρίσκει τι αρχείο είναι το πρώτο και καλεί
		same_file ή same_direct για να δει αν και το δεύτερο link δείχνει στο ίδιο αρχείο (εντός της δικιάς του ιεραρχίας).
		Επίσης αν κάποιο link δείχνει εκτός της ιεραρχίας του τερματίζει το πρόγραμμα καθώς θα έπρεπε όλα να δείχνουν εντός.

	
	int compareAndCreate(const char *firstDir,const char *firstDirStart,const char *secondDir,const char *secondDirStart,
	const char *newDir,Queue *linkQueue,hash_table *hashTable,int create,int common):
		Είναι η συνάρτηση που διαβάζει ένα directory και ελέγχει αν είναι το ίδιο με το άλλο. Το όρισμα create θέτετε
		1 αν θέλουμε να δημιουργήσει στο new Dir τα αρχεία του firstDir που δεν υπάρχουν στο secondDir και το common 1
		αν θέλουμε να δημιουργήσει και τα κοινά τους αρχεία. Αρχικά κάνει opendir(firstDir) και μέσα σε μια while διαβάζει
		ένα ένα τα αρχεία του με readdir(). Μετά φτιάχνει το absolute path του αρχείου και το αντίστοιχο path στον δεύτερο
		φάκελο Στην συνεχεία κάνει lstat για να βρει τι αρχείο είναι και μέσω μιας switch καλεί την αντίστοιχη συνάρτηση
		για να το συγκρίνει με το αντίστοιχο του δεύτερου φακέλου. Αν το αρχείο πρέπει να δημιουργηθεί στο newDir καλεί 
		την κατάλληλη συνάρτηση ανάλογος τι αρχείο είναι φτιάχνοντας πάντα πρώτα το όνομα του και περνώντας την κατάλληλη
		βάση δεδομένων αν χρειάζεται (linkQueue για links και HashTable για αρχεία και hardlinks). Αν το αρχείο που διάβασε
		είναι κατάλογος καλεί αναδρομικά τον εαυτό της για τον κατάλογο αλλάζοντας αν χρειάζεται το create και common 
		(κατάλογος με το ίδιο όνομα με άλλο αρχείο που είναι όμως πιο παλιός). Έτσι ελέγχει όλα τα αρχεία μέσω DFS ανάδρομης.
		Χρησιμοποιεί τις συναρτήσεις time και difftime για να πάρει τους χρόνους που θα συγκρίνει σε περίπτωση που έχουμε
		διαφορετικά αρχεία με το ίδιο όνομα Κάτι τελευταίο που κάνει  που θέλω να σημειωθεί είναι όταν αντιγραφεί αρχείο που
		έχουν και οι δυο κατάλογοι προσθέτει και το inode του δεύτερου αρχείο στο hashtable έτσι ώστε όταν πάμε να αντιγράψουμε
		το hardlink από το άλλο directory να αποφύγουμε την δημιουργία  κανονικού αρχείου αφού υπάρχει ήδη.

	int merge_dir(const char *firstDir,const char *secondDir,const char *newDir):
		Είναι η συνάρτηση που καλούμε από την main αν θέλουμε να κάνουμε τυπώσουμε τα μοναδικά αρχεία των δύο καταλόγων και να τους
		κάνουμε merge. Πρώτα φτιάχνει το hashtable και το queue και μετά καλεί δύο φορές την compareAndCreate(). Μια 
		compareAndCreate(firstDir,firstDir,secondDir,secondDir,newDir,LinkQueue,HTable,1,1); για να τυπωθούν τα αρχεία του πρώτου
		καταλόγου που δεν υπάρχουν στον δεύτερο και συγχρόνως να δημιουργηθούν αυτά που θέλουμε μόνο από τον πρώτο αλλά και τα κοινά
		μεταξύ τους αρχεία στον καινούργιο φάκελο. Και την δεύτερη φορά την καλεί ως 
		compareAndCreate(secondDir,secondDir,firstDir,firstDir,newDir,LinkQueue,HTable,1,0); για να τυπωθούν και να φτιαχτούν και τα
		αρχεία του δεύτερου καταλόγου. Τέλος κάνει processQueue() για να φτιαχτούν τα links που δεν μπόρεσαν πριν και κάνει free τις
		δομές. Με αυτόν τον τρόπο κάνουμε merge και print διαβάζοντας μια φορά τον κάθε κατάλογο.
	
	int print_dif(const char *firstDir,const char *secondDir):
		Είναι η συνάρτηση που καλεί η main αν θέλουμε να μόνο να τυπώσουμε τα διαφορετικά αρχεία κάθε καταλόγου. Καλεί δυο φορές την
		compareAndCreate() μια ως compareAndCreate(firstDir,firstDir,secondDir,secondDir,NULL,NULL,NULL,0,0);και άλλη μια ως 
		compareAndCreate(secondDir,secondDir,firstDir,firstDir,NULL,NULL,NULL,0,0);. Έτσι τυπώνει τα αρχεία χωρίς να φτιάξει κάποια
		δομή ή να δημιούργισει αντίγραφα.
