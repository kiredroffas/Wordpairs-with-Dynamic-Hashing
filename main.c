/* Erik Safford
 * Word Pair Counting using a Dynamic Hash
 * Spring 2019 */

#include "main.h"

#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "getWord.h"
#include "getWord.c"
#include "crc64.h"
#include "bucket.h"
#include "hash.h"
#include "sortHash.h"

#define DEBUG 0  //Debug print statements, 0 = OFF, 1 = ON

int searchHash(Bucket *b,char *newPair);                             //Searches a spot in a Hash structure for a specified wordPair
void insertHash(Hash *h,long long int spot,char *newPair);           //Mallocs and inserts a new wordPair Bucket to the front of the Bucket chain
void insertReHash(Hash *h,long long int spot,Bucket *b);             //Repoints a current Bucket to a new spot in a larger Hash
void freeHash(Hash *h,int hashSize);                                 //Free the memory from a Hash, including all Bucket and wordPairs

/* combineWords() is called whenever getWordPairs() attemps to read in another wordPair (two calls of getNextWord()) and is passed two 
 * words. It allocates space for the new wordPair, copies the first word, addes a space in between, copies the second word, to create a 
 * wordPair, which it then returns to getWordPairs().  */
char *combineWords(char *ch,char*ch2) { 
	//Combine char *words into a single string
	char *newPair = malloc(sizeof(char) * 100); //allocate space for a new wordPair, maxsize 100 characters
	int place = 0;                    //Keep track of what spot in the newPair we're in

	for(int i=0;i < strlen(ch);i++) { //Copy the first word ch to the newPair
		newPair[place] = ch[i];
		place++;
	}
	newPair[place] = ' ';             //Add a space in between
	place++;
	for(int i=0;i < strlen(ch2);i++) {//Copy the second word ch2 to the newPair
                newPair[place] = ch2[i];
                place++;
        }
	newPair[place] = '\0';            //Add null term to end of newPair

	if(DEBUG == 1) { printf("newPair is '%s'\n",newPair); } //Test
        free(ch); //Free the memory allocated for the first word by getNextWord()
	          //We don't want to free the second word ch2 b/c it will be used in the next wordpair
	return(newPair);
}  

/* reHash() is called when a certain collision count is reached when adding new wordPair Buckets to a Hash (kept track of by getWordPairs()). 
 * It is passed a pointer to a wordPair Bucket-filled Hash and a multiplier to increase the Hash SIZE by. It allocates memory for a new Hash
 * the SIZE of which is specified by the passed multiplier. Hash sentinel information stored in h->entry[0] is copied over, and all
 * of the active wordPair Buckets stored in the smaller Hash are rehashed into the appropriate spot in the new Hash with crc64().
 * Chaining in the new Hash is handled by insertReHash(), which adds a Bucket to the front of a Bucket chain if necessary. Last the old Hash
 * structure is freed, leaving its Buckets in the new larger Hash which is returned. */
Hash *reHash(Hash *h,int multiplier) {
	if(DEBUG == 1) { printf("\nRehashing --- multiplier = %d, Hash size is %d\n",multiplier,SIZE * multiplier); }

	Hash *newh = (Hash*) malloc(sizeof(Hash) * multiplier);  //Create hash w/ SIZE buckets * multiplier
	newh->entry[0] = h->entry[0];                            //Repoint Bucket collision/mutiplier counter
	newh->entry[0]->collisions = 0;                          //Reset count
	newh->entry[0]->multiplier = h->entry[0]->multiplier;    //Copy over multiplier

	for(int i = 1;i < (SIZE * (multiplier/3) );i++) { //For as many entries are in the old Hash, (SIZE * (multiplier/3) = old Hash size
		Bucket *temp = h->entry[i];      //temp is bucket to be copied over
		while(temp != NULL) {            //While there is chained buckets at h->entry[i]
			long long int spot = crc64(h->entry[i]->wordPair) % (SIZE * multiplier); //Find the new entry spot for wordPair in new larger hash
			if(DEBUG == 1) { printf("Rehashing: h->entry[i]->wordPair = %s, spot = %llu\n",temp->wordPair,spot);printf("Rehashing: h->entry[i]->value = %d\n",temp->value);}
			Bucket *pretemp = temp->next;      //Set pretemp to next Bucket in chain (either another Bucket or NULL
							   //temp->next gets overwritten in insertReHash()
			insertReHash(newh,spot,temp);      //Insert bucket from old hash to new spot in larger hash
			temp = pretemp;                    //Set temp to next Bucket in chain from pretemp (either another Bucket or NULL)
		}
	}

	free(h); //Free original hash
	return(newh); //Return new larger hash with same wordpair values (most likely at different spots in hash)
}	

/* getWordPairs() is called when a FILE containing words is read in by main(), and is passed a FILE pointer and a Hash structure containing a 
 * sentinel Bucket at entry[0]. It begins by reading in the current multiplier of the Hash from the sentinel at h->entry[0], which will be used
 * in the case of too many collisions occuring to reHash the Hash to a larger SIZE * 3. Then starting by initially reading two words from the 
 * FILE using getNextWord(), while the two words read in don't equal NULL:
 *
 * 	1. Check the collisions stored in Hash sentinel entry[0] to see if Hash needs to be reHashed to larger SIZE
 *		-If so increase the Hash multiplier * 3, and pass the Hash and multipier to reHash() to get larger Hash
 * 	2. Create a new wordPair: save the second word read for the next wordPair in temp, pass combineWords() both words
 * 	3. Pass hashing function crc64() the new wordPair to determine its spot in the Hash.
 * 	4. Attempt to enter the new wordPair into the Hash:
 * 		-If the spot in the Hash doesnt have a active Bucket there (NULL), malloc a new Bucket containing the wordPair
 * 		 and enter it at that spot in the Hash.
 *		-Else if theres a Bucket already at the spot in the Hash call searchHash() to either increment the occurance value
 *		 of a matching wordPair Bucket or..
 *		-If theres no matching wordPair Bucket call insertHash() to insert a new Bucket to the front of the Bucket chain (separate chaining)
 *		 and increment the collsion counter in the Hash sentinel.
 *	5. Assign the 2nd temp word saved earlier to be the new first word in the newPair
 *	6. Call getNextWord() to get a second word for the newPair
 *
 * Once getNextWord() reads a NULL (meaning the end of the file or no more words), it frees the word saved in temp and returns the wordPair Bucket-filled Hash to main. */
Hash *getWordPairs(FILE *fp, Hash *h) { 
	//Passed a opened file pointer, reads 2 words from the file at a time as a wordpair
	
	char *ch, *ch2;
	int multiplier = h->entry[0]->multiplier; //Store current Hash multiplier, this will increase by a factor of 3 if too many collisions occur when hashing

        ch = getNextWord(fp); //Get first word
        ch2 = getNextWord(fp); //Get second word

        while(ch != NULL && ch2 != NULL) {
		//If h->entry[0]->collisions is greater then 1/3 of (SIZE * multiplier) then call reHash()
		//(pass reHash() pointer to hash, increased multiplier x 3)
		//reHash() all old hash values into new larger hash, free old hash, return new hash
		if(h->entry[0]->collisions > ((SIZE * multiplier) / 3)) {  
			h->entry[0]->multiplier *= 3;         //Multiply multiplier by 3
			h = reHash(h,h->entry[0]->multiplier);//ReHash current Hash and stored Buckets into larger Hash
			multiplier = h->entry[0]->multiplier; //Set local multiplier to new multiplier
			h->entry[0]->collisions = 0;          //Reset collisions to 0 since we just reHashed
		}
		if(DEBUG == 1) {printf("getWordPairs: multiplier = %d\n",h->entry[0]->multiplier);printf("getWordPairs: collisions = %d\n\n",h->entry[0]->collisions);}

		char *newPair,*temp;
        	temp = ch2;                     //Save 2nd word for next pair
                newPair = combineWords(ch,ch2); //Combine the 2 words
                if(DEBUG == 1) {printf("getWordPairs: combined = '%s' , hash returns:  %llu\n",newPair,crc64(newPair) % (SIZE * multiplier));} //Test

		long long int spot = crc64(newPair) % (SIZE * multiplier); //Spot in hash table for wordpair as defined by hashing function
              
		if(h->entry[spot] == NULL) { //If the spot in our hash table has no previous buckets
			Bucket *b = (Bucket*) malloc(sizeof(Bucket)); //Make a new bucket
			b->wordPair = newPair; //Assign new wordPair to bucket
			b->value = 1;          //Increment occurance value
			b->next = NULL;        //Set next bucket to NULL
			h->entry[spot] = b;    //Assign new bucket to spot in hash table

			if(DEBUG == 1) {printf("getWordPairs: the bucket is null\n");printf("getWordPairs: Put '%s' at h[%llu]\n",b->wordPair,spot);  //Test
			printf("getWordPairs: h->wordpair is: %s\n",h->entry[spot]->wordPair);printf("getWordPairs: h->value is: %d\n\n",h->entry[spot]->value);}
		}
		else { //Search through a h[spot] with bucket values to find the bucket with matching wordPair
			//if matching bucket found, increment value +1
			//otherwise add new wordpair bucket to front of h[spot] bucket chain
			if(spot == 0) { //If crc64 returns 0, dont want to insert where collision counter is
                        	spot = 1;
                        }
			int found = searchHash(h->entry[spot],newPair); //Returns 1 if wordpair found and value++				 
					                                //Returns 0 if wordpair is not found and needs to be added
			if(found == 0) {
				insertHash(h,spot,newPair); //Insert new wordpair to front of bucket chain
				h->entry[0]->collisions++;                 //Increment collision counter in sentinel
			}
		}
		
		ch = temp;             //Take the word saved in temp and make it the new first word
                ch2 = getNextWord(fp); //Try to get a new second word
		if(DEBUG == 1) {printf("collisions = %d\n",h->entry[0]->collisions);printf("Word1: %s , Word2: %s\n",ch,ch2);}  //Test
        }
	free(ch); //Free the word saved in temp
	return(h);
}

/* searchHash() is called when a wordpair is attemping to be inserted into the Hash, but the entry spot of the hash (returned by crc64) already
 * has an active wordpair Bucket at that location (h->entry[spot] != NULL). It is passed the location in the hash of the first bucket in the
 * chain and a wordPair, which is then used to attempt to find a matching wordPaird among the first Bucket passed and any other Bucket's that
 * may be chained to it. If a matching wordPair is found in a Bucket in the chain, the Bucket's occurance value is incremented by one, and 1 
 * is returned. If a matching wordPair is not found anywhere in the Bucket chain, 0 is returned to indicate a new Bucket needs to be added to
 * the front of the chain at that spot in the Hash.   */
int searchHash(Bucket *b,char *newPair) {
	//Search through a h[spot] with bucket values to find the bucket with matching wordPair,if matching wordpair is found value is
	//incremented and returns 1, if matching wordpair not found returns 0 
   
       	/*if(b == NULL) {
		fprintf(stderr,"stderr: Newpair cannot be read : -1\n");
		exit(-1);
	} */

	if(DEBUG == 1) { printf("search: newpair is: %s\n",newPair);printf("search: b->wordpair: %s, b->value: %d\n",b->wordPair,b->value); } //Test
	
	Bucket *temp = b;                                 //Assign Bucket pointer temp to first Bucket in chain
	while(temp != NULL) {                             //While the Bucket isn't NULL
		if(strcmp(temp->wordPair,newPair) == 0) { //Test to see if the wordPairs match, 0 means their equal
			temp->value++;                    //Add one to the occurance value of the matching wordPair Bucket
			if(DEBUG == 1) { printf("search: Bucket value is now: %d\n",temp->value); } //Test
			return(1);                        //Return 1 for matching wordPair found
		}
		temp = temp->next;                        //Otherwise keep incrementing down the Bucket chain until the end (NULL)
	}
	return(0);                                        //If no matching wordPair Buckets can be found return 0
}

/* insertHash() is called when searchHash() returns 0 in getWordPairs(), indicating that a new bucket containing the new wordpair must be
 * inserted to the front of the chain at the spot in the Hash. Passed a Hash structure, a spot in the Hash, and a newPair to add, a new 
 * Bucket is malloced with the new wordPair, an occurance of 1, and a next value of the original Bucket in the Hash spot. The the front
 * spot in the Hash is replaced with the new wordPair Bucket, essentially inserting the new bucket at the front of the h->entry[spot]
 * Bucket chain.  */
void insertHash(Hash *h,long long int spot,char *newPair) {
	Bucket *b = h->entry[spot];                      //Save current front Bucket at spot in Hash
	Bucket *newB = (Bucket*) malloc(sizeof(Bucket)); //Make a new Bucket
        newB->wordPair = newPair;                        //Assign new Bucket the new wordPair
        newB->value = 1;                                 //Occurance value of 1

	//Insert bucket to front of h[spot] bucket chain
	h->entry[spot] = newB;    //Front spot in Hash equals new wordPair Bucket
	newB->next = b;           //New Bucket next is old front Bucket
	
	if(DEBUG == 1) {  //Test
        	Bucket *temp = h->entry[spot];
		int i=0;
		while(temp != NULL) {
			printf("insert: bucket %d is: %s\n",i,temp->wordPair);
			temp = temp->next;
			i++;
		}
		printf("\n");
	}
}

/* insertReHash() is called when reHash() attempts to repoint an existing Bucket in an old smaller Hash to a new larger Hash with a different
 * hashing index. It is passed a new larger Hash to insert into, the spot in the Hash to insert, and the Bucket containing the wordPair from the
 * old Hash. If the spot in the new Hash has no Buckets in it, assign the new wordPair Bucket to the front in that spot with a next pointer of
 * NULL. If there is a previous Bucket in the spot to be added to, make the new wordPair Bucket the front of the Bucket chain and push the 
 * previous front to next.  */
void insertReHash(Hash *h,long long int spot,Bucket *b) {
	Bucket *oldb = h->entry[spot];
	if(oldb == NULL) {
        	h->entry[spot] = b;  //Insert bucket to front of h[spot] bucket chain
		b->next = NULL;
	}
	else {
		h->entry[spot] = b;  //Insert to front and push previous front Bucket to next
		b->next = oldb;
	}

        if(DEBUG == 1) {
        	Bucket *temp = h->entry[spot];
        	int i=0;
        	while(temp != NULL) {
                	printf("insertReHash: bucket %d is: %s\n",i,temp->wordPair);
                	temp = temp->next;
                	i++;
        	}
        	printf("\n");
	}
}

/* freeHash() is called after all wordPair counting/sorting/printing has been done, and is passed a Hash structure to free,
 * and the current size of the Hash (SIZE * multiplier). Using the size of the Hash, it loops through and frees every malloced
 * wordPair, Bucket, and finally the Hash itself.  */
void freeHash(Hash *h,int hashSize) {
	for(int i=0;i < hashSize;i++) { //Loop through every entry in the Hash
		Bucket *b = h->entry[i];//b is the Bucket at the front of the chain (could be NULL)
		Bucket *nextBucket;
		if(b != NULL) {                //If the spot in the Hash isnt NULL
			nextBucket = b->next;  //Save the next Bucket in the chain (could be NULL)
			if(i != 0) {               //wordPair isn't malloced for entry[0] sentinel Bucket
				free(b->wordPair); //Free the wordPair within the Bucket
			}
			free(b);                   //Free the Bucket
			b = nextBucket;	           //Move to next Bucket in chain in there is one
		}
	}
	free(h);      //Free the Hash structure
}

int main(int argc, char **argv) { //Read arguments in from cmdline: wordpairs -count fileName1 fileName2 fileName3...
				  //                                wordpairs fileName1 fileName2 fileName3 ...
				  //(where count is a positive integer number of wordpairs to count, no count specifies all pairs)
	if(argc < 2) { //If file to read from isnt specified
		fprintf(stderr,"stderr: Please specify file to read from\n");
		exit(-1);
	}		
	if(argv[4] && argv[3] && argv[2] && argv[1][0] != '-') { //If there is 4 cmdLine args without count(wordpairs fileName1 fileName2 fileName3 fileName4)
		assert(argv[1]);
 	        assert(argv[2]);
       	        assert(argv[3]);
		assert(argv[4]);
		Hash *h = (Hash*) malloc(sizeof(Hash));             //Create hash w/ SIZE buckets initially,x3 after collisions
                Bucket *cBucket = (Bucket*) malloc(sizeof(Bucket)); //Create collision/multiplier bucket to keep track of collisions
                cBucket->wordPair = "collisions";
                cBucket->collisions = 0;
                cBucket->multiplier = 1;
                h->entry[0] = cBucket;

                FILE *fp;
                fp = fopen(argv[1],"r");  //Open the first FILE for reading
                h = getWordPairs(fp,h);   //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp);
                FILE *fp2;
                fp2 = fopen(argv[2],"r"); //Open the second FILE for reading
                h = getWordPairs(fp2,h);  //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp2);
                FILE *fp3;
                fp3 = fopen(argv[3],"r"); //Open the third FILE for reading
                h = getWordPairs(fp3,h);  //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp3);
		FILE *fp4;
                fp4 = fopen(argv[4],"r"); //Open the fourth FILE for reading
                h = getWordPairs(fp4,h);  //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp4);

                if(DEBUG == 1) {printf("SIZE of hash is: %d\n",SIZE * h->entry[0]->multiplier);}

                int activeBuckets = 0;                          //Keep track of how many wordPair buckets are in the Hash
                int hashSize = SIZE * h->entry[0]->multiplier;  //Get the current size of the Hash
                for(int i=1;i < hashSize;i++) {   //Count the active wordPair Buckets
                        Bucket *temp = h->entry[i];
                        while(temp != NULL) {
                                activeBuckets++;
                                temp = temp->next;
                        }
                }
                if(DEBUG == 1) {printf("activebuckets = %d\n",activeBuckets);}

                h = sortHash(h,hashSize,activeBuckets,-1); //Sort and print the wordPairs stored in the Hash
                freeHash(h,hashSize);                      //Free the memory allocated for the Hash
	}
	else if(argv[4] && argv[3] && argv[2] && argv[1][0] == '-') { //If there is 4 cmdLine args with count(wordpairs -count fileName1 fileName2 fileName3)
		assert(argv[1]);
                assert(argv[2]);
                assert(argv[3]);
		assert(argv[4]);
		int pairCount = 0; //Initialize word pairs to count to 0
                pairCount = atoi(argv[1]); //Convert char * count to int count
                pairCount = pairCount * -1; //Turn count into positive number
                if(DEBUG == 1) {printf("paircount = %d\n",pairCount);}  //Test

                Hash *h = (Hash*) malloc(sizeof(Hash));             //Create hash w/ SIZE buckets initially,x3 after collisions
                Bucket *cBucket = (Bucket*) malloc(sizeof(Bucket)); //Create collision/multiplier bucket to keep track of collisions
                cBucket->wordPair = "collisions";
                cBucket->collisions = 0;
                cBucket->multiplier = 1;
                h->entry[0] = cBucket;

                FILE *fp;
                fp = fopen(argv[2],"r");  //Open the first FILE for reading
                h = getWordPairs(fp,h);   //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp);
                FILE *fp2;
                fp2 = fopen(argv[3],"r"); //Open the second FILE for reading
                h = getWordPairs(fp2,h);  //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp2);
		FILE *fp3;
                fp3 = fopen(argv[4],"r"); //Open the third FILE for reading
                h = getWordPairs(fp3,h);  //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp3);

                if(DEBUG == 1) {printf("SIZE of hash is: %d\n",SIZE * h->entry[0]->multiplier);}

                int activeBuckets = 0;                          //Keep track of how many wordPair buckets are in the Hash
                int hashSize = SIZE * h->entry[0]->multiplier;  //Get the current size of the Hash
                for(int i=1;i < hashSize;i++) {   //Count the active wordPair Buckets
                        Bucket *temp = h->entry[i];
                        while(temp != NULL) {
                                activeBuckets++;
                                temp = temp->next;
                        }
                }
                if(DEBUG == 1) {printf("activebuckets = %d\n",activeBuckets);}

                h = sortHash(h,hashSize,activeBuckets,pairCount); //Sort and print the wordPairs stored in the Hash
                freeHash(h,hashSize);                      //Free the memory allocated for the Hash
	}
	else if(argv[3] && argv[2] && argv[1][0] != '-') { //If there is 3 cmdline args without count(wordpairs fileName1 fileName2 fileName3)
		assert(argv[1]);
                assert(argv[2]);
                assert(argv[3]);
		Hash *h = (Hash*) malloc(sizeof(Hash));             //Create hash w/ SIZE buckets initially,x3 after collisions
                Bucket *cBucket = (Bucket*) malloc(sizeof(Bucket)); //Create collision/multiplier bucket to keep track of collisions
                cBucket->wordPair = "collisions";
                cBucket->collisions = 0;
                cBucket->multiplier = 1;
                h->entry[0] = cBucket;

                FILE *fp;
                fp = fopen(argv[1],"r");  //Open the first FILE for reading
                h = getWordPairs(fp,h);   //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp);
                FILE *fp2;
                fp2 = fopen(argv[2],"r"); //Open the second FILE for reading
                h = getWordPairs(fp2,h);  //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp2);
		FILE *fp3;
                fp3 = fopen(argv[3],"r"); //Open the third FILE for reading
                h = getWordPairs(fp3,h);  //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp3);

                if(DEBUG == 1) {printf("SIZE of hash is: %d\n",SIZE * h->entry[0]->multiplier);}

                int activeBuckets = 0;                          //Keep track of how many wordPair buckets are in the Hash
                int hashSize = SIZE * h->entry[0]->multiplier;  //Get the current size of the Hash
                for(int i=1;i < hashSize;i++) {   //Count the active wordPair Buckets
                        Bucket *temp = h->entry[i];
                        while(temp != NULL) {
                                activeBuckets++;
                                temp = temp->next;
                        }
                }
                if(DEBUG == 1) {printf("activebuckets = %d\n",activeBuckets);}

                h = sortHash(h,hashSize,activeBuckets,-1); //Sort and print the wordPairs stored in the Hash
                freeHash(h,hashSize);                      //Free the memory allocated for the Hash
	}
	else if(argv[3] && argv[2] && argv[1][0] == '-') { //If there is a 3 cmdLine args with count(wordpairs -count fileName1 fileName2)
		assert(argv[1]);
		assert(argv[2]);
		assert(argv[3]);
		int pairCount = 0; //Initialize word pairs to count to 0
                pairCount = atoi(argv[1]); //Convert char * count to int count
                pairCount = pairCount * -1; //Turn count into positive number
                if(DEBUG == 1) {printf("paircount = %d\n",pairCount);}  //Test

                Hash *h = (Hash*) malloc(sizeof(Hash));             //Create hash w/ SIZE buckets initially,x3 after collisions
                Bucket *cBucket = (Bucket*) malloc(sizeof(Bucket)); //Create collision/multiplier bucket to keep track of collisions
                cBucket->wordPair = "collisions";
                cBucket->collisions = 0;
                cBucket->multiplier = 1;
                h->entry[0] = cBucket;

                FILE *fp;
                fp = fopen(argv[2],"r");  //Open the first FILE for reading
                h = getWordPairs(fp,h);   //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
		fclose(fp);
		FILE *fp2;
                fp2 = fopen(argv[3],"r"); //Open the second FILE for reading
                h = getWordPairs(fp2,h);  //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp2);

                if(DEBUG == 1) {printf("SIZE of hash is: %d\n",SIZE * h->entry[0]->multiplier);}

                int activeBuckets = 0;                          //Keep track of how many wordPair buckets are in the Hash
                int hashSize = SIZE * h->entry[0]->multiplier;  //Get the current size of the Hash
                for(int i=1;i < hashSize;i++) {   //Count the active wordPair Buckets
                        Bucket *temp = h->entry[i];
                        while(temp != NULL) {
                                activeBuckets++;
                                temp = temp->next;
                        }
                }
                if(DEBUG == 1) {printf("activebuckets = %d\n",activeBuckets);}

                h = sortHash(h,hashSize,activeBuckets,pairCount); //Sort and print the wordPairs stored in the Hash
                freeHash(h,hashSize);                      //Free the memory allocated for the Hash
	}
	else if(argv[2] && argv[1][0] != '-') { //If there is 2 cmd line args without count(wordpairs fileName1 fileName2)
		assert(argv[1]);
		assert(argv[2]);
		Hash *h = (Hash*) malloc(sizeof(Hash));             //Create hash w/ SIZE buckets initially,x3 after collisions
                Bucket *cBucket = (Bucket*) malloc(sizeof(Bucket)); //Create collision/multiplier bucket to keep track of collisions
                cBucket->wordPair = "collisions";
                cBucket->collisions = 0;
                cBucket->multiplier = 1;
                h->entry[0] = cBucket;

                FILE *fp;
                fp = fopen(argv[1],"r");  //Open the first FILE for reading
                h = getWordPairs(fp,h);   //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
		fclose(fp);
		FILE *fp2;
                fp2 = fopen(argv[2],"r"); //Open the second FILE for reading
                h = getWordPairs(fp2,h);  //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
                fclose(fp2);

                if(DEBUG == 1) {printf("SIZE of hash is: %d\n",SIZE * h->entry[0]->multiplier);}

                int activeBuckets = 0;                          //Keep track of how many wordPair buckets are in the Hash
                int hashSize = SIZE * h->entry[0]->multiplier;  //Get the current size of the Hash
                for(int i=1;i < hashSize;i++) {   //Count the active wordPair Buckets
                        Bucket *temp = h->entry[i];
                        while(temp != NULL) {
                                activeBuckets++;
                                temp = temp->next;
                        }
                }
                if(DEBUG == 1) {printf("activebuckets = %d\n",activeBuckets);}

                h = sortHash(h,hashSize,activeBuckets,-1); //Sort and print the wordPairs stored in the Hash
                freeHash(h,hashSize);                      //Free the memory allocated for the Hash
	}
	else if(argv[1][0] == '-') { //If there is a 2nd cmdline arg (wordpairs -count fileName1)
		assert(argv[1]);
		assert(argv[2]);
		int pairCount = 0; //Initialize word pairs to count to 0
		pairCount = atoi(argv[1]); //Convert char * count to int count
		pairCount = pairCount * -1; //Turn count into positive number
		if(DEBUG == 1) {printf("paircount = %d\n",pairCount);}  //Test	
			
		Hash *h = (Hash*) malloc(sizeof(Hash));             //Create hash w/ SIZE buckets initially,x3 after collisions
                Bucket *cBucket = (Bucket*) malloc(sizeof(Bucket)); //Create collision/multiplier bucket to keep track of collisions
                cBucket->wordPair = "collisions";
                cBucket->collisions = 0;
                cBucket->multiplier = 1;
                h->entry[0] = cBucket;

		FILE *fp;
                fp = fopen(argv[2],"r"); //Open the first FILE for reading
		h = getWordPairs(fp,h);   //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets

                if(DEBUG == 1) {printf("SIZE of hash is: %d\n",SIZE * h->entry[0]->multiplier);}

                int activeBuckets = 0;                          //Keep track of how many wordPair buckets are in the Hash
                int hashSize = SIZE * h->entry[0]->multiplier;  //Get the current size of the Hash
                for(int i=1;i < hashSize;i++) {   //Count the active wordPair Buckets
                        Bucket *temp = h->entry[i];
                        while(temp != NULL) {
                                activeBuckets++;
                                temp = temp->next;
                        }
                }
                if(DEBUG == 1) {printf("activebuckets = %d\n",activeBuckets);}

                h = sortHash(h,hashSize,activeBuckets,pairCount); //Sort and print the wordPairs stored in the Hash
                freeHash(h,hashSize);                      //Free the memory allocated for the Hash
                fclose(fp);                                //Close file pointer
	}
	else if(argv[1][0] != '-') { //If there is a 1st cmdline arg (wordpairs fileName1)
                assert(argv[1]);
		Hash *h = (Hash*) malloc(sizeof(Hash));             //Create hash w/ SIZE buckets initially,x3 after collisions
                Bucket *cBucket = (Bucket*) malloc(sizeof(Bucket)); //Create collision/multiplier bucket to keep track of collisions
                cBucket->wordPair = "collisions";
                cBucket->collisions = 0;
                cBucket->multiplier = 1;
		h->entry[0] = cBucket;

                FILE *fp;
                fp = fopen(argv[1],"r");  //Open the first FILE for reading
                h = getWordPairs(fp,h);   //Get all wordPairs from the FILE inserted into the passed Hash structure as Buckets
		
		if(DEBUG == 1) {printf("SIZE of hash is: %d\n",SIZE * h->entry[0]->multiplier);}

		int activeBuckets = 0;                          //Keep track of how many wordPair buckets are in the Hash
		int hashSize = SIZE * h->entry[0]->multiplier;  //Get the current size of the Hash
		for(int i=1;i < hashSize;i++) {   //Count the active wordPair Buckets
			Bucket *temp = h->entry[i];
			while(temp != NULL) {
				activeBuckets++;
				temp = temp->next;
			}
		}
		if(DEBUG == 1) {printf("activebuckets = %d\n",activeBuckets);}

		h = sortHash(h,hashSize,activeBuckets,-1); //Sort and print the wordPairs stored in the Hash
		freeHash(h,hashSize);                      //Free the memory allocated for the Hash
                fclose(fp);                                //Close file pointer
	}
	return(0);
}

