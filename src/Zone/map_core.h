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

#define MAX_TMP_PATH 512
#define HP_TIME_T 1000
#define HP_TIME_S 500
#define HP_TIME_R 3000
#define SP_TIME_T 1000
#define SP_TIME_S 500
#define SP_TIME_R 5000

struct mmo_map_data map_data[MAX_MAP_PER_SERVER];
struct mmo_party party_dat[MAX_PARTYS];
struct mmo_guild guild_dat[MAX_GUILDS];
struct pet_info pet_dat[MAX_MONS];
struct block_list *object[MAX_OBJECTS];
struct map_flag flag_data[MAX_MAP_PER_SERVER];
const int packet_len_table[0x210];

/* Attack Base */
void mmo_mons_send_death(unsigned int fd, short m, int n);
void mmo_player_send_death(struct map_session_data *sd);
void walk_char(int tid, unsigned int tick, int id, int data);
int map_setcell(short m, short x, short y, short t);
void mmo_player_attack_no(struct map_session_data *sd);
void mmo_player_stop_walk(struct map_session_data *sd);
void mmo_mons_walk_stop(short m, int n);
void mmo_player_stop_skills_timer(struct map_session_data *sd);
int mmo_map_setoption(struct map_session_data *sd, unsigned char *buf, short flag);

void set_pos(unsigned char *p, short x, short y);
void set_2pos(unsigned char *p, short x0, short y0, short x1, short y1);
int in_range(short x1, short y1, short x2, short y2, int range);
/* Attack Exp */
void mmo_map_set0079(struct map_session_data *sd, unsigned char *buf);
int mmo_map_level_mons(struct map_session_data *sd, short m, int n);
void wait_exit(int tid, unsigned int tick, int fd, int data);

/* Attack Monster */
void check_new_target_monster(short m, int n, unsigned int fd);
int calc_dir2(short srcx, short srcy, short x, short y);
void mmo_mons_attack_no(short m, int n);
void mmo_mons_attack_continue(int tid, unsigned int tick, int n, int m);
int mmo_map_mvp_do(unsigned int fd,short m,int n);
int mmo_map_search_monster(short m, long id);
/* Attack Player */
void attack_player(unsigned int fd,unsigned int target_fd,int damage);
void attack_monster(unsigned int fd,short m,int n,long target_id,int damage);
void mmo_map_attack_continue(int tid,unsigned int tick,int id,int data);
int mmo_map_once_attack(unsigned int fd, long target_id, unsigned long tick, int alt_damage);
int mmo_map_pvp_attack(unsigned int fd,long target_id,unsigned long tick);
/* General Server Functions */
//int calc_next_walk_step(int fd);
void mmo_send_selfdata(struct map_session_data *sd);
int mmo_map_sendchat(unsigned int srcfd, char *buf, int len, int wos);
int calc_weight_stat(int tid, unsigned int tick, int fd, int data);
int mmo_map_reset_vars(int tid, unsigned int tick, int fd, int data);
void add_block(struct block_list *bl, short m, short x, short y);
void del_block(struct block_list *bl);
int search_free_object_id(void);
void delete_object(int id);
void set_equip(struct map_session_data *sd, int item_num, int item_view);
void set_lvup_table(void);
int mmo_map_set_pet(struct npc_data *nd, unsigned char *buf);
void wait_close(int tid, unsigned int tick, int id, int data);
int find_char(char* cname);
void mmo_map_movechar(struct map_session_data *sd, unsigned char *buf);
int mmo_map_checkpvpmap(struct map_session_data *sd);
int mmo_map_flagload(char mapname[24], int type);
int mmo_map_set_npc(struct npc_data *nd, unsigned char *buf);
int mmo_map_set_spawn(struct npc_data *nd, unsigned char *buf);
int mmo_map_send0095(unsigned int fd, struct map_session_data *sd, long id);
int calc_dir(short srcx, short srcy, short x, short y);
int mmo_map_send_bonus_skills(struct map_session_data *sd);
short calc_distance(short x0, short y0, short x1, short y1);
int mmo_map_set_dropitem(unsigned char *buf, struct flooritem_data *fitem);
int mmo_map_set_frameinitem(unsigned int fd, struct flooritem_data *fitem);

void remove_new_on_map(int timer_id, unsigned int tick, int fd, int nothing);
int mmo_map_sendblock(short m, short bx, short by, char *dat, int len, unsigned int srcfd, int wos);
int mmo_map_sendarea(int srcfd, char *dat, int len, int wos);
int  mmo_map_sendarea_mxy(short m, short x, short y, char *dat, int len);
int mmo_map_sendall(unsigned int srcfd, char *dat, int len, int wos);
int mmo_map_getblockchar(unsigned int fd, short m, short bx, short by);
int mmo_map_clrblockchar(unsigned int fd, short m, short bx, short by);
void mmo_map_changemap(struct map_session_data *sd, char *mapname, short x, short y, char type);
int mmo_map_set_look(unsigned char *buf, long id, int type, int val);
int mmo_map_jobchange(unsigned int fd, int class);

/* Various Character Settings */
int mmo_map_set011f(unsigned char *buf, int id, int x, int y, int owner_id, int skill_type);
void mmo_map_set0078(struct map_session_data *sd, unsigned char *buf);
//int mmo_map_set0079(int fd, unsigned char *buf);
//int mmo_map_send0095(int fd, unsigned long id);
int calc_dir3(short srcx, short srcy, short x, short y);
int mmo_map_set007b(struct map_session_data *sd, unsigned char *buf);
void mmo_map_set00b1(unsigned int fd, int type, int val);
int mmo_map_set00bd(unsigned char *buf, struct map_session_data *sd);
int mmo_map_set00d7(unsigned int fd, unsigned char *buf);
int mmo_map_set010a(unsigned int fd, int item_id);
int mmo_map_set010b(int exp, struct map_session_data *sd);
/* Parameter Settings */
int mmo_map_set_param(unsigned int fd, unsigned char *buf, int type);
int mmo_map_update_param(unsigned int fd, int type, int val);
/* Character Calculations */
void init_job_stats(void);
int mmo_map_set_monster(struct npc_data *nd, unsigned char *buf);
void calc_job_status(struct mmo_charstatus *p, int class, int level);
int calc_need_status_point(struct map_session_data *sd, int type);
void mmo_map_calc_overweight(struct map_session_data *sd);
int mmo_map_calc_sigma(int k, double val);
int mmo_map_calc_status(unsigned int fd, int item_num);
void mmo_map_skill_lv_up(struct map_session_data *sd, int skill_id);
int mmo_map_job_lv_up(unsigned int fd, int val);
int map_getcell(short m, short x, short y);
void mmo_map_status_up(struct map_session_data *sd, int type);
int mmo_map_level_up(unsigned int fd, int val);

/* Field Calculations */
//short calc_cost(short x1, short y1, short x2, short y2);
//int add_path(struct mmo_tmp_path *path, int *wp, short x, short y, short dist, char dir, short before, short x1, short y1);
//int can_move(struct mmo_map_data *m, short x0, short y0, short x1, short y1);
short search_path(struct map_session_data *sd, short mapno, short x0, short y0, short x1, short y1, char easy_only);
int calc_next_walk_step(struct map_session_data *sd);
