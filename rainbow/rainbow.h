#ifndef RAINBOW_H
#define RAINBOW_H

#include <stdio.h>

typedef struct
{
	unsigned int n_chains;    // number of currently active chains
	unsigned int a_chains;    // space available for 'a_chains' chains
	unsigned int sizeofChain; // memory size of a chain
	char*        chains;      // data (chain array)

	unsigned int hlen;
	unsigned int slen;
	char*        charset;
	unsigned int clen;
	unsigned int l_chains;

	// TODO
	char* bufstr1;
	char* bufstr2;
	char* bufhash;
	char* bufchain;
} RTable;

// generation
RTable* Rainbow_New      (unsigned int length, char* chars, unsigned int depth, unsigned int count);
void    Rainbow_Delete   (RTable* rt);
char    Rainbow_FindChain(RTable* rt);
void    Rainbow_Sort     (RTable* rt);

// save and load
void Rainbow_ToFile   (RTable* rt, FILE* f);
void Rainbow_FromFile (RTable* rt, FILE* f);

// use
void Rainbow_Print    (RTable* rt);
char Rainbow_Reverse  (RTable* rt, char* target, char* dest);

// internal use
void         Rainbow_Mask (RTable* rt, unsigned int step, char* hash, char* str); // hash to str "mask function"
void         Rainbow_QSort(RTable* rt, unsigned int left, unsigned int right);    // quick sort
int          Rainbow_BFind(RTable* rt, char* hash);                               // binary search
unsigned int Rainbow_HFind(RTable* rt, char* str);                                // hash table search

// useful functions
char bstrncmp   (char* a, char* b, int n);
void hex2hash   (char* hex, char* hash, unsigned int hlen);
void printHash  (char* hash, unsigned int hlen);
void printString(char* str, unsigned int slen);

#endif
