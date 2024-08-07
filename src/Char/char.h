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

#ifndef _CHAR_H_
#define _CHAR_H_
#include "config.h"
#include "common.h"

#define ACCOUNT_ID_INIT 150000
#define PET_ID_INIT 1
#define MAX_CHAR_SLOTS 9

int SERVER_TYPE = 2;
enum { CHAR_STATE_WAITAUTH, CHAR_STATE_AUTHOK };
enum { LAN_CON, WWW_CON, NO_INFO };
const char *logN = "Character Server Version 1.8.0";

struct {
	long account_id, char_id, login_id1;
	short char_pos;
	int delflag, network;
}auth_fifo[AUTH_FIFO_SIZE];

struct char_session_data
{
	int state, network;
	long account_id, login_id1, login_id2;
	int found_char[MAX_CHAR_SLOTS];
	char sex;
	char email[40];
};

struct mmo_map_server
{
	long ip;
	long xip;
	short port;
	long users;
	char map[MAX_MAP_PER_SERVER][16];
}server[MAX_SERVERS];
#endif
