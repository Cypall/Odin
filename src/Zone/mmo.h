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

#ifndef _MMO_H_
#define _MMO_H_
#include "core.h"
#include "socket.h"
#include "timer.h"
#include "common.h"

#define MAX_MAP 768
#define MAX_NPC_PER_MAP 512
#define BLOCK_SIZE 8
#define AREA_SIZE 3
#define LOCAL_REG_NUM 16
#define DEFAULT_MONSTER_WALK_SPEED 200
#define MAX_CLASSES 24
#define MAX_MONS 4000
#define MAX_OBJECTS 40000
#define MAX_SKILL_REFINE 100
#define MAX_MAP_WARPSKILL 155000
#define MAX_LOOT 10
#define LIFETIME_FLOORITEM 60

#define ST_SLEEP 0x0004
#define ST_POISON 0x0040
#define ST_STUN 0x0400
#define ST_STONE 0x4000
#define ST_SILENCE 0x0008
#define ST_BLIND 0x0080
#define ST_ZOMBIE 0x0800
#define ST_FROZEN 0x8000

#define RBN(a, b) (rand() % (b - a + 1) + a);
#define add_block_npc(m, n) { map_data[m].npc[n]->block.type = BL_NPC; \
                             add_block(&map_data[m].npc[n]->block, m, map_data[m].npc[n]->x, map_data[m].npc[n]->y); }

enum { LAN_CON, WWW_CON, NO_INFO };
enum { BL_NUL, BL_PC, BL_NPC, BL_ITEM, BL_SKILL, BL_PET };
enum { MONS, WARP, SHOP, SCRIPT };
enum { FS_NULL, FS_SANCT, FS_MAGNUS, FS_QUAG, FS_TRAP, FS_WARP, FS_ICEWALL, FS_SWALL, FS_FWALL, FS_VENOM, FS_PNEUMA };
enum { NOVICE, SWORDMAN, MAGE, ARCHER, ACOLYTE, MERCHANT, THIEF, KNIGHT, PRIEST, WIZARD,
	 BLACKSMITH, HUNTER, ASSASSIN, KNIGHT_PECOPECO, CRUSADER, MONK, SAGE, ROGUE,
       ALCHEMIST, BARD, DANCER, CRUSADER_PECOPECO, WEDDING, SUPER_NOVICE = 23 };
enum { SP_SPEED,SP_BASEEXP,SP_JOBEXP,SP_KARMA,SP_MANNER,SP_HP,SP_MAXHP,SP_SP,
       SP_MAXSP,SP_STATUSPOINT,SP_0a,SP_BASELEVEL,SP_SKILLPOINT,SP_STR,SP_AGI,SP_VIT,
       SP_INT,SP_DEX,SP_LUK,SP_13,SP_ZENY,SP_15,SP_NEXTBASEEXP,SP_NEXTJOBEXP,
       SP_WEIGHT,SP_MAXWEIGHT,SP_1a,SP_1b,SP_1c,SP_1d,SP_1e,SP_1f,
       SP_USTR,SP_UAGI,SP_UVIT,SP_UINT,SP_UDEX,SP_ULUK,SP_26,SP_27,
       SP_28,SP_ATK1,SP_ATK2,SP_MATK1,SP_MATK2,SP_DEF1,SP_DEF2,SP_MDEF1,
       SP_MDEF2,SP_HIT,SP_FLEE1,SP_FLEE2,SP_CRITICAL,SP_ASPD,SP_36,SP_JOBLEVEL,SP_GETJOB,
       SP_SCRIPT_FLAG,SP_CHECKSTORAGE,SP_CHECKCART,SP_CHECKFALCON, SP_CHECKPECOPECO,SP_ACCOUNTID,SP_SEX,SP_HASCART,
       SP_GETWLVL,SP_RANDOM };
enum { LOOK_BASE, LOOK_HAIR, LOOK_WEAPON, LOOK_HEAD_BOTTOM, LOOK_HEAD_TOP, LOOK_HEAD_MID, LOOK_HAIR_COLOR,
	 LOOK_CLOTHES_COLOR, LOOK_SHIELD, LOOK_SHOES };
enum { PVP, MEMO, BRANCH, TELEPORT, WATER, CANLOSE };
enum { R_CANT, R_NOSP, R_NOHP, R_NOMEMO, R_INDELAY, R_NOZENY, R_NOWEAPON, R_NOREDG, R_NOBLUEG, R_UNKN, R_NONE };
enum { SK_EMERGENCYCARE = 142, SK_ACTDEAD, SK_MOVINGRECOVER, SK_FATALBLOW, SK_AUTOBERSERK, SK_MAKINGARROW,
	 SK_CHARGEARROW, SK_THROWSAND, SK_BACKSLIDING, SK_PICKSTONE, SK_THROWSTONE, SK_CARTREVOLOTION, SK_CHANGECART,
	 SK_LORDEXCLAMATION, SK_HOLYLIGHT, SK_ENERGYCOAT };

#ifdef _MMS_
struct map_servers
{
	long ip;
	short port;
	char map[16];
};
#endif

struct block_list
{
	struct block_list *next, *prev;
	int type, subtype;
};

struct mmo_map_data
{
	struct block_list *block;
	unsigned char *gat;
	short xs, ys;
	short bxs, bys;
	long npc_num;
	long users;
	struct npc_data *npc[MAX_NPC_PER_MAP];
};

struct map_flag
{
	char map_name[16];
	short pvp;
	short memo;
	short teleport;
	short branch;
	short water;
	short can_lose;
};

struct gat_1cell
{
	float high[4];
	int type;
};

struct mmo_tmp_path
{
	short x, y, dist, before, cost;
	int dir, flag;
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

struct map_session_data
{
	struct block_list block;
	struct mmo_charstatus status;
	struct {
		short auth;
		int remove_new_on_map;
	}state;
	struct {
		char name[24];
	}*wisblock;
	int limit, max;
	long account_id, char_id, login_id1, login_id2;
	char sex;
	unsigned int fd;
	unsigned int torihiki_fd;
	unsigned long client_tick, server_tick;

	int weight, max_weight;
	short overweight_status;

	char mapname[16];
	short mapno;
	short x, y;
	short to_x, to_y;

	int speed;
	int walktimer;
	char walkpath[64];
	int walkpath_len, walkpath_pos;

	char sitting, dir, head_dir;
	unsigned long chatID;
	short no_whispers;
	short using_card;
	int guild_invited;
	int feed_pet_timer;
	int new_on_map;

	long npc_id;
	long npc_n;
	long npc_pc;
	int local_reg[LOCAL_REG_NUM];

	short pvprank;
	int attacktimer;
	int attacktarget;

	short skill_x;
	short skill_y;
	short skill_target;
	short skill_lvl;
	short skill_num;
	int skilltimer;
	int skill_timeamount[MAX_SKILL][2];
	int skilltimer_delay;
	int item_skill;
	int card_skill[MAX_SKILL];

	int spheres_timeamount[5];
	short spheres;
	short casting;
	short cignum_crusis;
	short hidden;
	short act_dead;
	short auto_blitz;
	short no_cost_sp;
	int drain_hp;
	int drain_sp;
	int sp_count;
	int sp_count_lvl;
	int heal_hp_delay;
	int heal_sp_delay;
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
	unsigned int usersfd[20];
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
	long id;
	short m, x, y, dir;
	int class;
	char name[24];
	struct {
		unsigned int fd;
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
			unsigned int target_fd;
			int attacktimer;
			int attackdelay;
			int walktimer;
			short hidden;
			short summon;
			short steal;
			int speed;
			short to_x;
			short to_y;
			int walkpath_len, walkpath_pos;
			char walkpath[64];
			int hp;
			int atk1;
			int atk2;
			int def1;
			int def2;
			int option_x;
			int option_y;
			int option_z;
			int lexaeterna;
			int equip;
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
		short emotion;
	}mons_emotion;
}mons_data[MAX_MONS];
#endif
