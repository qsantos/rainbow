#ifndef RAINBOW_H
#define RAINBOW_H

#include <stdio.h>

// a chain is made of a status byte, a hash and a string
// the length of a chain is therefore 1+hlen+slen (sizeofChain)

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

	char* bufstr1;
	char* bufstr2;
	char* bufhash;
	char* bufchain;
} RTable;

// generation
RTable* Rainbow_New      (unsigned int length, char* chars, unsigned int depth, unsigned int count);
void    Rainbow_Delete   (RTable* rt);
char    Rainbow_AddChain (RTable* rt, char* hash, char* str);
void    Rainbow_Transfer (RTable* rt1, RTable* rt2);
char    Rainbow_FindChain(RTable* rt);
void    Rainbow_Sort     (RTable* rt);

// loading and storing
void    Rainbow_ToFile   (RTable* rt, FILE* f);
RTable* Rainbow_FromFile (unsigned int slen, char* charset, unsigned int l_chains, FILE* f);
void    Rainbow_ToFileN  (RTable* rt, const char* filename);
RTable* Rainbow_FromFileN(unsigned int slen, char* charset, unsigned int l_chains, const char* filename);

// misc
RTable* Rainbow_Merge    (RTable* rt1, RTable* rt2);
void    Rainbow_Print    (RTable* rt);
char    Rainbow_Reverse  (RTable* rt, char* target, char* dest);

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
