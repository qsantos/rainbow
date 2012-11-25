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
		"  rtnew  n  forces to start a new table (ignore existing file)\n"
		"  merge  m  merges with another table (PARAM: file2 a_chains2)\n"
		"            NOTE: the two table must be sorted ('Done' message)\n"
		"            EXPERIMENTAL\n"
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
	MERGE,
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
	char*        param1   = argc >= 7 ? argv[6] : NULL;
	char*        param2   = argc >= 8 ? argv[7] : NULL;

	Mode mode;
	if (!strcmp(modestr, "rtgen") || !strcmp(modestr, "g"))
		mode = RTGEN;
	else if (!strcmp(modestr, "rtnew") || !strcmp(modestr, "n"))
		mode = RTNEW;
	else if (!strcmp(modestr, "merge") || !strcmp(modestr, "m"))
		mode = MERGE;
	else if (!strcmp(modestr, "tests") || !strcmp(modestr, "t"))
		mode = TESTS;
	else if (!strcmp(modestr, "crack") || !strcmp(modestr, "c"))
		mode = CRACK;
	else
	{
		fprintf(stderr, "Invalid mode '%s'\n", modestr);
		return 1;
	}

	RTable* rt = Rainbow_New(slen, charset, l_chains, a_chains);

	FILE* f;
	char* str = (char*) malloc(slen);
	char* tmp = (char*) malloc(slen);
	char hash[16];

	switch (mode)
	{
	case RTGEN:
		// load table
		f = fopen(filename, "r");
		if (f)
		{
			Rainbow_FromFile(rt, f);
			fclose(f);
		}

	case RTNEW: // skip file loading
		// generate more chains
		printf("Generating chains\n");
		signal(SIGINT, stopGenerating);
		while (generate && rt->n_chains < a_chains)
		{
			Rainbow_FindChain(rt);
			if (rt->n_chains % 1024 == 0)
			{
				rewriteLine();
				printf("Progress: %.2f%%", (float) 100 * rt->n_chains / a_chains);
				fflush(stdout);
			}
		}
		rewriteLine();

		// finish generation
		if (generate)
		{
			printf("Sorting table\n");
			Rainbow_Sort(rt);
			printf("Done\n");
		}
		else
			printf("Pausing table generation\n");

		// save table
		f = fopen(filename, "w");
		assert(f);
		Rainbow_ToFile(rt, f);
		fclose(f);
		break;

	case MERGE:
		fprintf(stderr, "WARNING: this feature is experimental\n");

		if (!param1 || !param2)
		{
			fprintf(stderr, "Second table information not supplied\n");
			fprintf(stderr, "\n");
			usage(argc, argv);
			return 1;
		}
		unsigned int a_chains2 = atoi(param2);

		// resize global table
		rt->chains = (char*) realloc(rt->chains, (a_chains+a_chains2) * rt->sizeofChain);
		assert(rt->chains);

		// load second table
		f = fopen(param1, "r");
		assert(f);
		fread(rt->chains + a_chains*rt->sizeofChain, rt->sizeofChain, a_chains2, f);
		fclose(f);

		// sort-merge
		a_chains += a_chains2;
		Rainbow_Sort(rt);

		// save merged table
		f = fopen(filename, "w");
		assert(f);
		Rainbow_ToFile(rt, f);
		fclose(f);
		break;

	case TESTS:
		// load table
		f = fopen(filename, "r");
		assert(f);
		Rainbow_FromFile(rt, f);
		fclose(f);

		printf("Cracking some hashes\n");

		unsigned int count = 0;
		srandom(17);
		unsigned int n_tests = param1 ? atoi(param1) : 1000;
		for (unsigned int i = 0; i < n_tests; i++)
		{
			// generate random string
			for (unsigned int j = 0; j < slen; j++)
				str[j] = charset[random() % clen];

			// hash it
			MD5(slen, (uint8_t*) str, (uint8_t*) hash);

			// crack the hash
			if (Rainbow_Reverse(rt, hash, tmp))
			{
				// check the cracked string
				if (bstrncmp(str, tmp, slen))
				{
					rewriteLine();
					printString(str, slen);
					printf(" != ");
					printString(tmp, 16);
					printf("\n");
					return 1;
				}
				count++;
			}

			// progression
			rewriteLine();
			printString(str, slen);
			printf("  %i / %i", count, i+1);
			fflush(stdout);
		}
		printf("\n");

		break;

	case CRACK:
		if (!param1)
		{
			fprintf(stderr, "Hash not supplied\n");
			fprintf(stderr, "\n");
			usage(argc, argv);
			Rainbow_Delete(rt);
			return 1;
		}

		// load table
		f = fopen(filename, "r");
		assert(f);
		Rainbow_FromFile(rt, f);
		fclose(f);

		// try and crack hash
		hex2hash(param1, hash, 16);
		int res = Rainbow_Reverse(rt, hash, str);

		if (res)
		{
			printHash(hash, 16);
			printf(" ");
			printString(str, slen);
			printf("\n");
		}
		else
		{
			fprintf(stderr, "Could not reverse hash\n");
			Rainbow_Delete(rt);
			return 1;
		}

		break;
	}

	free(tmp);
	free(str);
	Rainbow_Delete(rt);
	return 0;
}
