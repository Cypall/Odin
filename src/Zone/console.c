/* log of message area */
void send_msg_console(char *string)
{
  FILE *temp;
  temp=fopen("save/console/temp.txt","a");
  if(temp){
    fprintf(temp,"%s\n",string);
    fclose(temp);
  }
}

void console_cmd(int fd){
	/* This function handles console commands.
	------------------------------------------------------------------------------------\
			Declaration of variables used in console
	--------------------------------------------------------------------------------------\
	 Commented variables will be needed, but not used yet.*/
		//int x = 0, y = 0, z = 0, len;
		int x = 0, y = 0, len = 0;
		int i = 0;
		int num_scanf_args = 0;
		//int count = 0;

		// files variables
		//FILE *file;

		// varibales used to hold commands.
		char command[70];								// Holds the actual command
		char temp[70], temp0[70], temp1[70], moji[200]; // Holds the command arguments
		char *message;						// Holds messages

	//	unsigned char buf[256];

		// item variables
	//	struct item tmp_item; // Holds an item's data temporarily

		// stores the data of a player
		struct map_session_data *csd = NULL;
		// the ID of a player
	//	int cfd;

	/****************************Init Vars****************************/
		strcpy(temp, "");
		strcpy(temp0, "");
		strcpy(temp1, "");
		strcpy(command,RFIFOP(fd,10));
		message = strchr(RFIFOP(fd,10), 177); 

		if (strncmp(command,"!",1) == 0)
			message = strchr(RFIFOP(fd,10),'!'); // ! type command !shout etc

		if (strncmp(command,"/",1) == 0)
			message = strchr(RFIFOP(fd,10),'/'); // /b etc command

		sscanf(message,"%s",command);	// Gets current command, stores in char command[70].
		strtolower(command); 
		len = 4;
	/******************************************************************/
	if (command[0] == '!' || command[0] == '/') {
		/*------------------------------------------------------------------------
		Command:       !broadcast <text> || /b
		Parameters:     <text>, message displayed.
		Purpose:        Broadcasts a message to all players (annon).
		------------------------------------------------------------------------*/
		if (	strncmp(command,"!global",7) == 0 ||
				strncmp(command,"!broadcast",10) == 0 ||
				(strncmp(command,"/b",2) == 0 && strncmp(command,"/bn",3) != 0 )) {

			num_scanf_args = sscanf(message,"%s%[^\n]",command,temp0);

			if(num_scanf_args != 1){
				strcpy(moji,temp0);

				printf("moji: %s\n", moji);

				x = strlen(message) + len;
				WFIFOW(fd,0) = 0x9a;
				WFIFOW(fd,2) = x;
				strcpy(WFIFOP(fd,4),moji);
				mmo_map_sendall(fd, WFIFOP(fd,0), WFIFOW(fd,2), 0);
				return;
			}
		}
		/*------------------------------------------------------------------------
		Command:       !shout <text> || /bn
		Parameters:     <text>, message displayed.
		Purpose:        Broadcasts a message to all players (prefixed by [server]).
		------------------------------------------------------------------------*/
		if (strncmp(command,"!shout",6) == 0 || strncmp(command,"/bn",3) == 0 ) {

			num_scanf_args = sscanf(message,"%s%[^\n]",command,temp0);

			if(num_scanf_args != 1){
				strcpy(moji,"Server Operator: ");
				strcat(moji,temp0);

				printf("moji: %s\n", moji);

				x = strlen(message) + len + 17;
				WFIFOW(fd,0) = 0x9a;
				WFIFOW(fd,2) = x;
				strcpy(WFIFOP(fd,4),moji);
				mmo_map_sendall(fd, WFIFOP(fd,0), WFIFOW(fd,2), 0);
				return;
			}
		}
		/*------------------------------------------------------------------------
		Command:       !who [gm|others|all|<text>]
		Parameters:     [gm|others|all|<text>], which type of uyser to display
				        or a part of a player name to look up.
		Purpose:        Lists players who are currently online,
				        the map they're on and their coordinates (x & y).
		------------------------------------------------------------------------*/
		if (strncmp(command, "!who", 4) == 0 || strncmp(command, "/who", 4) == 0) {
			num_scanf_args = sscanf(message, "%s %s", command, temp);
			if (num_scanf_args == 1)
				strcpy(temp, "all"); // default, [all], show all players

			strtolower(temp);
				                                // (1: others, 2: gm, 3: all, 4: <text>) for below.
			if (strcmp(temp, "others") == 0) // asked for all players except (sub-)GM
				x = 1;
			else if (strcmp(temp, "gm") == 0) // asked for (sub-)GM players
				x = 2;
			else if (strcmp(temp, "all") == 0) // asked for all players
				x = 3;
			else // asked for <text> in name of players
				x = 4;


			y = 0; // count the number of displayed players
			send_msg_console("<who>");
			for (i = 0; i < FD_SETSIZE; i++) {
				if (session[i] != NULL && session[i]->session_data != NULL) {
					csd = session[i]->session_data; // get structure of the player
					strcpy(temp0, csd->status.name); // copy, otherwise str_lower modify source
					if ((x == 2 && csd->account_id > 95000) || // if asked for (sub-)GM
						(x == 1 && csd->account_id <= 95000) || // if asked for normal players
						(x == 4 && strstr(strtolower(temp0), temp) != NULL) || // if asked for <text> in name of players
						(x == 3)) { // if asked for all players
						if (y == 0) { // first display, we display a title
							if (x == 1) { // asked for all players except (sub-)GM
								send_msg_console("Players (except GM) currently online:");
							} else if (x == 2) { // asked for (sub-)GM players
								send_msg_console("(Sub-)GM currently online:");
							} else if (strcmp(temp, "all") == 0) { // asked for all players
								send_msg_console("People currently online:");
							} else { // asked for <text> in name of players
								sprintf(temp0, "People that have '%s' in their name:", temp);
								send_msg_console(temp0);
							}
						}
						y++; // count the number of displayed players
						if (csd->account_id > 100000) // if asked for GM, test to write: (GM)
							sprintf(temp0, " %s (GM): %s (%i, %i)", csd->status.name, csd->mapname, csd->x, csd->y);

						else if (csd->account_id > 95000) // if asked for GM, test to write: (Sub-GM)
							sprintf(temp0, " %s (Sub-GM): %s (%i, %i)", csd->status.name, csd->mapname, csd->x, csd->y);

						else // if asked for normal players (we write nothing)
							sprintf(temp0, " %s: %s (%i, %i)", csd->status.name, csd->mapname, csd->x, csd->y);

						if (csd->status.class >= 0 && csd->status.class <= 20) {
							sprintf(temp1, " - (Job: %d)", csd->status.class);
							strcat(temp0, temp1);
						} else {
							strcat(temp0, " - (Unknown job)");
						}
						send_msg_console(temp0);
					}
				}
			}
			// prep temp0 for display.
			if (x == 1) {
				if (y == 0)
					strcpy(temp0, "No non-GM players found.");
				else if (y == 1)
					strcpy(temp0, "One non-GM player found.");
				else
					sprintf(temp0, "%i non-GM players found.", y);
			} else if (x == 2) {
				if (y == 0)
					strcpy(temp0, "No GMs found.");
				else if (y == 1)
					strcpy(temp0, "One gm found.");
				else
					sprintf(temp0, "%i GMs found.", y);
			} else if (x == 3) {
				if (y == 0)
					strcpy(temp0, "No players found."); // normally, impossible (the GM does the command)
				else if (y == 1)
					strcpy(temp0, "One player found.");
				else
					sprintf(temp0, "%i players found.", y);
			} else {
				if (y == 0)
					sprintf(temp0, "Nobody has '%s' in its name.", temp);
				else if (y == 1)
					sprintf(temp0, "One player has '%s' in its name.", temp);
				else
					sprintf(temp0, "%i players have '%s' in their name.", y, temp);
			}
			send_msg_console(temp0);
			send_msg_console("</who>");
		}
		/* end commands */
	}
}
// main console func.
int parse_console(int fd, int console_enabled, char *c_pswd){
	char temp[24];

	if(RFIFOW(fd,0) == 177) {
		strcpy(temp,RFIFOP(fd,2));
		if((console_enabled == 1) && (strcmp(temp, c_pswd)==0))
		{
			console_cmd(fd); // handles console commands.
		}else{
			printf("[Caution] Possible server attack, attempt to login as \n\tOdin Server Control Console, with wrong password.\n");
		}
		RFIFOSKIP(fd,80);
		return 1;
	}else{
		return 0;
	}
	return 0;
}
//
