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
		"Usage: %s hash src\n"
		"try and reverse a hash\n"
		"\n"
		"PARAMS:\n"
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

	if (argc < 3)
	{
		usage(argc, argv);
		exit(1);
	}

	char* hashstr  = argv[1];
	char* filename = argv[2];

	// load table
	RTable rt;
	if (!RTable_FromFile(&rt, filename))
		ERROR("Could no load table\n")

	// try and crack hash
	char hash[16];
	hex2hash(hashstr, hash, 16);

	char* str = (char*) malloc(rt.l_string);
	char res = RTable_Reverse(&rt, hash, str);

	if (res)
	{
		printHash(hash, 16);
		printf(" ");
		printString(str, rt.l_string);
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
