#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

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
	int  maxfd     = server;
	fd_set fds;
	FD_ZERO(&fds);
	while (1)
	{
		FD_SET(server, &fds);
		for (int i = 0; i < n_clients; i++)
			FD_SET(clients[i], &fds);
		int res = select(maxfd+1, &fds, NULL, NULL, NULL);

		if (res < 0)
		{
			perror("select()");
			return 1;
		}
		else if (FD_ISSET(server, &fds))
		{
			int client = TCP_Accept(server);
			if (client < 0)
				continue;
			if (n_clients >= a_clients)
			{
				a_clients = a_clients ? 2*a_clients : 1;
				clients = (int*) realloc(clients, sizeof(int) * a_clients);
			}
			if (client > maxfd)
				maxfd = client;
			clients[n_clients++] = client;
		}
	}
	close(server);
	return 0;
}
