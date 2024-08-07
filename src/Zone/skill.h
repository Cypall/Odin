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

#define MAX_FLOORSKILL 5120
#define MAX_BLOCKS_AREA_FLOORSKILL 49

struct skillwaittime
{
	int wait;
	int duration;
}skillwaitdata;

struct skilldelay
{
	int delay;
} skilldelaydata;

struct floorskill_data
{
	struct block_list block;
	int id;
	int owner_id;
	short x, y;
	short skill_type;
	short index;
};

struct floorskill_db
{
	struct {
		int bl_id;
		short x, y;
	}skill[MAX_BLOCKS_AREA_FLOORSKILL];
	unsigned used : 1;
	short trap_used;
	signed short m : 15;
	int owner_id;
	int owner_fd;
	int n;
	short skill_lvl;
	short skill_num;
	int timer;
	short type;
	short count;
	char mapname[16];
	short to_x, to_y;
}floorskill[MAX_FLOORSKILL];

void skill_drain_hp_monster(int tid, unsigned int tick, int m, int n);
void skill_drain_hp(int tid, unsigned int tick, int fd, int data);
void skill_do_delayed_target(int tid, unsigned int tick, int fd, int skill_num);
int last_floor_item_id;
void skill_stop_wait(struct map_session_data *sd);
int skill_attack_target(unsigned int fd, long target_id, int skill_num, int skill_lvl, unsigned long tick);
int skill_attack_place(unsigned int fd, int skill_num,int skill_lvl, unsigned long tick, int skill_x, int skill_y);
int skill_can_use(int skill_num, int *skill_lvl, unsigned int fd);
int extra_sp_cost (int skill_num,int skill_lvl); // for display correction may usefull!!!
int skill_cloaking_check(struct map_session_data *sd);
int skill_calc_damage(unsigned int fd, int skill_num, int skill_lvl, long target_id);

void skill_reset_stats (int tid, unsigned int tick, int fd, int skill_num);
void skill_reset_monster_stats(int tid, unsigned int tick,int m, int n);
void skill_icon_effect(struct map_session_data *sd,int value,int flag);
void skill_do_damage(unsigned int fd, int damage, long target_id, unsigned long tick, short healed);

int skill_can_forge(struct map_session_data *sd, int nameid, int flag);
void skill_do_forge(unsigned int fd,struct map_session_data *sd,int nameid, int flag, int ele);
int delete_forge_material(unsigned int fd,struct map_session_data *sd,int name_id);

void remove_floorskill(int tid, unsigned int tick, int fd, int index);
int next_floorskill_index(void);
int search_floorskill_index(short m, short x, short y);
int search_floorskill_index2(long account_id, int skill, int start_index);
void make_floorskill(unsigned int fd, short x, short y, int skill_lvl, int skill_type, int subtype, int skill_num, int index);
void init_floorskill_data(void);
void skill_restart_cast_delay(int tid, unsigned int tick, int id, int data);

void skill_callspirits(unsigned int fd, int spheres_num);
void skill_combodelay(unsigned int fd, int delay);
void skill_spiritcadence(int tid, unsigned int tick, int fd, int data);
