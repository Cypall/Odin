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

void card_send_equip(int fd, int card_loc);
void card_finish_equip(int fd, int srcid, int descid);
void mmo_map_cardskills1(int fd, struct map_session_data *sd);
void mmo_map_cardskills2(int fd, struct map_session_data *sd);
int card_wait(struct map_session_data *sd, int wait);
void card_skill_effect(struct map_session_data *sd, int fd, int m, int n);
int card_add_damage(struct map_session_data *sd, int fd, int damage, int m, int n);
int card_reduce_sp(struct map_session_data *sd, int sp);
int card_element_immunity(struct map_session_data *sd, int avoid, int m, int n);
int card_parmor_element(struct map_session_data *sd, int target_ele);
int card_damage_reduce(struct map_session_data *sd, int damage, int m, int n);
int mmo_map_calc_card(int fd, int item_id, int inv_num, int type);
