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
 Project:       Project Odin Account Server
 Creation Date: Dicember 6, 2003
 Modified Date: October 20, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#include "config.h"

#define AUTH_FIFO_SIZE 256
#define MAX_SERVERS 30
#define ACCOUNT_ID_INIT 31
#define GM_ACCOUNT_ID_INIT 100001
#define SUB_GM_ACCOUNT_ID_INIT 95000

enum network { LAN_CON, WWW_CON, NO_INFO };
const char *logN = "Account Server Version 1.7.1";
const int SERVER_TYPE = 1;
const int PacketLenTable[0x100] = {
	1, 0, 2, 0, 23, 0, 0, 0, 0, 0, 0, 0, 58, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 75, 0, 0, 0, 0, 55, 0, 1, 0, 1, 0, 1, 0, 79, 0, 0, 0,
	0, 0, 31, 0, 0, 0, 79, 0, 70, 0, 0, 0, 80, 0, 0, 0, 0, 56, 90, 0,
	10, 0, 0, 70, 0, 55, 0, 0, 0, 0, 67, 0, 79, 0, 0, 0, 0, 0, 10, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 0, 23, 0, 0, 0, 0,
	55, 0, 0, 0, 0, 79, 23, 0, 0, 0, 0, 0, 0, -16, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 13, 0, 0, 0, 0, 0, 23, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

struct {
	int account_id, login_id1, login_id2;
	int delflag, network;
}auth_fifo[AUTH_FIFO_SIZE];

struct {
	int account_id, ban, expired_id, server_dis, msg, state, logincount, auth_timer;
	char userid[24], pass[24], email[60], msgerr[24], lastlogin[24], sex, ip[16];
}*auth_dat;

struct mmo_account
{
	char *userid;
	char *passwd;
	char sex;
	int ver_1;
	int ver_2;
	long account_id;
	long login_id1;
	long login_id2;
	long char_id;
	char lastlogin[24];
	char ip[16];
};

struct mmo_char_server
{
	char name[32];
	long ip;
	long xip;
	short port;
	int users;
	int maintenance;
	int new;
}server[MAX_SERVERS];
