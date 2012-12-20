#include "rainbow.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "md5.h"

// chain parameters access
// TODO : might be a simpler way...
#define  CACTIVE(I) (rt->chains  [ (I)*rt->sizeofChain  ] )
#define CACTIVE1(I) (rt1->chains [ (I)*rt1->sizeofChain ] )
#define CACTIVE2(I) (rt2->chains [ (I)*rt2->sizeofChain ] )

#define   CHASH(I) (rt->chains  + (I)*rt->sizeofChain  + 1)
#define  CHASH1(I) (rt1->chains + (I)*rt1->sizeofChain + 1)
#define  CHASH2(I) (rt2->chains + (I)*rt2->sizeofChain + 1)

#define    CSTR(I) (rt->chains  + (I)*rt->sizeofChain  + 1 + rt->hlen)
#define   CSTR1(I) (rt1->chains + (I)*rt1->sizeofChain + 1 + rt1->hlen)
#define   CSTR2(I) (rt2->chains + (I)*rt2->sizeofChain + 1 + rt2->hlen)

RTable* RTable_New(unsigned int length, char* chars, unsigned int depth, unsigned int count)
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

void RTable_Delete(RTable* rt)
{
	free(rt->bufchain);
	free(rt->bufhash);
	free(rt->bufstr2);
	free(rt->bufstr1);
	free(rt->chains);
}

char RTable_AddChain(RTable* rt, char* hash, char* str)
{
	// collision detection
	unsigned int htid = RTable_HFind(rt, hash);
	if (!CACTIVE(htid))
	{
		CACTIVE(htid) = 1;
		memcpy(CHASH(htid), hash, rt->hlen);
		memcpy(CSTR (htid), str,  rt->slen);
		rt->n_chains++;
		return 1;
	}
	return 0;
}

void RTable_Transfer(RTable* rt1, RTable* rt2)
{
	for (unsigned int i = 0; i < rt1->a_chains; i++)
		if (CACTIVE1(i))
			RTable_AddChain(rt2, CHASH1(i), CSTR1(i));
}

char RTable_FindChain(RTable* rt)
{
	// pick a starting point
	for (unsigned int i = 0; i < rt->slen; i++)
		rt->bufstr1[i] = rt->charset[random() % rt->clen];

	// start a new chain from 'str'
	MD5((uint8_t*) rt->bufhash, (uint8_t*) rt->bufstr1, rt->slen);
	for (unsigned int step = 1; step < rt->l_chains; step++)
	{
		RTable_Mask(rt, step, rt->bufhash, rt->bufstr2);
		MD5((uint8_t*) rt->bufhash, (uint8_t*) rt->bufstr1, rt->slen);
	}

	return RTable_AddChain(rt, rt->bufhash, rt->bufstr1);
}

void RTable_Sort(RTable* rt)
{
	RTable_QSort(rt, 0, rt->a_chains-1);
}

void RTable_ToFile(RTable* rt, FILE* f)
{
	fwrite(rt->chains, rt->sizeofChain, rt->a_chains, f);
}

RTable* RTable_FromFile(unsigned int slen, char* charset, unsigned int l_chains, FILE* f)
{
	if (ftell(f) != 0)
	{
		fprintf(stderr, "Nothing to read\n");
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	unsigned int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// TODO
	unsigned int hlen = 16;
	unsigned int sizeofChain = 1 + hlen + slen;
	if (size % sizeofChain)
	{
		fprintf(stderr, "Invalid file\n");
		exit(1);
	}
	unsigned int a_chains = size / sizeofChain;

	RTable* rt = RTable_New(slen, charset, l_chains, a_chains);
	fread(rt->chains, rt->sizeofChain, rt->a_chains, f);
	rt->n_chains = 0;
	for (unsigned int i = 0; i < rt->a_chains; i++)
		if (CACTIVE(i))
			rt->n_chains++;
	printf("%u chains loaded\n", rt->n_chains);
	return rt;
}

void RTable_ToFileN(RTable* rt, const char* filename)
{
	FILE* f = filename ? fopen(filename, "w") : stdout;
	if (!f)
	{
		fprintf(stderr, "Could not open '%s'\n", filename);
		exit(1);
	}
	RTable_ToFile(rt, f);
	fclose(f);
}

RTable* RTable_FromFileN(unsigned int slen, char* charset, unsigned int l_chains, const char* filename)
{
	FILE* f = filename ? fopen(filename, "r") : stdin;
	if (!f)
		return NULL;
	RTable* rt = RTable_FromFile(slen, charset, l_chains, f);
	fclose(f);
	return rt;
}

RTable* RTable_Merge(RTable* rt1, RTable* rt2)
{
	assert(rt1->hlen == rt2->hlen);
	assert(rt1->slen == rt2->slen);
	assert(rt1->clen == rt2->clen);
	assert(rt1->l_chains == rt2->l_chains);

	assert(rt1->a_chains == rt1->n_chains);
	assert(rt2->a_chains == rt2->n_chains);

	RTable* rt = RTable_New(rt1->slen, rt1->charset, rt1->l_chains, rt1->a_chains + rt2->a_chains);
	unsigned int i1 = 0;
	unsigned int i2 = 0;
	unsigned int i  = 0;
	while (i1 < rt1->a_chains || i2 < rt2->a_chains)
	{
		int c = bstrncmp(CHASH1(i1), CHASH2(i2), rt1->hlen);
		if (c < 0)
		{
			memcpy(rt->chains + i*rt->sizeofChain, rt1->chains + i1*rt1->sizeofChain, rt->sizeofChain);
			i1++;
		}
		else if (c > 0)
		{
			memcpy(rt->chains + i*rt->sizeofChain, rt2->chains + i1*rt2->sizeofChain, rt->sizeofChain);
			i2++;
		}
		else
		{
			memcpy(rt->chains + i*rt->sizeofChain, rt1->chains + i1*rt1->sizeofChain, rt->sizeofChain);
			i1++;
			i2++;
		}
		i++;
	}
	rt->n_chains = i;
	rt->a_chains = i; // reduce the matrix which will be stored
	return rt;
}

void RTable_Print(RTable* rt)
{
	for (unsigned int i = 0; i < rt->a_chains; i++)
	{
		printHash(CHASH(i), rt->hlen);
		printf(" ");
		printString(CSTR(i), rt->slen);
		printf("\n");
	}
}

char RTable_Reverse(RTable* rt, char* target, char* dest)
{
	// test for every distance to the end point
	for (unsigned int firstStep = rt->l_chains; firstStep >= 1; firstStep--)
	{
		// get the end point hash
		memcpy(rt->bufhash, target, rt->hlen);
		for (unsigned int step = firstStep; step < rt->l_chains; step++)
		{
			RTable_Mask(rt, step, rt->bufhash, rt->bufstr1);
			MD5((uint8_t*) rt->bufhash, (uint8_t*) rt->bufstr1, rt->slen);
		}

		// find the hash's chain
		int res = RTable_BFind(rt, rt->bufhash);
		if (res < 0)
			continue;

		// get the previous string
		memcpy(rt->bufstr1, CSTR(res), rt->slen);
		MD5((uint8_t*) rt->bufhash, (uint8_t*) rt->bufstr1, rt->slen);
		unsigned int step = 1;
		while (step < rt->l_chains && bstrncmp(rt->bufhash, target, rt->hlen) != 0)
		{
			RTable_Mask(rt, step++, rt->bufhash, rt->bufstr1);
			MD5((uint8_t*) rt->bufhash, (uint8_t*) rt->bufstr1, rt->slen);
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

void RTable_Mask(RTable* rt, unsigned int step, char* hash, char* str)
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
void RTable_QSort(RTable* rt, unsigned int left, unsigned int right)
{
	if (left >= right)
		return;

	swap(rt, (left+right)/2, right);
	char* pivotValue = CHASH(right);
	unsigned int storeIndex = left;
	for (unsigned int i = left; i < right; i++)
		if (bstrncmp(CHASH(i), pivotValue, rt->hlen) < 0)
			swap(rt, i, storeIndex++);

	swap(rt, storeIndex, right);

	if (storeIndex)
		RTable_QSort(rt, left, storeIndex-1);
	RTable_QSort(rt, storeIndex+1, right);
}

int RTable_BFind(RTable* rt, char* hash)
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
unsigned int RTable_HFind(RTable* rt, char* str)
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
