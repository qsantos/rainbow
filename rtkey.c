#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

typedef unsigned long      u32;
typedef unsigned long long u64;

char index2key(u64 index, char* key, u32 l_min, u32 l_max, const char* charset, u32 n_charset)
{
	u32 l_key = l_min;
	u64 n_key = pow(n_charset, l_min);
	while (index >= n_key)
	{
		index -= n_key;
		n_key *= n_charset;
		l_key++;
	}
	if (l_key > l_max)
		return 0;

	for (u32 i = 0; i < l_key; i++, key++)
	{
		*key = charset[index % n_charset];
		index /= n_charset;
	}
	return 1;
}

void usage(int argc, char** argv)
{
	(void) argc;

	fprintf(stderr,
		"Usage: %s l_min l_max n\n"
		"compute the n-th string of the keyspace\n"
		"\n"
		"l_min  minimum key length\n"
		"l_max  maximum key length\n"
		"n      key index\n"
		,
		argv[0]
	);
}

int main(int argc, char** argv)
{
	if (argc == 1 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
	{
		usage(argc, argv);
		exit(0);
	}
	else if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)
	{
		printf("rtkey\n");
		printf("Compiled on %s at %s\n", __DATE__, __TIME__);
		exit(0);
	}

	if (argc < 4)
	{
		usage(argc, argv);
		exit(1);
	}

	const u32   l_min     = atol(argv[1]);
	const u32   l_max     = atol(argv[2]);
	const char* charset   = "0123456789abcdefghijklmnopqrstuvwxyz";
	const u32   n_charset = strlen(charset);
	const u64   index     = atoll(argv[3]);

	char* string = malloc(l_max+1);
	assert(string);
	string[l_max] = 0;

	if (index2key(index, string, l_min, l_max, charset, n_charset))
		printf("key no %llu = '%s'\n", index, string);
	else
		printf("key out of range\n");

	free(string);
	return 0;
}
