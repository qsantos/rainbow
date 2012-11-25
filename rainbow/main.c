#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <time.h>

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
		"Usage: %s slen l_chains mode [PARAMS [FILE..]]\n"
		"                        rtgen n_chains   [file]\n"
		"                        rtnew n_chains   [file]\n"
		"                        merge src1 [src2 [file]]\n"
		"                        tests [n_tests   [file]]\n"
		"                        crack hash       [file]\n"
		"\n"
		"  slen       length of the non-hashed string / key\n"
		"  l_chains   length of the chains to generate\n"
		"  n_chains   the number of chains to be generated\n"
		"  n_tests    the number of tests to perform (default: 1000)\n"
		"  hash       the hash to be reversed\n"
		"  file       the file where the table is stored\n"
		"  src1,src2  tables to be merged (can be the same as 'file')\n"
		"\n"
		"mode:"
		"  rtgen  g  starts/resumes the computation of a rainbow table\n"
		"  rtnew  n  forces to start a new table (ignore existing file)\n"
		"  merge  m  merges two sorted ('Done') tables\n"
		"            EXPERIMENTAL\n"
		"  tests  t  runs cracking tests on random strings\n"
		"  crack  c  tries to reverse a hash\n"
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

	char*        charset  = "0123456789abcdefghijklmnopqrstuvwxyz";
	unsigned int clen     = strlen(charset);

	unsigned int slen     = atoi(argv[1]);
	unsigned int l_chains = atoi(argv[2]);
	char*        modestr  = argv[3];

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

	char*        param1   = argc > 4 ? argv[4] : NULL;
	char*        param2   = argc > 5 ? argv[5] : NULL;
	char*        param3   = argc > 6 ? argv[6] : NULL;

	RTable* rt = NULL;
	FILE* f;
	char* str = (char*) malloc(slen);
	char* tmp = (char*) malloc(slen);
	char hash[16];
	srandom(time(NULL));

	switch (mode)
	{
	case RTGEN:
	case RTNEW:
		if (!param1) // TODO
		{
			fprintf(stderr, "Missing parameter: number of chains to generate\n");
			return 1;
		}

		// load table
		if (mode != RTNEW)
		{
			f = param2 ? fopen(param2, "r") : stdin;
			if (f)
			{
				rt = Rainbow_FromFile(slen, charset, l_chains, f);
				fclose(f);
			}
		}
		if (!rt)
			rt = Rainbow_New(slen, charset, l_chains, atoi(param1));

		// generate more chains
		printf("Generating chains\n");
		signal(SIGINT, stopGenerating);
		while (generate && rt->n_chains < rt->a_chains)
		{
			Rainbow_FindChain(rt);
			if (rt->n_chains % 1024 == 0)
			{
				rewriteLine();
				printf("Progress: %.2f%%", (float) 100 * rt->n_chains / rt->a_chains);
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
			printf("Pausing table generation (%u chains generated)\n", rt->n_chains);

		// save table
		f = param2 ? fopen(param2, "w") : stdout;
		assert(f);
		Rainbow_ToFile(rt, f);
		fclose(f);
		break;

	case MERGE:
		fprintf(stderr, "WARNING: this feature is experimental\n");

		if (!param1)
		{
			fprintf(stderr, "At least the first table must be given in the parameters\n");
			fprintf(stderr, "\n");
			usage(argc, argv);
			return 1;
		}

		// load first table
		f = fopen(param1, "r");
		assert(f);
		RTable* rt1 = Rainbow_FromFile(slen, charset, l_chains, f);
		fclose(f);
		assert(rt1);

		// load second table
		f = param2 ? fopen(param2, "r") : stdin;
		assert(f);
		RTable* rt2 = Rainbow_FromFile(slen, charset, l_chains, f);
		fclose(f);
		assert(rt2);

		// merge tables
		rt = Rainbow_Merge(rt1, rt2);
		assert(rt);
		printf("%u chains after merge\n", rt->n_chains);

		// free tables
		Rainbow_Delete(rt2);
		Rainbow_Delete(rt1);

		// save merged table
		f = param3 ? fopen(param3, "w") : stdout;
		assert(f);
		Rainbow_ToFile(rt, f);
		fclose(f);
		break;

	case TESTS:
		// load table
		f = param2 ? fopen(param2, "r") : stdin;
		assert(f);
		rt = Rainbow_FromFile(slen, charset, l_chains, f);
		fclose(f);
		assert(rt);

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
			return 1;
		}

		// load table
		f = param2 ? fopen(param2, "r") : stdin;
		assert(f);
		rt = Rainbow_FromFile(slen, charset, l_chains, f);
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
