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

static void usage(int argc, char** argv)
{
	(void) argc;

	printf
	(
		"Usage: %s slen l_chains mode  [PARAMS..]\n"
		"                        rsize n_chains src [dst]\n"
		"                        tests [n_tests   [src]]\n"
		"\n"
		"PARAMS:\n"
		"  slen       length of the non-hashed string / key\n"
		"  l_chains   length of the chains to generate\n"
		"  n_chains   the number of chains to be generated\n"
		"  n_tests    the number of tests to perform (default: 1000)\n"
		"  src        file containing a rainbow table (only read)\n"
		"  dst        file containing a rainbow table (only write)\n"
		"\n"
		"mode:"
		"  rsize  s  resizes table to store n_chains (only to a bigger one)\n"
		"  tests  t  runs cracking tests on random strings\n"
		,
		argv[0]
	);
}

int main(int argc, char** argv)
{
	if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v"))
	{
		printf("rainbow\n");
		printf("Compiled on %s at %s\n", __DATE__, __TIME__);
		exit(0);
	}
	else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
	{
		usage(argc, argv);
		exit(0);
	}

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

	RTable* rt = NULL;
	char* str = (char*) malloc(slen);
	char* tmp = (char*) malloc(slen);
	char hash[16];
	srandom(time(NULL));

	switch (mode)
	{
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
				count++;

			// progression
			rewriteLine();
			printString(str, slen);
			printf("  %i / %i", count, i+1);
			fflush(stdout);
		}
		printf("\n");

		RTable_Delete(rt);
		break;
	}

	free(tmp);
	free(str);
	return 0;
}
