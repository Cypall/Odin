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

void display_xp(struct map_session_data *sd, int membercount, int base_exp, int job_exp, int per_damage)
{
	char msg[64];

	if (membercount) {
		sprintf(msg, "You are 1 of %d in %s; between levels %d and %d.", membercount, sd->status.party_name, sd->status.base_level - 10, sd->status.base_level + 10);
		send_msg(sd->fd, msg);
	}
	sprintf(msg, "EXP Gained: Base:[%d] Job:[%d] %%Dmg:[%d]", base_exp, job_exp, (per_damage / 1000));
	send_msg(sd->fd, msg);
}

void base_exp_up(struct map_session_data *sd, int base_exp, int per_damage)
{
	int i = 0;

	if (per_damage >= 10000)
		per_damage = 10000;

	else if (per_damage <= 0)
		per_damage = 1;

	sd->status.base_exp += (base_exp * per_damage) / 10000;
	while (sd->status.base_exp >= ExpData[sd->status.base_level + i] && (sd->status.base_level + i) <= 100) {
		sd->status.base_exp -= ExpData[sd->status.base_level + i];
		i++;
	}
	if (i)
		mmo_map_level_up(sd->fd, i);

	mmo_map_set00b1(sd->fd, 0x01, sd->status.base_exp);
	mmo_map_set00b1(sd->fd, 0x16, ExpData[sd->status.base_level]);
}

void job_exp_up(struct map_session_data *sd, int job_exp, int per_damage)
{
	int i = 0;
	unsigned int fd = sd->fd;

	if (per_damage >= 10000)
		per_damage = 10000;

	else if (per_damage <= 0)
		per_damage = 1;

	sd->status.job_exp += (job_exp * per_damage) / 10000;
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
	}
	mmo_map_set00b1(sd->fd, 0x02, sd->status.job_exp);
	switch (sd->status.class) {
	case NOVICE:
		mmo_map_set00b1(fd, 0x17, SkillExpData[0][sd->status.job_level]);
		break;
	case SWORDMAN:
	case MAGE:
	case ARCHER:
	case ACOLYTE:
	case MERCHANT:
	case THIEF:
		mmo_map_set00b1(fd, 0x17, SkillExpData[1][sd->status.job_level]);
		break;
	case KNIGHT:
	case PRIEST:
	case WIZARD:
	case BLACKSMITH:
	case HUNTER:
	case ASSASSIN:
	case KNIGHT_PECOPECO:
	case CRUSADER:
	case MONK:
	case SAGE:
	case ROGUE:
	case ALCHEMIST:
	case BARD:
	case DANCER:
	case CRUSADER_PECOPECO:
	case WEDDING:
	case SUPER_NOVICE:
		mmo_map_set00b1(fd, 0x17, SkillExpData[2][sd->status.job_level]);
		break;
	}
}

int party_earn_exp(int party_num, int base_exp, int job_exp, int per_damage, int mapno)
{
	int i, membercount = 0;
	unsigned int fd;
	struct map_session_data *sd;

	if (party_num < 0)
		return 1;

	if (party_dat[party_num].exp != 1)
		return 1;

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
				if (party_dat[party_num].member[i].offline != 0
				|| party_dat[party_num].member[i].mapno != mapno)
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
	int i;
	int per_damage = 1;
	int base_exp, job_exp;
	unsigned int target_fd = 0;
	struct map_session_data *target_sd = NULL;
	struct npc_data *monster;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP || map_data[m].npc[n]->block.subtype != MONS)
		return 0;

	if (!(monster = map_data[m].npc[n]))
		return 0;

	base_exp = mons_data[monster->class].base_exp;
	job_exp = mons_data[monster->class].job_exp;

	if (sd->status.damage_atk < 1)
		sd->status.damage_atk = 1;

	if (base_exp < 1)
		base_exp = 1;

	if (job_exp < 1)
		job_exp = 1;

	for (i = 5; i < FD_SETSIZE; i++) {
		if (session[i] && (target_sd = session[i]->session_data)) {
			if (monster->u.mons.target_fd == (unsigned)target_sd->account_id) {
				target_fd = i;
				break;
			}
		}
	}
	if (sd->status.damage_atk > mons_data[monster->class].max_hp)
		sd->status.damage_atk = mons_data[monster->class].max_hp;

	if (mons_data[monster->class].max_hp > 0) {
		if (target_fd > 0) {
			if (sd->fd == target_fd)
				per_damage = 10000 * sd->status.damage_atk * 2 / (mons_data[monster->class].max_hp + sd->status.damage_atk);

			else
				per_damage = 10000 * sd->status.damage_atk / (mons_data[monster->class].max_hp + target_sd->status.damage_atk);

		}
		else
			per_damage = 10000 * sd->status.damage_atk / mons_data[monster->class].max_hp;

	}
	if (sd->status.party_status > 0 && party_dat[party_exists(sd->status.party_id)].exp == 1)
		party_earn_exp(party_exists(sd->status.party_id), mons_data[monster->class].base_exp, mons_data[monster->class].job_exp, per_damage, sd->mapno);

	else {
		base_exp_up(sd, base_exp, per_damage);
		job_exp_up(sd, job_exp, per_damage);
		if (display_exp)
			display_xp(sd, 0, base_exp, job_exp, per_damage);

	}
	return 0;
}
