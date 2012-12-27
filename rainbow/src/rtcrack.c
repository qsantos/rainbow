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

static char reverseHash(RTable* rt, u32 n_rt, const char* hashstr, u32 l_string, char* strbuf)
{
	char hash[16];
	hex2hash(hashstr, hash, 16);
	for (u32 i = 0; i < n_rt; i++)
	{
		if (RTable_Reverse(&rt[i], hash, strbuf))
		{
			printHash(hash, 16);
			printf(" ");
			printString(strbuf, l_string);
			printf("\n");
			return 1;
		}
	}
	printf("Could not reverse hash\n");
	return 0;
}

static void usage(int argc, char** argv)
{
	(void) argc;

	printf
	(
		"Usage: %s TARGET src1 [src2 [...]]\n"
		"try and reverse one or several hashes\n"
		"\n"
		"TARGET:\n"
		" -x, --hash HASH  sets the target to HASH\n"
		" -f, --file FILE  read the targets from FILE (- for stdin)\n"
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

	if (argc < 4)
	{
		usage(argc, argv);
		exit(1);
	}

	char  fromfile = strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--file") == 0;
	char* target   = argv[2];

	// load tables
	u32 n_rt = argc-3;
	RTable* rt = malloc(sizeof(RTable) * n_rt);
	assert(rt);
	for (u32 i = 0; i < n_rt; i++)
		if (!RTable_FromFile(&rt[i], argv[i+3]))
			ERROR("Could no load table '%s'\n", argv[i+3])

	// try and crack hash(es)
	char n_crack = 0;
	char* bufstr = (char*) malloc(rt[0].l_string);
	if (fromfile)
	{
		FILE* f = strcmp(target, "-") == 0 ? stdin : fopen(target, "r");
		assert(f);

		char hashstr[33];
		while (1)
		{
			fread(hashstr, 1, 33, f);
			n_crack += reverseHash(rt, n_rt, hashstr, rt[0].l_string, bufstr);
			if (feof(f))
				break;
		}

		fclose(f);
	}
	else
		n_crack += reverseHash(rt, n_rt, target, rt[0].l_string, bufstr);
	free(bufstr);

	for (u32 i = 0; i < n_rt; i++)
		RTable_Delete(&rt[i]);

	return n_crack;
}
