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
 Modified Date: October 31, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "save.h"
#include "grfio.h"
#include "npc.h"
#include "item.h"
#include "itemdb.h"
#include "skill.h"
#include "skill_db.h"
#include "party.h"
#include "guild.h"
#include "pet.h"
#include "chat.h"
#include "element_damage.h"
#include "card.h"
#include "equip.h"
#include "trade.h"
#include "storage.h"
#include "monster_skills.h"

#define NOM_ATTACK(atk1, atk2, def) ((atk1 + rand() % (atk2)) - (def) - 3 + rand() % 8);
#define BOW_ATTACK(atk1, atk2, dex, def) ((dex + atk1 + rand() % (atk2)) - (def) - 3 + rand() % 8);
#define CRI_ATTACK(atk1, atk2, s_lv, s_type) (atk1 + atk2 + (s_lv) * (s_type) - 3 + rand() % 8);
#define KAT_ATTACK(damage) (((damage) / 5) + 1);
#define MD(min, max) ((min >= max) ? max : (min + rand() % (max - (min))));

#ifdef _MMS_
struct map_servers map_to_serv[MAX_MAP];
#endif
struct skill_db skill_db[MAX_SKILL];

/*
 * Network releate variables
 */

char userid[24];
char passwd[24];
unsigned int char_fd;
char char_ip_str[16];
short char_port = 6121;
char map_ip_str[16];
int map_ip;
char map_xip_str[256];
int map_xip;
short map_port = 5121;
int map_port_fd;

/*
 * Timer variables
 */

int check_connect_timer;
int send_users_tochar_timer;
int timed_out_timer;
int clear_flooritem_timer;
int loop_monstersearch_timer;
int heal_hp_timer;
int heal_sp_timer;
int check_save_timer;
int check_keep_alive;

/*
 * Configuration variables
 */

char map[MAX_MAP_PER_SERVER][16];
char msg[256];
long ExpData[101];
long SkillExpData[3][101];
int aspd_stats[MAX_CLASSES][13];
int job_stats[MAX_CLASSES][50];
int job_bonus[MAX_CLASSES][50][6];
int del_block_errors = 0;
int users_global;
int PVP_flag = 0;
int charcon = 0;
long next_job_exp = 0;
int first_free_object_id;
int last_object_id;
int current_attack_m = 0;
int do_timer_save = 0;
int map_number;
int display_exp;
int super_gms;
int gm_equip_all;
int show_hp;
int base_mult = 1, job_mult = 1, drop_mult = 1;
int base_max = 99;
int job_max = 50;
int console_enabled = 0;
char c_pswd[24];

int walk_char(int tid, unsigned int tick, int id, int data);
const char *logN = "Zone Server Version 1.7.1";
const int SERVER_TYPE = 3;
const int packet_len_table[0x210] = {
	10,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

	0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0, 55, 17,  3, 37,  46, -1, 23, -1,  3,108,  3,  2,
	3, 28, 19, 11,  3, -1,  9,  5,  54, 53, 58, 60, 41,  2,  6,  6,

	7,  3,  2,  2,  2,  5, 16, 12,  10,  7, 29, 23, -1, -1, -1,  0,
	7, 22, 28,  2,  6, 30, -1, -1,   3, -1, -1,  5,  9, 17, 17,  6,
	23,  6,  6, -1, -1, -1, -1,  8,   7,  6,  7,  4,  7,  0, -1,  6,
	8,  8,  3,  3, -1,  6,  6, -1,   7,  6,  2,  5,  6, 44,  5,  3,

	7,  2,  6,  8,  6,  7, -1, -1,  -1, -1,  3,  3,  6,  6,  2, 27,
	3,  4,  4,  2, -1, -1,  3, -1,   6, 14,  3, -1, 28, 29, -1, -1,
	30, 30, 26,  2,  6, 26,  3,  3,   8, 19,  5,  2,  3,  2,  2,  2,
	3,  2,  6,  8, 21,  8,  8,  2,   2, 26,  3, -1,  6, 27, 30, 10,

	2,  6,  6, 30, 79, 31, 10, 10,  -1, -1,  4,  6,  6,  2, 11, -1,
	10, 39,  4, 10, 31, 35, 10, 18,   2, 13, 15, 20, 68,  2,  3, 16,
	6, 14, -1, -1, 21,  8,  8,  8,   8,  8,  2,  2,  3,  4,  2, -1,
	6, 86,  6, -1, -1,  7, -1,  6,   3, 16,  4,  4,  4,  6, 24, 26,

	22, 14,  6, 10, 23, 19,  6, 39,   8,  9,  6, 27, -1,  2,  6,  6,
	110,  6, -1, -1, -1, -1, -1,  6,  -1, 54, 66, 54, 90, 42,  6, 42,
	-1, -1, -1, -1, -1, 30, -1,  3,  14,  3, 30, 10, 43, 14,186,182,
	14, 30, 10,  3, -1,  6,106, -1,   4,  5,  4, -1,  6,  7, -1, -1,

	6,  3,106, 10, 10, 34,  0,  6,   8,  4,  4,  4, 29, -1, 10,  6,
	90, 86, 24,  6, 30,102,  9,  4,   8,  4, 14, 10, -1,  6,  2,  6,
	3,  3, 35,  5, 11, 26, -1,  4,   4,  6, 10, 12,  6, -1,  4,  4,
	11,  7, -1, 67, 12, 18,114,  6,   3,  6, 26, 26, 26, 26,  2,  3,

	2, 14, 10, -1, 22, 22,  0,  0,  13, 97,  0,  0,  0,  0,  0,  0,
	8,  0, 10,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0, 33,  0,
	0,  8,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
};

/*======================================
 *	ZONE: Extern Functions
 *--------------------------------------
 */

#include "attack_exp.c"
#include "attack_monster.c"
#include "attack_player.c"
#include "commands.c"

/*======================================
 *	ZONE: General Zone Functions
 *--------------------------------------
 */

void add_block(struct block_list *bl, short m, short x, short y)
{
	struct block_list *head;
	long blocknumber;

	if ((m < 0) || (m >= map_number) || (x < 0) || (x >= map_data[m].xs) || (y < 0) || (y >= map_data[m].ys)) {
		printf("ADD_BLOCK Link Error: Fail to create block on map %d, x axis %d, y axis %d\n", m, x, y);
		return;
	}
	if (!map[m][0]) {
		printf("ADD_BLOCK Link Error: Map Block\n");
		return;
	}
	if (!bl) {
		printf("ADD_BLOCK Link Error: Empty Block\n");
		return;
	}
	blocknumber = x / BLOCK_SIZE + (y / BLOCK_SIZE) * map_data[m].bxs;
	if (!(head = &map_data[m].block[blocknumber]))
		return;

	bl->next = head->next;
	bl->prev = head;
	if (head->next)
		head->next->prev = bl;

	head->next = bl;
}

void del_block(struct block_list *bl)
{
	if (!bl->prev) {
		printf("DEL_BLOCK Link Error [%d]\n", del_block_errors);
		del_block_errors++;
		return;
	}
	if (bl->next)
		bl->next->prev = bl->prev;

	if (bl->prev)
		bl->prev->next = bl->next;

	bl->next = NULL;
	bl->prev = NULL;
}

int search_free_object_id(void)
{
	int i;

	if (first_free_object_id < 2 || first_free_object_id >= MAX_OBJECTS)
		first_free_object_id = 2;

	for (i = first_free_object_id; i < MAX_OBJECTS; i++) {
		if (!object[i])
			break;
	}
	if (i == MAX_OBJECTS)
		return 0;

	first_free_object_id = i;
	if (last_object_id < i)
		last_object_id = i;

	return i;
}

void delete_object(int id)
{
	if (id < 2 || !object[id])
		return;

	del_block(object[id]);
	free(object[id]);
	object[id] = NULL;

	if (first_free_object_id > id)
		first_free_object_id = id;

	while (last_object_id > 2 && !object[last_object_id]) {
		last_object_id--;
	}
}

void set_pos(unsigned char *p, short x, short y)
{
	*p = x >> 2;
	p[1] = (x << 6) | ((y >> 4) & 0x3f);
	p[2] = y << 4;
}

void set_2pos(unsigned char *p, short x0, short y0, short x1, short y1)
{
	*p = x0 >> 2;
	p[1] = (x0 << 6) | ((y0 >> 4) & 0x3f);
	p[2] = (y0 << 4) | ((x1 >> 6) & 0x0f);
	p[3] = (x1 << 2) | ((y1 >> 8) & 0x03);
	p[4] = y1;
}

void set_map(struct map_session_data *sd, char *mapname, short x, short y)
{
	int i;

	sd->x = x;
	sd->y = y;
	strncpy(sd->mapname, mapname, 16);
	for (i = 0; map[i][0]; i++) {
		if (strcmp(map[i], sd->mapname) == 0) {
			sd->mapno = i;
			break;
		}
	}
	if (sd->x < 0 || sd->x >= map_data[i].xs)
		sd->x = 0;

	if (sd->y < 0 || sd->y >= map_data[i].ys)
		sd->y = 0;

	map_data[i].users++;
	add_block(&sd->block, i, sd->x, sd->y);
}

int set_equip(struct map_session_data *sd, int item_num, int item_view)
{
	switch (item_num) {
		case 2:
			sd->status.weapon = item_view;
			break;
		case 3:
			sd->status.head_bottom = item_view;
			break;
		case 4:
			sd->status.head_top = item_view;
			break;
		case 5:
			sd->status.head_mid = item_view;
			break;
		case 8:
			sd->status.shield = item_view;
			break;
	}
	return 0;
}

int mmo_map_set_look(unsigned char *buf, int id, int type, int val)
{
	if (type == LOOK_HAIR_COLOR && val == 0)
		return 0;

	memset(buf, 0, packet_len_table[0xc3]);
	WBUFW(buf, 0) = 0xc3;
	WBUFL(buf, 2) = id;
	WBUFB(buf, 6) = type;
	WBUFB(buf, 7) = val;
	return 8;
}

int mmo_map_sendblock(short m, short bx, short by, char *dat, int len, unsigned int srcfd, int wos)
{
	unsigned int fd;
	int blocknumber;
	struct block_list *bl;
	struct map_session_data *srcsd, *dstsd;

	if (bx < 0 || bx >= map_data[m].bxs || by < 0 || by >= map_data[m].bys)
		return 0;

	blocknumber = bx + by * map_data[m].bxs;
	bl = map_data[m].block[blocknumber].next;
	if (!bl)
		return 0;

	if (session[srcfd])
		srcsd = session[srcfd]->session_data;

	else
		srcsd = NULL;

	for (;bl;bl = bl->next) {
		if (bl->type == BL_PC) {
			dstsd = (struct map_session_data *)bl;
			fd = dstsd->fd;
			if (wos && fd == srcfd)
				continue;

			if (wos == 2 && dstsd->chatID)
				continue;

			if (wos == 3 && srcsd && srcsd->chatID == dstsd->chatID)
				continue;

			memcpy(WFIFOP(fd, 0), dat, len);
			WFIFOSET(fd, len);
		}
	}
	return 1;
}

int mmo_map_sendarea(int srcfd, char *dat, int len, int wos)
{
	short bx, by;
	struct map_session_data *srcsd;

	if (!session[srcfd] || !(srcsd = session[srcfd]->session_data))
		return 0;

	if (srcsd->mapno >= 0 && srcsd->mapno < map_number) {
		for (by = (short)((srcsd->y / BLOCK_SIZE) - AREA_SIZE); by <= (short)((srcsd->y / BLOCK_SIZE) + AREA_SIZE); by++) {
			if (by < 0)
				continue;

			else if (by >= map_data[srcsd->mapno].bys)
				break;

			for (bx = (short)((srcsd->x / BLOCK_SIZE) - AREA_SIZE); bx <= (short)((srcsd->x / BLOCK_SIZE) + AREA_SIZE); bx++) {
				if (bx < 0)
					continue;

				else if (bx >= map_data[srcsd->mapno].bxs)
					break;

				mmo_map_sendblock(srcsd->mapno, bx, by, dat, len, srcfd, wos);
			}
		}
	}
	return 0;
}

int mmo_map_sendarea_mxy(short m, short x, short y, char *dat, int len)
{
	short bx, by;

	if (x < 0 || y < 0)
		return 1;

	if (m > 0 && m < map_number) {
		for (by = (short)((y / BLOCK_SIZE) - AREA_SIZE); by <= (short)((y / BLOCK_SIZE) + AREA_SIZE); by++) {
			if (by < 0)
				continue;

			else if (by >= map_data[m].bys)
				break;

			for (bx = (short)((x / BLOCK_SIZE) - AREA_SIZE); bx <= (short)((x / BLOCK_SIZE) + AREA_SIZE); bx++) {
				if (bx < 0)
					continue;

				else if (bx >= map_data[m].bxs)
					break;

				mmo_map_sendblock(m, bx, by, dat, len, 0, 0);
			}
		}
	}
	return 0;
}

int mmo_map_sendall(unsigned int srcfd, char *dat, int len, int wos)
{
	unsigned int fd;

	for (fd = 5; fd < FD_SETSIZE; fd++) {
		if (!session[fd] || !(session[fd]->session_data))
			continue;

		if (wos && fd == srcfd)
			continue;

		memcpy(WFIFOP(fd, 0), dat, len);
		WFIFOSET(fd, len);
	}
	return 0;
}

int mmo_map_sendchat(unsigned int srcfd, char *buf, int len, int wos)
{
	unsigned int fd, i;
	struct map_session_data *srcsd, *dstsd;
	struct mmo_chat* chat;

	srcsd = session[srcfd]->session_data;
	if (!srcsd)
		return 1;

	chat = (struct mmo_chat*)srcsd->chatID;
	if (!chat)
		return 1;

	for (i = 0; i < chat->users; i++) {
		fd = chat->usersfd[i];
		if (!session[fd] || !(dstsd = session[fd]->session_data))
			continue;

		if (wos && fd == srcfd)
			continue;

		memcpy(WFIFOP(fd, 0), buf, len);
		WFIFOSET(fd, len);
	}
	return 0;
}

int mmo_map_getblockchar(unsigned int fd, short m, short bx, short by)
{
	int len;
	int blocknumber;
	struct block_list *bl;

	if (bx < 0 || bx >= map_data[m].bxs)
		return 0;

	if (by < 0 || by >= map_data[m].bys)
		return 0;

	blocknumber = bx + by * map_data[m].bxs;
	bl = map_data[m].block[blocknumber].next;
	if (!bl)
		return 0;

	for (;bl;bl = bl->next) {
		switch (bl->type) {
			case BL_PC:
			{
				struct map_session_data *srcsd = (struct map_session_data *)bl;
				struct mmo_chat* temp_chat;
				if (fd == srcsd->fd)
					continue;

				if (srcsd->walktimer != 0) {
					mmo_map_set007b(srcsd, WFIFOP(fd, 0));
					WFIFOSET(fd, packet_len_table[0x7b]);
				}
				else {
					mmo_map_set0078(srcsd, WFIFOP(fd, 0));
					WFIFOSET(fd, packet_len_table[0x78]);
				}
				if ((temp_chat = (struct mmo_chat*)srcsd->chatID) && (signed)temp_chat->ownID == srcsd->account_id) {
					len = mmo_map_set00d7(srcsd->fd, WFIFOP(fd, 0));
					if (len > 0)
						WFIFOSET(fd, len);
				}
				if (srcsd->status.shopname[0] != 0) {
					WFIFOW(fd, 0) = 0x131;
					WFIFOL(fd, 2) = srcsd->account_id;
					memcpy(WFIFOP(fd, 6), &srcsd->status.shopname, 80);
					WFIFOSET(fd, packet_len_table[0x131]);
				}
				if (srcsd->spheres != 0) {
					WFIFOW(fd, 0) = 0x1d0;
					WFIFOL(fd, 2) = srcsd->account_id;
					WFIFOW(fd, 6) = srcsd->spheres;
					WFIFOSET(fd, packet_len_table[0x1d0]);
				}
				break;
			}
			case BL_NPC:
			{
				struct npc_data *nd = (struct npc_data*)bl;
				mmo_map_set_npc(WFIFOP(fd, 0), nd->id, nd->class, nd->x, nd->y, nd->dir, nd->u.mons.speed, nd->option);
				WFIFOSET(fd, packet_len_table[0x78]);
				break;
			}
			case BL_ITEM:
			{
				struct flooritem_data *nd = (struct flooritem_data*)bl;
				mmo_map_set_frameinitem(fd, nd);
				WFIFOSET(fd, packet_len_table[0x9d]);
				break;
			}
			case BL_SKILL:
			{
				struct floorskill_data *nd = (struct floorskill_data*)bl;
				mmo_map_set011f(WFIFOP(fd, 0), nd->id, nd->x, nd->y, nd->owner_id, nd->skill_type);
				WFIFOSET(fd, packet_len_table[0x11f]);
				break;
			}
		}
	}
	return 0;
}

static int mmo_map_getareachar(struct map_session_data *srcsd)
{
	int i, j;
	short bx, by;

	for (by = (short)((srcsd->y / BLOCK_SIZE) - AREA_SIZE), i = 0; i < (AREA_SIZE * 2 + 1); by++, i++) {
		if (by < 0 || by >= map_data[srcsd->mapno].bys)
			continue;

			for (bx = (short)((srcsd->x / BLOCK_SIZE) - AREA_SIZE), j = 0; j < (AREA_SIZE * 2 + 1); bx++, j++) {
				if (bx < 0 || bx >= map_data[srcsd->mapno].bxs)
					continue;

				mmo_map_getblockchar(srcsd->fd, srcsd->mapno, bx, by);
			}
	}
	return 0;
}

int mmo_map_clrblockchar(unsigned int fd, short m, short bx, short by)
{
	int blocknumber;
	struct block_list *bl;

	if (bx < 0 || bx >= map_data[m].bxs)
		return 0;

	if (by < 0 || by >= map_data[m].bys)
		return 0;

	blocknumber = bx + by * map_data[m].bxs;
	bl = map_data[m].block[blocknumber].next;
	if (!bl)
		return 0;

	for (;bl;bl = bl->next) {
		switch (bl->type) {
			case BL_PC:
			{
				struct map_session_data *srcsd = (struct map_session_data *)bl;
				struct mmo_chat* temp_chat;
				if (fd == srcsd->fd)
					continue;

				WFIFOW(fd, 0) = 0x80;
				WFIFOL(fd, 2) = srcsd->account_id;
				WFIFOB(fd, 6) = 0;
				WFIFOSET(fd, packet_len_table[0x80]);
				if ((temp_chat = (struct mmo_chat*)srcsd->chatID) && (signed)temp_chat->ownID == srcsd->account_id) {
					WFIFOW(fd, 0) = 0xd8;
					WFIFOW(fd, 2) = srcsd->chatID;
					WFIFOSET(fd, packet_len_table[0xd8]);
				}
				break;
			}
			case BL_NPC:
			{
				struct npc_data *nd = (struct npc_data*)bl;
				WFIFOW(fd, 0) = 0x80;
				WFIFOL(fd, 2) = nd->id;
				WFIFOB(fd, 6) = 0;
				WFIFOSET(fd, packet_len_table[0x80]);
				break;
			}
			case BL_ITEM:
			{
				struct flooritem_data *nd = (struct flooritem_data*)bl;
				WFIFOW(fd, 0) = 0xa1;
				WFIFOL(fd, 2) = nd->id;
				WFIFOSET(fd, packet_len_table[0xa1]);
				break;
			}
			case BL_SKILL:
			{
				struct floorskill_data *nd = (struct floorskill_data*)bl;
				WFIFOW(fd, 0) = 0x120;
				WFIFOL(fd, 2) = nd->id;
				WFIFOSET(fd,packet_len_table[0x120]);
				break;
			}
		}
	}
	return 0;
}

int mmo_map_jobchange(unsigned int fd, int class)
{
	int len;
	unsigned char buf[256];
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	sd->status.class = class;
	sd->status.job_level = 1;
	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x37;
	WFIFOL(fd, 4) = 1;
	WFIFOSET(fd, packet_len_table[0xb0]);

	len = mmo_map_set_look(buf, sd->account_id, LOOK_BASE, sd->status.class);
	if (len > 0)
		mmo_map_sendarea(fd, buf, len, 0);

	mmo_map_send_skills(fd, 1);
	mmo_map_calc_status(fd, 0);
	return 0;
}

int mmo_map_checkpvpmap(struct map_session_data *sd)
{
	unsigned int i;
	struct map_session_data *temp_sd;

	if (mmo_map_flagload(sd->mapname, PVP)) {
		for (i = 0; i < FD_SETSIZE; i++) {
			if (session[i] && (temp_sd = session[i]->session_data)) {
				if (temp_sd->mapno == sd->mapno) {
					WFIFOW(i, 0) = 0x199;
					WFIFOW(i, 2) = 1;
					WFIFOSET(i, packet_len_table[0x199]);

					WFIFOW(i, 0) = 0x19a;
					WFIFOL(i, 2) = temp_sd->account_id;
					WFIFOL(i, 6) = temp_sd->pvprank;
					WFIFOL(i, 10) = map_data[temp_sd->mapno].users;
					WFIFOSET(i, packet_len_table[0x19a]);
				}
			}
		}
	}
	return 0;
}

void mmo_map_changemap(struct map_session_data *sd, char *mapname, short x, short y, char type)
{
	int j, i;
	unsigned fd = sd->fd;
	unsigned char buf[256];
#ifdef _MMS_
	short check_map_on_server = 0;
#endif

	if (sd->status.hp <= 0)
		return;

	del_block(&sd->block);
	map_data[sd->mapno].users--;
	WBUFW(buf, 0) = 0x80;
	WBUFL(buf, 2) = sd->account_id;
	WBUFB(buf, 6) = type;
	mmo_map_sendarea(fd, buf, packet_len_table[0x80], 1);
	if (sd->status.pet.activity) {
		del_block(&map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block);
		WBUFW(buf, 0) = 0x80;
		WBUFL(buf, 2) = sd->status.pet.pet_id_as_npc;
		WBUFB(buf, 6) = 0;
		mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
	}
	for (i = 0; map[i][0]; i++) {
		if (strcmp(map[i], mapname) == 0) {
			mmo_map_checkpvpmap(sd);
			sd->new_on_map = 1;
#ifdef _MMS_
			check_map_on_server = 1;
#endif
			if (type != 2) {
				if (sd->status.party_status > 0)
					party_update_member_map(party_exists(sd->status.party_id), sd->account_id, sd->char_id, sd->mapno, sd->mapname);
			}
			set_map(sd, mapname, x, y);
			sd->state.remove_new_on_map = add_timer(gettick() + 25000, remove_new_on_map, fd, 0);

			WFIFOW(fd, 0) = 0x91;
			memcpy(WFIFOP(fd, 2), mapname, 16);
			WFIFOW(fd, 18) = x;
			WFIFOW(fd, 20) = y;
			WFIFOSET(fd, packet_len_table[0x91]);
		}
	}
#ifdef _MMS_
	if (check_map_on_server == 0) {
		for (j = 0; map_to_serv[j].map[0]; j++) {
			if (strncmp(map_to_serv[j].map, mapname, 16) == 0) {
				WFIFOW(char_fd, 0) = 0x2c01;
				WFIFOL(char_fd, 2) = sd->account_id;
				WFIFOL(char_fd, 6) = sd->char_id;
				WFIFOL(char_fd, 10) = sd->login_id1;
				WFIFOSET(char_fd, 14);

				strcpy(sd->status.last_point.map, mapname);
				sd->status.last_point.x = x;
				sd->status.last_point.y = y;
				sd->x = x;
				sd->y = y;
				memcpy(sd->mapname, mapname, 16);
				mmo_char_save(sd);

				WFIFOW(fd, 0) = 0x92;
				memcpy(WFIFOP(fd, 2), mapname, 16);
				WFIFOW(fd, 18) = x;
				WFIFOW(fd, 20) = y;
				WFIFOL(fd, 22) = map_to_serv[j].ip;
				WFIFOW(fd, 26) = map_to_serv[j].port;
				WFIFOSET(fd, packet_len_table[0x92]);
			}
		}
	}
#endif
}

void mmo_map_set0078(struct map_session_data *sd, unsigned char *buf)
{
	memset(buf, 0, packet_len_table[0x78]);
	WBUFW(buf, 0) = 0x78;
	WBUFL(buf, 2) = sd->account_id;
	WBUFW(buf, 6) = sd->speed;
	WBUFW(buf, 8) = sd->status.option_x;
	WBUFW(buf, 10) = sd->status.option_y;
	if (sd->status.special != 0)
		WBUFW(buf, 12) = sd->status.special;
	else
		WBUFW(buf, 12) = sd->status.option_z;

	WBUFW(buf, 14) = sd->status.class;
	WBUFW(buf, 16) = sd->status.hair;
	WBUFW(buf, 18) = sd->status.weapon;
	WBUFW(buf, 20) = sd->status.head_bottom;
	WBUFW(buf, 22) = sd->status.shield;
	WBUFW(buf, 24) = sd->status.head_top;
	WBUFW(buf, 26) = sd->status.head_mid;
	WBUFW(buf, 28) = sd->status.hair_color;
	WBUFW(buf, 30) = sd->status.clothes_color;
	WBUFW(buf, 32) = sd->head_dir;
	WBUFL(buf, 34) = 0;
	WBUFL(buf, 38) = 0;
	WBUFW(buf, 42) = sd->status.manner;
	WBUFW(buf, 44) = sd->status.karma;
	WBUFB(buf, 45) = sd->sex;
	set_pos(WBUFP(buf, 46), sd->x, sd->y);
	WBUFB(buf, 48) |= sd->dir & 0x0f;
	WBUFB(buf, 49) = 5;
	WBUFB(buf, 50) = 5;
	WBUFB(buf, 51) = sd->sitting;
	if (sd->status.base_level > 99)
		WBUFW(buf, 52) = 99;

	else
		WBUFW(buf, 52) = sd->status.base_level;
}

void mmo_map_set0079(struct map_session_data *sd, unsigned char *buf)
{
	memset(buf, 0, packet_len_table[0x79]);
	mmo_map_set0078(sd, buf);
	WBUFW(buf, 0) = 0x79;
	if (sd->status.base_level > 99)
		WBUFW(buf, 51) = 99;

	else
		WBUFW(buf, 51) = sd->status.base_level;
}

int mmo_map_set007b(struct map_session_data *sd, unsigned char *buf)
{
	memset(buf, 0, packet_len_table[0x7b]);
	WBUFW(buf, 0) = 0x7b;
	WBUFL(buf, 2) = sd->account_id;
	WBUFW(buf, 6) = sd->speed;
	WBUFW(buf, 8) = sd->status.option_x;
	WBUFW(buf, 10) = sd->status.option_y;
	if (sd->status.special != 0)
		WBUFW(buf, 12) = sd->status.special;
	else
		WBUFW(buf, 12) = sd->status.option_z;

	WBUFW(buf, 14) = sd->status.class;
	WBUFW(buf, 16) = sd->status.hair;
	WBUFW(buf, 18) = sd->status.weapon;
	WBUFW(buf, 20) = sd->status.head_bottom;
	WBUFL(buf, 22) = gettick();
	WBUFW(buf, 26) = sd->status.shield;
	WBUFW(buf, 28) = sd->status.head_top;
	WBUFW(buf, 30) = sd->status.head_mid;
	WBUFW(buf, 32) = sd->status.hair_color;
	WBUFW(buf, 34) = sd->status.clothes_color;
	WBUFW(buf, 36) = sd->head_dir;
	WBUFW(buf, 38) = 0;
	WBUFW(buf, 42) = 0;
	WBUFW(buf, 46) = sd->status.manner;
	WBUFB(buf, 48) = sd->status.karma;
	WBUFB(buf, 49) = sd->sex;
	set_2pos(WBUFP(buf, 50), sd->x, sd->y, sd->to_x, sd->to_y);
	WBUFB(buf, 55) = 0;
	WBUFB(buf, 56) = 5;
	WBUFB(buf, 57) = 5;
	if (sd->status.base_level > 99)
		WBUFW(buf, 58) = 99;

	else
		WBUFW(buf, 58) = sd->status.base_level;

	return 60;
}

void mmo_map_pet007b(struct map_session_data *sd, unsigned char *buf, unsigned int tick)
{
	memset(buf, 0, packet_len_table[0x7b]);
	WBUFW(buf, 0) = 0x7b;
	WBUFL(buf, 2) = sd->status.pet.pet_id_as_npc;
	WBUFW(buf, 6) = DEFAULT_MONSTER_WALK_SPEED;
	WBUFW(buf, 8) = 0;
	WBUFW(buf, 10) = 0;
	WBUFW(buf, 12) = 0;
	WBUFW(buf, 14) = sd->status.pet.pet_class;
	WBUFW(buf, 16) = 14;
	WBUFW(buf, 18) = 0;
	WBUFW(buf, 20) = 0;
	WBUFL(buf, 22) = tick;
	WBUFW(buf, 26) = 0;
	WBUFW(buf, 28) = 0;
	WBUFW(buf, 30) = 0;
	WBUFW(buf, 32) = 0;
	WBUFW(buf, 34) = 0;
	WBUFW(buf, 36) = 0;
	WBUFW(buf, 38) = 0;
	WBUFW(buf, 40) = 0;
	WBUFW(buf, 42) = 0;
	WBUFW(buf, 44) = 0;
	WBUFW(buf, 46) = 0;
	WBUFB(buf, 48) = 0;
	WBUFB(buf, 49) = 0;
	set_2pos(WBUFP(buf, 50), map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->x, map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->y, sd->x, sd->y);
	WBUFB(buf, 55) = 0;
	WBUFB(buf, 56) = 0;
	WBUFB(buf, 57) = 0;
	mmo_map_sendarea(sd->fd, buf, packet_len_table[0x7b], 0);

	WBUFW(buf, 0) = 0x1a4;
	WBUFB(buf, 2) = 0;
	WBUFL(buf, 3) = sd->status.pet.pet_id_as_npc;
	if (sd->status.pet.pet_accessory > 0)
		WBUFL(buf, 7) = sd->status.pet.pet_accessory;

	else
		WBUFL(buf, 7) = 0;

	mmo_map_sendarea(sd->fd, buf, packet_len_table[0x1a4], 0);

	WBUFW(buf, 0) = 0x1a4;
	WBUFB(buf, 2) = 5;
	WBUFL(buf, 3) = sd->status.pet.pet_id_as_npc;
	WBUFL(buf, 7) = 14;
	mmo_map_sendarea(sd->fd, buf, packet_len_table[0x1a4], 0);

	WFIFOW(sd->fd, 0) = 0x1a2;
	memcpy(WFIFOP(sd->fd, 2), sd->status.pet.pet_name, 24);
	WFIFOB(sd->fd, 26) = sd->status.pet.pet_name_flag;
	WFIFOW(sd->fd, 27) = sd->status.pet.pet_level;
	WFIFOW(sd->fd, 29) = sd->status.pet.pet_hungry;
	WFIFOW(sd->fd, 31) = sd->status.pet.pet_friendly;
	WFIFOW(sd->fd, 33) = sd->status.pet.pet_accessory;
	WFIFOSET(sd->fd, packet_len_table[0x1a2]);

	map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->x = sd->x;
	map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->y = sd->y;

	del_block(&map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block);
	add_block_npc(sd->mapno, sd->status.pet.pet_npc_id_on_map[sd->mapno]);
}

int mmo_map_set_npc(unsigned char *buf, unsigned long id, unsigned long class, int x, int y, int dir, int speed, int option)
{
	memset(buf, 0, packet_len_table[0x78]);
	WBUFW(buf, 0) = 0x78;
	WBUFL(buf, 2) = id;
	WBUFW(buf, 6) = speed;
	WBUFW(buf, 8) = 0;
	WBUFW(buf, 10) = 0;
	WBUFW(buf, 12) = option;
	WBUFW(buf, 14) = class;
	WBUFW(buf, 16) = 0;
	WBUFW(buf, 18) = 0;
	WBUFW(buf, 20) = 0;
	WBUFW(buf, 22) = 0;
	WBUFW(buf, 24) = 0;
	WBUFW(buf, 26) = 0;
	WBUFW(buf, 28) = 0;
	WBUFW(buf, 30) = 0;
	WBUFW(buf, 32) = 0;
	WBUFW(buf, 34) = 0;
	WBUFW(buf, 36) = 0;
	WBUFW(buf, 38) = 0;
	WBUFW(buf, 40) = 0;
	WBUFW(buf, 42) = 0;
	WBUFB(buf, 44) = 0;
	WBUFB(buf, 45) = 0;
	set_pos(WBUFP(buf, 46), x, y);
	WBUFB(buf, 48) |= dir & 0x0f;
	WBUFB(buf, 49) = 5;
	WBUFB(buf, 50) = 5;
	WBUFB(buf, 51) = 0;
	WBUFW(buf, 52) = 0;
	return 54;
}

int mmo_map_set_npc007c(unsigned char *buf, unsigned long id, unsigned long class, short x, short y, int speed, int option)
{
	memset(buf, 0, packet_len_table[0x7c]);
	WBUFW(buf, 0) = 0x7c;
	WBUFL(buf, 2) = id;
	WBUFW(buf, 6) = speed;
	WBUFW(buf, 8) = 0;
	WBUFW(buf, 10) = 0;
	WBUFW(buf, 12) = option;
	WBUFW(buf, 20) = class;
	set_pos(WBUFP(buf, 36), x, y);
	return 41;
}

int mmo_map_set_frameinitem(unsigned int fd, struct flooritem_data *fitem)
{
	if (!fitem->item_data.nameid)
		return 0;

	WFIFOW(fd, 0) = 0x9d;
	WFIFOL(fd, 2) = fitem->id;
	WFIFOW(fd, 6) = fitem->item_data.nameid;
	WFIFOB(fd, 8) = fitem->item_data.identify;
	WFIFOW(fd, 9) = fitem->x;
	WFIFOW(fd, 11) = fitem->y;
	WFIFOW(fd, 13) = fitem->item_data.amount;
	WFIFOB(fd, 15) = fitem->subx;
	WFIFOB(fd, 16) = fitem->suby;
	return 17;
}

int mmo_map_set_dropitem(unsigned char *buf, struct flooritem_data *fitem)
{
	memset(buf, 0, packet_len_table[0x9e]);
	WBUFW(buf, 0) = 0x9e;
	WBUFL(buf, 2) = fitem->id;
	WBUFW(buf, 6) = fitem->item_data.nameid;
	WBUFB(buf, 8) = fitem->item_data.identify;
	WBUFW(buf, 9) = fitem->x;
	WBUFW(buf, 11) = fitem->y;
	WBUFB(buf, 13) = fitem->subx;
	WBUFB(buf, 14) = fitem->suby;
	WBUFW(buf, 15) = fitem->item_data.amount;
	return 17;
}

int mmo_map_set011f(unsigned char *buf, int id, int x, int y, int owner_id, int skill_type)
{
	memset(buf, 0, packet_len_table[0x11f]);
	WBUFW(buf, 0) = 0x11f;
	WBUFL(buf, 2) = id;
	WBUFL(buf, 6) = owner_id;
	WBUFW(buf, 10) = x;
	WBUFW(buf, 12) = y;
	WBUFB(buf, 14) = skill_type;
	WBUFB(buf, 15) = 1;
	return 16;
}

int mmo_map_set00d7(unsigned int fd, unsigned char *buf)
{
	struct mmo_chat* temp_chat;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	if (!(temp_chat = (struct mmo_chat*)(sd->chatID)))
		return 0;

	WBUFW(buf, 0) = 0xd7;
	WBUFW(buf, 2) = strlen(temp_chat->title) + 17;
	WBUFL(buf, 4) = temp_chat->ownID;
	WBUFL(buf, 8) = temp_chat->chatID;
	WBUFW(buf, 12) = temp_chat->limit;
	WBUFW(buf, 14) = temp_chat->users;
	WBUFB(buf, 16) = temp_chat->pub;
	memcpy(WBUFP(buf, 17), temp_chat->title, 64);
	return WBUFW(buf, 2);
}

int mmo_map_send0095(unsigned int fd, struct map_session_data *sd, long id)
{
	unsigned int i;
	short char_id = 0, monst_id = 0;

	for (i = 5; i < FD_SETSIZE; i++) {
		if (!session[i] || !(session[i] && (sd = session[i]->session_data))) continue;
		if (sd->account_id == id) {
			char_id = 1;
			break;
		}
		else if (sd->status.pet.mons_id == id) {
			monst_id = 1;
			break;
		}
	}
	if (char_id) {
		if (sd->status.party_name || sd->status.guild_name) {
			WFIFOW(fd, 0) = 0x195;
			WFIFOL(fd, 2) = id;
			memcpy(WFIFOP(fd, 6), sd->status.name, 24);
			memcpy(WFIFOP(fd, 30), sd->status.party_name, 24);
			memcpy(WFIFOP(fd, 54), sd->status.guild_name, 24);
			memcpy(WFIFOP(fd, 78), sd->status.class_name, 24);
			WFIFOSET(fd, 102);
		}
		else {
			WFIFOW(fd, 0) = 0x95;
			WFIFOL(fd, 2) = id;
			memcpy(WFIFOP(fd, 6), sd->status.name, 24);
			WFIFOSET(fd, 30);
		}
		return 1;
	}
	else if (monst_id) {
		WFIFOW(fd, 0) = 0x95;
		WFIFOL(fd, 2) = id;
		memcpy(WFIFOP(fd, 6), sd->status.pet.pet_name, 24);
		WFIFOSET(fd, 30);
		return 1;
	}
	else {
		sd = session[fd]->session_data;
		for (i = 0; i < (unsigned)map_data[sd->mapno].npc_num; i++) {
			if (map_data[sd->mapno].npc[i]->id == id) {
				WFIFOW(fd, 0) = 0x95;
				WFIFOL(fd, 2) = id;
				memcpy(WFIFOP(fd, 6), map_data[sd->mapno].npc[i]->name, 24);
				WFIFOSET(fd, 30);
				return 1;
			}
		}
	}
	return 0;
}

void mmo_map_set00b1(unsigned int fd, int type, int val)
{
	WFIFOW(fd, 0) = 0xb1;
	WFIFOW(fd, 2) = type;
	WFIFOL(fd, 4) = val;
	WFIFOSET(fd, packet_len_table[0xb1]);
}

int mmo_map_set00bd(unsigned char *buf, struct map_session_data *sd)
{
	memset(buf, 0, packet_len_table[0xbd]);
	WBUFW(buf, 0) = 0xbd;
	WBUFW(buf, 2) = sd->status.status_point;
	WBUFB(buf, 4) = sd->status.str;
	WBUFB(buf, 5) = (sd->status.str + 9) / 10 + 1;
	WBUFB(buf, 6) = sd->status.agi;
	WBUFB(buf, 7) = (sd->status.agi + 9) / 10 + 1;
	WBUFB(buf, 8) = sd->status.vit;
	WBUFB(buf, 9) = (sd->status.vit + 9) / 10 + 1;
	WBUFB(buf, 10) = sd->status.int_;
	WBUFB(buf, 11) = (sd->status.int_ + 9) / 10 + 1;
	WBUFB(buf, 12) = sd->status.dex;
	WBUFB(buf, 13) = (sd->status.dex + 9) / 10 + 1;
	WBUFB(buf, 14) = sd->status.luk;
	WBUFB(buf, 15) = (sd->status.luk + 9) / 10 + 1;
	WBUFW(buf, 16) = sd->status.str + (sd->status.str % 10) * (sd->status.str % 10) + (sd->status.dex / 5);
	WBUFW(buf, 18) = 0;
	WBUFW(buf, 20) = sd->status.int_ + (sd->status.int_ * sd->status.int_ / 25);
	WBUFW(buf, 22) = sd->status.int_ + (sd->status.int_ * sd->status.int_ / 49);
	WBUFW(buf, 24) = 0;
	WBUFW(buf, 26) = sd->status.vit;
	WBUFW(buf, 28) = 0;
	WBUFW(buf, 30) = sd->status.int_;
	WBUFW(buf, 32) = sd->status.base_level + sd->status.dex;
	WBUFW(buf, 34) = sd->status.base_level + sd->status.agi;
	WBUFW(buf, 36) = (sd->status.luk / 10) + 1;
	WBUFW(buf, 38) = 1 + (sd->status.luk / 3);
	WBUFW(buf, 40) = sd->status.karma;
	WBUFW(buf, 42) = sd->status.manner;
	return 44;
}

int mmo_map_set010b(int exp, struct map_session_data *sd)
{
	int i = 0;
	unsigned int fd = sd->fd;

	WFIFOW(fd, 0) = 0x10b;
	WFIFOL(fd, 2) = exp;
	WFIFOSET(fd, packet_len_table[0x10b]);
	sd->status.base_exp += exp;
	while(sd->status.base_exp >= ExpData[sd->status.base_level + i] && (sd->status.base_level <= 100)) {
		sd->status.base_exp = sd->status.base_exp - ExpData[sd->status.base_level + i];
		i++;
	}
	if (i)
		mmo_map_level_up(fd, i);

	mmo_map_set00b1(fd, 0x01, sd->status.base_exp);
	mmo_map_set00b1(fd, 0x16, ExpData[sd->status.base_level]);
	return 0;
}

int mmo_map_set010a(unsigned int fd, int item_id)
{
	struct item tmp_item;

	WFIFOW(fd, 0) = 0x10a;
	WFIFOW(fd, 2) = item_id;
	WFIFOSET(fd, packet_len_table[0x10a]);

	memset(&tmp_item, 0, sizeof(tmp_item));
	tmp_item.nameid = item_id;
	tmp_item.amount = 1;
	tmp_item.identify = 1;
	mmo_map_get_item(fd, &tmp_item);
	return 0;
}

int mmo_map_set_param(unsigned int fd, unsigned char *buf, int type)
{
	int len = 8;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	memset(buf, 0, packet_len_table[0xb0]);
	WBUFW(buf, 0) = 0xb0;
	WBUFW(buf, 2) = type;
	switch (type) {
		case SP_STATUSPOINT:
			WBUFL(buf, 4) = sd->status.status_point;
			break;
		case SP_ASPD:
			WBUFL(buf, 4) = sd->status.aspd;
			break;
		case SP_ATK1:
			WBUFW(buf, 4) = sd->status.atk1;
			break;
		case SP_MATK1:
			WBUFL(buf, 4) = sd->status.matk1;
			break;
		case SP_MATK2:
			WBUFL(buf, 4) = sd->status.matk2;
			break;
		case SP_DEF2:
			WBUFW(buf, 4) = sd->status.def2;
			break;
		case SP_MDEF2:
			WBUFW(buf, 4) = sd->status.mdef2;
			break;
		case SP_HIT:
			WBUFL(buf, 4) = sd->status.hit;
			break;
		case SP_HP:
			WBUFL(buf, 4) = sd->status.hp;
			break;
		case SP_FLEE1:
			WBUFW(buf, 4) = sd->status.flee1;
			break;
		case SP_FLEE2:
			WBUFW(buf, 4) = sd->status.flee2;
			break;
		case SP_SKILLPOINT:
			WBUFL(buf, 4) = sd->status.skill_point;
			break;
		case SP_ZENY:
			WBUFW(buf, 0) = 0xb1;
			WBUFL(buf, 4) = sd->status.zeny;
			break;
		case SP_WEIGHT:
			WBUFL(buf, 4) = sd->weight;
			break;
		case SP_MAXWEIGHT:
			WBUFL(buf, 4) = sd->max_weight;
			break;
		case SP_STR:
			WBUFW(buf, 0) = 0x141;
			WBUFL(buf, 2) = type;
			WBUFL(buf, 6) = sd->status.str;
			WBUFL(buf, 10) = 0;
			len = 14;
			break;
		case SP_AGI:
			WBUFW(buf, 0) = 0x141;
			WBUFL(buf, 2) = type;
			WBUFL(buf, 6) = sd->status.agi;
			WBUFL(buf, 10) = 0;
			len = 14;
			break;
		case SP_VIT:
			WBUFW(buf, 0) = 0x141;
			WBUFL(buf, 2) = type;
			WBUFL(buf, 6) = sd->status.vit;
			WBUFL(buf, 10) = 0;
			len = 14;
			break;
		case SP_INT:
			WBUFW(buf, 0) = 0x141;
			WBUFL(buf, 2) = type;
			WBUFL(buf, 6) = sd->status.int_;
			WBUFL(buf, 10) = 0;
			len = 14;
			break;
		case SP_DEX:
			WBUFW(buf, 0) = 0x141;
			WBUFL(buf, 2) = type;
			WBUFL(buf, 6) = sd->status.dex;
			WBUFL(buf, 10) = 0;
			len = 14;
			break;
		case SP_LUK:
			WBUFW(buf, 0) = 0x141;
			WBUFL(buf, 2) = type;
			WBUFL(buf, 6) = sd->status.luk;
			WBUFL(buf, 10) = 0;
			len = 14;
			break;
		default:
			len = 0;
			break;
	}
	return len;
}

int mmo_map_update_param(unsigned int fd, int type, int val)
{
	int len;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	switch (type) {
		case SP_ZENY:
			sd->status.zeny = sd->status.zeny + val;
			break;
		case SP_STATUSPOINT:
			sd->status.status_point = sd->status.status_point + val;
			break;
		case SP_SKILLPOINT:
			sd->status.skill_point = sd->status.skill_point + val;
			break;
	}
	len = mmo_map_set_param(fd, WFIFOP(fd, 0), type);
	if (len > 0)
		WFIFOSET(fd, len);

	return 0;
}

/*======================================
 *	ZONE: General Player Calculations
 *--------------------------------------
 */

void set_lvup_table(void)
{
	int i = 1;
	char line[2040];
	FILE *fp = fopen("data/tables/exp.txt", "r");
	if (fp) {
		long tExpData[101];
		long tSkillExpData[3][101];
		while (fgets(line, 1020, fp)) {
			if (sscanf(line, "%ld,%ld,%ld,%ld",
				&tExpData[i],
				&tSkillExpData[0][i],
				&tSkillExpData[1][i],
				&tSkillExpData[2][i]) != 4)
				continue;

			ExpData[i] = tExpData[i];
			SkillExpData[0][i] = tSkillExpData[0][i];
			SkillExpData[1][i] = tSkillExpData[1][i];
			SkillExpData[2][i] = tSkillExpData[2][i];
			i++;
			if (i >= 110)
				break;
		}
	}
	else {
		printf("**Error: Cannot load exp.txt!**\n");
		sleep(2);
		exit(0);
	}
	fclose(fp);
}

void init_aspd_stats(void)
{
	int i, j = 0;
	char line[1024], *p;
	FILE *fp = fopen("data/tables/attack.txt", "r");
	if (fp == NULL){
		printf("**Error: Cannot load attack.txt!**\n");
		sleep(2);
		exit(0);
	}
	while (fgets(line, 1020, fp)) {
		for (i = 0, p = line; i < 13 && p; i++) {
			if (sscanf(p, "%d", &aspd_stats[j][i]) == 0)
				break;

			p = strchr(p,',');
			if (p)
				p++;
		}
		j++;
		if (j == MAX_CLASSES)
			break;
	}
	fclose(fp);
}

void init_job_stats(void)
{
	int i, k, j = 0;
	char line[1024], *p;
	FILE *fp = fopen("data/tables/job_bonus.txt", "r");
	if (fp == NULL){
		printf("**Error: Cannot load job_bonus.txt!**\n");
		sleep(2);
		exit(0);
	}
	while (fgets(line, 1020, fp)) {
		for (i = 0, p = line; i < 50 && p; i++) {
			if (sscanf(p, "%d", &job_stats[j][i]) == 0)
				break;
			p = strchr(p,',');
			if (p)
				p++;
		}
		j++;
		if (j == MAX_CLASSES)
			break;
	}
	fclose(fp);

	for (i = 0; i < MAX_CLASSES; i++) {
		for (k = 0; k < 6; k++) {
			job_bonus[i][0][k] = 0;
		}
		if (job_stats[i][0] > 0)
			job_bonus[i][0][job_stats[i][0]-1]++;

		for (j = 1; j < 50; j++) {
			for (k = 0; k < 6; k++) {
				job_bonus[i][j][k] = job_bonus[i][j-1][k];
			}
			if(job_stats[i][j] > 0)
				job_bonus[i][j][job_stats[i][j]-1]++;
		}
	}
}

void calc_job_status(struct mmo_charstatus *p, int class, int level)
{
	int j;

	for (j = 0; j < level; j++) {
		switch (job_stats[class][j]) {
			case 1:
				p->str2++;
				break;
			case 5:
				p->dex2++;
				break;
			case 3:
				p->vit2++;
				break;
			case 2:
				p->agi2++;
				break;
			case 4:
				p->int_2++;
				break;
			case 6:
				p->luk2++;
				break;
		}
	}
}

int calc_need_status_point(struct map_session_data *sd, int type)
{
	int val;

	if ((type < SP_STR) || (type > SP_LUK))
		return -1;

	val = type == SP_STR ? sd->status.str :
		type == SP_AGI ? sd->status.agi :
		type == SP_VIT ? sd->status.vit :
		type == SP_INT ? sd->status.int_:
		type == SP_DEX ? sd->status.dex : sd->status.luk;
	return (val + 9) / 10 + 1;
}

void mmo_map_calc_overweight(struct map_session_data *sd)
{
	if (sd->weight >= sd->max_weight * 0.9) {
		if (sd->overweight_status != 2) {
			if (sd->overweight_status == 1)
				skill_icon_effect(sd, 35, 0);

			skill_icon_effect(sd, 36, 1);
			sd->overweight_status = 2;
		}
	}
	else if (sd->weight >= sd->max_weight / 2) {
		if (sd->overweight_status != 1) {
			if (sd->overweight_status == 2)
				skill_icon_effect(sd, 36, 0);

			skill_icon_effect(sd, 35, 1);
			sd->overweight_status = 1;
		}
	}
	else {
		if (sd->overweight_status == 1)
			skill_icon_effect(sd, 35, 0);

		if (sd->overweight_status == 2)
			skill_icon_effect(sd, 36, 0);

		sd->overweight_status = 0;
	}
}

int mmo_map_calc_sigma(int k, double val)
{
	int i;
	double j = 0;

	for (i = 1; i <= k; i++) {
		j = (double)(i * val) + j;
	}
	return (int)j;
}

int mmo_map_update_status(struct map_session_data *sd, unsigned char *buf)
{
	if (sd->status.option_z == 2 || sd->status.option_z == 8192 || sd->status.option_z == 1) {
		sd->status.option_z = 0;
		memset(buf, 0, packet_len_table[0x119]);
		WBUFW(buf, 0) = 0x119;
		WBUFL(buf, 2) = sd->account_id;
		WBUFW(buf, 6) = 0;
		WBUFW(buf, 8) = 0;
		WBUFW(buf, 10) = 0;
		WBUFB(buf, 12) = 0;
		mmo_map_sendarea(sd->fd, buf, packet_len_table[0x119], 0);
	}
	/*
	Todo: Add here effects after a player change of map or relog.
	Example: Poison, the poison will continue until effect is removed by medicine or skill.
	*/
	return 0;
}

int mmo_map_calc_effect(struct map_session_data *sd)
{
	if (sd->status.effect & 0xdf
	|| sd->status.effect & 0x80
	|| sd->status.effect & 0x20
	|| sd->status.effect & 0x0f)
		return 1;

	return 0;
}

int mmo_map_calc_status(unsigned int fd, int item_num)
{
	int i;
	int iWeapon_class2;
	struct item_db2 item_equip;
	struct map_session_data *sd;
	struct mmo_charstatus *p;
	short temp_str;
	short temp_agi;
	short temp_vit;
	short temp_int;
	short temp_dex;
	short temp_luk;
	struct item_db2 item_cd;
	int j;

	item_equip = item_database(item_num);
	sd = session[fd]->session_data;
	p = &sd->status;
	// 255 fix
	iWeapon_class2 = 0;
	p->str2 = 0;
	p->agi2 = 0;
	p->dex2 = 0;
	p->vit2 = 0;
	p->int_2 = 0;
	p->luk2 = 0;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if ((p->inventory[i].nameid) && (p->inventory[i].equip)) {
			item_equip = item_database(p->inventory[i].nameid);
			p->str2 += item_equip.str;
			p->agi2 += item_equip.agi;
			p->dex2 += item_equip.dex;
			p->vit2 += item_equip.vit;
			p->int_2 += item_equip.int_;
			p->luk2 += item_equip.luk;
			if (item_equip.slot != 0) {
				for (j = 0; j < item_equip.slot; j++) {
					if (sd->status.inventory[i].card[j] != 0) {
						item_cd = item_database(sd->status.inventory[i].card[j]);
						p->str2 += item_cd.str;
						p->agi2 += item_cd.agi;
						p->dex2 += item_cd.dex;
						p->vit2 += item_cd.vit;
						p->int_2 += item_cd.int_;
						p->luk2 += item_cd.luk;
					}
				}
			}
		}
	}

	// ******************* Stat adding skills *******************
	// for skills that affect the stats of a character, define them here

	p->dex2 += p->skill[43-1].lv; // correction on Owl's Eye skill

	if (sd->skill_timeamount[29-1][0] > 0) { // Increase Agility
		sd->status.agi2 += sd->skill_timeamount[29-1][1];
	}
	if (sd->skill_timeamount[30-1][0] > 0) { // Decrease Agility
		sd->status.agi2 -= sd->skill_timeamount[30-1][1];
	}
	if (sd->skill_timeamount[34-1][0] > 0) {
		sd->status.str2 += sd->skill_timeamount[34-1][1];
		sd->status.dex2 += sd->skill_timeamount[34-1][1];
		sd->status.int_2 += sd->skill_timeamount[34-1][1];
	}
	if (sd->skill_timeamount[45-1][0] > 0) {
		sd->status.dex2 += sd->skill_timeamount[45-1][1];
		sd->status.agi2 += sd->skill_timeamount[45-1][1];
	}
	if (sd->skill_timeamount[75-1][0] > 0) {
		sd->status.luk2 += sd->skill_timeamount[75-1][1];
	}
	if (sd->skill_timeamount[155-1][0] > 0) {
		sd->status.str2 += sd->skill_timeamount[155-1][1];
	}

	// **********************************************************

	//Job bonuses
	p->str2 += job_bonus[p->class][p->job_level-1][0];
	p->agi2 += job_bonus[p->class][p->job_level-1][1];
	p->vit2 += job_bonus[p->class][p->job_level-1][2];
	p->int_2 += job_bonus[p->class][p->job_level-1][3];
	p->dex2 += job_bonus[p->class][p->job_level-1][4];
	p->luk2 += job_bonus[p->class][p->job_level-1][5];

	temp_str = p->str + p->str2;
	temp_agi = p->agi + p->agi2;
	temp_vit = p->vit + p->vit2;
	temp_int = p->int_+ p->int_2;
	temp_dex = p->dex + p->dex2;
	temp_luk = p->luk + p->luk2;

	// HP calculation
	switch(p->class) {
	case 0: // novice
		p->max_hp = (double)(35 + p->base_level * 5) * (double)(1 + temp_vit / 100);
		p->max_sp = p->base_level * 2 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 20000 + temp_str * 300; // (2000 + temp_str * 30) * 10 (display is 10 times less of real value)
		break;
	case 1: // swordsman
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.7)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 2 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 28000 + temp_str * 300; // (2000 + temp_str * 30 + 800) * 10 (display is 10 times less of real value)
		break;
	case 2: // mage
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.3)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 6 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 22000 + temp_str * 300; // (2000 + temp_str * 30 + 200) * 10 (display is 10 times less of real value)
		break;
	case 3: // archer
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.5)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 2 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 26000 + temp_str * 300; // (2000 + temp_str * 30 + 600) * 10 (display is 10 times less of real value)
		break;
	case 4: // acolyte
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.4)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 5 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 24000 + temp_str * 300; // (2000 + temp_str * 30 + 400) * 10 (display is 10 times less of real value)
		break;
	case 5: // merchant
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.4)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 3 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 28000 + temp_str * 300; // (2000 + temp_str * 30 + 800) * 10 (display is 10 times less of real value)
		break;
	case 6: // theif
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.5)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 2 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 24000 + temp_str * 300; // (2000 + temp_str * 30 + 400) * 10 (display is 10 times less of real value)
		break;
	case 7: // knight
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 1.5)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 3 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 28000 + temp_str * 300; // (2000 + temp_str * 30 + 800) * 10 (display is 10 times less of real value)
		break;
	case 8: // priest
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.75)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 7 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 26000 + temp_str * 300; // (2000 + temp_str * 30 + 600) * 10 (display is 10 times less of real value)
		break;
	case 9: // wizard
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.55)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 9 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 24000 + temp_str * 300; // (2000 + temp_str * 30 + 400) * 10 (display is 10 times less of real value)
		break;
	case 10: // blacksmith
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.9)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 4 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 30000 + temp_str * 300; // (2000 + temp_str * 30 + 1000) * 10 (display is 10 times less of real value)
		break;
	case 11: // hunter
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.85)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 4 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 27000 + temp_str * 300; // (2000 + temp_str * 30 + 700) * 10 (display is 10 times less of real value)
		break;
	case 12: // assassin
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 1.1)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 4 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 24000 + temp_str * 300; // (2000 + temp_str * 30 + 400) * 10 (display is 10 times less of real value)
		break;
	case 14: // crusader
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 1.5)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 3 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 28000 + temp_str * 300; // (2000 + temp_str * 30 + 800) * 10 (display is 10 times less of real value)
		break;
	case 15: // monk
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.75)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 7 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 26000 + temp_str * 300; // (2000 + temp_str * 30 + 600) * 10 (display is 10 times less of real value)
		break;
	case 16: // sage
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.55)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 9 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 24000 + temp_str * 300; // (2000 + temp_str * 30 + 400) * 10 (display is 10 times less of real value)
		break;
	case 17: // rogue
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 1.1)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 4 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 24000 + temp_str * 300; // (2000 + temp_str * 30 + 400) * 10 (display is 10 times less of real value)
		break;
	case 18: // alchemist
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.9)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 4 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 30000 + temp_str * 300; // (2000 + temp_str * 30 + 1000) * 10 (display is 10 times less of real value)
		break;
	case 19: // bard
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.85)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 4 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 27000 + temp_str * 300; // (2000 + temp_str * 30 + 700) * 10 (display is 10 times less of real value)
		break;
	case 20: // dancer
		p->max_hp = (double)(35 + p->base_level * 5 + mmo_map_calc_sigma(p->base_level, 0.85)) * (double)(1 + ((double)temp_vit / 100));
		p->max_sp = p->base_level * 4 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 27000 + temp_str * 300; // (2000 + temp_str * 30 + 700) * 10 (display is 10 times less of real value)
		break;
	default: //Give novice stats
		p->max_hp = (double)(35 + p->base_level * 5) * (double)(1 + temp_vit / 100);
		p->max_sp = p->base_level * 2 * (double)(1 + ((double)temp_int / 100));
		sd->max_weight = 20000 + temp_str * 300; // (2000 + temp_str * 30) * 10 (display is 10 times less of real value)
		break;
	}
	//CR_TRUST
	p->max_hp += 200 * p->skill[248-1].lv;

	if (p->max_hp > 30000 || p->max_hp < 0) { // max HP available is 30000
		p->max_hp = 30000;
	}
	// This is problematic, and causes GMs to get overhealed to death.
	if (super_gms == 1) {
		if (sd->account_id > 100000) {
			p->max_hp = 32500;
			p->max_sp = 32500;
		}
	}
	// ATK, ASPD, MATK, DEF, MDEF, HIT, FLEE calculations
	// fix starts with Poots additions

	// wpn search -> for use in calc of atk1 and aspd only
	p->iWeapon_class = 0;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if ((p->inventory[i].nameid) && (p->inventory[i].equip & 0x02)) {
			if (p->inventory[i].nameid > 1100 &&  p->inventory[i].nameid < 1150) {
				if (p->inventory[i].nameid > 1115 && p->inventory[i].nameid < 1119) {
					p->iWeapon_class = 3;
				} else {
					p->iWeapon_class = 2;
				}
			} else if (p->inventory[i].nameid > 1150 && p->inventory[i].nameid < 1180) {
				p->iWeapon_class = 3;
			} else if (p->inventory[i].nameid > 1200 && p->inventory[i].nameid < 1240) {
				p->iWeapon_class = 1;
			} else if (p->inventory[i].nameid > 1240 && p->inventory[i].nameid < 1270) {
				p->iWeapon_class = 11;
			} else if (p->inventory[i].nameid > 1300 && p->inventory[i].nameid < 1370) {
				if (p->inventory[i].nameid > 1359 && p->inventory[i].nameid < 1363) {
					p->iWeapon_class = 10;
				} else {
					p->iWeapon_class = 9;
				}
			} else if (p->inventory[i].nameid > 1400 && p->inventory[i].nameid < 1420) {
				p->iWeapon_class = 4;
			} else if (p->inventory[i].nameid > 1430 && p->inventory[i].nameid < 1480) {
				p->iWeapon_class = 5;
			} else if (p->inventory[i].nameid > 1500 && p->inventory[i].nameid < 1530) {
				p->iWeapon_class = 8;
			} else if (p->inventory[i].nameid > 1540 && p->inventory[i].nameid < 1560) {
				p->iWeapon_class = 12;
			} else if (p->inventory[i].nameid > 1600 && p->inventory[i].nameid < 1620) {
				p->iWeapon_class = 7;
			} else if (p->inventory[i].nameid > 1700 && p->inventory[i].nameid < 1730) {
				p->iWeapon_class = 6;
			}
		} else if ((p->inventory[i].nameid) && (p->inventory[i].equip == 32)) {
			if (p->inventory[i].nameid > 1100 && p->inventory[i].nameid < 1150) {
				if (p->inventory[i].nameid > 1115 && p->inventory[i].nameid < 1119) {
					iWeapon_class2 = 3;
				} else {
					iWeapon_class2 = 2;
				}
			} else if (p->inventory[i].nameid > 1150 && p->inventory[i].nameid < 1180) {
				iWeapon_class2 = 3;
			} else if (p->inventory[i].nameid > 1200 && p->inventory[i].nameid < 1240) {
				iWeapon_class2 = 1;
			} else if (p->inventory[i].nameid > 1240 && p->inventory[i].nameid < 1270) {
				iWeapon_class2 = 11;
			} else if (p->inventory[i].nameid > 1300 && p->inventory[i].nameid < 1370) {
				if (p->inventory[i].nameid > 1359 && p->inventory[i].nameid < 1363) {
					iWeapon_class2 = 10;
				} else {
					iWeapon_class2 = 9;
				}
			} else if (p->inventory[i].nameid > 1400 && p->inventory[i].nameid < 1420) {
				iWeapon_class2 = 4;
			} else if (p->inventory[i].nameid > 1430 && p->inventory[i].nameid < 1480) {
				iWeapon_class2 = 5;
			} else if (p->inventory[i].nameid > 1500 && p->inventory[i].nameid < 1530) {
				iWeapon_class2 = 8;
			} else if (p->inventory[i].nameid > 1540 && p->inventory[i].nameid < 1560) {
				iWeapon_class2 = 12;
			} else if (p->inventory[i].nameid > 1600 && p->inventory[i].nameid < 1620) {
				iWeapon_class2 = 7;
			} else if (p->inventory[i].nameid > 1700 && p->inventory[i].nameid < 1730) {
				iWeapon_class2 = 6;
			}
		}
	}

	if (p->iWeapon_class == 6) {
		p->atk1 = temp_dex + (temp_dex / 10) * (temp_dex / 10) + (temp_str / 5) + (temp_luk / 5); // we create progression by step. Don't simplify with (temp_dex * temp_dex / 100)
	} else {
		p->atk1 = temp_str + (temp_str / 10) * (temp_str / 10) + (temp_dex / 5) + (temp_luk / 5); // we create progression by step. Don't simplify with (temp_str * temp_str / 100)
	}
	p->atk2 = 1;
	p->aspd = (long)((aspd_stats[sd->status.class][p->iWeapon_class]) * (float)(((250 - (temp_agi) - ((temp_dex) / 4.0f))) / 250.0f));
	if (iWeapon_class2) {
		p->aspd += (long)((aspd_stats[sd->status.class][iWeapon_class2]) * (float)(((250 - (temp_agi) - ((temp_dex) / 4.0f))) / 250.0f));
		p->aspd = p->aspd * 3 / 4;
	}
	p->matk1 = temp_int + (temp_int / 5) * (temp_int / 5); // we create progression by step. Don't simplify with (temp_int * temp_int / 25);
	p->matk2 = temp_int + (temp_int / 7) * (temp_int / 7); // we create progression by step. Don't simplify with (temp_int * temp_int / 49);
	p->def1 = temp_vit;
	p->def2 = 1;
	p->mdef1 = temp_int;
	p->mdef2 = 1;
	p->hit = sd->status.base_level + temp_dex;
	p->flee1 = sd->status.base_level + temp_agi;
	p->flee2 = (temp_luk / 10);
	p->critical = 1 + temp_luk / 3;
	p->range = 1;
	sd->speed = DEFAULT_WALK_SPEED - (p->agi + p->agi2) / 5;

	for (i=0;i<MAX_INVENTORY;i++) {
		if ((p->inventory[i].nameid)&&(p->inventory[i].equip)) {
			item_equip = item_database(p->inventory[i].nameid);
			if ((p->inventory[i].equip!=32)||(item_equip.type != 4)) {
				if(item_equip.atk != 0)
					p->atk1 += item_equip.atk;
				if(item_equip.atk != 0)
					if(item_database(sd->status.inventory[i].nameid).wlv == 1)
						p->atk2 += (p->inventory[i].refine*2);
				if(item_database(sd->status.inventory[i].nameid).wlv == 2)
					p->atk2 += (p->inventory[i].refine*3);
				if(item_database(sd->status.inventory[i].nameid).wlv >= 3)
					p->atk2 += (p->inventory[i].refine*4);
			}

			WFIFOW(fd, 0) = 0x13a;
			if (item_equip.range < 1) {
				//WFIFOW(fd, 2) = 1;
				p->range = 1;
			}
			else {
				//WFIFOW(fd, 2) = item_equip.range;
				p->range = item_equip.range;
			}
			if (p->iWeapon_class == 6) {
				p->range += sd->status.skill[44-1].lv;
			}
			WFIFOW(fd, 2) = p->range;
			WFIFOSET(fd, packet_len_table[0x13a]);

			if(item_equip.matk != 0)
				p->matk1 += item_equip.matk;
			if(item_equip.def != 0)
				p->def2 += item_equip.def;
			if(item_equip.def != 0)
				p->def2 += (p->inventory[i].refine);
			if(item_equip.mdef != 0)
				p->mdef2 += item_equip.mdef;
			p->hit += item_equip.hit;
			p->flee1 += item_equip.flee;
			p->critical += item_equip.critical;
			// Legato fix ends
			if(item_equip.slot != 0) {
				for(j=0;j<item_equip.slot;j++){
					if(sd->status.inventory[i].card[j] != 0){
						item_cd = item_database(sd->status.inventory[i].card[j]);

						if ((p->inventory[i].equip!=32)||(item_equip.type != 4)) {
							if(item_cd.atk != 0)
								p->atk1 += item_cd.atk;
						}
						if(item_cd.matk != 0)
							p->matk1 += item_cd.matk;
						if(item_cd.def != 0)
							p->def2 += item_cd.def;
						if(item_cd.mdef != 0)
							p->mdef2 += item_cd.mdef;
						p->hit += item_cd.hit;
						p->flee1 += item_cd.flee;
						p->critical += item_cd.critical;
						p->max_hp += item_cd.hp1;
						p->max_sp += item_cd.sp1;
					}
				}
			}
		}
	}


	// ***************************skill**********************************
	// Skills that need to do something when a persons status is updated.
	// similar as in mmo_skil_lvl_up .. hmmm
	// edit: place skills that affect substats here

	// Merc skills
	// Update weight limit
	sd->max_weight += 1000 * sd->status.skill[36-1].lv;

	// Update Speed if they are a pecopeco rider
	if (sd->status.special & 0x20) { // pecopeco rider
		sd->status.aspd = sd->status.aspd * (2.0 - 0.2 * sd->status.skill[64-1].lv);
		sd->speed = sd->speed * 3 / 4;
	}
	// Update Speed if they have a Pushcart
	if ((sd->status.special & 0x8) == 8) {
		sd->speed = sd->speed + (10 - sd->status.skill[39-1].lv) * DEFAULT_WALK_SPEED / 10;
	}

	// Swordsman
	// sword mastery
	// all mastery skills are in skill.c damage function
	// same construct as in player attack here

	// skill bonuses
	if (sd->skill_timeamount[111-1][0] > 0) {
		sd->status.aspd = sd->status.aspd * sd->skill_timeamount[111-1][1] / 100;
	}
	if (sd->skill_timeamount[60-1][0] > 0) {
		sd->status.aspd = sd->status.aspd * sd->skill_timeamount[60-1][1] / 100;
	}
	if (sd->skill_timeamount[33-1][0] > 0) {
		sd->status.def1 += sd->skill_timeamount[33-1][1];
	}
	if (sd->skill_timeamount[1-1][0] > 0) {
		sd->status.aspd = sd->status.aspd * 100 / sd->skill_timeamount[1-1][1];
	}
	if (sd->skill_timeamount[29-1][0] > 0) { // INCREASE AGI
		sd->speed = sd->speed * 3 / 4; // +25% speed (in msec)
	}
	if (sd->skill_timeamount[30-1][0] > 0) { // DECREASE AGI
		sd->speed = sd->speed * 5 / 4; // -25% speed (in msec)
	}
	if (sd->skill_timeamount[92-1][0] > 0) {
		sd->speed = sd->speed * 3 / 2;
	}
	if (sd->status.skill[265-1].lv > 0) {
		switch (sd->status.skill[265-1].lv) {
			case 1:
				p->flee1 += 1;
				break;
			case 2:
				p->flee1 += 3;
				break;
			case 3:
				p->flee1 += 4;
				break;
			case 4:
				p->flee1 += 6;
				break;
			case 5:
				p->flee1 += 7;
				break;
			case 6:
				p->flee1 += 9;
				break;
			case 7:
				p->flee1 += 10;
				break;
			case 8:
				p->flee1 += 12;
				break;
			case 9:
				p->flee1 += 13;
				break;
			case 10:
				p->flee1 += 15;
				break;
		}

	}

	if (sd->speed < 10) {
		sd->speed = 10;
	}

	// ***************************skill*end*******************************
	if (p->aspd < 1) {
		p->aspd = 1;
	}

	// MAX_HP renewal
	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x06;
	WFIFOL(fd, 4) = p->max_hp;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x08;
	WFIFOL(fd, 4) = p->max_sp;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x19;
	WFIFOL(fd, 4) = sd->max_weight; // weight fix
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0x141;
	WFIFOL(fd, 2) = SP_STR;
	WFIFOL(fd, 6) = p->str;
	WFIFOL(fd, 10) = p->str2;
	WFIFOSET(fd, packet_len_table[0x141]);

	WFIFOW(fd, 0) = 0x141;
	WFIFOL(fd, 2) = SP_AGI;
	WFIFOL(fd, 6) = p->agi;
	WFIFOL(fd, 10) = p->agi2;
	WFIFOSET(fd, packet_len_table[0x141]);

	WFIFOW(fd, 0) = 0x141;
	WFIFOL(fd, 2) = SP_VIT;
	WFIFOL(fd, 6) = p->vit;
	WFIFOL(fd, 10) = p->vit2;
	WFIFOSET(fd, packet_len_table[0x141]);

	WFIFOW(fd, 0) = 0x141;
	WFIFOL(fd, 2) = SP_INT;
	WFIFOL(fd, 6) = p->int_;
	WFIFOL(fd, 10) = p->int_2;
	WFIFOSET(fd, packet_len_table[0x141]);

	WFIFOW(fd, 0) = 0x141;
	WFIFOL(fd, 2) = SP_DEX;
	WFIFOL(fd, 6) = p->dex;
	WFIFOL(fd, 10) = p->dex2;
	WFIFOSET(fd, packet_len_table[0x141]);

	WFIFOW(fd, 0) = 0x141;
	WFIFOL(fd, 2) = SP_LUK;
	WFIFOL(fd, 6) = p->luk;
	WFIFOL(fd, 10) = p->luk2;
	WFIFOSET(fd, packet_len_table[0x141]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x29;
	WFIFOL(fd, 4) = p->atk1;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x2a;
	WFIFOL(fd, 4) = p->atk2-1;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x2b;
	WFIFOL(fd, 4) = p->matk1;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x2c;
	WFIFOL(fd, 4) = p->matk2;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x2d;
	WFIFOL(fd, 4) = p->def2;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x2e;
	WFIFOL(fd, 4) = p->def1;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x2f;
	WFIFOL(fd, 4) = p->mdef2;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x30;
	WFIFOL(fd, 4) = p->mdef1;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x31;
	WFIFOL(fd, 4) = p->hit;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x32;
	WFIFOL(fd, 4) = p->flee1;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x33;
	WFIFOL(fd, 4) = p->flee2;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x34;
	WFIFOL(fd, 4) = p->critical;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x35;
	WFIFOL(fd, 4) = p->aspd;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x00;
	WFIFOL(fd, 4) = sd->speed;
	WFIFOSET(fd, packet_len_table[0xb0]);
	return 0;
}

void mmo_map_skill_lv_up(struct map_session_data *sd, int skill_id)
{
	unsigned int fd = sd->fd;
	if ((sd->status.skill[skill_id-1].lv + 1) > (unsigned)skill_db[skill_id].type_lv)
		return;

	sd->status.skill[skill_id-1].lv++;
	sd->status.skill_point--;

	WFIFOW(fd, 0) = 0x10e;
	WFIFOW(fd, 2) = skill_id;
	WFIFOW(fd, 4) = sd->status.skill[skill_id-1].lv;
	WFIFOW(fd, 6) = skill_db[skill_id].sp;
	WFIFOW(fd, 8) = skill_db[skill_id].range;
	WFIFOB(fd, 10) = 1;
	WFIFOSET(fd, packet_len_table[0x10e]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x0c;
	WFIFOL(fd, 4) = sd->status.skill_point;
	WFIFOSET(fd, packet_len_table[0xb0]);

	mmo_map_send_skills(fd, 1);
	mmo_map_calc_status(fd, 0);
}

int mmo_map_job_lv_up(unsigned int fd, int val)
{
	unsigned char buf[256];
	struct map_session_data	*sd = NULL;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	if (sd->status.class != 0) {
		if ((sd->status.job_level + val) > job_max)
			val = job_max - sd->status.job_level;
	}
	else {
		if ((sd->status.job_level + val) > 10)
			val = 10 - sd->status.job_level;
	}
	if (val == 0)
		return 1;

	sd->status.job_level += val;
	sd->status.skill_point += val;

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x37;
	WFIFOL(fd, 4) = sd->status.job_level;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x0c;
	WFIFOL(fd, 4) = sd->status.skill_point;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WBUFW(buf, 0) = 0x19b;
	WBUFL(buf, 2) = sd->account_id;
	WBUFL(buf, 6) = 1;
	mmo_map_sendarea(fd, buf, packet_len_table[0x19b], 0);

	mmo_map_calc_status(fd, 0);
	return 0;
}

void mmo_map_status_up(struct map_session_data *sd, int type)
{
	int len, fd = sd->fd;
	int need;
	int succeed = 1;
	short *stat;

	switch (type) {
	case SP_STR: stat = &sd->status.str; break;
	case SP_AGI: stat = &sd->status.agi; break;
	case SP_VIT: stat = &sd->status.vit; break;
	case SP_INT: stat = &sd->status.int_; break;
	case SP_DEX: stat = &sd->status.dex; break;
	case SP_LUK: stat = &sd->status.luk; break;
	default: succeed = 0; stat = &sd->status.str; break;
	}
	if (succeed) {
  		need = (*stat + 9) / 10 + 1;
		if ((sd->status.status_point >= need) || (need > 0)) {
			sd->status.status_point -= need;
			WFIFOW(fd, 0) = 0xb0;
			WFIFOW(fd, 2) = 9;
			WFIFOL(fd, 4) = sd->status.status_point;
			WFIFOSET(fd, packet_len_table[0xb0]);

			(*stat)++;
			WFIFOW(fd, 0) = 0x141;
			WFIFOL(fd, 2) = type;
			WFIFOL(fd, 6) = *stat;
			WFIFOL(fd, 10) = 0;
			WFIFOSET(fd, packet_len_table[0x141]);
		}
		else
			succeed = 0;

	}
  	WFIFOW(fd, 0) = 0xbc;
	WFIFOW(fd, 2) = type;
	WFIFOB(fd, 4) = *stat;
	WFIFOB(fd, 5) = succeed;
	WFIFOSET(fd, packet_len_table[0xbc]);

	len = mmo_map_set00bd(WFIFOP(fd, 0), sd);
	if (len > 0)
		WFIFOSET(fd, len);

	mmo_map_calc_status(fd, 0);
}

int mmo_map_level_up(unsigned int fd, int val)
{
	int i;
	int party_index;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	if ((sd->status.base_level + val) > base_max)
		val = base_max - sd->status.base_level;

	if (!val)
		return -1;

	for(i = 0; i < val; i++) {
		sd->status.base_level++;
		sd->status.status_point += sd->status.base_level / 5 + 3;
	}
	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 9;
	WFIFOL(fd, 4) = sd->status.status_point;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd,0) = 0xb0;
	WFIFOW(fd,2) = 0x0b;
	WFIFOL(fd,4) = sd->status.base_level;
	WFIFOSET(fd, packet_len_table[0xb0]);

	mmo_map_calc_status(fd, 0);
	sd->status.hp = sd->status.max_hp;
	sd->status.sp = sd->status.max_sp;

	if (sd->status.party_status > 0) {
		party_index = party_exists(sd->status.party_id);
		if (party_index >= 0 && party_index < MAX_PARTYS) {
			if (sd->status.party_status == 2)
				party_dat[party_index].leader_level = sd->status.base_level;

			if (party_dat[party_index].exp == 1 && check_party_share(party_index) == 0) {
				party_dat[party_index].exp = 0;
				send_party_setup_all(party_index);
				mmo_party_save(party_index);
			}
		}
	}
	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x05;
	WFIFOL(fd, 4) = sd->status.hp;
	WFIFOSET(fd, packet_len_table[0xb0]);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x07;
	WFIFOL(fd, 4) = sd->status.sp;
	WFIFOSET(fd, packet_len_table[0xb0]);
	return 0;
}

int mmo_send_selfdata(struct map_session_data *sd)
{
	int len, i;
	unsigned int fd = sd->fd;
	struct item *n_item;

	sd->max_weight = 100000;
	sd->weight = 0;
	sd->status.damage_atk = 0;
	for (i = 0; i < MAX_INVENTORY; i++) {
		n_item = &sd->status.inventory[i];
		if (n_item->nameid == 0)
			continue;

		WFIFOW(fd, 0) = 0x13a;
		WFIFOW(fd, 2) = sd->status.range;
		WFIFOSET(fd, packet_len_table[0x13a]);
		sd->weight += itemdb_weight(n_item->nameid) * n_item->amount;
	}
	len = mmo_map_all_nonequip(fd, WFIFOP(fd, 0));
	if (len > 0)
		WFIFOSET(fd, len);

	len = mmo_map_all_equip(fd, WFIFOP(fd, 0));
	if (len > 0)
		WFIFOSET(fd,len);

	len = mmo_map_set00bd(WFIFOP(fd, 0), sd);
	if (len > 0)
		WFIFOSET(fd, len);

	len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_MAXWEIGHT);
	if (len > 0)
		WFIFOSET(fd, len);

	len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_WEIGHT);
	if (len > 0)
		WFIFOSET(fd, len);

	len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_ASPD);
	if (len > 0)
		WFIFOSET(fd, len);

	mmo_map_send_skills(fd, 1);
	mmo_map_set00b1(fd, 0x01, sd->status.base_exp);
	mmo_map_set00b1(fd, 0x16, ExpData[sd->status.base_level]);
	mmo_map_set00b1(fd, 0x02, sd->status.job_exp);
	switch (sd->status.class) {
		case 0:
			mmo_map_set00b1(fd, 0x17, SkillExpData[0][sd->status.job_level]);
			break;
		case 1: case 2: case 3: case 4: case 5:
			mmo_map_set00b1(fd, 0x17, SkillExpData[1][sd->status.job_level]);
			break;
		case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 14: case 15:
		case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
			mmo_map_set00b1(fd, 0x17, SkillExpData[2][sd->status.job_level]);
			break;
	}
	if (sd->status.special == 0x08 || sd->status.special == 0x80
	|| sd->status.special == 0x100 || sd->status.special == 0x200
	|| sd->status.special == 0x400)
		mmo_cart_send_items(fd);

	return 0;
}

/*======================================
 *	ZONE: Field Calculations
 *--------------------------------------
 */

int in_range(short x1, short y1, short x2, short y2, short range)
{
	short x_distance = 0;
	short y_distance = 0;

	if (x1 >= x2)
		x_distance = x1 - x2;

	else if (x2 > x1)
		x_distance = x2 - x1;

	if (y1 >= y2)
		y_distance = y1 - y2;

	else if (y2 > y1)
		y_distance = y2 - y1;

	if ((x_distance <= range) && (y_distance <= range))
		return 1;

	return 0;
}

int calc_dir2(short srcx, short srcy, short x, short y)
{
	short dx = x - srcx;
	short dy = y - srcy;

	if (dx == 0 && dy > 0)
		return 0;

	else if (dx == 0 && dy < 0)
		return 0;

	else if (dx < 0 && dy > 0)
		return 1;

	else if (dx < 0 && dy == 0)
		return 2;

	else if (dx < 0 && dy < 0)
		return 3;

	else if (dx > 0 && dy > 0)
		return 3;

	else if (dx > 0 && dy == 0)
		return 2;

	else if (dx > 0 && dy < 0)
		return 1;

	return -1;
}

int calc_dir3(short srcx, short srcy, short x, short y)
{
	short dx = x - srcx;
	short dy = y - srcy;

	if (dx == 0 && dy > 0)
		return 0;

	else if (dx == 0 && dy < 0)
		return 4;

	else if (dx < 0 && dy > 0)
		return 1;

	else if (dx < 0 && dy == 0)
		return 2;

	else if (dx < 0 && dy < 0)
		return 3;

	else if (dx > 0 && dy > 0)
		return 7;

	else if (dx > 0 && dy == 0)
		return 6;

	else if (dx > 0 && dy < 0)
		return 5;

	return -1;
}

int calc_distance(short x0, short y0, short x1, short y1)
{
	short xd = x1 - x0;
	short yd = y1 - y0;

	if (xd < 0)
		xd = -xd;

	if (yd < 0)
		yd = -yd;

	return (xd + yd);
}

int add_path(struct mmo_tmp_path *path, int *wp, short x, short y, short dist, char dir, short before, short x1, short y1)
{
	int i;

	for (i = 0; i < *wp; i++) {
		if (path[i].x == x && path[i].y == y) {
			if (path[i].dist > dist) {
				path[i].dist = dist;
				path[i].dir = dir;
				path[i].before = before;
				path[i].cost = calc_distance(x, x1, y, y1) * 10 + dist;
				path[i].flag = 0;
			}
			return 0;
		}
	}
	path[*wp].x = x;
	path[*wp].y = y;
	path[*wp].dist = dist;
	path[*wp].dir = dir;
	path[*wp].before = before;
	path[*wp].cost = calc_distance(x, x1, y, y1) * 10 + dist;
	path[*wp].flag = 0;
	*wp = *wp + 1;
	return 0;
}

int can_move(struct mmo_map_data *m, short x0, short y0, short x1, short y1)
{
	int c;

	if (x1 < 0 || y1 < 0 || x1 >= m->xs || y1 >= m->ys)
		return 0;

	if (x0 - x1 < -1 || x0 - x1 > 1 || y0 - y1 < -1 || y0 - y1 > 1)
		return 0;

	if ((c = m->gat[x0 + y0 * m->xs]) == 1 || c == 5)
		return 0;

	if ((c = m->gat[x1 + y1 * m->xs]) == 1 || c == 5)
		return 0;

	return 1;
}

int search_path(struct map_session_data *sd, short mapno, short x0, short y0, short x1, short y1, char easy_only)
{
	int rp, wp, i;
	int len, j;
	short x, y, dx, dy;
	static int dirx[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	static int diry[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	struct mmo_tmp_path tmp_path[MAX_TMP_PATH];
	struct mmo_map_data *m;

	if (!map[mapno][0])
		return -1;

	if (x0 < 0 || x0 >= map_data[mapno].xs)
		return -1;

	if (x1 < 0 || x1 >= map_data[mapno].xs)
		return -1;

	if (y0 < 0 || y0 >= map_data[mapno].ys)
		return -1;

	if (y1 < 0 || y1 >= map_data[mapno].ys)
		return -1;

	if (x0 == x1 && y0 == y1)
		return -1;

	if ((i = map_data[mapno].gat[x1 + y1 * map_data[mapno].xs]) == 1 || i == 5)
		return -1;

	m = &map_data[mapno];
	dx = (x1 - x0 < 0) ? -1 : 1;
	dy = (y1 - y0 < 0) ? -1 : 1;
	for (x = x0, y = y0, i = 0; (x != x1 || y != y1) && (i < 64); ) {
		if (x != x1 && y != y1 && can_move(m, x, y, x + dx, y + dy)) {
				x += dx;
				y += dy;
				sd->walkpath[i++] = (dx < 0) ? ((dy > 0) ? 1 : 3) : ((dy < 0) ? 5 : 7);
		}
		else if (x != x1 && can_move(m, x, y, x + dx, y)) {
				x += dx;
				sd->walkpath[i++] = (dx < 0) ? 2 : 6;
		}
		else if (y != y1 && can_move(m, x, y, x, y + dy)) {
				y += dy;
				sd->walkpath[i++] = (dy > 0) ? 0 : 4;
		}
		else {
			len = 6;
			if (x != x1) {
				for (len = 0; len < 6; len++) {
					if (can_move(m, x, y - dy * (len + 1), x + dx, y - dy * (len + 1))) {
						short dx1, dy1;
						while (i > 0 && len > 0 && (dy1 = diry[(int)sd->walkpath[i - 1]]) != -dy) {
							dx1 = dirx[(int)sd->walkpath[i - 1]];
							dy1 = dy1 - dy;
							y -= dy;
							len = len - 1;
							if (dx1 == 0 && dy1 == 0)
								i--;

							else if (dy1 == 0)
								sd->walkpath[i - 1] = (dx1 < 0) ? 2 : 6;

							else if (dx1 == 0)
								sd->walkpath[i - 1] = (dy1 > 0) ? 0 : 4;

							else
								sd->walkpath[i - 1] = (dx1 < 0) ? ((dy1 > 0) ? 1 : 3) : ((dy1 < 0) ? 5 : 7);
						}
						for (j = 0; j < len; j++) {
							y -= dy;
							sd->walkpath[i++] = (-dy > 0) ? 0 : 4;
						}
						x += dx;
						y -= dy;
						sd->walkpath[i++] = (dx < 0) ? ((-dy > 0) ? 1 : 3) : ((-dy < 0) ? 5 : 7);
						break;
					}
					else if (!can_move(m, x, y - dy * len, x, y - dy * (len + 1)) && i < 64 - (len + 2)) {
						len = 6;
						break;
					}
				}
			}
			if (len == 6 && y != y1) {
				for (len = 0; len < 6; len++) {
					if (can_move(m, x - dx * (len + 1), y, x - dx * (len + 1), y + dy)) {
						short dx1, dy1;
						if (i > 0 && len > 0 && (dx1 = dirx[(int)sd->walkpath[i - 1]]) != -dx) {
							dy1 = diry[(int)sd->walkpath[i - 1]];
							dx1 = dx1 - dx;
							x -= dx;
							len = len - 1;
							if (dx1 == 0 && dy1 == 0)
								i--;

							else if (dy1 == 0)
								sd->walkpath[i - 1] = (dx1 < 0) ? 2 : 6;

							else if (dx1 == 0)
								sd->walkpath[i - 1] = (dy1 > 0) ? 0 : 4;

							else
								sd->walkpath[i - 1] = (dx1 < 0) ? ((dy1 > 0) ? 1 : 3) : ((dy1 < 0) ? 5 : 7);
						}
						for (j = 0; j < len; j++) {
							x -= dx;
							sd->walkpath[i++] = (-dx < 0) ? 2 : 6;
						}
						x -= dx;
						y += dy;
						sd->walkpath[i++] = (-dx < 0) ? ((dy > 0) ? 1 : 3) : ((dy < 0) ? 5 : 7);
						break;
					}
					else if (!can_move(m, x - dx * len, y, x - dx * (len + 1), y) && i < 64 - (len + 2)) {
						len = 6;
						break;
					}
				}
			}
			if (len == 6)
				break;

		}
		if (x == x1 && y == y1) {
			sd->walkpath_len = i;
			sd->walkpath_pos = 0;
			return 0;
		}
	}
	if (easy_only)
		return -1;

	wp = 0;
	tmp_path[wp].x = x0;
	tmp_path[wp].y = y0;
	tmp_path[wp].dist = 0;
	tmp_path[wp].dir = 0;
	tmp_path[wp].before = 0;
	tmp_path[wp].cost = calc_distance(x0, x1, y0, y1) * 10;
	tmp_path[wp++].flag = 0;
	while (1) {
		int min_cost, min_pos, max_dist;
		for (i = 0, min_cost = 65536, max_dist = -1, min_pos = -1; i < wp; i++) {
			if (tmp_path[i].flag == 0 && (tmp_path[i].cost < min_cost ||
			   (tmp_path[i].cost == min_cost && tmp_path[i].dist > max_dist))) {
				min_pos = i;
				min_cost = tmp_path[i].cost;
				max_dist = tmp_path[i].dist;
			}
		}
		if (min_pos < 0)
			return -1;

		rp = min_pos;
		x = tmp_path[rp].x;
		y = tmp_path[rp].y;
		if (x == x1 && y == y1) {
			for (len = 0, i = rp; len < 64 && i != 0; i = tmp_path[i].before, len++) ;
			if (len == 64)
				return -1;

			sd->walkpath_len = len;
			sd->walkpath_pos = 0;
			for (i = rp, j = len - 1; i != 0; i = tmp_path[i].before, j--) {
				sd->walkpath[j] = tmp_path[i].dir;
			}
			return 0;
		}
		if (can_move(m, x, y, x + 1, y - 1))
			add_path(tmp_path, &wp, x + 1, y - 1, tmp_path[rp].dist + 14, 5, rp, x1, y1);

		if (can_move(m, x, y, x + 1, y))
			add_path(tmp_path, &wp, x + 1, y, tmp_path[rp].dist + 10, 6, rp, x1, y1);

		if (can_move(m, x, y, x + 1, y + 1))
			add_path(tmp_path, &wp, x + 1, y + 1, tmp_path[rp].dist + 14, 7, rp, x1, y1);

		if (can_move(m, x, y, x, y + 1))
			add_path(tmp_path, &wp, x, y + 1, tmp_path[rp].dist + 10, 0, rp, x1, y1);

		if (can_move(m, x, y, x - 1, y + 1))
			add_path(tmp_path, &wp, x - 1, y + 1, tmp_path[rp].dist + 14, 1, rp, x1, y1);

		if (can_move(m, x, y, x - 1, y))
			add_path(tmp_path, &wp, x - 1, y, tmp_path[rp].dist + 10, 2, rp, x1, y1);

		if (can_move(m, x, y, x - 1, y - 1))
			add_path(tmp_path, &wp, x - 1, y - 1, tmp_path[rp].dist + 14, 3, rp, x1, y1);

		if (can_move(m, x, y, x , y- 1))
			add_path(tmp_path, &wp, x, y - 1, tmp_path[rp].dist + 10, 4, rp, x1, y1);

		if (wp >= MAX_TMP_PATH - 8)
			return -1;

		tmp_path[rp].flag = 1;
	}
	return -1;
}

int calc_next_walk_step(struct map_session_data *sd)
{
	if (sd->walkpath_pos >= sd->walkpath_len)
		return 0;

	if (sd->walkpath[sd->walkpath_pos] & 1)
		return (int)(sd->speed * 14 / 10);

	return sd->speed;
}

int walk_char(int tid, unsigned int tick, int id, int data)
{
	int i;
	short m, nx, ny, dx, dy;
	static int dirx[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	static int diry[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	unsigned char buf[256];
	struct map_session_data *sd;

	if (!session[id] || !(sd = session[id]->session_data))
		return 0;

	if (sd->walktimer != tid)
		return 0;

  	if (sd->walkpath_pos >= sd->walkpath_len || sd->walkpath_pos != data) {
    		sd->walktimer = 0;
		return 0;
	}
 	dx = (short)dirx[(int)sd->walkpath[sd->walkpath_pos]];
  	dy = (short)diry[(int)sd->walkpath[sd->walkpath_pos]];
	sd->dir = sd->head_dir = sd->walkpath[sd->walkpath_pos];
  	nx = sd->x + dx;
  	ny = sd->y + dy;
   	if(nx/BLOCK_SIZE != sd->x/BLOCK_SIZE || ny/BLOCK_SIZE != sd->y/BLOCK_SIZE){
    		int bx,by;
    		bx=sd->x/BLOCK_SIZE;
    		by=sd->y/BLOCK_SIZE;
    		WBUFW(buf,0)=0x80;
    		WBUFL(buf,2)=sd->account_id;
    		WBUFB(buf,6)=0;
    		if(nx/BLOCK_SIZE != sd->x/BLOCK_SIZE && bx-dx*AREA_SIZE>=0 && bx-dx*AREA_SIZE<map_data[sd->mapno].bxs){
      			for(i=-AREA_SIZE;i<=AREA_SIZE;i++){
				if(by+i < 0 || by+i >= map_data[sd->mapno].bys)
	  				continue;
				mmo_map_sendblock(sd->mapno,bx-dx*AREA_SIZE,by+i,buf,packet_len_table[0x80],id,1);
				mmo_map_clrblockchar(id,sd->mapno,bx-dx*AREA_SIZE,by+i);
      			}
    		}
    		if(ny/BLOCK_SIZE != sd->y/BLOCK_SIZE && by-dy*AREA_SIZE>=0 && by-dy*AREA_SIZE<map_data[sd->mapno].bys){
      			for(i=-AREA_SIZE;i<=AREA_SIZE;i++){
				if(bx+i < 0 || bx+i >= map_data[sd->mapno].bxs)
	  				continue;
				mmo_map_sendblock(sd->mapno,bx+i,by-dy*AREA_SIZE,buf,packet_len_table[0x80],id,1);
				mmo_map_clrblockchar(id,sd->mapno,bx+i,by-dy*AREA_SIZE);
     			}
    		}
    		del_block(&sd->block);
    		add_block(&sd->block,sd->mapno,nx,ny);
    		mmo_map_set007b(sd,buf);
    		bx=nx/BLOCK_SIZE;
    		by=ny/BLOCK_SIZE;
    		if(nx/BLOCK_SIZE != sd->x/BLOCK_SIZE && bx+dx*AREA_SIZE>=0 && bx+dx*AREA_SIZE<map_data[sd->mapno].bxs){
      			for(i=-AREA_SIZE;i<=AREA_SIZE;i++){
				if(by+i < 0 || by+i >= map_data[sd->mapno].bys)
	  				continue;
				mmo_map_sendblock(sd->mapno,bx+dx*AREA_SIZE,by+i,buf,packet_len_table[0x7b],id,1);
				mmo_map_getblockchar(id,sd->mapno,bx+dx*AREA_SIZE,by+i);
      			}
   		}
    		if(ny/BLOCK_SIZE != sd->y/BLOCK_SIZE && by+dy*AREA_SIZE>=0 && by+dy*AREA_SIZE<map_data[sd->mapno].bys){
      			for(i=-AREA_SIZE;i<=AREA_SIZE;i++){
				if(bx+i < 0 || bx+i >= map_data[sd->mapno].bxs)
	  				continue;
				mmo_map_sendblock(sd->mapno,bx+i,by+dy*AREA_SIZE,buf,packet_len_table[0x7b],id,1);
				mmo_map_getblockchar(id,sd->mapno,bx+i,by+dy*AREA_SIZE);
      			}
    		}
  	}
  	if (sd->status.pet.activity)
		mmo_map_pet007b(sd, buf, tick);

	sd->x += dx;
  	sd->y += dy;
	m = sd->mapno;
	if (map_data[m].gat[sd->x + sd->y * map_data[m].xs] & 0x80) {
		for (i = 0; i < map_data[m].npc_num; i++) {
			if (map_data[m].npc[i]->block.subtype == WARP) {
				dx = map_data[m].npc[i]->u.warp.x_range;
				dy = map_data[m].npc[i]->u.warp.y_range;
				if (sd->x >= map_data[m].npc[i]->x - dx && sd->x <= map_data[m].npc[i]->x + dx &&
				    sd->y >= map_data[m].npc[i]->y - dy && sd->y <= map_data[m].npc[i]->y + dy) {
					sd->walkpath_len = 0;
					sd->walktimer = 0;
					mmo_map_changemap(sd, map_data[m].npc[i]->u.warp.name, map_data[m].npc[i]->u.warp.to_x, map_data[m].npc[i]->u.warp.to_y, 0);
					return 0;
				}
			}
		}
	}
	int floorskill_index = search_floorskill_index(m, sd->x, sd->y);
	if (floorskill_index > -1) {
		switch (floorskill[floorskill_index].type) {
			case FS_QUAG:
				if (sd->skill_timeamount[92-1][0] == 0) {
					sd->skill_timeamount[92-1][0] = add_timer(timer_data[floorskill[floorskill_index].timer]->tick, skill_reset_stats, id, 92);
					mmo_map_calc_status(id, 0);
				}
				break;
			case FS_WARP:
				if (floorskill[floorskill_index].count > 0) {
					if (floorskill[floorskill_index].owner_id == sd->account_id)
						floorskill[floorskill_index].count = 0;

					else
						floorskill[floorskill_index].count--;

					sd->walkpath_len = 0;
					sd->walktimer = 0;
					mmo_map_changemap(sd, floorskill[floorskill_index].mapname, floorskill[floorskill_index].to_x, floorskill[floorskill_index].to_y, 2);
					if (floorskill[floorskill_index].count < 1) {
						if (floorskill[floorskill_index].timer > 0)
							delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

						remove_floorskill(0, tick, sd->fd, floorskill_index);
					}
					return 0;
				}
				break;
		}
	}
  	sd->walkpath_pos++;
  	if (sd->walkpath_pos >= sd->walkpath_len) {
    		sd->walkpath_pos = 0;
		sd->walkpath_len = 0;
		sd->walktimer = 0;
		return 0;
	}
	if ((i = calc_next_walk_step(sd)) > 0)
		sd->walktimer = add_timer(tick + i , walk_char, id, sd->walkpath_pos);

	return 0;
}

/*======================================
 *	ZONE: Parse Functions
 *--------------------------------------
 */

int parse_tochar(unsigned int fd)
{
	int i, j, a, k, fdc;
#ifdef _MMS_
	unsigned char buf[256];
#endif
	struct map_session_data *sd = NULL, *tmp_sd = NULL;

	if (session[fd]->eof) {
		if (fd == char_fd)
			char_fd = -1;

		close(fd);
		delete_session(fd);
		mmo_online_check();
		return 0;
	}
	while (RFIFOREST(fd) >= 2) {
		switch (RFIFOW(fd, 0)) {
			case 0x2af9:
				if (RFIFOREST(fd) < 3)
					return 0;

				if (RFIFOB(fd, 2))
					exit(1);

				RFIFOSKIP(fd, 3);
				WFIFOW(fd, 0) = 0x2afa;
				for (i = 0; map[i][0]; i++) {
					memcpy(WFIFOP(fd, 4 + i * 16), map[i], 16);
				}
				WFIFOW(fd, 2) = 4 + i * 16;
				WFIFOSET(fd, WFIFOW(fd, 2));
				if (!charcon && (char_fd > 0)) {
					WFIFOW(char_fd, 0) = 0x2b06;
					WFIFOSET(char_fd, 2);
					charcon = 1;
				}
				break;

			case 0x2afb:
				if (RFIFOREST(fd) < 3)
					return 0;

				if (RFIFOB(fd, 2) != 0)
					exit(1);

				RFIFOSKIP(fd,3);
				break;

			case 0x2afd:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				for (i = 5; i < FD_SETSIZE; i++) {
					if (session[i] && (sd = session[i]->session_data) && sd->account_id == (signed)RFIFOL(fd, 4))
						break;
				}
				for (j = i + 1; j < FD_SETSIZE; j++) {
					if (session[j] && (tmp_sd = session[j]->session_data) && tmp_sd->account_id == (signed)RFIFOL(fd, 4))
						break;
				}
				if (j != FD_SETSIZE) {
					if ((i < FD_SETSIZE) && session[i] && session[i]->session_data)
						add_timer(gettick() + 100, wait_close, i, 0);
					if ((j < FD_SETSIZE) && session[j] && session[j]->session_data) {
						WFIFOW(j, 0) = 0x81;
						WFIFOB(j, 2) = 2;
						WFIFOSET(j, packet_len_table[0x81]);
						add_timer(gettick() + 100, wait_close, j, 0);
					}
				}
				else if (i != FD_SETSIZE) {
					fdc = i;
					memcpy(&sd->status, RFIFOP(fd, 12), sizeof(sd->status));
					set_map(sd, sd->status.last_point.map ,sd->status.last_point.x, sd->status.last_point.y);
					if (sd->status.party_id != 0) {
						for (a = 0; a < MAX_PARTY_MEMBERS; a++) {
							if (party_dat[party_exists(sd->status.party_id)].member[a].char_id == sd->char_id)
								break;
						}
						if (a == MAX_PARTY_MEMBERS) {
							sd->status.party_id = -1;
							sd->status.party_status = 0;
							strcpy(sd->status.party_name, "");
						}
					}
					sd->state.remove_new_on_map = 0;
					sd->reset_state = 0;
					sd->speed = DEFAULT_WALK_SPEED - (sd->status.agi + sd->status.agi2) / 5;
					sd->walkpath_len = 0;
					sd->walkpath_pos = 0;
					sd->walktimer = 0;
					sd->sitting = 0;
					sd->dir = 0;
					sd->head_dir = 0;
					sd->endure = 0;
					sd->spheres = 0;
					sd->state.auth = 1;
					sd->server_tick = gettick();
					sd->feed_pet_timer = 0;
					sd->status.pet.pet_id_as_npc = 0;

					for (i = 0; i < MAX_SKILL; i++) {
						if (sd->skill_timeamount[i][0] > 0) {
							delete_timer(sd->skill_timeamount[i][0], skill_reset_stats);
							sd->skill_timeamount[i][0] = 0;
						}
						if (sd->skill_timeamount[i][1] > 0)
							sd->skill_timeamount[i][1] = 0;
					}
					WFIFOW(fdc, 0) = 0x73;
					WFIFOL(fdc, 2) = gettick();
					set_pos(WFIFOP(fdc, 6), sd->x, sd->y);
					WFIFOB(fdc, 9) = 5;
					WFIFOB(fdc, 10) = 5;
					WFIFOSET(fdc, packet_len_table[0x73]);

					WFIFOW(char_fd, 0) = 0x3b45;
					WFIFOSET(char_fd, 2);

					mmo_online_check();
				}
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;

			case 0x2afe:
				if (RFIFOREST(fd) < 7)
					return 0;

				for (i = 5; i < FD_SETSIZE; i++) {
					if (session[i] && (sd = session[i]->session_data) && sd->account_id == (signed)RFIFOL(fd, 4))
						break;
				}
				if (i != FD_SETSIZE) {
					close(i);
					session[i]->eof = 1;
				}
				RFIFOSKIP(fd, 7);
				break;

			case 0x2b00:
				if (RFIFOREST(fd) < 6)
					return 0;

				users_global = RFIFOL(fd, 2);
				RFIFOSKIP(fd, 6);
				break;

			case 0x2b03:
				if (RFIFOREST(fd) < 7)
					return 0;

				for (i = 5; i < FD_SETSIZE; i++) {
					if (session[i] && (sd = session[i]->session_data) && sd->account_id == (signed)RFIFOL(fd, 2))
						break;
				}
				if (i != FD_SETSIZE) {
					WFIFOW(i, 0) = 0xb3;
					WFIFOB(i, 2) = 1;
					WFIFOSET(i, packet_len_table[0xb3]);
				}
				RFIFOSKIP(fd, 7);
				break;

#ifdef _MMS_
			case 0x2c00:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				long ip = RFIFOL(fd, 4);
				short port = RFIFOW(fd, 8);

				for (i = 10; i < RFIFOW(fd, 2); i += 16) {
					for (j = 0; map_to_serv[j].map[0]; j++) {
						if (strcmp(map_to_serv[j].map, RFIFOP(fd, i)) == 0) {
							map_to_serv[j].ip = ip;
							map_to_serv[j].port = port;
							break;
						}
					}
					if (!map_to_serv[j].map[0]) {
						memcpy(map_to_serv[j].map, RFIFOP(fd, i), 16);
						map_to_serv[j].ip = ip;
						map_to_serv[j].port = port;
						if (j++ < MAX_MAP)
							map_to_serv[j].map[0] = 0;

					}
				}
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;

			case 0x2c03:
				if (RFIFOREST(fd) < 28)
					return 0;

				for (i = 5; i < FD_SETSIZE; i++) {
					if (!session[i] || fd == (unsigned)i || !(sd = session[i]->session_data))
						continue;

					if (strncmp(sd->status.name, RFIFOP(fd, 4), 24) == 0) {
						WFIFOW(i, 0) = 0x98;
						WFIFOB(i, 2) = 1;
						WFIFOSET(i, packet_len_table[0x98]);
						break;
					}
				}
				RFIFOSKIP(fd, 28);
				break;

			case 0x2c04:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				for (i = 5; i < FD_SETSIZE; i++) {
					if (!session[i] || fd == (unsigned)i || !(sd = session[i]->session_data))
						continue;

					if (strcmp(sd->status.name, RFIFOP(fd, 4)) == 0) {
						WFIFOW(i, 0) = 0x97;
						WFIFOW(i, 2) = RFIFOW(fd, 2);
						memcpy(WFIFOP(i, 4), RFIFOP(fd, 28), 24);
						memcpy(WFIFOP(i, 28), RFIFOP(fd, 52), RFIFOW(fd, 2) - 52);
						WFIFOSET(i, RFIFOW(fd, 2));
						break;
					}
				}
				if (i == FD_SETSIZE) {
					WFIFOW(fd, 0) = 0x2c05;
					WFIFOW(fd, 2) = 28;
					memcpy(WFIFOP(fd, 4), RFIFOP(fd, 28), 24);
					WFIFOSET(fd, 28);
				}
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;

			case 0x2c07:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				memcpy(WBUFP(buf, 0), RFIFOP(fd, 0), RFIFOW(fd, 2));
				WBUFW(buf, 0) = 0x9a;
				mmo_map_sendall(fd, buf, RFIFOW(fd, 2), 0);
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;
#endif

			case 0x2b07:
				if (RFIFOREST(fd) < 12)
					return 0;

				i = party_exists(RFIFOL(fd, 2));
				a = RFIFOL(fd, 6);
				k = RFIFOW(fd, 10);
				RFIFOSKIP(fd, 12);
				if (i != -1) {
					if (a != -1) {
						for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
							if (party_dat[i].member[j].char_id == a) {
								party_dat[i].member[j].leader = 0;
								party_dat[i].leader_level = k;
								fdc = party_dat[i].member[j].fd;
								if (fdc != 0 && session[fdc] != NULL && session[fdc]->session_data != NULL) {
									sd = session[fdc]->session_data;
									sd->status.party_status = 2;
								}
								if (party_dat[i].exp == 1 && check_party_share(i) == 0) {
									party_dat[i].exp = 0;
									send_party_setup_all(i);
									mmo_party_save(i);
								}
								update_party(a);
								break;
							}
						}
					}
					else {
						for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
							fdc = party_dat[i].member[j].fd;
							if (fdc != 0 && session[fdc] != NULL && session[fdc]->session_data != NULL) {
								sd = session[fdc]->session_data;
								WFIFOW(fdc, 0) = 0x105;
								WFIFOL(fdc, 2) = sd->account_id;
								strcpy(WFIFOP(fdc, 6), sd->status.name);
								WFIFOB(fdc, 30) = 0;
								WFIFOSET(fdc, packet_len_table[0x105]);
								party_update_member_location(i, sd->account_id, sd->char_id, 0, 0, sd->mapno);
								reset_member_data(i, j);
								sd->status.party_status = 0;
								sd->status.party_id = -1;
								strcpy(sd->status.party_name, "");
								mmo_map_send0095(fdc, sd, sd->account_id);
							}
						}
						init_party_data(i);
					}
				}
				break;

			case 0x2dab:
				if (RFIFOREST(fd) < 18)
					return 0;

				i = party_exists(RFIFOL(fd, 10));
				if (i != -1) {
					a = 1;
					for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
						if (party_dat[i].member[j].char_id == (signed)RFIFOL(fd, 6)) {
							a = party_dat[i].member[j].leader;
							reset_member_data(i, j);
							break;
						}
					}
					if (j != MAX_PARTY_MEMBERS) {
						party_dat[i].member_count--;
						if (party_dat[i].member_count < 1)
							delete_party(i);

						else {
							if (a == 0) {
								party_dat[i].leader_level = 0;
								for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
									if (party_dat[i].member[j].char_id > 0) {
										fdc = party_dat[i].member[j].fd;
										if (fdc != 0) {
											if (session[fdc] != NULL && session[fdc]->session_data != NULL) {
												sd = session[fdc]->session_data;
												if (sd->status.skill[0].lv >= 7) {
													sd->status.party_status = 2;
													party_dat[i].member[j].leader = 0;
													party_dat[i].leader_level = sd->status.base_level;
													if (party_dat[i].exp == 1 && check_party_share(i) == 0) {
														party_dat[i].exp = 0;
														send_party_setup_all(i);
													}
													mmo_char_save(sd);
													break;
												}
											}
										}
									}
								}
							}
							update_party(i);
							mmo_party_save(i);
						}
					}
				}
				RFIFOSKIP(fd, 18);
				break;

			case 0x2dac:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				i = RFIFOL(fd, 4);
				party_dat[i].party_id = RFIFOL(fd, 4);
				party_dat[i].member_count = RFIFOB(fd, 8);
				party_dat[i].exp = RFIFOW(fd, 9);
				party_dat[i].item = RFIFOW(fd, 11);
				party_dat[i].leader_level = RFIFOW(fd, 13);
				strcpy(party_dat[i].party_name, RFIFOP(fd, 15));

				for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
					party_dat[i].member[j].account_id = 0;
					party_dat[i].member[j].fd = 0;
					party_dat[i].member[j].x = 0;
					party_dat[i].member[j].y = 0;
					party_dat[i].member[j].hp = 0;
					party_dat[i].member[j].max_hp = 0;
					party_dat[i].member[j].offline = 1;
					party_dat[i].member[j].leader = 1;
					party_dat[i].member[j].char_id = 0;
					party_dat[i].member[j].mapno = 0;
				}
				for (j = 0; j < RFIFOW(fd, 8); j++) {
					if (RFIFOW(fd, 39 + j * 68) == 0) {
						for (a = 0; a < map_number; a++) {
							if (strncmp(RFIFOP(fd, 39 + j * 68 + 28), map[a], 16) == 0)
								break;
						}
					}
					else
						a = RFIFOW(fd, 39 + j * 68);

					if (a < 0 || a >= map_number)
						a = 0;

					party_dat[i].member[j].mapno = a;
					party_dat[i].member[j].leader = RFIFOB(fd, 39 + j * 68 + 2);
					party_dat[i].member[j].account_id = RFIFOL(fd, 39 + j * 68 + 3);
					party_dat[i].member[j].char_id = RFIFOL(fd, 39 + j * 68 + 7);
					party_dat[i].member[j].x = RFIFOW(fd, 39 + j * 68 + 11);
					party_dat[i].member[j].y = RFIFOW(fd, 39 + j * 68 + 13);
					party_dat[i].member[j].hp = RFIFOL(fd, 39 + j * 68 + 15);
					party_dat[i].member[j].max_hp = RFIFOL(fd, 39 + j * 68 + 19);
					party_dat[i].member[j].offline = 1;
					party_dat[i].member[j].fd = 0;
					strncpy(party_dat[i].member[j].map_name, RFIFOP(fd, 39 + j * 68 + 28), 16);
					strncpy(party_dat[i].member[j].nick, RFIFOP(fd, 39 + j * 68 + 44), 24);
				}
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;

			case 0x3ff6:
				if (RFIFOREST(fd) < 4)
					return 0;

				i = RFIFOL(fd, 2);
				pet_dat[i].pet_id = RFIFOL(fd, 2);
				pet_dat[i].pet_class = RFIFOL(fd, 6);
				memcpy(pet_dat[i].pet_name, RFIFOP(fd, 8), 24);
				pet_dat[i].account_id = RFIFOL(fd, 32);
				pet_dat[i].char_id = RFIFOL(fd, 36);
				pet_dat[i].pet_level = RFIFOB(fd, 40);
				pet_dat[i].mons_id = RFIFOW(fd, 41);
				pet_dat[i].pet_accessory = RFIFOW(fd, 43);
				pet_dat[i].pet_friendly = RFIFOW(fd, 45);
				pet_dat[i].pet_hungry = RFIFOW(fd, 47);
				pet_dat[i].pet_name_flag = RFIFOW(fd, 49);
				pet_dat[i].activity = RFIFOB(fd, 51);
				RFIFOSKIP(fd, 52);
				break;

			default:
				close(fd);
				session[fd]->eof = 1;
				return 0;
		}
	}
	return 0;
}

int parse_map(unsigned int fd)
{
	int i, a, x;
	int parsing_packet_len, len, ret;
	unsigned char buf[256];

	int ep;
	int j = 0;
	int j1 = 0;
	int item_num = 0;
	int item_type = 0;
	int item_view = 0;
	int itemindex;
	int itemamount;
	int iItems = 0;
	int iItemCount = 0;
	int skill_lvl;
	int skill_num = 0;

	struct map_session_data *sd, *target_sd;
	struct mmo_chat *temp_chat;
	struct item *n_item;

	if (!session[char_fd])
		session[fd]->eof = 1;

	sd = session[fd]->session_data;
	if (session[fd]->eof) {
		if (fd == char_fd)
			char_fd = -1;

		if (sd && sd->state.auth) {
			mmo_map_cardskills2(fd, sd);
			if (sd->status.hp <= 0) {
				sd->status.hp = 1;
				mmo_map_changemap(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 2);
			}
			if (sd->chatID)
				mmo_map_leavechat(fd, (struct mmo_chat*)sd->chatID, sd->status.name);

			if (sd->status.party_status > 0)
				party_member_logout(sd->status.party_id, sd->account_id, sd->char_id, sd->mapno);

			map_data[sd->mapno].users--;
			mmo_map_checkpvpmap(sd);
			strncpy(sd->status.last_point.map, sd->mapname, 16);
			sd->status.last_point.x = sd->x;
			sd->status.last_point.y = sd->y;
			mmo_char_save(sd);
			del_block(&sd->block);
			WBUFW(buf, 0) = 0x80;
			WBUFL(buf, 2) = sd->account_id;
			WBUFB(buf, 6) = 2;
			mmo_map_sendarea(fd, buf, packet_len_table[0x80], 1);
			if (sd->status.pet.activity) {
				del_block(&map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block);
				set_monster_no_point(sd->mapno, sd->status.pet.pet_npc_id_on_map[sd->mapno]);
				WBUFW(buf, 0) = 0x80;
				WBUFL(buf, 2) = sd->status.pet.pet_id_as_npc;
				WBUFB(buf, 6) = 0;
				mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
				sd->status.pet.pet_hungry = 0;
				mmo_pet_save(sd);
			}
		}
		close(fd);
		delete_session(fd);
		mmo_online_check();
		return 0;
	}
	while (RFIFOREST(fd) >= 2) {
		if (RFIFOW(fd, 0) >= 0x210 || !packet_len_table[RFIFOW(fd, 0)]
		|| ((!sd || (sd && !sd->state.auth)) && RFIFOW(fd, 0) != 0x72)) {
			close(fd);
			session[fd]->eof = 1;
			return 0;
		}
		parsing_packet_len = packet_len_table[RFIFOW(fd, 0)];
		if (parsing_packet_len == -1) {
			if (RFIFOREST(fd) < 4)
				return 0;

			parsing_packet_len = RFIFOW(fd, 2);
		}
		if (RFIFOREST(fd) < parsing_packet_len)
			return 0;

		switch (RFIFOW(fd, 0)) {
			case 0x72: /* Want to Connect */
				if (!sd) {
					sd = session[fd]->session_data = malloc(sizeof(*sd));
					memset(sd, 0, sizeof(*sd));
				}
				WFIFOW(char_fd, 0) = 0x2afc;
				sd->account_id = WFIFOL(char_fd, 2) = RFIFOL(fd, 2);
				sd->char_id = WFIFOL(char_fd, 6) = RFIFOL(fd, 6);
				sd->login_id1 = WFIFOL(char_fd, 10) = RFIFOL(fd, 10);
				sd->client_tick = RFIFOL(fd, 14);
				sd->sex = RFIFOB(fd, 18);
				sd->state.auth = 0;
				sd->fd = fd;
				sd->block.prev = NULL;
				sd->block.next = NULL;
				sd->block.type = BL_PC;
				WFIFOSET(char_fd, 14);
				WFIFOL(fd, 0) = RFIFOL(fd, 2);
				WFIFOSET(fd, 4);
				break;

			case 0x7d: /* Map Loading Complete */
				mmo_send_selfdata(sd);
				sd->attacktimer = 0;
				sd->attacktarget = 0;
				if (sd->skilltimer > 0)
					delete_timer(sd->skilltimer, skill_reset_stats);

				mmo_map_calc_status(fd, 0);
				mmo_map_calc_card(fd, 0, 0, 1);
				mmo_map_calc_overweight(sd);

				len = mmo_map_set_look(buf, sd->account_id, LOOK_WEAPON, sd->status.weapon);
				if (len > 0)
					mmo_map_sendarea(fd, buf, len, 0);

				len = mmo_map_set_look(buf, sd->account_id, LOOK_SHIELD, sd->status.shield);
				if (len > 0)
					mmo_map_sendarea(fd, buf, len, 0);

				mmo_map_getareachar(sd);
				mmo_map_update_status(sd, buf);
				mmo_map_set0079(sd, buf);
				mmo_map_sendarea(fd, buf, packet_len_table[0x79], 1);
				mmo_map_checkpvpmap(sd);

				if (sd->spheres != 0)
					skill_callspirits(fd, sd->spheres);

				sd->new_on_map = 1;
				sd->torihiki_fd = 0;
				sd->hidden = 0;
				sd->auto_blitz = 0;
				sd->casting = 0;
				sd->pvprank = 0;
				sd->no_whispers = 0;
				sd->status.effect = 00000000;
				sd->status.talking_to_npc = 0;
				sd->status.party_invited = 0;
				sd->status.trade_partner = 0;
				sd->status.deal_locked = 0;
				sd->state.remove_new_on_map = add_timer(gettick() + 25000, remove_new_on_map, fd, 0);

				if (sd->status.hp <= 0) {
					WFIFOW(fd, 0) = 0xb0;
					WFIFOW(fd, 2) = 0x05;
					WFIFOL(fd, 4) = sd->status.hp = 1;
					WFIFOSET(fd, packet_len_table[0xb0]);
				}
				for (j = 0; j < 10; j++) {
					sd->status.deal_item[j] = 0;
					sd->status.deal_inventory[j].amount = 0;
					sd->status.deal_inventory[j].nameid = 0;
					sd->status.deal_inventory[j].identify = 0;
					sd->status.deal_inventory[j].attribute = 0;
					sd->status.deal_inventory[j].refine = 0;
					sd->status.deal_inventory[j].card[0] = 0;
					sd->status.deal_inventory[j].card[1] = 0;
					sd->status.deal_inventory[j].card[2] = 0;
					sd->status.deal_inventory[j].card[3] = 0;
					sd->status.deal_item[j] = 0;
				}
				sd->status.deal_zeny = 0;

				if (sd->status.party_status > 0)
					party_member_login(sd->status.party_id, fd);

				pet_init(sd);
				if (sd->status.pet.activity) {
					if (sd->status.pet.pet_npc_id_on_map[sd->mapno] == 0) {
						sd->status.pet.pet_npc_id_on_map[sd->mapno] = map_data[sd->mapno].npc_num;
						map_data[sd->mapno].npc_num++;
					}
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]] = malloc(sizeof(struct npc_data));
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->m = sd->mapno;
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->x = sd->x;
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->y = sd->y;
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->u.mons.speed = 200;
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->dir = 0;
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->class = sd->status.pet.pet_class;
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->id = sd->status.pet.pet_id_as_npc;
					memcpy(map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->name, sd->status.pet.pet_name, 24);

					mmo_map_set_npc(buf, map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->id, map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->class,
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->x, map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->y, 0,
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->u.mons.speed, 0);

					mmo_map_sendarea_mxy(sd->mapno, map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->x, map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->y, buf, packet_len_table[0x78]);

					WBUFW(buf, 0) = 0x1a4;
					WBUFB(buf, 2) = 0;
					WBUFL(buf, 3) = sd->status.pet.pet_id_as_npc;
					if (sd->status.pet.pet_accessory > 0)
						WBUFL(buf, 7) = sd->status.pet.pet_accessory;
					else
						WBUFL(buf, 7) = 0;

					mmo_map_sendarea(fd, buf, packet_len_table[0x1a4], 0);

					WBUFW(buf, 0) = 0x1a4;
					WBUFB(buf, 2) = 5;
					WBUFL(buf, 3) = sd->status.pet.pet_id_as_npc;
					WBUFL(buf, 7) = 14;
					mmo_map_sendarea(fd, buf, packet_len_table[0x1a4], 0);

					WFIFOW(fd, 0) = 0x1a2;
					memcpy(WFIFOP(fd, 2), sd->status.pet.pet_name, 24);
					WFIFOB(fd, 26) = sd->status.pet.pet_name_flag;
					WFIFOW(fd, 27) = sd->status.pet.pet_level;
					WFIFOW(fd, 29) = sd->status.pet.pet_hungry;
					WFIFOW(fd, 31) = sd->status.pet.pet_friendly;
					WFIFOW(fd, 33) = sd->status.pet.pet_accessory;
					WFIFOSET(fd, packet_len_table[0x1a2]);

					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block.next = NULL;
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block.prev = NULL;
					map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block.subtype = BL_PET;
					add_block_npc(sd->mapno, sd->status.pet.pet_npc_id_on_map[sd->mapno]);
				}
				break;

			case 0x7e: /* Tick send */
				sd->client_tick = RFIFOL(fd, 2);
				WFIFOL(fd, 0) = 0x7f;
				sd->server_tick = WFIFOL(fd, 2) = gettick();
				WFIFOSET(fd, packet_len_table[0x7f]);
				break;


			/*** General Cases ***/
			case 0x85: /* Walk to Current Location x, y */
				if (sd->casting || sd->hidden || sd->speed <= 0 || mmo_map_calc_effect(sd)) {
					delete_timer(sd->walktimer, walk_char);
					sd->walktimer = 0;
					memset(buf, 0, packet_len_table[0x88]);
					WBUFW(buf, 0) = 0x88;
					WBUFL(buf, 2) = sd->account_id;
					WBUFW(buf, 6) = sd->x;
					WBUFW(buf, 8) = sd->y;
					mmo_map_sendarea(fd, buf, packet_len_table[0x88], 0);
					break;
				}
				if (sd->walktimer) {
					delete_timer(sd->walktimer, walk_char);
					sd->walktimer = 0;
				}
				if (sd->attacktimer) {
					delete_timer(sd->attacktimer, mmo_map_attack_continue);
					sd->attacktarget = 0;
					sd->attacktimer = 0;
      			}
				sd->status.talking_to_npc = 0;
				sd->to_x = (short)((RFIFOB(fd, 2) * 4) + (RFIFOB(fd, 3) >> 6));
				sd->to_y = (short)(((RFIFOB(fd, 3) & 0x3f) << 4) + (RFIFOB(fd, 4) >> 4));
				if (!search_path(sd, sd->mapno, sd->x, sd->y, sd->to_x, sd->to_y, 0)) {
					if (sd->status.party_status > 0)
						party_update_member_location(party_exists(sd->status.party_id), sd->account_id, sd->char_id, sd->to_x, sd->to_y, sd->mapno);

					WFIFOW(fd, 0) = 0x87;
					WFIFOL(fd, 2) = gettick();
					set_2pos(WFIFOP(fd, 6), sd->x, sd->y, sd->to_x, sd->to_y);
					WFIFOB(fd, 11) = 0;
					WFIFOSET(fd, packet_len_table[0x87]);
					if ((i = calc_next_walk_step(sd)) > 0) {
						i = i >> 2;
						sd->walktimer = add_timer(gettick() + i, walk_char, fd, 0);
					}
					mmo_map_set007b(sd, buf);
					mmo_map_sendarea(fd, buf, packet_len_table[0x7b], 1);
				}
				break;

			case 0x89: /* Attack, Sitting, and Stand Up Request */
				if (sd->walktimer) {
					delete_timer(sd->walktimer, walk_char);
					sd->walktimer = 0;
				}
				if (mmo_map_calc_effect(sd))
					break;

				switch (RFIFOB(fd, 6)) {
					case 0x00: /* Once Attack */
						if (sd->attacktimer > 0) {
							delete_timer(sd->attacktimer, mmo_map_attack_continue);
							sd->attacktimer = 0;
							sd->attacktarget = 0;
						}
						sd->attacktarget = RFIFOL(fd, 2);
						sd->attacktimer = add_timer(gettick() + 10, mmo_map_attack_continue, fd, 1);
						break;

					case 0x02: /* Sit */
						if (sd->status.skill[0].lv < 3) {
							WFIFOW(fd, 0) = 0x110;
							WFIFOW(fd, 2) = 1;
							WFIFOW(fd, 4) = 2;
							WFIFOW(fd, 6) = 0;
							WFIFOB(fd, 8) = 0;
							WFIFOB(fd, 9) = 0;
							WFIFOSET(fd, packet_len_table[0x110]);
						}
						else {
							sd->sitting = 2;
							memset(buf, 0, packet_len_table[0x8a]);
							WBUFW(buf, 0) = 0x8a;
							WBUFL(buf, 2) = sd->account_id;
							WBUFB(buf, 26) = 2;
							mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);

							if (sd->status.skill[260-1].lv > 0) {
								if (sd->weight >= sd->max_weight / 2)
									sd->skill_timeamount[260-1][0] = add_timer(gettick() + 20000, skill_spiritcadence, fd, 0);
								else
									sd->skill_timeamount[260-1][0] = add_timer(gettick() + 10000, skill_spiritcadence, fd, 0);
							}
						}
						break;

					case 0x03: /* Stand Up */
						sd->sitting = 0;
						memset(buf, 0, packet_len_table[0x8a]);
						WBUFW(buf, 0) = 0x8a;
						WBUFL(buf, 2) = sd->account_id;
						WBUFB(buf, 26) = 3;
						mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);

						delete_timer(sd->skill_timeamount[260-1][0], skill_spiritcadence);
						sd->skill_timeamount[260-1][0] = -1;

						break;

					case 0x07: /* CTRL Click */
						if (sd->attacktimer > 0) {
							delete_timer(sd->attacktimer, mmo_map_attack_continue);
							sd->attacktimer = 0;
							sd->attacktarget = 0;
						}
						sd->attacktarget = RFIFOL(fd, 2);
						sd->attacktimer = add_timer(gettick() + 10, mmo_map_attack_continue, fd, 0);
						break;
				}
				break;

			case 0x118:	/* Stop Attack When a Monster is Killed */
				delete_timer(sd->attacktimer, mmo_map_attack_continue);
				sd->attacktarget = 0;
				sd->attacktimer = 0;
				break;

			case 0x8c: /* Console Global Message and Chat */
				if (parse_commands(fd) == 0) {
					WBUFW(buf, 0) = 0x8d;
					WBUFW(buf, 2) = RFIFOW(fd, 2) + 4;
					WBUFL(buf, 4) = sd->account_id;
					memcpy(WBUFP(buf, 8), RFIFOP(fd, 4), RFIFOW(fd, 2) - 4);
					if (sd->chatID)
						mmo_map_sendchat(fd, buf, RFIFOW(fd, 2) + 4, 1);

					else
						mmo_map_sendarea(fd, buf, RFIFOW(fd, 2) + 4, 2);

					memcpy(WFIFOP(fd, 0), RFIFOP(fd, 0), RFIFOW(fd, 2));
					WFIFOW(fd, 0) = 0x8e;
					WFIFOSET(fd, WFIFOW(fd, 2));
				}
				break;

			case 0x9b: /* Change Direction */
				memset(buf, 0, packet_len_table[0x9c]);
				WBUFW(buf, 0) = 0x9c;
				WBUFL(buf, 2) = sd->account_id;
				sd->head_dir = WBUFW(buf, 6) = RFIFOW(fd, 2);
				sd->dir = WBUFB(buf, 8) = RFIFOB(fd, 4);
				mmo_map_sendarea(fd, buf, packet_len_table[0x9c], 1);

				mmo_map_set0078(sd, buf);
				mmo_map_sendarea(fd, buf, packet_len_table[0x78], 1);
				break;

			case 0x94: /* Character Name Request */
				mmo_map_send0095(fd, sd, RFIFOL(fd, 2));
				break;

			case 0x193: /* Guild Name Display*/
				mmo_map_send0095(fd, sd, RFIFOL(fd, 2));
				break;

			case 0xbf: /* Character Use Emotion */
				if (sd->status.skill[0].lv < 2) {
					WFIFOW(fd, 0) = 0x110;
					WFIFOW(fd, 2) = 1;
					WFIFOW(fd, 4) = 1;
					WFIFOW(fd, 6) = 0;
					WFIFOB(fd, 8) = 0;
					WFIFOB(fd, 9) = 0;
					WFIFOSET(fd, packet_len_table[0x110]);
				}
				else {
					memset(buf, 0, packet_len_table[0xc0]);
					WBUFW(buf, 0) = 0xc0;
					WBUFL(buf, 2) = sd->account_id;
					WBUFB(buf, 6) = RFIFOB(fd, 2);
					mmo_map_sendarea(fd, buf, packet_len_table[0xc0], 0);
				}
				break;

			case 0xc1: /* How Many Connected */
				WFIFOW(fd, 0) = 0xc2;
				WFIFOL(fd, 2) = users_global;
				WFIFOSET(fd, packet_len_table[0xc2]);
				break;

			case 0x12a: /* Remove PecoPeco, Cart.. */
				sd->status.special = 0;
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = sd->account_id;
				WBUFW(buf, 6) = 0;
				WBUFW(buf, 8) = 0;
				WBUFW(buf, 10) = 0;
				WBUFB(buf, 12) = 0;
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
				WFIFOW(fd, 0) = 0x12b;
				WFIFOSET(fd, packet_len_table[0x12b]);
				mmo_map_calc_status(fd, 0);
				break;

			case 0x149: /* Manner Point is Recieved */
				WFIFOW(fd, 0) = 0x14a;
				WFIFOB(fd, 2) = 0;
				WFIFOSET(fd, packet_len_table[0x14a]);

				WFIFOW(fd, 0) = 0x14b;
				if (RFIFOB(fd, 6) == 0) {
					WFIFOB(fd, 2) = 0;
					sd->status.manner++;
				}
				else if (RFIFOB(fd, 6) == 1) {
					WFIFOB(fd, 2) = 1;
					sd->status.manner--;
				}
				WFIFOSET(fd, packet_len_table[0x14b]);
				break;

			case 0x112: /* Skill Up */
				mmo_map_skill_lv_up(sd, RFIFOW(fd, 2));
				break;

			case 0xbb: /* Status Up */
				mmo_map_status_up(sd, RFIFOW(fd, 2));
				break;

			case 0xa2: /* Drop Item */
				if (sd->using_card)
					break;

				mmo_map_dropitem(sd, RFIFOW(fd, 2), RFIFOW(fd, 4));
				sd->status.talking_to_npc = 0;
				break;

			case 0x9f: /* Take Item */
				mmo_map_take_item(sd, RFIFOL(fd, 2));
				sd->status.talking_to_npc = 0;
				break;

			case 0x113: /* Designated Target Attack and Skill Place */
				skill_lvl = RFIFOW(fd, 2);
				skill_num = RFIFOW(fd, 4);
				sd->attacktarget = RFIFOL(fd, 6);

				if (sd->skill_timeamount[143-1][1] != 1 && !(sd->casting))
					skill_attack_target(fd, sd->attacktarget, skill_num, skill_lvl, gettick());

				break;

			case 0x116: /* Partner Skill Reaction */
				skill_lvl = RFIFOW(fd, 2);
				skill_num = RFIFOW(fd, 4);

				if ((sd->skill_timeamount[143-1][1] != 1) && !(sd->casting)) {
					int skill_x, skill_y;
					skill_x = RFIFOW(fd, 6);
					skill_y = RFIFOW(fd, 8);
					skill_attack_place(fd, skill_num, skill_lvl, gettick(), skill_x, skill_y);
				}
				break;

			case 0x11b: /* Teleport and Warp Skill */
				if (mmo_map_flagload(sd->mapname, TELEPORT) == 0) {
					WFIFOW(fd, 0) = 0x189;
					WFIFOW(fd, 2) = 0;
					WFIFOSET(fd, packet_len_table[0x189]);
					break;
				}
				else if (RFIFOW(fd, 2) == 26) {
					if (strcmp(RFIFOP(fd, 4), "Random") == 0) {
						WBUFW(buf, 0) = 0x80;
						WBUFL(buf, 2) = sd->account_id;
						WBUFB(buf, 6) = 2;
						mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
						do {
							sd->x = rand() % (map_data[sd->mapno].xs - 2) + 1;
							sd->y = rand() % (map_data[sd->mapno].ys - 2) + 1;
						}
						while ((j = map_data[sd->mapno].gat[sd->x + sd->y * map_data[sd->mapno].xs]) == 1 || j == 5 || (j & 0x80) == 0x80);
						mmo_map_changemap(sd, sd->mapname, sd->x, sd->y, 2);
					}
					else if (strcmp(RFIFOP(fd, 4), sd->status.save_point.map) == 0) {
						WBUFW(buf, 0) = 0x80;
						WBUFL(buf, 2) = sd->account_id;
						WBUFB(buf, 6) = 2;
						mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
						mmo_map_changemap(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 2);
					}
				}
				else {
					j = search_floorskill_index2(sd->account_id, 27, 0);
					while (j != -1 && floorskill[j].skill[1].bl_id != -1) {
						j = search_floorskill_index2(sd->account_id, 27, j + 1);
					}
					if (j > -1) {
						if (strcmp(RFIFOP(fd, 4), sd->status.save_point.map) == 0) {
							strcpy(floorskill[j].mapname, sd->status.save_point.map);
							floorskill[j].to_x = sd->status.save_point.x;
							floorskill[j].to_y = sd->status.save_point.y;
						}
						else if (strcmp(RFIFOP(fd, 4), sd->status.memo_point[0].map) == 0) {
							strcpy(floorskill[j].mapname, sd->status.memo_point[0].map);
							floorskill[j].to_x = sd->status.memo_point[0].x;
							floorskill[j].to_y = sd->status.memo_point[0].y;
						}
						else if (strcmp(RFIFOP(fd, 4), sd->status.memo_point[1].map) == 0) {
							strcpy(floorskill[j].mapname, sd->status.memo_point[1].map);
							floorskill[j].to_x = sd->status.memo_point[1].x;
							floorskill[j].to_y = sd->status.memo_point[1].y;
						}
						else if (strcmp(RFIFOP(fd, 4), sd->status.memo_point[2].map) == 0) {
							strcpy(floorskill[j].mapname, sd->status.memo_point[2].map);
							floorskill[j].to_x = sd->status.memo_point[2].x;
							floorskill[j].to_y = sd->status.memo_point[2].y;
						}
						else {
							if (floorskill[j].timer > 0) {
								delete_timer(floorskill[j].timer, remove_floorskill);
							}
							remove_floorskill(0, gettick(), fd, j);
							break;
						}
						make_floorskill(fd, floorskill[j].skill[0].x, floorskill[j].skill[0].y, floorskill[j].skill_lvl, 0x80, FS_WARP, 27, j);
						timer_data[floorskill[j].timer]->tick = gettick() + floorskill[j].count * 5000;
					}
				}
				break;

			case 0x11d: /* Current Location Memo */
				if (mmo_map_flagload(sd->mapname, MEMO) == 0 || (sd->status.skill[27-1].lv <= 0)) {
					WFIFOW(fd, 0) = 0x11e;
					WFIFOB(fd, 2) = 1;
					WFIFOSET(fd, packet_len_table[0x11e]);
					break;
				}
				else {
					strncpy(sd->status.memo_point[sd->status.last_memo_index].map, sd->mapname, 16);
					sd->status.memo_point[sd->status.last_memo_index].x = sd->x;
					sd->status.memo_point[sd->status.last_memo_index].y = sd->y;

					WFIFOW(fd,0) = 0x11e;
					WFIFOB(fd,2) = 0;
					WFIFOSET(fd, packet_len_table[0x11e]);

					sd->status.last_memo_index++;
					if (sd->status.last_memo_index > 2) {
						sd->status.last_memo_index = 0;
						break;
					}
				}
				break;


			/*** Whisper Code ***/
			case 0x96: /* Whisper */
				mmo_map_sendwis(fd, RFIFOW(fd, 2), RFIFOP(fd, 4), RFIFOP(fd, 28));
				break;

			case 0xcf: /* Deny Whisper From a Player */
				mmo_map_wisuserblock(sd, RFIFOP(fd, 2), RFIFOB(fd, 26));
				break;

			case 0xd0: /* Deny Whispers From Everyone */
				mmo_map_wisblock(sd);
				break;


			/*** GM Commands ***/
			case 0x99: /* GM Broadcast */
				if (sd->account_id > 100000) {
#ifdef _MMS_
					memcpy(WFIFOP(char_fd, 0), RFIFOP(fd, 0), RFIFOW(fd, 2));
					WFIFOW(char_fd, 0) = 0x2c06;
					WFIFOSET(char_fd, RFIFOW(fd, 2));
#endif
					memcpy(WBUFP(buf, 0), RFIFOP(fd, 0), RFIFOW(fd, 2));
					WBUFW(buf, 0) = 0x9a;
					mmo_map_sendall(fd, buf, RFIFOW(fd, 2), 0);
				}
				break;

			case 0x19c: /* GM Local Broadcast */
				if (sd->account_id > 100000) {
					memcpy(WBUFP(buf, 0), RFIFOP(fd, 0), RFIFOW(fd, 2));
					WBUFW(buf, 0) = 0x9a;
					mmo_map_sendarea(fd, buf, RFIFOW(fd, 2), 0);
				}
				break;

			case 0x140: /* GM Map Move Command */
				if (sd->status.account_id > 100000)
					mmo_map_changemap(sd, RFIFOP(fd, 2), RFIFOB(fd, 6), RFIFOB(fd, 7), 2);

				break;

			case 0x1bb: /* GM Shift Command */
				if (sd->status.account_id > 100000) {
					int csds = find_char(RFIFOP(fd, 2));
					if (csds == -1) {
						WFIFOW(fd, 0) = 0x98;
						WFIFOB(fd, 2) = 1;
						WFIFOSET(fd, packet_len_table[0x98]);
						break;
					}
					else {
						struct map_session_data *ccsd = NULL;
						ccsd = session[csds]->session_data;
						mmo_map_changemap(sd, ccsd->mapname, ccsd->x, ccsd->y, 2);
					}
				}
				break;

			case 0x19d: /* GM Hide Command */
				if (sd->status.account_id > 100000) {
					if (!sd->hidden) {
						sd->hidden = 1;
						 sd->status.option_z = 2;
					}
					else {
						sd->hidden = 0;
						sd->status.option_z = 0;
					}
					WBUFW(buf, 0) = 0x119;
					WBUFL(buf, 2) = sd->account_id;
					WBUFW(buf, 6) = 0;
					WBUFW(buf, 8) = 0;
					WBUFW(buf, 10) = sd->status.option_z;
					WBUFB(buf, 12) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
				}
				break;

			case 0x13f: /* GM Monster Command */
				if (sd->account_id > 100000) {
					strtolower(RFIFOP(fd, 2));
					for (i = 0; i < MAX_MONS; i++) {
						if (strncmp(RFIFOP(fd, 2), mons_data[i].name, 24) == 0) {
							spawn_monster(mons_data[i].class, sd->x, sd->y, sd->mapno, fd);
							break;
						}
					}
				}
				break;

			case 0xce: /* GM Kill all Command */
				if (sd->status.account_id > 100000) {
				}
				break;

			case 0x3f: /* GM Item Command */
				if (sd->status.account_id > 100000) {
				}
				break;

			case 0x1bd: /* Summon GM command */
				if (sd->account_id > 100000) {
				}
				break;

			/*** GM Right Click Menu? ***/
			case 0xcc: /* GM Menu: Kill Command */
				if (sd->account_id > 10000) {
				}
				break;


			/*** Create Pub Code ***/
			case 0x00d5: /* Create Chat Room */
				if (sd->status.skill[0].lv < 4) {
					WFIFOW(fd, 0) = 0x110;
					WFIFOW(fd, 2) = 1;
					WFIFOW(fd, 4) = 03;
					WFIFOW(fd, 6) = 0;
					WFIFOB(fd, 8) = 00;
					WFIFOB(fd, 9) = 00;
					WFIFOSET(fd, packet_len_table[0x110]);
				}
				else {
					temp_chat = (struct mmo_chat*)malloc(sizeof(struct mmo_chat));
					memset(temp_chat, 0, sizeof(struct mmo_chat));
					temp_chat->len = RFIFOW(fd, 2);
					temp_chat->limit = RFIFOW(fd, 4);
					temp_chat->pub = RFIFOB(fd, 6);
					memset(temp_chat->pass, 0, 8);
					memcpy(temp_chat->pass, RFIFOP(fd, 7), 8);
					memset(temp_chat->title, 0, 62);
					memcpy(temp_chat->title, RFIFOP(fd, 15), (temp_chat->len - 15));
					temp_chat->ownID = sd->account_id;
					temp_chat->usersfd[0] = fd;
					temp_chat->usersID[0] = temp_chat->ownID;
					temp_chat->chatID = (unsigned long)temp_chat;
					sd->chatID = temp_chat->chatID;
					memcpy(temp_chat->usersName[0], sd->status.name, 24);
					temp_chat->users = 1;
					mmo_map_createchat(temp_chat);

					WFIFOW(fd, 0) = 0x0d6;
					WFIFOB(fd, 2) = 0;
					WFIFOSET(fd, packet_len_table[0xd6]);

					i = mmo_map_set00d7(fd, buf);
					mmo_map_sendarea(fd, buf, i, 1);
				}
				break;

			case 0x00d9: /* Chat Add Member */
				mmo_map_addchat(fd, (struct mmo_chat*)RFIFOL(fd, 2), RFIFOP(fd, 6));
				break;

			case 0x00de: /* Chat Room Status Change */
				if (0)
					break;

				temp_chat = (struct mmo_chat*)sd->chatID;
				temp_chat->len = RFIFOW(fd, 2);
				temp_chat->limit = RFIFOW(fd, 4);
				temp_chat->pub = RFIFOB(fd, 6);
				memset(temp_chat->pass, 0, 8);
				memcpy(temp_chat->pass, RFIFOP(fd, 7), 8);
				memset(temp_chat->title, 0, 62);
				memcpy(temp_chat->title, RFIFOP(fd, 15), (temp_chat->len - 15));

				WBUFW(buf, 0) = 0xdf;
				WBUFW(buf, 2) = strlen(temp_chat->title) + 1 + 17;
				WBUFL(buf, 4) = temp_chat->ownID;
				WBUFL(buf, 8) = temp_chat->chatID;
				WBUFW(buf, 12) = temp_chat->limit;
				WBUFW(buf, 14) = temp_chat->users;
				WBUFB(buf, 16) = temp_chat->pub;
				strcpy(WBUFP(buf, 17), temp_chat->title);
				mmo_map_sendchat(fd, buf, strlen(temp_chat->title) + 1 + 17, 0);

				i = mmo_map_set00d7(fd, buf);
				mmo_map_sendarea(temp_chat->usersfd[0], buf, i, 3);
				break;

			case 0x00e0: /* Change Owner */
				if (0)
					break;

				mmo_map_changeowner(fd, (struct mmo_chat*)sd->chatID, RFIFOP(fd, 6));
				break;

			case 0x00e2: /* Kick From Chat  */
				if (0)
					break;

				mmo_map_leavechat(fd, (struct mmo_chat*)sd->chatID, RFIFOP(fd, 2));
				break;

			case 0x00e3: /* Chat Leave */
				mmo_map_leavechat(fd, (struct mmo_chat*)sd->chatID, sd->status.name);
				break;


			/*** Exit Zone to Character and Restart on Dead Code ***/
			case 0xb2:
				switch (RFIFOB(fd, 2)) {
					case 0x00: /* Restart On Dead */
						if (sd->status.class == 0)
							sd->status.hp = (sd->status.max_hp / 2);

						else
							sd->status.hp = 1;

						sd->sitting = 0;
						WFIFOW(fd, 0) = 0xb0;
						WFIFOW(fd, 2) = 0x05;
						WFIFOL(fd, 4) = sd->status.hp;
						WFIFOSET(fd, packet_len_table[0xb0]);

						WBUFW(buf, 0) = 0x80;
						WBUFL(buf, 2) = sd->account_id;
						WBUFB(buf, 6) = 2;
						mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
						mmo_map_changemap(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 2);
						break;

					case 0x01: /* Quit To Character Select */
						if (sd->attacktimer != 0 || sd->attacktarget != 0 || sd->casting != 0
						|| sd->status.trade_partner != 0 || sd->status.deal_locked != 0
						|| sd->hidden != 0) {
							WFIFOW(fd, 0) = 0x18b;
							WFIFOW(fd, 2) = 1;
							WFIFOSET(fd, packet_len_table[0x18b]);
							sd->reset_state = add_timer(gettick() + 10000, mmo_map_reset_vars, fd, 0);
							break;
						}
						WFIFOW(fd, 0) = 0xb3;
						WFIFOB(fd, 2) = 1;
						WFIFOSET(fd, packet_len_table[0xb3]);

						WFIFOW(char_fd, 0) = 0x2b02;
						WFIFOL(char_fd, 2) = sd->account_id;
						WFIFOL(char_fd, 6) = sd->login_id1;
						WFIFOSET(char_fd, 10);
						break;
				}
				break;

			case 0x18a: /* Quit the game */
				if (sd->attacktimer != 0 || sd->attacktarget != 0 || sd->casting != 0
				|| sd->status.trade_partner != 0 || sd->status.deal_locked != 0
				|| sd->hidden != 0) {
					WFIFOW(fd, 0) = 0x18b;
					WFIFOW(fd, 2) = 1;
					WFIFOSET(fd, packet_len_table[0x18b]);
					sd->reset_state = add_timer(gettick() + 10000, mmo_map_reset_vars, fd, 0);
					break;
				}
				add_timer(gettick() + 5000, wait_close, fd, 0);

				WFIFOW(fd, 0) = 0x18b;
				WFIFOW(fd, 2) = 0;
				WFIFOSET(fd, packet_len_table[0x18b]);
				break;


			/*** NPC Script Code ***/
			case 0x90: /* NPC Clicked */
				if (RFIFOL(fd, 2) > 0)
					npc_click(sd, RFIFOL(fd, 2));

				break;

			case 0xb8: /* NPC Select Menu */
				if (RFIFOL(fd, 2) > 0)
					npc_menu_select(sd, RFIFOB(fd, 6));

				break;

			case 0xb9: /* NPC Next Clicked */
				if (RFIFOL(fd, 2) > 0)
					npc_next_click(sd);

				sd->status.talking_to_npc = 0;
				break;

			case 0x143: /* NPC Amount Input */
				if (RFIFOL(fd, 2) > 0)
					npc_amount_input(sd, RFIFOL(fd, 6));

				break;

			case 0x146: /* NPC Close Clicked */
				sd->status.talking_to_npc = 0;
				break;

			case 0xc5: /* NPC Buy or Sell Selected */
				npc_buysell_selected(sd, RFIFOL(fd, 2), RFIFOB(fd, 6));
				sd->status.talking_to_npc = 0;
				break;

			case 0xc8: /* NPC Buy List Send */
				npc_buy_selected(sd, RFIFOP(fd, 4), (RFIFOW(fd, 2) - 4) / 4);
				sd->status.talking_to_npc = 0;
				break;

			case 0xc9: /* NPC Sell List Send */
				npc_sell_selected(sd, RFIFOP(fd, 4), (RFIFOW(fd, 2) - 4) / 4);
				sd->status.talking_to_npc = 0;
				break;


			/*** Item Usage Code ***/
			case 0xa7: /* Item Use */
				if (sd->status.hp > 0)
					use_item(sd, RFIFOW(fd, 2));

				break;

			case 0x178: /* Item Apraisal */
				if (RFIFOW(fd, 2) > 1) {
					WFIFOW(fd, 0) = 0x179;
					WFIFOW(fd, 2) = RFIFOW(fd, 2);
					WFIFOB(fd, 4) = 0;
					WFIFOSET(fd, packet_len_table[0x179]);
					i = RFIFOW(fd, 2) - 2;
					if (i < MAX_INVENTORY) {
						sd->status.inventory[i].identify = 1;
					}
				}
				break;


			/*** Trade Code ***/
			case 0xe4: /* Request Trade */
				mmo_map_trade_request(sd);
				break;

			case 0xe6: /* Send Deal Accept */
				mmo_map_trade_accept(sd);
				break;

			case 0xe8: /* Send Deal Add Item */
				mmo_map_trade_additem(sd);
				break;

			case 0xeb: /* Send Deal OK*/
				mmo_map_trade_deal(sd);
				break;

			case 0xed: /* Send Current Deal Cancel */
				mmo_map_trade_cancel(sd);
				break;

			case 0xef: /* Send Deal Trade Transfer */
				mmo_map_trade_send(sd);
				break;


			/*** Party Code ***/
			case 0xf9: /* Client Send Organize Party */
				if (sd->status.skill[0].lv < 7) {
					WFIFOW(fd, 0) = 0x110;
					WFIFOW(fd, 2) = 1;
					WFIFOW(fd, 4) = 4;
					WFIFOW(fd, 6) = 0;
					WFIFOB(fd, 8) = 0;
					WFIFOB(fd, 9) = 0;
					WFIFOSET(fd, packet_len_table[0x110]);
				}
				else {
					create_party(fd, RFIFOP(fd, 2));
					mmo_char_save(sd);
				}
				break;

			case 0xfc: /* Client Send Party Join Request */
				ret = party_exists(sd->status.party_id);
				for (x = 0; x < MAX_PARTY_MEMBERS; x++) {
					if (party_dat[ret].member[x].account_id == 0)
						break;

				}
				if (x != MAX_PARTY_MEMBERS) {
					if ((sd->status.party_status == 2) && (sd->status.party_id >= 0)) {
						for (i = 5; i < FD_SETSIZE; i++) {
							if (session[i] && (target_sd = session[i]->session_data)) {
								if (target_sd->account_id == (signed)RFIFOL(fd, 2)) {
									if (target_sd->status.skill[0].lv < 5) {
										WFIFOW(fd, 0) = 0x110;
										WFIFOW(fd, 2) = 1;
										WFIFOW(fd, 4) = 04;
										WFIFOW(fd, 6) = 0;
										WFIFOB(fd, 8) = 00;
										WFIFOB(fd, 9) = 00;
										WFIFOSET(fd, packet_len_table[0x110]);
										break;
									}
									else if (target_sd->status.party_status == 0) {
										WFIFOW(i, 0) = 0xfe;
										WFIFOL(i, 2) = sd->account_id;
										strcpy(WFIFOP(i, 6), sd->status.party_name);
										WFIFOSET(i, packet_len_table[0xfe]);
										target_sd->status.party_invited = sd->status.party_id;
										target_sd->torihiki_fd = fd;
										break;
									}
									else {
										WFIFOW(fd, 0) = 0xfd;
										strcpy(WFIFOP(fd, 2), target_sd->status.name);
										WFIFOW(fd, 26) = 0;
										WFIFOSET(fd, packet_len_table[0xfd]);
										break;
									}
								}
							}
						}
					}
				}
				break;

			case 0xff: /* Client Send Party Join */
				if (sd->status.party_invited >= 0) {
					if (RFIFOL(fd, 6) == 1) {
						ret = party_exists(sd->status.party_invited);
						if (ret >= 0) {
							for (x = 0; x < MAX_PARTY_MEMBERS; x++) {
								if (party_dat[ret].member[x].account_id == 0)
									break;
							}
							if (x != MAX_PARTY_MEMBERS) {
								if (session[sd->torihiki_fd] != NULL) {
									WFIFOW(sd->torihiki_fd, 0) = 0xfd;
									strcpy(WFIFOP(sd->torihiki_fd, 2), sd->status.name);
									WFIFOW(sd->torihiki_fd, 26) = 2;
									WFIFOSET(sd->torihiki_fd, packet_len_table[0xfd]);
								}
								party_make_member(fd, x, ret);
								sd->status.party_id = sd->status.party_invited;
								sd->status.party_status = 1;
								party_dat[ret].member_count++;
								for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
									if (party_dat[ret].member[j].fd != 0) {
										if (session[party_dat[ret].member[j].fd] != NULL) {
											WFIFOW(party_dat[ret].member[j].fd, 0) = 0x104;
											WFIFOL(party_dat[ret].member[j].fd, 2) = sd->account_id;
											WFIFOL(party_dat[ret].member[j].fd, 6) = sd->status.party_id;
											WFIFOW(party_dat[ret].member[j].fd, 10) = sd->x;
											WFIFOW(party_dat[ret].member[j].fd, 12) = sd->y;
											WFIFOW(party_dat[ret].member[j].fd, 14) = 0;
											strcpy(WFIFOP(party_dat[ret].member[j].fd, 15), sd->status.party_name);
											strcpy(WFIFOP(party_dat[ret].member[j].fd, 39), sd->status.name);
											strcpy(WFIFOP(party_dat[ret].member[j].fd, 63), sd->mapname);
											WFIFOSET(party_dat[ret].member[j].fd, packet_len_table[0x104]);
										}
									}
								}
								update_party(ret);
								party_update_member_location(ret, sd->account_id, sd->char_id, sd->x, sd->y, sd->mapno);
								mmo_map_send0095(fd, sd, sd->account_id);
								if (party_dat[ret].exp == 1 && check_party_share(ret) == 0) {
									party_dat[ret].exp = 0;
									send_party_setup_all(ret);
								}
								else
									send_party_setup(fd);

								mmo_party_save(ret);
								mmo_char_save(sd);
							}
							else {
								WFIFOW(fd, 0) = 0x105;
								WFIFOL(fd, 2) = sd->account_id;
								strcpy(WFIFOP(fd, 6), sd->status.name);
								WFIFOB(fd, 30) = 0;
								WFIFOSET(fd, packet_len_table[0x105]);
							}
						}
						else {
							WFIFOW(fd, 0) = 0x105;
							WFIFOL(fd, 2) = sd->account_id;
							strcpy(WFIFOP(fd, 6), sd->status.name);
							WFIFOB(fd, 30) = 0;
							WFIFOSET(fd, packet_len_table[0x105]);
						}
					}
					else {
						if (session[sd->torihiki_fd] != NULL) {
							WFIFOW(sd->torihiki_fd, 0) = 0xfd;
							strcpy(WFIFOP(sd->torihiki_fd, 2), sd->status.name);
							WFIFOW(sd->torihiki_fd, 26) = 1;
							WFIFOSET(sd->torihiki_fd, packet_len_table[0xfd]);
						}
					}
				}
				sd->torihiki_fd = 0;
				sd->status.party_invited = 0;
				break;

			case 0x100: /* Client Send Leave Party */
				leaveparty(fd);
				mmo_char_save(sd);
				break;

			case 0x0102: /* Client Send Change Party Setup */
				if (sd->status.party_status == 2) {
					j = RFIFOW(fd, 2);
					i = RFIFOW(fd, 4);
					ret = party_exists(sd->status.party_id);
					party_dat[ret].item = i;
					if (j == 2)
						j = 0;

					if (j != party_dat[ret].exp) {
						party_dat[ret].exp = j;
						if (party_dat[ret].exp == 1 && check_party_share(ret) == 0) {
							party_dat[ret].exp = 0;
							send_party_setup(fd);
						}
						else {
							send_party_setup_all(ret);
							mmo_party_save(ret);
						}
					}
				}
				break;

			case 0x103: /* Client Send Kick Party */
				if (sd->status.party_status != 2)
					break;

				a = party_exists(sd->status.party_id);
				if (a < 0)
					break;

				for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
					if (party_dat[a].member[i].account_id == (signed)RFIFOL(fd, 2))
						break;
				}
				if (i == MAX_PARTY_MEMBERS)
					break;

				if (session[party_dat[a].member[i].fd] != NULL) {
					leaveparty(party_dat[a].member[i].fd);
					break;
				}
				for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
					if (party_dat[a].member[j].fd != 0) {
						WFIFOW(party_dat[a].member[j].fd, 0) = 0x105;
						WFIFOL(party_dat[a].member[j].fd, 2) = party_dat[a].member[i].account_id;
						strcpy(WFIFOP(party_dat[a].member[j].fd, 6), party_dat[a].member[i].nick);
						WFIFOB(party_dat[a].member[j].fd, 30) = 0;
						WFIFOSET(party_dat[a].member[j].fd, packet_len_table[0x105]);
					}
				}
				party_dat[a].member[i].account_id = 0;
				party_dat[a].member[i].char_id = 0;
				party_dat[a].member[i].x = 0;
				party_dat[a].member[i].y = 0;
				party_dat[a].member[i].hp = 0;
				party_dat[a].member[i].max_hp = 0;
				party_dat[a].member[i].fd = 0;
				party_dat[a].member[i].mapno = -1;
				party_dat[a].member_count--;
				mmo_party_save(a);
				break;

			case 0x108: /* Client Send Party Chat */
				ret = party_exists(sd->status.party_id);
				if (ret < 0) {
					sd->status.party_id = -1;
					sd->status.party_status = 0;
					strcpy(sd->status.party_name, "");
					break;
				}
				if (sd->status.party_status > 0) {
					for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
						if (session[party_dat[ret].member[i].fd] != NULL) {
							if (party_dat[ret].member[i].fd != 0) {
								WFIFOW(party_dat[ret].member[i].fd, 0) = 0x109;
								WFIFOW(party_dat[ret].member[i].fd, 2) = RFIFOW(fd, 2) + 4;
								WFIFOL(party_dat[ret].member[i].fd, 4) = fd;
								strcpy(WFIFOP(party_dat[ret].member[i].fd, 8), RFIFOP(fd, 4));
								WFIFOSET(party_dat[ret].member[i].fd, RFIFOW(fd, 2) + 4);
							}
						}
						else
							party_dat[ret].member[i].fd = 0;

					}
				}
				break;


			/*** Pet Code ***/
			case 0x19f: /* Pet catching */
				mmo_map_pet_catch(fd, sd, RFIFOL(fd, 2));
				break;

			case 0x1a1: /* Pet status menu select*/
				mmp_map_pet_stat_select(fd, sd, RFIFOB(fd, 2));
				break;

			case 0x1a5: /* Pet name modification */
				mmo_map_pet_name(fd, sd);
				break;

			case 0x1a7: /* Pet block handler */
				mmo_map_pet_act(fd, sd, RFIFOW(fd, 2) - 2);
				break;

			case 0x1a9: /* Pet emotion */
				mmo_map_pet_emotion(fd, sd, RFIFOL(fd, 2));
				break;


			/*** Card Equipment Code ***/
			case 0x17a: /* When using card start. */
				card_send_equip(fd, itemdb[search_itemdb_index(sd->status.inventory[RFIFOW(fd,2)-2].nameid)].loc);
				break;

			case 0x17c: /* When using card and equipment selection end. */
				if (RFIFOW(fd, 2) == 65535 && RFIFOW(fd, 4) == 65535) {
					sd->using_card = 0;
					break;
				}
				card_finish_equip(fd, RFIFOW(fd, 2), RFIFOW(fd, 4));
				sd->using_card = 0;
				break;


			/*** Storage Code ***/
			case 0xf3: /* Send Storage Add */
				if (sd->status.storage_status) {
					if (RFIFOW(fd, 2) >= 2 && RFIFOW(fd, 2) < MAX_INVENTORY + 2) {
						if ((RFIFOL(fd, 4) > 0) && ((signed)RFIFOL(fd, 4) <= sd->status.inventory[RFIFOW(fd, 2) - 2].amount))
							mmo_add_storage(sd, RFIFOW(fd, 2), RFIFOL(fd, 4));

					}
				}
				break;

			case 0xf5: /* Send Storage Get */
				if (sd->status.storage_status) {
					if (RFIFOW(fd, 2) >= 1 && RFIFOW(fd, 2) < MAX_STORAGE + 1) {
						if ((RFIFOL(fd, 4) > 0) && ((signed)RFIFOL(fd, 4) <= sd->status.storage[RFIFOW(fd, 2)-1].amount))
							mmo_remove_storage(sd, RFIFOW(fd, 2), RFIFOL(fd, 4));

					}
				}
				break;

			case 0xf7: /* Send Storage Close */
				sd->status.storage_status = 0;
				WFIFOW(fd, 0) = 0xf8;
				WFIFOSET(fd, 2);
				break;


			/*** Cart Code ***/
			case 0x126: /* Insert item in cart */
			{
				int newi;
				int item_weight;
				int itemid = search_itemdb_index(sd->status.inventory[RFIFOW(fd, 2) - 2].nameid);
				if (itemdb[itemid].type == 4 || itemdb[itemid].type == 5) {
					for (i = 0; i < MAX_CART; i++) {
						if (sd->status.cart[i].amount == 0) {
							break;
						}
					}
					if (i == MAX_CART) {
						WFIFOW(fd, 0) = 0x12c;
						WFIFOB(fd, 2) = 1;
						WFIFOSET(fd, packet_len_table[0x12c]);
						break;
					}
				}
				else {
					newi = -1;
					for (i = 0; i < MAX_CART; i++) {
						if (sd->status.cart[i].amount == 0 && newi == -1) {
							newi = i;
						}
						if (sd->status.cart[i].nameid == sd->status.inventory[RFIFOW(fd, 2) - 2].nameid &&
						    sd->status.cart[i].identify == sd->status.inventory[RFIFOW(fd, 2) - 2].identify) {
							break;
						}
					}
					if (i == MAX_CART) {
						if (newi != -1) {
							i = newi;
						}
						else {
							WFIFOW(fd, 0) = 0x12c;
							WFIFOB(fd, 2) = 1;
							WFIFOSET(fd, packet_len_table[0x12c]);
							break;
						}
					}
				}
				item_weight = itemdb[itemid].weight;
				if ((sd->status.cartweight + item_weight * RFIFOL(fd, 4)) > 80000) {
					WFIFOW(fd, 0) = 0x12c;
					WFIFOB(fd, 2) = 0;
					WFIFOSET(fd, packet_len_table[0x12c]);
					break;
				}
				if (RFIFOL(fd, 4) > 0) {
					if (sd->status.cart[i].amount > 0) {
						sd->status.cart[i].amount += RFIFOL(fd, 4);
					}
					else {
						sd->status.cartcount++;
						memcpy(&sd->status.cart[i], &sd->status.inventory[RFIFOW(fd, 2) - 2], sizeof(struct item));
						sd->status.cart[i].amount = RFIFOL(fd, 4);
					}
					len = mmo_map_lose_item(fd, 0, RFIFOW(fd, 2) - 2+2, RFIFOL(fd, 4));
					if (len > 0) {
						if (itemdb[itemid].type == 4 || itemdb[itemid].type == 5) {
							WFIFOW(fd, 0) = 0x122;
							WFIFOW(fd, 2) = 24;
							WFIFOW(fd, 4) = i + 1;
							WFIFOW(fd, 6) = sd->status.cart[i].nameid;
							WFIFOB(fd, 8) = itemdb_type(sd->status.cart[i].nameid);
							WFIFOB(fd, 9) = sd->status.cart[i].identify;
							WFIFOW(fd, 10) = itemdb_equip_point(sd->status.cart[i].nameid, sd);
							WFIFOW(fd, 12) = sd->status.cart[i].equip;
							WFIFOB(fd, 14) = sd->status.cart[i].attribute;
							WFIFOB(fd, 15) = sd->status.cart[i].refine;
							WFIFOW(fd, 16) = sd->status.cart[i].card[0];
							WFIFOW(fd, 18) = sd->status.cart[i].card[1];
							WFIFOW(fd, 20) = sd->status.cart[i].card[2];
							WFIFOW(fd, 22) = sd->status.cart[i].card[3];
							WFIFOSET(fd, 24);
						}
						else {
							WFIFOW(fd, 0) = 0x124;
							WFIFOW(fd, 2) = i + 1;
							WFIFOL(fd, 4) = RFIFOL(fd, 4);
							WFIFOW(fd, 8) = sd->status.cart[i].nameid;
							WFIFOB(fd, 10) = sd->status.cart[i].identify;
							WFIFOB(fd, 11) = sd->status.cart[i].attribute;
							WFIFOB(fd, 12) = sd->status.cart[i].refine;
							WFIFOW(fd, 13) = sd->status.cart[i].card[0];
							WFIFOW(fd, 15) = sd->status.cart[i].card[1];
							WFIFOW(fd, 17) = sd->status.cart[i].card[2];
							WFIFOW(fd, 19) = sd->status.cart[i].card[3];
							WFIFOSET(fd, packet_len_table[0x124]);
						}
						sd->status.cartweight += item_weight * RFIFOL(fd, 4);
						WFIFOW(fd, 0) = 0x121;
						WFIFOW(fd, 2) = sd->status.cartcount;
						WFIFOW(fd, 4) = 100;
						WFIFOL(fd, 6) = sd->status.cartweight;
						WFIFOL(fd, 10) = 80000;
						WFIFOSET(fd, packet_len_table[0x121]);
					}
				}
				break;
			}

			case 0x127: /* Remove item from cart */
			{
				int temp;
				int item_weight = itemdb[search_itemdb_index(sd->status.cart[RFIFOW(fd, 2) - 1].nameid)].weight;
				if (sd->weight + item_weight * (signed)RFIFOL(fd, 4) > sd->max_weight) {
					WFIFOW(fd, 0) = 0x12c;
					WFIFOB(fd, 2) = 0;
					WFIFOSET(fd, packet_len_table[0x12c]);
					break;
				}
				if (RFIFOL(fd, 4) > 0) {
					temp = sd->status.cart[RFIFOW(fd, 2) - 1].amount;
					sd->status.cart[RFIFOW(fd, 2) - 1].amount = RFIFOL(fd, 4);
					len = mmo_map_get_item(fd, &sd->status.cart[RFIFOW(fd, 2) - 1]);
					sd->status.cart[RFIFOW(fd, 2) - 1].amount = temp;
					if (len > 0) {
						len = mmo_cart_item_remove(fd, WFIFOP(fd, 0), RFIFOW(fd, 2) - 1 + 1, RFIFOL(fd, 4));
						if (len > 0) {
							WFIFOSET(fd, len);
						}
					}
					else {
						WFIFOW(fd, 0) = 0xa0;
						WFIFOB(fd, 2) = 6;
						WFIFOSET(fd, packet_len_table[0xa0]);
						break;
					}
				}
				break;
			}

			case 0x1af: /* Change cart */
				switch (RFIFOW(fd, 2)) {
				case 1:
					sd->status.special = 0x08;
					break;
				case 2:
					sd->status.special = 0x80;
					break;
				case 3:
					sd->status.special = 0x100;
					break;
				case 4:
					sd->status.special = 0x200;
					break;
				case 5:
					sd->status.special = 0x400;
					break;
				}
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = sd->account_id;
				WBUFW(buf, 6) = 0;
				WBUFW(buf, 8) = 0;
				WBUFW(buf, 10) = sd->status.special;
				WBUFB(buf, 12) = 0;
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
				break;


			/*** Equip Code ***/
			case 0xa9:
				if (sd->using_card) {
					break;
				}
				if ((sd->status.inventory[RFIFOW(fd, 2) - 2].nameid >= 10001) && (sd->status.inventory[RFIFOW(fd, 2) - 2].nameid <= 10019)) {
					mmo_pet_equip(sd, RFIFOW(fd, 2));
					break;
				}
				ep = itemdb_equip_point(sd->status.inventory[RFIFOW(fd, 2) - 2].nameid, sd);
				item_num = itemdb_stype(sd->status.inventory[RFIFOW(fd, 2) - 2].nameid);
				item_type = RFIFOW(fd, 4);
				item_view = itemdb_view_point(sd->status.inventory[RFIFOW(fd, 2) - 2].nameid);
				struct item_db2 item_db;

				if (sd->status.base_level < itemdb_elv(sd->status.inventory[RFIFOW(fd, 2) - 2].nameid)) {
					WFIFOW(fd, 0) = 0xaa;
					WFIFOW(fd, 2) = RFIFOW(fd, 2);
					WFIFOB(fd, 6) = 0;
					WFIFOSET(fd, packet_len_table[0xaa]);
					break;
				}
				if (!itemdb_can_equipt(sd->status.inventory[RFIFOW(fd, 2) - 2].nameid, sd->status.class)) {
					if (sd->account_id <= 100000 || (gm_equip_all == 0 && sd->account_id > 100000)) {
						WFIFOW(fd, 0) = 0xaa;
						WFIFOW(fd, 2) = RFIFOW(fd, 2);
						WFIFOB(fd, 6) = 0;
						WFIFOSET(fd, packet_len_table[0xaa]);
						break;
					}
				}
				if (item_num == 2) {
					skill_reset_stats (-1, 0 , fd, 60);
				}
				if ((ep & RFIFOW(fd, 4)) == 0) {
					WFIFOW(fd, 0) = 0xaa;
					WFIFOW(fd, 2) = RFIFOW(fd, 2);
					WFIFOB(fd, 6) = 0;
					WFIFOSET(fd, packet_len_table[0xaa]);
				}
				else {
					if (ep == 0x88) {
						for (i = 0; i < MAX_STORAGE; i++) {
							if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip & 0x80)) {
								ep = ep & 0x08;
								j = i;
							}
							else if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip & 0x08)) {
								ep = ep & 0x80;
							}
						}
						if (ep == 0) {
							WFIFOW(fd, 0) = 0xac;
							WFIFOW(fd, 2) = j;
							WFIFOW(fd, 4) = sd->status.inventory[j].equip;
							sd->status.inventory[j].equip = 0;
							WFIFOB(fd, 6) = 1;
							WFIFOSET(fd, packet_len_table[0xac]);
							mmo_map_calc_status(fd, sd->status.inventory[j].nameid);
							mmo_map_calc_card(fd, sd->status.inventory[j].nameid, j, 0);
							ep = 0x80;
						}
						else if (ep == 0x88) {
							ep = 0x08;
						}
						WFIFOW(fd, 0) = 0xaa;
						WFIFOW(fd, 2) = RFIFOW(fd, 2);
						WFIFOW(fd, 4) = ep;
						WFIFOB(fd, 6) = 1;
						WFIFOSET(fd, packet_len_table[0xaa]);
						sd->status.inventory[RFIFOW(fd, 2) - 2].equip = ep;
						mmo_map_calc_status(fd, sd->status.inventory[RFIFOW(fd, 2) - 2].nameid);
						mmo_map_calc_card(fd, 0, 0, 1);
					}
					else if ((sd->status.class == 12) && (sd->status.inventory[RFIFOW(fd, 2) - 2].nameid > 1200) && (sd->status.inventory[RFIFOW(fd, 2) - 2].nameid < 1250)) {
						for (i = 0; i < MAX_STORAGE; i++) {
							if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip & 0x02)) {
								j1 = 1;
								j = i;
								if ((sd->status.inventory[i].nameid > 1200) && (sd->status.inventory[i].nameid < 1250)) {
									ep = 0x20;
									j1 = 3;
								}
							}
						}
						if (j1 == 3) {
							for (i = 0; i < MAX_STORAGE; i++) {
								if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip & 0x20)) {
									j1 = 4;
									j = i;
								}
							}
						}
						if ((j1 == 1) || (j1 == 4)) {
							WFIFOW(fd, 0) = 0xac;
							WFIFOW(fd, 2) = j;
							WFIFOW(fd, 4) = sd->status.inventory[j].equip;
							sd->status.inventory[j].equip = 0;
							WFIFOB(fd, 6) = 1;
							WFIFOSET(fd, packet_len_table[0xac]);
							mmo_map_calc_status(fd, sd->status.inventory[j].nameid);
							mmo_map_calc_card(fd, sd->status.inventory[j].nameid, j, 0);
						}
						WFIFOW(fd, 0) = 0xaa;
						WFIFOW(fd, 2) = RFIFOW(fd, 2);
						WFIFOW(fd, 4) = ep;
						WFIFOB(fd, 6) = 1;
						WFIFOSET(fd, packet_len_table[0xaa]);
						sd->status.inventory[RFIFOW(fd, 2) - 2].equip = ep;
						if (item_num != LOOK_SHOES) {
							len = mmo_map_set_look(buf, sd->account_id, item_num, item_view);
							if (len > 0) {
								mmo_map_sendall(fd, buf, len, 0);
							}
							set_equip(sd, item_num, item_view);
						}
						mmo_map_calc_status(fd, sd->status.inventory[RFIFOW(fd, 2) - 2].nameid);
						mmo_map_calc_card(fd, 0, 0, 1);
					}
					else {
						for (i = 0; i < MAX_STORAGE; i++) {
							if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip & ep)) {
								remove_item(fd, i + 2);
							}
						}
						item_db = item_database(sd->status.inventory[RFIFOW(fd, 2) - 2].nameid);
						WFIFOW(fd, 0) = 0xaa;
						WFIFOW(fd, 2) = RFIFOW(fd, 2);
						WFIFOW(fd, 4) = ep;
						WFIFOB(fd, 6) = 1;
						WFIFOSET(fd, packet_len_table[0xaa]);
						sd->status.inventory[RFIFOW(fd, 2) - 2].equip = ep;
						if (item_num != LOOK_SHOES) {
							len = mmo_map_set_look(buf, sd->account_id, item_num, item_view);
							if (len > 0) {
								mmo_map_sendall(fd, buf, len, 0);
							}
							set_equip(sd, item_num, item_view);
						}
						mmo_map_calc_status(fd, sd->status.inventory[RFIFOW(fd, 2) - 2].nameid);
						mmo_map_calc_card(fd, 0, 0, 1);
					}
				}
				break;

			case 0xab: /* Equipment Cancellation */
				if (sd->using_card)
					break;

				remove_item(fd, RFIFOW(fd, 2));
				break;

			/* Down below is a set of cases most of them
			 * incompletes or have some kind of errors.
			 * End of code optimization.
			 */

			case 0x14d: /* Guild creation information */
				break;

			case 0x14f: /* Guild creation tab */
				break;

			case 0x151: /* Emblem request */
				break;

			case 0x159: /* Guild secession */
				break;

			case 0x15b: /* Guild exile */
				break;

			case 0x165: /* Guild creation */
				break;

			case 0x168: /* Guild join request */
				break;

			case 0x16b: /* Guild join request reply */
				break;

			case 0x16e: /* Guild notification setting */
				break;

			case 0x170: /* Alliance join request */
				break;

			case 0x172: /* Alliance join request reply */
				break;

			case 0x17e: /* Guild chat */
				WFIFOW(fd,0) = 0x17f;
				WFIFOW(fd,2) = RFIFOW(fd,2);
				strcpy(WFIFOP(fd,4),RFIFOP(fd,4));
				WFIFOSET(fd,RFIFOW(fd,2));
				break;


			/*** Vending Code ***/
			case 0x12e: /* Close vending */
				WBUFW(buf, 0) = 0x0132;
				WBUFL(buf, 2) = sd->account_id;
				mmo_map_sendarea(fd, buf, packet_len_table[0x0132], 1);
				memset(&sd->status.shopname, 0, 80);
				for (i = 0; i < MAX_PC_SHOPITEMS; i++) {
					if (sd->status.shopitems[i].amount) {
						n_item = &sd->status.cart[sd->status.shopitems[i].index-1];
						WFIFOW(fd, 0) = 0x1c5;
						WFIFOW(fd, 2) = sd->status.shopitems[i].index;
						WFIFOL(fd, 4) = sd->status.shopitems[i].amount;
						WFIFOW(fd, 8) = n_item->nameid;
						WFIFOB(fd, 10) = itemdb_type(n_item->nameid);
						WFIFOB(fd, 11) = n_item->identify;
						WFIFOB(fd, 12) = n_item->attribute;
						WFIFOB(fd, 13) = n_item->refine;
						WFIFOW(fd, 14) = n_item->card[0];
						WFIFOW(fd, 16) = n_item->card[1];
						WFIFOW(fd, 18) = n_item->card[2];
						WFIFOW(fd, 20) = n_item->card[3];
						WFIFOSET(fd, packet_len_table[0x1c5]);
					}
					memset(&sd->status.shopitems[i], 0, sizeof(struct vendlist));
				}
				WFIFOW(fd, 0) = 0x121;
				WFIFOW(fd, 2) = sd->status.cartcount;
				WFIFOW(fd, 4) = 100;
				WFIFOL(fd, 6) = sd->status.cartweight;
				WFIFOL(fd, 10) = 80000;
				WFIFOSET(fd, packet_len_table[0x121]);
				break;

			case 0x130: /* Vending item list required */
			{
				int found = 0;
				for (i = 5; i < FD_SETSIZE && !found; i++) {
					if (session[i] && (target_sd = session[i]->session_data)) {
						if (target_sd->account_id == (signed)RFIFOL(fd, 2)) {
							WFIFOW(fd, 0) = 0x0133;
							WFIFOL(fd, 4) = RFIFOL(fd, 2);
							while ((iItemCount < MAX_PC_SHOPITEMS) && (target_sd->status.shopitems[iItemCount].amount)) {
								n_item = &target_sd->status.cart[target_sd->status.shopitems[iItemCount].index-1];
								WFIFOL(fd, 8 + iItemCount * 22) = target_sd->status.shopitems[iItemCount].price;
								WFIFOW(fd, 12 + iItemCount * 22) = target_sd->status.shopitems[iItemCount].amount;
								WFIFOW(fd, 14 + iItemCount * 22) = target_sd->status.shopitems[iItemCount].index;
								WFIFOB(fd, 16 + iItemCount * 22) = itemdb_type(n_item->nameid);
								WFIFOW(fd, 17 + iItemCount * 22) = n_item->nameid;
								WFIFOB(fd, 19 + iItemCount * 22) = n_item->identify;
								WFIFOB(fd, 20 + iItemCount * 22) = n_item->attribute;
								WFIFOB(fd, 21 + iItemCount * 22) = n_item->refine;
								WFIFOW(fd, 22 + iItemCount * 22) = n_item->card[0];
								WFIFOW(fd, 24 + iItemCount * 22) = n_item->card[1];
								WFIFOW(fd, 26 + iItemCount * 22) = n_item->card[2];
								WFIFOW(fd, 28 + iItemCount * 22) = n_item->card[3];
								iItemCount++;
							}
							WFIFOW(fd, 2) = 8 + iItemCount * 22;
							WFIFOSET(fd, 8 + iItemCount * 22);
							found = 1;
						}
					}
				}
				break;
			}

			case 0x134: /* Vending purchase request */
			{
				int k, nosell = 0, found = 0;
				struct item t_item;

				iItemCount = (RFIFOW(fd, 2) - 8) / 4;
				for (i = 5; i < FD_SETSIZE && !found; i++) {
					if (session[i] && (target_sd = session[i]->session_data)) {
						if (target_sd->account_id == (signed)RFIFOL(fd, 4)) {
							for (j = 0; j < iItemCount; j++) {
								itemindex = RFIFOW(fd, 10 + j * 4);
								itemamount = RFIFOW(fd, 8 + j * 4);
								for(k = 0; k < MAX_PC_SHOPITEMS; k++) {
									if (target_sd->status.shopitems[k].index == itemindex) {
										break;
									}
								}
								if (itemamount < 0 || itemamount > target_sd->status.shopitems[k].amount) {
									nosell = 1;
									break;
								}
								n_item = &target_sd->status.cart[itemindex-1];
								memcpy(&t_item, n_item, sizeof(struct item));
								t_item.amount = itemamount;
								mmo_map_get_item(fd, &t_item);
								target_sd->status.cartweight -= (itemdb_weight(n_item->nameid)) * itemamount;
								WFIFOW(target_sd->fd, 0) = 0x0137;
								WFIFOW(target_sd->fd, 2) = itemindex;
								WFIFOW(target_sd->fd, 4) = itemamount;
								WFIFOSET(target_sd->fd, packet_len_table[0x137]);
								target_sd->status.zeny += target_sd->status.shopitems[k].price * itemamount;
								sd->status.zeny -= target_sd->status.shopitems[k].price * itemamount;
								len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_ZENY);
								if (len > 0) {
									WFIFOSET(fd, len);
								}
								len = mmo_map_set_param(target_sd->fd, WFIFOP(target_sd->fd, 0), SP_ZENY);
								if (len > 0) {
									WFIFOSET(target_sd->fd, len);
								}
								if ((target_sd->status.shopitems[k].amount - itemamount) <= 0) {
									target_sd->status.cartcount--;
									memset(n_item, 0, sizeof (struct item));
									memset(&target_sd->status.shopitems[k], 0, sizeof (struct vendlist));
								}
								else {
									n_item->amount -= itemamount;
									target_sd->status.shopitems[k].amount -= itemamount;
								}
							}
							if (nosell) {
								break;
							}
							found = 1;
						}
					}
				}
				break;
			}

			case 0x12f:
			case 0x1b2: /* Options for Vending skill */
			{
				iItems = (RFIFOW(fd, 2) - 85) / 8;
				if (RFIFOB(fd, 4)) {
					memcpy(sd->status.shopname, RFIFOP(fd, 4), 80);
					for (i = 0; i < iItems; i++) {
						memcpy(&sd->status.shopitems[i], RFIFOP(fd, 85 + (i * 8)), 8);
					}
					for (i = 0; i < iItems; i++) {
						WFIFOW(fd, 0) = 0x125;
						WFIFOW(fd, 2) = sd->status.shopitems[i].index;
						WFIFOL(fd, 4) = sd->status.shopitems[i].amount;
						WFIFOSET(fd, packet_len_table[0x125]);
					}
					WFIFOW(fd, 0) = 0x136;
					WFIFOW(fd, 2) = 8 + (iItems * 22);
					WFIFOL(fd, 4) = sd->account_id;
					for (i = 0; i < iItems; i++) {
						n_item = &sd->status.cart[sd->status.shopitems[i].index-1];
						WFIFOL(fd, 8 + i * 22) = sd->status.shopitems[i].price;
						WFIFOW(fd, 12 + i * 22) = sd->status.shopitems[i].index;
						WFIFOW(fd, 14 + i * 22) = sd->status.shopitems[i].amount;
						WFIFOB(fd, 16 + i * 22) = itemdb_type(n_item->nameid);
						WFIFOW(fd, 17 + i * 22) = n_item->nameid;
						WFIFOB(fd, 19 + i * 22) = n_item->identify;
						WFIFOB(fd, 20 + i * 22) = n_item->attribute;
						WFIFOB(fd, 21 + i * 22) = n_item->refine;
						WFIFOW(fd, 22 + i * 22) = n_item->card[0];
						WFIFOW(fd, 24 + i * 22) = n_item->card[1];
						WFIFOW(fd, 26 + i * 22) = n_item->card[2];
						WFIFOW(fd, 28 + i * 22) = n_item->card[3];
					}
					WFIFOSET(fd, 8 + (iItems * 22));
					WBUFW(buf, 0) = 0x0131;
					WBUFL(buf, 2) = sd->account_id;
					memcpy(WBUFP(buf, 6), &sd->status.shopname, 80);
					mmo_map_sendarea(fd, buf, packet_len_table[0x0131], 1);
				}
				break;
			}


			case 0x18e: /* Forge Items */

			{
				int index, slot[2];
				int element_s[4] = { 3, 1, 4, 2 };
				int ele_c = 0;
				int num, chance = 0, star_mod = 0, enc_stone = 0, anvil = 0;

				int name_id = RFIFOW(fd,2);
				slot[0] = RFIFOW(fd,4);
				slot[1] = RFIFOW(fd,6);
				slot[2] = RFIFOW(fd,8);

				for (i = 0; i < 3; i++) {
					if (slot[i] > 0) {
						if (slot[i] >= 994 && slot[i] <= 997 && ele_c == 0) {
							ele_c = element_s[slot[i] - 994];
						}
						index = search_item2(sd, slot[i]);
						if (index) {
							mmo_map_lose_item(fd, 0, index, 1);
						}
					}
				}
				num = rand() % 100;
				chance = sd->status.job_level * 0.2 + (sd->status.dex + sd->status.dex2) * 0.1 + (sd->status.luk + sd->status.luk2) * 0.1 + 20 + sd->status.skill[107-1].lv;
				switch (RFIFOW(fd, 2)) {
					case 985: /* Elunium */
					case 984: /* Oridecon */
						chance += (22 + (6 * sd->status.skill[97-1].lv));
						break;

					case 998: /* Iron */
						chance += (22 + (6 * sd->status.skill[94-1].lv));
						break;

					case 999: /* Steel */
						chance += (10 + (5 * sd->status.skill[85-1].lv));
						break;

					default: /* Ench Stone */
						chance += (10 + (5 * sd->status.skill[96-1].lv));
						break;
				}
				if (slot[0] == 1000) {
					star_mod += 10 * sd->status.forge_lvl;
				}
				else if ((slot[0] >= 994) && (slot[0] <= 997)) {
					enc_stone += 15;
				}
				if (slot[1] == 1000) {
					star_mod += 10 * sd->status.forge_lvl;
				}
				else if ((slot[1] >= 994) && (slot[1] <= 997)) {
					enc_stone += 15;
				}
				if (slot[2] == 1000) {
					star_mod += 10 * sd->status.forge_lvl;
				}
				else if ((slot[2] >= 994) && (slot[2] <= 997)) {
					enc_stone += 15;
				}
				for(i = 986; i <= 989; i++) {
					if (search_item2(sd, i)) {
						anvil = i;
					}
				}
				if (anvil == 0) {
					anvil = 0;
				}
				else if (anvil == 986) {
					anvil = 0;
				}
				else if (anvil == 987) {
					anvil = 3;
				}
				else if (anvil == 988) {
					anvil = 5;
				}
				else {
					anvil = 10;
				}
				switch (sd->status.forge_lvl) {
					case 0: /* Iron, Steel, Oridecon, Ench Stones, Elunium */
						chance = 100 - chance;
						if (num >= chance) {
							skill_do_forge(fd, sd, name_id, 1, ele_c);
						}
						else {
							skill_do_forge(fd, sd, name_id, 0, ele_c);
						}
						break;

					case 1: /* Weapon lvl 1 */
						chance += 10 + anvil + sd->status.skill[107-1].lv - enc_stone - star_mod;
						chance = 100 - chance;
						if (num >= chance) {
							skill_do_forge(fd, sd, name_id, 1, ele_c);
						}
						else {
							skill_do_forge(fd, sd, name_id, 0, ele_c);
						}
						break;

					case 2: /* Weapon lvl 2 */
						chance += 20 + anvil + sd->status.skill[107-1].lv - 20 - enc_stone - star_mod;
						chance = 100 - chance;
						if (num >= chance) {
							skill_do_forge(fd, sd, name_id, 1, ele_c);
						}
						else {
							skill_do_forge(fd, sd, name_id, 0, ele_c);
						}
						break;

					case 3: /* Weapon lvl 3 */
						chance += 30 + anvil + sd->status.skill[107-1].lv - 40 - enc_stone - star_mod; //- #*ElementalAttributeDifficulty - #*StarCrumbDifficulty
						chance = 100 - chance;
						if (num >= chance){
							skill_do_forge(fd, sd, name_id, 1, ele_c);
						}
						else {
							skill_do_forge(fd, sd, name_id, 0, ele_c);
						}
						break;
				}
				break;
			}
			default:
				printf("Packet Sniffer: fd->[%d], RFIFOREST((fd)->[%d]), Packet->[0x%x]\n", fd, RFIFOREST(fd), RFIFOW(fd, 0));
				break;
		}
		RFIFOSKIP(fd, parsing_packet_len);
	}
	return 0;
}

/*======================================
 *	ZONE: Map Functions
 *--------------------------------------
 */

void unload_mapdata(void)
{
	int i, j;

	if (map_number != 0) {
		for (i = 0; i < map_number; i++) {
			if (map_data[i].gat)
				free(map_data[i].gat);

			for (j = 0; j < map_data[i].npc_num; j++) {
				if (map_data[i].npc[j]->block.subtype == SCRIPT) {
					if (map_data[i].npc[j]->u.script)
						free(map_data[i].npc[j]->u.script);

				}
				if (map_data[i].npc[j]->block.subtype == SHOP) {
					if (map_data[i].npc[j]->u.shop_item)
						free(map_data[i].npc[j]->u.shop_item);

				}
				if (map_data[i].npc[j]->block.subtype == MONS) {
					if (map_data[i].npc[j]->u.mons.timer)
						delete_timer(map_data[i].npc[j]->u.mons.timer, mons_walk);

					if (map_data[i].npc[j]->u.mons.attacktimer > 0)
						delete_timer(map_data[i].npc[j]->u.mons.attacktimer, mmo_map_attack_continue);

				}
				if (map_data[i].npc[j])
					free(map_data[i].npc[j]);

			}
			if (map_data[i].block)
				free(map_data[i].block);

		}
	}
}

void mmo_map_allclose(void)
{
	unsigned int fd;
	struct map_session_data *sd;

	for (fd = 5; fd < FD_SETSIZE; fd++) {
		if (session[fd] && (sd = session[fd]->session_data) && (sd->state.auth)) {
			strncpy(sd->status.last_point.map, sd->mapname, 16);
			sd->status.last_point.x = sd->x;
			sd->status.last_point.y = sd->y;
			mmo_char_save(sd);
			if (sd->status.pet.activity) {
				sd->status.pet.pet_hungry = 0;
				mmo_pet_save(sd);
			}
		}
		close(fd);
		delete_session(fd);
		mmo_online_check();
	}
	if (char_fd > 0) {
		close(char_fd);
		delete_session(char_fd);
		char_fd = -1;
	}
}

int mmo_map_flagload(char mapname[24], int type)
{
	int i;

	for (i = 0; i < MAX_MAP_PER_SERVER; i++) {
		if (strncmp(flag_data[i].map_name, mapname, 16) == 0) {
			switch (type) {
				case PVP:
					return flag_data[i].pvp;
				case MEMO:
					return flag_data[i].memo;
				case BRANCH:
					return flag_data[i].branch;
				case TELEPORT:
					return flag_data[i].teleport;
			}
		}
	}
	return 2;
}

void init_mapflag(void)
{
	int i = 0, var[3];
	char m[16];
	char line[1024];
	FILE *fp = fopen("data/databases/map_flag_db.txt", "r");
	if (fp) {
		while (fgets(line, 1023, fp)) {
			if ((line[0] == '/') && (line[1] == '/'))
				continue;

			sscanf(line, "%[^\t]\t%d\t%d\t%d\t%d", m, &var[0], &var[1], &var[2], &var[3]);
			strncpy(flag_data[i].map_name, m, 16);
			flag_data[i].pvp = var[0];
			flag_data[i].memo = var[1];
			flag_data[i].branch = var[2];
			flag_data[i].teleport = var[3];
			i++;
		}
	}
	else {
		printf("**Error: Cannot load map_flag_db.txt!**\n");
		sleep(2);
		exit(0);
	}
	fclose(fp);
}

void read_mapdata(int map_num)
{
	int i;
	char fn[256];
	unsigned char *gat;
	short x, y, xs, ys;
	struct gat_1cell *p = NULL;

	map_number = 0;
	for (i = 0; map[i][0]; i++) {
		if (strstr(map[i], ".gat") == NULL)
			continue;

		sprintf(fn, "data\\%s", map[i]);
		gat = grfio_read(fn);
		printf("\rLoading %-20s [%d/%d]", fn, i, map_num);
		fflush(stdout);
		if (gat) {
			xs = map_data[i].xs = *(int*)(gat + 6);
			ys = map_data[i].ys = *(int*)(gat + 10);
			map_data[i].gat = malloc(xs * ys);
			map_data[i].npc_num = 0;
			map_data[i].users = 0;
			for (y = 0; y < ys; y++) {
				p = (struct gat_1cell*)(gat + y * xs * 20 + 14);
				for (x = 0; x < xs; x++) {
					if (p->type == 0)
						map_data[i].gat[x + y * xs] = (p->high[0] > 3 || p->high[1] > 3 || p->high[2] > 3 || p->high[3] > 3) ? 3 : 0;

					else
						map_data[i].gat[x + y * xs] = p->type;

					p++;
				}
			}
			free(gat);
			map_data[i].bxs = (xs + BLOCK_SIZE - 1) / BLOCK_SIZE;
			map_data[i].bys = (ys + BLOCK_SIZE - 1) / BLOCK_SIZE;
			map_data[i].block = malloc(map_data[i].bxs * map_data[i].bys * sizeof(struct block_list));
			if (map_data[i].block == NULL) {
				printf("Out of Memory: Map Read Block.\n");
				exit(1);
			}
			for (x = 0; x < map_data[i].bxs * map_data[i].bys; x++) {
				map_data[i].block[x].next = NULL;
				map_data[i].block[x].prev = NULL;
				map_data[i].block[x].type = BL_NUL;
			}
			map_number++;
		}
	}
	printf("\r%d Maps Hosted on this Server. %24s\n",  map_number, "");
}

/*======================================
 *	ZONE: Set Timers Functions
 *--------------------------------------
 */

int check_connect_char_server(int tid, unsigned int tick, int id, int data)
{
	id = 0;
	tid = 0;
	tick = 0;
	data = 0;
	if (!session[char_fd]) {
		char_fd = make_connection(inet_addr(char_ip_str), char_port);
		session[char_fd]->func_parse = parse_tochar;
		realloc_fifo(char_fd, FIFO_SIZE, FIFO_SIZE);
		WFIFOW(char_fd, 0) = 0x2af8;
		memcpy(WFIFOP(char_fd, 2), userid, 24);
		memcpy(WFIFOP(char_fd, 26), passwd, 24);
		WFIFOL(char_fd, 54) = map_ip;
		WFIFOW(char_fd, 58) = map_port;
		WFIFOL(char_fd, 60) = map_xip;
		WFIFOSET(char_fd, 64);
	}
	return 0;
}

int send_users_tochar(int tid, unsigned int tick, int id, int data)
{
	int i, users = 0;
	id = 0;
	tid = 0;
	tick = 0;
	data = 0;
	struct map_session_data *sd;
	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && (sd = session[i]->session_data) && sd->state.auth)
			users++;
	}
	if (session[char_fd]) {
		WFIFOW(char_fd, 0) = 0x2aff;
		WFIFOL(char_fd, 2) = users;
		WFIFOSET(char_fd, 6);
	}
	return 0;
}

int kick_timed_out_clients(int tid, unsigned int tick, int fd, int data)
{
	int i;
	unsigned char buf[256];
	tid = 0;
	data = 0;
	struct map_session_data *sd;
	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && (sd = session[i]->session_data)) {
			if (sd->state.auth) {
				if ((tick - sd->server_tick) > 60000) {
					printf("Client Timed Out\n");
					mmo_map_cardskills2(i, sd);
					memcpy(sd->status.last_point.map, sd->mapname, 16);
					sd->status.last_point.x = sd->x;
					sd->status.last_point.y = sd->y;
					mmo_char_save(sd);
					if (sd->chatID)
						mmo_map_leavechat(i, (struct mmo_chat*)sd->chatID, sd->status.name);

					del_block(&sd->block);
					WBUFW(buf, 0) = 0x80;
					WBUFL(buf, 2) = sd->account_id;
					WBUFB(buf, 6) = 2;
					mmo_map_sendarea(i, buf, packet_len_table[0x80], 1);
					map_data[sd->mapno].users--;
					if (sd->status.pet.activity) {
						del_block(&map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block);
						set_monster_no_point(sd->mapno, sd->status.pet.pet_npc_id_on_map[sd->mapno]);
						WBUFW(buf, 0) = 0x80;
						WBUFL(buf, 2) = sd->status.pet.pet_id_as_npc;
						WBUFB(buf, 6) = 0;
						mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
						sd->status.pet.pet_hungry = 0;
						mmo_pet_save(sd);
					}
					close(i);
					delete_session(i);
					mmo_online_check();
				}
			}
		}
	}
	return 0;
}

int wait_close(int tid, unsigned int tick, int id, int data)
{
	tid = 0;
	tick = 0;
	data = 0;
	if (session[id])
		session[id]->eof = 1;

	return 0;
}

int remove_new_on_map(int timer_id, unsigned int tick, int fd, int data)
{
	tick = 0;
	data = 0;
	timer_id = 0;
	struct map_session_data *sd;
	if (session[fd] && (sd = session[fd]->session_data)) {
		sd->new_on_map = 0;
		delete_timer(sd->state.remove_new_on_map, remove_new_on_map);
	}
	return 0;
}

int mmo_map_reset_vars(int tid, unsigned int tick, int fd, int data)
{
	tid = 0;
	tick = 0;
	data = 0;
	struct map_session_data *sd;
	if (session[fd] && (sd = session[fd]->session_data)) {
		sd->attacktimer = 0;
		sd->attacktarget = 0;
		delete_timer(sd->reset_state, mmo_map_reset_vars);
	}
	return 0;
}

int clear_flooritem(int tid, unsigned int tick, int id, int data)
{
	int i;
	unsigned char buf[256];
	id = 0;
	tid = 0;
	data = 0;
	struct flooritem_data *fitem;
	for (i = 2; i <= last_object_id; i++) {
		if (!object[i] || object[i]->type != BL_ITEM)
			continue;

		fitem = (struct flooritem_data*)object[i];
		if (tick - fitem->drop_tick < LIFETIME_FLOORITEM * 1000)
			continue;

		WBUFW(buf, 0) = 0xa1;
		WBUFL(buf, 2) = i;
		mmo_map_sendarea_mxy(fitem->m, fitem->x, fitem->y, buf, packet_len_table[0xa1]);
		delete_object(i);
	}
	return 0;
}

int loop_monster_search(int fd, unsigned int tick, int id, int data)
{
	int n;
	short m;
	unsigned int i;
	fd = 0;
	id = 0;
	tick = 0;
	data = 0;
	struct map_session_data *sd;
	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && (sd = session[i]->session_data)) {
			if (sd->status.hp > 0) {
				m = sd->mapno;
				for (n = 0; n < MAX_NPC_PER_MAP; n++) {
					if (map_data[m].npc[n] && map_data[m].npc[n]->block.subtype == MONS && (rand() % 2) != 0) {
						if (map_data[m].npc[n]->u.mons.target_fd == 0 && map_data[m].npc[n]->u.mons.hp > 0) {
							if (mons_data[map_data[m].npc[n]->class].aggressive == 1 ||
							    mons_data[map_data[m].npc[n]->class].aggressive == 3 ||
							    mons_data[map_data[m].npc[n]->class].aggressive == 4) {
								if (in_range(map_data[m].npc[n]->x, map_data[m].npc[n]->y, sd->x, sd->y, BLOCK_SIZE * 3 / 2)) {
									check_new_target_monster(m, n, i);
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

int heal_hp(int tid, unsigned int tick, int id, int data)
{
	unsigned int i;
	int j, hp, heal_point;
	double magnif = 1;
	double bonus;
	unsigned char buf[256];
	id = 0;
	tid = 0;
	tick = 0;
	data = 0;
	struct map_session_data *sd;
	for (i = 0; i < MAX_FLOORSKILL; i++) {
		if (floorskill[i].type == FS_SANCT)
			floorskill[i].count = 3 + floorskill[i].skill_lvl;

	}
	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && (sd = session[i]->session_data)) {
			if (sd->status.party_status > 0) {
				j = party_exists(sd->status.party_id);
				if (j != -1)
					party_update_member_hp(j, sd->account_id, sd->char_id, sd->status.hp, sd->status.max_hp, sd->mapno);

			}
			j = search_floorskill_index(sd->mapno, sd->x, sd->y);
			if (j > -1) {
				switch (floorskill[j].type) {
					case FS_SANCT:
						if (floorskill[j].count > 0 && sd->status.hp < sd->status.max_hp) {
							if (floorskill[j].skill_lvl > 6)
								heal_point = 777;

							else
								heal_point = floorskill[j].skill_lvl * 100;

							if (heal_point > sd->status.max_hp - sd->status.hp)
								heal_point = sd->status.max_hp - sd->status.hp;

							if (sd->status.hp <= 0) {
								sd->status.heal_time = 0;
								sd->status.heal_time_skill = 0;
								continue;
							}
							WBUFW(buf, 0) = 0x11a;
							WBUFW(buf, 2) = 28;
							WBUFW(buf, 4) = heal_point;
							WBUFL(buf, 6) = sd->account_id;
							WBUFL(buf, 10) = 0;
							WBUFB(buf, 14) = 1;
							mmo_map_sendarea(sd->fd, buf, packet_len_table[0x11a], 0);
							sd->status.hp += heal_point;
							if ((sd->status.hp > sd->status.max_hp) || (sd->status.hp <= 0))
								sd->status.hp = sd->status.max_hp;

							WFIFOW(sd->fd, 0) = 0xb0;
							WFIFOW(sd->fd, 2) = 0005;
							WFIFOL(sd->fd, 4) = sd->status.hp;
							WFIFOSET(sd->fd, packet_len_table[0xb0]);
							floorskill[j].count--;
						}
						break;
				}
			}
			if (sd->weight >= sd->max_weight / 2)
				continue;

			if (sd->hidden)
				continue;

			if (sd->status.hp > sd->status.max_hp) {
				sd->status.heal_time = 0;
				sd->status.heal_time_skill = 0;
				sd->status.hp = sd->status.max_hp;
				continue;
			}
			if (sd->status.hp == sd->status.max_hp) {
				sd->status.heal_time = 0;
				sd->status.heal_time_skill = 0;
				continue;
			}
			if (sd->status.hp <= 0) {
				sd->status.heal_time = 0;
				sd->status.heal_time_skill = 0;
				continue;
			}
			if (sd->walktimer > 0) {
				sd->status.heal_time = 0;
				sd->status.heal_time_skill = 0;
				continue;
			}
			bonus = (double)((sd->status.vit + sd->status.vit2 + 100) / 100);
			if (sd->skill_timeamount[74-1][0] > 0)
				magnif = 1.75;

			sd->status.heal_time += HP_TIME_T * bonus * magnif;

			if (sd->sitting == 2)
				sd->status.heal_time += HP_TIME_S * bonus * magnif;

			if (sd->status.heal_time >= HP_TIME_R) {
				sd->status.heal_time -= HP_TIME_R;
				sd->status.hp++;
				WFIFOW(i, 0) = 0xb0;
				WFIFOW(i, 2) = 0x05;
				WFIFOL(i, 4) = sd->status.hp;
				WFIFOSET(i, packet_len_table[0xb0]);
			}
			if (sd->status.skill[4-1].lv > 0) {
				sd->status.heal_time_skill += HP_TIME_T;
				if (sd->status.heal_time_skill >= 10000) {
					sd->status.heal_time_skill = 0;
					hp = (sd->status.skill[4-1].lv * 5) + (sd->status.max_hp / (550 - (sd->status.skill[4-1].lv * 50))) ;
					if ((hp + sd->status.hp) > sd->status.max_hp)
						sd->status.hp = sd->status.max_hp;

					else
						sd->status.hp = sd->status.hp + hp;

					WFIFOW(i, 0) = 0x13d;
					WFIFOW(i, 2) = 5;
					WFIFOL(i, 4) = hp;
					WFIFOSET(i, packet_len_table[0x13d]);
				}
			}
		}
	}
	return 0;
}

int heal_sp(int tid, unsigned int tick, int id, int data)
{
	int sp;
	unsigned int i;
	double magnif = 1;
	double bonus;
	id = 0;
	tid = 0;
	tick = 0;
	data = 0;
	struct map_session_data *sd;
	for (i = 5; i < FD_SETSIZE; i++) {
		if (!session[i] || !(sd = session[i]->session_data))
			continue;

		if (sd->weight >= sd->max_weight / 2)
			continue;

		if (sd->hidden)
			continue;

		if (sd->status.sp < sd->status.max_sp) {
			bonus = (double)((sd->status.int_ + sd->status.int_2 + 100) / 100);
			if (sd->skill_timeamount[74-1][0] > 0)
				magnif = 1.75;

			sd->status.sp_time += SP_TIME_T * bonus * magnif;
			if (sd->walktimer == 0) {
				if (sd->sitting == 2)
					sd->status.sp_time += SP_TIME_S * bonus * magnif;

				if (sd->status.sp_time >= SP_TIME_R) {
					sd->status.sp_time -= SP_TIME_R;
					sd->status.sp++;
					WFIFOW(i, 0) = 0xb0;
					WFIFOW(i, 2) = 0x07;
					WFIFOL(i, 4) = sd->status.sp;
					WFIFOSET(i, packet_len_table[0xb0]);
				}
				if (sd->status.skill[9-1].lv > 0) {
					sd->status.sp_time_skill += SP_TIME_T;
					if (sd->status.sp_time_skill >= 10000) {
						sd->status.sp_time_skill = 0;
						sp = (sd->status.skill[9-1].lv * 3) * bonus;
						if ((sp + sd->status.sp) > sd->status.max_sp)
							sd->status.sp = sd->status.max_sp;

						else
							sd->status.sp = sd->status.sp + sp;

						WFIFOW(i, 0) = 0x13d;
						WFIFOW(i, 2) = 7;
						WFIFOL(i, 4) = sp;
						WFIFOSET(i, packet_len_table[0x13d]);
					}
				}
			}
			else {
				if (sd->status.sp_time >= SP_TIME_R) {
					sd->status.sp_time -= SP_TIME_R;
					sd->status.sp++;
					WFIFOW(i, 0) = 0xb0;
					WFIFOW(i, 2) = 0x07;
					WFIFOL(i, 4) = sd->status.sp;
					WFIFOSET(i, packet_len_table[0xb0]);
				}
			}
		}
		else {
			sd->status.sp_time = 0;
			sd->status.sp_time_skill = 0;
		}
	}
	return 0;
}

int do_save(int tid, unsigned int tick, int id, int data)
{
	unsigned int i;
	id = 0;
	tid = 0;
	tick = 0;
	data = 0;
	struct map_session_data *sd;
	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && (sd = session[i]->session_data)) {
			strncpy(sd->status.last_point.map,sd->mapname, 16);
			sd->status.last_point.x = sd->x;
			sd->status.last_point.y = sd->y;
			mmo_char_save(sd);
			if (sd->status.pet.activity)
				mmo_pet_save(sd);

		}
	}
	return 0;
}

int keep_alive(int tid, unsigned int tick, int id, int data)
{
	unsigned int i;
	id = 0;
	tid = 0;
	tick = 0;
	data = 0;
	struct map_session_data *sd;
	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && (sd = session[i]->session_data)) {
			WFIFOW(i, 0) = 0x187;
			WFIFOL(i, 2) = sd->account_id;
			WFIFOSET(i, 6);
		}
	}
	return 0;
}

/*======================================
 *	ZONE: Program Initialization
 *--------------------------------------
 */

int do_init(void)
{
	int i;
	int j[10];
	int npc_num_f = 0, map_num = 0;;
	char line[1024], option1[1024], option2[1024];
	struct hostent *h;
	struct timeval tv;
	char tmpstr[256];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	FILE *irc = NULL, *fp = NULL;

	printf("\033[1;41m.----------------------------------------.\033[0m\n");
	printf("\033[1;41m|             Odin Project(c)            |\n");
	printf("\033[1;41m|                                        |\n");
	printf("\033[1;41m|        ___    __     ___   __  _       |\n");
	printf("\033[1;41m|       / _ \\  (   \\  (   ) (  )( )      |\n");
	printf("\033[1;41m|      | ( ) | ( |) |  | |   | \\| |      |\n");
	printf("\033[1;41m|       \\___/  (__ /  (___)  |_|\\_|      |\n");
	printf("\033[1;41m|                                        |\n");
	printf("\033[1;41m|                                        |\n");
	printf("\033[1;41m|        %s       |\n", logN);
	printf("\033[1;41m`========================================'\033[0m\n");
	printf("\n**Server Started**\n");

	if ((irc = fopen("save/status.txt", "w"))) {
		fprintf(irc,"[status]\nserver = 1\n");
	}
	fclose(irc);
	if (!(fp = fopen("config.ini", "r"))) {
		fprintf(fp, "ERROR: %s : Access Violation\n", tmpstr);
		sleep(2);
		abort();
	}
	else {
		while (fgets(line, 1023, fp)) {
			i = sscanf(line, "%[^=]=%s", option1, option2);
			if (i != 2) {
				continue;
			}
			if (strcmp(option1, "ServerUsername") == 0) {
				j[0] = 1;
				memcpy(userid, option2, 24);
			}
			else if (strcmp(option1, "ServerPassword") == 0) {
				j[1] = 1;
				memcpy(passwd,option2, 24);
			}
			else if (strcmp(option1, "CharacterIP") == 0) {
				memcpy(char_ip_str, option2, 16);
			}
			else if (strcmp(option1, "CharacterPort") == 0) {
				char_port = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "ZoneIP") == 0) {
				memcpy(map_ip_str, option2, 16);
			}
			else if (strcmp(option1,"ZonePort") == 0) {
				map_port = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "InetIPS") == 0) {
				memcpy(map_xip_str, option2, 256);
			}
			else if (strcmp(option1, "BaseMultiplier") == 0) {
				j[2] = 1;
				base_mult = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "JobMultiplier") == 0) {
				j[3] = 1;
				job_mult = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "DropMultiplier") == 0) {
				j[4] = 1;
				drop_mult = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "ShowExperience") == 0) {
				j[5] = 1;
				display_exp = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "SuperGM") == 0) {
				j[6] = 1;
				super_gms = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "GMEquipAll") == 0) {
				j[7] = 1;
				gm_equip_all = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "MaxBaseLevel") == 0) {
				j[8] = 1;
				base_max = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "MaxJobLevel") == 0) {
				j[9] = 1;
				job_max = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "SaveTime") == 0) {
				j[10] = 1;
				do_timer_save = (int)strtol(option2, (char**)NULL, 10);
				if ((do_timer_save > 120) || (do_timer_save == 0)) {
					do_timer_save = 15;
				}
			}
			else if (strcmp(option1, "ConsoleEnabled") == 0) {
				j[11] = 1;
				console_enabled = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "ConsolePassword") == 0) {
				j[12] = 1;
				memcpy(c_pswd, option2, 24);
				memset(c_pswd + strlen(c_pswd), 0, 24 - strlen(c_pswd));
			}
			else if (strcmp(option1, "ShowHP") == 0) {
				j[13] = 1;
				show_hp = (int)strtol(option2, (char**)NULL, 10);
			}
		}
		fclose(fp);
	}
	if ((j[0] != 1) || (j[1] != 1) || (j[2] != 1) || (j[3] != 1) || (j[4] != 1) || (j[5] != 1)
	|| (j[6] != 1) || (j[7] != 1) || (j[8] != 1) || (j[9] != 1) || (j[10] != 1) || (j[11] != 1)
	|| (j[12] != 1) || (j[13] != 1)) {
		fprintf(stderr, "ERROR: %s: Read of Data Error\n", tmpstr);
		sleep(2);
		exit(0);
	}
	fp = fopen("data/databases/map_db.txt", "r");
	if (!fp) {
		printf("**Error: Cannot load map_db.txt!**\n");
		sleep(2);
		exit(1);
	}
	else {
		while (fgets(line, 1023, fp)) {
			i = sscanf(line, "%[^:]:%s", option1, option2);
			if (i != 2)
				continue;

			if (strcmp(option1, "map") == 0) {
				strncpy(map[map_num], option2, 16);
				map_num++;
				map[map_num+1][0] = 0;
			}
		}
		fclose(fp);
	}
	map_ip = inet_addr(map_ip_str);
	map_xip = inet_addr(map_xip_str);
	h = gethostbyname(map_xip_str);
	if (h)
		map_xip = ((unsigned char)h->h_addr[3] << 24) + ((unsigned char)h->h_addr[2] << 16) + ((unsigned char)h->h_addr[1] << 8) + ((unsigned char)h->h_addr[0]);

	if (map_xip == -1)
		map_xip = map_ip;

#ifdef _MMS_
	map_to_serv[0].map[0] = 0;
#endif
	set_termfunc(mmo_map_allclose);
	grfio_init();
	read_mapdata(map_num);
	npc_num_f = read_npc_fromdir("script/");
	if (npc_num_f == 0) {
		printf("**Error: Cannot load files from script/!**\n");
		sleep(2);
		exit(2);
	}
	read_npcdata();
	mmo_party_do_init();
	mmo_pet_db_init();
	itemdb_init();
	skilldb_init();
	set_lvup_table();
	ele_init();
	init_aspd_stats();
	init_job_stats();
	init_floorskill_data();
	init_monster_skills();
	init_mapflag();
	set_defaultparse(parse_map);
	map_port_fd = make_listen_port(map_port);

	check_connect_timer = add_timer(gettick() + 10, check_connect_char_server, 0, 0);
	timer_data[check_connect_timer]->type = TIMER_INTERVAL;
	timer_data[check_connect_timer]->interval = 10000;

	send_users_tochar_timer = add_timer(gettick() + 10, send_users_tochar, 0, 0);
	timer_data[send_users_tochar_timer]->type = TIMER_INTERVAL;
	timer_data[send_users_tochar_timer]->interval = 5000;

	timed_out_timer = add_timer(gettick() + 10, kick_timed_out_clients, 0, 0);
	timer_data[timed_out_timer]->type = TIMER_INTERVAL;
	timer_data[timed_out_timer]->interval = 10000;

	clear_flooritem_timer = add_timer(gettick() + 10, clear_flooritem, 0, 0);
	timer_data[clear_flooritem_timer]->type = TIMER_INTERVAL;
	timer_data[clear_flooritem_timer]->interval = 1000;

	loop_monstersearch_timer = add_timer(gettick() + 1000, loop_monster_search, 0, 0);
	timer_data[loop_monstersearch_timer]->type = TIMER_INTERVAL;
	timer_data[loop_monstersearch_timer]->interval = DEFAULT_WALK_SPEED * BLOCK_SIZE / 2;

	heal_hp_timer = add_timer(gettick() + 10, heal_hp, 0, 0);
	timer_data[heal_hp_timer]->type = TIMER_INTERVAL;
	timer_data[heal_hp_timer]->interval = 1000;

	heal_sp_timer = add_timer(gettick() + 20, heal_sp, 0, 0);
	timer_data[heal_sp_timer]->type = TIMER_INTERVAL;
	timer_data[heal_sp_timer]->interval = 1000;

	check_save_timer = add_timer(gettick() + do_timer_save * 60000, do_save, 0, 0);
	timer_data[check_save_timer]->type = TIMER_INTERVAL;
	timer_data[check_save_timer]->interval = do_timer_save * 60000;

	check_keep_alive = add_timer(gettick() + 12000, keep_alive, 0, 0);
	timer_data[check_keep_alive]->type = TIMER_INTERVAL;
	timer_data[check_keep_alive]->interval = 12000;

	if ((irc = fopen("save/status.txt", "w")))
		fprintf(irc,"[status]\nserver = 0\n");

	fclose(irc);
	printf("\nMAX USER CAPACITY: %d\n", FD_SETSIZE);
	printf("MAX PARTIES: %d\n", MAX_PARTYS);
	printf("MAX GUILDS: %d\n", MAX_GUILDS);
	printf("\033[1;41m\nServer is online.\033[0m\n");
	return 0;
}

int do_exit()
{
	delete_timer(check_connect_timer, check_connect_char_server);
	delete_timer(send_users_tochar_timer, send_users_tochar);
	delete_timer(clear_flooritem_timer, clear_flooritem);
	delete_timer(loop_monstersearch_timer, loop_monster_search);
	delete_timer(heal_hp_timer, heal_hp);
	delete_timer(heal_sp_timer, heal_sp);
	delete_timer(timed_out_timer, kick_timed_out_clients);
	delete_timer(check_save_timer, do_save);
	delete_timer(check_keep_alive, keep_alive);
	delete_session(map_port_fd);
	unload_mapdata();
	mmo_map_allclose();
	return 0;
}
