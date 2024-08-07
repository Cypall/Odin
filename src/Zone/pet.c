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
  Modified Date: November 22, 2004
  Description:   Ragnarok Online Server Emulator
  ------------------------------------------------------------------------*/

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "npc.h"
#include "item.h"
#include "itemdb.h"
#include "pet.h"
#include "save.h"

unsigned char buf[256];
int *init_npc_id;
int feed_the_pet(int tid, unsigned int tick, int fd, int data);

/*======================================
 *	PET: Pet Functions
 *--------------------------------------
 */

void pet_store_init_npc_id(int *id)
{
	init_npc_id = id;
}

int pet_init(struct map_session_data *sd)
{
	int i, *id, current_id;

	if (sd->status.pet.pet_id_as_npc == 0) {
		id = return_npc_current_id();
		current_id = npc_add_npc_id();
		if ((*id - *init_npc_id) < MAX_NPC_PER_MAP) {
			for (i = 0; i < MAX_MONS; i++) {
				if (pet_dat[i].account_id == sd->account_id && pet_dat[i].char_id == sd->char_id && pet_dat[i].pet_id == sd->status.pet_id)
					break;
			}
			if (i != MAX_MONS) {
				sd->status.pet.pet_id = pet_dat[i].pet_id;
				sd->status.pet.pet_class = pet_dat[i].pet_class;
				memcpy(sd->status.pet.pet_name, pet_dat[i].pet_name, 24);
				sd->status.pet.pet_level = pet_dat[i].pet_level;
				sd->status.pet.mons_id = pet_dat[i].mons_id;
				sd->status.pet.pet_accessory = pet_dat[i].pet_accessory;
				sd->status.pet.pet_friendly = pet_dat[i].pet_friendly;
				sd->status.pet.pet_hungry = pet_dat[i].pet_hungry;
				sd->status.pet.pet_name_flag = pet_dat[i].pet_name_flag;
				sd->status.pet.activity = pet_dat[i].activity;
			}
			sd->status.pet.pet_id_as_npc = current_id;
			for (i = 0; i < MAX_MAP_PER_SERVER; i++)
				sd->status.pet.pet_npc_id_on_map[i] = 0;

			if (sd->feed_pet_timer == 0) {
				i = search_pet_id(sd->status.pet.pet_class);
				sd->feed_pet_timer = add_timer(gettick() + pet_db[i].hungry_delay * 60000, feed_the_pet, sd->fd, pet_db[i].hungry_delay);
			}
		}
		else {
			sd->status.pet.pet_id_as_npc = 0;
			sd->status.pet.activity = 0;
		}
	}
	return 0;
}

int mmo_map_resetpet_stat(struct map_session_data *sd)
{
	sd->status.pet.pet_class = 0;
	strcpy(sd->status.pet.pet_name, "");
	sd->status.pet.pet_level = 0;
	sd->status.pet.mons_id = 0;
	sd->status.pet.pet_accessory = 0;
	sd->status.pet.pet_friendly = 0;
	sd->status.pet.pet_hungry = 0;
	sd->status.pet.pet_name_flag = 0;
	sd->status.pet.activity = 0;
	set_monster_no_point(sd->mapno, sd->status.pet.pet_npc_id_on_map[sd->mapno]);
	del_block(&map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block);
	WBUFW(buf, 0) = 0x80;
	WBUFL(buf, 2) = sd->status.pet.pet_id_as_npc;
	WBUFB(buf, 6) = 0;
	mmo_map_sendarea(sd->fd, buf, packet_len_table[0x80], 0);
	return 0;
}

int mmo_map_pet_catch(int fd, struct map_session_data *sd, int index)
{
	int i, j;
	int pet_chance = (10 + rand() % 10) * 2, mons_pet_id = 0, flag = 0;
	struct item tmp_item;

	for (j = 0; j < map_data[sd->mapno].npc_num; j++) {
		if (map_data[sd->mapno].npc[j]->block.subtype != MONS)
			continue;

		else {
			if (map_data[sd->mapno].npc[j]->id == index) {
				mons_pet_id = map_data[sd->mapno].npc[j]->class;
				break;
			}
		}
	}
	if (mons_pet_id == sd->status.pet.mons_id) {
		if (pet_chance > rand() % 100) {
			flag = 1;
			memset(&tmp_item, 0, sizeof(tmp_item));
			i = search_pet_id(mons_pet_id);
			tmp_item.nameid = pet_db[i].egg_id;
			tmp_item.identify = 1;
			tmp_item.amount = 1;
			mmo_map_get_item(fd, &tmp_item);
		}
	}
	WFIFOW(fd, 0) = 0x1a0;
	WFIFOB(fd, 2) = flag;
	WFIFOSET(fd, packet_len_table[0x1a0]);

	if (flag) {
		delete_timer(map_data[sd->mapno].npc[j]->u.mons.walktimer, mons_walk);
		map_data[sd->mapno].npc[j]->u.mons.walktimer = 0;
		map_data[sd->mapno].npc[j]->u.mons.speed = 0;
		set_monster_no_point(sd->mapno, j);
		del_block(&map_data[sd->mapno].npc[j]->block);
		WBUFW(buf, 0) = 0x80;
		WBUFL(buf, 2) = map_data[sd->mapno].npc[j]->id;
		WBUFB(buf, 6) = 2;
		mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
	}
	return 0;
}

enum { PSTAT, PFEED, PPER, PREGG, PREQUIP };
int mmp_map_pet_stat_select(int fd, struct map_session_data *sd, int type)
{
	int i, itemid = 0, index = 0, hungry_delay = 0;

	switch (type) {
	case PSTAT:
		WFIFOW(fd, 0) = 0x1a2;
		memcpy(WFIFOP(fd, 2), sd->status.pet.pet_name, 24);
		WFIFOB(fd, 26) = sd->status.pet.pet_name_flag;
		WFIFOW(fd, 27) = sd->status.pet.pet_level;
		WFIFOW(fd, 29) = sd->status.pet.pet_hungry;
		WFIFOW(fd, 31) = sd->status.pet.pet_friendly;
		WFIFOW(fd, 33) = sd->status.pet.pet_accessory;
		WFIFOSET(fd, packet_len_table[0x1a2]);
		break;

	case PFEED:
		i = search_pet_id(sd->status.pet.pet_class);
		itemid = pet_db[i].food_id;
		hungry_delay = pet_db[i].hungry_delay;

		if (itemid > 0)
			index = search_item2(sd, itemid);

		else
			index = 0;

		if (index) {
			mmo_map_lose_item(fd, 0, index, 1);
			sd->status.pet.pet_hungry += 80;
			if (sd->status.pet.pet_hungry > 100)
				sd->status.pet.pet_hungry = 100;

			WFIFOW(fd, 0) = 0x1a4;
			WFIFOB(fd, 2) = 2;
			WFIFOL(fd, 3) = sd->status.pet.pet_id_as_npc;
			WFIFOL(fd, 7) = sd->status.pet.pet_hungry;
			WFIFOSET(fd, packet_len_table[0x1a4]);

			sd->status.pet.pet_friendly += 20;
			if (sd->status.pet.pet_friendly > 1000)
				sd->status.pet.pet_friendly = 1000;

			WFIFOW(fd, 0) = 0x1a4;
			WFIFOB(fd, 2) = 1;
			WFIFOL(fd, 3) = sd->status.pet.pet_id_as_npc;
			WFIFOL(fd, 7) = sd->status.pet.pet_friendly;
			WFIFOSET(fd, packet_len_table[0x1a4]);

			WFIFOW(fd, 0) = 0x1a3;
			WFIFOB(fd, 2) = 1;
			WFIFOW(fd, 3) = pet_db[i].food_id;
			WFIFOSET(fd, packet_len_table[0x1a3]);

			sd->feed_pet_timer = add_timer(gettick() + hungry_delay * 60000, feed_the_pet, fd, hungry_delay);
		}
		else {
			WFIFOW(fd, 0) = 0x1a3;
			WFIFOB(fd, 2) = 0;
			WFIFOW(fd, 3) = pet_db[i].food_id;
			WFIFOSET(fd, packet_len_table[0x1a3]);
		}
		break;

	case PPER:
		memset(buf, 0, packet_len_table[0x1a4]);
		WBUFW(buf, 0) = 0x1a4;
		WBUFB(buf, 2) = 4;
		WBUFL(buf, 3) = sd->status.pet.pet_id_as_npc;
		WBUFL(buf, 7) = 2;
		mmo_map_sendarea(fd, buf, packet_len_table[0x1a4], 0);
		break;

	case PREGG:
		set_monster_no_point(sd->mapno, sd->status.pet.pet_npc_id_on_map[sd->mapno]);
		del_block(&map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->block);
		WBUFW(buf, 0) = 0x80;
		WBUFL(buf, 2) = sd->status.pet.pet_id_as_npc;
		WBUFB(buf, 6) = 0;
		mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
		sd->status.pet.pet_hungry = 0;
		sd->status.pet.activity = 0;
		mmo_pet_save(sd);
		break;

	case PREQUIP:
		memset(buf, 0, packet_len_table[0x1a4]);
		WBUFW(buf, 0) = 0x1a4;
		WBUFB(buf, 2) = 3;
		WBUFL(buf, 3) = sd->status.pet.pet_id_as_npc;
		WBUFL(buf, 7) = 0;
		mmo_map_sendarea(fd, buf, packet_len_table[0x1a4], 0);
		sd->status.pet.pet_accessory = 0;
		break;
	}
	return 0;
}

int mmo_map_pet_name(int fd, struct map_session_data *sd)
{
	if (sd->status.pet.pet_name_flag == 0) {
		sd->status.pet.pet_name_flag = 1;
		WBUFW(buf, 0) = 0x95;
		WBUFL(buf, 2) = sd->status.pet.pet_id_as_npc;
		memcpy(WBUFP(buf, 6), RFIFOP(fd, 2), 24);
		mmo_map_sendall(fd, buf, packet_len_table[0x95], 0);
		memcpy(sd->status.pet.pet_name, WBUFP(buf, 6), 24);
		memcpy(map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]]->name, sd->status.pet.pet_name, 24);
	}
	return 0;
}

int mmo_map_pet_act(int fd, struct map_session_data *sd, int index)
{
	int i, j;
	int name_id;
	unsigned char buf[256];

	if ((sd->status.pet.activity == 0) && (sd->status.pet.pet_id_as_npc != 0) && (map_data[sd->mapno].npc_num < MAX_NPC_PER_MAP)) {
		if (index)
			mmo_map_lose_item(fd, 0, index, 1);

		for (i = 0; i < MAX_MONS; i++) {
			if (pet_dat[i].account_id == sd->account_id
			    && pet_dat[i].char_id == sd->char_id
			    && pet_dat[i].pet_id == sd->status.pet_id) {
				name_id = sd->status.inventory[index].nameid;
				j = search_mons_id(name_id);
				if (sd->status.pet.pet_class == pet_db[j].class)
					break;
			}
		}
		if (i != MAX_MONS) {
			memcpy(sd->status.pet.pet_name, pet_dat[i].pet_name, 24);
			sd->status.pet.pet_level = pet_dat[i].pet_level;
			sd->status.pet.pet_hungry = pet_dat[i].pet_hungry;
			sd->status.pet.pet_friendly = pet_dat[i].pet_friendly;
			sd->status.pet.pet_accessory = pet_dat[i].pet_accessory;
			sd->status.pet.activity = 1;
			sd->status.pet.pet_class = pet_dat[i].pet_class;
			sd->feed_pet_timer = add_timer(gettick() + pet_db[i].hungry_delay * 60000, feed_the_pet, fd, pet_db[i].hungry_delay);
		}
		else {
			name_id = sd->status.inventory[index].nameid;
			j = search_mons_id(name_id);
			strncpy(sd->status.pet.pet_name, pet_db[j].name, 24);
			sd->status.pet.pet_name_flag = 0;
			sd->status.pet.pet_level = 1;
			sd->status.pet.pet_hungry = 0;
			sd->status.pet.pet_friendly = 0xff;
			sd->status.pet.pet_accessory = 0;
			sd->status.pet.activity = 1;
			sd->status.pet.pet_class = pet_db[j].class;
			sd->feed_pet_timer = add_timer(gettick() + pet_db[j].hungry_delay * 60000, feed_the_pet, fd, pet_db[j].hungry_delay);
		}
		if (sd->status.pet.pet_npc_id_on_map[sd->mapno] == 0) {
			sd->status.pet.pet_npc_id_on_map[sd->mapno] = map_data[sd->mapno].npc_num;
			map_data[sd->mapno].npc_num++;
		}
		map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]] = malloc(sizeof(struct npc_data));
		struct npc_data *pet = map_data[sd->mapno].npc[sd->status.pet.pet_npc_id_on_map[sd->mapno]];
		pet->m = sd->mapno;
		pet->x = sd->x;
		pet->y = sd->y;
		pet->u.mons.speed = sd->speed;
		pet->u.mons.equip = sd->status.pet.pet_accessory;
		pet->dir = sd->dir;
		pet->class = sd->status.pet.pet_class;
		pet->id = sd->status.pet.pet_id_as_npc;
		memcpy(pet->name, sd->status.pet.pet_name, 24);

		mmo_map_set_npc(pet, buf);
		mmo_map_sendarea_mxy(sd->mapno, pet->x, pet->y, buf, packet_len_table[0x78]);

		WFIFOW(fd, 0) = 0x1a4;
		WFIFOB(fd, 2) = 0;
		WFIFOL(fd, 3) = sd->status.pet.pet_id_as_npc;
		WFIFOL(fd, 7) = 0;
		WFIFOSET(fd, packet_len_table[0x1a4]);

		WFIFOW(fd, 0) = 0x1a4;
		WFIFOB(fd, 2) = 5;
		WFIFOL(fd, 3) = sd->status.pet.pet_id_as_npc;
		WFIFOL(fd, 7) = 14;
		WFIFOSET(fd, packet_len_table[0x1a4]);

		WFIFOW(fd, 0) = 0x1a2;
		memcpy(WFIFOP(fd, 2),sd->status.pet.pet_name,24);
		WFIFOB(fd, 26) = sd->status.pet.pet_name_flag;
		WFIFOW(fd, 27) = sd->status.pet.pet_level;
		WFIFOW(fd, 29) = sd->status.pet.pet_hungry;;
		WFIFOW(fd, 31) = sd->status.pet.pet_friendly;
		WFIFOW(fd, 33) = sd->status.pet.pet_accessory;
		WFIFOSET(fd, packet_len_table[0x1a2]);

		pet->block.next = NULL;
		pet->block.prev = NULL;
		pet->block.subtype = BL_PET;
		add_block_npc(sd->mapno, sd->status.pet.pet_npc_id_on_map[sd->mapno]);
	}
	return 0;
}

int mmo_map_pet_emotion(int fd, struct map_session_data *sd, int emotion)
{
	WBUFW(buf, 0) = 0x1aa;
	WBUFL(buf, 2) = sd->status.pet.pet_id_as_npc;
	WBUFL(buf, 6) = emotion;
	mmo_map_sendarea(fd, buf, packet_len_table[0x1aa], 0);
	return 0;
}

int mmo_pet_equip(struct map_session_data *sd, int item_id)
{
	int index = 0;
	int equip_able = 0;

	if (sd->status.pet.activity == 1) {
		index = search_pet_id(sd->status.pet.pet_class);
		if (sd->status.inventory[item_id - 2].nameid == pet_db[index].accesory_id)
			equip_able = 1;

		if (equip_able) {
			WBUFW(buf, 0) = 0x1a4;
			WBUFB(buf, 2) = 3;
			WBUFL(buf, 3) = sd->status.pet.pet_id_as_npc;
			WBUFL(buf, 7) = sd->status.inventory[item_id - 2].nameid;
			mmo_map_sendall(sd->fd, buf, packet_len_table[0x1a4], 0);
			sd->status.pet.pet_accessory = sd->status.inventory[item_id - 2].nameid;
		}
	}
	return 0;
}

/*======================================
 *	PET: Pet Sub Functions
 *--------------------------------------
 */

int search_pet_item_id(int item_id)
{
	int i;

	for (i = 0; i < MAX_PET_DB; i++) {
		if (pet_db[i].item_id == item_id)
			return i;
	}
	return 0;
}

int search_mons_id(int egg_id)
{
	int i;

	for (i = 0; i < MAX_PET_DB; i++) {
		if (pet_db[i].egg_id == egg_id)
			return i;
	}
	return 0;
}

int search_pet_id(int class)
{
	int i;

	for (i = 0; i < MAX_PET_DB; i++) {
		if (pet_db[i].class == class)
			return i;
	}
	return 0;
}

int search_pet_class_item(int item)
{
	int i;

	for (i = 0; i < MAX_PET_DB; i++) {
		if (pet_db[i].item_id == item)
			return pet_db[i].class;
	}
	return 0;
}

int feed_the_pet(int tid, unsigned int tick, int fd, int data)
{
	struct map_session_data *sd;

	tid = 0;
	tick = 0;
	if (session[fd] && (sd = session[fd]->session_data)) {
		if (sd->feed_pet_timer > 0) {
			delete_timer(sd->feed_pet_timer, feed_the_pet);
			sd->feed_pet_timer = 0;
		}
		if (sd->status.pet.activity) {
			if (sd->status.pet.pet_hungry > 60) {
				sd->status.pet.pet_hungry /= 2;
				sd->status.pet.pet_friendly -= 40;
			}
			else {
				sd->status.pet.pet_hungry -= 10;
				sd->status.pet.pet_friendly -= 40;
			}
			if (sd->status.pet.pet_hungry < 0)
				sd->status.pet.pet_hungry = 0;

			else if (sd->status.pet.pet_friendly < 0)
				sd->status.pet.pet_friendly = 0;

			WFIFOW(fd, 0) = 0x1a4;
			WFIFOB(fd, 2) = 2;
			WFIFOL(fd, 3) = sd->status.pet.pet_id_as_npc;
			WFIFOL(fd, 7) = sd->status.pet.pet_hungry;
			WFIFOSET(fd, packet_len_table[0x1a4]);

			WFIFOW(fd, 0) = 0x1a4;
			WFIFOB(fd, 2) = 1;
			WFIFOL(fd, 3) = sd->status.pet.pet_id_as_npc;
			WFIFOL(fd, 7) = sd->status.pet.pet_friendly;
			WFIFOSET(fd, packet_len_table[0x1a4]);

			if (sd->status.pet.pet_friendly == 0)
				mmo_map_resetpet_stat(sd);

			else
				sd->feed_pet_timer = add_timer(gettick() + data * 60000, feed_the_pet, fd, data);
		}
	}
	return 0;
}

/*======================================
 *	PET: Pet Initialize
 *--------------------------------------
 */

void mmo_pet_db_init(void)
{
	int i = 0, x;
	char line[1024];
	int tmp[13];
	char name[24];
	FILE *fp = NULL;

	fp = fopen("data/databases/pet_db.txt", "r");
	printf("Loading Pets Data...          ");
	if (fp) {
		while(fgets(line, 1023, fp)) {
			if ((line[0] == '/') && (line[1] == '/'))
				continue;

			if ((x = sscanf(line,"%d,%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
					&tmp[0], name, &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5],
					&tmp[6], &tmp[7], &tmp[8], &tmp[9], &tmp[10], &tmp[11], &tmp[12])) != 14) {
				printf("**Error: pet_db.txt is corrupted!**\n");
				return;
			}
			pet_db[i].class = tmp[0];
			strncpy(pet_db[i].name, name, 24);
			pet_db[i].item_id = tmp[1];
			pet_db[i].egg_id = tmp[2];
			pet_db[i].accesory_id = tmp[3];
			pet_db[i].food_id = tmp[4];
			pet_db[i].fullness = tmp[5];
			pet_db[i].hungry_delay = tmp[6];
			pet_db[i].r_hungry = tmp[7];
			pet_db[i].r_full = tmp[8];
			pet_db[i].friendly = tmp[9];
			pet_db[i].die = tmp[10];
			pet_db[i].capture = tmp[11];
			pet_db[i].speed = tmp[12];
			i++;
		}
		printf("Done\n");
		fclose(fp);
	}
	else {
		printf ("**Error: Cannot load pet_db.txt!**\n");
		sleep(1);
		exit(0);
	}
}
