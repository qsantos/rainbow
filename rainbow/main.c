#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> // TODO

#include "md5.h"

#define slen 6

typedef struct
{
	char used;
	char hash[16];
	char str[slen];
} Chain;

static char*        charset = "0123456789abcdefghijklmnopqrstuvwxyz";
static int          clen    = 36;
static unsigned int l_chains;
static unsigned int n_chains;
Chain* chains;

void mask(unsigned int step, char* hash, char* str)
{
	for (unsigned int j = 0; j < slen; j++, str++, hash++)
		*str = charset[(unsigned char)(*hash ^ step) % clen];
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
	for (int i = 0; i < 16; i++)
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
	for (int i = 0; i < 16; i++, hash++)
		printf("%.2x", (unsigned char) *hash);
}

void printString(char* str)
{
	for (unsigned int j = 0; j < slen; j++, str++)
		printf("%c", *str);
}

void swap(unsigned int a, unsigned int b)
{
	Chain tmp;
	memcpy(&tmp,       &chains[a], sizeof(Chain));
	memcpy(&chains[a], &chains[b], sizeof(Chain));
	memcpy(&chains[b], &tmp,       sizeof(Chain));
}

void quicksort(unsigned int left, unsigned int right)
{
	if (left >= right)
		return;

	char* pivotValue = chains[right].hash;
	unsigned int storeIndex = left;
	for (unsigned int i = left; i < right; i++)
		if (bstrncmp(chains[i].hash, pivotValue, 16) < 0)
			swap(i, storeIndex++);

	swap(storeIndex, right);

	if (storeIndex)
		quicksort(left, storeIndex-1);
	quicksort(storeIndex, right);
}

int binaryFind(char* hash)
{
	unsigned int start = 0;
	unsigned int end   = n_chains-1;
	while (start != end)
	{
		unsigned int middle = (start + end) / 2;
		if (bstrncmp(hash, chains[middle].hash, 16) <= 0)
			end = middle;
		else
			start = middle + 1;
	}
	if (bstrncmp(chains[start].hash, hash, 16) == 0)
		return start;
	else
		return -1;
}

char reverse(char* target, char* dest)
{
	char str[slen];
	char hash[16];
	int res = 0;
	for (int firstStep = l_chains; firstStep >= 1; firstStep--)
	{
		memcpy(hash, target, 16);
		for (unsigned int step = firstStep; step < l_chains; step++)
		{
			mask(step, hash, str);
			MD5((uint64_t) slen, (uint8_t*) str, (uint8_t*) hash);
		}
		res = binaryFind(hash);

		if (res < 0)
			continue;

		memcpy(str, chains[res].str, slen);
		MD5((uint64_t) slen, (uint8_t*) str, (uint8_t*) hash);
		unsigned int step = 1;
		while (step < l_chains && bstrncmp(hash, target, 16) != 0)
		{
			mask(step++, hash, str);
			MD5((uint64_t) slen, (uint8_t*) str, (uint8_t*) hash);
		}
		if (step < l_chains)
		{
			if (dest)
				memcpy(dest, str, slen);
			return 1;
		}
	}
	return 0;
}

// Hash table implementation
static unsigned int HashFun(const char* str, unsigned int len);
static unsigned int HTFind(char* str)
{
	unsigned int cur = HashFun(str, slen) % n_chains;
	while (chains[cur].used && bstrncmp(chains[cur].hash, str, slen) != 0)
		if (++cur >= n_chains)
			cur = 0;
	return cur;
}

int main(int argc, char** argv)
{
	l_chains = argc >= 2 ? atoi(argv[1]) : 100000;
	n_chains = argc >= 3 ? atoi(argv[2]) : 10000;

	chains = (Chain*) malloc(sizeof(Chain) * n_chains);
	memset(chains, 0, sizeof(Chain) * n_chains);
	assert(chains);

	char SP [slen];
	char str[slen];
	char hash[16];

	unsigned int n_foundChains = 0;
	srandom(42);
	while (n_foundChains < n_chains)
	{
		// pick a starting point
		for (int i = 0; i < slen; i++)
			SP[i] = charset[random() % clen];

		// start a new chain from 'str'
		MD5((uint64_t) slen, (uint8_t*) SP, (uint8_t*) hash);
		for (unsigned int step = 1; step < l_chains; step++)
		{
			mask(step, hash, str);
			MD5((uint64_t) slen, (uint8_t*) str, (uint8_t*) hash);
		}

		// collision detection
		unsigned int htid = HTFind(hash);
		Chain* c = &chains[htid];
		if (!c->used)
		{
			c->used = 1;
			memcpy(c->hash, hash, 16);
			memcpy(c->str,  SP, slen);
			n_foundChains++;
		}
	}

	quicksort(0, n_chains-1);

	printf("Rainbow table generated\n");
	return 0;

	printf("Cracking some hashes\n");

	int count = 0;
	memcpy(str, "aaaaaa", 7);
	for (int i = 0; i < clen; i++)
	{
		str[0] = charset[i];
		for (int j = 0; j < clen; j++)
		{
			str[1] = charset[j];
			printf("%s\n", str);
			MD5(slen, (uint8_t*) str, (uint8_t*) hash);
			if (reverse(hash, NULL))
				count++;
		}
	}
	printf("%i\n", count);

	free(chains);
	return 0;
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
