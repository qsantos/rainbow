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
	if (server < 0)
	{
		fprintf(stderr, "Could not connect to server\n");
		return 1;
	}

	printf("Connected %i\n", server);

	{
		int command = 0x01; // REQUEST
		write(server, &command, 4);
	}

	printf("Request sent\n");

	int n_hashes;
	read(server, &n_hashes, 4);

	char* hashes = (char*) malloc(16*n_hashes);
	read(server, hashes, 16*n_hashes);

	int length;
	read(server, &length, 4);

	int clen;
	read(server, &clen, 4);

	char* charset = (char*) malloc(clen+1);
	read(server, charset, clen);
	charset[clen] = 0;

	int plen;
	read(server, &plen, 4);

	char* hash = (char*) malloc(length);
	read(server, hash, plen);
	memset(hash+plen, charset[0], length-plen);

	printf("Got chunk\n");

	char done = 0;
	while (!done)
	{
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
			if (digit >= 5)
				printf("%c%c%c%c\n", hash[0], hash[1], hash[2], hash[3]);
		}
	}

	{
		int command = 3; // DONE
		write(server, &command, 4);
	}

	close(server);
	return 0;
}
