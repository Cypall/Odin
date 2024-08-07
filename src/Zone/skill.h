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

int last_floor_item_id;
void skill_stop_wait (int fd);
int skill_do_delayed_target (int tid, unsigned int tick, int fd, int skill_num);
int skill_do_delayed_place (int tid, unsigned int tick, int fd, int skill_num);
int skill_attack_target (int fd,int target_id, int skill_num,int skill_lvl, unsigned long tick);
int skill_attack_place (int fd, int skill_num,int skill_lvl,unsigned long tick, int skill_x, int skill_y);
int skill_can_use(int skill_num,int *skill_lvl,int fd);

int extra_sp_cost (int skill_num,int skill_lvl); // for display correction may usefull!!!


int skill_reset_stats (int tid, unsigned int tick, int fd, int skill_num);
int skill_reset_monster_stats(int tid, unsigned int tick,int m, int n);
void skill_icon_effect(struct map_session_data *sd,int value,int flag);
int skill_do_damage(int fd, int damage, int target_id, unsigned long tick, int healed);

int skill_can_forge(struct map_session_data *sd, int nameid, int flag);
void skill_do_forge(int fd,struct map_session_data *sd,int nameid, int flag, int ele);
int delete_forge_material(int fd,struct map_session_data *sd,int name_id);
void check_cast_sensor(int fd,int m,int n,int target,int range,int skill_x,int skill_y);

int remove_floorskill(int tid, unsigned int tick, int fd, int index);
int next_floorskill_index(void);
int search_floorskill_index(short m, short x, short y);
int search_floorskill_index2(int account_id, int skill, int start_index);
void make_floorskill(int fd, short x, short y, int skill_lvl, int skill_type, int subtype, int skill_num, int index);
void init_floorskill_data(void);
int skill_restart_cast_delay(int tid, unsigned int tick, int id, int data);

void skill_callspirits(int fd, int spheres_num);
void skill_combodelay(int fd, int delay);
int skill_spiritcadence(int tid, unsigned int tick, int fd, int data);
