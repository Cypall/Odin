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

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "itemdb.h"
#include "skill.h"
#include "skill_db.h"
#include "card.h"

void card_send_equip(unsigned int fd, int card_loc)
{
	int i, j, k, c = 0, dataid;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data)) 
		return;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid <= 0)
			continue;

		if (sd->status.inventory[i].identify == 0)
			continue;

		if (sd->status.inventory[i].equip != 0)
			continue;

		dataid = search_itemdb_index(sd->status.inventory[i].nameid);
		if (itemdb[dataid].type != 4 && itemdb[dataid].type != 5)
			continue;

		if (itemdb[dataid].slot < 1)
			continue;

		for (j = 0, k = 0; j < itemdb[dataid].slot; j++) {
			if (sd->status.inventory[i].card[j] == 0) {
				k = 1;
				break;
			}
		}
		if (k == 0)
			continue;

		k = 0;
		switch(card_loc) {
		case 2: // WEAPON
			if (itemdb[dataid].loc == 2 || itemdb[dataid].loc == 34)
				k = 1;

			break;
		case 769: // HEAD
			if (itemdb[dataid].loc == 0 || itemdb[dataid].loc == 1
			|| itemdb[dataid].loc == 256 || itemdb[dataid].loc == 512
			|| itemdb[dataid].loc == 513 || itemdb[dataid].loc == 769)
				k = 1;

			break;
		default: // ALL OTHERS
			if (itemdb[dataid].loc == card_loc)
				k = 1;

		}
		if (k != 1)
			continue;
		
		if (c == 0) {
			sd->using_card = 1;
			WFIFOW(fd, 0) = 0x17b;
		}
		WFIFOW(fd, 4 + c * 2) = i + 2;
		c++;
	}
	if (c > 0) {
		WFIFOW(fd, 2) = 4 + c * 2;
		WFIFOSET(fd, 4 + c * 2);
	}
}

void card_finish_equip(unsigned int fd, int srcid, int descid)
{
	int i, dataid;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data)) 
		return;

	dataid = search_itemdb_index(sd->status.inventory[descid - 2].nameid);
	WFIFOW(fd, 0) = 0x17d;
	for (i = 0; i < itemdb[dataid].slot; i++) {
		if (sd->status.inventory[descid - 2].card[i] == 0) {
			sd->status.inventory[descid - 2].card[i] = sd->status.inventory[srcid - 2].nameid;
			sd->status.inventory[srcid - 2].amount--;
			if(sd->status.inventory[srcid - 2].amount < 1) sd->status.inventory[srcid - 2].nameid = 0;
			WFIFOW(fd, 2) = descid;
			WFIFOW(fd, 4) = srcid;
			WFIFOB(fd, 6) = 0;
			break;
		}
	}
	if (i == itemdb[dataid].slot) {
		WFIFOW(fd, 2) = descid;
		WFIFOW(fd, 4) = srcid;
		WFIFOB(fd, 6) = 1;
	}
	WFIFOSET(fd, packet_len_table[0x17d]);
}

int mmo_map_calc_card(unsigned int fd, int item_id, int inv_num, int type)
{
	int i, j;
	struct item_db2 item_db, item_cd;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data)) 
		return 1;

	if (type) {
		for (i = 0; i < MAX_INVENTORY; i++) {
			if (sd->status.inventory[i].nameid && sd->status.inventory[i].equip) {
				item_db = item_database(sd->status.inventory[i].nameid);
				if (item_db.slot != 0) {
					for (j = 0; j < item_db.slot; j++) {
						if (sd->status.inventory[i].card[j] != 0) {
							item_cd = item_database(sd->status.inventory[i].card[j]);
							if (item_cd.skill_id != 0) {
								switch (item_id) {
								case 4073: // Pirate Skeleton Discount lv 5
									sd->status.skill[item_cd.skill_id-1].id = item_cd.skill_id;
									sd->status.skill[item_cd.skill_id-1].lv = 5;
									sd->status.skill[item_cd.skill_id-1].flag = 1;
									break;
								default: // All others
									sd->status.skill[item_cd.skill_id-1].id = item_cd.skill_id;
									sd->status.skill[item_cd.skill_id-1].lv = 1;
									sd->status.skill[item_cd.skill_id-1].flag = 1;
									break;
								}
								mmo_map_send_skills(fd, 0);
							}
							else
								mmo_map_calc_status(fd, sd->status.inventory[i].card[j]);

						}
					}
				}
			}
		}
	}
	else {
		item_db = item_database(item_id);
		if (item_db.slot != 0) {
			for (j = 0; j < item_db.slot; j++) {
				if (sd->status.inventory[inv_num].card[j] != 0) {
					item_cd = item_database(sd->status.inventory[inv_num].card[j]);
					if (item_cd.skill_id != 0) {
						sd->status.skill[item_cd.skill_id-1].id = 0;
						sd->status.skill[item_cd.skill_id-1].lv = 0;
						sd->status.skill[item_cd.skill_id-1].flag = 0;
						mmo_map_send_skills(fd, 1);
					}
					else
						mmo_map_calc_status(fd, sd->status.inventory[inv_num].card[j]);

				}
			}
		}
	}
	return 0;
}

int mmo_card_equiped(struct map_session_data *sd, int nameid)
{
	int i, j, num = 0;
	struct item_db2 item_equip;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if ((sd->status.inventory[i].nameid) && (sd->status.inventory[i].equip)) {
			item_equip = item_database(sd->status.inventory[i].nameid);
			if (item_equip.slot == 0)
				continue;

			for (j = 0; j < item_equip.slot; j++)
				if (sd->status.inventory[i].card[j] == nameid)
					num++;


		}
	}
	return num;
}
