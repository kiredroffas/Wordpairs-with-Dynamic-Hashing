#ifndef BUCKET_H
#define BUCKET_H

#include <stdio.h>

/* This bucket structure contains: a character pointer to a wordpair (two words separated by space),
 * a integer value to hold the number of occurrances of a particular wordpair, a bucket structure
 * pointer to the 'next' bucket (separate chaining). Multiple bucket structures are used to form the
 * hash. If a unique bucket attemps to be stored at a spot in the hash already containing another unique
 * bucket a collision will occur. Collisions encountered while chaining will be tracked in the first 
 * bucket of the hash using a long long int in case file is large. If too many collisions encountered
 * hash will grow by factor of 3. The first Bucket entry of each Hash at entry[0] acts as a sentinel
 * to hold the current number of collisions encountered and current SIZE multiplier.  */

#endif

typedef struct _bucket {
	char *wordPair;
	int value;
	int collisions;
	int multiplier;
	struct _bucket *next;
} Bucket;


