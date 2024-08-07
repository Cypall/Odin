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

#ifndef _CARD_H_
#define _CARD_H_

void card_send_equip(unsigned int fd, int card_loc);
void card_finish_equip(unsigned int fd, int srcid, int descid);
int mmo_map_calc_card(unsigned int fd, int item_id, int inv_num, int type);
int mmo_card_equiped(struct map_session_data *sd, int nameid);
#endif
