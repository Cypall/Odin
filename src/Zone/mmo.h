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
 Module:        Version 1.7.0 SP1
 Author:        Odin Developer Team Copyrights (c) 2004
 Project:       Project Odin Zone Server
 Creation Date: Dicember 6, 2003
 Modified Date: Semptember 3, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#define _MMS_
#define MAX_SERVERS 30
#define MAX_MAP 768
#define MAX_MAP_PER_SERVER 768
#define MAX_NPC_PER_MAP 512
#define BLOCK_SIZE 8
#define AREA_SIZE 2
#define LOCAL_REG_NUM 16
#define GLOBAL_REG_NUM 96
#define DEFAULT_WALK_SPEED 140
#define DEFAULT_MONSTER_WALK_SPEED 200
#define MAX_STORAGE 100
#define MAX_INVENTORY 100
#define MAX_CART 100
#define MAX_SKILL 336
#define MAX_CLASSES 24
#define MAX_PC_SHOPITEMS 12
#define MAX_PARTYS 2048
#define MAX_PARTY_MEMBERS 12
#define MAX_GUILDS 2048
#define MAX_MONS 4000
#define MAX_OBJECTS 50000
#define MAX_SKILL_REFINE 100
#define MAX_MAP_WARPSKILL 155000
#define MAX_ZENY 1000000000
#define MAX_LOOT 10
#define LIFETIME_FLOORITEM 60

#define add_block_npc(m, n) {map_data[m].npc[n]->block.type = BL_NPC; \
                             add_block(&map_data[m].npc[n]->block, m, map_data[m].npc[n]->x, map_data[m].npc[n]->y);}

enum { LAN_CON, WWW_CON, NO_INFO };
enum { BL_NUL, BL_PC, BL_NPC, BL_ITEM, BL_SKILL, BL_PET };
enum { MONS, WARP, SHOP, SCRIPT };
enum { FS_NULL, FS_SANCT, FS_MAGNUS, FS_QUAG, FS_TRAP, FS_WARP, FS_ICEWALL };
enum { SP_SPEED,SP_BASEEXP,SP_JOBEXP,SP_KARMA,SP_MANNER,SP_HP,SP_MAXHP,SP_SP,
	 SP_MAXSP,SP_STATUSPOINT,SP_0a,SP_BASELEVEL,SP_SKILLPOINT,SP_STR,SP_AGI,SP_VIT,
	 SP_INT,SP_DEX,SP_LUK,SP_13,SP_ZENY,SP_15,SP_NEXTBASEEXP,SP_NEXTJOBEXP,
	 SP_WEIGHT,SP_MAXWEIGHT,SP_1a,SP_1b,SP_1c,SP_1d,SP_1e,SP_1f,
	 SP_USTR,SP_UAGI,SP_UVIT,SP_UINT,SP_UDEX,SP_ULUK,SP_26,SP_27,
	 SP_28,SP_ATK1,SP_ATK2,SP_MATK1,SP_MATK2,SP_DEF1,SP_DEF2,SP_MDEF1,
	 SP_MDEF2,SP_HIT,SP_FLEE1,SP_FLEE2,SP_CRITICAL,SP_ASPD,SP_36,SP_JOBLEVEL,SP_GETJOB,
	 SP_SCRIPT_FLAG,SP_CHECKSTORAGE,SP_CHECKCART,SP_CHECKFALCON, SP_CHECKPECOPECO,SP_ACCOUNTID,SP_SEX,SP_HASCART,
	 SP_GETWLVL,SP_RANDOM };
enum { LOOK_BASE, LOOK_HAIR, LOOK_WEAPON, LOOK_HEAD_BOTTOM, LOOK_HEAD_TOP, LOOK_HEAD_MID, LOOK_HAIR_COLOR, LOOK_CLOTHES_COLOR, LOOK_SHIELD, LOOK_SHOES };
enum { NOVICE, SWORDMAN, MAGE, ARCHER, ACOLYTE, MERCHANT, THIEF, KNIGHT, PRIEST, WIZARD, BACKSMITH, HUNTER, ASSASSIN, CRUSADER = 14, MONK, SAGE, ROGUE,
	 ALCHEMIST, BARD, DANCER, CRUSADER_PECOPECO, WEDDING, SUPER_NOVICE = 23 };
enum { PVP, MEMO, BRANCH, TELEPORT };

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

struct map_flag
{
	char map_name[16];
	short pvp;
	short memo;
	short teleport;
	short branch;
};

struct mmo_tmp_path
{
	short x, y, dist, before, cost;
	char dir, flag;
};

struct gat_1cell
{
	float high[4];
	int type;
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

struct skill_forge_db
{
	int nameid;
	int req_skill, itemlv;
	int mat_id[5], mat_amount[5];
}forge_db[MAX_SKILL_REFINE];

struct item_db2
{
	int nameid;
	int type;
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
	int hit, critical, flee, skill_id;
	int weap_typ;
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

struct mmo_guild_member
{
	int account_id;
	int char_id;
	short hair_style;
	short hair_color;
	short sex;
	short job;
	short lv;
	int present_exp;
	short online;
	short position;
	char name[24];
};

struct mmo_guild
{
	int guild_id;
	char guild_name[24];
	short guild_lv;
	short connum;
	short max_num;
	short average_lv;
	char guild_master[24];
	int exp;
	int next_exp;
	int emblem_id;
	struct mmo_guild_member member[16];
};

struct pet_info
{
	char pet_name[24];
	short pet_class;
	short pet_level;
	short pet_hungry;
	short pet_friendly;
	short pet_accessory;
	char pet_name_flag;
	char activity;
	int pet_id;
	int char_id;
	int account_id;
	int mons_id;
	int pet_id_as_npc;
	int pet_npc_id_on_map[MAX_MAP_PER_SERVER];
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

struct block_list
{
	struct block_list *next, *prev;
	int type, subtype;
};

#ifdef _MMS_
struct map_servers
{
	long ip;
	short port;
	char map[16];
};
#endif

struct mmo_map_data
{
	struct block_list *block;
	unsigned char *gat;
	short xs, ys;
	short bxs, bys;
	int npc_num;
	int users;
	struct npc_data *npc[MAX_NPC_PER_MAP];
};

struct map_session_data
{
	struct block_list block;
	struct {
		short auth;
		int remove_new_on_map;
	}state;
	int account_id, char_id, login_id1, login_id2;
	char sex;
	struct mmo_charstatus status;
	int weight, max_weight;
	char mapname[16];
	short mapno;
	unsigned int fd;
	short x, y;
	signed to_x, to_y;
	char sitting, dir, head_dir;
	int no_whispers;
	unsigned long client_tick, server_tick;
	int torihiki_fd;
	int pvprank;
	int feed_pet_timer;
	int speed;
	int walktimer;
	char walkpath[64];
	signed walkpath_len, walkpath_pos;
	int npc_id;
	int npc_n;
	int npc_pc;
	int reset_state;
	int local_reg[LOCAL_REG_NUM];
	unsigned long chatID;
	int attacktimer;
	int attacktarget;
	short skill_x;
	short skill_y;
	short skill_target;
	short skill_lvl;
	int skilltimer;
	int skill_timeamount[MAX_SKILL][2];
	int skilltimer_delay;
	int card_skill[MAX_SKILL];
	int endure;
	int auto_blitz;
	int hidden;
	int drain_hp;
	int drain_sp;
	int casting;
	int using_card;
	int item_skill;
	short cignum_crusis;
	int overweight_status;
	int new_on_map;
	short spheres;
	int spheres_timeamount[5];
};

struct flooritem_data
{
	struct block_list block;
	int id;
	short m, x, y;
	short subx, suby;
	int drop_tick;
	struct item item_data;
};

struct mmo_chat
{
	struct mmo_chat* next;
	struct mmo_chat* prev;
	unsigned short len;
	unsigned long ownID;
	unsigned long chatID;
	unsigned short limit;
	unsigned short users;
	unsigned char pub;
	unsigned char title[62];
	unsigned char pass[8];
	int usersfd[20];
	unsigned long usersID[20];
	unsigned char usersName[20][24];
};

struct npc_item_list
{
	int nameid, value;
};

struct npc_data
{
	struct block_list block;
	int id;
	short m, x, y;
	short class, dir;
	char name[24];
	short option;
	short hidden;
	struct {
		int fd;
		int skill_timer[MAX_SKILL][1];
		int skill_num;
		int effect;
	}skilldata;
	union {
		char *script;
		struct npc_item_list *shop_item;
		struct {
			short x_range, y_range;
			short to_x, to_y;
			char name[16];
		}warp;
		struct {
			int target_fd;
			int attacktimer;
			int attackdelay;
			int timer;
			short summon;
			short steal;
			signed to_x;
			signed to_y;
			int speed;
			signed char walkpath_len, walkpath_pos;
			char walkpath[64];
			int hp;
			int atk1;
			int atk2;
			int def1;
			int def2;
			struct {
				int id;
				short loot_count;
				struct item looted_items[MAX_LOOT];
			}lootdata;
		}mons;
	}u;
};

struct mons_skill
{
	int nameid;
	int skill[6];
	int option[12];
}mons_skill[MAX_MONS];

struct mons_data
{
	int class;
	char name[24], real_name[24];
	int monsterid;
	int type;
	int max_hp;
	int npc_num;
	int job_exp;
	int base_exp;
	int lv;
	int range, atk1, atk2, def1, def2, mdef1, mdef2, hit, flee;
	int scale, race, ele, mode, speed, adelay, amotion, dmotion;
	short aggressive;
	short looter;
	short assist;
	short cast_sense;
	short steal;
	short boss;
	struct {
		int nameid, p;
	}dropitem[16];
	struct {
		int nameid, p;
	}mvpdropitem[5];
	struct {
		char discovery_1[80], discovery_2[80], discovery_3[80];
		char attack_1[80], attack_2[80], attack_3[80];
		char hp50_1[80], hp50_2[80];
		char hp25_1[80], hp25_2[80];
		char kill_1[80], kill_2[80];
		char dead_1[80], dead_2[80];
	}mons_say;
	struct {
		int emotion;
	}mons_emotion;
}mons_data[MAX_MONS];
