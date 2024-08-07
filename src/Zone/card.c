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

#include <stdio.h>

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "itemdb.h"
#include "skill.h"
#include "skill_db.h"
#include "card.h"

struct item_db2 item_equip;
struct item_db2 item_card;

int x, y;

/*------------------------------------------------------------------------
 Procedure:     mmo_map_cardskills1 ID:1
 Purpose:       Function set for card skills and effects.
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
void mmo_map_cardskills1(int fd, struct map_session_data *sd)
{
	int i;
	for (i = 0; i < MAX_SKILL; i++){
		if (sd->card_skill[i] == 1) {
			sd->status.skill[i].id = i + 1;
			sd->status.skill[i].lv = sd->status.skill[i].lv;
		}
	}

}

/*------------------------------------------------------------------------
 Procedure:     mmo_map_cardskills2 ID:1
 Purpose:       If character is out, get card skillpoints.
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
void mmo_map_cardskills2(int fd, struct map_session_data *sd)
{
	int i;
	int len;

	for (i = 0; i < MAX_SKILL; i++) {
		if (sd->card_skill[i] == 1) {
			if (sd->status.skill[i].lv > 1) {
				sd->status.skill_point += (sd->status.skill[i].lv -1);
				len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_SKILLPOINT);
      			if(len > 0) {
					WFIFOSET(fd, len);
				}
			}
		}
	}
}

/*------------------------------------------------------------------------
 Procedure:     card_wait ID:1
 Purpose:       Card Elements.
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
int card_wait(struct map_session_data *sd, int wait)
{

	for (x = 0; x < MAX_INVENTORY; x++) {
		if ((sd->status.inventory[x].nameid) && (sd->status.inventory[x].equip)) {
			item_equip = item_database(sd->status.inventory[x].nameid);
			if (item_equip.slot != 0) {
				for(y = 0; y < item_equip.slot; y++) {
					if (sd->status.inventory[x].card[y] != 0) {
						item_card = item_database(sd->status.inventory[x].card[y]);
						if (item_card.ele == 66) {
							wait *= 0.7;
						}
					}
				}
			}
		}
	}
	return wait;

}

/*------------------------------------------------------------------------
 Procedure:     card_skill_effect ID:1
 Purpose:       Character card effects; as stun, sleep etc..
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
void card_skill_effect(struct map_session_data *sd, int fd, int m, int n)
{
	unsigned char buf[256];

	for (x = 0; x < MAX_INVENTORY; x++) {
		if ((sd->status.inventory[x].nameid) && (sd->status.inventory[x].equip)) {
			item_equip = item_database(sd->status.inventory[x].nameid);
			if (item_equip.slot != 0) {
				for (y = 0; y < item_equip.slot; y++) {
					if (sd->status.inventory[x].card[y] != 0) {
						item_card = item_database(sd->status.inventory[x].card[y]);
						if((item_card.eff !=0) && (rand()%100 < 5)) {
							add_timer((unsigned int)(unsigned int)gettick() + 10000 , skill_reset_monster_stats, m, n);
							map_data[m].npc[n]->skilldata.skill_num = 1;
							if (item_card.eff < 5) {
								WBUFW(buf, 0) = 0x119;
								WBUFL(buf, 2) = map_data[m].npc[n]->id;
								WBUFW(buf, 6) = item_card.eff;
								WBUFW(buf, 8) = 0;
								WBUFW(buf, 10) = 0;
								WBUFB(buf, 12) = 0;
								mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
								map_data[m].npc[n]->option = item_card.eff|0|0;
								map_data[m].npc[n]->skilldata.effect = item_card.eff;
							}
							else {
								switch (item_card.eff) {
									case 5: /* Silent */
										item_card.eff = 4;
										map_data[m].npc[n]->skilldata.effect = 5;
										break;

									case 6: /* Poison */
										item_card.eff = 1;
										map_data[m].npc[n]->skilldata.effect=6;
										break;

									case 7: /* Confuse */
										map_data[m].npc[n]->skilldata.effect=7;
										break;

									case 8: /* Curse */
										item_card.eff = 2;
										map_data[m].npc[n]->skilldata.effect=8;
										break;

									case 9: /* Dark */
										item_card.eff = 16;
										map_data[m].npc[n]->skilldata.effect=16;
										break;
								}
								WBUFW(buf, 0) = 0x119;
								WBUFL(buf, 2) = map_data[m].npc[n]->id;
								WBUFW(buf, 6) = 0;
								WBUFW(buf, 8) = item_card.eff;
								WBUFW(buf, 10) = 0;
								WBUFB(buf, 12) = 0;
								mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
								map_data[m].npc[n]->option = 0|item_card.eff|0;
							}
						}
					}
				}
			}
		}
	}
}


/*------------------------------------------------------------------------
 Procedure:     card_add_damage ID:1
 Purpose:       Card element damage.
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
int card_add_damage(struct map_session_data *sd, int fd, int damage, int m, int n)
{
	int tc_hp = 0;
	int tc_sp = 0;

	for (x = 0; x < MAX_INVENTORY; x++) {
		if ((sd->status.inventory[x].nameid) && (sd->status.inventory[x].equip)) {
			item_equip = item_database(sd->status.inventory[x].nameid);
			if (item_equip.slot != 0) {
				for (y = 0; y < item_equip.slot; y++) {
					if (sd->status.inventory[x].card[y] != 0) {
						item_card = item_database(sd->status.inventory[x].card[y]);
						if ((item_card.ele > 0) && (item_card.ele <=10)) {
							if ((item_card.ele != 0) && (mons_data[map_data[m].npc[n]->class].ele == item_card.ele)) {
								damage += (damage * 0.2);
							}
							if ((item_card.ele == 10) && (mons_data[map_data[m].npc[n]->class].ele == 0)) {
								damage += (damage * 0.2);
							}
						}
						/* Type damage */
						if ((item_card.ele >= 100) && (item_card.ele < 120)) {
							switch (item_card.ele - 100) {
								case 0: /* No Type */
									if(mons_data[map_data[m].npc[n]->class].race == 0) damage += (damage * 0.15);
									break;
								case 1: /* Human */
									if(mons_data[map_data[m].npc[n]->class].race == 7) damage += (damage * 0.15);
									break;
								case 2: /* Plant */
									if(mons_data[map_data[m].npc[n]->class].race == 3) damage += (damage * 0.15);
									break;
								case 3: /* Animal */
									if(mons_data[map_data[m].npc[n]->class].race == 2) damage += (damage * 0.15);
									break;
								case 4: /* Bug */
									if(mons_data[map_data[m].npc[n]->class].race == 4) damage += (damage * 0.15);
									break;
								case 5: /* Fish */
									if(mons_data[map_data[m].npc[n]->class].race == 5) damage += (damage * 0.15);
									break;
								case 6: /* Small */
									if(mons_data[map_data[m].npc[n]->class].scale == 0) damage += (damage * 0.15);
									break;
								case 7: /* Medium */
									if(mons_data[map_data[m].npc[n]->class].scale == 1) damage += (damage * 0.15);
									break;
								case 8: /* Large */
									if(mons_data[map_data[m].npc[n]->class].scale == 2) damage += (damage * 0.15);
									break;
								case 9: /* Flying */
									break;
								case 10: /* Devil */
									if(mons_data[map_data[m].npc[n]->class].race == 6) damage += (damage * 0.15);
									break;
								case 11: /* Dragon */
									if(mons_data[map_data[m].npc[n]->class].race == 9) damage += (damage * 0.15);
									break;
								case 12: /* Angel */
									if(mons_data[map_data[m].npc[n]->class].race == 8) damage += (damage * 0.15);
									break;
								case 13: /* Boss */
									break;
								case 14: /* Human,Animal,Plant,Bugs */
									if((mons_data[map_data[m].npc[n]->class].race == 1) || (mons_data[map_data[m].npc[n]->class].race == 2) || (mons_data[map_data[m].npc[n]->class].race == 3) || (mons_data[map_data[m].npc[n]->class].race == 4))
									damage += (damage * 0.07);
									break;
								case 15: /* Small, Medium, Large */
									if((mons_data[map_data[m].npc[n]->class].scale == 0) || (mons_data[map_data[m].npc[n]->class].scale == 1) || (mons_data[map_data[m].npc[n]->class].scale == 2))
									damage *=1.1;
									break;
							}
						}
						if (item_card.ele == 61) {
							tc_hp++;
						}
						if (item_card.ele == 62) {
							tc_sp++;
						}
					}
				}
			}
		}
	}
	/* Damage to HP */
	if ((tc_hp > 0) && (rand()%100 < 3)) {
		sd->status.hp += (damage * tc_hp * 0.15);
		if (sd->status.hp > sd->status.max_hp) {
			sd->status.hp = sd->status.max_hp;
		}
            WFIFOW(fd, 0) = 0x13d;
            WFIFOW(fd, 2) = 5;
            WFIFOL(fd, 4) = (damage * tc_hp * 0.15);
            WFIFOSET(fd, 6);

            WFIFOW(fd, 0) = 0xb0;
            WFIFOW(fd, 2) = 0x05;
            WFIFOL(fd, 4) = sd->status.hp;
		WFIFOSET(fd, 8);
	}
	/* Damage to SP */
	if ((tc_sp > 0) && (rand()%100 < 3)) {
		sd->status.sp += (damage * tc_sp * 0.05);
		if (sd->status.sp > sd->status.max_sp) {
			sd->status.sp = sd->status.max_sp;
		}
            WFIFOW(fd, 0) = 0x13d;
            WFIFOW(fd, 2) = 7;
            WFIFOL(fd, 4) = (damage * tc_sp * 0.05);
            WFIFOSET(fd, 6);

            WFIFOW(fd, 0) = 0xb0;
            WFIFOW(fd, 2) = 0x07;
            WFIFOL(fd, 4) = sd->status.sp;
		WFIFOSET(fd, 8);
	}
	return damage;
}


/*------------------------------------------------------------------------
 Procedure:     card_reduce_sp ID:1
 Purpose:       Cards reduce cost of sp.
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
int card_reduce_sp(struct map_session_data *sd, int sp)
{
	for (x = 0; x < MAX_INVENTORY; x++) {
		if ((sd->status.inventory[x].nameid) && (sd->status.inventory[x].equip)) {
			item_equip = item_database(sd->status.inventory[x].nameid);
			if (item_equip.slot != 0) {
				for (y = 0; y < item_equip.slot; y++) {
					if (sd->status.inventory[x].card[y] != 0) {
						item_card = item_database(sd->status.inventory[x].card[y]);
						if (item_card.ele == 73) {
							sp -= (sp * 0.3);
						}
					}
				}
			}
		}
	}
	return sp;

}


/*------------------------------------------------------------------------
 Procedure:     card_element_immunity ID:1
 Purpose:       Card element damage inmunity.
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
int card_element_immunity(struct map_session_data *sd, int avoid, int m, int n)
{
 	for (x = 0; x < MAX_INVENTORY; x++) {
		if ((sd->status.inventory[x].nameid) && (sd->status.inventory[x].equip)) {
			item_equip = item_database(sd->status.inventory[x].nameid);
			if (item_equip.slot != 0) {
				for (y = 0; y < item_equip.slot; y++) {
					if (sd->status.inventory[x].card[y] != 0) {
						item_card = item_database(sd->status.inventory[x].card[y]);
						if ((item_card.ele > 30) && (item_card.ele <= 40) && (rand()%100 < 30)) {
							if ((item_card.ele -30) == mons_data[map_data[m].npc[n]->class].ele) {
								avoid = 100;
							}
							if (((item_card.ele -30) == 10) && mons_data[map_data[m].npc[n]->class].ele == 0) {
								avoid = 100;
							}
						}
					}
				}
			}
		}
	}
	return avoid;
}


/*------------------------------------------------------------------------
 Procedure:     card_parmor_element ID:1
 Purpose:       Player armor element card.
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
int card_parmor_element(struct map_session_data *sd, int target_ele)
{
	for (x = 0; x < MAX_INVENTORY; x++) {
		if ((sd->status.inventory[x].nameid) && (sd->status.inventory[x].equip)) {
			item_equip = item_database(sd->status.inventory[x].nameid);
			if ((item_equip.slot != 0) && (item_equip.loc == 16)) {
				for(y = 0; y < item_equip.slot; y++) {
					if (sd->status.inventory[x].card[y] != 0) {
						item_card = item_database(sd->status.inventory[x].card[y]);
						if ((item_card.ele > 50) && (item_card.ele <= 60)) {
							target_ele = item_card.ele - 50;
							if ((item_card.ele -50) == 10) {
								target_ele = 0;
							}
						}
					}
				}
			}
		}
	}
	return target_ele;
}

/*------------------------------------------------------------------------
 Procedure:     card_damage_reduce ID:1
 Purpose:       Card element damage reduce.
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
int card_damage_reduce(struct map_session_data *sd, int damage, int m, int n)
{
	for (x = 0; x < MAX_INVENTORY; x++) {
		if ((sd->status.inventory[x].nameid) && (sd->status.inventory[x].equip)) {
			item_equip = item_database(sd->status.inventory[x].nameid);
			if (item_equip.slot != 0) {
				for(y = 0; y < item_equip.slot; y++) {
					if (sd->status.inventory[x].card[y] != 0) {
						item_card = item_database(sd->status.inventory[x].card[y]);
						if ((item_card.ele >= 80) && (item_card.ele < 100)) {
							switch(item_card.ele - 80) {
								case 0: /* No Type */
									if(mons_data[map_data[m].npc[n]->class].race == 0) damage -= (damage * 0.15);
									break;
								case 1: /* Human */
									if(mons_data[map_data[m].npc[n]->class].race == 7) damage -= (damage * 0.15);
									break;
								case 2: /* Plant */
									if(mons_data[map_data[m].npc[n]->class].race == 3) damage -= (damage * 0.15);
									break;
								case 3: /* Animal */
									if(mons_data[map_data[m].npc[n]->class].race == 2) damage -= (damage * 0.15);
									break;
								case 4: /* Bug */
									if(mons_data[map_data[m].npc[n]->class].race == 4) damage -= (damage * 0.15);
									break;
								case 5: /* Fish */
									if(mons_data[map_data[m].npc[n]->class].race == 5) damage -= (damage * 0.15);
									break;
								case 6: /* Small */
									if(mons_data[map_data[m].npc[n]->class].scale == 0) damage -= (damage * 0.15);
									break;
								case 7: /* Medium */
									if(mons_data[map_data[m].npc[n]->class].scale == 1) damage -= (damage * 0.15);
									break;
								case 8: /* Large */
									if(mons_data[map_data[m].npc[n]->class].scale == 2) damage -= (damage * 0.15);
									break;
								case 9: /* Flying */
									break;
								case 10: /* Devil */
									if(mons_data[map_data[m].npc[n]->class].race == 6) damage -= (damage * 0.15);
									break;
								case 11: /* Dragon */
									if(mons_data[map_data[m].npc[n]->class].race == 9) damage -= (damage * 0.15);
									break;
								case 12: /* Angel */
									if(mons_data[map_data[m].npc[n]->class].race == 8) damage -= (damage * 0.15);
									break;
								case 13: /* Boss */
									break;
							}
						}
					}
				}
			}
		}
	}
	return damage;
}

void card_send_equip(int fd, int card_loc)
{
	// Find card
	// Look for items with the following qualifications:
	//1. Has the same loc as the card.
	//2. Has Slots availible.
	//3. Not equiped.
	// Send the invetory numbers of all the items that meet these qualifications.
	struct map_session_data *sd;
	int dataid;
	int i, j, k, c = 0;

	sd = session[fd]->session_data;

	for(i = 0; i < MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].nameid == 0) continue;
		if(sd->status.inventory[i].equip != 0) continue;

		// Get the database id for the current item your checking
		dataid = search_itemdb_index(sd->status.inventory[i].nameid);

		// Continue Checks
		if(itemdb[dataid].type != 4 && itemdb[dataid].type != 5) continue;
		if(itemdb[dataid].slot < 1) continue;

		// Available Slot Check
		k = 0;
		for (j = 0; j < itemdb[dataid].slot; j++) {
			if (sd->status.inventory[i].card[j] == 0) {
				k = 1;
				break;
			}
		}
		if(k == 0) continue;

		// Loc check
		k = 0;
		switch(card_loc) {
		case 2: //Weapon
			if(itemdb[dataid].loc == 2 || itemdb[dataid].loc == 34) k = 1;
			break;
		case 769: //Head
			if(itemdb[dataid].loc == 0 || itemdb[dataid].loc == 1 || itemdb[dataid].loc == 256 || itemdb[dataid].loc == 512 || itemdb[dataid].loc == 513 || itemdb[dataid].loc == 769) k = 1;
			break;
		default:
			if(itemdb[dataid].loc == card_loc) k = 1;
		}
		if (k != 1) continue;
		
		if(c == 0) {
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

void card_finish_equip(int fd, int srcid, int descid)
{
	struct map_session_data *sd;
	int dataid;
	int i;

	sd = session[fd]->session_data;
	dataid = search_itemdb_index(sd->status.inventory[descid - 2].nameid);

	WFIFOW(fd, 0) = 0x17d;
	// Find the correct slot again.
	for (i = 0; i < itemdb[dataid].slot; i++) {
		if(sd->status.inventory[descid - 2].card[i] == 0) {
			// Add card to equipment.
			sd->status.inventory[descid - 2].card[i] = sd->status.inventory[srcid - 2].nameid;
			// Subtract a card.
			sd->status.inventory[srcid - 2].amount--;
			// If there are no cards left set nameid to 0.
			if(sd->status.inventory[srcid - 2].amount < 1) sd->status.inventory[srcid - 2].nameid = 0;
			// Send that card was added successfully.
			WFIFOW(fd, 2) = descid;
			WFIFOW(fd, 4) = srcid;
			WFIFOB(fd, 6) = 0;
			break;
		}
	}
	if(i == itemdb[dataid].slot) {
		WFIFOW(fd, 2) = descid;
		WFIFOW(fd, 4) = srcid;
		WFIFOB(fd, 6) = 1;
	}
	WFIFOSET(fd, 7);
}

/*------------------------------------------------------------------------
 Procedure:     mmo_map_calc_card ID:1
 Purpose:       To calculate card equiped.
 Input:         None
 Output:        None
 Errors:        None
------------------------------------------------------------------------*/
int mmo_map_calc_card(int fd, int item_id, int inv_num, int type)
{
	int i, j;
	struct item_db2 item_db;
	struct item_db2 item_cd;
	struct map_session_data *sd;

	sd = session[fd]->session_data;
	if (type == 1) {
		for (i = 0; i < MAX_INVENTORY; i++) {
			if ((sd->status.inventory[i].nameid) && (sd->status.inventory[i].equip)) {
				/* Equipment exists */
				item_db = item_database(sd->status.inventory[i].nameid);
				/* Check if card is already in slot */
				if (item_db.slot != 0) {
					for(j = 0; j < item_db.slot; j++) {
						if (sd->status.inventory[i].card[j] != 0) {
							/* Card has stuck */
							item_cd = item_database(sd->status.inventory[i].card[j]);
							if (item_cd.skill_id == 0) {
								;
							}
							else {
								sd->status.skill[item_cd.skill_id-1].id = item_cd.skill_id;
								sd->status.skill[item_cd.skill_id-1].lv = 1;
								sd->card_skill[item_cd.skill_id-1] = 1;
							}
						}
					}
				}
			}
		}
		return 0;
	}
	/* Card cancellation */
	else if (type == 0) {
		item_db = item_database(item_id);
		if (item_db.slot != 0) {
			for (j = 0; j < item_db.slot; j++) {
				if (sd->status.inventory[inv_num].card[j] != 0) {
					item_cd = item_database(sd->status.inventory[inv_num].card[j]);
					if (item_cd.skill_id == 0) {
						;
					}
					else{
						sd->status.skill[item_cd.skill_id-1].id = 0;
						sd->card_skill[item_cd.skill_id-1] = 0;
						if (sd->status.skill[item_cd.skill_id-1].lv > 1) {
							mmo_map_update_param(fd, SP_SKILLPOINT, sd->status.skill[item_cd.skill_id-1].lv -1);
						}
					}	
				}
			}
		}
		return 0;
	}
	return 0;
}
