#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "md5.h"
#include "rainbow.h"

#define ERROR(...)                    \
{                                     \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n");        \
	usage(argc, argv);            \
	exit(1);                      \
}

// some variables used internally by reverseHash()
static RTable* rt     = NULL;
static u32     n_rt   = 0;
static char**  files  = NULL;
static char*   bufstr = NULL;

static char reverseHash(const char hash[16])
{
	if (files)
	{
		for (u32 i = 0; i < n_rt; i++)
		{
			RTable_FromFile(rt, files[i]);
			char res = RTable_Reverse(rt, hash, bufstr);
			RTable_Delete(rt);
			if (res)
				return 1;
		}
	}
	else
	{
		for (u32 i = 0; i < n_rt; i++)
			if (RTable_Reverse(&rt[i], hash, bufstr))
				return 1;
	}
	return 0;
}

static inline void rewriteLine(void)
{
	printf("\r\33[K");
}

static void usage(int argc, char** argv)
{
	(void) argc;

	fprintf(stderr,
		"Usage: %s TARGET src1 [src2 [...]]\n"
		"Try and reverse one or several hashes\n"
		"\n"
		"TARGET:\n"
		" -x, --hash HASH  sets the target to HASH\n"
		" -f, --file FILE  read the targets from FILE (- for stdin)\n"
		" -r, --random N   targets are N random string hashes (bench)\n"
		,
		argv[0]
	);
}

typedef enum
{
	T_HASH,
	T_FILE,
	T_RAND,
} Target;

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

	Target ttype;
	char* tstr = argv[1];
	if (strcmp(tstr, "-x") == 0 || strcmp(tstr, "--hash") == 0)
		ttype = T_HASH;
	else if (strcmp(tstr, "-f") == 0 || strcmp(tstr, "--file") == 0)
		ttype = T_FILE;
	else if (strcmp(tstr, "-r") == 0 || strcmp(tstr, "--random") == 0)
		ttype = T_RAND;
	else
		ERROR("Invalid target '%s'\n", tstr);

	char* tparam = argv[2];

	// load tables
	n_rt = argc-3;
	if (1) // load the tables one after another for each reversing
	{
		rt = malloc(sizeof(RTable));
		files = argv + 3;
		assert(rt);
	}
	else
	{
		rt = malloc(sizeof(RTable) * n_rt);
		assert(rt);
		for (u32 i = 0; i < n_rt; i++)
			if (!RTable_FromFile(&rt[i], argv[i+3]))
				ERROR("Could no load table '%s'\n", argv[i+3])
	}

	// some parameters
	// TODO
	RTable_FromFile(rt, argv[4]);
	u32   l_string  = rt[0].l_string;
	char* charset   = rt[0].charset;
	u32   n_charset = rt[0].n_charset;
	RTable_Delete(rt);

	// some buffers
	char hash[16];
	bufstr = malloc(l_string);
	assert(bufstr);

	// try and crack hash(es)
	switch (ttype)
	{
	case T_HASH:
		hex2hash(tparam, hash, 16);
		if (reverseHash(hash))
		{
			printHash(hash, 16);
			printf(" ");
			printString(bufstr, l_string);
			printf("\n");
		}
		else
			printf("Could not reverse hash\n");

		break;
	case T_FILE:
		(void) 0;
		FILE* f = strcmp(tparam, "-") == 0 ? stdin : fopen(tparam, "r");
		assert(f);

		while (1)
		{
			char hashstr[33];
			fread(hashstr, 1, 33, f);
			if (feof(f))
				break;

			hex2hash(hashstr, hash, 16);
			if (reverseHash(hash))
			{
				printHash(hash, 16);
				printf(" ");
				printString(bufstr, l_string);
				printf("\n");
			}
			else
				printf("Could not reverse hash\n");
		}

		fclose(f);
		break;
	case T_RAND:
		srandom(time(NULL));
		u32 n = atoi(tparam);
		u32 n_crack = 0;
		for (u32 i = 0; i < n; i++)
		{
			for (u32 j = 0; j < l_string; j++)
				bufstr[j] = charset[random() % n_charset];

			char hash[16];
			MD5((u8*) hash, (u8*) bufstr, l_string);
			if (reverseHash(hash))
				n_crack++;

			rewriteLine();
			printf("%lu / %lu", n_crack, i+1);
			fflush(stdout);
		}
		printf("\n");
	}
	free(bufstr);

	if (!files)
		for (u32 i = 0; i < n_rt; i++)
			RTable_Delete(&rt[i]);
	free(rt);

	return 0;
}
