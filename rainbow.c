#include "rainbow.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "md5.h"

// chain parameters access
#define  CACTIVE(I) (rt->chains  [ (I)*rt->sizeofChain  ] )
#define    CHASH(I) (rt->chains  + (I)*rt->sizeofChain  + 1)
#define     CSTR(I) (rt->chains  + (I)*rt->sizeofChain  + 1 + rt->l_hash)

void RTable_New(RTable* rt, u32 l_string, const char* charset, u32 s_reduce, u32 l_chains, u32 a_chains)
{
	rt->n_chains = 0;

	rt->l_hash = 16;
	rt->l_string = l_string;
	rt->sizeofChain = 1 + rt->l_hash + rt->l_string;

	rt->charset   = strdup(charset);
	rt->n_charset = strlen(charset);
	rt->s_reduce  = s_reduce;
	rt->l_chains  = l_chains;
	rt->a_chains  = a_chains;

	rt->chains   = (char*) malloc(rt->sizeofChain * rt->a_chains);
	rt->curstr   = (char*) malloc(rt->l_string);
	rt->bufstr   = (char*) malloc(rt->l_string);
	rt->bufhash  = (char*) malloc(rt->l_hash);
	rt->bufchain = (char*) malloc(rt->sizeofChain);

	assert(rt->chains);
	assert(rt->curstr);
	assert(rt->bufstr);
	assert(rt->bufhash);
	assert(rt->bufchain);

	memset(rt->chains, 0, rt->sizeofChain * rt->a_chains);
}

void RTable_Delete(RTable* rt)
{
	free(rt->bufchain);
	free(rt->bufhash);
	free(rt->bufstr);
	free(rt->curstr);
	free(rt->chains);
}

// Hash table implementation
static u32 HashFun(const char* str, u32 len);
static u32 RTable_HFind(RTable* rt, const char* str)
{
	u32 cur = HashFun(str, rt->l_string) % rt->a_chains;
	while (CACTIVE(cur) && bstrncmp(CHASH(cur), str, rt->l_string) != 0)
		if (++cur >= rt->a_chains)
			cur = 0;
	return cur;
}
char RTable_AddChain(RTable* rt, const char* hash, const char* str)
{
	// collision detection
	u32 htid = RTable_HFind(rt, hash);
	if (!CACTIVE(htid))
	{
		CACTIVE(htid) = 1;
		memcpy(CHASH(htid), hash, rt->l_hash);
		memcpy(CSTR (htid), str,  rt->l_string);
		rt->n_chains++;
		return 1;
	}
	return 0;
}

char RTable_StartAt(RTable* rt, u64 index)
{
	// pick a starting point
	if (!index2key(index, rt->curstr, rt->l_string, rt->l_string, rt->charset, rt->n_charset))
		return -1;

	// start a new chain from 'str'
	MD5((u8*) rt->bufhash, (u8*) rt->curstr, rt->l_string);
	for (u32 step = 1; step < rt->l_chains; step++)
	{
		RTable_Reduce(rt, step, rt->bufhash, rt->bufstr);
		MD5((u8*) rt->bufhash, (u8*) rt->bufstr, rt->l_string);
	}

	return RTable_AddChain(rt, rt->bufhash, rt->curstr);
}

static void swap(RTable* rt, u32 a, u32 b)
{
	memcpy(rt->bufchain,                   rt->chains + a*rt->sizeofChain, rt->sizeofChain);
	memcpy(rt->chains + a*rt->sizeofChain, rt->chains + b*rt->sizeofChain, rt->sizeofChain);
	memcpy(rt->chains + b*rt->sizeofChain, rt->bufchain,                   rt->sizeofChain);
}
static void quicksort(RTable* rt, u32 left, u32 right)
{
	if (left >= right)
		return;

	swap(rt, (left+right)/2, right);
	char* pivotValue = CHASH(right);
	u32 storeIndex = left;
	for (u32 i = left; i < right; i++)
		if (bstrncmp(CHASH(i), pivotValue, rt->l_hash) < 0)
			swap(rt, i, storeIndex++);

	swap(rt, storeIndex, right);

	if (storeIndex)
		quicksort(rt, left, storeIndex-1);
	quicksort(rt, storeIndex+1, right);
}

void RTable_Sort(RTable* rt)
{
	quicksort(rt, 0, rt->a_chains-1);
}

void RTable_ToFile(RTable* rt, const char* filename)
{
	FILE* f = filename ? fopen(filename, "w") : stdout;
	if (!f)
	{
		fprintf(stderr, "Could not open '%s'\n", filename);
		exit(1);
	}

	RTF_header h =
	{
		0x00305254,
		rt->l_string,
		rt->s_reduce,
		rt->l_chains,
		rt->n_chains,
		rt->n_charset
	};
	fwrite(&h,          sizeof(RTF_header), 1,             f);
	fwrite(rt->charset, 1,                  rt->n_charset, f);
	fwrite(rt->curstr,  1,                  rt->l_string,  f);
	fwrite(rt->chains,  rt->sizeofChain,    rt->a_chains,  f);

	fclose(f);
}

char RTable_FromFile(RTable* rt, const char* filename)
{
	FILE* f = filename ? fopen(filename, "r") : stdin;
	if (!f)
		return 0;

	RTF_header h;
	fread(&h, sizeof(RTF_header), 1, f);

	// TODO : avoid allocating and freeing charset
	char* charset = malloc(h.n_charset + 1);
	assert(charset);
	fread(charset, 1, h.n_charset, f);
	charset[h.n_charset] = 0;

//	if (rt) RTable_Delete(rt); // TODO
	RTable_New(rt, h.l_string, charset, h.s_reduce, h.l_chains, h.n_chains);
	free(charset);

	fread(rt->curstr, 1,               rt->l_string, f);
	fread(rt->chains, rt->sizeofChain, rt->a_chains, f);
	rt->n_chains = 0;
	for (u32 i = 0; i < rt->a_chains; i++)
		if (CACTIVE(i))
			rt->n_chains++;

	fclose(f);
	return 1;
}

void RTable_Print(RTable* rt)
{
	for (u32 i = 0; i < rt->a_chains; i++)
	{
		printHash(CHASH(i), rt->l_hash);
		printf(" ");
		printString(CSTR(i), rt->l_string);
		printf("\n");
	}
}


static s32 binaryFind(RTable* rt, const char* hash)
{
	u32 start = 0;
	u32 end   = rt->a_chains-1;
	while (start != end)
	{
		u32 middle = (start + end) / 2;
		if (bstrncmp(hash, CHASH(middle), rt->l_hash) <= 0)
			end = middle;
		else
			start = middle + 1;
	}
	if (bstrncmp(CHASH(start), hash, rt->l_hash) == 0)
		return start;
	else
		return -1;
}
char RTable_Reverse(RTable* rt, const char* hash, char* dst)
{
	// test for every distance to the end point
	for (u32 firstStep = rt->l_chains; firstStep >= 1; firstStep--)
	{
		// get the end point hash
		memcpy(rt->bufhash, hash, rt->l_hash);
		for (u32 step = firstStep; step < rt->l_chains; step++)
		{
			RTable_Reduce(rt, step, rt->bufhash, rt->bufstr);
			MD5((u8*) rt->bufhash, (u8*) rt->bufstr, rt->l_string);
		}

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
		if (bstrncmp(rt->bufhash, hash, rt->l_hash) == 0)
		{
			if (dst)
				memcpy(dst, rt->bufstr, rt->l_string);
			return 1;
		}
	}
	return 0;
}

void RTable_Reduce(RTable* rt, u32 step, const char* hash, char* str)
{
	step += rt->s_reduce;
	for (u32 j = 0; j < rt->l_string; j++, str++, hash++, step>>=8)
		*str = rt->charset[(u8)(*hash ^ step) % rt->n_charset];
}

char bstrncmp(const char* a, const char* b, u32 n)
{
	for (u32 i = 0; i < n; i++, a++, b++)
		if (*a != *b)
			return *(u8*)a < *(u8*)b ? -1 : 1;
	return 0;
}

void hex2hash(const char* hex, char* hash, u32 l_hash)
{
	for (u32 i = 0; i < l_hash; i++)
	{
		*hash  = *hex - (*hex <= '9' ? '0' : 87);
		hex++;
		*hash *= 16;
		*hash += *hex - (*hex <= '9' ? '0' : 87);
		hex++;
		hash++;
	}
}

void printHash(const char* hash, u32 l_hash)
{
	for (u32 i = 0; i < l_hash; i++, hash++)
		printf("%.2x", (u8) *hash);
}

void printString(const char* str, u32 l_string)
{
	for (u32 j = 0; j < l_string; j++, str++)
		printf("%c", *str);
}

char index2key(u64 index, char* key, u32 l_min, u32 l_max, const char* charset, u32 n_charset)
{
	u32 l_key = l_min;
	u64 n_key = pow(n_charset, l_min);
	while (index >= n_key)
	{
		index -= n_key;
		n_key *= n_charset;
		l_key++;
	}
	if (l_key > l_max)
		return 0;

	for (u32 i = 0; i < l_key; i++, key++)
	{
		*key = charset[index % n_charset];
		index /= n_charset;
	}
	return 1;
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
static u32 HashFun(const char* str, u32 len)
{
	u32 b    = 378551;
	u32 a    = 63689;
	u32 hash = 0;
	u32 i    = 0;

	for (i = 0; i < len; str++, i++)
	{
		hash = hash * a + (*str);
		a    = a * b;
	}

	return hash;
}
