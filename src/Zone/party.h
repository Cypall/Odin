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
 Module:        Version 1.7.0 SP1
 Author:        Odin Developer Team Copyrights (c) 2004
 Project:       Project Odin Zone Server
 Creation Date: Dicember 6, 2003
 Modified Date: Semtember 3, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

void mmo_party_do_init(void);
void init_party_data(int index);
void reset_member_data(int index, short mbr);
int find_party_slot(void);
void create_party(int fd, char partyname[24]);
void party_make_member(int fd, int mbr, int index);
int party_exists(int party_num);
void update_party(int party_num);
void delete_party(int index);
void party_remove_member(int fd, int ldr, int index);
int leaveparty(int fd);
void send_party_setup(int fd);
void send_party_setup_all(int index);
int check_party_share(int index);
void party_member_login(int party_num, int fd);
void party_member_logout(int party_num, int account_id, int char_id, short mapno);
void party_update_member_location(int party_num, int account_id, int char_id, short x, short y, short mapno);
void party_update_member_map(int party_num, int account_id, int char_id, short mapno, char map_name[16]);
void party_update_member_hp(int party_num, int account_id, int char_id, int hp, int max_hp, short mapno);
void mmo_party_save(int party_num);
