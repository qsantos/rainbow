#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"
#include "md5.h"

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

	char* hashes = (char*) malloc(16*n_hashes+1);
	read(server, hashes, 16*n_hashes);
	hashes[16*n_hashes] = 0; // TODO

	int length;
	read(server, &length, 4);

	int clen;
	read(server, &clen, 4);

	char* charset = (char*) malloc(clen+1);
	read(server, charset, clen);
	charset[clen] = 0;

	int plen;
	read(server, &plen, 4);

	char* str = (char*) malloc(length + 1);
	read(server, str, plen);
	memset(str+plen, charset[0], length-plen);
	str[length] = 0; // TODO

	printf("Got chunk\n");

	char done = 0;
	while (!done)
	{
		// proceed to computations
		char result[16];
		MD5((uint64_t) length, (uint8_t*) str, (uint8_t*) result);
		
		// if hash found, warns server
		if (!strncmp(result, hashes, 16))
		{
			char* message = (char*) malloc(24 + length);
			*(int*)message = 0x02;
			memcpy(message+4, hashes, 16);
			*(int*)(message+20) = length;
			memcpy(message+24, str, length);
			write(server, message, 24 + length);
		}

		// get next string
		char digit = 1;
		while (1)
		{
			char* cur = strchr(charset, str[length-digit]);
			if (cur[1]) // if cur is not the last element of charset
			{
				str[length-digit] = cur[1];
				break;
			}

			str[length-digit] = charset[0];
			if (digit >= 3)
				printf("%c%c%c%c\n", str[0], str[1], str[2], str[3]);
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
