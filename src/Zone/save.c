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
#include <string.h>

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "save.h"
#include "party.h"

extern int char_fd;

void mmo_char_save(struct map_session_data *sd)
{
	WFIFOW(char_fd, 0) = 0x2b01;
	WFIFOW(char_fd, 2) = sizeof(sd->status) + 12;
	WFIFOL(char_fd, 4) = sd->account_id;
	WFIFOL(char_fd, 8) = sd->char_id;
	memcpy(WFIFOP(char_fd, 12), &sd->status, sizeof(sd->status));
	WFIFOSET(char_fd, WFIFOW(char_fd, 2));
	mmo_online_check();
}

void mmo_map_delete_pet(int pet_id)
{
	WFIFOW(char_fd, 0) = 0x3b46;
	WFIFOL(char_fd, 2) = pet_id;
	WFIFOSET(char_fd, 6);
}

void mmo_pet_save(struct map_session_data *sd)
{
	WFIFOW(char_fd, 0) = 0x3a44;
	WFIFOL(char_fd, 2) = sd->status.pet.pet_id;
	WFIFOW(char_fd, 6) = sd->status.pet.pet_class;
	memcpy(WFIFOP(char_fd, 8), sd->status.pet.pet_name, 24);
	WFIFOL(char_fd, 32) = sd->account_id;
	WFIFOL(char_fd, 36) = sd->char_id;
	WFIFOB(char_fd, 40) = sd->status.pet.pet_level;
	WFIFOW(char_fd, 41) = sd->status.pet.mons_id;
	WFIFOW(char_fd, 43) = sd->status.pet.pet_accessory;
	WFIFOW(char_fd, 45) = sd->status.pet.pet_friendly;
	WFIFOW(char_fd, 47) = sd->status.pet.pet_hungry;
	WFIFOW(char_fd, 49) = sd->status.pet.pet_name_flag;
	WFIFOB(char_fd, 51) = sd->status.pet.activity;
	WFIFOSET(char_fd, 52);
}

void mmo_party_save(int party_num)
{
	int i, j;
	int index;

	index = party_exists(party_num);
	if (index >= 0) {
		WFIFOW(char_fd, 0) = 0x2b08;
		WFIFOL(char_fd, 4) = party_dat[party_num].party_id;
		WFIFOW(char_fd, 9) = party_dat[party_num].exp;
		WFIFOW(char_fd, 11) = party_dat[party_num].item;
		WFIFOW(char_fd, 13) = party_dat[party_num].leader_level;
		memcpy(WFIFOP(char_fd, 15), party_dat[party_num].party_name, 24);
		j = 0;
		for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
			if (party_dat[party_num].member[i].char_id > 0) {
				WFIFOW(char_fd, 39 + j * 68) = party_dat[party_num].member[i].mapno;
				WFIFOB(char_fd, 39 + j * 68 + 2) = party_dat[party_num].member[i].leader;
				WFIFOL(char_fd, 39 + j * 68 + 3) = party_dat[party_num].member[i].account_id;
				WFIFOL(char_fd, 39 + j * 68 + 7) = party_dat[party_num].member[i].char_id;
				WFIFOW(char_fd, 39 + j * 68 + 11) = party_dat[party_num].member[i].x;
				WFIFOW(char_fd, 39 + j * 68 + 13) = party_dat[party_num].member[i].y;
				WFIFOL(char_fd, 39 + j * 68 + 15) = party_dat[party_num].member[i].hp;
				WFIFOL(char_fd, 39 + j * 68 + 19) = party_dat[party_num].member[i].max_hp;
				WFIFOB(char_fd, 39 + j * 68 + 23) = party_dat[party_num].member[i].offline; // 0 = player online, 1 = player offline
				WFIFOL(char_fd, 39 + j * 68 + 24) = party_dat[party_num].member[i].fd;
				memcpy(WFIFOP(char_fd, 39 + j * 68 + 28), party_dat[party_num].member[i].map_name, 16);
				memcpy(WFIFOP(char_fd, 39 + j * 68 + 44), party_dat[party_num].member[i].nick, 24);
			j++;
			}
		}
		WFIFOB(char_fd, 8) = j;
		WFIFOW(char_fd, 2) = 39 + j * 68;
		WFIFOSET(char_fd, 39 + j * 68);
	}
}

void mmo_online_check()
{
	unsigned int x;
	struct map_session_data *sd = NULL;

	FILE *fp = fopen("save/online.txt", "w");
	if (fp) {
		for (x = 5; x < FD_SETSIZE; x++) {
			if (session[x] != NULL && session[x]->session_data != NULL) {
				sd = session[x]->session_data;
				if (sd->status.name) {
					fprintf(fp, "%s\t%s,%d,%d\n", sd->status.name, sd->mapname, sd->x, sd->y);
				}
			}
		}
	}
	fclose(fp);
}
