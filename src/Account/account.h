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

#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_
#include "config.h"
#include "common.h"

#define ACCOUNT_ID_INIT 2000001
#define GM_ACCOUNT_ID_INIT 100001
#define SUB_GM_ACCOUNT_ID_INIT 95001
#define END_ACCOUNT_NUM 100000000

int SERVER_TYPE = 1;
enum { LAN_CON, WWW_CON, NO_INFO };
const char *logN = "Account Server Version 1.8.0";

struct {
	long account_id, login_id1, login_id2;
	int delflag, network;
}auth_fifo[AUTH_FIFO_SIZE];

struct {
	long account_id;
	int ban, expired_id, server_dis, msg, state, logincount, timer;
	char userid[24], pass[24], email[40], msgerr[24], lastlogin[24], sex, ip[16];
}*auth_dat;

struct mmo_account
{
	char *userid;
	char *passwd;
	char sex;
	int ver_1;
	short ver_2;
	long account_id;
	long login_id1;
	long login_id2;
	long char_id;
	char lastlogin[24];
	char ip[16];
};

struct mmo_char_server
{
	char name[24];
	long ip;
	long xip;
	short port;
	long users;
	int new;
	int maintenance;
}server[MAX_SERVERS];
#endif
