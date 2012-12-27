#ifndef RAINBOW_H
#define RAINBOW_H

#include <stdio.h>

// a chain is made of a status byte, a hash and a string
// the length of a chain is therefore 1+l_hash+l_string (sizeofChain)

typedef unsigned char u8;
typedef unsigned long u32;
typedef signed   long s32;

typedef struct
{
	u32   n_chains;    // number of currently active chains
	u32   a_chains;    // space available for 'a_chains' chains
	u32   sizeofChain; // memory size of a chain
	char* chains;      // data (chain array)

	u32   l_hash;      // hash length
	u32   l_string;    // string length
	char* charset;     // character set
	u32   n_charset;   // character count
	u32   l_chains;    // chain length

	char* curstr;      // current starting point
	char* bufstr;      // temporary string
	char* bufhash;     // temporary hash
	char* bufchain;    // temporary chain
} RTable;

// generation
void RTable_New      (RTable* rt, u32 length, const char* chars, u32 depth, u32 count);
void RTable_Delete   (RTable* rt);
char RTable_AddChain (RTable* rt, const char* hash, const char* str);
char RTable_FindChain(RTable* rt);
void RTable_Sort     (RTable* rt);

// loading and storing
void RTable_ToFile  (RTable* rt, const char* filename);
char RTable_FromFile(RTable* rt, const char* filename);

// misc
void RTable_Print    (RTable* rt);
char RTable_Reverse  (RTable* rt, const char* hash, char* dst);

// internal use
void RTable_Reduce(RTable* rt, u32 step, const char* hash, char* str); // hash to string reduce function
void RTable_QSort (RTable* rt, u32 left, u32 right);                   // quick sort
s32  RTable_BFind (RTable* rt, const char* hash);                      // binary search
u32  RTable_HFind (RTable* rt, const char* str);                       // hash table search

// useful functions
char bstrncmp   (const char* a, const char* b, u32 n);
void hex2hash   (const char* hex, char* hash, u32 l_hash);
void printHash  (const char* hash, u32 l_hash);
void printString(const char* str, u32 l_string);

#endif
