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

static char reverseHash(RTable* rt, u32 n_rt, const char* hashstr, u32 l_string, char* bufstr)
{
	char hash[16];
	hex2hash(hashstr, hash, 16);
	for (u32 i = 0; i < n_rt; i++)
	{
		if (RTable_Reverse(&rt[i], hash, bufstr))
		{
			printHash(hash, 16);
			printf(" ");
			printString(bufstr, l_string);
			printf("\n");
			return 1;
		}
	}
	printf("Could not reverse hash\n");
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
	u32 n_rt = argc-3;
	RTable* rt = malloc(sizeof(RTable) * n_rt);
	assert(rt);
	for (u32 i = 0; i < n_rt; i++)
		if (!RTable_FromFile(&rt[i], argv[i+3]))
			ERROR("Could no load table '%s'\n", argv[i+3])

	// try and crack hash(es)
	u32   l_string  = rt[0].l_string;
	char* charset   = rt[0].charset;
	u32   n_charset = rt[0].n_charset;
	char* bufstr    = malloc(l_string);
	u32   n_crack   = 0;
	assert(bufstr);
	switch (ttype)
	{
	case T_HASH:
		reverseHash(rt, n_rt, tparam, l_string, bufstr);
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
			n_crack += reverseHash(rt, n_rt, hashstr, l_string, bufstr);
		}

		fclose(f);
		break;
	case T_RAND:
		srandom(time(NULL));
		u32 n = atoi(tparam);
		for (u32 i = 0; i < n; i++)
		{
			for (u32 j = 0; j < l_string; j++)
				bufstr[j] = charset[random() % n_charset];

			char hash[16];
			MD5((u8*) hash, (u8*) bufstr, l_string);
			for (u32 i = 0; i < n_rt; i++)
				if (RTable_Reverse(&rt[i], hash, bufstr))
				{
					n_crack++;
					break;
				}
			rewriteLine();
			printf("%lu / %lu", n_crack, i+1);
			fflush(stdout);
		}
		printf("\n");
	}
	free(bufstr);

	for (u32 i = 0; i < n_rt; i++)
		RTable_Delete(&rt[i]);

	return 0;
}
