#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "socket.h"

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

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
			if (events[n].data.fd == server)
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
			}
			else
			{
			}
		}
	}

	for (int i = 0; i < n_clients; i++)
		close(clients[i]);
	close(server);
	return 0;
}
