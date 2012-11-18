#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

	int server = TCP_Connect("127.0.0.1", "4242");

	{
		int command = 0x01; // REQUEST
		write(server, &command, 4);
	}

	int n_hashes;
	read(server, &n_hashes, 1);

	char* hashes = (char*) malloc(16*n_hashes);
	read(server, hashes, 16*n_hashes);

	int length;
	read(server, &length, 4);

	int clen;
	read(server, &clen, 4);

	char* charset = (char*) malloc(clen);
	read(server, charset, clen);

	int plen;
	read(server, &plen, 4);

	// TODO: remove the '+1'
	char* hash = (char*) malloc(length+1);
	memset(hash+plen+1, 0, length-plen);
	read(server, hash, plen);

	char done = 0;
	while (!done)
	{
		printf("%s\n", hash);
		char digit = 1;
		while (1)
		{
			char* cur = strchr(charset, hash[length-digit]);
			if (cur[1]) // if cur is not the last element of charset
			{
				hash[length-digit] = cur[1];
				break;
			}

			hash[length-digit] = charset[0];
			digit++;
			if (digit > length-plen)
			{
				done = 1;
				break;
			}
		}
	}

	{
		int command = 3; // DONE
		write(server, &command, 4);
	}

	close(server);
	return 0;
}
