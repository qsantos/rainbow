#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <time.h>

#include "rainbow.h"
#include "md5.h"

#define ERROR(...)                    \
{                                     \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n");        \
	usage(argc, argv);            \
	exit(1);                      \
}

static void rewriteLine(void)
{
	printf("\r\33[K");
}

static void usage(int argc, char** argv)
{
	(void) argc;

	printf
	(
		"Usage: %s slen l_chains mode  [PARAMS..]\n"
		"                        rtgen n_chains   [file]\n"
		"                        rtnew n_chains   [dst]\n"
		"                        rtres            [file]\n"
		"                        rsize n_chains src [dst]\n"
		"                        merge src1 [src2 [dst]]\n"
		"                        tests [n_tests   [src]]\n"
		"                        crack hash       [src]\n"
		"\n"
		"PARAMS:\n"
		"  slen       length of the non-hashed string / key\n"
		"  l_chains   length of the chains to generate\n"
		"  n_chains   the number of chains to be generated\n"
		"  n_tests    the number of tests to perform (default: 1000)\n"
		"  hash       the hash to be reversed\n"
		"  src        file containing a rainbow table (only read)\n"
		"  dst        file containing a rainbow table (only write)\n"
		"  file       file containing a rainbow table (both)\n"
		"\n"
		"mode:"
		"  rtgen  g  starts/resumes the computation of a rainbow table\n"
		"  rtnew  n  starts (only) a new table (ignore existing file)\n"
		"  rtres  r  resumes (only) a table generation (stops if no file)\n"
		"  merge  m  merges two sorted ('Done') tables\n"
		"            EXPERIMENTAL\n"
		"  rsize  s  resizes table to store n_chains (only to a bigger one)\n"
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
	RTRES,
	MERGE,
	RSIZE,
	TESTS,
	CRACK,
} Mode;

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		usage(argc, argv);
		exit(1);
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
	else if (!strcmp(modestr, "rtres") || !strcmp(modestr, "r"))
		mode = RTRES;
	else if (!strcmp(modestr, "merge") || !strcmp(modestr, "m"))
		mode = MERGE;
	else if (!strcmp(modestr, "rsize") || !strcmp(modestr, "s"))
		mode = RSIZE;
	else if (!strcmp(modestr, "tests") || !strcmp(modestr, "t"))
		mode = TESTS;
	else if (!strcmp(modestr, "crack") || !strcmp(modestr, "c"))
		mode = CRACK;
	else
		ERROR("Invalid mode '%s'\n", modestr);

	char* param1 = argc > 4 ? argv[4] : NULL;
	char* param2 = argc > 5 ? argv[5] : NULL;
	char* param3 = argc > 6 ? argv[6] : NULL;

	RTable* rt = NULL;
	char* str = (char*) malloc(slen);
	char* tmp = (char*) malloc(slen);
	char hash[16];
	srandom(time(NULL));

	switch (mode)
	{
	case RTGEN:
	case RTNEW:
	case RTRES:
		if (mode == RTRES)
			param2 = param1;
		else if (!param1)
			ERROR("Missing parameter: number of chains to generate\n");

		// load table
		if (mode != RTNEW)
			rt = RTable_FromFileN(slen, charset, l_chains, param2);

		if (!rt)
		{
			if (mode == RTRES)
				ERROR("No table computation to resume\n")
			else
				rt = RTable_New(slen, charset, l_chains, atoi(param1));
		}

		// generate more chains
		printf("Generating chains\n");
		signal(SIGINT, stopGenerating);
		unsigned int progressStep = rt->a_chains / 10000;
		if (!progressStep) progressStep = 1;
		while (generate && rt->n_chains < rt->a_chains)
		{
			if (RTable_FindChain(rt) && rt->n_chains % progressStep == 0)
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
			RTable_Sort(rt);
			printf("Done\n");
		}
		else
			printf("Pausing table generation (%u chains generated)\n", rt->n_chains);

		// save table
		RTable_ToFileN(rt, param2);
		RTable_Delete(rt);
		break;

	case MERGE:
		fprintf(stderr, "WARNING: this feature is experimental\n");

		if (!param1)
			ERROR("At least the first table must be given in the parameters\n")

		// load tables
		RTable* rt1 = RTable_FromFileN(slen, charset, l_chains, param1);
		RTable* rt2 = RTable_FromFileN(slen, charset, l_chains, param2);

		if (!rt1) ERROR("Could not load first table\n")
		if (!rt2) ERROR("Could not load second table\n")

		// merge tables
		rt = RTable_Merge(rt1, rt2);
		printf("%u chains after merge\n", rt->n_chains);

		// free tables
		RTable_Delete(rt2);
		RTable_Delete(rt1);

		// save merged table
		RTable_ToFileN(rt, param3);
		RTable_Delete(rt);
		break;

	case RSIZE:
		if (!param1) ERROR("The new size and the source table must be provided\n")
		if (!param2) ERROR("At least the source table must be given in the parameters\n")

		RTable* src = RTable_FromFileN(slen, charset, l_chains, param2);
		RTable* dst = RTable_New      (slen, charset, l_chains, atoi(param1));

		if (!src) ERROR("Could no load source table\n")

		RTable_Transfer(src, dst);
		printf("%u chains transfered\n", dst->n_chains);

		RTable_ToFileN(dst, param3);
		RTable_Delete(src);
		RTable_Delete(dst);
		break;

	case TESTS:
		// load table
		rt = RTable_FromFileN(slen, charset, l_chains, param2);
		if (!rt) ERROR("Could no load table\n")

		printf("Cracking some hashes\n");

		unsigned int count = 0;
		unsigned int n_tests = param1 ? atoi(param1) : 1000;
		for (unsigned int i = 0; i < n_tests; i++)
		{
			// generate random string
			for (unsigned int j = 0; j < slen; j++)
				str[j] = charset[random() % clen];

			// hash it
			MD5((uint8_t*) hash, (uint8_t*) str, slen);

			// crack the hash
			if (RTable_Reverse(rt, hash, tmp))
			{
				// check the cracked string
				if (bstrncmp(str, tmp, slen))
				{
					rewriteLine();
					printString(str, slen);
					printf(" != ");
					printString(tmp, 16);
					printf("\n");
					exit(1);
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

		RTable_Delete(rt);
		break;

	case CRACK:
		if (!param1) ERROR("Hash not supplied\n")

		// load table
		rt = RTable_FromFileN(slen, charset, l_chains, param2);
		if (!rt) ERROR("Could no load table\n")

		// try and crack hash
		hex2hash(param1, hash, 16);
		int res = RTable_Reverse(rt, hash, str);

		if (res)
		{
			printHash(hash, 16);
			printf(" ");
			printString(str, slen);
			printf("\n");
		}
		else
			printf("Could not reverse hash\n");

		RTable_Delete(rt);
		break;
	}

	free(tmp);
	free(str);

	return 0;
}
