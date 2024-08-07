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

#ifndef _SOCKET_H_
#define _SOCKET_H_

#define RFIFOP(fd,pos) (session[fd]->rdata + session[fd]->rdata_pos + (pos))
#define RFIFOB(fd,pos) (*(unsigned char*)(session[fd]->rdata + session[fd]->rdata_pos + (pos)))
#define RFIFOW(fd,pos) (*(unsigned short*)(session[fd]->rdata + session[fd]->rdata_pos + (pos)))
#define RFIFOL(fd,pos) (*(unsigned int*)(session[fd]->rdata + session[fd]->rdata_pos + (pos)))
#define RFIFOSKIP(fd,len) ((session[fd]->rdata_size - session[fd]->rdata_pos - (len) < 0) ? (debug_output("RFIFOSKIP: Too many skips at %s:%d.\n", __FILE__, __LINE__), exit(1)) : (session[fd]->rdata_pos += (len)))
#define RFIFOREST(fd) (session[fd]->rdata_size - session[fd]->rdata_pos)
#define RFIFOFLUSH(fd) (memmove(session[fd]->rdata, RFIFOP(fd,0), RFIFOREST(fd)), session[fd]->rdata_size = RFIFOREST(fd), session[fd]->rdata_pos = 0)
#define RFIFOSPACE(fd) (session[fd]->max_rdata - session[fd]->rdata_size)
#define RBUFP(p,pos) (((unsigned char*)(p)) + (pos))
#define RBUFB(p,pos) (*(unsigned char*)((p) + (pos)))
#define RBUFW(p,pos) (*(unsigned short*)((p) + (pos)))
#define RBUFL(p,pos) (*(unsigned int*)((p) + (pos)))

#define WFIFOSPACE(fd) (session[fd]->max_wdata - session[fd]->wdata_size)
#define WFIFOP(fd,pos) (session[fd]->wdata + session[fd]->wdata_size + (pos))
#define WFIFOB(fd,pos) (*(unsigned char*)(session[fd]->wdata + session[fd]->wdata_size + (pos)))
#define WFIFOW(fd,pos) (*(unsigned short*)(session[fd]->wdata + session[fd]->wdata_size + (pos)))
#define WFIFOL(fd,pos) (*(unsigned int*)(session[fd]->wdata + session[fd]->wdata_size + (pos)))
#define WBUFP(p,pos) (((unsigned char*)(p)) + (pos))
#define WBUFB(p,pos) (*(unsigned char*)((p) + (pos)))
#define WBUFW(p,pos) (*(unsigned short*)((p) + (pos)))
#define WBUFL(p,pos) (*(unsigned int*)((p) + (pos)))

#define RFIFO_SIZE 32767
#define WFIFO_SIZE 65536
#define FIFO_SIZE (128 * 1024)

extern unsigned int fd_max;

struct socket_data
{
	int eof;
	unsigned char *rdata, *wdata;
	int max_rdata, max_wdata;
	int rdata_size, wdata_size;
	int rdata_pos;
	unsigned int (*func_recv)(unsigned int);
	unsigned int (*func_send)(unsigned int);
	int (*func_parse)(unsigned int);
	void *session_data;
	struct sockaddr_in client_addr;
}*session[FD_SETSIZE];

void set_defaultparse(int (*defaultparse)(unsigned int));
void do_socket(void);
unsigned int make_listen_port(short);
unsigned int make_connection(long, short);
void delete_session(unsigned int fd);
void realloc_fifo(unsigned int, long , long);
void WFIFOSET(unsigned int, int);
void do_sendrecv(register int);
void do_parsepacket(void);
#endif
