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
 Module:        Version 1.7.1 - Alkie
 Author:        Odin Developer Team Copyrights (c) 2004
 Project:       Project Odin Zone Server
 Creation Date: Dicember 6, 2003
 Modified Date: October 31, 2004
 Description:   Ragnarok Online Server Emulator
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
extern char map[][16];

int search_item(int object)
{
	char file_name[40];
	char strgat[80];
	char gomi[40];
	int scount = 0,fcount = 0,nameid = 0;
	FILE *fp= NULL;

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
	if ((fp = fopen(file_name,"r"))) {
		for (fcount = 0; fcount < scount; fcount++) {
			fgets(strgat, 80, fp);
		}
	}
	fclose(fp);
	sscanf(strgat, "%d%s", &nameid, gomi);
	return nameid;
}

int mmo_map_lose_item(int fd, int silent, int index, int amount)
{
	struct map_session_data *sd = session[fd]->session_data;
	struct item *ditem = &sd->status.inventory[index - 2];

	if ((index >= MAX_INVENTORY +2) || (index < 2)) return -1;
	if (ditem->nameid == 0 || ditem->amount < amount || amount < 1) return -1;

	ditem->amount -= amount;
	sd->weight -= itemdb[search_itemdb_index(ditem->nameid)].weight * amount;
	if (ditem->amount == 0) memset(ditem, 0, sizeof(*ditem));

	if(!silent) {
		WFIFOW(fd, 0) = 0xaf;
		WFIFOW(fd, 2) = index;
		WFIFOW(fd, 4) = amount;
		WFIFOSET(fd, 6);
	}
	mmo_map_calc_overweight(sd);

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x18;
	WFIFOL(fd, 4) = sd->weight;
	WFIFOSET(fd, 8);
	return 1;
}

int mmo_map_get_item(int fd, struct item* item)
{
	struct map_session_data *sd = session[fd]->session_data;
	struct item *n_item;
	int dataid = search_itemdb_index(item->nameid);
	int n_id;
	int i = -1;
	int newi = -1;

	if(sd->max_weight < sd->weight + itemdb[dataid].weight * item->amount) {
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

	WFIFOW(fd, 0) = 0xb0;
	WFIFOW(fd, 2) = 0x18;
	WFIFOL(fd, 4) = sd->weight;
	WFIFOSET(fd, 8);

	return 1;
}

void mmo_map_make_flooritem(struct item *item_data, int amount, short m, short x, short y)
{
	int i, j, c, free_cell, r, id;
	unsigned char buf[256];
	struct flooritem_data *fitem;

	if (!(id = search_free_object_id()))
		return;

	free_cell = 0;
	for (i = y - 1; i <= y + 1; i++) {
		if (i < 0)
			continue;

		else if (i >= map_data[m].ys)
			break;

		for (j = x - 1; j <= x + 1; j++) {
			if (j < 0)
				continue;

			else if (j >= map_data[m].xs)
				break;

			if ((c = map_data[m].gat[j + i * map_data[m].xs]) == 1 || c == 5 || (c & 0x80) == 0x80)
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

			if ((c = map_data[m].gat[j + x + (i + y) * map_data[m].xs]) == 1 || c == 5 || (c & 0x80) == 0x80)
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
	fitem = malloc(sizeof(*fitem));
	fitem->id = id;
	fitem->m = m;
	fitem->x = x;
	fitem->y = y;
	fitem->subx = (r & 3) * 3 + 3;
	fitem->suby = ((r >> 2) & 3) * 3 + 3;
	memcpy(&fitem->item_data, item_data, sizeof(*item_data));
	fitem->item_data.amount = amount;
	fitem->drop_tick = gettick();

	object[id] = &fitem->block;
	fitem->block.prev = NULL;
	fitem->block.next = NULL;
	fitem->block.type = BL_ITEM;
	add_block(&fitem->block, fitem->m, fitem->x, fitem->y);

	mmo_map_set_dropitem(buf, fitem);
	mmo_map_sendarea_mxy(m, x, y, buf, packet_len_table[0x9e]);
}

int mmo_map_dropitem(struct map_session_data *sd, int index, int amount)
{
	struct item tmp_item;

	if (index < 2 || index >= MAX_INVENTORY + 2)
		return -1;

	memcpy(&tmp_item, &sd->status.inventory[index - 2], sizeof(tmp_item));
	if (mmo_map_lose_item(sd->fd, 0, index, amount) == -1)
		return -1;

	mmo_map_make_flooritem(&tmp_item, amount, sd->mapno, sd->x, sd->y);
	return 0;
}

void mmo_map_take_item(struct map_session_data *sd, int item_id)
{
	unsigned char buf[256];
	struct flooritem_data *fitem;

	if (item_id < 2 || item_id >= MAX_OBJECTS || !object[item_id] || object[item_id]->type != BL_ITEM)
		return;

	fitem = (struct flooritem_data*)object[item_id];
	if (mmo_map_get_item(sd->fd, &fitem->item_data)) {
		memset(buf, 0, packet_len_table[0x8a]);
		WBUFW(buf, 0) = 0x8a;
		WBUFL(buf, 2) = sd->account_id;
		WBUFL(buf, 6) = item_id;
		WBUFL(buf, 10) = gettick();
		WBUFL(buf, 14) = 0;
		WBUFL(buf, 18) = 0;
		WBUFW(buf, 22) = 0;
		WBUFW(buf, 24) = 0;
		WBUFB(buf, 26) = 1;
		WBUFW(buf, 27) = 0;
		mmo_map_sendarea(sd->fd, buf, packet_len_table[0x8a], 0);

		delete_object(item_id);
		memset(buf, 0, packet_len_table[0xa1]);
		WBUFW(buf, 0) = 0xa1;
		WBUFL(buf, 2) = item_id;
		mmo_map_sendarea(sd->fd, buf, packet_len_table[0xa1], 0);
	}
}

int mmo_map_delay_item_drop(int tid, unsigned int tick, int id, int data)
{
	struct delay_item_drop *ditem;
	struct item temp_item;

	tid = 0;
	tick = 0;
	data = 0;
	ditem = (struct delay_item_drop *)id;
	memset(&temp_item, 0, sizeof(temp_item));
	temp_item.nameid = ditem->nameid;
	temp_item.amount = 1;
	if ((itemdb[search_itemdb_index(ditem->nameid)].type == 4) || (itemdb[search_itemdb_index(ditem->nameid)].type == 5))
		temp_item.identify = 0;

	else
		temp_item.identify = 1;

	mmo_map_make_flooritem(&temp_item, 1, ditem->m,ditem->x, ditem->y);
	free(ditem);
	return 0;
}

void mmo_map_item_drop(short m, int n)
{
	int i;
	struct delay_item_drop *ditem;

	for (i = 0; i < 16; i++) {
		if (mons_data[map_data[m].npc[n]->class].dropitem[i].nameid != 0) {
			if (rand() % 10000 < mons_data[map_data[m].npc[n]->class].dropitem[i].p * drop_mult) {
				ditem = malloc(sizeof(*ditem));
				ditem->nameid = mons_data[map_data[m].npc[n]->class].dropitem[i].nameid;
				ditem->m = m;
				ditem->x = map_data[m].npc[n]->x;
				ditem->y = map_data[m].npc[n]->y;
				add_timer(gettick() + 300, mmo_map_delay_item_drop, (int)ditem, 0);
			}
		}
	}
	if (mons_data[map_data[m].npc[n]->class].looter == 1 && map_data[m].npc[n]->u.mons.lootdata.loot_count > 0) {
		for (i = 0; i < map_data[m].npc[n]->u.mons.lootdata.loot_count; i++) {
			mmo_map_make_flooritem(&map_data[m].npc[n]->u.mons.lootdata.looted_items[i], map_data[m].npc[n]->u.mons.lootdata.looted_items[i].amount, m, map_data[m].npc[n]->x, map_data[m].npc[n]->y);
		}
	}
	map_data[m].npc[n]->u.mons.lootdata.loot_count = 0;
}

int mmo_map_item_steal(int fd, short m, int n)
{
	int i;
	struct item tmp_item;

	if (!session[fd] )
		return 0;

	if (!map[m][0])
		return 0;

	for (i = 0; i < 16; i++) {
		if (!mons_data[map_data[m].npc[n]->class].dropitem[i].nameid)
			continue;

		if (rand() % 10000 < mons_data[map_data[m].npc[n]->class].dropitem[i].p) {
			map_data[m].npc[n]->u.mons.steal = 1;
			memset(&tmp_item, 0, sizeof(tmp_item));
			tmp_item.nameid = mons_data[map_data[m].npc[n]->class].dropitem[i].nameid;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			mmo_map_get_item(fd, &tmp_item);
		}
	}
	return 0;
}

void use_item(struct map_session_data *sd, int indexid)
{
	int i,j, len, fd = sd->fd;
	int hpheal = 0, spheal = 0;
	int nameid, dataid;
	int item_type;
	double bonus = 0;
//	short option;
	int fail = 0;
	FILE *fp;
	struct mmo_charstatus *p = &sd->status;
	struct item tmp_item;

	nameid = sd->status.inventory[indexid-2].nameid; //An Identifier that server and client both agree on.
	dataid = search_itemdb_index(nameid); //The id that the datebase refers to
	item_type = itemdb[dataid].type; 

	switch(item_type) {
	case 0: //Recovery Item
		//Calculate bonus
		//Recovery calculation (new +2% per vit information from www.rodatazone.com)
		//Recovery amount =(hp recover of item)*(1+(VIT/50)+(Hp recovery skill/10))
		if(p->skill[3].lv > 0) bonus = ((double)p->skill[3].lv)/10.0;
		bonus += ((double)(p->vit+50)/(double)50);

		//Calculate heal amount
		hpheal = (double)((((double)rand()/(double)RAND_MAX) * itemdb[dataid].hp2)+itemdb[dataid].hp1) * bonus;
		spheal = (double)((((double)rand()/(double)RAND_MAX) * itemdb[dataid].sp2)+itemdb[dataid].sp1);

		//Calculate new weight
		sd->weight -= itemdb[dataid].weight;

		//Send max weight, I don't know why but aegis does it so i'm going to too
		WFIFOW(fd, 0) = 0xb0;
		WFIFOW(fd, 2) = 0x19; //Max Weight
		WFIFOL(fd, 4) = sd->max_weight;
		WFIFOSET(fd, 8);

		//Send new weight
		WFIFOW(fd, 0) = 0xb0;
		WFIFOW(fd, 2) = 0x18; //Weight
		WFIFOL(fd, 4) = sd->weight;
		WFIFOSET(fd, 8);

		//Send hp and/or sp
		if(hpheal > 0) {
			if ((p->hp + hpheal) > p->max_hp) {
				p->hp = p->max_hp;
			}else{ 
				p->hp += hpheal;
			}
			WFIFOW(fd, 0) = 0xb0;
			WFIFOW(fd, 2) = 0x5;  //HP
			WFIFOL(fd, 4) = p->hp;
			WFIFOSET(fd, 8);
		}
		if(spheal > 0) {
			if ((p->sp + spheal) > p->max_sp) {
				p->sp = p->max_sp;
			}else{ 
				p->sp += spheal;
			}
			WFIFOW(fd, 0) = 0xb0;
			WFIFOW(fd, 2) = 0x7;  //SP
			WFIFOL(fd, 4) = p->sp;
			WFIFOSET(fd, 8);
		}
/*		if (itemdb[dataid].eff > 0) { // recovers a status effect
			// needs to be fixed later, currently, any item that heals one status effect will heal them all
			option = 0x0038;
			p->option_z = p->option_z & option;//turn off flag unless peco or cart
			WFIFOW(fd,0)=0x0119;
			WFIFOL(fd,2)=sd->account_id;
			WFIFOW(fd,6)=0;
			WFIFOW(fd,8)=0;
			WFIFOW(fd,10)=p->option_z;
			mmo_map_sendarea(fd,WFIFOP(fd,0),12,0);
		}*/
		//New item packet, it has the cool effects :D
		WFIFOW(fd, 0) = 0x1c8;
		WFIFOW(fd, 2) = indexid; //Index
		WFIFOW(fd, 4) = nameid; //Item id
		WFIFOL(fd, 6) = sd->account_id;
		WFIFOW(fd, 10) = --p->inventory[indexid-2].amount;
		WFIFOB(fd, 12) = 1; //Success... i donno when it would ever be failure
		mmo_map_sendarea(fd,WFIFOP(fd,0),13,0);

		if (p->inventory[indexid-2].amount <= 0) p->inventory[indexid-2].nameid = 0;

		break;
	case 2: // Special Effect Items
/*		// Most items use skills:
		//Aloevera - Use Provoke level 1 on a target (once)
		//Anodyne - Use level 1 endure on yourself (once)
		//Yggdrasil Leaf - Use Ressurection level 1 on a target (once)
		//Magnifier - Use merchant's identify. (once)
		//Butterfly wing - use Teleport level 3? (once) i got the level 3 from packet sniffing.... even though max is 2...weird
		//Fly wing - use Teleport level 1. (once)
		// Not yet implemented:
		if (itemdb[dataid].skill > 0 && !fail) {
			--p->inventory[indexid-2].amount;
			if (p->inventory[indexid-2].amount <= 0) p->inventory[indexid-2].nameid = 0;

			sd->used_item_skill = itemdb[dataid].skill;

			WFIFOW(fd, 0) = 0x147;
			WFIFOW(fd, 2) = itemdb[dataid].skill; //skill id
			WFIFOW(fd, 4) = skill_db[itemdb[dataid].skill].type; //target type. Butterflywing & flywing = 4;
			WFIFOW(fd, 6) = 0; //??
			WFIFOW(fd, 8) = itemdb[dataid].skill_lv; //skill level.
			WFIFOW(fd, 10) = skill_sp_cost(itemdb[dataid].skill, itemdb[dataid].skill_lv); // sp cost?
			WFIFOW(fd, 12) = 0; //range?
			memcpy(WFIFOP(fd, 14), skill_db[itemdb[dataid].skill].name, 24); // skill name. example: AL_TELEPORT
			WFIFOB(fd, 38) = 1; //up?
			WFIFOSET(fd, 39);
		}
*/
		switch(nameid) {
		case 601: // Fly Wing
			if (strcmp(sd->mapname,"prt_in.gat")==0||strcmp(sd->mapname,"alberta_in.gat")==0
				||strcmp(sd->mapname,"geffen_in.gat")==0||strcmp(sd->mapname,"izlude_in.gat")==0
				||strcmp(sd->mapname,"morocc_in.gat")==0||strcmp(sd->mapname,"payon_in01.gat")==0
				||strcmp(sd->mapname,"payon_in02.gat")==0||strcmp(sd->mapname,"aldeba_in.gat")==0
				||strcmp(sd->mapname,"cmd_in01.gat")==0||strcmp(sd->mapname,"cmd_in02.gat")==0
				||strcmp(sd->mapname,"yuno_in01.gat")==0||strcmp(sd->mapname,"yuno_in02.gat")==0
				||strcmp(sd->mapname,"yuno_in03.gat")==0||strcmp(sd->mapname,"yuno_in04.gat")==0
				||strcmp(sd->mapname,"yuno_in05.gat")==0) {
				WFIFOW(fd, 0) = 0x189;
				WFIFOW(fd, 2) = 0;
				WFIFOSET(fd, 4);
				fail = 1;
			}
			else {
				WFIFOW(fd,0) = 0x80;
				WFIFOL(fd,2) = sd->account_id;
				WFIFOB(fd,6) = 2;
				mmo_map_sendarea( fd, WFIFOP(fd,0),packet_len_table[0x80], 0 );
				do {
					sd->x=rand()%(map_data[sd->mapno].xs-2)+1;
					sd->y=rand()%(map_data[sd->mapno].ys-2)+1;
				} 
				while(map_data[sd->mapno].gat[sd->x+sd->y*map_data[sd->mapno].xs]==1 || map_data[sd->mapno].gat[sd->x+sd->y*map_data[sd->mapno].xs]==5);
				mmo_map_changemap(sd,sd->mapname,sd->x,sd->y,2);
			}
			break;
		case 602: // Butterfly Wing
			if (strcmp(sd->mapname,"prt_in.gat")==0||strcmp(sd->mapname,"alberta_in.gat")==0
				||strcmp(sd->mapname,"geffen_in.gat")==0||strcmp(sd->mapname,"izlude_in.gat")==0
				||strcmp(sd->mapname,"morocc_in.gat")==0||strcmp(sd->mapname,"payon_in01.gat")==0
				||strcmp(sd->mapname,"payon_in02.gat")==0||strcmp(sd->mapname,"aldeba_in.gat")==0
				||strcmp(sd->mapname,"cmd_in01.gat")==0||strcmp(sd->mapname,"cmd_in02.gat")==0
				||strcmp(sd->mapname,"yuno_in01.gat")==0||strcmp(sd->mapname,"yuno_in02.gat")==0
				||strcmp(sd->mapname,"yuno_in03.gat")==0||strcmp(sd->mapname,"yuno_in04.gat")==0
				||strcmp(sd->mapname,"yuno_in05.gat")==0) {
				WFIFOW(fd, 0) = 0x189;
				WFIFOW(fd, 2) = 0;
				WFIFOSET(fd, 4);
				fail = 1;
			}
			else {
				WFIFOW(fd,0) = 0x80;
				WFIFOL(fd,2) = sd->account_id;
				WFIFOB(fd,6) = 2;
				mmo_map_sendarea( fd, WFIFOP(fd,0),7, 0 );
				mmo_map_changemap(sd,p->save_point.map,p->save_point.x,p->save_point.y,2);
			}
			break;
		case 603: // Old Blue Box
			i = search_item(1);
			memset(&tmp_item,0,sizeof(tmp_item));
			tmp_item.nameid=i;
			tmp_item.amount=1;
			tmp_item.identify=1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		case 604: // Dead Branch
			if (strcmp(sd->mapname,"prt_in.gat")==0||strcmp(sd->mapname,"alberta_in.gat")==0
			||strcmp(sd->mapname,"geffen_in.gat")==0||strcmp(sd->mapname,"izlude_in.gat")==0
			||strcmp(sd->mapname,"morocc_in.gat")==0||strcmp(sd->mapname,"payon_in01.gat")==0
			||strcmp(sd->mapname,"payon_in02.gat")==0||strcmp(sd->mapname,"aldeba_in.gat")==0
			||strcmp(sd->mapname,"cmd_in01.gat")==0||strcmp(sd->mapname,"cmd_in02.gat")==0
			||strcmp(sd->mapname,"yuno_in01.gat")==0||strcmp(sd->mapname,"yuno_in02.gat")==0
			||strcmp(sd->mapname,"yuno_in03.gat")==0||strcmp(sd->mapname,"yuno_in04.gat")==0
			||strcmp(sd->mapname,"yuno_in05.gat")==0) {
				fail = 1;
			}
			else {
				i = 1000 + rand()%300;
				while (!((i >=1001 && i <= 1005) || (i>=1007 && i<=1016) || (i>=1018 && i<=1020) ||
				(i>=1023 && i<=1026) || (i>=1029 && i<=1037) || (i>=1040 && i<=1042) ||
				(i>=1044 && i<=1045) || (i>=1047 && i<=1058) || (i>=1060 && i<=1061) ||
				(i>=1063 && i<=1064) || (i>=1066 && i<=1068) || (i>=1070 && i<=1071) ||
				(i>=1076 && i<=1077) || (i>=1088 && i<=1091) || (i>=1093 && i<=1097) ||
				(i>=1099 && i<=1107) || (i>=1109 && i<=1111) || (i>=1113 && i<=1114) ||
				(i>=1116 && i<=1126) || (i>=1128 && i<=1131) || (i>=1133 && i<=1135) ||
				(i>=1138 && i<=1141) || (i>=1143 && i<=1146) || (i>=1151 && i<=1153) ||
				(i>=1155 && i<=1156) || (i>=1160 && i<=1161) || (i>=1163 && i<=1164) ||
				(i>=1166 && i<=1170) || (i>=1174 && i<=1180)))
					i = 1000 + rand()%300;
					spawn_monster(i, sd->x, sd->y, sd->mapno, fd);
			}
			break;
		case 605: // Anodyne
			WFIFOW(fd,0) = 0x147;
			WFIFOW(fd,2) = 8;
			WFIFOW(fd,4) = skill_db[8].type_inf;
			WFIFOW(fd,6) = 0;
			WFIFOW(fd,8) = 1;
			WFIFOW(fd,10) = skill_db[8].sp;
			WFIFOW(fd,12) = skill_db[8].range;
			memcpy(WFIFOP(fd,14),"Anodyne",24);
			WFIFOB(fd,38) = 0;
			mmo_map_sendarea(fd,WFIFOP(fd,0),39,0);
			break;
		case 606: // Aloevera
			p->sp = p->max_sp;
			WFIFOW(fd,0) = 0xb0;
			WFIFOW(fd,2) = 0007;
			WFIFOL(fd,4) = p->sp;
			WFIFOSET(fd,8);
			break;
		case 607: // Yggdrasil Berry
			p->hp = p->max_hp;
			p->sp = p->max_sp;
			WFIFOW(fd,0) = 0xb0;
			WFIFOW(fd,2) = 0005;
			WFIFOL(fd,4) = p->hp;
			WFIFOSET(fd,8);
			WFIFOW(fd,0) = 0xb0;
			WFIFOW(fd,2) = 0007;
			WFIFOL(fd,4) = p->sp;
			WFIFOSET(fd,8);
			break;
		case 608: // Yggdrasil Seed
			p->hp += p->max_hp/2;
			p->sp += p->max_sp/2;
			if(p->hp > p->max_hp)p->hp = p->max_hp;
			if(p->sp > p->max_sp)p->sp = p->max_sp;
			WFIFOW(fd,0) = 0xb0;
			WFIFOW(fd,2) = 0005;
			WFIFOL(fd,4) = p->hp;
			WFIFOSET(fd,8);
			WFIFOW(fd,0) = 0xb0;
			WFIFOW(fd,2) = 0007;
			WFIFOL(fd,4) = p->sp;
			WFIFOSET(fd,8);
			break;
		case 609: // Amulet
			WFIFOW(fd, 0)= 0x19e;
			WFIFOSET(fd, packet_len_table[0x19e]);
			break;
		case 610: // Yggdrasil Leaf
			WFIFOW(fd,0) = 0x147;
			WFIFOW(fd,2) = 54;
			WFIFOW(fd,4) = skill_db[54].type_inf;
			WFIFOW(fd,6) = 0;
			WFIFOW(fd,8) = 1;
			WFIFOW(fd,10) = skill_db[54].sp;
			WFIFOW(fd,12) = skill_db[54].range;
			memcpy(WFIFOP(fd,14),"Leaf of Yggdrasil",24);
			WFIFOB(fd,38) = 0;
			mmo_map_sendarea(fd,WFIFOP(fd,0),39,0);
			break;
		case 611: // Magnifier
			WFIFOW(fd,0) = 0x177;
			for (i = j = 0; i < MAX_INVENTORY; i++) {
				if (p->inventory[i].identify != 1) {
					WFIFOW(fd,4 + j * 2) = i + 2;
					j++;
				}

			}
			WFIFOW(fd,2) = 4 + j * 2;
			WFIFOSET(fd,4 + j * 2);
			break;
		case 612: // Mini Furnace
			p->forge_lvl = 0;
			WFIFOW(fd, 0)=0x18d;
			for(i=0,j=0;i<MAX_SKILL_REFINE;i++){
				if(skill_can_forge(sd,forge_db[i].nameid,p->forge_lvl)>0){
					WFIFOW(fd,j*8+ 4) = forge_db[i].nameid;
					WFIFOW(fd,j*8+ 6) = 0012;
					WFIFOL(fd,j*8+ 8) = sd->status.char_id;
					j++;
				}
			}
			WFIFOW(fd, 2)=j*8+8;
			WFIFOSET(fd,WFIFOW(fd,2));
			break;
		case 613: // Iron Hammer
			p->forge_lvl = 1;
			WFIFOW(fd, 0)=0x18d;
			for(i=0,j=0;i<MAX_SKILL_REFINE;i++){
				if(skill_can_forge(sd,forge_db[i].nameid,sd->status.forge_lvl)>0){
					WFIFOW(fd,j*8+ 4) = forge_db[i].nameid;
					WFIFOW(fd,j*8+ 6) = 0012;
					WFIFOL(fd,j*8+ 8) = sd->status.char_id;
					j++;
				}
			}
			WFIFOW(fd, 2)=j*8+8;
			WFIFOSET(fd,WFIFOW(fd,2));
			break;
		case 614: // Golden Hammer
			p->forge_lvl = 2;
			WFIFOW(fd, 0)=0x18d;
			for(i=0,j=0;i<MAX_SKILL_REFINE;i++){
				if(skill_can_forge(sd,forge_db[i].nameid,sd->status.forge_lvl)>0){
					WFIFOW(fd,j*8+ 4) = forge_db[i].nameid;
					WFIFOW(fd,j*8+ 6) = 0012;
					WFIFOL(fd,j*8+ 8) = sd->status.char_id;
					j++;
				}
			}
			WFIFOW(fd, 2)=j*8+8;
			WFIFOSET(fd,WFIFOW(fd,2));
			break;
		case 615: // Oridecon Hammer
			p->forge_lvl = 3;
			WFIFOW(fd, 0)=0x18d;
			for(i=0,j=0;i<MAX_SKILL_REFINE;i++){
				if(skill_can_forge(sd,forge_db[i].nameid,sd->status.forge_lvl)>0){
					WFIFOW(fd,j*8+ 4) = forge_db[i].nameid;
					WFIFOW(fd,j*8+ 6) = 0012;
					WFIFOL(fd,j*8+ 8) = sd->status.char_id;
					j++;
				}
			}
			WFIFOW(fd, 2)=j*8+8;
			WFIFOSET(fd,WFIFOW(fd,2));
			break;
		case 616: // Old Card Album
			i = search_item(3);
			memset(&tmp_item,0,sizeof(tmp_item));
			tmp_item.nameid=i;
			tmp_item.amount=1;
			tmp_item.identify=1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		case 617: // Old Purple Box
			i = search_item(2);
			memset(&tmp_item,0,sizeof(tmp_item));
			tmp_item.nameid=i;
			tmp_item.amount=1;
			tmp_item.identify=1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		case 618: // Worn Out Scroll
			i = search_item(3);
			memset(&tmp_item,0,sizeof(tmp_item));
			tmp_item.nameid=i;
			tmp_item.amount=1;
			tmp_item.identify=1;
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
			sd->status.pet.mons_id = search_pet_class_item(nameid);
			WFIFOW(fd, 0)= 0x19e;
			WFIFOSET(fd, packet_len_table[0x19e]);
			break;

		case 643: // Pet Incubator
			if (sd->status.pet.activity == 1) {
				fail = 1;
				break;
			}
			WFIFOW(fd,0) = 0x1a6;
			for(i=j=0;i<100;i++) { //if egg
				if((sd->status.inventory[i].nameid >= 9001)&&(sd->status.inventory[i].nameid <=9024)) {
					WFIFOW(fd,4+j*2) = i+2;//sd->status.inventory[i].nameid;
					j++;
				}
			}
			WFIFOW(fd,2) = 4+j*2;
			WFIFOSET(fd,4+j*2);
			break;

		case 644: // X-Mas Present
			i = search_item(4);
			memset(&tmp_item,0,sizeof(tmp_item));
			tmp_item.nameid=i;
			tmp_item.amount=1;
			tmp_item.identify=1;
			mmo_map_get_item(fd, &tmp_item);
			break;
		case 645: // Concentration Potion
			break;
		case 656: // Awakening Potion
			break;
		case 657: // Berserk Potion
			break;
		case 658: // Tribal Solidarity
			break;
		case 659: // Her Heart
			break;
		case 660: // Fobidden Red Candle
			break;
		case 661: // Flapping Apron
			break;
		default:
			break;
		}
		if (fail == 0) {
			sd->weight -= itemdb[dataid].weight;
			len=mmo_map_set_param(fd,WFIFOP(fd,0),SP_WEIGHT);
			if (len>0) {
				WFIFOSET(fd,len);
			}
			WFIFOW(fd,0) = 0xa8;
			WFIFOW(fd,2) = RFIFOW(fd,2);
			WFIFOW(fd,4) = --p->inventory[indexid-2].amount;
			WFIFOB(fd,6) = 1;
			WFIFOSET(fd,7);
			if (p->inventory[indexid-2].amount <= 0) {
				p->inventory[indexid-2].nameid = 0;
			}
		}
		else {
			WFIFOW(fd,0) = 0xa8;
			WFIFOW(fd,2) = RFIFOW(fd,2);
			WFIFOW(fd,4) = p->inventory[indexid-2].amount;
			WFIFOB(fd,6) = 0;
			WFIFOSET(fd,7);
		}
		break;
	default:
		fp = fopen("Item.log", "a");
		if (fp) {
			fprintf(fp,"USE ITEM: Unknown item type: %d. nameid: %d. dataid: %d.\n",item_type, nameid, dataid);
			fclose(fp);
		}
		break;
	}
}
