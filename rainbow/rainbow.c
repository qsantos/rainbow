#include "rainbow.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "md5.h"

// a chain is made of a status byte, a hash and a string
// the length of a chain is therefore 1+hlen+slen (sizeofChain)

// chain parameters access
#define CACTIVE(I) (chains [ (I)*sizeofChain ] )
#define   CHASH(I) (chains + (I)*sizeofChain + 1)
#define    CSTR(I) (chains + (I)*sizeofChain + 1 + hlen)

static unsigned int hlen;
static unsigned int slen;
static unsigned int sizeofChain;
static char*        charset;
static unsigned int clen;
static unsigned int l_chains;
static unsigned int n_chains;
static char*        chains;

static char* bufstr1;
static char* bufstr2;
static char* bufhash;
static char* bufchain;

static void         sort      (unsigned int left, unsigned int right);    // sort the table
static int          binaryFind(char* hash);                               // search hash in table
static void         mask      (unsigned int step, char* hash, char* str); // hash to str ("mask function")
static unsigned int HTFind    (char* str);                                // Hash table implementation

void Rainbow_Init(unsigned int length, char* chars, unsigned int depth, unsigned int count)
{
	hlen = 16;
	slen = length;
	sizeofChain = 1 + hlen + slen;

	charset  = strdup(chars);
	clen     = strlen(chars);
	l_chains = depth;
	n_chains = count;

	chains   = (char*) malloc(sizeofChain   * n_chains);
	bufstr1  = (char*) malloc(slen);
	bufstr2  = (char*) malloc(slen);
	bufhash  = (char*) malloc(hlen);
	bufchain = (char*) malloc(sizeofChain);

	assert(chains);
	assert(bufstr1);
	assert(bufstr2);
	assert(bufhash);

	memset(chains, 0, sizeofChain * n_chains);

	srandom(42);
}

void Rainbow_Deinit(void)
{
	free(bufchain);
	free(bufhash);
	free(bufstr2);
	free(bufstr1);
	free(chains);
}

char Rainbow_FindChain(void)
{
	// pick a starting point
	for (unsigned int i = 0; i < slen; i++)
		bufstr1[i] = charset[random() % clen];

	// start a new chain from 'str'
	MD5((uint64_t) slen, (uint8_t*) bufstr1, (uint8_t*) bufhash);
	for (unsigned int step = 1; step < l_chains; step++)
	{
		mask(step, bufhash, bufstr2);
		MD5((uint64_t) slen, (uint8_t*) bufstr2, (uint8_t*) bufhash);
	}

	// collision detection
	unsigned int htid = HTFind(bufhash);
	if (!CACTIVE(htid))
	{
		CACTIVE(htid) = 1;
		memcpy(CHASH(htid),    bufhash, hlen);
		memcpy(CSTR (htid), bufstr1, slen);
		return 1;
	}

	return 0;
}

void Rainbow_Sort(void)
{
	sort(0, n_chains-1);
}

void Rainbow_ToFile(FILE* f)
{
	fwrite(chains, sizeofChain, n_chains, f);
}

void Rainbow_FromFile(FILE* f)
{
	fread(chains, sizeofChain, n_chains, f);
}

void Rainbow_Print(void)
{
	for (unsigned int i = 0; i < n_chains; i++)
	{
		printHash(CHASH(i));
		printf(" ");
		printString(CSTR(i));
		printf("\n");
	}
}

char Rainbow_Reverse(char* target, char* dest)
{
	int res = 0;
	for (unsigned int firstStep = l_chains; firstStep >= 1; firstStep--)
	{
		memcpy(bufhash, target, hlen);
		for (unsigned int step = firstStep; step < l_chains; step++)
		{
			mask(step, bufhash, bufstr1);
			MD5((uint64_t) slen, (uint8_t*) bufstr1, (uint8_t*) bufhash);
		}
		res = binaryFind(bufhash);

		if (res < 0)
			continue;

		memcpy(bufstr1, CSTR(res), slen);
		MD5((uint64_t) slen, (uint8_t*) bufstr1, (uint8_t*) bufhash);
		unsigned int step = 1;
		while (step < l_chains && bstrncmp(bufhash, target, hlen) != 0)
		{
			mask(step++, bufhash, bufstr1);
			MD5((uint64_t) slen, (uint8_t*) bufstr1, (uint8_t*) bufhash);
		}
		if (step < l_chains)
		{
			if (dest)
				memcpy(dest, bufstr1, slen);
			return 1;
		}
	}
	return 0;
}

char bstrncmp(char* a, char* b, int n)
{
	for (int i = 0; i < n; i++, a++, b++)
		if (*a != *b)
			return *(unsigned char*)a < *(unsigned char*)b ? -1 : 1;
	return 0;
}

void hex2hash(char* hex, char* hash)
{
	for (unsigned int i = 0; i < hlen; i++)
	{
		*hash = *hex - (*hex < '9' ? '0' : 86);
		hex++;
		*hash *= 16;
		*hash += *hex - (*hex < '9' ? '0' : 87);
		hex++;
		hash++;
	}
}

void printHash(char* hash)
{
	for (unsigned int i = 0; i < hlen; i++, hash++)
		printf("%.2x", (unsigned char) *hash);
}

void printString(char* str)
{
	for (unsigned int j = 0; j < slen; j++, str++)
		printf("%c", *str);
}

static void swap(unsigned int a, unsigned int b)
{
	memcpy(bufchain,               chains + a*sizeofChain, sizeofChain);
	memcpy(chains + a*sizeofChain, chains + b*sizeofChain, sizeofChain);
	memcpy(chains + b*sizeofChain, bufchain,               sizeofChain);
}
static void sort(unsigned int left, unsigned int right)
{
	if (left >= right)
		return;

	char* pivotValue = CHASH(right);
	unsigned int storeIndex = left;
	for (unsigned int i = left; i < right; i++)
		if (bstrncmp(CHASH(i), pivotValue, hlen) < 0)
			swap(i, storeIndex++);

	swap(storeIndex, right);

	if (storeIndex)
		sort(left, storeIndex-1);
	sort(storeIndex, right);
}

static int binaryFind(char* hash)
{
	unsigned int start = 0;
	unsigned int end   = n_chains-1;
	while (start != end)
	{
		unsigned int middle = (start + end) / 2;
		if (bstrncmp(hash, CHASH(middle), hlen) <= 0)
			end = middle;
		else
			start = middle + 1;
	}
	if (bstrncmp(CHASH(start), hash, hlen) == 0)
		return start;
	else
		return -1;
}

static void mask(unsigned int step, char* hash, char* str)
{
	for (unsigned int j = 0; j < slen; j++, str++, hash++)
		*str = charset[(unsigned char)(*hash ^ step) % clen];
}

// Hash table implementation
static unsigned int HashFun(const char* str, unsigned int len);
static unsigned int HTFind(char* str)
{
	unsigned int cur = HashFun(str, slen) % n_chains;
	while (CACTIVE(cur) && bstrncmp(CHASH(cur), str, slen) != 0)
		if (++cur >= n_chains)
			cur = 0;
	return cur;
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
