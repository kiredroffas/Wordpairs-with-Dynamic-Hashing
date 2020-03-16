/* Erik Safford
 * Word Pair Counting using a Dynamic Hash
 * CS 360 - Spring 2019 */

#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include "bucket.h"
#include "hash.h"
#include "sortHash.h"

/*Pointer magic to sort the array of Buckets by occurance value */
int compare(const void *a,const void *b) {
	Bucket *BucketA = (Bucket *)a;
	Bucket *BucketB = (Bucket *)b;

	return( BucketB->value - BucketA->value);
}

/* sortHash() is called after getWordPairs() has finished and returned a Hash filled with wordPair Bucket structures, and is passed
 * a Hash structure to sort, the SIZE * multiplier of the Hash, the number of active Buckets in the Hash, and the number of sorted
 * Buckets to print (-1 indicaties that all sorted wordPairs should be printed). It starts by copying the wordPair and occurance information
 * from the Buckets in the Hash to a new Bucket array the size of the number of activeBuckets. This is then used to qsort() the wordPairs
 * by order of occurance value. The specified number of highest occuring wordPairs is then printed.  */
Hash *sortHash(Hash *h,int hashSize,int activeBuckets,int numPrint) {
	Bucket *list = (Bucket*) malloc(sizeof(Bucket) * activeBuckets);  //Malloc a new array of Buckets the size of activeBuckets
	
	int count = 0;
	Bucket *temp;
	for(int i=1;i < hashSize;i++) {  //Copy over the wordPair/occurance information into the new list Bucket array
                while(count < activeBuckets) {
                        temp = h->entry[i];

                        while(temp != NULL) {
                                list[count].wordPair = temp->wordPair;
				list[count].value = temp->value;
                                count++;
                                temp = temp->next;
                        }
                        break;
                }
        }

	qsort(list,activeBuckets,sizeof(Bucket),compare);  //Sort the Bucket list by order of occurance value
	
	if(numPrint != -1) {  //If a specified number of wordPair occurances should be printed
		if(numPrint > activeBuckets) { //If the specified number to print is more then the number of wordPairs
			fprintf(stderr,"stderr: Invalid print count, count is higher then amount of wordPairs\n");
			exit(-1);
		}
		for(int i=0; i < numPrint;i++) { //Print numPrint wordPairs
                	printf("%10d %s\n",list[i].value,list[i].wordPair);
        	}
	}
	else { //Else if all sorted wordPair occurances should be printed
		for(int i=0; i < activeBuckets ;i++) { //Print all the wordPairs 
                        printf("%10d %s\n",list[i].value,list[i].wordPair);
                }
	}
	
	free(list);

	return(h);
}
