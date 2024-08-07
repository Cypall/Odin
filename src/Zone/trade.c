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
#include "item.h"
#include "trade.h"
#include "save.h"

int mmo_map_trade_request(struct map_session_data *sd)
{
	int i, ep = 0;
	unsigned int fd = sd->fd;
	struct map_session_data *target_sd;

	if (sd->status.skill[0].lv >= 1) {
		for (i = 5; i < FD_SETSIZE; i++) {
			if (session[i] && session[i]->session_data) {
				target_sd = session[i]->session_data;
				if ((unsigned)target_sd->account_id == RFIFOL(fd, 2)) {
					ep = 1;
					if (target_sd->status.skill[0].lv >= 1) {
						if (target_sd->status.trade_partner != 0 || sd->status.trade_partner != 0) {
							WFIFOW(fd, 0) = 0xe7;
							WFIFOB(fd, 2) = 2;
							WFIFOSET(fd, packet_len_table[0xe7]);
						}
						else {
							if (sd->mapno != target_sd->mapno) {
								WFIFOW(fd, 0) = 0xe7;
								WFIFOB(fd, 2) = 0;
								WFIFOSET(fd, packet_len_table[0xe7]);
							}
							else if (!in_range(sd->x, sd->y, target_sd->x, target_sd->y, 5)) {
								WFIFOW(fd, 0) = 0xe7;
								WFIFOB(fd, 2) = 0;
								WFIFOSET(fd, packet_len_table[0xe7]);
							}
							else {
								target_sd->status.trade_partner = sd->account_id;
								sd->status.trade_partner = target_sd->account_id;
								WFIFOW(i, 0) = 0xe5;
								strncpy(WFIFOP(i, 2), sd->status.name, 24);
								WFIFOSET(i, packet_len_table[0xe5]);
							}
						}
					}
					else {
						WFIFOW(fd, 0) = 0xe7;
						WFIFOB(fd, 2) = 4;
						WFIFOSET(fd, packet_len_table[0xe7]);
					}
				}
			}
		}
		if (ep == 0) {
			WFIFOW(fd, 0) = 0xe7;
			WFIFOB(fd, 2) = 1;
			WFIFOSET(fd, packet_len_table[0xe7]);
		}
	}
	else {
		WFIFOW(fd, 0) = 0x110;
		WFIFOW(fd, 2) = 1;
		WFIFOW(fd, 4) = 0;
		WFIFOW(fd, 6) = 0;
		WFIFOW(fd, 8) = 0;
		WFIFOB(fd, 10) =0;
		WFIFOSET(fd, packet_len_table[0x110]);
	}
	return 0;
}

int mmo_map_trade_accept(struct map_session_data *sd)
{
	int i;
	unsigned int fd = sd->fd;
	struct map_session_data *target_sd;

	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && session[i]->session_data) {
			target_sd = session[i]->session_data;
			if (target_sd->account_id == sd->status.trade_partner) {
				if (RFIFOB(fd, 2) == 3) {
					WFIFOW(i, 0) = 0xe7;
					WFIFOB(i, 2) = 3;
					WFIFOSET(i, packet_len_table[0xe7]);

					WFIFOW(fd, 0) = 0xe7;
					WFIFOB(fd, 2) = 3;
					WFIFOSET(fd, packet_len_table[0xe7]);
				}
				if (RFIFOB(fd, 2) == 4) {
					sd->status.deal_locked = 0;
					sd->status.trade_partner = 0;
					target_sd->status.deal_locked = 0;
					target_sd->status.trade_partner = 0;

					WFIFOW(i, 0) = 0xe7;
					WFIFOB(i, 2) = 4;
					WFIFOSET(i, packet_len_table[0xe7]);

					WFIFOW(fd, 0) = 0xe7;
					WFIFOB(fd, 2) = 4;
					WFIFOSET(fd, packet_len_table[0xe7]);
				}
			}
		}
	}
	return 0;
}

int mmo_map_trade_additem(struct map_session_data *sd)
{
	int i, ep = 0;
	int trade_i;
	unsigned int fd = sd->fd;
	struct map_session_data *target_sd;

	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && session[i]->session_data) {
			target_sd = session[i]->session_data;
			if (target_sd->account_id == sd->status.trade_partner && sd->status.deal_locked < 1) {
				if (RFIFOW(fd, 2) < 2 || (RFIFOW(fd, 2) >= MAX_INVENTORY + 2)) {
					if (RFIFOW(fd, 2) == 0 && RFIFOL(fd, 4) > 0 && RFIFOL(fd, 4) <= (unsigned)sd->status.zeny) {
						sd->status.deal_zeny = RFIFOL(fd, 4);
						WFIFOW(i, 0) = 0xe9;
						WFIFOL(i, 2) = RFIFOL(fd, 4);
						WFIFOW(i, 6) = RFIFOW(fd, 2);
						WFIFOB(i, 8) = 0;
						WFIFOB(i, 10) = 0;
						WFIFOB(i, 12) = 0;
						WFIFOW(i, 14) = 0;
						WFIFOW(i, 16) = 0;
						WFIFOW(i, 18) = 0;
						WFIFOW(i, 20) = 0;
						WFIFOSET(i, packet_len_table[0xe9]);
					}
				}
				else if (RFIFOL(fd, 4) <= (unsigned)sd->status.inventory[RFIFOW(fd, 2)-2].amount && RFIFOL(fd, 4) > 0) {
					for (trade_i = 0; trade_i < 10; trade_i++) {
						if (sd->status.deal_inventory[trade_i].amount == 0) {
							ep = 1;
							break;
						}
					}
					if (ep == 1) {
						if (target_sd->weight + itemdb_weight(sd->status.inventory[RFIFOW(fd, 2)-2].nameid) * sd->status.inventory[RFIFOW(fd, 2)-2].amount > target_sd->max_weight) {
							WFIFOW(fd, 0) = 0xea;
							WFIFOW(fd, 2) = RFIFOW(fd, 2);
							WFIFOB(fd, 4) = 1;
							WFIFOSET(fd, packet_len_table[0xea]);
						}
						else {
							sd->status.deal_inventory[trade_i] = sd->status.inventory[RFIFOW(fd, 2)-2];
							sd->status.deal_inventory[trade_i].amount = RFIFOL(fd, 4);
							sd->status.deal_item[trade_i] = RFIFOW(fd, 2);
							WFIFOW(fd, 0) = 0xea;
							WFIFOW(fd, 2) = RFIFOW(fd, 2);
							WFIFOB(fd, 4) = 0;
							WFIFOSET(fd, packet_len_table[0xea]);

							WFIFOW(i, 0) = 0xe9;
							WFIFOL(i, 2) = RFIFOL(fd, 4);
							WFIFOW(i, 6) = sd->status.inventory[RFIFOW(fd, 2)-2].nameid;
							WFIFOB(i, 8) = sd->status.inventory[RFIFOW(fd, 2)-2].identify;
							WFIFOB(i, 10) = sd->status.inventory[RFIFOW(fd, 2)-2].attribute;
							WFIFOB(i, 12) = sd->status.inventory[RFIFOW(fd, 2)-2].refine;
							WFIFOW(i, 14) = sd->status.inventory[RFIFOW(fd, 2)-2].card[0];
							WFIFOW(i, 16) = sd->status.inventory[RFIFOW(fd, 2)-2].card[1];
							WFIFOW(i, 18) = sd->status.inventory[RFIFOW(fd, 2)-2].card[2];
							WFIFOW(i, 20) = sd->status.inventory[RFIFOW(fd, 2)-2].card[3];
							WFIFOSET(i, packet_len_table[0xe9]);
						}
					}
				}
			}
		}
	}
	return 0;
}

int mmo_map_trade_deal(struct map_session_data *sd)
{
	int i;
	unsigned int fd = sd->fd;
	struct map_session_data *target_sd;

	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && session[i]->session_data) {
			target_sd = session[i]->session_data;
			if (target_sd->account_id == sd->status.trade_partner) {
				sd->status.deal_locked = 1;
				WFIFOW(fd, 0) = 0xea;
				WFIFOW(fd, 2) = 0;
				WFIFOB(fd, 4) = 0;
				WFIFOSET(fd, packet_len_table[0xea]);

				WFIFOW(fd, 0) = 0xec;
				WFIFOL(fd, 2) = 0;
				WFIFOSET(fd, packet_len_table[0xec]);

				WFIFOW(i, 0) = 0xec;
				WFIFOL(i, 2) = 1;
				WFIFOSET(i, packet_len_table[0xec]);
			}
		}
	}
	return 0;
}

int mmo_map_trade_cancel(struct map_session_data *sd)
{
	int i, len;
	int trade_i;
	unsigned int fd = sd->fd;
	struct map_session_data *target_sd;

	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && session[i]->session_data) {
			target_sd = session[i]->session_data;
			if (target_sd->account_id == sd->status.trade_partner) {
				for (trade_i = 0; trade_i < 10; trade_i++) {
					if (sd->status.deal_inventory[trade_i].amount > 0) {
						mmo_map_get_item(fd, &sd->status.deal_inventory[trade_i]);
						sd->status.deal_inventory[trade_i].amount = 0;
						sd->status.deal_item[trade_i] = 0;
					}
				}
				if (sd->status.deal_zeny) {
					len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_ZENY);
					if (len > 0)
						WFIFOSET(fd, len);

					sd->status.deal_zeny = 0;
				}
				sd->status.deal_locked = 0;
				sd->status.trade_partner = 0;

				WFIFOW(fd, 0) = 0xee;
				WFIFOSET(fd, packet_len_table[0xee]);

				for (trade_i = 0; trade_i < 10; trade_i++) {
					if (target_sd->status.deal_inventory[trade_i].amount > 0) {
						mmo_map_get_item(i, &target_sd->status.deal_inventory[trade_i]);
						target_sd->status.deal_inventory[trade_i].amount = 0;
						target_sd->status.deal_item[trade_i] = 0;
					}
				}
				if (target_sd->status.deal_zeny) {
					len = mmo_map_set_param(i, WFIFOP(i, 0), SP_ZENY);
					if (len > 0)
						WFIFOSET(i, len);

					target_sd->status.deal_zeny = 0;
				}
				target_sd->status.deal_locked = 0;
				target_sd->status.trade_partner = 0;

				WFIFOW(i, 0) = 0xee;
				WFIFOSET(i, packet_len_table[0xee]);
			}
		}
	}
	return 0;
}

int mmo_map_trade_send(struct map_session_data *sd)
{
	int i, len;
	int trade_i;
	unsigned int fd = sd->fd;
	struct map_session_data *target_sd;

	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && session[i]->session_data) {
			target_sd = session[i]->session_data;
			if (target_sd->account_id == sd->status.trade_partner) {
				if (sd->status.deal_locked >= 1 && target_sd->status.deal_locked >= 1) {
					if (sd->status.deal_locked < 2)
						sd->status.deal_locked = 2;

					if (target_sd->status.deal_locked == 2) {
						for (trade_i = 0; trade_i < 10; trade_i++) {
							if (sd->status.deal_inventory[trade_i].amount != 0) {
								mmo_map_lose_item(fd, 1, sd->status.deal_item[trade_i], sd->status.deal_inventory[trade_i].amount);
								mmo_map_get_item(i, &sd->status.deal_inventory[trade_i]);
								sd->status.deal_inventory[trade_i].amount = 0;
								sd->status.deal_item[trade_i] = 0;
							}
						}
						for (trade_i = 0; trade_i < 10;trade_i++) {
							if (target_sd->status.deal_inventory[trade_i].amount != 0) {
								mmo_map_lose_item(i, 1, target_sd->status.deal_item[trade_i], target_sd->status.deal_inventory[trade_i].amount);
								mmo_map_get_item(fd, &target_sd->status.deal_inventory[trade_i]);
								target_sd->status.deal_inventory[trade_i].amount = 0;
								target_sd->status.deal_item[trade_i] = 0;
							}
						}
						if (sd->status.deal_zeny) {
							sd->status.zeny = sd->status.zeny - sd->status.deal_zeny;
							len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_ZENY);
							if (len > 0)
								WFIFOSET(fd, len);

							target_sd->status.zeny = target_sd->status.zeny + sd->status.deal_zeny;
							len = mmo_map_set_param(i, WFIFOP(i, 0), SP_ZENY);
							if (len > 0)
								WFIFOSET(i, len);

							sd->status.deal_zeny = 0;
						}
						if (target_sd->status.deal_zeny) {
							target_sd->status.zeny = target_sd->status.zeny - target_sd->status.deal_zeny;
							len = mmo_map_set_param(i, WFIFOP(i, 0), SP_ZENY);
							if (len > 0)
								WFIFOSET(i, len);

							sd->status.zeny = sd->status.zeny + target_sd->status.deal_zeny;
							len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_ZENY);
							if (len > 0)
								WFIFOSET(fd, len);

							target_sd->status.deal_zeny = 0;
						}
						sd->status.deal_locked = 0;
						sd->status.trade_partner = 0;
						target_sd->status.deal_locked = 0;
						target_sd->status.trade_partner = 0;

						mmo_char_save(sd);
						mmo_char_save(target_sd);

						WFIFOW(i,0) = 0xf0;
						WFIFOL(i,2) = 0;
						WFIFOSET(i, packet_len_table[0xf0]);

						WFIFOW(fd,0) = 0xf0;
						WFIFOL(fd,2) = 0;
						WFIFOSET(fd, packet_len_table[0xf0]);
					}
				}
			}
		}
	}
	return 0;
}
