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

#ifndef _EQUIP_H
#define _EQUIP_H

void mmo_map_all_nonequip(struct map_session_data *sd, unsigned char *buf);
void mmo_map_all_equip(struct map_session_data *sd, unsigned char *buf);
void mmo_cart_send_items(unsigned int fd);
int mmo_cart_item_remove(unsigned int fd, unsigned char *buf, int index, int amount);
void remove_item(unsigned int fd, int id);
#endif
