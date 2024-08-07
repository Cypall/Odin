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
#include "grfio.h"
#include "itemdb.h"

static int itemdb_hash[ITEMDB_HASH_SIZE];
static int itemdb_size, itemdb_num;

struct item_db2 ret_item;
struct item_db2 item_database(int item_id)
{
	int i;

	i = search_itemdb_index(item_id);
	ret_item.nameid = itemdb[i].nameid;
	ret_item.type = itemdb[i].type;
	ret_item.sell = itemdb[i].price;
	ret_item.weight = itemdb[i].weight;
	ret_item.atk = itemdb[i].atk;
	ret_item.matk = itemdb[i].matk;
	ret_item.def = itemdb[i].def;
	ret_item.mdef = itemdb[i].mdef;
	ret_item.range = itemdb[i].range;
	ret_item.slot = itemdb[i].slot;
	ret_item.str = itemdb[i].str;
	ret_item.agi = itemdb[i].agi;
	ret_item.vit = itemdb[i].vit;
	ret_item.int_ = itemdb[i].int_;
	ret_item.dex = itemdb[i].dex;
	ret_item.luk = itemdb[i].luk;
	ret_item.job = itemdb[i].job;
	ret_item.gender = itemdb[i].gender;
	ret_item.loc = itemdb[i].loc;
	ret_item.wlv = itemdb[i].wlv;
	ret_item.elv = itemdb[i].elv;
	ret_item.view = itemdb[i].view;
	ret_item.ele = itemdb[i].ele;
	ret_item.eff = itemdb[i].eff;
	ret_item.hp1 = itemdb[i].hp1;
	ret_item.hp2 = itemdb[i].hp2;
	ret_item.sp1 = itemdb[i].sp1;
	ret_item.sp2 = itemdb[i].sp2;
	ret_item.hit = itemdb[i].hit;
	ret_item.critical = itemdb[i].critical;
	ret_item.flee = itemdb[i].flee;
	ret_item.skill_id = itemdb[i].skill_id;
	return ret_item;
}

int itemdb_can_equipt(int nameid, short class)
{
	return itemdb[search_itemdb_index(nameid)].job_equipt[class];
}

int itemdb_type(int nameid)
{
	return itemdb[search_itemdb_index(nameid)].type;
}

int itemdb_loc(int nameid)
{
	return itemdb[search_itemdb_index(nameid)].loc;
}

int itemdb_sellvalue(int nameid)
{
	return itemdb[search_itemdb_index(nameid)].sell;
}

int itemdb_weight(int nameid)
{
	return itemdb[search_itemdb_index(nameid)].weight;
}

int itemdb_isequip(int nameid)
{
	int type = itemdb_type(nameid);
	if (type == 0 || type == 2 || type == 3 || type == 6 || type == 10)
		return 0;

	else
		return itemdb[search_itemdb_index(nameid)].equip;
}

int itemdb_elv(int nameid)
{
	return itemdb[search_itemdb_index(nameid)].elv;
}

int itemdb_equip_point(int nameid)
{
	return itemdb[search_itemdb_index(nameid)].equip;
}

int itemdb_view_point(int nameid)
{
	return itemdb[search_itemdb_index(nameid)].view;
}

int search_itemdb_nameid(char *name)
{
	int i, j;
	
	for (j = 501; j < 10020; j++) {
		i = search_itemdb_index(j);
		if (strcmp(itemdb[i].name, name) == 0)
			return j;

	}
	return -1;
}

int search_item2(struct map_session_data *sd, int nameid)
{
	int i, index = 0;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid == nameid) {
			index = i + 2;
			break;
		}
	}
	return index;
}

int forge_db_index(int nameid)
{
	
	int i;

	for (i = 0; i < MAX_SKILL_REFINE; i++)
		if (forge_db[i].nameid == nameid)
			break;


	return i;
}

int itemdb_stype(int nameid)
{
	if (itemdb_type(nameid) == 4) {
		switch (itemdb_loc(nameid)) {
		case 2: // One Handed
			return LOOK_WEAPON;
			break;
		case 34: // Two Handed
			return LOOK_WEAPON;
			break;
		default:
			printf("%d item: Unknown item loc: %d\n", nameid, itemdb_loc(nameid));
			return LOOK_HAIR_COLOR;
			break;
		}
	}
	else if (itemdb_type(nameid) == 5) {
		switch (itemdb_loc(nameid)) {
		case 0: // Head Bottom
			return LOOK_HEAD_BOTTOM;
			break;
		case 1: // Head Bottom
			return LOOK_HEAD_BOTTOM;
			break;
		case 4: // Garment
			return LOOK_HAIR_COLOR;
			break;
		case 16: // Armor
			return LOOK_HAIR_COLOR;
			break;
		case 32: // Shield
			return LOOK_SHIELD;
			break;
		case 64: // Shoes
			return LOOK_HAIR_COLOR;
			break;
		case 136: // Accessorys
			return LOOK_HAIR_COLOR;
			break;
		case 256: // Head Top
			return LOOK_HEAD_TOP;
			break;
		case 512: // Head Middle
			return LOOK_HEAD_MID;
			break;
		case 513: // Head Top + Middle
			return LOOK_HEAD_TOP;
			break;
		case 769: // Head Top + Middle + Bottom
			return LOOK_HEAD_BOTTOM;
			break;
		default:
			printf("%d item: Unknown item loc: %d\n", nameid, itemdb_loc(nameid));
			return LOOK_HAIR_COLOR;
			break;
		}
	}
	else if (itemdb_type(nameid) == 8) // Pet equipment
		return LOOK_HAIR_COLOR;

	else if (itemdb_type(nameid) == 10) // Arrows equipment
		return LOOK_HAIR_COLOR;

	else {
		printf("%d item: Unknown item Type: %d\n", nameid, itemdb_type(nameid));
		return LOOK_HAIR_COLOR;
	}
	return LOOK_SHOES;
}

int skill_forge_db_init(void)
{
	int i = 0;
	int nameid, req_skill, itemlv, mat_id[5], mat_amount[5];
	char line[2040];
	FILE *fp = fopen("data/databases/forge_db.txt","rt");
	if (fp) {
		while (fgets(line, 2039, fp)) {
			if (line[0] == '/' && line[1] == '/')
				continue;

			sscanf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
					  &nameid, &itemlv,&req_skill, &mat_id[0], &mat_amount[0],
				        &mat_id[1], &mat_amount[1], &mat_id[2], &mat_amount[2],
				        &mat_id[3], &mat_amount[3], &mat_id[4], &mat_amount[4]);
				
			forge_db[i].nameid = nameid;
			forge_db[i].itemlv = itemlv;
			forge_db[i].req_skill = req_skill;
			forge_db[i].mat_id[0] = mat_id[0];
			forge_db[i].mat_amount[0] = mat_amount[0];
			forge_db[i].mat_id[1] = mat_id[1];
			forge_db[i].mat_amount[1] = mat_amount[1];
			forge_db[i].mat_id[2] = mat_id[2];
			forge_db[i].mat_amount[2] = mat_amount[2];
			forge_db[i].mat_id[3] = mat_id[3];
			forge_db[i].mat_amount[3] = mat_amount[3];
			forge_db[i].mat_id[4] = mat_id[4];
			forge_db[i].mat_amount[4] = mat_amount[4];
			i++;
		}
		fclose(fp);	
	}
	else {
		debug_output("skill_forge_db_init: Fail to open 'data/databases/forge_db.txt'.\n");
		abort();
	}
	return 0;
}

int itemdb_equipment_init(void)
{
	int item_id;
	int i, n, j = 0;
	int job[MAX_CLASSES];
	char name[128], line[1024];
	FILE *fp = fopen("data/tables/item_equip.txt", "rt");
	if (fp) {
		while (fgets(line, 1023, fp)) {
			if (line[0] == '/' && line[1] == '/')
				continue;

			for (i = 0; i < MAX_CLASSES; i++) 
				job[i] = 111;

			if ((n = sscanf(line, "%d,%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &item_id, name, 
						     &job[0], &job[1], &job[2], &job[3], &job[4], &job[5], &job[6], &job[7], &job[8], 
						     &job[9], &job[10], &job[11], &job[12], &job[13], &job[14], &job[15], &job[16],
						     &job[17], &job[18], &job[19], &job[20], &job[21], &job[22], &job[23])) < 3) continue;
			
			j = search_itemdb_index(item_id);
			for (i = 0; i < MAX_CLASSES; i++)
				itemdb[j].job_equipt[i] = 0;

			if (job[0] == 98)
				for (i = 1; i < MAX_CLASSES; i++)  
					itemdb[j].job_equipt[i] = 1;


			else if (job[0] == 99)
				for( i = 0; i < MAX_CLASSES; i++)  
					itemdb[j].job_equipt[i] = 1;


			else
				for (i = 0; i < n ; i++)
					if (job[i] != 111) 
						itemdb[j].job_equipt[job[i]] = 1;	


		}
		fclose(fp);
	}
	else {
		debug_output("itemdb_equipment_init: Fail to open 'data/tables/table_equip.txt'.\n");
		abort();
	}	
	return 0;
}

int itemdb_read_itemslottable(void)
{
	int s;
	char *buf, *p;
	buf = grfio_read("data\\itemslottable.txt");
	if (buf == NULL)
		return -1;

	s = grfio_size("data\\itemslottable.txt");
	buf[s] = 0;
	for (p = buf; p - buf < s;) { 
		int nameid, equip;

		sscanf(p, "%d#%d#", &nameid, &equip);
		itemdb[search_itemdb_index(nameid)].equip = equip;
		p = strchr(p,10);
		if(!p)
			break;

		p++;
		p = strchr(p,10);
		if(!p)
			break;

		p++;
	}
	free(buf);
	buf = NULL;
	return 0;
}

int search_itemdb_index(int nameid)
{
	int i, j;

	for (i = itemdb_hash[nameid % ITEMDB_HASH_SIZE]; i >= 0; i = itemdb[i].next)
		if (itemdb[i].nameid == nameid)
			return i;


	if (itemdb_size <= itemdb_num) {
		itemdb_size += ITEMDB_HASH_SIZE;
		itemdb = realloc(itemdb, sizeof(*itemdb) * itemdb_size);
	}
	i = itemdb_num;
	itemdb_num++;
	itemdb[i].nameid = nameid;
	itemdb[i].next = itemdb_hash[nameid % ITEMDB_HASH_SIZE];
	itemdb_hash[nameid % ITEMDB_HASH_SIZE] = i;

	// init on first search
	itemdb[i].type = 9;
	itemdb[i].job = 0;
	itemdb[i].equip = 0;
	itemdb[i].weight = 10;
	itemdb[i].atk = 0;
	itemdb[i].def = 0;
	itemdb[i].slot = 0;
	itemdb[i].view = 0;

	// init value fix
	itemdb[i].price = 10;
	itemdb[i].sell = 1;
	itemdb[i].matk = 0;
	itemdb[i].mdef = 0;
	itemdb[i].range = 0;
	itemdb[i].str = 0;
	itemdb[i].agi = 0;
	itemdb[i].vit = 0;
	itemdb[i].int_ = 0;
	itemdb[i].dex = 0;
	itemdb[i].luk = 0;
	itemdb[i].gender = 0;
	itemdb[i].loc = 0;
	itemdb[i].wlv = 0;
	itemdb[i].elv = 0;
	itemdb[i].ele = 0;
	itemdb[i].eff = 0;
	itemdb[i].hp1 = 0;
	itemdb[i].hp2 = 0;
	itemdb[i].sp1 = 0;
	itemdb[i].sp2 = 0;
	itemdb[i].rare = 0;
	itemdb[i].box = 0;
	itemdb[i].hit = 0,
	itemdb[i].critical = 0;
	itemdb[i].flee = 0;
	itemdb[i].skill_id = 0;
	
	// init equip value
	for (j = 0; j < MAX_CLASSES; j++) 
		itemdb[i].job_equipt[j] = 0;
	
	return i;
}

int itemdb_db2_init(void)
{
	int item_index;
	int item_id, Type, Price, Sell, Weight, ATK, MATK, DEF, MDEF, Range, Slot;
	int STR, AGI, VIT, INT, DEX, LUK, Job, Gender, Loc, wLV, eLV, View, Ele, Eff;
	int HP1, HP2, SP1, SP2, Rare, Box, HIT, CRITICAL, FLEE, SKILL_ID, weap_typ;
	char name[128], JName[256];
	char line[2040];
	FILE *fp;

	fp = fopen("data/databases/item_db.txt", "rt");
	if (fp) {
		while (fgets(line, 2039, fp)) {
			if (line[0] == '/' && line[1] == '/')
				continue;

			item_id = Type = Price = Sell = Weight = ATK = MATK = DEF = MDEF = Range = Slot
				  = STR = AGI = VIT = INT = DEX = LUK = Job = Gender = Loc = wLV = eLV 
				  = View = Ele = Eff = HP1 = HP2 = SP1 = SP2 = Rare = Box = HIT = CRITICAL
				  = FLEE = SKILL_ID = -1;
			sscanf(line,"%d,%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				       &item_id, name, JName, &Type, &Price, &Sell, &Weight, &ATK, &MATK, &DEF,
			      	 &MDEF, &Range, &Slot, &STR, &AGI, &VIT, &INT, &DEX, &LUK, &Job,
				       &Gender, &Loc, &wLV, &eLV, &View, &Ele, &Eff, &HP1, &HP2, &SP1,
				       &SP2, &Rare, &Box, &HIT, &CRITICAL, &FLEE, &SKILL_ID, &weap_typ);

			if (item_id < 0) 
				continue;

			item_index = search_itemdb_index(item_id);
			strcpy(itemdb[item_index].name, name);
			itemdb[item_index].type = Type;
			itemdb[item_index].price = Price;
			itemdb[item_index].sell = Sell;
			itemdb[item_index].weight = Weight;
			itemdb[item_index].atk = ATK;
			itemdb[item_index].matk = MATK;
			itemdb[item_index].def = DEF;
			itemdb[item_index].mdef = MDEF;
			itemdb[item_index].range = Range;
			itemdb[item_index].slot = Slot;
			itemdb[item_index].str = STR;
			itemdb[item_index].agi = AGI;
			itemdb[item_index].vit = VIT;
			itemdb[item_index].int_ = INT;
			itemdb[item_index].dex = DEX;
			itemdb[item_index].luk = LUK;
			itemdb[item_index].job = Job;
			itemdb[item_index].gender = Gender;
			itemdb[item_index].loc = Loc;
			if (itemdb[item_index].equip != Loc)
				itemdb[item_index].equip = Loc;

			itemdb[item_index].wlv = wLV;
			itemdb[item_index].elv = eLV;
			itemdb[item_index].view = View;
			itemdb[item_index].ele = Ele;
			itemdb[item_index].eff = Eff;
			itemdb[item_index].hp1 = HP1;
			itemdb[item_index].hp2 = HP2;
			itemdb[item_index].sp1 = SP1;
			itemdb[item_index].sp2 = SP2;
			itemdb[item_index].rare = Rare;
			itemdb[item_index].box = Box;
			itemdb[item_index].hit = HIT,
			itemdb[item_index].critical = CRITICAL;
			itemdb[item_index].flee = FLEE;
			itemdb[item_index].skill_id = SKILL_ID;
		}
		fclose(fp);
	}
	else {
		debug_output("itemdb_db2_init: Fail to open 'data/database/item_db.txt'.\n");
		abort();
	}
	return 0;
}

int itemdb_init(void)
{
	int i;

	for (i = 0; i < ITEMDB_HASH_SIZE; i++)
		itemdb_hash[i] = -1;

	itemdb_size = ITEMDB_HASH_SIZE;
	itemdb_num = 0;
	itemdb = malloc(sizeof(*itemdb) * itemdb_size);
  
	itemdb_db2_init();
	itemdb_read_itemslottable();
	itemdb_equipment_init();
	skill_forge_db_init();
#if 0
	for (i = 0; i < ITEMDB_HASH_SIZE; i++) {
		int j, c;

		for (j = itemdb_hash[i], c = 0; j >= 0; j = itemdb[j].next, c++);
		printf("%4d", c);
		if (i % 16 == 15) 
			printf("\n");

	}
	abort();
#endif
	return 0;
}
