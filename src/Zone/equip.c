#include <stdio.h>

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "itemdb.h"
#include "card.h"

int mmo_map_all_nonequip(int fd,unsigned char *buf)
{
  int i,c;
  struct map_session_data *sd;
  struct item *n_item;

  sd=session[fd]->session_data;
  WBUFW(buf,0)=0xa3;
  for(i=c=0;i<MAX_INVENTORY;i++){
    n_item=&sd->status.inventory[i];
    if(n_item->nameid==0 || itemdb_isequip(n_item->nameid))
      continue;
    WBUFW(buf,4+c*10)=i+2;
    WBUFW(buf,4+c*10+2)=n_item->nameid;
    WBUFB(buf,4+c*10+4)=itemdb_type(n_item->nameid);
    WBUFB(buf,4+c*10+5)=n_item->identify;
    WBUFW(buf,4+c*10+6)=n_item->amount;
    WBUFB(buf,4+c*10+8)=0;
    WBUFB(buf,4+c*10+9)=0;	// while equipping arrow 0x80?
    c++;
  }
  if(c==0)
    return 0;
  WBUFW(buf,2)=4+c*10;
  return 4+c*10;
}

int mmo_map_all_equip(int fd,unsigned char *buf)
{
  int i,c;
  struct map_session_data *sd;
  struct item *n_item;

  sd=session[fd]->session_data;
  WBUFW(buf,0)=0xa4;
  for(i=c=0;i<MAX_INVENTORY;i++){
    n_item=&sd->status.inventory[i];
    if(n_item->nameid==0 || !itemdb_isequip(n_item->nameid))
      continue;
    WBUFW(buf,4+c*20)=i+2;
    WBUFW(buf,4+c*20+2)=n_item->nameid;
	//beginning equipment position somewhere?
    WBUFB(buf,4+c*20+4)=itemdb_type(n_item->nameid);
    WBUFB(buf,4+c*20+5)=n_item->identify;
    WBUFW(buf,4+c*20+6)=itemdb_equip_point(n_item->nameid,sd);
    WBUFW(buf,4+c*20+8)=n_item->equip;
    WBUFB(buf,4+c*20+10)=n_item->attribute;
    WBUFB(buf,4+c*20+11)=n_item->refine;
    WBUFW(buf,4+c*20+12)=n_item->card[0];
    WBUFW(buf,4+c*20+14)=n_item->card[1];
    WBUFW(buf,4+c*20+16)=n_item->card[2];
    WBUFW(buf,4+c*20+18)=n_item->card[3];
    c++;
  }
  if(c==0)
    return 0;
  WBUFW(buf,2)=4+c*20;
  return 4+c*20;
}

void mmo_cart_send_items(int fd)
{
	int i; // for loop
	short c; // count the number of items
	short e; // count the number of equipement items
	int equip[MAX_CART]; // to save slot of equipement items
	struct item *n_item;
	int item_id; // to have the id of the item
	int item_type; // to have the type of the item
	struct map_session_data *sd;

	// if player exists
	if (session[fd] && (sd = session[fd]->session_data)) {
		// initialise cartweight
		sd->status.cartweight = 0;

		// send information about consumptive and collecter items.
		c = 0; // initialize number of items
		e = 0; // initialize number of equipement items
		WFIFOW(fd, 0) = 0x123;
		for (i = 0; i < MAX_CART; i++) {
			n_item = &sd->status.cart[i];
			// if we have at least 1 item
			if (n_item->nameid != 0 && n_item->amount > 0) {
				// obtain id of the item
				item_id = search_itemdb_index(n_item->nameid);
				// increase sd->status.cartweight
				sd->status.cartweight += (itemdb[item_id].weight * n_item->amount);
				// Obtain Type of item
				item_type = itemdb[item_id].type;
				// If item is an equipment
				if (item_type == 4 || item_type == 5) {
					equip[e] = i; // Save index of equipement
					e++; // Increase the number of equipements
				} else {
					WFIFOW(fd, 4 + (c * 10)) = i + 1;
					WFIFOW(fd, 6 + (c * 10)) = n_item->nameid;
					WFIFOB(fd, 8 + (c * 10)) = item_type;
					WFIFOB(fd, 9 + (c * 10)) = n_item->identify;
					WFIFOW(fd, 10 + (c * 10)) = n_item->amount;
					if (item_type == 10) {
						WFIFOW(fd, 12 + (c * 10)) = 32768; // unknown value (?)
					} else {
						WFIFOW(fd, 12 + (c * 10)) = 0; // unknown value (?)
					}
					c++;
				}
			}
		}
		if (c != 0) {
			WFIFOW(fd, 2) = 4 + (c * 10);
			WFIFOSET(fd, 4 + (c * 10));
		}

		// Send weight & count info
		sd->status.cartcount = c + e;
		WFIFOW(fd, 0) = 0x121; // if send at last, char is sometimes drawn with weapon out ==> send before equipement items
		WFIFOW(fd, 2) = sd->status.cartcount;
		WFIFOW(fd, 4) = MAX_CART;
		WFIFOL(fd, 6) = sd->status.cartweight;
		WFIFOL(fd, 10) = 80000;
		WFIFOSET(fd, packet_len_table[0x121]);

		// send information about equipement items
		// even if we have no equipement, otherwise we have problem with sprite of player that is sometimes drawn with weapon out)
		WFIFOW(fd, 0) = 0x122;
		for (i = 0; i < e; i++) {
			n_item = &sd->status.cart[equip[i]]; // Use index of equipement
			WFIFOW(fd, 4 + (i * 20)) = equip[i] + 1;
			WFIFOW(fd, 6 + (i * 20)) = n_item->nameid;
			WFIFOB(fd, 8 + (i * 20)) = itemdb[search_itemdb_index(n_item->nameid)].type;
			WFIFOB(fd, 9 + (i * 20)) = n_item->identify;
			WFIFOW(fd, 10 + (i * 20)) = itemdb_equip_point(n_item->nameid, sd);
			WFIFOW(fd, 12 + (i * 20)) = n_item->equip;
			WFIFOB(fd, 14 + (i * 20)) = n_item->attribute;
			WFIFOB(fd, 15 + (i * 20)) = n_item->refine;
			WFIFOW(fd, 16 + (i * 20)) = n_item->card[0];
			WFIFOW(fd, 18 + (i * 20)) = n_item->card[1];
			WFIFOW(fd, 20 + (i * 20)) = n_item->card[2];
			WFIFOW(fd, 22 + (i * 20)) = n_item->card[3];
		}
		WFIFOW(fd, 2) = 4 + (i * 20); // here: e is like i
		WFIFOSET(fd, 4 + (i * 20)); // here: e is like i
	}
	return;
}

void remove_item(int fd, int id)
{
	int i;
	int len;
	int item_view;
	int item_num = 0;
	struct item_db2 weapon;
	struct item_db2 item_db;
	unsigned char buf[64];
	struct map_session_data *sd;

	sd = session[fd]->session_data;
	item_db = item_database(sd->status.inventory[id - 2].nameid);
	item_num = itemdb_stype(sd->status.inventory[id - 2].nameid);
	if (sd->status.inventory[id - 2].equip) {
		WFIFOW(fd, 0) = 0xac;
		WFIFOW(fd, 2) = id;
		WFIFOW(fd, 4) = sd->status.inventory[id-2].equip;
		WFIFOW(fd, 6) = 1;
		WFIFOSET(fd, packet_len_table[0xac]);
		sd->status.inventory[id-2].equip = 0;
	}
	else {
		WFIFOW(fd, 0) = 0xac;
		WFIFOW(fd, 2) = id;
		WFIFOW(fd, 6) = 0;
		WFIFOSET(fd, packet_len_table[0xac]);
	}
	if (item_num != LOOK_SHOES) {
		len = mmo_map_set_look(buf, sd->account_id, item_num, 0);
		if (len > 0) {
			mmo_map_sendall(fd, buf, len, 0);
		}
		set_equip(sd, item_num, 0);
	}
	for (i = 0; i < MAX_INVENTORY; i++) {
		if ((sd->status.inventory[i].nameid) && (sd->status.inventory[i].equip&0x22)) {
			weapon = item_database(sd->status.inventory[i].nameid);
			/* type = weapon */
			if (weapon.type == 4) {
				item_num = itemdb_stype(sd->status.inventory[i].nameid);
				item_view = itemdb_view_point(sd->status.inventory[i].nameid);
				set_equip(sd, item_num, item_view);
				break;
			}
		}
	}
	mmo_map_calc_status(fd, sd->status.inventory[id-2].nameid);
	mmo_map_calc_card(fd, sd->status.inventory[id-2].nameid, RFIFOW(fd, 2)-2, 0);
	WFIFOW(fd, 0) = 0x13a;
	WFIFOW(fd, 2) = 1;
	WFIFOSET(fd, packet_len_table[0x13a]);
}

void equip_check(int fd)
{
	int i;
	struct map_session_data *sd = session[fd]->session_data;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip)) {
			mmo_map_calc_status(fd, sd->status.inventory[i].nameid);
		}
	}
}

int mmo_cart_item_remove(int fd, unsigned char* buf, int index, int amount) {
	struct map_session_data *sd;
	struct mmo_charstatus *p;
	struct item *n_item;

#if MAP_LOG
	sprintf(map_log, "function: mmo_cart_item_remove");
	write_map_log(map_log);
#endif

	sd = session[fd]->session_data;
	p = &sd->status;

	// Check to see if the index is valid.
	if (index < 1 || index >= MAX_CART + 1) {
		return -1;
	}

	n_item = &p->cart[index - 1];
	// Check to see if the nameid and the quantity are valid.
	if (n_item->nameid == 0 || amount > n_item->amount) {
		return -1;
	}

	// -Serverside-
	// Remove item.
	n_item->amount -= amount;
	p->cartweight -= (itemdb[search_itemdb_index(n_item->nameid)].weight * amount);
	if (n_item->amount < 1) { // first when we dont need it anymore set it to 0
		memset(n_item, 0, sizeof(*n_item));
		p->cartcount--;
	}

	// -Clientside-
	// Send client code to remove item from cart
	WBUFW(buf, 0) = 0x125; // R 0125 <index>.w <amount>.l: delete item in cart.
	WBUFW(buf, 2) = index;
	WBUFL(buf, 4) = amount;
	// Send client code to update cart weight/count stats
	WBUFW(buf, 8) = 0x121; // R 0121 <num>.w <num limit>.w <weight>.l <weight limit>l: kind of cart, weight and max weight.
	WBUFW(buf, 10) = p->cartcount;
	WBUFW(buf, 12) = MAX_CART;
	WBUFL(buf, 14) = p->cartweight;
	WBUFL(buf, 18) = 80000;

	return 22;
}
