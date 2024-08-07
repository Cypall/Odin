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
  Modified Date: November 22, 2004
  Description:   Ragnarok Online Server Emulator
  ------------------------------------------------------------------------*/

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "itemdb.h"
#include "item.h"
#include "save.h"
#include "guild.h"

void init_member_data(int index, short mbr)
{
	if (index >= 0 && index < MAX_PARTYS) {
		if (mbr >= 0 && mbr < MAX_GUILD_MEMBERS) {
			guild_dat[index].member[mbr].account_id = 0;
			guild_dat[index].member[mbr].char_id = 0;
			guild_dat[index].member[mbr].hair_style = 0;
			guild_dat[index].member[mbr].hair_color = 0;
			guild_dat[index].member[mbr].sex = 0;
			guild_dat[index].member[mbr].job = 0;
			guild_dat[index].member[mbr].lv = 0;
			guild_dat[index].member[mbr].present_exp = 0;
			guild_dat[index].member[mbr].online = 0;
			guild_dat[index].member[mbr].position = 0;
			memset(guild_dat[index].member[mbr].name, 0, 24);
		}
	}
}

void init_guild_data(int index)
{
	int mbr;

	if (index >= 0 && index < MAX_PARTYS) {
		guild_dat[index].guild_id = 0;
		guild_dat[index].guild_lv = 0;
		guild_dat[index].connum = 0;
		guild_dat[index].max_num = 0;
		guild_dat[index].average_lv = 0;
		memset(guild_dat[index].guild_master, 0, 24);
		guild_dat[index].exp = 0;
		guild_dat[index].next_exp = 0;
		guild_dat[index].emblem_id = 0;
		for (mbr = 0; mbr < MAX_GUILD_MEMBERS; mbr++)
			init_member_data(index, mbr);
	}
}

void mmo_guild_do_init(void)
{
	int index;

	printf("Loading Guilds Data...        ");
	for (index = 0; index < MAX_GUILDS; index++)
		init_guild_data(index);

	printf("Done\n");
}

int init_guild_slot(void)
{
	int i;

	for (i = 1; i < MAX_GUILDS; i++) {
		if (guild_dat[i].guild_id == 0)
			break;
	}
	if (i == MAX_GUILDS)
		i = -1;

	return i;
}

int create_guild(struct map_session_data *sd, char *guildname)
{
	unsigned int fd = sd->fd;
	int index, i, emperium;

	if (sd->status.guild_id != 0) {
		emperium = search_item2(sd, 714);
		if (emperium) {
			for (i = 0; i < MAX_GUILDS; i++) {
				if (strcmp(guild_dat[i].guild_name, guildname) == 0)
					break;
			}
			if (i != MAX_GUILDS) {
				WFIFOW(fd, 0) = 0x167;
				WFIFOB(fd, 2) = 2;
				WFIFOSET(fd, packet_len_table[0x167]);
			}
			else {
				index = init_guild_slot();
				if (index == -1) {
					WFIFOW(fd, 0) = 0x167;
					WFIFOB(fd, 2) = 1;
					WFIFOSET(fd, packet_len_table[0x167]);
				}
				else {
					// Server Side
					sd->status.guild_id = index;
					guild_dat[index].guild_id = sd->status.guild_id;
					memcpy(guild_dat[index].guild_name, guildname, 24);
					guild_dat[index].guild_lv = 1;
					guild_dat[index].max_num = MAX_GUILD_MEMBERS;
					guild_dat[index].average_lv = sd->status.base_level / 1;
					memcpy(guild_dat[index].guild_master, sd->status.name, 24);
					guild_dat[index].exp = 0;
					guild_dat[index].next_exp = 0;
					guild_dat[index].emblem_id = 0;

					guild_dat[index].member[0].account_id = sd->account_id;
					guild_dat[index].member[0].char_id = sd->char_id;
					guild_dat[index].member[0].hair_style = sd->status.hair;
					guild_dat[index].member[0].hair_color = sd->status.hair_color;
					guild_dat[index].member[0].sex = sd->sex;
					guild_dat[index].member[0].job = sd->status.class;
					guild_dat[index].member[0].lv = sd->status.base_level;
					guild_dat[index].member[0].present_exp = sd->status.base_exp;
					guild_dat[index].member[0].online = 1;
					guild_dat[index].member[0].position = 0;
					memcpy(guild_dat[index].member[0].name, sd->status.name, 24);

					// Client Side
					WFIFOW(fd, 0) = 0x14e;
					WFIFOL(fd, 2) = 0xd7;
					WFIFOSET(fd, packet_len_table[0x14e]);

					WFIFOW(fd, 0) = 0x1b6;
					WFIFOL(fd, 2) = guild_dat[index].guild_id;
					WFIFOL(fd, 6) = guild_dat[index].guild_lv;
					WFIFOL(fd, 10) = connected_members(index);
					WFIFOL(fd, 14) = guild_dat[index].max_num;
					WFIFOL(fd, 18) = guild_dat[index].average_lv;
					WFIFOL(fd, 22) = guild_dat[index].exp;
					WFIFOL(fd, 26) = guild_dat[index].next_exp;
					WFIFOL(fd, 30) = 0;
					WFIFOL(fd, 34) = 0;
					WFIFOL(fd, 38) = 0;
					WFIFOL(fd, 42) = 0;
					memcpy(WFIFOP(fd, 46), guild_dat[index].guild_name, 24);
					memcpy(WFIFOP(fd, 70), guild_dat[index].guild_master, 24);
					memcpy(WFIFOP(fd, 94), "None Taken", 20);
					WFIFOSET(fd, packet_len_table[WFIFOW(fd, 0)]);

					mmo_map_lose_item(fd, 0, emperium, 1);
					WFIFOW(fd, 0) = 0x167;
					WFIFOB(fd, 2) = 0;
					WFIFOSET(fd, packet_len_table[0x167]);
					mmo_char_save(sd);
				}
			}
		}
		else {
			WFIFOW(fd, 0) = 0x167;
			WFIFOB(fd, 2) = 3;
			WFIFOSET(fd, packet_len_table[0x167]);
		}
	}
	else {
		WFIFOW(fd, 0) = 0x167;
		WFIFOB(fd, 2) = 1;
		WFIFOSET(fd, packet_len_table[0x167]);
	}
	return 0;
}

int check_guild_exists(int guild_id)
{
	int i;

	for (i = 1; i < MAX_GUILDS; i++) {
		if (guild_dat[i].guild_id == guild_id)
			return i;
	}
	return -1;
}

int connected_members(int guild_id)
{
	int members = 0;
	unsigned int fd;
	struct map_session_data *sd;

	for (fd = 5; fd < FD_SETSIZE; fd++) {
		if (session[fd] && (sd = session[fd]->session_data)) {
			if (sd->status.guild_id == guild_id)
				members++;
		}
	}
	return members;
}
