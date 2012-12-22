#ifndef RAINBOW_H
#define RAINBOW_H

#include <stdio.h>

typedef unsigned char  u8;
typedef unsigned long u32;

typedef struct RBNode
{
	char           color;
	struct RBNode* parent;
	struct RBNode* left;
	struct RBNode* right;

	char* hash;
	char* str;
} RBNode;

typedef struct
{
	RBNode* root;

	u32 hlen;
	u32 slen;
	char*        charset;
	u32 clen;
	u32 l_chains;

	char* bufstr1;
	char* bufstr2;
	char* bufhash;
} RBTable;

// red black tree handling
void    RBTable_New      (RBTable* rbt, u32 length, const char* chars, u32 depth);
void    RBTable_Delete   (RBTable* rbt);
RBNode* RBTable_Find     (RBTable* rbt, const char* hash);
RBNode* RBTable_FindNew  (RBTable* rbt, const char* hash);

// chain handling
char RBTable_AddChain (RBTable* rbt, const char* hash, const char* str);
char RBTable_FindChain(RBTable* rbt);
char RBTable_Reverse  (RBTable* rbt, const char* hash, char* dst);
void RBTable_Mask     (RBTable* rbt, u32 step, const char* hash, char* str);

// input / output
u32  RBTable_FromFile (RBTable* rbt, FILE* f);
void RBTable_ToFile   (RBTable* rbt, FILE* f);
u32  RBTable_FromFileN(RBTable* rbt, const char* filename);
void RBTable_ToFileN  (RBTable* rbt, const char* filename);

// useful functions
char  bstrncmp   (const char* a, const char* b, int n);
char* bstrndup   (const char* str, u32 len);
void  hex2hash   (const char* hex, char* hash, u32 hlen);
void  printHash  (const char* hash, u32 hlen);
void  printString(const char* str, u32 slen);

#endif
