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
 Modified Date: October 29, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "npc.h"
#include "item.h"
#include "itemdb.h"
#include "script.h"
#include "pet.h"
#include "skill.h"
#include "chat.h"
#include "monster_skills.h"

char *npc_txt[256];
int npc_txt_num = 0;
static int npc_id = MAX_OBJECTS;

extern char map[][16];
extern int base_mult, job_mult, drop_mult;
extern int last_object_id;
int DC_table [] = { 100,93,91,89,87,85,83,81,79,77,76 };
int OC_table [] = { 100,107,109,111,113,115,117,119,121,123,124 };

/*======================================
 *	NPC: Npc Functions
 *--------------------------------------
 */

int mmo_map_npc_say(unsigned char* buf, unsigned long id, char *string)
{
	WBUFW(buf, 0) = 0xb4;
	WBUFW(buf, 2) = strlen(string) + 9;
	WBUFL(buf, 4) = id;
	strncpy(WBUFP(buf, 8), string, strlen(string) + 1);
	return WBUFW(buf, 2);
}

int mmo_map_npc_next(unsigned char* buf, unsigned long id)
{
	WBUFW(buf, 0) = 0xb5;
	WBUFL(buf, 2) = id;
	return 6;
}

int mmo_map_npc_close(unsigned char* buf, unsigned long id)
{
	WBUFW(buf, 0) = 0xb6;
	WBUFL(buf, 2) = id;
	return 6;
}

int mmo_map_npc_select(unsigned char* buf, unsigned long id, char *string)
{
	WBUFW(buf, 0) = 0xb7;
	WBUFW(buf, 2) = strlen(string) + 9;
	WBUFL(buf, 4) = id;
	strncpy(WBUFP(buf, 8), string, strlen(string) + 1);
	return WBUFW(buf, 2);
}

int mmo_map_npc_amount_request(unsigned char* buf, unsigned long id)
{
	WBUFW(buf, 0) = 0x142;
	WBUFL(buf, 2) = id;
	return 6;
}

int mmo_map_npc_buysell(unsigned char* buf, unsigned long id)
{
	WBUFW(buf, 0) = 0xc4;
	WBUFL(buf, 2) = id;
	return 6;
}

int mmo_map_npc_buy(struct map_session_data *sd, unsigned char* buf, struct npc_item_list *items)
{
	int i;
	int DCPercent = 100;
	int DC_price;

	WBUFW(buf, 0) = 0xc6;
	if(sd->status.skill[37-1].lv > 0) {
		if(sd->status.skill[37-1].lv < 10)
			DCPercent = 95 - 2 * sd->status.skill[37-1].lv;

		else
			DCPercent = 76;

	}
	if(sd->status.skill[224-1].lv > 0)
		DCPercent = 95 - 4 * sd->status.skill[224-1].lv;

	for (i = 0; items[i].nameid; i++) {
		DC_price = (items[i].value * DCPercent) / 100;
		if (DC_price < 1)
			DC_price = 1;

		WBUFL(buf, 4 + i * 11) = items[i].value;
		WBUFL(buf, 8 + i * 11) = DC_price ;
		WBUFB(buf, 12 +i * 11) = itemdb_type(items[i].nameid);
		WBUFW(buf, 13 + i * 11) = items[i].nameid;
	}
	WBUFW(buf, 2) = i * 11 + 4;
	return i * 11 + 4;
}

int mmo_map_npc_sell(struct map_session_data *sd, unsigned char *buf)
{
	int i, c_item = 0;
	int OClvl;
	int OC_price;

	OClvl = sd->status.skill[38-1].lv;
	WBUFW(buf, 0) = 0xc7;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid) {
			OC_price = (itemdb_sellvalue(sd->status.inventory[i].nameid)*OC_table[OClvl]) / 100;
			if (OC_price < 1)
				OC_price = 1;

			WBUFW(buf, 4 + c_item * 10) = i + 2;
			WBUFL(buf, 6 + c_item * 10) = itemdb_sellvalue(sd->status.inventory[i].nameid);
			WBUFL(buf, 10 + c_item * 10) = OC_price;
			c_item++;
		}
	}
	WBUFW(buf, 2) = c_item * 10 + 4;
	return c_item * 10 + 4;
}

int npc_click(struct map_session_data *sd, int npc_id)
{
	int npc_len, n;

	if (!sd->status.talking_to_npc) {
		sd->status.talking_to_npc = 1;
		for (n = 0; n < map_data[sd->mapno].npc_num; n++) {
			if (map_data[sd->mapno].npc[n]->id == npc_id)
				break;
		}
		if ((n == map_data[sd->mapno].npc_num) || (map_data[sd->mapno].npc[n]->block.type != BL_NPC))
			return -1;

		switch (map_data[sd->mapno].npc[n]->block.subtype) {
			case SCRIPT:
				sd->npc_pc = 0;
				sd->npc_id = npc_id;
				sd->npc_n = n;
				run_script(sd->fd);
				break;
			case SHOP:
				sd->status.talking_to_npc = 0;
				npc_len = mmo_map_npc_buysell(WFIFOP(sd->fd, 0), npc_id);
				WFIFOSET(sd->fd, npc_len);
				sd->npc_id = npc_id;
				sd->npc_n = n;
				break;
		}
	}
	return 0;
}

int npc_menu_select(struct map_session_data *sd, int sel)
{
	sd->local_reg[15] = sel;
	if (sel == 0xff) {
		sd->npc_pc = 0;
		sd->npc_id = 0;
		sd->npc_n = 0;
		sd->status.talking_to_npc = 0;
		return 0;
  	}
	run_script(sd->fd);
	return 0;
}

int npc_next_click(struct map_session_data *sd)
{
	run_script(sd->fd);
	return 0;
}

int npc_amount_input(struct map_session_data *sd, int val)
{
	sd->local_reg[14] = val;
	run_script(sd->fd);
	return 0;
}

int npc_buysell_selected(struct map_session_data *sd, int npc_id, int sell)
{
	int npc_len ,n;

	for (n = 0; n < map_data[sd->mapno].npc_num; n++) {
		if (map_data[sd->mapno].npc[n]->id == npc_id)
			break;
	}
	if ((n == map_data[sd->mapno].npc_num) || (map_data[sd->mapno].npc[n]->block.type != BL_NPC)
	|| (map_data[sd->mapno].npc[n]->block.subtype != SHOP) || (sd->npc_id != npc_id))
		return -1;

	if (sell)
		npc_len = mmo_map_npc_sell(sd, WFIFOP(sd->fd, 0));

	else
		npc_len = mmo_map_npc_buy(sd, WFIFOP(sd->fd, 0), map_data[sd->mapno].npc[n]->u.shop_item);

	WFIFOSET(sd->fd, npc_len);
	return 0;
}

struct map_session_data *make_rollback_point(struct map_session_data *sd)
{
	struct map_session_data *back = malloc(sizeof(*back));
	memcpy(back, sd, sizeof(*back));
	return back;
}

int do_rollback(struct map_session_data *sd, struct map_session_data *back)
{
	memcpy(sd, back, sizeof(*back));
	free(back);
	return 0;
}

int delete_rollback_point(struct map_session_data *back)
{
	free(back);
	return 0;
}

int npc_buy_selected(struct map_session_data *sd, void *list, int num)
{
	int i, j, n, fd = sd->fd;
	int zeny = 0;
	int DCPercent = 100;
	int DC_price;
	struct item tmp_item;
	struct map_session_data rollback;

	for (n = 0; n < map_data[sd->mapno].npc_num; n++) {
		if (map_data[sd->mapno].npc[n]->id == sd->npc_id)
			break;
	}
	if ((n == map_data[sd->mapno].npc_num) || (map_data[sd->mapno].npc[n]->block.type != BL_NPC)
	|| (map_data[sd->mapno].npc[n]->block.subtype != SHOP))
		return -1;

	if (sd->status.skill[37-1].lv > 0) {
		if(sd->status.skill[37-1].lv < 10)
			DCPercent = 95 - 2 * sd->status.skill[37-1].lv;

		else
			DCPercent = 76;

	}
	if (sd->status.skill[224-1].lv > 0)
		DCPercent = 95 - 4 * sd->status.skill[224-1].lv;

	for (i = 0; i < num; i++) {
		for (j = 0; map_data[sd->mapno].npc[n]->u.shop_item[j].nameid; j++) {
			if (map_data[sd->mapno].npc[n]->u.shop_item[j].nameid == RBUFW(list, i * 4 + 2))
				break;
		}
		if (map_data[sd->mapno].npc[n]->u.shop_item[j].nameid == 0) {
			WFIFOW(fd, 0) = 0xca;
			WFIFOB(fd, 2) = 1;
			WFIFOSET(fd, 3);
			session[fd]->eof = 1;
			return -1;
		}
		DC_price = (map_data[sd->mapno].npc[n]->u.shop_item[j].value * RBUFW(list, i * 4) * DCPercent)/100;
		if (DC_price < 1)
			DC_price = 1;

		zeny += DC_price;
	}
	if (sd->status.zeny < zeny) {
		WFIFOW(fd, 0) = 0xca;
		WFIFOB(fd, 2) = 1;
		WFIFOSET(fd, 3);
		return -1;
	}
	memcpy(&rollback, sd, sizeof(rollback));
	memset(&tmp_item, 0, sizeof(tmp_item));
	tmp_item.identify = 1;
	for (i = 0; i < num; i++) {
		tmp_item.amount = RBUFW(list, i * 4);
		tmp_item.nameid = RBUFW(list, i * 4 + 2);
		if ((tmp_item.amount < 0) || (tmp_item.amount > 10000)) {
			memcpy(sd, &rollback, sizeof(rollback));
			return 0;
		}
		if (mmo_map_get_item(fd,&tmp_item) != 1) {
			WFIFOW(fd, 0) = 0xca;
			WFIFOB(fd, 2) = 2;
			WFIFOSET(fd, 3);
			memcpy(sd, &rollback, sizeof(rollback));
			return -1;
		}
	}
	sd->status.zeny -= zeny;

	WFIFOW(fd, 0) = 0xb1;
	WFIFOW(fd, 2) = 0x14;
	WFIFOL(fd, 4) = sd->status.zeny;
	WFIFOSET(fd, 8);

	WFIFOW(fd, 0) = 0xca;
	WFIFOB(fd, 2) = 0;
	WFIFOSET(fd, 3);

	return 0;
}

int npc_sell_selected(struct map_session_data *sd, void *list, int num)
{
	int i;
	struct map_session_data *back;
	int fail = 0;
	int OClvl;
	int OC_price;
	int orig_value;

	OClvl = sd->status.skill[38-1].lv;
	back = make_rollback_point(sd);
	for (i = 0; i < num; i++) {
		orig_value =itemdb_sellvalue(sd->status.inventory[RBUFW(list, i * 4) - 2].nameid);
		if (orig_value > 1) {
			OC_price = (int)(((double)orig_value * RBUFW(list, i * 4 + 2) * OC_table[OClvl]) / 100);
			if (OC_price < 1)
				OC_price = 1;

		}
		else
			OC_price = orig_value;

		sd->status.zeny += OC_price;
		mmo_map_lose_item(sd->fd, 0, RBUFW(list, i * 4), RBUFW(list,i * 4 + 2));
	}
	if (fail)
		do_rollback(sd, back);

	else
		delete_rollback_point(back);

	WFIFOW(sd->fd, 0) = 0xb1;
	WFIFOW(sd->fd, 2) = 0x14;
	WFIFOL(sd->fd, 4) = sd->status.zeny;
	WFIFOSET(sd->fd, 8);

	WFIFOW(sd->fd, 0) = 0xcb;
	WFIFOB(sd->fd, 2) = fail;
	WFIFOSET(sd->fd, packet_len_table[0xcb]);

	mmo_map_calc_overweight(sd);
	return 0;
}

/*======================================
 *	NPC: Monster Functions
 *--------------------------------------
 */

int set_monster_random_point(short m, int n)
{
	short x, y;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP)
		return 0;

	do {
		x = rand() % (map_data[m].xs - 2) + 1;
		y = rand() % (map_data[m].ys - 2) + 1;
	}
	while (map_data[m].gat[x + y * map_data[m].xs] == 1 || map_data[m].gat[x + y * map_data[m].xs] == 5);

	map_data[m].npc[n]->u.mons.to_x = map_data[m].npc[n]->x = x;
	map_data[m].npc[n]->u.mons.to_y = map_data[m].npc[n]->y = y;
	return 0;
}

int set_monster_no_point(short m, int n)
{
	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP)
		return 0;

	map_data[m].npc[n]->u.mons.to_x = map_data[m].npc[n]->x = 0;
	map_data[m].npc[n]->u.mons.to_y = map_data[m].npc[n]->y = 0;
	map_data[m].npc[n]->dir = 0;
	return 0;
}

int mons_skill_floor_search(unsigned int tick, short m, int n)
{
	int i;
	int damage;
	unsigned char buf[256];
	struct npc_data *monster = map_data[m].npc[n];

	short x = monster->x;
	short y = monster->y;
	int floorskill_index = search_floorskill_index(m, x, y);
	if (floorskill_index > -1) {
		switch (floorskill[floorskill_index].type) {
		case FS_SANCT: // Sanctuary Case
			// This Skill Only Hits the Undead
			if ((mons_data[monster->class].ele % 10 == 9)) {
				if (floorskill[floorskill_index].skill_lvl > 6) {
					damage = 777 / 2;
				} else {
					damage = (floorskill[floorskill_index].skill_lvl * 100) / 2;
				}
				if (damage > monster->u.mons.hp) {
					damage = monster->u.mons.hp;
				}
				WBUFW(buf, 0) = 0x114; // R 0114 <skill ID>.w <src ID>.l <dst ID>.l <server tick>.l <src speed>.l <dst speed>.l <param1>.w <param2>.w <param3>.w <type>.B: skill effect for attack.
				WBUFW(buf, 2) = floorskill[floorskill_index].skill_num;
				WBUFL(buf, 4) = floorskill[floorskill_index].owner_id;
				WBUFL(buf, 8) = monster->id;
				WBUFL(buf, 12) = tick;
				WBUFL(buf, 16) = 100; // src_speed
				WBUFL(buf, 20) = monster->u.mons.speed;
				WBUFW(buf, 24) = damage;
				WBUFW(buf, 26) = floorskill[floorskill_index].skill_lvl;
				WBUFW(buf, 28) = 1; //type num
				WBUFB(buf, 30) = 6; //type hit
				mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x114]);
				skill_do_damage(floorskill[floorskill_index].owner_fd, damage, monster->id, tick, 0);
				if (monster->u.mons.hp <= 0) {
					monster->u.mons.hp = 0;
					return 0;
				}
			}
			break;
		case FS_MAGNUS: // Magnus Exorcism Case
			// This Skill Only Hits Undead and Demons
			if ((floorskill[floorskill_index].count > 0) && ((mons_data[monster->class].ele % 10 == 9) || (mons_data[monster->class].ele % 10 == 7))) {
				damage = 500 * floorskill[floorskill_index].skill_lvl;
				for (i = 0; i < floorskill[floorskill_index].skill_lvl; i++) {
					if (damage > monster->u.mons.hp) {
						damage = monster->u.mons.hp;
					}
					WBUFW(buf, 0) = 0x114; // R 0114 <skill ID>.w <src ID>.l <dst ID>.l <server tick>.l <src speed>.l <dst speed>.l <param1>.w <param2>.w <param3>.w <type>.B: skill effect for attack.
					WBUFW(buf, 2) = floorskill[floorskill_index].skill_num;
					WBUFL(buf, 4) = floorskill[floorskill_index].owner_id;
					WBUFL(buf, 8) = monster->id;
					WBUFL(buf, 12) = tick;
					WBUFL(buf, 16) = 100; // src_speed
					WBUFL(buf, 20) = monster->u.mons.speed;
					WBUFW(buf, 24) = damage;
					WBUFW(buf, 26) = floorskill[floorskill_index].skill_lvl;
					WBUFW(buf, 28) = 1; //type num
					WBUFB(buf, 30) = 6; //type hit
					mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x114]);
					skill_do_damage(floorskill[floorskill_index].owner_fd, damage, monster->id, tick, 0);
					if (monster->u.mons.hp <= 0) {
						monster->u.mons.hp = 0;
						return 0;
					}
				}
				floorskill[floorskill_index].count--; // counter for skill (0 for no count/end of count)
				// No more Waves of attacks left, so skill must dissapear
				if (floorskill[floorskill_index].count == 0) { // counter for skill (0 for no count/end of count)
					if (floorskill[floorskill_index].timer > 0) {
						delete_timer(floorskill[floorskill_index].timer, remove_floorskill);
						// remove_floorskill reset floorskill[j].timer
					}
					remove_floorskill(0, tick, 0, floorskill_index);
				}
			}
			break;
		case FS_TRAP:
			switch(floorskill[floorskill_index].skill_num) {
			case 115: // SKIDTRAP
				break;
			case 116: // LANDMINE
				break;
			case 117: // ANKLE SNARE
				// if no monster already on the trap
				if (floorskill[floorskill_index].n == -1) {
					floorskill[floorskill_index].n = n;
					// stop monster move
					WBUFW(buf, 0) = 0x88; // R 0088 <entity_id>.L <X>.W <Y>.W: stop walking
					WBUFL(buf, 2) = monster->id;
					WBUFW(buf, 6) = x;
					WBUFW(buf, 8) = y;
					mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x88]);
					// block move of monster until end of trap (based on mons_data[x].flee: 0 to 300)
					i = (floorskill[floorskill_index].skill_lvl * 5000) / (mons_data[monster->class].flee / 30 + 1); // (5 sec * skill level) / flee value (1 to 11)
					if (mons_data[monster->class].boss == 1) { // 1/5 of normal trapped time.
						i = i / 5;
					}
					timer_data[monster->u.mons.timer]->interval = i + 50; // 50 msec after end of trap // TIMER_INTERVAL
					return 0; // stop moving
				}
				break;
			case 118: // SHOCKWAVE
				break;
			case 119: // SANDMAN
				break;
			case 120: // FLASHER
				break;
			case 121: // FREEZING
				break;
			case 122: // BLASTMINE
				break;
			case 123: // CLAYMORE
				break;
			}
			break;
		default:
			break;
		} // end of floorskill switch
	} // end of floorskill part
	return 1;
}

int mons_loot_search(short m, short c_x, short c_y, short *x, short *y, short range)
{
	struct flooritem_data *fitem;
	int i, j, k;
	int item_id;
	short item_x, item_y;
	item_id = 0;
	item_x = map_data[m].xs * 2;
	item_y = map_data[m].ys * 2;

	for (i = 2; i <= last_object_id; i++) {
		if (!object[i] || object[i]->type != BL_ITEM)
			continue;

		fitem = (struct flooritem_data *)object[i];
		if (fitem->m != m)
			continue;

		for (j = c_x - range; j <= c_x + range; j++) {
			if (j != fitem->x)
				continue;

			for (k = c_y - range; k <= c_y + range; k++) {
				if (k == fitem->y) {
					if (calc_distance(c_x, c_y, item_x, item_y) > calc_distance(c_x, c_y, j, k)) {
						item_id = i;
						item_x = j;
						item_y = k;
						if (c_x == item_x && c_y == item_y) {
							*x = item_x;
							*y = item_y;
							return item_id;
						}
					}
				}
			}
		}
	}
	if (item_id > 0) {
		*x = item_x;
		*y = item_y;
	}
	return item_id;
}

int mons_walk(int tid, unsigned int tick, int m, int n)
{
	unsigned char buf[256];
	int i; // for loops
	short x, y; // for current coordonates
	short dx, dy; // for move in x and y axis
	short bx, by; // for block calculation
	short next_x, next_y; // for next coordonates
	short loot_x, loot_y; // for loot destination
	int res;
	static int dirx[8] = {0, -1, -1, -1, 0, 1, 1, 1};
	static int diry[8] = {1, 1, 0, -1, -1, -1, 0, 1};
	int fd; // when attacking, for fd of player
	struct map_session_data *sd, sd_clean;
	struct flooritem_data *fitem; // for floor items
	struct npc_data *monster; // for speed up the function
	tid = 0;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP ||
	    !map_data[m].npc[n] || map_data[m].npc[n]->block.subtype != MONS)
		return 0;

	monster = map_data[m].npc[n];
	x = monster->x;
	y = monster->y;

	if (monster->u.mons.hp <= 0)
		return 0;

	if (monster->hidden)
		return 0;

	if (!mons_skill_floor_search(tick, m, n))
		return 0;

	if (!mons_data[monster->class].speed)
		return 0;

	if (monster->skilldata.effect & 0xdf || // 1: stoned
	    monster->skilldata.effect & 0x80 || // 2: frozen
	    monster->skilldata.effect & 0x20 || // 3: stunned
	    monster->skilldata.effect & 0x0f)   // 4: slept
		return 0;

	if (monster->skilldata.effect & 0x04 && (rand() % 4) == 0) // blind
		return 0;

	// Looting Code
	if (mons_data[monster->class].looter == 1) {
		// if monster is not attacking a player
		if (monster->u.mons.target_fd == 0) { // player fd that the monster attacks (-1 no player fd)

			// it was chasing an item
			if (monster->u.mons.lootdata.id != 0) {
				// if block is always an item
				if (object[monster->u.mons.lootdata.id] != NULL && object[monster->u.mons.lootdata.id]->type == BL_ITEM) {
					fitem = (struct flooritem_data *)object[monster->u.mons.lootdata.id]; // index of the item block
					// if the item is not always the same (or an other at same place)
					if (fitem->x != monster->u.mons.to_x || fitem->y != monster->u.mons.to_y) {
						// remove information about item
						monster->u.mons.lootdata.id = 0;
						// put like if the monster is arrived
						monster->u.mons.walkpath_len = 0;
					// if item is always the same and monster is arrived.
					} else if (monster->u.mons.walkpath_pos >= monster->u.mons.walkpath_len) {
						monster->u.mons.lootdata.looted_items[monster->u.mons.lootdata.loot_count] = fitem->item_data;
						monster->u.mons.lootdata.loot_count++;
						WBUFW(buf, 0) = 0xa1; // delete floor item
						WBUFL(buf, 2) = monster->u.mons.lootdata.id; // index of the item block
						mmo_map_sendarea_mxy(m, fitem->x, fitem->y, buf, packet_len_table[0xa1]);
						delete_object(monster->u.mons.lootdata.id); // index of the item block
						// remove reference about loot item
						monster->u.mons.lootdata.id = 0;
						// put like if the monster is arrived
						monster->u.mons.walkpath_len = 0;
					// else monster is not arrived, so continue move
					}
				// block is not more an item... remove information
				} else {
					monster->u.mons.lootdata.id = 0;
					// put like if the monster is arrived
					monster->u.mons.walkpath_len = 0;
				}
			}

			// if monster is actually not chasing a item and is not full
			if (monster->u.mons.lootdata.id == 0 && monster->u.mons.lootdata.loot_count < MAX_LOOT) {
				// we search another possible floor item
				res = mons_loot_search(m, x, y, &loot_x, &loot_y, BLOCK_SIZE *3 / 2);
				// we have found another floor item and a path exist to loot it
				if (res != 0) {
					// if same place
					if (x == loot_x && y == loot_y) {
						monster->u.mons.walkpath_len = 0;
						monster->u.mons.lootdata.id = res;
						monster->u.mons.to_x = loot_x;
						monster->u.mons.to_y = loot_y;
						timer_data[monster->u.mons.timer]->interval = monster->u.mons.speed; // TIMER_INTERVAL
						return 0;
					// we have found another floor item and a path exist to loot it
					} else if (search_path(&sd_clean, m, x, y, loot_x, loot_y, 0) == 0) { // 0: difficult search, 1: easy search (0 found, -1 not found)
						monster->u.mons.walkpath_len = sd_clean.walkpath_len;
						monster->u.mons.walkpath_pos = sd_clean.walkpath_pos;
						memcpy(monster->u.mons.walkpath, sd_clean.walkpath, sizeof(sd_clean.walkpath));
						monster->u.mons.lootdata.id = res;
						monster->u.mons.to_x = loot_x;
						monster->u.mons.to_y = loot_y;
					// no path found, do nothing
					}
				}
			}

		// if monster attack a player
		} else if (monster->u.mons.lootdata.id != 0) {
			// remove all code about loot item (path is calculated after in attacking code)
			monster->u.mons.lootdata.id = 0;
		}
	}
	if (!map_data[m].users && !monster->u.mons.lootdata.id) {
		timer_data[monster->u.mons.timer]->interval = 5000 + monster->u.mons.speed;
		return 0;
	}
	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && (sd = session[i]->session_data)) {
			if (in_range(sd->x, sd->y, map_data[m].npc[n]->x, map_data[m].npc[n]->y, 5)
			&& map_data[m].npc[n]->x != 0 && map_data[m].npc[n]->y != 0) {
				check_monster_skills(sd, i, m, n);
				if ((rand() % 90) == 10) {
					monster_say(i, map_data[m].npc[n]->id, map_data[m].npc[n]->class, "discovery");
				}
				if ((rand() % 90) == 10) {
					if (mons_data[map_data[m].npc[n]->class].mons_emotion.emotion > 0) {
						WBUFW(buf, 0) = 0x1aa;
						WBUFL(buf, 2) = map_data[m].npc[n]->id;
						WBUFL(buf, 6) = mons_data[map_data[m].npc[n]->class].mons_emotion.emotion;
						mmo_map_sendarea(sd->fd, buf, packet_len_table[0x1aa], 0);
					}
				}
			}
		}
	}
	// if monster is attacking a player
	if (monster->u.mons.target_fd != 0) { // player fd that the monster attacks (-1 no player fd)
		fd = monster->u.mons.target_fd;
		// if player is disconnect, or have changed of map, or is dead
		if (fd < 5 || session[fd] == NULL || (sd = session[fd]->session_data) == NULL || sd->mapno != m || sd->status.hp <= 0) {
			// stop attaking
			mmo_mons_attack_no(m, n);
			// no more move in the actual path
			monster->u.mons.walkpath_len = 0;
		// if monster is in range
		} else if (in_range(x, y, sd->x, sd->y, mons_data[monster->class].range)) { // 1: in range, 0 not in range
			// don't move
			timer_data[monster->u.mons.timer]->interval = monster->u.mons.speed; // TIMER_INTERVAL
			return 0;
		// if target coordonates are not good
		} else if (monster->u.mons.to_x != sd->x || monster->u.mons.to_y != sd->y) {
			// try to approach the target (we use in_range, because path can have lenght more important to avoid obstacles)
			if (in_range(x, y, sd->x, sd->y, BLOCK_SIZE * (AREA_SIZE + 1)) && // 1: in range, 0 not in range
			    search_path(&sd_clean, m, x, y, sd->x, sd->y, 0) == 0) { // 0: difficult search, 1: easy search (0 found, -1 not found)
				monster->u.mons.walkpath_len = sd_clean.walkpath_len;
				monster->u.mons.walkpath_pos = sd_clean.walkpath_pos;
				memcpy(monster->u.mons.walkpath, sd_clean.walkpath, sizeof(sd_clean.walkpath));
				monster->u.mons.to_x = sd->x;
				monster->u.mons.to_y = sd->y;
			// if we can not approach the target
			} else {
				// stop attaking
				mmo_mons_attack_no(m, n);
				// no more move in the actual path
				monster->u.mons.walkpath_len = 0;
			}
		// if destination is good, do nothing
		}
	}
	// ---------------- if monster is arrived

	// (looting at same place is impossible here (return in loot part)
	// attack in near of player is impossible (return in the attack part)
	if (monster->u.mons.walkpath_pos >= monster->u.mons.walkpath_len || monster->u.mons.walkpath_len == 0) {
		// We try to determinate a random point for next move
		for (i = 0; i < 5; i++) {
			// Set new target
			do {
				dx = rand() % (BLOCK_SIZE * 3 + 1) - (BLOCK_SIZE * 3 / 2); // from -(BLOCK_SIZE * 1.5) to +(BLOCK_SIZE * 1.5)
				dy = rand() % (BLOCK_SIZE * 3 + 1) - (BLOCK_SIZE * 3 / 2); // from -(BLOCK_SIZE * 1.5) to +(BLOCK_SIZE * 1.5)
				// monster walks on a shorter distance 50% of the time
				if (rand() % 2 == 0) {
					dx = dx * 2 / 3;
					dy = dy * 2 / 3;
				}
			} while (dx == 0 && dy == 0); // not same position
			if (search_path(&sd_clean, m, x, y, x + dx, y + dy, 1) == 0) { // 0: difficult search, 1: easy search (0 found, -1 not found)
				monster->u.mons.walkpath_len = sd_clean.walkpath_len;
				monster->u.mons.walkpath_pos = sd_clean.walkpath_pos;
				memcpy(monster->u.mons.walkpath, sd_clean.walkpath, sizeof(sd_clean.walkpath));
				monster->u.mons.to_x = x + dx;
				monster->u.mons.to_y = y + dy;
				// monster small sleeps before restart.
				timer_data[monster->u.mons.timer]->interval = rand() % 4000 + 1000 + monster->u.mons.speed * 2; // TIMER_INTERVAL
				return 0;
			}
		}
		// When i == 5 it means monster can not found a good path.
		if (i == 5) {
			// try again after a normal move wait
			timer_data[monster->u.mons.timer]->interval = monster->u.mons.speed; // TIMER_INTERVAL
			return 0;
		}
	}
	dx = dirx[(int)monster->u.mons.walkpath[monster->u.mons.walkpath_pos]];
	dy = diry[(int)monster->u.mons.walkpath[monster->u.mons.walkpath_pos]];
	next_x = x + dx;
	next_y = y + dy;

	/* Todo: add here floorskill that can stop monster (walls, etc.)? */

	monster->x = next_x;
	monster->y = next_y;
	monster->dir = monster->u.mons.walkpath[monster->u.mons.walkpath_pos];

	memset(buf, 0, packet_len_table[0x7b]);
	WBUFW(buf, 0) = 0x7b;
	WBUFL(buf, 2) = monster->id;
	WBUFW(buf, 6) = monster->u.mons.speed;
	WBUFW(buf, 12) = monster->option;
	WBUFW(buf, 14) = monster->class;
	WBUFL(buf, 22) = tick;
	WBUFW(buf, 36) = monster->dir;
	set_2pos(WBUFP(buf, 50), x, y, next_x, next_y);
	WBUFB(buf, 55) = 0;
	WBUFB(buf, 56) = 5;
	WBUFB(buf, 57) = 5;
	mmo_map_sendarea_mxy(m, next_x, next_y, buf, packet_len_table[0x7b]);

	if (monster->u.mons.walkpath[monster->u.mons.walkpath_pos] & 1)
		i = (int)monster->u.mons.speed * 14 / 10;

	else
		i = monster->u.mons.speed;

	if (monster->u.mons.target_fd == 0 && monster->u.mons.lootdata.id == 0)
		i *= 1.2;

	if (i < 10)
		i = 10;

	timer_data[monster->u.mons.timer]->interval = i;
	monster->u.mons.walkpath_pos++;

	if (x / BLOCK_SIZE != next_x / BLOCK_SIZE || y / BLOCK_SIZE != next_y / BLOCK_SIZE) {
		bx = x / BLOCK_SIZE;
		by = y / BLOCK_SIZE;
		WBUFW(buf, 0) = 0x80;
		WBUFL(buf, 2) = monster->id;
		WBUFB(buf, 6) = 0;
		if (next_x / BLOCK_SIZE != bx) {
			for (i = by - AREA_SIZE; i <= by + AREA_SIZE; i++) {
				if (i < 0)
					continue;

				else if (i >= map_data[m].bys)
					break;

				if (dx > 0 && bx - AREA_SIZE >= 0)
					mmo_map_sendblock(m, bx - AREA_SIZE, i, buf, packet_len_table[0x80], 0, 0);

				if (dx < 0 && bx + AREA_SIZE < map_data[m].bxs)
					mmo_map_sendblock(m, bx + AREA_SIZE, i, buf, packet_len_table[0x80], 0, 0);

			}
		}
		if (next_y / BLOCK_SIZE != by) {
			for (i = bx - AREA_SIZE; i <= bx + AREA_SIZE; i++) {
				if (i < 0)
					continue;

				else if (i >= map_data[m].bxs)
					break;

				if (dy > 0 && by - AREA_SIZE >= 0)
					mmo_map_sendblock(m, i, by - AREA_SIZE, buf, packet_len_table[0x80], 0, 0);

				if (dy < 0 && by + AREA_SIZE < map_data[m].bys)
					mmo_map_sendblock(m, i, by + AREA_SIZE, buf, packet_len_table[0x80], 0, 0);

			}
		}
		del_block(&monster->block);
		add_block(&monster->block, m, next_x, next_y);
	}
	return 0;
}

int spawn_monster(int class, int x, int y, int m, int fd)
{
	int len, i;
	unsigned char buf[254];

	if (m < 0 || m >= MAX_MAP_PER_SERVER)
		return 0;

	if (!map_data[m].npc[map_data[m].npc_num])
		map_data[m].npc[map_data[m].npc_num] = malloc(sizeof(struct npc_data));

	else
		map_data[m].npc[map_data[m].npc_num] = realloc(map_data[m].npc[map_data[m].npc_num], sizeof(struct npc_data));

	for (i = 0; i < 10000; i++) {
		set_monster_random_point(m, map_data[m].npc_num);
		if (in_range(map_data[m].npc[map_data[m].npc_num]->x, map_data[m].npc[map_data[m].npc_num]->y, x, y, 12))
			break;
	}
	if (i == 10000)
		return 0;

	map_data[m].npc[map_data[m].npc_num]->u.mons.to_x = map_data[m].npc[map_data[m].npc_num]->x;
	map_data[m].npc[map_data[m].npc_num]->u.mons.to_y = map_data[m].npc[map_data[m].npc_num]->y;
	map_data[m].npc[map_data[m].npc_num]->dir = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.speed = mons_data[class].speed;

	map_data[m].npc[map_data[m].npc_num]->u.mons.timer = add_timer(gettick() + 10, mons_walk, m, map_data[m].npc_num);
	timer_data[map_data[m].npc[map_data[m].npc_num]->u.mons.timer]->type = TIMER_INTERVAL;
	timer_data[map_data[m].npc[map_data[m].npc_num]->u.mons.timer]->interval = map_data[m].npc[map_data[m].npc_num]->u.mons.speed;

	map_data[m].npc[map_data[m].npc_num]->class = class;
	map_data[m].npc[map_data[m].npc_num]->id = npc_id++;

	strncpy(map_data[m].npc[map_data[m].npc_num]->name, mons_data[map_data[m].npc[map_data[m].npc_num]->class].name, 24);
	map_data[m].npc[map_data[m].npc_num]->u.mons.hp = mons_data[map_data[m].npc[map_data[m].npc_num]->class].max_hp;
	map_data[m].npc[map_data[m].npc_num]->u.mons.target_fd = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.attacktimer = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.attackdelay = mons_data[map_data[m].npc[map_data[m].npc_num]->class].adelay;
	map_data[m].npc[map_data[m].npc_num]->u.mons.summon = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.walkpath_len = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.walkpath_pos = 0;
	map_data[m].npc[map_data[m].npc_num]->option = 0;
	map_data[m].npc[map_data[m].npc_num]->hidden = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.steal = mons_data[map_data[m].npc[map_data[m].npc_num]->class].steal;
	map_data[m].npc[map_data[m].npc_num]->skilldata.fd = 0;
	map_data[m].npc[map_data[m].npc_num]->skilldata.skill_num = 0;
	map_data[m].npc[map_data[m].npc_num]->skilldata.effect = 00000000;
	map_data[m].npc[map_data[m].npc_num]->u.mons.lootdata.id = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.lootdata.loot_count = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.atk1 = mons_data[map_data[m].npc[map_data[m].npc_num]->class].atk1;
	map_data[m].npc[map_data[m].npc_num]->u.mons.atk2 = mons_data[map_data[m].npc[map_data[m].npc_num]->class].atk2;
	map_data[m].npc[map_data[m].npc_num]->u.mons.def1 = mons_data[map_data[m].npc[map_data[m].npc_num]->class].def1;
	map_data[m].npc[map_data[m].npc_num]->u.mons.def2 = mons_data[map_data[m].npc[map_data[m].npc_num]->class].def2;

	if (map_data[m].npc[map_data[m].npc_num]->class > 20) {
		len = mmo_map_set_npc007c(buf, npc_id - 1, class, map_data[m].npc[map_data[m].npc_num]->u.mons.to_x, map_data[m].npc[map_data[m].npc_num]->u.mons.to_y, map_data[m].npc[map_data[m].npc_num]->u.mons.speed, map_data[m].npc[map_data[m].npc_num]->option);
		if (len > 0)
			mmo_map_sendarea(fd, buf, len, 0);
	}
	else {
  		len = mmo_map_set_npc(buf, npc_id - 1, class, map_data[m].npc[map_data[m].npc_num]->u.mons.to_x, map_data[m].npc[map_data[m].npc_num]->u.mons.to_y, 0, map_data[m].npc[map_data[m].npc_num]->u.mons.speed, map_data[m].npc[map_data[m].npc_num]->option);
		if (len > 0)
			mmo_map_sendarea(fd, buf, len, 0);
	}
	map_data[m].npc[map_data[m].npc_num]->block.prev = NULL;
	map_data[m].npc[map_data[m].npc_num]->block.next = NULL;
	map_data[m].npc[map_data[m].npc_num]->block.subtype = MONS;
	add_block_npc(m, map_data[m].npc_num);
	map_data[m].npc_num++;
	return 1;
}

int spawn_to_pos(struct map_session_data *sd, int class, char *name, short x, short y, short m, int fd)
{
	int i, len;
	short dx, dy;
	unsigned char buf[256];

	if (m < 0 || m >= MAX_MAP_PER_SERVER)
		return 0;

	for (i = 0; i < 30; i++) {
		do {
			dx = rand() % 5 - 2;
			dy = rand() % 5 - 2;
		}
		while ((dx == 0 && dy == 0) ||
		        (sd->x + dx) < 0 || (sd->x + dx) > map_data[m].xs ||
		        (sd->y + dy) < 0 || (sd->y + dy) > map_data[m].ys ||
		        map_data[m].gat[sd->x + dx + (sd->y + dy) * map_data[m].xs] == 1 ||
		        map_data[m].gat[sd->x + dx + (sd->y + dy) * map_data[m].xs] == 5);
		if (in_range(sd->x + dx, sd->y + dy, x, y, 8))
			break;
	}
	if (i == 30)
		return 0;

	if (!map_data[m].npc[map_data[m].npc_num])
		map_data[m].npc[map_data[m].npc_num] = malloc(sizeof(struct npc_data));

	else
		map_data[m].npc[map_data[m].npc_num] = realloc(map_data[m].npc[map_data[m].npc_num], sizeof(struct npc_data));

	map_data[m].npc[map_data[m].npc_num]->x = sd->x + dx;
	map_data[m].npc[map_data[m].npc_num]->y = sd->y + dy;
	map_data[m].npc[map_data[m].npc_num]->dir = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.to_x = map_data[m].npc[map_data[m].npc_num]->x;
	map_data[m].npc[map_data[m].npc_num]->u.mons.to_y = map_data[m].npc[map_data[m].npc_num]->y;
	map_data[m].npc[map_data[m].npc_num]->u.mons.speed = mons_data[class].speed;

	map_data[m].npc[map_data[m].npc_num]->u.mons.timer = add_timer(gettick() + 10, mons_walk, m, map_data[m].npc_num);
	timer_data[map_data[m].npc[map_data[m].npc_num]->u.mons.timer]->type = TIMER_INTERVAL;
	timer_data[map_data[m].npc[map_data[m].npc_num]->u.mons.timer]->interval = map_data[m].npc[map_data[m].npc_num]->u.mons.speed;

	map_data[m].npc[map_data[m].npc_num]->class = class;
	map_data[m].npc[map_data[m].npc_num]->id = npc_id++;

	if (name == NULL)
		strncpy(map_data[m].npc[map_data[m].npc_num]->name, mons_data[map_data[m].npc[map_data[m].npc_num]->class].name, 24);

	else
		strcpy(map_data[m].npc[map_data[m].npc_num]->name, name);

	map_data[m].npc[map_data[m].npc_num]->u.mons.hp = mons_data[map_data[m].npc[map_data[m].npc_num]->class].max_hp;
	map_data[m].npc[map_data[m].npc_num]->u.mons.target_fd = fd;
	map_data[m].npc[map_data[m].npc_num]->u.mons.attackdelay = mons_data[map_data[m].npc[map_data[m].npc_num]->class].adelay;
	map_data[m].npc[map_data[m].npc_num]->u.mons.attacktimer = add_timer(gettick() + map_data[m].npc[map_data[m].npc_num]->u.mons.attackdelay, mmo_mons_attack_continue, m, map_data[m].npc_num);
	map_data[m].npc[map_data[m].npc_num]->u.mons.summon = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.walkpath_len = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.walkpath_pos = 0;
	map_data[m].npc[map_data[m].npc_num]->option = 0;
	map_data[m].npc[map_data[m].npc_num]->hidden = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.steal = mons_data[map_data[m].npc[map_data[m].npc_num]->class].steal;
	map_data[m].npc[map_data[m].npc_num]->skilldata.fd = 0;
	map_data[m].npc[map_data[m].npc_num]->skilldata.skill_num = 0;
	map_data[m].npc[map_data[m].npc_num]->skilldata.effect = 00000000;
	map_data[m].npc[map_data[m].npc_num]->u.mons.lootdata.id = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.lootdata.loot_count = 0;
	map_data[m].npc[map_data[m].npc_num]->u.mons.atk1 = mons_data[map_data[m].npc[map_data[m].npc_num]->class].atk1;
	map_data[m].npc[map_data[m].npc_num]->u.mons.atk2 = mons_data[map_data[m].npc[map_data[m].npc_num]->class].atk2;
	map_data[m].npc[map_data[m].npc_num]->u.mons.def1 = mons_data[map_data[m].npc[map_data[m].npc_num]->class].def1;
	map_data[m].npc[map_data[m].npc_num]->u.mons.def2 = mons_data[map_data[m].npc[map_data[m].npc_num]->class].def2;

	if (map_data[m].npc[map_data[m].npc_num]->class > 20) {
		len = mmo_map_set_npc007c(buf, npc_id - 1, class, map_data[m].npc[map_data[m].npc_num]->x, map_data[m].npc[map_data[m].npc_num]->y, map_data[m].npc[map_data[m].npc_num]->u.mons.speed, map_data[m].npc[map_data[m].npc_num]->option);
		if (len > 0)
			mmo_map_sendarea(fd, buf, len, 0);
	}
	else {
		len = mmo_map_set_npc(buf, npc_id - 1, class, map_data[m].npc[map_data[m].npc_num]->x, map_data[m].npc[map_data[m].npc_num]->y, 0, map_data[m].npc[map_data[m].npc_num]->u.mons.speed, map_data[m].npc[map_data[m].npc_num]->option);
		if (len > 0)
			mmo_map_sendarea(fd, buf, len, 0);
	}
	map_data[m].npc[map_data[m].npc_num]->block.prev = NULL;
	map_data[m].npc[map_data[m].npc_num]->block.next = NULL;
	map_data[m].npc[map_data[m].npc_num]->block.subtype = MONS;
	add_block_npc(m, map_data[m].npc_num);
	map_data[m].npc_num++;
	return 1;
}

int spawn_delay(int tid, unsigned int tick, int m, int n)
{
	unsigned char buf[256];

	tid = 0;
	tick = 0;
	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP ||
	    !map_data[m].npc[n] || map_data[m].npc[n]->block.subtype != MONS)
		return 0;

	set_monster_random_point(m, n);
	map_data[m].npc[n]->u.mons.to_x = map_data[m].npc[n]->x;
	map_data[m].npc[n]->u.mons.to_y = map_data[m].npc[n]->y;
	map_data[m].npc[n]->dir = 0;
	map_data[m].npc[n]->u.mons.speed = mons_data[map_data[m].npc[n]->class].speed;

	map_data[m].npc[n]->u.mons.hp = mons_data[map_data[m].npc[n]->class].max_hp;
	map_data[m].npc[n]->option = 0;
	map_data[m].npc[n]->hidden = 0;
	map_data[m].npc[n]->u.mons.attackdelay = mons_data[map_data[m].npc[n]->class].adelay;
	map_data[m].npc[n]->u.mons.steal = mons_data[map_data[m].npc[n]->class].steal;
	map_data[m].npc[n]->u.mons.atk1 = mons_data[map_data[m].npc[n]->class].atk1;
	map_data[m].npc[n]->u.mons.atk2 = mons_data[map_data[m].npc[n]->class].atk2;
	map_data[m].npc[n]->u.mons.def1 = mons_data[map_data[m].npc[n]->class].def1;
	map_data[m].npc[n]->u.mons.def2 = mons_data[map_data[m].npc[n]->class].def2;

	if (map_data[m].npc[n]->class > 20) {
		mmo_map_set_npc007c(buf, map_data[m].npc[n]->id, map_data[m].npc[n]->class, map_data[m].npc[n]->x, map_data[m].npc[n]->y, map_data[m].npc[n]->u.mons.speed, map_data[m].npc[n]->option);
		mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x7c]);
	}
	else {
		mmo_map_set_npc(buf, map_data[m].npc[n]->id, map_data[m].npc[n]->class, map_data[m].npc[n]->x, map_data[m].npc[n]->y, 0, map_data[m].npc[n]->u.mons.speed, map_data[m].npc[n]->option);
		mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x7c]);
	}
	del_block(&map_data[m].npc[n]->block);
	add_block_npc(m, n);
	return 0;
}

int remove_monsters(int m, short x, short y, int fd)
{
	int i, n;
	unsigned char buf[256];
	struct map_session_data *temp_sd;

	if (m >= 0 && m < MAX_MAP_PER_SERVER) {
		for (n = 0; n < MAX_NPC_PER_MAP; n++) {
			if (map_data[m].npc[n] != NULL && map_data[m].npc[n]->block.subtype == MONS) {
				if (in_range(x, y, map_data[m].npc[n]->x, map_data[m].npc[n]->y, 8)) {
					for (i = 5; i < FD_SETSIZE; i++) {
						if (session[i] && (temp_sd = session[i]->session_data) && temp_sd->attacktarget == map_data[m].npc[n]->id) {
							delete_timer(temp_sd->attacktimer, mmo_map_attack_continue);
							temp_sd->attacktimer = 0;
							temp_sd->attacktarget = 0;
						}
					}
					if (map_data[m].npc[n]->class > 20) {
						WBUFW(buf, 0) = 0x80;
						WBUFL(buf, 2) = map_data[m].npc[n]->id;
						WBUFB(buf, 6) = 1;
						mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
					}
					else {
						WBUFW(buf, 0) = 0x80;
						WBUFL(buf, 2) = map_data[m].npc[n]->id;
						WBUFB(buf, 6) = 2;
						mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
					}
					if (map_data[m].npc[n]->u.mons.attacktimer > 0) {
						delete_timer(map_data[m].npc[n]->u.mons.attacktimer, mmo_mons_attack_continue);
						map_data[m].npc[n]->u.mons.attacktimer = 0;
					}
					map_data[m].npc[n]->u.mons.hp = 0;
					map_data[m].npc[n]->u.mons.target_fd = 0;
					map_data[m].npc[n]->u.mons.walkpath_len = 0;
					map_data[m].npc[n]->u.mons.walkpath_pos = 0;
					map_data[m].npc[n]->u.mons.speed = 0;
					map_data[m].npc[n]->skilldata.fd = 0;
					map_data[m].npc[n]->skilldata.skill_num = 0;
					map_data[m].npc[n]->skilldata.effect = 00000000;
					map_data[m].npc[n]->u.mons.lootdata.id = 0;
					map_data[m].npc[n]->u.mons.lootdata.loot_count = 0;
					set_monster_no_point(m, n);
					if (strncmp(map_data[m].npc[n]->name, mons_data[map_data[m].npc[n]->class].name, 24) != 0) {
						if (mons_data[map_data[m].npc[n]->class].boss == 1) {
							add_timer(gettick() + 60 * 60000, spawn_delay, m, n);
							timer_data[map_data[m].npc[n]->u.mons.timer]->tick = gettick() + 60 + 500;
						}
						else if (mons_data[map_data[m].npc[n]->class].boss == 2) {
							add_timer(gettick() + 45 * 60000, spawn_delay, m, n);
							timer_data[map_data[m].npc[n]->u.mons.timer]->tick = gettick() + 45 + 500;
						}
						else {
							add_timer(gettick() + 60000, spawn_delay, m, n);
							timer_data[map_data[m].npc[n]->u.mons.timer]->tick = gettick() + 1 + 500;
						}
					}
					else {
						delete_timer(map_data[m].npc[n]->u.mons.timer, mons_walk);
						map_data[m].npc[n]->u.mons.timer = 0;
						del_block(&map_data[m].npc[n]->block);
					}
				}
			}
		}
	}
	return 0;
}

int boss_monster(int m, int n)
{
	if ( map_data[m].npc[n]->class != 1038 && /* Osiris */
		map_data[m].npc[n]->class != 1039 && /* Baphomet */
		map_data[m].npc[n]->class != 1046 && /* Doppelganger */
		map_data[m].npc[n]->class != 1086 && /* Golden Bug */
		map_data[m].npc[n]->class != 1087 && /* Ork Hero */
		map_data[m].npc[n]->class != 1112 && /* Drake */
		map_data[m].npc[n]->class != 1115 && /* Eddga */
		map_data[m].npc[n]->class != 1150 && /* Moonlight */
		map_data[m].npc[n]->class != 1059 && /* Mistress */
		map_data[m].npc[n]->class != 1159 && /* Phreeoni */
		map_data[m].npc[n]->class != 1147 && /* Maya */
		map_data[m].npc[n]->class != 1190 && /* Orc Lord */
		map_data[m].npc[n]->class != 1283 && /* Chimera */
		map_data[m].npc[n]->class != 1272 && /* Dark Lord */
		map_data[m].npc[n]->class != 1389 && /* Dracula */
		map_data[m].npc[n]->class != 1259 && /* Griffon */
		map_data[m].npc[n]->class != 1262 && /* Mutant Dragonoid */
		map_data[m].npc[n]->class != 1157 && /* Pharao */
		map_data[m].npc[n]->class != 1312 && /* Turtle General */
		map_data[m].npc[n]->class != 1022 && /* Werewolf */
		map_data[m].npc[n]->class != 1200)   /* Zherlthsh */
		return 0;

	else
		return 1;
}

int init_boss_miniboss(int class)
{
	int type = 0;

	if ((class == 1039) || (class == 1283) || (class == 1272) || (class == 1046) || (class == 1389) ||
	    (class == 1181) || (class == 1112) || (class == 1115) || (class == 1086) || (class == 1373) ||
	    (class == 1147) || (class == 1059) || (class == 1150) || (class == 1087) || (class == 1190) ||
	    (class == 1038) || (class == 1157) || (class == 1159) || (class == 1251) || (class == 1312))
		type = 1;

	else if ((class == 1096) || (class == 1388) || (class == 1268) || (class == 1214) || (class == 1293) ||
	         (class == 1091) || (class == 1093) || (class == 1120) || (class == 1303) || (class == 1186) ||
	         (class == 1299) || (class == 1259) || (class == 1294) || (class == 1296) || (class == 1090) ||
		   (class == 1262) || (class == 1308) || (class == 1089) || (class == 1092) || (class == 1088))
		type = 2;

	return type;
}

int monster_emotion(char *mons_name)
{
	char name[24], line[256];
	int var;
	FILE *fp = fopen("data/databases/mons_emotion_db.txt", "r");
	if (fp) {
		while (fgets(line, 255, fp)) {
			if ((line[0] == '/') && (line[1] == '/'))
				continue;

			sscanf(line, "%[^\t]\t%d", name, &var);
			if (strncmp(mons_name, name, 24) == 0) {
				fclose(fp);
				return var;
			}
		}
	}
	else
		printf("**Error: Cannot load mons_emotion_db.txt!**\n");

	return -1;
}

int search_monster_id(char *name)
{
	int j = 0;

	for (j = 1000; j < 1392; j++) {
		if (strncmp(mons_data[j].name, name, 24) == 0)
			return j;

		else if (strncmp(strtolower(mons_data[j].name), name, 24) == 0)
			return j;

		else if (strncmp(mons_data[j].real_name, name, 24) == 0)
			return j;

		else if (strncmp(strtolower(mons_data[j].real_name), name, 24) == 0)
			return j;

	}
	return -1;
}

int *return_npc_current_id(void)
{
	return &npc_id;
}

int npc_add_npc_id(void)
{
	npc_id++;
	return (npc_id - 1);
}

int read_npc_fromdir(char *dirname)
{
	int i = 0;
	char temp[24];
	char dirnext[48];
	DIR *dirstream;
	struct dirent *dirfile;

	if ((dirstream = opendir(dirname)) == NULL)
		printf("**Error: %s directory doesn't exist!**\n", dirname);

	else {
		while ((dirfile = readdir(dirstream)) != NULL) {
			if ((strstr(dirfile->d_name, ".txt") != NULL) || (strstr(dirfile->d_name, ".npc")) != NULL) {
				i++;
				sprintf(temp,"%s/%s", dirname, dirfile->d_name);
				npc_txt[npc_txt_num] = malloc(strlen(temp) + 1);
				strcpy(npc_txt[npc_txt_num++], temp);
			}
			else {
				if ((strncmp(dirfile->d_name, ".", 1) != 0) && (strncmp(dirfile->d_name, "..", 2) != 0)){
					sprintf(dirnext,"%s/%s", dirname, dirfile->d_name);
					i = i + read_npc_fromdir(dirnext);
				}
			}
		}
	}
	closedir(dirstream);
	return i;
}

/*======================================
 *	NPC: Load Data
 *--------------------------------------
 */

void read_npcdata(void)
{
	int i, npc_txt_count;
	char line[2040];
	FILE *fp;

	pet_store_init_npc_id(&npc_id);
	printf("Loading Databases...          ");
	fp = fopen("data/databases/monster_db.txt", "r");
	if (fp) {
		while (fgets(line, 2040, fp)) {
			int  class, lv, max_hp, base_exp, job_exp;
			int  range, atk1, atk2, def1, def2, mdef1, mdef2, hit, flee;
			int  scale, race, ele, mode, speed, adelay, amotion, dmotion;
			char name[128], JName[256], dropitem[1024], mvpdropitem[512], *p;

			if (sscanf(line,"%d,%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\t]%[^\n]",
					&class,name,JName,&lv,&max_hp,&base_exp,&job_exp,
					&range,&atk1,&atk2,&def1,&def2,&mdef1,&mdef2,&hit,&flee,
					&scale,&race,&ele,&mode,&speed,&adelay,&amotion,&dmotion,
					dropitem,mvpdropitem) != 26) {
				if (strstr(line,"//") != line)
					printf("Problem Loading Monster: %d\n", class);

				continue;
			}
			if (class >= MAX_MONS)
				continue;

			if (class < 1000 || ele < 20) {
				if (speed != 0)
					printf("Error in monster_db.txt: monster %d (%s) - speed %d (must be 0).\n", class, name, speed);

				if (amotion != 0)
					printf("Error in monster_db.txt: monster %d (%s) - amotion %d (must be 0).\n", class, name, amotion);

				if (dmotion != 0)
					printf("Error in monster_db.txt: monster %d (%s) - dmotion %d (must be 0).\n", class, name, dmotion);

				if (range != 0)
					printf("Error in monster_db.txt: monster %d (%s) - range %d (must be 0).\n", class, name, range);

				if (adelay != 0)
					printf("Error in monster_db.txt: monster %d (%s) - adelay %d (must be 0).\n", class, name, adelay);

				if (atk1 != 0)
					printf("Error in monster_db.txt: monster %d (%s) - atk1 %d (must be 0).\n", class, name, atk1);

				if (atk2 != 0)
					printf("Error in monster_db.txt: monster %d (%s) - atk2 %d (must be 0).\n", class, name, atk2);

			}
			else {
				if (speed == 0) {
					if (amotion != 1)
						printf("Error in monster_db.txt: monster %d (%s) - speed 0 and amotion %d (must be 1).\n", class, name, amotion);

					if (dmotion != 1)
						printf("Error in monster_db.txt: monster %d (%s) - speed 0 and dmotion %d (must be 1).\n", class, name, dmotion);

				}
				else {
					if (amotion <= 10)
						printf("Error in monster_db.txt: monster %d (%s) - speed %d and amotion %d (must be upper than 9).\n", class, name, speed, amotion);

					if (dmotion <= 10)
						printf("Error in monster_db.txt: monster %d (%s) - speed %d and dmotion %d (must be upper than 9).\n", class, name, speed, dmotion);

				}
				if (range == 0) {
					if (atk1 != 0)
						printf("Error in monster_db.txt: monster %d (%s) - range 0 and atk1 %d (must be 0).\n", class, name, atk1);

					if (atk2 != 0)
						printf("Error in monster_db.txt: monster %d (%s) - range 0 and atk2 %d (must be 0).\n", class, name, atk2);

					if (adelay != 1)
						printf("Error in monster_db.txt: monster %d (%s) - range 0 and adelay %d (must be 1).\n", class, name, adelay);

				}
				else {
					if (atk1 == 0)
						printf("Error in monster_db.txt: monster %d (%s) - range %d and atk1 0 (must be upper than 0).\n", class, name, range);

					if (adelay <= 10)
						printf("Error in monster_db.txt: monster %d (%s) - range %d and adelay %d (must be upper than 9).\n", class, name, range, adelay);

				}
			}
			mons_data[class].class = class;
			strncpy(mons_data[class].name, name, 24);
			strncpy(mons_data[class].real_name, JName, 24);
			mons_data[class].max_hp = max_hp;
			mons_data[class].base_exp = base_exp * base_mult;
			mons_data[class].job_exp = job_exp * job_mult;
			mons_data[class].lv = lv;
			mons_data[class].range = range;
			mons_data[class].atk1 = atk1;
			mons_data[class].atk2 = atk2;
			mons_data[class].def1 = def1;
			mons_data[class].def2 = def2;
			mons_data[class].mdef1 = mdef1;
			mons_data[class].mdef2 = mdef2;
			mons_data[class].hit = hit;
			mons_data[class].flee = flee;
			mons_data[class].scale = scale;
			mons_data[class].race = race;
			mons_data[class].ele = ele;
			mons_data[class].mode = mode;
			switch (mons_data[class].mode) {
			case 1:
				mons_data[class].aggressive = 1;
				break;
			case 2:
				mons_data[class].aggressive = 2;
				break;
			case 3:
				mons_data[class].aggressive = 3;
				break;
			case 4:
				mons_data[class].aggressive = 4;
				break;
			case 5:
				mons_data[class].looter = 1;
				break;
			case 6:
				mons_data[class].assist = 1;
				break;
			case 7:
				mons_data[class].cast_sense = 1;
				break;
			case 8:
				mons_data[class].looter = 1;
				mons_data[class].assist = 1;
				break;
			case 9:
				mons_data[class].looter = 1;
				mons_data[class].cast_sense = 1;
				break;
			case 10:
				mons_data[class].assist = 1;
				mons_data[class].cast_sense = 1;
				break;
			case 11:
				mons_data[class].aggressive = 1;
				mons_data[class].looter = 1;
				break;
			case 12:
				mons_data[class].aggressive = 1;
				mons_data[class].assist = 1;
				break;
			case 13:
				mons_data[class].aggressive = 1;
				mons_data[class].cast_sense = 1;
				break;
			case 14:
				mons_data[class].aggressive = 1;
				mons_data[class].looter = 1;
				mons_data[class].assist = 1;
				break;
			case 15:
				mons_data[class].aggressive = 1;
				mons_data[class].looter = 1;
				mons_data[class].cast_sense = 1;
				break;
			case 16:
				mons_data[class].aggressive = 1;
				mons_data[class].assist = 1;
				mons_data[class].cast_sense = 1;
				break;
			case 17:
				mons_data[class].aggressive = 1;
				mons_data[class].looter = 1;
				mons_data[class].assist = 1;
				mons_data[class].cast_sense = 1;
				break;
			case 18:
				mons_data[class].looter = 1;
				mons_data[class].assist = 1;
				mons_data[class].cast_sense = 1;
				break;
			}
			switch (speed) {
			case 0:
				mons_data[class].speed = 0;
				break;
			case 1:
				mons_data[class].speed = DEFAULT_MONSTER_WALK_SPEED * 2;
				break;
			case 2:
				mons_data[class].speed = DEFAULT_MONSTER_WALK_SPEED * 3 / 2;
				break;
			case 4:
				mons_data[class].speed = DEFAULT_MONSTER_WALK_SPEED * 3 / 4;
				break;
			case 5:
				mons_data[class].speed = DEFAULT_MONSTER_WALK_SPEED / 2;
				break;
			default:
				mons_data[class].speed = DEFAULT_MONSTER_WALK_SPEED;
				break;
			};
			mons_data[class].adelay = adelay;
			mons_data[class].amotion = amotion;
			mons_data[class].dmotion = dmotion;
			mons_data[class].boss = init_boss_miniboss(class);
			if (mons_data[class].boss == 1 || mons_data[class].boss == 2)
				mons_data[class].steal = 1;

			else
				mons_data[class].steal = 0;

			mons_data[class].mons_emotion.emotion = monster_emotion(name);
			for (p = dropitem, i = 0; p && i < 16; i++, p = strchr(p, ',')) {
				int nameid, value;
				if (*p == ',')
					p++;

				if (sscanf(p, "%d:%d", &nameid, &value) != 2)
					break;

				mons_data[class].dropitem[i].nameid = nameid;
				mons_data[class].dropitem[i].p = value * drop_mult;
			}
			for (p = mvpdropitem, i = 0; p && i < 5; i++, p = strchr(p, ',')) {
				int nameid, value;
				if (*p == ',')
					p++;

				if (sscanf(p, "%d:%d", &nameid, &value) != 2)
					break;

				mons_data[class].mvpdropitem[i].nameid = nameid;
				mons_data[class].mvpdropitem[i].p = value * drop_mult;
			}
		}
		fclose(fp);
	}
	else
		printf("**ERROR: Cannot load monsters_db.txt!**\n");

	fp = fopen("data/databases/mons_say_db.txt","r");
	if (fp) {
		while (fgets(line, 2040, fp)) {
			int class = 0;
			char tag[24], mons_name[24];
			char temp[48], temp0[80], temp1[80];
			strncpy(tag, "", 24);
			strncpy(temp, "", 48);
			strncpy(temp0, "", 80);
			strncpy(temp1, "", 80);

			if ((sscanf(line, "%[^=]=%s", tag, temp) == 2) && (strcmp(tag,"begin_mons") == 0)) {
				strncpy(temp1, temp, (strlen(temp)-1));
				strncpy(mons_name, temp1, 24);
				class = search_monster_id(temp1);
				if (class > -1) {
					while (fgets(line,2040,fp)) {
						if (sscanf(line, "%[^=]=%[^\n]", tag, temp0) == 2) {
							if (strcmp(tag, "\tdiscovery_1") == 0)
								strncpy(mons_data[class].mons_say.discovery_1, temp0, (strlen(temp0)-1));

							if (strcmp(tag, "\tdiscovery_2") == 0)
								strncpy(mons_data[class].mons_say.discovery_2, temp0, (strlen(temp0)-1));

							if (strcmp(tag, "\tdiscovery_3") == 0)
								strncpy(mons_data[class].mons_say.discovery_3, temp0, (strlen(temp0)-1));

							if (strcmp(tag, "\tattack_1") == 0)
								strncpy(mons_data[class].mons_say.attack_1, temp0, (strlen(temp0)-1));

							if (strcmp(tag, "\tattack_2") == 0)
								strncpy(mons_data[class].mons_say.attack_2, temp0, (strlen(temp0)-1));

							if (strcmp(tag, "\tattack_3") == 0)
								strncpy(mons_data[class].mons_say.attack_3, temp0, (strlen(temp0)-1));

							if (strcmp(tag, "\thp50_1") == 0)
								strncpy(mons_data[class].mons_say.hp50_1, temp0, (strlen(temp0)-1));

							if (strcmp(tag, "\thp50_2") == 0)
								strncpy(mons_data[class].mons_say.hp50_2, temp0, (strlen(temp0)-1));

							if (strcmp(tag, "\thp25_2") == 0)
								strncpy(mons_data[class].mons_say.hp25_2, temp0, (strlen(temp0)-1));

							if (strcmp(tag, "\thp25_1") == 0)
								strncpy(mons_data[class].mons_say.hp25_1, temp0, (strlen(temp0)-1));

						}
						if ((sscanf(line, "%s", tag) == 1) && (strcmp(tag, "}") == 0))
							break;

					}
				}
			}
		}
		fclose(fp);
	}
	else
		printf("**ERROR: Cannot load mons_say_db.txt!**\n");

	printf("Done\n");
	printf("Loading Scripts Data...       ");
	for (npc_txt_count = 0; npc_txt_count < npc_txt_num; npc_txt_count++) {
		fp = fopen(npc_txt[npc_txt_count], "r");
		if (!fp) {
			printf("ERROR : Cannot load %s!\n", npc_txt[npc_txt_count]);
			continue;
		}
		while (fgets(line, 1020, fp)) {
			char mapname[256], w1[1024], w2[1024], w3[1024], w4[1024];
			int x, y, dir;
			if ((sscanf(line, "%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]", w1, w2, w3, w4) != 4) || (sscanf(w1,"%[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4))
				continue;

			for (i = 0; map[i][0]; i++) {
				if (strncmp(map[i], mapname, 16) == 0)
					break;
			}
			if (map[i][0] == 0)
				continue;

			if (strcmp(w2, "warp") == 0) {
				int j, k;
				short xs, ys;
				struct npc_data *warp;
				map_data[i].npc[map_data[i].npc_num] = malloc(sizeof(struct npc_data));
				warp = map_data[i].npc[map_data[i].npc_num];
				warp->m = i;
				warp->x = x;
				warp->y = y;
				warp->dir = 0;
				warp->option = 0;
				memcpy(warp->name, w3, 24);

				warp->class = WARP_CLASS;
				warp->id = npc_id++;
				sscanf(w4, "%hd,%hd,%[^,],%hd,%hd", &xs, &ys, warp->u.warp.name, &warp->u.warp.to_x, &warp->u.warp.to_y);
				xs = ((xs + 2) / 2);
				if (xs < 1)
					xs = 1;

				ys = ((ys + 2) / 2);
				if (ys < 1)
					ys = 1;

				warp->u.warp.x_range = xs;
				warp->u.warp.y_range = ys;
				for (j = y - ys; j <= y + ys; j++) {
					if (j < 0)
						continue;

					else if (j >= map_data[i].ys)
						break;

					for (k = x - xs; k <= x + xs; k++) {
						unsigned char *p;
						if (k < 0)
							continue;

						else if (k >= map_data[i].xs)
							break;

						p = &map_data[i].gat[k + j * map_data[i].xs];
						if (*p == 1 || *p == 5)
							continue;

						*p |= 0x80;
					}
				}
				warp->block.prev = NULL;
				warp->block.next = NULL;
				warp->block.subtype = WARP;
				add_block_npc(i, map_data[i].npc_num);
				map_data[i].npc_num++;
			}
			else if (strcmp(w2, "monster") == 0) {
				short j, num;
				int class;
				struct npc_data *monster;
				sscanf(w4, "%d,%hd", &class, &num);
				for (j = 0; j < num; j++) {
					map_data[i].npc[map_data[i].npc_num] = malloc(sizeof(struct npc_data));
					monster = map_data[i].npc[map_data[i].npc_num];
					if (num == 1 && x && y) {
						monster->x = x;
						monster->y = y;
					}
					else
						set_monster_random_point(i, map_data[i].npc_num);

					monster->u.mons.to_x = monster->x;
					monster->u.mons.to_y = monster->y;
					monster->dir = 0;
					monster->u.mons.speed = mons_data[class].speed;

					monster->u.mons.timer = add_timer(gettick() + 10, mons_walk, i, map_data[i].npc_num);
					timer_data[monster->u.mons.timer]->type = TIMER_INTERVAL;
					timer_data[monster->u.mons.timer]->interval = monster->u.mons.speed;

					monster->class = class;
					monster->id = npc_id++;

					memcpy(monster->name, w3, 24);
					monster->u.mons.hp = mons_data[class].max_hp;
					monster->u.mons.target_fd = 0;
					monster->u.mons.attacktimer = 0;
					monster->u.mons.attackdelay = mons_data[class].adelay;
					monster->u.mons.summon = 0;
					monster->u.mons.walkpath_len = 0;
					monster->u.mons.walkpath_pos = 0;
					monster->option = 0;
					monster->hidden = 0;
					monster->u.mons.steal = mons_data[class].steal;
					monster->skilldata.fd = 0;
					monster->skilldata.skill_num = 0;
					monster->skilldata.effect = 00000000;
					monster->u.mons.lootdata.id = 0;
					monster->u.mons.lootdata.loot_count = 0;
					monster->u.mons.atk1 = mons_data[class].atk1;
					monster->u.mons.atk2 = mons_data[class].atk2;
					monster->u.mons.def1 = mons_data[class].def1;
					monster->u.mons.def2 = mons_data[class].def2;
					monster->block.prev = NULL;
					monster->block.next = NULL;
					monster->block.subtype = MONS;
					add_block_npc(i, map_data[i].npc_num);
					map_data[i].npc_num++;
				}
			}
			else if (strcmp(w2, "shop") == 0) {
				char *p;
				int max = 32, pos = 0;
				struct npc_item_list *item_list;
				struct npc_data *shop;
				map_data[i].npc[map_data[i].npc_num] = malloc(sizeof(struct npc_data));
				shop = map_data[i].npc[map_data[i].npc_num];
				shop->m = i;
				shop->x = x;
				shop->y = y;
				shop->dir = dir;
				shop->option = 0;
				memcpy(shop->name, w3, 24);
				shop->class = atoi(w4);
				shop->id = npc_id++;
				item_list = malloc(sizeof(item_list[0]) * (max + 1));
				p = strchr(w4, ',');
				while (p && pos < max) {
					int nameid, value;
					p++;
					if (sscanf(p, "%d:%d", &nameid, &value) != 2)
						break;

					item_list[pos].nameid = nameid;
					item_list[pos].value = value;
					pos++;
					p = strchr(p, ',');
				}
				item_list = realloc(item_list, sizeof(item_list[0]) * (pos + 1));
				item_list[pos].nameid = 0;
				shop->u.shop_item = item_list;
				shop->block.prev = NULL;
				shop->block.next = NULL;
				shop->block.subtype = SHOP;
				add_block_npc(i, map_data[i].npc_num);
				map_data[i].npc_num++;
			}
			else if (strcmp(w2, "script") == 0) {
				char *buf;
				int size = 65536, j;
				struct npc_data *script;
				map_data[i].npc[map_data[i].npc_num] = malloc(sizeof(struct npc_data));
				script = map_data[i].npc[map_data[i].npc_num];
				script->m = i;
				script->x = x;
				script->y = y;
				script->dir = dir;
				script->option = 0;
				memcpy(script->name, w3, 24);

				script->class = atoi(w4);
				script->id = npc_id++;
				buf = malloc(size);
				if (strchr(w4, '{'))
					strcpy(buf, strchr(w4, '{'));

				else
					buf[0] = 0;

				while (1) {
					for (j = strlen(buf) - 1; j >= 0 && isspace(buf[j]); j--);
					if (j >= 0 && buf[j] == '}')
						break;

					fgets(line, 1023, fp);
					if (feof(fp))
						break;

					if (strlen(buf) + strlen(line) >= (unsigned)size) {
						size += 65536;
						buf = realloc(buf, size);
					}
					if (strchr(buf, '{') == NULL) {
						if (strchr(line, '{'))
							strcpy(buf, strchr(line, '{'));
					}
					else
						strcat(buf, line);

				}
				script->u.script = parse_script(buf);
				free(buf);
				script->block.prev = NULL;
				script->block.next = NULL;
				script->block.subtype = SCRIPT;
				add_block_npc(i, map_data[i].npc_num);
				map_data[i].npc_num++;
			}
		}
		fclose(fp);
	}
	printf("Done\n");
}
