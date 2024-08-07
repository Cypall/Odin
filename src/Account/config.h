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

extern int closesrv;
extern int secure_mode;

int make_backup(void);
void reload_config(void);

int make_backup(void)
{
	char *dirct = "save/backup";
	FILE *bak, *acc;
	DIR *backup;

	acc = fopen("save/accounts/accounts.txt", "rt");
	if (!acc) {
		debug_output("make_backup: Cannot open file 'save/accounts/accounts.txt'.\n");
		debug_output("make_backup: Backup has fail!\n");
		return 1;
	}
	else if (!(backup = opendir(dirct))) {
		debug_output("make_backup: Cannot Open Folder: '%s'. Please create one.\n", dirct);
		debug_output("make_backup: Backup has fail!");
		fclose(acc);
		return 1;
	}
	else {
		bak = fopen("save/backup/accounts.txt.bak", "wt");
		filecopy(acc, bak);
		fclose(bak);
	}
	closedir(backup);
	fclose(acc);
	return 0;
}

void reload_config(void)
{
	int i;
	char line[1024], option1[1024], option2[1024];
	FILE *fp = NULL;

	if (!(fp = fopen("cfg/account_cfg.ini", "rt"))) {
		debug_output("reload_config: Cannot open file 'cfg/account_cfg.ini'.\n");
		abort();
	}
	else {
		while (fgets(line, 1023, fp)) {
			i = sscanf(line,"%[^=]=%s", option1, option2);
			if (i != 2)
				continue;

			if (strcmp(option1, "SecureMode") == 0) {
				if ((int)strtol(option2, (char**)NULL, 10) == 1)
					secure_mode = 1;

				else
					secure_mode = 0;

			}
		}
		fclose(fp);
	}
}
#endif
