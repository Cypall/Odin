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
 Module:        Version 1.7.0 SP3 - Angel Ex
 Author:        Odin Developer Team Copyrights (c) 2004
 Project:       Project Odin Zone Server
 Creation Date: Dicember 6, 2003
 Modified Date: October 24, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "npc.h"
#include "script.h"
#include "itemdb.h"
#include "save.h"
#include "item.h"
#include "equip.h"
#include "skill_db.h"
#include "storage.h"

#define SCRIPT_BLOCK_SIZE 256

static unsigned char * script_buf;
static int script_pos, script_size;

int menu_goto[32];
int str_pos, str_size;
int alias_ext_max = 0;
int str_num = 1, str_data_size;
int unget_com_data = -1;
int str_hash[16];
char *str_buf;

enum {
	C_NOP, C_POS, C_INT, C_LOCAL, C_GLOBAL, C_PARAM, C_TEMPORAL,

	C_LOR, C_LAND, C_LE, C_LT, C_GE, C_GT, C_EQ, C_NE,
	C_XOR, C_OR, C_AND, C_ADD, C_SUB, C_MUL, C_DIV, C_MOD, C_NEG, C_LNOT, C_NOT,
	C_COUNTITEM, C_READPARAM, C_REFINELVL, C_CHECKEQUIP,

	C_MES, C_NEXT, C_CLOSE, C_MENU, C_MENU_GOTO, C_GOTO, C_JOBCHANGE, C_INPUT, C_IF,
	C_SET, C_SETLOOK, C_SETZENY, C_WARP, C_GETITEM, C_DELITEM, C_CUTIN, C_VIEWPOINT,
	C_MONSTER, C_SAVE, C_STORAGE, C_GIVESPECIAL, C_FULLHEAL, C_RANDOM, C_RESETSKILL,
	C_RESETSTATUS, C_RELEASE, C_RELEASEALL, C_REFINE, C_FAILURE, C_HEALHP, C_HEALSP,
	C_EMOTION
};

struct {
	unsigned char *str;
	int com;
	unsigned char *arg;
} com_tbl[] = {
	{"mes", C_MES, "s"},
	{"next", C_NEXT, ""},
	{"close", C_CLOSE, ""},
	{"menu", C_MENU, "m"},
	{"goto", C_GOTO, "l"},
	{"jobchange", C_JOBCHANGE, "i"},
	{"input", C_INPUT, ""},
	{"warp", C_WARP, "sii"},
	{"set", C_SET, "ie"},
	{"setlook", C_SETLOOK, "ii"},
	{"setzeny", C_SETZENY, "Ie"},
	{"if", C_IF, "eg"},
	{"getitem", C_GETITEM, "Ie"},
	{"delitem", C_DELITEM, "Ie"},
	{"cutin", C_CUTIN, "si"},
	{"viewpoint", C_VIEWPOINT, "iiiii"},
	{"monster", C_MONSTER, "siiii"},
	{"save", C_SAVE, "sii"},
	{"open_storage", C_STORAGE, ""},
	{"give_special", C_GIVESPECIAL, "i"},
	{"full_heal", C_FULLHEAL, ""},
	{"random", C_RANDOM, "Ii"},
	{"release", C_RELEASE, "i"},
	{"release_all", C_RELEASEALL, ""},
	{"resetskill", C_RESETSKILL, ""},
	{"resetstatus", C_RESETSTATUS, ""},
	{"refine", C_REFINE, "i"},
	{"failure", C_FAILURE, "i"},
	{"healhp", C_HEALHP, "i"},
	{"healsp", C_HEALSP, "i"},
	{"emotion", C_EMOTION, "i"},
	{NULL, 0, NULL}
};

struct {
	unsigned char *str;
	int type;
	int val;
} alias_tbl[] = {
	{"Job_Novice", C_INT, 0},
	{"Job_Swordman", C_INT, 1},
	{"Job_Mage", C_INT, 2},
	{"Job_Archer", C_INT, 3},
	{"Job_Acolyte", C_INT, 4},
	{"Job_Merchant", C_INT, 5},
	{"Job_Thief", C_INT, 6},
	{"Job_Knight", C_INT, 7},
	{"Job_Priest", C_INT, 8},
	{"Job_Wizard", C_INT, 9},
	{"Job_Blacksmith", C_INT, 10},
	{"Job_Hunter", C_INT, 11},
	{"Job_Assassin", C_INT, 12},
	{"Job_Crusader", C_INT, 14},
	{"Job_Monk", C_INT, 15},
	{"Job_Sage", C_INT, 16},
	{"Job_Alchemist", C_INT, 17},
	{"Job_Bard", C_INT, 18},
	{"Job_Dancer", C_INT, 19},
	{"Job_Rogue", C_INT, 20},
	{"Job_Kami", C_INT, 100},
	{"S_FLAG", C_LOCAL, 0},
	{"Script_flag_num", C_PARAM, SP_SCRIPT_FLAG},
	{"SkillPoint", C_PARAM, SP_SKILLPOINT},
	{"StatusPoint", C_PARAM, SP_STATUSPOINT},
	{"Zeny", C_PARAM, SP_ZENY},
	{"BaseLevel", C_PARAM, SP_BASELEVEL},
	{"JobLevel", C_PARAM, SP_JOBLEVEL},
	{"GetJob", C_PARAM, SP_GETJOB},
	{"CheckWeight", C_PARAM, SP_WEIGHT},
	{"CheckMaxWeight", C_PARAM, SP_MAXWEIGHT},
	{"CheckCart", C_PARAM, SP_CHECKCART},
	{"CheckStorage", C_PARAM, SP_CHECKSTORAGE},
	{"CheckFalcon", C_PARAM, SP_CHECKFALCON},
	{"CheckPecoPeco", C_PARAM, SP_CHECKPECOPECO},
	{"Account_ID", C_PARAM, SP_ACCOUNTID},
	{"sex", C_PARAM, SP_SEX},
	{"HasCart", C_PARAM, SP_HASCART},
	{"getwlvl", C_PARAM, SP_GETWLVL},
	{"randRef", C_PARAM, SP_RANDOM},
	{NULL, 0, 0}
}, *alias_ext_tbl = NULL;

struct temporal_reg {
	int tbl_val;
	int value;
};

struct temporal_reg *temporal_r(int char_id) {
	struct charlist {
		int char_id;
		int temporal_reg_max;
		struct temporal_reg *temporal_reg;
		struct charlist *next;
	};
	static struct charlist *charlist = NULL;
	int i;
	struct charlist *cl = charlist;
	struct charlist **cm = NULL;
	while (cl) {
		if (cl->char_id == char_id) {
			for (i = 0; i < cl->temporal_reg_max - 1; i++) {
				if (cl->temporal_reg[i].tbl_val != -1)
					return cl->temporal_reg;
			}
			cl->temporal_reg_max += 16;
			cl->temporal_reg = realloc(cl->temporal_reg, sizeof(struct temporal_reg) * cl->temporal_reg_max);
			for(i = cl->temporal_reg_max - 16; i < cl->temporal_reg_max; i++) {
				cl->temporal_reg[i].tbl_val = -1;
			}
			return cl->temporal_reg;
		}
		cm = &cl->next;
		cl = cl->next;
	}
	if (!charlist)
		cl = charlist = malloc(sizeof(struct charlist));

	else
		*cm = cl = malloc(sizeof(struct charlist));

	cl->char_id = char_id;
	cl->temporal_reg_max = 16;
	cl->temporal_reg = malloc(sizeof(struct temporal_reg) * 16);
	for (i = 0; i < 16; i++) {
		cl->temporal_reg[i].tbl_val = -1;
	}
	cl->next = NULL;
	return cl->temporal_reg;
}

static struct {
	int str;
	int backpatch;
	int label;
	int next;
} *str_data;

int calc_hash(unsigned char *p)
{
	int h = 0;

	while (*p) {
		h = (h << 1) + (h >> 3) + (h >> 5) + (h >> 8);
		h += *p++;
	}
	return h & 15;
}

int search_str(unsigned char *p)
{
	int i = str_hash[calc_hash(p)];

	while (i) {
		if (strcmp(str_buf + str_data[i].str, p) == 0)
			return i;

		i = str_data[i].next;
	}
	return -1;
}

int add_str(unsigned char *p)
{
	int i = calc_hash(p);
	if (str_hash[i] == 0)
		str_hash[i] = str_num;

	else {
		i = str_hash[i];
		for (;;) {
			if (strcmp(str_buf + str_data[i].str, p) == 0)
				return i;

			if (str_data[i].next == 0)
				break;

			i = str_data[i].next;
		}
		str_data[i].next = str_num;
	}
	if (str_num >= str_data_size) {
		str_data_size += 128;
		str_data = realloc(str_data, sizeof(str_data[0]) * str_data_size);
	}
	while(str_pos + strlen(p) + 1 >= (unsigned)str_size) {
		str_size += 256;
		str_buf = realloc(str_buf, str_size);
	}
	strcpy(str_buf + str_pos, p);
	str_data[str_num].str = str_pos;
	str_data[str_num].next = 0;
	str_data[str_num].backpatch = -1;
	str_data[str_num].label = -1;
	str_pos += strlen(p) + 1;
	return str_num++;
}

void check_script_buf(int size)
{
	if (script_pos + size >= script_size) {
		script_size += SCRIPT_BLOCK_SIZE;
		script_buf = realloc(script_buf, script_size);
		if (script_buf == NULL) {
			printf("Error on script buffer.\n");
			sleep(2);
			exit(1);
		}
	}
}

void add_scriptb(int a)
{
	check_script_buf(1);
	script_buf[script_pos++] = a;
}

void add_scriptc(int a)
{
	while (a >= 0x40) {
		add_scriptb((a & 0x3f) | 0x40);
		a = (a - 0x40) >> 6;
	}
	add_scriptb(a&0x3f);
}

void add_scripti(int a)
{
	while (a >= 0x40) {
		add_scriptb(a | 0xc0);
		a = (a - 0x40) >> 6;
	}
	add_scriptb(a|0x80);
}

void add_scriptl(int l)
{
	check_script_buf(5);
	if (str_data[l].label < 0) {
		script_buf[script_pos++] = C_POS;
		script_buf[script_pos++] = str_data[l].backpatch;
		script_buf[script_pos++] = str_data[l].backpatch >> 8;
		script_buf[script_pos++] = str_data[l].backpatch >> 16;
		script_buf[script_pos++] = str_data[l].backpatch >> 24;
		str_data[l].backpatch = script_pos - 4;
	}
	else {
		script_buf[script_pos++] = C_POS;
		script_buf[script_pos++] = str_data[l].label;
		script_buf[script_pos++] = str_data[l].label >> 8;
		script_buf[script_pos++] = str_data[l].label >> 16;
		script_buf[script_pos++] = str_data[l].label >> 24;
	}
}

void set_label(int l, int pos)
{
	int i, n;

	str_data[l].label = pos;
	for (i = str_data[l].backpatch; i >= 0;) {
		n = *(int*)(script_buf + i);
		script_buf[i] = pos;
		script_buf[i + 1] = pos >> 8;
		script_buf[i + 2] = pos >> 16;
		script_buf[i + 3] = pos >> 24;
		i = n;
	}
}

unsigned char *skip_space(unsigned char *p)
{
	while (1) {
		while (isspace(*p)) {
			p++;
		}
		if (p[0] == '/' && p[1] == '/') {
			while (*p && *p != '\n') {
				p++;
			}
		}
		else
			break;
	}
	return p;
}

unsigned char *skip_word(unsigned char *p)
{
	while (isalnum(*p) || *p == '_' || *p >= 0x81)
	if (*p >= 0x81 && p[1])
		p += 2;

	else
		p++;

	return p;
}

int comcmp(unsigned char *str, unsigned char *com)
{
	while(*com && tolower(*str) == tolower(*com)) {
		str++;
		com++;
	}
	return isalnum(*str)|| *str == '_' || *com;
}

unsigned char *parse_int(char *p_fix)
{
	int i;
	unsigned char *p;

	p = p_fix;
	p = skip_space(p);
	if (isdigit(*p) || *p == '-') {
		i = strtoul(p, &p_fix, 0);
		p = p_fix;
		add_scripti(i);
	}
	else if ((*p == 'g' || *p == 'l') && isdigit(p[1])) {
		add_scriptc(*p == 'g' ? C_GLOBAL : C_LOCAL);
		i = strtoul(p + 1, &p_fix, 0);
		p = p_fix;
		add_scripti(i);
	}
	else {
		for (i = 0; alias_tbl[i].str; i++) {
			if (comcmp(p, alias_tbl[i].str) == 0)
				break;
		}
		if (alias_tbl[i].str == NULL) {
			if (alias_ext_tbl == NULL) {
				alias_ext_tbl = malloc(sizeof(alias_ext_tbl[0]) * 16);
				memset(alias_ext_tbl, 0, sizeof(alias_ext_tbl[0]) * 16);
				alias_ext_max = 16;
			}
			for (i = 0; i < alias_ext_max; i++) {
				if (alias_ext_tbl[i].str == NULL)
					break;

				if (comcmp(p, alias_ext_tbl[i].str) == 0)
					break;
			}
			if (i >= alias_ext_max) {
				alias_ext_max += 16;
				alias_ext_tbl = realloc(alias_ext_tbl, sizeof(alias_ext_tbl[0]) * alias_ext_max);
				memset(alias_ext_tbl + alias_ext_max - 16, 0, sizeof(alias_ext_tbl[0]) * 16);
			}
			if (alias_ext_tbl[i].str == NULL) {
				unsigned char *q;
				q = skip_word(p + (*p == '@' ? 1 : 0));
				if (q - p + 1 > 32) {
					printf("global valiable name length is too long (%d >= 32!) %s\n", q - p, p);
					sleep(2);
					exit(1);
				}
				alias_ext_tbl[i].str = malloc(sizeof(char) * (q - p + 1));
				strncpy(alias_ext_tbl[i].str, p, q- p);
				alias_ext_tbl[i].str[q - p] = '\0';
				alias_ext_tbl[i].type = (*p == '@' ? C_TEMPORAL : C_GLOBAL);
				alias_ext_tbl[i].val = i;
			}
			add_scriptc(alias_ext_tbl[i].type);
			add_scripti(alias_ext_tbl[i].val);
			return skip_word(p + (*p == '@' ? 1 : 0));
		}
		if (alias_tbl[i].type != C_INT)
			add_scriptc(alias_tbl[i].type);

		add_scripti(alias_tbl[i].val);
		p = skip_word(p);
	}
	return p;
}

unsigned char *parse_simpleexpr(unsigned char *p)
{
	int func;

	if (*p == '(') {
		p = parse_subexpr(p + 1, -1);
		p = skip_space(p);
		if (*p++ != ')') {
			printf("unmatch ')' : %s\n", p);
			exit(1);
		}
	}
	else if ((func = C_COUNTITEM, comcmp(p, "countitem") == 0) ||
	           (func = C_REFINELVL, comcmp(p, "refinelvl") == 0) ||
	           (func = C_CHECKEQUIP, comcmp(p, "checkequip") == 0) ||
	           (func = C_READPARAM, comcmp(p, "readparam") == 0)) {
		p = skip_word(p);
		p = skip_space(p);
		if (*p != '(') {
			printf("func reqest '(' ')' : %s\n", p);
			exit(1);
		}
		p = parse_subexpr(p + 1, -1);
		p = skip_space(p);
		if (*p++ != ')') {
			printf("func reqest '(' ')' : %s\n", p - 1);
			exit(1);
		}
		add_scriptc(func);
	}
	else
		p = parse_int(p);

	return p;
}

unsigned char *parse_subexpr(unsigned char *p, int limit)
{
	int op, opl, len;

	p = skip_space(p);
	if ((op = C_NEG, *p == '-') || (op = C_LNOT, *p == '!') || (op = C_NOT, *p == '~')) {
		p = parse_subexpr(p + 1, 100);
		add_scriptc(op);
	}
	p = skip_space(p);
	p = parse_simpleexpr(p);
	p = skip_space(p);
	while (((op = C_ADD, opl = 6, len = 1, *p == '+') ||
	        (op = C_SUB, opl = 6, len = 1, *p == '-') ||
	        (op = C_MUL, opl = 7, len = 1, *p == '*') ||
	        (op = C_DIV, opl = 7, len = 1, *p == '/') ||
	        (op = C_MOD, opl = 7, len = 1, *p == '%') ||
	        (op = C_LAND, opl = 1, len = 2, *p == '&' && p[1] == '&') ||
	        (op = C_AND, opl = 5, len = 1, *p == '&') ||
	        (op = C_LOR, opl = 0, len = 2, *p == '|' && p[1] == '|') ||
	        (op = C_OR, opl = 4, len = 1, *p == '|') ||
	        (op = C_XOR, opl = 3, len = 1, *p == '^') ||
	        (op = C_EQ, opl = 2, len = 2, *p == '=' && p[1] == '=') ||
	        (op = C_NE, opl = 2, len = 2, *p == '!' && p[1] == '=') ||
	        (op = C_GE, opl = 2, len = 2, *p == '>' && p[1] == '=') ||
	        (op = C_GT, opl = 2, len = 1, *p == '>') ||
	        (op = C_LE, opl = 2, len = 2, *p == '<' && p[1] == '=') ||
	        (op = C_LT, opl = 2, len = 1, *p == '<')) && opl > limit) {
		p += len;
		p = parse_subexpr(p, opl);
		add_scriptc(op);
		p = skip_space(p);
	}
	return p;
}

unsigned char *parse_expr(unsigned char *p)
{
	p = parse_subexpr(p, -1);
	add_scriptc(C_NOP);
	return p;
}

unsigned char* parse_arg(int com, unsigned char *p)
{
	int argc, c, i;
	unsigned char *tmp, bak;

	for (argc = 0; com_tbl[com].arg[argc]; argc++) {
		p = skip_space(p);
		if (argc && *p ==',')
			p++;

		p = skip_space(p);
		switch (com_tbl[com].arg[argc]) {
		case 's':
			if (*p != '"') {
				printf("parse_arg : string must start \"\n");
				exit(1);
			}
			p++;
			while (*p && *p != '"') {
				if (p[-1] <= 0x7e && *p == '\\')
					p++;

				add_scriptb(*p++);
			}
			p++;
			add_scriptb(0);
			break;
		case 'm':
			for (c = 0; ; c++) {
				if (*p != '"') {
					printf("parse_arg : menu must start \"\n");
					exit(1);
				}
				p++;
				while (*p && *p != '"') {
					if (p[-1] <= 0x7e && *p == '\\')
						p++;

					add_scriptb(*p++);
				}
				add_scriptb(':');
				p++;
				p = skip_space(p);
				if (*p != ',') {
					printf("parse_arg : not found ','\n");
					exit(1);
				}
				p = skip_space(p+1);
				tmp = skip_word(p);
				bak = *tmp;
				*tmp = 0;
				menu_goto[c] = add_str(p);
				*tmp = bak;
				p = skip_space(tmp);
				if (*p == ',')
					p = skip_space(p + 1);

				else {
					c++;
					break;
				}
			}
			add_scriptb(0);
			add_scriptc(C_MENU_GOTO);
			for (i = 0; i < c; i++) {
				add_scriptl(menu_goto[i]);
			}
			add_scriptc(C_NOP);
			break;
		case 'l':
			tmp = skip_word(p);
			bak = *tmp;
			*tmp = 0;
			add_scriptl(add_str(p));
			*tmp = bak;
			p = skip_space(tmp);
			break;
		case 'i':
			p = parse_int(p);
			break;
		case 'I':
			p = parse_int(p);
			break;
		case 'e':
			p = parse_expr(p);
			break;
		case 'g':
			p = skip_space(p);
			if (comcmp(p, "goto")) {
				printf("if statement is 'if <cmp> goto <label>;' : %s\n", p);
				exit(1);
			}
			p = skip_space(p + 4);
			tmp = skip_word(p);
			bak = *tmp;
			*tmp = 0;
			add_scriptl(add_str(p));
			*tmp = bak;
			p = skip_space(tmp);
			break;
		default:
			printf("unknown arg type : '%c' @ %s\n", com_tbl[com].arg[argc], com_tbl[com].str);
			exit(1);
		}
	}
	return p;
}

unsigned char* parse_script(unsigned char *src)
{
	unsigned char *p, *tmp;
	int i;

	str_num = 1;
	for (i = 0; i < 16; i++)
	str_hash[i] = 0;

	script_buf = malloc(SCRIPT_BLOCK_SIZE);
	script_pos = 0;
	script_size = SCRIPT_BLOCK_SIZE;

	p = src;
	p = skip_space(p);
	if (*p != '{') {
		printf("not found '{' : %c\n", *p);
		free((void *)script_buf);
		return NULL;
	}
	for (p++; *p && *p != '}'; ) {
		p = skip_space(p);
		for (i = 0; com_tbl[i].str; i++) {
			if (comcmp(p, com_tbl[i].str) == 0) {
				break;
			}
		}
		if (com_tbl[i].str) {
			add_scriptc(com_tbl[i].com);
			p = skip_word(p);
			p = parse_arg(i, p);
		}
		else {
			tmp = skip_space(skip_word(p));
			if (*tmp == ':') {
				int l;
				*skip_word(p) = 0;
				l = add_str(p);
				set_label(l, script_pos);
				p = tmp + 1;
				continue;
			}
			else {
				printf("parse_script : unknown command %s\n", p);
				free((void *)script_buf);
				return NULL;
			}
		}
		p = skip_space(p);
		if (*p != ';') {
			printf("not found ';' : %s\n", p);
			free((void *)script_buf);
			return NULL;
		}
		p = skip_space(p + 1);
	}
	p = skip_space(p);
	if (*p != '}') {
		printf("not found '}' : %s\n", p);
		free((void *)script_buf);
		return NULL;
	}
	add_scriptc(C_CLOSE);
#ifdef DEBUG_PARSER_OUTPUT
	for (i = 0; i < script_pos; i++) {
		if ((i & 15) == 0) {
			printf("%04x : ", i);
		}
		printf("%02x ", script_buf[i]);
		if ((i & 15) == 15) {
			printf("\n");
		}
	}
	printf("\n");
#endif
	script_buf = realloc(script_buf, script_pos);
	return script_buf;
}

int get_com(unsigned char *script, int *pos)
{
	int i, j;

	if (unget_com_data >= 0) {
		i = unget_com_data;
		unget_com_data = -1;
		return i;
	}
	if (script[*pos] >= 0x80) {
		return C_INT;
	}
	i = 0; j = 0;
	while (script[*pos] >= 0x40) {
		i = script[(*pos)++] << j;
		j += 6;
	}
	return i + (script[(*pos)++] << j);
}

void unget_com(int c)
{
	if (unget_com_data != -1)
		printf("unget_com can back one data\n");

	unget_com_data = c;
}

int mmo_map_cutin(unsigned char* buf, char *img_name, int type)
{
	WBUFW(buf, 0) = 0x145;
	memcpy(WBUFP(buf, 2), img_name, 16);
	WBUFB(buf, 18) = type;
	return 19;
}

int mmo_map_refinelvl(struct map_session_data *sd, int equip)
{
	int i, check;

	for (i = check = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].equip == equip) {
			check += sd->status.inventory[i].refine;
		}
	}
	return check;
}

int mmo_map_checkequip(struct map_session_data *sd, int equip)
{
	int i;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].equip == equip) {
			return 1;
		}
	}
	return 99;
}

int mmo_map_countitem(struct map_session_data *sd, int nameid)
{
	int i, count;

	for (i = count = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid == nameid) {
			count += sd->status.inventory[i].amount;
		}
	}
	return count;
}

int mmo_map_readparam(struct map_session_data *sd, int i)
{
	switch(i) {
	case SP_SKILLPOINT:
		return sd->status.skill_point;
	case SP_STATUSPOINT:
		return sd->status.status_point;
	case SP_ZENY:
		return sd->status.zeny;
	case SP_BASELEVEL:
		return sd->status.base_level;
	case SP_JOBLEVEL:
		return sd->status.job_level;
	case SP_SCRIPT_FLAG:
		return sd->local_reg[14];
	case SP_GETJOB:
		return sd->status.class;
	case SP_WEIGHT:
		return sd->weight;
	case SP_MAXWEIGHT:
		return sd->max_weight;
	case SP_CHECKSTORAGE:
		if (sd->status.skill[0].lv > 5) {
			return 1;
		}
		else {
			return 0;
		}
	case SP_CHECKCART:
		if (sd->status.skill[38].lv > 0) {
			return 0;
		}
		else {
			return 1;
		}
	case SP_CHECKFALCON:
		if (sd->status.skill[126].lv > 0) {
			return 1;
		}
		else {
			return 0;
		}
	case SP_CHECKPECOPECO:
		if (sd->status.skill[62].lv > 0) {
			return 1;
		}
		else {
			return 0;
		}
	case SP_SEX:
		return sd->sex;
	case SP_ACCOUNTID:
		return sd->account_id;
	case SP_HASCART:
		if (sd->status.option_z & 0x000008) {
			return 1;
		}
		else {
			return 0;
		}
	case SP_GETWLVL:
		for (i = 0; i < MAX_INVENTORY; i++) {
			if (sd->status.inventory[i].equip == 2 || sd->status.inventory[i].equip == 34) {
				return item_database(sd->status.inventory[i].nameid).wlv;
			}
		}
		return 0;
	case SP_RANDOM:
		return (1 + rand() % 100);
	default:
		return 0;
	}
}

int get_num(struct map_session_data *sd, unsigned char *script, int *pos)
{
	int i, j, c;

	switch (c = get_com(script, pos)) {
	case C_PARAM:
		i = get_num(sd, script, pos);
		return mmo_map_readparam(sd, i);
	case C_LOCAL:
		i = get_num(sd, script, pos);
		if (i >= 0 && i < LOCAL_REG_NUM) {
			return sd->local_reg[i];
		}
		else {
			return 0;
		}
	case C_GLOBAL:
		i = get_num(sd, script, pos);
		for (j = 0; j < GLOBAL_REG_NUM; j++) {
			if (!sd->status.global_reg[j].str[0]) {
				break;
			}
			if (comcmp(sd->status.global_reg[j].str, alias_ext_tbl[i].str) == 0) {
				return sd->status.global_reg[j].value;
			}
		}
		return 0;
	case C_TEMPORAL:
		i = get_num(sd, script, pos);
		{
			struct temporal_reg *temporal_reg = temporal_r(sd->char_id);
			for (j = 0; temporal_reg[j].tbl_val != -1; j++) {
				if (temporal_reg[j].tbl_val == i) {
					return temporal_reg[j].value;
				}
			}
		}
		return 0;
	case C_INT:
		i = 0; j = 0;
		while (script[*pos] >= 0xc0) {
			i += (script[(*pos)++] & 0x7f) << j;
			j += 6;
		}
		return i + ((script[(*pos)++] & 0x7f) << j);
	default:
		printf("get_num : unknown command %d\n", c);
		return 0;
	}
	return 0;
}

int get_expr(struct map_session_data *sd, unsigned char *script, int *pos)
{
	int stack[32], sp, c;

	for (sp = 0; sp >= 0 && (c = get_com(script, pos)) != C_NOP; ) {
		switch(c) {
		case C_INT:
		case C_PARAM:
		case C_LOCAL:
		case C_GLOBAL:
		case C_TEMPORAL:
			unget_com(c);
			stack[sp++] = get_num(sd, script, pos);
			break;
		case C_ADD:
			sp--;
			stack[sp - 1] += stack[sp];
			break;
		case C_SUB:
			sp--;
			stack[sp - 1] -= stack[sp];
			break;
		case C_MUL:
			sp--;
			stack[sp - 1] *= stack[sp];
			break;
		case C_DIV:
			sp--;
			if (stack[sp] == 0) {
				stack[sp] = 1;
			}
			stack[sp - 1] /= stack[sp];
			break;
		case C_MOD:
			sp--;
			if (stack[sp] == 0) {
				stack[sp] = 1;
			}
			stack[sp - 1] %= stack[sp];
			break;
		case C_NEG:
			stack[sp - 1] = 0 - stack[sp - 1];
			break;
		case C_EQ:
			sp--;
			stack[sp - 1] = (stack[sp - 1] == stack[sp]);
			break;
		case C_NE:
			sp--;
			stack[sp - 1] = (stack[sp - 1] != stack[sp]);
			break;
		case C_GT:
			sp--;
			stack[sp - 1] = (stack[sp - 1] > stack[sp]);
			break;
		case C_GE:
			sp--;
			stack[sp - 1] = (stack[sp - 1] >= stack[sp]);
			break;
		case C_LT:
			sp--;
			stack[sp - 1] = (stack[sp - 1] < stack[sp]);
			break;
		case C_LE:
			sp--;
			stack[sp - 1] = (stack[sp - 1] <= stack[sp]);
			break;
		case C_LAND:
			sp--;
			stack[sp - 1] = (stack[sp - 1] && stack[sp]);
			break;
		case C_LOR:
			sp--;
			stack[sp - 1] = (stack[sp - 1] || stack[sp]);
			break;
		case C_LNOT:
			stack[sp - 1] = (!stack[sp - 1]);
			break;
		case C_AND:
			sp--;
			stack[sp - 1] = (stack[sp - 1] & stack[sp]);
			break;
		case C_OR:
			sp--;
			stack[sp - 1] = (stack[sp - 1] | stack[sp]);
			break;
		case C_XOR:
			sp--;
			stack[sp - 1] = (stack[sp - 1] ^ stack[sp]);
			break;
		case C_NOT:
			stack[sp - 1]= (~stack[sp-1]);
			break;
		case C_COUNTITEM:
			stack[sp - 1] = mmo_map_countitem(sd, stack[sp - 1]);
			break;
		case C_REFINELVL:
			stack[sp - 1] = mmo_map_refinelvl(sd, stack[sp - 1]);
			break;
		case C_CHECKEQUIP:
			stack[sp - 1] = mmo_map_checkequip(sd, stack[sp - 1]);
			break;
		case C_READPARAM:
			stack[sp - 1] = mmo_map_readparam(sd, stack[sp - 1]);
			break;
		default:
			printf("get_expr : unknown command %d\n", c);
			return 0;
		}
	}
	if (sp != 1) {
		if (sp <= 0) {
			printf("get_expr : stack empty???\n");
		}
		else {
			printf("get_expr : too many data???\n");
		}
		return 0;
	}
	return stack[sp - 1];
}

int run_script(int fd)
{
	int stop, end, len;
	int j;
	int s;
	int temp_map;
	int pos, c, i;
	int i1, i2, i3, i4;
	unsigned char *script, *s1;
	unsigned char buf[256];
	struct item tmp_item;

	struct map_session_data *sd = session[fd]->session_data;
	struct mmo_charstatus *p = &sd->status;
	pos = sd->npc_pc;
	script = map_data[sd->mapno].npc[sd->npc_n]->u.script;
	if (!script) {
		sd->npc_pc = 0;
		sd->npc_id = 0;
		sd->npc_n = 0;
		return 0;
	}
	for (stop = end = 0; !stop && !end;) {
		s = get_com(script, &pos);
		switch(s) {
			case C_MES:
				{
					char *charname="$CharName";
					char *replace_pos=script+pos;
					int match=0;
					while((replace_pos=strstr(replace_pos,charname))!=NULL) {
						match++;
						replace_pos+=10;
					}
					if(match) {
						static char *mes=NULL;
						static int mes_max;
						int new_len=strlen(sd->status.name);
						int str_len=strlen(script+pos);
						if(mes==NULL) {
							mes_max=str_len+1+(new_len-10)*match;
							mes=realloc(mes,mes_max);
						} else if(mes_max<str_len+1+(new_len-10)*match) {
							mes_max=str_len+1+(new_len-10)*match;
							mes=malloc(mes_max);
						}
						strcpy(mes,script+pos);
						while((replace_pos=strstr(mes,charname))!=NULL) {
							memmove(replace_pos+new_len,replace_pos+10,strlen(replace_pos)+1-10);
							memmove(replace_pos,sd->status.name,new_len);
							if(10>new_len) {
								*(mes+str_len-10+new_len)='\0';
								str_len=strlen(mes);
							}
						}
						len=mmo_map_npc_say(WFIFOP(fd,0),sd->npc_id,mes);
					}
					else {
						len=mmo_map_npc_say(WFIFOP(fd,0),sd->npc_id,script+pos);
					}
				}
				if(len>0) WFIFOSET(fd,len);
				pos+=strlen(script+pos)+1;
				break;
			case C_NEXT:
				len=mmo_map_npc_next(WFIFOP(fd,0),sd->npc_id);
				if(len>0) WFIFOSET(fd,len);
				stop=1;
				break;
			case C_CLOSE:
				len=mmo_map_npc_close(WFIFOP(fd,0),sd->npc_id);
				if(len>0) WFIFOSET(fd,len);
				end=1;
				break;
			case C_MENU:
				len=mmo_map_npc_select(WFIFOP(fd,0),sd->npc_id,script+pos);
				if(len>0) WFIFOSET(fd,len);
				pos+=strlen(script+pos)+1;
				stop=1;
				break;
			case C_MENU_GOTO:
				for(i=1,c=C_POS;i<sd->local_reg[15] && (c=get_com(script,&pos))==C_POS;i++)
				pos+=4;
				if(c!=C_POS || (c=get_com(script,&pos))!=C_POS) {
					end=1;
					break;
				}
				pos=*(int*)(script+pos);
				break;
			case C_JOBCHANGE:
				mmo_map_jobchange(fd, get_num(sd,script,&pos));
				break;
			case C_RESETSKILL:
				j=0;
				for(i=0;i<=MAX_SKILL;i++) {
					if(sd->status.skill[i].lv > 0 ) {
						j+=sd->status.skill[i].lv;
						sd->status.skill[i].lv=0;
					}
				}
				mmo_map_send_skills(fd, 1);
				mmo_map_update_param(fd,SP_SKILLPOINT,j);
				break;
			case C_RESETSTATUS:
				if(sd->status.int_>1) {
					while(sd->status.int_>1) {
						sd->status.int_--;
						j=calc_need_status_point(sd,SP_INT);
						mmo_map_update_param(fd,SP_STATUSPOINT,j);
					}
				}
				if(sd->status.agi>1) {
					while(sd->status.agi>1) {
						sd->status.agi--;
						j=calc_need_status_point(sd,SP_AGI);
						mmo_map_update_param(fd,SP_STATUSPOINT,j);
					}
				}
				if(sd->status.str>1) {
					while(sd->status.str>1) {
						sd->status.str--;
						j=calc_need_status_point(sd,SP_STR);
						mmo_map_update_param(fd,SP_STATUSPOINT,j);
					}
				}
				if(sd->status.vit>1) {
					while(sd->status.vit>1) {
						sd->status.vit--;
						j=calc_need_status_point(sd,SP_VIT);
						mmo_map_update_param(fd,SP_STATUSPOINT,j);
					}
				}
				if(sd->status.dex>1) {
					while(sd->status.dex>1) {
						sd->status.dex--;
						j=calc_need_status_point(sd,SP_DEX);
						mmo_map_update_param(fd,SP_STATUSPOINT,j);
					}
				}
				if(sd->status.luk>1) {
					while(sd->status.luk>1) {
						sd->status.luk--;
						j=calc_need_status_point(sd,SP_LUK);
						mmo_map_update_param(fd,SP_STATUSPOINT,j);
					}
				}
				mmo_send_selfdata(sd);
				break;
			case C_GOTO:
				if(get_com(script,&pos)!=C_POS) {
					end=1;
					break;
				}
				pos=*(int*)(script+pos);
				break;
			case C_INPUT:
				len=mmo_map_npc_amount_request(WFIFOP(fd,0),sd->npc_id);
				if(len>0) WFIFOSET(fd,len);
				stop=1;
				break;
			case C_SETLOOK:
				//アイテムナンバー
				i1=get_num(sd,script,&pos);
				i2=get_num(sd,script,&pos);
				len=mmo_map_set_look(WFIFOP(fd,0),sd->account_id,itemdb_stype(i1),itemdb_view_point(i1));
				if(len>0) mmo_map_sendarea(fd,WFIFOP(fd,0),len,0);
				break;
			case C_WARP:
				s1=script+pos;
				pos+=strlen(script+pos)+1;
				i1=get_num(sd,script,&pos);
				i2=get_num(sd,script,&pos);
				sd->status.talking_to_npc=0;
				mmo_map_changemap(sd,s1,i1,i2,2);
				break;
			case C_SETZENY:
				c=get_com(script,&pos);
				i1=get_num(sd,script,&pos);
				i2=get_expr(sd,script,&pos);
				switch(c) {
					case C_PARAM:
						mmo_map_update_param(fd,i1,i2);
						break;
					case C_LOCAL:
						sd->local_reg[i1]=i2;
						break;
					case C_GLOBAL:
//						sd->status.global_reg[i1]=i2;
						break;
				}
				break;
			case C_SET:
			case C_RANDOM:
				c=get_com(script,&pos);
				i1=get_num(sd,script,&pos);
				if(s==C_SET) {
					i2=get_expr(sd,script,&pos);
				}
				else {
					i2=randN(get_num(sd,script,&pos));
				}
				switch(c) {
					case C_PARAM:
						mmo_map_update_param(fd,i1,i2);
						break;
					case C_LOCAL:
						sd->local_reg[i1]=i2;
						break;
					case C_GLOBAL:
						if(i2==0) {
							int j=-1;
							for(i=0;i<GLOBAL_REG_NUM;i++) {
								if(!sd->status.global_reg[i].str[0])
								break;
								if(comcmp(sd->status.global_reg[i].str,alias_ext_tbl[i1].str)==0)
								j=i;
							}
							if(0<=j && 0<i) {
								sd->status.global_reg[j]=sd->status.global_reg[i-1];
								sd->status.global_reg[i-1].str[0]='\0';
							}
						}
						else {
    						for(i=0;i<GLOBAL_REG_NUM;i++) {
								if(!sd->status.global_reg[i].str[0])
								break;
								if(comcmp(sd->status.global_reg[i].str,alias_ext_tbl[i1].str)==0)
    							break;
							}
							if(i<GLOBAL_REG_NUM) {
								if(sd->status.global_reg[i].str[0]==0)
								strcpy(sd->status.global_reg[i].str,alias_ext_tbl[i1].str);
								sd->status.global_reg[i].value=i2;
							} else {
								printf("too many global variables used by %s.\n", sd->status.name);
							}
						}
    					break;
					case C_TEMPORAL: {
						struct temporal_reg *temporal_reg=temporal_r(sd->char_id);
						for(i=0;temporal_reg[i].tbl_val!=-1;i++) {
							if(temporal_reg[i].tbl_val==i1)
							break;
						}
						if(temporal_reg[i].tbl_val==-1)
						temporal_reg[i].tbl_val=i1;
						temporal_reg[i].value=i2;
					}
					break;
				}
				break;
			case C_IF:
				i1=get_expr(sd,script,&pos);
				if(get_com(script,&pos)!=C_POS) {
					end=1;
					break;
				}
				if(i1)
					pos=*(int*)(script+pos);
				else
					pos+=4;
				break;
			case C_GETITEM:
				i1=get_num(sd,script,&pos);
				i2=get_expr(sd,script,&pos);
				memset(&tmp_item,0,sizeof(tmp_item));
				tmp_item.nameid=i1;
				tmp_item.amount=i2;
				tmp_item.identify=1;
				mmo_map_get_item(fd, &tmp_item);
				break;
			case C_DELITEM:
				i1=get_num(sd,script,&pos);
				i2=get_expr(sd,script,&pos);
				for(i=0;i<100;i++) {
					if(sd->status.inventory[i].nameid==i1) {
						if(sd->status.inventory[i].amount>=i2) {
							if(mmo_map_lose_item(fd,0,i+2,i2)) i2 = 0;
						} else {
							if(mmo_map_lose_item(fd,0,i+2,sd->status.inventory[i].amount)) i2-=sd->status.inventory[i].amount;
						}
						break;
					}
				}
				if(i2>0)
				sd->local_reg[13]++;
				break;
			case C_CUTIN:
				s1=script+pos;
				pos+=strlen(script+pos)+1;
				i1=get_num(sd,script,&pos);
				len=mmo_map_cutin(WFIFOP(fd,0),s1,i1);
				if(len>0) WFIFOSET(fd,len);
				break;
			case C_MONSTER:
				temp_map = sd->mapno;
				s1=script+pos;
				pos+=strlen(script+pos)+1;
				i1=get_num(sd,script,&pos);
				i2=get_num(sd,script,&pos);
				i3=get_num(sd,script,&pos);
				i4=get_num(sd,script,&pos);
				for(j=0;j <i2;j++) spawn_to_pos(sd,i1,s1,i3,i4,temp_map,fd);
				break;
			case C_VIEWPOINT:
				WFIFOW(fd,0)=0x144;
				WFIFOL(fd,2)=sd->npc_id;
				WFIFOL(fd,6)=get_num(sd,script,&pos);	// type
				WFIFOL(fd,10)=get_num(sd,script,&pos);	// x
				WFIFOL(fd,14)=get_num(sd,script,&pos);	// y
				WFIFOB(fd,18)=get_num(sd,script,&pos);	// point id
				WFIFOL(fd,19)=get_num(sd,script,&pos);	// color
				WFIFOSET(fd,23);
				break;
			case C_SAVE:
				s1=script+pos;
				pos+=strlen(script+pos)+1;
				i1=get_num(sd,script,&pos);
				i2=get_num(sd,script,&pos);
				strcpy(sd->status.save_point.map,s1);
				sd->status.save_point.x = i1;
				sd->status.save_point.y = i2;
				mmo_char_save(sd);
				break;
			case C_GIVESPECIAL:
			i1 = get_num(sd, script, &pos);
			if (i1 == 1) { // if give_special 1 -- give target a cart
				sd->status.special = 0x08;
				
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = sd->account_id;
				WBUFW(buf, 6) = sd->status.option_x;
				WBUFW(buf, 8) = sd->status.option_y;
				WBUFW(buf, 10) = 0x08; // sd->status.option_z
				WBUFB(buf, 12) = 0x00; // without this, char is sometimes drawn with weapon out
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
				// Send list of cart items (+ cartcount and cartweight)
				mmo_cart_send_items(fd);
			}
			if (i1 == 2) { // if give_special 2 -- give target a falcon
				sd->status.special = 0x10;
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = sd->account_id;
				WBUFW(buf, 6) = sd->status.option_x;
				WBUFW(buf, 8) = sd->status.option_y;
				WBUFW(buf, 10) = 0x10; // sd->status.option_z
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
			}
			if (i1 == 3) { // if give_special 3 -- give target a pecopeco
				sd->status.special = 0x20;
				WBUFW(buf, 0) = 0x119;
				WBUFL(buf, 2) = sd->account_id;
				WBUFW(buf, 6) = sd->status.option_x;
				WBUFW(buf, 8) = sd->status.option_y;
				WBUFW(buf, 10) = 0x20; // sd->status.option_z
				mmo_map_sendarea(fd, buf, packet_len_table[0x119], 0);
				mmo_map_calc_status(fd, 0);
			}
			break;
			case C_FULLHEAL:
				p->hp = p->max_hp;
				p->sp = p->max_sp;
				WFIFOW(fd,0) = 0xb0;
			    WFIFOW(fd,2) = 0005;
			    WFIFOL(fd,4) = p->hp;
			    WFIFOSET(fd,8);

			    WFIFOW(fd,0) = 0xb0;
			    WFIFOW(fd,2) = 0007;
			    WFIFOL(fd,4) = p->sp;
			    WFIFOSET(fd,8);
			    break;
			case C_RELEASE:
				i1=get_num(sd,script,&pos);
				for(i=0;i<100;i++) {
					if(sd->status.inventory[i].amount>=1) {
						int item_num;
						item_num = itemdb_stype(sd->status.inventory[i].nameid);
						if ((sd->status.inventory[i].equip) && (item_num == i1)) {
							WFIFOW(fd,0)=0xac;
							WFIFOW(fd,2)=i+2;
							WFIFOW(fd,4)=sd->status.inventory[i].equip;
							WFIFOW(fd,6)=1;
							WFIFOSET(fd,7);
							sd->status.inventory[i].equip=0;
						} else {
							WFIFOW(fd,0)=0xac;
							WFIFOW(fd,2)=i+2;
							WFIFOW(fd,6)=0;
							WFIFOSET(fd,7);
						}
						len=mmo_map_set_look(WFIFOP(fd,0),sd->account_id,item_num,0);
						if(len>0) mmo_map_sendall(fd,WFIFOP(fd,0),len,0);
						mmo_map_calc_status(fd,0);
						set_equip(sd, item_num, 0);
					}
				}
				break;
			case C_RELEASEALL:
				for(i=0;i<100;i++) {
					if(sd->status.inventory[i].amount>=1) {
						int item_num;
						item_num = itemdb_stype(sd->status.inventory[i].nameid);
						if(sd->status.inventory[i].equip) {
							WFIFOW(fd,0)=0xac;
							WFIFOW(fd,2)=i+2;
							WFIFOW(fd,4)=sd->status.inventory[i].equip;
							WFIFOW(fd,6)=1;
							WFIFOSET(fd,7);
							sd->status.inventory[i].equip=0;
						} else {
							WFIFOW(fd,0)=0xac;
							WFIFOW(fd,2)=i+2;
							WFIFOW(fd,6)=0;
							WFIFOSET(fd,7);
						}
						len=mmo_map_set_look(WFIFOP(fd,0),sd->account_id,item_num,0);
						if(len>0) mmo_map_sendall(fd,WFIFOP(fd,0),len,0);
						mmo_map_calc_status(fd,0);
						set_equip(sd,item_num,0);
					}
				}
				break;
			case C_REFINE:
				i1=get_num(sd,script,&pos);
				for(i=0;i<MAX_INVENTORY;i++)
				if(p->inventory[i].equip == i1) {
					WFIFOW(fd,0) = 0x188;
					WFIFOW(fd,2) = 0;
					WFIFOSET(fd,8);
					WFIFOW(fd, 0) = 0x19b;
					WFIFOL(fd, 2) = sd->account_id;
					WFIFOL(fd, 6) = 3; //success send to all
					mmo_map_sendarea(fd, WFIFOP(fd, 0), packet_len_table[0x19b], 0);
					p->inventory[i].refine = p->inventory[i].refine + 1;
					mmo_send_selfdata(sd);
					mmo_map_calc_status(fd,0);
				}
				break;
			case C_FAILURE:
				i1=get_num(sd,script,&pos);
				for(i=0;i<MAX_INVENTORY;i++)
				if(p->inventory[i].equip == i1) {
					WFIFOW(fd,0) = 0x188;
					WFIFOW(fd,2) = 1;
					WFIFOSET(fd,8);
					WFIFOW(fd, 0) = 0x19b;
					WFIFOL(fd, 2) = sd->account_id;
					WFIFOL(fd, 6) = 2; //failed to all
					mmo_map_sendarea(fd, WFIFOP(fd, 0), packet_len_table[0x19b], 0);
					p->inventory[i].refine -= p->inventory[i].refine;
					mmo_send_selfdata(sd);
					mmo_map_calc_status(fd,0);
				}
				break;
			case C_HEALHP:
				i1 = get_num(sd, script, &pos);
				p->hp += i1;
				WFIFOW(fd, 0) = 0xb0;
			   	WFIFOW(fd, 2) = 0005;
			   	WFIFOL(fd, 4) = p->hp;
			    	WFIFOSET(fd, packet_len_table[0xb0]);
			   	break;
			case C_HEALSP:
				i1 = get_num(sd, script, &pos);
				p->sp += i1;
				WFIFOW(fd, 0) = 0xb0;
			   	WFIFOW(fd, 2) = 0007;
			    	WFIFOL(fd, 4) = p->sp;
			    	WFIFOSET(fd, packet_len_table[0xb0]);
			   	break;
			case C_STORAGE:
				mmo_open_storage(fd);
				break;
			case C_EMOTION:
				i1=get_num(sd,script,&pos);
				WBUFW(buf, 0) = 0x1aa;
				WBUFL(buf, 2) = map_data[sd->mapno].npc[sd->npc_n]->id;
				WBUFL(buf, 6) = i1;
				mmo_map_sendarea(fd, buf, packet_len_table[0x1aa], 0);
				break;
			default:
			{
				int i;
				fprintf(stderr, "run_script error %04x : ", pos);
				for (i = -5; i <= 5; i++)
				fprintf(stderr, "%02x%c\n", script[pos+i],(i==-1) ? '[':((i==0) ? ']':' '));
				end = 1;
				break;
			}
		}
	}
	if (end) {
		sd->npc_pc = 0;
		sd->npc_id = 0;
		sd->npc_n = 0;
	}
	else {
		sd->npc_pc = pos;
	}
	return 0;
}
