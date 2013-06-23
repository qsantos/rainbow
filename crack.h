/*\
 *  Implementation of rainbow tables for hash cracking
 *  Copyright (C) 2012-2013 Quentin SANTOS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

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
