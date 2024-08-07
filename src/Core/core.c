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

/*------------------------------------------------------------------------
 Module:        Version 1.7.1 - Angel Ex
 Author:        Odin Developer Team Copyrights (c) 2004
 Project:       Project Odin Core
 Creation Date: Dicember 6, 2003
 Modified Date: October 20, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#include "core.h"

fd_set readfds;
unsigned int fd_max = 0;
int timer_data_max = 0;
int timer_data_num = 0;

char core_log[16] = "Critical.log";
extern int SERVER_TYPE;

static int null_parse(unsigned int fd);
static int (*default_func_parse)(unsigned int) = null_parse;
static void (*term_func)(void) = NULL;

static void write_log(int num)
{
	struct timeval tv;
	char msgstr[64];
	char tmpstr[256];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	FILE *fp = NULL;

	if (SERVER_TYPE == 1)
		strcpy(msgstr, "Account Server");

	else if (SERVER_TYPE == 2)
		strcpy(msgstr, "Character Server");

	else
		strcpy(msgstr, "Zone Server");
	
	fp = fopen(core_log, "a");
	switch (num) {
		case SIGABRT:
			fprintf(fp, "===============================================================\n");
			fprintf(fp, "%s Failure\n", msgstr);
			fprintf(fp, "Time Reported: %s \n", tmpstr);
			fprintf(fp, "Force to terminate the program using abort().\n");
			fprintf(fp, "CHKSUM 0x%x\n", num);
			fprintf(fp, "End Report...\n");
			fprintf(fp, "===============================================================\n");
			break;

		case SIGFPE:
			fprintf(fp, "===============================================================\n");
			fprintf(fp, "%s Failure\n", msgstr);
			fprintf(fp, "Time Reported: %s \n", tmpstr);
			fprintf(fp, "An error occurs while trying to do internal calculations.\n");
			fprintf(fp, "CHKSUM 0x%x\n", num);
			fprintf(fp, "End Report...\n");
			fprintf(fp, "===============================================================\n");
			break;

		case SIGSEGV:
			fprintf(fp, "===============================================================\n");
			fprintf(fp, "%s Failure\n", msgstr);
			fprintf(fp, "Time Reported: %s \n", tmpstr);
			fprintf(fp, "Segmentation Violation.\n");
			fprintf(fp, "Memory Overflow or Leak!\n");
			fprintf(fp, "CHKSUM 0x%x\n", num);
			fprintf(fp, "End Report...\n");
			fprintf(fp, "===============================================================\n");
			break;

		case SIGTERM:
			fprintf(fp, "===============================================================\n");
			fprintf(fp, "%s Failure\n", msgstr);
			fprintf(fp, "Time Reported: %s \n", tmpstr);
			fprintf(fp, "Software termination signal from kill.\n");
			fprintf(fp, "Terminate request send to program.\n");
			fprintf(fp, "CHKSUM 0x%x\n", num);
			fprintf(fp, "End Report...\n");
			fprintf(fp, "===============================================================\n");
			break;

		default:
			fprintf(fp, "===============================================================\n");
			fprintf(fp, "%s Failure\n", msgstr);
			fprintf(fp, "Time Reported: %s \n", tmpstr);
			fprintf(fp, "Unkown Error\n");
			fprintf(fp, "CHKSUM 0x%x\n", num);
			fprintf(fp, "End Report...\n");
			fprintf(fp, "===============================================================\n");
			break;
	}
	fclose(fp);
}

/*======================================
 *	CORE: Set Default Packet Func
 *--------------------------------------
 */

void set_defaultparse(int (*defaultparse)(unsigned int))
{
	default_func_parse = defaultparse;
}

/*======================================
 *	CORE: Set Term Function
 *--------------------------------------
 */

void set_termfunc(void (*termfunc)(void))
{
	term_func = termfunc;
}

static int null_parse(unsigned int fd)
{
	RFIFOSKIP(fd, RFIFOREST(fd));
	return 0;
}

/*======================================
 *	CORE: Signal Handler
 *--------------------------------------
 */

void sig_proc(int sn)
{
	unsigned int i;
	switch (sn) {
		case SIGINT:
			do_exit();
			sleep(2);
			exit(0);
			break;

		case SIGABRT:
		case SIGFPE:
		case SIGSEGV:
		case SIGTERM:
	 	 	if (term_func) {
		      	term_func();
			}
			if (!raise(sn)) {
				write_log(sn);
				do_exit();
				sleep(2);
				exit(1);
			}
			for (i = 0; i < fd_max; i++) {
				if (!session[i]) {
					continue;
				}
				close(i);
			}
			write_log(sn);
			do_exit();
			sleep(2);
			exit(2);
			break;
	}
}

/*======================================
 *	CORE: Socket Functions
 *--------------------------------------
 */

void do_socket(void)
{
	FD_ZERO(&readfds);
}

static int recv_to_fifo(unsigned int fd)
{
	if (session[fd]->eof)
		return 1;

	int len = read(fd, session[fd]->rdata + session[fd]->rdata_size, RFIFOSPACE(fd));
	if (len > 0)
		session[fd]->rdata_size += len;

	else if (len <= 0)
		session[fd]->eof = 1;

	return 0;
}

int send_from_fifo(unsigned int fd)
{
	if (session[fd]->eof)
		return 1;

	int len = write(fd, session[fd]->wdata, session[fd]->wdata_size);
	if (len > 0) {
		if (len < session[fd]->wdata_size) {
			memmove(session[fd]->wdata, session[fd]->wdata + len, session[fd]->wdata_size - len);
			session[fd]->wdata_size -= len;
		}
		else
			session[fd]->wdata_size = 0;

	}
	else
		session[fd]->eof = 1;

	return 0;
}

int connect_client(unsigned int listen_fd)
{
	unsigned int fd;
	int yes = 1;
	struct sockaddr_in client_address;
	unsigned int len = sizeof(client_address);

	fd = accept(listen_fd,(struct sockaddr*)&client_address, &len);
	if (fd_max <= fd)
		fd_max = fd + 1;

	fcntl(fd, F_SETFL, O_NONBLOCK);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&yes, sizeof yes);
#endif
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof yes);

	if (fd == (unsigned)-1)
		perror("accept");

	else
		FD_SET(fd, &readfds);

	session[fd] = calloc(sizeof(*session[fd]), 1);
	if (!session[fd]) {
		fprintf(stderr, "Connect Client Fail!\n");
		sleep(2);
		exit(0);
	}
	memset(session[fd], 0, sizeof(*session[fd]));
	session[fd]->rdata = calloc(RFIFO_SIZE, 1);
	if (!session[fd]->rdata) {
		fprintf(stderr, "Connect Client RDATA Error!\n");
		sleep(2);
		exit(1);
	}
	session[fd]->wdata = calloc(WFIFO_SIZE, 1);
	if (!session[fd]->wdata) {
		fprintf(stderr, "Connect Client WDATA Error!\n");
		sleep(2);
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

int make_listen_port(short port)
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
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(port);

	if (bind(fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
		perror("bind");
		sleep(2);
		exit(0);
	}
	if (listen(fd, 5) == -1) {
		perror("listen");
		sleep(2);
		exit(1);
	}
	FD_SET(fd, &readfds);
	session[fd] = calloc(sizeof(*session[fd]), 1);
	if (!session[fd]) {
		fprintf(stderr, "Make Listen Port Fail!\n");
		sleep(2);
		exit(2);
	}
	memset(session[fd], 0, sizeof(*session[fd]));
	session[fd]->func_recv = connect_client;
	return fd;
}

int make_connection(long ip, short port)
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
		fprintf(stderr, "Make Connection Fail!\n");
		sleep(2);
		exit(1);
	}
	memset(session[fd], 0, sizeof(*session[fd]));
	session[fd]->rdata = calloc(RFIFO_SIZE, 1);
	if (!session[fd]) {
		fprintf(stderr, "Make Connection RDATA Error!\n");
		sleep(2);
		exit(2);
	}
	session[fd]->wdata = calloc(WFIFO_SIZE, 1);
	if (!session[fd]) {
		fprintf(stderr, "Make Connection RDATA Error!\n");
		sleep(2);
		exit(3);
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

		session[fd] = NULL;
	}
}

static void do_sendrecv(long next)
{
	int ret;
	auto unsigned int i;
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

		if (FD_ISSET(i, &wfd)) {
			if(session[i]->func_send)
				session[i]->func_send(i);
		}
		if (FD_ISSET(i, &rfd)) {
			if (session[i]->func_recv)
				session[i]->func_recv(i);
		}
	}
}

static void do_parsepacket(void)
{
	auto unsigned int i;

	for (i = 0; i < fd_max; i++) {
		if (!session[i])
 			continue;

		if ((session[i]->rdata_size == 0) && (session[i]->eof == 0))
			continue;

		if (session[i]->func_parse) {
			session[i]->func_parse(i);
			if (!session[i])
				continue;
		}
		RFIFOFLUSH(i);
	}
}

void realloc_fifo(unsigned  fd, long rfifo_size, long wfifo_size)
{
	struct socket_data *s = session[fd];
	if ((s->max_rdata != rfifo_size) && (s->rdata_size < rfifo_size)) {
		s->rdata = realloc(s->rdata, rfifo_size);
		if (!s->rdata) {
			fprintf(stderr, "Realloc Fifo RDATA Error!\n");
			sleep(2);
			exit(0);
		}
		s->max_rdata  = rfifo_size;
	}
	if ((s->max_wdata != wfifo_size) && (s->wdata_size < wfifo_size)) {
		s->wdata = realloc(s->wdata, wfifo_size);
		if (!s->wdata) {
			fprintf(stderr, "Realloc Fifo WDATA Error!\n");
			sleep(2);
			exit(1);
		}
		s->max_wdata = wfifo_size;
	}
}

void WFIFOSET(unsigned int fd, int len)
{
	struct socket_data *s = session[fd];
	if (s->wdata_size + len + 16384 > s->max_wdata ) {
		realloc_fifo(fd, s->max_rdata, s->max_wdata << 1);
		printf("socket: %d wdata expanded to %d bytes.\n",fd, s->max_wdata);
	}
	s->wdata_size = (s->wdata_size + (len) + 2048 < s->max_wdata) ?
		 s->wdata_size + len : (printf("socket: %d wdata lost!\n", fd), s->wdata_size);
}

/*======================================
 *	CORE: Timer Functions
 *--------------------------------------
 */

unsigned int gettick(void)
{
	struct timeval st;
	gettimeofday(&st, NULL);
	return ((st.tv_sec * 1000) + (st.tv_usec / 1000));
}

static long do_timer(unsigned int tick)
{
	auto int i;
	long nextmin = 1000;

	for (i = 1; i < timer_data_num; i++) {
		if (timer_data[i]) {
			if ((int)(timer_data[i]->tick - tick) < nextmin)
				nextmin = (long)(timer_data[i]->tick - tick);

			if (((int)(timer_data[i]->tick - tick)) <= 0) {
				if (timer_data[i]->func)
					timer_data[i]->func(i, tick, timer_data[i]->id, timer_data[i]->data);

				if (timer_data[i]) {
					switch (timer_data[i]->type) {
						case TIMER_ONCE_AUTODEL:
							free(timer_data[i]);
							timer_data[i] = NULL;
							break;
						case TIMER_INTERVAL:
							timer_data[i]->tick += timer_data[i]->interval;
							break;
					}
				}
			}
		}
	}
	if (nextmin < 10)
		nextmin = 10;

	return nextmin;
}

int add_timer(unsigned int tick, int (*func)(int, unsigned int, int, int), int id, int data)
{
	int i;
	struct TimerData *td = malloc(sizeof(struct TimerData));
	for (i = 1; i < timer_data_num; i++) {
		if (!timer_data[i])
			break;
	}
	if ((i >= timer_data_num) && (i >= timer_data_max)) {
		if (timer_data_max == 0) {
			timer_data_max = 256;
			timer_data = malloc(sizeof(struct TimerData*)*timer_data_max);
			timer_data[0] = NULL;
		}
		else {
			timer_data_max += 256;
			timer_data = realloc(timer_data, sizeof(struct TimerData*) * timer_data_max);
		}
	}
	td->tick = tick;
	td->func = func;
	td->id = id;
	td->data = data;
	td->type = TIMER_ONCE_AUTODEL;
	td->interval = 1000;
	timer_data[i] = td;
	if (i >= timer_data_num)
		timer_data_num = i + 1;

	return i;
}

int delete_timer(int id, int (*func)(int, unsigned int, int, int))
{
	if (id <= 0 || id >= timer_data_num || !timer_data[id])
		return 1;

	if (timer_data[id]->func != func)
		return 2;

	free(timer_data[id]);
	timer_data[id] = NULL;
	return 0;
}

/*======================================
 *	CORE: Lowercase Uppercases
 *--------------------------------------
 */

char *strtolower(char *string)
{
	int i = 0;

	while (string[i] != '\0') {
		if (string[i] >= 65 && string[i] <= 90) {
			string[i] = string[i] + 32;
		}
		i++;
	}
	return string;
}

/*======================================
 *	CORE: Copy File
 *--------------------------------------
 */

void filecopy(FILE *from, FILE *to)
{
	int i;

	while ((i = getc(from)) != EOF) {
		putc(i, to);
	}
}

/*======================================
 *	CORE: RandN
 *--------------------------------------
 */

int randN(int n)
{
	int i;

	while ((i = (int)((double)rand() / (double)RAND_MAX * n)) == n);
	return i;
}

/*======================================
 *	CORE: Program Initialization
 *--------------------------------------
 */

int main()
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGABRT, sig_proc);
	signal(SIGFPE,  sig_proc);
	signal(SIGINT,  sig_proc);
	signal(SIGSEGV, sig_proc);
	signal(SIGTERM, sig_proc);
	do_socket();
	do_init();

	for (;;) {
		do_sendrecv(do_timer(gettick()));
		do_parsepacket();
	}
	return 0;
}
