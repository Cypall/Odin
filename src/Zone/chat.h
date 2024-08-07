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

#ifndef _CHAT_H_
#define _CHAT_H_

struct mmo_chat *last_chat;
void send_msg(unsigned int fd, char* msg);
void send_msg_p(unsigned int fd, char* from, char* msg);
void send_msg_mon(unsigned int fd, int monster_id, char* msg, int wisp_to);
void monster_say(unsigned int fd, int monster_id, int class, char *type);
int mmo_map_sendwis(unsigned int fd, int len, char *name, char *buf);
int mmo_map_wisuserblock(struct map_session_data *sd, char *name, short type);
int mmo_map_wisblock(struct map_session_data *sd);
int mmo_map_createchat(struct mmo_chat* chat);
int mmo_map_delchat(struct mmo_chat* chat);
int mmo_map_addchat(unsigned int fd, struct mmo_chat* chat, char *pass);
int mmo_map_leavechat(unsigned int fd, struct mmo_chat* chat, char* leavecharname);
int mmo_map_changeowner(unsigned int fd, struct mmo_chat* chat, char *nextownername);
#endif
