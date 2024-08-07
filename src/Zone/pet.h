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

#define MAX_PET_DB 100

struct pet_db
{
	int class;
	char name[24];
	int item_id;
	int accesory_id;
	int egg_id;
	int food_id;
	int fullness;
	int hungry_delay;
	int r_full;
	int r_hungry;
	int friendly;
	int die;
	int capture;
	int speed;
}pet_db[MAX_PET_DB];

void pet_store_init_npc_id(int *id);
int pet_init(struct map_session_data *sd);
int mmo_map_pet_catch(int fd, struct map_session_data *sd, int index);
int mmp_map_pet_stat_select(int fd, struct map_session_data *sd, int type);
int mmo_map_pet_name(int fd, struct map_session_data *sd);
int mmo_map_pet_act(int fd, struct map_session_data *sd, int index);
int mmo_map_pet_emotion(int fd, struct map_session_data *sd, int emotion);
int mmo_pet_equip(struct map_session_data *sd, int item_id);
int search_pet_item_id(int item_id);
int search_mons_id(int egg_id);
int search_pet_id(int class);
int search_pet_class_item(int item);
void mmo_pet_db_init(void);
