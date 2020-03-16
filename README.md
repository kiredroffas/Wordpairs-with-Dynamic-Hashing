# Wordpairs-with-Dynamic-Hashing
This C program targeted as the Linux platform reads from one or more files and prints out a list of the most frequently occurring sequential pairs of words and the number of times they occured, in decreasing order of occurance.

Program Interface:
./wordpairs (-count) fileName1 (fileName2) (fileName3) ...

Where: count is the integer number of word pairs to print out and fileNameN are pathnames from which to read words. If no count argument is specified, ALL word pairs are printed to stdout. Tokens enclosed in parenthesis are optional.

All normal output of the list of word pairs and the number of occurances are printed to stdout.

A procedure getWord is used which reads successive words from an open file descriptor.

A hash table is used to store and count occurrences of sequences of words. The hash table keeps track of how full it is and grows itself to a larger size (i.e. more buckets), as needed. The separate chaining technique is used to implement hash table buckets.

The hash table evaluates a measure of its search performance by keeping track of the number of collisions that occur when inserting into the hash. When the collisions reach a certain threshold, the hash table grows its number of buckets by a factor of 3. Growth of the hash table is transparent to the code using the hash table module.

A crc64 hashing function is used to hash the strings that are inserted and looked up in the hash table.

Sorting of the hashed wordpairs is done through the standard library procedure qsort().

When the program outputs word pairs and their occurence counts, one word pair per line is outputed using the format %10d %s\n, where the decimal number is the number of occurrences and the string is the word pair (with one space between the words).

The source code is broken into two main source files: main.c and sortHash.c. Main controls the implementation of the word pair counting, and sortHash controls the implemenation of the word pair sorting with qsort.

The assert() function is used where appropriate.

The program exits with a zero exit code if no errors occur.

A makefile is included to build the program and clean .o files, with the path of the getWord file requiring modification depending on its filepath.

A define DEBUG flag is included to see hashing operations or word pairs, 0 = off, 1 = on.

# Screenshots
![Alt text](/screenshots/sample_text.png?raw=true "Screenshot 1")
![Alt text](/screenshots/output_w_count.png?raw=true "Screenshot 2")
![Alt text](/screenshots/output_wo_count_1.png?raw=true "Screenshot 3")
![Alt text](/screenshots/output_wo_count_2.png?raw=true "Screenshot 4")
![Alt text](/screenshots/output_wo_count_3.png?raw=true "Screenshot 5")
![Alt text](/screenshots/output_wo_count_4.png?raw=true "Screenshot 6")
![Alt text](/screenshots/output_wo_count_5.png?raw=true "Screenshot 7")
