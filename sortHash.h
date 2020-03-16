#ifndef SORTHASH_H
#define SORTHASH_H

/* Sorts the hash containing buckets with wordpairs by order of occurance using qsort, then prints them out based on count greatest to least. */

Hash *sortHash(Hash *h,int hashSize,int activeBuckets,int numPrint);

#endif
