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
#include "mmo.h"
#include "map_core.h"
#include "npc.h"
#include "item.h"
#include "itemdb.h"
#include "script.h"
#include "pet.h"
#include "skill.h"
#include "skill_db.h"
#include "chat.h"
#include "element_damage.h"
#include "monster_skills.h"

extern char map[][16];
extern int base_mult, job_mult, drop_mult;
extern int last_object_id;

int DC_table [] = { 100, 93, 91, 89, 87, 85, 83, 81, 79, 77, 76 };
int OC_table [] = { 100, 107, 109, 111, 113, 115, 117, 119, 121, 123, 124 };
int npc_txt_num = 0;
char *npc_txt[256];
static int npc_id = MAX_OBJECTS;

int *return_npc_current_id(void)
{
	return &npc_id;
}

int npc_add_npc_id(void)
{
	npc_id++;
	return (npc_id - 1);
}

int mmo_map_npc_say(unsigned char* buf, unsigned long id, char *string)
{
	WBUFW(buf, 0) = 0xb4;
	WBUFW(buf, 2) = strlen(string) + 9;
	WBUFL(buf, 4) = id;
	memcpy(WBUFP(buf, 8), string, strlen(string) + 1);
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
	memcpy(WBUFP(buf, 8), string, strlen(string) + 1);
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
	if (sd->status.skill[37-1].lv > 0) {
		if(sd->status.skill[37-1].lv < 10)
			DCPercent = 95 - 2 * sd->status.skill[37-1].lv;

		else
			DCPercent = 76;

	}
	if (sd->status.skill[224-1].lv > 0)
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
		for (n = 0; n < map_data[sd->mapno].npc_num; n++)
			if (map_data[sd->mapno].npc[n]->id == npc_id)
				break;


		if ((n == map_data[sd->mapno].npc_num) 
		|| (map_data[sd->mapno].npc[n]->block.type != BL_NPC))
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

	for (n = 0; n < map_data[sd->mapno].npc_num; n++)
		if (map_data[sd->mapno].npc[n]->id == npc_id)
			break;


	if ((n == map_data[sd->mapno].npc_num) 
	|| (map_data[sd->mapno].npc[n]->block.type != BL_NPC)
	|| (map_data[sd->mapno].npc[n]->block.subtype != SHOP) 
	|| (sd->npc_id != npc_id))
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

	for (n = 0; n < map_data[sd->mapno].npc_num; n++)
		if (map_data[sd->mapno].npc[n]->id == sd->npc_id)
			break;


	if ((n == map_data[sd->mapno].npc_num) 
	|| (map_data[sd->mapno].npc[n]->block.type != BL_NPC)
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
		for (j = 0; map_data[sd->mapno].npc[n]->u.shop_item[j].nameid; j++)
			if (map_data[sd->mapno].npc[n]->u.shop_item[j].nameid == RBUFW(list, i * 4 + 2))
				break;


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

int set_monster_random_point(short m, int n)
{
	short x, y;
	struct npc_data *monster;

	if (!(monster = map_data[m].npc[n]))
		return 0;

	do {
		x = rand() % (map_data[m].xs - 2) + 1;
		y = rand() % (map_data[m].ys - 2) + 1;
	}
	while (map_data[m].gat[x + y * map_data[m].xs] == 1 || map_data[m].gat[x + y * map_data[m].xs] == 5);

	monster->dir = 0;
	monster->u.mons.to_x = monster->x = x;
	monster->u.mons.to_y = monster->y = y;
	return 0;
}

int set_monster_no_point(short m, int n)
{
	struct npc_data *monster;

	if (!(monster = map_data[m].npc[n]))
		return 0;

	monster->dir = 0;
	monster->u.mons.to_x = monster->x = 0;
	monster->u.mons.to_y = monster->y = 0;
	return 0;
}

static int mons_skill_floor_search(unsigned int tick, struct npc_data *monster, short m, int n)
{
	int i, damage = 0;
	unsigned char buf[256];
	short x = monster->x;
	short y = monster->y;
	int floorskill_index = search_floorskill_index(m, x, y);
	if (floorskill_index > -1) {
		switch (floorskill[floorskill_index].type) {
		case FS_SANCT:
			if ((mons_data[monster->class].ele % 10 == 9) || (mons_data[monster->class].ele % 10 == 7)) {
				damage = skill_calc_damage(floorskill[floorskill_index].owner_fd, floorskill[floorskill_index].skill_num, floorskill[floorskill_index].skill_lvl, monster->id);

				WBUFW(buf, 0) = 0x114;
				WBUFW(buf, 2) = floorskill[floorskill_index].skill_num;
				WBUFL(buf, 4) = monster->id;
				WBUFL(buf, 8) = monster->id;
				WBUFL(buf, 12) = tick;
				WBUFL(buf, 16) = 100;
				WBUFL(buf, 20) = monster->u.mons.speed;
				WBUFW(buf, 24) = damage;
				WBUFW(buf, 26) = floorskill[floorskill_index].skill_lvl;
				WBUFW(buf, 28) = 1;
				WBUFB(buf, 30) = 6;
				mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x114]);

				skill_do_damage(floorskill[floorskill_index].owner_fd, damage, monster->id, tick, 0);
				if (monster->u.mons.hp <= 0) {
					monster->u.mons.hp = 0;
					return 0;
				}
			}
			break;
		case FS_MAGNUS:
			if (floorskill[floorskill_index].count > 0 && ((mons_data[monster->class].ele % 10 == 9) || (mons_data[monster->class].ele % 10 == 7))) {
				damage = skill_calc_damage(floorskill[floorskill_index].owner_fd, floorskill[floorskill_index].skill_num, floorskill[floorskill_index].skill_lvl, monster->id);

				WBUFW(buf, 0) = 0x114;
				WBUFW(buf, 2) = floorskill[floorskill_index].skill_num;
				WBUFL(buf, 4) = monster->id;
				WBUFL(buf, 8) = monster->id;
				WBUFL(buf, 12) = tick;
				WBUFL(buf, 16) = 100;
				WBUFL(buf, 20) = monster->u.mons.speed;
				WBUFW(buf, 24) = damage;
				WBUFW(buf, 26) = floorskill[floorskill_index].skill_lvl;
				WBUFW(buf, 28) = 1;
				WBUFB(buf, 30) = 6;
				mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x114]);

				skill_do_damage(floorskill[floorskill_index].owner_fd, damage, monster->id, tick, 0);
				floorskill[floorskill_index].count--;
				if (floorskill[floorskill_index].count == 0) {
					if (floorskill[floorskill_index].timer > 0)
						delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

					remove_floorskill(0, tick, 0, floorskill_index);
				}
				if (monster->u.mons.hp <= 0) {
					monster->u.mons.hp = 0;
					return 0;
				}
			}
			break;
		case FS_FWALL:
			if (floorskill[floorskill_index].count > 0) {
				damage = skill_calc_damage(floorskill[floorskill_index].owner_fd, floorskill[floorskill_index].skill_num, floorskill[floorskill_index].skill_lvl, monster->id);

				WBUFW(buf, 0) = 0x114;
				WBUFW(buf, 2) = floorskill[floorskill_index].skill_num;
				WBUFL(buf, 4) = monster->id;
				WBUFL(buf, 8) = monster->id;
				WBUFL(buf, 12) = tick;
				WBUFL(buf, 16) = 100;
				WBUFL(buf, 20) = monster->u.mons.speed;
				WBUFW(buf, 24) = damage;
				WBUFW(buf, 26) = floorskill[floorskill_index].skill_lvl;
				WBUFW(buf, 28) = 1;
				WBUFB(buf, 30) = 6;
				mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x114]);

				skill_do_damage(floorskill[floorskill_index].owner_fd, damage, monster->id, tick, 0);
				floorskill[floorskill_index].count--;
				if (floorskill[floorskill_index].count == 0) {
					if (floorskill[floorskill_index].timer > 0)
						delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

					remove_floorskill(0, tick, 0, floorskill_index);
				}
			}
			return 0;
		case FS_QUAG:
			if ((mons_data[monster->class].def1 - 10) != monster->u.mons.def1) {
				monster->u.mons.speed = monster->u.mons.speed * 125 / 100;
				monster->u.mons.def1 -= 10;
				monster->skilldata.fd = floorskill[floorskill_index].owner_fd;
				monster->skilldata.skill_num = floorskill[floorskill_index].skill_num;
				monster->skilldata.skill_timer[floorskill[floorskill_index].skill_num-1][0] = add_timer(timer_data[floorskill[floorskill_index].timer]->tick, skill_reset_monster_stats, m, n);
			}
			break;
		case FS_VENOM:
			if (monster->u.mons.option_y == 1 || monster->skilldata.skill_timer[52-1][0] > 0)
				break;

			monster->u.mons.option_y = 1;
			WBUFW(buf, 0) = 0x119;
			WBUFL(buf, 2) = monster->id;
			WBUFW(buf, 6) = monster->u.mons.option_x;
			WBUFW(buf, 8) = monster->u.mons.option_y;
			WBUFW(buf, 10) = monster->u.mons.option_z;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, monster->x, monster->y, buf, packet_len_table[0x119]);

			monster->skilldata.fd = floorskill[floorskill_index].owner_fd;
			monster->skilldata.skill_num = floorskill[floorskill_index].skill_num;
			monster->skilldata.effect |= ST_POISON;
			monster->skilldata.skill_timer[52-1][0] = add_timer(timer_data[floorskill[floorskill_index].timer]->tick, skill_reset_monster_stats, m, n);
			monster->skilldata.skill_timer[52-1][1] = floorskill[floorskill_index].skill_lvl;
			add_timer(gettick() + 3000, skill_drain_hp_monster, m, n);
			break;
		case FS_TRAP:
			if (floorskill[floorskill_index].trap_used == 0)
				floorskill[floorskill_index].trap_used = 1;

			else
				break;

			switch (floorskill[floorskill_index].skill_num) {
			case 115: // SKIDTRAP
				break;
			case 116: // LANDMINE
				damage = skill_calc_damage(floorskill[floorskill_index].owner_fd, floorskill[floorskill_index].skill_num, floorskill[floorskill_index].skill_lvl, monster->id);

				WBUFW(buf, 0) = 0x114;
				WBUFW(buf, 2) = floorskill[floorskill_index].skill_num;
				WBUFL(buf, 4) = monster->id;
				WBUFL(buf, 8) = monster->id;
				WBUFL(buf, 12) = tick;
				WBUFL(buf, 16) = 100;
				WBUFL(buf, 20) = monster->u.mons.speed;
				WBUFW(buf, 24) = damage;
				WBUFW(buf, 26) = floorskill[floorskill_index].skill_lvl;
				WBUFW(buf, 28) = 1;
				WBUFB(buf, 30) = 6;
				mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x114]);

				skill_do_damage(floorskill[floorskill_index].owner_fd, damage, monster->id, tick, 0);
				if (floorskill[floorskill_index].timer > 0)
					delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

				remove_floorskill(0, tick, 0, floorskill_index);
				if (monster->u.mons.hp <= 0) {
					monster->u.mons.hp = 0;
					return 0;
				}
				break;
			case 117: // ANKLE SNARE
				if (floorskill[floorskill_index].n == -1) {
					floorskill[floorskill_index].n = n;
					WBUFW(buf, 0) = 0x88;
					WBUFL(buf, 2) = monster->id;
					WBUFW(buf, 6) = x;
					WBUFW(buf, 8) = y;
					mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x88]);
					i = (floorskill[floorskill_index].skill_lvl * 5000) / (mons_data[monster->class].flee / 30 + 1);
					if (mons_data[monster->class].boss == 1)
						i = i / 5;

					timer_data[monster->u.mons.walktimer]->interval = i + 50;
					return 0;
				}
				break;
			case 118: // SHOCKWAVE
				if (floorskill[floorskill_index].timer > 0)
					delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

				remove_floorskill(0, tick, 0, floorskill_index);
				break;
			case 119: // SANDMAN
				monster->u.mons.option_x = 4;
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = monster->id;
				WBUFW(buf, 6) = monster->u.mons.option_x;
				WBUFW(buf, 8) = monster->u.mons.option_y;
				WBUFW(buf, 10) = monster->u.mons.option_z;
				WBUFB(buf, 12) = 0;
				mmo_map_sendarea_mxy(m, monster->x, monster->y, buf, packet_len_table[0x119]);

				monster->skilldata.fd = floorskill[floorskill_index].owner_fd;
				monster->skilldata.skill_num = 1;
				monster->skilldata.effect |= ST_SLEEP;
				monster->skilldata.skill_timer[1-1][0] = add_timer(timer_data[floorskill[floorskill_index].timer]->tick, skill_reset_monster_stats, m, n);
				if (floorskill[floorskill_index].timer > 0)
					delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

				remove_floorskill(0, tick, 0, floorskill_index);
				break;
			case 120: // FLASHER
				monster->u.mons.option_x = 6;
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = monster->id;
				WBUFW(buf, 6) = monster->u.mons.option_x;
				WBUFW(buf, 8) = monster->u.mons.option_y;
				WBUFW(buf, 10) = monster->u.mons.option_z;
				WBUFB(buf, 12) = 0;
				mmo_map_sendarea_mxy(m, monster->x, monster->y, buf, packet_len_table[0x119]);

				monster->skilldata.fd = floorskill[floorskill_index].owner_fd;
				monster->skilldata.skill_num = 1;
				monster->skilldata.effect |= ST_BLIND;
				monster->skilldata.skill_timer[1-1][0] = add_timer(timer_data[floorskill[floorskill_index].timer]->tick, skill_reset_monster_stats, m, n);
				if (floorskill[floorskill_index].timer > 0)
					delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

				remove_floorskill(0, tick, 0, floorskill_index);
				break;
			case 121: // FREEZING
				monster->u.mons.option_x = 2;
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = monster->id;
				WBUFW(buf, 6) = monster->u.mons.option_x;
				WBUFW(buf, 8) = monster->u.mons.option_y;
				WBUFW(buf, 10) = monster->u.mons.option_z;
				WBUFB(buf, 12) = 0;
				mmo_map_sendarea_mxy(m, monster->x, monster->y, buf, packet_len_table[0x119]);

				monster->skilldata.fd = floorskill[floorskill_index].owner_fd;
				monster->skilldata.skill_num = 1;
				monster->skilldata.effect |= ST_FROZEN;
				monster->skilldata.skill_timer[1-1][0] = add_timer(timer_data[floorskill[floorskill_index].timer]->tick, skill_reset_monster_stats, m, n);
				if (floorskill[floorskill_index].timer > 0)
					delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

				remove_floorskill(0, tick, 0, floorskill_index);
				break;
			case 122: // BLASTMINE
				damage = skill_calc_damage(floorskill[floorskill_index].owner_fd, floorskill[floorskill_index].skill_num, floorskill[floorskill_index].skill_lvl, monster->id);

				WBUFW(buf, 0) = 0x114;
				WBUFW(buf, 2) = floorskill[floorskill_index].skill_num;
				WBUFL(buf, 4) = monster->id;
				WBUFL(buf, 8) = monster->id;
				WBUFL(buf, 12) = tick;
				WBUFL(buf, 16) = 100;
				WBUFL(buf, 20) = monster->u.mons.speed;
				WBUFW(buf, 24) = damage;
				WBUFW(buf, 26) = floorskill[floorskill_index].skill_lvl;
				WBUFW(buf, 28) = 1;
				WBUFB(buf, 30) = 6;
				mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x114]);

				skill_do_damage(floorskill[floorskill_index].owner_fd, damage, monster->id, tick, 0);
				if (floorskill[floorskill_index].timer > 0)
					delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

				remove_floorskill(0, tick, 0, floorskill_index);
				if (monster->u.mons.hp <= 0) {
					monster->u.mons.hp = 0;
					return 0;
				}
				break;
			case 123: // CLAYMORE
				damage = skill_calc_damage(floorskill[floorskill_index].owner_fd, floorskill[floorskill_index].skill_num, floorskill[floorskill_index].skill_lvl, monster->id);

				WBUFW(buf, 0) = 0x114;
				WBUFW(buf, 2) = floorskill[floorskill_index].skill_num;
				WBUFL(buf, 4) = monster->id;
				WBUFL(buf, 8) = monster->id;
				WBUFL(buf, 12) = tick;
				WBUFL(buf, 16) = 100;
				WBUFL(buf, 20) = monster->u.mons.speed;
				WBUFW(buf, 24) = damage;
				WBUFW(buf, 26) = floorskill[floorskill_index].skill_lvl;
				WBUFW(buf, 28) = (rand() % 10 == 8) ? 5 : 7;
				WBUFB(buf, 30) = 8;
				mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x114]);

				skill_do_damage(floorskill[floorskill_index].owner_fd, damage, monster->id, tick, 0);
				if (floorskill[floorskill_index].timer > 0)
					delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

				remove_floorskill(0, tick, 0, floorskill_index);
				if (monster->u.mons.hp <= 0) {
					monster->u.mons.hp = 0;
					return 0;
				}
				break;
			}
			break;
		}
	}
	return 1;
}

static int mons_loot_search(short m, short c_x, short c_y, short *x, short *y, short range)
{
	int i, j, k;
	int item_id = 0;
	struct flooritem_data *fitem;

	short item_x = map_data[m].xs * 2;
	short item_y = map_data[m].ys * 2;
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

void mons_walk(int tid, unsigned int tick, int m, int n)
{
	int i, res;
	short x, y, dx, dy, bx, by, next_x, next_y, loot_x, loot_y;
	static int dirx[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	static int diry[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	unsigned int fd;
	unsigned char buf[256];
	struct map_session_data *sd, sd_clean;
	struct flooritem_data *fitem;
	struct npc_data *monster;

	if (!(monster = map_data[m].npc[n]))
		return;

	x = monster->x;
	y = monster->y;
	if (monster->u.mons.hp <= 0 
	|| monster->u.mons.hidden
	|| !mons_skill_floor_search(tick, monster, m, n) 
	|| !mons_data[monster->class].speed)
		return;

	if ((monster->skilldata.effect & ST_SLEEP)
	|| (monster->skilldata.effect & ST_STUN)
	|| (monster->skilldata.effect & ST_STONE)
	|| (monster->skilldata.effect & ST_FROZEN))
		return;

	if ((monster->skilldata.effect & ST_BLIND) && (rand() % 4) == 0)
		return;

	if (mons_data[monster->class].looter == 1) {
		if (monster->u.mons.target_fd == 0) {
			if (monster->u.mons.lootdata.id != 0) {
				if (object[monster->u.mons.lootdata.id] && object[monster->u.mons.lootdata.id]->type == BL_ITEM) {
					fitem = (struct flooritem_data *)object[monster->u.mons.lootdata.id];
					if (fitem->x != monster->u.mons.to_x || fitem->y != monster->u.mons.to_y) {
						monster->u.mons.lootdata.id = 0;
						monster->u.mons.walkpath_len = 0;
						monster->u.mons.walkpath_pos = 0;
					}
					else if (monster->u.mons.walkpath_pos >= monster->u.mons.walkpath_len) {
						monster->u.mons.lootdata.looted_items[monster->u.mons.lootdata.loot_count] = fitem->item_data;
						monster->u.mons.lootdata.loot_count++;
						delete_object(monster->u.mons.lootdata.id);
						WBUFW(buf, 0) = 0xa1;
						WBUFL(buf, 2) = monster->u.mons.lootdata.id;
						mmo_map_sendarea_mxy(m, fitem->x, fitem->y, buf, packet_len_table[0xa1]);
						monster->u.mons.lootdata.id = 0;
						monster->u.mons.walkpath_len = 0;
						monster->u.mons.walkpath_pos = 0;
					}
				}
				else {
					monster->u.mons.lootdata.id = 0;
					monster->u.mons.walkpath_len = 0;
					monster->u.mons.walkpath_pos = 0;
				}
			}
			if (monster->u.mons.lootdata.id == 0 && monster->u.mons.lootdata.loot_count < MAX_LOOT) {
				res = mons_loot_search(m, x, y, &loot_x, &loot_y, BLOCK_SIZE * 3 / 2);
				if (res != 0) {
					if (x == loot_x && y == loot_y) {
						monster->u.mons.lootdata.id = res;
						monster->u.mons.walkpath_len = 0;
						monster->u.mons.walkpath_pos = 0;
						monster->u.mons.to_x = loot_x;
						monster->u.mons.to_y = loot_y;
						timer_data[monster->u.mons.walktimer]->interval = monster->u.mons.speed;
						return;
					}
					else if (search_path(&sd_clean, m, x, y, loot_x, loot_y, 0) == 0) {
						monster->u.mons.lootdata.id = res;
						monster->u.mons.walkpath_len = sd_clean.walkpath_len;
						monster->u.mons.walkpath_pos = sd_clean.walkpath_pos;
						memmove(monster->u.mons.walkpath, sd_clean.walkpath, sizeof(sd_clean.walkpath));
						monster->u.mons.to_x = loot_x;
						monster->u.mons.to_y = loot_y;
					}
				}
			}
		}
		else if (monster->u.mons.lootdata.id != 0)
			monster->u.mons.lootdata.id = 0;

	}
	if (!map_data[m].users && !monster->u.mons.lootdata.id) {
		timer_data[monster->u.mons.walktimer]->interval = 5000 + monster->u.mons.speed;
		return;
	}
	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && (sd = session[i]->session_data)) {
			if (in_range(sd->x, sd->y, monster->x, monster->y, 8) && monster->m == sd->mapno
			&& !sd->hidden && sd->skill_timeamount[135-1][0] <= 0 && !sd->act_dead) {
				if (!(monster->skilldata.effect & ST_SILENCE))
					check_monster_skills(sd, i, m, n);

				if (!monster->u.mons.lootdata.id || !monster->u.mons.target_fd) {
					if ((rand() % 100) == 20)
						monster_say(i, monster->id, monster->class, "discovery");

					if ((rand() % 100) == 10) {
						if (mons_data[monster->class].mons_emotion.emotion > 0) {
							WBUFW(buf, 0) = 0x1aa;
							WBUFL(buf, 2) = monster->id;
							WBUFL(buf, 6) = mons_data[monster->class].mons_emotion.emotion;
							mmo_map_sendarea(sd->fd, buf, packet_len_table[0x1aa], 0);
						}
					}
				}
			}
		}
	}
	if (monster->u.mons.target_fd != 0) {
		fd = monster->u.mons.target_fd;
		if (!session[fd] || !(sd = session[fd]->session_data) || sd->mapno != monster->m || sd->sitting == 1) {
			mmo_mons_attack_no(m, n);
			monster->u.mons.walkpath_len = 0;
			monster->u.mons.walkpath_pos = 0;
		}
		else if (in_range(x, y, sd->x, sd->y, mons_data[monster->class].range)) {
			timer_data[monster->u.mons.walktimer]->interval = monster->u.mons.speed;
			return;
		}
		else if (monster->u.mons.to_x != sd->x || monster->u.mons.to_y != sd->y) {
			if (in_range(x, y, sd->x, sd->y, BLOCK_SIZE * (AREA_SIZE + 1))
			&& !search_path(&sd_clean, m, x, y, sd->x, sd->y, 0)) {
				monster->u.mons.walkpath_len = sd_clean.walkpath_len;
				monster->u.mons.walkpath_pos = sd_clean.walkpath_pos;
				memmove(monster->u.mons.walkpath, sd_clean.walkpath, sizeof(sd_clean.walkpath));
				monster->u.mons.to_x = sd->x;
				monster->u.mons.to_y = sd->y;
			}
			else {
				mmo_mons_attack_no(m, n);
				monster->u.mons.walkpath_len = 0;
				monster->u.mons.walkpath_pos = 0;
			}
		}
	}
	if (monster->u.mons.walkpath_pos >= monster->u.mons.walkpath_len || monster->u.mons.walkpath_len == 0) {
		for (i = 0; i < 5; i++) {
			do {
				dx = (short)(rand() % (BLOCK_SIZE * 3 + 1) - (BLOCK_SIZE * 3 / 2));
				dy = (short)(rand() % (BLOCK_SIZE * 3 + 1) - (BLOCK_SIZE * 3 / 2));
				if (rand() % 2 == 0) {
					dx *= (short)(2 / 3);
					dy *= (short)(2 / 3);
				}
			}
			while (dx == 0 && dy == 0);
			if (!search_path(&sd_clean, m, x, y, x + dx, y + dy, 1)) {
				monster->u.mons.walkpath_len = sd_clean.walkpath_len;
				monster->u.mons.walkpath_pos = sd_clean.walkpath_pos;
				memmove(monster->u.mons.walkpath, sd_clean.walkpath, sizeof(sd_clean.walkpath));
				monster->u.mons.to_x = x + dx;
				monster->u.mons.to_y = y + dy;
				timer_data[monster->u.mons.walktimer]->interval = rand() % 4000 + 1000 + monster->u.mons.speed * 2;
				return;
			}
		}
		if (i == 5) {
			timer_data[monster->u.mons.walktimer]->interval = monster->u.mons.speed;
			return;
		}
	}
	dx = dirx[(int)monster->u.mons.walkpath[monster->u.mons.walkpath_pos]];
	dy = diry[(int)monster->u.mons.walkpath[monster->u.mons.walkpath_pos]];
	monster->dir = monster->u.mons.walkpath[monster->u.mons.walkpath_pos];
	next_x = x + dx;
	next_y = y + dy;
	monster->x = next_x;
	monster->y = next_y;
	memset(buf, 0, packet_len_table[0x7b]);
	WBUFW(buf, 0) = 0x7b;
	WBUFL(buf, 2) = monster->id;
	WBUFW(buf, 6) = monster->u.mons.speed;
	WBUFW(buf, 8) = monster->u.mons.option_x;
	WBUFW(buf, 10) = monster->u.mons.option_y;
	WBUFW(buf, 12) = monster->u.mons.option_z;
	WBUFW(buf, 14) = monster->class;
	WBUFL(buf, 22) = gettick();
	WBUFW(buf, 36) = monster->dir;
	set_2pos(WBUFP(buf, 50), x, y, next_x, next_y);
	WBUFB(buf, 55) = 0;
	WBUFB(buf, 56) = 5;
	WBUFB(buf, 57) = 5;
	mmo_map_sendarea_mxy(m, next_x, next_y, buf, packet_len_table[0x7b]);

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
		monster->block.prev = NULL;
		monster->block.next = NULL;
		monster->block.type = BL_NPC;
		monster->block.subtype = MONS;
		add_block(&monster->block, m, next_x, next_y);
	}
	monster->u.mons.walkpath_pos++;
	if (monster->u.mons.walkpath[monster->u.mons.walkpath_pos] & 1)
		i = (int)(monster->u.mons.speed * 14 / 10);

	else
		i = monster->u.mons.speed;

	if (!monster->u.mons.target_fd && !monster->u.mons.lootdata.id)
		i *= 1.2;

	timer_data[monster->u.mons.walktimer]->interval = i;
}

int spawn_monster(int class, short x, short y, short m, unsigned int fd)
{
	int len, i;
	unsigned char buf[64];
	struct npc_data *monster;

	if (m < 0 || m >= MAX_MAP_PER_SERVER)
		return 0;

	if (!map_data[m].npc[map_data[m].npc_num])
		map_data[m].npc[map_data[m].npc_num] = malloc(sizeof(struct npc_data));

	else
		map_data[m].npc[map_data[m].npc_num] = realloc(map_data[m].npc[map_data[m].npc_num], sizeof(struct npc_data));

	if (!(monster = map_data[m].npc[map_data[m].npc_num]))
		return 0;

	for (i = 0; i < 10000; i++) {
		set_monster_random_point(m, map_data[m].npc_num);
		if (in_range(monster->x, monster->y, x, y, 12))
			break;

	}
	if (i == 10000)
		return 0;

	monster->m = m;
	monster->dir = 0;
	monster->u.mons.to_x = monster->x;
	monster->u.mons.to_y = monster->y;
	monster->u.mons.speed = mons_data[class].speed;

	monster->u.mons.walktimer = add_timer(gettick() + 10, mons_walk, m, map_data[m].npc_num);
	timer_data[monster->u.mons.walktimer]->type = TIMER_INTERVAL;
	timer_data[monster->u.mons.walktimer]->interval = monster->u.mons.speed;

	monster->class = class;
	monster->id = npc_id++;

	strcpy(monster->name, "*");
	strncpy(monster->name + 1, mons_data[monster->class].real_name, 23);
	monster->u.mons.hp = mons_data[monster->class].max_hp;
	monster->u.mons.target_fd = 0;
	monster->u.mons.walkpath_len = 0;
	monster->u.mons.walkpath_pos = 0;
	monster->u.mons.attacktimer = 0;
	monster->u.mons.attackdelay = mons_data[monster->class].adelay;
	monster->u.mons.summon = 0;
	monster->u.mons.hidden = 0;
	monster->u.mons.option_x = 0;
	monster->u.mons.option_y = 0;
	monster->u.mons.option_z = 0;
	monster->u.mons.steal = mons_data[monster->class].steal;
	monster->skilldata.fd = 0;
	monster->skilldata.skill_num = 0;
	monster->skilldata.effect = 00000000;
	monster->u.mons.lootdata.id = 0;
	monster->u.mons.lootdata.loot_count = 0;
	monster->u.mons.atk1 = mons_data[monster->class].atk1;
	monster->u.mons.atk2 = mons_data[monster->class].atk2;
	monster->u.mons.def1 = mons_data[monster->class].def1;
	monster->u.mons.def2 = mons_data[monster->class].def2;

	if (monster->class > 20) {
		len = mmo_map_set_spawn(monster, buf);
		if (len > 0)
			mmo_map_sendarea(fd, buf, len, 0);
	}
	else {
  		len = mmo_map_set_npc(monster, buf);
		if (len > 0)
			mmo_map_sendarea(fd, buf, len, 0);

	}
	monster->block.prev = NULL;
	monster->block.next = NULL;
	monster->block.type = BL_NPC;
	monster->block.subtype = MONS;
	add_block_npc(m, map_data[m].npc_num);
	map_data[m].npc_num++;
	return 1;
}

int spawn_to_pos(struct map_session_data *sd, int class, char *name, short x, short y, short m, unsigned int fd)
{
	int i, len;
	short dx, dy;
	unsigned char buf[64];
	struct npc_data *monster;

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

	if (!(monster = map_data[m].npc[map_data[m].npc_num]))
		return 0;

	monster->m = m;
	monster->x = sd->x + dx;
	monster->y = sd->y + dy;
	monster->dir = 0;
	monster->u.mons.to_x = monster->x;
	monster->u.mons.to_y = monster->y;
	monster->u.mons.speed = mons_data[class].speed;

	monster->u.mons.walktimer = add_timer(gettick() + 10, mons_walk, m, map_data[m].npc_num);
	timer_data[monster->u.mons.walktimer]->type = TIMER_INTERVAL;
	timer_data[monster->u.mons.walktimer]->interval = monster->u.mons.speed;

	monster->class = class;
	monster->id = npc_id++;

	if (!name) {
		strcpy(monster->name, "*");
		strncpy(monster->name + 1, mons_data[monster->class].real_name, 23);
	}
	else {
		strcpy(monster->name, "*");
		strncpy(monster->name + 1, name, 23);
	}
	monster->u.mons.hp = mons_data[monster->class].max_hp;
	monster->u.mons.target_fd = fd;
	monster->u.mons.walkpath_len = 0;
	monster->u.mons.walkpath_pos = 0;
	monster->u.mons.attackdelay = mons_data[monster->class].adelay;
	monster->u.mons.attacktimer = add_timer(gettick() + monster->u.mons.attackdelay, mmo_mons_attack_continue, m, map_data[m].npc_num);
	monster->u.mons.summon = 0;
	monster->u.mons.hidden = 0;
	monster->u.mons.option_x = 0;
	monster->u.mons.option_y = 0;
	monster->u.mons.option_z = 0;
	monster->u.mons.steal = mons_data[monster->class].steal;
	monster->skilldata.fd = 0;
	monster->skilldata.skill_num = 0;
	monster->skilldata.effect = 00000000;
	monster->u.mons.lootdata.id = 0;
	monster->u.mons.lootdata.loot_count = 0;
	monster->u.mons.atk1 = mons_data[monster->class].atk1;
	monster->u.mons.atk2 = mons_data[monster->class].atk2;
	monster->u.mons.def1 = mons_data[monster->class].def1;
	monster->u.mons.def2 = mons_data[monster->class].def2;

	if (monster->class > 20) {
		len = mmo_map_set_spawn(monster, buf);
		if (len > 0)
			mmo_map_sendarea(fd, buf, len, 0);
	}
	else {
		len = mmo_map_set_npc(monster, buf);
		if (len > 0)
			mmo_map_sendarea(fd, buf, len, 0);

	}
	monster->block.prev = NULL;
	monster->block.next = NULL;
	monster->block.type = BL_NPC;
	monster->block.subtype = MONS;
	add_block_npc(m, map_data[m].npc_num);
	map_data[m].npc_num++;
	return 1;
}

void spawn_delay(int tid, unsigned int tick, int m, int n)
{
	unsigned char buf[64];
	struct npc_data *monster;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP || map_data[m].npc[n]->block.subtype != MONS)
		return;

	if (!(monster = map_data[m].npc[n]))
		return;

	set_monster_random_point(m, n);
	monster->dir = 0;
	monster->u.mons.to_x = monster->x;
	monster->u.mons.to_y = monster->y;
	monster->u.mons.speed = mons_data[monster->class].speed;
	monster->u.mons.hp = mons_data[monster->class].max_hp;
	monster->u.mons.attackdelay = mons_data[monster->class].adelay;
	monster->u.mons.steal = mons_data[monster->class].steal;
	monster->u.mons.atk1 = mons_data[monster->class].atk1;
	monster->u.mons.atk2 = mons_data[monster->class].atk2;
	monster->u.mons.def1 = mons_data[monster->class].def1;
	monster->u.mons.def2 = mons_data[monster->class].def2;

	if (monster->class > 20) {
		mmo_map_set_spawn(monster, buf);
		mmo_map_sendarea_mxy(m, monster->x, monster->y, buf, packet_len_table[0x7c]);
	}
	else {
		mmo_map_set_npc(monster, buf);
		mmo_map_sendarea_mxy(m, monster->x, monster->y, buf, packet_len_table[0x7c]);
	}
	monster->block.prev = NULL;
	monster->block.next = NULL;
	monster->block.type = BL_NPC;
	monster->block.subtype = MONS;
	add_block_npc(m, n);
}

int remove_monsters(short m, short x, short y, unsigned int fd)
{
	int i, n;
	struct map_session_data *sd;
	struct npc_data *monster;

	if (m >= 0 && m < MAX_MAP_PER_SERVER) {
		for (n = 0; n < MAX_NPC_PER_MAP; n++) {
			monster = map_data[m].npc[n];
			if (monster && monster->block.subtype == MONS) {
				if (in_range(x, y, monster->x, monster->y, 8)) {
					mmo_mons_send_death(fd, m, n);
					for (i = 5; i < FD_SETSIZE; i++)
						if (session[i] && (sd = session[i]->session_data))
							if (sd->attacktarget == monster->id)
								mmo_player_attack_no(sd);



				}
			}
		}
	}
	return 0;
}

int boss_monster(short m, int n)
{
	struct npc_data *monster;

	if ((monster = map_data[m].npc[n])) {
		if ( monster->class != 1038 && /* Osiris */
		     monster->class != 1039 && /* Baphomet */
		     monster->class != 1046 && /* Doppelganger */
		     monster->class != 1086 && /* Golden Bug */
		     monster->class != 1087 && /* Ork Hero */
		     monster->class != 1112 && /* Drake */
		     monster->class != 1115 && /* Eddga */
		     monster->class != 1150 && /* Moonlight */
		     monster->class != 1059 && /* Mistress */
		     monster->class != 1159 && /* Phreeoni */
		     monster->class != 1147 && /* Maya */
		     monster->class != 1190 && /* Orc Lord */
		     monster->class != 1283 && /* Chimera */
		     monster->class != 1272 && /* Dark Lord */
		     monster->class != 1389 && /* Dracula */
		     monster->class != 1259 && /* Griffon */
		     monster->class != 1262 && /* Mutant Dragonoid */
		     monster->class != 1157 && /* Pharao */
		     monster->class != 1312 && /* Turtle General */
		     monster->class != 1022 && /* Werewolf */
		     monster->class != 1200)   /* Zherlthsh */
			return 0;
	}
	return 1;
}

int search_monster_id(char *name)
{
	int j;

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

static int init_boss_miniboss(int class)
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

static int monster_emotion(char *mons_name)
{
	int var;
	char name[24], line[256];
	FILE *fp = fopen("data/databases/mons_emotion_db.txt", "rt");
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
	else {
		debug_output("monster_emotion: Fail to open 'data/databases/mons_emotion_db.txt'.\n");
		abort();
	}
	return -1;
}

int read_npc_fromdir(char *dirname)
{
	int i = 0;
	char temp[64];
	char dirnext[64];
	DIR *dirstream;
	struct dirent *dirfile;

	if ((dirstream = opendir(dirname)) == NULL) {
		debug_output("read_npc_fromdir: '%s' directory doesn't exist.\n", dirname);
		abort();
	}
	else
		while ((dirfile = readdir(dirstream)) != NULL) {
		if ((strstr(dirfile->d_name, ".txt") != NULL) || (strstr(dirfile->d_name, ".npc")) != NULL) {
			i++;
			snprintf(temp, sizeof(temp), "%s/%s", dirname, dirfile->d_name);
			npc_txt[npc_txt_num] = malloc(strlen(temp) + 1);
			strcpy(npc_txt[npc_txt_num++], temp);
		}
		else if ((strcmp(dirfile->d_name, "CVS") != 0) && (strncmp(dirfile->d_name, ".", 1) != 0) && (strncmp(dirfile->d_name, "..", 2) != 0)) {
			struct stat buf;

			snprintf(dirnext, sizeof(dirnext), "%s/%s", dirname, dirfile->d_name);
			if (stat(dirnext, &buf) == 0 && S_ISDIR(buf.st_mode)) {
				i += read_npc_fromdir(dirnext);

			}
		}
	}
	closedir(dirstream);
	return i;
}

static void mons_data_init(void)
{
	int i;
	char line[2040];
	FILE *fp = NULL;
 
	printf("Loading Databases...          ");
	fp = fopen("data/databases/monster_db.txt", "rt");
	if (fp) {
		while (fgets(line, 2039, fp)) {
			int  class, lv, max_hp, base_exp, job_exp;
			int  range, atk1, atk2, def1, def2, mdef1, mdef2, hit, flee;
			int  scale, race, ele, mode, speed, adelay, amotion, dmotion;
			char name[128], JName[128], dropitem[1024], mvpdropitem[512], *p;

			if (sscanf(line,"%d,%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\t]%[^\n]",
					    &class, name, JName, &lv, &max_hp, &base_exp, &job_exp,
					    &range, &atk1, &atk2, &def1, &def2, &mdef1, &mdef2, &hit, &flee,
					    &scale, &race, &ele, &mode, &speed, &adelay, &amotion, &dmotion,
					    dropitem, mvpdropitem) != 26) {
				if (strstr(line,"//") != line)
					printf("Problem Loading Monster: %d\n", class);

				continue;
			}
			if (class >= MAX_MONS)
				continue;

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
	else {
		debug_output("mons_data_init: Fail to open 'data/databases/monster_db.txt'.\n");
		abort();
	}
	fp = fopen("data/databases/mons_say_db.txt","rt");
	if (fp) {
		while (fgets(line, 2039, fp)) {
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
					while (fgets(line, 2039, fp)) {
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
	else {
		debug_output("mons_data_init: Fail to open 'data/databases/mons_say_db.txt'.\n");
		abort();
	}
	printf("Done\n");
}

static void script_data_init(void)
{
	int i, npc_txt_count;
	char line[2040];
	FILE *fp = NULL;

	printf("Loading Scripts Data...       ");
	for (npc_txt_count = 0; npc_txt_count < npc_txt_num; npc_txt_count++) {
		fp = fopen(npc_txt[npc_txt_count], "rt");
		if (!fp) {
			printf("ERROR : Cannot load %s!\n", npc_txt[npc_txt_count]);
			continue;
		}
		while (fgets(line, 2039, fp)) {
			char mapname[256], w1[1024], w2[1024], w3[1024], w4[1024];
			short x, y, dir;
			if ((sscanf(line, "%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]", w1, w2, w3, w4) != 4) || (sscanf(w1,"%[^,],%hd,%hd,%hd", mapname, &x, &y, &dir) != 4))
				continue;

			for (i = 0; map[i][0]; i++)
				if (strncmp(map[i], mapname, 16) == 0)
					break;


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
				warp->block.type = BL_NPC;
				warp->block.subtype = WARP;
				add_block_npc(i, map_data[i].npc_num);
				map_data[i].npc_num++;
			}
			else if (strcmp(w2, "monster") == 0) {
				int j, class;
				short num;
				struct npc_data *monster;
				sscanf(w4, "%d,%hd", &class, &num);
				for (j = 0; j < num; j++) {
					map_data[i].npc[map_data[i].npc_num] = malloc(sizeof(struct npc_data));
					monster = map_data[i].npc[map_data[i].npc_num];
					monster->m = i;
					if (num == 1 && x && y) {
						monster->x = x;
						monster->y = y;
					}
					else
						set_monster_random_point(i, map_data[i].npc_num);

					monster->dir = 0;
					monster->u.mons.to_x = monster->x;
					monster->u.mons.to_y = monster->y;
					monster->u.mons.speed = mons_data[class].speed;

					monster->u.mons.walktimer = add_timer(gettick() + 10, mons_walk, i, map_data[i].npc_num);
					timer_data[monster->u.mons.walktimer]->type = TIMER_INTERVAL;
					timer_data[monster->u.mons.walktimer]->interval = monster->u.mons.speed;

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
					monster->u.mons.hidden = 0;
					monster->u.mons.option_x = 0;
					monster->u.mons.option_y = 0;
					monster->u.mons.option_z = 0;
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
					monster->block.type = BL_NPC;
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
				shop->block.type = BL_NPC;
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

					fgets(line, 2039, fp);
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
				script->block.type = BL_NPC;
				script->block.subtype = SCRIPT;
				add_block_npc(i, map_data[i].npc_num);
				map_data[i].npc_num++;
			}
		}
		fclose(fp);
	}
	printf("Done\n");
}

void read_npcdata(void)
{
	pet_store_init_npc_id(&npc_id);
	mons_data_init();
	script_data_init();
}
