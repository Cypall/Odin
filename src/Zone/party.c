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
  Project:       Project Odin Zone Server
  Creation Date: Dicember 6, 2003
  Modified Date: Semtember 3, 2004
  Description:   Ragnarok Online Server Emulator
  ------------------------------------------------------------------------*/

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "party.h"
#include "save.h"

int exp_party_share_level = 10;
int exp_party_share_leader_level = 10;

void mmo_party_do_init(void)
{
	int index;

	printf("Loading Parties Data...       ");
	for (index = 0; index < MAX_PARTYS; index++)
		init_party_data(index);

	printf("Done\n");
}

void init_party_data(int index)
{
	int mbr;

	if (index >= 0 && index < MAX_PARTYS) {
		party_dat[index].party_id = -1;
		party_dat[index].exp = 0;
		party_dat[index].item = 0;
		party_dat[index].member_count = 0;
		party_dat[index].leader_level = 0;
		memset(party_dat[index].party_name, 0, 24);

		for (mbr = 0; mbr < MAX_PARTY_MEMBERS; mbr++)
			reset_member_data(index, mbr);
	}
}

void reset_member_data(int index, short mbr)
{
	if (index >= 0 && index < MAX_PARTYS) {
		if (mbr >= 0 && mbr < MAX_PARTY_MEMBERS) {
			memset(party_dat[index].member[mbr].nick, 0, 24);
			memset(party_dat[index].member[mbr].map_name, 0, 16);
			party_dat[index].member[mbr].account_id = 0;
			party_dat[index].member[mbr].fd = 0;
			party_dat[index].member[mbr].x = 0;
			party_dat[index].member[mbr].y = 0;
			party_dat[index].member[mbr].hp = 0;
			party_dat[index].member[mbr].max_hp = 0;
			party_dat[index].member[mbr].offline = 1;
			party_dat[index].member[mbr].leader = 1;
			party_dat[index].member[mbr].char_id = -1;
			party_dat[index].member[mbr].mapno = -1;
		}
	}
}

int find_party_slot(void)
{
	int i;

	for (i = 0; i < MAX_PARTYS; i++) {
		if (party_dat[i].party_id == -1)
			break;
	}
	if (i == MAX_PARTYS)
		i = -1;

	return i;
}

void create_party(int fd, char partyname[24])
{
	int i;
	int index;
	unsigned char buf[256];
	struct map_session_data *sd;

	if (session[fd] && (sd = session[fd]->session_data)) {
		if (sd->status.party_status > 0) {
			WFIFOW(fd, 0) = 0xfa;
			WFIFOB(fd, 2) = 2;
			WFIFOSET(fd, packet_len_table[0xfa]);
		}
		else {
			for (i = 0; i < MAX_PARTYS; i++) {
				if (strcmp(party_dat[i].party_name, partyname) == 0)
					break;
			}
			if (i != MAX_PARTYS) {
				WFIFOW(fd, 0) = 0xfa;
				WFIFOB(fd, 2) = 1;
				WFIFOSET(fd, packet_len_table[0xfa]);
			}
			else {
				index = find_party_slot();
				if (index == -1) {
					WFIFOW(fd, 0) = 0xfa;
					WFIFOB(fd, 2) = 1;
					WFIFOSET(fd, packet_len_table[0xfa]);
				}
				else {
					init_party_data(index);
					party_dat[index].party_id = index;
					party_dat[index].member_count = 1;
					memcpy(party_dat[index].party_name, partyname, 24);
					party_dat[index].exp = 0;
					party_dat[index].item = 0;

					party_make_member(fd, 0, index);
					party_dat[index].member[0].leader = 0;
					party_dat[index].leader_level = sd->status.base_level;
					sd->status.party_status = 2;

					WFIFOW(fd, 0) = 0xfa;
					WFIFOB(fd, 2) = 0;
					WFIFOSET(fd, packet_len_table[0xfa]);

					memset(buf, 0, packet_len_table[0x104]);
					WBUFW(buf, 0) = 0x104;
					WBUFL(buf, 2) = sd->account_id;
					WBUFL(buf, 6) = sd->status.party_id;
					WBUFW(buf, 10) = sd->x;
					WBUFW(buf, 12) = sd->y;
					WBUFB(buf, 14) = 0;
					memcpy(WBUFP(buf, 15), sd->status.party_name, 24);
					memcpy(WBUFP(buf, 39), sd->status.name, 24);
					memcpy(WBUFP(buf, 63), sd->mapname, 16);
					mmo_map_sendarea(fd, buf, packet_len_table[0x104], 0);

					WFIFOW(fd, 0) = 0xfb;
					WFIFOW(fd, 2) = 74;
					strncpy(WFIFOP(fd, 4), sd->status.party_name, 24);
					WFIFOL(fd, 28) = sd->account_id;
					strncpy(WFIFOP(fd, 32), sd->status.name, 24);
					strncpy(WFIFOP(fd, 56), sd->mapname, 16);
					WFIFOB(fd, 72) = 0;
					WFIFOB(fd, 73) = 0;
					WFIFOSET(fd, WFIFOW(fd, 2));

					send_party_setup(fd);
					party_update_member_location(index, sd->account_id, sd->char_id, sd->x, sd->y, sd->mapno);
					mmo_party_save(index);
				}
			}
		}
	}
}

void party_make_member(int fd, int mbr, int index)
{
	struct map_session_data *sd;

	if (index >= 0 && index < MAX_PARTYS) {
		if (mbr >= 0 && mbr < MAX_PARTY_MEMBERS) {
			if (session[fd] && (sd = session[fd]->session_data)) {
				strcpy(party_dat[index].member[mbr].nick, sd->status.name);
				strcpy(party_dat[index].member[mbr].map_name, sd->mapname);
				party_dat[index].member[mbr].account_id = sd->account_id;
				party_dat[index].member[mbr].fd = fd;
				party_dat[index].member[mbr].x = sd->x;
				party_dat[index].member[mbr].y = sd->y;
				party_dat[index].member[mbr].hp = sd->status.hp;
				party_dat[index].member[mbr].max_hp = sd->status.max_hp;
				party_dat[index].member[mbr].offline = 0;
				party_dat[index].member[mbr].leader = 1;
				party_dat[index].member[mbr].char_id = sd->status.char_id;
				party_dat[index].member[mbr].mapno = sd->mapno;
				sd->status.party_status = 1;
				sd->status.party_id = index;
				strcpy(sd->status.party_name, party_dat[index].party_name);
			}
		}
	}
}

int party_exists(int party_id)
{
	int i;

	for (i = 0; i < MAX_PARTYS; i++) {
		if (party_dat[i].party_id == party_id)
			return i;
	}
	return -1;
}

void update_party(int party_num)
{
	int i = 0, k;
	int fd;
	char buf[28 + MAX_PARTY_MEMBERS * 46];

	if (party_num >= 0 && party_num < MAX_PARTYS) {
		WBUFW(buf, 0) = 0xfb;
		strcpy(WBUFP(buf, 4), party_dat[party_num].party_name);
		for (k = 0; k < MAX_PARTY_MEMBERS; k++) {
			fd = party_dat[party_num].member[k].fd;
			if (session[fd] && session[fd]->session_data) {
				WBUFL(buf, 28 + i * 46) = party_dat[party_num].member[k].account_id;
				strcpy(WBUFP(buf, 28 + i * 46 + 4), party_dat[party_num].member[k].nick);
				strcpy(WBUFP(buf, 28 + i * 46 + 28), party_dat[party_num].member[k].map_name);
				WBUFB(buf, 28 + i * 46 + 44) = party_dat[party_num].member[k].leader;
				WBUFB(buf, 28 + i * 46 + 45) = party_dat[party_num].member[k].offline;
				i++;
			}
			else {
				WBUFL(buf, 28 + i * 46) = party_dat[party_num].member[k].account_id;
				strcpy(WBUFP(buf, 28 + i * 46 + 4), party_dat[party_num].member[k].nick);
				strcpy(WBUFP(buf, 28 + i * 46 + 28), party_dat[party_num].member[k].map_name);
				WBUFB(buf, 28 + i * 46 + 44) = party_dat[party_num].member[k].leader;
				WBUFB(buf, 28 + i * 46 + 45) = party_dat[party_num].member[k].offline;
				i++;
			}
		}
		WBUFW(buf, 2) = 28 + i * 46;
		if (i > 0) {
			for (k = 0; k < MAX_PARTY_MEMBERS; k++) {
				fd = party_dat[party_num].member[k].fd;
				if (session[fd] && session[fd]->session_data) {
					memcpy(WFIFOP(fd, 0), buf, 28 + i * 46);
					WFIFOSET(fd, 28 + i * 46);
				}
			}
		}
	}
}

void party_remove_member(int fd, int party_status, int index)
{
	int j;
	int cfd;
	struct map_session_data *sd;
	struct map_session_data *target_sd = NULL;

	if (index >= 0 && index < MAX_PARTYS) {
		if (session[fd] && (sd = session[fd]->session_data)) {
			for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
				if (party_dat[index].member[j].char_id == sd->status.char_id) {
					reset_member_data(index, j);
					break;
				}
			}
			sd->status.party_status = 0;
			sd->status.party_id = -1;
			strcpy(sd->status.party_name, "");
			party_dat[index].member_count--;
			if (party_dat[index].member_count < 1)
				mmo_map_delete_party(index);

			else {
				if (party_status == 2) {
					party_dat[index].leader_level = 0;
					for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
						if (party_dat[index].member[j].char_id > 0) {
							cfd = party_dat[index].member[j].fd;
							if (cfd != 0) {
								if (session[cfd] && (target_sd = session[cfd]->session_data)) {
									if (target_sd->status.skill[0].lv >= 7) {
										target_sd->status.party_status = 2;
										party_dat[index].member[j].leader = 0;
										party_dat[index].leader_level = target_sd->status.base_level;
										mmo_char_save(target_sd);
										break;
									}
								}
							}
						}
					}
				}
				update_party(index);
				mmo_party_save(index);
			}
		}
	}
}

int leaveparty(int fd)
{
	int j;
	int index;
	struct map_session_data *sd = session[fd]->session_data;

	if (sd->status.hp <= 0 || sd->sitting == 1)
		return -1;

	index = party_exists(sd->status.party_id);
	if (index == -1) {
		sd->status.party_status = 0;
		sd->status.party_id = -1;
		strcpy(sd->status.party_name, "");
		return -1;
	}
	else {
		if (sd->status.party_status > 0) {
			for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
				if (party_dat[index].member[j].fd != 0) {
					WFIFOW(party_dat[index].member[j].fd, 0) = 0x105;
					WFIFOL(party_dat[index].member[j].fd, 2) = sd->account_id;
					strcpy(WFIFOP(party_dat[index].member[j].fd, 6), sd->status.name);
					WFIFOB(party_dat[index].member[j].fd, 30) = 0;
					WFIFOSET(party_dat[index].member[j].fd, packet_len_table[0x105]);
				}
			}
			party_update_member_location(index, sd->account_id, sd->char_id, 0, 0, sd->mapno);
			party_remove_member(fd, sd->status.party_status, index);
		}
	}
	return 0;
}

void send_party_setup(int fd)
{
	int index;
	struct map_session_data *sd;

	if (fd != 0) {
		if (session[fd] && (sd = session[fd]->session_data)) {
			if (sd->status.party_status > 0) {
				index = party_exists(sd->status.party_id);
				if (index >= 0) {
					WFIFOW(fd, 0) = 0x101;
					WFIFOW(fd, 2) = party_dat[index].exp;
					WFIFOW(fd, 4) = party_dat[index].item;
					WFIFOSET(fd, packet_len_table[0x101]);
				}
			}
		}
	}
	return;
}

void send_party_setup_all(int index)
{
	int i;

	if (index >= 0) {
		for (i = 0; i < MAX_PARTY_MEMBERS; i++)
			send_party_setup(party_dat[index].member[i].fd);
	}
	return;
}

int check_party_share(int index)
{
	int i;
	int cfd;
	struct map_session_data *sd;
	short lower, upper;

	lower = 0;
	upper = 0;
	if (index >= 0 && party_dat[index].leader_level > 0) {
		for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
			cfd = party_dat[index].member[i].fd;
			if (cfd != 0) {
				if (session[cfd] && (sd = session[cfd]->session_data)) {
					if (lower == 0) {
						lower = sd->status.base_level;
						upper = sd->status.base_level;
					}
					else {
						if (sd->status.base_level < lower)
							lower = sd->status.base_level;

						if (sd->status.base_level > upper)
							upper = sd->status.base_level;
					}
				}
			}
		}
	}
	if (lower == 0 || upper == 0 || party_dat[index].leader_level < 1)
		return 2;

	else if ((upper - lower) > exp_party_share_level ||
		 abs(party_dat[index].leader_level - lower) > exp_party_share_leader_level ||
		 abs(party_dat[index].leader_level - upper) > exp_party_share_leader_level)
		return 0;

	else
		return 1;

}

void party_member_login(int party_num, int fd)
{
	int j;
	int index;
	struct map_session_data *sd;

	if (fd != 0 && session[fd] && (sd = session[fd]->session_data)) {
		index = party_exists(party_num);
		if (index < 0) {
			sd->status.party_status = 0;
			sd->status.party_id = -1;
			strcpy(sd->status.party_name, "");
		}
		else {
			party_update_member_map(index, sd->account_id, sd->char_id, sd->mapno, sd->mapname);
			for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
				if (party_dat[index].member[j].char_id == sd->char_id) {
					party_dat[index].member[j].fd = fd;
					party_dat[index].member[j].offline = 0;
					party_dat[index].member[j].x = sd->x;
					party_dat[index].member[j].y = sd->y;
					party_dat[index].member[j].hp = sd->status.hp;
					party_dat[index].member[j].max_hp = sd->status.max_hp;
					party_dat[index].member[j].mapno = sd->mapno;
					strcpy(party_dat[index].member[j].map_name, sd->mapname);
					break;
				}
			}
			if (j == MAX_PARTY_MEMBERS)
				return;

			if (party_dat[index].exp == 1 && check_party_share(index) == 0) {
				party_dat[index].exp = 0;
				send_party_setup_all(index);
				mmo_party_save(index);
			}
			else
				send_party_setup(fd);

			memcpy(sd->status.party_name, party_dat[index].party_name, 24);
			update_party(index);
			party_update_member_location(index, sd->account_id, sd->char_id, sd->x, sd->y, sd->mapno);
			party_update_member_hp(index, sd->account_id, sd->char_id, sd->status.hp, sd->status.max_hp, sd->mapno);
		}
	}
}

void party_member_logout(int party_num, int account_id, int char_id, short mapno)
{
	int i;
	int index;

	index = party_exists(party_num);
	if (index >= 0) {
		for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
			if (party_dat[index].member[i].char_id == char_id) {
				party_dat[index].member[i].fd = 0;
				party_dat[index].member[i].offline = 1;
				party_dat[index].member[i].x = 0;
				party_dat[index].member[i].y = 0;
				party_dat[index].member[i].hp = 0;
				party_dat[index].member[i].max_hp = 0;
				break;
			}
		}
		update_party(index);
		party_update_member_location(index, account_id, char_id, 0, 0, mapno);
	}
}

void party_update_member_location(int party_num, int account_id, int char_id, short x, short y, short mapno)
{
	short i;
	int fd;

	for (i = 0; i < MAX_PARTY_MEMBERS;i++) {
		fd = party_dat[party_num].member[i].fd;
		if (fd != 0) {
			if (session[fd]) {
				if (party_dat[party_num].member[i].char_id != char_id &&
				    party_dat[party_num].member[i].mapno == mapno) {
					WFIFOW(fd, 0) = 0x107;
					WFIFOL(fd, 2) = account_id;
					WFIFOW(fd, 6) = x;
					WFIFOW(fd, 8) = y;
					WFIFOSET(fd, packet_len_table[0x107]);
				}
				else if (party_dat[party_num].member[i].mapno != mapno) {
					WFIFOW(fd, 0) = 0x107;
					WFIFOL(fd, 2) = account_id;
					WFIFOW(fd, 6) = 0;
					WFIFOW(fd, 8) = 0;
					WFIFOSET(fd, packet_len_table[0x107]);
				}
				else if (party_dat[party_num].member[i].char_id == char_id) {
					party_dat[party_num].member[i].x = x;
					party_dat[party_num].member[i].y = y;
				}
			}
		}
	}
}

void party_update_member_map(int party_num, int account_id, int char_id, short mapno, char map_name[16])
{
	int i, fd;
	int index;

	index = party_exists(party_num);
	if (index >= 0) {
		for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
			fd = party_dat[index].member[i].fd;
			if (session[fd]) {
				if (party_dat[index].member[i].offline == 0) {
					if (party_dat[index].member[i].char_id == char_id) {
						strcpy(party_dat[index].member[i].map_name, map_name);
						party_dat[index].member[i].mapno = mapno;
						update_party(index);
						party_update_member_location(index, account_id, char_id, 0, 0, mapno);
						mmo_party_save(index);
					}
				}
			}
		}
	}
}

void party_update_member_hp(int party_num, int account_id, int char_id, int hp, int max_hp, short mapno)
{
	int i, fd;
	int index;

	index = party_exists(party_num);
	if (index >= 0) {
		for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
			fd = party_dat[index].member[i].fd;
			if (session[fd]) {
				if (party_dat[index].member[i].offline == 0) {
					if ((party_dat[index].member[i].char_id != char_id) &&
					    (party_dat[index].member[i].mapno == mapno)) {
						WFIFOW(fd, 0) = 0x106;
						WFIFOL(fd, 2) = account_id;
						WFIFOW(fd, 6) = hp;
						WFIFOW(fd, 8) = max_hp;
						WFIFOSET(fd, packet_len_table[0x106]);
					}
					else if (party_dat[index].member[i].char_id == char_id) {
						party_dat[index].member[i].hp = hp;
						party_dat[index].member[i].max_hp = max_hp;
					}
				}
			}
		}
	}
}
