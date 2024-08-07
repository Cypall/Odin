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
 Creation Date: Dicember 6, 2003
 Modified Date: October 31, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

#include "core.h"
#include "mmo.h"
#include "map_core.h"
#include "chat.h"

/*======================================
 *	CHAT: General Chat Functions
 *--------------------------------------
 */
extern int char_fd;

void send_msg(int fd, char* msg)
{
	char moji[200];
	memset(moji, 0, 200);
	strncpy(moji, msg, 200);
	WFIFOW(fd, 0) = 0x8e;
	WFIFOW(fd, 2) = 4 + 200;
	memcpy(WFIFOP(fd, 4), moji, 200);
	WFIFOSET(fd, 4 + 200);
}

void send_msg_p(int fd, char* from, char* msg)
{
	unsigned short msg_len = strlen(msg) + 1;
	WFIFOW(fd, 0) = 0x97;
	WFIFOW(fd, 2) = 28 + msg_len;
	memcpy(WFIFOP(fd, 4), from, 24);
	memcpy(WFIFOP(fd, 28), msg, msg_len);
	WFIFOSET(fd, WFIFOW(fd, 2));
}

void send_msg_mon(int fd, int monster_id, char* msg, int wisp_to)
{
	unsigned short msg_len = strlen(msg) + 1;
	unsigned char buf[256];

	WBUFW(buf, 0) = 0x8d;
	WBUFW(buf, 2) = msg_len + 8;
	WBUFL(buf, 4) = monster_id;
	memcpy(WBUFP(buf, 8), msg, msg_len);

	if (wisp_to == 1) {
		memcpy(WFIFOP(fd, 0), buf, msg_len + 8);
		WFIFOSET(fd, msg_len + 8);
	}
	else
		mmo_map_sendarea(fd, buf, msg_len + 8, 0);
}

void monster_say(int fd, int monster_id, int class, char *type)
{
	int i = 0;
	char msg[80];

	if (rand() % 2 == 0)
		i = 1;

	if (strncmp(type, "discovery", 3) == 0) {
		if (i == 1)
			sprintf(msg, "%s", mons_data[class].mons_say.discovery_1);

		else {
			if(strlen(mons_data[class].mons_say.discovery_2) > 3)
				sprintf(msg, "%s", mons_data[class].mons_say.discovery_2);
			else
				sprintf(msg, "%s", mons_data[class].mons_say.discovery_1);
		}
	}
	if (strncmp(type, "attack",3) == 0) {
		if (i == 1)
			sprintf(msg, "%s", mons_data[class].mons_say.attack_1);

		else {
			if(strlen(mons_data[class].mons_say.attack_2) > 3)
				sprintf(msg, "%s", mons_data[class].mons_say.attack_2);
			else
				sprintf(msg, "%s", mons_data[class].mons_say.attack_1);
		}
	}
	if (strncmp(type, "hp50", 3) == 0) {
		if (i == 1)
			sprintf(msg, "%s", mons_data[class].mons_say.hp50_1);

		else {
			if(strlen(mons_data[class].mons_say.hp50_2) > 3)
				sprintf(msg, "%s", mons_data[class].mons_say.hp50_2);
			else
				sprintf(msg, "%s", mons_data[class].mons_say.hp50_1);
		}
	}
	if (strncmp(type, "hp25", 3) == 0) {
		if (i == 1)
			sprintf(msg, "%s", mons_data[class].mons_say.hp25_1);

		else {
			if(strlen(mons_data[class].mons_say.hp25_2) > 3)
				sprintf(msg, "%s", mons_data[class].mons_say.hp25_2);
			else
				sprintf(msg, "%s", mons_data[class].mons_say.hp25_1);
		}
	}
	if (strncmp(type, "kill", 3) == 0) {
		if(i == 1)
			sprintf(msg, "%s", mons_data[class].mons_say.kill_1);

		else {
			if(strlen(mons_data[class].mons_say.kill_2) > 3)
				sprintf(msg, "%s", mons_data[class].mons_say.kill_2);
			else
				sprintf(msg, "%s", mons_data[class].mons_say.kill_1);
		}
	}
	if (strncmp(type, "dead", 3) == 0) {
		if (i == 1)
			sprintf(msg, "%s", mons_data[class].mons_say.dead_1);

		else {
			if(strlen(mons_data[class].mons_say.dead_2) > 3)
				sprintf(msg, "%s", mons_data[class].mons_say.dead_2);
			else
				sprintf(msg, "%s", mons_data[class].mons_say.dead_1);
		}
	}
	if (strlen(msg) > 3)
		send_msg_mon(fd, monster_id, msg, 0);

}

/*======================================
 *	CHAT: Whisper Functions
 *--------------------------------------
 */

int mmo_map_sendwis(unsigned int fd, int len, char *name, char *buf)
{
	unsigned int i;
	struct map_session_data *srcsd, *dstsd;

	if (!session[fd] || !(srcsd = session[fd]->session_data))
		return 1;

	for (i = 5; i < FD_SETSIZE; i++) {
		if (!session[i] || fd == i || !(dstsd = session[i]->session_data))
			continue;

		if (strncmp(dstsd->status.name, name, 24) == 0 && !dstsd->no_whispers) {
			WFIFOW(i, 0) = 0x97;
			WFIFOW(i, 2) = len;
			memcpy(WFIFOP(i, 4), srcsd->status.name, 24);
			memcpy(WFIFOP(i, 28), buf, len - 28);
			WFIFOSET(i, len);

			WFIFOW(fd, 0) = 0x98;
			WFIFOB(fd, 2) = i == FD_SETSIZE;
			WFIFOSET(fd, packet_len_table[0x98]);
			return 0;
		}
		else if (strncmp(dstsd->status.name, name, 24) == 0 && dstsd->no_whispers) {
			WFIFOW(fd, 0) = 0x98;
			WFIFOB(fd, 2) = 3;
			WFIFOSET(fd, packet_len_table[0x98]);
			return 0;
		}
	}
#ifdef _MMS_
	if (i == FD_SETSIZE) {
		WFIFOW(char_fd, 0) = 0x2c02;
		WFIFOW(char_fd, 2) = len + 28;
		memcpy(WFIFOP(char_fd, 4), name, 24);
		memcpy(WFIFOP(char_fd, 28), srcsd->status.name, 24);
		memcpy(WFIFOP(char_fd, 52), buf, len - 28);
		WFIFOSET(char_fd, len + 28);
		return 0;
	}
#endif
	return 0;
}

int mmo_map_wisuserblock(struct map_session_data *sd, char *name, short type)
{
	if (!sd)
		return 0;

	printf("Todo: Add User Block Packet\n");
	printf("Recived->Player %s, Type %d\n", name, type);
	return 0;
}

int mmo_map_wisblock(struct map_session_data *sd)
{
	int fd = sd->fd;

	if (RFIFOB(fd, 2) == 0) {
		sd->no_whispers = 1;
		WFIFOW(fd, 0) = 0xd2;
		WFIFOB(fd, 2) = 0;
		WFIFOB(fd, 3) = 0;
		WFIFOSET(fd, packet_len_table[0xd2]);
	}
	else {
		sd->no_whispers = 0;
		WFIFOW(fd, 0) = 0xd2;
		WFIFOB(fd, 2) = 1;
		WFIFOB(fd, 3) = 0;
		WFIFOSET(fd, packet_len_table[0xd2]);
	}
	return 0;
}

/*======================================
 *	CHAT: Pub Creation Functions
 *--------------------------------------
 */

int mmo_map_createchat(struct mmo_chat* chat)
{
	if (0 != last_chat)
		last_chat->next = chat;

	else
		chat->next = 0;

	chat->prev = last_chat;
	last_chat = chat;
	return 0;
}

int mmo_map_delchat(struct mmo_chat* chat)
{
	struct mmo_chat* temp_chat;

	temp_chat = last_chat;
	for ( ; ; ) {
		if (0 == temp_chat)
			break;

		if (temp_chat == chat) {
			if (temp_chat == last_chat)
				last_chat = temp_chat->prev;

			if (0 != temp_chat->next)
				temp_chat->next->prev = temp_chat->prev;

			if (0 != temp_chat->prev)
				temp_chat->prev->next = temp_chat->next;

			break;
		}
		else
			temp_chat = temp_chat->prev;

	}
	return 0;
}

#if 0
struct mmo_chat* mmo_map_getchat(unsigned long id)
{
	struct mmo_chat* temp_chat;
	temp_chat = last_chat;
	for ( ; ; ) {
		if (0 == temp_chat)
			break;

		if (1) {
			if (temp_chat->chatID == id)
				return (temp_chat);
		}
		temp_chat = temp_chat->prev;
	}
	return (0);
}
#endif

int mmo_map_addchat(int fd, struct mmo_chat* chat, char *pass)
{
	int i, len;
	unsigned char buf[256];
	struct map_session_data *sd;

	if (session[fd] == NULL || session[fd]->session_data == NULL || chat == NULL)
		return 0;

	sd = session[fd]->session_data;
	if (chat->limit == chat ->users || (chat->pub == 0 && strncmp(pass, chat->pass, 8))) {
		WFIFOW(fd, 0) = 0xda;
		WFIFOB(fd, 2) = 1;
		WFIFOSET(fd, packet_len_table[0xda]);
		return 0;
	}
	chat->usersfd[chat->users] = fd;
	chat->usersID[chat->users] = sd->account_id;
	memcpy(chat->usersName[chat->users], sd->status.name, 24);
	chat->users++;

	sd->chatID = chat->chatID;

	WFIFOW(fd, 0) = 0xdb;
	WFIFOW(fd, 2) = 8 + (28 * chat->users);
	WFIFOL(fd, 4) = chat->chatID;
	for (i = 0; i < chat->users; i++) {
		WFIFOL(fd, 8 + i * 28) = i != 0;
		memcpy(WFIFOP(fd, 8 + i * 28 + 4), chat->usersName[i], 24);
	}
	WFIFOSET(fd, WFIFOW(fd, 2));

	WBUFW(buf, 0) = 0x0dc;
	WBUFW(buf, 2) = chat->users;
	memcpy(WBUFP(buf, 4), sd->status.name, 24);
	mmo_map_sendchat(fd, buf, packet_len_table[0xdc], 1);

	len = mmo_map_set00d7(chat->usersfd[0], buf);
	mmo_map_sendarea(chat->usersfd[0], buf, len, 3);
	return 0;
}

int mmo_map_leavechat(int fd, struct mmo_chat* chat, char* leavecharname)
{
	int i, leavechar, leavechar_fd;
	int len;
	unsigned char buf[256];
	struct map_session_data* sd;

	if (!chat)
		return -1;

	for (i = 0, leavechar = -1; i < chat->users; i++) {
		if (strcmp(chat->usersName[i], leavecharname) == 0) {
			leavechar = i;
			break;
		}
	}
	if (leavechar < 0)
		return -1;

	leavechar_fd = chat->usersfd[leavechar];
	if (session[leavechar_fd] == NULL || session[leavechar_fd]->session_data == NULL)
		return -1;

	sd = session[leavechar_fd]->session_data;

	if (leavechar == 0 && chat->users > 1) {
		WBUFW(buf, 0) = 0xe1;
		WBUFL(buf, 2) = 1;
		memcpy(WBUFP(buf, 6), chat->usersName[0], 24);
		WBUFW(buf, 30) = 0xe1;
		WBUFL(buf, 32) = 0;
		memcpy(WBUFP(buf, 36), chat->usersName[1], 24);
		mmo_map_sendchat(fd, buf, packet_len_table[0xe1] * 2, 0);

		WBUFW(buf, 0) = 0xd8;
		WBUFL(buf, 2) = (unsigned long)chat;
		mmo_map_sendarea(chat->usersfd[0], buf, 6, 3);

		chat->ownID = chat->usersID[1];
	}
	WBUFW(buf, 0) = 0xdd;
	WBUFW(buf, 2) = chat->users - 1;
	memcpy(WBUFP(buf, 4), chat->usersName[leavechar], 24);
	WBUFB(buf, 28) = 0;
	mmo_map_sendchat(fd, buf, packet_len_table[0xdd], 0);

	chat->users--;
	sd->chatID = 0;

	if (chat->users == 0) {
		WBUFW(buf, 0) = 0xd8;
		WBUFL(buf, 2) = (unsigned long)chat;
		mmo_map_sendarea(chat->usersfd[0], buf, 6, 0);
		mmo_map_delchat(chat);
		free(chat);
	}
	else {
		for (i = leavechar; i < chat->users; i++) {
			chat->usersfd[i] = chat->usersfd[i + 1];
			chat->usersID[i] = chat->usersID[i + 1];
			memcpy(chat->usersName[i], chat->usersName[i + 1], 24);
		}
		len = mmo_map_set00d7(chat->usersfd[0], buf);
		mmo_map_sendarea(chat->usersfd[0], buf, len, 3);
	}
	return 0;
}

int mmo_map_changeowner(int fd, struct mmo_chat* chat, char *nextownername)
{
	int i, nextowner, len;
	unsigned char buf[256];

	if (!chat)
		return -1;

	for (i = 0, nextowner = -1; i < chat->users; i++) {
		if (strcmp(chat->usersName[i], nextownername) == 0) {
			nextowner = i;
			break;
		}
	}
	if (nextowner < 0)
		return -1;

	WBUFW(buf, 0) = 0xe1;
	WBUFL(buf, 2) = 1;
	memcpy(WBUFP(buf, 6), chat->usersName[0], 24);
	WBUFW(buf, 30) = 0xe1;
	WBUFL(buf, 32) = 0;
	memcpy(WBUFP(buf, 36), chat->usersName[nextowner], 24);
	mmo_map_sendchat(fd, buf, packet_len_table[0xe1] * 2, 0);

	WBUFW(buf, 0) = 0xd8;
	WBUFL(buf, 2) = (unsigned long)chat;
	mmo_map_sendarea(fd, buf, 6, 3);

	i = chat->usersfd[0];
	chat->usersfd[0] = chat->usersfd[nextowner];
	chat->usersfd[nextowner] = i;
	i = chat->usersID[0];
	chat->usersID[0] = chat->usersID[nextowner];
	chat->usersID[nextowner] = i;
	memcpy(buf, chat->usersName[0], 24);
	memcpy(chat->usersName[0], chat->usersName[nextowner], 24);
	memcpy(chat->usersName[nextowner], buf, 24);

	chat->ownID = chat->usersID[0];

	len = mmo_map_set00d7(chat->usersfd[0], buf);
	mmo_map_sendarea(chat->usersfd[0], buf, len, 3);
	return 0;
}
