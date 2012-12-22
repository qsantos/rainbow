#ifndef RAINBOW_H
#define RAINBOW_H

#include <stdio.h>

// a chain is made of a status byte, a hash and a string
// the length of a chain is therefore 1+hlen+slen (sizeofChain)

typedef unsigned char u8;
typedef unsigned int  u32;

typedef struct
{
	u32   n_chains;    // number of currently active chains
	u32   a_chains;    // space available for 'a_chains' chains
	u32   sizeofChain; // memory size of a chain
	char* chains;      // data (chain array)

	u32   hlen;
	u32   slen;
	char* charset;
	u32   clen;
	u32   l_chains;

	char* bufstr1;
	char* bufstr2;
	char* bufhash;
	char* bufchain;
} RTable;

// generation
RTable* RTable_New      (u32 length, const char* chars, u32 depth, u32 count);
void    RTable_Delete   (RTable* rt);
char    RTable_AddChain (RTable* rt, const char* hash, const char* str);
void    RTable_Transfer (RTable* rt1, RTable* rt2);
char    RTable_FindChain(RTable* rt, const char* startString);
void    RTable_Sort     (RTable* rt);

// loading and storing
void    RTable_ToFile   (RTable* rt, FILE* f);
RTable* RTable_FromFile (u32 slen, const char* charset, u32 l_chains, FILE* f);
void    RTable_ToFileN  (RTable* rt, const char* filename);
RTable* RTable_FromFileN(u32 slen, const char* charset, u32 l_chains, const char* filename);

// misc
RTable* RTable_Merge    (RTable* rt1, RTable* rt2);
void    RTable_Print    (RTable* rt);
char    RTable_Reverse  (RTable* rt, const char* target, char* dest);

// internal use
void RTable_Reduce(RTable* rt, u32 step, const char* hash, char* str); // hash to string reduce function
void RTable_QSort (RTable* rt, u32 left, u32 right);                   // quick sort
int  RTable_BFind (RTable* rt, const char* hash);                      // binary search
u32  RTable_HFind (RTable* rt, const char* str);                       // hash table search

// useful functions
char bstrncmp   (const char* a, const char* b, int n);
void hex2hash   (const char* hex, char* hash, u32 hlen);
void printHash  (const char* hash, u32 hlen);
void printString(const char* str, u32 slen);

#endif
