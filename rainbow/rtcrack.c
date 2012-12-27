#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rainbow.h"

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
		"Usage: %s slen l_chains hash src\n"
		"try and reverse a hash\n"
		"\n"
		"PARAMS:\n"
		"  slen       length of the non-hashed string / key\n"
		"  l_chains   length of the chains to generate\n"
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

	if (argc < 5)
	{
		usage(argc, argv);
		exit(1);
	}

	char*        charset  = "0123456789abcdefghijklmnopqrstuvwxyz";
	unsigned int slen     = atoi(argv[1]);
	unsigned int l_chains = atoi(argv[2]);
	char*        hashstr  = argv[3];
	char*        filename = argv[4];

	// load table
	RTable* rt = RTable_FromFileN(slen, charset, l_chains, filename);
	if (!rt) ERROR("Could no load table\n")

	// try and crack hash
	char hash[16];
	hex2hash(hashstr, hash, 16);

	char* str = (char*) malloc(slen);
	int res = RTable_Reverse(rt, hash, str);

	if (res)
	{
		printHash(hash, 16);
		printf(" ");
		printString(str, slen);
		printf("\n");
		free(str);
		RTable_Delete(rt);
		exit(0);
	}
	else
	{
		printf("Could not reverse hash\n");
		free(str);
		RTable_Delete(rt);
		exit(1);
	}
}
