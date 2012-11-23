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
		"  rtgen  starts or resumes the computation of a rainbow table\n"
		"  tests  \n"
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
		{
			c += Rainbow_FindChain();
			if (c % 1024 == 0)
			{
				printf("\rProgress: %.2f%%", (float) 100 * c / n_chains);
				fflush(stdout);
			}
		}
		printf("\r");

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
		char* tmp = (char*) malloc(slen);
		int count = 0;
		srandom(42);
		for (unsigned int i = 0; i < 1000; i++)
		{
			for (unsigned int j = 0; j < slen; j++)
				str[j] = charset[random() % clen];

			char hash[16];
			MD5(slen, (uint8_t*) str, (uint8_t*) hash);
			if (Rainbow_Reverse(hash, tmp))
				count++;

			if (bstrncmp(str, tmp, slen))
			{
				printString(str);
				printf(" != ");
				printString(tmp);
				printf("\n");
				return 1;
			}

			printf("%i / %i\n", count, i+1);
		}
		printf("%i\n", count);

		free(str);
	}
	Rainbow_Deinit();
	return 0;
}
