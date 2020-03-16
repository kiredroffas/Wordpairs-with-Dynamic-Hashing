/* Erik Safford
 * Wordpair Counting with a Dynamic Hash
 * CS360 - Spring 2019 */

/* This program reads in words from one or more files, and prints out a list of the most frequently occuring sequential pairs of words
 * and the number of times they have occured, in decreasing order of occurance.
 *
 * Program reads the following input:   wordpairs -count fileName1 fileName2 fileName3...
 *                                      wordpairs fileName1 fileName2 fileName3 ...
 *                                           (where count is a positive integer number of wordpairs to count, no count specifies all pairs)
 *
 * This is done through the use of the following structures:
 *
 * typedef struct _hash {
 *      struct _bucket *entry[SIZE];
 * } Hash;
 *
 * typedef struct _bucket {
 *      char *wordPair;
 *      int value;
 *      int collisions;
 *      int multiplier;
 *      struct _bucket *next;
 * } Bucket;
 *
 * The Hash structure is used to store and count occurrences of sequences of words in Buckets (separate chaining), and keeps track of how
 * full it is through a collision/multiplier counter at entry[0] in the Hash. The SIZE of the Hash structure is defined in Hash.h.  Once SIZE/3 collisions
 * occur, the Hash grows itself to a larger size (factor of 3), and rehashes any Buckets that were contained in the old Hash. To determine where in the
 * Hash table a wordPair needs to be entered a crc64 hashing function is utilized. Once a Hash has been filled with wordPair Buckets, the stored
 * wordPairs and occurances are sorted using qsort(). Once sorted, the wordPairs and occurances are printed using the following format: "%10d %s\n".
 * The program is broken into two main files: main.c and sortHash.c, main.c implements the Hash table and sortHash.c implements wordPair counting
 * and sorting. The program manages the heap in a way that attempts to avoid memory leaks, by freeing any allocated memory used in the hashing process.
 * A makefile is included that when executed with no parameters builds a target program "wordpairs". In the makefile, the environment variable GET_WORD
 * is defined as the pathname of a directory which contains directories "include" and "lib" containing getWord.c,getWord.h, and libget.a
 * (as specified by assignment).  */

