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
 Project:       Project Odin Account Server
 Creation Date: Dicember 6, 2003
 Modified Date: October 20, 2004
 Description:   Ragnarok Online Server Emulator
------------------------------------------------------------------------*/

extern int closesrv;
extern int secure_mode;
extern int allow_gm_mode;

/*======================================
 *	CONFIG: Set Backup
 *--------------------------------------
 */

int make_backup(void)
{
	char *dirct = "save/backup";
	struct timeval tv;
	char tmpstr[256];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	FILE *bak, *acc;
	DIR *backup;

	acc = fopen("save/accounts/accounts.txt", "r");
	if (!acc) {
		fprintf(stderr,"ERROR: %s : Backup Not Succed!\n", tmpstr);
		fclose(acc);
		return 1;
	}
	else if (!(backup = opendir(dirct))) {
		fprintf(stderr,"ERROR: %s : Cannot Open Folder: %s\n", tmpstr, dirct);
		closedir(backup);
		fclose(acc);
		return 1;
	}
	else {
		bak = fopen("save/backup/accounts.txt.bak", "w");
		filecopy(acc, bak);
		fclose(bak);
	}
	fclose(acc);
	return 0;
}

/*======================================
 *	CONFIG: Reload Configuration
 *--------------------------------------
 */

void reload_config(void)
{
	int i;
	char line[MAX_LINE], option1[MAX_LINE], option2[MAX_LINE];
	struct timeval tv;
	char tmpstr[256];
	gettimeofday(&tv, NULL);
	strftime(tmpstr, 24, "%d/%m/%Y %I:%M:%S", localtime(&(tv.tv_sec)));
	FILE *fp = NULL;

	if (!(fp = fopen("config.ini", "r"))) {
		fprintf(stderr, "ERROR: %s : Access Violation\n", tmpstr);
		sleep(2);
		abort();
	}
	else {
		while (fgets(line, 1023, fp)) {
			i = sscanf(line,"%[^=]=%s", option1, option2);
			if (i != 2) {
				continue;
			}
			if (strcmp(option1, "CloseServer") == 0) {
				if ((int)strtol(option2, (char**)NULL, 10) == 1) {
					closesrv = 1;
				}
				else {
					closesrv = 0;
				}
			}
			else if (strcmp(option1, "SecureMode") == 0) {
				if ((int)strtol(option2, (char**)NULL, 10) == 1) {
					secure_mode = 1;
				}
				else {
					secure_mode = 0;
				}
			}
			else if (strcmp(option1, "AllowGMReg") == 0) {
				if ((int)strtol(option2, (char**)NULL, 10) == 1) {
					allow_gm_mode = 1;
				}
				else {
					allow_gm_mode = 0;
				}
			}
		}
		fclose(fp);
	}
}
