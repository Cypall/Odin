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

#ifndef _CONFIG_H_
#define _CONFIG_H_

int make_backup(void)
{
	char *dirct = "save/backup";
	FILE *bak, *players, *storage, *guild_data, *party_members, *parties, *guild_members, *pets;
	DIR *backup;

	/*
	 * Character Backup
	 */

	players = fopen("save/characters/characters.txt", "rt");
	if (!players) {
		debug_output("make_backup: Cannot open file 'save/characters/characters.txt'.\n");
		debug_output("make_backup: Backup has fail!\n");
		return 1;
	}
	else if (!(backup = opendir(dirct))) {
		debug_output("make_backup: Cannot Open Folder: '%s'. Please create one.\n", dirct);
		debug_output("make_backup: Backup has fail!\n");
		fclose(players);
		return 1;
	}
	else {
		bak = fopen("save/backup/characters.txt.bak", "wt");
		fflush(bak);
		filecopy(players, bak);
		fclose(bak);
	}
	fclose(players);

	/* 
	 * Storage Backup
	 */

	storage = fopen("save/storages/storage.txt", "rt");
	if (!storage) {
		debug_output("make_backup: Cannot open file 'save/storages/storage.txt'.\n");
		debug_output("make_backup: Backup has fail!\n");
		return 1;
	}
	else if (!(backup = opendir(dirct))) {
		debug_output("make_backup: Cannot Open Folder: '%s'. Please create one.\n", dirct);
		debug_output("make_backup: Backup has fail!\n");
		fclose(storage);
		return 1;
	}
	else {
		bak = fopen("save/backup/storage.txt.bak", "wt");
		fflush(bak);
		filecopy(storage, bak);
		fclose(bak);
	}
	fclose(storage);

	/*
	 * Guilds Backup
	 */

	guild_data = fopen("save/guilds/guild_data.txt", "rt");
	if (!guild_data) {
		debug_output("make_backup: Cannot open file 'save/guilds/guild_data.txt'.\n");
		debug_output("make_backup: Backup has fail!\n");
		return 1;
	}
	else if (!(backup = opendir(dirct))) {
		debug_output("make_backup: Cannot Open Folder: '%s'. Please create one.\n", dirct);
		debug_output("make_backup: Backup has fail!\n");
		fclose(guild_data);
		return 1;
	}
	else {
		bak = fopen("save/backup/guild_data.txt.bak", "wt");
		fflush(bak);
		filecopy(guild_data, bak);
		fclose(bak);
	}
	fclose(guild_data);

	/****************************************************************************/

	guild_members = fopen("save/guilds/guild_members.txt", "rt");
	if (!guild_members) {
		debug_output("make_backup: Cannot open file 'save/guilds/guild_members.txt'.\n");
		debug_output("make_backup: Backup has fail!\n");
		return 1;
	}
	else if (!(backup = opendir(dirct))) {
		debug_output("make_backup: Cannot Open Folder: '%s'. Please create one.\n", dirct);
		debug_output("make_backup: Backup has fail!\n");
		fclose(guild_members);
		return 1;
	}
	else {
		bak = fopen("save/backup/guild_members.txt.bak", "wt");
		fflush(bak);
		filecopy(guild_members, bak);
		fclose(bak);
	}
	fclose(guild_members);

	/*
	 * Party Backup
	 */

	party_members = fopen("save/parties/party_members.txt", "rt");
	if (!party_members) {
		debug_output("make_backup: Cannot open file 'save/parties/party_members.txt'.\n");
		debug_output("make_backup: Backup has fail!\n");
		return 1;
	}
	else if (!(backup = opendir(dirct))) {
		debug_output("make_backup: Cannot Open Folder: '%s'. Please create one.\n", dirct);
		debug_output("make_backup: Backup has fail!\n");
		fclose(party_members);
		return 1;
	}
	else {
		bak = fopen("save/backup/party_members.txt.bak", "wt");
		fflush(bak);
		filecopy(party_members, bak);
		fclose(bak);
	}
	fclose(party_members);

	/****************************************************************************/

	parties = fopen("save/parties/parties.txt", "rt");
	if (!parties) {
		debug_output("make_backup: Cannot open file 'save/parties/parties.txt'.\n");
		debug_output("make_backup: Backup has fail!\n");
		return 1;
	}
	else if (!(backup = opendir(dirct))) {
		debug_output("make_backup: Cannot Open Folder: '%s'. Please create one.\n", dirct);
		debug_output("make_backup: Backup has fail!\n");
		fclose(parties);
		return 1;
	}
	else {
		bak = fopen("save/backup/parties.txt.bak", "wt");
		fflush(bak);
		filecopy(parties, bak);
		fclose(bak);
	}
	fclose(parties);

	/*
	 * Pets backup 
	 */

	pets = fopen("save/pets/pet.txt", "rt");
	if (!pets) {
		debug_output("make_backup: Cannot open file 'save/pets/pet.txt'.\n");
		debug_output("make_backup: Backup has fail!\n");
		return 1;
	}
	else if (!(backup = opendir(dirct))) {
		debug_output("make_backup: Cannot Open Folder: '%s'. Please create one.\n", dirct);
		debug_output("make_backup: Backup has fail!\n");
		fclose(pets);
		return 1;
	}
	else {
		bak = fopen("save/backup/pet.txt.bak", "wt");
		fflush(bak);
		filecopy(pets, bak);
		fclose(bak);
	}
	fclose(pets);
	return 0;
}
#endif
