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

#include <stdio.h>

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "itemdb.h"
#include "item.h"
#include "save.h"

void mmo_open_storage(int fd)
{
	int i, e, n, item_type;
	struct item *n_item;
	int equip[MAX_STORAGE];
	struct map_session_data *sd;

	if (session[fd] && (sd = session[fd]->session_data)) {
		sd->status.storage_status = 1;
		WFIFOW(fd, 0) = 0xa5;
		e = 0;
		n = 0;
		for (i = 0; i < MAX_STORAGE; i++) {
			n_item = &sd->status.storage[i];
			if (n_item->amount != 0) {
				item_type = itemdb[search_itemdb_index(n_item->nameid)].type;
				if (item_type == 4 || item_type == 5) {
					equip[e] = i;
					e++;
				}
				else {
					WFIFOW(fd, 4 + n * 10) = i + 1;
					WFIFOW(fd, 6 + n * 10) = n_item->nameid;
					WFIFOB(fd, 8 + n * 10) = item_type;
					WFIFOB(fd, 9 + n * 10) = n_item->identify;
					WFIFOW(fd, 10 + n * 10) = n_item->amount;
					if (item_type == 10) {
						WFIFOW(fd, 12 + n * 10) = 32768;
					}
					else {
						WFIFOW(fd, 12 + n * 10) = 0;
					}
					n++;
				}
			}
		}
		WFIFOW(fd, 2) = 4 + n * 10;
		WFIFOSET(fd, 4 + n * 10);

		WFIFOW(fd, 0) = 0xa6;
		for (i = 0; i < e; i++) {
			n_item = &sd->status.storage[equip[i]];
			WFIFOW(fd, 4 + i * 20) = equip[i] + 1;
			WFIFOW(fd, 6 + i * 20) = n_item->nameid;
			WFIFOB(fd, 8 + i * 20) = itemdb_type(n_item->nameid);
			WFIFOB(fd, 9 + i * 20) = n_item->identify;
			WFIFOW(fd, 10 + i * 20) = itemdb_equip_point(n_item->nameid, sd);
			WFIFOW(fd, 12 + i * 20) = n_item->equip;
			WFIFOB(fd, 14 + i * 20) = n_item->attribute;
			WFIFOB(fd, 15 + i * 20) = n_item->refine;
			WFIFOW(fd, 16 + i * 20) = n_item->card[0];
			WFIFOW(fd, 18 + i * 20) = n_item->card[1];
			WFIFOW(fd, 20 + i * 20) = n_item->card[2];
			WFIFOW(fd, 22 + i * 20) = n_item->card[3];
		}
		WFIFOW(fd, 2) = 4 + i * 20;
		WFIFOSET(fd, 4 + i * 20);

		sd->status.storage_amount = n + e;
		WFIFOW(fd, 0) = 0xf2;
		WFIFOW(fd, 2) = sd->status.storage_amount;
		WFIFOW(fd, 4) = MAX_STORAGE;
		WFIFOSET(fd, packet_len_table[0xf2]);
	}
	return;
}

void mmo_add_storage(struct map_session_data *sd, int index, int amount)
{
	int i;
	int item_type, len, fd = sd->fd;
	int newi = -1;
	int iindex = index - 2;

	item_type = itemdb[search_itemdb_index(sd->status.inventory[iindex].nameid)].type;
	if (item_type == 4 || item_type == 5) {
		if (amount > 1) {
			amount = 1;
		}
		for (i = 0; i < MAX_STORAGE; i++) {
			if (sd->status.storage[i].amount == 0) {
				break;
			}
		}
		if (i == MAX_STORAGE) {
			return;
		}
	}
	else {
		for (i = 0; i < MAX_STORAGE; i++) {
			if (sd->status.storage[i].amount == 0 && newi == -1) {
				newi = i;
			}
			if (sd->status.storage[i].nameid == sd->status.inventory[iindex].nameid &&
			    sd->status.storage[i].identify == sd->status.inventory[iindex].identify) {
				break;
			}
		}
		if (i == MAX_STORAGE) {
			if (newi != -1) {
				i = newi;
			}
			else {
				return;
			}
		}
	}
	if (sd->status.storage[i].amount == 0) {
		sd->status.storage[i] = sd->status.inventory[iindex];
		sd->status.storage[i].amount = amount;
		sd->status.storage_amount++;
	}
	else {
		if (sd->status.storage[i].amount + amount < 0) {
			return;
		}
		else {
			sd->status.storage[i].amount += amount;
		}
	}
	len = mmo_map_lose_item(fd, 0, index, amount);
	if (len > 0) {
		if (item_type == 4 || item_type == 5) {
			WFIFOW(fd, 0) = 0xa6;
			WFIFOW(fd, 2) = 24;
			WFIFOW(fd, 4) = i + 1;
			WFIFOW(fd, 6) = sd->status.storage[i].nameid;
			WFIFOB(fd, 8) = itemdb_type(sd->status.storage[i].nameid);
			WFIFOB(fd, 9) = sd->status.storage[i].identify;
			WFIFOW(fd, 10) = itemdb_equip_point(sd->status.storage[i].nameid, sd);
			WFIFOW(fd, 12) = sd->status.storage[i].equip;
			WFIFOB(fd, 14) = sd->status.storage[i].attribute;
			WFIFOB(fd, 15) = sd->status.storage[i].refine;
			WFIFOW(fd, 16) = sd->status.storage[i].card[0];
			WFIFOW(fd, 18) = sd->status.storage[i].card[1];
			WFIFOW(fd, 20) = sd->status.storage[i].card[2];
			WFIFOW(fd, 22) = sd->status.storage[i].card[3];
			WFIFOSET(fd, 24);
		}
		else {
			WFIFOW(fd, 0) = 0xf4;
			WFIFOW(fd, 2) = i + 1;
			WFIFOL(fd, 4) = amount;
			WFIFOW(fd, 8) = sd->status.storage[i].nameid;
			WFIFOB(fd, 10) = sd->status.storage[i].identify;
			WFIFOB(fd, 11) = sd->status.storage[i].attribute;
			WFIFOB(fd, 12) = sd->status.storage[i].refine;
			WFIFOW(fd, 13) = sd->status.storage[i].card[0];
			WFIFOW(fd, 15) = sd->status.storage[i].card[1];
			WFIFOW(fd, 17) = sd->status.storage[i].card[2];
			WFIFOW(fd, 19) = sd->status.storage[i].card[3];
			WFIFOSET(fd, packet_len_table[0xf4]);
		}
		WFIFOW(fd, 0) = 0xf2;
		WFIFOW(fd, 2) = sd->status.storage_amount;
		WFIFOW(fd, 4) = MAX_STORAGE;
		WFIFOSET(fd, packet_len_table[0xf2]);
	}
	mmo_char_save(sd);
}

void mmo_remove_storage(struct map_session_data *sd, int index, int amount)
{
	int i, fd = sd->fd;
	int temp, len;

	if (amount > 0) {
		temp = sd->status.storage[index-1].amount;
		sd->status.storage[index-1].amount = amount;
		len = mmo_map_get_item(fd, &sd->status.storage[index-1]);
		if (len > 0) {
			sd->status.storage[index-1].amount = temp - amount;
			if (sd->status.storage[index-1].amount < 1) {
				memset(&sd->status.storage[index-1], 0, sizeof(sd->status.storage[index-1]));
				sd->status.storage_amount--;
				if (sd->status.storage_amount < 0) {
					sd->status.storage_amount = 0;
					for (i = 0; i < MAX_STORAGE; i++) {
						if (sd->status.storage[i].amount > 0) {
							sd->status.storage_amount++;
						}
					}
				}
			}
			WFIFOW(fd, 0) = 0xf6;
			WFIFOW(fd, 2) = index;
			WFIFOL(fd, 4) = amount;
			WFIFOSET(fd, packet_len_table[0xf6]);

			WFIFOW(fd, 0) = 0xf2;
			WFIFOW(fd, 2) = sd->status.storage_amount;
			WFIFOW(fd, 4) = MAX_STORAGE;
			WFIFOSET(fd, packet_len_table[0xf2]);
		}
	}
	mmo_char_save(sd);
}
