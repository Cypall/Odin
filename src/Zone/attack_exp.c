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
 Modified Date: October 31, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

void display_xp(struct map_session_data *sd, int membercount, int base_exp, int job_exp, int per_damage)
{
	if (membercount) {
		sprintf(msg, "You are 1 of %d in %s Between levels %d and %d", membercount, sd->status.party_name, sd->status.base_level - 10, sd->status.base_level + 10);
		send_msg(sd->fd, msg);
	}
	sprintf(msg, "EXP Gained: Base:[%d] Job:[%d] %%dmg:[%d]", base_exp, job_exp, per_damage);
	send_msg(sd->fd, msg);
	strcpy(msg, "");
}

void base_exp_up(struct map_session_data *sd, int base_exp, int per_damage)
{
	int i = 0;
	unsigned char buf[256];

	if (per_damage >= 100)
		per_damage = 100;

	else if (per_damage <= 0)
		per_damage = 1;

	sd->status.base_exp += (base_exp * per_damage) / 100;
	while (sd->status.base_exp >= ExpData[sd->status.base_level + i] && (sd->status.base_level + i) <= 100) {
		sd->status.base_exp -= ExpData[sd->status.base_level + i];
		i++;
	}
	if (i) {
		mmo_map_level_up(sd->fd, i);
		WBUFW(buf, 0) = 0x19b;
		WBUFL(buf, 2) = sd->account_id;
		WBUFL(buf, 6) = 0;
		mmo_map_sendarea(sd->fd, buf, packet_len_table[0x19b], 0);
		i = 0;
	}
	mmo_map_set00b1(sd->fd, 0x01, sd->status.base_exp);
	mmo_map_set00b1(sd->fd, 0x16, ExpData[sd->status.base_level]);
}

void job_exp_up(struct map_session_data *sd, int job_exp, int per_damage)
{
	int i = 0;
	unsigned int fd = sd->fd;
	unsigned char buf[256];

	if (per_damage >= 100)
		per_damage = 100;

	else if (per_damage <= 0)
		per_damage = 1;

	sd->status.job_exp += (job_exp * per_damage) / 100;
	next_job_exp = 0;
	do {
		if (sd->status.class == 0)
			next_job_exp = SkillExpData[0][sd->status.job_level + i];

		else if (sd->status.class > 0 && sd->status.class < 7)
			next_job_exp = SkillExpData[1][sd->status.job_level + i];

		else if (sd->status.class > 6 && sd->status.class < 21)
			next_job_exp = SkillExpData[2][sd->status.job_level + i];

		if (sd->status.job_exp >= next_job_exp && (sd->status.job_level + i) <= 50) {
			sd->status.job_exp -= next_job_exp;
			i++;
		}
		else
			i = -i;
	}
	while (i > 0);
	if (i) {
		i = -i;
		mmo_map_job_lv_up(sd->fd, i);
		WBUFW(buf, 0) = 0x19b;
		WBUFL(buf, 2) = sd->account_id;
		WBUFL(buf, 6) = 1;
		mmo_map_sendarea(sd->fd, buf, packet_len_table[0x19b], 0);
	}
	mmo_map_set00b1(sd->fd, 0x02, sd->status.job_exp);
	switch (sd->status.class) {
		case 0:
			mmo_map_set00b1(fd, 0x17, SkillExpData[0][sd->status.job_level]);
			break;
		case 1: case 2: case 3: case 4: case 5:
			mmo_map_set00b1(fd, 0x17, SkillExpData[1][sd->status.job_level]);
			break;
		case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 14: case 15:
		case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
			mmo_map_set00b1(fd, 0x17, SkillExpData[2][sd->status.job_level]);
			break;
	}
}

int party_earn_exp(int party_num, int base_exp, int job_exp, short mapno)
{
	int i, per_damage;
	int membercount = 0;
	unsigned int fd;
	struct map_session_data *sd;

	if (party_num < 0)
		return 0;

	if (party_dat[party_num].exp != 1)
		return 0;

	per_damage = 100;
	for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
		if (party_dat[party_num].member[i].fd != 0 && party_dat[party_num].member[i].offline == 0 && party_dat[party_num].member[i].mapno == mapno)
			membercount++;

		else
			continue;

	}
	if (membercount < 1)
		membercount = 1;

	base_exp += base_exp * (membercount - 1) / 10;
	job_exp += job_exp * (membercount - 1) / 10;
	base_exp = (int)(base_exp / membercount);
	job_exp = (int)(job_exp / membercount);

	for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
		if (party_dat[party_num].member[i].fd != 0) {
			if (session[party_dat[party_num].member[i].fd] != NULL) {
				fd = party_dat[party_num].member[i].fd;
				sd = session[fd]->session_data;
				if (party_dat[party_num].member[i].offline != 0 ||
				    party_dat[party_num].member[i].mapno != mapno)
					continue;

				if (base_exp < 1)
					base_exp = 1;

				if (job_exp < 1)
					job_exp = 1;

				base_exp_up(sd, base_exp, per_damage);
				job_exp_up(sd, job_exp, per_damage);
				if (display_exp == 1)
					display_xp(sd, membercount, base_exp, job_exp, per_damage);

			}
			else
				party_dat[party_num].member[i].fd = 0;

		}
	}
	return 0;
}

int mmo_map_level_mons(struct map_session_data *sd, short m, int n)
{
	int per_damage = 1;
	int base_exp, job_exp;

	if (n < 1)
		return 0;

	if (!map[m][0])
		return 0;

	if (sd->status.party_status > 0 && party_dat[party_exists(sd->status.party_id)].exp == 1) {
		party_earn_exp(party_exists(sd->status.party_id), mons_data[(map_data[m].npc[n]->class)].base_exp, mons_data[map_data[m].npc[n]->class].job_exp, sd->mapno);
		return 0;
	}
	else {
		base_exp = mons_data[map_data[m].npc[n]->class].base_exp;
		job_exp = mons_data[map_data[m].npc[n]->class].job_exp;
	}
	if (sd->status.damage_atk < 1)
		sd->status.damage_atk = 1;

	if (mons_data[map_data[m].npc[n]->class].max_hp < 1)
		mons_data[map_data[m].npc[n]->class].max_hp = 1;

	if (sd->status.damage_atk > mons_data[map_data[m].npc[n]->class].max_hp)
		sd->status.damage_atk = mons_data[map_data[m].npc[n]->class].max_hp;

	if (mons_data[map_data[m].npc[n]->class].max_hp >= 0)
		per_damage = (sd->status.damage_atk / mons_data[map_data[m].npc[n]->class].max_hp) * 100;

	sd->status.damage_atk = 0;
	if (base_exp < 1)
		base_exp = 1;

	if (job_exp < 1)
		job_exp = 1;

	base_exp_up(sd, base_exp, per_damage);
	job_exp_up(sd, job_exp, per_damage);
	if (display_exp == 1)
		display_xp(sd, 0, base_exp, job_exp, per_damage);

	return 0;
}
