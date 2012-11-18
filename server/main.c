#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "socket.h"

static char hex2byte(const char digit[2])
{
	char ret = 0;
	char a = digit[0]; ret += a - (a <= '9' ? '0' : '@');
	ret <<= 4;
	char b = digit[0]; ret += b - (b <= '9' ? '0' : '@');
	return ret;
}

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

	const int n_hashes = 1;
	const char* hex_hash = "bc11f06afb9b27070673471a23ecc6a9";
	char hash[16];
	for (unsigned int i = 0; i < 16; i++)
		hash[i] = hex2byte(hex_hash + 2*i);

	const int length = 8;
	const char* charset = "abcdefghijklmnopqrstuvwxyz";
	const int clen = strlen(charset);

	const int plen = 2;
	char prefix[plen];
	memset(prefix, 0, plen);

	char* port = "4242";
	int server = TCP_Listen(port);
	if (server < 0)
	{
		fprintf(stderr, "Could not listen on port %s\n", port);
		return 1;
	}

	int* clients   = NULL;
	int  n_clients = 0;
	int  a_clients = 0;

	int epollfd = epoll_create(10);
	if (epollfd < 0)
	{
		perror("epoll_create");
		return 1;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = server;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server, &ev) < 0)
	{
		perror("epoll_ctl: server");
		return 1;
	}

#define MAX_EVENTS 10
	struct epoll_event events[MAX_EVENTS];
	while (1)
	{
		int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1)
		{
			perror("epoll_pwait");
			return 1;
		}
		for (int n = 0; n < nfds; ++n)
		{
			int fd = events[n].data.fd;
			if (fd == server)
			{
				// accept client
				int client = TCP_Accept(server);
				if (client < 0)
					continue;

				// add socket to list
				if (n_clients >= a_clients)
				{
					a_clients = a_clients ? 2*a_clients : 1;
					clients = (int*) realloc(clients, sizeof(int) * a_clients);
				}
				clients[n_clients++] = client;

				// non blocking socket
				int flags = fcntl(client, F_GETFL, 0);
				fcntl(client, F_SETFL, flags | O_NONBLOCK);

				// add socket to monitor
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = client;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client, &ev) == -1)
				{
					perror("epoll_ctl: client");
					return 1;
				}
				continue;
			}

			int message;
			read(fd, &message, 1);
			switch (message)
			{
			case 0x01: // REQUEST
				write(fd, &n_hashes, sizeof(int));
				write(fd, hash, 16);
				write(fd, &length, sizeof(int));
				write(fd, &clen, sizeof(int));
				write(fd, charset, clen);
				write(fd, &plen, sizeof(int));
				write(fd, prefix, plen);
				break;
			case 0x02: // FOUND
				printf("Yeah !\n");
				break;
			case 0x03: // DONE
				printf("Ok, next\n");
				break;
			}
		}
	}

	for (int i = 0; i < n_clients; i++)
		close(clients[i]);
	close(server);
	return 0;
}
