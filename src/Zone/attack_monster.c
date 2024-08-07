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

int mmo_map_search_monster(int m, int id)
{
	int i;

	for (i = 0; i < map_data[m].npc_num; i++) {
		if (map_data[m].npc[i]->id == id) {
			if (map_data[m].npc[i]->block.subtype != MONS)
				return -1;

			return i;
		}
	}
	return -1;
}

void check_new_target_monster(short m, short n, unsigned int fd)
{
	struct map_session_data *sd;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP ||
	    !map_data[m].npc[n] || map_data[m].npc[n]->block.subtype != MONS)
		return;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	if (!map_data[m].npc[n]->u.mons.target_fd) {
		if (!mons_data[map_data[m].npc[n]->class].range || m != sd->mapno
		|| map_data[m].npc[n]->u.mons.hp <= 0 || sd->status.hp <= 0
		|| sd->hidden || sd->skill_timeamount[143-1][1])
			return;

		map_data[m].npc[n]->u.mons.target_fd = fd;
		map_data[m].npc[n]->u.mons.attacktimer = add_timer(gettick() + map_data[m].npc[n]->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
		if (mons_data[map_data[m].npc[n]->class].speed != 0 && timer_data[map_data[m].npc[n]->u.mons.timer]->tick > gettick() + map_data[m].npc[n]->u.mons.speed)
			timer_data[map_data[m].npc[n]->u.mons.timer]->tick = gettick() + map_data[m].npc[n]->u.mons.speed;

	}
	else if ((unsigned)map_data[m].npc[n]->u.mons.target_fd != fd && mons_data[map_data[m].npc[n]->class].aggressive == 1) {
		if (rand() % 10 == 0)
			map_data[m].npc[n]->u.mons.target_fd = fd;
	}
}

void mmo_mons_attack_no(short m, int n)
{
	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP ||
	    !map_data[m].npc[n] || map_data[m].npc[n]->block.subtype != MONS)
		return;

	delete_timer(map_data[m].npc[n]->u.mons.attacktimer, mmo_mons_attack_continue);
	map_data[m].npc[n]->u.mons.attacktimer = 0;
	map_data[m].npc[n]->u.mons.target_fd = 0;
}

int mmo_mons_attack_continue(int tid, unsigned int tick, int m, int n)
{
	unsigned int fd;
	int damage = 0, avoid;
	int len;
	int i, j;
	int target_ele = 0;
	int monster_ele;
	unsigned char buf[256];
	static int dirx[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	static int diry[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	static int improve_dodge[10] = { 3, 6, 9, 12, 15, 18, 21, 24, 27, 30 };
	struct map_session_data *sd;
	tid = 0;

	if (m < 0 || m >= MAX_MAP_PER_SERVER || n < 0 || n >= MAX_NPC_PER_MAP)
		return 0;

	if (!map_data[m].npc[n] || map_data[m].npc[n]->block.subtype != MONS)
		return 0;

	fd = map_data[m].npc[n]->u.mons.target_fd;

	if (fd < 5 || !session[fd] || !(sd = session[fd]->session_data)) {
		mmo_mons_attack_no(m, n);
		return 0;
	}
	if (map_data[m].npc[n]->hidden) {
		mmo_mons_attack_no(m, n);
		return 0;
	}
	if (!mons_data[map_data[m].npc[n]->class].def1 / 2 == map_data[m].npc[n]->u.mons.def1
	&& !map_data[m].npc[n]->u.mons.def2 / 2 == map_data[m].npc[n]->u.mons.def2) {
		if (sd->cignum_crusis && (mons_data[map_data[m].npc[n]->class].ele % 10 == 8 || mons_data[map_data[m].npc[n]->class].ele % 10 == 9)) {
			map_data[m].npc[n]->u.mons.def1 /= 2;
			map_data[m].npc[n]->u.mons.def2 /= 2;
		}
	}
	if (sd->endure)
		map_data[m].npc[n]->u.mons.attackdelay *= 1.1;
	else
		map_data[m].npc[n]->u.mons.attackdelay = mons_data[map_data[m].npc[n]->class].adelay;

	if (!mons_data[map_data[m].npc[n]->class].range || m != sd->mapno
	|| map_data[m].npc[n]->u.mons.hp <= 0 || sd->status.hp <= 0
	|| sd->hidden || sd->skill_timeamount[143-1][0] > 0) {
		mmo_mons_attack_no(m, n);
		return 0;
	}
	if (mons_data[map_data[m].npc[n]->class].assist == 1) {
		for (j = 0; j < MAX_NPC_PER_MAP; j++) {
			if (j != n && map_data[m].npc[j] && map_data[m].npc[j]->block.subtype == MONS) {
				if (map_data[m].npc[j]->class == map_data[m].npc[n]->class && !map_data[m].npc[j]->u.mons.target_fd
				&& map_data[m].npc[j]->u.mons.hp > 0) {
					if (in_range(map_data[m].npc[j]->x, map_data[m].npc[j]->y, map_data[m].npc[n]->x, map_data[m].npc[n]->y, BLOCK_SIZE * 3 / 2))
						check_new_target_monster(m, j, fd);

				}
			}
		}
	}
	if (map_data[m].npc[n]->skilldata.effect & 0xdf
	|| map_data[m].npc[n]->skilldata.effect & 0x80
	|| map_data[m].npc[n]->skilldata.effect & 0x20
	|| map_data[m].npc[n]->skilldata.effect & 0x0f) {
		map_data[m].npc[n]->u.mons.attacktimer = add_timer(tick + map_data[m].npc[n]->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
		return 0;
	}
	if (!in_range(map_data[m].npc[n]->x, map_data[m].npc[n]->y, sd->x, sd->y, mons_data[map_data[m].npc[n]->class].range)) {
		map_data[m].npc[n]->u.mons.attacktimer = add_timer(tick + map_data[m].npc[n]->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
		return 0;
	}
	if ((rand() % 80) == 10)
		monster_say(fd, map_data[m].npc[n]->id, map_data[m].npc[n]->class, "attack");

	avoid = 10 + sd->status.flee1 - mons_data[map_data[m].npc[n]->class].hit;

	if (sd->status.skill[49-1].lv > 0)
		avoid = avoid + improve_dodge[sd->status.skill[49-1].lv - 1];

	if (map_data[m].npc[n]->skilldata.effect & 0x04)
		avoid += 25;

	if (avoid >= 95)
		avoid = 95;

	if (rand() % 100 <= avoid) {
		WBUFW(buf, 0) = 0x8a;
		WBUFL(buf, 2) = map_data[m].npc[n]->id;
		WBUFL(buf, 6) = sd->account_id;
		WBUFL(buf, 10) = tick;
		WBUFL(buf, 14) = map_data[m].npc[n]->u.mons.attackdelay;
		WBUFL(buf, 18) = sd->status.aspd;
		WBUFW(buf, 22) = 0;
		WBUFW(buf, 24) = 0;
		WBUFB(buf, 26) = 0;
		WBUFW(buf, 27) = 0;
		mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
	}
	else {
		target_ele = 0;
		monster_ele = mons_data[map_data[m].npc[n]->class].ele;
		/* if (Benedictio Sanctissimi Sacramenti (Priest) enabled) target_ele = 6; Holy*/
		if (sd->casting)
			skill_stop_wait(fd);

		if (rand() % 100 <= (1 + mons_data[map_data[m].npc[n]->class].flee / 8)) {
			damage = map_data[m].npc[n]->u.mons.atk1 + map_data[m].npc[n]->u.mons.atk2;
			damage = (int)(damage * get_ele_attack_factor(monster_ele, target_ele));
			damage -= (damage * sd->status.size_def_mod[mons_data[map_data[m].npc[n]->class].scale]) / 100;
			damage -= (damage * sd->status.race_def_mod[mons_data[map_data[m].npc[n]->class].race]) / 100;
			damage -= (damage * sd->status.ele_def_mod[(mons_data[map_data[m].npc[n]->class].ele % 10)]) / 100;

			if (damage < 1)
				damage = 1;

			WBUFW(buf, 0) = 0x8a;
			WBUFL(buf, 2) = map_data[m].npc[n]->id;
			WBUFL(buf, 6) = sd->account_id;
			WBUFL(buf, 10) = tick;
			WBUFL(buf, 14) = map_data[m].npc[n]->u.mons.attackdelay;
			WBUFL(buf, 18) = sd->status.aspd;
			WBUFW(buf, 22) = damage;
			WBUFW(buf, 24) = 0;
			WBUFB(buf, 26) = 0x0a;
			WBUFW(buf, 27) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
			if (sd->walktimer > 0) {
				delete_timer(sd->walktimer, walk_char);
				sd->walktimer = 0;
			}
		}
		else {
			if (sd->skill_timeamount[61-1][0] > 0 && sd->status.skill[61-1].lv > 0) {
				for (i = sd->dir - 2; i <= sd->dir + 2; i++) {
					if ((dirx[((i < 0) ? i + 8 : ((i > 7) ? i - 8 : i))] == map_data[m].npc[n]->x - sd->x) && (diry[((i < 0) ? i+8 : ((i > 7) ? i-8 : i))] == map_data[m].npc[n]->y - sd->y)) {
						mmo_map_once_attack(fd, map_data[m].npc[n]->id, tick);
						if (map_data[m].npc[n]->u.mons.hp <= 0)
							mmo_mons_attack_no(m, n);

						else
							map_data[m].npc[n]->u.mons.attacktimer = add_timer(tick + map_data[m].npc[n]->u.mons.attackdelay, mmo_mons_attack_continue, m, n); // TIMER_ONCE_AUTODEL with pointer

						return 0;
					}
				}
			}
			if (map_data[m].npc[n]->u.mons.atk1)
				damage = (rand() % map_data[m].npc[n]->u.mons.atk1 + 1) + map_data[m].npc[n]->u.mons.atk2;

			else
				damage = map_data[m].npc[n]->u.mons.atk2;

			damage = (int)(damage * get_ele_attack_factor(monster_ele, target_ele));
			damage -= (damage * sd->status.size_def_mod[mons_data[map_data[m].npc[n]->class].scale]) / 100;
			damage -= (damage * sd->status.race_def_mod[mons_data[map_data[m].npc[n]->class].race]) / 100;
			damage -= (damage * sd->status.ele_def_mod[(mons_data[map_data[m].npc[n]->class].ele % 10)]) / 100;

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

					WFIFOW(fd, 0) = 0xb0;
					WFIFOW(fd, 2) = 0x07;
					WFIFOL(fd, 4) = sd->status.sp;
					WFIFOSET(fd, packet_len_table[0xb0]);
					damage = ((damage * (100 - sd->skill_timeamount[157-1][1])) / 100);
				}
				else
					skill_reset_stats(0, 0, fd, 157);

			}
			if (damage > 0) {
				if (sd->status.def2 >= 95)
					damage -= (damage * 95 / 100);

				else
					damage -= (damage * sd->status.def2 / 100);

				if ((sd->status.skill[22-1].lv > 0) && (mons_data[map_data[m].npc[n]->class].race == 1 || mons_data[map_data[m].npc[n]->class].race == 6))
					damage -= sd->status.skill[22-1].lv * 3;

				damage -= sd->status.def1;
				damage -= MD(0, (sd->status.def1 * sd->status.def1 / 400));
				if (damage < 0)
					damage = 1;

			}
			WBUFW(buf, 0) = 0x8a;
			WBUFL(buf, 2) = map_data[m].npc[n]->id;
			WBUFL(buf, 6) = sd->account_id;
			WBUFL(buf, 10) = tick;
			WBUFL(buf, 14) = map_data[m].npc[n]->u.mons.attackdelay;
			WBUFL(buf, 18) = sd->status.aspd;
			WBUFW(buf, 22) = damage;
			WBUFW(buf, 24) = 1;
			WBUFB(buf, 26) = 0;
			WBUFW(buf, 27) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
			if (sd->walktimer > 0) {
				delete_timer(sd->walktimer, walk_char);
				sd->walktimer = 0;
			}
		}
		sd->status.hp -= damage;
		if (sd->status.hp <= 0) {
			sd->status.hp = 0;
			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_HP);
			if (len > 0)
				WFIFOSET(fd, len);

			sd->sitting = 1;
			WBUFW(buf, 0) = 0x80;
			WBUFL(buf, 2) = sd->account_id;
			WBUFB(buf, 6) = 1;
			mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);

			mmo_mons_attack_no(m, n);

			if (sd->walktimer > 0) {
				delete_timer(sd->walktimer, walk_char);
				sd->walktimer = 0;
			}
			return 0;
		}
		len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_HP);
		if (len > 0)
			WFIFOSET(fd, len);

	}
	map_data[m].npc[n]->u.mons.attacktimer = add_timer(tick + map_data[m].npc[n]->u.mons.attackdelay, mmo_mons_attack_continue, m, n);
	return 0;
}

int mmo_map_mvp_do(unsigned int fd, short m, int n)
{
	int real_luc;
	unsigned char buf[256];
	struct map_session_data *sd;

	if (!map[m][0])
		return 0;

	if (n < 1)
		return 0;

	if (session[fd] && (sd = session[fd]->session_data)) {
		if (!boss_monster(m, n))
			return 0;

		else {
			WBUFW(buf, 0) = 0x10c;
			WBUFL(buf, 2) = sd->account_id;
			mmo_map_sendarea(fd, buf, packet_len_table[0x10c], 0);
			real_luc = rand() % 100;
			switch (map_data[m].npc[n]->class) {
				case 1038: // Osiris
					if(real_luc < 40) {
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
				case 1039: // Baphomet
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
				case 1046: // Doppleganger
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
				case 1059: // Mistress
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
				case 1086: // Gold Thief Bug
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
				case 1087: // Orc Hero
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
				case 1090: // Ord Lord
					if (real_luc < 40) {
						mmo_map_set010a(fd, 2607);
						break;
					}
					else
					    mmo_map_set010b(25600 + rand()%200, sd);

					break;
				case 1112: // Drakee
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
				case 1115: // Eddga
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
				case 1150: // Moonlight
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
				case 1159: // Pheeroni
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
				case 1147: // Maya
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
