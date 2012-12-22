#ifndef RAINBOW_H
#define RAINBOW_H

#include <stdio.h>

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

	unsigned int hlen;
	unsigned int slen;
	char*        charset;
	unsigned int clen;
	unsigned int l_chains;

	char* bufstr1;
	char* bufstr2;
	char* bufhash;
} RBTable;

void    RBTable_New      (RBTable* rbt, unsigned int length, char* chars, unsigned int depth);
void    RBTable_Delete   (RBTable* rbt);
RBNode* RBTable_Find     (RBTable* rbt, const char* hash);
RBNode* RBTable_FindNew  (RBTable* rbt, const char* hash);
char    RBTable_AddChain (RBTable* rbt, char* hash, char* str);
char    RBTable_FindChain(RBTable* rbt);
void    RBTable_Mask(RBTable* rbt, unsigned int step, char* hash, char* str);

// useful functions
char bstrncmp   (const char* a, const char* b, int n);
void hex2hash   (char* hex, char* hash, unsigned int hlen);
void printHash  (char* hash, unsigned int hlen);
void printString(char* str, unsigned int slen);

#endif
