#ifndef CRACK_H
#define CRACK_H

#include "rtable.h"

// a hash is associated with a chain of successors
// these successors are searched in each table
// the following structures stores:
//   * the status of the hash (reversed or not);
//   * the value of the hash;
//   * its successors (candidates). 
struct Hash
{
	char   reversed;
	char   hash[16];

	size_t n_candidates;
	size_t a_candidates;
	char** candidates;
};

struct HashList
{
	size_t       n_hashes;
	size_t       a_hashes;
	struct Hash* hashes;
};

void HashList_New    (struct HashList* hl);
void HashList_Delete (struct HashList* hl);
void HashList_Add    (struct HashList* hl, const char hash[16]);
void HashList_Proceed(struct HashList* hl);

char Hash_Reverse(struct Hash*, RTable* rt);

#endif
