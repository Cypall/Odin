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
 Project:       Project Odin Zone Server
 Creation Date: Dicember 6, 2003
 Modified Date: October 31, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

struct delay_item_drop
{
	int m, x, y;
	int nameid, amount;
};

int search_item(int object);
int mmo_map_lose_item(int fd, int slient, int index, int amount);
int mmo_map_get_item(int fd, struct item* item);
void mmo_map_make_flooritem(struct item *item_data, int amount, short m, short x, short y);
int mmo_map_dropitem(struct map_session_data *sd, int index, int amount);
void mmo_map_take_item(struct map_session_data *sd, int item_id);
int mmo_map_delay_item_drop(int tid, unsigned int tick, int id, int data);
void mmo_map_item_drop(short m, int n);
int mmo_map_item_steal(int fd, short m, int n);
void use_item(struct map_session_data *sd, int indexid);
