#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "rainbow.h"

static inline void rewriteLine(void)
{
	printf("\r\33[K");
}

static char generate = 1;
static void stopGenerating(int signal)
{
	(void) signal;
	generate = 0;
}

static void usage(int argc, char** argv)
{
	(void) argc;

	printf
	(
		"Usage: %s l_string s_reduce l_chains n_chains part dst\n"
		"create a new Rainbow Table in dst\n"
		"\n"
		"PARAMS:\n"
		"  l_string   length of the non-hashed string / key\n"
		"  s_reduce   reduction function seed\n"
		"  l_chains   length of the chains to generate\n"
		"  n_chains   the number of chains to be generated\n"
		"  part       number of the table part (contains n_chains chains)\n"
		"  dst        destination file\n"
		"\n"
		"Examples\n"
		"  $ %s 6 0 1000 500000 0\n"
		"  $ %s 6 0 1000 500000 1\n"
		"  $ %s 6 0 1000 500000 2\n"
		"  $ %s 6 0 1000 500000 3\n"
		"\n"
		"  $ %s 6 1 1000 500000 0\n"
		"  $ %s 6 1 1000 500000 1\n"
		"  $ %s 6 1 1000 500000 2\n"
		"  $ %s 6 1 1000 500000 3\n"
		"...\n"
		,
		argv[0],
		argv[0],
		argv[0],
		argv[0],
		argv[0],
		argv[0],
		argv[0],
		argv[0],
		argv[0]
	);
}

int main(int argc, char** argv)
{
	if (argc == 1 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
	{
		usage(argc, argv);
		exit(0);
	}
	else if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v"))
	{
		printf("rtgen\n");
		printf("Compiled on %s at %s\n", __DATE__, __TIME__);
		exit(0);
	}

	if (argc < 6)
	{
		usage(argc, argv);
		exit(1);
	}

	char* charset  = "0123456789abcdefghijklmnopqrstuvwxyz";
	u32   l_string = atoi(argv[1]);
	u32   s_reduce = atoi(argv[2]);
	u32   l_chains = atoi(argv[3]);
	u32   n_chains = atoi(argv[4]);
	char* filename = argv[5];

	RTable rt;
	RTable_New(&rt, l_string, charset, s_reduce, l_chains, n_chains);
	srandom(time(NULL));

	// load table, if exists
	RTable_FromFile(&rt, filename);

	// generate more chains
	signal(SIGINT, stopGenerating);
	u32 progressStep = rt.a_chains / 10000;
	if (!progressStep) progressStep = 1;
	u64 startPointIdx = 0;
	while (generate && rt.n_chains < rt.a_chains)
	{
		char res = RTable_StartAt(&rt, startPointIdx++);
		if (res < 0)
		{
			printf("\n");
			printf("Nothing more to do\n");
			generate = 0;
		}
		else if (res && rt.n_chains % progressStep == 0)
		{
			rewriteLine();
			printf("Progress: %.2f%%", (float) 100 * rt.n_chains / rt.a_chains);
			fflush(stdout);
		}
	}
	rewriteLine();

	// finish generation
	if (generate)
		RTable_Sort(&rt);
	else
		printf("Pausing table generation (%lu chains generated)\n", rt.n_chains);

	// save table
	RTable_ToFile(&rt, filename);
	RTable_Delete(&rt);
	return 0;
}
