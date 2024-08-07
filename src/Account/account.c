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
#include "timer.h"
#include "account.h"

unsigned int server_fd[MAX_SERVERS];
int server_num = 0;
short accountport = 6900;
unsigned int listen_port_fd;

int secure_mode = 0;
int closesrv = 1;
int do_backup = 0;
int do_timer_bak = 0;
long max_users = 0;
int auth_num = 0;
int auth_max = 0;
int auth_fifo_pos = 0;
int check_backup_timer;

long account_id_count = ACCOUNT_ID_INIT;
long gm_account_id_count = GM_ACCOUNT_ID_INIT;
long sub_gm_account_id_count = SUB_GM_ACCOUNT_ID_INIT;
void mmo_auth_delay(int, unsigned int, int, int);

static const int PacketLenTable[0x100] = {
	1, 0, 2, 0, 23, 0, 0, 0, 0, 0, 0, 0, 58, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 75, 0, 0, 0, 0, 55, 0, 1, 0, 1, 0, 1, 0, 79, 0, 0, 0,
	0, 0, 31, 0, 0, 0, 79, 0, 70, 0, 0, 0, 80, 0, 0, 0, 0, 56, 90, 0,
	10, 0, 0, 70, 0, 55, 0, 0, 0, 0, 67, 0, 79, 0, 0, 0, 0, 0, 10, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 0, 23, 0, 0, 0, 0,
	55, 0, 0, 0, 0, 79, 23, 0, 0, 0, 0, 0, 0, -16, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 13, 0, 0, 0, 0, 0, 23, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static void mmo_auth_init(void)
{
	int i;
	long account_id;
	int logincount, ban, expired_id, server_dis, msg, state;
	char line[1024], userid[24], pass[24], email[40], msgerr[24], lastlogin[24], sex;

	auth_dat = calloc(sizeof(*auth_dat) * 256, 1);
	if (!auth_dat) {
		debug_output("mmo_auth_init: Cannot allocate memory on 'auth_dat'.\n");
		abort();
	}
	auth_max = 256;
	FILE *fp = fopen("save/accounts/accounts.txt", "rt");
	if (fp) {
		while (fgets(line, 1023, fp)) {
			i = sscanf(line,"%ld,%[^,],%[^,],%c,%[^,],%[^,],%d,%d,%d,%d,%[^,],%d,%d",
				   &account_id, userid, pass, &sex, email, lastlogin, &ban, &expired_id,
				   &server_dis, &msg, msgerr, &state, &logincount);

			if (account_id >= END_ACCOUNT_NUM) {
				debug_output("mmo_auth_init: Account Server reach database limit.\n");
				continue;
			}
			if (i >= 12) {
				if (auth_num >= auth_max) {
					auth_max += 256;
					auth_dat = realloc(auth_dat, sizeof(*auth_dat) * auth_max);
				}
				memset(&auth_dat[auth_num], 0, sizeof(*auth_dat));
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

				auth_dat[auth_num].timer = 0;
				auth_num++;
			}
			if (account_id >= account_id_count && account_id < END_ACCOUNT_NUM)
				account_id_count = account_id + 1;

			if (account_id >= gm_account_id_count && account_id < ACCOUNT_ID_INIT)
				gm_account_id_count = account_id + 1;

			if (account_id >= sub_gm_account_id_count && account_id < GM_ACCOUNT_ID_INIT)
				sub_gm_account_id_count = account_id + 1;

		}
		fclose(fp);
	}
	else {
		debug_output("mmo_auth_init: Fail to open 'save/accounts/accounts.txt'.\n");
		abort();
	}
}

void mmo_auth_sync(void)
{
	int i;
	FILE *fp = fopen("save/accounts/accounts.txt", "wt");
	if (fp)
		for (i = 0; i < auth_num; i++)
			fprintf (fp, "%ld,%s,%s,%c,%s,%s,%d,%d,%d,%d,%s,%d,%d\t%s\n",
				 auth_dat[i].account_id, auth_dat[i].userid, auth_dat[i].pass,
				 auth_dat[i].sex == 2 ? 'S' : (auth_dat[i].sex ? '1' : '0'),
				 auth_dat[i].email, auth_dat[i].lastlogin, auth_dat[i].ban,
				 auth_dat[i].expired_id, auth_dat[i].server_dis, auth_dat[i].msg,
				 auth_dat[i].msgerr, auth_dat[i].state, auth_dat[i].logincount, auth_dat[i].ip);


	fclose(fp);
}

static int mmo_auth_admn(struct mmo_account *account)
{
	int i;
	struct timeval tv;
	char tmpstr[24];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));

	strtolower(account->userid);
	for (i = 0; i < auth_num; i++)
		if (strncmp(auth_dat[i].userid, account->userid, 24) == 0)
			break;



	if (i != auth_num) {
		if (strncmp(account->passwd, auth_dat[i].pass, 24))
			return 1;

	}
	else
		return 0;

	if (account->ver_1 < 18)
		return 2;

	if (account->ver_2 != 0 && account->ver_2 != 1)
		return 3;

	if (auth_dat[i].sex != 2)
		return 4;

	account->account_id = auth_dat[i].account_id;
	account->login_id1 = rand();
	account->login_id2 = rand();
	account->sex = auth_dat[i].sex;
	memcpy(auth_dat[i].lastlogin, tmpstr, 24);
	memcpy(account->lastlogin, auth_dat[i].lastlogin, 24);
	memcpy(auth_dat[i].ip, account->ip, 16);
	auth_dat[i].logincount++;
	mmo_auth_sync();
	return 100;
}

static int mmo_auth(int fd, struct mmo_account *account)
{
	int i, j, p, len;
	int flag = 0;
	int user_count = 0, gmode = 0, sex = 0;
	int newaccount = 0, isAccount = 0;
	char line[1024], userid[24], passwd[24], email[40], temp[70];
	struct timeval tv;
	char tmpstr[24];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	FILE *fp = NULL;

	if ((fp = fopen("save/accounts/addplayer.txt", "rt"))) {
		while (fgets(line, 1023, fp)) {
			p = sscanf(line, "%[^,],%[^,],%d,%[^,],%d", userid, passwd, &sex, email, &gmode);
			if (p != 5) {
				debug_output("mmo_auth: File 'save/accounts/addplayer.txt' has an invalid format.\n");
				debug_output("mmo_auth: Fail to add querry from 'save/accounts/addplayer.txt'.\n");
				fclose(fp);
				isAccount++;
			}
			if (sex > 1 || sex < 0)
				isAccount++;

			if (gmode > 2 || gmode < 0)
				isAccount++;

			if (isAccount)
				debug_output("mmo_auth: Fail to add accounts using add_player.txt because it have wrong values.\n");

			strtolower(userid);
			for (i = 0; i < auth_num; i++) {
				strncpy(temp, auth_dat[i].userid, 24);
				if (strstr(strtolower(temp), userid) != NULL) {
					debug_output("mmo_auth: Fail to add accounts using add_player.txt because account already exist.\n");
					isAccount++;
				}
				if (strlen(userid) < 4) {
					debug_output("mmo_auth: Fail to add accounts using add_player.txt because account name is less than 4 characters.\n");
					isAccount++;
				}
			}
			if (isAccount == 0) {
				debug_output("mmo_auth: Auth new %s %s.\n",account->userid, account->passwd);
				if (auth_num >= auth_max) {
					auth_max += 256;
					auth_dat = realloc(auth_dat, sizeof(*auth_dat) * auth_max);
				}
				if (gmode == 0)
					auth_dat[i].account_id = account_id_count++;

				else if (gmode == 1)
					auth_dat[i].account_id = sub_gm_account_id_count++;

				else if (gmode == 2)
					auth_dat[i].account_id = gm_account_id_count++;

				strncpy(auth_dat[i].userid, userid, 24);
				strncpy(auth_dat[i].pass, passwd, 24);
				if (sex == 1)
					auth_dat[i].sex = 1;

				else
					auth_dat[i].sex = 0;

				strncpy(auth_dat[i].email, email, 60);
				strcpy(auth_dat[i].msgerr, "NULL");
				auth_dat[i].ban = 0;
				auth_dat[i].expired_id = 0;
				auth_dat[i].server_dis = 0;
				auth_dat[i].msg = 0;
				auth_dat[i].state = 0;
				auth_dat[i].logincount = 0;
				auth_dat[i].timer = 0;
				auth_num++;
			}
		}
		mmo_auth_sync();
	}
	fclose(fp);
	fp = fopen("save/accounts/addplayer.txt", "wt");
	fclose(fp);

	len = strlen(account->userid) - 2;
	if (account->userid[len] == '_' && strcmp(account->userid, "\n") != 0 &&
	(account->userid[len+1] == 'M' || account->userid[len+1] == 'F')) {
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
	strtolower(account->userid);
	for (i = 0; i < auth_num; i++)
		if (strncmp(auth_dat[i].userid, account->userid, 24) == 0)
			break;



	if (i != auth_num) {
		if (strncmp(auth_dat[i].pass, account->passwd, 24)) {
			debug_output("mmo_auth: Auth failed pass error %s %s %s %d.\n", tmpstr, account->userid, account->passwd, newaccount);
			return 1;
		}
		else if (strncmp(auth_dat[i].pass, account->passwd, 24) && newaccount) {
			debug_output("mmo_auth: Auth failed pass error %s %s %d.\n", account->userid, account->passwd, newaccount);
			return 0;
		}
		debug_output("mmo_auth: Auth ok %s %s %d.\n", account->userid, account->passwd, newaccount);
		debug_output("mmo_auth: %s logged in.\n", account->userid);
	}
	else {
		if (newaccount == 0) {
			debug_output("mmo_auth: Auth failed no account %s %s %d.\n", account->userid, account->passwd, newaccount);
			return 0;
		}
		else {
			debug_output("mmo_auth: Auth new %s %s %d.\n",account->userid, account->passwd, newaccount);
			if (auth_num >= auth_max) {
				auth_max += 256;
				auth_dat = realloc(auth_dat, sizeof(*auth_dat) * auth_max);
			}
			auth_dat[i].account_id = account_id_count++;
			strncpy(auth_dat[i].userid, account->userid, 24);
			strncpy(auth_dat[i].pass, account->passwd, 24);
			auth_dat[i].sex = account->userid[len + 1] == '1';
			strcpy(auth_dat[i].email, "NULL");
			strcpy(auth_dat[i].msgerr, "NULL");
			auth_dat[i].ban = 0;
			auth_dat[i].expired_id = 0;
			auth_dat[i].server_dis = 0;
			auth_dat[i].msg = 0;
			auth_dat[i].state = 0;
			auth_dat[i].timer = 0;
			auth_dat[i].logincount = 0;
			auth_num++;
		}
	}
	if (closesrv == 1)
		flag = 1;

	if (auth_dat[i].state) {
		if (auth_dat[auth_num].timer == 0)
			auth_dat[i].timer = add_timer(gettick() + 21000, mmo_auth_delay, auth_dat[i].account_id, 0);

		flag = 8;
	}
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
		for (j = 0; j < MAX_SERVERS; j++)
			if (server[j].users > user_count)
				user_count = server[j].users;

		if (user_count >= max_users)
			return 7;

	}
	if (flag) {
		WFIFOW(fd, 0) = 0x81;
		WFIFOB(fd, 2) = flag;
		WFIFOSET(fd, PacketLenTable[0x81]);
		return -1;
	}
	account->account_id = auth_dat[i].account_id;
	account->login_id1 = rand();
	account->login_id2 = rand();
	account->sex = auth_dat[i].sex;
	memcpy(auth_dat[i].lastlogin, tmpstr, 24);
	memcpy(account->lastlogin, auth_dat[i].lastlogin, 24);
	memcpy(auth_dat[i].ip, account->ip, 16);
	auth_dat[i].state = 1;
	auth_dat[i].timer = add_timer(gettick() + 15000, mmo_auth_delay, auth_dat[i].account_id, 0);
	auth_dat[i].logincount++;
	mmo_auth_sync();
	return 100;
}

int parse_fromchar(unsigned int fd)
{
	int i, j, id;
	long account_id, login_id;

	for (id = 0; id < MAX_SERVERS; id++)
		if (server_fd[id] == fd)
     			break;


	if (id == MAX_SERVERS)
   		session[fd]->eof = 1;

 	if (session[fd]->eof) {
   		for (i = 0; i < MAX_SERVERS; i++)
   			if (server_fd[i] == fd)
				server_fd[i] = 0;


		close(fd);
		delete_session(fd);
		closesrv = 1;
		return 0;
	}
	while (RFIFOREST(fd) >= 2) {
		switch (RFIFOW(fd, 0)) {
		case 0x2712:
			if (RFIFOREST(fd) < 15)
				return 0;

			debug_output("parse_fromchar: 0x2712 %l %l.\n", RFIFOL(fd, 2), RFIFOB(fd, 6));
			account_id = RFIFOL(fd, 2);
			login_id = RFIFOL(fd, 6);
			for (i = 0; i < AUTH_FIFO_SIZE; i++) {
				if (auth_fifo[i].account_id == account_id
				&&  auth_fifo[i].login_id1 == login_id
				&& !auth_fifo[i].delflag) {
					auth_fifo[i].delflag = 1;
					break;
				}
			}
			WFIFOW(fd, 0) = 0x2713;
			WFIFOL(fd, 2) = account_id;

			if (i != AUTH_FIFO_SIZE)
				WFIFOB(fd, 6) = 0;

			else
				WFIFOB(fd, 6) = 1;

			WFIFOB(fd, 7) = auth_fifo[i].network;
			if (i != AUTH_FIFO_SIZE) {
				for (j = 0; j < auth_num; j++) {
					if (auth_fifo[i].account_id == auth_dat[j].account_id) {
						memcpy(WFIFOP(fd, 8), auth_dat[j].email, 40);
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

			debug_output("parse_fromchar: 0x30a1 %l %d.\n", RFIFOL(fd, 2), RFIFOB(fd, 6));
			account_id = RFIFOL(fd, 2);
			j = RFIFOB(fd, 6);
			for (i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == account_id) {
					auth_dat[i].state = j;
					mmo_auth_sync();
					break;
				}
			}
			if (i == auth_num)
				debug_output("parse_fromchar: 0x30a1 cannot update state of %l.\n", RFIFOL(fd, 2));

			RFIFOSKIP(fd, 7);
			break;

		case 0x30fa:
			if (RFIFOREST(fd) < 6)
				return 0;

			debug_output("parse_fromchar: 0x30fa %l.\n", RFIFOL(fd, 2));
			account_id = RFIFOL(fd, 2);
			for (i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == account_id) {
					auth_dat[i].ban = 1;
					mmo_auth_sync();
					break;
				}
			}
			if (i == auth_num)
				debug_output("parse_fromchar: 0x30fa cannot ban %l.\n", RFIFOL(fd, 2));

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
	int i = 0;
	int p0, p1, p2, p3;
	char line[64];

	FILE *fp = fopen("banned.ini", "rt");
	if (fp) {
		while (fgets(line, 63, fp)) {
			sscanf(line, "%d.%d.%d.%d", &p0, &p1, &p2, &p3);
			if (p0 == ip0 && p1 == ip1 && p2 == ip2 && p3 == ip3) {
				debug_output("check_ip: IP %d.%d.%d.%d is banned.\n", ip0, ip1, ip2, ip3);
				i = 1;
				break;
			}
		}
	}
	fclose(fp);
	return i;
}

int parse_login(unsigned int fd)
{
	int i, result = 0, net = 0, len;
	long servernet, clientnet, serverip;
	unsigned char *ip = (unsigned char *)&session[fd]->client_addr.sin_addr;
	char buf[16];
	struct mmo_account account;

	if (session[fd]->eof) {
		for (i = 0; i < MAX_SERVERS; i++)
			if (server_fd[i] == fd)
				server_fd[i] = 0;


		close(fd);
		delete_session(fd);
		return 0;
	}
	while (RFIFOREST(fd) >= 2) {
		switch (RFIFOW(fd, 0)) {
		case 0x200:
			if (RFIFOREST(fd) < 26)
				return 0;

			RFIFOSKIP(fd, 26);
			break;

		case 0x204:
			if (RFIFOREST(fd) < 18)
				return 0;

			RFIFOSKIP(fd, 18);
			break;

		case 0x64:
			if (RFIFOREST(fd) < PacketLenTable[0x64])
				return 0;

			reload_config();
			if ((len = check_ip(ip[0], ip[1], ip[2], ip[3])) > 0) {
				WFIFOW(fd, 0) = 0x81;
				WFIFOB(fd, 2) = 5;
				WFIFOSET(fd, PacketLenTable[0x81]);
				RFIFOSKIP(fd, PacketLenTable[0x64]);
				break;
			}
			account.ver_1 = RFIFOL(fd, 2);
			account.userid = RFIFOP(fd, 6);
			account.passwd = RFIFOP(fd, 30);
			account.ver_2 = RFIFOB(fd, 54);
			sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
			memcpy(account.ip, buf, 16);
			debug_output("parse_login: Client connection request %s from %s.\n", RFIFOP(fd, 6), buf);
			result = mmo_auth(fd, &account);
			net = NO_INFO;
			if (result == 100) {
				server_num = 0;
				for (i = 0; i < MAX_SERVERS; i++) {
					if (server_fd[i] > 0) {
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

			if ((len = check_ip(ip[0], ip[1], ip[2], ip[3])) > 0) {
				RFIFOSKIP(fd, 90);
				return 0;
			}
			account.userid = RFIFOP(fd, 2);
			account.passwd = RFIFOP(fd, 26);
			account.ver_1 = RFIFOW(fd, 86);
			account.ver_2 = RFIFOB(fd, 88);
			sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
			memcpy(account.ip, buf, 16);
			debug_output("parse_login: Server connection request %s @ %d.%d.%d.%d:%d (%s).\n",
			RFIFOP(fd, 60), RFIFOB(fd, 54), RFIFOB(fd, 55), RFIFOB(fd, 56), RFIFOB(fd, 57), RFIFOW(fd, 58), buf);
			result = mmo_auth_admn(&account);
			if (result == 0)
				debug_output("parse_login: Admn account doesn't exist.\n");

			else if (result == 1)
				debug_output("parse_login: Wrong password for admn account.\n");

			else if (result == 2 || result == 3)
				debug_output("parse_login: Character Server is too old.\n");

			else if (result == 4)
				debug_output("parse_login: Conflicts with admn account.\n");

			else if (result == 100 && account.account_id < MAX_SERVERS && server_fd[account.account_id] < 1) {
				memcpy(server[account.account_id].name, RFIFOP(fd, 60), 16);
				server[account.account_id].ip = RFIFOL(fd, 54);
				server[account.account_id].port = RFIFOW(fd, 58);
				server[account.account_id].xip = RFIFOL(fd, 76);
				server[account.account_id].users = 0;
				server[account.account_id].maintenance = RFIFOW(fd, 82);
				server[account.account_id].new = RFIFOW(fd, 84);
				server_fd[account.account_id] = fd;
				session[fd]->func_parse = parse_fromchar;
				realloc_fifo(fd, FIFO_SIZE, FIFO_SIZE);
				WFIFOW(fd, 0) = 0x2711;
				WFIFOB(fd, 2) = 0;
				WFIFOSET(fd, 3);
				closesrv = 0;
			}
			else {
				WFIFOW(fd, 0) = 0x2711;
				WFIFOB(fd, 2) = result;
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
	return 0;
}

void mmo_auth_delay(int tid, unsigned int tick, int id, int data)
{
	int i;

	for (i = 0; i < auth_num; i++) {
		if (auth_dat[i].account_id == id) {
			auth_dat[i].timer = 0;
			auth_dat[i].state = 0;
			break;
		}
	}
	mmo_auth_sync();
}

void make_backup_timer(int tid, unsigned int tick, int id, int data)
{
	make_backup();
}

int do_init(void)
{
	int i;
	char line[1024], option1[1024], option2[1024];
	FILE *fp = NULL;

	fprintf(stdout, "\033[1;44m.----------------------------------------.\n");
	fprintf(stdout, "\033[1;44m|             Odin Project(c)            |\n");
	fprintf(stdout, "\033[1;44m|                                        |\n");
	fprintf(stdout, "\033[1;44m|        ___    __     ___   __  _       |\n");
	fprintf(stdout, "\033[1;44m|       / _ \\  (   \\  (   ) (  )( )      |\n");
	fprintf(stdout, "\033[1;44m|      | ( ) | ( |) |  | |   | \\| |      |\n");
	fprintf(stdout, "\033[1;44m|       \\___/  (__ /  (___)  |_|\\_|      |\n");
	fprintf(stdout, "\033[1;44m|                                        |\n");
	fprintf(stdout, "\033[1;44m|                                        |\n");
	fprintf(stdout, "\033[1;44m|      %s      |\n", logN);
	fprintf(stdout, "\033[1;44m`========================================'\033[0m\n");
	fprintf(stdout, "\n**Server Started**\n");

	for (i = 0; i < AUTH_FIFO_SIZE; i++)
		auth_fifo[i].delflag = 1;

	for (i = 0; i < MAX_SERVERS; i++)
		server_fd[i] = 0;

	mmo_auth_init();
	reload_config();

	if (!(fp = fopen("cfg/account_cfg.ini", "rt"))) {
		debug_output("do_init: Cannot open file 'cfg/account_cfg.ini'.\n");
		return 1;
	}
	else {
		while (fgets(line, 1023, fp)) {
			i = sscanf(line,"%[^=]=%s", option1, option2);
			if (i != 2)
				continue;

			if (strcmp(option1, "AccountPort") == 0)
				accountport = (int)strtol(option2, (char**)NULL, 10);

			else if (strcmp(option1, "Backup") == 0) {
				if ((int)strtol(option2, (char**)NULL, 10) == 1)
					do_backup = 1;

				else
					do_backup = 0;

			}
			else if (strcmp(option1, "BackupTime") == 0) {
				do_timer_bak = (int)strtol(option2, (char**)NULL, 10);
				if ((do_backup == 1) && (do_timer_bak > 120 || do_timer_bak <= 0))
					do_timer_bak = 30;

			}
			else if (strcmp(option1, "MaxUsers") == 0) {
				max_users = (long)strtol(option2, (char**)NULL, 10);
				if (max_users > FD_SETSIZE - 5)
					max_users = FD_SETSIZE - 5;

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
	fprintf(stdout, "\033[1;44m\nReady to accept new connections.\033[0m\n");
	return 0;
}

void do_exit(void)
{
	if (check_backup_timer > 0)
		delete_timer(check_backup_timer, make_backup_timer);

	if (auth_dat) {
		free(auth_dat);
		auth_dat = NULL;
	}
	delete_session(listen_port_fd);
	debug_output("do_exit: Server Shutdown.\n");
}
