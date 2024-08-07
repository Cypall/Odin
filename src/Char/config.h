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

/*======================================
 *	CHARACTER: Set Backup
 *--------------------------------------
 */

int make_backup(void)
{
	char *dirct = "save/backup";
	struct timeval tv;
	char tmpstr[MAX_BUFFER];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	FILE *bak, *players, *storage, *guild_data, *party_members, *parties,
	*guild_members, *pets;
	DIR *backup;

	/*
	 * Character Backup
	 */

	players = fopen("save/characters/characters.txt", "r");
	/* Check for valid character file */
	if (players == NULL) {
		fprintf(stderr, "ERROR: %s : Backup Not Succed!\n", tmpstr);
		fclose(players);
		return 1;
	}
	/* Check for folder */
	else if ((backup = opendir(dirct)) == NULL) {
		fprintf(stderr,"ERROR: %s : Cannot Open Folder: %s\n", tmpstr, dirct);
		closedir(backup);
		fclose(players);
		return 1;
	}
	/* Do the backup */
	else {
		bak = fopen("save/backup/characters.txt.bak", "w");
		fflush(bak);
		filecopy(players, bak);
		fclose(bak);
	}
	fclose(players);

	/* 
	 * Storage Backup
	 */

	storage = fopen("save/storages/storage.txt", "r");
	if (storage == NULL) {
		fprintf(stderr,"ERROR: %s : Backup Not Succed!\n", tmpstr);
		fclose(storage);
		return 1;
	}
	/* Check for folder */
	else if ((backup = opendir(dirct)) == NULL) {
		fprintf(stderr,"ERROR: %s : Cannot Open Folder: %s\n", tmpstr, dirct);
		closedir(backup);
		fclose(storage);
		return 1;
	}
	/* Do the backup */
	else {
		bak = fopen("save/backup/storage.txt.bak", "w");
		fflush(bak);
		filecopy(storage, bak);
		fclose(bak);
	}
	fclose(storage);

	/*
	 * Guilds Backup
	 */

	guild_data = fopen("save/guilds/guild_data.txt", "r");
	if (guild_data == NULL) {
		fprintf(stderr,"ERROR: %s : Backup Not Succed!\n", tmpstr);
		fclose(guild_data);
		return 1;
	}
	/* Check for folder */
	else if ((backup = opendir(dirct)) == NULL) {
		fprintf(stderr,"ERROR: %s : Cannot Open Folder: %s\n", tmpstr, dirct);
		closedir(backup);
		fclose(guild_data);
		return 1;
	}
	/* Do the backup */
	else {
		bak = fopen("save/backup/guild_data.txt.bak", "w");
		fflush(bak);
		filecopy(guild_data, bak);
		fclose(bak);
	}
	fclose(guild_data);
	/****************************************************************************/
	guild_members = fopen("save/guilds/guild_members.txt", "r");
	if (guild_members == NULL) {
		fprintf(stderr,"ERROR: %s : Backup Not Succed!\n", tmpstr);
		fclose(guild_members);
		return 1;
	}
	/* Check for folder */
	else if ((backup = opendir(dirct)) == NULL) {
		fprintf(stderr,"ERROR: %s : Cannot Open Folder: %s\n", tmpstr, dirct);
		closedir(backup);
		fclose(guild_members);
		return 1;
	}
	/* Do the backup */
	else {
		bak = fopen("save/backup/guild_members.txt.bak", "w");
		fflush(bak);
		filecopy(guild_members, bak);
		fclose(bak);
	}
	fclose(guild_members);

	/*
	 * Party Backup
	 */

	party_members = fopen("save/parties/party_members.txt", "r");
	if (party_members == NULL) {
		fprintf(stderr,"ERROR: %s : Backup Not Succed!\n", tmpstr);
		fclose(party_members);
		return 1;
	}
	/* Check for folder */
	else if ((backup = opendir(dirct)) == NULL) {
		fprintf(stderr,"ERROR: %s : Cannot Open Folder: %s\n", tmpstr, dirct);
		closedir(backup);
		fclose(party_members);
		return 1;
	}
	/* Do the backup */
	else {
		bak = fopen("save/backup/party_members.txt.bak", "w");
		fflush(bak);
		filecopy(party_members, bak);
		fclose(bak);
	}
	fclose(party_members);

	/****************************************************************************/
	parties = fopen("save/parties/parties.txt", "r");
	if (parties == NULL) {
		fprintf(stderr,"ERROR: %s : Backup Not Succed!\n", tmpstr);
		fclose(parties);
		return 1;
	}
	/* Check for folder */
	else if ((backup = opendir(dirct)) == NULL) {
		fprintf(stderr,"ERROR: %s : Cannot Open Folder: %s\n", tmpstr, dirct);
		closedir(backup);
		fclose(parties);
		return 1;
	}
	/* Do the backup */
	else {
		bak = fopen("save/backup/parties.txt.bak", "w");
		fflush(bak);
		filecopy(parties, bak);
		fclose(bak);
	}
	fclose(parties);

	/*
	 * Pets backup 
	 */

	pets = fopen("save/pets/pet.txt", "r");
	if (pets == NULL) {
		fprintf(stderr,"ERROR: %s : Backup Not Succed!\n", tmpstr);
		fclose(pets);
		return 1;
	}
	/* Check for folder */
	else if ((backup = opendir(dirct)) == NULL) {
		fprintf(stderr,"ERROR: %s : Cannot Open Folder: %s\n", tmpstr, dirct);
		closedir(backup);
		fclose(pets);
		return 1;
	}
	/* Do the backup */
	else {
		bak = fopen("save/backup/pet.txt.bak", "w");
		fflush(bak);
		filecopy(pets, bak);
		fclose(bak);
	}
	fclose(pets);
	return 0;
}
