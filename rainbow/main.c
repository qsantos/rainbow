#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "md5.h"
#include "hash.h"

#define slen 6

typedef struct
{
	char hash[16];
	char str[slen];
} Chain;

int main(int argc, char** argv)
{
	int l_chains = argc >= 2 ? atoi(argv[1]) : 100000;
	int n_chains = argc >= 3 ? atoi(argv[2]) : 300;

	Chain* chains = (Chain*) malloc(sizeof(Chain) * n_chains);
	assert(chains);

	HashTable* ht = HashTable_New(n_chains);

	char* charset = "0123456789abcdefghijklnopqrstuvwxyz";
	int clen = strlen(charset);

	char str[slen+1];
	str[slen] = 0;

	char hash[17];
	hash[16] = 0;

	int n_foundChains = 0;
	srandom(42);
	while (n_foundChains < n_chains)
	{
		// pick a starting point
		for (int i = 0; i < slen; i++)
			str[i] = charset[random() % clen];

		// start a new chain from 'str'
		char collision = 0;
		for (int step = 0; step < l_chains; step++)
		{
			MD5((uint64_t) slen, (uint8_t*) str, (uint8_t*) hash);

			// collision detection
			if (HashTable_Exists(ht, hash))
			{
				printf("%i: %s ", n_foundChains, str);
				for (int i = 0; i < 16; i++)
					printf("%.2x", (unsigned char) hash[i]);
				printf("\n");
				collision = 1;
				break;
			}

			// "mask function"
			for (int j = 0; j < slen; j++)
				str[j] = charset[(unsigned char)(hash[j] ^ step) % clen];

		}

		if (!collision)
		{
			int chainId = HashTable_Find(ht, hash);
			Chain* c = &chains[chainId];
			memcpy(c->hash, hash, 16);
			memcpy(c->str,  str, slen);
			n_foundChains++;
		}
	}

/*
	for (int i = 0; i < n_chains; i++)
	{
		Chain* c = &chains[i];
		for (int j = 0; j < 16; j++)
			printf("%.2x", (unsigned char) c->hash[j]);
		printf(" ");
		for (int j = 0; j < slen; j++)
			printf("%c", c->str[j]);
		printf("\n");
	}
*/

	HashTable_Delete(ht);
	free(chains);
	return 0;
}
