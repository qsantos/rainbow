#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef unsigned long      u32;
typedef unsigned long long u64;

void index2key(u64 index, char* key, u32 l_string, const char* charset, u32 n_charset)
{
	for (u32 i = 0; i < l_string; i++, key++)
	{
		*key = charset[index % n_charset];
		index /= n_charset;
	}
}

void usage(int argc, char** argv)
{
	(void) argc;

	fprintf(stderr,
		"Usage: %s l_string n\n"
		"compute the n-th string of the keyspace\n"
		"\n"
		"l_string  key length\n"
		"n         key index\n"
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

	if (argc < 3)
	{
		usage(argc, argv);
		exit(1);
	}

	const u32   l_string  = atol(argv[1]);
	const char* charset   = "0123456789abcdefghijklmnopqrstuvwxyz";
	const u32   n_charset = strlen(charset);
	const u64   index     = atoll(argv[2]);

	char* string = malloc(l_string);
	assert(string);
	index2key(index, string, l_string, charset, n_charset);
	printf("key no %llu = '%s'\n", index, string);
	free(string);

	return 0;
}
