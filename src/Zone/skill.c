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
 Creation Date: December 6, 2003
 Modified Date: October 31, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#include <math.h>

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "item.h"
#include "itemdb.h"
#include "element_damage.h"
#include "npc.h"
#include "party.h"
#include "chat.h"
#include "skill.h"
#include "skill_db.h"

#define NOM_ATTACK(atk1, atk2, def) ((atk1 + rand() % (atk2)) - (def));
#define CRI_ATTACK(atk1, atk2, s_lv, s_type) (atk1 + atk2 + (s_lv) * (s_type));
#define KAT_ATTACK(damage)  (((damage) / 5) + 1);
#define SKILL_MATTACK(matk1, matk2) ((matk1 + rand() % (matk2)) * skill_db[skill_num].type_num);
#define SKILL_HEAL(int_, blvl, lvl) (((blvl + int_) / 8) * (12 * (lvl))); // ([BLv + Int] / 8) * (12 * (SLv))
#define SKILL_ATTACK(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (100 + (30 * (lvl))) / 100) - 0); // skilllvlbonus = 100% 130 160 je nach skill lvl
#define SKILL_MAGNUM(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (100 + (15 * (lvl))) / 100) - 0); // skilllvlbonus = 100% 130 160 je nach skill lvl
#define SKILL_ENVENOM(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (100 + (15 * (lvl))) / 100) - 0); // skilllvlbonus = 100% 130 160 je nach skill lvl
#define SKILL_BOWLING(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (100 + (50 * (lvl))) / 100) - 0);
#define SKILL_NAPALM(matk1, matk2, lvl) ((((matk2) * (70 + (10 * (lvl))) / 100) + rand() % ((matk1) * (70 + (10 * (lvl))) / 100)) * skill_db[skill_num].type_num);
#define SKILL_FIREBALL(matk1, matk2, lvl) ((95 * (matk1 + rand() % (matk2)) / 100) + (5 * (skill_lvl) / 100));
#define SKILL_SONICBLOW(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (300 + (50 * (lvl))) / 100) - 0);
#define SKILL_BRANDISH(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (100 + (20 * (lvl))) / 100) - 0);
#define SKILL_PIERCE(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (100 + (10 * (lvl))) / 100) - 0);
#define SKILL_TRASHER(matk1, matk2, lvl) ((((matk2) * (100 + (20 * (lvl))) / 100) + rand() % ((matk1) * (100 + (20 * (lvl))) / 100)) * skill_db[skill_num].type_num);
#define SKILL_FROSTDIVER(matk1) ((1 + rand() % (matk1)) + (100 + (10 * (skill_lvl))));
#define SKILL_FIREPILLAR(matk1, lvl) ((lvl) * (50 + ((matk1) / 5)));
#define SKILL_BLITZBEAT(dex, int_, crow_lvl, skill_lvl) ((((dex) / 10 + (int_) / 2) * 2 + 80 + 6 * (crow_lvl))* (skill_lvl));
#define SKILL_MIN_WAIT 100

extern int users_global;
extern int PVP_flag;

/*======================================
 *	SKILL: Skills Configuration
 *--------------------------------------
 */

struct skillwaittime
{
	int wait;
	int duration;
}skillwaitdata;

struct skilldelay
{
	int delay;
} skilldelaydata;

struct skillwaittime skill_calc_wait(int fd, int skill_num, int skill_lvl)
{
	int wait = 100;
	int duration = 0;
	struct map_session_data *sd = session[fd]->session_data;

	switch (skill_num) {
		case 8: /* ENDURE */
			switch (skill_lvl) {
				case 1:
					duration = 10000;
					break;
				case 2:
					duration = 13000;
					break;
				case 3:
					duration = 16000;
					break;
				case 4:
					duration = 19000;
					break;
				case 5:
					duration = 22000;
					break;
				case 6:
					duration = 25000;
					break;
				case 7:
					duration = 28000;
					break;
				case 8:
					duration = 31000;
					break;
				case 9:
					duration = 34000;
					break;
				case 10:
					duration = 37000;
					break;
			}
			break;
		case 10: /* SIGHT */
			duration = 12000;
			break;
		case 11: /*NAPALM BEAT */
			wait = 500;
			break;
		case 12: /* SAFETY WALL */
			wait = (skill_lvl * 800);
			break;
		case 13: /* SOUL STRIKE */
			switch (skill_lvl) {
				case 1:
				case 3:
				case 5:
				case 7:
				case 9:
					wait = 1000;
					break;
				case 2:
				case 4:
				case 6:
				case 8:
				case 10:
					wait = 500;
					break;
			}
			break;
		case 14: /* COLD BOLT */
			wait = (skill_lvl * 700);
			break;
		case 15: /* FROST DIVER */
			wait = 800;
			duration = (skill_lvl * 3000);
			break;
		case 16: /* STONE CURSE */
			wait = 100;
			duration = 30000 * skill_lvl;
			break;
		case 17: /* FIRE BALL */
			wait = 1600;
			break;
		case 18: /* FIRE WALL */
			wait = (skill_lvl * 800);
			break;
		case 19: /* FIRE BOLT */
			wait = (skill_lvl * 700);
			break;
		case 20: /* LIGHTNING BOLT */
			wait = (skill_lvl * 700);
			break;
		case 21: /* THUNDER STORM */
			wait = (skill_lvl * 1000);
			break;
		case 24: /* RUWACH */
			duration = 12000;
			break;
		case 27: /* WARP */
			wait = 2000;
			duration = 25000;
			break;
		case 29:  /* INCREASE AGI */
			wait = 1000;
			duration = (skill_lvl * 15000) + 50000;
			break;
		case 30: /* DECREASE AGI */
			wait = 1000;
			duration = (skill_lvl * 15000) + 50000;
			break;
		case 32: /* SIGNUM CRUCIS */
			wait = 100;
			break;
		case 33: /* ANGELUS */
			wait = 500;
			duration = (skill_lvl * 30000);
			break;
		case 34: /* BLESSING */
			wait = 100;
			switch (skill_lvl) {
				case 1: case 2: case 3: case 4: case 5:
					duration = 60000;
					break;
				case 6: case 7: case 8: case 9:
					duration = 90000;
					break;
				case 10:
					duration = 120000;
					break;
			}
			break;
		case 45: /* IMPROVE CONCENTRATION */
			duration = (40000 + (skill_lvl * 20000));
			break;
		case 51: /* HIDE */
			wait = 100;
			duration = (skill_lvl * 30000);
			break;
		case 52: /* ENVENOM */
			duration = 30000;
			break;
		case 57: /* BRANDISH SPEAR */
			wait = 1000;
			break;
		case 59: /* SPEAR BOOMERANG */
			wait = 1000;
			break;
		case 60: /* TWO HAND QUICKEN */
			wait = 100;
			duration = (skill_lvl * 30000);
			break;
		case 61: /* AUTO COUNTER */
			wait = 100;
			duration = 300 * skill_lvl;
			break;
		case 62: /* BOWLING BASH */
			wait = 1000;
			break;
		case 66: /* IMPOSITIO MANUS */
			duration = 60000;
			break;
		case 70: /* SANCTUARY */
			wait = 7000;
			duration = (1000 + (skill_lvl * 3000));
			break;
		case 73: /* KYRIE */
			duration = (10000 * skill_lvl);
			break;
		case 74: /* MAGNIFICANT */
			wait = 4000;
			duration = (skill_lvl * 20000);
			break;
		case 75: /* GLORIA */
			wait = 1000;
			duration = (5000 + (skill_lvl * 5000));
			break;
		case 76: /* LEX DIVINA */
			duration = (25000 + (skill_lvl * 5000));
			break;
		case 77: /* TURN UNDEAD */
			wait = 3000;
			break;
		case 79: /* MAGNUS EXORCISM */
			wait = 15000;
			break;
		case 80: /* FIRE PILLAR */
			wait = (skill_lvl * 1000);
			duration = 3 + (2* skill_lvl);
			break;
		case 81: /* SIGHT RASHER */
			wait = 1000;
			break;
		case 83: /* METEOR STORM */
			wait = 15500;
			break;
		case 84: /* JUPITEL THUNDER */
			wait = (2000 + (skill_lvl * 500));
			break;
		case 85: /* LORD OF VERMILION */
			wait = (15500 - (skill_lvl * 500));
			break;
		case 86: /* WATER BALL */
			wait = 10000;
			break;
		case 87: /* ICE WALL */
			wait = 10000;
			break;
		case 89: /* STORM GUST */
			wait = (15500 - (skill_lvl * 500));
			break;
		case 90: /* EARTH SPIKE */
			wait = (skill_lvl * 1000);
			break;
		case 91: /* HEAVEN DRIVE */
			wait = (skill_lvl * 1000);
			break;
		case 92: /* QUAGMIRE */
			wait = (skill_lvl * 1000);
			break;
		case 111: /* ADRENALINE RUSH */
			duration = (30000 * skill_lvl);
			break;
		case 112: /* WEAPON PERFECTION */
			duration = (10000 * skill_lvl);
			break;
		case 113: /* POWER THRUST */
			switch (skill_lvl) {
				case 1: case 2:
					duration = (20000 * skill_lvl);
					break;
				case 3: case 4: case 5:
					duration = (20000 + (skill_lvl * 10000));
					break;
				case 6:
					duration = 100000;
					break;
			}
			break;
		case 114: /* MAXIMIZE POWER */
			duration = (10000 * skill_lvl);
			break;
		case 117: /* ANKLE SNARE */
			wait = (3000 - (skill_lvl * 500));
			duration = (skill_lvl * 5);
			break;
		case 135: /* CLOACKING */
			wait = 100;
			duration = (3000 + (skill_lvl * 1000));
			break;
		case 136: /* SONIC BLOW */
			duration = 15000;
			break;
		case 138: /* ENCHANT POISON */
			duration = 15000 + (skill_lvl * 15000);
			break;
		case 145: /* FATAL BLOW */
			duration = 15000;
			break;
		case 149: /* SAND ATTACK */
			duration = 15000;
			break;
		case 151: /* FIND STONE */
			wait = 500;
			break;
		case 152: /* STONE FLING */
			wait = 1000;
			duration = 15000;
			break;
		case 155: /* CRAZY UPLOAR */
			wait = 100;
			duration = 300000;
			break;
		case 156: /* HOLY LIGHT */
			wait = 2000;
			break;
		case 157: /* ENERGY COAT */
			wait = 5000;
			duration = 300000;
			break;
		case 261: /* SUMMON SPIRIT SPHERES */
			wait = 1000;
			duration = 1000000;
			break;
		case 267: /* THROW SPIRIT SHPERES */
			switch (skill_lvl) {
				case 1:
					wait = 1000 + (1*1);
					break;
				case 2:
					if (sd->spheres == 1) wait = 1000 + (1*1);
					else wait = 1000 + (2*1);
					break;
				case 3:
					if (sd->spheres == 1) wait = 1000 + (1*1);
					else if (sd->spheres == 2) wait = 1000 + (2*1);
					else wait = 1000 + (3*1);
					break;
				case 4:
					if (sd->spheres == 1) wait = 1000 + (1*1);
					else if (sd->spheres == 2) wait = 1000 + (2*1);
					else if (sd->spheres == 3) wait = 1000 + (3*1);
					else wait = 1000 + (4*1);
					break;
				case 5:
					if (sd->spheres == 1) wait = 1000 + (1*1);
					else if (sd->spheres == 2) wait = 1000 + (2*1);
					else if (sd->spheres == 3) wait = 1000 + (3*1);
					else if (sd->spheres == 4) wait = 1000 + (4*1);
					else wait = 1000 + (5*1);
					break;
			}
			break;
		default: /* ALL OTHER SKILLS */
			wait = SKILL_MIN_WAIT;
			break;
	}
	if (sd->status.class == 2 || sd->status.class == 9) {
		skillwaitdata.wait = wait * (1 - (floor(sd->status.dex / 1.5) / 100));
	}
	else {
		skillwaitdata.wait = wait;
	}
	skillwaitdata.duration = duration;
	return skillwaitdata;
};

struct skilldelay skill_do_cast_delay(int skill_num, int skill_lvl)
{
	int delay = 0;

	switch (skill_num) {
		case 13: // Soul Strike
			delay = 1000 + (skill_lvl * 200);
			if (skill_lvl == 10) delay -= 300;
			break;
		case 15: // Frost Driver
			delay = 1500;
			break;
		case 16: // Stone Curse
			break;
		case 17: // Fireball
			delay = 1600;
			break;
		case 31: // Aqua Benedicta
		case 150: // Back Slide
		case 229: // Demonstration
		case 230: // Acid Terror
		case 231: // Aid Potion
		case 232: // Bio-Canibalize
		case 233: // Sphere Mine
		case 234: // Chemical Protection Weapon
		case 235: // Chemical Protection Shield
		case 236: // Chemical Protection Armor
		case 237: // Chemical Protection Helm
		case 266: // Occult Impact
		case 267: // Throw Spirit Sphere
			delay = 500;
			break;
		case 90: // Earth Spikes
			delay = 800;
			break;
		case 83: // Meteor Storm
			delay = 2000 + (800 * skill_lvl);
			break;
		case 89: // Storm Gust
			delay = 5000;
			break;
		case 7: // Magnum Break
		case 21: // Thunderstorm
		case 32: // Signum Crucis
		case 67: // Suffragium
		case 68: // Aspersio
		case 72: // Status Recovery
		case 73: // Kyrie Eleison
		case 74: // Magnificat
		case 75: // Gloria
			delay = 2000;
			break;
		case 66: // Impositio Manus
		case 76: // Lex Divina
		case 77: // Turn Undead
		case 78: // Lex Aeterna
		case 255: // Devotion
			delay = 3000;
			break;
		case 79: // Magnus Exorcismus
			delay = 4000;
			break;
		case 33: // Angelus
			delay = 3200;
			break;
		case 54: // Resurrection
			delay = skill_lvl - 1;
			break;
		case 11: // Napalm Beat
		case 24: // Ruwach
		case 25: // Pneuma
		case 28: // Heal
		case 29: // Increase Agi
		case 30: // Decrease Agi
		case 35: // Cure
		case 50: // Steal
		case 51: // Hiding
		case 52: // Envenom
		case 53: // Detoxify
		case 57: // Brandish Spear
		case 58: // Spear Stab
		case 59: // Spear Boomerang
		case 60: // Two Hand Quicken
		case 62: // Bowling Bash
		case 88: // Frost Nova
		case 91: // Heaven's Drive
		case 110: // Hammerfall
		case 111: // Adrenaline Rush
		case 112: // Weapon Perfection
		case 113: // Power Thrust
		case 114: // Maximize Power
		case 115: // Skid Trap
		case 116: // Land Mine
		case 117: // Ankle Snare
		case 118: // Shockwave Trap
		case 119: // Sandman
		case 120: // Flasher
		case 121: // Freezing Trap
		case 122: // Blast Mine
		case 123: // Claymore Trap
		case 129: // Blitz Beat
		case 135: // Cloaking
		case 138: // Enchant Poison
		case 149: // Sand Attack
		case 152: // Throw Stone
			delay = 1000;
			break;
		case 254: // Grand Cross
			delay = 300;
			break;
		case 271: // Asura Strike
			delay = 3500 - (500 * skill_lvl);
			break;
		default:
			delay = 0;
			break;

	}
	return skilldelaydata;
};

int extra_sp_cost(int skill_num, int skill_lvl)
{
	int sp = 0;
	switch(skill_num) {
	case 5: // bash
		if (skill_lvl > 5) {
			sp = 7;
		}
		break;
	case 6: // provoke
		break;
	case 7: // magnum break
		break;
	case 8: // endure
		break;
	case 10: // sight
		break;
	case 11: // napalm beat
		break;
	case 12: // safety wall
		if (skill_lvl > 3) sp = 5;
		if (skill_lvl > 6) sp = 10;
		break;
	case 13: // soul strike
		sp = 3 * skill_lvl;
		break;
	case 14: // cold bolt
		sp = 2 * skill_lvl;
		break;
	case 15: // frost diver
		if (skill_lvl > 1) sp = -1;
		if (skill_lvl > 2) sp = -2;
		if (skill_lvl > 3) sp = -3;
		if (skill_lvl > 4) sp = -4;
		if (skill_lvl > 5) sp = -5;
		if (skill_lvl > 6) sp = -6;
		if (skill_lvl > 7) sp = -7;
		if (skill_lvl > 8) sp = -8;
		if (skill_lvl > 9) sp = -9;
		break;
	case 16: // stone curse
		if (skill_lvl > 1) sp = -1;
		if (skill_lvl > 2) sp = -2;
		if (skill_lvl > 3) sp = -3;
		if (skill_lvl > 4) sp = -4;
		if (skill_lvl > 5) sp = -5;
		if (skill_lvl > 6) sp = -6;
		if (skill_lvl > 7) sp = -7;
		if (skill_lvl > 8) sp = -8;
		if (skill_lvl > 9) sp = -9;
		break;
	case 17: // fire ball
		break;
	case 18: // fire wall
		break;
	case 19: // fire bolt
	case 20: // lightening bolt
		sp = 2 * skill_lvl;
		break;
	case 21: // thunder storm
		sp = 5 * skill_lvl;
		break;
	case 24: // ruwach
		break;
	case 25: // pneuma
		break;
	case 26: // teleport
		break;
	case 27: // warp
		break;
	case 28: // heal
		sp = 3 * skill_lvl;
		break;
	case 29: // increase agi
		sp = 3 * skill_lvl;
		break;
	case 30: // decrease agi
		sp = 2 * skill_lvl;
		break;
	case 31: // aqua_benedicta
		break;
	case 32: // signum crusis
		break;
	case 33: // angelus
		sp = 3 * skill_lvl;
		break;
	case 34: // blessing
		sp = 4 * skill_lvl;
		break;
	case 35: // cure
		break;
	case 40: // item appraisal
		break;
	case 41: // vending
		break;
	case 42: // mammonite
		break;
	case 45: // concentration
		sp = 5 * skill_lvl;
		break;
	case 46: // double strafe
		break;
	case 47: // arrow shower
		break;
	case 50: // steal
		break;
	case 51: // hiding
		break;
	case 52: // envenom
		break;
	case 53: // detoxify
		break;
	case 56: // pierce
		break;
	case 57: // brandish spear
		break;
	case 58: // spear stab
		break;
	case 59: // spear boomerang
		break;
	case 60: // two hand quicken
		sp = 4 * skill_lvl;
		break;
	case 61: // counter attack
		break;
	case 62: // bowling bash
		sp = skill_lvl;
		break;
	case 66: // impositio manus
		sp = 3 * skill_lvl;
		break;
	case 67: // suffragium
		break;
	case 68: // aspersio
		sp = 4 * skill_lvl;
		break;
	case 69: // benedictio sanctissimi sacramenti
		break;
	case 70: // sanctuary
		sp = 3 * skill_lvl;
		break;
	case 71: // slow poison
		sp = 2 * skill_lvl;
		break;
	case 72: // status recovery
		break;
	case 73: // kyrie eleison
		sp = 5 * ((skill_lvl - 1) / 3);
		break;
	case 74: // magnificat
		break;
	case 75: // gloria
		break;
	case 76: // lex divina
		if (skill_lvl > 5) sp = -2;
		if (skill_lvl > 6) sp = -4;
		if (skill_lvl > 7) sp = -6;
		if (skill_lvl > 8) sp = -8;
		if (skill_lvl > 9) sp = -10;
		break;
	case 77: // turn undead
		break;
	case 78: // lex aeterna
		break;
	case 79: // magnus exorcismus
		sp = 2*skill_lvl;
		break;
	case 80: // fire pillar
		break;
	case 81: // sight trasher
		sp = 2 * skill_lvl;
		break;
	case 82: // fireivy
	case 83: // meteor storm
		if (skill_lvl > 1) sp = 4;
		if (skill_lvl > 2) sp = 10;
		if (skill_lvl > 3) sp = 14;
		if (skill_lvl > 4) sp = 20;
		if (skill_lvl > 5) sp = 24;
		if (skill_lvl > 6) sp = 30;
		if (skill_lvl > 7) sp = 34;
		if (skill_lvl > 8) sp = 40;
		if (skill_lvl > 9) sp = 44;
		break;
	case 84: // jupiter thunder
		sp = 3 * skill_lvl;
		break;
	case 85: // lord of vermilion
		sp = 4 * skill_lvl;
		break;
	case 86: // water ball
		if (skill_lvl > 1) sp = 5;
		if (skill_lvl > 3) sp = 15;
		break;
	case 87: // ice wall
		break;
	case 88: // frost nova
		if (skill_lvl > 1) sp = -2;
		if (skill_lvl > 2) sp = -4;
		if (skill_lvl > 3) sp = -6;
		if (skill_lvl > 4) sp = -8;
		if (skill_lvl > 5) sp = -10;
		if (skill_lvl > 6) sp = -12;
		if (skill_lvl > 7) sp = -14;
		if (skill_lvl > 8) sp = -16;
		if (skill_lvl > 9) sp = -18;
		break;
	case 89: // storm gust
		break;
	case 90: // earth spike
		sp = 2 * skill_lvl;
		break;
	case 91: // heaven's drive
		sp = 4 * skill_lvl;
		break;
	case 92: // quagmire
		break;
	case 93: // sense
		break;
	case 110: // hammer fall
		break;
	case 111: // adrenaline rush
		sp = 3 * skill_lvl;
		break;
	case 112: // weapon perfection
		if (skill_lvl > 1) sp = -2;
		if (skill_lvl > 2) sp = -4;
		if (skill_lvl > 3) sp = -6;
		if (skill_lvl > 4) sp = -8;
		break;
	case 113: // power-thrust
		if (skill_lvl > 1) sp = -2;
		if (skill_lvl > 2) sp = -4;
		if (skill_lvl > 3) sp = -6;
		if (skill_lvl > 4) sp = -8;
		break;
	case 114: // power maximize
		break;
	case 115: // skid trap
		break;
	case 116: // land mine
		break;
	case 117: // ankle snare
		break;
	case 118: // shockwave
		break;
	case 119: // sandman
		break;
	case 120: // flasher
		break;
	case 121: // freezing trap
		break;
	case 122: // blastmine
		break;
	case 123: // claymore trap
		break;
	case 129: // blitz beat
		sp = 3 * skill_lvl;
		break;
	case 135: // cloacking
		break;
	case 136: // sonic blow
		sp = 2 * skill_lvl;
		break;
	case 139: // poison react
		if (skill_lvl > 1) sp = -5;
		if (skill_lvl > 2) sp = -10;
		if (skill_lvl > 3) sp = -15;
		if (skill_lvl > 4) sp = -20;
		if (skill_lvl > 5) sp = -25;
		if (skill_lvl > 6) sp = -30;
		if (skill_lvl > 7) sp = -35;
		if (skill_lvl > 8) sp = -40;
		if (skill_lvl > 9) sp = -45;
		break;
	case 141: // venom splasher
		sp = 3 * skill_lvl;
		break;
	case 266: // occult impact
	case 267: // throw Spirit Sphere
		sp = 4 * skill_lvl;
		break;
	}
	return sp;
}

int skill_calc_damage(int fd, int skill_num, int skill_lvl, int target_id)
{
	int damage = 0;
	int s_type, s_lv;
	int i, a1 = 0, a2 = 0;
	int m, n;
	int attack_element = 0;
	int target_element = 0;
	int chance = 0;
	double aux = 0;
	int aux2 = 0;
	struct map_session_data *sd = NULL;

	if (session[fd] == NULL || (sd = session[fd]->session_data) == NULL) {
		return 0;
	}
	for (i = 0; i < 100; i++) {
		struct item_db2 weapon;
		if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip != 0)) {
			weapon = item_database(sd->status.inventory[i].nameid);
			if (weapon.type == 4) {
				attack_element = weapon.ele;
				break;
			}
		}
	}
	//if (aspersion enabled) attack_element = 6; // Holy
	m = sd->mapno;
	n = mmo_map_search_monster(m, target_id);
	if (n >= 0) {
		target_element = mons_data[map_data[m].npc[n]->class].ele;
	} else {
		// if (Benedictio Sanctissimi Sacramenti (Priest) enabled) target_element = 6; // Holy
	}
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
	else if (sd->status.class == 15 && (sd->status.weapon == 0 || sd->status.weapon == 12)) {
		s_type = 3;
		s_lv = sd->status.skill[259-1].lv;
	}
	else {
		s_lv = s_type = 0;
	}
	switch (skill_num) {
	case 5: // BASH
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl);
		damage = (int) (damage * get_ele_attack_factor(attack_element, target_element));
		break;

	case 7: // MAGNUM_BREAK
		damage = SKILL_MAGNUM (sd->status.atk1, sd->status.atk2, skill_lvl);
		damage = (int) (damage * get_ele_attack_factor(3, target_element));
		break;

	case 11: // NAPALM_BEAT
		damage = SKILL_NAPALM (sd->status.matk1, sd->status.matk2, skill_lvl);
		damage = (int) (damage * get_ele_attack_factor(8, target_element));
		break;

	case 13: // SOUL STRIKE
		/*damage = sd->status.matk1 + rand()%(sd->status.matk2 - sd->status.matk1 + 1);
		if (n >= 0) {
			aux = get_ele_attack_factor(8, target_element);
			aux2 = (1 - (mons_data[map_data[m].npc[n]->class].mdef1 / 100));
			if (aux2 == 0) {
				aux2 = 1;
			}
			damage = (int)(damage * (aux / aux2));
			damage -= mons_data[map_data[m].npc[n]->class].mdef2;
			damage *= skill_lvl;
		}
		else {
			damage = (int) (damage * get_ele_attack_factor(8, target_element));
			damage *= skill_lvl;
			if (damage < 0) damage = 0;
		}
		break;*/

	case 14: // COLDBOLT
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		if (n >= 0) {
			aux = get_ele_attack_factor(4, target_element);
			aux2 = (1 - (mons_data[map_data[m].npc[n]->class].mdef1 / 100));
			if (aux2 == 0) {
				aux2 = 1;
			}
			damage = (int)(damage * (aux / aux2));
			damage -= mons_data[map_data[m].npc[n]->class].mdef2;
		}
		else {
			damage = (int) (damage * get_ele_attack_factor(4, target_element));
		}
		damage *= skill_lvl;
		break;

	case 15: // FROST DIVER
		damage = SKILL_FROSTDIVER(sd->status.matk1);
		break;

	case 17: // FIREBALL
		damage = SKILL_FIREBALL (sd->status.matk1, sd->status.matk2, skill_lvl);
		if (n >= 0) {
			aux = get_ele_attack_factor(3, target_element);
			aux2 = (1 - (mons_data[map_data[m].npc[n]->class].mdef1 / 100));
			if (aux2 == 0) {
				aux2 = 1;
			}
			damage = (int)(damage * (aux / aux2));
			damage -= mons_data[map_data[m].npc[n]->class].mdef2;
		}
		else {
			damage = (int) (damage * get_ele_attack_factor(3, target_element));
		}
		break;

	case 19: // FIREBOLT
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		if (n >= 0) {
			aux = get_ele_attack_factor(3, target_element);
			aux2 = (1 - (mons_data[map_data[m].npc[n]->class].mdef1 / 100));
			if (aux2 == 0) {
				aux2 = 1;
			}
			damage = (int)(damage * (aux / aux2));
			damage -= mons_data[map_data[m].npc[n]->class].mdef2;
		}
		else {
			damage = (int) (damage * get_ele_attack_factor(3, target_element));
		}
		damage *= skill_lvl;
		break;

	case 20: // LIGHTINGBOLT
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		if (n >= 0) {
			aux = get_ele_attack_factor(4, target_element);
			aux2 = (1 - (mons_data[map_data[m].npc[n]->class].mdef1 / 100));
			if (aux2 == 0) {
				aux2 = 1;
			}
			damage = (int)(damage * (aux / aux2));
			damage -= mons_data[map_data[m].npc[n]->class].mdef2;
		}
		else {
			damage = (int)(damage * get_ele_attack_factor(4, target_element));
		}
		damage *= skill_lvl;
		break;

	case 28: // HEAL
		s_lv = s_type = 0;
		damage = -SKILL_HEAL(sd->status.int_, sd->status.base_level, skill_lvl);
		if (((target_element%10) == 8) || ((target_element%10) == 9)) {
			damage = -0.5 * ((int)(damage * get_ele_attack_factor(6, target_element)));
		}
		break;

	case 42: // MAMMONITE
		damage = SKILL_BOWLING(sd->status.atk1, sd->status.atk2, skill_lvl);
		break;

	case 52: // ENVENOM
		damage = SKILL_ENVENOM(sd->status.atk1, sd->status.atk2, skill_lvl);
		damage = (int)(damage * get_ele_attack_factor(5, target_element));
		break;

	case 54: // RESURECTION
		s_lv = s_type = 0;
		damage = -1;
		break;

	case 56: // PIERCE
		damage = SKILL_PIERCE(sd->status.atk1, sd->status.atk2, skill_lvl);
		break;

	case 57: // BRANDISH SPEAR
		damage = SKILL_BRANDISH(sd->status.atk1, sd->status.atk2, skill_lvl);
		break;

	case 58: // SPEAR STAB
		damage = SKILL_MAGNUM(sd->status.atk1, sd->status.atk2, skill_lvl);
		break;

	case 59: // SPEAR BOOMERANG
		damage = SKILL_BOWLING(sd->status.atk1, sd->status.atk2, skill_lvl);
		break;

	case 62: // BOWLING_BASH
		damage = SKILL_BOWLING(sd->status.atk1, sd->status.atk2, skill_lvl);
		damage = (int)(damage * get_ele_attack_factor(attack_element, target_element));
		break;

	case 77: // TURN UNDEAD
		if (n >= 0) {
			if ((mons_data[map_data[m].npc[n]->class].ele % 10 == 9)) {
				chance = (skill_lvl * 2 + (sd->status.base_level * 0.1) + (sd->status.luk * 0.1) + (sd->status.int_ * 0.1) + (1 - (map_data[m].npc[n]->u.mons.hp / mons_data[map_data[m].npc[n]->class].max_hp)) * 20);
				chance = 100 - chance;
				if (rand() % 100 > chance) {
					damage = sd->status.hp;
				}
				else {
					damage = sd->status.base_level + sd->status.int_ + skill_lvl * 10;
				}
				damage = (int)(damage * get_ele_attack_factor(6, target_element));
			}
			else {
				damage = 0;
			}
		}
		break;

	case 80: // FIRE PILLAR
		damage = SKILL_FIREPILLAR(sd->status.matk1, skill_lvl);
		damage = (int)(damage * get_ele_attack_factor(3, target_element));
		break;

	case 81:
		damage = SKILL_TRASHER(sd->status.matk1, sd->status.matk2, skill_lvl);
		break;

	case 83: //METEOR STORM
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		damage = (int)(damage * get_ele_attack_factor(3, target_element));
		break;

	case 84: // JUPITEL THUNDER
		damage = sd->status.matk1 + rand() % (sd->status.matk2 - sd->status.matk1 + 1);
		if (n >= 0) {
			damage = damage *(100 - mons_data[map_data[m].npc[n]->class].mdef1) / 100;
			damage = damage - mons_data[map_data[m].npc[n]->class].mdef1;
			if (damage < 1) {
				damage = 1;
			}
			damage = (int)(damage * get_ele_attack_factor(4, target_element));
			if (damage < 0) {
				damage = 0;
			}
		}
		else {
			damage = (int)(damage * get_ele_attack_factor(4, target_element));
		}
		damage = damage * (2 + skill_lvl);
		break;

	case 90: // EARTH SPIKE
		damage = sd->status.matk1 + rand() % (sd->status.matk2 - sd->status.matk1 + 1);
		if (n >= 0) {
			damage = damage * (100 - mons_data[map_data[m].npc[n]->class].mdef1) / 100;
			damage = damage - mons_data[map_data[m].npc[n]->class].mdef1;
			if (damage < 1) {
				damage = 1;
			}
			damage = (int)(damage * get_ele_attack_factor(2, target_element));
			damage = damage * skill_lvl;
			if (damage < 0) {
				damage = 0;
			}
		}
		else {
			damage = (int)(damage * get_ele_attack_factor(2, target_element));
		}
		break;

	case 91: // HEAVEN DIVE
		damage = sd->status.matk1 + rand() % (sd->status.matk2 - sd->status.matk1 + 1) * 5;
		if (damage < 1) {
			damage = 1;
		}
		damage = (int)(damage * get_ele_attack_factor(2, target_element));
		if (damage < 0) {
			damage = 0;
		}
		break;

	case 85: // VERMILION
		damage = SKILL_MATTACK (sd->status.matk1, sd->status.matk2);
		damage = (int)(damage * get_ele_attack_factor(4, target_element));
		damage *= 1.5;
		break;

	case 129: // BLITZ BEAT
		if (sd->auto_blitz) {
			if (sd->status.job_level <= 10) {
				skill_lvl = 1;
			}
			else if (sd->status.job_level > 10 && sd->status.job_level <= 20) {
				skill_lvl = 2;
			}
			else if (sd->status.job_level > 20 && sd->status.job_level <= 30) {
				skill_lvl = 3;
			}
			else if (sd->status.job_level > 30 && sd->status.job_level <= 40) {
				skill_lvl = 4;
			}
			else {
				skill_lvl = 5;
			}
		}
		damage = SKILL_BLITZBEAT (sd->status.dex, sd->status.int_, sd->status.skill[128-1].lv, skill_lvl);
		damage = (int)(damage * get_ele_attack_factor(0, target_element));
		break;

	case 136: // SONIC BLOW
		damage = SKILL_SONICBLOW (sd->status.atk1, sd->status.atk2, skill_lvl);
		break;

	case 137: // GRIMTOOTH
		if (n >= 0) {
			damage = NOM_ATTACK(sd->status.atk1, sd->status.atk2, map_data[m].npc[n]->u.mons.def1);
		}
		else {
			damage = NOM_ATTACK(sd->status.atk1, sd->status.atk2, 0);
		}
		damage = (int)(damage * get_ele_attack_factor(0, target_element));
		break;

	case 142: // FIRST AID
		s_lv = s_type = 0;
		damage = 5;
		break;

	case 152: // STONE FLING
		damage = 30;
		break;

	case 156: // HOLY LIGHT
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		damage *= 1.5;
		break;

	case 263: // TRIPLE ATTACK
		damage = NOM_ATTACK(sd->status.atk1, sd->status.atk2, map_data[m].npc[n]->u.mons.def1);
		damage *= ((100 + (20 * sd->status.skill[263-1].lv))/100);
		damage *= 3;
		break;

	case 266: // OCCULT IMPACT
		damage = sd->status.atk1 + rand() % (sd->status.atk2);
		damage *= ((100 + (75 * sd->status.skill[266-1].lv))/100);
		damage = (damage * (map_data[m].npc[n]->u.mons.def2+map_data[m].npc[n]->u.mons.def2))/100;
		break;

	case 267: // Throw Spirit Sphere
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		damage *= ((100 + (50 * skill_lvl))/100);
		switch (skill_lvl) {
			case 1:
				if (sd->spheres >= 1) damage *= 1;
				break;
			case 2:
				if (sd->spheres >= 2) damage *= 2;
				else damage *= sd->spheres;
				break;
			case 3:
				if (sd->spheres >= 3) damage *= 3;
				else damage *= sd->spheres;
				break;
			case 4:
				if (sd->spheres >= 4) damage *= 4;
				else damage *= sd->spheres;
				break;
			case 5:
				if (sd->spheres == 5) damage *= 5;
				else damage *= sd->spheres;
				break;
		}
		break;

	default: // ALL OTHER skills
		if (sd->status.matk1 <= 0) {
			a1 = 1;
		}
		else {
			a1 = sd->status.matk1;
		}
		if (sd->status.matk2 <= 0) {
			a2 = 1;
		}
		else {
			a2 = sd->status.matk2;
		}
		damage = SKILL_MATTACK (a1, a2);
		damage *= skill_lvl ;
		break;

	}
	if (damage) {
		damage += s_lv*s_type;
	}
	return damage;
}

int skill_do_damage(int fd, int damage, int target_id, unsigned long tick, int healed)
{
	int n;
  	short m;
	int mvp_damage = 0;
  	int target_fd;
	unsigned int i;
  	unsigned int mvp_fd = 0;
  	unsigned char buf[256];
  	struct map_session_data *sd, *target_sd, *temp_sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

  	m = sd->mapno;
  	n = mmo_map_search_monster(m, target_id);

	if (n < 0) {
		if (!mmo_map_flagload(sd->mapname, PVP) || !PVP_flag)
			return 0;

		target_fd = 0;
		target_sd = NULL;
		for (i = 5; i < FD_SETSIZE; i++) {
			if (session[i] && (target_sd = session[i]->session_data)) {
				if (target_sd->account_id == target_id) {
					target_fd = i;
					break;
				}
			}
		}
		target_sd->status.hp -= damage;
		if (target_sd->status.hp < 0)
			target_sd->status.hp = 0;

		WFIFOW(target_fd, 0) = 0xb0;
		WFIFOW(target_fd, 2) = 0005;
		WFIFOL(target_fd, 4) = target_sd->status.hp;
		WFIFOSET(target_fd, packet_len_table[0xb0]);

		if (target_sd->status.hp <= 0) {
			target_sd->sitting = 1;
			WBUFW(buf, 0) = 0x80;
			WBUFL(buf, 2) = target_id;
			WBUFB(buf, 6) = 1;
			mmo_map_sendarea(fd, buf, packet_len_table[0x80], 0);
			for (i = 5; i < FD_SETSIZE; i++) {
				if (session[i] && (temp_sd = session[i]->session_data)) {
					if (temp_sd->attacktarget == target_sd->account_id) {
						delete_timer(temp_sd->attacktimer, mmo_map_attack_continue);
						temp_sd->attacktimer = 0;
						temp_sd->attacktarget = 0;
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
	else {
		if (map_data[m].npc[n]->u.mons.target_fd != fd && map_data[m].npc[n]->u.mons.hp > 0)
			check_new_target_monster(m, n, fd);

		if (healed && ((mons_data[map_data[m].npc[n]->class].ele % 10 == 8) || (mons_data[map_data[m].npc[n]->class].ele % 10 == 9))) {
			damage /= 2;
			map_data[m].npc[n]->u.mons.hp -= damage;

			WBUFW(buf, 0) = 0x114;
			WBUFW(buf, 2) = 28;
			WBUFL(buf, 4) = sd->account_id;
			WBUFL(buf, 8) = target_id;
			WBUFL(buf, 12) = gettick();
			WBUFL(buf, 16) = sd->speed;
			WBUFL(buf, 20) = map_data[m].npc[n]->u.mons.speed;
			WBUFW(buf, 24) = damage;
			WBUFW(buf, 26) = sd->status.skill[28-1].lv;
			WBUFW(buf, 28) = skill_db[28].type_num;
			WBUFB(buf, 30) = skill_db[28].type_hit;
			mmo_map_sendarea(fd, buf, packet_len_table[0x114], 0);

			WBUFW(buf, 0) = 0x88;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = sd->x;
			WBUFW(buf, 8) = sd->y;
			mmo_map_sendarea(fd, buf, packet_len_table[0x88], 0);
		}
		else if (healed && !((mons_data[map_data[m].npc[n]->class].ele % 10 == 8) || (mons_data[map_data[m].npc[n]->class].ele % 10 == 9))) {
			damage *= -1;
			map_data[m].npc[n]->u.mons.hp += damage;

			WBUFW(buf, 0) = 0x11a;
			WBUFW(buf, 2) = 28;
			WBUFW(buf, 4) = damage;
			WBUFL(buf, 6) = target_id;
			WBUFL(buf, 10) = sd->account_id;
			WBUFB(buf, 14) = 1;
			mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);

    			if (map_data[m].npc[n]->u.mons.hp > mons_data[map_data[m].npc[n]->class].max_hp)
				map_data[m].npc[n]->u.mons.hp = mons_data[map_data[m].npc[n]->class].max_hp;

			return 0;
		}
		else {
			map_data[m].npc[n]->u.mons.hp -= damage;
			sd->status.damage_atk += damage;
		}
		if ((rand() % 90) == 10) {
			if (map_data[m].npc[n]->u.mons.hp >= (mons_data[map_data[m].npc[n]->class].max_hp / 2))
				monster_say(fd, target_id, map_data[m].npc[n]->class, "hp50");

			else if (map_data[m].npc[n]->u.mons.hp <= (mons_data[map_data[m].npc[n]->class].max_hp / 4))
				monster_say(fd, target_id, map_data[m].npc[n]->class, "hp25");

			else if (map_data[m].npc[n]->u.mons.hp <= 0)
				monster_say(fd, target_id, map_data[m].npc[n]->class, "dead");

		}
		if (map_data[m].npc[n]->u.mons.hp <= 0) {
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
				if (session[i] && (temp_sd = session[i]->session_data) && temp_sd->attacktarget == target_id) {
					if (temp_sd->status.damage_atk >= mvp_damage) {
						mvp_damage = temp_sd->status.damage_atk;
						mvp_fd = i;
					}
          				mmo_map_level_mons (temp_sd, m, n);
					delete_timer(temp_sd->attacktimer, mmo_map_attack_continue);
          				temp_sd->attacktimer = 0;
					temp_sd->attacktarget = 0;
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

int skill_do_delayed_place(int tid, unsigned int tick, int fd, int skill_num)
{
	int i = 0, j, res;
	short x, y;
	int skill_x;
	int skill_y;
	int skill_lvl;
	int skill_type;
	int damage;
	int target_id = 0;
	unsigned char buf[256];
	struct map_session_data *sd = NULL;

	if (session[fd] == NULL || (sd = session[fd]->session_data) == NULL) {
		return 0;
	}
	skill_type = search_placeskill(skill_num);
	sd->casting = 0;
	skill_x = sd->skill_x;
	skill_y = sd->skill_y;
	skill_lvl = sd->skill_lvl;

	WBUFW(buf, 0) = 0x117;
	WBUFW(buf, 2) = skill_num;
	WBUFL(buf, 4) = sd->account_id;
	WBUFW(buf, 8) = 100;
	WBUFW(buf, 10) = sd->skill_x;
	WBUFW(buf, 12) = sd->skill_y;
	WBUFL(buf, 14) = gettick();
	mmo_map_sendarea(fd, buf, packet_len_table[0x117], 0);

	switch (skill_num) {
	case 12: // SAFETY WALL
		break;
	case 27: // WARP
		WFIFOW(fd, 0)= 0x11c;
		WFIFOW(fd, 2)= skill_num;
		strcpy(WFIFOP(fd, 4), sd->status.save_point.map);
		if (skill_lvl > 1) {
			strcpy(WFIFOP(fd, 20), sd->status.memo_point[0].map);
		}
		else {
			strcpy(WFIFOP(fd, 20), "");
		}
		if (skill_lvl > 2) {
			strcpy(WFIFOP(fd, 36), sd->status.memo_point[1].map);
		}
		else {
			strcpy(WFIFOP(fd, 36), "");
		}
		if (skill_lvl > 3) {
			strcpy(WFIFOP(fd, 52), sd->status.memo_point[2].map);
		}
		else {
			strcpy(WFIFOP(fd, 52), "");
		}
		WFIFOSET(fd, packet_len_table[0x11c]);
		i = next_floorskill_index();
		make_floorskill(fd, skill_x, skill_y, skill_lvl, 0x81, FS_WARP, skill_num, i);
		break;
	case 70: // SANCTUARY
		i = next_floorskill_index();
		for (x = skill_x - 2; x <= skill_x + 2; x++) {
			for (y = skill_y - 2;y <= skill_y + 2; y++) {
				if ((x == skill_x - 2 && y == skill_y - 2) ||
				    (x == skill_x + 2 && y == skill_y + 2) ||
				    (x == skill_x - 2 && y == skill_y + 2) ||
				    (x == skill_x + 2 && y == skill_y - 2)) {
					continue;
				}
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_SANCT, skill_num, i);
			}
		}
		break;
	case 79: // MAGNUS EXORCISMUS
		i = next_floorskill_index();
		for (x = skill_x - 3; x < skill_x + 3; x++) {
			for (y = skill_y - 3; y < skill_y + 3; y++) {
				if ((x == skill_x - 3 && y == skill_y - 3) ||
				    (x == skill_x + 2 && y == skill_y + 2) ||
				    (x == skill_x - 3 && y == skill_y + 2) ||
				    (x == skill_x + 2 && y == skill_y - 3)) {
					continue;
				}
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_MAGNUS, skill_num, i);
			}
		}
		break;
	case 83: // METEOR STORM
		for (x = -3;x <= 3; x++) {
			for (y = -3;y <= 3; y++) {
				for (j = 0; j < map_data[sd->mapno].npc_num; j++) {
					if (map_data[sd->mapno].npc[j]->block.subtype == MONS) {
						if (map_data[sd->mapno].npc[j]->x == (skill_x + x) &&
						    map_data[sd->mapno].npc[j]->y == (skill_y + y)) {
							target_id = map_data[sd->mapno].npc[j]->id;
							damage = skill_calc_damage (fd, skill_num, skill_lvl, target_id);
							if (damage > map_data[sd->mapno].npc[j]->u.mons.hp) {
								damage = map_data[sd->mapno].npc[j]->u.mons.hp;
							}
							WBUFW(buf, 0) = 0x11f;
							WBUFL(buf, 2) = target_id;
							WBUFL(buf, 6) = sd->account_id;
							WBUFW(buf, 10) = skill_x + x;
							WBUFW(buf, 12) = skill_y + y;
							WBUFB(buf, 14) = skill_type;
							WBUFB(buf, 15) = 1;
							mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);
							WBUFW(buf, 0) = 0x114;
							WBUFW(buf, 2) = skill_num;
							WBUFL(buf, 4) = sd->account_id;
							WBUFL(buf, 8) = target_id;
							WBUFL(buf, 12) = gettick();
							WBUFL(buf, 16) = sd->speed;
							WBUFL(buf, 20) = 250;
							WBUFW(buf, 24) = damage;
							WBUFW(buf, 26) = skill_lvl;
							WBUFW(buf, 28) = (skill_lvl / 3 + 1);
							WBUFB(buf, 30) = skill_db[skill_num].type_hit;
							mmo_map_sendarea(fd, buf, packet_len_table[0x114], 0);
							skill_do_damage(fd, damage, target_id, gettick(), 0);
						}
					}
				}
			}
		}
		break;
	case 87: // ICE WALL
		i = next_floorskill_index();
		res = calc_dir2(sd->x, sd->y, skill_x, skill_y);
		if (res == 0) {
			for (x = -2; x <= 2; x++) {
				make_floorskill(fd, skill_x + x, skill_y, skill_lvl, skill_type, FS_ICEWALL, skill_num, i);
			}
		}
		else if (res == 1) {
			for (x = skill_x + 2, y = skill_y + 2; x >= skill_x - 2; x--, y--) {
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_ICEWALL, skill_num, i);
			}
		}
		else if (res == 2) {
			for (x = -2; x <= 2; x++) {
				make_floorskill(fd, skill_x, skill_y + x, skill_lvl, skill_type, FS_ICEWALL, skill_num, i);
			}
		}
		else if (res == 3) {
			for (x = skill_x - 2, y = skill_y + 2; x <= skill_x + 2; x++, y--) {
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_ICEWALL, skill_num, i);
			}
		}
		break;
	case 91: // HEAVEN DIVE
		for (x = -2; x <= 2; x++) {
			for (y = -2; y <= 2; y++) {
				for (j = 0; j < map_data[sd->mapno].npc_num; j++) {
					if (map_data[sd->mapno].npc[j]->block.subtype == MONS) {
						if (map_data[sd->mapno].npc[j]->x == (skill_x + x) &&
						    map_data[sd->mapno].npc[j]->y == (skill_y + y)) {
							target_id = map_data[sd->mapno].npc[j]->id;
							damage = skill_calc_damage(fd, skill_num, skill_lvl, target_id);
							if (damage > map_data[sd->mapno].npc[j]->u.mons.hp) {
								damage = map_data[sd->mapno].npc[j]->u.mons.hp;
							}
							WBUFW(buf, 0) = 0x11f;
							WBUFL(buf, 2) = target_id;
							WBUFL(buf, 6) = sd->account_id;
							WBUFW(buf, 10) = skill_x + x;
							WBUFW(buf, 12) = skill_y + y;
							WBUFB(buf, 14) = skill_type;
							WBUFB(buf, 15) = 1;
							mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);
							WBUFW(buf, 0) = 0x114;
							WBUFW(buf, 2) = skill_num;
							WBUFL(buf, 4) = sd->account_id;
							WBUFL(buf, 8) = target_id;
							WBUFL(buf, 12) = gettick();
							WBUFL(buf, 16) = sd->speed;
							WBUFL(buf, 20) = 250;
							WBUFW(buf, 24) = damage;
							WBUFW(buf, 26) = skill_lvl;
							WBUFW(buf, 28) = skill_db[skill_num].type_num * skill_lvl;
							WBUFB(buf, 30) = skill_db[skill_num].type_hit;
							mmo_map_sendarea(fd, buf, packet_len_table[0x114], 0);
							skill_do_damage(fd, damage, target_id, gettick(), 0);
						}
					}
				}
			}
		}
		break;
	case 92: // QUAGMIRE
		i = next_floorskill_index();
		for (x = skill_x - 2; x <= skill_x + 2; x++) {
			for (y = skill_y - 2; y <= skill_y + 2; y++) {
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_QUAG, skill_num, i);
			}
		}
		break;
	case 110: // HAMMER FALL
		WBUFW(buf, 0) = 0x11f;
		WBUFL(buf, 2) = target_id;
		WBUFL(buf, 6) = sd->account_id;
		WBUFW(buf, 10) = skill_x;
		WBUFW(buf, 12) = skill_y;
		WBUFB(buf, 14) = skill_type;
		WBUFB(buf, 15) = 1;
		mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);
		for (x = -3; x <= 3; x++) {
			for (y = -3; y <= 3; y++) {
				for (j = 0; j < map_data[sd->mapno].npc_num; j++) {
					if (map_data[sd->mapno].npc[j]->block.subtype == MONS) {
						if (map_data[sd->mapno].npc[j]->x == (skill_x + x)&&
						    map_data[sd->mapno].npc[j]->y == (skill_y + y)) {
							target_id = map_data[sd->mapno].npc[j]->id;
							if ((rand() % 100) >= (100 -(20 + (skill_lvl * 10)))) {
								WBUFW(buf, 0) = 0x119;
								WBUFL(buf, 2) = target_id;
								WBUFW(buf, 6) = 3;
								WBUFW(buf, 8) = 0;
								WBUFW(buf, 10) = 0;
								WBUFB(buf, 12) = 0;
								mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
								map_data[sd->mapno].npc[j]->skilldata.skill_num = skill_num;
								map_data[sd->mapno].npc[j]->skilldata.fd = fd;
		    						map_data[sd->mapno].npc[j]->u.mons.speed = -1;
		  						map_data[sd->mapno].npc[j]->option=3|0|0;
								map_data[sd->mapno].npc[j]->skilldata.effect |= 0x20;
								map_data[sd->mapno].npc[j]->skilldata.skill_timer[110-1][0] = add_timer((unsigned int)(gettick() + 15000), skill_reset_monster_stats, sd->mapno, j);
							}
						}
					}
				}
			}
		}
		break;
	case 117: // ANKLE SNARE
	case 115: // SKIDTRAP
	case 116: // LANDMINE
	case 118: // SHOCKWAVE
	case 119: // SANDMAN
	case 120: // FLASHER
	case 121: // FREEZING
	case 122: // BLASTMINE
	case 123: // CLAYMORE
		i = next_floorskill_index();
		make_floorskill(fd, skill_x, skill_y, skill_lvl, skill_type, FS_TRAP, skill_num, i);
		break;
	default:
		for (x = -5; x <= 5; x++) {
			for (y = -5; y <= 5; y++) {
				for (j = 0; j < map_data[sd->mapno].npc_num; j++) {
					if (map_data[sd->mapno].npc[j]->block.subtype == MONS) {
						if (map_data[sd->mapno].npc[j]->x == (skill_x + x) &&
						    map_data[sd->mapno].npc[j]->y == (skill_y + y)) {
							target_id = map_data[sd->mapno].npc[j]->id;
							damage = skill_calc_damage (fd, skill_num, skill_lvl, target_id);
							if (damage > map_data[sd->mapno].npc[j]->u.mons.hp) {
								damage = map_data[sd->mapno].npc[j]->u.mons.hp;
							}
							WBUFW(buf, 0) = 0x11f;
							WBUFL(buf, 2) = target_id;
							WBUFL(buf, 6) = sd->account_id;
							WBUFW(buf, 10) = skill_x + x;
							WBUFW(buf, 12) = skill_y + y;
							WBUFB(buf, 14) = skill_type;
							WBUFB(buf, 15) = 1;
							mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);
							WBUFW(buf, 0) = 0x114;
							WBUFW(buf, 2) = skill_num;
							WBUFL(buf, 4) = sd->account_id;
							WBUFL(buf, 8) = target_id;
							WBUFL(buf, 12) = gettick();
							WBUFL(buf, 16) = sd->speed;
							WBUFL(buf, 20) = 250;
							WBUFW(buf, 24) = damage;
							WBUFW(buf, 26) = skill_lvl;
							if (skill_num == 91) {
								WBUFW(buf, 28) = skill_db[skill_num].type_num * skill_lvl;
							}
							else {
								WBUFW(buf, 28) = skill_db[skill_num].type_num;
							}
							WBUFB(buf, 30) = skill_db[skill_num].type_hit;
							mmo_map_sendarea(fd, buf, packet_len_table[0x114], 0);
							skill_do_damage(fd, damage, target_id, gettick(), 0);
						}
					}
				}
			}
		}
		break;
	}
  	return 0;
}

int skill_drain_hp(int tid, unsigned int tick, int fd, int data)
{
	unsigned char buf[256];
	struct map_session_data *sd;

	if (session[fd] && (sd = session[fd]->session_data)) {
		if (sd->status.option_y == 1) {
			sd->status.hp -= 4;

			if (sd->status.hp <= 0) {
				sd->status.hp = 0;
				sd->sitting = 1;
				WBUFW(buf, 0) = 0x80;
				WBUFL(buf, 2) = sd->account_id;
				WBUFB(buf, 6) = 1;
				mmo_map_sendarea(sd->fd, buf, packet_len_table[0x80], 0);
				skill_reset_stats(0, 0, fd, 52);
			}
			WFIFOW(fd, 0) = 0xb0;
			WFIFOW(fd, 2) = 0005;
			WFIFOL(fd, 4) = sd->status.hp;
			WFIFOSET(fd, packet_len_table[0xb0]);
		}
		sd->drain_hp = add_timer(tick + 1 * 3000, skill_drain_hp, fd, 0);
	}
	return 0;
}

int skill_drain_sp(int tid, unsigned int tick, int fd, int data)
{
	struct map_session_data *sd;

	if (session[fd] && (sd = session[fd]->session_data)) {
		if (sd->hidden) {
			sd->status.sp -= 1;

			if (sd->status.sp <= 0) {
				sd->status.sp = 0;
				skill_reset_stats(0, 0, fd, 51);
			}
			WFIFOW(fd, 0) = 0xb0;
			WFIFOW(fd, 2) = 0007;
			WFIFOL(fd, 4) = sd->status.sp;
			WFIFOSET(fd, packet_len_table[0xb0]);
		}
		sd->drain_sp = add_timer(tick + 1 * 1000, skill_drain_sp, fd, 0);
	}
	return 0;
}

int skill_reset_monster_stats(int tid, unsigned int tick,int m, int n)
{
	unsigned char buf[256];

	switch (map_data[m].npc[n]->skilldata.skill_num) {
		case 5: // BASH + FATAL BLOW
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[5-1][0], skill_reset_monster_stats);
			WBUFW(buf, 0) = 0x119;
		   	WBUFL(buf, 2) = map_data[m].npc[n]->id;
		    	WBUFW(buf, 6) = 0;
		   	WBUFW(buf, 8) = 0;
		    	WBUFW(buf, 10) = 0;
		    	WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
			map_data[m].npc[n]->skilldata.effect &= 0xdf;
			map_data[m].npc[n]->u.mons.speed = (short)((float)mons_data[map_data[m].npc[n]->class].speed * (float)(90 + rand() % 21) / 100.0); // all monsters will not have same speed (+/- 10%)
			break;
		case 15: // FROST DIVER
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[15-1][0], skill_reset_monster_stats);
			WBUFW(buf, 0) = 0x119;
		   	WBUFL(buf, 2) = map_data[m].npc[n]->id;
		    	WBUFW(buf, 6) = 0;
		   	WBUFW(buf, 8) = 0;
		    	WBUFW(buf, 10) = 0;
		    	WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
			map_data[m].npc[n]->skilldata.effect &= 0x7f;
			map_data[m].npc[n]->u.mons.speed = mons_data[map_data[m].npc[n]->class].speed;
			map_data[m].npc[n]->u.mons.def1 = mons_data[map_data[m].npc[n]->class].def1;
			map_data[m].npc[n]->u.mons.def2 = mons_data[map_data[m].npc[n]->class].def2;
			break;
		case 16: // STONE CURSE
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[16-1][0], skill_reset_monster_stats);
			map_data[m].npc[n]->u.mons.def1 = mons_data[map_data[m].npc[n]->class].def1;
			break;
		case 29: // INCREASE AGI
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[29-1][0], skill_reset_monster_stats);
			map_data[m].npc[n]->u.mons.speed = (short)((float)mons_data[map_data[m].npc[n]->class].speed * (float)(90 + rand() % 21) / 100.0); // all monsters will not have same speed (+/- 10%)
        		break;
		case 30: // DECREASE AGI
          		delete_timer(map_data[m].npc[n]->skilldata.skill_timer[30-1][0], skill_reset_monster_stats);
			map_data[m].npc[n]->u.mons.speed = (short)((float)mons_data[map_data[m].npc[n]->class].speed * (float)(90 + rand() % 21) / 100.0); // all monsters will not have same speed (+/- 10%)
        		break;
		case 51: // HIDDEN
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[51-1][0], skill_reset_monster_stats);
			WBUFW(buf, 0) = 0x119;
		   	WBUFL(buf, 2) = map_data[m].npc[n]->id;
		    	WBUFW(buf, 6) = 0;
		   	WBUFW(buf, 8) = 0;
		    	WBUFW(buf, 10) = 0;
		    	WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
			map_data[m].npc[n]->option = 0|0|0;
			map_data[m].npc[n]->hidden = 0;
			break;
		case 52: // ENVENOM
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[52-1][0], skill_reset_monster_stats);
			WBUFW(buf, 0) = 0x119;
		   	WBUFL(buf, 2) = map_data[m].npc[n]->id;
		    	WBUFW(buf, 6) = 0;
		   	WBUFW(buf, 8) = 0;
		    	WBUFW(buf, 10) = 0;
		    	WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
			map_data[m].npc[n]->option = 0|0|0;
			map_data[m].npc[n]->skilldata.effect &= 0xbf;
			break;
		case 76: // LEX DIVINA
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[76-1][0], skill_reset_monster_stats);
			WBUFW(buf, 0) = 0x119;
		   	WBUFL(buf, 2) = map_data[m].npc[n]->id;
		    	WBUFW(buf, 6) = 0;
		   	WBUFW(buf, 8) = 0;
		    	WBUFW(buf, 10) = 0;
		    	WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
			map_data[m].npc[n]->skilldata.effect &= 0xef;
			break;
		case 110: // HAMMER FALL
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[110-1][0], skill_reset_monster_stats);
			WBUFW(buf, 0) = 0x119;
		   	WBUFL(buf, 2) = map_data[m].npc[n]->id;
		    	WBUFW(buf, 6) = 0;
		   	WBUFW(buf, 8) = 0;
		    	WBUFW(buf, 10) = 0;
		    	WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
			map_data[m].npc[n]->skilldata.effect &= 0xdf;
			map_data[m].npc[n]->u.mons.speed = (short)((float)mons_data[map_data[m].npc[n]->class].speed * (float)(90 + rand() % 21) / 100.0); // all monsters will not have same speed (+/- 10%)
			break;
		case 136: // SONIC BLOW
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[136-1][0], skill_reset_monster_stats);
			WBUFW(buf, 0)=0x119;
		   	WBUFL(buf, 2)=map_data[m].npc[n]->id;
		    	WBUFW(buf, 6)=0;
		   	WBUFW(buf, 8)=0;
		    	WBUFW(buf, 10)=0;
		    	WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
			map_data[m].npc[n]->skilldata.effect &= 0xdf;
			map_data[m].npc[n]->u.mons.speed = (short)((float)mons_data[map_data[m].npc[n]->class].speed * (float)(90 + rand() % 21) / 100.0); // all monsters will not have same speed (+/- 10%)
			break;
		case 152: // STONE FLING
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[152-1][0], skill_reset_monster_stats);
			WBUFW(buf, 0) = 0x119;
		   	WBUFL(buf, 2) = map_data[m].npc[n]->id;
		    	WBUFW(buf, 6) = 0;
		   	WBUFW(buf, 8) = 0;
		    	WBUFW(buf, 10) = 0;
		    	WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
			map_data[m].npc[n]->skilldata.effect &= 0xdf;
			map_data[m].npc[n]->u.mons.speed = (short)((float)mons_data[map_data[m].npc[n]->class].speed * (float)(90 + rand() % 21) / 100.0); // all monsters will not have same speed (+/- 10%)
			break;
		case 149: // SAND ATTACK
			delete_timer(map_data[m].npc[n]->skilldata.skill_timer[149-1][0],skill_reset_monster_stats);
			WBUFW(buf, 0) = 0x119;
		   	WBUFL(buf, 2) = map_data[m].npc[n]->id;
		    	WBUFW(buf, 6) = 0;
		   	WBUFW(buf, 8) = 0;
		    	WBUFW(buf, 10) = 0;
		    	WBUFB(buf, 12) = 0;
			mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
			map_data[m].npc[n]->skilldata.effect &= 0xfb;
			break;
	}
	return 0;
}

int skill_reset_stats(int tid, unsigned int tick, int fd, int skill_num)
{
	unsigned char buf[256];
	struct map_session_data *sd = NULL;
	int i;

	if (session[fd] == NULL || (sd = session[fd]->session_data) == NULL) {
		return 0;
	}
	switch (skill_num) {
		case 8: // ENDURE
			delete_timer(sd->skill_timeamount[8-1][0], skill_reset_stats);
			sd->skill_timeamount[8-1][0] = 0;
			sd->endure = 0;
	          	skill_icon_effect(sd, 1, 0);
			break;
		case 10: // SIGHT
		case 24: // RUWACH
			delete_timer(sd->skill_timeamount[10-1][0], skill_reset_stats);
			sd->skill_timeamount[10-1][0] = 0;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
			sd->status.option_z = 0;
			sd->status.effect &= 0xf7;
	          	skill_icon_effect(sd, 151, 0);
			break;
		case 15: // FROST DIVER
			delete_timer(sd->skill_timeamount[15-1][0], skill_reset_stats);
			sd->skill_timeamount[15-1][0] = -1;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
			sd->status.option_x = 0;
			sd->status.effect &= 0x7f;
			break;
		case 29: // INCREASE AGI
			delete_timer(sd->skill_timeamount[29-1][0],skill_reset_stats);
			sd->skill_timeamount[29-1][0] = -1;
			sd->skill_timeamount[29-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			skill_icon_effect(sd, 12, 0);
			break;
		case 30: // DECREASE AGI
			delete_timer(sd->skill_timeamount[30-1][0], skill_reset_stats);
			sd->skill_timeamount[30-1][0] = -1;
			sd->skill_timeamount[30-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			break;
		case 32: //CIGNUM CRISIS
			sd->cignum_crusis = 0;
			skill_icon_effect(sd, 11, 0);
			break;
		case 33: // ANGELUS
			delete_timer(sd->skill_timeamount[33-1][0], skill_reset_stats);
			sd->skill_timeamount[33-1][0] = -1;
			sd->skill_timeamount[33-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			skill_icon_effect(sd, 9, 0);
			break;
		case 34: // BLESSING
			delete_timer(sd->skill_timeamount[34-1][0], skill_reset_stats);
			sd->skill_timeamount[34-1][0] = -1;
			sd->skill_timeamount[34-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			skill_icon_effect(sd, 10, 0);
			break;
		case 45: // IMPROVE CONCENTRATION
			delete_timer(sd->skill_timeamount[45-1][0], skill_reset_stats);
			sd->skill_timeamount[45-1][0] = -1;
			sd->skill_timeamount[45-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			skill_icon_effect( sd, 3, 0);
			break;
		case 51: // HIDDEN
			delete_timer(sd->skill_timeamount[51-1][0],skill_reset_stats);
			delete_timer(sd->drain_sp, skill_drain_sp);
			sd->skill_timeamount[51-1][0] = 0;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
			sd->hidden = 0;
			sd->drain_sp = 0;
			sd->status.option_z = 0;
			sd->speed = DEFAULT_WALK_SPEED - (sd->status.agi + sd->status.agi2) / 5;
			skill_icon_effect(sd, 4, 0);
			break;
		case 52: // ENVENOM
			delete_timer(sd->skill_timeamount[52-1][0],skill_reset_stats);
			delete_timer(sd->drain_hp, skill_drain_hp);
			sd->skill_timeamount[52-1][0] = 0;
			sd->status.option_y = 0;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
			sd->status.effect &= 0xbf;
			break;
		case 60: // TWO HAND QUICKEN
			delete_timer(sd->skill_timeamount[60-1][0], skill_reset_stats);
			sd->skill_timeamount[60-1][0] = -1;
			sd->skill_timeamount[60-1][1] = 0;
                	mmo_map_calc_status(fd, 0);
			skill_icon_effect(sd, 2, 0);
			break;
		case 61: // AUTO COUNTER
			delete_timer(sd->skill_timeamount[61-1][0], skill_reset_stats);
			sd->skill_timeamount[61-1][0] = -1;
			sd->skill_timeamount[61-1][1] = 0;
			skill_icon_effect(sd, 152, 0);
			break;
		case 66: // IMPOSITIO MAGNUS
			delete_timer(sd->skill_timeamount[66-1][0], skill_reset_stats);
			sd->skill_timeamount[66-1][0] = -1;
			sd->skill_timeamount[66-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			skill_icon_effect(sd, 15, 0);
			break;
		case 73: // KYRIE
			delete_timer(sd->skill_timeamount[73-1][0],skill_reset_stats);
			sd->skill_timeamount[73-1][0] = -1;
			sd->skill_timeamount[73-1][1] = 0;
			skill_icon_effect(sd, 19, 0);
			break;
		case 74: // MAGNIFICANT
			delete_timer(sd->skill_timeamount[74-1][0], skill_reset_stats);
			sd->skill_timeamount[74-1][0] = 0;
			skill_icon_effect(sd, 20, 0);
			break;
		case 75: // GLORIA
			delete_timer(sd->skill_timeamount[75-1][0], skill_reset_stats);
			sd->skill_timeamount[75-1][0] = -1;
			sd->skill_timeamount[75-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			skill_icon_effect(sd, 21, 0);
			break;
		case 76: // LEX DIVINA
			delete_timer(sd->skill_timeamount[76-1][0], skill_reset_stats);
			sd->skill_timeamount[76-1][0] = -1;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
			sd->status.option_y = 0;
			sd->status.effect &= 0xef;
			break;
		case 111: // ADRENALINE RUSH
			delete_timer(sd->skill_timeamount[111-1][0], skill_reset_stats);
			sd->skill_timeamount[111-1][0] = -1;
			sd->skill_timeamount[111-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			skill_icon_effect(sd, 23, 0);
			break;
		case 112: // WEAPON PERFECTION
			delete_timer(sd->skill_timeamount[112-1][0], skill_reset_stats);
      	  	sd->skill_timeamount[111-1][0] = -1;
			sd->skill_timeamount[111-1][1] = 0;
			skill_icon_effect(sd,24,0);
			break;
		case 113: // POWER THRUST
			delete_timer(sd->skill_timeamount[113-1][0], skill_reset_stats);
			sd->skill_timeamount[111-1][0] = -1;
			sd->skill_timeamount[111-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			skill_icon_effect(sd, 25, 0);
			break;
		case 114: // MAXIMIZE POWER
			delete_timer(sd->skill_timeamount[114-1][0], skill_reset_stats);
			sd->skill_timeamount[111-1][0] = -1;
			sd->skill_timeamount[111-1][1] = 0;
			skill_icon_effect(sd, 26, 0);
			break;
		case 135: // CLOAKING
			delete_timer(sd->skill_timeamount[135-1][0], skill_reset_stats);
			sd->skill_timeamount[135-1][0] = -1;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
			sd->hidden = 0;
			sd->status.option_z &= 0xfd;
	        	skill_icon_effect(sd, 5, 0);
			break;
		case 136: // SONIC BLOW
			delete_timer(sd->skill_timeamount[136-1][0],skill_reset_stats);
			sd->skill_timeamount[136-1][0] = -1;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);            	sd->status.option_x=0;
			sd->status.effect &= 0xdf;
			break;
		case 145: // FATAL BLOW
			delete_timer(sd->skill_timeamount[145-1][0], skill_reset_stats);
			sd->skill_timeamount[145-1][0] = -1;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
			sd->status.option_x = 0;
			sd->status.effect &= 0xdf;
			break;
		case 149: // SAND ATTACK
			delete_timer(sd->skill_timeamount[149-1][0], skill_reset_stats);
			sd->skill_timeamount[149-1][0] = -1;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
			sd->status.effect &= 0xfb;
			break;
		case 152: // STONE FLING
			delete_timer(sd->skill_timeamount[152-1][0], skill_reset_stats);
			sd->skill_timeamount[152-1][0] = -1;
			WBUFW(buf, 0) = 0x0119;
			WBUFL(buf, 2) = sd->account_id;
			WBUFW(buf, 6) = 0;
			WBUFW(buf, 8) = 0;
			WBUFW(buf, 10) = 0;
			WBUFB(buf, 12) = 0;
			mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
            	sd->status.option_x = 0;
			sd->status.effect &= 0xdf;
			break;
		case 155: // CRAZY UPLOAR
			delete_timer(sd->skill_timeamount[155-1][0], skill_reset_stats);
			sd->skill_timeamount[155-1][0] = -1;
			sd->skill_timeamount[155-1][1] = 0;
			mmo_map_calc_status(fd, 0);
			skill_icon_effect(sd, 30, 0);
			break;
		case 157: // ENERGY COAT
			delete_timer(sd->skill_timeamount[157-1][0], skill_reset_stats);
			sd->skill_timeamount[155-1][0] = -1;
	        sd->skill_timeamount[155-1][1] = 0;
			skill_icon_effect(sd, 31, 0);
			break;
		case 261: // Summon Spirit spheres
			sd->spheres_timeamount[0] = -1;
			for (i = 1; i <= sd->spheres; i++) {
				sd->spheres_timeamount[i-1] = sd->spheres_timeamount[i];
				sd->spheres_timeamount[i] = 0;
			}
			sd->spheres--;
			skill_callspirits(fd, sd->spheres);
			break;
	}
	return 0;
}

int square_collection(int x, int y, short map, int extend, struct npc_data * pmon[])
{
	int i, j;
	short c = 0;

	for (i = 0; i <= extend; i++) {
		for (j = 0; j < map_data[map].npc_num; j++) {
			if (map_data[map].npc[j]->block.subtype == MONS) {
				if ((map_data[map].npc[j]->x == (x + i)) || (map_data[map].npc[j]->x == (x - i))) {
					if ((map_data[map].npc[j]->y <= (y + i)) && (map_data[map].npc[j]->y >= (y - i))) {
						pmon[c++] = (struct npc_data *) map_data[map].npc[j];
						continue;
					}
				}
				if ((map_data[map].npc[j]->y == (y + i)) || (map_data[map].npc[j]->y == (y - i))) {
					if ((map_data[map].npc[j]->x <= (x + i)) && (map_data[map].npc[j]->x >= (x - i))) {
						pmon[c++] = (struct npc_data *) map_data[map].npc[j];
						continue;
					}
				}
			}
		}
	}
	return c;
}

int skill_do_delayed_target (int tid, unsigned int tick, int fd, int skill_num)
{
	//get damage, handle extra_packets, do hp update
	int damage, heal_point;
	//int skill_type;
	int forplayer = 0; //to know target
	unsigned char buf[256];
	int a = 0;
	int i, j;
	int m, n;
	int target_fd = 0;
	int target_id;
	int skill_lvl;
	int inc_bless_counter;
	struct npc_data * target [256];
	static int steal[10]={5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
	int steal_luck = 0;
	struct map_session_data *sd;
	struct map_session_data *target_sd;
	struct item tmp_item;
	int chance_base,frozen_d,stun_c,blind_c;
	int cant = 0, x = 0, y = 0, cell_range = 0;
	int temp_sp=0;
	int delay;

	if (session[fd] == NULL || (sd = session[fd]->session_data) == NULL) return 0;
	//get all the information before delay
    target_id = sd->attacktarget;
    skill_lvl = sd->skill_lvl;
	//ok done
    sd->speed = DEFAULT_WALK_SPEED; //reset to normal after casting
    sd->casting = 0;//finished casting
    target_sd = NULL;
    m = sd->mapno;
    n = mmo_map_search_monster(m,target_id);

	if (n < 0) {
		forplayer = 1;

		for (i = 5; i < FD_SETSIZE; i++) {
			if (session[i] && (target_sd = session[i]->session_data)) {
				if (target_sd->account_id == target_id) {
					target_fd = i;
					break;
				}
			}
		}
		if (i == FD_SETSIZE)
			return 0;
	}

    //  target_fd != 0  or n<0  we have a p2p; player to player .. whatever
    //  do a check before changing on sessiondata! (it could be a mons)
    //  look resurection

    //printf ("SKILL_INF = %d %d\n", skill_db[skill_num].type_inf, skill_num);

    if (skill_db[skill_num].type_inf == 1
		|| skill_db[skill_num].type_inf == 4
		|| skill_db[skill_num].type_inf == 16 || skill_num == 263) {

		damage = skill_calc_damage (fd, skill_num, skill_lvl, target_id);

		if ((delay = skill_do_cast_delay(skill_num, skill_lvl).delay) != 0) {
			sd->skilltimer_delay = add_timer(gettick() + delay, skill_restart_cast_delay, fd, skill_num);
		}

		switch (skill_db[skill_num].type_nk) {
			case 0:
         		WFIFOW (fd, 0) = 0x114;
          		WFIFOW (fd, 2) = skill_num;
          		WFIFOL (fd, 4) = sd->account_id;
          		WFIFOL (fd, 8) = target_id;
          		WFIFOL (fd, 12) = tick;
          		WFIFOL (fd, 16) = sd->status.aspd;
          		WFIFOL (fd, 20) = map_data[m].npc[n]->u.mons.attackdelay;
          		WFIFOW (fd, 24) = damage;
          		WFIFOW (fd, 26) = skill_lvl;
          		if ((skill_num == 14) || (skill_num == 19) || (skill_num == 20) || (skill_num == 90)) { //Firebolt, Coldbolt, Thunderbolt, Earthspikes
            		WFIFOW(fd,28) = skill_db[skill_num].type_num * skill_lvl;
        		}
				else if (skill_num == 84){ //Jupitel Thunder
        			WFIFOW(fd,28) = (2 + skill_lvl);
         		}
				else if (skill_num == 13) { //Soul Strike
					switch (skill_lvl) {
						case 1: case 2:
							WFIFOW(fd,28) = 1;
							break;
						case 3: case 4:
							WFIFOW(fd,28) = 2;
							break;
						case 5: case 6:
							WFIFOW(fd,28) = 3;
							break;
						case 7: case 8:
							WFIFOW(fd,28) = 4;
							break;
						case 9: case 10:
							WFIFOW(fd,28) = 5;
							break;
						}
				}
				else if (skill_num == 267) {
					switch (skill_lvl) {
						case 1:
							WFIFOW(fd,28) = 1;
							break;
						case 2:
							if (sd->spheres == 1) WFIFOW(fd,28) = 1;
							else WFIFOW(fd,28) = 2;
							break;
						case 3:
							if (sd->spheres == 1) WFIFOW(fd,28) = 1;
							else if (sd->spheres == 2) WFIFOW(fd,28) = 2;
							else WFIFOW(fd,28) = 3;
							break;
						case 4:
							if (sd->spheres == 1) WFIFOW(fd,28) = 1;
							else if (sd->spheres == 2) WFIFOW(fd,28) = 2;
							else if (sd->spheres == 3) WFIFOW(fd,28) = 3;
							else WFIFOW(fd,28) = 4;
							break;
						case 5:
							if (sd->spheres == 1) WFIFOW(fd,28) = 1;
							else if (sd->spheres == 2) WFIFOW(fd,28) = 2;
							else if (sd->spheres == 3) WFIFOW(fd,28) = 3;
							else if (sd->spheres == 4) WFIFOW(fd,28) = 4;
							else WFIFOW(fd,28) = 5;
							break;
					}
				}
				else {
            		WFIFOW(fd,28) = skill_db[skill_num].type_num; // Single hit spells
				}
          		WFIFOB (fd, 30) = skill_db[skill_num].type_hit;   //6：単発 8：連発
          		mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x114],0);

          		skill_do_damage (fd, damage, target_id, tick,0);

	  			switch(skill_num) {
	  				case 5: //BASH +FATAL BLOW (stun effect)
	  					if(skill_lvl > 5 && sd->status.skill[145-1].lv > 0) {
	  						chance_base = rand()%100;
	  						if(skill_lvl == 6) stun_c = 95;
							else if(skill_lvl == 7) stun_c = 90;
							else if(skill_lvl == 8) stun_c = 85;
							else if(skill_lvl == 9) stun_c = 80;
							else stun_c = 75;
							if(chance_base >= stun_c){
		  						if(n > 0) { //MONSTER
		  							add_timer((unsigned int)(unsigned int)gettick() + 15000 , skill_reset_monster_stats, m, n);
		  							WFIFOW(fd,0)=0x119;
		    						WFIFOL(fd,2)=map_data[m].npc[n]->id;
		    						WFIFOW(fd,6)=3;
		   							WFIFOW(fd,8)=0;
									WFIFOW(fd,10)=0;
									WFIFOB(fd,12) = 0;
									mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
									map_data[m].npc[n]->skilldata.skill_num=skill_num;
									map_data[m].npc[n]->skilldata.fd = fd;
		    						map_data[m].npc[n]->u.mons.speed = 0;//stop monster
		    						map_data[m].npc[n]->option=3|0|0;
									map_data[m].npc[n]->skilldata.effect |= 0x20; //stun
								}
								else { //PLAYER
									WFIFOW(fd,0) = 0x119;
									WFIFOL(fd,2) = target_sd->account_id;
									WFIFOW(fd,6) = 3;
									WFIFOW(fd,8) = target_sd->status.option_y;
									WFIFOW(fd,10) = target_sd->status.option_z;
									WFIFOB(fd,12) = 0;
									mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x0119], 0);
									target_sd->speed = -1; //stop player
									target_sd->status.effect |= 0x20; //stun
									target_sd->skill_timeamount[145-1][0] = add_timer (tick + skill_calc_wait(target_fd,skill_num, skill_lvl).duration , skill_reset_stats, target_fd, skill_num);
								}
		  					}
	  					}
	  					break;

          		case 15: //FROST DIVER
          			chance_base = rand()%100;
            			frozen_d = (100-(35+(skill_lvl*3)));
            			if(chance_base >= frozen_d){
            				if(n>=0) { //MONSTER

            					WFIFOW(fd,0)=0x119;
		    				WFIFOL(fd,2)=map_data[m].npc[n]->id;
		    				WFIFOW(fd,6)=2;
		   				WFIFOW(fd,8)=0;
		    				WFIFOW(fd,10)=0;
		    				WFIFOB(fd,12) = 0;

		    				mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
						map_data[m].npc[n]->skilldata.skill_num=skill_num;
						map_data[m].npc[n]->skilldata.fd = fd;
		    				map_data[m].npc[n]->option=2|0|0;
						map_data[m].npc[n]->skilldata.effect |= 0x80;
						map_data[m].npc[n]->u.mons.speed = -1;//stop monster
						//map_data[m].npc[n]->u.mons.range = 0;
						map_data[m].npc[n]->u.mons.def1 *= (0.50);//def -50%
						map_data[m].npc[n]->u.mons.def2 *= (0.50);//def -50%
						//map_data[m].npc[n]->u.mons.mdef1 *= (1.25);//mdef +25%
						//map_data[m].npc[n]->u.mons.mdef2 *= (1.25);//mdef +25%
						map_data[m].npc[n]->skilldata.skill_timer[15-1][0] = add_timer ((unsigned int)((unsigned int)gettick() + (unsigned int)skill_calc_wait(fd, skill_num, skill_lvl).duration) , skill_reset_monster_stats, m, n);
					} else {//PLAYER
        						if(target_sd->skill_timeamount[15-1][0]>0){
          							skill_reset_stats (0, 0 , fd, 15);
        						} else {

           							WFIFOW(target_fd,0)=0x119;
		    						WFIFOL(target_fd,2)=target_id;
		    						WFIFOW(target_fd,6)=2;
		    						WFIFOW(target_fd,8)=target_sd->status.option_y;
		    						WFIFOW(target_fd,10)=target_sd->status.option_z;
		    						WFIFOB(target_fd,12) = 0;

								mmo_map_sendarea( target_fd, WFIFOP(target_fd,0), packet_len_table[0x0119], 0 );

            							target_sd->status.option_x=2;
		    						target_sd->speed = -1; //u cant move if ur frozen
		    						target_sd->status.effect |= 0x80;
		    						target_sd->skill_timeamount[15-1][0] = add_timer (tick + skill_calc_wait(target_fd,skill_num, skill_lvl).duration , skill_reset_stats, target_fd, skill_num);
							}
						}
					}

		  		break;
		 	case 136: //SONIC BLOW (stun effect)
		  		chance_base = rand()%100;
		  		stun_c = ( 100 -(10+(skill_lvl*2)) );
		  		if(chance_base >= stun_c){
					if(n>=0) { //MONSTER

		  				WFIFOW(fd,0)=0x119;
		    				WFIFOL(fd,2)=map_data[m].npc[n]->id;
		    				WFIFOW(fd,6)=3;
		   				WFIFOW(fd,8)=0;
		    				WFIFOW(fd,10)=0;
		    				WFIFOB(fd,12) = 0;
		    				mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );

						map_data[m].npc[n]->skilldata.skill_num=skill_num;
						map_data[m].npc[n]->skilldata.fd = fd;
		    				map_data[m].npc[n]->u.mons.speed = -1;//stop monster
		    				map_data[m].npc[n]->option=3|0|0;
						map_data[m].npc[n]->skilldata.effect |= 0x20; //stun

						map_data[m].npc[n]->skilldata.skill_timer[136-1][0] = add_timer((unsigned int)(unsigned int)gettick() + 15000 , skill_reset_monster_stats, m, n);

					} else { //PLAYER
						target_fd = 0;
      						target_sd = NULL;

      						// This code points to the correct target user.
      						for (i = 5; i < FD_SETSIZE; i++) {
        						if (session[i] && (target_sd = session[i]->session_data)) {
          							if (target_sd->account_id == target_id) {
            								target_fd = i;
            								break;
          							}
        						}
      						}


						WFIFOW(fd,0)=0x119;
		    				WFIFOL(fd,2)=target_sd->account_id;
		    				WFIFOW(fd,6)=3;
		   				WFIFOW(fd,8)=target_sd->status.option_y;
		    				WFIFOW(fd,10)=target_sd->status.option_z;
		    				WFIFOB(fd,12) = 0;

						mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
						target_sd->speed = -1;//stop player
						target_sd->status.effect |= 0x20;//stun
						target_sd->skill_timeamount[136-1][0] = add_timer (tick + skill_calc_wait(target_fd,skill_num, skill_lvl).duration , skill_reset_stats, target_fd, skill_num);
					}
		  		}
		  		break;
		  	case 149: //SAND ATTACK (blind effect by 15% chance)
		  		chance_base = rand()%100;
		  		blind_c = 85;
		  		if(chance_base >= blind_c){
		  			if (n>=0){ //MONSTER

		  				WFIFOW(fd,0)=0x119;
		    				WFIFOL(fd,2)=map_data[m].npc[n]->id;
		    				WFIFOW(fd,6)=6;
		   				WFIFOW(fd,8)=0;
		    				WFIFOW(fd,10)=0;
		    				WFIFOB(fd,12) = 0;
		    				mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
						map_data[m].npc[n]->skilldata.skill_num=skill_num;
						map_data[m].npc[n]->skilldata.fd = fd;
		    				map_data[m].npc[n]->option=6|0|0;
						map_data[m].npc[n]->skilldata.effect |= 0x04; //blind
						map_data[m].npc[n]->skilldata.skill_timer[149-1][0] = add_timer((unsigned int)(unsigned int)gettick() + 15000 , skill_reset_monster_stats, m, n);

					} else { //PLAYER
						target_fd = 0;
      						target_sd = NULL;

      						// This code points to the correct target user.
      						for (i = 5; i < FD_SETSIZE; i++) {
        						if (session[i] && (target_sd = session[i]->session_data)) {
          							if (target_sd->account_id == target_id) {
            								target_fd = i;
            								break;
          							}
        						}
      						}


						WFIFOW(fd,0)=0x119;
		    				WFIFOL(fd,2)=target_sd->account_id;
		    				WFIFOW(fd,6)=6;
		   				WFIFOW(fd,8)=target_sd->status.option_y;
		    				WFIFOW(fd,10)=target_sd->status.option_z;
		    				WFIFOB(fd,12) = 0;

						mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );

						target_sd->status.effect |= 0x04;//BLIND
						target_sd->skill_timeamount[149-1][0] = add_timer (tick + skill_calc_wait(target_fd,skill_num, skill_lvl).duration , skill_reset_stats, target_fd, skill_num);
					}


				}
		  		break;
			case 152: //STONE FLING (stun effect)
				chance_base = rand()%100;
				stun_c = 95;
				if(chance_base >= stun_c){
					if (n>=0){ //monster

						WFIFOW(fd,0)=0x119;
		  				WFIFOL(fd,2)=map_data[m].npc[n]->id;
		  				WFIFOW(fd,6)=3;
		  				WFIFOW(fd,8)=0;
		 				WFIFOW(fd,10)=0;
		    				WFIFOB(fd,12) = 0;
		    				mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
						map_data[m].npc[n]->skilldata.skill_num=skill_num;
						map_data[m].npc[n]->skilldata.fd = fd;
		    				map_data[m].npc[n]->u.mons.speed = -1;//stop monster
		    				map_data[m].npc[n]->option=3|0|0;
						map_data[m].npc[n]->skilldata.effect |= 0x20; //stun
						map_data[m].npc[n]->skilldata.skill_timer[152-1][0] = add_timer((unsigned int)(unsigned int)gettick() + 15000 , skill_reset_monster_stats, m, n);
					} else { //PLAYER

						target_fd = 0;
      						target_sd = NULL;

      						// This code points to the correct target user.
      						for (i = 5; i < FD_SETSIZE; i++) {
        						if (session[i] && (target_sd = session[i]->session_data)) {
          							if (target_sd->account_id == target_id) {
            								target_fd = i;
            								break;
          							}
        						}
      						}


						WFIFOW(fd,0)=0x119;
							WFIFOL(fd,2)=target_sd->account_id;
							WFIFOW(fd,6)=3;
						WFIFOW(fd,8)=target_sd->status.option_y;
							WFIFOW(fd,10)=target_sd->status.option_z;
							WFIFOB(fd,12) = 0;

						mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
						target_sd->speed = -1;//stop player
						target_sd->status.effect |= 0x20;//stun
						target_sd->skill_timeamount[152-1][0] = add_timer (tick + skill_calc_wait(target_fd,skill_num, skill_lvl).duration , skill_reset_stats, target_fd, skill_num);
					}
				}
				break;
			case 263: //TRIPLE ATTACK
				delay = 1300;
				skill_combodelay(fd, delay);
				sd->skill_timeamount[263-1][0] = tick + delay;
				break;
			case 266: //OCCULT IMPACT
				for (j = 1; j <= sd->spheres; j++) {
					sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
					sd->spheres_timeamount[j] = -1;
				}
				sd->spheres--;
				skill_callspirits(fd, sd->spheres);
				break;
			case 267: //THROW SPIRITS SPHERES
				i = 0;
				switch (skill_lvl) {
					case 1:
						do {
							for (j = 1; j <= 5; j++) {
								sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
								sd->spheres_timeamount[j] = -1;
							}
							i++;
						} while (i == 1);
						sd->spheres--;
						break;
					case 2:
						do {
							for (j = 1; j <= 5; j++) {
								sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
								sd->spheres_timeamount[j] = -1;
							}
							i++;
						} while (i == 2);
						sd->spheres -= 2;
						break;
					case 3:
						do {
							for (j = 1; j <= 5; j++) {
								sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
								sd->spheres_timeamount[j] = -1;
							}
							i++;
						} while (i == 3);
						sd->spheres -= 3;
						break;
					case 4:
						do {
							for (j = 1; j <= 5; j++) {
								sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
								sd->spheres_timeamount[j] = -1;
							}
							i++;
						} while (i == 4);
						sd->spheres -= 4;
						break;
					case 5:
						do {
							for (j = 1; j <= 5; j++) {
								sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
								sd->spheres_timeamount[j] = -1;
							}
							i++;
						} while (i == 5);
						sd->spheres = 0;
						break;
				}
				skill_callspirits(fd, sd->spheres);
				break;
			}
        break;

	case 1:
		heal_point = 0;
		switch (skill_num) {
			case 6: // PROVOKE
				skill_do_damage(fd, 0, target_id, gettick(), 0);
				if (n > 0) {
					map_data[m].npc[n]->u.mons.atk1 = (map_data[m].npc[n]->u.mons.atk1 * (100 + (skill_lvl * 2))) / 100;
					map_data[m].npc[n]->u.mons.atk2 = (map_data[m].npc[n]->u.mons.atk2 * (100 + (skill_lvl * 2))) / 100;
					map_data[m].npc[n]->u.mons.def1 = (map_data[m].npc[n]->u.mons.def1 * (100 - (skill_lvl * 6))) / 100;
					map_data[m].npc[n]->u.mons.def2 = (map_data[m].npc[n]->u.mons.def2 * (100 - (skill_lvl * 6))) / 100;
					if (map_data[m].npc[n]->u.mons.target_fd == 0)
						check_new_target_monster(m, n, fd);
					else
						map_data[m].npc[n]->u.mons.target_fd = fd;
				}
				break;
			case 8: // ENDURE
				if (sd->skill_timeamount[8-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->endure = 1;
				sd->skill_timeamount[8-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				skill_icon_effect(sd, 1, 1);
				break;

			case 10: // SIGHT
				if (sd->skill_timeamount[10-1][0] > 0)
					skill_reset_stats(0, 0, fd, 10);

				sd->status.option_z = 1;
				memset(buf, 0, packet_len_table[0x119]);
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = sd->account_id;
				WBUFW(buf, 6) = 0;
				WBUFW(buf, 8) = 0;
				WBUFW(buf,10) = sd->status.option_z;
				WBUFB(buf, 12) = 0;
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);

				for (i = 5; i < FD_SETSIZE; i++) {
					if (!session[i] || !(target_sd = session[i]->session_data))
						continue;

					target_sd = session[i]->session_data;
					target_fd = i;

					if (target_sd->hidden && target_sd->mapno == sd->mapno
					&& target_sd->x >= sd->x - 10 && target_sd->x <= sd->x + 10
					&& target_sd->y >= sd->y - 10 && target_sd->y <= sd->y + 10)
						skill_reset_stats(0, 0, target_fd, 51);
				}
				for (j = 0; j < map_data[m].npc_num; j++) {
					if (map_data[m].npc[j]->block.subtype == MONS) {
						if (map_data[m].npc[j]->hidden
						&& map_data[m].npc[j]->x >= sd->x - 10 && map_data[m].npc[j]->x <= sd->x + 10
						&& map_data[m].npc[j]->y >= sd->y - 10 && map_data[m].npc[j]->y <= sd->y + 10) {
							map_data[m].npc[j]->skilldata.skill_num = 51;
							skill_reset_monster_stats(0, 0, m, j);
						}
					}
				}
				sd->skill_timeamount[10-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
		          	skill_icon_effect(sd, 150, 1);
				break;
			case 24: // RUWACH
				if (sd->skill_timeamount[10-1][0] > 0)
					skill_reset_stats(0, 0, fd, 10);

				sd->status.option_z = 8192;
				memset(buf, 0, packet_len_table[0x119]);
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = sd->account_id;
				WBUFW(buf, 6) = 0;
				WBUFW(buf, 8) = 0;
				WBUFW(buf,10) = sd->status.option_z;
				WBUFB(buf, 12) = 0;
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);

				for (i = 5; i < FD_SETSIZE; i++) {
					if (!session[i] || !(target_sd = session[i]->session_data))
						continue;

					target_sd = session[i]->session_data;
					target_fd = i;

					if (target_sd->hidden && target_sd->mapno == sd->mapno
					&& target_sd->x >= sd->x - 10 && target_sd->x <= sd->x + 10
					&& target_sd->y >= sd->y - 10 && target_sd->y <= sd->y + 10)
						skill_reset_stats(0, 0, target_fd, 51);
				}
				for (j = 0; j < map_data[m].npc_num; j++) {
					if (map_data[m].npc[j]->block.subtype == MONS) {
						if (map_data[m].npc[j]->hidden
						&& map_data[m].npc[j]->x >= sd->x - 10 && map_data[m].npc[j]->x <= sd->x + 10
						&& map_data[m].npc[j]->y >= sd->y - 10 && map_data[m].npc[j]->y <= sd->y + 10) {
							map_data[m].npc[j]->skilldata.skill_num = 51;
							skill_reset_monster_stats(0, 0, m, j);
						}
					}
				}
				sd->skill_timeamount[10-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
		          	skill_icon_effect(sd, 151, 1);
				break;
			case 26: // TELEPORT
				if (sd->status.skill[skill_num-1].lv == 1) {
					WFIFOW(fd, 0)= 0x11c;
					WFIFOW(fd, 2)= skill_num;
					strcpy(WFIFOP(fd, 4), "Random");
					WFIFOL(fd, 20) = 0;
					WFIFOL(fd, 36) = 0;
					WFIFOL(fd, 52) = 0;
					WFIFOSET(fd, 68);
				}
				else {
					WFIFOW(fd, 0) = 0x11c;
					WFIFOW(fd, 2) = skill_num;
					strcpy(WFIFOP(fd, 4), "Random");
					strcpy(WFIFOP(fd, 20), sd->status.save_point.map);
					WFIFOL(fd, 36) = 0;
					WFIFOL(fd, 52) = 0;
					WFIFOSET(fd, 68);
				}
				break;
			case 28: // HEAL
				if (forplayer) {
					if (target_sd->status.hp < 0)
						break;

					heal_point = damage * -1;
					memset(buf, 0, packet_len_table[0x11a]);
					WBUFW(buf, 0) = 0x11a;
					WBUFW(buf, 2) = skill_num;
					WBUFW(buf, 4) = heal_point;
					WBUFL(buf, 6) = target_id;
					WBUFL(buf, 10) = sd->account_id;
					WBUFB(buf, 14) = 1;
					mmo_map_sendarea(target_fd, buf, packet_len_table[0x11a], 0);

					target_sd->status.hp -= damage;
					if (target_sd->status.hp < 0)
						target_sd->status.hp = 0;

					if ((target_sd->status.hp > target_sd->status.max_hp) || ((target_sd->status.hp <= 0) && (damage < 0)))
						target_sd->status.hp = target_sd->status.max_hp;

					WFIFOW(target_fd, 0) = 0xb0;
					WFIFOW(target_fd, 2) = 0005;
					WFIFOL(target_fd, 4) = target_sd->status.hp;
					WFIFOSET(target_fd, 8);
				}
				else
					skill_do_damage(fd, damage, target_id, tick, 1);

      			break;
			case 29: // INCREASE AGI
				if (forplayer) {
					if (target_sd->skill_timeamount[29-1][0] > 0)
						skill_reset_stats(0, 0, fd, 29);


					WFIFOW(fd, 0) = 0x141;
					WFIFOL(fd, 2) = SP_AGI;
					WFIFOL(fd, 6) = sd->status.agi;
					WFIFOL(fd, 10) = sd->status.agi2;
					WFIFOSET(fd, packet_len_table[0x141]);
					target_sd->skill_timeamount[29-1][1] = 2 + skill_lvl;
					target_sd->skill_timeamount[29-1][0] = add_timer(gettick() + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					skill_icon_effect(target_sd, 12, 1);
					mmo_map_calc_status(fd, 0);
				}
				break;
			case 30: // DECREASE AGI
				if (n > 0) {
					if (map_data[m].npc[n]->skilldata.skill_timer[30-1][0] > 0)
						skill_reset_monster_stats(gettick(), 0, m, n);

					if (mons_data[map_data[m].npc[n]->class].speed != 0)
						map_data[m].npc[n]->u.mons.speed = map_data[m].npc[n]->u.mons.speed * 125 / 100;

					map_data[m].npc[n]->skilldata.skill_num = 30;
					map_data[m].npc[n]->skilldata.skill_timer[30-1][0] = add_timer((unsigned int)(gettick() + (unsigned int)skill_calc_wait(fd, skill_num, skill_lvl).duration), skill_reset_monster_stats, m, n);
				}
				break;
			case 31: // HOLY WATER
				memset(&tmp_item, 0, sizeof(tmp_item));
				tmp_item.nameid =523;
				tmp_item.amount = 1;
				tmp_item.identify = 1;
				mmo_map_get_item(fd, &tmp_item);
				break;
			case 32: // CIGNUM CRUSIS
				if (sd->cignum_crusis) {
					sd->cignum_crusis = 0;
					skill_icon_effect(sd, 11, 0);
				}
				sd->cignum_crusis = 1;
				skill_icon_effect(sd, 11, 1);
				break;
			case 33: // ANGELUS
				if (sd->status.party_status > 0) {
					for (i = 5; i < FD_SETSIZE; i++) {
						if (!session[i] || !(target_sd = session[i]->session_data))
							continue;

						target_sd = session[i]->session_data;
						target_fd = i;
						if (target_sd->status.party_id == sd->status.party_id) {
							if ((target_sd->mapno == sd->mapno) && (target_sd->x >= sd->x - 13) && (target_sd->x <= sd->x + 13) && (target_sd->y >= sd->y - 10) && (target_sd->y <= sd->y + 10)) {
								if (target_sd->skill_timeamount[33-1][0] > 0)
									skill_reset_stats(0, 0, target_fd, 33);

								else {
									WBUFW(buf, 0) = 0x11a;
									WBUFW(buf, 2) = skill_num;
									WBUFW(buf, 4) = heal_point;
									WBUFL(buf, 6) = target_sd->account_id;
									WBUFL(buf, 10) = target_sd->account_id;
									WBUFB(buf, 14) = 1;
									mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
									target_sd->skill_timeamount[33-1][1] = (sd->status.vit + sd->status.vit2) * (0.10 + (skill_lvl * 0.05));
									target_sd->skill_timeamount[33-1][0] = add_timer(gettick() + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
									mmo_map_calc_status(target_fd, 0);
									skill_icon_effect(target_sd, 9, 1);
								}
							}
						}
					}
				}
				else {
					if (sd->skill_timeamount[33-1][0] > 0)
						skill_reset_stats(0, 0, fd, 33);

					else {
						WBUFW(buf, 0) = 0x11a;
						WBUFW(buf, 2) = skill_num;
						WBUFW(buf, 4) = heal_point;
						WBUFL(buf, 6) = sd->account_id;
						WBUFL(buf, 10) = sd->account_id;
						WBUFB(buf, 14) = 1;
						mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
						sd->skill_timeamount[33-1][1] = (sd->status.vit + sd->status.vit2) * (0.10 + (skill_lvl * 0.05));
						sd->skill_timeamount[33-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
						mmo_map_calc_status(fd, 0);
						skill_icon_effect(sd, 9, 1);
					}
				}
				break;
			case 34: // BLESSING
				if (forplayer) {
					if (target_sd->skill_timeamount[34-1][0] > 0)
						skill_reset_stats(0, 0 , fd, 34);

					for (inc_bless_counter = 0; inc_bless_counter < skill_lvl; inc_bless_counter++) {
						target_sd->skill_timeamount[34-1][1]++;
					}
					target_sd->skill_timeamount[34-1][0] = add_timer(gettick() + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					mmo_map_calc_status(target_fd, 0);
					skill_icon_effect(target_sd, 10, 1);
				}
				break;
			case 35: // CURE
				if (forplayer) {
					if (target_sd->status.effect & 0x40)
						skill_reset_stats(0, 0, target_fd, 52);
				}
				break;
			case 40: // ITEM APPRRAISSAL
				WFIFOW(fd, 0) = 0x177;
				for (i = j = 0; i < 100; i++) {
					if (sd->status.inventory[i].identify != 1) {
						WFIFOW(fd, 4 + j * 2) = i + 2;
						j++;
					}
				}
				WFIFOW(fd, 2) = 4 + j * 2;
				WFIFOSET(fd, 4 + j * 2);
				break;
			case 41: // VENDING
				WFIFOW(fd, 0) = 0x12d;
				WFIFOW(fd, 2) = 2 + skill_lvl;
				WFIFOSET(fd, packet_len_table[0x12d]);
				break;
			case 45: // IMPROVE CONCENTRATION
				for (i = 5; i < FD_SETSIZE; i++) {
					if (!session[i] || !(target_sd = session[i]->session_data))
						continue;

					target_sd = session[i]->session_data;
					target_fd = i;
					if (target_sd->hidden && target_sd->mapno == sd->mapno
					&& target_sd->x >= sd->x - 3 && target_sd->x <= sd->x + 3
					&& target_sd->y >= sd->y - 3 && target_sd->y <= sd->y + 3)
						skill_reset_stats(0, 0, target_fd, 51);
				}
				if (sd->skill_timeamount[45-1][0] > 0)
					skill_reset_stats(0, 0, fd, 45);

				if (skill_lvl < 4)
					sd->skill_timeamount[45-1][1]= 2 + skill_lvl;

				else
					sd->skill_timeamount[45-1][1]= 1 + skill_lvl;

				sd->skill_timeamount[45-1][0] = add_timer(gettick() + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
				mmo_map_calc_status(target_fd, 0);
				skill_icon_effect(sd, 3, 1);
				break;
			case 50: // STEAL
				if (n > 0) {
					if (rand() % 100 >= steal[sd->status.skill[50-1].lv - 1] && map_data[m].npc[n]->u.mons.steal == 0)
						mmo_map_item_steal(fd, m, n);

					else {
						WBUFW(buf, 0) = 0x11a;
						WBUFW(buf, 2) = skill_num;
						WBUFW(buf, 4) = 0;
						WBUFL(buf, 6) = sd->account_id;
						WBUFL(buf, 10) = sd->account_id;
						WBUFB(buf, 14) = 1;
						mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);

						if (map_data[m].npc[n]->u.mons.target_fd != fd)
							check_new_target_monster(m, n, fd);
					}
				}
				break;
			case 51: // HIDE
				for(i = 5; i < FD_SETSIZE; i++) {
					if (!session[i] || !(target_sd = session[i]->session_data))
						continue;

					target_sd = session[i]->session_data;
					target_fd = i;

					if ((target_sd->status.option_z == 1 || target_sd->status.option_z == 8192) && target_sd->mapno == sd->mapno
					&& target_sd->x >= sd->x -10 && target_sd->x <= sd->x +10
					&& target_sd->y >= sd->y -10 && target_sd->y <= sd->y +10) {
						cant = 1;
						break;
					}
				}
				if (sd->skill_timeamount[51-1][0] > 0)
					skill_reset_stats (0, 0 , fd, 51);

				else if (cant)
					break;

				else {
					sd->status.option_z = 2;
			    		WBUFW(buf, 0) = 0x119;
					WBUFL(buf, 2) = sd->account_id;
					WBUFW(buf, 6) = 0;
					WBUFW(buf, 8) = 0;
					WBUFW(buf, 10) = sd->status.option_z;
					WBUFB(buf, 12) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
       				sd->hidden = 1;
       				sd->speed = 0;
					sd->drain_sp = add_timer(tick + 1 * 1000, skill_drain_sp, fd, 0);
        				sd->skill_timeamount[51-1][0] = add_timer(tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
					skill_icon_effect(sd, 4, 1);
    				}
	   			break;
			case 52: // ENVENOM
				skill_do_damage (fd, damage, target_id, tick, 0);
				if (forplayer) {
		  			target_sd->status.option_y = 1;
			    		WBUFW(buf, 0) = 0x119;
					WBUFL(buf, 2) = target_sd->account_id;
					WBUFW(buf, 6) = 0;
					WBUFW(buf, 8) = target_sd->status.option_y;
					WBUFW(buf, 10) = 0;
					WBUFB(buf, 12) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
			    		target_sd->status.effect |= 0x40;
					target_sd->drain_hp = add_timer(tick + 1 * 3000, skill_drain_hp, target_fd, 0);
		  			target_sd->skill_timeamount[52-1][0] = add_timer(tick + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
				}
				else {
			    		WBUFW(buf, 0) = 0x119;
					WBUFL(buf, 2) = map_data[m].npc[n]->id;
					WBUFW(buf, 6) = 0;
					WBUFW(buf, 8) = 1;
					WBUFW(buf, 10) = 0;
					WBUFB(buf, 12) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x0119], 0);
					map_data[m].npc[n]->option = 0|1|0;
					map_data[m].npc[n]->skilldata.effect |= 0x40;
					map_data[m].npc[n]->skilldata.skill_num = skill_num;
					map_data[m].npc[n]->skilldata.skill_timer[52-1][0] = add_timer((unsigned int)((unsigned int)gettick() + (unsigned int)skill_calc_wait(fd, skill_num, skill_lvl).duration) , skill_reset_monster_stats, m, n);
				}
				break;
			case 54: // RESURECTION
				if (forplayer) {
					damage *= (((target_sd->status.max_hp * 16.6 * (skill_lvl - 1)) / 100) + 1);
					skill_do_damage(fd, damage, target_id, gettick(), 0);
					WBUFW(buf, 0) = 0x148;
					WBUFL(buf, 2) = target_id;
					mmo_map_sendarea(fd, buf, packet_len_table[0x148], 0);

					WBUFW(buf, 0) = 0x11a;
					WBUFW(buf, 2) = skill_num;
					WBUFW(buf, 4) = heal_point;
					WBUFL(buf, 6) = target_id;
					WBUFL(buf, 10) = sd->account_id;
					WBUFB(buf, 14) = 1;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
				}
				else {
					if (rand() % 10 == 5) {
						if (mons_data[map_data[m].npc[n]->class].ele % 10 == 8 || mons_data[map_data[m].npc[n]->class].ele % 10 == 9)
							skill_do_damage(fd, map_data[m].npc[n]->u.mons.hp, target_id, tick, 0);
					}
				}
				break;
			case 60: // TWO_HAND_QUICKEN
				if (sd->skill_timeamount[60-1][0] > 0)
					skill_reset_stats(0, 0, fd, 60);

				sd->skill_timeamount[60-1][1] = 70;
				sd->skill_timeamount[60-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 2, 1);
				break;
			case 61: // AUTO COUNTER
				if (sd->skill_timeamount[61-1][0] > 0)
					skill_reset_stats(0, 0, fd, 61);

				sd->skill_timeamount[61-1][1] = sd->status.critical;
				sd->skill_timeamount[61-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				skill_icon_effect(sd, 152, 1);
				break;
			case 66: // IMPOSITIO MANGUS
				if (forplayer) {
					if (target_sd->skill_timeamount[66-1][0] > 0)
						skill_reset_stats(0, 0, fd, 66);

					target_sd->skill_timeamount[66-1][1] = 5 * skill_lvl;
					target_sd->skill_timeamount[66-1][0] = add_timer(gettick() + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					mmo_map_calc_status(target_fd, 0);
					skill_icon_effect(target_sd, 15, 1);
				}
				break;
			case 72: // STATUS RECOVERY
				if (forplayer) {
					WBUFW(buf, 0) = 0x119;
					WBUFL(buf, 2) = target_sd->account_id;
					WBUFW(buf, 6) = 0;
					WBUFW(buf, 8) = 0;
					WBUFW(buf, 10) = 0;
					WBUFB(buf, 12) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
					target_sd->status.effect = 00000000;
					target_sd->status.option_x = 0;
					target_sd->status.option_y = 0;
					target_sd->status.option_z = 0;
				}
				break;
        case 73: //KYRIE
        	if(sd->status.party_status>0){ //PARTY
	    		for(i=5;i<=users_global+5;i++){
				if(!session[i] || !(target_sd=session[i]->session_data)) continue;
				target_sd=session[i]->session_data;
				target_fd=i;
				if(target_sd->status.party_id == sd->status.party_id) {
        				if((target_sd->mapno == sd->mapno) && (target_sd->x >= sd->x -13) && (target_sd->x <= sd->x +13) && (target_sd->y >= sd->y -10) && (target_sd->y <= sd->y +10)) {

		        			if(target_sd->skill_timeamount[73-1][0]>0){
							skill_reset_stats(0,0,target_fd,73);
						}
      						WFIFOW (fd, 0) = 0x11a;
          					WFIFOW (fd, 2) = skill_num;
          					WFIFOW (fd, 4) = heal_point;
          					WFIFOL (fd, 6) = target_sd->account_id;
          					WFIFOL (fd, 10) = target_sd->account_id;
         					WFIFOB (fd, 14) = 1;
          					mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a],0);

       						skill_icon_effect(target_sd,19,1);

       						if(skill_lvl <= 5) target_sd->skill_timeamount[73-1][1] = 1;
       						else if (skill_lvl >5 && skill_lvl <10) target_sd->skill_timeamount[73-1][1] = 2;
       						else target_sd->skill_timeamount[73-1][1] = 3;
       						target_sd->skill_timeamount[73-1][0] = add_timer (tick + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);


					}
				}
			}

		} else {//ONLY CASTER

		        if(sd->skill_timeamount[73-1][0]>0){
				delete_timer(sd->skill_timeamount[73-1][0], skill_reset_stats);
         		}

			WFIFOW (fd, 0) = 0x11a;
          		WFIFOW (fd, 2) = skill_num;
          		WFIFOW (fd, 4) = heal_point;
          		WFIFOL (fd, 6) = sd->account_id;
          		WFIFOL (fd, 10) = sd->account_id;
         		WFIFOB (fd, 14) = 1;
          		mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a],0);

          		skill_icon_effect(sd,19,1);

       			if(skill_lvl < 6) target_sd->skill_timeamount[73-1][1] = 1;
       			else if (skill_lvl >=6 && skill_lvl <10) target_sd->skill_timeamount[73-1][1] = 2;
       			else target_sd->skill_timeamount[73-1][1] = 3;

       			sd->skill_timeamount[73-1][0] = add_timer (tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);

		}
        	break;
			case 74: // MAGNIFICANT
				if (sd->status.party_status > 0) {
					for (i = 5; i < FD_SETSIZE; i++) {
						if (a >= users_global) {
							break;
						}
						if (!session[i] || !(target_sd = session[i]->session_data)) {
							continue;
						}
						a++;
						target_sd = session[i]->session_data;
						target_fd = i;
						if (target_sd->status.party_id == sd->status.party_id) {
							if ((target_sd->mapno == sd->mapno) &&
							    (target_sd->x >= sd->x - 13) && (target_sd->x <= sd->x + 13) &&
							    (target_sd->y >= sd->y - 13) && (target_sd->y <= sd->y + 13)) {
								if (target_sd->skill_timeamount[74-1][0] > 0) {
									skill_reset_stats(0, 0, target_fd, skill_num);
								}
								WBUFW(buf, 0) = 0x11a;
								WBUFW(buf, 2) = skill_num;
								WBUFW(buf, 4) = heal_point;
								WBUFL(buf, 6) = target_sd->account_id;
								WBUFL(buf, 10) = target_sd->account_id;
								WBUFB(buf, 14) = 1;
								mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
								skill_icon_effect(sd, 20, 1);
								target_sd->skill_timeamount[74-1][0] = add_timer(gettick() + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
							}
						}
					}
				}
				else {
					if (sd->skill_timeamount[74-1][0] > 0) {
						skill_reset_stats(0, 0, fd, skill_num);
					}
					WBUFW(buf, 0) = 0x11a;
					WBUFW(buf, 2) = skill_num;
					WBUFW(buf, 4) = heal_point;
					WBUFL(buf, 6) = sd->account_id;
					WBUFL(buf, 10) = sd->account_id;
					WBUFB(buf, 14) = 1;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
					skill_icon_effect(sd, 20, 1);
				}
				break;
        case 75: //GLORIA
        	if(sd->status.party_status>0){ //PARTY
	    		for(i=5;i<=users_global+5;i++){
				if(!session[i] || !(target_sd=session[i]->session_data)) continue;
				target_sd=session[i]->session_data;
				target_fd=i;
				if(target_sd->status.party_id == sd->status.party_id) {
        				if((target_sd->mapno == sd->mapno) && (target_sd->x >= sd->x -13) && (target_sd->x <= sd->x +13) && (target_sd->y >= sd->y -10) && (target_sd->y <= sd->y +10)) {

		        			if(target_sd->skill_timeamount[75-1][0]>0){
							delete_timer(target_sd->skill_timeamount[75-1][0], skill_reset_stats);
         						target_sd->skill_timeamount[75-1][0] = add_timer (tick + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
						} else {
      							WFIFOW (fd, 0) = 0x11a;
          						WFIFOW (fd, 2) = skill_num;
          						WFIFOW (fd, 4) = heal_point;
          						WFIFOL (fd, 6) = target_sd->account_id;
          						WFIFOL (fd, 10) = target_sd->account_id;
         						WFIFOB (fd, 14) = 1;
          						mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a],0);

       							skill_icon_effect(target_sd,21,1);

       							target_sd->skill_timeamount[75-1][1] = 30;
       							target_sd->skill_timeamount[75-1][0] = add_timer (tick + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);

      							mmo_map_calc_status(target_fd,0); // Update the stats on the client.

						}

					}
				}
			}

		} else {//ONLY CASTER

		        if(sd->skill_timeamount[75-1][0]>0){
				delete_timer(sd->skill_timeamount[75-1][0], skill_reset_stats);
         			sd->skill_timeamount[75-1][0] = add_timer (tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
			} else {
      				WFIFOW (fd, 0) = 0x11a;
          			WFIFOW (fd, 2) = skill_num;
          			WFIFOW (fd, 4) = heal_point;
          			WFIFOL (fd, 6) = sd->account_id;
          			WFIFOL (fd, 10) = sd->account_id;
         			WFIFOB (fd, 14) = 1;
          			mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a],0);

       				skill_icon_effect(sd,21,1);

       				sd->skill_timeamount[75-1][1] = 30;
       				sd->skill_timeamount[75-1][0] = add_timer (tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);

				mmo_map_calc_status(fd,0); // Update the stats on the client.
			}

		}
        	break;
        case 76: //LEX DIVINA
        //XeL: This needs a chance of succeed formula (couldnt find it )
        	if(n>=0) { //MONSTER

            		WFIFOW(fd,0)=0x119;
		    	WFIFOL(fd,2)=map_data[m].npc[n]->id;
		    	WFIFOW(fd,6)=0;
		   	WFIFOW(fd,8)=4;
		    	WFIFOW(fd,10)=0;
		    	WFIFOB(fd,12) = 0;

		    	mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
			map_data[m].npc[n]->skilldata.skill_num=skill_num;
			map_data[m].npc[n]->skilldata.fd = fd;
		    	map_data[m].npc[n]->option=0|4|0;
			map_data[m].npc[n]->skilldata.effect |= 0x10; //silenced
			map_data[m].npc[n]->skilldata.skill_timer[76-1][0] = add_timer ((unsigned int)((unsigned int)gettick() + (unsigned int)skill_calc_wait(fd, skill_num, skill_lvl).duration) , skill_reset_monster_stats, m, n);
		} else {//PLAYER

        			if(target_sd->skill_timeamount[76-1][0]>0){
          				skill_reset_stats (0, 0 , fd, 76);
        			} else {
      					target_sd->skill_timeamount[76-1][0] = add_timer (tick + skill_calc_wait(target_fd, skill_num, skill_lvl).duration , skill_reset_stats, target_fd, skill_num);

           				WFIFOW(target_fd,0)=0x119;
		    			WFIFOL(target_fd,2)=target_id;
		    			WFIFOW(target_fd,6)=target_sd->status.option_x;
		    			WFIFOW(target_fd,8)=4;
		    			WFIFOW(target_fd,10)=target_sd->status.option_z;
		    			WFIFOB(target_fd,12) = 0;

					mmo_map_sendarea( target_fd, WFIFOP(target_fd,0), packet_len_table[0x0119], 0 );

            				target_sd->status.option_x=target_sd->status.option_x;
		    			target_sd->status.option_y=4;
		    			target_sd->status.option_z=target_sd->status.option_z;
		    			target_sd->status.effect |= 0x10;//silenced
				}

		}
        	break;
		case 77: // TURN UNDEAD
			if (n > 0) {
				if (mons_data[map_data[m].npc[n]->class].ele % 10 == 9)
					mons_data[map_data[m].npc[n]->class].ele = 0;
			}
			break;
        case 81: //SIGHT TRASHER [XeL]
        	if(skill_lvl <3) cell_range = 1;
        	else if ((skill_lvl >=3) && (skill_lvl <5)) cell_range = 2;
        	else if ((skill_lvl >=5) && (skill_lvl <7)) cell_range = 3;
        	else if ((skill_lvl >=7) && (skill_lvl <9)) cell_range = 4;
        	else cell_range = 5;

        	for (x=-cell_range;x<=cell_range;x++){
        	  for(y=-cell_range;y<=cell_range;y++){
        	     if( (x == y) || (x == -y) || (x == 0) || (y == 0) ){//star form
        	   	for (j = 0; j < map_data[sd->mapno].npc_num; j++){
      			     if (map_data[sd->mapno].npc[j]->block.subtype == MONS){
      				if (map_data[sd->mapno].npc[j]->x == (sd->x + x)){
          		      	     if (map_data[sd->mapno].npc[j]->y == (sd->y + y)){
      				         target_id=map_data[sd->mapno].npc[j]->id;
      				         damage = skill_calc_damage (fd,skill_num,skill_lvl,target_id);
      				         skill_do_damage (fd,damage,target_id,tick,0);
      				     }
      				}
      			     }
      			}
      		      }
      		   }
        	}
        	break;
      	case 93: //SENSE
      		WFIFOW(fd, 0) = 0x18c;
      		WFIFOW(fd, 2) = map_data[m].npc[n]->class;
      		WFIFOW(fd, 4) = mons_data[map_data[m].npc[n]->class].lv;
      		WFIFOW(fd, 6) = mons_data[map_data[m].npc[n]->class].scale;
      		WFIFOL(fd, 8) = map_data[m].npc[n]->u.mons.hp;
      		WFIFOW(fd,12) = map_data[m].npc[n]->u.mons.def1;
      		WFIFOW(fd,14) = mons_data[map_data[m].npc[n]->class].race;
      		WFIFOW(fd,16) = mons_data[map_data[m].npc[n]->class].mdef1;
      		WFIFOW(fd,18) = mons_data[map_data[m].npc[n]->class].ele%10;//to pick 2nd bit of "ele"
      		for(j=0;j<9;j++){
			if( (mons_data[map_data[m].npc[n]->class].ele%10) < 0 )
				WFIFOB(fd, 20+j) = 0;
			else
				WFIFOB(fd, 20+j) = 100*get_ele_attack_factor( (mons_data[map_data[m].npc[n]->class].ele%10),j+1 );
      		}
      		mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x18c],0);
      		break;
        case 111: //ADRENALINE RUSH
        	if(sd->status.party_status>0){//Party
	    		for(i=5;i<=users_global+5;i++){//number of people loop
				if(!session[i] || !(target_sd=session[i]->session_data)) continue;
				target_sd=session[i]->session_data;
				target_fd=i;
				if(target_sd->status.party_id == sd->status.party_id) {
        				if((target_sd->mapno == sd->mapno) && (target_sd->x >= sd->x -13) && (target_sd->x <= sd->x +13) && (target_sd->y >= sd->y -13) && (target_sd->y <= sd->y +13)) {
		        			if(target_sd->skill_timeamount[111-1][0]>0){
							delete_timer(target_sd->skill_timeamount[111-1][0], skill_reset_stats);
         						target_sd->skill_timeamount[111-1][0] = add_timer (tick + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
						} else {
      							WFIFOW (fd, 0) = 0x11a;
          						WFIFOW (fd, 2) = skill_num;
          						WFIFOW (fd, 4) = heal_point;
          						WFIFOL (fd, 6) = target_sd->account_id;
          						WFIFOL (fd, 10) = target_sd->account_id;
         						WFIFOB (fd, 14) = 1;
          						mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a],0);

      							skill_icon_effect(target_sd,23,1);

      							if(target_sd->account_id == sd->account_id) target_sd->skill_timeamount[111-1][1] = 2;
       							else target_sd->skill_timeamount[111-1][1] = 2;

       							target_sd->skill_timeamount[111-1][0] = add_timer (tick + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
      							mmo_map_calc_status(target_fd,0);

						}

					}
				}
			}

		} else {//No party

		        if(sd->skill_timeamount[111-1][0]>0){
				delete_timer(sd->skill_timeamount[111-1][0], skill_reset_stats);
         			sd->skill_timeamount[111-1][0] = add_timer (tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
			} else {
      				WFIFOW (fd, 0) = 0x11a;
          			WFIFOW (fd, 2) = skill_num;
          			WFIFOW (fd, 4) = heal_point;
          			WFIFOL (fd, 6) = sd->account_id;
          			WFIFOL (fd, 10) = sd->account_id;
         			WFIFOB (fd, 14) = 1;
          			mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a],0);

          			skill_icon_effect(sd,23,1);

       				sd->skill_timeamount[111-1][1] = 2;
       				sd->skill_timeamount[111-1][0] = add_timer (tick + skill_calc_wait(fd,skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);

      				mmo_map_calc_status(fd, 0);
			}

		}
        	break;
        case 112:	//WEAPON PERFECTION
        	if(sd->skill_timeamount[112-1][0] > 0) skill_reset_stats (0,0,fd,112);
        	sd->skill_timeamount[112-1][1] = 1; //enable 100% damage on every size mons.
        	skill_icon_effect(sd,24,1);
        	sd->skill_timeamount[112-1][0] = add_timer (tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
             	break;

        case 113: //POWER THRUST
         	if (sd->skill_timeamount[113-1][0] > 0)
        		skill_reset_stats(0,0,fd,113);

             	skill_icon_effect(sd,25,1);

        	sd->skill_timeamount[113-1][1] = ( 5 * skill_lvl );
        	mmo_map_calc_status(fd,0);
        	sd->skill_timeamount[113-1][0] = add_timer (tick + skill_calc_wait(fd,skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
        	break;

        case 114:	//MAXIMIZE POWER
        	if(sd->skill_timeamount[114-1][0] > 0) skill_reset_stats (0,0,fd,114);
        	sd->skill_timeamount[114-1][1] = 1;
        	skill_icon_effect(sd,26,1);
        	sd->skill_timeamount[114-1][0] = add_timer (tick + skill_calc_wait(fd,skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
           	break;

        case 135:	//CLOACK by XeL: Doesnt look for near Walls
		if(sd->skill_timeamount[135-1][0] > 0) {
	    		skill_reset_stats (0, 0 , fd, 135);
	    	}
	    	else {
	    		skill_icon_effect(sd,5,1);

	    		WFIFOW(fd,0)=0x0119;
			WFIFOL(fd,2)=sd->account_id;
			WFIFOW(fd,6)=0;
			WFIFOW(fd,8)=0;
			WFIFOW(fd,10)=2;
			WFIFOB(fd,12) = 0;

			mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x0119], 0);

			sd->status.option_z |=2;
			sd->hidden = 1;
	        	sd->skill_timeamount[135-1][0] = add_timer (tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);

	    	}
	  	break;
	case 142: //FIRST AID
		if(sd->status.hp + damage > sd->status.max_hp) {
			damage=sd->status.max_hp - sd->status.hp;
			sd->status.hp += damage;
		} else sd->status.hp += damage;

             	WFIFOW (fd, 0) = 0x11a;   //no damage skill effect
          	WFIFOW (fd, 2) = skill_num;
          	WFIFOW (fd, 4) = heal_point;
          	WFIFOL (fd, 6) = sd->account_id;
          	WFIFOL (fd, 10) = sd->account_id;
          	WFIFOB (fd, 14) = 1;
          	mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a],0);

		if(sd->status.hp >= sd->status.max_hp) sd->status.hp = sd->status.max_hp;

             	WFIFOW(fd,0) = 0xb0;
             	WFIFOW(fd,2) = 0x05;
             	WFIFOL(fd,4) = sd->status.hp;
             	WFIFOSET(fd,8);

		break;
	case 143: //ACT DEAD
		if(sd->skill_timeamount[143-1][1] == 0){
			skill_icon_effect(sd,29,1);
			sd->speed = -1;
                	sd->skill_timeamount[143-1][1] = 1;
                } else {
                	skill_icon_effect(sd,29,0);
                	sd->speed = DEFAULT_WALK_SPEED;
                	sd->skill_timeamount[143-1][1] = 0;
                }
	  	break;
	case 151: //FIND STONE
		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.nameid=7049;
		tmp_item.amount=1;
		tmp_item.identify=1;
		mmo_map_get_item(fd, &tmp_item);
		break;
	case 154: //CHANGE CART
		WFIFOW (fd, 0) = 0x11a;   //no damage skill effect
          	WFIFOW (fd, 2) = skill_num;
          	WFIFOW (fd, 4) = heal_point;
          	WFIFOL (fd, 6) = sd->account_id;
          	WFIFOL (fd, 10) = sd->account_id;
          	WFIFOB (fd, 14) = 1;
         	mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a],0);
		break;
	case 155: //CRAZY UPROAR
		if(sd->skill_timeamount[155-1][0] > 0 ) skill_reset_stats(0,0,fd,155);
		skill_icon_effect(sd,30,1);
		sd->skill_timeamount[155-1][1] = 4;
		sd->skill_timeamount[155-1][0] = add_timer (tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
		mmo_map_calc_status(fd,0);
		break;
	case 157: //ENERGY COAT
		// This code points to the correct target user.
      		for (i = 5; i < FD_SETSIZE; i++) {
        		if (session[i] && (target_sd = session[i]->session_data)) {
          			if (target_sd->account_id == target_id) {
            				target_fd = i;
            				break;
          			}
        		}
      		}
		if (target_sd->skill_timeamount[157-1][0] > 0) skill_reset_stats(0,0,target_fd,157);
		skill_icon_effect(target_sd,31,1);
		target_sd->skill_timeamount[157-1][0] = add_timer (tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
		break;
	case 261: //SUMMON SPIRIT SPHERES
		sd->spheres++;
		skill_callspirits(fd, sd->spheres);
		sd->spheres_timeamount[sd->spheres-1] = add_timer(tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
		break;
	case 262: //ABSORB SPIRITS
		switch(skill_lvl) {
			case 1: case 2: case 3: case 4: case 5:
				temp_sp = 7*sd->spheres;
				break;
		}
		if ((sd->status.sp+temp_sp) > sd->status.max_sp) {
			temp_sp = sd->status.max_sp - sd->status.sp;
			sd->status.sp = sd->status.max_sp;
		} else sd->status.sp += temp_sp;

		switch (sd->spheres) {
			case 1:
				sd->spheres_timeamount[1-1] = -1;
				break;
			case 2:
				sd->spheres_timeamount[1-1] = -1;
				sd->spheres_timeamount[2-1] = -1;
				break;
			case 3:
				sd->spheres_timeamount[1-1] = -1;
				sd->spheres_timeamount[2-1] = -1;
				sd->spheres_timeamount[3-1] = -1;
				break;
			case 4:
				sd->spheres_timeamount[1-1] = -1;
				sd->spheres_timeamount[2-1] = -1;
				sd->spheres_timeamount[3-1] = -1;
				sd->spheres_timeamount[4-1] = -1;
				break;
			case 5:
				sd->spheres_timeamount[1-1] = -1;
				sd->spheres_timeamount[2-1] = -1;
				sd->spheres_timeamount[3-1] = -1;
				sd->spheres_timeamount[4-1] = -1;
				sd->spheres_timeamount[5-1] = -1;
				break;
		}

		sd->spheres = 0;
		skill_callspirits(fd, sd->spheres);

		WFIFOW(fd, 0) = 0x13d;
		WFIFOW(fd, 2) = 7;
		WFIFOL(fd, 4) = temp_sp;
		WFIFOSET(fd, packet_len_table[0x13d]);

		mmo_map_calc_status(fd, 0);
		break;
	default:
		printf ("case_1 UNHANDLED_SKILL: %d\n", skill_num);
        break;
    }
	// printf ("case_1: %d.%d.%d\n", fd, damage, target_id);


    //R 011a <skill ID>.w <val>.w <dst ID>.w <src ID>.w <fail>.B
	if(skill_num != 28 && skill_num != 33 && skill_num != 75 && skill_num != 73 && skill_num != 111 && skill_num != 154){
		WFIFOW (fd, 0) = 0x11a;   //no damage skill effect
          	WFIFOW (fd, 2) = skill_num;
          	WFIFOW (fd, 4) = heal_point;
          	WFIFOL (fd, 6) = target_id;
          	WFIFOL (fd, 10) = sd->account_id;
          	WFIFOB (fd, 14) = 1;
          	mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a],0);
	}
	break;

        //type_nk 2
	//R 0115 <skill ID>.w <src ID>.l <dst ID>.l <server tick>.l <src speed>.l <dst speed>.l <X>.w <Y>.w <param1>.w <param2>.w <param3>.w <type>.B
        case 2:     //magnumbreak arrow_rain bowlingbash .. what else?
            switch (skill_num) {
            case 7:


                    //MAGNUM BREAK testing

            j = square_collection(sd->x,sd->y,sd->mapno,2,target); // extend 1 should be iRo .. but i seen some problems with the monsters positions .. so i give here 5x5 instead of 3x3 field
            for (i=0;i<j;i++) {

                WFIFOW (fd, 0) = 0x115;   //blow up type
                WFIFOW (fd, 2) = skill_num;
                WFIFOL (fd, 4) = sd->account_id;
                WFIFOL (fd, 8) = target[i]->id;
                WFIFOL (fd, 12) = tick;
                WFIFOL (fd, 16) = sd->status.aspd;//src_speed
                WFIFOL (fd, 20) = 250;    //dst_speed
                WFIFOW (fd, 24) = target[i]->x ;  //x
                WFIFOW (fd, 26) = target[i]->y ;  //y
                WFIFOW (fd, 28) = damage; //ダメージ合計
                WFIFOW (fd, 30) = skill_lvl; //LEVEL
                WFIFOW (fd, 32) = skill_db[skill_num].type_num;   //分割数?
                WFIFOB (fd, 34) = skill_db[skill_num].type_hit;   //type=05 ダメージ&弾き飛ばし。param1はダメージ合計、param2はlevel、param3は分割数と予想
                  // type=06 爆心地? 少なくともparam1はゴミの模様
                mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x115],0);
                  // printf ("case_2: %d.%d.%d\n", fd, damage, target_id);
                skill_do_damage (fd, damage,target[i]->id, tick,0);
                damage = skill_calc_damage (fd, skill_num, skill_lvl, target_id); // calc new damage for next monster

            }
            break;
            default:
          	WFIFOW (fd, 0) = 0x115;   //blow up type
          	WFIFOW (fd, 2) = skill_num;
         	WFIFOL (fd, 4) = sd->account_id;
          	WFIFOL (fd, 8) = target_id;
          	WFIFOL (fd, 12) = tick;
          	WFIFOL (fd, 16) = sd->status.aspd;//src_speed
          	WFIFOL (fd, 20) = map_data[m].npc[n]->u.mons.attackdelay;    //dst_speed
          	WFIFOW (fd, 24) = sd->x + 3;  //x
          	WFIFOW (fd, 26) = sd->y + 3;  //y
          	WFIFOW (fd, 28) = damage; //damage
          	WFIFOW (fd, 30) = skill_lvl; //LEVEL
          	WFIFOW (fd, 32) = skill_db[skill_num].type_num;  //number of hits
          	WFIFOB (fd, 34) = skill_db[skill_num].type_hit;
          	mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x115],0);
          	skill_do_damage (fd, damage, target_id, tick,0);
            break;
      	}
          break;
        default:
          break;
        }
    }
	else  printf("0x113:%d %d\n",skill_num, skill_db[skill_num].type_inf);

return 0;
}

void check_cast_sensor(int fd, int m, int n, int target, int range, int skill_x, int skill_y)
{
	int j,x,y;
	if(range <= 0){ //single target
		if(mons_data[map_data[m].npc[n]->class].cast_sense == 1 && map_data[m].npc[n]->u.mons.attacktimer <= 0){
			map_data[m].npc[n]->u.mons.target_fd = fd;
			if(map_data[m].npc[n]->u.mons.timer > 0)
				delete_timer(map_data[m].npc[n]->u.mons.timer,mons_walk);
			map_data[m].npc[n]->u.mons.attacktimer =add_timer(gettick() + 300, mmo_mons_attack_continue, n, m);
		}
	} else { //area skills search monster in the range of skill
		for(x=-range;x<=range;x++)
      		   for(y=-range;y<=range;y++)
      		     for (j = 0; j < map_data[m].npc_num; j++)
      		       if (map_data[m].npc[j]->block.subtype == MONS)
        		 if (map_data[m].npc[j]->x == (skill_x + x))
          		   if (map_data[m].npc[j]->y == (skill_y + y))
        		   	if(mons_data[map_data[m].npc[j]->class].cast_sense == 1 && map_data[m].npc[j]->u.mons.attacktimer <= 0){
					map_data[m].npc[j]->u.mons.target_fd = fd;
					if(map_data[m].npc[j]->u.mons.timer > 0)
						delete_timer(map_data[m].npc[n]->u.mons.timer,mons_walk);
					map_data[m].npc[j]->u.mons.attacktimer =add_timer(gettick() + 300, mmo_mons_attack_continue, j, m);

				}
	}
}

int skill_attack_place(int fd, int skill_num,int skill_lvl, unsigned long tick, int skill_x, int skill_y)
{
	int i;
	int skill_wait;
	unsigned char buf[64];
	struct map_session_data *sd = NULL;

	skill_wait = skill_calc_wait(fd, skill_num, skill_lvl).wait;

	if (session[fd] == NULL || session[fd]->session_data == NULL) {
		return 0;
	}
	sd = session[fd]->session_data;

	if((skill_can_use(skill_num, &skill_lvl, fd)) == 0) return 0;

	if (skill_num == 87) {
		for (i = -2; i <= 2; i++) {
			WBUFW(buf, 0) = 0x117;
			WBUFW(buf, 2) = skill_num;
			WBUFL(buf, 4) = sd->account_id;
			WBUFW(buf, 8) = skill_lvl;
			WBUFW(buf, 10) = skill_x + i;
			WBUFW(buf, 12) = skill_y + i;
			WBUFL(buf, 14) = tick;
			mmo_map_sendarea(fd, buf, packet_len_table[0x117], 0);
		}
		return 0;
	}
	if (skill_db[skill_num].type_inf == 2) {
		if (skill_wait > SKILL_MIN_WAIT) {
			WBUFW(buf, 0) = 0x13e;
			WBUFL(buf, 2) = sd->account_id;
			WBUFL(buf, 6) = 0;
			WBUFW(buf, 10) = skill_x;
			WBUFW(buf, 12) = skill_y;
			WBUFW(buf, 14) = skill_num;
			WBUFL(buf, 16) = skill_db[skill_num].type_pl;
			WBUFL(buf, 20) = skill_wait;
			mmo_map_sendarea(fd, buf, packet_len_table[0x13e], 0);

			sd->skill_x = skill_x;
			sd->skill_y = skill_y;
			sd->skill_lvl = skill_lvl;

			check_cast_sensor(fd, sd->mapno, 0, sd->account_id, 5, skill_x, skill_y);
			sd->speed = 0;
			sd->casting = 1;
			sd->skilltimer = add_timer(tick + skill_wait, skill_do_delayed_place, fd, skill_num);
		} else {
			skill_do_delayed_place(-1, tick, fd, skill_num);
		}
	}
	else {
		printf("0x116:%d %d\n",skill_num, skill_db[skill_num].type_inf);
	}
	return 0;
}

enum { R_CANT,R_NOSP,R_NOHP,R_NOMEMO,R_INDELAY,R_NOZENY,R_NOWEAPON,R_NOREDG,R_NOBLUEG,R_UNKN,R_NONE };
int skill_can_use(int skill_num, int *skill_lvl, int fd)
{
	int index;
	short reason = R_NONE;
	struct map_session_data *sd = NULL;

	if (session[fd] == NULL || (sd = session[fd]->session_data) == NULL) {
		return 0;
	}
	if (sd->status.hp <= 0) {
		return 0;
	}
	if (sd->status.sp >= (skill_db[skill_num].sp + extra_sp_cost(skill_num, *skill_lvl))) {
		if (sd->status.skill[skill_num-1].lv < *skill_lvl) {
			*skill_lvl = sd->status.skill[skill_num-1].lv;
		}
		switch (skill_num) {
			case 7: /* MAGNUM BREAK */
				sd->status.hp -= (20 - ((*skill_lvl - 1) / 2));
				if (sd->status.hp < 0) {
					sd->status.hp = 0;
				}
				WFIFOW(fd, 0) = 0xb0;
				WFIFOW(fd, 2) = 0005;
				WFIFOL(fd, 4) = sd->status.hp;
				WFIFOSET(fd, packet_len_table[0xb0]);
				break;

			case 16: /* STONE CURSE */
				index = search_item2(sd, 716);
				if (index) {
					mmo_map_lose_item(fd, 0, index, 1);
				}
				else {
					reason = R_NOREDG;
				}
				break;

			case 31: /* HOLY WATER */
				index = search_item2(sd, 713);
				if (index) {
					mmo_map_lose_item(fd, 0, index, 1);
				}
				else {
					reason = R_CANT;
				}
				break;

			case 41: /* VENDING */
				if (sd->status.special != 0x08) {
					if (sd->status.special != 0x80) {
						if (sd->status.special != 0x100) {
							if (sd->status.special != 0x200) {
								if (sd->status.special != 0x400) {
									reason = R_CANT;
								}
							}
						}
					}
				}
				break;

			case 42: /* MAMONITE */
				if (sd->status.zeny >= (*skill_lvl * 100)) {
					mmo_map_update_param(fd, SP_ZENY, (*skill_lvl * -100));
				}
				else {
					reason = R_NOZENY;
				}
				break;

			case 51: /* HIDE */
				if (sd->hidden == 1) {
					return 1;
				}
				break;

			case 56: /* PIERCE */
			case 58: /* SPEAR TAB */
			case 59: /* SPEAR BOOMERANG */
				if (sd->status.weapon != 4 && sd->status.weapon != 5) {
					reason = R_NOWEAPON;
				}
				break;

			case 57: /* BRANDISH SPEAR */
				if (sd->status.weapon != 4 && sd->status.weapon != 5) {
					reason = R_NOWEAPON;
				}
				if (sd->status.special != 0x20) {
					reason = R_CANT;
				}
				break;

			case 12: /* SAFETY WALL */
			case 27: /* WARP */
			case 54: /* RESURRECTION */
			case 70: /* SANCTUARY */
			case 79: /* MAGNUS EXORCISM */
			case 80: /* FIREPILLAR */
				index = search_item2(sd, 717);
				if (index) {
					mmo_map_lose_item(fd, 0, index, 1);
				}
				else {
					reason = R_NOBLUEG;
				}
				break;

			case 60: /* TWO HAND QUICKEN */
				if (sd->status.weapon != 3) {
					reason = R_NOWEAPON;
				}
				break;

 			case 129: /* BLITZBEAT */
			case 130: /* DETECT */
			case 131: /* SPRING TRAP */
				if (sd->status.special != 0x10) {
					reason = R_CANT;
				}
				break;

			case 135: /* CLOACKING */
				if (sd->hidden == 1) {
					return 1;
				}
				break;

			case 136: /* SONIC BLOW */
				if (!(sd->status.weapon == 16)) {
					reason = R_NOWEAPON;
				}
				break;

			case 137: /* GRIMTOOTH */
				if (!sd->hidden) {
					reason = R_CANT;
				}
				if (!(sd->status.weapon == 16)) {
					reason = R_NOWEAPON;
				}
				break;

			case 143: /* ACT DEAD */
				if (sd->status.hp <= 0) {
					reason = R_CANT;
				}
				if (sd->hidden == 1) {
					reason = R_CANT;
				}
				if (sd->skill_timeamount[143-1][1] == 1) {
					return 1;
				}
				break;

			case 151: /* FIND STONE */
				if (sd->weight >= (sd->max_weight / 2)) {
					reason = R_UNKN;
				}
				break;

			case 152: /* STONE FLING */
				index = search_item2(sd, 7049);
				if (index) {
					mmo_map_lose_item(fd, 0, index, 1);
				}
				else {
					reason = R_CANT;
				}
				break;
			case 261: /* SUMMON SPIRIT SPHERES */
				switch (*skill_lvl) {
					case 1: case 2: case 3: case 4: case 5:
						if (*skill_lvl == sd->spheres) {
							reason = R_CANT;
						}
					break;
				}
				break;
			case 262: /* ABSORB SPIRITS */
			case 266: /* OCCULT IMPACT */
				if (sd->spheres == 0) reason = R_CANT;
				break;
			case 267: /* THROW SPIRIT SPHERES */
				if (sd->spheres == 0) reason = R_CANT;
				break;

		}
		if (reason == R_NONE) {
			if (sd->skilltimer_delay > 0) {
				reason = R_INDELAY;
			}
			else {
				sd->status.sp -= (skill_db[skill_num].sp + extra_sp_cost(skill_num, *skill_lvl));
				WFIFOW(fd, 0) = 0xb0;
				WFIFOW(fd, 2) = 0007;
				WFIFOL(fd, 4) = sd->status.sp;
				WFIFOSET(fd, packet_len_table[0xb0]);
				return 1;
			}
		}
	}
	else {
		reason = R_NOSP;
	}
	WFIFOW(fd, 0) = 0x110;
	WFIFOW(fd, 2) = skill_num;
	WFIFOW(fd, 4) = 05;
	WFIFOW(fd, 6) = 0;
	WFIFOB(fd, 8) = 00;
	WFIFOB(fd, 9) = reason;
	WFIFOSET(fd, packet_len_table[0x110]);
	return 0;
}

int skill_attack_target(int fd, int target_id, int skill_num, int skill_lvl, unsigned long tick)
{
	int n;
	short m;
	int target;
	int skill_wait;
	unsigned char buf[256];
	struct map_session_data *sd = NULL;

	if (session[fd] == NULL || (sd = session[fd]->session_data) == NULL) {
		return 0;
	}
	target = sd->account_id;
	m = sd->mapno;
	n = mmo_map_search_monster(sd->mapno, target_id);

	if ((n < 0) && !(skill_db[skill_num].type_inf == 16 || skill_db[skill_num].type_inf == 4) && (PVP_flag == 0) && (strstr(sd->mapname, "pvp") != sd->mapname)) {
		return 0;
	}

	if((skill_can_use(skill_num, &skill_lvl, fd)) == 0) return 0;

	if (skill_db[skill_num].type_inf == 1
		|| skill_db[skill_num].type_inf == 4
		|| skill_db[skill_num].type_inf == 16) {

		skill_wait = skill_calc_wait(fd, skill_num, skill_lvl).wait;

		sd->skill_target = target_id;
		sd->skill_lvl = skill_lvl;

		if (n >= 0 && skill_num == 28 && ((mons_data[map_data[m].npc[n]->class].ele % 10 == 8) || (mons_data[map_data[m].npc[n]->class].ele % 10 == 9))) {
			skill_wait = 1000;
		}
		if (skill_wait > SKILL_MIN_WAIT) {
			WBUFW(buf, 0) = 0x13e;
			WBUFL(buf, 2) = sd->account_id;
			WBUFL(buf, 6) = target_id;
			WBUFW(buf, 10) = sd->x;
			WBUFW(buf, 12) = sd->y;
			WBUFW(buf, 14) = skill_num;
			WBUFL(buf, 16) = skill_db[skill_num].type_pl;
			WBUFL(buf, 20) = skill_wait;
			mmo_map_sendarea(fd, buf, packet_len_table[0x13e], 0);
			if (n >= 0) {
				check_cast_sensor(fd, m, n, target, 0, 0, 0);
			}
			sd->casting = 1;
			sd->skilltimer = add_timer (tick + skill_wait, skill_do_delayed_target, fd, skill_num);
		}
		else {
			skill_do_delayed_target(-1, tick, fd, skill_num);
		}
    }
  	return 0;
}

void skill_stop_wait(int fd)
{
	struct map_session_data *sd = NULL;

	if (session[fd] == NULL || (sd = session[fd]->session_data) == NULL) {
		return;
	}
	delete_timer(sd->skilltimer, skill_do_delayed_target);
	delete_timer(sd->skilltimer, skill_do_delayed_place);
	sd->casting = 0;
}

int skill_can_forge(struct map_session_data *sd, int nameid, int flag)
{
	int i, j, id, x, y;

	if (nameid <= 0) {
		return 0;
	}
	for (i = 0; i < MAX_SKILL_REFINE; i++) {
		if (forge_db[i].nameid == nameid) {
			break;
		}
	}
	if (i == MAX_SKILL_REFINE) {
		return 0;
	}
	j = forge_db[i].req_skill;
	if (j > 0 && sd->status.skill[j-1].lv <= 0) {
		return 0;
	}
	if (flag > 0) {
		if (forge_db[i].nameid < 1101 || forge_db[i].itemlv != flag) {
			return 0;
		}
	}
	else {
		if (forge_db[i].nameid > 1100) {
			return 0;
		}
	}
	for (j = 0; j < 5; j++) {
		if ((id = forge_db[i].mat_id[j]) == 0) {
			break;
		}
		for (y = 0, x = 0; y < MAX_INVENTORY; y++) {
			if (sd->status.inventory[y].nameid == id) {
				x += sd->status.inventory[y].amount;
			}
		}
		if (x < forge_db[i].mat_amount[j]) {
			return 0;
		}
	}
	return i + 1;
}

int delete_forge_material(int fd, struct map_session_data *sd, int name_id)
{
	int i;
	int i1 = 0, i2 = 0;

	i1 = forge_db_index(name_id);
	for (i = 0; i < 5; i++) {
		if (forge_db[i1].mat_id[i] != 0) {
			i2 = search_item2(sd, forge_db[i1].mat_id[i]);
			if (index) {
				mmo_map_lose_item(fd, 0, i2, forge_db[i1].mat_amount[i]);
			}
		}
	}
	return 0;
}

void skill_do_forge(int fd, struct map_session_data *sd, int name_id, int flag, int ele)
{
	struct item tmp_item;

	if (flag == 1) {
		WFIFOW(fd, 0) = 0x18f;
		WFIFOW(fd, 2) = 0;
		WFIFOW(fd, 4) = name_id;
		WFIFOSET(fd, packet_len_table[0x18f]);

		memset(&tmp_item, 0, sizeof(tmp_item));
		tmp_item.nameid = name_id;
		tmp_item.amount = 1;
		tmp_item.identify = 1;

		if (ele != 0) {
			tmp_item.card[0] = 0x00ff;
			tmp_item.card[1] = ele;
			*((unsigned long *)(&tmp_item.card[2])) = sd->char_id;
		}
		WFIFOW(fd, 0) = 0x194;
		WFIFOL(fd, 2) = sd->status.char_id;
		memcpy(WFIFOP(fd, 6), sd->status.name, 24);
		WFIFOSET(fd, packet_len_table[0x194]);
		mmo_map_get_item(fd, &tmp_item);
	}
	else {
		WFIFOW(fd, 0) = 0x18f;
		WFIFOW(fd, 2) = 1;
		WFIFOW(fd, 4) = name_id;
		WFIFOSET(fd, packet_len_table[0x18f]);
	}
	delete_forge_material(fd, sd, name_id);
}

void skill_icon_effect(struct map_session_data *sd, int icon, int flag)
{
	int fd = sd->fd;

	WFIFOW(fd, 0) = 0x196;
	WFIFOW(fd, 2) = icon;
	WFIFOL(fd, 4) = sd->account_id;
	WFIFOB(fd, 8) = flag;
	WFIFOSET(fd, packet_len_table[0x196]);
}

int remove_floorskill(int tid, unsigned int tick, int fd, int index)
{
	int i;
	unsigned char buf[256];

	for (i = 0; i < MAX_BLOCKS_AREA_FLOORSKILL; i++) {
		if (floorskill[index].skill[i].x == -1) {
			break;
		}
		WBUFW(buf, 0) = 0x120;
		WBUFL(buf, 2) = floorskill[index].skill[i].bl_id;
		mmo_map_sendarea_mxy(floorskill[index].m, floorskill[index].skill[i].x, floorskill[index].skill[i].y, buf, packet_len_table[0x120]);
		delete_object(floorskill[index].skill[i].bl_id);
	}
	for (i = 0; i < MAX_BLOCKS_AREA_FLOORSKILL; i++) {
		if (floorskill[index].skill[i].x == -1) {
			break;
		}
		floorskill[index].skill[i].bl_id = -1;
		floorskill[index].skill[i].x = -1;
		floorskill[index].skill[i].y = -1;
	}
	floorskill[index].used = 0;
	floorskill[index].m = -1;
	floorskill[index].owner_id = -1;
	floorskill[index].owner_fd = -1;
	floorskill[index].n = -1;
	floorskill[index].skill_lvl = -1;
	floorskill[index].skill_num = -1;
	floorskill[index].count = 0;
	strcpy(floorskill[index].mapname, "");
	floorskill[index].to_x = -1;
	floorskill[index].to_y = -1;
	floorskill[index].timer = 0;

	while(last_floor_item_id >= 0 && floorskill[last_floor_item_id].used == 0) {
		last_floor_item_id--;
	}
	return 0;
}

int next_floorskill_index(void)
{
	int i;

	for (i = 0; i < MAX_FLOORSKILL; i++) {
		if (floorskill[i].used == 0) {
			return i;
		}
	}
	return -1;
}

int search_floorskill_index(short m, short x, short y)
{
	int i;
	short j;
	struct block_list *bl;
	short bx, by;
	struct floorskill_data *fskill;

	if (last_floor_item_id < 50) {
		for (i = 0; i <= last_floor_item_id; i++) {
			if (floorskill[i].m == m) {
				for (j = 0; j < MAX_BLOCKS_AREA_FLOORSKILL; j++) {
					if (floorskill[i].skill[j].x == -1) {
						break;
					}
					if (floorskill[i].skill[j].x == x && floorskill[i].skill[j].y == y) {
						return i;
					}
				}
			}
		}
	}
	else {
		bx = x / BLOCK_SIZE;
		by = y / BLOCK_SIZE;
		if (bx < 0 || bx >= map_data[m].bxs || by < 0 || by >= map_data[m].bys) {
			return -1;
		}
		bl = map_data[m].block[bx + by * map_data[m].bxs].next;
		while(bl != NULL) {
			if (bl->type == BL_SKILL) {
				fskill = (struct floorskill_data*)bl;
				if (fskill->x == x && fskill->y == y) {
					return fskill->index;
				}
			}
			bl = bl->next;
		}
	}
	return -1;
}

int search_floorskill_index2(int account_id, int skill, int start_index)
{
	int i;

	if (start_index >= 0 && start_index < MAX_FLOORSKILL) {
		for (i = start_index; i <= last_floor_item_id; i++) {
			if (floorskill[i].owner_id == account_id && floorskill[i].skill_num == skill) { // (owner_id/skill_num == -1 if not used)
				return i;
			}
		}
	}
	return -1;
}

void make_floorskill(int fd, short x, short y, int skill_lvl, int skill_type, int subtype, int skill_num, int index)
{
	int id;
	int i;
	struct floorskill_data *fskill;
	struct map_session_data *sd;
	unsigned char buf[16];
	int duration = 0;

	if (session[fd] == NULL || (sd = session[fd]->session_data) == NULL) {
		return;
	}
	if (x < 0 || x >= map_data[sd->mapno].xs || y < 0 || y >= map_data[sd->mapno].ys) {
		return;
	}
	for (i = 0; i < MAX_BLOCKS_AREA_FLOORSKILL; i++) {
		if (floorskill[index].skill[i].bl_id == -1) {
			break;
		}
	}
	if (i == MAX_BLOCKS_AREA_FLOORSKILL) {
		return;
	}
	if ((id = search_free_object_id()) == 0) {
		return;
	}
	fskill = (struct floorskill_data *)malloc(sizeof(struct floorskill_data));
	object[id] = &fskill->block;
	fskill->block.prev = NULL;
	fskill->block.next = NULL;
	fskill->block.type = BL_SKILL;
	fskill->block.subtype = subtype;
	fskill->id = id;
	fskill->owner_id = sd->account_id;
	fskill->x = x;
	fskill->y = y;
	fskill->skill_type = skill_type;
	fskill->index = index;
	add_block(&fskill->block, sd->mapno, fskill->x, fskill->y);

	WBUFW(buf, 0) = 0x11f;
	WBUFL(buf, 2) = id;
	WBUFL(buf, 6) = sd->account_id;
	WBUFW(buf, 10) = x;
	WBUFW(buf, 12) = y;
	WBUFB(buf, 14) = skill_type;
	WBUFB(buf, 15) = 1;
	mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);

	floorskill[index].skill[i].bl_id = id;
	floorskill[index].skill[i].x = x;
	floorskill[index].skill[i].y = y;

	if (floorskill[index].used == 0) {
		floorskill[index].type = subtype;
		floorskill[index].m = sd->mapno;
		floorskill[index].n = -1;
		floorskill[index].owner_id = sd->account_id;
		floorskill[index].owner_fd = fd;
		floorskill[index].used = 1;
		floorskill[index].skill_lvl = skill_lvl;
		floorskill[index].skill_num = skill_num;
		switch (subtype) {
			case FS_SANCT:
				floorskill[index].count = 3 + skill_lvl;
				duration = (1 + 3 * skill_lvl) * 1000;
				break;
			case FS_MAGNUS:
				floorskill[index].count = skill_lvl;
				duration = 600000;
				break;
			case FS_QUAG:
				floorskill[index].count = 0;
				duration = (5 * skill_lvl) * 1000;
				break;
			case FS_TRAP:
				floorskill[index].count = 0;
				switch(skill_num) {
					case 115: /* SKIDTRAP */
						duration = (360 - 60 * skill_lvl) * 1000;
						break;
					case 116: /* LANDMINE */
						duration = (80 - 15 * skill_lvl) * 1000;
						break;
					case 117: /* ANKLE SNARE */
						duration = (300 - 50 * skill_lvl) * 1000;
						break;
					case 118: /* SHOCKWAVE */
						duration = (240 - 40 * skill_lvl) * 1000;
						break;
					case 119: /* SANDMAN */
						duration = (180 - 30 * skill_lvl) * 1000;
						break;
					case 120: /* FLASHER */
						duration = (180 - 30 * skill_lvl) * 1000;
						break;
					case 121: /* FREEZING */
						duration = (180 - 30 * skill_lvl) * 1000;
						break;
					case 122: /* BLASTMINE */
						duration = (12 - 2 * skill_lvl) * 1000;
						break;
					case 123: /* CLAYMORE */
						duration = 20 * skill_lvl * 1000;
						break;
				}
				break;
			case FS_WARP:
				floorskill[index].count = 6 + skill_lvl;
				duration = 60000;
				break;
			case FS_ICEWALL:
				floorskill[index].count = 200 + 200 * skill_lvl;
				duration = 300000;
				break;
		}
		floorskill[index].timer = add_timer(gettick() + duration, remove_floorskill, fd, index);
		if (last_floor_item_id < index) {
			last_floor_item_id = index;
		}
	}
}

void init_floorskill_data(void)
{
	int i, j;

	last_floor_item_id = -1;
	for (i = 0; i < MAX_FLOORSKILL; i++) {
		for (j = 0; j < MAX_BLOCKS_AREA_FLOORSKILL; j++) {
			floorskill[i].skill[j].bl_id = -1;
			floorskill[i].skill[j].x = -1;
			floorskill[i].skill[j].y = -1;
		}
		floorskill[i].used = 0;
		floorskill[i].m = -1;
		floorskill[i].owner_id = -1;
		floorskill[i].owner_fd = -1;
		floorskill[i].n = -1;
		floorskill[i].skill_lvl = -1;
		floorskill[i].skill_num = -1;
		floorskill[i].timer = 0;
		floorskill[i].count = 0;
		strcpy(floorskill[i].mapname, "");
		floorskill[i].to_x = -1;
		floorskill[i].to_y = -1;
	}
}

int skill_restart_cast_delay(int tid, unsigned int tick, int fd, int data)
{
	struct map_session_data *sd = session[fd]->session_data;

	sd->skilltimer_delay = 0;
	return 0;
}

void skill_callspirits(int fd, int spheres_num)
{
	struct map_session_data *sd = session[fd]->session_data;

	WFIFOW(fd, 0) = 0x1d0;
	WFIFOL(fd, 2) = sd->account_id;
	WFIFOW(fd, 6) = spheres_num;
	mmo_map_sendarea (fd, WFIFOP(fd, 0), packet_len_table[0x1d0], 0);
}

void skill_combodelay(int fd, int delay)
{
	struct map_session_data *sd = session[fd]->session_data;

	WFIFOW(fd, 0) = 0x1d2;
	WFIFOL(fd, 2) = sd->account_id;
	WFIFOL(fd, 6) = delay;
	mmo_map_sendarea (fd, WFIFOP(fd, 0), packet_len_table[0x1d2], 0);

}

int skill_spiritcadence(int tid, unsigned int tick, int fd, int data)
{
	struct map_session_data *sd = session[fd]->session_data;
	int temp_hp, temp_sp, sp, hp;

	if (sd->status.hp < sd->status.max_hp) {
		hp = sd->status.skill[260-1].lv * (sd->status.max_hp/500 + 4);
		temp_hp = hp + sd->status.hp;
		if (temp_hp > sd->status.max_hp) {
			hp = (sd->status.max_hp - sd->status.hp);
			sd->status.hp = sd->status.max_hp;
		} else sd->status.hp = sd->status.hp + hp;

		WFIFOW(fd, 0) = 0x13d;
		WFIFOW(fd, 2) = 5;
		WFIFOL(fd, 4) = hp;
		WFIFOSET(fd, packet_len_table[0x13d]);
	}

	if (sd->status.sp < sd->status.max_sp) {
		sp = sd->status.skill[260-1].lv * (sd->status.max_sp/500 + 2);
		temp_sp = sp + sd->status.sp;
		if (temp_sp > sd->status.max_sp) {
			sp = (sd->status.max_sp - sd->status.sp);
			sd->status.sp = sd->status.max_sp;
		} else sd->status.sp = sd->status.sp + sp;

		WFIFOW(fd, 0) = 0x13d;
		WFIFOW(fd, 2) = 7;
		WFIFOL(fd, 4) = sp;
		WFIFOSET(fd, packet_len_table[0x13d]);
	}

	mmo_map_calc_status(fd, 0);

	if (sd->weight >= sd->max_weight / 2) sd->skill_timeamount[260-1][0] = add_timer(gettick() + 20000, skill_spiritcadence, fd, 0);
	else sd->skill_timeamount[260-1][0] = add_timer(gettick() + 10000, skill_spiritcadence, fd, 0);

	return 0;
}


