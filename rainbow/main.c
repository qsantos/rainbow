#include <stdio.h>
#include <string.h>

#include "rainbow.h"
#include "md5.h"

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

	unsigned int slen = 6;
	char* charset = "0123456789abcdefghijklmnopqrstuvwxyz";
	unsigned int clen = strlen(charset);
	unsigned int l_chains = 1000;
	unsigned int n_chains = 1000;

	Rainbow_Init(slen, charset, l_chains, n_chains);

	for (unsigned int i = 0; i < n_chains; i++)
		Rainbow_FindChain();

	printf("Rainbow table generated\n");

	printf("Cracking some hashes\n");

	char str [slen];
	char hash[16];
	int count = 0;
	memcpy(str, "aaaaaa", 7);
	for (unsigned int i = 0; i < clen; i++)
	{
		str[0] = charset[i];
		for (unsigned int j = 0; j < clen; j++)
		{
			str[1] = charset[j];
			printf("%s (%i)\n", str, count);
			MD5(slen, (uint8_t*) str, (uint8_t*) hash);
			if (Rainbow_Reverse(hash, NULL))
				count++;
		}
	}
	printf("%i\n", count);

	Rainbow_Deinit();
	return 0;
}
