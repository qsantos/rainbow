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

#include "crack.h"

void HashList_New(struct HashList* hl)
{
	hl->n_hashes = 0;
	hl->a_hashes = 0;
	hl->hashes   = NULL;
}

void HashList_Delete(struct HashList* hl)
{
	for (size_t i = 0; i < hl->n_hashes; i++)
		free(hl->hashes[i]->candidates);
	free(hl->hashes);
}

void HashList_Add(struct HashList* hl, const char hash[16])
{
	if (hl->n_hashes >= hl->a_hashes)
	{
		hl->a_hashes = hl->a_hashes ? 2*hl->a_hashes : 1;
		hl->hashes   = (struct Hash*) realloc(hl->hashes, sizeof(struct Hash) * hl->n_hashes);
		assert(hl->hashes);
	}

	struct Hash* last = hl->hashes[hl->n_hashes++];
	last->reverse = 0;
	memcpy(last->hash, hash, 16);
	last->candidates = NULL;
}

void HashList_Proceed(struct HashList* hl)
{
	// for each table
	{
		// load table
		// unload table
	}
}

static void computeCandidate(struct Hash* hash, size_t firstStep, size_t lastStep)
{
	if (hash->n_candidates >= hash->a_candidates)
	{
		hash->a_candidates = hash->a_candidates ? hash->2*a_candidates : 1;
		hash->candidates   = (char**) realloc(hash->candidates, hash->a_candidates*sizeof(char*));
		assert(hash->candidates);
	}

	char* candidate = malloc(16); // TODO
	assert(candidate);

	char* bufstr = malloc(l_string); // TODO
	assert(bufstr);

	memcpy(candidate, hash->hash, 16); // TODO
	for (size_t step = 0; step < n_steps; step++)
	{
		RTable_Reduce(rt, step, candidate, bufstr);
		MD5((u8*) candidate, (u8*) bufstr, l_string); // TODO
	}

	free(bufstr);

	hash->candidates[hash->n_candidates++] = candidate;
}

char Hash_Reverse(struct Hash* hash, RTable* rt)
{
	// lazy-evaluation of candidates
	for (size_t i = hash->n_candidates; i < rt->l_chains)
		computeCandidate(hash, i);

	// test for every distance to the end point
	for (size_t firstStep = rt->l_chains; firstStep >= 1; firstStep--)
	{
		// find the hash's chain
		s32 res = binaryFind(rt, rt->bufhash);
		if (res < 0)
			continue;

		// get the previous string
		memcpy(rt->bufstr, CSTR(res), rt->l_string);
		MD5((u8*) rt->bufhash, (u8*) rt->bufstr, rt->l_string);
		for (u32 step = 1; step < firstStep; step++)
		{
			RTable_Reduce(rt, step, rt->bufhash, rt->bufstr);
			MD5((u8*) rt->bufhash, (u8*) rt->bufstr, rt->l_string);
		}

		// check for its hash
		if (memcmp(rt->bufhash, hash, rt->l_hash) == 0)
		{
			if (dst)
				memcpy(dst, rt->bufstr, rt->l_string);
			return 1;
		}
	}
	return 0;
}
