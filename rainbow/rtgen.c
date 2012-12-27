#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "rainbow.h"

#define ERROR(...)                    \
{                                     \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n");        \
	usage(argc, argv);            \
	exit(1);                      \
}

static inline void rewriteLine(void)
{
	printf("\r\33[K");
}

static void usage(int argc, char** argv)
{
	(void) argc;

	printf
	(
		"Usage: %s slen l_chains n_chains dst\n"
		"\n"
		"create a new Rainbow Table in dst\n"
		"\n"
		"PARAMS:\n"
		"  slen       length of the non-hashed string / key\n"
		"  l_chains   length of the chains to generate\n"
		"  n_chains   the number of chains to be generated\n"
		"  dst        destination file\n"
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

int main(int argc, char** argv)
{
	if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v"))
	{
		printf("rtgen\n");
		printf("Compiled on %s at %s\n", __DATE__, __TIME__);
		exit(0);
	}
	else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
	{
		usage(argc, argv);
		exit(0);
	}

	if (argc < 5)
	{
		usage(argc, argv);
		exit(1);
	}

	char*        charset  = "0123456789abcdefghijklmnopqrstuvwxyz";
	unsigned int slen     = atoi(argv[1]);
	unsigned int l_chains = atoi(argv[2]);
	unsigned int n_chains = atoi(argv[3]);
	char*        filename = argv[4];

	RTable* rt = NULL;
	srandom(time(NULL));

	// load table
	rt = RTable_FromFileN(slen, charset, l_chains, filename);

	if (!rt)
		rt = RTable_New(slen, charset, l_chains, n_chains);

	// generate more chains
	printf("Generating chains\n");
	signal(SIGINT, stopGenerating);
	unsigned int progressStep = rt->a_chains / 10000;
	if (!progressStep) progressStep = 1;
	while (generate && rt->n_chains < rt->a_chains)
	{
		char res = RTable_FindChain(rt);
		if (res < 0)
		{
			printf("\n");
			printf("Nothing more to do\n");
			generate = 0;
		}
		else if (res && rt->n_chains % progressStep == 0)
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
	RTable_ToFileN(rt, filename);
	RTable_Delete(rt);
	return 0;
}
