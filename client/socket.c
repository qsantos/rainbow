/*\
 *  This is an awesome programm simulating awesome battles of awesome robot tanks
 *  Copyright (C) 2012  Quentin SANTOS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

#include "socket.h"

#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>

static struct timeval timeout = { 5, 0 };

int TCP_Connect(const char* node, const char* service)
{
// see getaddrinfo(2)
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo* result;
	getaddrinfo(node, service, &hints, &result);

	int sock;
	struct addrinfo* cur = result;
	while (cur)
	{
		sock = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));

		if (sock == -1)
			continue; // try next addrinfo

		if (connect(sock, cur->ai_addr, cur->ai_addrlen) == 0)
			break; // keep this one

		close(sock);
		cur = cur->ai_next;
	}

	if (!cur)
		sock = -1;

	freeaddrinfo(result);
	return sock;
}

int TCP_ListenTo(const char* node, const char* service)
{
// see getaddrinfo(2)
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;

	struct addrinfo* result;
	if (getaddrinfo(node, service, &hints, &result))
		return -1;

	int sock;
	struct addrinfo* cur = result;
	while (cur)
	{
		sock = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
		if (sock == -1)
			continue;

		int v = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(int));

		if (bind(sock, cur->ai_addr, cur->ai_addrlen) != -1 && listen(sock, 10) != -1)
			break;

		close(sock);
		cur = cur->ai_next;
	}

	if (!cur)
		sock = -1;
	freeaddrinfo(result);
	return sock;
}

int TCP_Listen(const char* port)
{
	return TCP_ListenTo(NULL, port);
}

int TCP_Accept(int sock)
{
	int client = accept(sock, NULL, NULL);
	if (client < 0)
		return -1;

	return client;
}

void TCP_Timeout(int sec, int usec)
{
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;
}
