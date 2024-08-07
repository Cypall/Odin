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
#define SKILL_ATTACK(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (100 + (30 * (lvl))) / 100) - 0);
#define SKILL_MAGNUM(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (100 + (15 * (lvl))) / 100) - 0);
#define SKILL_ENVENOM(atk1, atk2, lvl) (((atk1 + rand() % (atk2)) * (100 + (15 * (lvl))) / 100) - 0);
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
#define SKILL_STONEFLING(atk1,atk2) ((atk1+rand()%atk2)+30);
#define Double_Strafe(dex,atk1,atk2,lvl) (((dex+atk1+rand()%atk2) * ((double)200+(10*lvl))/100 ) * 2 );
#define Grimtooth(atk1,atk2,lvl) ( ( (atk1+rand()%atk2) * ((double)100+(10*lvl))/100) - 0 );
#define SKILL_MIN_WAIT 100

extern int PVP_flag;

struct skillwaittime skill_calc_wait(unsigned int fd, int skill_num, int skill_lvl)
{
	int wait = SKILL_MIN_WAIT;
	int duration = 0;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data)) {
		skillwaitdata.wait = SKILL_MIN_WAIT;
		skillwaitdata.duration = 0;
		return skillwaitdata;
	}
	switch (skill_num) {
	case 8: /* ENDURE */
	case 249: /* GUARD */
		duration = 10000 + (3000 * (skill_lvl - 1));
		break;
	case 10: /* SIGHT */
		duration = 3000;
		break;
	case 11: /* NAPALM BEAT */
		wait = (skill_lvl * 500);
		break;
	case 12: /* SAFETY WALL */
		wait = (skill_lvl * 300);
		break;
	case 13: /* SOUL STRIKE */
		switch (skill_lvl) {
		case 1: case 3: case 5: case 7: case 9:
			wait = 1000;
			break;
		case 2: case 4: case 6: case 8: case 10:
			wait = 500;
			break;
		}
		break;
	case 14: /* COLD BOLT */
		wait = (skill_lvl * 800);
		break;
	case 15: /* FROST DIVER */
		wait = 800;
		duration = (skill_lvl * 3000);
		break;
	case 16: /* STONE CURSE */
		duration = 30000 * skill_lvl;
		break;
	case 17: /* FIRE BALL */
		wait = (skill_lvl * 800);
		break;
	case 18: /* FIRE WALL */
		wait = (skill_lvl * 800);
		break;
	case 19: /* FIRE BOLT */
		wait = (skill_lvl * 800);
		break;
	case 20: /* LIGHTNING BOLT */
		wait = (skill_lvl * 800);
		break;
	case 21: /* THUNDER STORM */
		wait = (skill_lvl * 800);
		break;
	case 24: /* RUWACH */
		duration = 1000;
		break;
	case 27: /* WARP */
		wait = 2000;
		break;
	case 29:  /* INCREASE AGI */
		wait = (skill_lvl * 500);
		duration = (skill_lvl * 20000) + 40000;
		break;
	case 30: /* DECREASE AGI */
		wait = (skill_lvl * 500);
		duration = (skill_lvl * 20000) + 40000;
		break;
	case 32: /* SIGNUM CRUCIS */
		wait = (skill_lvl * 500);
		break;
	case 33: /* ANGELUS */
		wait = (skill_lvl * 500);
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
		duration = (skill_lvl * 20000) + 40000;
		break;
	case 51: /* HIDE */
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
		duration = (skill_lvl * 30000);
		break;
	case 61: /* AUTO COUNTER */
		duration = (skill_lvl * 400);
		break;
	case 62: /* BOWLING BASH */
		wait = 1000;
		break;
	case 66: /* IMPOSITIO MANUS */
		wait = (skill_lvl * 500);
		duration = 60000;
		break;
	case 67: /* SUFFRAGIUM */
		wait = (skill_lvl * 500);
		duration = 60000 - (skill_lvl * 10000);
		break;
	case 68: /* ASPERCIO */
		wait = (skill_lvl * 500);
		duration = 30000 + (skill_lvl * 30000);
		break;
	case 69: /* B.S SACRAMENTI */
		wait = (skill_lvl * 500);
		duration = (skill_lvl * 40000);
		break;
	case 70: /* SANCTUARY */
		wait = (skill_lvl * 500);
		break;
	case 73: /* KYRIE */
		wait = (skill_lvl * 500);
		duration = 120000;
		break;
	case 74: /* MAGNIFICANT */
		wait = (skill_lvl * 500);
		duration = 15000 + (skill_lvl * 15000);
		break;
	case 75: /* GLORIA */
		wait = (skill_lvl * 500);
		duration = 35000 + (skill_lvl * 5000);
		break;
	case 76: /* LEX DIVINA */
		duration = (25000 + (skill_lvl * 5000));
		break;
	case 77: /* TURN UNDEAD */
		wait = 1000;
		break;
	case 78: /* LEX AETERNA */
		wait = 1000;
		duration = 3000;
		break;
	case 79: /* MAGNUS EXORCISM */
		wait = 15000;
		break;
	case 80: /* FIRE PILLAR */
		wait = (skill_lvl * 1000);
		duration = 3 + (2 * skill_lvl);
		break;
	case 81: /* SIGHT RASHER */
		wait = 1000;
		break;
	case 83: /* METEOR STORM */
		wait = (12000 - (skill_lvl * 500));
		break;
	case 84: /* JUPITEL THUNDER */
		wait = (10000 - (skill_lvl * 500));
		break;
	case 85: /* LORD OF VERMILION */
		wait = (13000 - (skill_lvl * 500));
		break;
	case 86: /* WATER BALL */
		wait = (10000 - (skill_lvl * 500));
		break;
	case 87: /* ICE WALL */
		wait = (10000 - (skill_lvl * 500));
		break;
	case 89: /* STORM GUST */
		wait = (13000 - (skill_lvl * 500));
		break;
	case 90: /* EARTH SPIKE */
		wait = (skill_lvl * 1000);
		break;
	case 91: /* HEAVEN DRIVE */
		wait = (10000 - (skill_lvl * 800));
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
	case 129: /* BLITZ BEAT */
		wait = 2000;
		break;
	case 135: /* CLOAKING */
		duration = (3000 + (skill_lvl * 1000));
		break;
	case 136: /* SONIC BLOW */
		duration = 15000;
		break;
	case 138: /* ENCHANT POISON */
		duration = 15000 + (skill_lvl * 15000);
		break;
	case 149: /* SAND ATTACK */
		duration = 15000;
		break;
	case 151: /* FIND STONE */
		wait = 500;
		break;
	case 152: /* THROW STONE */
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
	case 252: /* REFLECT SHIELD */
		wait = 100;
		duration = 300000;
		break;
	case 256: /* PROVIDENCE */
		wait = 3000;
		duration = 180000;
		break;
	case 257: /* DEFENDER */
		wait = 100;
		duration = 180000;
		break;
	case 258: /* SPEAR QUICKEN */
		wait = 100;
		duration = 10000 + (skill_lvl * 5000);
		break;
	case 261: /* SUMMON SPIRIT SPHERES */
		wait = 1000;
		duration = 300000;
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
	case 268: /* MENTAL STRENGTH */
		duration = (skill_lvl * 30000);
		break;
	case 269: /* ROOT */
		wait = 300 + (skill_lvl * 200);
		switch (skill_lvl) {
		case 1:
			duration = 20000;
			break;
		case 2:
			duration = 30000;
			break;
		case 3:
			duration = 40000;
			break;
		case 4:
			duration = 50000;
			break;
		case 5:
			duration = 60000;
			break;
		}
		break;
	case 270: /* FURY */
		duration = 60000;
		break;
	default: /* ALL OTHER SKILLS */
		wait = SKILL_MIN_WAIT;
		duration = 0;
		break;
	}
	switch (sd->status.class) {
	case MAGE:
	case ACOLYTE:
	case PRIEST:
	case WIZARD:
	case MONK:
	case SAGE:
		skillwaitdata.wait = wait * (1 - (floor(sd->status.dex / 1.5) / 100));
		break;
	default:
		skillwaitdata.wait = wait;
		break;
	}
	if (sd->skill_timeamount[67-1][0] > 0)
		skillwaitdata.wait -= skillwaitdata.wait * sd->skill_timeamount[67-1][1] / 100;

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

	switch (skill_num) {
	case 5: // bash
		if (skill_lvl > 5) sp = 7;
		break;
	case 12: // safety wall
		if (skill_lvl > 3) sp = 5;
		if (skill_lvl > 6) sp = 10;
		break;
	case 13: // soul strike
		sp = (skill_lvl - 1) * 3;
		break;
	case 14: // cold bolt
		sp = (skill_lvl - 1) * 2;
		break;
	case 15: // frost diver
		sp = (skill_lvl - 1) * (-1);
		break;
	case 16: // stone curse
		sp = (skill_lvl - 1) * (-1);
		break;
	case 19: // fire bolt
	case 20: // lightening bolt
		sp = (skill_lvl - 1) * 2;
		break;
	case 21: // thunder storm
		sp = (skill_lvl - 1) * 5;
		break;
	case 28: // heal
		sp = (skill_lvl - 1) * 3;
		break;
	case 29: // increase agi
		sp = (skill_lvl - 1) * 3;
		break;
	case 30: // decrease agi
		sp = (skill_lvl - 1) * 2;
		break;
	case 33: // angelus
		sp = (skill_lvl - 1) * 3;
		break;
	case 34: // blessing
		sp = (skill_lvl - 1) * 3;
		break;
	case 45: // concentration
		sp = (skill_lvl - 1) * 5;
		break;
	case 52: // envenom
		if(skill_lvl > 1) sp = 20;
		if(skill_lvl > 3) sp = 19;
		if(skill_lvl > 5) sp = 18;
		if(skill_lvl > 7) sp = 17;
		if(skill_lvl > 9) sp = 16;
		break;
	case 60: // two hand quicken
		sp = (skill_lvl - 1) * 4;
		break;
	case 62: // bowling bash
		sp = (skill_lvl - 1);
		break;
	case 66: // impositio manus
		sp = (skill_lvl - 1) * 3;
		break;
	case 68: // aspersio
		sp = (skill_lvl - 1) * 4;
		break;
	case 70: // sanctuary
		sp = (skill_lvl - 1) * 3;
		break;
	case 71: // slow poison
		if(skill_lvl>1) sp = 2;
		if(skill_lvl>2) sp = 4;
		if(skill_lvl>3) sp = 6;
		break;
	case 73: // kyrie eleison
            if(skill_lvl>5) sp = 5;
            if(skill_lvl>9) sp = 15;
		break;
	case 76: // lex divina
		if (skill_lvl > 5) sp = -2;
		if (skill_lvl > 6) sp = -4;
		if (skill_lvl > 7) sp = -6;
		if (skill_lvl > 8) sp = -8;
		if (skill_lvl > 9) sp = -10;
		break;
	case 79: // magnus exorcismus
		sp = (skill_lvl - 1) * 2;
		break;
	case 81: // sight trasher
		sp = (skill_lvl - 1) * 2;
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
		sp = (skill_lvl - 1) * 3;
		break;
	case 85: // lord of vermilion
		sp = (skill_lvl - 1) * 4;
		break;
	case 86: // water ball
		if (skill_lvl > 1) sp = 5;
		if (skill_lvl > 3) sp = 15;
		break;
	case 88: // frost nova
		sp = (skill_lvl - 1) * (-2);
		break;
	case 90: // earth spike
		sp = (skill_lvl - 1) * 2;
		break;
	case 91: // heaven's drive
		sp = (skill_lvl-1) * 4;
		break;
	case 111: // adrenaline rush
		sp = (skill_lvl - 1) * 3;
		break;
	case 112: // weapon perfection
		sp = (skill_lvl - 1) * (-2);
		break;
	case 113: // power-thrust
		sp = (skill_lvl - 1) * (-2);
		break;
	case 129: // blitz beat
		sp = (skill_lvl - 1) * 3;
		break;
	case 136: // sonic blow
		sp = (skill_lvl - 1) * 2;
		break;
	case 139: // poison react
		sp = (skill_lvl - 1) * (-5);
		break;
	case 141: // venom splasher
		if(skill_lvl>1) sp = 3;
		if(skill_lvl>2) sp = 6;
		if(skill_lvl>3) sp = 9;
		if(skill_lvl>4) sp = 12;
		break;
	case 254: // grand cross
		sp = (skill_lvl - 1) * 7;
		break;
	case 252: // reflect shield
		sp = (skill_lvl - 1) * 5;
            break;
	case 258: // spear quicken
		sp = (skill_lvl - 1) * 4;
		break;
	case 249: // auto guard
		sp = (skill_lvl - 1) * 2;
		break;
	case 253: // holy cross
		sp = (skill_lvl - 1);
		break;
	case 266: // occult impact
	case 267: // throw spirit sphere
		if (skill_lvl > 1) sp = 4;
		if (skill_lvl > 2) sp = 7;
		if (skill_lvl > 3) sp = 9;
		if (skill_lvl > 4) sp = 10;
		break;
	}
	return sp;
}

int skill_calc_damage(unsigned int fd, int skill_num, int skill_lvl, long target_id)
{
	int attack_element = 0, target_element = 0;
	int damage = 0, s_type = 0, s_lv = 0;
	double aux = 0;
	int aux2 = 0;
	int a1 = 0, a2 = 0;
	int i, n;
	short m;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	for (i = 0; i < MAX_INVENTORY; i++) {
		struct item_db2 weapon;
		if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip != 0)) {
			weapon = item_database(sd->status.inventory[i].nameid);
			if (weapon.type == 4) {
				attack_element = weapon.ele;
				break;
			}
		}
	}
	if (sd->skill_timeamount[68-1][0] > 0)
		attack_element = 6;

	m = sd->mapno;
	n = mmo_map_search_monster(m, target_id);
	if (n >= 0)
		target_element = mons_data[map_data[m].npc[n]->class].ele;

	else {
		if (sd->skill_timeamount[69-1][0] > 0)
			target_element = 6;

	}
	if (sd->status.weapon == 4 || sd->status.weapon == 5) {
		s_type = 5; // spear
		s_lv = sd->status.skill[55-1].lv;
	}
	else if (sd->status.weapon == 2 || sd->status.weapon == 3) {
		s_type = 4; // 1hs and 2hs
		s_lv = sd->status.weapon == 2 ? sd->status.skill[2-1].lv : sd->status.skill[3-1].lv;
	}
	else if (sd->status.weapon == 16 || sd->status.weapon == 8) {
		s_type = 3; // katar, mace
		s_lv = sd->status.weapon == 16 ? sd->status.skill[134-1].lv : sd->status.skill[65-1].lv;
	}
	else
		s_lv = s_type = 0;

	switch (skill_num) {
	case 5: // BASH
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl);
		damage = (int) (damage * get_ele_attack_factor(attack_element, target_element));
		damage *= 0.85;
		break;
	case 7: // MAGNUM_BREAK
		damage = SKILL_MAGNUM (sd->status.atk1, sd->status.atk2, skill_lvl);
		damage = (int) (damage * get_ele_attack_factor(3, target_element));
		damage *= 1.2;
		break;
	case 11: // NAPALM_BEAT
		damage = SKILL_NAPALM (sd->status.matk1, sd->status.matk2, skill_lvl);
		damage = (int) (damage * get_ele_attack_factor(8, target_element));
		damage *= (skill_lvl * 0.6);
		break;
	case 13: // SOUL STRIKE
		damage = sd->status.matk1 + rand()%(sd->status.matk2 - sd->status.matk1 + 1);
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
		break;
	case 14: // COLDBOLT
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		if (n >= 0) {
			aux = get_ele_attack_factor(4, target_element);
			aux2 = (1 - (mons_data[map_data[m].npc[n]->class].mdef1 / 100));
			if (aux2 == 0)
				aux2 = 1;

			damage = (int)(damage * (aux / aux2));
			damage -= mons_data[map_data[m].npc[n]->class].mdef2;
		}
		else
			damage = (int) (damage * get_ele_attack_factor(4, target_element));

		damage *= skill_lvl;
		break;
	case 15: // FROST DIVER
		damage = SKILL_FROSTDIVER(sd->status.matk1);
		damage *= get_ele_attack_factor(1, target_element);
		damage *= (skill_lvl * 0.3);
		break;
	case 17: // FIREBALL
		damage = SKILL_FIREBALL (sd->status.matk1, sd->status.matk2, skill_lvl);
		if (n >= 0) {
			aux = get_ele_attack_factor(3, target_element);
			aux2 = (1 - (mons_data[map_data[m].npc[n]->class].mdef1 / 100));
			if (aux2 == 0)
				aux2 = 1;

			damage = (int)(damage * (aux / aux2));
			damage -= mons_data[map_data[m].npc[n]->class].mdef2;
		}
		else
			damage = (int) (damage * get_ele_attack_factor(3, target_element));

		break;
	case 18: // FIRE WALL
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		damage *= get_ele_attack_factor(3,target_element);
		damage *= (skill_lvl * 0.3);
		break;
	case 19: // FIREBOLT
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		if (n >= 0) {
			aux = get_ele_attack_factor(3, target_element);
			aux2 = (1 - (mons_data[map_data[m].npc[n]->class].mdef1 / 100));
			if (aux2 == 0)
				aux2 = 1;

			damage = (int)(damage * (aux / aux2));
			damage -= mons_data[map_data[m].npc[n]->class].mdef2;
		}
		else
			damage = (int) (damage * get_ele_attack_factor(3, target_element));

		damage *= skill_lvl;
		break;
	case 20: // LIGHTINGBOLT
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		if (n >= 0) {
			aux = get_ele_attack_factor(4, target_element);
			aux2 = (1 - (mons_data[map_data[m].npc[n]->class].mdef1 / 100));
			if (aux2 == 0)
				aux2 = 1;

			damage = (int)(damage * (aux / aux2));
			damage -= mons_data[map_data[m].npc[n]->class].mdef2;
		}
		else
			damage = (int)(damage * get_ele_attack_factor(4, target_element));

		damage *= skill_lvl;
		break;
	case 28: // HEAL
		damage = -SKILL_HEAL(sd->status.int_, sd->status.base_level, skill_lvl);
		if (((target_element % 10) == 8) || ((target_element % 10) == 9))
			damage = -0.5 * ((int)(damage * get_ele_attack_factor(6, target_element)));

		break;
	case 42: // MAMMONITE
		damage = SKILL_BOWLING(sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= 2;
		break;

	case 46: // DOUBLE STRAFE
		damage = Double_Strafe(sd->status.dex, sd->status.atk1, sd->status.atk2, skill_lvl);
		break;

	case 47: // ARROW SHOWER
		damage = Double_Strafe(sd->status.dex, sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= 1.1;
		break;
	case 52: // ENVENOM
		damage = SKILL_ENVENOM(sd->status.atk1, sd->status.atk2, skill_lvl);
		damage = (int)(damage * get_ele_attack_factor(5, target_element));
		break;
	case 56: // PIERCE
		damage = SKILL_PIERCE(sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= 2;
		break;
	case 57: // BRANDISH SPEAR
		damage = SKILL_BRANDISH(sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= 2;
		break;
	case 58: // SPEAR STAB
		damage = SKILL_MAGNUM(sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= 2.3;
		break;
	case 59: // SPEAR BOOMERANG
		damage = SKILL_BOWLING(sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= 1.5;
		break;
	case 62: // BOWLING_BASH
		damage = SKILL_BOWLING(sd->status.atk1, sd->status.atk2, skill_lvl);
		damage = (int)(damage * get_ele_attack_factor(attack_element, target_element));
		damage *= 1.2;
		break;
	case 70: // SANCTUARY
		if (skill_lvl > 6)
			damage = 777 / 2;

		else
			damage = (skill_lvl * 100) / 2;

		break;
	case 79: // MAGNUM EXORCISE
		damage = 500 * skill_lvl;
		break;
	case 80: // FIRE PILLAR
		damage = SKILL_FIREPILLAR(sd->status.matk1, skill_lvl);
		damage = (int)(damage * get_ele_attack_factor(3, target_element));
		damage *= (skill_lvl * 0.35);
		break;
	case 81: // SIGHT TRASHER
		damage = SKILL_TRASHER(sd->status.matk1, sd->status.matk2, skill_lvl);
		damage *= get_ele_attack_factor(3,target_element);
		break;
	case 83: // METEOR STORM
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		damage = (int)(damage * get_ele_attack_factor(3, target_element));
		damage *= (skill_lvl * 0.8);
		break;
	case 84: // JUPITEL THUNDER
		damage = sd->status.matk1 + rand() % (sd->status.matk2 - sd->status.matk1 + 1);
		if (n >= 0) {
			damage = damage *(100 - mons_data[map_data[m].npc[n]->class].mdef1) / 100;
			damage = damage - mons_data[map_data[m].npc[n]->class].mdef1;
			if (damage < 1)
				damage = 1;

			damage = (int)(damage * get_ele_attack_factor(4, target_element));
			if (damage < 0)
				damage = 0;

		}
		else
			damage = (int)(damage * get_ele_attack_factor(4, target_element));

		damage = damage * (2 + skill_lvl);
		break;
	case 85: // VERMILION
		damage = SKILL_MATTACK (sd->status.matk1, sd->status.matk2);
		damage = (int)(damage * get_ele_attack_factor(4, target_element));
		damage *= 1.5;
		break;
	case 88: // FROST NOVA
		damage = SKILL_FROSTDIVER(sd->status.matk1);
		damage *= get_ele_attack_factor(1, target_element);
		damage *= (skill_lvl * 0.3);
	case 90: // EARTH SPIKE
		damage = sd->status.matk1 + rand() % (sd->status.matk2 - sd->status.matk1 + 1);
		if (n >= 0) {
			damage = damage * (100 - mons_data[map_data[m].npc[n]->class].mdef1) / 100;
			damage = damage - mons_data[map_data[m].npc[n]->class].mdef1;
			if (damage < 1)
				damage = 1;

			damage = (int)(damage * get_ele_attack_factor(2, target_element));
			damage = damage * skill_lvl;
			if (damage < 0)
				damage = 0;

		}
		else
			damage = (int)(damage * get_ele_attack_factor(2, target_element));

		break;
	case 91: // HEAVEN DIVE
		damage = sd->status.matk1 + rand() % (sd->status.matk2 - sd->status.matk1 + 1) * 5;
		if (damage < 1)
			damage = 1;

		damage = (int)(damage * get_ele_attack_factor(2, target_element));
		if (damage < 0)
			damage = 0;

		break;
	case 115:  // SKID TRAP
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl);
		break;
	case 116: // LAND MINE
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl);
		break;
	case 123: // CLAYMORE
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl);
		break;
	case 129: // BLITZ BEAT
		if (sd->auto_blitz) {
			if (sd->status.job_level <= 10)
				skill_lvl = 1;

			else if (sd->status.job_level > 10 && sd->status.job_level <= 20)
				skill_lvl = 2;

			else if (sd->status.job_level > 20 && sd->status.job_level <= 30)
				skill_lvl = 3;

			else if (sd->status.job_level > 30 && sd->status.job_level <= 40)
				skill_lvl = 4;

			else
				skill_lvl = 5;

		}
		damage = SKILL_BLITZBEAT (sd->status.dex, sd->status.int_, sd->status.skill[128-1].lv, skill_lvl);
		damage = (int)(damage * get_ele_attack_factor(0, target_element));
		break;
	case 136: // SONIC BLOW
		damage = SKILL_SONICBLOW (sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= 2;
		break;
	case 137: // GRIMTOOTH
		damage = Grimtooth (sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= 3;
		break;
	case 148: // CHARGE ARROW
		damage = Double_Strafe(sd->status.dex, sd->status.atk1, sd->status.atk2, skill_lvl*10);
		damage *= 1.2;
		break;
	case 152: // THROW STONE
		damage = SKILL_STONEFLING(sd->status.atk1, sd->status.atk2);
		break;
	case 153: // CART REVOLUTION
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl*10);
		damage *= 2.5;
		break;
	case 156: // HOLY LIGHT
		damage = SKILL_MATTACK(sd->status.matk1, sd->status.matk2);
		damage *= 1.5;
		break;
	case 250: // SHIELD CHARGE
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= (1 + skill_lvl * 0.2);
		break;
	case 251: // SHIELD BOOMERANG
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= (1 + skill_lvl * 0.3);
		break;
	case 253: // HOLY CROSS
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl);
		damage *= (1 + skill_lvl * 0.3);
		break;
	case 254: // GRAND CROSS
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl) + SKILL_MATTACK (sd->status.matk1, sd->status.matk2);
		damage *= (1 + skill_lvl * 0.40);
		break;
	case 266: // OCCULT IMPACT
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl) + SKILL_MATTACK (sd->status.matk1, sd->status.matk2);
		break;
	case 267: // FINGER OFFENSIVE
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl) + SKILL_MATTACK (sd->status.matk1, sd->status.matk2);
		break;
	case 271: // EXTREMITY FIST
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl) + SKILL_MATTACK (sd->status.matk1, sd->status.matk2);
		damage += (250 + skill_lvl * 150);
		break;
	case 272: // CHAIN COMBO
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl) + SKILL_MATTACK (sd->status.matk1, sd->status.matk2);
		break;
	case 273: // COMBO FINNISH
		damage = SKILL_ATTACK (sd->status.atk1, sd->status.atk2, skill_lvl) + SKILL_MATTACK (sd->status.matk1, sd->status.matk2);
		break;
	default: // ALL OTHER skills
		if (sd->status.matk1 <= 0)
			a1 = 1;

		else
			a1 = sd->status.matk1;

		if (sd->status.matk2 <= 0)
			a2 = 1;

		else
			a2 = sd->status.matk2;

		damage = SKILL_MATTACK (a1, a2);
		damage *= skill_lvl ;
		break;
	}
	if (n >= 0) {
		if (map_data[m].npc[n]->u.mons.lexaeterna)
			damage *= 2;

	}
	else {
		if (sd->skill_timeamount[78-1][0] > 0)
			damage *= 2;

	}
	if (damage)
		damage += s_lv * s_type;

	return damage;
}

void skill_do_damage(unsigned int fd, int damage, long target_id, unsigned long tick, short healed)
{
  	short m;
	int mvp_damage = 0, n, len;
  	unsigned int i, target_fd = 0, mvp_fd = 0;
  	unsigned char buf[256];
  	struct map_session_data *sd, *target_sd = NULL, *temp_sd = NULL;
	struct npc_data *monster;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

  	m = sd->mapno;
  	n = mmo_map_search_monster(m, target_id);
	if (n < 0) {
		if (mmo_map_flagload(sd->mapname, PVP) || PVP_flag) {
			for (i = 5; i < FD_SETSIZE; i++) {
				if (session[i] && (target_sd = session[i]->session_data)) {
					if (target_sd->account_id == target_id) {
						target_fd = i;
						target_sd->status.hp -= damage;
						if (target_sd->status.hp < 0)
							target_sd->status.hp = 0;

						len = mmo_map_set_param(target_fd, WFIFOP(target_fd, 0), SP_HP);
						if (len > 0)
							WFIFOSET(target_fd, len);

						if (target_sd->status.hp <= 0) {
							mmo_player_send_death(target_sd);
							for (i = 5; i < FD_SETSIZE; i++) {
								if (session[i] && (temp_sd = session[i]->session_data)) {
									if (temp_sd->attacktarget == target_sd->account_id) {
										if (temp_sd->attacktimer > 0) {
											delete_timer(temp_sd->attacktimer, mmo_map_attack_continue);
											temp_sd->attacktimer = 0;
										}
										temp_sd->attacktarget = 0;
									}
								}
							}
							sd->pvprank++;
							mmo_map_checkpvpmap(sd);
						}
					}
					break;
				}
			}
		}
	}
	else {
		if (!(monster = map_data[m].npc[n]))
			return;

		if (monster->u.mons.target_fd != fd && monster->u.mons.hp > 0)
			check_new_target_monster(m, n, fd);

		if (healed && (mons_data[monster->class].ele % 10 == 8 || mons_data[monster->class].ele % 10 == 9)) {
			damage /= 2;
			monster->u.mons.hp -= damage;
			sd->status.damage_atk += damage;

			WBUFW(buf, 0) = 0x114;
			WBUFW(buf, 2) = 28;
			WBUFL(buf, 4) = sd->account_id;
			WBUFL(buf, 8) = target_id;
			WBUFL(buf, 12) = tick;
			WBUFL(buf, 16) = sd->speed;
			WBUFL(buf, 20) = monster->u.mons.speed;
			WBUFW(buf, 24) = damage;
			WBUFW(buf, 26) = sd->status.skill[28-1].lv;
			WBUFW(buf, 28) = skill_db[28].type_num;
			WBUFB(buf, 30) = skill_db[28].type_hit;
			mmo_map_sendarea(fd, buf, packet_len_table[0x114], 0);
		}
		else if (healed && !(mons_data[monster->class].ele % 10 == 8 || mons_data[monster->class].ele % 10 == 9)) {
			damage *= -1;
			monster->u.mons.hp += damage;

			WBUFW(buf, 0) = 0x11a;
			WBUFW(buf, 2) = 28;
			WBUFW(buf, 4) = damage;
			WBUFL(buf, 6) = target_id;
			WBUFL(buf, 10) = sd->account_id;
			WBUFB(buf, 14) = 1;
			mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);

    			if (monster->u.mons.hp > mons_data[monster->class].max_hp)
				monster->u.mons.hp = mons_data[monster->class].max_hp;

			return;
		}
		else {
			monster->u.mons.hp -= damage;
			sd->status.damage_atk += damage;
		}
		if ((rand() % 90) == 10) {
			if (monster->u.mons.hp >= (mons_data[monster->class].max_hp / 2))
				monster_say(fd, target_id, monster->class, "hp50");

			else if (monster->u.mons.hp <= (mons_data[monster->class].max_hp / 4))
				monster_say(fd, target_id, monster->class, "hp25");

			else if (monster->u.mons.hp <= 0)
				monster_say(fd, target_id, monster->class, "dead");

		}
		if (monster->u.mons.hp <= 0) {
			sd->status.damage_atk += monster->u.mons.hp;
			mmo_mons_send_death(fd, m, n);
			for (i = 5; i < FD_SETSIZE; i++) {
				if (session[i] && (temp_sd = session[i]->session_data) && temp_sd->attacktarget == target_id) {
					if (temp_sd->status.damage_atk >= mvp_damage) {
						mvp_damage = temp_sd->status.damage_atk;
						mvp_fd = i;
					}
          				mmo_map_level_mons(temp_sd, m, n);
					if (temp_sd->attacktimer > 0) {
						delete_timer(temp_sd->attacktimer, mmo_map_attack_continue);
          					temp_sd->attacktimer = 0;
						temp_sd->attacktarget = 0;
					}
					temp_sd->status.damage_atk = 0;
        			}
			}
			if (mvp_fd > 0)
				mmo_map_mvp_do(mvp_fd, m, n);

		}
	}
}

int square_collection(short x, short y, short map, int extend, struct npc_data *pmon[])
{
	int i, j, c = 0;

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

void set_option(struct map_session_data *sd, short m, int n)
{
	unsigned char buf[256];
	if (sd) {
		WBUFW(buf, 0) = 0x119;
		WBUFL(buf, 2) = sd->account_id;
		WBUFW(buf, 6) = sd->status.option_x;
		WBUFW(buf, 8) = sd->status.option_y;
		if (sd->status.special)
			WBUFW(buf, 10) = sd->status.special;
		else
			WBUFW(buf, 10) = sd->status.option_z;
		WBUFB(buf, 12) = 0;
		mmo_map_sendarea(sd->fd, buf, packet_len_table[0x119], 0);
	}
	else {
		WBUFW(buf, 0) = 0x119;
		WBUFL(buf, 2) = map_data[m].npc[n]->id;
		WBUFW(buf, 6) = map_data[m].npc[n]->u.mons.option_x;
		WBUFW(buf, 8) = map_data[m].npc[n]->u.mons.option_y;
		WBUFW(buf, 10) = map_data[m].npc[n]->u.mons.option_z;
		WBUFB(buf, 12) = 0;
		mmo_map_sendarea_mxy(m, map_data[m].npc[n]->x, map_data[m].npc[n]->y, buf, packet_len_table[0x119]);
	}
}

void skill_drain_hp_monster(int tid, unsigned int tick, int m, int n)
{
	int damage = 0;

	if (map_data[m].npc[n]->skilldata.skill_timer[52-1][0] > 0) {
		damage = map_data[m].npc[n]->u.mons.hp;
		damage -= 10 * map_data[m].npc[n]->skilldata.skill_timer[52-1][1];
		set_option(NULL, m, n);
		mmo_mons_send_death(0, m, n);
		add_timer(tick + 3000, skill_drain_hp_monster, m, n);
	}
}

void skill_drain_hp(int tid, unsigned int tick, int fd, int data)
{
	int len;
	struct map_session_data *sd;

	if (session[fd] && (sd = session[fd]->session_data)) {
		if (sd->skill_timeamount[52-1][0] > 0) { 
			if (sd->status.effect & ST_POISON) {
				set_option(sd, 0, 0);
				sd->status.hp -= 10 + data;

				if (sd->status.hp <= 0) 
					sd->status.hp = 0;

				len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_HP);
				if (len > 0)
					WFIFOSET(fd, len);

				if (sd->status.hp <= 0)
					mmo_player_send_death(sd);

			}
			else
				skill_reset_stats(0, 0, fd, 52);

		}
	}
}

void skill_reset_monster_stats(int tid, unsigned int tick, int m, int n)
{
	int i, skill_num;
	struct npc_data *monster;

	if (!(monster = map_data[m].npc[n]))
		return;

	skill_num = monster->skilldata.skill_num;
	switch (skill_num) {
	case 1: // REMOVE ALL
		for (i = 0; i < MAX_SKILL; i++) {
			if (monster->skilldata.skill_timer[i][0] > 0) {
				delete_timer(monster->skilldata.skill_timer[i][0], skill_reset_monster_stats);
				monster->skilldata.skill_timer[i][0] = 0;
				monster->skilldata.skill_timer[i][1] = 0;
			}
		}
		monster->u.mons.option_x = 0;
		monster->u.mons.option_y = 0;
		monster->u.mons.option_z = 0;
		set_option(NULL, m, n);
		monster->skilldata.effect = 00000000;
		break;
	case 5: // BASH
	case 110: // HAMMER FALL
	case 136: // SONIC BLOW
	case 152: // THROW STONE
	case 269: // ROOT
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0; 
		monster->u.mons.option_x = 0;
		set_option(NULL, m, n);
		monster->skilldata.effect &= ~ST_STUN;
		break;
	case 10: // SIGHT
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0; 
		monster->u.mons.option_z = 0;
		set_option(NULL, m, n);
		break;
	case 15: // FROST DIVER
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0;
		monster->u.mons.option_x = 0;
		set_option(NULL, m, n);
		monster->skilldata.effect &= ~ST_FROZEN;
		break;
	case 16: // STONE CURSE
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0;
		monster->u.mons.option_x = 0;
		set_option(NULL, m, n);
		monster->skilldata.effect &= ~ST_STONE;
		break;
	case 29: // INCREASE AGI
	case 30: // DECREASE AGI
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0;
		monster->u.mons.speed = mons_data[monster->class].speed;
		break;
	case 51: // HIDDEN
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0;
		monster->u.mons.option_z = 0;
		set_option(NULL, m, n);
		monster->u.mons.hidden = 0;
		break;
	case 52: // ENVENOM
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0;
		monster->u.mons.option_y = 0;
		set_option(NULL, m, n);
		monster->skilldata.effect &= ~ST_POISON;
		break;
	case 76: // LEX DIVINA
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0;
		monster->u.mons.option_y = 0;
		set_option(NULL, m, n);
		monster->skilldata.effect &= ~ST_SILENCE;
		break;
	case 92: // QUAGMIRE
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0;
		monster->u.mons.speed = mons_data[monster->class].speed;
		monster->u.mons.def1 = mons_data[monster->class].def1;
		break;
	case 149: // SAND ATTACK
	case 253: // HOLY CROSS
		delete_timer(monster->skilldata.skill_timer[skill_num-1][0], skill_reset_monster_stats);
		monster->skilldata.skill_timer[skill_num-1][0] = 0; 
		monster->u.mons.option_x = 0;
		set_option(NULL, m, n);
		monster->skilldata.effect &= ~ST_BLIND;
		break;
	}
}

void skill_reset_stats(int tid, unsigned int tick, int fd, int skill_num)
{
	int i;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	debug_output("skill reset %d\n", skill_num);
	switch (skill_num) {
	case 1: // POTIONS
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 37, 0);
		break;
	case 5: // BASH
	case 136: // SONIC BLOW
	case 152: // THROW STONE
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->status.option_x = 0;
		set_option(sd, 0, 0);
		sd->status.effect &= ~ST_STUN;
		break;
	case 8: // ENDURE
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		skill_icon_effect(sd, 1, 0);
		break;
	case 10: // SIGHT
	case 24: // RUWACH
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->status.option_z = 0;
		set_option(sd, 0, 0);
		break;
	case 15: // FROST DIVER
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->status.option_x = 0;
		set_option(sd, 0, 0);
		sd->status.effect &= ~ST_FROZEN;
		break;
	case 29: // INCREASE AGI
		delete_timer(sd->skill_timeamount[skill_num-1][0],skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 12, 0);
		break;
	case 30: // DECREASE AGI
		delete_timer(sd->skill_timeamount[30-1][0], skill_reset_stats);
		sd->skill_timeamount[30-1][0] = 0;
		sd->skill_timeamount[30-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 14, 0);
		break;
	case 33: // ANGELUS
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 9, 0);
		break;
	case 34: // BLESSING
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 10, 0);
		break;
	case 45: // IMPROVE CONCENTRATION
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 3, 0);
		break;
	case 51: // HIDDEN
		if (sd->skill_timeamount[skill_num-1][0] > 0) {
			if ((sd->sp_count < (30 * sd->sp_count_lvl)) && (sd->status.sp > 0)) {
				sd->status.sp -= 10;
				sd->sp_count += 5;

				if (sd->status.sp <= 0)
					sd->status.sp = 0;

				WFIFOW(fd, 0) = 0xb0;
				WFIFOW(fd, 2) = 0x07;
				WFIFOL(fd, 4) = sd->status.sp;
				WFIFOSET(fd, packet_len_table[0xb0]);
				sd->skill_timeamount[skill_num-1][0] == add_timer(gettick() + 5000, skill_reset_stats, fd, skill_num);
			}
			else {
				delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
				sd->skill_timeamount[skill_num-1][0] = 0;
				sd->status.option_z = 0;
				sd->sp_count = 0;
				sd->hidden = 0;
				set_option(sd, 0, 0);
				skill_icon_effect(sd, 4, 0);
			}
		}
		else {
			delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
			sd->skill_timeamount[skill_num-1][0] = 0;
			sd->status.option_z = 0;
			sd->sp_count = 0;
			sd->hidden = 0;
			set_option(sd, 0, 0);
			skill_icon_effect(sd, 4, 0);
		}
		break;
	case 52: // ENVENOM
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		delete_timer(sd->drain_hp, skill_drain_hp);
		sd->skill_timeamount[52-1][0] = 0;
		sd->drain_hp = 0;
		sd->status.option_y = 0;
		set_option(sd, 0, 0);
		sd->status.effect &= ~ST_POISON;
		break;
	case 60: // TWO HAND QUICKEN
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 2, 0);
		break;
	case 61: // AUTO COUNTER
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		skill_icon_effect(sd, 152, 0);
		break;
	case 66: // IMPOSITIO MAGNUS
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 15, 0);
		break;
	case 67: // SUFFRAGIUM
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		skill_icon_effect(sd, 16, 0);
		break;
	case 68: //ASPERCIO
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		skill_icon_effect(sd, 17, 0);
		break;
	case 69: // B.S SACRAMENTI
	 	delete_timer(sd->skill_timeamount[69-1][0], skill_reset_stats);
		sd->skill_timeamount[69-1][0] = 0;
		sd->skill_timeamount[69-1][1] = 0;
		skill_icon_effect(sd, 18, 0);
		break;
	case 73: // KYRIE
		delete_timer(sd->skill_timeamount[skill_num-1][0],skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		skill_icon_effect(sd, 19, 0);
		break;
	case 74: // MAGNIFICANT
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		skill_icon_effect(sd, 20, 0);
		break;
	case 75: // GLORIA
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 21, 0);
		break;
	case 76: // LEX DIVINA
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->status.option_y = 0;
		set_option(sd, 0, 0);
		sd->status.effect &= ~ST_SILENCE;
		break;
	case 92: // QUAGMIRE
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		mmo_map_calc_status(fd, 0);
		break;
	case 111: // ADRENALINE RUSH
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 23, 0);
		break;
	case 112: // WEAPON PERFECTION
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
      	sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		skill_icon_effect(sd, 24, 0);
		break;
	case 113: // POWER THRUST
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 25, 0);
		break;
	case 114: // MAXIMIZE POWER
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		skill_icon_effect(sd, 26, 0);
		break;
	case 135: // CLOAKING
		if (sd->skill_timeamount[skill_num-1][0] > 0) {
			if ((sd->sp_count < (30 * sd->sp_count_lvl)) && (sd->status.sp > 0)) {
				sd->status.sp -= 10;
				sd->sp_count += 5;

				if (sd->status.sp <= 0)
					sd->status.sp = 0;

				WFIFOW(fd, 0) = 0xb0;
				WFIFOW(fd, 2) = 0x07;
				WFIFOL(fd, 4) = sd->status.sp;
				WFIFOSET(fd, packet_len_table[0xb0]);
				sd->skill_timeamount[skill_num-1][0] == add_timer(gettick() + 5000, skill_reset_stats, fd, skill_num);
			}
			else {
				delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
				sd->skill_timeamount[skill_num-1][0] = 0;
				sd->status.option_z = 0;
				sd->sp_count = 0;
				set_option(sd, 0, 0);
				skill_icon_effect(sd, 5, 0);
			}
		}
		else {
			delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
			sd->skill_timeamount[skill_num-1][0] = 0;
			sd->status.option_z = 0;
			sd->sp_count = 0;
			set_option(sd, 0, 0);
			skill_icon_effect(sd, 5, 0);
		}
		break;
	case 138: // ENCHANT POISON
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 6, 0);
		break;
	case 149: // SAND ATTACK
	case 253: // HOLY CROSS
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->status.option_x = 0;
		set_option(sd, 0, 0);
		sd->status.effect &= ~ST_BLIND;
		break;
	case 153: // POISON REACT
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		skill_icon_effect(sd, 7, 0);
		break;
	case 155: // CRAZY UPLOAR
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 30, 0);
		break;
	case 157: // ENERGY COAT
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		skill_icon_effect(sd, 31, 0);
		break;
	case 249: // GUARD
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		skill_icon_effect(sd, 58, 0);
		break;
	case 252: // REFLECT SHIELD
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		skill_icon_effect(sd, 59, 0);
		break;
	case 256: // PROVIDENCE
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 61, 0);
		break;
	case 257: // DEFENDER
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 62, 0);
		break;
	case 258: // SPEAR QUICKEN
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 68, 0);
		break;
	case 261: // SUMMON SPIRITS
		sd->spheres_timeamount[0] = -1;
		for (i = 1; i <= sd->spheres; i++) {
			sd->spheres_timeamount[i-1] = sd->spheres_timeamount[i];
			sd->spheres_timeamount[i] = 0;
		}
		sd->spheres--;
		skill_callspirits(fd, sd->spheres);
		break;
	case 268: // MENTAL STRENGH
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 69, 0);
		break;
	case 269: // ROOT
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		mmo_map_calc_status(fd, 0);
		break;
	case 270: // FURY
		delete_timer(sd->skill_timeamount[skill_num-1][0], skill_reset_stats);
		sd->skill_timeamount[skill_num-1][0] = 0;
		sd->skill_timeamount[skill_num-1][1] = 0;
		mmo_map_calc_status(fd, 0);
		skill_icon_effect(sd, 70, 0);
	}
}

void skill_do_delayed_target(int tid, unsigned int tick, int fd, int skill_num)
{
	int damage = 0, heal_point = 0;
	int chance_base = 0, frozen_d = 0, stun_c = 0, blind_c = 0;
	int cant = 0, cell_range = 0;
	int temp_sp = 0, delay, dst;
	int i, j, n, len;
	short m;
	short bx, by, x, y;
	long target_id;
	int forplayer = 0;
	int skill_lvl;
	unsigned target_fd = 0;
	unsigned char buf[256];
	struct item tmp_item;
	struct map_session_data *sd, *target_sd = NULL;
	struct npc_data *target[256], *monster = NULL;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	sd->casting = 0;
	target_id = sd->attacktarget;
	skill_lvl = sd->skill_lvl;
	m = sd->mapno;
	n = mmo_map_search_monster(m, target_id);
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
	}
	else {
		if (!(monster = map_data[m].npc[n]))
			return;

	}
	if (skill_db[skill_num].type_inf == 1
	|| skill_db[skill_num].type_inf == 4
	|| skill_db[skill_num].type_inf == 16) {
		damage = skill_calc_damage(fd, skill_num, skill_lvl, target_id);
		if ((delay = skill_do_cast_delay(skill_num, skill_lvl).delay) != 0)
			sd->skilltimer_delay = add_timer(gettick() + delay, skill_restart_cast_delay, fd, skill_num);

		switch (skill_db[skill_num].type_nk) {
		case 0: // HIT SKILLS
			WBUFW(buf, 0) = 0x114;
			WBUFW(buf, 2) = skill_num;
			WBUFL(buf, 4) = sd->account_id;
			WBUFL(buf, 8) = target_id;
			WBUFL(buf, 12) = gettick();
			WBUFL(buf, 16) = sd->speed;
			if (n >= 0)
				WBUFL(buf, 20) = monster->u.mons.speed;
			else
				WBUFL(buf, 20) = target_sd->speed;
			WBUFW(buf, 24) = damage;
			WBUFW(buf, 26) = skill_lvl;
			if (skill_num == 14 || skill_num == 19 || skill_num == 20 || skill_num == 90 || skill_num == 129)
				WBUFW(buf, 28) = skill_db[skill_num].type_num * skill_lvl;

			else if (skill_num == 13)
				WBUFW(buf, 28) = skill_lvl / 2 + 1;

			else if (skill_num == 267) {
				switch (skill_lvl) {
				case 1:
					WBUFW(buf, 28) = 1;
					break;
				case 2:
					if (sd->spheres == 1) WBUFW(buf, 28) = 1;
					else WBUFW(buf, 28) = 2;
					break;
				case 3:
					if (sd->spheres == 1) WBUFW(buf, 28) = 1;
					else if (sd->spheres == 2) WBUFW(buf, 28) = 2;
					else WBUFW(buf, 28) = 3;
					break;
				case 4:
					if (sd->spheres == 1) WBUFW(buf, 28) = 1;
					else if (sd->spheres == 2) WBUFW(buf, 28) = 2;
					else if (sd->spheres == 3) WBUFW(buf, 28) = 3;
					else WBUFW(buf, 28) = 4;
					break;
				case 5:
					if (sd->spheres == 1) WBUFW(buf, 28) = 1;
					else if (sd->spheres == 2) WBUFW(buf, 28) = 2;
					else if (sd->spheres == 3) WBUFW(buf, 28) = 3;
					else if (sd->spheres == 4) WBUFW(buf, 28) = 4;
					else WBUFW(buf, 28) = 5;
					break;
				}
			}
			else
				WBUFW(buf, 28) = skill_db[skill_num].type_num;

			WBUFB(buf, 30) = skill_db[skill_num].type_hit;
			mmo_map_sendarea(fd, buf, packet_len_table[0x114], 0);

			skill_do_damage(fd, damage, target_id, gettick(), 0);
  			switch(skill_num) {
			case 5: // BASH + FATAL BLOW
				if (skill_lvl > 5 && sd->status.skill[145-1].lv > 0) {
					chance_base = rand() % 100;
					if (skill_lvl == 6)
						stun_c = 95;

					else if (skill_lvl == 7)
						stun_c = 90;

					else if (skill_lvl == 8)
						stun_c = 85;

					else if (skill_lvl == 9)
						stun_c = 80;

					else
						stun_c = 75;

					if (chance_base >= stun_c) {
						if (n >= 0) {
							monster->u.mons.option_x = 3;
							set_option(NULL, m, n);
							monster->skilldata.fd = fd;
							monster->skilldata.skill_num = skill_num;
							monster->skilldata.effect |= ST_STUN;
							monster->skilldata.skill_timer[skill_num-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
						}
						else if (forplayer) {
							if (target_sd->skill_timeamount[skill_num-1][0] > 0)
								skill_reset_stats(0, 0 , target_fd, skill_num);

							target_sd->status.option_x = 3;
							set_option(target_sd, 0, 0);
							target_sd->status.effect |= ST_STUN;
							target_sd->skill_timeamount[skill_num-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
						}
					}
				}
				break;
			case 15: // FROST DIVER
				chance_base = rand() % 100;
				if (n >= 0) {
					if (mons_data[monster->class].boss == 1 || mons_data[monster->class].ele % 10 == 9)
						chance_base = 0;

					frozen_d = 100 - (35 + (skill_lvl * 3));
					if (mons_data[monster->class].ele % 10 == 1 || mons_data[monster->class].ele % 10 == 3)
						frozen_d += 20;

				}
				else
					frozen_d = 100 - (35 + (skill_lvl * 3));

				if (chance_base >= frozen_d) {
					if (n >= 0) {
						monster->u.mons.option_x = 2;
						set_option(NULL, m, n);
						monster->skilldata.fd = fd;
						monster->skilldata.skill_num = skill_num;
						monster->skilldata.effect |= ST_FROZEN;
						monster->skilldata.skill_timer[15-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
					}
					else if (forplayer) {
						if (target_sd->skill_timeamount[15-1][0] > 0)
							skill_reset_stats(0, 0 , target_fd, 15);

						target_sd->status.option_x = 2;
						set_option(target_sd, 0, 0);
						target_sd->status.effect |= ST_FROZEN;
						target_sd->skill_timeamount[15-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					}
				}
				break;
			case 58: // SPEAR TAB
				if (n >= 0) {
					for (j = 0; j < map_data[m].npc_num; j++) {
						if (map_data[m].npc[j]->block.subtype == MONS) {
							if (in_range(sd->x, sd->y, map_data[m].npc[j]->x, map_data[m].npc[j]->y, skill_db[skill_num].range)) {
								target_id = map_data[m].npc[j]->id;
								damage = skill_calc_damage(fd, skill_num, skill_lvl, target_id);

								WBUFW(buf, 0) = 0x114;
								WBUFW(buf, 2) = skill_num;
								WBUFL(buf, 4) = sd->account_id;
								WBUFL(buf, 8) = target_id;
								WBUFL(buf, 12) = gettick();
								WBUFL(buf, 16) = sd->speed;
								WBUFL(buf, 20) = map_data[m].npc[j]->u.mons.speed;
								WBUFW(buf, 24) = damage;
								WBUFW(buf, 26) = skill_lvl;
								WBUFW(buf, 28) = skill_db[skill_num].type_num;
								WBUFB(buf, 30) = skill_db[skill_num].type_hit;
								mmo_map_sendarea(fd, buf, packet_len_table[0x114], 0);

								skill_do_damage(fd, damage, map_data[m].npc[j]->id, gettick(), 0);
							}
						}
					}
				}
				break;
			case 136: // SONIC BLOW
				chance_base = rand() % 100;
				stun_c = 100 - (10 + (skill_lvl * 2));
				if (chance_base >= stun_c) {
					if (n >= 0) {
						monster->u.mons.option_x = 3;
						set_option(NULL, m, n);
						monster->skilldata.fd = fd;
						monster->skilldata.skill_num = skill_num;
						monster->skilldata.effect |= ST_STUN;
						monster->skilldata.skill_timer[136-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
					}
					else if (forplayer) {
						if (target_sd->skill_timeamount[136-1][0] > 0)
							skill_reset_stats(0, 0 , target_fd, 136);

						target_sd->status.option_x = 3;
						set_option(target_sd, 0, 0);
						target_sd->status.effect |= ST_STUN;
						target_sd->skill_timeamount[136-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					}
				}
				break;
			case 149: // SAND ATTACK
			case 253: // HOLY CROSS
				chance_base = rand() % 100;
				blind_c = 85;
				if (chance_base >= blind_c) {
					if (n >= 0) {
						monster->u.mons.option_x = 6;
						set_option(NULL, m, n);
						monster->skilldata.fd = fd;
						monster->skilldata.skill_num = skill_num;
						monster->skilldata.effect |= ST_BLIND;
						monster->skilldata.skill_timer[skill_num-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
					}
					else if (forplayer) {
						if (target_sd->skill_timeamount[skill_num-1][0] > 0)
							skill_reset_stats(0, 0 , target_fd, skill_num);

						target_sd->status.option_x = 6;
						set_option(target_sd, 0, 0);
						target_sd->status.effect |= ST_BLIND;
						target_sd->skill_timeamount[skill_num-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					}
				}
				break;
			case 152: // THROW STONE
				chance_base = rand() % 100;
				stun_c = 95;
				if (chance_base >= stun_c) {
					if (n >= 0) {
						monster->u.mons.option_x = 3;
						set_option(NULL, m, n);
						monster->skilldata.fd = fd;
						monster->skilldata.skill_num = skill_num;
						monster->skilldata.effect |= ST_STUN;
						monster->skilldata.skill_timer[152-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
					}
					else if (forplayer)  {
						if (target_sd->skill_timeamount[152-1][0] > 0)
							skill_reset_stats(0, 0 , target_fd, 136);

						target_sd->status.option_x = 3;
						set_option(target_sd, 0, 0);
						target_sd->status.effect |= ST_STUN;
						target_sd->skill_timeamount[152-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					}
				}
				break;
			case 263: // TRIPLE ATTACK
				delay = 1300;
				skill_combodelay(fd, delay);
				break;
			case 266: // OCCULT IMPACT
				for (j = 1; j <= sd->spheres; j++) {
					sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
					sd->spheres_timeamount[j] = -1;
				}
				sd->spheres--;
				skill_callspirits(fd, sd->spheres);
				break;
			case 267: // THROW SPIRITS SPHERES
				i = 0;
				switch (skill_lvl) {
				case 1:
					do {
						for (j = 1; j <= 5; j++) {
							sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
							sd->spheres_timeamount[j] = -1;
						}
						i++;
					}
					while (i == 1);
					sd->spheres--;
					break;
				case 2:
					do {
						for (j = 1; j <= 5; j++) {
							sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
							sd->spheres_timeamount[j] = -1;
						}
						i++;
					}
					while (i == 2);
					sd->spheres -= 2;
					break;
				case 3:
					do {
						for (j = 1; j <= 5; j++) {
							sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
							sd->spheres_timeamount[j] = -1;
						}
						i++;
					}
					while (i == 3);
					sd->spheres -= 3;
					break;
				case 4:
					do {
						for (j = 1; j <= 5; j++) {
							sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
							sd->spheres_timeamount[j] = -1;
						}
						i++;
					}
					while (i == 4);
					sd->spheres -= 4;
					break;
				case 5:
					do {
						for (j = 1; j <= 5; j++) {
							sd->spheres_timeamount[j-1] = sd->spheres_timeamount[j];
							sd->spheres_timeamount[j] = -1;
						}
						i++;
					}
					while (i == 5);
					sd->spheres = 0;
					break;
				}
				skill_callspirits(fd, sd->spheres);
				break;
			}
			if (n >= 0) {
				if ((monster->skilldata.effect & ST_FROZEN) && skill_num != 15)
					skill_reset_monster_stats(0, 0, m, n);

				else if ((monster->skilldata.effect & ST_STUN) && !(skill_num == 5 || skill_num == 110 || skill_num == 136 || skill_num == 152))
					skill_reset_monster_stats(0, 0, m, n);

			}
			else {
				if ((target_sd->status.effect & ST_FROZEN) && skill_num != 15)
					skill_reset_stats(0, 0, target_fd, 15);

				if ((target_sd->status.effect & ST_STUN) && !(skill_num == 5 || skill_num == 110 || skill_num == 136 || skill_num == 152))
					skill_reset_stats(0, 0, target_fd, 5);

			}
			break;


		case 1: // EFFECT SKILLS
			switch (skill_num) {
			case 6: // PROVOKE
				if (n >= 0) {
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

				sd->skill_timeamount[8-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				skill_icon_effect(sd, 1, 1);
				break;

			case 10: // SIGHT
				if (sd->skill_timeamount[10-1][0] > 0)
					skill_reset_stats(0, 0, fd, 10);

				for (i = 5; i < FD_SETSIZE; i++) {
					if (!session[i] || !(target_sd = session[i]->session_data))
						continue;

					target_sd = session[i]->session_data;
					target_fd = i;
					if (target_sd->hidden && target_sd->mapno == sd->mapno
					&& target_sd->x >= sd->x - 10 && target_sd->x <= sd->x + 10
					&& target_sd->y >= sd->y - 10 && target_sd->y <= sd->y + 10) {
						sd->skill_timeamount[51-1][0] = 0;
						skill_reset_stats(0, 0, target_fd, 51);
					}
				}
				for (j = 0; j < map_data[m].npc_num; j++) {
					if (map_data[m].npc[j]->block.subtype == MONS) {
						if (map_data[m].npc[j]->u.mons.hidden && map_data[m].npc[j]->m == sd->mapno
						&& map_data[m].npc[j]->x >= sd->x - 10 && map_data[m].npc[j]->x <= sd->x + 10
						&& map_data[m].npc[j]->y >= sd->y - 10 && map_data[m].npc[j]->y <= sd->y + 10) {
							map_data[m].npc[j]->skilldata.fd = fd;
							map_data[m].npc[j]->skilldata.skill_num = 51;
							skill_reset_monster_stats(0, 0, m, j);
							if (mons_data[map_data[m].npc[n]->class].ele % 10 == 8 || mons_data[map_data[m].npc[n]->class].ele % 10 == 9)
								skill_do_damage(fd, damage, map_data[m].npc[n]->id, gettick(), 0);
						}
					}
				}
				sd->status.option_z = 1;
				set_option(sd, m, n);
				sd->skill_timeamount[10-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				break;
			case 16: // STONE CURSE
				if (n >= 0) {
					if (mons_data[map_data[m].npc[n]->class].boss == 1)
						break;

					chance_base = 20 + 4 * skill_lvl;
					chance_base = 100 - chance_base;
					if (rand() % 100 > chance_base) {
						map_data[m].npc[n]->u.mons.option_x = 1;
						set_option(NULL, m, n);
						map_data[m].npc[n]->skilldata.effect |= ST_STONE;
						map_data[m].npc[n]->skilldata.fd = fd;
						map_data[m].npc[n]->skilldata.skill_num = skill_num;
						map_data[m].npc[n]->skilldata.skill_timer[16-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
					}
				}
				break;
			case 24: // RUWACH
				if (sd->skill_timeamount[10-1][0] > 0)
					skill_reset_stats(0, 0, fd, 24);

				for (i = 5; i < FD_SETSIZE; i++) {
					if (!session[i] || !(target_sd = session[i]->session_data))
						continue;

					target_sd = session[i]->session_data;
					target_fd = i;
					if (target_sd->hidden && target_sd->mapno == sd->mapno
					&& target_sd->x >= sd->x - 10 && target_sd->x <= sd->x + 10
					&& target_sd->y >= sd->y - 10 && target_sd->y <= sd->y + 10) {
						sd->skill_timeamount[51-1][0] = 0;
						skill_reset_stats(0, 0, target_fd, 51);
					}
				}
				for (j = 0; j < map_data[m].npc_num; j++) {
					if (map_data[m].npc[j]->block.subtype == MONS) {
						if (map_data[m].npc[j]->u.mons.hidden && map_data[m].npc[j]->m == sd->mapno
						&& map_data[m].npc[j]->x >= sd->x - 10 && map_data[m].npc[j]->x <= sd->x + 10
						&& map_data[m].npc[j]->y >= sd->y - 10 && map_data[m].npc[j]->y <= sd->y + 10) {
							map_data[m].npc[j]->skilldata.fd = fd;
							map_data[m].npc[j]->skilldata.skill_num = 51;
							skill_reset_monster_stats(0, 0, m, j);
						}
					}
				}
				sd->status.option_z = 8192;
				set_option(sd, 0, 0);
				sd->skill_timeamount[10-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
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
					if (target_sd->status.hp <= 0 || target_sd->sitting == 1)
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

					if (target_sd->status.hp > target_sd->status.max_hp || (target_sd->status.hp <= 0 && damage < 0))
						target_sd->status.hp = target_sd->status.max_hp;

					len = mmo_map_set_param(target_fd, WFIFOP(target_fd, 0), SP_HP);
					if (len > 0)
						WFIFOSET(target_fd, len);

				}
				else
					skill_do_damage(fd, damage, target_id, tick, 1);

				break;
			case 29: // INCREASE AGI
				if (forplayer) {
					if (target_sd->skill_timeamount[29-1][0] > 0)
						skill_reset_stats(0, 0, target_fd, skill_num);

					target_sd->skill_timeamount[29-1][1] = 2 + skill_lvl;
					target_sd->skill_timeamount[29-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					skill_icon_effect(target_sd, 12, 1);
					mmo_map_calc_status(target_fd, 0);
				}
				else {
					if (map_data[m].npc[n]->skilldata.skill_timer[29-1][0] > 0) {
						map_data[m].npc[n]->skilldata.fd = fd;
						map_data[m].npc[n]->skilldata.skill_num = skill_num;
						skill_reset_monster_stats(gettick(), 0, m, n);
					}
					map_data[m].npc[n]->skilldata.skill_timer[29-1][1] = 2 + skill_lvl;
					if (mons_data[map_data[m].npc[n]->class].speed != 0) {
						map_data[m].npc[n]->skilldata.fd = fd;
						map_data[m].npc[n]->skilldata.skill_num = skill_num;
						map_data[m].npc[n]->u.mons.speed = map_data[m].npc[n]->u.mons.speed;
						map_data[m].npc[n]->skilldata.skill_timer[29-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
					}
				}
				break;
			case 30: // DECREASE AGI
				if (forplayer) {
					if (target_sd->skill_timeamount[29-1][0] > 0)
						skill_reset_stats(0, 0, target_fd, skill_num);

					target_sd->skill_timeamount[30-1][1] = 2 + skill_lvl;
					target_sd->skill_timeamount[30-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					skill_icon_effect(target_sd, 14, 1);
					mmo_map_calc_status(target_fd, 0);
				}
				else {
					if (map_data[m].npc[n]->skilldata.skill_timer[30-1][0] > 0) {
						map_data[m].npc[n]->skilldata.fd = fd;
						map_data[m].npc[n]->skilldata.skill_num = skill_num;
						skill_reset_monster_stats(0, 0, m, n);
					}
					if (mons_data[map_data[m].npc[n]->class].speed != 0)
						map_data[m].npc[n]->u.mons.speed = map_data[m].npc[n]->u.mons.speed * 125 / 100;

					map_data[m].npc[n]->skilldata.fd = fd;
					map_data[m].npc[n]->skilldata.skill_num = skill_num;
					map_data[m].npc[n]->skilldata.skill_timer[30-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
				}
				break;
			case 31: // HOLY WATER
				memset(&tmp_item, 0, sizeof(tmp_item));
				tmp_item.nameid = 523;
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
									skill_reset_stats(0, 0, target_fd, skill_num);

								WBUFW(buf, 0) = 0x11a;
								WBUFW(buf, 2) = skill_num;
								WBUFW(buf, 4) = heal_point;
								WBUFL(buf, 6) = target_sd->account_id;
								WBUFL(buf, 10) = target_sd->account_id;
								WBUFB(buf, 14) = 1;
								mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
								target_sd->skill_timeamount[33-1][1] = (sd->status.vit + sd->status.vit2) * (0.10 + (skill_lvl * 0.05));
								target_sd->skill_timeamount[33-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
								mmo_map_calc_status(target_fd, 0);
								skill_icon_effect(target_sd, 9, 1);
							}
						}
					}
				}
				else {
					if (sd->skill_timeamount[33-1][0] > 0)
						skill_reset_stats(0, 0, fd, skill_num);

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
				break;
			case 34: // BLESSING
				if (forplayer) {
					int inc_bless_counter;
					if (target_sd->skill_timeamount[34-1][0] > 0)
						skill_reset_stats(0, 0 , target_fd, skill_num);

					for (inc_bless_counter = 0; inc_bless_counter < skill_lvl; inc_bless_counter++)
						target_sd->skill_timeamount[34-1][1]++;

					target_sd->skill_timeamount[34-1][0] = add_timer(gettick() + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					mmo_map_calc_status(target_fd, 0);
					skill_icon_effect(target_sd, 10, 1);
				}
				else {
					if (mons_data[map_data[m].npc[n]->class].speed != 0) {
						map_data[m].npc[n]->u.mons.speed -= 50;
						if (map_data[m].npc[n]->u.mons.speed < 10)
							map_data[m].npc[n]->u.mons.speed = 10;

					}
				}
				break;
			case 35: // CURE
				if (forplayer)
					if ((target_sd->status.effect & ST_POISON))
						skill_reset_stats(0, 0, target_fd, 52);

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
					&& target_sd->y >= sd->y - 3 && target_sd->y <= sd->y + 3) {
						sd->skill_timeamount[51-1][0] = 0;
						skill_reset_stats(0, 0, target_fd, 51);
					}
				}
				for (j = 0; j < map_data[m].npc_num; j++) {
					if (map_data[m].npc[j]->block.subtype == MONS) {
						if (map_data[m].npc[j]->u.mons.hidden && map_data[m].npc[j]->m == sd->mapno
						&& map_data[m].npc[j]->x >= sd->x - 10 && map_data[m].npc[j]->x <= sd->x + 10
						&& map_data[m].npc[j]->y >= sd->y - 10 && map_data[m].npc[j]->y <= sd->y + 10) {
							map_data[m].npc[n]->skilldata.fd = fd;
							map_data[m].npc[j]->skilldata.skill_num = 51;
							skill_reset_monster_stats(0, 0, m, j);
						}
					}
				}
				if (sd->skill_timeamount[45-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				if (skill_lvl < 4)
					sd->skill_timeamount[45-1][1]= 2 + skill_lvl;

				else
					sd->skill_timeamount[45-1][1]= 1 + skill_lvl;

				sd->skill_timeamount[45-1][0] = add_timer(gettick() + skill_calc_wait(target_fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
				mmo_map_calc_status(target_fd, 0);
				skill_icon_effect(sd, 3, 1);
				break;
			case 50: // STEAL
				if (n >= 0) {
					int steal[10] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50 };
					if (rand() % 100 >= steal[sd->status.skill[50-1].lv - 1] && map_data[m].npc[n]->u.mons.steal == 0)
						mmo_map_item_steal(fd, m, n);

					else {
						mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
						if (map_data[m].npc[n]->u.mons.target_fd != fd)
							check_new_target_monster(m, n, fd);
					}
				}
				break;
			case 51: // HIDE
				for (i = 5; i < FD_SETSIZE; i++) {
					if (session[i] && (target_sd = session[i]->session_data)) {
						target_fd = i;
						if ((target_sd->status.option_z = 8192) && (target_sd->status.option_z == 1) && (target_sd->mapno == sd->mapno) &&
						(target_sd->x >= sd->x - 10) && (target_sd->x <= sd->x + 10) &&
						(target_sd->y >= sd->y - 10) && (target_sd->y <= sd->y + 10)) {
							cant = 1;
							break;
						}
					}
				}
				if (sd->hidden) {
					sd->skill_timeamount[51-1][0] = 0;
					skill_reset_stats(0, 0 , fd, 51);
					break;
				}
				if (sd->sp_count > 0)
					skill_reset_stats(0, 0 , fd, 51);

				else if (cant == 1)
					break;

				else {
					sd->status.option_z = 2;
					set_option(sd, 0, 0);
					sd->hidden = 1;
					sd->sp_count = 5;
					sd->sp_count_lvl = skill_lvl;
					sd->skill_timeamount[51-1][0] = add_timer(gettick() + 5000, skill_reset_stats, fd, skill_num);
					skill_icon_effect(sd, 4, 1);
				}
				break;
			case 52: // ENVENOM
				if (forplayer) {
					chance_base = 20 + 4 * skill_lvl;
					chance_base = 100 - chance_base;
					if (rand() % 100 > chance_base) {
		  				target_sd->status.option_y = 1;
						set_option(target_sd, 0, 0);
			    			target_sd->status.effect |= ST_POISON;
						target_sd->drain_hp = add_timer(tick + 3000, skill_drain_hp, target_fd, skill_lvl);
						timer_data[target_sd->drain_hp]->type = TIMER_INTERVAL;
						timer_data[target_sd->drain_hp]->interval = 3000;
		  				target_sd->skill_timeamount[52-1][0] = add_timer(tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					}
					WBUFW(buf, 0) = 0x114;
					WBUFW(buf, 2) = skill_num;
					WBUFL(buf, 4) = sd->account_id;
					WBUFL(buf, 8) = target_sd->account_id;
					WBUFL(buf, 12) = gettick();
					WBUFL(buf, 16) = sd->speed;
					WBUFL(buf, 20) = target_sd->speed;
					WBUFW(buf, 24) = damage;
					WBUFW(buf, 26) = skill_lvl;
					WBUFW(buf, 28) = skill_db[skill_num].type_num;
					WBUFB(buf, 30) = skill_db[skill_num].type_hit;
					mmo_map_sendarea(fd, buf, packet_len_table[0x114], 0);
				}
				else {
					chance_base = 20 + 4 * skill_lvl;
					chance_base = 100 - chance_base;
					if (rand() % 100 > chance_base) {
						map_data[m].npc[n]->u.mons.option_y = 1;
						set_option(NULL, m, n);
						map_data[m].npc[n]->skilldata.fd = fd;
						map_data[m].npc[n]->skilldata.skill_num = skill_num;
						map_data[m].npc[n]->skilldata.effect |= ST_POISON;
						map_data[m].npc[n]->skilldata.skill_timer[52-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
						map_data[m].npc[n]->skilldata.skill_timer[52-1][1] = skill_lvl;
						add_timer(gettick() + 3000, skill_drain_hp_monster, m, n);
					}
					WBUFW(buf, 0) = 0x114;
					WBUFW(buf, 2) = skill_num;
					WBUFL(buf, 4) = sd->account_id;
					WBUFL(buf, 8) = map_data[m].npc[n]->id;
					WBUFL(buf, 12) = gettick();
					WBUFL(buf, 16) = sd->speed;
					WBUFL(buf, 20) = map_data[m].npc[n]->u.mons.speed;
					WBUFW(buf, 24) = damage;
					WBUFW(buf, 26) = skill_lvl;
					WBUFW(buf, 28) = skill_db[skill_num].type_num;
					WBUFB(buf, 30) = skill_db[skill_num].type_hit;
					mmo_map_sendarea(fd, buf, packet_len_table[0x114], 0);
				}
				skill_do_damage(fd, damage, target_id, tick, 0);
				break;
			case 53: // DETOXIFY
				if (forplayer) {
					if ((target_sd->status.effect & ST_POISON)) {
						if (target_sd->skill_timeamount[52-1][0] > 0)
							skill_reset_stats(0, 0, target_fd, 52);

						else {
		  					target_sd->status.option_y = 0;
							set_option(target_sd, 0, 0);
			    				target_sd->status.effect &= ~ST_POISON;
						}
					}
				}
				else {
					if ((map_data[m].npc[n]->skilldata.effect & ST_POISON)) {
						map_data[m].npc[n]->u.mons.option_y = 0;
						set_option(NULL, m, n);
						map_data[m].npc[n]->skilldata.fd = fd;
						map_data[m].npc[n]->skilldata.skill_num = 52;
						skill_reset_monster_stats(0, 0, m, n);
					}
				}
				break;
			case 54: // RESURECTION
				if (forplayer) {
					if (target_sd->account_id != sd->account_id && target_sd->sitting == 1) {
						damage *= (((target_sd->status.max_hp * 16.6 * (skill_lvl - 1)) / 100) + 1); // 16%
						skill_do_damage(fd, damage, target_id, gettick(), 0);

						WBUFW(buf, 0) = 0x148;
						WBUFL(buf, 2) = target_id;
						mmo_map_sendarea(fd, buf, packet_len_table[0x148], 0);
					}
				}
				else {
					chance_base = 20 + 4 * skill_lvl;
					chance_base = 100 - chance_base;
					if (rand() % 100 > chance_base)
						if (mons_data[map_data[m].npc[n]->class].ele % 10 == 8 || mons_data[map_data[m].npc[n]->class].ele % 10 == 9)
							skill_do_damage(fd, map_data[m].npc[n]->u.mons.hp, target_id, tick, 0);

				}
				break;
			case 60: // TWO_HAND_QUICKEN
				if (sd->skill_timeamount[60-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[60-1][1] = 70;
				sd->skill_timeamount[60-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 2, 1);
				break;
			case 61: // AUTO COUNTER
				if (sd->skill_timeamount[61-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[61-1][1] = sd->status.critical;
				sd->skill_timeamount[61-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				skill_icon_effect(sd, 152, 1);
				break;
			case 66: // IMPOSITIO MANGUS
				if (forplayer) {
					if (target_sd->skill_timeamount[66-1][0] > 0)
						skill_reset_stats(0, 0, target_fd, skill_num);

					target_sd->skill_timeamount[66-1][1] = 5 * skill_lvl;
					target_sd->skill_timeamount[66-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					mmo_map_calc_status(target_fd, 0);
					skill_icon_effect(target_sd, 15, 1);
				}
				break;
			case 67: // SUFFRAGIUM
				if (forplayer) {
					if (target_sd->skill_timeamount[67-1][0] > 0)
						skill_reset_stats(0, 0 , target_fd, skill_num);

					target_sd->skill_timeamount[67-1][1] = 15 * skill_lvl;
					target_sd->skill_timeamount[67-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					mmo_map_calc_status(target_fd, 0);
					skill_icon_effect(target_sd, 16, 1);
				}
				break;
			case 68: // ASPERCIO
				if (forplayer) {
					if (target_sd->skill_timeamount[68-1][0] > 0)
						skill_reset_stats(0, 0 , target_fd, skill_num);

					target_sd->skill_timeamount[68-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					skill_icon_effect(target_sd, 17, 1);
				}
				break;
			case 71: // SLOW POISON
				if (forplayer)
					if ((target_sd->status.effect & ST_POISON))
						if (target_sd->drain_hp > 0)
							timer_data[target_sd->drain_hp]->interval = 5000;


				break;
			case 72: // STATUS RECOVERY
				if (forplayer) {
					if (target_sd->skill_timeamount[15-1][0] > 0)
						skill_reset_stats(0, 0, fd, 15);

					if (target_sd->skill_timeamount[52-1][0] > 0)
						skill_reset_stats(0, 0, fd, 52);

					target_sd->status.option_x = 0;
					target_sd->status.option_y = 0;
					target_sd->status.option_z = 0;
					set_option(target_sd, 0, 0);
					target_sd->status.effect = 00000000;
				}
				else {
					map_data[m].npc[n]->skilldata.fd = fd;
					map_data[m].npc[n]->skilldata.skill_num = 1;
					skill_reset_monster_stats(0, 0, m, n);
				}
				break;
			case 73: // KYRIE
				if (sd->status.party_status > 0) {
					for (i = 5; i < FD_SETSIZE; i++) {
						if (!session[i] || !(target_sd = session[i]->session_data))
							continue;

						target_sd = session[i]->session_data;
						target_fd = i;
						if (target_sd->status.party_id == sd->status.party_id) {
							if (target_sd->mapno == sd->mapno &&
							target_sd->x >= sd->x - 13 && target_sd->x <= sd->x + 13 &&
							target_sd->y >= sd->y - 10 && target_sd->y <= sd->y + 10) {
								if (target_sd->skill_timeamount[73-1][0] > 0)
									skill_reset_stats(0, 0, target_fd, skill_num);

								WBUFW(buf, 0) = 0x11a;
								WBUFW(buf, 2) = skill_num;
								WBUFW(buf, 4) = heal_point;
								WBUFL(buf, 6) = target_sd->account_id;
								WBUFL(buf, 10) = target_sd->account_id;
								WBUFB(buf, 14) = 1;
								mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
								if (skill_lvl == 1)
									target_sd->skill_timeamount[73-1][1] = 5;

								else if (skill_lvl == 2 || skill_lvl == 3)
									target_sd->skill_timeamount[73-1][1] = 6;

								else if (skill_lvl == 4 || skill_lvl == 5)
									target_sd->skill_timeamount[73-1][1] = 7;

								else if (skill_lvl == 6 || skill_lvl == 7)
									target_sd->skill_timeamount[73-1][1] = 8;

								else if (skill_lvl == 8 || skill_lvl == 8)
									target_sd->skill_timeamount[73-1][1] = 9;

								else
									target_sd->skill_timeamount[73-1][1] = 10;

								target_sd->skill_timeamount[73-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
								skill_icon_effect(target_sd, 19, 1);
							}
						}
					}
				}
				else {
					if (target_sd->skill_timeamount[73-1][0] > 0)
						skill_reset_stats(0, 0, target_fd, skill_num);

					WBUFW(buf, 0) = 0x11a;
					WBUFW(buf, 2) = skill_num;
					WBUFW(buf, 4) = heal_point;
					WBUFL(buf, 6) = target_sd->account_id;
					WBUFL(buf, 10) = sd->account_id;
					WBUFB(buf, 14) = 1;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
					if (skill_lvl == 1)
						target_sd->skill_timeamount[73-1][1] = 5;

					else if (skill_lvl == 2 || skill_lvl == 3)
						target_sd->skill_timeamount[73-1][1] = 6;

					else if (skill_lvl == 4 || skill_lvl == 5)
						target_sd->skill_timeamount[73-1][1] = 7;

					else if (skill_lvl == 6 || skill_lvl == 7)
						target_sd->skill_timeamount[73-1][1] = 8;

					else if (skill_lvl == 8 || skill_lvl == 8)
						target_sd->skill_timeamount[73-1][1] = 9;

					else
						target_sd->skill_timeamount[73-1][1] = 10;

					target_sd->skill_timeamount[73-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
					skill_icon_effect(target_sd, 19, 1);
				}
				break;
			case 74: // MAGNIFICANT
				if (sd->status.party_status > 0) {
					for (i = 5; i < FD_SETSIZE; i++) {
						if (!session[i] || !(target_sd = session[i]->session_data))
							continue;

						target_sd = session[i]->session_data;
						target_fd = i;
						if (target_sd->status.party_id == sd->status.party_id) {
							if (target_sd->mapno == sd->mapno &&
							target_sd->x >= sd->x - 13 && target_sd->x <= sd->x + 13 &&
							target_sd->y >= sd->y - 13 && target_sd->y <= sd->y + 13) {
								if (target_sd->skill_timeamount[74-1][0] > 0)
									skill_reset_stats(0, 0, target_fd, skill_num);

								WBUFW(buf, 0) = 0x11a;
								WBUFW(buf, 2) = skill_num;
								WBUFW(buf, 4) = heal_point;
								WBUFL(buf, 6) = target_sd->account_id;
								WBUFL(buf, 10) = target_sd->account_id;
								WBUFB(buf, 14) = 1;
								mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
								target_sd->skill_timeamount[74-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
								skill_icon_effect(target_sd, 20, 1);
							}
						}
					}
				}
				else {
					if (sd->skill_timeamount[74-1][0] > 0)
						skill_reset_stats(0, 0, fd, skill_num);

					WBUFW(buf, 0) = 0x11a;
					WBUFW(buf, 2) = skill_num;
					WBUFW(buf, 4) = heal_point;
					WBUFL(buf, 6) = sd->account_id;
					WBUFL(buf, 10) = sd->account_id;
					WBUFB(buf, 14) = 1;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
					sd->skill_timeamount[74-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
					skill_icon_effect(sd, 20, 1);
				}
				break;
			case 75: // GLORIA
				if (sd->status.party_status > 0) {
					for (i = 5; i < FD_SETSIZE; i++) {
						if (!session[i] || !(target_sd = session[i]->session_data))
							continue;

						target_sd = session[i]->session_data;
						target_fd = i;
						if (target_sd->status.party_id == sd->status.party_id) {
							if (target_sd->mapno == sd->mapno &&
							target_sd->x >= sd->x - 13 && target_sd->x <= sd->x + 13 &&
							target_sd->y >= sd->y - 10 && target_sd->y <= sd->y + 10) {
								if (target_sd->skill_timeamount[75-1][0] > 0)
									skill_reset_stats(0, 0, target_fd, skill_num);

								WBUFW(buf, 0) = 0x11a;
								WBUFW(buf, 2) = skill_num;
								WBUFW(buf, 4) = heal_point;
								WBUFL(buf, 6) = target_sd->account_id;
								WBUFL(buf, 10) = target_sd->account_id;
								WBUFB(buf, 14) = 1;
								mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
								target_sd->skill_timeamount[75-1][1] = 30;
								target_sd->skill_timeamount[75-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
								mmo_map_calc_status(target_fd, 0);
								skill_icon_effect(target_sd, 21, 1);
							}
						}
					}
				}
				else {
					if (sd->skill_timeamount[75-1][0] > 0)
						skill_reset_stats(0, 0, fd, skill_num);

					WBUFW(buf, 0) = 0x11a;
					WBUFW(buf, 2) = skill_num;
					WBUFW(buf, 4) = heal_point;
					WBUFL(buf, 6) = sd->account_id;
					WBUFL(buf, 10) = sd->account_id;
					WBUFB(buf, 14) = 1;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
					sd->skill_timeamount[75-1][1] = 30;
					sd->skill_timeamount[75-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
					mmo_map_calc_status(fd, 0);
					skill_icon_effect(sd, 21, 1);
				}
				break;
			case 76: // LEX DIVINA
				if (n >= 0) {
					chance_base = 20 + 4 * skill_lvl;
					chance_base = 100 - chance_base;
					if (rand() % 100 > chance_base) {
						map_data[m].npc[n]->u.mons.option_y = 4;
						set_option(NULL, m, n);
						map_data[m].npc[n]->skilldata.effect |= ST_SILENCE;
						map_data[m].npc[n]->skilldata.fd = fd;
						map_data[m].npc[n]->skilldata.skill_num = skill_num;
						map_data[m].npc[n]->skilldata.skill_timer[76-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
					}
				}
				break;
			case 77: // TURN UNDEAD
				if (n >= 0) {
					chance_base = 20 + 4 * skill_lvl;
					chance_base = 100 - chance_base;
					if (rand() % 100 > chance_base) {
						if (mons_data[map_data[m].npc[n]->class].ele % 10 == 9)
							mons_data[map_data[m].npc[n]->class].ele = 0;

					}
				}
				break;
			case 78: // LEX AETERNA
				if (forplayer)
					target_sd->skill_timeamount[78-1][0] = 1;

				else
					map_data[m].npc[n]->u.mons.lexaeterna = 1;

				break;
			case 81: // SIGHT TRASHER
				if (skill_lvl < 3)
					cell_range = 1;

				else if ((skill_lvl >= 3) && (skill_lvl < 5))
					cell_range = 2;

				else if ((skill_lvl >= 5) && (skill_lvl < 7))
					cell_range = 3;

				else if ((skill_lvl >= 7) && (skill_lvl < 9))
					cell_range = 4;

				else
					cell_range = 5;

				for (x = -cell_range; x <= cell_range; x++) {
					for (y = -cell_range; y <= cell_range; y++) {
						if ((x == y) || (x == -y) || (x == 0) || (y == 0)) {
							for (j = 0; j < map_data[sd->mapno].npc_num; j++) {
								if (map_data[sd->mapno].npc[j]->block.subtype == MONS) {
									if (map_data[sd->mapno].npc[j]->x == (sd->x + x)) {
										if (map_data[sd->mapno].npc[j]->y == (sd->y + y)) {
											target_id = map_data[sd->mapno].npc[j]->id;
											damage = skill_calc_damage(fd, skill_num, skill_lvl, target_id);
											skill_do_damage(fd, damage, target_id, gettick(), 0);
										}
									}
								}
							}
						}
					}
				}
				break;
			case 88: // FROST NOVA
				for (j = 0; j < map_data[m].npc_num; j++) {
					if (map_data[m].npc[j]->block.subtype == MONS) {
						if (in_range(sd->x, sd->y, map_data[m].npc[j]->x, map_data[m].npc[j]->y, skill_db[skill_num].range)) {
							chance_base = rand() % 100;
							if (mons_data[map_data[m].npc[j]->class].boss == 1 || mons_data[map_data[m].npc[j]->class].ele % 10 == 9)
								chance_base = 0;

							frozen_d = 100 - (35 + (skill_lvl * 3));
							if (mons_data[map_data[m].npc[j]->class].ele % 10 == 1 || mons_data[map_data[m].npc[j]->class].ele % 10 == 3)
								frozen_d += 20;

							if (chance_base >= frozen_d) {
								map_data[m].npc[j]->u.mons.option_x = 2;
								set_option(NULL, m, j);
								map_data[m].npc[j]->skilldata.fd = fd;
								map_data[m].npc[j]->skilldata.skill_num = 15;
								map_data[m].npc[j]->skilldata.effect |= ST_FROZEN;
								map_data[m].npc[j]->skilldata.skill_timer[15-1][0] = add_timer(gettick() + skill_calc_wait(fd, 15, skill_lvl).duration, skill_reset_monster_stats, m, j);
							}
						}
					}
				}
				break;
			case 93: // SENSE
				if (n >= 0) {
					WBUFW(buf, 0) = 0x18c;
					WBUFW(buf, 2) = map_data[m].npc[n]->class;
					WBUFW(buf, 4) = mons_data[map_data[m].npc[n]->class].lv;
					WBUFW(buf, 6) = mons_data[map_data[m].npc[n]->class].scale;
					WBUFL(buf, 8) = map_data[m].npc[n]->u.mons.hp;
					WBUFW(buf, 12) = map_data[m].npc[n]->u.mons.def1;
					WBUFW(buf, 14) = mons_data[map_data[m].npc[n]->class].race;
					WBUFW(buf, 16) = mons_data[map_data[m].npc[n]->class].mdef1;
					WBUFW(buf, 18) = mons_data[map_data[m].npc[n]->class].ele % 10;
					for (j = 0; j < 9; j++)
						WBUFB(buf, 20 + j) = 100 * get_ele_attack_factor(mons_data[map_data[m].npc[n]->class].ele % 10, j + 1);

					mmo_map_sendarea(fd, buf, packet_len_table[0x18c], 0);
				}
				break;
			case 111: // ADRENALINE RUSH
				if (sd->status.party_status > 0) {
					for (i = 5; i < FD_SETSIZE; i++) {
						if (!session[i] || !(target_sd = session[i]->session_data))
							continue;

						target_sd = session[i]->session_data;
						target_fd = i;
						if (target_sd->status.party_id == sd->status.party_id) {
							if (target_sd->mapno == sd->mapno &&
							target_sd->x >= sd->x - 13 && target_sd->x <= sd->x + 13 &&
							target_sd->y >= sd->y - 13 && target_sd->y <= sd->y + 13) {
								if (target_sd->skill_timeamount[111-1][0] > 0)
									skill_reset_stats(0, 0, target_fd, skill_num);

								WBUFW(buf, 0) = 0x11a;
								WBUFW(buf, 2) = skill_num;
								WBUFW(buf, 4) = heal_point;
								WBUFL(buf, 6) = target_sd->account_id;
								WBUFL(buf, 10) = target_sd->account_id;
								WBUFB(buf, 14) = 1;
								mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
								if (target_sd->account_id == sd->account_id)
									target_sd->skill_timeamount[111-1][1] = 70;

								else
									target_sd->skill_timeamount[111-1][1] = 80;

								target_sd->skill_timeamount[111-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
								mmo_map_calc_status(target_fd, 0);
								skill_icon_effect(target_sd, 23, 1);
							}
						}
					}
				}
				else {
					if (sd->skill_timeamount[111-1][0] > 0)
						skill_reset_stats(0, 0, fd, skill_num);

					WBUFW(buf, 0) = 0x11a;
					WBUFW(buf, 2) = skill_num;
					WBUFW(buf, 4) = heal_point;
					WBUFL(buf, 6) = sd->account_id;
					WBUFL(buf, 10) = sd->account_id;
					WBUFB(buf, 14) = 1;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
					sd->skill_timeamount[111-1][1] = 70;
					sd->skill_timeamount[111-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
					mmo_map_calc_status(fd, 0);
					skill_icon_effect(sd, 23, 1);
				}
				break;
			case 112: // WEAPON PERFECTION
				if (sd->skill_timeamount[112-1][0] > 0)
					skill_reset_stats (0, 0, fd, skill_num);

				sd->skill_timeamount[112-1][1] = 1;
				sd->skill_timeamount[112-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				skill_icon_effect(sd, 24, 1);
				break;
			case 113: // POWER THRUST
				if (sd->skill_timeamount[113-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[113-1][1] = 5 * skill_lvl;
				sd->skill_timeamount[113-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 25, 1);
				break;
			case 114: // MAXIMIZE POWER
				if (sd->skill_timeamount[114-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[114-1][1] = 1;
				sd->skill_timeamount[114-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				skill_icon_effect(sd, 26, 1);
				break;
			case 135: // CLOAK
				if (sd->skill_timeamount[135-1][0] > 0) {
					sd->skill_timeamount[135-1][0] = 0;
					skill_reset_stats (0, 0, fd, skill_num);
					break;
				}
				if (skill_cloaking_check(sd)) {
					sd->skill_timeamount[135-1][0] = 0;
					skill_reset_stats (0, 0 , fd, skill_num);
					break;
				}
				sd->status.option_z = 2;
				set_option(sd, 0, 0);
				sd->sp_count = 5;
				sd->sp_count_lvl = skill_lvl;
				sd->skill_timeamount[135-1][0] = add_timer(gettick() + 5000, skill_reset_stats, fd, skill_num);
				skill_icon_effect(sd, 5, 1);
				break;
			case 138: // ENCHANT POISON
				if (sd->skill_timeamount[138-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[138-1][1] = 2 + ((1 + skill_lvl) / 2);
				sd->skill_timeamount[138-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 6, 1);
				break;
			case 139: // POISON REACT
				if (sd->skill_timeamount[153-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[153-1][0] = 1;
				skill_icon_effect(sd, 7, 1);
				break;
			case 142: // FIRST AID
				sd->status.hp += 5;

				WBUFW(buf, 0) = 0x11a;
				WBUFW(buf, 2) = skill_num;
				WBUFW(buf, 4) = heal_point;
				WBUFL(buf, 6) = sd->account_id;
				WBUFL(buf, 10) = sd->account_id;
				WBUFB(buf, 14) = 1;
				mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
				if (sd->status.hp >= sd->status.max_hp)
					sd->status.hp = sd->status.max_hp;

				len = mmo_map_set_param(target_fd, WFIFOP(target_fd, 0), SP_HP);
				if (len > 0)
					WFIFOSET(target_fd, len);

				break;
			case 143: // ACT DEAD
				if (sd->act_dead) {
					sd->sitting = 0;
					sd->act_dead = 0;
					sd->speed = DEFAULT_WALK_SPEED - (sd->status.agi + sd->status.agi2) / 5;
					WBUFW(buf, 0) = 0x8a;
					WBUFL(buf, 2) = sd->account_id;
					WBUFB(buf, 26) = 3;
					mmo_map_sendarea(fd, buf, packet_len_table[0x8a], 0);
					mmo_map_set0078(sd, buf);
					mmo_map_sendarea(fd, buf, packet_len_table[0x78], 1);
					skill_icon_effect(sd, 29, 0);
				}
				else {
					sd->sitting = 1;
					sd->act_dead = 1;
					sd->speed = 0;
					mmo_map_set0078(sd, buf);
					mmo_map_sendarea(fd, buf, packet_len_table[0x78], 1);
					skill_icon_effect(sd, 29, 1);
				}
				break;
			case 149: // SAND ATTACK
				break;
			case 150: // BACK SLIDING
				if (sd->dir == 4)
					sd->y = sd->y + 5;
				if (sd->dir == 5) {
					sd->x = sd->x - 5;
					sd->y = sd->y + 5;
				}
				if (sd->dir == 6)
					sd->x = sd->x - 5;
				if (sd->dir == 7) {
					sd->x = sd->x - 5;
					sd->y = sd->y - 5;
				}
				if (sd->dir == 0)
					sd->y = sd->y - 5;
				if (sd->dir == 1) {
					sd->x = sd->x + 5;
					sd->y = sd->y - 5;
				}
				if (sd->dir == 2)
					sd->x = sd->x + 5;
				if (sd->dir == 3) {
					sd->x = sd->x + 5;
					sd->y = sd->y + 5;
				}
				WBUFW(buf, 0) = 0x88;
				WBUFL(buf, 2) = sd->account_id;
				WBUFW(buf, 6) = sd->x;
				WBUFW(buf, 8) = sd->y;
				mmo_map_sendarea(fd, buf, packet_len_table[0x88], 0);
				break;
			case 151: // FIND STONE
				memset(&tmp_item, 0, sizeof(tmp_item));
				tmp_item.nameid=7049;
				tmp_item.amount=1;
				tmp_item.identify=1;
				mmo_map_get_item(fd, &tmp_item);
				break;
			case 154: // CHANGE CART
				WBUFW(buf, 0) = 0x11a;
				WBUFW(buf, 2) = skill_num;
				WBUFW(buf, 4) = heal_point;
				WBUFL(buf, 6) = sd->account_id;
				WBUFL(buf, 10) = sd->account_id;
				WBUFB(buf, 14) = 1;
				mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
				break;
			case 155: // CRAZY UPROAR
				if (sd->skill_timeamount[155-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[155-1][1] = 4;
				sd->skill_timeamount[155-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 30, 1);
				break;
			case 157: // ENERGY COAT
				if (sd->skill_timeamount[157-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[157-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				skill_icon_effect(sd, 31, 1);
				break;
			case 249: // GUARD
				if (sd->skill_timeamount[249-1][0] > 0) {
					skill_reset_stats(0, 0, fd, skill_num);
					break;
				}
				sd->status.effect |= ST_STUN;
				add_timer(gettick() + 1 * 3000, skill_reset_stats, fd, 136);
				sd->skill_timeamount[249-1][0] = 1;
				skill_icon_effect(sd, 58, 1);
				break;
			case 252: // REFLECT SHIELD
				if (sd->skill_timeamount[252-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[252-1][1] = skill_lvl;
				sd->skill_timeamount[252-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				skill_icon_effect(sd, 59, 1);
				break;
			case 255: // SACRIFICE
				break;
			case 256: // PROVIDENCE
				if (sd->skill_timeamount[256-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[256-1][0] = 1;
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 61, 1);
				break;
			case 257: // DEFENDER
				if (sd->skill_timeamount[256-1][0] > 0) {
					skill_reset_stats(0, 0, fd, skill_num);
					break;
				}
				sd->skill_timeamount[257-1][1] = 10; 
				sd->skill_timeamount[257-1][0] = 1;
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 62, 1);
				break;
			case 258: // SPEAR QUICKEN
				if (sd->skill_timeamount[258-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[258-1][1] = 70;
				sd->skill_timeamount[258-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 68, 1);
				break;
			case 261: // SUMMON SPIRIT SPHERES
				sd->spheres++;
				skill_callspirits(fd, sd->spheres);
				sd->spheres_timeamount[sd->spheres-1] = add_timer(tick + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				break;
			case 262: // ABSORB SPIRITS
				switch(skill_lvl) {
				case 1: case 2: case 3: case 4: case 5:
					temp_sp = 7*sd->spheres;
					break;
				}
				if ((sd->status.sp+temp_sp) > sd->status.max_sp) {
					temp_sp = sd->status.max_sp - sd->status.sp;
					sd->status.sp = sd->status.max_sp;
				}
				else
					sd->status.sp += temp_sp;

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
			case 268: // MENTAL STRENGTH
			{
				int ret;
				if (sd->skill_timeamount[268-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				for (ret = 0; ret < skill_lvl; ret++)
					sd->skill_timeamount[268-1][1]++;

				sd->skill_timeamount[268-1][1] = skill_lvl * 2;
				sd->skill_timeamount[268-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 69, 1);
				sd->spheres = 0;
				skill_callspirits(fd, sd->spheres);
				break;
			}
			case 269: // ROOT
				if (sd->skill_timeamount[269-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				if (n >= 0) {
					if (boss_monster(m, n))
						break;

					map_data[m].npc[n]->skilldata.fd = fd;
					map_data[m].npc[n]->skilldata.skill_num = skill_num;
					map_data[m].npc[n]->skilldata.effect |= ST_STUN;
					map_data[m].npc[n]->skilldata.skill_timer[skill_num-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_monster_stats, m, n);
					sd->speed = 0;
					sd->spheres--;
					skill_callspirits(fd, sd->spheres);
					sd->skill_timeamount[269-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				}
				break;
			case 270: // FURY
				if (sd->skill_timeamount[270-1][0] > 0)
					skill_reset_stats(0, 0, fd, skill_num);

				sd->skill_timeamount[270-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
				sd->skill_timeamount[270-1][1] = skill_lvl;
				mmo_map_calc_status(fd, 0);
				skill_icon_effect(sd, 70, 1);
				break;
			default:
				debug_output("case_1 UNHANDLED_SKILL: %d\n", skill_num);
				break;
			}
			if (skill_num != 28 && skill_num != 33 && skill_num!= 74 &&	skill_num != 75 && skill_num != 73 && skill_num != 111 && skill_num != 154) {
				WBUFW(buf, 0) = 0x11a;
				WBUFW(buf, 2) = skill_num;
				WBUFW(buf, 4) = heal_point;
				WBUFL(buf, 6) = target_id;
				WBUFL(buf, 10) = sd->account_id;
				WBUFB(buf, 14) = 1;
				mmo_map_sendarea(fd, buf, packet_len_table[0x11a], 0);
			}
			break;


		case 2: // BLOW SKILLS
			switch (skill_num) {
			case 7: // MAGNUM BREAK
				j = square_collection(sd->x, sd->y, sd->mapno, 2, target);
				for (i = 0; i < j; i++) {
					WBUFW(buf, 0) = 0x115;
					WBUFW(buf, 2) = skill_num;
					WBUFL(buf, 4) = sd->account_id;
					WBUFL(buf, 8) = target[i]->id;
					WBUFL(buf, 12) = gettick();
					WBUFL(buf, 16) = sd->speed;
					WBUFL(buf, 20) = target[i]->u.mons.speed;
					WBUFW(buf, 24) = target[i]->x;
					WBUFW(buf, 26) = target[i]->y;
					WBUFW(buf, 28) = damage;
					WBUFW(buf, 30) = skill_lvl;
					WBUFW(buf, 32) = skill_db[skill_num].type_num;
					WBUFB(buf, 34) = skill_db[skill_num].type_hit;
					mmo_map_sendarea(fd, buf, packet_len_table[0x115], 0);

					skill_do_damage(fd, damage, target[i]->id, gettick(), 0);
				}
				break;
			case 84: // JUPITEL THUNDER
				WBUFW(buf, 0) = 0x115;
				WBUFW(buf, 2) = skill_num;
				WBUFL(buf, 4) = sd->account_id;
				WBUFL(buf, 8) = target_id;
				WBUFL(buf, 12) = gettick();
				WBUFL(buf, 16) = sd->status.aspd;
				if (n >= 0) {
					WBUFL(buf, 20) = monster->u.mons.speed;
					bx = monster->x;
					by = monster->y;
				}
				else {
					WBUFL(buf, 20) = target_sd->speed;
					bx = target_sd->x;
					by = target_sd->y;
				}
				dst = calc_dir(sd->x, sd->y, bx, by);
				if (dst == -2 || dst == -3 || dst == -4)
					bx -= (2 + skill_lvl);

				else if (dst == 0 || dst == 2 || dst == 3 || dst == 4)
					bx += (2 + skill_lvl);

				if (dst == -1 || dst == -4 || dst == 3)
					by -= (2 + skill_lvl);

				else if (dst == -3 || dst == 0 || dst == 1 || dst == 4)
					by += (2 + skill_lvl);

				WBUFW(buf, 24) = bx;
				WBUFW(buf, 26) = by;
				WBUFW(buf, 28) = damage;
				WBUFW(buf, 30) = skill_lvl;
				WBUFW(buf, 32) = 2 + skill_lvl;
				WBUFB(buf, 34) = skill_db[skill_num].type_hit;
				mmo_map_sendarea(fd, buf, packet_len_table[0x115], 0);

				skill_do_damage(fd, damage, target_id, gettick(), 0);
				break;
			default: // ALL OTHERS
				WBUFW(buf, 0) = 0x115;
				WBUFW(buf, 2) = skill_num;
				WBUFL(buf, 4) = sd->account_id;
				WBUFL(buf, 8) = target_id;
				WBUFL(buf, 12) = gettick();
				WBUFL(buf, 16) = sd->status.aspd;
				if (n >= 0) {
					WBUFL(buf, 20) = monster->u.mons.speed;
					bx = monster->x;
					by = monster->y;
				}
				else {
					WBUFL(buf, 20) = target_sd->speed;
					bx = target_sd->x;
					by = target_sd->y;
				}
				dst = calc_dir(sd->x, sd->y, bx, by);
				if (dst == -2 || dst == -3 || dst == -4)
					bx -= (2 + skill_lvl);

				else if (dst == 0 || dst == 2 || dst == 3 || dst == 4)
					bx += (2 + skill_lvl);

				if (dst == -1 || dst == -4 || dst == 3)
					by -= (2 + skill_lvl);

				else if (dst == -3 || dst == 0 || dst == 1 || dst == 4)
					by += (2 + skill_lvl);

				WBUFW(buf, 24) = bx;
				WBUFW(buf, 26) = by;
				WBUFW(buf, 28) = damage;
				WBUFW(buf, 30) = skill_lvl;
				WBUFW(buf, 32) = skill_db[skill_num].type_num;
				WBUFB(buf, 34) = skill_db[skill_num].type_hit;
				mmo_map_sendarea(fd, buf, packet_len_table[0x115], 0);

				skill_do_damage(fd, damage, target_id, gettick(), 0);
				break;
			}
			break;
		default:
			debug_output("UNHANDLED_TYPE_NK: %d\n", skill_db[skill_num].type_nk);
			break;
		}
	}
}

void skill_do_delayed_place(int tid, unsigned int tick, int fd, int skill_num)
{
	int i = 0, j, res, damage;
	short x, y;
	int skill_x, skill_y, skill_lvl, skill_type;
	int target_id = 0;
	int floorskill_index;
	unsigned int target_fd;
	unsigned char buf[256];
	struct item tmp_item;
	struct map_session_data *sd, *target_sd = NULL;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	sd->casting = 0;
	skill_type = search_placeskill(skill_num);
	skill_lvl = sd->skill_lvl;
	skill_x = sd->skill_x;
	skill_y = sd->skill_y;

	WBUFW(buf, 0) = 0x117;
	WBUFW(buf, 2) = skill_num;
	WBUFL(buf, 4) = sd->account_id;
	WBUFW(buf, 8) = SKILL_MIN_WAIT;
	WBUFW(buf, 10) = skill_x;
	WBUFW(buf, 12) = skill_y;
	WBUFL(buf, 14) = gettick();
	mmo_map_sendarea(fd, buf, packet_len_table[0x117], 0);

	switch (skill_num) {
	case 12: // SAFETY WALL
		i = next_floorskill_index();
		make_floorskill(fd, skill_x, skill_y, skill_lvl, skill_type, FS_SWALL, skill_num, i);
		break;
	case 18: // FIRE WALL
		i = next_floorskill_index();
		res = calc_dir2(sd->x, sd->y, skill_x, skill_y);
		if (res == 0)
			for (x = -1; x <= 1; x++)
				make_floorskill(fd, skill_x + x, skill_y, skill_lvl, skill_type, FS_FWALL, skill_num, i);


		else if (res == 1)
			for (x = skill_x + 1, y = skill_y + 1; x >= skill_x - 1; x--, y--)
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_FWALL, skill_num, i);


		else if (res == 2)
			for (x = -1; x <= 1; x++)
				make_floorskill(fd, skill_x, skill_y + x, skill_lvl, skill_type, FS_FWALL, skill_num, i);


		else if (res == 3)
			for (x = skill_x - 1, y = skill_y + 1; x <= skill_x + 1; x++, y--)
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_FWALL, skill_num, i);


		break;
	case 25: // PNEUMA
		i = next_floorskill_index();
		make_floorskill(fd, skill_x, skill_y, skill_lvl, skill_type, FS_PNEUMA, skill_num, i);
		break;
	case 27: // WARP
		WFIFOW(fd, 0)= 0x11c;
		WFIFOW(fd, 2)= skill_num;
		strcpy(WFIFOP(fd, 4), sd->status.save_point.map);
		if (skill_lvl > 1)
			strcpy(WFIFOP(fd, 20), sd->status.memo_point[0].map);

		else
			strcpy(WFIFOP(fd, 20), "");

		if (skill_lvl > 2)
			strcpy(WFIFOP(fd, 36), sd->status.memo_point[1].map);

		else
			strcpy(WFIFOP(fd, 36), "");

		if (skill_lvl > 3)
			strcpy(WFIFOP(fd, 52), sd->status.memo_point[2].map);

		else
			strcpy(WFIFOP(fd, 52), "");

		WFIFOSET(fd, packet_len_table[0x11c]);
		i = next_floorskill_index();
		make_floorskill(fd, skill_x, skill_y, skill_lvl, 0x81, FS_WARP, skill_num, i);
		break;
	case 69: // B.S SACRAMENTI
		if (sd->status.party_status > 0) {
			for (i = 5; i < FD_SETSIZE; i++) {
				if (!session[i] || !(target_sd = session[i]->session_data))
					continue;

				target_sd = session[i]->session_data;
				target_fd = i;
				if (target_sd->status.party_id == sd->status.party_id) {
					if (target_sd->mapno == sd->mapno && (target_sd->x >= sd->x - 13) && (target_sd->x <= sd->x + 13) && (target_sd->y >= sd->y - 10) && (target_sd->y <= sd->y + 10)) {
						if (target_sd->skill_timeamount[69-1][0] > 0)
							skill_reset_stats(0, 0, target_fd, skill_num);

						target_sd->skill_timeamount[69-1][0] = add_timer(gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, target_fd, skill_num);
						target_sd->skill_timeamount[69-1][1] = skill_lvl + 1;			
						skill_icon_effect(target_sd, 18, 1);
					}
				}
			}
		}
		else {
			if (sd->skill_timeamount[69-1][0] > 0)
				skill_reset_stats(0, 0, fd, skill_num);

			sd->skill_timeamount[69-1][0] = add_timer (gettick() + skill_calc_wait(fd, skill_num, skill_lvl).duration, skill_reset_stats, fd, skill_num);
			sd->skill_timeamount[69-1][1] = skill_lvl + 1;			
			skill_icon_effect(sd, 18, 1);
		}
		break;
	case 70: // SANCTUARY
		i = next_floorskill_index();
		for (x = skill_x - 2; x <= skill_x + 2; x++) {
			for (y = skill_y - 2; y <= skill_y + 2; y++) {
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
							damage = skill_calc_damage(fd, skill_num, skill_lvl, target_id);

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
							WBUFL(buf, 20) = map_data[sd->mapno].npc[j]->u.mons.speed;
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
		if (res == 0)
			for (x = -2; x <= 2; x++)
				make_floorskill(fd, skill_x + x, skill_y, skill_lvl, skill_type, FS_ICEWALL, skill_num, i);


		else if (res == 1)
			for (x = skill_x + 2, y = skill_y + 2; x >= skill_x - 2; x--, y--)
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_ICEWALL, skill_num, i);


		else if (res == 2)
			for (x = -2; x <= 2; x++)
				make_floorskill(fd, skill_x, skill_y + x, skill_lvl, skill_type, FS_ICEWALL, skill_num, i);


		else if (res == 3)
			for (x = skill_x - 2, y = skill_y + 2; x <= skill_x + 2; x++, y--)
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_ICEWALL, skill_num, i);


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
							WBUFL(buf, 20) = map_data[sd->mapno].npc[j]->u.mons.speed;
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
		for (x = skill_x - 2; x <= skill_x + 2; x++)
			for (y = skill_y - 2; y <= skill_y + 2; y++)
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_QUAG, skill_num, i);


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
						if (map_data[sd->mapno].npc[j]->x == (skill_x + x) &&
						    map_data[sd->mapno].npc[j]->y == (skill_y + y)) {
							target_id = map_data[sd->mapno].npc[j]->id;
							if ((rand() % 100) >= (100 -(20 + (skill_lvl * 10)))) {
		  						map_data[sd->mapno].npc[j]->u.mons.option_x = 3;
								set_option(NULL, sd->mapno, j);
								map_data[sd->mapno].npc[j]->skilldata.fd = fd;
								map_data[sd->mapno].npc[j]->skilldata.skill_num = skill_num;
								map_data[sd->mapno].npc[j]->skilldata.effect |= ST_STUN;
								map_data[sd->mapno].npc[j]->skilldata.skill_timer[110-1][0] = add_timer(gettick() + 15000, skill_reset_monster_stats, sd->mapno, j);
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
	case 125: // TALKIEBOX
		i = next_floorskill_index();
		make_floorskill(fd, sd->skill_x, sd->skill_y, skill_lvl, skill_type, FS_TRAP, skill_num, i);
		break;
	case 124: // REMOVE TRAP
		floorskill_index = search_floorskill_index(sd->mapno, skill_x, skill_y);
		if (floorskill_index > -1) {
			switch (floorskill[floorskill_index].type) {
			case FS_TRAP:
				if (floorskill[floorskill_index].trap_used == 0)
					floorskill[floorskill_index].trap_used = 1;

				else {
					WBUFW(buf, 0) = 0x11f;
					WBUFL(buf, 2) = target_id;
					WBUFL(buf, 6) = sd->account_id;
					WBUFW(buf, 10) = skill_x;
					WBUFW(buf, 12) = skill_y;
					WBUFB(buf, 14) = skill_type;
					WBUFB(buf, 15) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);
					break;
				}
				switch(floorskill[floorskill_index].skill_num) {
				case 117: // ANKLE SNARE
				case 115: // SKIDTRAP
				case 116: // LANDMINE
				case 118: // SHOCKWAVE
				case 119: // SANDMAN
				case 120: // FLASHER
				case 121: // FREEZING
				case 122: // BLASTMINE
				case 123: // CLAYMORE
				case 125: // TALKIEBOX
					memset(&tmp_item, 0, sizeof(tmp_item));
					tmp_item.nameid = 1065;
					tmp_item.amount = 1;
					tmp_item.identify = 1;
					mmo_map_get_item(fd, &tmp_item);
					WBUFW(buf, 0) = 0x11f;
					WBUFL(buf, 2) = target_id;
					WBUFL(buf, 6) = sd->account_id;
					WBUFW(buf, 10) = skill_x;
					WBUFW(buf, 12) = skill_y;
					WBUFB(buf, 14) = skill_type;
					WBUFB(buf, 15) = 1;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);
					if (floorskill[floorskill_index].timer > 0)
						delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

					remove_floorskill(0, tick, 0, floorskill_index);
					break;
				}
			}
			break;
		}
		break;
	case 130: // DETECT
		WBUFW(buf, 0) = 0x11f;
		WBUFL(buf, 2) = target_id;
		WBUFL(buf, 6) = sd->account_id;
		WBUFW(buf, 10) = skill_x;
		WBUFW(buf, 12) = skill_y;
		WBUFB(buf, 14) = skill_type;
		WBUFB(buf, 15) = 1;
		mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);
		for (i = 5; i < FD_SETSIZE; i++) {
			if (!session[i] || !(target_sd = session[i]->session_data))
				continue;

			target_sd = session[i]->session_data;
			target_fd = i;
			if (target_sd->hidden && target_sd->mapno == sd->mapno
			&& in_range(skill_x, skill_y, target_sd->x, target_sd->y, skill_lvl)) {
				sd->skill_timeamount[51-1][0] = 0;
				skill_reset_stats(0, 0, target_fd, 51);
			}
		}
		for (j = 0; j < map_data[sd->mapno].npc_num; j++) {
			if (map_data[sd->mapno].npc[j]->block.subtype == MONS) {
				if (map_data[sd->mapno].npc[j]->u.mons.hidden && map_data[sd->mapno].npc[j]->m == sd->mapno
				&& in_range(skill_x, skill_y, map_data[sd->mapno].npc[j]->x, map_data[sd->mapno].npc[j]->y, skill_lvl)) {
					map_data[sd->mapno].npc[j]->skilldata.fd = fd;
					map_data[sd->mapno].npc[j]->skilldata.skill_num = 51;
					skill_reset_monster_stats(0, 0, sd->mapno, j);
				}
			}
		}
		break;
	case 131: // SPRINGTRAP
		floorskill_index = search_floorskill_index(sd->mapno, skill_x, skill_y);
		if (floorskill_index > -1) {
			switch (floorskill[floorskill_index].type) {
			case FS_TRAP:
				if (floorskill[floorskill_index].trap_used == 0)
					floorskill[floorskill_index].trap_used = 1;

				else {
					WBUFW(buf, 0) = 0x11f;
					WBUFL(buf, 2) = target_id;
					WBUFL(buf, 6) = sd->account_id;
					WBUFW(buf, 10) = skill_x;
					WBUFW(buf, 12) = skill_y;
					WBUFB(buf, 14) = skill_type;
					WBUFB(buf, 15) = 0;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);
					break;
				}
				switch(floorskill[floorskill_index].skill_num) {
				case 117: // ANKLE SNARE
				case 115: // SKIDTRAP
				case 116: // LANDMINE
				case 118: // SHOCKWAVE
				case 119: // SANDMAN
				case 120: // FLASHER
				case 121: // FREEZING
				case 122: // BLASTMINE
				case 123: // CLAYMORE
				case 125: // TALKIEBOX
					memset(&tmp_item, 0, sizeof(tmp_item));
					tmp_item.nameid = 1065;
					tmp_item.amount = 1;
					tmp_item.identify = 1;
					mmo_map_get_item(fd, &tmp_item);
					WBUFW(buf, 0) = 0x11f;
					WBUFL(buf, 2) = target_id;
					WBUFL(buf, 6) = sd->account_id;
					WBUFW(buf, 10) = skill_x;
					WBUFW(buf, 12) = skill_y;
					WBUFB(buf, 14) = skill_type;
					WBUFB(buf, 15) = 1;
					mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);
					if (floorskill[floorskill_index].timer > 0)
						delete_timer(floorskill[floorskill_index].timer, remove_floorskill);

					remove_floorskill(0, tick, 0, floorskill_index);
					break;
				}
			}
			break;
		}
		break;
	case 140: // VENOM DUST
		i = next_floorskill_index();
		for (x = skill_x - 1; x <= skill_x + 1; x++) {
			for (y = skill_y - 1; y <= skill_y + 1; y++) {
				if ((x == skill_x - 1 && y == skill_y - 1) ||
				    (x == skill_x + 1 && y == skill_y + 1) ||
				    (x == skill_x - 1 && y == skill_y + 1) ||
				    (x == skill_x + 1 && y == skill_y - 1)) {
					continue;
				}
				make_floorskill(fd, x, y, skill_lvl, skill_type, FS_VENOM, skill_num, i);
			}
		}
		break;
	default:
		for (x = -5; x <= 5; x++) {
			for (y = -5; y <= 5; y++) {
				for (j = 0; j < map_data[sd->mapno].npc_num; j++) {
					if (map_data[sd->mapno].npc[j]->block.subtype == MONS) {
						if (map_data[sd->mapno].npc[j]->x == (skill_x + x) &&
						    map_data[sd->mapno].npc[j]->y == (skill_y + y)) {
							target_id = map_data[sd->mapno].npc[j]->id;
							damage = skill_calc_damage(fd, skill_num, skill_lvl, target_id);

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
							WBUFL(buf, 20) = map_data[sd->mapno].npc[j]->u.mons.speed;
							WBUFW(buf, 24) = damage;
							WBUFW(buf, 26) = skill_lvl;
							if (skill_num == 91)
								WBUFW(buf, 28) = skill_db[skill_num].type_num * skill_lvl;

							else
								WBUFW(buf, 28) = skill_db[skill_num].type_num;

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
}

void check_cast_sensor(unsigned int fd, short m, short skill_x, short skill_y, short range)
{
	int j;

	for (j = 0; j < map_data[m].npc_num; j++) {
		if (map_data[m].npc[j]->block.subtype == MONS) {
			if (mons_data[map_data[m].npc[j]->class].cast_sense == 1 &&
			    map_data[m].npc[j]->u.mons.hp > 0 &&
			    map_data[m].npc[j]->u.mons.target_fd == 0) {
				if (map_data[m].npc[j]->x >= (skill_x - range) && map_data[m].npc[j]->x <= (skill_x + range) &&
				map_data[m].npc[j]->y >= (skill_y - range) && map_data[m].npc[j]->y <= (skill_y + range)) {
					check_new_target_monster(m, j, fd);
				}
			}
		}
	}
}

int skill_can_use(int skill_num, int *skill_lvl, unsigned int fd)
{
	int index, len;
	short reason = R_NONE;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	if (sd->sitting == 1 || sd->hidden || sd->act_dead)
		return 1;

	if (sd->no_cost_sp) {
		sd->no_cost_sp = 0;
		return 1;
	}
	if (sd->status.sp >= (skill_db[skill_num].sp + extra_sp_cost(skill_num, *skill_lvl))) {
		if (sd->status.skill[skill_num-1].lv < (unsigned)*skill_lvl)
			*skill_lvl = sd->status.skill[skill_num-1].lv;

		switch (skill_num) {
		case 7: /* MAGNUM BREAK */
			sd->status.hp -= (20 - ((*skill_lvl - 1) / 2));
			if (sd->status.hp < 0) {
				mmo_player_send_death(sd);
				sd->status.hp = 0;
			}
			len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_HP);
			if (len > 0)
				WFIFOSET(fd, len);

			break;
		case 16: /* STONE CURSE */
		case 140: /* VENOM DUST */
			index = search_item2(sd, 716);
			if (index)
				mmo_map_lose_item(fd, 0, index, 1);

			else
				reason = R_NOREDG;

			break;
		case 31: /* HOLY WATER */
			if (mmo_map_flagload(sd->mapname, WATER)) {
				reason = R_CANT;
				break;
			}
			index = search_item2(sd, 713);
			if (index)
				mmo_map_lose_item(fd, 0, index, 1);

			else
				reason = R_CANT;

			break;
		case 68: /* ASPERCIO */
			index = search_item2(sd, 523);
			if (index && sd->account_id != sd->attacktarget)
				mmo_map_lose_item(fd, 0, index, 1);

			else
				reason = R_CANT;

			break;
		case 41: /* VENDING */
			if (sd->status.special != 0x08)
				if (sd->status.special != 0x80)
					if (sd->status.special != 0x100)
						if (sd->status.special != 0x200)
							if (sd->status.special != 0x400)
								reason = R_CANT;




			break;
		case 42: /* MAMONITE */
			if (sd->status.zeny >= (*skill_lvl * 100))
				mmo_map_update_param(fd, SP_ZENY, (*skill_lvl * -100));

			else
				reason = R_NOZENY;

			break;
		case 56: /* PIERCE */
		case 58: /* SPEAR TAB */
		case 59: /* SPEAR BOOMERANG */
		case 258: /* SPEAR QUICKEN */
			if (sd->status.weapon != 4 && sd->status.weapon != 5)
				reason = R_NOWEAPON;

			break;
		case 57: /* BRANDISH SPEAR */
			if (sd->status.weapon != 4 && sd->status.weapon != 5)
				reason = R_NOWEAPON;

			if (sd->status.special != 0x20)
				reason = R_CANT;

			break;
		case 60: /* TWO HAND QUICKEN */
			if (sd->status.weapon != 3)
				reason = R_NOWEAPON;

			break;
		case 67: /* SUFFRAGIUM */
			if (sd->account_id == sd->attacktarget)
				reason = R_CANT;

			break;
		case 12: /* SAFETY WALL */
		case 27: /* WARP */
		case 54: /* RESURRECTION */
		case 70: /* SANCTUARY */
		case 79: /* MAGNUS EXORCISM */
		case 80: /* FIREPILLAR */
			index = search_item2(sd, 717);
			if (index)
				mmo_map_lose_item(fd, 0, index, 1);

			else
				reason = R_NOBLUEG;

			break;
		case 115: /* SKIDTRAP */
		case 116: /* LANDMINE */
		case 117: /* ANKLE SNARE */
		case 118: /* SHOCKWAVE */
		case 119: /* SANDMAN */
		case 120: /* FLASHER */
		case 121: /* FREEZING */
		case 122: /* BLASTMINE */
		case 123: /* CLAYMORE */
		case 125: /* TALKIEBOX */
			index = search_item2(sd, 1065);
			if (index && sd->account_id != sd->attacktarget)
				mmo_map_lose_item(fd, 0, index, 1);

			else
				reason = R_CANT;

			break;
		case 129: /* BLITZBEAT */
			if (sd->status.special != 0x10)
				reason = R_CANT;

			if ((rand() % 100) < (sd->status.luk / 1000)) {
				sd->auto_blitz = 1;
				return 1;
			}
			break;
		case 130: /* DETECT */
		case 131: /* SPRING TRAP */
			if (sd->status.special != 0x10)
				reason = R_CANT;

			break;
		case 135: /* CLOAK */
			if (skill_cloaking_check(sd))
				reason = R_CANT;

			break;
		case 136: /* SONIC BLOW */
			if (sd->status.weapon != 16)
				reason = R_NOWEAPON;

			break;
		case 137: /* GRIMTOOTH */
			if (!sd->hidden && sd->skill_timeamount[135-1][0] <= 0)
				reason = R_CANT;

			if (sd->status.weapon != 16)
				reason = R_NOWEAPON;

			break;
		case 151: /* FIND STONE */
			if (sd->weight >= (sd->max_weight / 2))
				reason = R_UNKN;

			break;
		case 152: /* THROW STONE */
			index = search_item2(sd, 7049);
			if (index)
				mmo_map_lose_item(fd, 0, index, 1);

			else
				reason = R_CANT;

			break;
		case 249: /* GUARD */
		case 250: /* SHIELD CHARGED */
		case 251: /* SHIELD BOOMERANG */
		case 257: /* DEFENDER */
			if (!sd->status.shield)
				reason = R_CANT;

			break;
		case 261: /* SUMMON SPIRIT SPHERES */
			switch(*skill_lvl) {
			case 1: case 2: case 3: case 4: case 5:
				if (*skill_lvl == sd->spheres)
					reason = R_CANT;
				break;
			}
			break;
		case 262: /* ABSORB SPIRITS */
			if (!sd->spheres)
				reason = R_CANT;

			break;
		case 266: /* OCCULT IMPACT */
			if (!sd->spheres || (sd->skill_timeamount[269-1][0] > 0 && sd->status.skill[269-1].lv < 3))
				reason = R_CANT;

			break;
		case 267: /* THROW SPIRIT SPHERES */
			if (!sd->spheres || (sd->skill_timeamount[269-1][0] > 0 && sd->status.skill[269-1].lv < 2))
				reason = R_CANT;

			break;
		case 268: /* MENTAL STRENGTH */
			if (sd->skill_timeamount[268-1][0] > 0 && sd->spheres != 5)
				reason = R_CANT;

			break;
		case 269: /* ROOT */
			if (!sd->spheres)
				reason = R_CANT;

			break;
		case 271: /* RAGING QUADRUPLE */
			if (sd->skill_timeamount[269-1][0] > 0 && sd->status.skill[269-1].lv < 4)
				reason = R_CANT;

			break;
		case 272: /* GUILLOTINE FIST */
			if (sd->skill_timeamount[269-1][0] > 0 && sd->status.skill[269-1].lv < 5)
				reason = R_CANT;

			break;
		}
		if (reason == R_NONE) {
			if (sd->skilltimer_delay > 0)
				reason = R_INDELAY;

			else {
				sd->status.sp -= (skill_db[skill_num].sp + extra_sp_cost(skill_num, *skill_lvl));
				len = mmo_map_set_param(fd, WFIFOP(fd, 0), SP_SP);
				if (len > 0)
					WFIFOSET(fd, len);

				return 1;
			}
		}
	}
	else
		reason = R_NOSP;

	WFIFOW(fd, 0) = 0x110;
	WFIFOW(fd, 2) = skill_num;
	WFIFOW(fd, 4) = 5;
	WFIFOW(fd, 6) = 0;
	WFIFOB(fd, 8) = 0;
	WFIFOB(fd, 9) = reason;
	WFIFOSET(fd, packet_len_table[0x110]);
	return 0;
}

int skill_attack_place(unsigned int fd, int skill_num, int skill_lvl, unsigned long tick, int skill_x, int skill_y)
{
	int skill_wait;
	unsigned char buf[256];
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	if ((skill_can_use(skill_num, &skill_lvl, fd)) == 0)
		return 0;

	sd->casting = 1;
	skill_wait = skill_calc_wait(fd, skill_num, skill_lvl).wait;
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

			sd->skill_lvl = skill_lvl;
			sd->skill_x = skill_x;
			sd->skill_y = skill_y;

			check_cast_sensor(fd, sd->mapno, skill_x, skill_y, BLOCK_SIZE * 3 / 2);
			sd->skilltimer = add_timer(tick + skill_wait, skill_do_delayed_place, fd, skill_num);
		}
		else
			skill_do_delayed_place(0, tick, fd, skill_num);
	}
	return 0;
}

int skill_attack_target(unsigned int fd, long target_id, int skill_num, int skill_lvl, unsigned long tick)
{
	int n;
	short m;
	int target;
	int skill_wait;
	unsigned char buf[256];
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return 0;

	target = sd->account_id;
	m = sd->mapno;
	n = mmo_map_search_monster(sd->mapno, target_id);

	if ((n < 0) && !(skill_db[skill_num].type_inf == 16 || skill_db[skill_num].type_inf == 4) && !(mmo_map_flagload(sd->mapname, PVP) || PVP_flag))
		return 0;

	if ((skill_can_use(skill_num, &skill_lvl, fd)) == 0)
		return 0;

	sd->casting = 1;
	skill_wait = skill_calc_wait(fd, skill_num, skill_lvl).wait;
	sd->skill_target = target_id;
	sd->skill_lvl = skill_lvl;

	if (n >= 0 && skill_num == 28 && ((mons_data[map_data[m].npc[n]->class].ele % 10 == 8) || (mons_data[map_data[m].npc[n]->class].ele % 10 == 9)))
		skill_wait = 1000;

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

		if (n >= 0 && mons_data[map_data[m].npc[n]->class].cast_sense == 1 && map_data[m].npc[n]->u.mons.target_fd == 0)
			check_new_target_monster(m, n, fd);

		sd->skilltimer = add_timer(tick + skill_wait, skill_do_delayed_target, fd, skill_num);
	}
	else
		skill_do_delayed_target(0, tick, fd, skill_num);

  	return 0;
}

void skill_stop_wait(struct map_session_data *sd)
{
	if (sd->skilltimer > 0) {
		delete_timer(sd->skilltimer, skill_do_delayed_target);
		delete_timer(sd->skilltimer, skill_do_delayed_place);
		sd->skilltimer = 0;
	}
	sd->casting = 0;
}

int skill_cloaking_check(struct map_session_data *sd)
{
	int fail = 1;
	unsigned int i;
	short m = sd->mapno, x = sd->x, y = sd->y, cell;
	static int dx[] ={ -1, 0, 1, -1, 1, -1, 0, 1};
	static int dy[] ={ -1, -1, -1, 0, 0, 1, 1, 1};

	for (i = 0; i < sizeof(dx) / sizeof(dx[0]); i++) {
		cell = map_getcell(m, x + dx[i], y + dy[i]);
		if (cell == 1 || cell == 5)
			fail = 0;

	}
	return fail;
}

int skill_can_forge(struct map_session_data *sd, int nameid, int flag)
{
	int i, j, id, x, y;

	if (nameid <= 0)
		return 0;

	for (i = 0; i < MAX_SKILL_REFINE; i++)
		if (forge_db[i].nameid == nameid)
			break;


	if (i == MAX_SKILL_REFINE)
		return 0;

	j = forge_db[i].req_skill;
	if (j > 0 && sd->status.skill[j-1].lv <= 0)
		return 0;

	if (flag > 0) {
		if (forge_db[i].nameid < 1101 || forge_db[i].itemlv != flag)
			return 0;

	}
	else {
		if (forge_db[i].nameid > 1100)
			return 0;

	}
	for (j = 0; j < 5; j++) {
		if ((id = forge_db[i].mat_id[j]) == 0)
			break;

		for (y = 0, x = 0; y < MAX_INVENTORY; y++)
			if (sd->status.inventory[y].nameid == id)
				x += sd->status.inventory[y].amount;


		if (x < forge_db[i].mat_amount[j])
			return 0;

	}
	return i + 1;
}

int delete_forge_material(unsigned int fd, struct map_session_data *sd, int name_id)
{
	int i;
	int i1 = 0, i2 = 0;

	i1 = forge_db_index(name_id);
	for (i = 0; i < 5; i++) {
		if (forge_db[i1].mat_id[i] != 0) {
			i2 = search_item2(sd, forge_db[i1].mat_id[i]);
			//if (index)
			mmo_map_lose_item(fd, 1, i2, forge_db[i1].mat_amount[i]);

		}
	}
	return 0;
}

void skill_do_forge(unsigned int fd, struct map_session_data *sd, int name_id, int flag, int ele)
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
	WFIFOW(sd->fd, 0) = 0x196;
	WFIFOW(sd->fd, 2) = icon;
	WFIFOL(sd->fd, 4) = sd->account_id;
	WFIFOB(sd->fd, 8) = flag;
	WFIFOSET(sd->fd, packet_len_table[0x196]);
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
		floorskill[i].trap_used = 0;
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

int next_floorskill_index(void)
{
	int i;

	for (i = 0; i < MAX_FLOORSKILL; i++)
		if (floorskill[i].used == 0)
			return i;


	return -1;
}

int search_floorskill_index(short m, short x, short y)
{
	int i;
	short j, bx, by;
	struct block_list *bl;
	struct floorskill_data *fskill;

	if (last_floor_item_id < 50) {
		for (i = 0; i <= last_floor_item_id; i++) {
			if (floorskill[i].m == m) {
				for (j = 0; j < MAX_BLOCKS_AREA_FLOORSKILL; j++) {
					if (floorskill[i].skill[j].x == -1 && floorskill[i].skill[j].y == -1)
						break;

					else if (floorskill[i].skill[j].x == x && floorskill[i].skill[j].y == y)
						return i;

				}
			}
		}
	}
	else {
		bx = x / BLOCK_SIZE;
		by = y / BLOCK_SIZE;
		if (bx < 0 || bx >= map_data[m].bxs || by < 0 || by >= map_data[m].bys)
			return -1;

		bl = map_data[m].block[bx + by * map_data[m].bxs].next;
		while (bl != NULL) {
			if (bl->type == BL_SKILL) {
				fskill = (struct floorskill_data*)bl;
				if (fskill->x == x && fskill->y == y)
					return fskill->index;

			}
			bl = bl->next;
		}
	}
	return -1;
}

int search_floorskill_index2(long account_id, int skill, int start_index)
{
	int i;

	if (start_index >= 0 && start_index < MAX_FLOORSKILL)
		for (i = start_index; i <= last_floor_item_id; i++)
			if (floorskill[i].owner_id == account_id && floorskill[i].skill_num == skill)
				return i;



	return -1;
}

void make_floorskill(unsigned int fd, short x, short y, int skill_lvl, int skill_type, int subtype, int skill_num, int index)
{
	int i, id;
	int duration = 0;
	unsigned char buf[16];
	struct floorskill_data *fskill;
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	if (x < 0 || x >= map_data[sd->mapno].xs || y < 0 || y >= map_data[sd->mapno].ys)
		return;

	for (i = 0; i < MAX_BLOCKS_AREA_FLOORSKILL; i++)
		if (floorskill[index].skill[i].bl_id == -1)
			break;


	if (i == MAX_BLOCKS_AREA_FLOORSKILL)
		return;

	if ((id = search_free_object_id()) == 0)
		return;

	fskill = (struct floorskill_data *)malloc(sizeof(struct floorskill_data));
	object[id] = &fskill->block;
	fskill->id = id;
	fskill->owner_id = sd->account_id;
	fskill->x = x;
	fskill->y = y;
	fskill->skill_type = skill_type;
	fskill->index = index;
	fskill->block.prev = NULL;
	fskill->block.next = NULL;
	fskill->block.type = BL_SKILL;
	fskill->block.subtype = subtype;
	add_block(&fskill->block, sd->mapno, fskill->x, fskill->y);

	WBUFW(buf, 0) = 0x11f;
	WBUFL(buf, 2) = fskill->id;
	WBUFL(buf, 6) = fskill->owner_id;
	WBUFW(buf, 10) = fskill->x;
	WBUFW(buf, 12) = fskill->y;
	WBUFB(buf, 14) = fskill->skill_type;
	WBUFB(buf, 15) = 1;
	mmo_map_sendarea(fd, buf, packet_len_table[0x11f], 0);

	floorskill[index].skill[i].bl_id = id;
	floorskill[index].skill[i].x = x;
	floorskill[index].skill[i].y = y;

	if (floorskill[index].used == 0) {
		floorskill[index].trap_used = 0;
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
			duration = 15000;
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
			case 125: /* TALKIEBOX */
				duration = 60000;
				break;
			}
			break;
		case FS_WARP:
			floorskill[index].count = 6 + skill_lvl;
			duration = 60000;
			break;
		case FS_ICEWALL:
			map_setcell(floorskill[index].m, floorskill[index].skill[i].x, floorskill[index].skill[i].y, 5);
			WBUFW(buf, 0) = 0x192;
			WBUFW(buf, 2) = floorskill[index].skill[i].x;
			WBUFW(buf, 4) = floorskill[index].skill[i].y;
			WBUFW(buf, 6) = 5;
			memcpy(WBUFP(buf, 8), sd->mapname, 16);
			mmo_map_sendall(fd, buf, packet_len_table[0x192], 0);
			floorskill[index].count = 0;
			duration = 300000;
			break;
		case FS_SWALL:
			floorskill[index].count = 1 + skill_lvl;
			duration = 45000;
			break;
		case FS_FWALL:
			floorskill[index].count = 2 + skill_lvl;
			duration = 4000 * skill_lvl;
			break;
		case FS_VENOM:
			floorskill[index].count = 0;
			duration = 5000 * skill_lvl;
			break;
		case FS_PNEUMA:
			floorskill[index].count = 0;
			duration = 10000;
			break;
		}
		floorskill[index].timer = add_timer(gettick() + duration, remove_floorskill, fd, index);
		if (last_floor_item_id < index)
			last_floor_item_id = index;

	}
}

void remove_floorskill(int tid, unsigned int tick, int fd, int index)
{
	int i;
	unsigned char buf[256];
	struct delay_item_drop *ditem;

	for (i = 0; i < MAX_BLOCKS_AREA_FLOORSKILL; i++) {
		if (floorskill[index].skill[i].x == -1 && floorskill[index].skill[i].y == -1)
			break;

		WBUFW(buf, 0) = 0x120;
		WBUFL(buf, 2) = floorskill[index].skill[i].bl_id;
		mmo_map_sendarea_mxy(floorskill[index].m, floorskill[index].skill[i].x, floorskill[index].skill[i].y, buf, packet_len_table[0x120]);
		delete_object(floorskill[index].skill[i].bl_id);

		if (!floorskill[index].trap_used) {
			switch (floorskill[index].skill_num) {
			case 115: /* SKIDTRAP */
			case 116: /* LANDMINE */
			case 117: /* ANKLE SNARE */
			case 118: /* SHOCKWAVE */
			case 119: /* SANDMAN */
			case 120: /* FLASHER */
			case 121: /* FREEZING */
			case 122: /* BLASTMINE */
			case 123: /* CLAYMORE */
				ditem = malloc(sizeof(*ditem));
				ditem->nameid = 1065;
				ditem->m = floorskill[index].m;
				ditem->x = floorskill[index].skill[i].x;
				ditem->y = floorskill[index].skill[i].y;
				add_timer(gettick() + 300, mmo_map_delay_item_drop, (int)ditem, 0);
				break;
			}
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

	while(last_floor_item_id >= 0 && floorskill[last_floor_item_id].used == 0)
		last_floor_item_id--;

}

void skill_restart_cast_delay(int tid, unsigned int tick, int fd, int data)
{
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	if (sd->skilltimer_delay > 0) {
		delete_timer(sd->skilltimer_delay, skill_restart_cast_delay);
		sd->skilltimer_delay = 0;
	}
}

void skill_callspirits(unsigned int fd, int spheres_num)
{
	unsigned char buf[256];
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	WBUFW(buf, 0) = 0x1d0;
	WBUFL(buf, 2) = sd->account_id;
	WBUFW(buf, 6) = spheres_num;
	mmo_map_sendarea(fd, buf, packet_len_table[0x1d0], 0);
}

void skill_combodelay(unsigned int fd, int delay)
{
	unsigned char buf[256];
	struct map_session_data *sd;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	WBUFW(buf, 0) = 0x1d2;
	WBUFL(buf, 2) = sd->account_id;
	WBUFL(buf, 6) = delay;
	mmo_map_sendarea(fd, buf, packet_len_table[0x1d2], 0);
}

void skill_spiritcadence(int tid, unsigned int tick, int fd, int data)
{
	struct map_session_data *sd;
	int temp_hp, temp_sp, sp, hp;

	if (!session[fd] || !(sd = session[fd]->session_data))
		return;

	if (sd->status.hp < sd->status.max_hp) {
		hp = sd->status.skill[260-1].lv * (sd->status.max_hp/500 + 4);
		temp_hp = hp + sd->status.hp;
		if (temp_hp > sd->status.max_hp) {
			hp = (sd->status.max_hp - sd->status.hp);
			sd->status.hp = sd->status.max_hp;
		}
		else
			sd->status.hp = sd->status.hp + hp;

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
		}
		else
			sd->status.sp = sd->status.sp + sp;

		WFIFOW(fd, 0) = 0x13d;
		WFIFOW(fd, 2) = 7;
		WFIFOL(fd, 4) = sp;
		WFIFOSET(fd, packet_len_table[0x13d]);
	}
	mmo_map_calc_status(fd, 0);
	if (sd->weight >= sd->max_weight / 2)
		sd->skill_timeamount[260-1][0] = add_timer(gettick() + 20000, skill_spiritcadence, fd, 0);

	else
		sd->skill_timeamount[260-1][0] = add_timer(gettick() + 10000, skill_spiritcadence, fd, 0);

}
