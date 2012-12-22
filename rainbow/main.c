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
		"  rtnew  n  same as rtgen, ignore existing file\n"
		"  rtres  r  same as rtgen, but no limit\n"
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

	char* charset  = "0123456789abcdefghijklmnopqrstuvwxyz";
	u32 clen        = strlen(charset);

	u32 slen       = atoi(argv[1]);
	u32 l_chains   = atoi(argv[2]);
	char* modestr  = argv[3];

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
//	char* param3 = argc > 6 ? argv[6] : NULL;

	char* str = (char*) malloc(slen);
	char* tmp = (char*) malloc(slen);
	char hash[16];
	srandom(time(NULL));

	RBTable rbt;
	RBTable_New(&rbt, slen, charset, l_chains);

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
		u32 n_chains = 0;
		u32 a_chains = mode == RTRES ? 0 : atoi(param1);
		if (mode != RTNEW)
			n_chains += RBTable_FromFileN(&rbt, param2);

		// generate more chains
		printf("Generating chains\n");
		signal(SIGINT, stopGenerating);
		while (generate && n_chains > a_chains)
		{
			n_chains += RBTable_FindChain(&rbt);
			if (n_chains % 1 == 0)
			{
				rewriteLine();
				printf("%lu chains generated", n_chains);
				fflush(stdout);
			}
			if (mode != RTRES && n_chains >= a_chains)
				generate = 0;
		}
		rewriteLine();

		// save table
		printf("Saving to '%s'\n", param2);
		RBTable_ToFileN(&rbt, param2);
		break;

	case TESTS:
		// load table
		RBTable_FromFileN(&rbt, param2);

		printf("Cracking some hashes\n");

		u32 count = 0;
		u32 n_tests = param1 ? atoi(param1) : 1000;
		for (u32 i = 0; i < n_tests; i++)
		{
			// generate random string
			for (u32 j = 0; j < slen; j++)
				str[j] = charset[random() % clen];

			// hash it
			MD5((uint8_t*) hash, (uint8_t*) str, slen);

			// crack the hash
			if (RBTable_Reverse(&rbt, hash, tmp))
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
			printf("  %lu / %lu", count, i+1);
			fflush(stdout);
		}
		printf("\n");

		break;

	case CRACK:
		if (!param1) ERROR("Hash not supplied\n")

		// load table
		RBTable_FromFileN(&rbt, param2);

		// try and crack hash
		hex2hash(param1, hash, 16);
		int res = RBTable_Reverse(&rbt, hash, str);

		if (res)
		{
			printHash(hash, 16);
			printf(" ");
			printString(str, slen);
			printf("\n");
		}
		else
			printf("Could not reverse hash\n");
		break;
	default:
		break;
	}
	RBTable_Delete(&rbt);

	free(tmp);
	free(str);
	return 0;
}
