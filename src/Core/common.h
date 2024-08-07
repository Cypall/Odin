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

#ifndef _COMMON_H_
#define _COMMON_H_

#define _MMS_
#define AUTH_FIFO_SIZE 256
#define MAX_SERVERS 30
#define MAX_MAP_PER_SERVER 512
#define GLOBAL_REG_NUM 96
#define MAX_STORAGE 100
#define MAX_INVENTORY 100
#define MAX_CART 100
#define MAX_SKILL 336
#define DEFAULT_WALK_SPEED 140
#define MAX_PC_SHOPITEMS 12
#define MAX_PARTYS 2048
#define MAX_PARTY_MEMBERS 12
#define MAX_GUILDS 2048
#define MAX_GUILD_MEMBERS 16
#define MAX_SKILL_REFINE 100
#define MAX_ZENY 1000000000

struct global_reg
{
	char str[32];
	int value;
};

struct point
{
	char map[16];
	short x;
	short y;
};

struct item
{
	int id;
	int nameid;
	short amount;
	int equip;
	short identify;
	short refine;
	short attribute;
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
	int id, lv;
	short type_num;
	short type_hit;
	short type_inf;
	short type_pl;
	short type_nk;
	short range;
	short sp;
	short flag;
};

struct mmo_party_member
{
	long account_id;
	long char_id;
	long leader;
	char nick[24];
	char map_name[16];
	short mapno;
	short x;
	short y;
	int hp;
	int max_hp;
	int offline;
	unsigned int fd;
};

struct mmo_party
{
	long party_id;
	char party_name[24];
	int member_count;
	short exp;
	short item;
	short leader_level;
	struct mmo_party_member member[MAX_PARTY_MEMBERS];
};

struct mmo_guild_member
{
	long account_id;
	long char_id;
	char name[24];
	short hair_style;
	short hair_color;
	short sex;
	short job;
	short lv;
	int present_exp;
	short online;
	short position;
};

struct mmo_guild
{
	long guild_id;
	char guild_name[24];
	char guild_master[24];
	short guild_lv;
	short connum;
	short max_num;
	short average_lv;
	int exp;
	int next_exp;
	int emblem_id;
	struct mmo_guild_member member[MAX_GUILD_MEMBERS];
};

struct pet_info
{
	long pet_id;
	long char_id;
	long account_id;
	char pet_name[24];
	short pet_level;
	short activity;
	int mons_id;
	int pet_class;
	int pet_hungry;
	int pet_friendly;
	int pet_accessory;
	int pet_name_flag;
	int pet_id_as_npc;
	long pet_npc_id_on_map[MAX_MAP_PER_SERVER];
};

struct mmo_charstatus
{
	long char_id, account_id;
	int base_exp, job_exp, zeny;

	short class;
	short status_point, skill_point;
	int hp, max_hp, sp, max_sp;
	int option_x, option_y, option_z, special;
	int effect;
	int karma, manner;
	short party_status;
	long party_id, guild_id, pet_id;
	short hair, hair_color, clothes_color;

	short weapon, shield;
	short head_top, head_mid, head_bottom;
	short storage_amount;

	char name[24];
	unsigned short char_num;
	unsigned char base_level, job_level;
	unsigned short str, agi, vit, int_, dex, luk;
	unsigned char str2, agi2, vit2, int_2, dex2, luk2;

	struct point last_point;
	struct point save_point;
	struct point memo_point[3];
	int last_memo_index;
	struct item inventory[MAX_INVENTORY], cart[MAX_CART];
	struct item storage[MAX_STORAGE];
	struct skill skill[MAX_SKILL];
	struct global_reg global_reg[GLOBAL_REG_NUM];
	struct pet_info pet;

	char party_name[24], guild_name[24], class_name[24];
	short party_invited;
	int heal_time, heal_time_skill;
	int sp_time, sp_time_skill;
	int damage_atk;
	int deal_item[10];
	int trade_partner;
	int deal_zeny;
	int range;
	int forge_lvl;
	short deal_locked;
	short storage_status;
	unsigned char job_str, job_agi, job_vit, job_int_, job_dex, job_luk;
	unsigned short atk1, temp_atk1, atk2, atk2b, matk1, matk2, def1, def2, mdef1, mdef2, hit, flee1, flee2, critical;
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
	int talking_to_npc;
	int cartcount;
	int cartweight;
	char shopname[80];
	struct vendlist shopitems[MAX_PC_SHOPITEMS];
#ifdef _MMS_
	unsigned int lastseen_map_fd;
#endif
};
#endif
