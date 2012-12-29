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
		"Usage: %s l_string s_reduce l_chains n_chains n_parts part dst\n"
		"create a new Rainbow Table in dst\n"
		"\n"
		"PARAMS:\n"
		"  l_string   length of the non-hashed string / key\n"
		"  s_reduce   reduction function seed (i.e. table index)\n"
		"  l_chains   length of the chains to generate\n"
		"  n_chains   the number of chains to be generated\n"
		"  n_parts    the number of parts to split the table in\n"
		"  part       number of the table part (i.e. table part)\n"
		"  dst        destination file\n"
		"\n"
		"Examples\n"
		"  $ %s 6 0 1000 500000 4 0 a0_0.rt\n"
		"  $ %s 6 0 1000 500000 4 1 a0_1.rt\n"
		"  $ %s 6 0 1000 500000 4 2 a0_2.rt\n"
		"  $ %s 6 0 1000 500000 4 3 a0_3.rt\n"
		"\n"
		"  $ %s 6 1 1000 500000 4 0 a0_0.rt\n"
		"  $ %s 6 1 1000 500000 4 1 a0_1.rt\n"
		"  $ %s 6 1 1000 500000 4 2 a0_2.rt\n"
		"  $ %s 6 1 1000 500000 4 3 a0_3.rt\n"
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

	if (argc < 8)
	{
		usage(argc, argv);
		exit(1);
	}

	char* charset  = "0123456789abcdefghijklmnopqrstuvwxyz";
	u32   l_string = atol(argv[1]);
	u32   s_reduce = atol(argv[2]);
	u32   l_chains = atol(argv[3]);
	u32   n_chains = atol(argv[4]);
	u32   n_parts  = atol(argv[5]);
	u32   part     = atol(argv[6]);
	char* filename = argv[7];

	RTable rt;
	RTable_New(&rt, l_string, charset, s_reduce, l_chains, n_chains);
	srandom(time(NULL));

	// load table, if exists
	RTable_FromFile(&rt, filename);

	// generate more chains
	signal(SIGINT, stopGenerating);
	u32 progressStep = rt.a_chains / 10000;
	if (!progressStep) progressStep = 1;
	u64 startPointIdx = part;
	while (generate && rt.n_chains < rt.a_chains)
	{
		char res = RTable_StartAt(&rt, startPointIdx);
		startPointIdx += n_parts;
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
