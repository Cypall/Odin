
//Monster Skills fully implemeted by 911sos
//URL : http://2byte.net
//E-Mail : demadosa@hanafos.com
//Forum : http://forum.projectyare.de

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "npc.h"
#include "skill.h"
#include "skill_db.h"
#include "card.h"

extern int users_global;

int myremove_monster(struct map_session_data *sd, int fd, int m, int n)
{

	if(map_data[m].npc[n]->u.mons.target_fd == sd->fd) {
		delete_timer(sd->attacktimer,mmo_map_attack_continue);
		sd->attacktimer=-1;
	}

	WFIFOW(fd,0) = 0x80;
	WFIFOL(fd,2) = map_data[m].npc[n]->id;
	WFIFOB(fd,6) = 1;
	mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );

	del_block(&map_data[m].npc[n]->block);

	if(map_data[m].npc[n]->u.mons.attacktimer>0) {
		delete_timer(map_data[m].npc[n]->u.mons.attacktimer,mmo_mons_attack_continue);
		map_data[m].npc[n]->u.mons.attacktimer=-1;
	}
	if(map_data[m].npc[n]->u.mons.walktimer>0) {
		delete_timer(map_data[m].npc[n]->u.mons.walktimer,mons_walk);
		map_data[m].npc[n]->u.mons.attacktimer=-1;
	}

	map_data[m].npc[n]->u.mons.hp=0;
	map_data[m].npc[n]->u.mons.target_fd = 0;
	map_data[m].npc[n]->u.mons.attacktimer=0;


	set_monster_no_point(m,n);

	return 0;

}

void init_monster_skills()
{
	char line[1024];
	int class, skilla, op1, op2, skillb, op3, op4;
	int skillc, op5, op6, skilld, op7, op8, skille, op9, op10, skillf, op11, op12;
	FILE *fp = fopen("data/databases/monster_skill_db.txt", "r");

	printf("Loading Monster Skill Data... ");
	if (fp) {
		while (fgets(line, 1023,  fp)) {
			if(sscanf(line,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				  &class,&skilla,&op1,&op2,&skillb,&op3,&op4,&skillc,&op5,&op6,&skilld,
				  &op7,&op8,&skille,&op9,&op10,&skillf,&op11,&op12)!=19)
				continue;

			if (class >= 4000)
				continue;

			if ((line[0] == '/') && (line[1] == '/'))
				continue;

			mons_skill[class].nameid = class;
			mons_skill[class].skill[0] = skilla;
			mons_skill[class].option[0] = op1;
			mons_skill[class].option[1] = op2;
			mons_skill[class].skill[1] = skillb;
			mons_skill[class].option[2] = op3;
			mons_skill[class].option[3] = op4;
			mons_skill[class].skill[2] = skillc;
			mons_skill[class].option[4] = op5;
			mons_skill[class].option[5] = op6;
			mons_skill[class].skill[3] = skilld;
			mons_skill[class].option[6] = op7;
			mons_skill[class].option[7] = op8;
			mons_skill[class].skill[4] = skille;
			mons_skill[class].option[8] = op9;
			mons_skill[class].option[9] = op10;
			mons_skill[class].skill[5] = skillf;
			mons_skill[class].option[10] = op11;
			mons_skill[class].option[11] = op12;
		}
		fclose(fp);
		printf("Done\n");
	}
	else {
		printf("**Error: Cannot load monster_skill_db.txt!**\n");
		sleep(2);
		exit(0);
	}	
}


void monster_heal(int fd, int m, int n)
{
	int heal_point = mons_data[map_data[m].npc[n]->class].max_hp / 2;

	WFIFOW (fd, 0) = 0x11a;   //no damage skill effect
	WFIFOW (fd, 2) = 28; //heal skill
	WFIFOW (fd, 4) = heal_point;
	WFIFOL (fd, 6) = map_data[m].npc[n]->id; //target
	WFIFOL (fd, 10) = map_data[m].npc[n]->id; //caster
	WFIFOB (fd, 14) = 1;
	mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a], 0);

	map_data[m].npc[n]->u.mons.hp += heal_point;

}

void monster_use_effects(struct map_session_data *sd, int fd, int myoption, int m, int n)
{

	int myoption2=0;
	int tick=gettick();
	int mytimer=5000 + (rand()%15 * 1000);//Time Length of Effected

	sd->status.effect = myoption;

	if(myoption < 5) {

		WFIFOW(fd,0)=0x119;
		WFIFOL(fd,2)=sd->account_id;
		WFIFOW(fd,6)=myoption;
		WFIFOW(fd,8)=sd->status.option_y;
		WFIFOW(fd,10)=sd->status.option_z;
		WFIFOB(fd,12) = 0;
		
		mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
		
		sd->status.option_x=myoption;
		sd->status.option_y=sd->status.option_y;
		sd->status.option_z=sd->status.option_z;

		switch(myoption) {
		case 1://Stone
		case 2://Freeze
		case 3://Stun
			if(sd->skill_timeamount[15-1][0] > -1) {
				delete_timer(sd->skill_timeamount[15-1][0], skill_reset_stats);
				sd->skill_timeamount[15-1][0] = add_timer (tick + mytimer , skill_reset_stats, fd, 15);
				sd->speed=-1;
			} else {
				sd->skill_timeamount[15-1][0] = add_timer (tick + mytimer , skill_reset_stats, fd, 15);
				sd->speed=-1;
			}
			break;
		case 4://Sleep
			WFIFOW(fd,0) = 0x8a;
			WFIFOL(fd,2) = sd->account_id;
			WFIFOB(fd,26) = 2;
			mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x8a], 0 );
			sd->sitting=2;

			if(sd->skill_timeamount[15-1][0] > -1) {
				delete_timer(sd->skill_timeamount[15-1][0], skill_reset_stats);
				sd->skill_timeamount[15-1][0] = add_timer (tick + mytimer , skill_reset_stats, fd, 15);
				sd->speed=-1;
			} else {
				sd->skill_timeamount[15-1][0] = add_timer (tick + mytimer , skill_reset_stats, fd, 15);
				sd->speed=-1;
			}
			break;
		}


	} else {

		switch(myoption) {
		case 5://silent
			myoption2 = 4;
			break;
		case 6://poison
			myoption2 = 1;
			break;
		case 7://confuse
			break;
		case 8://curse
			myoption2 = 2;
			break;
		case 9://dark 
			myoption2 = 16;
			break;
		}

		WFIFOW(fd,0)=0x119;
		WFIFOL(fd,2)=sd->account_id;
		WFIFOW(fd,6)=sd->status.option_x;
		WFIFOW(fd,8)=myoption2;
		WFIFOW(fd,10)=sd->status.option_z;
		WFIFOB(fd,12) = 0;
		
		mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
		
		sd->status.option_x=sd->status.option_x;
		sd->status.option_y=myoption2;
		sd->status.option_z=sd->status.option_z;

		if(sd->skill_timeamount[53-1][0] > -1) {
			delete_timer(sd->skill_timeamount[53-1][0], skill_reset_stats);
			sd->skill_timeamount[53-1][0] = add_timer (tick + mytimer , skill_reset_stats, fd, 53);
		} else {
			sd->skill_timeamount[53-1][0] = add_timer (tick + mytimer , skill_reset_stats, fd, 53);
		}


	}

}

void monster_use_skill1(struct map_session_data *sd, int fd, int skill_num, int m, int n)
{
	struct map_session_data *target_sd;
	int damage=rand()%(mons_data[map_data[m].npc[n]->class].atk1 + mons_data[map_data[m].npc[n]->class].atk2);
	int skill_lvl=rand()%10;
	int tick=gettick();
	int i=0;
	int target_fd = 0;


	if(skill_lvl == 0) skill_lvl = 1;
	if(damage < mons_data[map_data[m].npc[n]->class].lv) damage = mons_data[map_data[m].npc[n]->class].lv * skill_lvl;

	switch(sd->status.effect) {//Damage Add
	case 1://stone
	case 2://Freeze
	case 3://Stun
	case 4://Sleep
		damage *= 2;
		break;
	case 6://Poison
	case 8://Curse
		damage *= 1.3;
		break;
	}

	//damage = card_damage_reduce(sd,damage,m,n);

   	/*KYRIE*/
  	if(sd->skill_timeamount[73-1][0] > 0 && sd->skill_timeamount[73-1][1] > 0){
		sd->skill_timeamount[73-1][1]--;
		damage = 0;	
		if(sd->skill_timeamount[73-1][1] <= 0) skill_reset_stats(0,0,fd,73);
   	}

	switch(skill_db[skill_num].type_nk) {
	case 0:
		//skill has been casted
		WFIFOW (fd, 0) = 0x13e;
		WFIFOL (fd, 2) = map_data[m].npc[n]->id;
		WFIFOL (fd, 6) = sd->account_id;
		WFIFOW (fd, 10) = 0;
		WFIFOW (fd, 12) = 0;
		WFIFOW (fd, 14) = skill_num; //skill_num
		WFIFOL (fd, 16) = skill_db[skill_num].type_pl;  //13 -> skill_num
		WFIFOL (fd, 20) = 100;    //skill
		WFIFOSET (fd, 24);
		// calc damage

		//攻撃系スキルエフェクト
		WFIFOW (fd, 0) = 0x114;
		WFIFOW (fd, 2) = skill_num; //skill_num
		WFIFOL (fd, 4) = map_data[m].npc[n]->id;
		WFIFOL (fd, 8) = sd->account_id;
		WFIFOL (fd, 12) = tick;
		WFIFOL (fd, 16) = sd->speed;  //src_speed
		WFIFOL (fd, 20) = 250;        //dst_speed
		WFIFOW (fd, 24) = damage;
		WFIFOW (fd, 26) = skill_lvl;
		if ((skill_num == 13) || (skill_num == 14) || (skill_num ==18) || (skill_num == 19) || (skill_num == 20))	// Spell Fix (Mage spells) by Danzig
			WFIFOW(fd,28) = skill_db[skill_num].type_num*skill_lvl; //13->skill_num, 3->skill_lvl// Multiple hit spells
		else
			WFIFOW(fd,28) = skill_db[skill_num].type_num;	 // Single hit spells
		WFIFOB (fd, 30) = skill_db[skill_num].type_hit;   //6：単発 8：連発

		mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x114], 0);

		skill_do_damage (fd, damage, sd->account_id, tick,0);

		sd->status.hp -= damage;

		WFIFOW(fd,0) = 0xb0;
		WFIFOW(fd,2) = 0x05;
		WFIFOL(fd,4) = sd->status.hp;
		WFIFOSET(fd,8);			

		if(sd->status.hp <= 0) {//Send Death Packet
			WFIFOW(fd,0) = 0x80;
			WFIFOL(fd,2) = sd->account_id;
			WFIFOB(fd,6) = 1;
			mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );
		}

		switch(skill_num) {
		case 15://Frost Diver
			add_timer (tick + 10000 , skill_reset_stats, fd, 15);

			WFIFOW(fd,0)=0x119;
			WFIFOL(fd,2)=sd->account_id;
			WFIFOW(fd,6)=2;
			WFIFOW(fd,8)=sd->status.option_y;
			WFIFOW(fd,10)=sd->status.option_z;
			WFIFOB(fd,12) = 0;

			mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );

			sd->status.option_x=2;
			sd->status.option_y=sd->status.option_y;
			sd->status.option_z=sd->status.option_z;
			sd->speed = -1; //u cant move if ur frozen
			sd->status.effect = 2;
			break;
		}
 
		break;

	case 1:

		switch(skill_num){

		case 6: //PROVOKE
			skill_do_damage(fd,0,sd->account_id, tick,0);//No damage

			if(sd->skill_timeamount[6-1][0] > -1) {
				delete_timer(sd->skill_timeamount[6-1][0], skill_reset_stats);
				sd->skill_timeamount[6-1][0]=add_timer (tick + 10000 , skill_reset_stats, fd, skill_num);
			} else {
				sd->skill_timeamount[6-1][0]=add_timer (tick + 10000 , skill_reset_stats, fd, skill_num);
			}
			skill_icon_effect(sd,0,1);
			break;

		case 8://Endure
			map_data[m].npc[n]->u.mons.def1 += mons_data[map_data[m].npc[n]->class].lv;
			map_data[m].npc[n]->u.mons.def2 += (skill_lvl * 10);

			add_timer (tick + 10000, skill_reset_monster_stats, m, n);
			map_data[m].npc[n]->skilldata.skill_num=skill_num;
			map_data[m].npc[n]->skilldata.fd = fd;
			break;

		case 10://Sight

			WFIFOW(fd,0)=0x119;
			WFIFOL(fd,2)=map_data[m].npc[n]->id;
			WFIFOW(fd,6)=0;
			WFIFOW(fd,8)=0;
			WFIFOW(fd,10)=1;
			WFIFOB(fd,12) = 0;
		
			mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );
		
			//map_data[m].npc[n]->option=0|0|1;

			add_timer (tick + 3000, skill_reset_monster_stats, m, n);
			map_data[m].npc[n]->skilldata.skill_num=skill_num;
			map_data[m].npc[n]->skilldata.fd = fd;

			for(i=5;i<=users_global+5;i++){//number of people loop
				if(!session[i] || !(target_sd=session[i]->session_data)) continue;
				target_sd=session[i]->session_data;
				target_fd=i;
				if((target_sd->mapno == m) && (target_sd->x >= map_data[m].npc[n]->x -10) && (target_sd->x <= map_data[m].npc[n]->x +10) && (target_sd->y >= map_data[m].npc[n]->y -10) && (target_sd->y <= map_data[m].npc[n]->y +10)) {
					if(target_sd->hidden == 1) {

						skill_reset_stats(0,0,target_fd,51);
						skill_reset_stats(0,0,target_fd,135);
						target_sd->hidden=0;
					}
				}
			}

			break;

		case 26://Teleport - Couldn't be used
			break;

		case 29://Increase AGI
			map_data[m].npc[n]->u.mons.speed -= 100;
			if(map_data[m].npc[n]->u.mons.speed < 30) map_data[m].npc[n]->u.mons.speed = 50;

			add_timer (tick + 10000, skill_reset_monster_stats, m, n);
			map_data[m].npc[n]->skilldata.skill_num=skill_num;
			map_data[m].npc[n]->skilldata.fd = fd;

			WFIFOW (fd, 0) = 0x11a;   //no damage skill effect
			WFIFOW (fd, 2) = skill_num;
			WFIFOW (fd, 4) = 0; //Heal_point
			WFIFOL (fd, 6) = map_data[m].npc[n]->id;//target
			WFIFOL (fd, 10) = map_data[m].npc[n]->id;//caster
			WFIFOB (fd, 14) = 1;

			mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a], 0);
			break;

		case 30://Decrease AGI
			if(sd->skill_timeamount[30-1][0] > -1) {
				delete_timer(sd->skill_timeamount[30-1][0], skill_reset_stats);
				sd->skill_timeamount[30-1][0]=add_timer (tick + 10000 , skill_reset_stats, fd, skill_num);
				sd->speed += 100;
			} else {
				sd->skill_timeamount[30-1][0]=add_timer (tick + 10000 , skill_reset_stats, fd, skill_num);
				sd->speed += 100;
			}
			skill_icon_effect(sd,13,1);
			break;

		case 60://Two-Hand Quicken
		case 111://Adrenalin Rush
			delete_timer(map_data[m].npc[n]->u.mons.attacktimer, mmo_mons_attack_continue);
			map_data[m].npc[n]->u.mons.attacktimer = add_timer(gettick() + map_data[m].npc[n]->u.mons.attackdelay * 0.5 , mmo_mons_attack_continue, n, m);
			break;

		case 61://Auto Counter
			sd->status.effect=61;
			break;

		case 72://Recovery
			WFIFOW(fd,0)=0x119;
    			WFIFOL(fd,2)=map_data[m].npc[n]->id;
    			WFIFOW(fd,6)=0;
			WFIFOW(fd,8)=0;
    			WFIFOW(fd,10)=0;
    			WFIFOB(fd,12) = 0;

    			mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x0119], 0 );

			map_data[m].npc[n]->skilldata.effect=0;
    			//map_data[m].npc[n]->option=0|0|0;
			break;

		case 76://Lex Divina
			monster_use_effects(sd,fd,5,m,n);
			break;

		case 78://Lex Aeterna
			sd->status.effect = 78;
			break;

		case 52://Envenom
		case 140://Venom Dust
			monster_use_effects(sd,fd,6,m,n);
			break;

		case 117://Angkle Snare
			if(sd->skill_timeamount[117-1][0] > -1) {
				delete_timer(sd->skill_timeamount[117-1][0], skill_reset_stats);
				sd->skill_timeamount[117-1][0]=add_timer (tick + 10000 , skill_reset_stats, fd, skill_num);
				sd->speed = -1;
			} else {
				sd->skill_timeamount[117-1][0]=add_timer (tick + 10000 , skill_reset_stats, fd, skill_num);
				sd->speed = -1;
			}
			break;

		case 118://Shockwave Trap
			sd->status.sp -= (skill_lvl * 25);
			if(sd->status.sp < 0) sd->status.sp =0;

			WFIFOW(fd,0) = 0xb0;
			WFIFOW(fd,2) = 0x07;
			WFIFOL(fd,4) = sd->status.sp;
			WFIFOSET(fd,8);
			break;

		case 119://Sandman
			monster_use_effects(sd,fd,1,m,n);
			break;

		case 120://Flasher
			monster_use_effects(sd,fd,4,m,n);
			break;

		case 51://Hiding
		case 135://Cloaking

			WFIFOW(fd,0)=0x0119;
			WFIFOL(fd,2)=map_data[m].npc[n]->id;
			WFIFOW(fd,6)=0;
			WFIFOW(fd,8)=0;
			WFIFOW(fd,10)=2;

			mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x0119], 0);
			
			//map_data[m].npc[n]->option = 0|0|2;

			add_timer (tick + 10000, skill_reset_monster_stats, m, n);
			map_data[m].npc[n]->skilldata.skill_num=skill_num;
			map_data[m].npc[n]->skilldata.fd = fd;
			break;

		case 149://Sprinkle Sand //Not implemented skills
		case 150://Back Sliding
		case 152://Throw Stone
			break;
		}

		if(skill_num != 29) {
			WFIFOW (fd, 0) = 0x11a;   //no damage skill effect
			WFIFOW (fd, 2) = skill_num;
			WFIFOW (fd, 4) = 0; //Heal_point
			WFIFOL (fd, 6) = sd->account_id;//target
			WFIFOL (fd, 10) = map_data[m].npc[n]->id;//caster
			WFIFOB (fd, 14) = 1;

			mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x11a], 0);
		}

		break;

	case 2:

	        for(i=5;i<=users_global+5;i++){//number of people loop
			if(!session[i] || !(target_sd=session[i]->session_data)) continue;
			target_sd=session[i]->session_data;
			target_fd=i;
			if((target_sd->mapno == m) && (target_sd->x >= map_data[m].npc[n]->x -2) && (target_sd->x <= map_data[m].npc[n]->x +2) && (target_sd->y >= map_data[m].npc[n]->y -2) && (target_sd->y <= map_data[m].npc[n]->y +2)) {
		  
				WFIFOW (target_fd, 0) = 0x115;   //blow up type
				WFIFOW (target_fd, 2) = skill_num;
				WFIFOL (target_fd, 4) = map_data[m].npc[n]->id;
				WFIFOL (target_fd, 8) = target_sd->account_id;
				WFIFOL (target_fd, 12) = tick;
				WFIFOL (target_fd, 16) = sd->speed;//src_speed
				WFIFOL (target_fd, 20) = 250;    //dst_speed
				WFIFOW (target_fd, 24) = target_sd->x ;  //x
				WFIFOW (target_fd, 26) = target_sd->y ;  //y
				WFIFOW (target_fd, 28) = damage; //ダメージ合計
				WFIFOW (target_fd, 30) = skill_lvl; //LEVEL
				WFIFOW (target_fd, 32) = skill_db[skill_num].type_num;   //分割数?
				WFIFOB (target_fd, 34) = skill_db[skill_num].type_hit;   //type=05 ダメージ&弾き飛ばし。param1はダメージ合計、param2はlevel、param3は分割数と予想

				mmo_map_sendarea (target_fd, WFIFOP (target_fd, 0), packet_len_table[0x115],0);

				skill_do_damage (target_fd, damage, target_sd->account_id, tick,0);

				/* remove damage from player's hp */ 
				target_sd->status.hp = target_sd->status.hp - damage;

				WFIFOW(target_fd,0) = 0xb0;
				WFIFOW(target_fd,2) = 0x05;
				WFIFOL(target_fd,4) = target_sd->status.hp;
				WFIFOSET(target_fd,8);		

				if(target_sd->status.hp <= 0) {//Send Death Packet
					WFIFOW(target_fd,0) = 0x80;
					WFIFOL(target_fd,2) = target_sd->account_id;
					WFIFOB(target_fd,6) = 1;
					mmo_map_sendarea( target_fd, WFIFOP(target_fd,0), packet_len_table[0x80], 0 );
				}


			}
		}

		break;
	}

}

//Place Affect Skills
void monster_use_skill2(struct map_session_data *sd, int fd, int skill_num, int m, int n)
{
	int damage=rand()%(mons_data[map_data[m].npc[n]->class].atk1 + mons_data[map_data[m].npc[n]->class].atk2);
	int skill_lvl=rand()%10;
	int tick=gettick();
	int i=0;
	int target_fd=0;
	int skill_x, skill_y, skill_type;
	struct map_session_data *target_sd;

	target_sd = NULL;

	if(skill_lvl == 0) skill_lvl = 1;
	if(damage < mons_data[map_data[m].npc[n]->class].lv) damage = mons_data[map_data[m].npc[n]->class].lv * skill_lvl;

	switch(sd->status.effect) {//Damage Add
	case 1://stone
	case 2://Freeze
	case 3://Stun
	case 4://Sleep
		damage *= 2;
		break;
	case 6://Poison
	case 8://Curse
		damage *= 1.3;
		break;
	}

	//damage = card_damage_reduce(sd,damage,m,n);

   	/*KYRIE*/
  	if(sd->skill_timeamount[73-1][0] > 0 && sd->skill_timeamount[73-1][1] > 0){
		sd->skill_timeamount[73-1][1]--;
		damage = 0;	
		if(sd->skill_timeamount[73-1][1] <= 0) skill_reset_stats(0,0,fd,73);
   	}

	skill_type = search_placeskill (skill_num);

	skill_x = sd->x;
	skill_y = sd->y;

	WFIFOW (fd, 0) = 0x117;
	WFIFOW (fd, 2) = skill_num;
	WFIFOL (fd, 4) = map_data[m].npc[n]->id;
	WFIFOW (fd, 8) = 100;
	WFIFOW (fd, 10) = map_data[m].npc[n]->x;    
	WFIFOW (fd, 12) = map_data[m].npc[n]->y;   
	WFIFOL (fd, 14) = tick;

	mmo_map_sendarea (fd, WFIFOP (fd, 0), packet_len_table[0x117], 0);

	switch(skill_num){
	case 12://Safety Wall

		WFIFOW (fd, 0) = 0x11f;
          	WFIFOL (fd, 2) = sd->account_id;
          	WFIFOL (fd, 6) = map_data[m].npc[n]->id;
         	WFIFOW (fd, 10) = skill_x;
          	WFIFOW (fd, 12) = skill_y;
          	WFIFOB (fd, 14) = skill_type;
          	WFIFOB (fd, 15) = 1;	//fail0 ok1

          	mmo_map_sendarea (fd, WFIFOP (fd, 0),packet_len_table[0x11f], 0);

		map_data[m].npc[n]->skilldata.effect = 12;
		break;

	case 21://Thunder Storm
	case 83://Meteor Storm
	case 85://Lord of Vermillion
	case 86://Water Ball
	case 89://Storm Gust
	case 91://Heaven's Drive
	case 92://Quakmire
	case 110://Hammer Fall
	case 121://Freezing Trap
	case 123://Claymore Trap

		for(i=5;i<=users_global+5;i++){//number of people loop
			if(!session[i] || !(target_sd=session[i]->session_data)) continue;
			target_sd=session[i]->session_data;
			target_fd=i;
			if((target_sd->mapno == m) && (target_sd->x >= map_data[m].npc[n]->x -3) && (target_sd->x <= map_data[m].npc[n]->x +3) && (target_sd->y >= map_data[m].npc[n]->y -3) && (target_sd->y <= map_data[m].npc[n]->y +3)) {

				WFIFOW (target_fd, 0) = 0x11f;
				WFIFOL (target_fd, 2) = target_sd->account_id;//target
				WFIFOL (target_fd, 6) = map_data[m].npc[n]->id;//caster   
				WFIFOW (target_fd, 10) = skill_x + 3;
				WFIFOW (target_fd, 12) = skill_y + 3;
				WFIFOB (target_fd, 14) = skill_type;
				WFIFOB (target_fd, 15) = 1;              //fail0 ok1
				mmo_map_sendarea (target_fd, WFIFOP (target_fd, 0), packet_len_table[0x11f], 0);

				WFIFOW (target_fd, 0) = 0x114;
				WFIFOW (target_fd, 2) = skill_num;
				WFIFOL (target_fd, 4) = map_data[m].npc[n]->id;//caster
				WFIFOL (target_fd, 8) = target_sd->account_id;//target
				WFIFOL (target_fd, 12) = tick;
				WFIFOL (target_fd, 16) = sd->speed;  //src_speed
				WFIFOL (target_fd, 20) = 250;
				WFIFOW (target_fd, 24) = damage; //ダメージ合計
				WFIFOW (target_fd, 26) = skill_lvl; //LEVEL
				if ((skill_num == 13) || (skill_num == 14) || (skill_num == 18) || (skill_num == 19) || (skill_num == 20))	// Spell Fix (Mage spells) by Danzig
					WFIFOW(target_fd,28) = skill_db[skill_num].type_num*skill_lvl;								// Multiple hit spells
				else
					WFIFOW(target_fd,28) = skill_db[skill_num].type_num;										// Single hit spells
				WFIFOB (target_fd, 30) = skill_db[skill_num].type_hit;  //6：単発 8：連発

				mmo_map_sendarea (target_fd, WFIFOP (target_fd, 0), packet_len_table[0x114], 0);

				skill_do_damage (target_fd,damage,target_sd->account_id,tick,0);

				/* remove damage from player's hp */ 
				target_sd->status.hp = target_sd->status.hp - damage;

				WFIFOW(target_fd,0) = 0xb0;
				WFIFOW(target_fd,2) = 0x05;
				WFIFOL(target_fd,4) = target_sd->status.hp;
				WFIFOSET(target_fd,8);		

				if(target_sd->status.hp <= 0) {//Send Death Packet
					WFIFOW(target_fd,0) = 0x80;
					WFIFOL(target_fd,2) = target_sd->account_id;
					WFIFOB(target_fd,6) = 1;
					mmo_map_sendarea( target_fd, WFIFOP(target_fd,0), packet_len_table[0x80], 0 );
				}

				switch(skill_num) {
				case 110://Hammer Fall
					if(rand()%100 < skill_lvl * 5) monster_use_effects(target_sd,target_fd,3,m,n);
					break;
				case 121://Freezing Trap
					if(rand()%100 < skill_lvl * 5) monster_use_effects(target_sd,target_fd,2,m,n);
					break;
				}

			}
		}

		break;
	}


}

void check_monster_skills(struct map_session_data *sd, unsigned int fd, short m, int n)
{
	int skill_num = mons_skill[map_data[m].npc[n]->class].skill[rand() % 5];

	switch (skill_num) {
		/*case 1: // Summon Puppets
		  short x = rand() % 2;
		  if (x == 2)
		  x = -1;

		  short y = rand() % 2;
		  if(y == 2)
		  y = -1;

		  if(map_data[m].npc[n]->u.mons.summon > 3)
		  break;

		  spawn_to_pos(sd, mons_skill[map_data[m].npc[n]->class].option[rand() % 5],NULL, x, y, sd->mapno, fd);
		  map_data[m].npc[n]->u.mons.summon++;
		  break;*/
	case 2: // Heal
		if (map_data[m].npc[n]->u.mons.hp <= 0)
			break;

		if ((rand() % 80) == 10) {
			monster_heal(fd, m, n);
			if (map_data[m].npc[n]->u.mons.hp > mons_data[map_data[m].npc[n]->class].max_hp)
				map_data[m].npc[n]->u.mons.hp = mons_data[map_data[m].npc[n]->class].max_hp;
		}
		break;

	case 3: // Poly Morph
		if ((rand() % 70) == 10) {
			myremove_monster(sd, fd, m, n);
			spawn_to_pos(sd, mons_skill[map_data[m].npc[n]->class].option[rand() % 5], NULL, map_data[m].npc[n]->x, map_data[m].npc[n]->y, sd->mapno, fd);
		}
		break;

	case 26: // Teleport
		if ((rand() % 50) == 10) {
			set_monster_random_point(m, n);
			WFIFOW(fd, 0) = 0x80;
			WFIFOL(fd, 2) = map_data[m].npc[n]->id;
			WFIFOB(fd, 6) = 2;
			mmo_map_sendarea(fd, WFIFOP(fd, 0), packet_len_table[0x80], 0);
			del_block(&map_data[m].npc[n]->block);
			add_block_npc(m, n);
		}
		break;

	case 51: // Hiding
      	case 135: // Cloaking
		if ((rand() % 70) == 10) {
			WFIFOW(fd, 0) = 0x0119;
			WFIFOL(fd, 2) = map_data[m].npc[n]->id;
			WFIFOW(fd, 6) = 0;
			WFIFOW(fd, 8) = 0;
			WFIFOW(fd, 10) = 2;
			WFIFOW(fd, 12) = 2;
			mmo_map_sendarea(fd, WFIFOP(fd, 0), packet_len_table[0x0119], 0);
			map_data[m].npc[n]->u.mons.speed = 0;
			//map_data[m].npc[n]->option = 0|0|2;
			//map_data[m].npc[n]->u.mons.hidden = 1;
			map_data[m].npc[n]->skilldata.skill_num = skill_num;
			map_data[m].npc[n]->skilldata.skill_timer[51-1][0] = add_timer(gettick() * 30000, skill_reset_monster_stats, m, n);
		}
		break;
	}
}

//Main Function - Called from attack_monster.c
void monster_skills(struct map_session_data *sd, int fd, int damage, int m, int n)
{

	int myclass=mons_data[map_data[m].npc[n]->class].monsterid;
	int myrand=rand()%1;
	int mylv=mons_data[map_data[m].npc[n]->class].lv;
	int opt=rand()%5;
	int myswitcher=mons_skill[myclass].skill[opt];
	int i=mylv/10;
	int balance=45 - (5 - mylv);//Decreasing this will make monster cast skills more frequently(Default 50)
	int cnt=0;
	int temp_damage=0;
	int px,py,myx,myy;

	switch(myswitcher) {

	case 0://No Skill
		break;

	case 1://Summon puppets

		px=rand()%2; py=rand()%2;
		if(px==2) px=-1;
		if(py==2) py=-1;

		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) {
				if(map_data[m].npc[n]->u.mons.summon > 3) break;
				if(mons_skill[myclass].option[opt*2+myrand] != 0) spawn_to_pos(sd, mons_skill[myclass].option[opt*2+myrand],NULL,px,py,sd->mapno, fd);
				map_data[m].npc[n]->u.mons.summon++;
			}
		}
		break;

	case 2://Healing Skill
		if(mons_skill[myclass].option[opt*2+1] == 0) {

			if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
				if(rand()%((i+1)*balance) < mylv) monster_heal(fd,m,n);
			}

		} else {
			if((map_data[m].npc[n]->u.mons.hp >= (mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2] * 0.01)) && (map_data[m].npc[n]->u.mons.hp <= (mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2+1] * 0.01)) && (rand()%((i+1)*balance) < mylv)) 
				monster_heal(fd,m,n);
		}
		break;

	case 3://PolyMorph

		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) 
				if(mons_skill[myclass].option[opt*2+1] != 0) {
					px=map_data[m].npc[n]->x; py=map_data[m].npc[n]->y;				 
					myremove_monster(sd,fd,m,n);//Function in npc.c
					spawn_to_pos(sd,mons_skill[myclass].option[opt*2+myrand],NULL,px,py,sd->mapno,fd);
				}
		}
	 
		break;

	case 4://Effects

		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) 
				if(mons_skill[myclass].option[opt*2+1] != 0) {
					monster_use_effects(sd,fd,mons_skill[myclass].option[opt*2+myrand],m,n);
				}
		}

		break;

	case 26://Teleport

		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) 
				if(mons_skill[myclass].option[opt*2+1] != 0) {
					WFIFOW(fd,0) = 0x80;
					WFIFOL(fd,2) = map_data[m].npc[n]->id;
					WFIFOB(fd,6) = 2;
					mmo_map_sendarea( fd, WFIFOP(fd,0),packet_len_table[0x80], 0 );
				
					do {
						myx=rand()%(map_data[m].xs-2)+1;
						myy=rand()%(map_data[m].ys-2)+1;
					} while(map_data[m].gat[myx+myy*map_data[m].xs]==1 || map_data[m].gat[myx+myy*map_data[m].xs]==5);
				
					mmo_map_changemap(sd,sd->mapname,myx,myy,2);

				}
		}
		break;

	case 5://Bash
	case 6://Provoke
	case 7://Magnum Break
	case 8://Endure
	case 10://Sight
	case 11://Napalm Beat
	case 13://Soul Strike
	case 14://Cold Bolt
	case 15://Frost Diver
	case 16://Stone Curse
	case 17://Fire Ball
	case 18://Fire Wall
	case 19://Fire Bolt
	case 20://Lightning Bolt
	case 29://Increase AGI
	case 30://Decrease AGI
	case 42://Mammonite
	case 46://Double Strafe
	case 47://Arrow Shower
	case 51://Hiding
	case 52://Envenom
	case 56://Pierce
	case 57://Brandish Spear
	case 58://Spear Stab
	case 59://Spear Boomerang
	case 60://Two-hand Quicken
	case 61://Auto Counter
	case 72://Recovery
	case 76://Lex Divina
	case 78://Lex Aeterna
	case 84://Jupitel Thunder
	case 90://Earth Spike
	case 111://Adrenalin Rush
	case 114://Maximize Power
	case 115://Skid Trap
	case 117://Angkle Snare
	case 118://ShockwaveTrap
	case 119://Sandman
	case 120://Flasher
	case 135://Cloaking
	case 136://Sonic Blow
	case 137://GrimTooth
	case 138://Enchant Poison
	case 140://Venom Dust
	case 148://Charge Arrow
	case 149://Sprinkle Sand
	case 150://Back Sliding
	case 152://Throw Stone
	case 156://Holy Light

		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) {
				if(mons_skill[myclass].option[opt*2+1] == 0) {
					monster_use_skill1(sd,fd,myswitcher,m,n);
				} else {
					if((map_data[m].npc[n]->u.mons.hp >= mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2] * 0.01) && (map_data[m].npc[n]->u.mons.hp <= mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2+1] * 0.01)) {
						monster_use_skill1(sd,fd,myswitcher,m,n);
					}
				}
			}
		}

		break;

		//-- Place Affect Skills
	case 12://Safety Wall
	case 21://Thunder Storm
	case 83://Meteor Storm
	case 85://Lord of Vermillion
	case 86://Water Ball
	case 89://Storm Gust
	case 91://Heaven's Drive
	case 92://Quakmire
	case 110://Hammer Fall
	case 121://Freezing Trap
	case 123://Claymore Trap
	
		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) {
				if(mons_skill[myclass].option[opt*2+1] == 0) {
					monster_use_skill2(sd,fd,myswitcher,m,n);
				} else {
					if((map_data[m].npc[n]->u.mons.hp >= mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2] * 0.01) && (map_data[m].npc[n]->u.mons.hp <= mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2+1] * 0.01)) {
						monster_use_skill2(sd,fd,myswitcher,m,n);
					}
				}
			}
		}

		break;

	case 95://Suicide

		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) 
				if(mons_skill[myclass].option[opt*2+1] == 0) {

					map_data[m].npc[n]->u.mons.hp = -1;

					delete_timer(map_data[m].npc[n]->u.mons.attacktimer,mmo_mons_attack_continue);
					map_data[m].npc[n]->u.mons.target_fd = 0;
					map_data[m].npc[n]->u.mons.attacktimer = 0;
					map_data[m].npc[n]->u.mons.speed = -1;

					if(map_data[m].npc[n]->u.mons.hp <= 0 ){
	
						if(map_data[m].npc[n]->class >20 ) {
							WFIFOW(fd,0) = 0x80;
							WFIFOL(fd,2) = map_data[m].npc[n]->id;
							WFIFOB(fd,6) = 1;
							mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );
						} else {
							WFIFOW(fd,0) = 0x80;
							WFIFOL(fd,2) = map_data[m].npc[n]->id;
							WFIFOB(fd,6) = 2;
							mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );	
						}
					}
					if (mons_data[map_data[m].npc[n]->class].boss == 1) {
						add_timer(gettick() + 60 * 60000, spawn_delay, m, n);
						timer_data[map_data[m].npc[n]->u.mons.walktimer]->tick = gettick() + 60 + 500;
					}
					else if (mons_data[map_data[m].npc[n]->class].boss == 2) {
						add_timer(gettick() + 45 * 60000, spawn_delay, m, n);
						timer_data[map_data[m].npc[n]->u.mons.walktimer]->tick = gettick() + 45 + 500;
					}
					else {
						add_timer(gettick() + 60000, spawn_delay, m, n);
						timer_data[map_data[m].npc[n]->u.mons.walktimer]->tick = gettick() + 1 + 500;
					}
				}
		}
		break;

	case 98://Self-Destuction
		 
		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) 
				if(mons_skill[myclass].option[opt*2+1] == 0) {

					damage = map_data[m].npc[n]->u.mons.hp * 2;

					/* send damage packet */
					WFIFOW(fd,0) = 0x8a;
					WFIFOL(fd,2) = map_data[m].npc[n]->id;
					WFIFOL(fd,6) = sd->account_id;
					WFIFOL(fd,10) = gettick();
					WFIFOL(fd,14) = mons_data[map_data[m].npc[n]->class].speed;
					WFIFOL(fd,18) = sd->status.aspd;
					WFIFOW(fd,22) = damage;
					WFIFOW(fd,24) = 2;
					WFIFOB(fd,26) = 0x0a; //Critical 10=critical 11=lucky
					WFIFOW(fd,27) = 0;

					mmo_map_sendarea(fd, WFIFOP(fd, 0), packet_len_table[0x8a], 0);
		
					/* remove damage from player's hp */ 
					sd->status.hp = sd->status.hp - damage;

					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0x05;
					WFIFOL(fd,4) = sd->status.hp;
					WFIFOSET(fd,8);

					if(sd->status.hp <= 0) {//Send Death Packet
						WFIFOW(fd,0) = 0x80;
						WFIFOL(fd,2) = sd->account_id;
						WFIFOB(fd,6) = 1;
						mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );
					}

					map_data[m].npc[n]->u.mons.hp = -1;

					delete_timer(map_data[m].npc[n]->u.mons.attacktimer,mmo_mons_attack_continue);
					map_data[m].npc[n]->u.mons.target_fd = 0;
					map_data[m].npc[n]->u.mons.attacktimer = 0;
					map_data[m].npc[n]->u.mons.speed = -1;

					if(map_data[m].npc[n]->u.mons.hp <= 0 ){
	
						if(map_data[m].npc[n]->class >20 ) {
							WFIFOW(fd,0) = 0x80;
							WFIFOL(fd,2) = map_data[m].npc[n]->id;
							WFIFOB(fd,6) = 1;
							mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );
						} else {
							WFIFOW(fd,0) = 0x80;
							WFIFOL(fd,2) = map_data[m].npc[n]->id;
							WFIFOB(fd,6) = 2;
							mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );	
						}
					}

					if (mons_data[map_data[m].npc[n]->class].boss == 1) {
						add_timer(gettick() + 60 * 60000, spawn_delay, m, n);
						timer_data[map_data[m].npc[n]->u.mons.walktimer]->tick = gettick() + 60 + 500;
					}
					else if (mons_data[map_data[m].npc[n]->class].boss == 2) {
						add_timer(gettick() + 45 * 60000, spawn_delay, m, n);
						timer_data[map_data[m].npc[n]->u.mons.walktimer]->tick = gettick() + 45 + 500;
					}
					else {
						add_timer(gettick() + 60000, spawn_delay, m, n);
						timer_data[map_data[m].npc[n]->u.mons.walktimer]->tick = gettick() + 1 + 500;
					}
				}
		}

		break;

	case 48://Double Attack
	case 96://Critical Attack
	case 97://Triple Attack
	case 100://Attack(4)
	case 101://Attack(5)
		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) 
				if(mons_skill[myclass].option[opt*2+1] == 0) {

					switch(myswitcher) {//Counter to Attack
					case 48:
						cnt = 2;
						break;
					case 96:
						cnt = 1;
						break;
					case 97:
						cnt = 3;
						break;
					case 100:
						cnt = 4;
						break;
					case 101:
						cnt = 5;
						break;
					}

					switch(sd->status.effect) {//Damage Add
					case 1://stone
					case 2://Freeze
					case 3://Stun
					case 4://Sleep
						damage *= 2;
						break;
					case 6://Poison
					case 8://Curse
						damage *= 1.3;
						break;
					}

					//damage = card_damage_reduce(sd,damage,m,n);

					temp_damage = damage;//Damage backup

					/*KYRIE*/
					if(sd->skill_timeamount[73-1][0] > 0 && sd->skill_timeamount[73-1][1] > 0){
						sd->skill_timeamount[73-1][1]--;
						damage = 0;	
						if(sd->skill_timeamount[73-1][1] <= 0) skill_reset_stats(0,0,fd,73);
					} else damage=temp_damage;

					/* send damage packet */
					WFIFOW(fd,0) = 0x8a;
					WFIFOL(fd,2) = map_data[m].npc[n]->id;
					WFIFOL(fd,6) = sd->account_id;
					WFIFOL(fd,10) = gettick();
					WFIFOL(fd,14) = mons_data[map_data[m].npc[n]->class].speed;
					WFIFOL(fd,18) = sd->status.aspd;
					WFIFOW(fd,22) = (damage * cnt);
					WFIFOW(fd,24) = cnt;
					if(myswitcher==96) WFIFOB(fd,26) = 0x0a; //Critical 10=critical 11=lucky
					else WFIFOB(fd,26) = 8; //Normal;
					WFIFOW(fd,27) = 0;

					mmo_map_sendarea(fd, WFIFOP(fd, 0), packet_len_table[0x8a], 0);
		
					/* remove damage from player's hp */ 
					sd->status.hp = sd->status.hp - damage;

					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0x05;
					WFIFOL(fd,4) = sd->status.hp;
					WFIFOSET(fd,8);

					if(sd->status.hp <= 0) {//Send Death Packet
						WFIFOW(fd,0) = 0x80;
						WFIFOL(fd,2) = sd->account_id;
						WFIFOB(fd,6) = 1;
						mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );
					}

				}
		}
		break;

	case 99://DrainLife
		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) {
				if(mons_skill[myclass].option[opt*2+1] == 0) {
					map_data[m].npc[n]->u.mons.hp += (sd->status.hp * 0.1);
					if(map_data[m].npc[n]->u.mons.hp > mons_data[map_data[m].npc[n]->class].max_hp)
						map_data[m].npc[n]->u.mons.hp = mons_data[map_data[m].npc[n]->class].max_hp;

					/* remove HP from player's hp */ 
					sd->status.hp -= (sd->status.hp * 0.1);

					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0x05;
					WFIFOL(fd,4) = sd->status.hp;
					WFIFOSET(fd,8);

					if(sd->status.hp <= 0) {//Send Death Packet
						WFIFOW(fd,0) = 0x80;
						WFIFOL(fd,2) = sd->account_id;
						WFIFOB(fd,6) = 1;
						mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );
					}

				} else {
					if((map_data[m].npc[n]->u.mons.hp >= mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2] * 0.01) 
					   && (map_data[m].npc[n]->u.mons.hp <= mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2+1] * 0.01)) {

						map_data[m].npc[n]->u.mons.hp += (sd->status.hp * 0.1);
						if(map_data[m].npc[n]->u.mons.hp > mons_data[map_data[m].npc[n]->class].max_hp)
							map_data[m].npc[n]->u.mons.hp = mons_data[map_data[m].npc[n]->class].max_hp;

						/* remove HP from player's hp */ 
						sd->status.hp -= (sd->status.hp * 0.1);

						WFIFOW(fd,0) = 0xb0;
						WFIFOW(fd,2) = 0x05;
						WFIFOL(fd,4) = sd->status.hp;
						WFIFOSET(fd,8);

						if(sd->status.hp <= 0) {//Send Death Packet
							WFIFOW(fd,0) = 0x80;
							WFIFOL(fd,2) = sd->account_id;
							WFIFOB(fd,6) = 1;
							mmo_map_sendarea( fd, WFIFOP(fd,0), packet_len_table[0x80], 0 );
						}

					}
				}
			}
		}
		break;

	case 102://DrainMana(SP)
		if((mylv > (i*10)) && (mylv <= ((i+1)*10))) {
			if(rand()%((i+1)*balance) < mylv) {
				if(mons_skill[myclass].option[opt*2+1] == 0) {

					/* remove HP from player's hp */ 
					sd->status.sp -= (sd->status.sp * 0.1);

					WFIFOW(fd,0) = 0xb0;
					WFIFOW(fd,2) = 0x07;
					WFIFOL(fd,4) = sd->status.sp;
					WFIFOSET(fd,8);

				} else {
					if((map_data[m].npc[n]->u.mons.hp >= mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2] * 0.01) 
					   && (map_data[m].npc[n]->u.mons.hp <= mons_data[map_data[m].npc[n]->class].max_hp * mons_skill[myclass].option[opt*2+1] * 0.01)) {

						/* remove HP from player's hp */ 
						sd->status.sp -= (sd->status.sp * 0.1);

						WFIFOW(fd,0) = 0xb0;
						WFIFOW(fd,2) = 0x07;
						WFIFOL(fd,4) = sd->status.sp;
						WFIFOSET(fd,8);
					}
				}
			}
		}
		break;

	case 161://Change Element
		break;

	default:
		break;
	}

}

