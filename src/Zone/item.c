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
#include "item.h"
#include "itemdb.h"
#include "npc.h"
#include "skill.h"
#include "skill_db.h"
#include "pet.h"

extern int drop_mult;
extern char map[][];

int search_item(int object)
{
	char file_name[40];
	char strgat[80];
	char gomi[40];
	int scount = 0,fcount = 0,nameid = 0;
	FILE *fp = NULL;

	if (object == 1) {
		strcpy(file_name,"data/lists/item_all.lst");
		scount = (int)(((double)rand()/(double)RAND_MAX)*1061.0);
	}
	else if (object == 2) {
		strcpy(file_name,"data/lists/item_equipment.lst");
		scount = (int)(((double)rand()/(double)RAND_MAX)*490.0);
	}
	else if (object == 3) {
		strcpy(file_name,"data/lists/item_card.lst");
		scount = (int)(((double)rand()/(double)RAND_MAX)*149.0);
	}
	else if (object == 4) {
		strcpy(file_name,"data/lists/item_present.lst");
		scount = (int)(((double)rand()/(double)RAND_MAX)*77.0);
	}
	else if (object == 5) {
		strcpy(file_name,"data/lists/item_misc.lst");
		scount = (int)(((double)rand()/(double)RAND_MAX)*1061.0);
	}
	else if (object == 6) {
		strcpy(file_name,"data/lists/mob_branch.lst");
		scount = (int)(((double)rand()/(double)RAND_MAX)*77.0);
	}
	if ((fp = fopen(file_name,"rt"))) {
		for (fcount = 0; fcount < scount; fcount++)
			fgets(strgat, 80, fp);

	}
	fclose(fp);
	sscanf(strgat, "%d%s", &nameid, gomi);
	return nameid;
}

int mmo_map_lose_item(unsigned int fd, int silent, int index, int amount)
{
	int len;
	struct map_session_data *sd;
	struct item *ditem;

	if (!session[fd] || !(sd = session[fd]->session_data)) return -1;
	if (!(ditem = &sd->status.inventory[index - 2])) return -1;
	if ((index >= MAX_INVENTORY +2) || (index < 2)) return -1;
	if (ditem->nameid == 0 || ditem->amount < amount || amount < 1) return -1;

	ditem->amount -= amount;
	sd->weight -= itemdb[search_itemdb_index(ditem->nameid)].weight * amount;
	if (ditem->amount == 0) {
		if (ditem->equip == 0x8000) {
			WFIFOW(fd, 0) = 0x13c;
			WFIFOW(fd, 2) = 0;
			WFIFOSET(fd, packet_len_table[0x13c]);

			ditem->equip = 0;
		}
		memset(ditem, 0, sizeof(*ditem));
	}
	if (!silent) {
		WFIFOW(fd, 0) = 0xaf;
		WFIFOW(fd, 2) = index;
		WFIFOW(fd, 4) = amount;
		WFIFOSET(fd, packet_len_table[0xaf]);
	}
	mmo_map_calc_overweight(sd);
	len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_WEIGHT);
	if (len > 0)
		WFIFOSET(fd, len);

	return 1;
}

int mmo_map_get_item(unsigned int fd, struct item *item)
{
	struct map_session_data *sd;
	struct item *n_item;
	int dataid = search_itemdb_index(item->nameid);
	int n_id = 0;
	int i = -1, newi = -1, len;

	if (!session[fd] || !(sd = session[fd]->session_data)) return -1;
	if (sd->max_weight < sd->weight + itemdb[dataid].weight * item->amount) {
		n_item = &sd->status.inventory[i];
		n_id = search_itemdb_index(n_item->nameid);
		WFIFOW(fd, 0) = 0xa0;
		WFIFOW(fd, 2) = i + 2;
		WFIFOW(fd, 4) = item->amount;
		WFIFOW(fd, 6) = n_item->nameid;
		WFIFOB(fd, 8) = n_item->identify;
		WFIFOB(fd, 9) = n_item->attribute;
		WFIFOB(fd, 10) = n_item->refine;
		WFIFOW(fd, 11) = n_item->card[0];
		WFIFOW(fd, 13) = n_item->card[1];
		WFIFOW(fd, 15) = n_item->card[2];
		WFIFOW(fd, 17) = n_item->card[3];
		WFIFOW(fd, 19) = itemdb[n_id].equip;
		WFIFOB(fd, 21) = itemdb[n_id].type;
		WFIFOB(fd, 22) = 1;
		WFIFOSET(fd, 23);
		return -2;
	}
	if (itemdb[dataid].type == 4 || itemdb[dataid].type == 5) {
		for (i = 0; i < MAX_INVENTORY; i++) {
			n_item = &sd->status.inventory[i];
			if (!n_item->nameid) {
				memcpy(n_item, item, sizeof(*item));
				break;
			}
		}
		if (i == MAX_INVENTORY) return -1;
	}
	else {
		for (i = 0; i < MAX_INVENTORY; i++) {
			n_item = &sd->status.inventory[i];
			if (!n_item->nameid && newi == -1) newi = i;

			if (itemdb[dataid].type != 4 && itemdb[dataid].type != 5 &&
			    n_item->nameid == item->nameid &&
			    n_item->identify == item->identify) {
				n_item->amount += item->amount;
				break;
			}
		}
		if (i == MAX_INVENTORY) {
			if (newi != -1) {
				i = newi;
				n_item = &sd->status.inventory[i];
				memcpy(n_item, item, sizeof(*item));
			}
			else return -1;
		}
	}
	sd->weight += itemdb[dataid].weight * item->amount;
	n_item = &sd->status.inventory[i];
	n_id = search_itemdb_index(n_item->nameid);
	WFIFOW(fd, 0) = 0xa0;
	WFIFOW(fd, 2) = i + 2;
	WFIFOW(fd, 4) = item->amount;
	WFIFOW(fd, 6) = n_item->nameid;
	WFIFOB(fd, 8) = n_item->identify;
	WFIFOB(fd, 9) = n_item->attribute;
	WFIFOB(fd, 10) = n_item->refine;
	WFIFOW(fd, 11) = n_item->card[0];
	WFIFOW(fd, 13) = n_item->card[1];
	WFIFOW(fd, 15) = n_item->card[2];
	WFIFOW(fd, 17) = n_item->card[3];
	WFIFOW(fd, 19) = itemdb[n_id].equip;
	WFIFOB(fd, 21) = itemdb[n_id].type;
	WFIFOB(fd, 22) = 0;
	WFIFOSET(fd, 23);

	mmo_map_calc_overweight(sd);
	len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_WEIGHT);
	if (len > 0)
		WFIFOSET(fd, len);

	return 1;
}

void mmo_map_make_flooritem(struct item *item_data, int amount, short m, short x, short y)
{
	int i, j, c = 0, free_cell = 0, r = 0, id = 0;
	unsigned char buf[64];
	struct flooritem_data *fitem;

	if ((id = search_free_object_id()) == 0)
		return;

	for (free_cell = 0,i = -1; i <= 1; i++) {
		if (i + y < 0 || i + y >= map_data[m].ys)
			continue;

		for (j = -1; j <= 1; j++) {
			if (j + x < 0 || j + x >= map_data[m].xs)
				continue;

			if ((c = map_data[m].gat[j + x + (i + y) * map_data[m].xs]) == 1 || c == 5)
				continue;

			free_cell++;
		}
	}
	if (free_cell == 0)
		return;

	r = rand();
	free_cell = r % free_cell;
	r = r >> 4;
	for (i = -1; i <= 1; i++) {
		if (i + y < 0 || i + y >= map_data[m].ys)
			continue;

		for (j = -1; j <= 1; j++) {
			if (j + x < 0 || j + x >= map_data[m].xs)
				continue;

			if ((c = map_data[m].gat[j + x + (i + y) * map_data[m].xs]) == 1 || c == 5)
				continue;

			if (free_cell == 0) {
				x += j;
				y += i;
				i = 3;
				break;
			}
			free_cell--;
		}
	}
	fitem = calloc(sizeof(*fitem), 1);
	fitem->id = id;
	fitem->m = m;
	fitem->x = x;
	fitem->y = y;
	fitem->subx = (r & 3) * 3 + 3;
	fitem->suby = ((r >> 2) & 3) * 3 + 3;
	memcpy(&fitem->item_data, item_data, sizeof(*item_data));
	fitem->item_data.amount = amount;
	fitem->drop_tick = gettick();

	mmo_map_set_dropitem(buf, fitem);
	mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x9e]);

	object[id] = &fitem->block;
	fitem->block.prev = NULL;
	fitem->block.next = NULL;
	fitem->block.type = BL_ITEM;
	add_block(&fitem->block, fitem->m, fitem->x, fitem->y);
}

int mmo_map_dropitem(struct map_session_data *sd, int index, int amount)
{
	struct item tmp_item;

	memcpy(&tmp_item, &sd->status.inventory[index - 2], sizeof(tmp_item));
	if (mmo_map_lose_item(sd->fd, 0, index, amount))
		mmo_map_make_flooritem(&tmp_item, amount, sd->mapno, sd->x, sd->y);

	return 0;
}

void mmo_map_take_item(struct map_session_data *sd, int item_id)
{
	unsigned char buf[64];
	struct flooritem_data *fitem;

	fitem = (struct flooritem_data*)object[item_id];
	if (mmo_map_get_item(sd->fd, &fitem->item_data)) {
		memset(buf, 0, packet_len_table[0x8a]);
		WBUFW(buf, 0) = 0x8a;
		WBUFL(buf, 2) = sd->account_id;
		WBUFL(buf, 6) = item_id;
		WBUFL(buf, 10) = gettick();
		WBUFB(buf, 26) = 1;
		mmo_map_sendarea(sd->fd, buf, packet_len_table[0x8a], 0);

		delete_object(item_id);
		memset(buf, 0, packet_len_table[0xa1]);
		WBUFW(buf, 0) = 0xa1;
		WBUFL(buf, 2) = item_id;
		mmo_map_sendarea(sd->fd, buf, packet_len_table[0xa1], 0);
	}
}

void mmo_map_delay_item_drop(int tid, unsigned int tick, int id, int data)
{
	struct delay_item_drop *ditem;
	struct item temp_item;

	ditem = (struct delay_item_drop *)id;
	memset(&temp_item, 0, sizeof(temp_item));
	temp_item.nameid = ditem->nameid;
	temp_item.amount = 1;
	if ((itemdb[search_itemdb_index(ditem->nameid)].type == 4) || (itemdb[search_itemdb_index(ditem->nameid)].type == 5))
		temp_item.identify = 0;

	else
		temp_item.identify = 1;

	mmo_map_make_flooritem(&temp_item, 1, ditem->m, ditem->x, ditem->y);
	free(ditem);
	ditem = NULL;
}

void mmo_map_item_drop(short m, int n)
{
	int i;
	struct delay_item_drop *ditem;
	struct npc_data *monster;

	if (!(monster = map_data[m].npc[n]))
		return;

	for (i = 0; i < 16; i++) {
		if (mons_data[monster->class].dropitem[i].nameid != 0) {
			if (rand() % 10000 < mons_data[monster->class].dropitem[i].p * drop_mult) {
				ditem = malloc(sizeof(*ditem));
				ditem->nameid = mons_data[monster->class].dropitem[i].nameid;
				ditem->m = m;
				ditem->x = monster->x;
				ditem->y = monster->y;
				add_timer(gettick() + 300, mmo_map_delay_item_drop, (int)ditem, 0);
			}
		}
	}
	if (mons_data[monster->class].looter == 1 && monster->u.mons.lootdata.loot_count > 0) {
		for (i = 0; i < monster->u.mons.lootdata.loot_count; i++)
			mmo_map_make_flooritem(&monster->u.mons.lootdata.looted_items[i], 
			monster->u.mons.lootdata.looted_items[i].amount, 
			m, monster->x, monster->y);
	}
	monster->u.mons.lootdata.loot_count = 0;
}

int mmo_map_item_steal(unsigned int fd, short m, int n)
{
	int i;
	struct item tmp_item;
	struct npc_data *monster;

	if (!session[fd])
		return 0;

	if (!(monster = map_data[m].npc[n]))
		return 0;

	for (i = 0; i < 16; i++) {
		if (!mons_data[monster->class].dropitem[i].nameid)
			continue;

		if (rand() % 10000 < mons_data[monster->class].dropitem[i].p) {
			monster->u.mons.steal = 1;
			memset(&tmp_item, 0, sizeof(tmp_item));
			tmp_item.nameid = mons_data[map_data[m].npc[n]->class].dropitem[i].nameid;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			mmo_map_get_item(fd, &tmp_item);
		}
	}
	return 0;
}

void item_apraisal(struct map_session_data *sd, int indexid)
{
	int i = 0, flag = 1;
	unsigned int fd = sd->fd;

	if (indexid > 0) {
		i = indexid - 2;
		if (i < MAX_INVENTORY && sd->status.inventory[i].identify != 1) {
			sd->status.inventory[i].identify = 1;
			flag = 0;
		}
	}
	WFIFOW(fd, 0) = 0x179;
	WFIFOW(fd, 2) = indexid;
	WFIFOB(fd, 4) = flag;
	WFIFOSET(fd, packet_len_table[0x179]);
}

void use_item(struct map_session_data *sd, int indexid)
{
	int i, c, len, hpheal = 0, spheal = 0, fail = 0;
	int nameid =sd->status.inventory[indexid-2].nameid;
	int dataid = search_itemdb_index(nameid);
	int item_type = itemdb[dataid].type;
	int hp_min = itemdb[dataid].hp1;
	int sp_min = itemdb[dataid].sp1;
	double bonus = 0.0;
	unsigned int fd = sd->fd;
	unsigned char buf[64];
	struct mmo_charstatus *p = &sd->status;
	struct item tmp_item;

	switch(item_type) {
	case 0: // Recovery Item
		if (p->skill[3].lv > 0)
			bonus = (float)p->skill[3].lv / (float)10.0;

		bonus += (double)1 + ((float)(p->vit + p->vit2) / (float)50.0);
		hpheal = floor(((itemdb[dataid].hp2 - hp_min + 1) * ((float)rand() / (float)RAND_MAX) + hp_min) * bonus);
		spheal = floor(((itemdb[dataid].sp2 - sp_min + 1) * ((double)rand() / (double)RAND_MAX) + sp_min));

		if (hpheal > 0) {
			if ((p->hp + hpheal) > p->max_hp)
				p->hp = p->max_hp;

			else
				p->hp += hpheal;

			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_HP);
			if (len > 0)
				WFIFOSET(fd, len);
		}
		if (spheal > 0) {
			if ((p->sp + spheal) > p->max_sp)
				p->sp = p->max_sp;

			else
				p->sp += spheal;

			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_SP);
			if (len > 0)
				WFIFOSET(fd, len);
		}
		if (itemdb[dataid].eff > 0) {
			switch(nameid) {
			case 506: // Green Potion
				if (sd->status.option_y == 1 || sd->status.option_y == 4)
					sd->status.option_y = 0;

				if (sd->status.option_x == 6)
					sd->status.option_x = 0;

				mmo_map_setoption(sd, buf, 0);
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
				break;
			case 511: // Green Herb
				if (sd->status.option_y == 1)
					sd->status.option_y = 0;

				mmo_map_setoption(sd, buf, 0);
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
				break;
			case 523: // Holy Water
				if (sd->status.option_y == 6)
					sd->status.option_y = 0;

				mmo_map_setoption(sd, buf, 0);
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
				break;
			case 525: // Panacea
				sd->status.option_x = 0;
				sd->status.option_y = 0;
				mmo_map_setoption(sd, buf, 0);
				mmo_map_sendarea(sd->fd, buf, packet_len_table[0x119], 0);
				break;
			}
		}
		sd->weight -= itemdb[dataid].weight;
		mmo_map_calc_overweight(sd);
		len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_WEIGHT);
		if (len > 0)
			WFIFOSET(fd, len);

		p->inventory[indexid-2].amount--;
		if (p->inventory[indexid-2].amount <= 0)
			p->inventory[indexid-2].nameid = 0;

		WBUFW(buf, 0) = 0x1c8;
		WBUFW(buf, 2) = indexid;
		WBUFW(buf, 4) = nameid;
		WBUFL(buf, 6) = sd->account_id;
		WBUFW(buf, 10) = p->inventory[indexid-2].amount;
		WBUFB(buf, 12) = 1;
		mmo_map_sendarea(fd, buf, 13, 0);
		break;
	case 2: // Special Effect Items
		switch(nameid) {
		case 601: // Fly Wing
			if (!mmo_map_flagload(sd->mapname, TELEPORT)) {
				WFIFOW(fd, 0) = 0x189;
				WFIFOW(fd, 2) = 0;
				WFIFOSET(fd, 4);
				fail = 1;
				break;
			}
			do {
				sd->x = rand() % (map_data[sd->mapno].xs - 2) + 1;
				sd->y = rand() % (map_data[sd->mapno].ys - 2) + 1;
			} 
			while(map_data[sd->mapno].gat[sd->x + sd->y * map_data[sd->mapno].xs] == 1 
			|| map_data[sd->mapno].gat[sd->x + sd->y * map_data[sd->mapno].xs] == 5);
			mmo_map_changemap(sd, sd->mapname, sd->x, sd->y, 2);
			break;
		case 602: // Butterfly Wing
			if (!mmo_map_flagload(sd->mapname, TELEPORT)) {
				WFIFOW(fd, 0) = 0x189;
				WFIFOW(fd, 2) = 0;
				WFIFOSET(fd, 4);
				fail = 1;
				break;
			}
			mmo_map_changemap(sd, p->save_point.map, p->save_point.x, p->save_point.y, 2);
			break;
		case 603: // Old Blue Box
			i = search_item(1);
			memset(&tmp_item, 0, sizeof(tmp_item));
			tmp_item.nameid = i;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		case 604: // Dead Branch
			if (!mmo_map_flagload(sd->mapname, BRANCH)) {
				fail = 1;
				break;
			}
			i = search_item(6);
			spawn_to_pos(sd, mons_data[i].class, mons_data[i].name, sd->x, sd->y, sd->mapno, fd);
			break;
		case 605: // Anodyne
			WBUFW(buf, 0) = 0x147;
			WBUFW(buf, 2) = 8;
			WBUFW(buf, 4) = skill_db[8].type_inf;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 1;
			WBUFW(buf, 10) = skill_db[8].sp;
			WBUFW(buf, 12) = skill_db[8].range;
			memcpy(WBUFP(buf, 14), "Anodyne", 24);
			WBUFB(buf, 38) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x147], 0);
			sd->no_cost_sp = 1;
			break;
		case 606: // Aloevera
			p->sp = p->max_sp;
			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_SP);
			if (len > 0)
				WFIFOSET(fd, len);

			break;
		case 607: // Yggdrasil Berry
			p->hp = p->max_hp;
			p->sp = p->max_sp;
			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_HP);
			if (len > 0)
				WFIFOSET(fd, len);

			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_SP);
			if (len > 0)
				WFIFOSET(fd, len);

			break;
		case 608: // Yggdrasil Seed
			p->hp += p->max_hp/2;
			p->sp += p->max_sp/2;
			if (p->hp > p->max_hp)
				p->hp = p->max_hp;

			if (p->sp > p->max_sp)
				p->sp = p->max_sp;

			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_HP);
			if (len > 0)
				WFIFOSET(fd, len);

			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_SP);
			if (len > 0)
				WFIFOSET(fd, len);

			break;
		case 609: // Amulet
			WFIFOW(fd, 0) = 0x19e;
			WFIFOSET(fd, packet_len_table[0x19e]);
			break;
		case 610: // Yggdrasil Leaf
			WBUFW(buf, 0) = 0x147;
			WBUFW(buf, 2) = 54;
			WBUFW(buf, 4) = skill_db[54].type_inf;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 1;
			WBUFW(buf, 10) = skill_db[54].sp;
			WBUFW(buf, 12) = skill_db[54].range;
			memcpy(WBUFP(buf, 14), "Leaf of Yggdrasil", 24);
			WBUFB(buf, 38) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x147], 0);
			sd->no_cost_sp = 1;
			break;
		case 611: // Magnifier
			WFIFOW(fd, 0) = 0x177;
			for (i = c = 0; i < MAX_INVENTORY; i++) {
				if (sd->status.inventory[i].identify != 1) {
					WFIFOW(fd, 4 + c * 2) = i + 2;
					c++;
				}
			}
			WFIFOW(fd, 2) = 4 + c * 2;
			WFIFOSET(fd, 4 + c * 2);
			break;
		case 612: // Mini Furnace
			p->forge_lvl = 0;
			WFIFOW(fd, 0) = 0x18d;
			for (i = 0, c = 0; i < MAX_SKILL_REFINE; i++) {
				if (skill_can_forge(sd, forge_db[i].nameid, sd->status.forge_lvl) > 0) {
					WFIFOW(fd, c * 8 + 4) = forge_db[i].nameid;
					WFIFOW(fd, c * 8 + 6) = 0012;
					WFIFOL(fd, c * 8 + 8) = sd->status.char_id;
					c++;
				}
			}
			WFIFOW(fd, 2) = c * 8 + 8;
			WFIFOSET(fd, WFIFOW(fd, 2));
			break;
		case 613: // Iron Hammer
			p->forge_lvl = 1;
			WFIFOW(fd, 0) = 0x18d;
			for (i = 0, c = 0; i < MAX_SKILL_REFINE; i++) {
				if (skill_can_forge(sd, forge_db[i].nameid, sd->status.forge_lvl) > 0) {
					WFIFOW(fd, c * 8 + 4) = forge_db[i].nameid;
					WFIFOW(fd, c * 8 + 6) = 0012;
					WFIFOL(fd, c * 8 + 8) = sd->status.char_id;
					c++;
				}
			}
			WFIFOW(fd, 2) = c * 8 + 8;
			WFIFOSET(fd, WFIFOW(fd, 2));
			break;
		case 614: // Golden Hammer
			p->forge_lvl = 2;
			WFIFOW(fd, 0) = 0x18d;
			for (i = 0, c = 0; i < MAX_SKILL_REFINE; i++) {
				if (skill_can_forge(sd, forge_db[i].nameid, sd->status.forge_lvl) > 0) {
					WFIFOW(fd, c * 8 + 4) = forge_db[i].nameid;
					WFIFOW(fd, c * 8 + 6) = 0012;
					WFIFOL(fd, c * 8 + 8) = sd->status.char_id;
					c++;
				}
			}
			WFIFOW(fd, 2) = c * 8 + 8;
			WFIFOSET(fd, WFIFOW(fd, 2));
			break;
		case 615: // Oridecon Hammer
			p->forge_lvl = 3;
			WFIFOW(fd, 0) = 0x18d;
			for (i = 0, c = 0; i < MAX_SKILL_REFINE; i++) {
				if (skill_can_forge(sd, forge_db[i].nameid, sd->status.forge_lvl) > 0) {
					WFIFOW(fd, c * 8 + 4) = forge_db[i].nameid;
					WFIFOW(fd, c * 8 + 6) = 0012;
					WFIFOL(fd, c * 8 + 8) = sd->status.char_id;
					c++;
				}
			}
			WFIFOW(fd, 2) = c * 8 + 8;
			WFIFOSET(fd, WFIFOW(fd, 2));
			break;
		case 616: // Old Card Album
			i = search_item(3);
			memset(&tmp_item, 0, sizeof(tmp_item));
			tmp_item.nameid = i;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		case 617: // Old Purple Box
			i = search_item(2);
			memset(&tmp_item, 0, sizeof(tmp_item));
			tmp_item.nameid = i;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		case 618: // Worn Out Scroll
			i = search_item(3);
			memset(&tmp_item, 0, sizeof(tmp_item));
			tmp_item.nameid = i;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		case 619: // Unripe Apple
		case 620: // Orange_Juice
		case 621: // Bitter Herb
		case 622: // Rainbow Carrot
		case 623: // Earthworm the Dude
		case 624: // Rotten Fish
		case 625: // Rusty Iron
		case 626: // Monster Juice
		case 627: // Sweet Milk
		case 628: // Well-Dried Bone
		case 629: // Singing Flower
		case 630: // Dew Laden Moss
		case 631: // Deadly Noxious Herb
		case 632: // Fatty Chubby Earthworm
		case 633: // Baked Yam
		case 634: // Tropical Banana
		case 635: // Horror of Tribe
		case 636: // No Recipient
		case 637: // Old Broom
		case 638: // Silver Knife of Chastity
		case 639: // Armlet of Obedience
		case 640: // Shining Stone
		case 641: // Contracts in Shadow
		case 642: // Book of Devil
		case 659: // Her Heart
		case 660: // Fobidden Red Candle
		case 661: // Flapping Apron
			sd->status.pet.mons_id = search_pet_class_item(nameid);
			WFIFOW(fd, 0)= 0x19e;
			WFIFOSET(fd, packet_len_table[0x19e]);
			break;
		case 643: // Pet Incubator
			if (sd->status.pet.activity == 1) {
				fail = 1;
				break;
			}
			for (i = c = 0; i < MAX_INVENTORY; i++)
				if ((sd->status.inventory[i].nameid >= 9001) && (sd->status.inventory[i].nameid <= 9024))
					c++;


			if (c == 0) {
				fail = 1;
				break;
			}
			WFIFOW(fd, 0) = 0x1a6;
			for (i = c = 0; i < MAX_INVENTORY; i++) {
				if ((sd->status.inventory[i].nameid >= 9001) && (sd->status.inventory[i].nameid <= 9024)) {
					WFIFOW(fd, 4 + c * 2) = i + 2;
					c++;
				}
			}
			WFIFOW(fd, 2) = 4 + c * 2;
			WFIFOSET(fd, 4 + c * 2);
			break;
		case 644: // X-Mas Present
			i = search_item(4);
			memset(&tmp_item, 0, sizeof(tmp_item));
			tmp_item.nameid = i;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		case 645: // Concentration Potion
			sd->skill_timeamount[1-1][0] = add_timer(gettick() + 1800000, skill_reset_stats, fd, 1);
			sd->skill_timeamount[1-1][1] = 110;
			skill_icon_effect(sd, 37, 1);
			mmo_map_calc_status(fd, 0);
			break;
		case 656: // Awakening Potion
			if (p->base_level < 40
			|| sd->status.class == ACOLYTE
			|| sd->status.class == PRIEST
			|| sd->status.class == MONK) {
				fail = 1;
				break;
			}
			sd->skill_timeamount[1-1][0] = add_timer(gettick() + 1800000, skill_reset_stats, fd, 1);
			sd->skill_timeamount[1-1][1] = 120;
			skill_icon_effect(sd, 37, 1);
			mmo_map_calc_status(fd, 0);
			break;
		case 657: // Berserk Potion
			if (p->base_level < 85
			|| sd->status.class == ACOLYTE
			|| sd->status.class == PRIEST
			|| sd->status.class == MONK
			|| sd->status.class == THIEF
			|| sd->status.class == ASSASSIN
			|| sd->status.class == ROGUE
			|| sd->status.class == ARCHER
			|| sd->status.class == HUNTER
			|| sd->status.class == BARD
			|| sd->status.class == DANCER) {
				fail = 1;
				break;
			}
			sd->skill_timeamount[1-1][0] = add_timer(gettick() + 1800000, skill_reset_stats, fd, 1);
			sd->skill_timeamount[1-1][1] = 130;
			skill_icon_effect(sd, 37, 1);
			mmo_map_calc_status(fd, 0);
			break;
		case 658: // Tribal Solidarity
			break;
		case 664: // Gift_Box
		case 665: // Gift_Box
		case 666: // Gift_Box
		case 667: // Gift_Box
			i = search_item(5);
			memset(&tmp_item, 0, sizeof(tmp_item));
			tmp_item.nameid = i;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		}
		if (!fail) {
			sd->weight -= itemdb[dataid].weight;
			mmo_map_calc_overweight(sd);
			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_WEIGHT);
			if (len > 0)
				WFIFOSET(fd, len);

			p->inventory[indexid-2].amount--;
			if (p->inventory[indexid-2].amount <= 0)
				p->inventory[indexid-2].nameid = 0;

			WBUFW(buf, 0) = 0x1c8;
			WBUFW(buf, 2) = indexid;
			WBUFW(buf, 4) = nameid;
			WBUFL(buf, 6) = sd->account_id;
			WBUFW(buf, 10) = p->inventory[indexid-2].amount;
			WBUFB(buf, 12) = 1;
			mmo_map_sendarea(fd, buf, 13, 0);
		}
		else {
			WFIFOW(fd, 0) = 0xa8;
			WFIFOW(fd, 2) = nameid;
			WFIFOW(fd, 4) = p->inventory[indexid-2].amount;
			WFIFOB(fd, 6) = 0;
			WFIFOSET(fd, 7);
		}
		break;
	}
}
