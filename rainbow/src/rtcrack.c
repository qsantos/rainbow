#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rainbow.h"

#define ERROR(...)                    \
{                                     \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n");        \
	usage(argc, argv);            \
	RTable_Delete(&rt);           \
	exit(1);                      \
}

static void usage(int argc, char** argv)
{
	(void) argc;

	printf
	(
		"Usage: %s l_string l_chains n_chains hash src\n"
		"try and reverse a hash\n"
		"\n"
		"PARAMS:\n"
		"  l_string   length of the non-hashed string / key\n"
		"  l_chains   length of the chains to generate\n"
		"  n_chains   the number of chains in the table_n"
		"  hash       the hash to be reversed\n"
		"  src        source file\n"
		,
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
		printf("rtcrack\n");
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
	u32   l_chains = atoi(argv[2]);
	u32   n_chains = atoi(argv[3]);
	char* hashstr  = argv[4];
	char* filename = argv[5];

	// load table
	RTable rt;
	RTable_New(&rt, l_string, charset, l_chains, n_chains);
	if (!RTable_FromFile(&rt, filename))
		ERROR("Could no load table\n")

	// try and crack hash
	char hash[16];
	hex2hash(hashstr, hash, 16);

	char* str = (char*) malloc(l_string);
	char res = RTable_Reverse(&rt, hash, str);

	if (res)
	{
		printHash(hash, 16);
		printf(" ");
		printString(str, l_string);
		printf("\n");
		free(str);
		RTable_Delete(&rt);
		exit(0);
	}
	else
	{
		printf("Could not reverse hash\n");
		free(str);
		RTable_Delete(&rt);
		exit(1);
	}
}
