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
#include "card.h"
#include "skill_db.h"

void mmo_map_all_nonequip(struct map_session_data *sd,unsigned char *buf)
{
	int i, n, arrow = -1;
	unsigned int fd = sd->fd;
	struct item *n_item;

	WBUFW(buf, 0) = 0xa3;
	for (i = 0, n = 0; i < MAX_INVENTORY; i++) {
		n_item = &sd->status.inventory[i];
		if (n_item->nameid <= 0 || itemdb_isequip(n_item->nameid))
			continue;

		WBUFW(buf, n * 10 + 4) = i + 2;
		WBUFW(buf, n * 10 + 6) = n_item->nameid;
		WBUFB(buf, n * 10 + 8) = itemdb_type(n_item->nameid);
		WBUFB(buf, n * 10 + 9) = n_item->identify;
		WBUFW(buf, n * 10 + 10)= n_item->amount;
		if (n_item->nameid >= 1750 && n_item->nameid <= 1770) {
			WBUFW(buf, n * 10 + 12) = 0x8000;
			if (n_item->equip) {
				arrow = i;
			}
		}
		else
			WBUFW(buf, n * 10 + 12) = 0;

		n++;

	}
	if (n) {
		WBUFW(buf, 2) = 4 + n * 10;
		WFIFOSET(fd, WFIFOW(fd, 2));
	}
	if (arrow >= 0) {
		WFIFOW(fd, 0) = 0x013c;
		WFIFOW(fd, 2) = arrow + 2;
		WFIFOSET(fd, packet_len_table[0x013c]);
	}
}

void mmo_map_all_equip(struct map_session_data *sd,unsigned char *buf)
{
	int i,n;
	unsigned int fd = sd->fd;
	struct item *n_item;

	WBUFW(buf, 0) = 0xa4;
	for (i = 0, n = 0; i < MAX_INVENTORY; i++) {
		n_item = &sd->status.inventory[i];
		if (n_item->nameid <= 0 || !itemdb_isequip(n_item->nameid))
			continue;

		WBUFW(buf, n * 20 + 4) = i + 2;
		WBUFW(buf, n * 20 + 6) = n_item->nameid;
		WBUFB(buf, n * 20 + 8) = (itemdb_type(n_item->nameid) == 7) ? 4 : itemdb_type(n_item->nameid);
		WBUFB(buf, n * 20 + 9) = n_item->identify;
		WBUFW(buf, n * 20 + 10) = itemdb_equip_point(n_item->nameid);
		WBUFW(buf, n * 20 + 12) = n_item->equip;
		WBUFB(buf, n * 20 + 14) = n_item->attribute;
		WBUFB(buf, n * 20 + 15) = n_item->refine;
		WBUFW(buf, n * 20 + 16) = n_item->card[0];
		WBUFW(buf, n * 20 + 18) = n_item->card[1];
		WBUFW(buf, n * 20 + 20) = n_item->card[2];
		WBUFW(buf, n * 20 + 22) = n_item->card[3];
		n++;
	}
	if (n) {
		WBUFW(buf, 2) = 4 + n * 20;
		WFIFOSET(fd, WFIFOW(fd, 2));
	}
}

void mmo_cart_send_items(unsigned int fd)
{
	int i, c = 0, e = 0;
	int equip[MAX_CART], item_id, item_type;
	struct item *n_item;
	struct map_session_data *sd;

	if (session[fd] && (sd = session[fd]->session_data)) {
		sd->status.cartweight = 0;
		WFIFOW(fd, 0) = 0x123;
		for (i = 0; i < MAX_CART; i++) {
			n_item = &sd->status.cart[i];
			if (n_item->nameid != 0 && n_item->amount > 0) {
				item_id = search_itemdb_index(n_item->nameid);
				sd->status.cartweight += (itemdb[item_id].weight * n_item->amount);
				item_type = itemdb[item_id].type;
				if (item_type == 4 || item_type == 5) {
					equip[e] = i;
					e++;
				}
				else {
					WFIFOW(fd, 4 + (c * 10)) = i + 1;
					WFIFOW(fd, 6 + (c * 10)) = n_item->nameid;
					WFIFOB(fd, 8 + (c * 10)) = item_type;
					WFIFOB(fd, 9 + (c * 10)) = n_item->identify;
					WFIFOW(fd, 10 + (c * 10)) = n_item->amount;
					WFIFOW(fd, 12 + (c * 10)) = 0;
					c++;
				}
			}
		}
		if (c != 0) {
			WFIFOW(fd, 2) = 4 + (c * 10);
			WFIFOSET(fd, 4 + (c * 10));
		}
		sd->status.cartcount = c + e;
		WFIFOW(fd, 0) = 0x121;
		WFIFOW(fd, 2) = sd->status.cartcount;
		WFIFOW(fd, 4) = MAX_CART;
		WFIFOL(fd, 6) = sd->status.cartweight;
		WFIFOL(fd, 10) = 80000;
		WFIFOSET(fd, packet_len_table[0x121]);

		WFIFOW(fd, 0) = 0x122;
		for (i = 0; i < e; i++) {
			n_item = &sd->status.cart[equip[i]];
			WFIFOW(fd, 4 + (i * 20)) = equip[i] + 1;
			WFIFOW(fd, 6 + (i * 20)) = n_item->nameid;
			WFIFOB(fd, 8 + (i * 20)) = itemdb[search_itemdb_index(n_item->nameid)].type;
			WFIFOB(fd, 9 + (i * 20)) = n_item->identify;
			WFIFOW(fd, 10 + (i * 20)) = itemdb_equip_point(n_item->nameid);
			WFIFOW(fd, 12 + (i * 20)) = n_item->equip;
			WFIFOB(fd, 14 + (i * 20)) = n_item->attribute;
			WFIFOB(fd, 15 + (i * 20)) = n_item->refine;
			WFIFOW(fd, 16 + (i * 20)) = n_item->card[0];
			WFIFOW(fd, 18 + (i * 20)) = n_item->card[1];
			WFIFOW(fd, 20 + (i * 20)) = n_item->card[2];
			WFIFOW(fd, 22 + (i * 20)) = n_item->card[3];
		}
		WFIFOW(fd, 2) = 4 + (i * 20);
		WFIFOSET(fd, 4 + (i * 20));
	}
}

int mmo_cart_item_remove(unsigned int fd, unsigned char* buf, int index, int amount)
{
	struct map_session_data *sd;
	struct mmo_charstatus *p;
	struct item *n_item;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return -1;

	p = &sd->status;
	if (index < 1 || index >= MAX_CART + 1)
		return -1;

	n_item = &p->cart[index - 1];
	if (n_item->nameid == 0 || amount > n_item->amount)
		return -1;

	n_item->amount -= amount;
	p->cartweight -= (itemdb[search_itemdb_index(n_item->nameid)].weight * amount);
	if (n_item->amount < 1) {
		memset(n_item, 0, sizeof(*n_item));
		p->cartcount--;
	}
	WBUFW(buf, 0) = 0x125;
	WBUFW(buf, 2) = index;
	WBUFL(buf, 4) = amount;

	WBUFW(buf, 8) = 0x121;
	WBUFW(buf, 10) = p->cartcount;
	WBUFW(buf, 12) = MAX_CART;
	WBUFL(buf, 14) = p->cartweight;
	WBUFL(buf, 18) = 80000;
	return 22;
}

void remove_item(unsigned int fd, int id)
{
	int i, len;
	int item_num = 0, item_view = 0;
	unsigned char buf[256];
	struct item_db2 weapon;
	struct item_db2 item_db;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	item_db = item_database(sd->status.inventory[id - 2].nameid);
	item_num = itemdb_stype(sd->status.inventory[id - 2].nameid);
	if (sd->status.inventory[id - 2].equip) {
		if (sd->status.inventory[id - 2].equip == 0x8000) {
			WFIFOW(fd, 0) = 0x013c;
			WFIFOW(fd, 2) = 0;
			WFIFOSET(fd, packet_len_table[0x13c]);
		}
		else {
			WFIFOW(fd, 0) = 0xac;
			WFIFOW(fd, 2) = id;
			WFIFOW(fd, 4) = sd->status.inventory[id - 2].equip;
			WFIFOW(fd, 6) = 1;
			WFIFOSET(fd, packet_len_table[0xac]);
		}
		sd->status.inventory[id - 2].equip = 0;
	}
	else {
		WFIFOW(fd, 0) = 0xac;
		WFIFOW(fd, 2) = id;
		WFIFOW(fd, 6) = 0;
		WFIFOSET(fd, packet_len_table[0xac]);
	}
	if (item_num != LOOK_SHOES) {
		len = mmo_map_set_look(buf, sd->account_id, item_num, 0);
		if (len > 0)
			mmo_map_sendarea(fd, buf, len, 0);

		set_equip(sd, item_num, 0);
	}
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip & 0x22)) {
			weapon = item_database(sd->status.inventory[i].nameid);
			if (weapon.type == 4) {
				item_num = itemdb_stype(sd->status.inventory[i].nameid);
				item_view = itemdb_view_point(sd->status.inventory[i].nameid);
				set_equip(sd, item_num, item_view);
				break;
			}
		}
	}
	WFIFOW(fd, 0) = 0x13a;
	WFIFOW(fd, 2) = 1;
	WFIFOSET(fd, packet_len_table[0x13a]);

	mmo_map_calc_status(fd, sd->status.inventory[id - 2].nameid);
	mmo_map_calc_card(fd, sd->status.inventory[id - 2].nameid, RFIFOW(fd, 2) - 2, 0);
}
