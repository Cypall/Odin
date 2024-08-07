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

struct skill_db {
	unsigned int id;
	int type_num;
	int type_hit;
	int type_inf;
	int type_pl;
	int type_nk;
	int type_lv;
	int sp;
	int range;
} skill_db[MAX_SKILL];

void skilldb_init(void);
void skilldb_read_db(void);
void skilldb_read_tree(void);
void mmo_map_send_skills(int fd, int do_check);
void skilldb_job_check(struct map_session_data *sd);
unsigned long search_placeskill(int skill_id);
