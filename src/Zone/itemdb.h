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

#ifndef ITEMDB_H
#define ITEMDB_H

#define ITEMDB_HASH_SIZE 64

struct item_database
{
	int nameid;
	int next;
	char name[24];
	int type;
	int price;
	int sell;
	int weight;
	int atk;
	int matk;
	int def;
	int mdef;
	int range;
	int slot;
	int str;
	int agi;
	int vit;
	int int_;
	int dex;
	int luk;
	int job;
	int gender;
	int loc;
	int wlv;
	int elv;
	int view;
	int ele;
	int eff;
	int hp1;
	int hp2;
	int sp1;
	int sp2;
	int rare;
	int box;
	int hit,critical,flee,skill_id;
	int equip;    
	short job_equipt[MAX_CLASSES];
}*itemdb; 

struct item_db2 item_database(int item_id);
int itemdb_can_equipt(int nameid, short class);
int itemdb_type(int nameid);
int itemdb_loc(int nameid);
int itemdb_sellvalue(int nameid);
int itemdb_weight(int nameid);
int itemdb_isequip(int nameid);
int itemdb_elv(int nameid);
int itemdb_equip_point(int nameid);
int itemdb_view_point(int nameid);
int search_itemdb_nameid(char *name);
int search_item2(struct map_session_data *sd, int nameid);
int forge_db_index(int nameid);
int itemdb_stype(int nameid);
int skill_forge_db_init(void);
int itemdb_equipment_init(void);
int itemdb_read_itemslottable(void);
int search_itemdb_index(int nameid);
int itemdb_db2_init(void);
int itemdb_init(void);
#endif
