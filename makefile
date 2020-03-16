GET_WORD := /home/erik/Desktop/Wordpairs-with-Dynamic-Hashing/getWord

wordpairs : main.o crc64.o sortHash.o
	cc -o wordpairs main.o crc64.o sortHash.o
main.o : main.c 
	cc -c main.c $(GET_WORD)/include/getWord.c $(GET_WORD)/include/getWord.h -I $(GET_WORD)/include
crc64.o : crc64.c crc64.h
	cc -c crc64.c
sortHash.o : sortHash.c sortHash.h
	cc -c sortHash.c
clean :
	rm wordpairs main.o crc64.o sortHash.o
