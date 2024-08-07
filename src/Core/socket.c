/*------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ------------------------------------------------------------------------*/

#include "core.h"
#include "socket.h"

fd_set readfds;
unsigned int fd_max = 0;

static int null_parse(unsigned int);
static int (*default_func_parse)(unsigned int) = null_parse;
void set_defaultparse(int (*defaultparse)(unsigned int))
{
	default_func_parse = defaultparse;
}

static int null_parse(unsigned int fd)
{
	debug_output("null_parse: No 'set_defaultparse()' function defined.\n");
	RFIFOSKIP(fd, RFIFOREST(fd));
	return 0;
}

void do_socket(void)
{
	FD_ZERO(&readfds);
}

static unsigned int recv_to_fifo(unsigned int fd)
{
	int len;
	int counter;

	if (session[fd]->eof) {
		debug_output("recv_to_fifo: eof %d.\n", session[fd]->eof);
		return 1;
	}
	counter = 0;
	do {
		counter++;
		len = read(fd, session[fd]->rdata + session[fd]->rdata_size, RFIFOSPACE(fd));
		if (len > 0) {
			session[fd]->rdata_size += len;
			if (session[fd]->rdata_size >= (session[fd]->max_rdata - 12800)) {
				if (session[fd]->max_rdata == RFIFO_SIZE) {
					len = 0;
					session[fd]->eof = 1;
				}
				else {
					realloc_fifo(fd, session[fd]->max_rdata << 1, session[fd]->max_wdata);
					len = 0;
				}
			}
		}
		else if ((counter == 1 && len <= 0) || (len < 0 && errno != EAGAIN && counter > 1))
			session[fd]->eof = 1;

	}
	while (len > 0);
	return 0;
}

static unsigned int send_from_fifo(unsigned int fd)
{
	int len;
	int counter = 0;

	do {
		counter++;
		len = write(fd, session[fd]->wdata, session[fd]->wdata_size);
		if (len > 0) {
			if (len < session[fd]->wdata_size) {
				memmove(session[fd]->wdata, session[fd]->wdata + len, session[fd]->wdata_size - len);
				session[fd]->wdata_size -= len;
			}
			else
				session[fd]->wdata_size = 0;

		}
		else if ((counter == 1 && len <= 0) || (counter > 1 && len < 0 && errno != EAGAIN))
			session[fd]->eof = 1;

	}
	while (len > 0 && session[fd]->wdata_size > 0);
	return 0;
}

static unsigned int connect_client(unsigned int listen_fd)
{
	int yes = 1;
	unsigned int fd;
	unsigned int err = -1;
	struct sockaddr_in client_address;
	unsigned int len = sizeof(client_address);

	fd = accept(listen_fd, (struct sockaddr*)&client_address, &len);
	if (fd_max <= fd)
		fd_max = fd + 1;

	fcntl(fd, F_SETFL, O_NONBLOCK);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&yes, sizeof yes);
#endif
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof yes);

	if (fd == err)
		perror("accept");

	else
		FD_SET(fd, &readfds);

	session[fd] = calloc(sizeof(*session[fd]), 1);
	if (!session[fd]) {
		debug_output("connect_client: Fail to allocate memory in session[%d].\n", fd);
		exit(0);
	}
	memset(session[fd], 0, sizeof(*session[fd]));
	session[fd]->rdata = calloc(RFIFO_SIZE, 1);
	if (!session[fd]->rdata) {
		debug_output("connect_client: Fail to allocate rdata.\n");
		exit(1);
	}
	session[fd]->wdata = calloc(WFIFO_SIZE, 1);
	if (!session[fd]->wdata) {
		debug_output("connect_client: Fail to allocate wdata.\n");
		exit(2);
	}
	session[fd]->max_rdata = RFIFO_SIZE;
	session[fd]->max_wdata = WFIFO_SIZE;
	session[fd]->func_recv = recv_to_fifo;
	session[fd]->func_send = send_from_fifo;
	session[fd]->func_parse = default_func_parse;
	session[fd]->client_addr = client_address;
	return fd;
}

unsigned int make_listen_port(short port)
{
	int yes = 1;
	unsigned int fd;
	struct sockaddr_in server_address;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_max <= fd)
		fd_max = fd + 1;

	fcntl(fd, F_SETFL, O_NONBLOCK);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&yes, sizeof yes);
#endif
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof yes);

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
		perror("bind");
		exit(0);
	}
	if (listen(fd, 5) == -1) {
		perror("listen");
		exit(1);
	}
	FD_SET(fd, &readfds);
	session[fd] = calloc(sizeof(*session[fd]), 1);
	if (!session[fd]) {
		debug_output("make_listen_port: Fail to allocate memory in session[%d].\n", fd);
		exit(2);
	}
	memset(session[fd], 0, sizeof(*session[fd]));
	session[fd]->func_recv = connect_client;
	return fd;
}

unsigned int make_connection(long ip, short port)
{
	unsigned int fd;
	int yes = 1;
	struct sockaddr_in server_address;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_max <= fd)
		fd_max = fd + 1;

	fcntl(fd, F_SETFL, O_NONBLOCK);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&yes, sizeof yes);
#endif
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof yes);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port = htons(port);

	connect(fd, (struct sockaddr *)(&server_address), sizeof(struct sockaddr_in));
	FD_SET(fd, &readfds);
	session[fd] = calloc(sizeof(*session[fd]), 1);
	if (!session[fd]) {
		debug_output("make_connection: Fail to allocate memory in session[%d].\n", fd);
		exit(0);
	}
	memset(session[fd], 0, sizeof(*session[fd]));
	session[fd]->rdata = calloc(RFIFO_SIZE, 1);
	if (!session[fd]) {
		debug_output("make_connection: Fail to allocate memory rdata.\n");
		exit(1);
	}
	session[fd]->wdata = calloc(WFIFO_SIZE, 1);
	if (!session[fd]) {
		debug_output("make_connection: Fail to allocate memory wdata.\n");
		exit(2);
	}
	session[fd]->max_rdata = RFIFO_SIZE;
	session[fd]->max_wdata = WFIFO_SIZE;
	session[fd]->func_recv = recv_to_fifo;
	session[fd]->func_send = send_from_fifo;
	session[fd]->func_parse = default_func_parse;
	return fd;
}

void delete_session(unsigned int fd)
{
	FD_CLR(fd, &readfds);
	if (session[fd]) {
		if (session[fd]->rdata)
			free(session[fd]->rdata);

		if (session[fd]->wdata)
			free(session[fd]->wdata);

		if (session[fd]->session_data)
			free(session[fd]->session_data);

		free(session[fd]);
		session[fd] = NULL;
	}
}

void realloc_fifo(unsigned  fd, long rfifo_size, long wfifo_size)
{
	struct socket_data *s = session[fd];

	if (s->max_rdata != rfifo_size && s->rdata_size < rfifo_size) {
		s->rdata = realloc(s->rdata, rfifo_size);
		if (!s->rdata) {
			debug_output("realloc_fifo: Fail to allocate memory rdata.\n");
			exit(0);
		}
		s->max_rdata  = rfifo_size;
	}
	if (s->max_wdata != wfifo_size && s->wdata_size < wfifo_size) {
		s->wdata = realloc(s->wdata, wfifo_size);
		if (!s->wdata) {
			debug_output("realloc_fifo: Fail to allocate memory wdata.\n");
			exit(1);
		}
		s->max_wdata = wfifo_size;
	}
}

void WFIFOSET(unsigned int fd, int len)
{
	struct socket_data *s = session[fd];

	if (s->wdata_size + len + 16384 > s->max_wdata) {
		unsigned char *sin_addr = (unsigned char *)&s->client_addr.sin_addr;
		debug_output("WFIFOSET: socket #%d (ip: %d.%d.%d.%d):\n", fd, (int)sin_addr[0], (int)sin_addr[1], (int)sin_addr[2], (int)sin_addr[3]);
		debug_output("  wdata (%d bytes used for %d bytes) need to be expanded.\n", s->max_wdata, s->wdata_size);
		debug_output("  (free: %d bytes including 16384 b. for security, available: %d b.).\n", s->max_wdata - s->wdata_size, s->max_wdata - s->wdata_size - 16384);
		debug_output("  packet 0x%X (%d bytes) need mode space.\n", WFIFOW(fd,0), len);
		realloc_fifo(fd, s->max_rdata, s->max_wdata << 1);
		debug_output("  wdata expanded to %d bytes.\n", s->max_wdata);
	}
	s->wdata_size = (s->wdata_size + (len) + 2048 < s->max_wdata) ? s->wdata_size + len : (debug_output("WFIFOSET: %d wdata lost!\n", fd), s->wdata_size);
}

void do_sendrecv(register int next)
{
	int ret;
	unsigned int i;
	fd_set rfd, wfd;
	struct timeval timeout;

	rfd = readfds;
	FD_ZERO(&wfd);
	for (i = 0; i < fd_max; i++) {
		if (!session[i] && FD_ISSET(i, &readfds)) {
			FD_CLR(i, &readfds);
			continue;
		}
		if (!session[i])
			continue;

		if (session[i]->wdata_size)
			FD_SET(i, &wfd);

	}
	timeout.tv_sec = next / 1000;
	timeout.tv_usec = next % 1000 * 1000;
	ret = select(fd_max, &rfd, &wfd, NULL, &timeout);
	if (ret <= 0)
		return;

	for (i = 0; i < fd_max; i++) {
		if (!session[i])
			continue;

		if (FD_ISSET(i, &wfd))
			if(session[i]->func_send)
				session[i]->func_send(i);


		if (FD_ISSET(i, &rfd))
			if (session[i]->func_recv)
				session[i]->func_recv(i);


	}
}

void do_parsepacket(void)
{
	unsigned int i;

	for (i = 0; i < fd_max; i++) {
		if (!session[i])
 			continue;

		if (session[i]->rdata_size == 0 && session[i]->eof == 0)
			continue;

		if (session[i]->func_parse) {
			session[i]->func_parse(i);
			if (!session[i])
				continue;

		}
		RFIFOFLUSH(i);
	}
}
