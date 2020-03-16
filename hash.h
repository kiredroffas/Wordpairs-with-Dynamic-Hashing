#ifndef HASH_H
#define HASH_H
#define SIZE 100


#include <stdio.h>
//Minimun SIZE 50?

/* This Hash structure contains an array of structure bucket's, initially of SIZE 100. The SIZE will grow 
 * by x3 if to many collisions occur. Each bucket contains a character pointer to a wordpair 
 * (two words separated by space), a integer value to hold the number of occurrances of a particular 
 * wordpair, and a bucket structure pointer to the 'next' bucket (separate chaining). Due to the Bucket
 * at entry[0] of the Hash being used as the collision/multiplier sentinel, SIZE of Hash is actually
 * SIZE - 1 for storing wordPairs. */

#endif


typedef struct _hash {
	struct _bucket *entry[SIZE];
} Hash;
