#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rainbow.h"
#include "md5.h"

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

	unsigned int slen = 4;
	char* charset = "0123456789abcdefghijklmnopqrstuvwxyz";
	unsigned int clen = strlen(charset);
	unsigned int l_chains = 1000;
	unsigned int n_chains = 1000;

	// generate rainbow table
	Rainbow_Init(slen, charset, l_chains, n_chains);
	unsigned int c = 0;
	while (c < n_chains)
		c += Rainbow_FindChain();
	printf("Sorting chains\n");
	Rainbow_Sort();
	printf("Rainbow table generated\n");

	// tests
	printf("Cracking some hashes\n");
	char* str = (char*) malloc(slen);
	memset(str, charset[0], slen);
	str[slen] = 0;
	char hash[16];
	int count = 0;
	for (unsigned int i = 0; i < clen; i++)
	{
		str[0] = charset[i];
		for (unsigned int j = 0; j < clen; j++)
		{
			str[1] = charset[j];
			printf("%i / %i\n", count, i*clen+j);
			MD5(slen, (uint8_t*) str, (uint8_t*) hash);
			if (Rainbow_Reverse(hash, NULL))
				count++;
		}
	}
	printf("%i\n", count);

	free(str);
	Rainbow_Deinit();
	return 0;
}
