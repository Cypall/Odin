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

int mmo_map_search_monster(short m, long id)
{
	int i;

	for (i = 0; i < map_data[m].npc_num; i++)
		if (map_data[m].npc[i]->id == id && map_data[m].npc[i]->block.subtype == MONS)
			return i;


	return -1;
}

void check_new_target_monster(short m, int n, unsigned int fd)
{
	struct map_session_data *sd;
	struct npc_data *monster;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP || map_data[m].npc[n]->block.subtype != MONS)
		return;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	if (!(monster = map_data[m].npc[n]))
		return;

	if (monster->u.mons.target_fd == 0) {
		if (sd->sitting == 1 || sd->hidden || sd->skill_timeamount[135-1][0] > 0 || sd->act_dead)
			return;

		if (!mons_data[monster->class].range || m != sd->mapno || monster->u.mons.hp <= 0)
			return;

		monster->u.mons.target_fd = fd;
		monster->u.mons.attacktimer = add_timer(gettick() + monster->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
		if (monster->u.mons.speed != 0 && timer_data[monster->u.mons.walktimer]->tick > gettick() + monster->u.mons.speed)
			timer_data[monster->u.mons.walktimer]->tick = gettick() + monster->u.mons.speed;

	}
	else
		if (monster->u.mons.target_fd != fd
		&& (mons_data[monster->class].aggressive == 1
		|| mons_data[monster->class].aggressive == 3
		|| mons_data[monster->class].aggressive == 4))
			if (rand() % 1 == 0)
				monster->u.mons.target_fd = fd;



}

void mmo_mons_send_death(unsigned int fd, short m, int n)
{
	unsigned char buf[64];
	struct npc_data *monster;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP || map_data[m].npc[n]->block.subtype != MONS)
		return;

	if (!(monster = map_data[m].npc[n]))
		return;

	if (monster->class > 20) {
		WBUFW(buf, 0) = 0x80;
		WBUFL(buf, 2) = monster->id;
		WBUFB(buf, 6) = 1;
		mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
	}
	else {
		WBUFW(buf, 0) = 0x80;
		WBUFL(buf, 2) = monster->id;
		WBUFB(buf, 6) = 2;
		mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
	}
	mmo_map_item_drop(m, n);
	mmo_mons_attack_no(m, n);
	monster->u.mons.option_x = 0;
	monster->u.mons.option_y = 0;
	monster->u.mons.option_z = 0;
	monster->skilldata.fd = 0;
	monster->skilldata.skill_num = 0;
	monster->skilldata.effect = 00000000;
	monster->u.mons.target_fd = 0;
	monster->u.mons.walkpath_len = 0;
	monster->u.mons.walkpath_pos = 0;
	monster->u.mons.speed = 0;
	monster->u.mons.summon = 0;
	monster->u.mons.hidden = 0;
	monster->u.mons.lootdata.id = 0;
	monster->u.mons.lootdata.loot_count = 0;
	del_block(&monster->block);
	if (map_data[m].npc[n]->name[0] != '*') {
		if (mons_data[monster->class].boss == 1) {
			add_timer(gettick() + 60 * 60000, spawn_delay, m, n);
			timer_data[monster->u.mons.walktimer]->tick = gettick() + 60 + 500;
		}
		else if (mons_data[monster->class].boss == 2) {
			add_timer(gettick() + 45 * 60000, spawn_delay, m, n);
			timer_data[monster->u.mons.walktimer]->tick = gettick() + 45 + 500;
		}
		else {
			add_timer(gettick() + 60000, spawn_delay, m, n);
			timer_data[monster->u.mons.walktimer]->tick = gettick() + 1 + 500;
		}
	}
	else {
		mmo_mons_walk_stop(m, n);
		set_monster_no_point(m, n);
	}
}

void mmo_mons_attack_no(short m, int n)
{
	struct npc_data *monster;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP || map_data[m].npc[n]->block.subtype != MONS)
		return;

	if (!(monster = map_data[m].npc[n]))
		return;

	if (monster->u.mons.attacktimer > 0) {
		delete_timer(monster->u.mons.attacktimer, mmo_mons_attack_continue);
		monster->u.mons.attacktimer = 0;
	}
	monster->u.mons.target_fd = 0;
}

void mmo_mons_walk_stop(short m, int n)
{
	struct npc_data *monster;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP || map_data[m].npc[n]->block.subtype != MONS)
		return;

	if (!(monster = map_data[m].npc[n]))
		return;

	if (monster->u.mons.walktimer > 0) {
		delete_timer(monster->u.mons.walktimer, mons_walk);
		monster->u.mons.walktimer = 0;
	}
}

int mmo_mons_attack_player(struct map_session_data *sd, int damage)
{
	int len;
	unsigned int fd = sd->fd;

	sd->status.hp -= damage;
	if (sd->status.hp <= 0)
		sd->status.hp = 0;

	len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_HP);
	if (len > 0)
		WFIFOSET(fd, len);

	if (sd->status.hp <= 0) {
		mmo_player_send_death(sd);
		if (mmo_map_flagload(sd->mapname, CANLOSE)) {
			sd->status.base_exp -= base_lose;
			if (sd->status.base_exp < 0)
				sd->status.base_exp = 0;

			sd->status.job_exp -= job_lose;
			if (sd->status.job_exp < 0)
				sd->status.job_exp = 0;

			mmo_map_set00b1(fd, SP_BASEEXP, sd->status.base_exp);
			mmo_map_set00b1(fd, SP_JOBEXP, sd->status.job_exp);
		}
		return 1;
	}
	return 0;
}

void mmo_mons_attack_continue(int tid, unsigned int tick, int m, int n)
{
	int len, i = 0, j = 0, target_ele = 0, monster_ele = 0, damage = 0, alt_damage = 0, avoid = 0;
	static int improve_dodge[10] = { 3, 6, 9, 12, 15, 18, 21, 24, 27, 30 };
	static int dirx[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	static int diry[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	int floorskill_index;
	unsigned int fd = 0;
	unsigned char buf[64];
	struct map_session_data *sd;
	struct npc_data *monster;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP || map_data[m].npc[n]->block.subtype != MONS)
		return;

	if (!(monster = map_data[m].npc[n]))
		return;

	fd = monster->u.mons.target_fd;
	if (!session[fd] || !(sd = session[fd]->session_data)) {
		mmo_mons_attack_no(m, n);
		return;
	}
	if (sd->sitting == 1 || sd->hidden || sd->skill_timeamount[135-1][0] > 0 || sd->act_dead) {
		mmo_mons_attack_no(m, n);
		return;
	}
	if (!mons_data[monster->class].range || m != sd->mapno || monster->u.mons.hp <= 0) {
		mmo_mons_attack_no(m, n);
		return;
	}
	if (mons_data[monster->class].assist == 1)
		for (j = 0; j < map_data[m].npc_num; j++)
			if (j != n && map_data[m].npc[j] && map_data[m].npc[j]->block.subtype == MONS)
				if (map_data[m].npc[j]->class == monster->class && !map_data[m].npc[j]->u.mons.target_fd
				&& map_data[m].npc[j]->u.mons.hp > 0)
					if (in_range(map_data[m].npc[j]->x, map_data[m].npc[j]->y, monster->x, monster->y, BLOCK_SIZE * 3 / 2))
						check_new_target_monster(m, j, fd);






	if ((monster->skilldata.effect & ST_SLEEP)
	|| (monster->skilldata.effect & ST_STUN)
	|| (monster->skilldata.effect & ST_STONE)
	|| (monster->skilldata.effect & ST_FROZEN)) {
		monster->u.mons.attacktimer = add_timer(tick + monster->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
		return;
	}
	if (!in_range(monster->x, monster->y, sd->x, sd->y, mons_data[monster->class].range)) {
		monster->u.mons.attacktimer = add_timer(tick + monster->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
		return;
	}
	floorskill_index = search_floorskill_index(sd->mapno, sd->x, sd->y);
	if (floorskill_index > -1) {
		switch (floorskill[floorskill_index].type) {
		case FS_PNEUMA:
			if (mons_data[monster->class].range > 1) {
				WBUFW(buf, 0) = 0x8a;
				WBUFL(buf, 2) = monster->id;
				WBUFL(buf, 6) = sd->account_id;
				WBUFL(buf, 10) = tick;
				WBUFL(buf, 14) = monster->u.mons.attackdelay;
				WBUFL(buf, 18) = sd->status.aspd;
				WBUFW(buf, 22) = 0;
				WBUFW(buf, 24) = 0;
				WBUFB(buf, 26) = 0;
				WBUFW(buf, 27) = 0;
				mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);

				monster->u.mons.attacktimer = add_timer(tick + monster->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
				return;
			}
			break;
		case FS_SWALL:
			WBUFW(buf, 0) = 0x8a;
			WBUFL(buf, 2) = monster->id;
			WBUFL(buf, 6) = sd->account_id;
			WBUFL(buf, 10) = tick;
			WBUFL(buf, 14) = monster->u.mons.attackdelay;
			WBUFL(buf, 18) = sd->status.aspd;
			WBUFW(buf, 22) = 0;
			WBUFW(buf, 24) = 0;
			WBUFB(buf, 26) = 0;
			WBUFW(buf, 27) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
			floorskill[floorskill_index].count--;
			if (floorskill[floorskill_index].count == 0) {
				if (floorskill[floorskill_index].timer > 0)
					delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

				remove_floorskill(0, tick, 0, floorskill_index);
			}
			monster->u.mons.attacktimer = add_timer(tick + monster->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
			return;
		}
	}
	if (monster->u.mons.def1 != mons_data[monster->class].def1 / 2 && monster->u.mons.def2 != monster->u.mons.def2 / 2) {
		if (sd->cignum_crusis && (mons_data[monster->class].ele % 10 == 9 || mons_data[monster->class].ele % 10 == 7)) {
			monster->u.mons.def1 /= 2;
			monster->u.mons.def2 /= 2;
		}
	}
	if (sd->skill_timeamount[8-1][0] > 0)
		monster->u.mons.attackdelay *= 1.1;

	else
		monster->u.mons.attackdelay = mons_data[monster->class].adelay;

	if ((rand() % 90) == 10)
		monster_say(fd, monster->id, monster->class, "attack");

	avoid = 10 + sd->status.flee1 - mons_data[monster->class].hit;
	if (sd->status.skill[49-1].lv > 0)
		avoid = avoid + improve_dodge[sd->status.skill[49-1].lv - 1];

	if ((monster->skilldata.effect & ST_BLIND))
		avoid += 25;

	for (j = 0; j < map_data[m].npc_num; j++)
		if (map_data[m].npc[j]->block.subtype == MONS)
			if (map_data[m].npc[j]->u.mons.target_fd == fd && map_data[m].npc[j]->u.mons.attacktimer > 0 && map_data[m].npc[j]->u.mons.hp > 0)
				avoid -= 10;



	avoid += 10;
	if (avoid >= 95)
		avoid = 95;

	if (rand() % 100 <= avoid) {
		WBUFW(buf, 0) = 0x8a;
		WBUFL(buf, 2) = monster->id;
		WBUFL(buf, 6) = sd->account_id;
		WBUFL(buf, 10) = tick;
		WBUFL(buf, 14) = monster->u.mons.attackdelay;
		WBUFL(buf, 18) = sd->status.aspd;
		WBUFW(buf, 22) = 0;
		WBUFW(buf, 24) = 0;
		WBUFB(buf, 26) = 0;
		WBUFW(buf, 27) = 0;
		mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
	}
	else {
		monster_ele = mons_data[monster->class].ele;
		if (sd->skill_timeamount[69-1][0] > 0)
		   target_ele = 6;

		if (sd->casting)
			skill_stop_wait(sd);

		if (rand() % 100 <= (1 + mons_data[monster->class].flee / 8)) {
			damage = monster->u.mons.atk1 + monster->u.mons.atk2;
			damage = (int)(damage * get_ele_attack_factor(monster_ele, target_ele));
			damage -= (damage * sd->status.size_def_mod[mons_data[monster->class].scale]) / 100;
			damage -= (damage * sd->status.race_def_mod[mons_data[monster->class].race]) / 100;
			damage -= (damage * sd->status.ele_def_mod[(mons_data[monster->class].ele % 10)]) / 100;
			if (damage < 1)
				damage = 1;

			if (sd->skill_timeamount[252-1][0] > 0) {
				if (in_range(sd->x, sd->y, monster->x, monster->y, sd->status.range)) {
					alt_damage = damage * sd->skill_timeamount[252-1][1];
					mmo_map_once_attack(fd, monster->id, tick, alt_damage);
					if (monster->u.mons.hp <= 0) {
						mmo_mons_attack_no(m, n);
						return;
					}
				}
			}
			WBUFW(buf, 0) = 0x8a;
			WBUFL(buf, 2) = monster->id;
			WBUFL(buf, 6) = sd->account_id;
			WBUFL(buf, 10) = tick;
			WBUFL(buf, 14) = monster->u.mons.attackdelay;
			WBUFL(buf, 18) = sd->status.aspd;
			WBUFW(buf, 22) = damage;
			WBUFW(buf, 24) = 0;
			WBUFB(buf, 26) = 0x0a;
			WBUFW(buf, 27) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
			mmo_player_stop_walk(sd);
		}
		else {
			if (sd->skill_timeamount[61-1][0] > 0) {
				for (i = sd->dir - 2; i <= sd->dir + 2; i++) {
					if ((dirx[((i < 0) ? i + 8 : ((i > 7) ? i - 8 : i))] == monster->x - sd->x) && (diry[((i < 0) ? i+8 : ((i > 7) ? i-8 : i))] == monster->y - sd->y)) {
						mmo_map_once_attack(fd, monster->id, tick, 0);
						if (monster->u.mons.hp <= 0)
							mmo_mons_attack_no(m, n);

						else
							monster->u.mons.attacktimer = add_timer(tick + mons_data[monster->class].adelay, mmo_mons_attack_continue, m, n);

						return;
					}
				}
			}
			if (monster->u.mons.atk1)
				damage = (rand() % monster->u.mons.atk1 + 1) + monster->u.mons.atk2;

			else
				damage = monster->u.mons.atk2;

			damage = (int)(damage * get_ele_attack_factor(monster_ele, target_ele));
			damage -= (damage * sd->status.size_def_mod[mons_data[monster->class].scale]) / 100;
			damage -= (damage * sd->status.race_def_mod[mons_data[monster->class].race]) / 100;
			damage -= (damage * sd->status.ele_def_mod[(mons_data[monster->class].ele % 10)]) / 100;

			if (sd->skill_timeamount[73-1][0] > 0 && sd->skill_timeamount[73-1][1] > 0) {
				sd->skill_timeamount[73-1][1]--;
				damage = 0;
				if (sd->skill_timeamount[73-1][1] <= 0)
					skill_reset_stats(0, 0, fd, 73);

			}
			if (sd->skill_timeamount[157-1][0] > 0) {
				if (sd->status.sp > 0) {
					i = (100 * sd->status.sp) / sd->status.max_sp;
					if ((i > 0) && (i <= 20)) {
						sd->skill_timeamount[157-1][1] = 6;
						sd->status.sp -= (0.010 * sd->status.max_sp);
					}
					else if ((i > 20) && (i <= 40)) {
						sd->skill_timeamount[157-1][1] = 12;
						sd->status.sp -= (0.015 * sd->status.max_sp);
					}
					else if ((i > 40) && (i <= 60)) {
						sd->skill_timeamount[157-1][1] = 18;
						sd->status.sp -= (0.020 * sd->status.max_sp);
					}
					else if ((i > 60) && (i <= 80)) {
						sd->skill_timeamount[157-1][1] = 24;
						sd->status.sp -= (0.025 * sd->status.max_sp);
					}
					else {
						sd->skill_timeamount[157-1][1] = 30;
						sd->status.sp -= (0.030 * sd->status.max_sp);
					}
					if (sd->status.sp < 0)
						sd->status.sp = 0;

					len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_SP);
					if (len > 0)
						WFIFOSET(fd, len);

					damage = ((damage * (100 - sd->skill_timeamount[157-1][1])) / 100);
				}
				else
					skill_reset_stats(0, 0, fd, 157);

			}
 			if (sd->skill_timeamount[249-1][0] > 0)
				damage = 0;

			if (damage > 0) {
				if (sd->status.def2 >= 95)
					damage -= (damage * 95 / 100);

				else
					damage -= (damage * sd->status.def2 / 100);

				if ((sd->status.skill[22-1].lv > 0) && (mons_data[monster->class].race == 1 || mons_data[monster->class].race == 6))
					damage -= sd->status.skill[22-1].lv * 3;

				damage -= sd->status.def1;
				damage -= MD(0, (sd->status.def1 * sd->status.def1 / 400));
				if (damage < 0)
					damage = 1;

			}
			if (sd->skill_timeamount[252-1][0] > 0) {
				if (in_range(sd->x, sd->y, monster->x, monster->y, sd->status.range)) {
					alt_damage = damage * sd->skill_timeamount[252-1][1];
					mmo_map_once_attack(fd, monster->id, tick, alt_damage);
					if (monster->u.mons.hp <= 0) {
						mmo_mons_attack_no(m, n);
						return;
					}
				}
			}
			WBUFW(buf, 0) = 0x8a;
			WBUFL(buf, 2) = monster->id;
			WBUFL(buf, 6) = sd->account_id;
			WBUFL(buf, 10) = tick;
			WBUFL(buf, 14) = monster->u.mons.attackdelay;
			WBUFL(buf, 18) = sd->status.aspd;
			WBUFW(buf, 22) = damage;
			WBUFW(buf, 24) = 1;
			WBUFB(buf, 26) = 0;
			WBUFW(buf, 27) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
			mmo_player_stop_walk(sd);
		}
	}
	if (damage > 0) {
		if (mmo_mons_attack_player(sd, damage)) {
			mmo_mons_attack_no(m, n);
			return;
		}
		monster->u.mons.attacktimer = add_timer(tick + monster->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
		return;
	}
	else {
		monster->u.mons.attacktimer = add_timer(tick + monster->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
		return;
	}
}

int mmo_map_mvp_do(unsigned int fd, short m, int n)
{
	int real_luc;
	unsigned char buf[64];
	struct map_session_data *sd;
	struct npc_data *monster;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP || map_data[m].npc[n]->block.subtype != MONS)
		return 0;

	if (!(monster = map_data[m].npc[n]))
		return 0;

	if (session[fd] && (sd = session[fd]->session_data)) {
		if (!boss_monster(m, n))
			return 0;

		else {
			WBUFW(buf, 0) = 0x10c;
			WBUFL(buf, 2) = sd->account_id;
			mmo_map_sendarea(fd, buf, packet_len_table[0x10c], 0);
			real_luc = rand() % 100;
			switch (monster->class) {
			case 1038: // OSIRIS
				if (real_luc < 40) {
					if (real_luc > 20)
						mmo_map_set010a(fd, 714);

					else if (real_luc >= 10)
						mmo_map_set010a(fd, 2614);

					else if (real_luc >= 0)
						mmo_map_set010a(fd, 2235);

					break;
				}
				else
					mmo_map_set010b(15420, sd);

				break;
			case 1039: // BAPHOMET
				if (real_luc < 40) {
					if (real_luc > 20)
						mmo_map_set010a(fd, 714);

					else if (real_luc >= 10)
						mmo_map_set010a(fd, 1413);

					else if (real_luc >= 0)
						mmo_map_set010a(fd, 2256);

					break;
				}
				else
					mmo_map_set010b(26000, sd);
		
				break;
			case 1046: // DOPPLEGANGER
				if (real_luc < 40) {
					if (real_luc > 25)
						mmo_map_set010a(fd, 985);

					else if (real_luc > 10)
						mmo_map_set010a(fd, 984);

					else if (real_luc > 0)
						mmo_map_set010a(fd, 2615);

					break;
				}
				else
					mmo_map_set010b(10680, sd);

				break;
			case 1059: // MISTRESS
				if (real_luc < 40) {
					if (real_luc > 25)
						mmo_map_set010a(fd, 985);

					else if (real_luc > 10)
						mmo_map_set010a(fd, 984);

					else if (real_luc > 0)
						mmo_map_set010a(fd, 2249);

					break;
				}
				else
					mmo_map_set010b(1284, sd);

				break;
			case 1086: // GOLD THIEF BUG
				if (real_luc < 40) {
					if (real_luc > 15)
						mmo_map_set010a(fd, 985);

					else if (real_luc > 0)
						mmo_map_set010a(fd, 2246);

					break;
				}
				else
					mmo_map_set010b(25, sd);
	
				break;
			case 1087: // ORC HERO
				if (real_luc < 40) {
					if (real_luc > 25)
						mmo_map_set010a(fd, 985);

					else if (real_luc > 5)
						mmo_map_set010a(fd, 1363);

					else if (real_luc > 0)
						mmo_map_set010a(fd, 1366);

					break;
				}
				else
					mmo_map_set010b(4300 + rand() % 200, sd);

				break;
			case 1090: // ORC LORD
				if (real_luc < 40) {
					mmo_map_set010a(fd, 2607);
					break;
				}
				else
					mmo_map_set010b(25600 + rand()%200, sd);

				break;
			case 1112: // DRAKEE
				if (real_luc < 40) {
					if (real_luc > 30)
						mmo_map_set010a(fd, 5019);
					else if (real_luc >= 0)
						mmo_map_set010a(fd, 984);
					break;
				}
				else
					mmo_map_set010b(8600, sd);

				break;
			case 1115: // EDDGA
				if (real_luc < 40) {
					if (real_luc > 20)
						mmo_map_set010a(fd, 985);

					else if (real_luc >= 0)
						mmo_map_set010a(fd, 984);

					break;
				}
				else
					mmo_map_set010b(3400, sd);

				break;
			case 1150: // MOONLIGHT
				if (real_luc < 40) {
					if (real_luc > 20)
						mmo_map_set010a(fd, 714);

					else if (real_luc >= 10)
						mmo_map_set010a(fd, 984);

					else if (real_luc >= 0)
						mmo_map_set010a(fd, 1128);

					break;
				}
				else
					mmo_map_set010b(4000, sd);

				break;
			case 1159: // PHEERONI
				if (real_luc < 40) {
					if (real_luc > 25)
						mmo_map_set010a(fd, 985);

					else if (real_luc > 10)
						mmo_map_set010a(fd, 984);

					else if (real_luc > 0)
						mmo_map_set010a(fd, 2288);

					break;
				}
				else
					mmo_map_set010b(3400, sd);

				break;
			case 1147: // MAYA
				if (real_luc < 40) {
					mmo_map_set010a(fd, 2234);
					break;
				}
				else
					mmo_map_set010b(11550, sd);

				break;
			}
		}
	}
	return 0;
}
