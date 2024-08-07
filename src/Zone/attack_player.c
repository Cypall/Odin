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
 Modified Date: October 24, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

int mmo_map_attack_continue(int tid, unsigned int tick, int id, int data)
{
	struct map_session_data *sd;

	tid = 0;
	if (!session[id] || !(sd = session[id]->session_data))
		return 0;

	if (sd->hidden)
		return 0;

	if (sd->skill_timeamount[143-1][1] == 1)
		return 0;

	if (sd->attacktimer > 0 && data != 1) {
		mmo_map_once_attack(id, sd->attacktarget, tick);
		sd->attacktimer = add_timer(tick + 1400 - (700 - sd->status.aspd) * 2, mmo_map_attack_continue, id, 0);
	}
	else if (sd->attacktimer > 0 && data == 1)
		mmo_map_once_attack(id, sd->attacktarget, tick);

	return 0;
}

int attack_player(int fd, int target_fd, int damage)
{
	int i;
	unsigned char buf[256];
	struct map_session_data *sd, *target_sd, *csd;

	if (session[fd] && session[target_fd] && (sd = session[fd]->session_data) && (target_sd = session[target_fd]->session_data)) {
		target_sd->status.hp -= damage;
		if (target_sd->status.hp <= 0)
			target_sd->status.hp = 0;

		WFIFOW(target_fd, 0) = 0xb0;
		WFIFOW(target_fd, 2) = 0005;
		WFIFOL(target_fd, 4) = target_sd->status.hp;
		WFIFOSET(target_fd, packet_len_table[0xb0]);

		if (target_sd->status.hp <= 0) {
			target_sd->status.hp = 0;
			target_sd->sitting = 1;
			WBUFW(buf, 0) = 0x80;
			WBUFL(buf, 2) = target_sd->account_id;
			WBUFB(buf, 6) = 1;
			mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);

			for (i = 5; i < FD_SETSIZE; i++) {
				if (session[i] && (csd = session[i]->session_data)) {
					if (csd->attacktarget == target_sd->account_id) {
						delete_timer(csd->attacktimer, mmo_map_attack_continue);
						csd->attacktimer = 0;
						csd->attacktarget = 0;
					}
				}
			}
			if (target_sd->attacktimer > 0) {
				delete_timer(target_sd->attacktimer, mmo_map_attack_continue);
				target_sd->attacktimer = 0;
				target_sd->attacktarget = 0;
			}
			sd->pvprank++;
			mmo_map_checkpvpmap(sd);
		}
	}
	return 0;
}

int attack_monster(int fd, short m, int n, int target_id, int damage)
{
	int x = 0, y = 0, z = 0;
	unsigned int mvp_fd = 0, i;
	int mvp_damage = 0;
	char msg[256];
	unsigned char buf[256];
	struct map_session_data *sd, *csd;

	if (session[fd] && (sd = session[fd]->session_data)) {
		if ((sd->skill_timeamount[114-1][0] == 0 || sd->skill_timeamount[114-1][1] != 1) && sd->skill_timeamount[61-1][0] == 0)
			damage -= map_data[m].npc[n]->u.mons.def1;

		if (damage < 0)
			damage = 1;

		if (show_hp > 0 && damage > 10) {
			x = map_data[m].npc[n]->u.mons.hp;
			y = mons_data[map_data[m].npc[n]->class].max_hp;
			z = show_hp;
			if (x > 0 && y > 0  && x < y && x != y) {
				sprintf(msg, "( %d / %d )", x, y);
				if (z == 1)
					z = 0;

				if (z == 2)
					z = 1;

				send_msg_mon(fd, map_data[m].npc[n]->id, msg, z);
			}
		}
		map_data[m].npc[n]->u.mons.hp -= damage;
		if ((rand() % 90) == 10) {
			if (map_data[m].npc[n]->u.mons.hp >= (mons_data[map_data[m].npc[n]->class].max_hp / 2))
				monster_say(fd, target_id, map_data[m].npc[n]->class, "hp50");

			else if (map_data[m].npc[n]->u.mons.hp <= (mons_data[map_data[m].npc[n]->class].max_hp / 4))
				monster_say(fd, target_id, map_data[m].npc[n]->class, "hp25");

			else if (map_data[m].npc[n]->u.mons.hp <= 0)
				monster_say(fd, target_id, map_data[m].npc[n]->class, "dead");

		}
		if (map_data[m].npc[n]->u.mons.target_fd != fd && map_data[m].npc[n]->u.mons.hp > 0)
			check_new_target_monster(m, n, fd);

		if (map_data[m].npc[n]->u.mons.hp <= 0) {
			map_data[m].npc[n]->u.mons.hp = 0;
			if (map_data[m].npc[n]->class > 20) {
				WBUFW(buf, 0) = 0x80;
				WBUFL(buf, 2) = target_id;
				WBUFB(buf, 6) = 1;
				mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
			}
			else {
				WBUFW(buf, 0) = 0x80;
				WBUFL(buf, 2) = target_id;
				WBUFB(buf, 6) = 2;
				mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
			}
			for (i = 5; i < FD_SETSIZE; i++) {
				if (session[i] && (csd = session[i]->session_data) && csd->attacktarget == target_id) {
					if (csd->status.damage_atk >= mvp_damage) {
						mvp_damage = csd->status.damage_atk;
						mvp_fd = i;
					}
					mmo_map_level_mons(csd, m, n);
					delete_timer(csd->attacktimer, mmo_map_attack_continue);
					csd->attacktimer = 0;
					csd->attacktarget = 0;
				}
			}
			if (mvp_fd > 0)
				mmo_map_mvp_do(mvp_fd, m, n);

			mmo_map_item_drop(m, n);
			if (map_data[m].npc[n]->u.mons.attacktimer > 0) {
				delete_timer(map_data[m].npc[n]->u.mons.attacktimer, mmo_mons_attack_continue);
				map_data[m].npc[n]->u.mons.attacktimer = 0;
			}
			map_data[m].npc[n]->u.mons.hp = 0;
			map_data[m].npc[n]->u.mons.target_fd = 0;
			map_data[m].npc[n]->u.mons.walkpath_len = 0;
			map_data[m].npc[n]->u.mons.walkpath_pos = 0;
			map_data[m].npc[n]->u.mons.speed = 0;
			map_data[m].npc[n]->skilldata.fd = 0;
			map_data[m].npc[n]->skilldata.skill_num = 0;
			map_data[m].npc[n]->skilldata.effect = 00000000;
			map_data[m].npc[n]->u.mons.lootdata.id = 0;
			map_data[m].npc[n]->u.mons.lootdata.loot_count = 0;
			set_monster_no_point(m, n);
			if (strncmp(map_data[m].npc[n]->name, mons_data[map_data[m].npc[n]->class].name, 24) != 0) {
				if (mons_data[map_data[m].npc[n]->class].boss == 1) {
					add_timer(gettick() + 60 * 60000, spawn_delay, m, n);
					timer_data[map_data[m].npc[n]->u.mons.timer]->tick = gettick() + 60 + 500;
				}
				else if (mons_data[map_data[m].npc[n]->class].boss == 2) {
					add_timer(gettick() + 45 * 60000, spawn_delay, m, n);
					timer_data[map_data[m].npc[n]->u.mons.timer]->tick = gettick() + 45 + 500;
				}
				else {
					add_timer(gettick() + 60000, spawn_delay, m, n);
					timer_data[map_data[m].npc[n]->u.mons.timer]->tick = gettick() + 1 + 500;
				}
			}
			else {
				delete_timer(map_data[m].npc[n]->u.mons.timer, mons_walk);
				map_data[m].npc[n]->u.mons.timer = 0;
				del_block(&map_data[m].npc[n]->block);
			}
		}
	}
	return 0;
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

int mmo_map_calc_damage(int fd, int atk1, int atk2, int atk2b, int atk_ele, short m, short n, int arrow_id, int critical)
{
	int damage = 0;
	char s_lv = 0, s_type = 0;
	int base_dmg, wpn_atk;
	int def1, def2;
	struct map_session_data *sd = NULL;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	if (arrow_id)
		base_dmg = (sd->status.dex + sd->status.dex2) + ((sd->status.dex + sd->status.dex2) * (sd->status.dex + sd->status.dex2) / 100) + ((sd->status.str + sd->status.str2) / 5) + ((sd->status.luk + sd->status.luk2) / 5);

	else
		base_dmg = (sd->status.str + sd->status.str2) + ((sd->status.str + sd->status.str2) * (sd->status.str + sd->status.str2) / 100) + ((sd->status.dex + sd->status.dex2) / 5) + ((sd->status.luk + sd->status.luk2) / 5);

	wpn_atk = atk1 - base_dmg;
	def1 = mons_data[map_data[m].npc[n]->class].def1;
	def2 = mons_data[map_data[m].npc[n]->class].def2;

	if (map_data[m].npc[n]->skilldata.effect & 0x40) {
		def1 = def1 * 75 / 100;
		def2 = def2 * 75 / 100;
	}
	if (map_data[m].npc[n]->skilldata.effect & 0x80) {
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
	if (sd->status.race_def_pierce[mons_data[map_data[m].npc[n]->class].race] == 1) {
		def1 = 0;
		def2 = 0;
	}
	if ((critical % 10) == 2)
		damage += wpn_atk;

	else {
		if (sd->skill_timeamount[114-1][0] > 0 || critical > 0) {
			damage += wpn_atk;
		}
		else {
			if (arrow_id) {
				damage += MD(sd->status.str + sd->status.str2, wpn_atk);
			}
			else {
				damage += MD(sd->status.dex + sd->status.dex2, wpn_atk);
			}
		}
		if (sd->skill_timeamount[112-1][0] == 0) {
			if ((sd->status.option_z == 32) && ((sd->status.weapon == 4) || (sd->status.weapon == 5))) {
				damage = damage * peco_size_mod[mons_data[map_data[m].npc[n]->class].scale] / 100;
			}
			else {
				damage = damage * size_mod[sd->status.weapon][mons_data[map_data[m].npc[n]->class].scale] / 100;
			}
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

	if (sd->status.skill[23-1].lv > 0 && (mons_data[map_data[m].npc[n]->class].race == 1 || mons_data[map_data[m].npc[n]->class].race == 6))
		damage += sd->status.skill[23-1].lv * 3;

	if (sd->status.skill[126-1].lv > 0 && (mons_data[map_data[m].npc[n]->class].race == 2 || mons_data[map_data[m].npc[n]->class].race == 4))
		damage += sd->status.skill[126-1].lv * 4;

	damage += atk2;

	if (atk2b > 0)
		damage += MD(1, atk2b);

	if (damage < 0)
		damage = 0;

	if (sd->status.weapon == 4 || sd->status.weapon == 5) {
		s_type = (sd->status.option_z&0x20) ? 5 : 4;
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
		damage = (int)(damage * get_ele_attack_factor(atk_ele, mons_data[map_data[m].npc[n]->class].ele));

	else if ((critical % 10) == 1) {
		if (get_ele_attack_factor(atk_ele, mons_data[map_data[m].npc[n]->class].ele) > 1)
			damage = (int)(damage * get_ele_attack_factor(atk_ele, mons_data[map_data[m].npc[n]->class].ele));
	}
	damage += (damage * sd->status.size_atk_mod[mons_data[map_data[m].npc[n]->class].scale]) / 100;
	damage += (damage * sd->status.race_atk_mod[mons_data[map_data[m].npc[n]->class].race]) / 100;
	damage += (damage * sd->status.ele_atk_mod[(mons_data[map_data[m].npc[n]->class].ele % 10)]) / 100;
	damage += (critical - 1) / 10;

	if (sd->skill_timeamount[78-1][0] > 0)
		damage *= 2;

	if (damage < 0)
		damage = 1;

	return damage;
}

int mmo_map_once_attack(int fd, int target_id, unsigned long tick)
{
	int m, n;
	int critical;
	int damage, hit;
	int damage2 = 0;
	int range = 0;
	int double_luck;
	int triple_luck;
	static int double_attack[10] = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
	static int double_attack_damage[10] = {20, 40, 60, 80, 100, 120, 140, 160, 180, 200};
	static int triple_attack[10] = {29, 28, 27, 26, 25, 24, 23, 22, 21, 20};
	int i = 0, j = 0;
	int found = 0;
	short index = 0;
	int arrow_id = 0;
	int dagger2_id = 0;
	int dagger2_ele = 0;
	int dagger2_atk1 = 0;
	int dagger2_atk2 = 0;
	int dagger2_atk2b = 0;
	int dagger2_base = 0;
	int dagger2_forged = 0;
	unsigned char buf[256];
	struct map_session_data *sd;

	if (session[fd] && (sd = session[fd]->session_data)) {
		m = sd->mapno;
		n = mmo_map_search_monster(m, target_id);
		if (n < 0) {
			mmo_map_pvp_attack(fd, target_id, tick);
			return 0;
		}
		if (map_data[m].npc[n]->hidden)
			return 0;

		if (sd->status.hp <= 0) {
			delete_timer(sd->attacktimer, mmo_map_attack_continue);
			sd->attacktimer = 0;
			sd->attacktarget = 0;
			return 0;
		}
		if (sd->status.skill[263-1].lv > 0) {
			triple_luck = rand() % 100;
			if (triple_luck <= triple_attack[sd->status.skill[263-1].lv-1]) {
				skill_do_delayed_target(-1, tick, fd, 263);
				delete_timer(sd->attacktimer, mmo_map_attack_continue);
				sd->attacktimer = 0;
				sd->attacktarget = 0;
				return 0;
			}
		}
		range = sd->status.range;
		if (sd->status.weapon == 11)
			range = sd->status.range + sd->status.skill[44-1].lv;

		if (sd->skill_timeamount[61-1][0] && calc_dir3(sd->x, sd->y, map_data[m].npc[n]->x, map_data[m].npc[n]->y) != -1)
			sd->dir = sd->head_dir = calc_dir3(sd->x, sd->y, map_data[m].npc[n]->x, map_data[m].npc[n]->y);

		mmo_map_set007b(sd, buf);
		mmo_map_sendarea(fd, buf, packet_len_table[0x7b], 1);

		if (sd->status.weapon == 11) {
			for (j = 1770; j >= 1750; j--) {
				if (found == 1)
					break;

				for (i = 0; i < MAX_INVENTORY; i++) {
					if (sd->status.inventory[i].nameid == j) {
						found = 1;
						index = i + 2;
						arrow_id = j;
						break;
					}
				}
			}
			if (found == 1) {
				WFIFOW(fd, 0) = 0x13c;
				WFIFOW(fd, 2) = j;
				WFIFOSET(fd, packet_len_table[0x13c]);

				WFIFOW(fd, 0) = 0x13c;
				WFIFOW(fd, 2) = index - 2;
				WFIFOSET(fd, packet_len_table[0x13c]);

				mmo_map_lose_item(fd, 0, index, 1);
			}
			else {
				WFIFOW(fd, 0) = 0x13b;
				WFIFOW(fd, 2) = 0;
				WFIFOSET(fd, packet_len_table[0x13b]);
				return 0;
			}
		}
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
		hit = (sd->status.hit - mons_data[map_data[m].npc[n]->class].flee) * 5 + 55;
		if (map_data[m].npc[n]->skilldata.effect & 0x04)
			hit += 25;

		if (sd->skill_timeamount[61-1][0] > 0)
			hit += 20;

		if (sd->status.skill[107-1].lv > 0)
			hit += sd->status.skill[107-1].lv * 2;

		if (hit < 50)
			hit = 60;

		if (sd->status.weapon == 16 || map_data[m].npc[n]->skilldata.effect & 0x20 || sd->skill_timeamount[61-1][0] > 0)
			critical = sd->status.critical * 2;

		else
			critical = sd->status.critical;

		if (rand() % 100 >= critical) {
			if (rand() % 100 >= hit) {
				damage = -1;
				WBUFW(buf, 0) = 0x8a;
				WBUFL(buf, 2) = sd->account_id;
				WBUFL(buf, 6) = target_id;
				WBUFL(buf, 10) = tick;
				WBUFL(buf, 14) = sd->status.aspd;
				WBUFL(buf, 18) = map_data[m].npc[n]->u.mons.attackdelay;
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
				WBUFL(buf, 18) = map_data[m].npc[n]->u.mons.attackdelay;
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
						if (map_data[m].npc[n]->skilldata.skill_timer[52-1][0] > 0) {
							delete_timer(map_data[m].npc[n]->skilldata.skill_timer[52-1][0], skill_reset_monster_stats);
							map_data[m].npc[n]->skilldata.skill_timer[52-1][0] = -1;
						}
						WBUFW(buf, 0) = 0x119;
						WBUFL(buf, 2) = map_data[m].npc[n]->id;
						WBUFW(buf, 6) = 0;
						WBUFW(buf, 8) = 1;
						WBUFW(buf, 10) = 0;
						WBUFB(buf, 12) = 0;
						mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
						map_data[m].npc[n]->skilldata.skill_num = 52;
						map_data[m].npc[n]->option = 0|1|0;
						map_data[m].npc[n]->skilldata.effect |= 0x40;
						map_data[m].npc[n]->skilldata.skill_timer[52-1][0] = add_timer((unsigned int)(gettick() + 30000), skill_reset_monster_stats, m, n);
					}
				}
				if (map_data[m].npc[n]->skilldata.effect & 0x80) {
					map_data[m].npc[n]->skilldata.skill_num = 15;
					map_data[m].npc[n]->skilldata.fd = fd;
					skill_reset_monster_stats(0, 0, m, n);
				}
				else if (map_data[m].npc[n]->skilldata.effect & 0x20) {
					WBUFW(buf, 0) = 0x119;
					WBUFL(buf, 2) = map_data[m].npc[n]->id;
					WBUFW(buf, 6) = 0;
					WBUFW(buf, 8) = 0;
					WBUFW(buf, 10) = 0;
					WBUFB(buf, 12) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
					map_data[m].npc[n]->skilldata.effect = 00000000;
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
			WBUFL(buf, 18) = map_data[m].npc[n]->u.mons.attackdelay;
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
					if (map_data[m].npc[n]->skilldata.skill_timer[52-1][0] > 0) {
						delete_timer(map_data[m].npc[n]->skilldata.skill_timer[52-1][0], skill_reset_monster_stats);
						map_data[m].npc[n]->skilldata.skill_timer[52-1][0] = -1;
					}
					WBUFW(buf, 0) = 0x119;
					WBUFL(buf, 2) = map_data[m].npc[n]->id;
					WBUFW(buf, 6) = 0;
					WBUFW(buf, 8) = 1;
					WBUFW(buf, 10) = 0;
					WBUFB(buf, 12) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
					map_data[m].npc[n]->skilldata.skill_num = 52;
					map_data[m].npc[n]->option = 0|1|0;
					map_data[m].npc[n]->skilldata.effect |= 0x40;
					map_data[m].npc[n]->skilldata.skill_timer[52-1][0] = add_timer((unsigned int)(gettick() + 30000), skill_reset_monster_stats, m, n);
				}
			}
			if (map_data[m].npc[n]->skilldata.effect & 0x80) {
				map_data[m].npc[n]->skilldata.skill_num = 15;
				map_data[m].npc[n]->skilldata.fd = fd;
				skill_reset_monster_stats(0, 0, m, n);
			}
			else if (map_data[m].npc[n]->skilldata.effect & 0x20) {
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = map_data[m].npc[n]->id;
				WBUFW(buf, 6) = 0;
				WBUFW(buf, 8) = 0;
				WBUFW(buf, 10) = 0;
				WBUFB(buf, 12) = 0;
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
				map_data[m].npc[n]->skilldata.effect = 00000000;
			}
		}
		for (i = 0; i < 10; i++) {
			if (sd->status.status_atk_mod[i] > 0) {
				if (rand() % 10000 <= sd->status.status_atk_mod[i])
					break;
			}
		}
		if (damage > 0) {
			sd->status.damage_atk += (damage + damage2);
			attack_monster(fd, m, n, target_id, damage + damage2);
		}
	}
	return 0;
}

int mmo_map_pvp_attack(int fd, int target_id, unsigned long tick)
{
	int range = 0;
	int damage, hit, k = 0;
	int damage2 = 0;
	int target_fd = 0;
	int critical;
	char s_lv = 0, s_type = 0;
	int double_luck;
	int i = 0, j = 0;
	int found = 0;
	short index = 0;
	static int double_attack[10] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50 };
	static int double_attack_damage[10] = { 20, 40, 60, 80, 100, 120, 140, 160, 180, 200 };
	unsigned char buf[256];
	struct map_session_data *sd, *target_sd = NULL;

	if (session[fd]&& (sd = session[fd]->session_data)) {
		if (mmo_map_flagload(sd->mapname, PVP) || PVP_flag) {
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

			sd->attacktarget = target_id;
			range = sd->status.range;
			if (sd->status.hp <= 0) {
				if (sd->attacktimer > 0) {
					delete_timer(sd->attacktimer, mmo_map_attack_continue);
					sd->attacktimer = 0;
				}
				sd->attacktarget = 0;
				return 0;
			}
			if (target_sd->status.hp <= 0) {
				if (sd->attacktimer > 0) {
					delete_timer(sd->attacktimer, mmo_map_attack_continue);
					sd->attacktimer = 0;
				}
				sd->attacktarget = 0;
				return 0;
			}
			if (target_sd->hidden == 1) {
				if (sd->attacktimer > 0) {
					delete_timer(sd->attacktimer, mmo_map_attack_continue);
					sd->attacktimer = 0;
				}
				sd->attacktarget = 0;
				return 0;
			}
			if (sd->status.weapon == 11)
				range = sd->status.range + sd->status.skill[44-1].lv;

			mmo_map_set007b(sd, buf);
			mmo_map_sendarea(fd, buf, packet_len_table[0x7b], 1);

			if (sd->status.weapon == 11) {
				for (i = 0; i < MAX_INVENTORY; i++) {
					if (found == 1)
						break;

					for (j = 1750; j <= 1760; j++) {
						if (sd->status.inventory[i].nameid == j) {
							found = 1;
							index = i + 2;
							break;
						}
					}
				}
				if (found) {
					WFIFOW(fd, 0) = 0x13c;
					WFIFOW(fd, 2) = j;
					WFIFOSET(fd, packet_len_table[0x13c]);

					WFIFOW(fd, 0) = 0x13c;
					WFIFOW(fd, 2) = index - 2;
					WFIFOSET(fd, packet_len_table[0x13c]);

					mmo_map_lose_item(fd, 0, index, 1);
				}
				else {
					WFIFOW(fd, 0) = 0x13c;
					WFIFOW(fd, 2) = 0;
					WFIFOSET(fd, packet_len_table[0x13c]);
					return 0;
				}
			}
			hit = (sd->status.hit - target_sd->status.flee1) * 5;
			if (hit <= 0) {
				hit = 20;
			}
			else if (hit >= 100) {
				hit = 95;
			}
			if (sd->status.weapon == 16) {
				critical = sd->status.critical * 2;
			}
			else {
				critical = sd->status.critical;
			}
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
					else {
						s_lv = s_type = 0;
					}
					if (sd->status.weapon == 11) {
						damage = BOW_ATTACK(sd->status.atk1, sd->status.atk2, sd->status.dex, target_sd->status.def1);
					}
					else {
						damage = NOM_ATTACK(sd->status.atk1, sd->status.atk2, target_sd->status.def1);
					}
					if (damage <= 0) {
						damage = 1;
					}
					damage += s_lv * s_type;
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

					if (target_sd->status.effect & 0x80) {
						skill_reset_stats(0, 0, target_fd, 15);
					}
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
				else {
					s_lv = s_type = 0;
				}
				damage = CRI_ATTACK(sd->status.atk1, sd->status.atk2, s_lv, s_type);
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

				if (target_sd->status.effect & 0x80) {
					skill_reset_stats(0, 0, target_fd, 15);
				}
			}
			if (damage > 0)
				attack_player(fd, target_fd, damage + damage2);
		}
	}
	return 0;
}
