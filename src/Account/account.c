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

#include "core.h"
#include "account.h"

/*
 * Network releate variables
 */

int server_fd[MAX_SERVERS];
int server_num;
int listen_port_fd;
short accountport = 6900;

/*
 * Set timer variables
 */

int check_backup_timer;

/*
 * Configuration variables
 */

int allow_gm_mode;
int secure_mode;
int closesrv;
int do_backup;
int do_timer_bak = 0;
int max_users = 0;
int auth_num = 0;
int auth_max = 0;
int auth_fifo_pos = 0;

/*
 * Give account ids default values
 */

long account_id_count 		= ACCOUNT_ID_INIT;
long gm_account_id_count	= GM_ACCOUNT_ID_INIT;
long sub_gm_account_id_count	= SUB_GM_ACCOUNT_ID_INIT;
int mmo_auth_delay(int tid, unsigned int tick, int id, int data);

/*======================================
 *	ACCOUNT: Account TxT Managment
 *--------------------------------------
 */

void mmo_auth_init(void)
{
	int i;
	int account_id, logincount, ban, expired_id, server_dis, msg, state;
	char line[MAX_LINE], userid[24], pass[24], email[60], msgerr[24], lastlogin[24], sex;

	FILE *fp = fopen("save/accounts/accounts.txt", "r");
	auth_dat = calloc(sizeof(auth_dat[0]) * MAX_BUFFER, 1);
	if (!auth_dat)
		exit(0);

	auth_max = MAX_BUFFER;
	if (fp) {
		while (fgets(line, 1023, fp) != NULL) {
			i = sscanf(line,"%d,%[^,],%[^,],%c,%[^,],%[^,],%d,%d,%d,%d,%[^,],%d,%d",
			&account_id, userid, pass, &sex, email, lastlogin, &ban, &expired_id,
			&server_dis, &msg, msgerr, &state, &logincount);
			if (i >= 12) {
				if (auth_num >= auth_max) {
					auth_max += MAX_BUFFER;
					auth_dat = realloc(auth_dat, sizeof(auth_dat[0]) * auth_max);
				}
				memset(&auth_dat[auth_num], 0, sizeof(auth_dat[auth_num]));
				auth_dat[auth_num].account_id = account_id;
				strncpy(auth_dat[auth_num].userid, userid, 24);
				strncpy(auth_dat[auth_num].pass, pass, 24);
				strncpy(auth_dat[auth_num].email, email, 60);
				strncpy(auth_dat[auth_num].msgerr, msgerr, 24);
				strncpy(auth_dat[auth_num].lastlogin, lastlogin, 24);
				auth_dat[auth_num].ban = ban;
				auth_dat[auth_num].expired_id = expired_id;
				auth_dat[auth_num].server_dis = server_dis;
				auth_dat[auth_num].msg = msg;
				auth_dat[auth_num].state = state;
				auth_dat[auth_num].sex = sex == 'S' ? 2 : sex == '1';
				if (i >= 13)
					auth_dat[auth_num].logincount = logincount;

				else
					auth_dat[auth_num].logincount = 1;

				auth_dat[auth_num].auth_timer = 0;
				auth_num++;
				if ((account_id >= gm_account_id_count) && (account_id >= GM_ACCOUNT_ID_INIT))
					gm_account_id_count = ++account_id;

				else if ((account_id >= sub_gm_account_id_count) && (account_id < GM_ACCOUNT_ID_INIT - 1))
					sub_gm_account_id_count = ++account_id;

				else if ((account_id >= account_id_count) && (account_id < SUB_GM_ACCOUNT_ID_INIT))
					account_id_count = ++account_id;
			}
		}
		fclose(fp);
	}
	else
		exit(1);
}

static void mmo_auth_sync(void)
{
	int i;
	FILE *fp = fopen("save/accounts/accounts.txt", "w");
	if (fp) {
		for (i = 0; i < auth_num; i++) {
			fprintf (fp, "%d,%s,%s,%c,%s,%s,%d,%d,%d,%d,%s,%d,%d\t%s\n",
			auth_dat[i].account_id, auth_dat[i].userid, auth_dat[i].pass,
			auth_dat[i].sex == 2 ? 'S' : (auth_dat[i].sex ? '1' : '0'),
			auth_dat[i].email, auth_dat[i].lastlogin, auth_dat[i].ban,
			auth_dat[i].expired_id, auth_dat[i].server_dis, auth_dat[i].msg,
			auth_dat[i].msgerr, auth_dat[i].state, auth_dat[i].logincount, auth_dat[i].ip);
		}
	}
	fclose(fp);
}

static int mmo_auth(int fd, struct mmo_account *account)
{
	int i, j, p, len;
	int flag = 0;
	int user_count, gmode;
	int newaccount = 0, newgm = 0, isAccount = 0;
	char line[MAX_LINE], userid[24], passwd[24], email[60], sex, temp[70];
	struct timeval tv;
	char tmpstr[256];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	FILE *fp = NULL;

	len = strlen(account->userid) - 2;
	if ((fp = fopen("save/accounts/addplayer.txt", "r"))) {
		while (fgets(line,1023,fp) != NULL) {
			p = sscanf(line, "%[^,],%[^,],%c,%[^,],%d", userid, passwd, &sex, email, &gmode);
			if (p != 5) {
				fclose(fp);
				goto Continue;
			}
			if (auth_num >= auth_max) {
				auth_max += MAX_BUFFER;
				auth_dat = realloc(auth_dat, sizeof(auth_dat[0]) * auth_max);
			}
			if (gmode == 0)
				auth_dat[auth_num].account_id = account_id_count++;

			else if (gmode == 1)
				auth_dat[auth_num].account_id = sub_gm_account_id_count++;

			else if (gmode == 2)
				auth_dat[auth_num].account_id = gm_account_id_count++;

			strtolower(userid);
			for (i = 0; i < auth_max; i++) {
				strncpy(temp, auth_dat[i].userid, 24);
				if (strstr(strtolower(temp), userid) != NULL)
					isAccount++;

				if (strlen(userid) < 4)
					isAccount++;

			}
			if (isAccount == 0 ) {
				strncpy(auth_dat[auth_num].userid, userid, 24);
				strncpy(auth_dat[auth_num].pass, passwd, 24);
				strncpy(auth_dat[auth_num].email, email, 60);
				strcpy(auth_dat[auth_num].msgerr, "3 days");
				auth_dat[auth_num].sex = sex;
				auth_dat[auth_num].ban = 0;
				auth_dat[auth_num].expired_id = 0;
				auth_dat[auth_num].server_dis = 0;
				auth_dat[auth_num].msg = 0;
				auth_dat[auth_num].state = 0;
				auth_dat[auth_num].logincount = 0;
				auth_num++;
			}
		}
	}
	fclose(fp);
	fp = fopen("save/accounts/addplayer.txt", "w");
	fclose(fp);

	Continue:
	/* For _M/_m _F/_f Registration */
	if(	(account->userid[len] == '_') &&
		(strcmp(account->userid, "\n") != 0) && (
		(account->userid[len+1] == '0') ||
		(account->userid[len+1] == '1') ||
		(account->userid[len+1] == 'M') ||
		(account->userid[len+1] == 'F')))
		{
		if (account->userid[len+1] == 'M')
			account->userid[len+1] = '1';

		else if (account->userid[len+1] == 'F')
			account->userid[len+1] = '0';

		if (secure_mode == 1)
			newaccount = 0;

		else
			newaccount = 1;

		account->userid[len] = 0;
		if (strlen(account->userid) < 4)
			newaccount = 0;

	}
	else if((account->userid[len] == '_') &&
		(strcmp(account->userid, "\n") != 0) && (
		(account->userid[len+1] == '2') ||
		(account->userid[len+1] == '3') ||
		(account->userid[len+1] == 'm') ||
		(account->userid[len+1] == 'f')))
		{
		if (account->userid[len+1] == 'm')
			account->userid[len+1] = '3';

		else if (account->userid[len+1] == 'f')
			account->userid[len+1] = '2';

		if (allow_gm_mode == 1)
			newgm = 1;

		else
			newgm = 0;

		account->userid[len] = 0;
		if (strlen(account->userid) < 4)
			newgm = 0;

	}
	for (i = 0; i < auth_num; i++) {
		if (strncmp(account->userid, auth_dat[i].userid, 24) == 0)
			break;

		else
			strtolower(account->userid);
	}
	if (i != auth_num) {
		if (strncmp(account->passwd, auth_dat[i].pass, 24))
			return 1;

		else if (strncmp(account->passwd, auth_dat[i].pass, 24) && (newaccount))
			return 0;

		else if (strncmp(account->passwd, auth_dat[i].pass, 24) && (newgm))
			return 0;

	}
	else {
		if ((newaccount == 0) && (newgm == 0))
			return 0;

		else if (newaccount == 1) {
			printf("\033[1;40m**New Account Registered**\033[0m\n");
			if (auth_num >= auth_max) {
				auth_max += MAX_BUFFER;
				auth_dat = realloc(auth_dat, sizeof(auth_dat[0]) * auth_max);
			}
			auth_dat[i].account_id = account_id_count++;
			strncpy(auth_dat[i].userid, account->userid, 24);
			strncpy(auth_dat[i].pass, account->passwd, 24);
			auth_dat[i].sex = account->userid[len + 1] == '1';
			strcpy(auth_dat[i].email, "sys@srv.com");
			strcpy(auth_dat[i].msgerr, "3 days");
			auth_dat[i].ban = 0;
			auth_dat[i].expired_id = 0;
			auth_dat[i].server_dis = 0;
			auth_dat[i].msg = 0;
			auth_dat[i].state = 0;
			auth_dat[i].logincount = 0;
			auth_num++;
		}
		else if (newgm == 1) {
			printf("\033[1;40m**GM Account Registered**\033[0m\n");
			if (auth_num >= auth_max) {
				auth_max += MAX_BUFFER;
				auth_dat = realloc(auth_dat, sizeof(auth_dat[0]) * auth_max);
			}
			auth_dat[i].account_id = gm_account_id_count++;
			strncpy(auth_dat[i].userid, account->userid, 24);
			strncpy(auth_dat[i].pass, account->passwd, 24);
			strcpy(auth_dat[i].email, "sys@srv.com");
			strcpy(auth_dat[i].msgerr, "3 days");
			auth_dat[i].sex = account->userid[len + 1] == '3';
			auth_dat[i].ban = 0;
			auth_dat[i].expired_id = 0;
			auth_dat[i].server_dis = 0;
			auth_dat[i].msg = 0;
			auth_dat[i].state = 0;
			auth_dat[i].logincount = 0;
			auth_num++;
		}
	}
	if (closesrv == 1)
		flag = 1;

	if (auth_dat[i].state && auth_dat[i].account_id > 30)
		flag = 8;

	if (account->ver_1 < 18)
		return 5;

	if (auth_dat[i].ban)
		return 4;

	if (auth_dat[i].expired_id)
		return 2;

	if (auth_dat[i].server_dis)
		return 3;

	if (auth_dat[i].msg)
		return 6;
	if (max_users != 0) {
		user_count = 0;
		for (j = 0; j < MAX_SERVERS; j++) {
			if (server[j].users > user_count)
				user_count = server[j].users;
		}
		if (user_count >= max_users)
			return 7;
	}
	if (flag) {
		memset(WFIFOP(fd, 0), 0, 23);
		WFIFOW(fd, 0) = 0x81;
		WFIFOB(fd, 2) = flag;
		WFIFOSET(fd, PacketLenTable[0x81]);
		return -1;
	}
	account->account_id = auth_dat[i].account_id;
	account->login_id1 = rand();
	account->login_id2 = rand();
	memcpy(account->lastlogin, auth_dat[i].lastlogin, 24);
	memcpy(auth_dat[i].lastlogin, tmpstr, 24);
	memcpy(auth_dat[i].ip, account->ip, 16);
	account->sex = auth_dat[i].sex;
	auth_dat[i].state = 1;
	auth_dat[i].auth_timer = add_timer(gettick() + 25000, mmo_auth_delay, auth_dat[i].account_id, 0);
	auth_dat[i].logincount++;
	mmo_auth_sync();
	return 100;
}

/*======================================
 *	ACCOUNT: Parse Functions
 *--------------------------------------
 */

int parse_fromchar(unsigned fd)
{
	int i, j, id;

	for (id = 0; id < MAX_SERVERS; id++) {
		if ((unsigned)server_fd[id] == fd)
     			break;
	}
	if (id == MAX_SERVERS)
   		session[fd]->eof = 1;

 	if (session[fd]->eof) {
   		for (i = 0; i < MAX_SERVERS; i++) {
   			if ((unsigned)server_fd[i] == fd)
				server_fd[i] = -1;
		}
		close(fd);
		delete_session(fd);
		return 0;
	}
	while (RFIFOREST(fd) >= 2) {
		switch (RFIFOW(fd, 0)) {
			case 0x2712:
				if (RFIFOREST(fd) < 15)
					return 0;

				for (i = 0; i < AUTH_FIFO_SIZE; i++) {
					if ((auth_fifo[i].account_id == (signed)RFIFOL(fd, 2))
					&& (auth_fifo[i].login_id1 == (signed)RFIFOL(fd, 6))
					&& (!auth_fifo[i].delflag)) {
						auth_fifo[i].delflag = 1;
						break;
					}
				}
				WFIFOW(fd, 0) = 0x2713;
				WFIFOL(fd, 2) = RFIFOL(fd, 2);

				if (i != AUTH_FIFO_SIZE)
					WFIFOB(fd, 6) = 0;

				else
					WFIFOB(fd, 6) = 1;

				WFIFOB(fd, 7) = auth_fifo[i].network ;

				if (i != AUTH_FIFO_SIZE) {
					for (j = 0; j < auth_num; j++) {
						if (auth_fifo[i].account_id == auth_dat[j].account_id) {
							memcpy(WFIFOP(fd, 8), auth_dat[j].email, 60);
							break;
						}
					}
				}
				WFIFOSET(fd, 48);
				RFIFOSKIP(fd, 15);
				break;

			case 0x2714:
				if (RFIFOREST(fd) < 6)
					return 0;

				server[id].users = RFIFOL(fd, 2);
				RFIFOSKIP(fd, 6);
				break;

			case 0x30a1:
				if (RFIFOREST(fd) < 7)
					return 0;

				for (i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == (signed)RFIFOL(fd, 2))
						break;
				}
				if (i != auth_num)
					auth_dat[i].state = RFIFOB(fd, 6);

				mmo_auth_sync();
				RFIFOSKIP(fd, 7);
				break;

			case 0x30fa:
				if (RFIFOREST(fd) < 6)
					return 0;

				for (i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == (signed)RFIFOL(fd, 2))
						break;

				}
				if (i != auth_num)
					auth_dat[i].ban = 1;

				mmo_auth_sync();
				RFIFOSKIP(fd, 6);
				break;

			default:
				close(fd);
				session[fd]->eof = 1;
				return 0;
		}
	}
	return 0;
}

static int check_ip(char ip0, char ip1, char ip2, char ip3)
{
	char line[MAX_BUFFER];
	int p0, p1, p2, p3, i = 0;
	FILE *fp = fopen("banned.ini", "r");
	if (fp) {
		while (fgets(line, 255, fp)) {
			sscanf(line, "%d.%d.%d.%d", &p0, &p1, &p2, &p3);
			if (p0 == ip0 && p1 == ip1 && p2 == ip2 && p3 == ip3) {
				i = 141;
				break;
			}
		}
	}
	fclose(fp);
	return i;
}

int parse_login(unsigned int fd)
{
	int i, result, net, len;
	long servernet, clientnet, serverip;
	char buf[16];
	unsigned char *ip = (unsigned char *) & session[fd]->client_addr.sin_addr;
	struct mmo_account account;

	if (session[fd]->eof) {
		for (i = 0; i < MAX_SERVERS; i++) {
			if ((unsigned)server_fd[i] == fd)
				server_fd[i] = -1;

			close(fd);
			delete_session(fd);
			return 0;
		}
	}
	while (RFIFOREST(fd) >= 2) {
		switch (RFIFOW(fd, 0)) {
			case 0x64:
				if (RFIFOREST(fd) < 55)
					return 0;

				reload_config();
				account.ver_1 = RFIFOL(fd, 2);
				account.userid = RFIFOP(fd, 6);
				account.passwd = RFIFOP(fd, 30);
				account.ver_2 = RFIFOB(fd, 54);
				len = check_ip(ip[0], ip[1], ip[2], ip[3]);
				if (len > 0) {
					memset(WFIFOP(fd, 0), 0, 23);
					WFIFOW(fd, 0) = 0x81;
					WFIFOB(fd, 2) = len;
					WFIFOSET(fd, PacketLenTable[0x81]);
				}
				sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
				memcpy(account.ip, buf, 16);
				result = mmo_auth(fd, &account);
				net = NO_INFO;
				if (result == 100) {
					server_num = 0;
					for (i = 0; i < MAX_SERVERS; i++) {
						if (server_fd[i] >= 0) {
							servernet =  server[i].ip & 0x00ffffff;
							clientnet = *(unsigned long *) &session[fd]->client_addr.sin_addr & 0x00ffffff;
							serverip = (servernet == clientnet) ? server[i].ip : server[i].xip;
							net = (servernet == clientnet) ? LAN_CON : WWW_CON;

							WFIFOL(fd, 47 + server_num * 32) = serverip;
							WFIFOW(fd, 47 + server_num * 32 + 4) = server[i].port;
							memcpy(WFIFOP(fd, 47 + server_num * 32 + 6), server[i].name, 16);
							WFIFOW(fd, 47 + server_num * 32 + 26) = server[i].users;
							WFIFOW(fd, 47 + server_num * 32 + 28) = server[i].maintenance;
							WFIFOW(fd, 47 + server_num * 32 + 30) = server[i].new;
							server_num++;
						}
					}
					WFIFOW(fd, 0) = 0x69;
					WFIFOW(fd, 2) = 47 + server_num * 32;
					WFIFOL(fd, 4) = account.login_id1;
					WFIFOL(fd, 8) = account.account_id;
					WFIFOL(fd, 12) = account.login_id2;
					WFIFOL(fd, 16) = 0;
					memcpy(WFIFOP(fd, 20), account.lastlogin, 24);
					WFIFOW(fd, 44) = 0;
					WFIFOB(fd, 46) = account.sex;
					WFIFOSET(fd, 47 + server_num * 32);
					if (auth_fifo_pos >= AUTH_FIFO_SIZE)
						auth_fifo_pos = 0;

					auth_fifo[auth_fifo_pos].account_id = account.account_id;
					auth_fifo[auth_fifo_pos].login_id1 = account.login_id1;
					auth_fifo[auth_fifo_pos].login_id2 = account.login_id2;
					auth_fifo[auth_fifo_pos].delflag = 0;
					auth_fifo[auth_fifo_pos].network = net;
					auth_fifo_pos++;
	    			}
				else {
					memset(WFIFOP(fd, 0), 0, 23);
					WFIFOW(fd, 0) = 0x6a;
					WFIFOB(fd, 2) = result;
					if (result == 6) {
						for (i = 0; i < auth_num; i++) {
							if (strncmp(auth_dat[i].userid, account.userid, 24) == 0) {
								memcpy(WFIFOP(fd, 3), auth_dat[i].msgerr, 24);
								break;
							}
						}
					}
					WFIFOSET(fd, PacketLenTable[0x6a]);
				}
				RFIFOSKIP(fd, PacketLenTable[0x64]);
				break;

	 		case 0x2710:
			      if (RFIFOREST(fd) < 90)
					return 0;

	 			account.userid = RFIFOP(fd, 2);
	 			account.passwd = RFIFOP(fd, 26);
				account.ver_1 = RFIFOW(fd, 86);
				account.ver_2 = RFIFOB(fd, 88);
	 			result = mmo_auth(fd, &account);
	 			if ((result == 100) && (account.sex == 2)
				&& (account.account_id < MAX_SERVERS)
				&& (server_fd[account.account_id] < 0)) {
					memcpy(server[account.account_id].name, RFIFOP(fd, 60), 16);
					server[account.account_id].ip = RFIFOL(fd, 54);
					server[account.account_id].port = RFIFOW(fd, 58);
					server[account.account_id].xip = RFIFOL(fd, 76);
					server[account.account_id].users = 0;
					server[account.account_id].maintenance = RFIFOW(fd, 82);
					server[account.account_id].new = RFIFOW(fd, 84);
					server_fd[account.account_id] = fd;
					WFIFOW(fd, 0) = 0x2711;
					WFIFOB(fd, 2) = 0;
					WFIFOSET(fd, 3);
					session[fd]->func_parse = parse_fromchar;
					realloc_fifo(fd, FIFO_SIZE, FIFO_SIZE);
	     			}
				else {
					WFIFOW(fd, 0) = 0x2711;
					WFIFOB(fd, 2) = 3;
					WFIFOSET(fd, 3);
			      }
				RFIFOSKIP(fd, 90);
				return 0;

			default:
				close(fd);
				session[fd]->eof = 1;
				return 0;
		}
	}
	RFIFOFLUSH(fd);
	return 0;
}

/*======================================
 *	ACCOUNT: Set Timers
 *--------------------------------------
 */

int mmo_auth_delay(int tid, unsigned int tick, int id, int data)
{
	int i;

	tid = 0;
	tick = 0;
	data = 0;
	for (i = 0; i < auth_num; i++) {
		if (auth_dat[i].account_id == id)
			break;
	}
	if (i != auth_num) {
		if (auth_dat[i].auth_timer > 0)
			auth_dat[i].auth_timer = 0;

		auth_dat[i].state = 0;
	}
	mmo_auth_sync();
	return 0;
}

int make_backup_timer(int tid, unsigned int tick, int id, int data)
{
	tid = 0;
	tick = 0;
	id = 0;
	data = 0;
	make_backup();
	return 0;
}

/*======================================
 *	ACCOUNT: Program Initialization
 *--------------------------------------
 */

int do_init(void)
{
	int i;
	char line[MAX_LINE], option1[MAX_LINE], option2[MAX_LINE];
	struct timeval tv;
	char tmpstr[256];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	FILE *fp = NULL;

	printf("\033[1;41m.----------------------------------------.\n");
	printf("\033[1;41m|             Odin Project(c)            |\n");
	printf("\033[1;41m|                                        |\n");
	printf("\033[1;41m|        ___    __     ___   __  _       |\n");
	printf("\033[1;41m|       / _ \\  (   \\  (   ) (  )( )      |\n");
	printf("\033[1;41m|      | ( ) | ( |) |  | |   | \\| |      |\n");
	printf("\033[1;41m|       \\___/  (__ /  (___)  |_|\\_|      |\n");
	printf("\033[1;41m|                                        |\n");
	printf("\033[1;41m|                                        |\n");
	printf("\033[1;41m|      %s      |\n", logN);
	printf("\033[1;41m`========================================'\033[0m\n");
	printf("\n**Server Started**\n");

	for (i = 0; i < AUTH_FIFO_SIZE; i++) {
		auth_fifo[i].delflag = 1;
	}
	for (i = 0; i < MAX_SERVERS; i++) {
		server_fd[i] = -1;
	}
	mmo_auth_init();
	for (i = 0; i < auth_num; i++) {
		auth_dat[i].state = 0;
	}
	mmo_auth_sync();
	reload_config();

	if (!(fp = fopen("config.ini", "r"))) {
		fprintf(stderr,"ERROR: %s : Access Violation\n", tmpstr);
		sleep(2);
		abort();
	}
	else {
		while (fgets(line, 1023, fp)) {
			i = sscanf(line,"%[^=]=%s", option1, option2);
			if (i != 2) {
				continue;
			}
			if (strcmp(option1, "AccountPort") == 0) {
				accountport = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "Backup") == 0) {
				if ((int)strtol(option2, (char**)NULL, 10) == 1) {
					do_backup = 1;
				}
				else {
					do_backup = 0;
				}
			}
			else if (strcmp(option1, "BackupTime") == 0) {
				do_timer_bak = (int)strtol(option2, (char**)NULL, 10);
				if ((do_backup == 1) && (do_timer_bak > 120 || do_timer_bak <= 0)) {
					do_timer_bak = 30;
				}
			}
			else if (strcmp(option1, "MaxUsers") == 0) {
				max_users = (int)strtol(option2, (char**)NULL, 10);
				if (max_users > FD_SETSIZE - 5) {
					max_users = FD_SETSIZE - 5;
				}
			}
		}
		fclose(fp);
	}
	set_termfunc(mmo_auth_sync);
	set_defaultparse(parse_login);
	listen_port_fd = make_listen_port(accountport);

	if (do_backup) {
		check_backup_timer = add_timer(gettick() + do_timer_bak * 60000, make_backup_timer, 0, 0);
		timer_data[check_backup_timer]->type = TIMER_INTERVAL;
		timer_data[check_backup_timer]->interval = do_timer_bak * 60000;
	}
	printf("\033[1;41m\nReady to accept new connections.\033[0m\n");
	return 0;
}

int do_exit(void)
{
	if (check_backup_timer > 0)
		delete_timer(check_backup_timer, make_backup_timer);

	if (auth_dat)
		free(auth_dat);

	delete_session(listen_port_fd);
	return 0;
}
