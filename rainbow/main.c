#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "md5.h"

#define slen 6

typedef struct
{
	char used;
	char hash[16];
	char str[slen];
} Chain;

static unsigned int l_chains;
static unsigned int n_chains;
Chain* chains;

char streq(char* a, char* b, int n)
{
	for (int i = 0; i < n; i++, a++, b++)
		if (*a != *b)
			return 0;
	return 1;
}

// Hash table implementation
static unsigned int HashFun(const char* str, unsigned int len);
static unsigned int HTFind(char* str)
{
	unsigned int cur = HashFun(str, slen) % n_chains;
	while (chains[cur].used && !streq(chains[cur].hash, str, slen))
		if (++cur >= n_chains)
			cur = 0;
	return cur;
}

int main(int argc, char** argv)
{
	l_chains = argc >= 2 ? atoi(argv[1]) : 100000;
	n_chains = argc >= 3 ? atoi(argv[2]) : 600;

	chains = (Chain*) malloc(sizeof(Chain) * n_chains);
	memset(chains, 0, sizeof(Chain) * n_chains);
	assert(chains);

	char* charset = "0123456789abcdefghijklnopqrstuvwxyz";
	int clen = strlen(charset);

	char str[slen+1];
	str[slen] = 0;

	char hash[17];
	hash[16] = 0;

	unsigned int n_foundChains = 0;
	srandom(42);
	while (n_foundChains < n_chains)
	{
		// pick a starting point
		for (int i = 0; i < slen; i++)
			str[i] = charset[random() % clen];

		// start a new chain from 'str'
		for (unsigned int step = 0; step < l_chains; step++)
		{
			MD5((uint64_t) slen, (uint8_t*) str, (uint8_t*) hash);

			// "mask function"
			for (unsigned int j = 0; j < slen; j++)
				str[j] = charset[(unsigned char)(hash[j] ^ step) % clen];

		}

		// collision detection
		unsigned int htid = HTFind(hash);
		Chain* c = &chains[htid];
		if (!c->used)
		{
			c->used = 1;
			memcpy(c->hash, hash, 16);
			memcpy(c->str,  str, slen);
			n_foundChains++;
		}
	}

	for (unsigned int i = 0; i < n_chains; i++)
	{
		Chain* c = &chains[i];
		for (unsigned int j = 0; j < 16; j++)
			printf("%.2x", (unsigned char) c->hash[j]);
		printf(" ");
		for (unsigned int j = 0; j < slen; j++)
			printf("%c", c->str[j]);
		printf("\n");
	}

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
