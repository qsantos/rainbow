#include "rainbow.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "md5.h"

// a chain is made of a status byte, a hash and a string
// the length of a chain is therefore 1+hlen+slen (sizeofChain)

// chain parameters access
#define CACTIVE(I) (rt->chains [ (I)*rt->sizeofChain ] )
#define   CHASH(I) (rt->chains + (I)*rt->sizeofChain + 1)
#define    CSTR(I) (rt->chains + (I)*rt->sizeofChain + 1 + rt->hlen)

RTable* Rainbow_New(unsigned int length, char* chars, unsigned int depth, unsigned int count)
{
	RTable* rt = (RTable*) malloc(sizeof(RTable));

	rt->n_chains = 0;

	rt->hlen = 16;
	rt->slen = length;
	rt->sizeofChain = 1 + rt->hlen + rt->slen;

	rt->charset  = strdup(chars);
	rt->clen     = strlen(chars);
	rt->l_chains = depth;
	rt->a_chains = count;

	rt->chains   = (char*) malloc(rt->sizeofChain * rt->a_chains);
	rt->bufstr1  = (char*) malloc(rt->slen);
	rt->bufstr2  = (char*) malloc(rt->slen);
	rt->bufhash  = (char*) malloc(rt->hlen);
	rt->bufchain = (char*) malloc(rt->sizeofChain);

	assert(rt->chains);
	assert(rt->bufstr1);
	assert(rt->bufstr2);
	assert(rt->bufhash);
	assert(rt->bufchain);

	memset(rt->chains, 0, rt->sizeofChain * rt->a_chains);

	return rt;
}

void Rainbow_Delete(RTable* rt)
{
	free(rt->bufchain);
	free(rt->bufhash);
	free(rt->bufstr2);
	free(rt->bufstr1);
	free(rt->chains);
}

char Rainbow_FindChain(RTable* rt)
{
	// pick a starting point
	for (unsigned int i = 0; i < rt->slen; i++)
		rt->bufstr1[i] = rt->charset[random() % rt->clen];

	// start a new chain from 'str'
	MD5((uint64_t) rt->slen, (uint8_t*) rt->bufstr1, (uint8_t*) rt->bufhash);
	for (unsigned int step = 1; step < rt->l_chains; step++)
	{
		Rainbow_Mask(rt, step, rt->bufhash, rt->bufstr2);
		MD5((uint64_t) rt->slen, (uint8_t*) rt->bufstr2, (uint8_t*) rt->bufhash);
	}

	// collision detection
	unsigned int htid = Rainbow_HFind(rt, rt->bufhash);
	if (!CACTIVE(htid))
	{
		CACTIVE(htid) = 1;
		memcpy(CHASH(htid), rt->bufhash, rt->hlen);
		memcpy(CSTR (htid), rt->bufstr1, rt->slen);
		rt->n_chains++;
		return 1;
	}

	return 0;
}

void Rainbow_Sort(RTable* rt)
{
	Rainbow_QSort(rt, 0, rt->a_chains-1);
}

void Rainbow_ToFile(RTable* rt, FILE* f)
{
	fwrite(rt->chains, rt->sizeofChain, rt->a_chains, f);
}

void Rainbow_FromFile(RTable* rt, FILE* f)
{
	fread(rt->chains, rt->sizeofChain, rt->a_chains, f);
	rt->n_chains = 0;
	for (unsigned int i = 0; i < rt->a_chains; i++)
		if (CACTIVE(i))
			rt->n_chains++;
}

void Rainbow_Print(RTable* rt)
{
	for (unsigned int i = 0; i < rt->a_chains; i++)
	{
		printHash(CHASH(i), rt->slen);
		printf(" ");
		printString(CSTR(i), rt->hlen);
		printf("\n");
	}
}

char Rainbow_Reverse(RTable* rt, char* target, char* dest)
{
	// test for every distance to the end point
	for (unsigned int firstStep = rt->l_chains; firstStep >= 1; firstStep--)
	{
		// get the end point hash
		memcpy(rt->bufhash, target, rt->hlen);
		for (unsigned int step = firstStep; step < rt->l_chains; step++)
		{
			Rainbow_Mask(rt, step, rt->bufhash, rt->bufstr1);
			MD5((uint64_t) rt->slen, (uint8_t*) rt->bufstr1, (uint8_t*) rt->bufhash);
		}

		// find the hash's chain
		int res = Rainbow_BFind(rt, rt->bufhash);
		if (res < 0)
			continue;

		// get the previous string
		memcpy(rt->bufstr1, CSTR(res), rt->slen);
		MD5((uint64_t) rt->slen, (uint8_t*) rt->bufstr1, (uint8_t*) rt->bufhash);
		unsigned int step = 1;
		while (step < rt->l_chains && bstrncmp(rt->bufhash, target, rt->hlen) != 0)
		{
			Rainbow_Mask(rt, step++, rt->bufhash, rt->bufstr1);
			MD5((uint64_t) rt->slen, (uint8_t*) rt->bufstr1, (uint8_t*) rt->bufhash);
		}
		if (step < rt->l_chains)
		{
			if (dest)
				memcpy(dest, rt->bufstr1, rt->slen);
			return 1;
		}
	}
	return 0;
}

void Rainbow_Mask(RTable* rt, unsigned int step, char* hash, char* str)
{
	for (unsigned int j = 0; j < rt->slen; j++, str++, hash++)
		*str = rt->charset[(unsigned char)(*hash ^ step) % rt->clen];
}

static void swap(RTable* rt, unsigned int a, unsigned int b)
{
	memcpy(rt->bufchain,                   rt->chains + a*rt->sizeofChain, rt->sizeofChain);
	memcpy(rt->chains + a*rt->sizeofChain, rt->chains + b*rt->sizeofChain, rt->sizeofChain);
	memcpy(rt->chains + b*rt->sizeofChain, rt->bufchain,                   rt->sizeofChain);
}
void Rainbow_QSort(RTable* rt, unsigned int left, unsigned int right)
{
	if (left >= right)
		return;

	char* pivotValue = CHASH(right);
	unsigned int storeIndex = left;
	for (unsigned int i = left; i < right; i++)
		if (bstrncmp(CHASH(i), pivotValue, rt->hlen) < 0)
			swap(rt, i, storeIndex++);

	swap(rt, storeIndex, right);

	if (storeIndex)
		Rainbow_QSort(rt, left, storeIndex-1);
	Rainbow_QSort(rt, storeIndex, right);
}

int Rainbow_BFind(RTable* rt, char* hash)
{
	unsigned int start = 0;
	unsigned int end   = rt->a_chains-1;
	while (start != end)
	{
		unsigned int middle = (start + end) / 2;
		if (bstrncmp(hash, CHASH(middle), rt->hlen) <= 0)
			end = middle;
		else
			start = middle + 1;
	}
	if (bstrncmp(CHASH(start), hash, rt->hlen) == 0)
		return start;
	else
		return -1;
}

// Hash table implementation
static unsigned int HashFun(const char* str, unsigned int len);
unsigned int Rainbow_HFind(RTable* rt, char* str)
{
	unsigned int cur = HashFun(str, rt->slen) % rt->a_chains;
	while (CACTIVE(cur) && bstrncmp(CHASH(cur), str, rt->slen) != 0)
		if (++cur >= rt->a_chains)
			cur = 0;
	return cur;
}

char bstrncmp(char* a, char* b, int n)
{
	for (int i = 0; i < n; i++, a++, b++)
		if (*a != *b)
			return *(unsigned char*)a < *(unsigned char*)b ? -1 : 1;
	return 0;
}

void hex2hash(char* hex, char* hash, unsigned int hlen)
{
	for (unsigned int i = 0; i < hlen; i++)
	{
		*hash  = *hex - (*hex <= '9' ? '0' : 87);
		hex++;
		*hash *= 16;
		*hash += *hex - (*hex <= '9' ? '0' : 87);
		hex++;
		hash++;
	}
}

void printHash(char* hash, unsigned int hlen)
{
	for (unsigned int i = 0; i < hlen; i++, hash++)
		printf("%.2x", (unsigned char) *hash);
}

void printString(char* str, unsigned int slen)
{
	for (unsigned int j = 0; j < slen; j++, str++)
		printf("%c", *str);
}


/*
**************************************************************************
*                                                                        *
*          General Purpose Hash Function Algorithms Library              *
*                                                                        *
* Author: Arash Partow - 2002                                            *
* URL: http://www.partow.net                                             *
* URL: http://www.partow.net/programming/hashfunctions/index.html        *
*                                                                        *
* Copyright notice:                                                      *
* Free use of the General Purpose Hash Function Algorithms Library is    *
* permitted under the guidelines and in accordance with the most current *
* version of the Common Public License.                                  *
* http://www.opensource.org/licenses/cpl1.0.php                          *
*                                                                        *
**************************************************************************
*/
static unsigned int HashFun(const char* str, unsigned int len)
{
	unsigned int b    = 378551;
	unsigned int a    = 63689;
	unsigned int hash = 0;
	unsigned int i    = 0;

	for (i = 0; i < len; str++, i++)
	{
		hash = hash * a + (*str);
		a    = a * b;
	}

	return hash;
}
