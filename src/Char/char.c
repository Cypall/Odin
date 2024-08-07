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
 Project:       Project Odin Character Server
 Creation Date: Dicember 6, 2003
 Modified Date: October 21, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#include "core.h"
#include "char.h"

struct mmo_party party_dat[MAX_PARTYS];
struct mmo_charstatus *char_dat;
struct mmo_charstatus *pet_dat;
struct point start_point = { "new_5-1.gat", 53, 111 };

/*
 * Network releate variables
 */

int server_fd[MAX_SERVERS];
int ver_1 = 20, ver_2 = 1;
unsigned int login_fd = 0;
char userid[24];
char passwd[24];
char server_name[16];
char login_ip_str[16];
short login_port = 6900;
char char_ip_str[16];
int char_ip;
char char_xip_str[MAX_BUFFER];
int char_xip;
short char_port = 6121;
int char_port_fd;

/*
 * Timer variables
 */

int check_connect_timer;
int send_users_tologin_timer;
int check_backup_timer;

/*
 * Configuration variables
 */

int maintenance;
int new;
int do_backup;
char email[60];
int start_zeny = 0;
int do_timer_bak = 0;
int char_num = 0;
int char_max = 0;
int pet_num = 0;
int pet_max = 0;
int auth_fifo_pos = 0;

/*
 * Give accounts ids default values
 */

long int char_id_count	= ACCOUNT_ID_INIT;
long int pet_id_count	= PET_ID_INIT;

/*======================================
 *	CHARACTER: Misc Sub Functions
 *--------------------------------------
 */

int mmo_char_tostr(char *str, struct mmo_charstatus *p)
{
	int i;

	if (p->party_id < 0)
		p->party_id = -1;

	else if (p->party_id > MAX_PARTYS)
		p->party_id = -1;

	if (p->party_status < 0)
		p->party_status = 0;

	else if (p->party_status > 2)
		p->party_status = 0;

	if (p->zeny < 0)
		p->zeny = 0;

	else if (p->zeny > MAX_ZENY)
		p->zeny = MAX_ZENY;

	sprintf(str, "%d\t%d,%d\t%s\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d,%d"
			 "\t%d,%d,%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
			 "\t%s,%d,%d\t%s,%d,%d",
			 p->char_id, p->account_id, p->char_num, p->name,
			 p->class, p->base_level, p->job_level,
			 p->base_exp, p->job_exp, p->zeny,
			 p->hp, p->max_hp, p->sp, p->max_sp,
			 p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
			 p->status_point, p->skill_point, p->special,
			 p->option_x, p->option_y, p->option_z, p->karma, p->manner,
			 p->party_id, p->party_status, p->pet_id,
			 p->hair, p->hair_color, p->clothes_color,
			 p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
			 p->last_point.map, p->last_point.x, p->last_point.y,
			 p->save_point.map, p->save_point.x, p->save_point.y);
	strcat(str, "\t");
	for (i = 0; i < 3; i++) {
		if (p->memo_point[i].map[0])
			sprintf(str + strlen(str), "%s,%d,%d", p->memo_point[i].map, p->memo_point[i].x, p->memo_point[i].y);
	}
  	strcat(str, "\t");
  	for (i = 0; i < MAX_INVENTORY; i++) {
		if (p->inventory[i].nameid)
			sprintf(str + strlen(str), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
				p->inventory[i].id, p->inventory[i].nameid, p->inventory[i].amount, p->inventory[i].equip,
				p->inventory[i].identify, p->inventory[i].refine, p->inventory[i].attribute,
				p->inventory[i].card[0], p->inventory[i].card[1], p->inventory[i].card[2],
				p->inventory[i].card[3]);
	}
	strcat(str, "\t");
	for (i = 0; i < MAX_CART; i++) {
		if (p->cart[i].nameid)
			sprintf(str + strlen(str), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
				p->cart[i].id, p->cart[i].nameid, p->cart[i].amount, p->cart[i].equip,
				p->cart[i].identify, p->cart[i].refine, p->cart[i].attribute,
				p->cart[i].card[0], p->cart[i].card[1], p->cart[i].card[2], p->cart[i].card[3]);
	}
	strcat(str, "\t");
	for (i = 0; i < MAX_SKILL; i++) {
		if (p->skill[i].id)
			sprintf(str + strlen(str), "%d,%d ", p->skill[i].id, p->skill[i].lv);
	}
  	strcat(str, "\t");
  	for (i = 0; i < GLOBAL_REG_NUM; i++) {
		if (p->global_reg[i].str[0])
			sprintf(str + strlen(str), "%s,%d ", p->global_reg[i].str, p->global_reg[i].value);
	}
	strcat(str, "\t");
  	return 0;
}

int mmo_char_fromstr(char *str, struct mmo_charstatus *p)
{
	int tmp_int[MAX_BUFFER];
	int set, next, len, i;

	if ((set = sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d,%d"
			     "\t%d,%d,%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
			     "\t%[^,],%d,%d\t%[^,],%d,%d%n",
			     &tmp_int[0], &tmp_int[1], &tmp_int[2], p->name,
			     &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
			     &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10],
			     &tmp_int[11], &tmp_int[12], &tmp_int[13], &tmp_int[14],
			     &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			     &tmp_int[19], &tmp_int[20], &tmp_int[21], &tmp_int[22], &tmp_int[23],
			     &tmp_int[24], &tmp_int[25], &tmp_int[26], &tmp_int[27],
			     &tmp_int[28], &tmp_int[29], &tmp_int[30], &tmp_int[31], &tmp_int[32],
			     &tmp_int[33], &tmp_int[34], &tmp_int[35], &tmp_int[36],
			     &tmp_int[37], p->last_point.map,
			     &tmp_int[38], &tmp_int[39], p->save_point.map, &tmp_int[40],
			     &tmp_int[41], &next)) != 45) {
		tmp_int[28] = 0;
		set = sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d,%d"
				     "\t%d,%d,%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
				     "\t%[^,],%d,%d\t%[^,],%d,%d%n",
				     &tmp_int[0], &tmp_int[1], &tmp_int[2], p->name,
				     &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
				     &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10],
				     &tmp_int[11], &tmp_int[12], &tmp_int[13], &tmp_int[14],
				     &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			 	     &tmp_int[19], &tmp_int[20], &tmp_int[21], &tmp_int[22], &tmp_int[23],
			  	     &tmp_int[24], &tmp_int[25], &tmp_int[26], &tmp_int[27],
				     &tmp_int[28], &tmp_int[29], &tmp_int[30], &tmp_int[31], &tmp_int[32],
				     &tmp_int[33], &tmp_int[34], &tmp_int[35], &tmp_int[36],
				     &tmp_int[37], p->last_point.map,
				     &tmp_int[38], &tmp_int[39], p->save_point.map, &tmp_int[40],
				     &tmp_int[41], &next);
		set++;
	}
	p->char_id = tmp_int[0];
	p->account_id = tmp_int[1];
	p->char_num = tmp_int[2];
	p->class = tmp_int[3];
	p->base_level = tmp_int[4];
	p->job_level = tmp_int[5];
	p->base_exp = tmp_int[6];
	p->job_exp = tmp_int[7];
	p->zeny = tmp_int[8];
	p->hp = tmp_int[9];
	p->max_hp = tmp_int[10];
	p->sp = tmp_int[11];
	p->max_sp = tmp_int[12];
	p->str = tmp_int[13];
	p->agi = tmp_int[14];
	p->vit = tmp_int[15];
	p->int_ = tmp_int[16];
	p->dex = tmp_int[17];
	p->luk = tmp_int[18];
	p->status_point = tmp_int[19];
	p->skill_point = tmp_int[20];
	p->special = tmp_int[21];
	p->option_x = tmp_int[22];
	p->option_y = tmp_int[23];
	p->option_z = tmp_int[24];
	p->karma = tmp_int[25];
	p->manner = tmp_int[26];
 	p->party_id = tmp_int[27];
	p->party_status = tmp_int[28];
	p->pet_id = tmp_int[29];
	p->hair = tmp_int[30];
	p->hair_color = tmp_int[31];
	p->clothes_color = tmp_int[32];
	p->weapon = tmp_int[33];
	p->shield = tmp_int[34];
	p->head_top = tmp_int[35];
	p->head_mid = tmp_int[36];
	p->head_bottom = tmp_int[37];
	p->last_point.x = tmp_int[38];
	p->last_point.y = tmp_int[39];
	p->save_point.x = tmp_int[40];
	p->save_point.y = tmp_int[41];
	p->party_invited = 0;
	p->last_memo_index = 0;
	if (set != 45)
		return 0;

	if ((str[next] == '\n') || (str[next] == '\r'))
		return 1;

	if (p->party_id < 0)
		p->party_id = -1;

	else if (p->party_id > MAX_PARTYS)
		p->party_id = -1;

	if (p->party_status < 0)
		p->party_status = 0;

	else if (p->party_status > 2)
		p->party_status = 0;

	if (p->zeny < 0)
		p->zeny = 0;

	else if (p->zeny > MAX_ZENY)
		p->zeny = 1000000000;

	next++;
	for (i = 0; str[next] && str[next] != '\t'; i++) {
		set = sscanf(str + next, "%[^,],%d,%d%n", p->memo_point[i].map, &tmp_int[0], &tmp_int[1], &len);
		if (set != 3)
			return 0;

		p->memo_point[i].x = tmp_int[0];
		p->memo_point[i].y = tmp_int[1];
		next += len;
		if (str[next] == ' ')
			next++;

	}
	next++;
	for (i = 0; str[next] && str[next] != '\t'; i++) {
		set = sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
			     &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
			     &tmp_int[4], &tmp_int[5], &tmp_int[6], &tmp_int[7],
			     &tmp_int[8], &tmp_int[9], &tmp_int[10], &len);
		if (set != 11)
			return 0;

		p->inventory[i].id = tmp_int[0];
		p->inventory[i].nameid = tmp_int[1];
		p->inventory[i].amount = tmp_int[2];
		p->inventory[i].equip = tmp_int[3];
		p->inventory[i].identify = tmp_int[4];
		p->inventory[i].refine = tmp_int[5];
		p->inventory[i].attribute = tmp_int[6];
		p->inventory[i].card[0] = tmp_int[7];
		p->inventory[i].card[1] = tmp_int[8];
		p->inventory[i].card[2] = tmp_int[9];
		p->inventory[i].card[3] = tmp_int[10];
		next += len;
		if (str[next] == ' ')
			next++;

	}
	next++;
	for (i = 0; str[next] && str[next] != '\t'; i++) {
		set = sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
			     &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
			     &tmp_int[4], &tmp_int[5], &tmp_int[6], &tmp_int[7],
			     &tmp_int[8], &tmp_int[9], &tmp_int[10], &len);
		if (set != 11)
			return 0;

		p->cart[i].id = tmp_int[0];
		p->cart[i].nameid = tmp_int[1];
		p->cart[i].amount = tmp_int[2];
		p->cart[i].equip = tmp_int[3];
		p->cart[i].identify = tmp_int[4];
		p->cart[i].refine = tmp_int[5];
		p->cart[i].attribute = tmp_int[6];
		p->cart[i].card[0] = tmp_int[7];
		p->cart[i].card[1] = tmp_int[8];
		p->cart[i].card[2] = tmp_int[9];
		p->cart[i].card[3] = tmp_int[10];
		next += len;
		if (str[next] == ' ')
			next++;

	}
	next++;
	for (i = 0; str[next] && str[next] != '\t'; i++) {
		set = sscanf(str + next, "%d,%d%n", &tmp_int[0], &tmp_int[1], &len);
		if (set != 2)
			return 0;

		p->skill[tmp_int[0] - 1].id = tmp_int[0];
		p->skill[tmp_int[0] - 1].lv = tmp_int[1];
		next += len;
		if (str[next] == ' ')
			next++;

	}
	next++;
	for (i = 0; str[next] && str[next] != '\t' && str[next] != '\r' && str[next] != '\n'; i++) {
		set = sscanf(str + next, "%[^,],%d%n", p->global_reg[i].str, &tmp_int[0], &len);
		if (set != 2)
			return 0;

		p->global_reg[i].value = tmp_int[0];
		next += len;
		if (str[next] == ' ')
			next++;

	}
	return 1;
}

int mmo_storage_tostr(char *str, struct mmo_charstatus *p)
{
	int i;

	sprintf(str, "%d,%d", p->account_id, p->storage_amount);
	strcat(str, "\t");
	for (i = 0; i < MAX_STORAGE; i++) {
		if ((p->storage[i].nameid) && (p->storage[i].amount))
			sprintf(str + strlen(str), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
				p->storage[i].id, p->storage[i].nameid, p->storage[i].amount, p->storage[i].equip,
				p->storage[i].identify, p->storage[i].refine, p->storage[i].attribute,
				p->storage[i].card[0], p->storage[i].card[1], p->storage[i].card[2], p->storage[i].card[3]);
	}
	strcat(str, "\t");
	return 0;
}

int mmo_storage_fromstr(char *str, struct mmo_charstatus *p)
{
	int set, next, len, i;
	int tmp_int[MAX_BUFFER];

	set = sscanf(str, "%d,%d%n", &tmp_int[0], &tmp_int[1], &next);
	p->storage_id = tmp_int[0];
	p->storage_amount = tmp_int[1];

	if (set != 2)
		return 0;

	if ((str[next] == '\n') || (str[next]== '\r'))
		return 1;

	next++;
	for (i = 0; str[next] && str[next] != '\t'; i++) {
		set = sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
			&tmp_int[4], &tmp_int[5], &tmp_int[6], &tmp_int[7],
			&tmp_int[8], &tmp_int[9], &tmp_int[10], &len);
		if (set != 11)
			return 0;

		p->storage[i].id = tmp_int[0];
		p->storage[i].nameid = tmp_int[1];
		p->storage[i].amount = tmp_int[2];
		p->storage[i].equip = tmp_int[3];
		p->storage[i].identify = tmp_int[4];
		p->storage[i].refine = tmp_int[5];
		p->storage[i].attribute = tmp_int[6];
		p->storage[i].card[0] = tmp_int[7];
		p->storage[i].card[1] = tmp_int[8];
		p->storage[i].card[2] = tmp_int[9];
		p->storage[i].card[3] = tmp_int[10];

		next += len;
		if (str[next] == ' ')
			next++;
	}
	return 1;
}

int mmo_pet_tostr(char *str, struct mmo_charstatus *p)
{
	if (p->pet.pet_hungry < 0)
		p->pet.pet_hungry = 0;

	else if (p->pet.pet_hungry > 100)
		p->pet.pet_hungry = 100;

	if (p->pet.pet_friendly < 0)
		p->pet.pet_friendly = 0;

	else if (p->pet.pet_friendly > 1000)
		p->pet.pet_friendly = 1000;

	sprintf(str,"%d,%d,%s\t%d,%d,%d,%d,%d,%d,%d,%d,%d",
		p->pet.pet_id, p->pet.pet_class, p->pet.pet_name, p->pet.account_id,
		p->pet.char_id, p->pet.pet_level, p->pet.mons_id, p->pet.pet_accessory,
		p->pet.pet_friendly, p->pet.pet_hungry, p->pet.pet_name_flag, p->pet.activity);
	return 0;
}

int mmo_pet_fromstr(char *str, struct mmo_charstatus *p)
{
	int i;
	int tmp_int[16];
	char tmp_str[MAX_BUFFER];

	i = sscanf(str,"%d,%d,%[^\t]\t%d,%d,%d,%d,%d,%d,%d,%d,%d",&tmp_int[0],&tmp_int[1],tmp_str,&tmp_int[2],
		&tmp_int[3],&tmp_int[4],&tmp_int[5],&tmp_int[6],&tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10]);

	if (i != 12)
		return 0;

	p->pet.pet_id = tmp_int[0];
	p->pet.pet_class = tmp_int[1];
	memcpy(p->pet.pet_name,tmp_str,24);
	p->pet.account_id = tmp_int[2];
	p->pet.char_id = tmp_int[3];
	p->pet.pet_level = tmp_int[4];
	p->pet.mons_id = tmp_int[5];
	p->pet.pet_accessory = tmp_int[6];
	p->pet.pet_friendly = tmp_int[7];
	p->pet.pet_hungry = tmp_int[8];
	p->pet.pet_name_flag= tmp_int[9];
	p->pet.activity = tmp_int[10];

	if (p->pet.pet_hungry < 0)
		p->pet.pet_hungry = 0;

	else if (p->pet.pet_hungry > 100)
		p->pet.pet_hungry = 100;

	if (p->pet.pet_friendly < 0)
		p->pet.pet_friendly = 0;

	else if (p->pet.pet_friendly > 1000)
		p->pet.pet_friendly = 1000;

	return 1;
}

/*======================================
 *	CHARACTER: Misc Functions
 *--------------------------------------
 */

static void mmo_char_init(void)
{
	int ret;
	char line[65536];
	FILE *fp = fopen("save/characters/characters.txt", "r");
	char_dat = calloc(sizeof(char_dat[0]) * MAX_BUFFER, 1);
	if (!char_dat)
		exit(0);

	char_max = MAX_BUFFER;
	if (!fp)
		exit(1);

	while (fgets(line, 65535, fp)) {
		if (char_num >= char_max) {
			char_max += MAX_BUFFER;
			char_dat = realloc(char_dat, sizeof(char_dat[0]) * char_max);
			memset(char_dat + (char_max - MAX_BUFFER), '\0', MAX_BUFFER * sizeof(*char_dat));
		}
		memset(&char_dat[char_num], 0, sizeof(char_dat[0]));
		ret = mmo_char_fromstr(line, &char_dat[char_num]);
		if (ret) {
			if (char_dat[char_num].char_id >= char_id_count)
				char_id_count = char_dat[char_num].char_id + 1;

			char_num++;
		}
	}
	fclose(fp);
}

static void mmo_pet_init(void)
{
	int ret;
	char line[65536];
	FILE *fp = fopen("save/pets/pet.txt", "r");
	pet_dat = calloc(sizeof(pet_dat[0]) * MAX_BUFFER, 1);
	if (!pet_dat)
		exit(0);

	pet_max = MAX_BUFFER;
	if (!fp)
		exit(1);

	while (fgets(line, 65535, fp)) {
		if (pet_num >= pet_max) {
			pet_max += MAX_BUFFER;
			pet_dat = realloc(pet_dat, sizeof(pet_dat[0]) * pet_max);
			memset(pet_dat + (pet_max - MAX_BUFFER), '\0', MAX_BUFFER * sizeof(*pet_dat));
		}
		memset(&pet_dat[pet_num], 0, sizeof(pet_dat[pet_num]));
		ret = mmo_pet_fromstr(line, &pet_dat[pet_num]);
		if (ret) {
			if (pet_dat[pet_num].pet.pet_id >= pet_id_count)
				pet_id_count = pet_dat[pet_num].pet.pet_id + 1;

			pet_num++;
		}
	}
	fclose(fp);
}

static void mmo_storage_init(void)
{
	int i, set;
	int account_id, tmp_int;
	char line[65536];
	FILE *fp = fopen("save/storages/storage.txt", "r");
	if (!fp)
		exit(0);

	while (fgets(line, 65535, fp)) {
		set = sscanf(line, "%d", &tmp_int);
		if (set == 1) {
			account_id = tmp_int;
			for (i = 0; i < char_num; i++) {
				if (char_dat[i].account_id == account_id)
					mmo_storage_fromstr(line, &char_dat[i]);
			}
		}
	}
	fclose(fp);
}

void reset_member_data(int index, short mbr)
{
	if (index >= 0 && index < MAX_PARTYS) {
		if (mbr >= 0 && mbr < MAX_PARTY_MEMBERS) {
			memset(party_dat[index].member[mbr].nick, 0, 24);
			memset(party_dat[index].member[mbr].map_name, 0, 16);
			party_dat[index].member[mbr].account_id = 0;
			party_dat[index].member[mbr].fd = 0;
			party_dat[index].member[mbr].x = 0;
			party_dat[index].member[mbr].y = 0;
			party_dat[index].member[mbr].hp = 0;
			party_dat[index].member[mbr].max_hp = 0;
			party_dat[index].member[mbr].offline = 1;
			party_dat[index].member[mbr].leader = 1;
			party_dat[index].member[mbr].char_id = -1;
			party_dat[index].member[mbr].mapno = -1;
		}
	}
}

void init_party_data(int index)
{
	int i;

	if (index >= 0 && index < MAX_PARTYS) {
		party_dat[index].party_id = -1;
		party_dat[index].exp = 0;
		party_dat[index].item = 0;
		party_dat[index].member_count = 0;
		party_dat[index].leader_level = 0;
		memset(party_dat[index].party_name, 0, 24);
		for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
			reset_member_data(index, i);
		}
	}
	return;
}

static void mmo_party_init(void)
{
	int i, j;
	int leader_flag;
	int member_index;
	char line[1024];
	FILE *fp = NULL;
	struct mmo_party party_temp;

	for (i = 0; i < MAX_PARTYS; i++) {
		init_party_data(i);
	}
	fp = fopen("save/parties/parties.txt", "r");
	if (fp) {
		while (fgets(line, 1023, fp)) {
			if (strncmp(line, "//", 2) != 0) {
				party_temp.party_id = -1;
				party_temp.exp = 0;
				party_temp.item = 0;
				party_temp.member_count = 0;
				party_temp.leader_level = 0;
				memset(party_temp.party_name, 0, 24);
				if (sscanf(line, "%d,%hd,%hd,%[^,],%d", &party_temp.party_id, &party_temp.exp,
				           &party_temp.item, party_temp.party_name, &i) == 5) {
					if (party_temp.party_id > -1 && party_temp.party_id < MAX_PARTYS &&
					    party_temp.exp >= 0 && party_temp.exp <= 2 &&
					    strlen(party_temp.party_name) > 0 && strlen(party_temp.party_name) < 25) {
						for(i = 0; i < MAX_PARTYS; i++) {
							if (strcmp(party_dat[i].party_name, party_temp.party_name) == 0)
								break;
						}
						if (i != MAX_PARTYS) {
							if (party_temp.exp == 2)
								party_temp.exp = 0;

							for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
								memset(party_temp.member[i].nick, 0, 24);
								memset(party_temp.member[i].map_name, 0, 16);
								party_temp.member[i].account_id = 0;
								party_temp.member[i].fd = 0;
								party_temp.member[i].x = 0;
								party_temp.member[i].y = 0;
								party_temp.member[i].hp = 0;
								party_temp.member[i].max_hp = 0;
								party_temp.member[i].offline = 1;
								party_temp.member[i].leader = 1;
								party_temp.member[i].char_id = -1;
								party_temp.member[i].mapno = -1;
							}
							party_temp.member_count = 0;
							member_index = -1;
							leader_flag = 0;
							for (i = 0; i < char_num; i++) {
								if (char_dat[i].party_id == party_temp.party_id) {
									if (party_temp.member_count == MAX_PARTY_MEMBERS ||
									    char_dat[i].skill[0].lv < 5) {
										char_dat[i].party_status = 0;
										char_dat[i].party_id = -1;
										strcpy(char_dat[i].party_name, "");
									}
									else {
										strcpy(party_temp.member[party_temp.member_count].nick, char_dat[i].name);
										strcpy(party_temp.member[party_temp.member_count].map_name, char_dat[i].last_point.map);
										party_temp.member[party_temp.member_count].account_id = char_dat[i].account_id;
										party_temp.member[party_temp.member_count].fd = 0;
										party_temp.member[party_temp.member_count].x = char_dat[i].last_point.x;
										party_temp.member[party_temp.member_count].y = char_dat[i].last_point.y;
										party_temp.member[party_temp.member_count].hp = char_dat[i].hp;
										party_temp.member[party_temp.member_count].max_hp = char_dat[i].max_hp;
										if (char_dat[i].party_status == 2) {
											if (leader_flag == 1 || char_dat[i].skill[0].lv < 7)
												char_dat[i].party_status = 1;

											else {
												party_temp.leader_level = char_dat[i].base_level;
												party_temp.member[party_temp.member_count].leader = 0;
												leader_flag = 1;
											}
										}
										else if (member_index == -1 && char_dat[i].skill[0].lv >= 7)
											member_index = i;

										party_temp.member[party_temp.member_count].char_id = char_dat[i].char_id;
										strcpy(char_dat[i].party_name, party_temp.party_name);
										party_temp.member_count++;
									}
								}
							}
							if (party_temp.member_count > 0) {
								if (leader_flag == 0) {
									if (member_index != -1) {
										for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
											if (party_temp.member[i].account_id == char_dat[member_index].account_id) {
												leader_flag = 1;
												party_temp.leader_level = char_dat[member_index].base_level;
												party_temp.member[i].leader = 0;
												char_dat[member_index].party_status = 2;
												break;
											}
										}
									}
									else {
										for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
											if (party_temp.member[j].char_id != -1) {
												for (i = 0; i < char_num; i++) {
													if (party_temp.member[j].char_id == char_dat[i].char_id) {
														char_dat[i].party_status = 0;
														char_dat[i].party_id = -1;
														strcpy(char_dat[i].party_name, "");
														break;
													}
												}
											}
										}
									}
								}
								if (leader_flag == 1)
									memcpy(&party_dat[party_temp.party_id], &party_temp, sizeof(party_temp));

							}
						}
					}
				}
			}
		}
		fclose(fp);
	}
	for(i = 0; i < char_num; i++) {
		if (char_dat[i].party_id != -1) {
			if (party_dat[char_dat[i].party_id].party_id == -1) {
				char_dat[i].party_status = 0;
				char_dat[i].party_id = -1;
				strcpy(char_dat[i].party_name, "");
			}
		}
	}
}

static void mmo_char_sync(void)
{
	int i;
	char line[65536];
	FILE *fp = NULL;

	fp = fopen("save/characters/characters.txt", "w");
	if (fp) {
		for (i = 0; i < char_num; i++) {
			mmo_char_tostr(line, &char_dat[i]);
			fprintf(fp, "%s\n", line);
		}
	}
	fclose(fp);
}

static void mmo_pet_sync(void)
{
	int i;
	char line[65536];
	FILE *fp = NULL;

	fp = fopen("save/pets/pet.txt", "w");
	if (fp) {
		for(i = 0; i < pet_num; i++) {
			mmo_pet_tostr(line, &pet_dat[i]);
     			fprintf(fp,"%s\n",line);
			mmo_storage_fromstr(line, &char_dat[i]);
   		}
	}
	fclose(fp);
}

static void mmo_storage_sync(void)
{
	int i;
	char line[65536];
	FILE *fp = NULL;

	fp = fopen("save/storages/storage.txt", "w");
	if (fp) {
		for (i = 0; i < char_num; i++) {
			mmo_storage_tostr(line, &char_dat[i]);
			fprintf(fp, "%s\n", line);
		}
	}
	fclose(fp);
}

static void mmo_party_sync(void)
{
	int i, k, j;
	int char_ids[MAX_PARTY_MEMBERS];
	FILE *fp;
	FILE *fp2;

	for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
		char_ids[i] = -1;
	}
	fp = fopen("save/parties/parties.txt", "w");
	if (fp) {
		fp2 = fopen("save/parties/party_members.txt", "w");
		if (fp2) {
			for(k = 0; k < MAX_PARTYS; k++) {
				if (party_dat[k].party_id >= 0 && party_dat[k].member_count > 0) {
					fprintf(fp, "%d,%d,%d,%s,%d\n", party_dat[k].party_id, party_dat[k].exp, party_dat[k].item, party_dat[k].party_name, party_dat[k].member_count);
					for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
						char_ids[i] = party_dat[k].member[i].char_id;
						if (char_ids[i] <= 0)
							char_ids[i] = -1;
					}
					for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
						for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
							if (i != j && party_dat[k].member[i].char_id == char_ids[j])
								char_ids[j] = -1;
						}
					}
					fprintf(fp2, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", party_dat[k].party_id, char_ids[0],
					        char_ids[1], char_ids[2], char_ids[3], char_ids[4], char_ids[5], char_ids[6],
					        char_ids[7], char_ids[8], char_ids[9], char_ids[10], char_ids[11]);
				}
			}
			fclose(fp2);
		}
		fclose(fp);
	}
}

/*======================================
 *	CHARACTER: Create New PC
 *--------------------------------------
 */

static int make_new_char(unsigned int fd, unsigned char *dat)
{
	int i;
	struct char_session_data *sd = NULL;

	if ((dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29] > 5 * 6) || (dat[30] >= MAX_CHAR_SLOTS || dat[33] == 0) || (dat[33] >= 20 || dat[31] >= 9))
		return -1;

	sd = session[fd]->session_data;
	for (i = 0; i < char_num; i++) {
		if ((strncmp(char_dat[i].name, dat, 24) == 0) || (char_dat[i].account_id == sd->account_id && char_dat[i].char_num == dat[30]))
			break;
	}
	if (i != char_num)
		return -1;

	if (char_num >= char_max) {
		char_max += MAX_BUFFER;
		char_dat = realloc(char_dat, sizeof(char_dat[0]) * char_max);
		if (!char_dat)
			exit(1);

		memset(char_dat + (char_max - MAX_BUFFER), '\0', MAX_BUFFER * sizeof(*char_dat));
	}
	memset(&char_dat[i], 0, sizeof(char_dat[0]));
	char_dat[i].char_id = char_id_count++;
	char_dat[i].account_id = sd->account_id;
	char_dat[i].char_num = dat[30];
	strncpy(char_dat[i].name, dat, 24);
	char_dat[i].class = 0;
	char_dat[i].base_level = 1;
	char_dat[i].job_level = 1;
	char_dat[i].base_exp = 0;
	char_dat[i].job_exp = 0;
	char_dat[i].zeny = start_zeny;
	char_dat[i].str = dat[24];
	char_dat[i].agi = dat[25];
	char_dat[i].vit = dat[26];
	char_dat[i].int_ = dat[27];
	char_dat[i].dex = dat[28];
	char_dat[i].luk = dat[29];
	char_dat[i].max_hp = 40;
	char_dat[i].max_sp = 2;
	char_dat[i].hp = char_dat[i].max_hp;
	char_dat[i].sp = char_dat[i].max_sp;
	char_dat[i].status_point = 0;
	char_dat[i].skill_point = 0;
	char_dat[i].special = 0;
	char_dat[i].option_x = 0;
	char_dat[i].option_y = 0;
	char_dat[i].option_z = 0;
	char_dat[i].karma = 0;
	char_dat[i].manner = 0;
	char_dat[i].party_id = -1;
	char_dat[i].party_status = 0;
	char_dat[i].party_invited = 0;
	strcpy(char_dat[i].party_name, "");
	char_dat[i].hair = dat[33];
	char_dat[i].hair_color = dat[31];
	char_dat[i].clothes_color = 0;
	char_dat[i].inventory[0].nameid = 1201;
	char_dat[i].inventory[0].amount = 1;
	char_dat[i].inventory[0].equip = 0x02;
	char_dat[i].inventory[0].identify = 1;
	char_dat[i].inventory[1].nameid = 2301;
	char_dat[i].inventory[1].amount = 1;
	char_dat[i].inventory[1].equip = 0x10;
	char_dat[i].inventory[1].identify = 1;
	char_dat[i].weapon = 1;
	char_dat[i].shield = 0;
	char_dat[i].head_top = 0;
	char_dat[i].head_mid = 0;
	char_dat[i].head_bottom = 0;
	memcpy(&char_dat[i].last_point, &start_point, sizeof(start_point));
	memcpy(&char_dat[i].save_point, &start_point, sizeof(start_point));
	char_num++;

	mmo_char_sync();
	return i;
}

/*======================================
 *	CHARACTER: Display Characters
 *--------------------------------------
 */

static int mmo_char_send006b(int fd, struct char_session_data *sd)
{
	const int offset = 24;
	int i, j, found_num;

	sd->state = CHAR_STATE_AUTHOK;
	for (i = found_num = 0; i < char_num; i++) {
		if (char_dat[i].account_id == sd->account_id) {
			sd->found_char[found_num] = i;
			found_num++;
			if (found_num == MAX_CHAR_SLOTS) {
				break;
			}
		}
	}
	for (i = found_num; i < MAX_CHAR_SLOTS; i++) {
		sd->found_char[i] = -1;
	}
	memset(WFIFOP(fd, 0), 0, offset + found_num * 106);
	WFIFOW(fd, 0) = 0x6b;
	WFIFOW(fd, 2) = offset + found_num * 106;

	for (i = 0; i < found_num; i++) {
		j = sd->found_char[i];

		memset(WFIFOP(fd, offset + (i * 106)), 0x00, 106);

		WFIFOL(fd, offset + (i * 106)) = char_dat[j].char_id;
		WFIFOL(fd, offset + (i * 106) + 4) = char_dat[j].base_exp;
		WFIFOL(fd, offset + (i * 106) + 8) = char_dat[j].zeny;
		WFIFOL(fd, offset + (i * 106) + 12) = char_dat[j].job_exp;
		WFIFOL(fd, offset + (i * 106) + 16) = char_dat[j].job_level;

		WFIFOL(fd, offset + (i * 106) + 20) = 0;
		WFIFOL(fd, offset + (i * 106) + 24) = 0;
		WFIFOL(fd, offset + (i * 106) + 28) = char_dat[j].special;

		WFIFOL(fd, offset + (i * 106) + 32) = char_dat[j].karma;
		WFIFOL(fd, offset + (i * 106) + 36) = char_dat[j].manner;

		WFIFOW(fd, offset + (i * 106) + 40) = char_dat[j].status_point;
		WFIFOW(fd, offset + (i * 106) + 42) = (char_dat[j].hp > 0x7fff)? 0x7fff:char_dat[j].hp;
		WFIFOW(fd, offset + (i * 106) + 44) = (char_dat[j].max_hp > 0x7fff)? 0x7fff:char_dat[j].max_hp;
		WFIFOW(fd, offset + (i * 106) + 46) = (char_dat[j].sp > 0x7fff)? 0x7fff:char_dat[j].sp;
		WFIFOW(fd, offset + (i * 106) + 48) = (char_dat[j].max_sp > 0x7fff)? 0x7fff:char_dat[j].max_sp;
		WFIFOW(fd, offset + (i * 106) + 50) = DEFAULT_WALK_SPEED;
		WFIFOW(fd, offset + (i * 106) + 52) = char_dat[j].class;
		WFIFOW(fd, offset + (i * 106) + 54) = char_dat[j].hair;
		WFIFOW(fd, offset + (i * 106) + 56) = char_dat[j].weapon;
		WFIFOW(fd, offset + (i * 106) + 58) = char_dat[j].base_level;
		WFIFOW(fd, offset + (i * 106) + 60) = char_dat[j].skill_point;
		WFIFOW(fd, offset + (i * 106) + 62) = char_dat[j].head_bottom;
		WFIFOW(fd, offset + (i * 106) + 64) = char_dat[j].shield;
		WFIFOW(fd, offset + (i * 106) + 66) = char_dat[j].head_top;
		WFIFOW(fd, offset + (i * 106) + 68) = char_dat[j].head_mid;
		WFIFOW(fd, offset + (i * 106) + 70) = char_dat[j].hair_color;
		WFIFOW(fd, offset + (i * 106) + 72) = char_dat[j].clothes_color;

 		memcpy( WFIFOP(fd, offset + (i * 106) + 74), char_dat[j].name, 24 );

		WFIFOB(fd, offset + (i * 106) + 98) = (char_dat[j].str > 255)? 255:char_dat[j].str;
		WFIFOB(fd, offset + (i * 106) + 99) = (char_dat[j].agi > 255)? 255:char_dat[j].agi;
		WFIFOB(fd, offset + (i * 106) + 100) = (char_dat[j].vit > 255)? 255:char_dat[j].vit;
		WFIFOB(fd, offset + (i * 106) + 101) = (char_dat[j].int_ > 255)? 255:char_dat[j].int_;
		WFIFOB(fd, offset + (i * 106) + 102) = (char_dat[j].dex > 255)? 255:char_dat[j].dex;
		WFIFOB(fd, offset + (i * 106) + 103) = (char_dat[j].luk > 255)? 255:char_dat[j].luk;
		WFIFOB(fd, offset + (i * 106) + 104) = char_dat[j].char_num;
	}
	WFIFOSET(fd, WFIFOW(fd, 2));
	return 0;
}

/*======================================
 *	CHARACTER: Parse Functions
 *--------------------------------------
 */

int parse_tologin(unsigned int fd)
{
	int i, fdc;
	struct char_session_data *sd = NULL;

	if (session[fd]->eof) {
		if (fd == login_fd)
			login_fd = -1;

		close(fd);
		delete_session(fd);
		return 0;
	}
	sd = session[fd]->session_data;
	while (RFIFOREST(fd) >= 2) {
		switch (RFIFOW(fd, 0)) {
			case 0x2711:
				if (RFIFOREST(fd) < 3)
					return 0;

				if (RFIFOB(fd, 2) == 3)
					exit(0);

				RFIFOSKIP(fd, 3);
				break;

			case 0x2713:
				if (RFIFOREST(fd) < 48)
					return 0;

				for (i = 5; i < FD_SETSIZE; i++) {
					if (session[i] && (sd = session[i]->session_data)) {
						if (sd->account_id == (signed)RFIFOL(fd, 2))
							break;
					}
				}
				fdc = i;
				if (fdc == FD_SETSIZE) {
					RFIFOSKIP(fd, 8);
					break;
				}
				if (RFIFOB(fd, 6) != 0) {
					WFIFOW(fdc, 0) = 0x6c;
					WFIFOB(fdc, 2) = 0x42;
					WFIFOSET(fdc, 3);
					RFIFOSKIP(fd, 8);
					break;
				}
				sd->network = RFIFOB(fd,7);
				memcpy(sd->email, RFIFOP(fd, 8), 60);
				if ((strlen(sd->email) < 3) || (strlen(sd->email) > 60) || (strchr(sd->email, '@') == NULL) || (sd->email[0] == '@') || (sd->email[strlen(sd->email)-1] == '@'))
					strcpy(sd->email, "sys@srv.com");

				else
					strtolower(sd->email);

				mmo_char_send006b(fdc, sd);
				RFIFOSKIP(fd, 48);
				break;

	    		default:
				close(fd);
				session[fd]->eof = 1;
				return 0;
		}
	}
	RFIFOFLUSH(fd);
	return 0;
}

int parse_frommap(unsigned int fd)
{
#ifdef _MMS_
	int c;
#endif
	int i, j, id;
	int leader_id;
	int curlin;

	for (id = 0; id < MAX_SERVERS; id++) {
		if ((unsigned)server_fd[id] == fd)
			break;
	}
	if (id == MAX_SERVERS)
		session[fd]->eof = 1;

	if (session[fd]->eof) {
		for (i = 0; i < MAX_SERVERS; i++) {
			if ((unsigned)server_fd[i] == fd)
				server_fd[i] = -1;
		}
		close(fd);
		delete_session(fd);
		return 0;
	}
	while (RFIFOREST(fd) >= 2) {
		switch (RFIFOW(fd, 0)) {
			case 0x2afa:
				if ((RFIFOREST(fd) < 4) || (RFIFOREST(fd) < RFIFOW(fd, 2)))
					return 0;

				for (i = 4, j = 0; i < RFIFOW(fd, 2); i += 16, j++) {
					memcpy(server[id].map[j], RFIFOP(fd, i), 16);
				}
				server[id].map[j][0] = 0;
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				WFIFOW(fd, 0) = 0x2afb;
				WFIFOW(fd, 2) = 0;
				WFIFOSET(fd, 3);
#ifdef _MMS_
				for (c = 0; c < MAX_SERVERS; c++) {
					if (server_fd[c] > 0) {
						for (i = 0; i < MAX_SERVERS; i++) {
							int fd;
							if ((fd = server_fd[i]) > 0) {
								if (i == c)
									continue;

								WFIFOW(fd, 0) = 0x2c00;
								WFIFOL(fd, 4) = server[c].ip;
								WFIFOW(fd, 8) = server[c].port;

								for (j = 0; server[c].map[j][0]; j++) {
									memcpy(WFIFOP(fd, 10 + j * 16), server[c].map[j], 16);
								}
								WFIFOW(fd, 2) = 10 + j * 16;
								WFIFOSET(fd, WFIFOW(fd, 2));
							}
						}
					}
				}
#endif

			case 0x2afc:
				if (RFIFOREST(fd) < 14)
					return 0;

				for (i = 0; i < AUTH_FIFO_SIZE; i++) {
					if (auth_fifo[i].account_id == (signed)RFIFOL(fd, 2)
					    && auth_fifo[i].char_id == (signed)RFIFOL(fd, 6)
					    && auth_fifo[i].login_id1 == (signed)RFIFOL(fd, 10)
					    && !auth_fifo[i].delflag) {
						auth_fifo[i].delflag = 1;
						break;
					}
				}
				if (i == AUTH_FIFO_SIZE) {
					WFIFOW(fd, 0) = 0x2afe;
					WFIFOW(fd, 2) = RFIFOL(fd, 2);
					WFIFOB(fd, 6) = 0;
					WFIFOSET(fd, 7);
				}
				else {
#ifdef _MMS_
					char_dat[auth_fifo[i].char_pos].lastseen_map_fd = fd;
#endif
					WFIFOW(fd, 0) = 0x2afd;
					WFIFOW(fd, 2) = 12 + sizeof(char_dat[0]);
					WFIFOL(fd, 4) = RFIFOL(fd, 2);
					WFIFOL(fd, 8) = RFIFOL(fd, 6);
					memcpy(WFIFOP(fd, 12), &char_dat[auth_fifo[i].char_pos], sizeof(char_dat[0]));
					WFIFOSET(fd, WFIFOW(fd, 2));
				}
				RFIFOSKIP(fd, 14);
				break;

			case 0x2aff:
				if (RFIFOREST(fd) < 6)
					return 0;

				server[id].users = RFIFOL(fd, 2);
				RFIFOSKIP(fd, 6);
				break;

			case 0x30ff:
				if (RFIFOREST(fd) < 6)
					return 0;

				WFIFOW(login_fd, 0) = 0x30fa;
				WFIFOL(login_fd, 2) = RFIFOL(fd, 2);
				WFIFOSET(login_fd, 6);
				RFIFOSKIP(fd, 6);
				break;

			case 0x2b01:
				if ((RFIFOREST(fd) < 4) || (RFIFOREST(fd) < RFIFOW(fd, 2)))
					return 0;

				for (i = 0; i < char_num; i++) {
					if (char_dat[i].account_id == (signed)RFIFOL(fd, 4)
					    && char_dat[i].char_id == (signed)RFIFOL(fd, 8))
						break;

				}
				if (i != char_num) {
					memcpy(&char_dat[i], RFIFOP(fd, 12), sizeof(char_dat[0]));
					mmo_char_sync();
					mmo_storage_sync();
				}
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;

			case 0x2b02:
				if (RFIFOREST(fd) < 10)
					return 0;

				if (auth_fifo_pos >= AUTH_FIFO_SIZE)
					auth_fifo_pos = 0;

				auth_fifo[auth_fifo_pos].account_id = RFIFOL(fd, 2);
				auth_fifo[auth_fifo_pos].char_id = 0;
				auth_fifo[auth_fifo_pos].login_id1 = RFIFOL(fd, 6);
				auth_fifo[auth_fifo_pos].delflag = 2;
				auth_fifo[auth_fifo_pos].char_pos = 0;
				auth_fifo_pos++;

				WFIFOW(fd, 0) = 0x2b03;
				WFIFOL(fd, 2)= RFIFOL(fd, 2);
				WFIFOB(fd, 6) = 0;
				WFIFOSET(fd, 7);

				RFIFOSKIP(fd, 10);
				break;

#ifdef _MMS_
			case 0x2c01:
				if (RFIFOREST(fd) < 14)
					return 0;

				for (i = 0; i < AUTH_FIFO_SIZE; i++) {
					if (auth_fifo[i].account_id == (signed)RFIFOL(fd, 2) &&
					    auth_fifo[i].char_id == (signed)RFIFOL(fd, 6) &&
					    auth_fifo[i].login_id1 == (signed)RFIFOL(fd, 10) &&
					    auth_fifo[i].delflag) {
						auth_fifo[i].delflag = 0;
						break;
					}
				}
				if (i == AUTH_FIFO_SIZE) {
					WFIFOW(fd, 0) = 0x2afe;
					WFIFOW(fd, 2) = RFIFOL(fd, 2);
					WFIFOB(fd, 6) = 0;
					WFIFOSET(fd, 7);
				}
				RFIFOSKIP(fd, 14);
				break;

			case 0x2c02:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				for (i = 0; i < char_num; i++) {
					if (char_dat[i].lastseen_map_fd || (char_dat[i].lastseen_map_fd == (signed)fd) || (strcmp(char_dat[i].name, RFIFOP(fd, 4)) != 0))
						break;
				}
				for (j = 0; j < MAX_SERVERS; j++) {
					if (server_fd[j] == char_dat[i].lastseen_map_fd)
						break;
				}
				if ((i == char_num)||(j == MAX_SERVERS)) {
					WFIFOW(fd, 0) = 0x2c03;
					WFIFOW(fd, 2) = 28;
					memcpy(WFIFOP(fd, 4), RFIFOP(fd, 28), 24);
					WFIFOSET(fd, 28);
				}
				else {
					id = char_dat[i].lastseen_map_fd;
					WFIFOW(id, 0) = 0x2c04;
					WFIFOW(id, 2) = RFIFOW(fd, 2);
					memcpy(WFIFOP(id, 4), RFIFOP(fd, 4), RFIFOW(fd, 2) - 4);
					WFIFOSET(id, RFIFOW(fd, 2));
				}
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;

			case 0x2c05:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				for (i = 0; i < char_num; i++) {
					if (char_dat[i].lastseen_map_fd == 1 || (char_dat[i].lastseen_map_fd == (signed)fd) || (strcmp(char_dat[i].name, RFIFOP(fd, 4)) != 0))
						break;
				}
				for (j = 0; j < MAX_SERVERS; j++) {
					if (server_fd[j] == char_dat[i].lastseen_map_fd)
						break;
				}
				if ((i != char_num) && (j != MAX_SERVERS)) {
					id = char_dat[i].lastseen_map_fd;
					WFIFOW(id, 0) = 0x2c03;
					WFIFOW(id, 2) = 28;
					memcpy(WFIFOP(id, 4), RFIFOP(fd, 28), 24);
					WFIFOSET(id, 28);
				}
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;

			case 0x2c06:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				for (j = 0; j < MAX_SERVERS; j++) {
					if ((server_fd[j] < 0) || ((unsigned)server_fd[j] == fd))
						continue;

					id = server_fd[j];
					memcpy(WFIFOP(id, 0), RFIFOP(fd, 0), RFIFOW(fd, 2));
					WFIFOW(id, 0) = 0x2c07;
					WFIFOSET(id, RFIFOW(fd, 2));
				}
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;
#endif

			case 0x2b08:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				curlin = RFIFOL(fd, 4);
				if (curlin < 0 || curlin >= MAX_PARTYS)
					break;

				init_party_data(curlin);
				party_dat[curlin].party_id = RFIFOL(fd, 4);
				party_dat[curlin].member_count = RFIFOB(fd, 8);
				party_dat[curlin].exp = RFIFOW(fd, 9);
				party_dat[curlin].item = RFIFOW(fd, 11);
				party_dat[curlin].leader_level = RFIFOW(fd, 13);
				strcpy(party_dat[curlin].party_name, RFIFOP(fd, 15));
				leader_id = -1;
				for (j = 0; j < party_dat[curlin].member_count; j++) {
					party_dat[curlin].member[j].mapno = RFIFOW(fd, 39 + j * 68);
					party_dat[curlin].member[j].leader = RFIFOB(fd, 39 + j * 68 + 2);
					party_dat[curlin].member[j].account_id = RFIFOL(fd, 39 + j * 68 + 3);
					party_dat[curlin].member[j].char_id = RFIFOL(fd, 39 + j * 68 + 7);
					party_dat[curlin].member[j].x = RFIFOW(fd, 39 + j * 68 + 11);
					party_dat[curlin].member[j].y = RFIFOW(fd, 39 + j * 68 + 13);
					party_dat[curlin].member[j].hp = RFIFOL(fd, 39 + j * 68 + 15);
					party_dat[curlin].member[j].max_hp = RFIFOL(fd, 39 + j * 68 + 19);
					party_dat[curlin].member[j].offline = RFIFOB(fd, 39 + j * 68 + 23);
					party_dat[curlin].member[j].fd = RFIFOL(fd, 39 + j * 68 + 24);
					strncpy(party_dat[curlin].member[j].map_name, RFIFOP(fd, 39 + j * 68 + 28), 16);
					strncpy(party_dat[curlin].member[j].nick, RFIFOP(fd, 39 + j * 68 + 44), 24);
					if (party_dat[curlin].member[j].leader == 0)
						leader_id = party_dat[curlin].member[j].char_id;

				}
				RFIFOSKIP(fd, RFIFOW(fd, 2));
				if (leader_id == -1) {
					party_dat[curlin].leader_level = 0;
					for (i = 0; i < char_num; i++) {
						if (char_dat[i].party_id == party_dat[curlin].party_id && char_dat[i].skill[0].lv >= 7) {
							for (j = 0; j < party_dat[curlin].member_count; j++) {
								if (party_dat[curlin].member[j].char_id == char_dat[i].char_id) {
									party_dat[curlin].member[j].leader = 0;
									char_dat[i].party_status = 2;
									party_dat[curlin].leader_level = char_dat[i].base_level;
									leader_id = char_dat[i].char_id;
									break;
								}
							}
						}
						if (leader_id != -1)
							break;

					}
					for (i = 0; i < MAX_SERVERS; i++) {
						if (server_fd[i] > 0) {
							WFIFOW(server_fd[i], 0) = 0x2b07;
							WFIFOL(server_fd[i], 2) = party_dat[curlin].party_id;
							WFIFOL(server_fd[i], 6) = leader_id;
							WFIFOW(server_fd[i], 10) = party_dat[curlin].leader_level;
							WFIFOSET(server_fd[i], 12);
						}
					}
					if (leader_id == -1) {
						for (i = 0; i < char_num; i++) {
							if (char_dat[i].party_id == party_dat[curlin].party_id) {
								char_dat[i].party_status = 0;
								char_dat[i].party_id = -1;
								strcpy(char_dat[i].party_name, "");
							}
						}
						init_party_data(curlin);
					}
					mmo_char_sync();
				}
				mmo_party_sync();
				break;

			case 0x2b04:
				if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd, 2))
					return 0;

				RFIFOSKIP(fd, RFIFOW(fd, 2));
				break;

			case 0x2b05:
				if (RFIFOREST(fd) < 6)
					return 0;

				init_party_data(RFIFOL(fd, 2));
				mmo_char_sync();
				mmo_party_sync();
				RFIFOSKIP(fd, 6);
				break;

			case 0x2b06:
				RFIFOSKIP(fd, 2);
				for (i = 0; i < MAX_PARTYS; i++) {
					if (party_dat[i].party_id >= 0) {
						WFIFOW(fd, 0) = 0x2dac;
						WFIFOL(fd, 4) = party_dat[i].party_id;
						WFIFOW(fd, 9) = party_dat[i].exp;
						WFIFOW(fd, 11) = party_dat[i].item;
						WFIFOW(fd, 13) = party_dat[i].leader_level;
						memcpy(WFIFOP(fd, 15), party_dat[i].party_name, 24);
						curlin = 0;
						for (j = 0; j < MAX_PARTY_MEMBERS; j++) {
							if (party_dat[i].member[j].account_id != 0) {
								WFIFOW(fd, 39 + j * 68) = party_dat[i].member[j].mapno;
								WFIFOB(fd, 39 + j * 68 + 2) = party_dat[i].member[j].leader;
								WFIFOL(fd, 39 + j * 68 + 3) = party_dat[i].member[j].account_id;
								WFIFOL(fd, 39 + j * 68 + 7) = party_dat[i].member[j].char_id;
								WFIFOW(fd, 39 + j * 68 + 11) = party_dat[i].member[j].x;
								WFIFOW(fd, 39 + j * 68 + 13) = party_dat[i].member[j].y;
								WFIFOL(fd, 39 + j * 68 + 15) = party_dat[i].member[j].hp;
								WFIFOL(fd, 39 + j * 68 + 19) = party_dat[i].member[j].max_hp;
								WFIFOB(fd, 39 + j * 68 + 23) = party_dat[i].member[j].offline;
								WFIFOL(fd, 39 + j * 68 + 24) = party_dat[i].member[j].fd;
								memcpy(WFIFOP(fd, 39 + j * 68 + 28), party_dat[i].member[j].map_name, 16);
								memcpy(WFIFOP(fd, 39 + j * 68 + 44), party_dat[i].member[j].nick, 24);
								curlin++;
							}
						}
						WFIFOB(fd, 8) = curlin;
						WFIFOW(fd, 2) = 39 + curlin * 68;
						WFIFOSET(fd, 39 + curlin * 68);
					}
				}
				break;

			case 0x3a44:
				if (RFIFOREST(fd) < 52)
					return 0;

				for (i = 0; i < pet_num; i++) {
					if (pet_dat[i].pet.pet_id == (signed)RFIFOL(fd, 2)
					&& pet_dat[i].pet.account_id == (signed)RFIFOL(fd, 32)
					&& pet_dat[i].pet.char_id == (signed)RFIFOL(fd, 36))
						break;
				}
				if (i != pet_num) {
					pet_dat[i].pet.pet_class = RFIFOW(fd, 6);
					memcpy(pet_dat[i].pet.pet_name, RFIFOP(fd, 8), 24);
					pet_dat[i].pet.account_id = RFIFOL(fd, 32);
					pet_dat[i].pet.char_id = RFIFOL(fd, 36);
					pet_dat[i].pet.pet_level = RFIFOB(fd, 40);
					pet_dat[i].pet.mons_id = RFIFOW(fd, 41);
					pet_dat[i].pet.pet_accessory = RFIFOW(fd, 43);
					pet_dat[i].pet.pet_friendly = RFIFOW(fd, 45);
					pet_dat[i].pet.pet_hungry = RFIFOW(fd, 47);
					pet_dat[i].pet.pet_name_flag = RFIFOW(fd, 49);
					pet_dat[i].pet.activity = RFIFOB(fd, 51);
				}
				else {
					if (pet_num >= pet_max) {
						pet_max += MAX_BUFFER;
						pet_dat = realloc(pet_dat, sizeof(pet_dat[0]) * pet_max);
					}
					pet_dat[i].pet.pet_id = pet_id_count++;
					pet_dat[i].pet.pet_class = RFIFOW(fd, 6);
					memcpy(pet_dat[i].pet.pet_name, RFIFOP(fd, 8), 24);
					pet_dat[i].pet.account_id = RFIFOL(fd, 32);
					pet_dat[i].pet.char_id = RFIFOL(fd, 36);
					pet_dat[i].pet.pet_level = RFIFOB(fd, 40);
					pet_dat[i].pet.mons_id = RFIFOW(fd, 41);
					pet_dat[i].pet.pet_accessory = RFIFOW(fd, 43);
					pet_dat[i].pet.pet_friendly = RFIFOW(fd, 45);
					pet_dat[i].pet.pet_hungry = RFIFOW(fd, 47);
					pet_dat[i].pet.pet_name_flag = RFIFOW(fd, 49);
					pet_dat[i].pet.activity = RFIFOB(fd, 51);
					pet_num++;
					for (j = 0; j < char_num; j++) {
						if (char_dat[j].account_id == pet_dat[i].pet.account_id
						&& char_dat[j].char_id == pet_dat[i].pet.char_id) {
							break;
						}
					}
					if (j != char_num) {
						char_dat[j].pet_id = pet_dat[i].pet.pet_id;
					}
				}
				RFIFOSKIP(fd, 52);
				mmo_char_sync();
				mmo_pet_sync();
				break;

			case 0x3b45:
				RFIFOSKIP(fd, 2);
				for (i = 0; i < pet_num; i++) {
					WFIFOW(fd, 0) = 0x3ff6;
					WFIFOL(fd, 2) = pet_dat[i].pet.pet_id;
					WFIFOW(fd, 6) = pet_dat[i].pet.pet_class;
					memcpy(WFIFOP(fd, 8), pet_dat[i].pet.pet_name, 24);
					WFIFOL(fd, 32) = pet_dat[i].pet.account_id;
					WFIFOL(fd, 36) = pet_dat[i].pet.char_id;
					WFIFOB(fd, 40) = pet_dat[i].pet.pet_level;
					WFIFOW(fd, 41) = pet_dat[i].pet.mons_id;
					WFIFOW(fd, 43) = pet_dat[i].pet.pet_accessory;
					WFIFOW(fd, 45) = pet_dat[i].pet.pet_friendly;
					WFIFOW(fd, 47) = pet_dat[i].pet.pet_hungry;
					WFIFOW(fd, 49) = pet_dat[i].pet.pet_name_flag;
					WFIFOB(fd, 51) = pet_dat[i].pet.activity;
					WFIFOSET(fd, 52);
				}
				break;

			case 0x3b46:
				if (RFIFOREST(fd) < 6)
					return 0;

				for (i = 0; i < char_num; i++) {
					if (char_dat[i].pet_id == (signed)RFIFOL(fd, 2))
						break;
				}
				for (j = 0; j < pet_num; j++) {
					if (pet_dat[j].pet.pet_id == char_dat[i].pet_id) {
						memcpy(&pet_dat[j], &char_dat[pet_num - 1], sizeof(pet_dat[0]));
						break;
					}
				}
				RFIFOSKIP(fd, 6);
				char_dat[i].pet_id = 0;
				pet_num--;
				mmo_char_sync();
				mmo_pet_sync();
				break;

			default:
				close(fd);
				session[fd]->eof = 1;
				return 0;
		}
	}
	return 0;
}

static void update_user_login(long int account_id, short int state_auth)
{
	WFIFOW(login_fd, 0) = 0x30a1;
	WFIFOL(login_fd, 2) = account_id;
	WFIFOB(login_fd, 6) = state_auth;
	WFIFOSET(login_fd, 7);
}

static int search_mapserver(char *map)
{
	int i, j;

	for (i = 0; i < MAX_SERVERS; i++) {
		if (server_fd[i] < 0)
			continue;

		for (j = 0; server[i].map[j][0]; j++) {
			if (strncmp(server[i].map[j], map, 16) == 0)
				return i;
		}
	}
	return -1;
}

int parse_char(unsigned int fd)
{
	int i, j, a, ch, ret;
	struct char_session_data *sd = NULL, *osd = NULL;

	if (!session[login_fd])
		session[fd]->eof = 1;

	if (session[fd]->eof) {
		if (fd == login_fd)
			login_fd = -1;

		close(fd);
		delete_session(fd);
		return 0;
	}
	sd = session[fd]->session_data;
	while (RFIFOREST(fd) >= 2) {
		switch (RFIFOW(fd, 0)) {
			case 0x65:
				if (RFIFOREST(fd) < 17)
					return 0;

				if (!sd) {
					sd = session[fd]->session_data = calloc(sizeof(*sd), 1);
					memset(sd, 0, sizeof(*sd));
				}
				sd->account_id = RFIFOL(fd, 2);
				sd->login_id1 = RFIFOL(fd, 6);
				sd->login_id2 = RFIFOL(fd, 10);
				sd->sex = RFIFOB(fd, 16);
				sd->state = CHAR_STATE_WAITAUTH;
				sd->network = NO_INFO;
				strcpy(sd->email, "sys@srv.com");
				update_user_login(RFIFOL(fd, 2), 0);
				WFIFOL(fd, 0) = RFIFOL(fd, 2);
				WFIFOSET(fd, 4);

				for (i = 0; i < AUTH_FIFO_SIZE; i++) {
					if ((auth_fifo[i].account_id == sd->account_id)
					&& (auth_fifo[i].login_id1 == sd->login_id1)
					&& (auth_fifo[i].delflag == 2)) {
						auth_fifo[i].delflag = 1;
						break;
					}
				}
				if (i == AUTH_FIFO_SIZE) {
					WFIFOW(login_fd, 0) = 0x2712;
					WFIFOL(login_fd, 2) = sd->account_id;
					WFIFOL(login_fd, 6) = sd->login_id1;
					WFIFOL(login_fd, 10) = sd->login_id2;
					WFIFOSET(login_fd, 15);
				}
				else {
					sd->network = auth_fifo[i].network;
					mmo_char_send006b(fd, sd);
				}
				RFIFOSKIP(fd, 17);
				break;

			case 0x66:
				if (RFIFOREST(fd) < 3)
					return 0;

				for (ch = 0; ch < MAX_CHAR_SLOTS; ch++) {
					if ((sd->found_char[ch] >= 0) && (char_dat[sd->found_char[ch]].char_num == RFIFOB(fd, 2)))
						break;
				}
				if (ch != MAX_CHAR_SLOTS) {
					WFIFOW(fd, 0) = 0x71;
					WFIFOL(fd, 2) = char_dat[sd->found_char[ch]].char_id;
					if (char_dat[sd->found_char[ch]].hp <= 0) {
						memcpy(WFIFOP(fd,6), char_dat[sd->found_char[ch]].save_point.map, 16);
						i = search_mapserver(char_dat[sd->found_char[ch]].save_point.map);
					}
					else {
						memcpy(WFIFOP(fd,6), char_dat[sd->found_char[ch]].last_point.map, 16);
						i = search_mapserver(char_dat[sd->found_char[ch]].last_point.map);
					}
					if (i < 0)
						i = 0;

					else {
    						if (sd->network == WWW_CON) {
							memcpy(WFIFOP(fd, 6), char_dat[sd->found_char[ch]].last_point.map, 16);
							WFIFOL(fd, 22) = server[i].xip;
							WFIFOW(fd, 26) = server[i].port;
						}
						else {
							memcpy(WFIFOP(fd, 6), char_dat[sd->found_char[ch]].last_point.map, 16);
							WFIFOL(fd, 22) = server[i].ip;
							WFIFOW(fd, 26) = server[i].port;
						}
					}
					WFIFOSET(fd, 28);
					if (auth_fifo_pos >= AUTH_FIFO_SIZE)
						auth_fifo_pos = 0;

					auth_fifo[auth_fifo_pos].account_id = sd->account_id;
					auth_fifo[auth_fifo_pos].char_id = char_dat[sd->found_char[ch]].char_id;
					auth_fifo[auth_fifo_pos].login_id1 = sd->login_id1;
					auth_fifo[auth_fifo_pos].delflag = 0;
					auth_fifo[auth_fifo_pos].char_pos = sd->found_char[ch];
					auth_fifo_pos++;
				}
				RFIFOSKIP(fd, 3);
				break;

			case 0x67:
				if (RFIFOREST(fd) < 37)
					return 0;

				i = make_new_char(fd, RFIFOP(fd, 2));
				if (i < 0) {
					WFIFOW(fd, 0) = 0x6e;
					WFIFOB(fd, 2) = 0x00;
					WFIFOSET(fd, 3);
					RFIFOSKIP(fd, 37);
					break;
				}
				WFIFOW(fd, 0) = 0x6d;
				memset(WFIFOP(fd, 2), 0x00, 106);

				WFIFOL(fd, 2) = char_dat[i].char_id;
				WFIFOL(fd, 2 + 4) = char_dat[i].base_exp;
				WFIFOL(fd, 2 + 8) = char_dat[i].zeny;
				WFIFOL(fd, 2 + 12) = char_dat[i].job_exp;
				WFIFOL(fd, 2 + 16) = char_dat[i].job_level;
				WFIFOL(fd, 2 + 28) = char_dat[i].karma;
				WFIFOL(fd, 2 + 32) = char_dat[i].manner;
				WFIFOW(fd, 2 + 40) = 0x30;
				WFIFOW(fd, 2 + 42) = (char_dat[i].hp > 0x7fff)? 0x7fff:char_dat[i].hp;
				WFIFOW(fd, 2 + 44) = (char_dat[i].max_hp > 0x7fff)? 0x7fff:char_dat[i].max_hp;
				WFIFOW(fd, 2 + 46) = (char_dat[i].sp > 0x7fff)? 0x7fff:char_dat[i].sp;
				WFIFOW(fd, 2 + 48) = (char_dat[i].max_sp > 0x7fff)? 0x7fff:char_dat[i].max_sp;
				WFIFOW(fd, 2 + 50) = DEFAULT_WALK_SPEED;
				WFIFOW(fd, 2 + 52) = char_dat[i].class;
				WFIFOW(fd, 2 + 54) = char_dat[i].hair;
				WFIFOW(fd, 2 + 58) = char_dat[i].base_level;
				WFIFOW(fd, 2 + 60) = char_dat[i].skill_point;
				WFIFOW(fd, 2 + 62) = char_dat[i].head_bottom;
				WFIFOW(fd, 2 + 64) = char_dat[i].shield;
				WFIFOW(fd, 2 + 66) = char_dat[i].head_top;
				WFIFOW(fd, 2 + 68) = char_dat[i].head_mid;
				WFIFOW(fd, 2 + 70) = char_dat[i].hair_color;

				memcpy(WFIFOP(fd, 2 + 74), char_dat[i].name, 24);

				WFIFOB(fd, 2 + 98) = (char_dat[i].str > 255)? 255:char_dat[i].str;
				WFIFOB(fd, 2 + 99) = (char_dat[i].agi > 255)? 255:char_dat[i].agi;
				WFIFOB(fd, 2 + 100) = (char_dat[i].vit > 255)? 255:char_dat[i].vit;
				WFIFOB(fd, 2 + 101) = (char_dat[i].int_ > 255)? 255:char_dat[i].int_;
				WFIFOB(fd, 2 + 102) = (char_dat[i].dex > 255)? 255:char_dat[i].dex;
				WFIFOB(fd, 2 + 103) = (char_dat[i].luk > 255)? 255:char_dat[i].luk;
				WFIFOB(fd, 2 + 104) = char_dat[i].char_num;

				WFIFOSET(fd, 108);
				RFIFOSKIP(fd, 37);
				for (ch = 0; ch < MAX_CHAR_SLOTS; ch++) {
					if (sd->found_char[ch] == -1) {
						sd->found_char[ch] = i;
						break;
					}
				}
				break;

			case 0x68:
				if (RFIFOREST(fd) < 46)
					return 0;

				ret = 0;
				memcpy(email, RFIFOP(fd, 6), 60);
				if ((strlen(email) < 3) || (strlen(email) > 60) || (strchr(email, '@') == NULL) || (email[0] == '@') || (email[strlen(email)-1] == '@'))
					strcpy(email, "sys@srv.com");

				else
					strtolower(email);

				if (strncmp(email, sd->email, 60) != 0) {
					WFIFOW(fd, 0) = 0x70;
					WFIFOB(fd, 2) = 0;
					WFIFOSET(fd, 3);
				}
				else {
					int account_id = 0;
					int char_id = 0;
					int pet_id = -1;
					int party_id = -1;
					int guild_id = -1;
					for (i = 0; i < MAX_CHAR_SLOTS; i++) {
						if ((sd->found_char[i] >= 0) && (char_dat[sd->found_char[i]].char_id == (signed)RFIFOL(fd, 2))) {
							account_id = char_dat[sd->found_char[i]].account_id;
							char_id = char_dat[sd->found_char[i]].char_id;
							party_id = char_dat[sd->found_char[i]].party_id;
							pet_id = char_dat[sd->found_char[i]].pet_id;
							if (sd->found_char[i] != (char_num - 1)) {
								memcpy(&char_dat[sd->found_char[i]], &char_dat[char_num - 1], sizeof(char_dat[0]));
								if (sd->found_char[0] == char_num - 1)
									sd->found_char[0] = sd->found_char[i];

								else if (sd->found_char[1] == char_num - 1)
									sd->found_char[1] = sd->found_char[i];

								else if (sd->found_char[2] == char_num - 1)
									sd->found_char[2] = sd->found_char[i];

								else {
									for (j = 5; j < FD_SETSIZE; j++) {
										if ((session[j]) && (osd = session[j]->session_data)) {
											if (osd->found_char[0] == char_num - 1)
												osd->found_char[0] = sd->found_char[i];

											else if (osd->found_char[1] == char_num - 1)
												osd->found_char[1] = sd->found_char[i];

											else if (osd->found_char[2] == char_num - 1)
												osd->found_char[2] = sd->found_char[i];

										}
									}
								}
							}
							if (pet_id > 0) {
								for (a = 0; a < pet_num; a++) {
									if (pet_dat[a].pet.pet_id == pet_id)
										memcpy(&pet_dat[a], &char_dat[pet_num - 1], sizeof(pet_dat[0]));

									break;
								}
								mmo_pet_sync();
								pet_num--;
							}
							char_num--;
							if (i == 0) {
								sd->found_char[0] = sd->found_char[1];
								sd->found_char[1] = sd->found_char[2];
							}
							else if (i == 1)
								sd->found_char[1] = sd->found_char[2];

							sd->found_char[2] = -1;
							ret = 1;
							mmo_char_sync();
							mmo_storage_sync();
							break;
						}
					}
					if (ret) {
						WFIFOW(fd, 0) = 0x6f;
						WFIFOSET(fd, 2);
						if (account_id != 0 && char_id != 0 && party_id != 0) {
							for (i = 0; i < MAX_SERVERS; i++) {
								if (server_fd[i] > 0) {
									WFIFOW(server_fd[i], 0) = 0x2dab;
									WFIFOL(server_fd[i], 2) = account_id;
									WFIFOL(server_fd[i], 6) = char_id;
									WFIFOL(server_fd[i], 10) = party_id;
									WFIFOL(server_fd[i], 14) = guild_id;
									WFIFOSET(server_fd[i], 18);
								}
							}
						}
					}
					else {
						WFIFOW(fd, 0) = 0x70;
						WFIFOB(fd, 2) = 1;
						WFIFOSET(fd, 3);
					}
				}
				RFIFOSKIP(fd, 46);
				break;

			case 0x2af8:
				if (RFIFOREST(fd) < 64)
					return 0;

				WFIFOW(fd, 0) = 0x2af9;
				for (i = 0; i < MAX_SERVERS; i++) {
					if (server_fd[i] < 0)
						break;
				}
				if (i == MAX_SERVERS || (strncmp(RFIFOP(fd, 2), userid, 24)) || (strncmp(RFIFOP(fd, 26), passwd, 24))) {
					WFIFOB(fd, 2) = 3;
					WFIFOSET(fd, 3);
					RFIFOSKIP(fd, 60);
				}
				else {
					memset(server[i].map, 0, sizeof(server[i].map));
					server[i].ip = RFIFOL(fd, 54);
					server[i].port = RFIFOW(fd, 58);
					server[i].xip = RFIFOL(fd, 60);
					server[i].users = 0;
					server_fd[i] = fd;
					WFIFOB(fd, 2) = 0;
					WFIFOSET(fd, 3);
					session[fd]->func_parse = parse_frommap;
					realloc_fifo(fd, FIFO_SIZE, FIFO_SIZE);
				}
				RFIFOSKIP(fd, 64);
				return 0;

			case 0x187:
				if (RFIFOREST(fd) < 6)
					return 0;

				RFIFOSKIP(fd, 6);
				break;

			default:
				close(fd);
				session[fd]->eof = 1;
				return 0;
		}
	}
	RFIFOFLUSH(fd);
	return 0;
}

/*======================================
 *	CHARACTER: Set Timers
 *--------------------------------------
 */

int check_connect_login_server(int tid, unsigned int tick, int id, int data)
{
	id = 0;
	tid = 0;
	tick = 0;
	data = 0;
	if (!session[login_fd]) {
		login_fd = make_connection(inet_addr(login_ip_str), login_port);
		session[login_fd]->func_parse = parse_tologin;
		realloc_fifo(login_fd, FIFO_SIZE, FIFO_SIZE);
		WFIFOW(login_fd, 0) = 0x2710;
		memcpy(WFIFOP(login_fd, 2), userid, 24);
		memcpy(WFIFOP(login_fd, 26), passwd, 24);
		WFIFOL(login_fd, 54) = char_ip;
		WFIFOW(login_fd, 58) = char_port;
		memcpy(WFIFOP(login_fd, 60), server_name, 16);
		WFIFOL(login_fd,76) = char_xip;
		WFIFOW(login_fd,82) = maintenance;
		WFIFOW(login_fd,84) = new;
		WFIFOW(login_fd, 86) = ver_1;
		WFIFOB(login_fd, 88) = ver_2;
		WFIFOSET(login_fd, 90);
	}
	return 0;
}

int send_users_tologin(int tid, unsigned int tick, int id, int data)
{
	int i, fd, users = 0;
	id = 0;
	tid = 0;
	tick = 0;
	data = 0;
	for (i = 0; i < MAX_SERVERS; i++) {
		if (server_fd[i] > 0 && session[server_fd[i]])
			users += server[i].users;
	}
	if (session[login_fd]) {
		WFIFOW(login_fd, 0) = 0x2714;
		WFIFOL(login_fd, 2) = users;
		WFIFOSET(login_fd, 6);
	}
	for (i = 0; i < MAX_SERVERS; i++) {
		if (server_fd[i] > 0 && session[server_fd[i]]) {
			fd = server_fd[i];
			WFIFOW(fd, 0) = 0x2b00;
			WFIFOL(fd, 2) = users;
			WFIFOSET(fd, 6);
		}
	}
	return 0;
}

int make_backup_timer(int tid, unsigned int tick, int id, int data)
{
	id = 0;
	tid = 0;
	tick = 0;
	data = 0;
	make_backup();
	return 0;
}

/*======================================
 *	CHARACTER: Program Initialization
 *--------------------------------------
 */

int do_init(void)
{
	int i, j[10];
	char line[MAX_LINE], option1[MAX_LINE], option2[MAX_LINE];
	struct timeval tv;
	char tmpstr[256];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	struct hostent *h;
	FILE *fp = NULL;

	printf("\033[1;41m.----------------------------------------.\n");
	printf("\033[1;41m|             Odin Project(c)            |\n");
	printf("\033[1;41m|                                        |\n");
	printf("\033[1;41m|        ___    __     ___   __  _       |\n");
	printf("\033[1;41m|       / _ \\  (   \\  (   ) (  )( )      |\n");
	printf("\033[1;41m|      | ( ) | ( |) |  | |   | \\| |      |\n");
	printf("\033[1;41m|       \\___/  (__ /  (___)  |_|\\_|      |\n");
	printf("\033[1;41m|                                        |\n");
	printf("\033[1;41m|                                        |\n");
	printf("\033[1;41m|      %s    |\n", logN);
	printf("\033[1;41m`========================================'\033[0m\n");
	printf("\n**Server Started**\n");

	if (!(fp = fopen("config.ini", "r"))) {
		fprintf(stderr, "ERROR: %s : Access Violation\n", tmpstr);
		sleep(2);
		abort();
	}
	else {
		while (fgets(line, 1023, fp)) {
			i = sscanf(line, "%[^=]=%s", option1, option2);
			if (i != 2) {
				continue;
			}
			if (strcmp(option1, "ServerUsername") == 0) {
				j[0] = 1;
				memcpy(userid, option2, 24);
			}
			else if (strcmp(option1, "ServerPassword") == 0) {
				j[1] = 1;
				memcpy(passwd, option2, 24);
			}
			else if (strcmp(option1, "ServerName") == 0) {
				j[2] = 1;
				sscanf(line, "%[^=]=%[^*]", option1, option2);
				memcpy(server_name, option2, 16);
			}
			else if (strcmp(option1, "AccountIP") == 0) {
				memcpy(login_ip_str, option2, 16);
			}
			else if (strcmp(option1, "AccountPort") == 0) {
				login_port = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "CharacterIP") == 0) {
				memcpy(char_ip_str, option2, 16);
			}
			else if (strcmp(option1, "CharacterPort") == 0) {
				char_port = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "InetIP") == 0) {
				memcpy(char_xip_str, option2, 256);
			}
			else if (strcmp(option1, "Maintenance") == 0) {
				j[3] = 1;
				maintenance = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "AddNew") == 0) {
				j[4] = 1;
				new = (int)strtol(option2, (char**)NULL, 10);
			}
			else if (strcmp(option1, "Backup") == 0) {
				j[5] = 1;
				if ((int)strtol(option2, (char**)NULL, 10) == 1) {
					do_backup = 1;
				}
				else {
					do_backup = 0;
				}
			}
			else if (strcmp(option1, "BackupTime") == 0) {
				j[6] = 1;
				do_timer_bak = (int)strtol(option2, (char**)NULL, 10);
				if ((do_backup == 1) && (do_timer_bak > 120 || do_timer_bak <= 0)) {
					do_timer_bak = 30;
				}
			}
			else if (strcmp(option1, "StartMap") == 0) {
				j[7] = 1;
				char map[32];
				int x, y;
				if (sscanf(option2,"%[^,],%d,%d",map, &x, &y) < 3 ) {
					continue;
				}
				memcpy(start_point.map, map, 16);
				start_point.x = x;
				start_point.y = y;
			}
			else if (strcmp(option1, "StartZeny") == 0) {
				j[8] = 1;
				start_zeny = (int)strtol(option2, (char**)NULL, 10);
				if(start_zeny < 0) {
					start_zeny = 0;
				}
			}
		}
	}
	fclose(fp);
	if ((j[0] != 1) || (j[1] != 1) || (j[2] != 1) || (j[3] != 1) || (j[4] != 1) || (j[5] != 1)
	|| (j[6] != 1) || (j[7] != 1) || (j[8] != 1))
		exit(0);

	char_ip = inet_addr(char_ip_str);
	char_xip =inet_addr(char_xip_str);
	h = gethostbyname(char_xip_str);
	if (h)
		char_xip = ((unsigned char)h->h_addr[3] << 24) + ((unsigned char)h->h_addr[2] << 16) + ((unsigned char)h->h_addr[1] << 8) + ((unsigned char)h->h_addr[0]);

	if (char_xip == -1)
		char_xip = char_ip;

	for (i = 0; i < MAX_SERVERS; i++) {
		server_fd[i] = -1;
	}
	mmo_char_init();
	mmo_storage_init();
	mmo_pet_init();
	mmo_party_init();
	set_defaultparse(parse_char);
	char_port_fd = make_listen_port(char_port);

	check_connect_timer = add_timer(gettick() + 10, check_connect_login_server, 0, 0);
	timer_data[check_connect_timer]->type = TIMER_INTERVAL;
	timer_data[check_connect_timer]->interval = 10000;

	send_users_tologin_timer = add_timer(gettick() + 10, send_users_tologin, 0, 0);
	timer_data[send_users_tologin_timer]->type = TIMER_INTERVAL;
	timer_data[send_users_tologin_timer]->interval = 5000;

	if (do_backup) {
		check_backup_timer = add_timer(gettick() + do_timer_bak * 60000, make_backup_timer, 0, 0);
		timer_data[check_backup_timer]->type = TIMER_INTERVAL;
		timer_data[check_backup_timer]->interval = do_timer_bak * 60000;
	}
	printf("\033[1;41m\nServer is online.\033[0m\n");
	return 0;
}

int do_exit()
{
	if (check_backup_timer > 0)
		delete_timer(check_backup_timer, make_backup_timer);

	if (char_dat)
		free(char_dat);

	delete_timer(send_users_tologin_timer, send_users_tologin);
	delete_timer(check_connect_timer, check_connect_login_server);
	delete_session(char_port_fd);
	return 0;
}
