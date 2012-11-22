#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include "rainbow.h"
#include "md5.h"

void usage(int argc, char** argv)
{
	(void) argc;

	printf
	(
		"Usage: %s file slen l_chains n_chains mode\n"
		"\n"
		"mode:"
		"  rtgen\n"
		"  tests\n"
		,
		argv[0]
	);
}

int main(int argc, char** argv)
{
	if (argc < 6)
	{
		usage(argc, argv);
		return 1;
	}

	char*        filename = argv[1];
	char*        charset  = "0123456789abcdefghijklmnopqrstuvwxyz";
	unsigned int slen     = atoi(argv[2]);
	unsigned int clen     = strlen(charset);
	unsigned int l_chains = atoi(argv[3]);
	unsigned int n_chains = atoi(argv[4]);
	char*        mode     = argv[5];

	Rainbow_Init(slen, charset, l_chains, n_chains);
	if (!strcmp(mode, "rtgen"))
	{
		// generate more chains
		printf("Generating chains\n");
		unsigned int c = 0;
		while (c < n_chains)
			c += Rainbow_FindChain();

		printf("Sorting table\n");
		Rainbow_Sort();
		printf("Done\n");

		FILE* f = fopen(filename, "w");
		assert(f);
		Rainbow_ToFile(f);
		fclose(f);
	}
	else if (!strcmp(mode, "tests"))
	{
		FILE* f = fopen(filename, "r");
		assert(f);
		Rainbow_FromFile(f);
		fclose(f);

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
	}
	Rainbow_Deinit();
	return 0;
}
