#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include "rainbow.h"
#include "md5.h"

char stop = 0;
void signal_handler(int sig)
{
	(void) sig;
	stop = 1;
}

int main(int argc, char** argv)
{
	assert(argc >= 2);

	unsigned int slen = 4;
	char* charset = "0123456789abcdefghijklmnopqrstuvwxyz";
	unsigned int clen = strlen(charset);
	unsigned int l_chains = 1000;
	unsigned int n_chains = 1000;

	// start / resume table
	Rainbow_Init(slen, charset, l_chains, n_chains);
	FILE* f = fopen(argv[1], "r");
	if (f)
	{
		Rainbow_FromFile(f);
		fclose(f);
	}
	signal(SIGINT, signal_handler);

	// generate more chains
	printf("Generating chains\n");
	unsigned int c = 0;
	while (!stop && c < n_chains)
		c += Rainbow_FindChain();

	if (!stop)
	{
		printf("Sorting table\n");
		Rainbow_Sort();
		printf("Done\n");
	}

	f = fopen(argv[1], "w");
	assert(f);
	Rainbow_ToFile(f);
	fclose(f);
	return 0;

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
