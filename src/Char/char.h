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
 Project:       Project Odin Character Server
 Creation Date: Dicember 6, 2003
 Modified Date: October 21, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Advise:	    If you change a structure here make sure you change
		    it at mmo.h. You can end harming zone server by
                sending wrong structure sizes.
------------------------------------------------------------------------*/

#include "config.h"

#define _MMS_
#define AUTH_FIFO_SIZE 256
#define ACCOUNT_ID_INIT 1
#define PET_ID_INIT 1
#define MAX_SERVERS 30
#define MAX_MAP_PER_SERVER 768
#define GLOBAL_REG_NUM 96
#define MAX_INVENTORY 100
#define MAX_STORAGE 100
#define MAX_CART 100
#define MAX_SKILL 336
#define DEFAULT_WALK_SPEED 140
#define MAX_PC_SHOPITEMS 12
#define MAX_PARTYS 2048
#define MAX_PARTY_MEMBERS 12
#define MAX_SKILL_REFINE 100
#define MAX_ZENY 1000000000
#define MAX_CHAR_SLOTS 5

enum flag { CHAR_STATE_WAITAUTH, CHAR_STATE_AUTHOK };
enum network { LAN_CON, WWW_CON, NO_INFO };
const char *logN = "Character Server Version 1.7.1";
const int SERVER_TYPE = 2;

struct {
	int account_id, char_id, login_id1, char_pos;
	int delflag, network;
}auth_fifo[AUTH_FIFO_SIZE];

struct char_session_data
{
	int state;
	int network;
	int account_id, login_id1, login_id2;
	char sex;
	int found_char[MAX_CHAR_SLOTS];
	char email[60];
};

struct mmo_map_server
{
	long ip;
	long xip;
	short port;
	int users;
	char map[MAX_MAP_PER_SERVER][16];
}server[MAX_SERVERS];

struct global_reg
{
	char str[32];
	int value;
};

struct point
{
	char map[16];
	short x, y;
};

struct item
{
	int id;
	short nameid;
	short amount;
	unsigned short equip;
	char identify;
	char refine;
	char attribute;
	int card[4];
};

struct vendlist
{
	unsigned short index;
	unsigned short amount;
	unsigned int price;
};

struct skill
{
	unsigned int id, lv;
	short type_num;
	char type_hit;
	short type_inf;
	int type_pl;
	int type_nk;
	short range;
	short sp;
};

struct mmo_party_member
{
	unsigned char nick[24];
	unsigned char map_name[16];
	short mapno;
	int account_id;
	int char_id;
	unsigned leader : 1;
	signed x : 15;
	unsigned offline : 1;
	signed y : 15;
	int hp;
	int max_hp;
	int fd;
};

struct mmo_party
{
	int party_id;
	unsigned char party_name[24];
	signed char member_count;
	short exp;
	short item;
	short leader_level;
	struct mmo_party_member member[MAX_PARTY_MEMBERS];
};

struct pet_info
{
	char pet_name[24];
	short pet_class;
	short pet_level;
	short pet_hungry;
	short pet_friendly;
	short pet_accessory;
	short pet_name_flag;
	short activity;
	int pet_id;
	int char_id;
	int account_id;
	int mons_id;
	int pet_id_as_npc;
	long pet_npc_id_on_map[MAX_MAP_PER_SERVER];
};

struct mmo_charstatus
{
	int char_id;
	int account_id;
	int base_exp, job_exp, zeny;

	int heal_time, heal_time_skill;
	int sp_time, sp_time_skill;
	int damage_atk;
	int deal_item[10];
	int trade_partner;
	int deal_zeny;
	int range;
	short class;
	short status_point, skill_point;
	int hp, max_hp, sp, max_sp;
	short option_x, option_y, option_z;
	int effect;
	int special;
	int forge_lvl;
	short karma, manner;
	short party_invited, party_status;
	int party_id, storage_id, pet_id;
	unsigned char party_name[24], guild_name[24], class_name[24];
	short hair, hair_color, clothes_color;
	short deal_locked;

	short weapon, shield;
	short head_top, head_mid, head_bottom;
	short storage_status;
	short storage_amount;

	char name[24];
	unsigned char base_level, job_level;
	short str, agi, vit, int_, dex, luk;
	unsigned char char_num;

	unsigned char str2, agi2, vit2, int_2, dex2, luk2;
	unsigned char job_str, job_agi, job_vit, job_int_, job_dex, job_luk;
	unsigned short atk1, atk2, atk2b, matk1, matk2, def1, def2, mdef1, mdef2, hit, flee1, flee2, critical;
	unsigned long aspd;
	int iWeapon_class;
	int matk_bonus;
	short attack_ele, defence_ele;
	short wpn_forged;
	int race_atk_mod[10];
	int race_def_mod[10];
	int race_def_pierce[10];
	int ele_atk_mod[10];
	int ele_def_mod[10];
	int size_atk_mod[3];
	int size_def_mod[3];
	int status_atk_mod[10];
	int status_def_mod[10];
	struct item deal_inventory[10];
	struct point last_point;
	struct point save_point;
	struct point memo_point[3];
	int last_memo_index;
	struct item inventory[MAX_INVENTORY], cart[MAX_CART];
	struct skill skill[MAX_SKILL];
	struct item storage[MAX_STORAGE];
	struct global_reg global_reg[GLOBAL_REG_NUM];
	struct pet_info pet;

	int talking_to_npc;
	int cartcount;
	int cartweight;
	char shopname[80];
	struct vendlist shopitems[MAX_PC_SHOPITEMS];
#ifdef _MMS_
	int lastseen_map_fd;
#endif
};
