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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "mmo.h"
#include "skill_db.h"
#include "map_core.h"
#include "script.h"
#include "item.h"

struct skill_tree_db {
	unsigned int id;
	struct {
		unsigned short id;
		unsigned short lv;
	} need[5];
} skill_tree_db[MAX_CLASSES][60];

void skilldb_init(void)
{
	printf("Loading Skills Data...        ");
	skilldb_read_db();
	skilldb_read_tree();
	printf("Done\n");
}

void skilldb_read_db(void) 
{
	int j;
	char line[1024];
	FILE *fp;
	
	memset(skill_db, 0, sizeof(skill_db));
	fp = fopen("data/databases/skill_db.txt", "r"); // open file.
	if(fp == NULL) {
		printf("can't read data/databases/skill_db.txt");
		sleep(2);
		exit(0);
	}
	while(fgets(line, 1020, fp)) {
		int split[9];

		if(line[0] == '/' && line[1] == '/') continue; // If line is a commented line, go to next line.
		
		j=0;
		// skill_id,attack_range,minimum_sp_cost,num_of_hits,type_inf,type_num,type_pl,type_nk,lv_max
		if (sscanf(line,"%d,%d,%d,%d,%d,%d,%d,%d,%d",
			&split[j],&split[j+1],&split[j+2],
			&split[j+3],&split[j+4],&split[j+5],
			&split[j+6],&split[j+7],&split[j+8]) != 9) continue;

		skill_db[split[0]].id = split[0];
		skill_db[split[0]].range = split[1];
		skill_db[split[0]].sp = split[2];
		skill_db[split[0]].type_hit = split[3];
		skill_db[split[0]].type_inf = split[4];
		skill_db[split[0]].type_num = split[5];
		skill_db[split[0]].type_pl = split[6];
		skill_db[split[0]].type_nk = split[7];
		skill_db[split[0]].type_lv = split[8];
	}
	fclose(fp);
}

void skilldb_read_tree(void)
{
	int j, i, k;
	char line[1024], *p;
	FILE *fp;
	
	memset(skill_tree_db,0,sizeof(skill_tree_db));
	fp = fopen("data/databases/skill_tree_db.txt", "r");
	if(fp == NULL){
		printf("can't read data/databases/skill_tree_db.txt");
		sleep(1);
		exit(0);
	}
	while(fgets(line, 1020,fp)) {
		char *split[50];
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<12 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		
		i = atoi(split[0]);
		for(j = 0; skill_tree_db[i][j].id; j++);
		skill_tree_db[i][j].id = atoi(split[1]);;
		for(k=0;k<5;k++) {
			skill_tree_db[i][j].need[k].id = atoi(split[k*2+2]);
			skill_tree_db[i][j].need[k].lv = atoi(split[k*2+3]);
		}
	}
	fclose(fp);
}

void mmo_map_send_skills(int fd, int do_check)
{
	int i;
	int c=0;
	struct map_session_data *sd = NULL;

	if (session[fd] == NULL || (sd = session[fd]->session_data) == NULL) {
		return;
	}
	if(do_check) skilldb_job_check(sd);

	WFIFOW(fd, 0) = 0x10f;
	for(i = 1; i < MAX_SKILL; i++) {
		if(sd->status.skill[i-1].id == 0) {
			if(sd->card_skill[i-1] == 1) {
				WFIFOW(fd, 4 + c * 37) = i;
				WFIFOW(fd, 6 + c * 37) = skill_db[i].type_inf;
				WFIFOW(fd, 8 + c * 37) = 0;
				WFIFOW(fd, 10 + c * 37) = 1; //Feature needed: Skill levels in item_db.txt
				WFIFOW(fd, 12 + c * 37) = skill_db[i].sp;
				WFIFOW(fd, 14 + c * 37) = skill_db[i].range;
				memcpy(WFIFOP(fd, 16 + c * 37), "", 24);
				WFIFOB(fd, 40 + c * 37) = 0;
				c++;
			}
			continue;
		}
		WFIFOW(fd, 4 + c * 37) = sd->status.skill[i-1].id;
		WFIFOW(fd, 6 + c * 37) = skill_db[i].type_inf;
		WFIFOW(fd, 8 + c * 37) = 0;
		WFIFOW(fd, 10 + c * 37) = sd->status.skill[i-1].lv;
		WFIFOW(fd, 12 + c * 37) = skill_db[i].sp;
		WFIFOW(fd, 14 + c * 37) = skill_db[i].range;
		memcpy(WFIFOP(fd, 16 + c * 37), "", 24);
		WFIFOB(fd, 40 + c * 37) = (skill_db[i].type_lv - sd->status.skill[i-1].lv);
		c++;
	}
	WFIFOW(fd, 2) = 4 + c * 37;
	WFIFOSET(fd, 4 + c * 37);
}

void skilldb_job_check(struct map_session_data *sd)
{
	struct mmo_charstatus *p;

	int i,c,k;
	int id;

	p=&sd->status;
	c=p->class;

	
	for (i = 1; i < MAX_SKILL; i++) {
		/* -- Todo --
		 * Add Bonus 1-1 skills support
		 */
		p->skill[i - 1].id = 0; // Restart Skill
	}

	for(i = 0; (id = skill_tree_db[c][i].id) > 0; i++) {
		int answer[5];
		for(k = 0; k < 5; k++) {
			if (skill_tree_db[c][i].need[k].id != 0) {
				if (skill_tree_db[c][i].need[k].lv <= p->skill[skill_tree_db[c][i].need[k].id - 1].lv) {
					answer[k] = 1;
				} else {
					answer[k] = 0;
				}
			} else answer[k] = 1;
		}
		if ((answer[0] && answer[1] && answer[2] && answer[3] && answer[4]) == 1) {
			p->skill[id - 1].id = id;
		}
	}
}

unsigned long search_placeskill(int skill_id)
{
	switch(skill_id) {
		case 12: return 0x7e;
		case 18: return 0x7f;
		case 25: return 0x85;
		case 27: return 0x80;
		case 69: return 0x82;
		case 70: return 0x83;
		case 79: return 0x84;
		case 81: return 0x86;
		case 83: return 0x86;
		case 85: return 0x86;
		case 87: return 0x8d;
		case 89: return 0x86;
		case 92: return 0x8e;
		case 115: return 0x90;
		case 116: return 0x93;
		case 117: return 0x91;
		case 118: return 0x94;
		case 119: return 0x95;
		case 120: return 0x96;
		case 121: return 0x97;
		case 122: return 0x8f;
		case 123: return 0x98;
		case 125: return 0x99;
		case 140: return 0x92;
		case 220: return 0xb0;
		case 253: return 0x86;
		case 254: return 0x86;
		case 271: return 0x86;
		case 285: return 0x9a;
		case 286: return 0x9b;
		case 287: return 0x9c;
		case 288: return 0x9d;
		case 306: return 0x9e;
		case 307: return 0x9f;
		case 308: return 0xa0;
		case 309: return 0xa1;
		case 310: return 0xa2;
		case 311: return 0xa3;
		case 312: return 0xa4;
		case 313: return 0xa5;
		case 317: return 0xa6;
		case 319: return 0xa7;
		case 320: return 0xa8;
		case 321: return 0xa9;
		case 322: return 0xaa;
		case 325: return 0xab;
		case 327: return 0xac;
		case 328: return 0xad;
		case 329: return 0xae;
		case 330: return 0xaf;
		default: return 0; break;
	}
}
