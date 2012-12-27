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
		"Usage: %s l_string l_chains n_chains src [dst]\n"
		"resize table to store n_chains\n"
		"\n"
		"PARAMS:\n"
		"  l_string       length of the non-hashed string / key\n"
		"  l_chains   length of the chains to generate\n"
		"  n_chains   the number of chains to be generated\n"
		"  src        file containing a rainbow table (only read)\n"
		"  dst        file containing a rainbow table (only write)\n"
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
	unsigned int l_string     = atoi(argv[1]);
	unsigned int l_chains = atoi(argv[2]);

	RTable* src = RTable_FromFileN(l_string, charset, l_chains, param2);
	RTable* dst = RTable_New      (l_string, charset, l_chains, atoi(param1));

	if (!src) ERROR("Could no load source table\n")

	RTable_Transfer(src, dst);
	printf("%u chains transfered\n", dst->n_chains);

	RTable_ToFileN(dst, param3);
	RTable_Delete(src);
	RTable_Delete(dst);
	return 0;
}
