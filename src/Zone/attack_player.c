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

#define NOM_ATTACK(atk1, atk2, def) ((atk1 + rand() % (atk2)) - (def) - 3 + rand() % 8);
#define BOW_ATTACK(atk1, atk2, dex, def) ((dex + atk1 + rand() % (atk2)) - (def) - 3 + rand() % 8);
#define CRI_ATTACK(atk1, atk2, s_lv, s_type) (atk1 + atk2 + (s_lv) * (s_type) - 3 + rand() % 8);
#define KAT_ATTACK(damage) (((damage) / 5) + 1);
#define MD(min, max) ((min >= max) ? max : (min + rand() % (max - (min))));

void mmo_map_attack_continue(int tid, unsigned int tick, int id, int data)
{
	struct map_session_data *sd;

	if (!session[id] || !(sd = session[id]->session_data))
		return;

	if (sd->sitting == 1 || sd->hidden || sd->skill_timeamount[135-1][0] > 0 || sd->act_dead)
		return;

	switch(data) {
	case 0:
		mmo_map_once_attack(id, sd->attacktarget, tick, 0);
		sd->attacktimer = add_timer(tick + 1400 - (700 - sd->status.aspd) * 2, mmo_map_attack_continue, id, 0);
		break;
	case 1:
		mmo_map_once_attack(id, sd->attacktarget, tick, 0);
		break;
	}
}

void mmo_player_send_death(struct map_session_data *sd)
{
	int len;
	unsigned int fd = sd->fd;
	unsigned char buf[64];

	WBUFW(buf, 0) = 0x80;
	WBUFL(buf, 2) = sd->account_id;
	WBUFB(buf, 6) = 1;
	mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);

	mmo_player_stop_skills_timer(sd);
	mmo_player_attack_no(sd);
	mmo_player_stop_walk(sd);
	sd->sitting = 1;
	sd->status.option_x = 0;
	sd->status.option_y = 0;
	sd->status.option_z = 0;
	sd->status.effect = 00000000;

	if (mmo_card_equiped(sd, 4144) > 0) {
		sd->status.hp = sd->status.max_hp;
		sd->status.sp = sd->status.max_sp;
		len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_SP);
		if (len > 0)
			WFIFOSET(fd, len);

	}
	else if (sd->status.class == 0)
		sd->status.hp = sd->status.max_hp / 2;

	else
		sd->status.hp = 1;

	len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_HP);
	if (len > 0)
		WFIFOSET(fd, len);

	mmo_map_setoption(sd, buf, 0);
	mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
}

void mmo_player_attack_no(struct map_session_data *sd)
{
	if (sd->attacktimer > 0) {
		delete_timer(sd->attacktimer, mmo_map_attack_continue);
		sd->attacktimer = 0;
	}
	sd->attacktarget = 0;
}

void mmo_player_stop_walk(struct map_session_data *sd)
{
	if (sd->walktimer > 0) {
		delete_timer(sd->walktimer, walk_char);
		sd->walktimer = 0;
		sd->walkpath_len = 0;
		sd->walkpath_pos = 0;
	}
}

void mmo_player_stop_skills_timer(struct map_session_data *sd)
{
	int i;

	for (i = 1; i < MAX_SKILL; i++) {
		if (sd->skill_timeamount[i-1][0] <= 0)
			continue;

		skill_reset_stats(0, 0, sd->fd, i);
	}
}

void attack_player(unsigned int fd, unsigned int target_fd, int damage)
{
	int i, len;
	struct map_session_data *sd, *target_sd, *csd;

	if (session[fd] && session[target_fd] && (sd = session[fd]->session_data) && (target_sd = session[target_fd]->session_data)) {
		target_sd->status.hp -= damage;
		if (target_sd->status.hp <= 0)
			target_sd->status.hp = 0;

		len = mmo_map_set_param(target_fd, WFIFOP(target_fd, 0), SP_HP);
		if (len > 0)
			WFIFOSET(target_fd, len);

		if (target_sd->status.hp <= 0) {
			mmo_player_send_death(target_sd);
			for (i = 5; i < FD_SETSIZE; i++)
				if (session[i] && (csd = session[i]->session_data))
					if (csd->attacktarget == target_sd->account_id)
						mmo_player_attack_no(csd);



			target_sd->pvprank--;
			if (target_sd->pvprank < 0)
				target_sd->pvprank = 0;

			mmo_map_checkpvpmap(target_sd);

			sd->pvprank++;
			mmo_map_checkpvpmap(sd);
		}
	}
}

void attack_monster(unsigned int fd, short m, int n, long target_id, int damage)
{
	int mvp_damage = 0, x = 0, y = 0, z = 0;
	char msg[64];
	unsigned int mvp_fd = 0, i;
	struct map_session_data *sd, *csd;
	struct npc_data *monster;

	if (session[fd] && (sd = session[fd]->session_data)) {
		if (!(monster = map_data[m].npc[n]))
			return;

		if (show_hp > 0 && damage > 10) {
			x = monster->u.mons.hp;
			y = mons_data[monster->class].max_hp;
			z = show_hp;
			if (x > 0 && y > 0  && x < y && x != y) {
				sprintf(msg, "( %d / %d )", x, y);
				if (z == 1)
					z = 0;

				if (z == 2)
					z = 1;

				send_msg_mon(fd, monster->id, msg, z);
			}
		}
		monster->u.mons.hp -= damage;
		if (monster->u.mons.hp <= 0)
			monster->u.mons.hp = 0;

		if ((rand() % 90) == 10) {
			if (monster->u.mons.hp >= (mons_data[monster->class].max_hp / 2))
				monster_say(fd, target_id, monster->class, "hp50");

			else if (monster->u.mons.hp <= (mons_data[monster->class].max_hp / 4))
				monster_say(fd, target_id, monster->class, "hp25");

			else if (monster->u.mons.hp <= 0)
				monster_say(fd, target_id, monster->class, "dead");

		}
		if (monster->u.mons.target_fd != fd && monster->u.mons.hp > 0)
			check_new_target_monster(m, n, fd);

		if (monster->u.mons.hp <= 0) {
			mmo_mons_send_death(fd, m, n);
			for (i = 5; i < FD_SETSIZE; i++) {
				if (session[i] && (csd = session[i]->session_data)) {
					if (csd->attacktarget == target_id) {
						if (csd->status.damage_atk >= mvp_damage) {
							mvp_damage = csd->status.damage_atk;
							mvp_fd = i;
						}
						mmo_map_level_mons(csd, m, n);
						mmo_player_attack_no(csd);
						csd->status.damage_atk = 0;
					}
				}
			}
			if (mvp_fd > 0)
				mmo_map_mvp_do(mvp_fd, m, n);

		}
	}
}

int size_mod[18][3] = {
	{100, 100, 100},
	{100,  75,  50},
	{ 75, 100,  75},
	{ 75,  75, 100},
	{ 75,  75, 100},
	{ 75,  75, 100},
	{ 50,  75, 100},
	{ 50,  75, 100},
	{ 75, 100, 100},
	{  0,   0,   0},
	{100, 100, 100},
	{100, 100,  75},
	{  0,   0,   0},
	{  0,   0,   0},
	{  0,   0,   0},
	{100, 100,  50},
	{ 75, 100,  75}
};

int d_atk_bonus[4][11] =  {
	{0, 0, 0, 0, 0, 0, 0, 0, 3, 6, 9},
	{0, 0, 0, 0, 0, 0, 0, 5,10,15,20},
	{0, 0, 0, 0, 0, 0, 8,16,24,32,40},
	{0, 0, 0, 0, 0,14,28,42,56,70,84}
};

int peco_size_mod[3] = {75, 100, 100};

int mmo_map_calc_damage(unsigned int fd, int atk1, int atk2, int atk2b, int atk_ele, short m, short n, int arrow_id, int critical)
{
	int base_dmg = 0, wpn_atk = 0, def1 = 0, def2 = 0, damage = 0;
	char s_lv = 0, s_type = 0;
	struct map_session_data *sd;
	struct npc_data *monster;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	if (!(monster = map_data[m].npc[n]))
		return 0;

	if (arrow_id)
		base_dmg = (sd->status.dex + sd->status.dex2) + ((sd->status.dex + sd->status.dex2) * (sd->status.dex + sd->status.dex2) / 100) + ((sd->status.str + sd->status.str2) / 5) + ((sd->status.luk + sd->status.luk2) / 5);

	else
		base_dmg = (sd->status.str + sd->status.str2) + ((sd->status.str + sd->status.str2) * (sd->status.str + sd->status.str2) / 100) + ((sd->status.dex + sd->status.dex2) / 5) + ((sd->status.luk + sd->status.luk2) / 5);

	wpn_atk = atk1 - base_dmg;
	def1 = mons_data[monster->class].def1;
	def2 = mons_data[monster->class].def2;
	if ((monster->skilldata.effect & ST_POISON)) {
		def1 = def1 * 75 / 100;
		def2 = def2 * 75 / 100;
	}
	if ((monster->skilldata.effect & ST_FROZEN)) {
		def1 = def1 * 50 / 100;
		def2 = def2 * 50 / 100;
	}
	if ((critical % 10) == 2) {
		def1 = 0;
		def2 = 0;
	}
	if (sd->skill_timeamount[61-1][0] > 0) {
		def1 = 0;
		def2 = 0;
	}
	if (sd->status.race_def_pierce[mons_data[monster->class].race] == 1) {
		def1 = 0;
		def2 = 0;
	}
	if ((critical % 10) == 2)
		damage += wpn_atk;

	else {
		if (sd->skill_timeamount[114-1][0] > 0 || critical > 0)
			damage += wpn_atk;

		else {
			if (arrow_id) {
				damage += MD(sd->status.str + sd->status.str2, wpn_atk);
			}
			else {
				damage += MD(sd->status.dex + sd->status.dex2, wpn_atk);
			}
		}
		if (sd->skill_timeamount[112-1][0] == 0) {
			if ((sd->status.option_z == 32) && ((sd->status.weapon == 4) || (sd->status.weapon == 5)))
				damage = damage * peco_size_mod[mons_data[monster->class].scale] / 100;

			else
				damage = damage * size_mod[sd->status.weapon][mons_data[monster->class].scale] / 100;

		}
	}
	damage += base_dmg;
	if (arrow_id)
		damage += item_database(arrow_id).atk;

	if (sd->skill_timeamount[113-1][0] > 0)
		damage += damage * sd->skill_timeamount[113-1][1] / 100;

	damage -= damage * def2 / 100;
	damage -= def1;
	if (sd->skill_timeamount[66-1][0] > 0)
		damage += sd->skill_timeamount[66-1][1];

	if (sd->status.skill[23-1].lv > 0 && (mons_data[monster->class].race == 1 || mons_data[monster->class].race == 6))
		damage += sd->status.skill[23-1].lv * 3;

	if (sd->status.skill[126-1].lv > 0 && (mons_data[monster->class].race == 2 || mons_data[monster->class].race == 4))
		damage += sd->status.skill[126-1].lv * 4;

	damage += atk2;
	if (atk2b > 0)
		damage += MD(1, atk2b);

	if (damage < 0)
		damage = 0;

	if (sd->status.weapon == 4 || sd->status.weapon == 5) {
		s_type = (sd->status.option_z & 0x20) ? 5 : 4;
		s_lv = sd->status.skill[55-1].lv;
	}
	else if (sd->status.weapon == 2 || sd->status.weapon == 3) {
		s_type = 4;
		s_lv = sd->status.weapon == 2 ? sd->status.skill[2-1].lv : sd->status.skill[3-1].lv;
	}
	else if (sd->status.weapon == 16 || sd->status.weapon == 8) {
		s_type = 3;
		s_lv = sd->status.weapon == 16 ? sd->status.skill[134-1].lv : sd->status.skill[65-1].lv;
	}
	else
		s_lv = s_type = 0;

	damage += s_lv * s_type;
	if (sd->status.skill[107-1].lv > 0)
		damage += sd->status.skill[107-1].lv * 2;

	if (critical == 0)
		damage = (int)(damage * get_ele_attack_factor(atk_ele, mons_data[monster->class].ele));

	else if ((critical % 10) == 1) {
		if (get_ele_attack_factor(atk_ele, mons_data[monster->class].ele) > 1)
			damage = (int)(damage * get_ele_attack_factor(atk_ele, mons_data[monster->class].ele));

	}
	damage += (damage * sd->status.size_atk_mod[mons_data[monster->class].scale]) / 100;
	damage += (damage * sd->status.race_atk_mod[mons_data[monster->class].race]) / 100;
	damage += (damage * sd->status.ele_atk_mod[(mons_data[monster->class].ele % 10)]) / 100;
	damage += (critical - 1) / 10;
	if ((sd->skill_timeamount[114-1][0] <= 0 || sd->skill_timeamount[114-1][1] != 1) && sd->skill_timeamount[61-1][0] == 0)
		damage -= monster->u.mons.def1;

	if (monster->u.mons.lexaeterna)
		damage *= 2;

	if (damage < 0)
		damage = 1;

	return damage;
}

int mmo_map_once_attack(unsigned int fd, long target_id, unsigned long tick, int alt_damage)
{
	int i = 0, j = 0, n;
	short m;
	int critical = 0, damage = 0, hit = 0, damage2 = 0, range = 0, double_luck = 0, triple_luck = 0;
	int dagger2_id = 0, dagger2_ele = 0, dagger2_atk1 = 0, dagger2_atk2 = 0, dagger2_atk2b = 0, dagger2_base = 0, dagger2_forged = 0;
	static int double_attack[10] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50 };
	static int double_attack_damage[10] = { 20, 40, 60, 80, 100, 120, 140, 160, 180, 200 };
	static int triple_attack[10] = { 29, 28, 27, 26, 25, 24, 23, 22, 21, 20 };
	int found = 0;
	short index = 0;
	int arrow_id = 0;
	unsigned char buf[64];
	struct map_session_data *sd;
	struct npc_data *monster;

	if (session[fd] && (sd = session[fd]->session_data)) {
		m = sd->mapno;
		n = mmo_map_search_monster(m, target_id);
		if (n < 0) {
			mmo_map_pvp_attack(fd, target_id, tick);
			return 0;
		}
		if (!(monster = map_data[m].npc[n])) {
			mmo_player_attack_no(sd);
			return 0;
		}
		if (sd->sitting == 1 || sd->hidden || sd->skill_timeamount[135-1][0] > 0 || sd->act_dead) {
			mmo_player_attack_no(sd);
			return 0;
		}
		if (monster->u.mons.hidden || monster->u.mons.hp <= 0) {
			mmo_player_attack_no(sd);
			return 0;
		}
		if (sd->status.skill[263-1].lv > 0) {
			triple_luck = rand() % 100;
			if (triple_luck <= triple_attack[sd->status.skill[263-1].lv-1]) {
				skill_do_delayed_target(0, tick, fd, 263);
				mmo_player_attack_no(sd);
				return 0;
			}
		}
		range = sd->status.range;
		if (sd->status.weapon == 11) {
			range = sd->status.range + sd->status.skill[44-1].lv;
			for (i = 0; i < MAX_INVENTORY; i++) {
				if (found == 1)	
					break;

				for (j = 1770; j >= 1750; j--) {
					if (sd->status.inventory[i].nameid == j && sd->status.inventory[i].equip == 0x8000) {
						found = 1;
						index = i + 2;
						break;
					}
				}
			}
			if (found == 1) {
				arrow_id = j;
				mmo_map_lose_item(fd, 0, index, 1);
			}
			else {
				WFIFOW(fd, 0) = 0x013b;
				WFIFOW(fd, 2) = 0;
				WFIFOSET(fd, packet_len_table[0x13b]);
				mmo_player_attack_no(sd);
				return 0;
			}
		}
		if (!in_range(sd->x, sd->y, monster->x, monster->y, range + 2)) {
			WFIFOW(fd, 0) = 0x139;
			WFIFOL(fd, 2) = target_id;
			WFIFOW(fd, 6) = monster->x;
			WFIFOW(fd, 8) = monster->y;
			WFIFOW(fd, 10) = sd->x;
			WFIFOW(fd, 12) = sd->y;
			WFIFOW(fd, 14) = range;
			WFIFOSET(fd, packet_len_table[0x139]);
			mmo_player_attack_no(sd);
			return 0;
		}
		if ((sd->skill_timeamount[61-1][0] <= 0) && (calc_dir3(sd->x, sd->y, monster->x, monster->y) != -1))
			sd->dir = sd->head_dir = calc_dir3(sd->x, sd->y, monster->x, monster->y);

		for (i = 0; i < MAX_INVENTORY; i++) {
			if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip == 32)) {
				struct item_db2 weapon;
				weapon = item_database(sd->status.inventory[i].nameid);
				if (weapon.type == 4) {
					dagger2_id = sd->status.inventory[i].nameid;
					dagger2_ele = weapon.ele;
					dagger2_base = (sd->status.str + sd->status.str2) + ((sd->status.str + sd->status.str2) * (sd->status.str + sd->status.str2) / 100) + ((sd->status.dex + sd->status.dex2) / 5) + ((sd->status.luk + sd->status.luk2) / 5);
					dagger2_atk1 = weapon.atk;
					if (sd->status.inventory[i].card[0] == 0x00ff) {
						dagger2_ele = sd->status.inventory[i].card[1];
						dagger2_forged = 1 + 10 * sd->status.inventory[i].card[3];
					}
					if (weapon.wlv == 1) {
						dagger2_atk2 += (sd->status.inventory[i].refine * 2);
						dagger2_atk2b += d_atk_bonus[0][sd->status.inventory[i].refine - 1];
					}
					if (weapon.wlv == 2) {
						dagger2_atk2 += (sd->status.inventory[i].refine * 3);
						dagger2_atk2b += d_atk_bonus[1][sd->status.inventory[i].refine - 1];
					}
					if (weapon.wlv == 3) {
						dagger2_atk2 += (sd->status.inventory[i].refine * 5);
						dagger2_atk2b += d_atk_bonus[2][sd->status.inventory[i].refine - 1];
					}
					if (weapon.wlv >= 4) {
						dagger2_atk2 += (sd->status.inventory[i].refine * 7);
						dagger2_atk2b += d_atk_bonus[3][sd->status.inventory[i].refine - 1];
					}
					if (dagger2_atk1 < 1)
						dagger2_atk1 = 1;

					if (dagger2_atk2 < 1)
						dagger2_atk2 = 1;

					if (sd->skill_timeamount[138-1][0] > 0)
						dagger2_ele = 5;

					break;
				}
			}
		}
		if (alt_damage) {
			if (in_range(sd->x, sd->y, monster->x, monster->y, range)) {
				WBUFW(buf, 0) = 0x8a;
				WBUFL(buf, 2) = sd->account_id;
				WBUFL(buf, 6) = target_id;
				WBUFL(buf, 10) = tick;
				WBUFL(buf, 14) = sd->status.aspd;
				WBUFL(buf, 18) = monster->u.mons.attackdelay;
				WBUFW(buf, 22) = alt_damage;
				WBUFW(buf, 24) = 0;
				WBUFB(buf, 26) = 0;
				WBUFW(buf, 27) = 0;
				mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
				attack_monster(fd, m, n, target_id, alt_damage);
			}
			mmo_player_attack_no(sd);
			return 0;
		}
		hit = (sd->status.hit - mons_data[monster->class].flee) * 5 + 55;
		if ((monster->skilldata.effect & ST_BLIND))
			hit += 25;

		if (sd->skill_timeamount[61-1][0] > 0)
			hit += 20;

		if (sd->status.skill[107-1].lv > 0)
			hit += sd->status.skill[107-1].lv * 2;

		if (hit < 50)
			hit = 60;

		if (sd->status.weapon == 16 || (monster->skilldata.effect & ST_STUN) || sd->skill_timeamount[61-1][0] > 0)
			critical = sd->status.critical * 2;

		else
			critical = sd->status.critical;

		if (rand() % 100 >= critical) {
			if (rand() % 100 >= hit) {
				damage = 0;
				WBUFW(buf, 0) = 0x8a;
				WBUFL(buf, 2) = sd->account_id;
				WBUFL(buf, 6) = target_id;
				WBUFL(buf, 10) = tick;
				WBUFL(buf, 14) = sd->status.aspd;
				WBUFL(buf, 18) = monster->u.mons.attackdelay;
				WBUFW(buf, 22) = 0;
				WBUFW(buf, 24) = 0;
				WBUFB(buf, 26) = 0;
				WBUFW(buf, 27) = 0;
				mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
			}
			else {
				if (dagger2_id) {
					damage = mmo_map_calc_damage(fd, sd->status.atk1 - dagger2_atk1, sd->status.atk2 - dagger2_atk2, sd->status.atk2b - dagger2_atk2b, sd->status.attack_ele, m, n, arrow_id, sd->status.wpn_forged);
					damage2 = mmo_map_calc_damage(fd, dagger2_base + dagger2_atk1, dagger2_atk2, dagger2_atk2b, dagger2_ele, m, n, arrow_id, dagger2_forged);
					damage = damage * (50 + 10 * sd->status.skill[132-1].lv) / 100;
					damage2 = damage2 * (30 + 10 * sd->status.skill[133-1].lv) / 100;
				}
				else
					damage = mmo_map_calc_damage(fd, sd->status.atk1, sd->status.atk2, sd->status.atk2b, sd->status.attack_ele, m, n, arrow_id, sd->status.wpn_forged);

				WBUFW(buf, 0) = 0x8a;
				WBUFL(buf, 2) = sd->account_id;
				WBUFL(buf, 6) = target_id;
				WBUFL(buf, 10) = tick;
				WBUFL(buf, 14) = sd->status.aspd;
				WBUFL(buf, 18) = monster->u.mons.attackdelay;
				if (sd->status.weapon == 16) {
					damage2 = KAT_ATTACK(damage);
					WBUFW(buf, 22) = damage;
					WBUFW(buf, 24) = 2;
					WBUFB(buf, 26) = 0;
					WBUFW(buf, 27) = damage2;
				}
				else if (sd->status.skill[48-1].lv > 0 && sd->status.weapon == 1) {
					double_luck = rand() % 100;
					if (double_luck <= double_attack[sd->status.skill[48-1].lv-1]) {
						if (dagger2_id) {
							damage = (int) ((damage * double_attack_damage[sd->status.skill[48-1].lv-1]) / 100) + damage + damage2;
							WBUFW(buf, 22) = damage;
							WBUFW(buf, 24) = 3;
							WBUFB(buf, 26) = 8;
							WBUFW(buf, 27) = 0;
						}
						else {
							damage = (int) ((damage * double_attack_damage[sd->status.skill[48-1].lv-1]) / 100) + damage;
							WBUFW(buf, 22) = damage;
							WBUFW(buf, 24) = 2;
							WBUFB(buf, 26) = 8;
							WBUFW(buf, 27) = 0;
						}
					}
					else {
						if (dagger2_id) {
							WBUFW(buf, 22) = damage;
							WBUFW(buf, 24) = 2;
							WBUFB(buf, 26) = 0;
							WBUFW(buf, 27) = damage2;
						}
						else {
							WBUFW(buf, 22) = damage;
							WBUFW(buf, 24) = 1;
							WBUFB(buf, 26) = 0;
							WBUFW(buf, 27) = 0;
						}
					}
				}
				else {
					if (dagger2_id) {
						WBUFW(buf, 22) = damage;
						WBUFW(buf, 24) = 2;
						WBUFB(buf, 26) = 0;
						WBUFW(buf, 27) = damage2;
					}
					else {
						WBUFW(buf, 22) = damage;
						WBUFW(buf, 24) = 1;
						WBUFB(buf, 26) = 0;
						WBUFW(buf, 27) = 0;
					}
				}
				mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);

				if (sd->skill_timeamount[138-1][0] > 0) {
					if (rand() % 100 <= sd->skill_timeamount[138-1][1]) {
						if (monster->skilldata.skill_timer[52-1][0] > 0) {
							monster->skilldata.skill_num = 52;
							monster->skilldata.fd = fd;
							skill_reset_monster_stats(0, 0, m, n);
						}
						monster->u.mons.option_y = 1;
						WBUFW(buf, 0) = 0x119;
						WBUFL(buf, 2) = monster->id;
						WBUFW(buf, 6) = monster->u.mons.option_x;
						WBUFW(buf, 8) = monster->u.mons.option_y;
						WBUFW(buf, 10) = monster->u.mons.option_z;
						WBUFB(buf, 12) = 0;
						mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);

						monster->skilldata.fd = fd;
						monster->skilldata.skill_num = 52;
						monster->skilldata.effect |= ST_POISON;
						monster->skilldata.skill_timer[52-1][0] = add_timer(gettick() + 30000, skill_reset_monster_stats, m, n);
						add_timer(gettick() + 30000, skill_drain_hp_monster, m, n);
					}
				}
				if ((monster->skilldata.effect & ST_FROZEN)) {
					monster->skilldata.fd = fd;
					monster->skilldata.skill_num = 15;
					skill_reset_monster_stats(0, 0, m, n);
				}
				else if ((monster->skilldata.effect & ST_STUN)) {
					monster->skilldata.fd = fd;
					monster->skilldata.skill_num = 15;
					skill_reset_monster_stats(0, 0, m, n);
				}
			}
		}
		else {
			if (dagger2_id) {
				damage = mmo_map_calc_damage(fd, sd->status.atk1 - dagger2_atk1, sd->status.atk2 - dagger2_atk2, sd->status.atk2b - dagger2_atk2b, sd->status.attack_ele, m, n, arrow_id, 2);
				damage2 = mmo_map_calc_damage(fd, dagger2_base + dagger2_atk1, dagger2_atk2, dagger2_atk2b, dagger2_ele, m, n, arrow_id, 2);
				damage = damage * (50 + 10 * sd->status.skill[132-1].lv) / 100;
				damage2 = damage2 * (30 + 10 * sd->status.skill[133-1].lv) / 100;
			}
			else
				damage = mmo_map_calc_damage(fd, sd->status.atk1, sd->status.atk2, sd->status.atk2b, sd->status.attack_ele, m, n, arrow_id, 2);

			WBUFW(buf, 0) = 0x8a;
			WBUFL(buf, 2) = sd->account_id;
			WBUFL(buf, 6) = target_id;
			WBUFL(buf, 10) = tick;
			WBUFL(buf, 14) = sd->status.aspd;
			WBUFL(buf, 18) = monster->u.mons.attackdelay;
			WBUFW(buf, 22) = damage;
			if (sd->status.weapon == 16) {
				damage2 = KAT_ATTACK(damage);
				WBUFW(buf, 24) = 2;
				WBUFB(buf, 26) = 0x0a;
				WBUFW(buf, 27) = damage2;
			}
			else if (dagger2_id) {
				WBUFW(buf, 24) = 2;
				WBUFB(buf, 26) = 0x0a;
				WBUFW(buf, 27) = damage2;
			}
			else {
				WBUFW(buf, 24) = 1;
				WBUFB(buf, 26) = 0x0a;
				WBUFW(buf, 27) = 0;
			}
			mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);

			if (sd->skill_timeamount[138-1][0] > 0) {
				if (rand() % 100 <= sd->skill_timeamount[138-1][1]) {
					if (monster->skilldata.skill_timer[52-1][0] > 0) {
						monster->skilldata.skill_num = 52;
						monster->skilldata.fd = fd;
						skill_reset_monster_stats(0, 0, m, n);
					}
					monster->u.mons.option_y = 1;
					WBUFW(buf, 0) = 0x119;
					WBUFL(buf, 2) = monster->id;
					WBUFW(buf, 6) = monster->u.mons.option_x;
					WBUFW(buf, 8) = monster->u.mons.option_y;
					WBUFW(buf, 10) = monster->u.mons.option_z;
					WBUFB(buf, 12) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);

					monster->skilldata.fd = fd;
					monster->skilldata.skill_num = 52;
					monster->skilldata.effect |= ST_POISON;
					monster->skilldata.skill_timer[52-1][0] = add_timer(gettick() + 30000, skill_reset_monster_stats, m, n);
					add_timer(gettick() + 30000, skill_drain_hp_monster, m, n);
				}
			}
			if ((monster->skilldata.effect & ST_FROZEN)) {
				monster->skilldata.fd = fd;
				monster->skilldata.skill_num = 15;
				skill_reset_monster_stats(0, 0, m, n);
			}
			else if ((monster->skilldata.effect & ST_STUN)) {
				monster->skilldata.fd = fd;
				monster->skilldata.skill_num = 15;
				skill_reset_monster_stats(0, 0, m, n);
			}
		}
		for (i = 0; i < 10; i++)
			if (sd->status.status_atk_mod[i] > 0)
				if (rand() % 10000 <= sd->status.status_atk_mod[i])
					break;



		if (damage > 0) {
			if ((damage + damage2) > monster->u.mons.hp)
				sd->status.damage_atk += monster->u.mons.hp;

			else
				sd->status.damage_atk += (damage + damage2);

			attack_monster(fd, m, n, target_id, damage + damage2);
		}
	}
	return 0;
}

int mmo_map_pvp_attack(unsigned int fd, long target_id, unsigned long tick)
{
	int i = 0, j = 0;
	char s_lv = 0, s_type = 0;
	int range = 0, damage = 0, hit = 0, damage2 = 0, critical = 0, double_luck = 0;
	static int double_attack[10] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50 };
	static int double_attack_damage[10] = { 20, 40, 60, 80, 100, 120, 140, 160, 180, 200 };
	int found = 0;
	short index = 0;
	unsigned int k = 0;
	unsigned int target_fd = 0;
	unsigned char buf[64];
	struct map_session_data *sd, *target_sd;

	if (session[fd] && (sd = session[fd]->session_data)) {
		if (mmo_map_flagload(sd->mapname, PVP) || PVP_flag) {
			target_sd = NULL;
			for (k = 5; k < FD_SETSIZE; k++) {
				if (session[k] && (target_sd = session[k]->session_data)) {
					if (target_sd->account_id == target_id) {
						target_fd = k;
						break;
					}
				}
			}
			if (k == FD_SETSIZE)
				return 0;

			if (sd->sitting == 1 || sd->hidden || sd->skill_timeamount[135-1][0] > 0 || sd->act_dead) {
				mmo_player_attack_no(sd);
				return 0;
			}
			if (target_sd->sitting == 1 || target_sd->hidden || sd->skill_timeamount[135-1][0] > 0 || target_sd->act_dead) {
				mmo_player_attack_no(target_sd);
				return 0;
			}
			sd->attacktarget = target_id;
			range = sd->status.range;
			if (sd->status.weapon == 11) {
				range = sd->status.range + sd->status.skill[44-1].lv;
				for (i = 0; i < MAX_INVENTORY; i++) {
					if (found == 1)	
						break;

					for (j = 1770; j >= 1750; j--) {
						if (sd->status.inventory[i].nameid == j && sd->status.inventory[i].equip == 0x8000) {
							found = 1;
							index = i + 2;
							break;
						}
					}
				}
				if (found == 1)
					mmo_map_lose_item(fd, 0, index, 1);

				else {
					WFIFOW(fd, 0) = 0x013b;
					WFIFOW(fd, 2) = 0;
					WFIFOSET(fd, packet_len_table[0x13b]);
					mmo_player_attack_no(sd);
					return 0;
				}
			}
			if (!in_range(sd->x, sd->y, target_sd->x, target_sd->y, range + 2)) {
				WFIFOW(fd, 0) = 0x139;
				WFIFOL(fd, 2) = target_id;
				WFIFOW(fd, 6) = target_sd->x;
				WFIFOW(fd, 8) = target_sd->y;
				WFIFOW(fd, 10) = sd->x;
				WFIFOW(fd, 12) = sd->y;
				WFIFOW(fd, 14) = range;
				WFIFOSET(fd, packet_len_table[0x139]);
				return 0;
			}
			hit = (sd->status.hit - target_sd->status.flee1) * 5;
			if (hit <= 0)
				hit = 20;

			else {
				if (hit >= 100)
					hit = 95;

			}
			if (sd->status.weapon == 16)
				critical = sd->status.critical * 2;

			else
				critical = sd->status.critical;

			if (rand() % 100 >= critical) {
				if (rand() % 100 >= hit) {
					damage = 0;
					WBUFW(buf, 0) = 0x8a;
					WBUFL(buf, 2) = sd->account_id;
					WBUFL(buf, 6) = target_id;
					WBUFL(buf, 10) = tick;
					WBUFL(buf, 14) = sd->status.aspd;
					WBUFL(buf, 18) = target_sd->status.aspd;
					WBUFW(buf, 22) = 0;
					WBUFW(buf, 24) = 0;
					WBUFB(buf, 26) = 0;
					WBUFW(buf, 27) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
				}
				else {
					if (sd->status.weapon == 4 || sd->status.weapon == 5) {
						s_type = 5;
						s_lv = sd->status.skill[55-1].lv;
					}
					else if (sd->status.weapon == 2 || sd->status.weapon == 3) {
						s_type = 4;
						s_lv = sd->status.weapon == 2 ? sd->status.skill[2-1].lv : sd->status.skill[3-1].lv;
					}
					else if (sd->status.weapon == 16 || sd->status.weapon == 8) {
						s_type = 3;
						s_lv = sd->status.weapon == 16 ? sd->status.skill[134-1].lv : sd->status.skill[65-1].lv;
					}
					else
						s_lv = s_type = 0;

					if (sd->status.weapon == 11) {
						damage = BOW_ATTACK(sd->status.atk1, sd->status.atk2, sd->status.dex, target_sd->status.def1);
					}
					else {
						damage = NOM_ATTACK(sd->status.atk1, sd->status.atk2, target_sd->status.def1);
					}
					if (damage <= 0)
						damage = 1;

					damage += s_lv * s_type;
					if (target_sd->skill_timeamount[78-1][0] > 0)
						damage *= 2;

					s_lv = s_type = 0;
					WBUFW(buf, 0) = 0x8a;
					WBUFL(buf, 2) = sd->account_id;
					WBUFL(buf, 6) = target_id;
					WBUFL(buf, 10) = tick;
					WBUFL(buf, 14) = sd->status.aspd;
					WBUFL(buf, 18) = target_sd->status.aspd;
					if (sd->status.weapon == 16) {
						damage2 = KAT_ATTACK(damage);
						WBUFW(buf, 22) = damage;
						WBUFW(buf, 24) = 2;
						WBUFB(buf, 26) = 0;
						WBUFW(buf, 27) = damage2;
					}
					else if (sd->status.skill[48-1].lv > 0) {
						double_luck = rand() % 100;
						if (double_luck >= double_attack[sd->status.skill[48-1].lv - 1]) {
							damage = (int) damage + (damage / 100 * double_attack_damage[sd->status.skill[48-1].lv - 1]);
							WBUFW(buf, 22) = damage;
							WBUFW(buf, 24) = 2;
							WBUFB(buf, 26) = 8;
							WBUFW(buf, 27) = 0;
						}
						else {
							WBUFW(buf, 22) = damage;
							WBUFW(buf, 24) = 1;
							WBUFB(buf, 26) = 0;
							WBUFW(buf, 27) = 0;
						}
					}
					else {
						WBUFW(buf, 22) = damage;
						WBUFW(buf, 24) = 1;
						WBUFB(buf, 26) = 0;
						WBUFW(buf, 27) = 0;
					}
					mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);

					if ((target_sd->status.effect & ST_STUN))
						skill_reset_stats(0, 0, target_fd, 5);

					if ((target_sd->status.effect & ST_FROZEN))
						skill_reset_stats(0, 0, target_fd, 15);

				}
			}
			else {
				if (sd->status.weapon == 4 || sd->status.weapon == 5) {
					s_type = 5;
					s_lv = sd->status.skill[55-1].lv;
				}
				else if (sd->status.weapon == 2 || sd->status.weapon == 3) {
					s_type = 4;
					s_lv = sd->status.weapon == 2 ? sd->status.skill[2-1].lv : sd->status.skill[3-1].lv;
				}
				else if (sd->status.weapon == 16 || sd->status.weapon == 8) {
					s_type = 3;
					s_lv = sd->status.weapon == 16 ? sd->status.skill[134-1].lv : sd->status.skill[65-1].lv;
				}
				else
					s_lv = s_type = 0;

				damage = CRI_ATTACK(sd->status.atk1, sd->status.atk2, s_lv, s_type);
				if (target_sd->skill_timeamount[78-1][0] > 0)
					damage *= 2;

				s_lv = s_type = 0;
				WBUFW(buf, 0) = 0x8a;
				WBUFL(buf, 2) = sd->account_id;
				WBUFL(buf, 6) = target_id;
				WBUFL(buf, 10) = tick;
				WBUFL(buf, 14) = sd->status.aspd;
				WBUFL(buf, 18) = target_sd->status.aspd;
				WBUFW(buf, 22) = damage;
				if (sd->status.weapon == 16) {
					damage2 = KAT_ATTACK(damage);
					WBUFW(buf, 24) = 2;
					WBUFB(buf, 26) = 0x0a;
					WBUFW(buf, 27) = damage2;
				}
				else {
					WBUFW(buf, 24) = 1;
					WBUFB(buf, 26) = 0x0a;
					WBUFW(buf, 27) = 0;
				}
				mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);

				if ((target_sd->status.effect & ST_STUN))
					skill_reset_stats(0, 0, target_fd, 5);

				if ((target_sd->status.effect & ST_FROZEN))
					skill_reset_stats(0, 0, target_fd, 15);

			}
			if (damage > 0)
				attack_player(fd, target_fd, (damage + damage2));

		}
	}
	return 0;
}
