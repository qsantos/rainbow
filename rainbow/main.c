#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include "rainbow.h"
#include "md5.h"

static void rewriteLine(void)
{
	printf("\r\33[K");
}

static void usage(int argc, char** argv)
{
	(void) argc;

	printf
	(
		"Usage: %s file slen l_chains a_chains mode [PARAM]\n"
		"\n"
		"  file      file where to store/load the rainbow table\n"
		"  slen      length of the non-hashed string / key\n"
		"  l_chains  length of the chains to generate\n"
		"  a_chains  allocate space for 'a_chains' chains\n"
		"\n"
		"mode:"
		"  rtgen  g  starts/resumes the computation of a rainbow table\n"
		"            NOTE: you must use the same parameters for resuming\n"
		"  rtnew  n  forces to start a new rainbow (ignore existing file)\n"
		"  tests  t  runs cracking tests on PARAM random strings (default: 1000)\n"
		"  crack  c  tries to crack PARAM (PARAM is requisite)\n"
		,
		argv[0]
	);
}

static char generate = 1;
static void stopGenerating(int signal)
{
	(void) signal;
	generate = 0;
}

typedef enum
{
	RTGEN,
	RTNEW,
	TESTS,
	CRACK,
} Mode;

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
	unsigned int a_chains = atoi(argv[4]);
	char*        modestr  = argv[5];
	char*        param    = argc >= 7 ? argv[6] : NULL;

	Mode mode;
	if (!strcmp(modestr, "rtgen") || !strcmp(modestr, "g"))
		mode = RTGEN;
	else if (!strcmp(modestr, "rtnew") || !strcmp(modestr, "n"))
		mode = RTNEW;
	else if (!strcmp(modestr, "tests") || !strcmp(modestr, "t"))
		mode = TESTS;
	else if (!strcmp(modestr, "crack") || !strcmp(modestr, "c"))
		mode = CRACK;
	else
	{
		fprintf(stderr, "Invalid mode '%s'\n", modestr);
		return 1;
	}

	Rainbow_Init(slen, charset, l_chains, a_chains);

	FILE* f;
	char* str = (char*) malloc(slen);
	char* tmp = (char*) malloc(slen);
	char hash[16];

	switch (mode)
	{
	case RTGEN:
		f = fopen(filename, "r");
		if (f)
		{
			Rainbow_FromFile(f);
			fclose(f);
		}

	case RTNEW: // skip file loading
		// generate more chains
		printf("Generating chains\n");
		signal(SIGINT, stopGenerating);
		while (generate && n_chains < a_chains)
		{
			Rainbow_FindChain();
			if (n_chains % 1024 == 0)
			{
				rewriteLine();
				printf("Progress: %.2f%%", (float) 100 * n_chains / a_chains);
				fflush(stdout);
			}
		}
		rewriteLine();

		if (generate)
		{
			printf("Sorting table\n");
			Rainbow_Sort();
			printf("Done\n");
		}
		else
			printf("Pausing table generation\n");

		f = fopen(filename, "w");
		assert(f);
		Rainbow_ToFile(f);
		fclose(f);
		break;

	case TESTS:
		f = fopen(filename, "r");
		assert(f);
		Rainbow_FromFile(f);
		fclose(f);

		printf("Cracking some hashes\n");

		unsigned int count = 0;
		srandom(17);
		unsigned int n_tests = param ? atoi(param) : 1000;
		for (unsigned int i = 0; i < n_tests; i++)
		{
			// generate random string
			for (unsigned int j = 0; j < slen; j++)
				str[j] = charset[random() % clen];

			// hash it
			MD5(slen, (uint8_t*) str, (uint8_t*) hash);

			// crack the hash
			if (Rainbow_Reverse(hash, tmp))
			{
				// check the cracked string
				if (bstrncmp(str, tmp, slen))
				{
					rewriteLine();
					printString(str);
					printf(" != ");
					printString(tmp);
					printf("\n");
					return 1;
				}
				count++;
			}

			// progression
			rewriteLine();
			printString(str);
			printf("  %i / %i", count, i+1);
			fflush(stdout);
		}
		printf("\n");

		break;

	case CRACK:
		if (!param)
		{
			usage(argc, argv);
			Rainbow_Deinit();
			return 1;
		}

		f = fopen(filename, "r");
		assert(f);
		Rainbow_FromFile(f);
		fclose(f);

		hex2hash(param, hash);
		int res = Rainbow_Reverse(hash, str);

		if (res)
		{
			printHash(hash);
			printf(" ");
			printString(str);
			printf("\n");
		}
		else
		{
			fprintf(stderr, "Could not reverse hash\n");
			Rainbow_Deinit();
			return 1;
		}

		break;
	}

	free(tmp);
	free(str);
	Rainbow_Deinit();
	return 0;
}
