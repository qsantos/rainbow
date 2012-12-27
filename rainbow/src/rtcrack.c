#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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
		"Usage: %s hash src1 [src2 [...]]\n"
		"try and reverse a hash\n"
		"\n"
		"PARAMS:\n"
		"  hash       the hash to be reversed\n"
		"  srcX       rainbow tables\n"
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

	if (argc < 3)
	{
		usage(argc, argv);
		exit(1);
	}

	char* hashstr  = argv[1];

	// load tables
	RTable* rt = malloc(sizeof(RTable) * argc - 2);
	u32 n_rt = argc-2;
	assert(rt);
	for (u32 i = 0; i < n_rt; i++)
		if (!RTable_FromFile(&rt[i], argv[i+2]))
			ERROR("Could no load table '%s'\n", argv[i+2])

	// try and crack hash
	char hash[16];
	hex2hash(hashstr, hash, 16);

	char* str = (char*) malloc(rt[0].l_string);

	char res;
	for (u32 i = 0; i < n_rt; i++)
	{
		res = RTable_Reverse(&rt[i], hash, str);
		if (res)
			break;
	}
	if (res)
	{
		printHash(hash, 16);
		printf(" ");
		printString(str, rt[0].l_string);
		printf("\n");
	}
	else
	{
		printf("Could not reverse hash\n");
	}

	free(str);
	for (u32 i = 0; i < n_rt; i++)
		RTable_Delete(&rt[i]);

	return res ? 0 : 1;
}
