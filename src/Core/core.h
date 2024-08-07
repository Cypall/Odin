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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define RFIFOP(fd,pos) (session[fd]->rdata+session[fd]->rdata_pos+(pos))
#define RFIFOB(fd,pos) (*(unsigned char*)(session[fd]->rdata+session[fd]->rdata_pos+(pos)))
#define RFIFOW(fd,pos) (*(unsigned short*)(session[fd]->rdata+session[fd]->rdata_pos+(pos)))
#define RFIFOL(fd,pos) (*(unsigned int*)(session[fd]->rdata+session[fd]->rdata_pos+(pos)))
#define RFIFOSKIP(fd,len) ((session[fd]->rdata_size-session[fd]->rdata_pos-(len)<0) ? (fprintf(stderr,"too many skip\n"),exit(1)) : (session[fd]->rdata_pos+=(len)))
#define RFIFOREST(fd) (session[fd]->rdata_size-session[fd]->rdata_pos)
#define RFIFOFLUSH(fd) (memmove(session[fd]->rdata,RFIFOP(fd,0),RFIFOREST(fd)),session[fd]->rdata_size=RFIFOREST(fd),session[fd]->rdata_pos=0)
#define RFIFOSPACE(fd) (session[fd]->max_rdata-session[fd]->rdata_size)

#define RBUFP(p,pos) (((unsigned char*)(p))+(pos))
#define RBUFB(p,pos) (*(unsigned char*)RBUFP((p),(pos)))
#define RBUFW(p,pos) (*(unsigned short*)RBUFP((p),(pos)))
#define RBUFL(p,pos) (*(unsigned int*)RBUFP((p),(pos)))

#define WFIFOSPACE(fd) (session[fd]->max_wdata-session[fd]->wdata_size)
#define WFIFOP(fd,pos) (session[fd]->wdata+session[fd]->wdata_size+(pos))
#define WFIFOB(fd,pos) (*(unsigned char*)(session[fd]->wdata+session[fd]->wdata_size+(pos)))
#define WFIFOW(fd,pos) (*(unsigned short*)(session[fd]->wdata+session[fd]->wdata_size+(pos)))
#define WFIFOL(fd,pos) (*(unsigned int*)(session[fd]->wdata+session[fd]->wdata_size+(pos)))

#define WBUFP(p,pos) (((unsigned char*)(p))+(pos))
#define WBUFB(p,pos) (*(unsigned char*)WBUFP((p),(pos)))
#define WBUFW(p,pos) (*(unsigned short*)WBUFP((p),(pos)))
#define WBUFL(p,pos) (*(unsigned int*)WBUFP((p),(pos)))

#define TIMER_ONCE_AUTODEL 0
#define TIMER_INTERVAL 1
#define RFIFO_SIZE 65536
#define WFIFO_SIZE 65536
#define FIFO_SIZE (128 * 1024)
#define MAX_BUFFER 256
#define MAX_LINE 1024

struct socket_data
{
	int eof;
	unsigned char *rdata, *wdata;
	int max_rdata, max_wdata;
	int rdata_size, wdata_size;
	int rdata_pos;
	int (*func_recv)(unsigned int);
	int (*func_send)(unsigned int);
	int (*func_parse)(unsigned int);
	void *session_data;
	struct sockaddr_in client_addr;
}*session[FD_SETSIZE];

struct TimerData
{
	unsigned int tick;
	int (*func)(int, unsigned int, int, int);
	int id;
	int data;
	int type;
	int interval;
}**timer_data;

void set_defaultparse(int (*defaultparse)(unsigned int));
void set_termfunc(void (*termfunc)(void));
int make_listen_port(short);
int make_connection(long, short);
void delete_session(unsigned int);
void realloc_fifo(unsigned int, long , long);
void WFIFOSET(unsigned int, int);
int add_timer(unsigned int, int (*)(int, unsigned int, int,int), int, int);
int delete_timer(int, int (*)(int, unsigned int, int, int));
unsigned int gettick(void);
char *strtolower(char *);
void filecopy(FILE *, FILE *);
int randN(int);
int do_init(void);
int do_exit(void);
