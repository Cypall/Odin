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
 Modified Date: Semtember 3, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#define WARP_CLASS 45

int mmo_map_npc_say(unsigned char* buf, unsigned long id, char *string);
int mmo_map_npc_next(unsigned char* buf, unsigned long id);
int mmo_map_npc_close(unsigned char* buf, unsigned long id);
int mmo_map_npc_select(unsigned char* buf, unsigned long id, char *string);
int mmo_map_npc_amount_request(unsigned char* buf, unsigned long id);
int mmo_map_npc_buysell(unsigned char* buf, unsigned long id);
int mmo_map_npc_buy(struct map_session_data *sd, unsigned char* buf, struct npc_item_list *items);
int mmo_map_npc_sell(struct map_session_data *sd, unsigned char *buf);
int npc_click(struct map_session_data *sd, int npc_id);
int npc_menu_select(struct map_session_data *sd, int sel);
int npc_next_click(struct map_session_data *sd);
int npc_amount_input(struct map_session_data *sd, int val);
int npc_buysell_selected(struct map_session_data *sd, int npc_id, int sell);
int do_rollback(struct map_session_data *sd, struct map_session_data *back);
int delete_rollback_point(struct map_session_data *back);
int npc_buy_selected(struct map_session_data *sd, void *list, int num);
int npc_sell_selected(struct map_session_data *sd, void *list, int num);

int set_monster_random_point(short m, int n);
int set_monster_no_point(short m, int n);
int mons_walk(int tid, unsigned int tick, int m, int n);
int spawn_monster(int class, int x, int y, int m, int fd);
int spawn_to_pos(struct map_session_data *sd, int class, char *name, short x, short y, short m, int fd);
int spawn_delay(int tid, unsigned int tick, int m, int n);
int remove_monsters(int m, short x, short y, int fd);
int boss_monster(int m, int n);
int init_boss_miniboss(int class);
int *return_npc_current_id(void);
int npc_add_npc_id(void);
int read_npc_fromdir(char *dirname);
int search_monster_id(char *name);
void read_npcdata(void);
