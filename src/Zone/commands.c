/*--------------------------------------------------------------------------------------
 * Commands	List |||||||||||||||||||||||||||||||||||||||||||||||||||||||||<
 * -------------------------------------------------------------------------------------
 * Commands *		* Sub-GM Commands *						* GM Commands *
 * -------- /		  --------------- /						  ----------- /
 * !unstuck			  !heal [hp] [sp]						  !goto <0-12>/<cityname>
 * !help			  !nopain								  !summon <player>
 * !save		      !shout <text>							  !baseup <lvl> [player]
 * !exp	              !broadcast <text>						  !jobchange <0-12/20> [player]
 *                    !jumpto <player>						  !jobup <lvl> [player]
 *				      !kick <player>						  !giveskillp <amount> [player]
 *				      !revive <player>						  !givestatp <amount> [player]
 *				      !killmob								  !givez <amount> [player]
 *				      !spawn <id> [num]						  !item <id> [quantity]
 *				      !msearch <id>/<name>					  !isearch <id>/<name>
 *				      !warp <map> <x> <y>					  !pvp [on/off]
 *				      !who								      !warpchar <map> <x> <y> [player]
 *					  								          !sprite <0-18> <0-8> <0-4> [player]
 *													          !hide [player]((disabled))
 *					  								          !status <param1> <param2> <param3>
 *					  								          !speed <1-140>
 *
 *
 *
------------------------------------------------------------------------------------*/
int is_item(int object)
{
	char str[80];
	int item_id = 0,result = 0;
	FILE *fp;

	fp = fopen("data/databases/item_db.txt", "r");
	if (fp == NULL) {
		return 0;
	}
	while (fgets(str, 80, fp) != NULL) {
		sscanf(str, "%d", &item_id);
		if (item_id == object) {
			result = 1;
			break;
		}
	}
	fclose(fp);
	return result;
}

int is_monster(int object)
{
	char str[80];
	int item_id = 0,result = 0;
	FILE *fp;

	fp = fopen("data/databases/monster_db.txt", "r");
	if (fp == NULL) {
		return 0;
	}
	while (fgets(str, 80, fp) != NULL) {
		sscanf(str, "%d", &item_id);
		if (item_id == object) {
			result = 1;
			break;
		}
	}
	fclose(fp);
	return result;
}

int find_char(char *cname)
{
	int i;
	struct map_session_data *sd;

	for (i = 5; i < FD_SETSIZE; i++) {
		if ((session[i] != NULL) && (session[i]->session_data != NULL)) {
			sd = session[i]->session_data;
			if (strncmp(sd->status.name, cname, 24) == 0) {
				return i;
			}
		}
	}
	return -1;
}

int parse_commands(int fd){
/*-----------------------------------------------------------------------------------
		Declaration of variables used in commands
--------------------------------------------------------------------------------------

			general variables*/
		int x = 0, y = 0, z = 0;  // Define parameters for commands.
		int i = 0;                // Define integer for 'for()' loops.
		int num_scanf_args = 0;   // Define integer to get number of found arguments in sscanf.
		int len = 0, count = 0;   // Used to count things.

		// files variables
		FILE *file;				// Used in commands that require the opening of files.

		// varibales used to hold commands.
		char command[70];								// Holds the actual command
		char temp[70], temp0[70], temp1[70], moji[200]; // Holds the command arguments
		char *iscommand, *message;						// Holds messages

		unsigned char buf[256];

		// item variables
		struct item tmp_item; // Holds an item's data temporarily

		// stores the data of a player other than the GM, in this struct
		struct map_session_data *csd = NULL;
		// the ID of a player other than GM
		int cfd;

		// point to session data
		struct map_session_data *sd;
		sd = session[fd]->session_data;

		// Just makes it a little easier... use 'p->'.
		struct mmo_charstatus *p;
		p = &sd->status;


	/****************************Init Vars****************************/
		strcpy(temp, "");
		strcpy(temp0, "");
		strcpy(temp1, "");
		iscommand = strchr(RFIFOP(fd,4)+strlen(sd->status.name),':');
		message = strchr(RFIFOP(fd,4)+strlen(sd->status.name),'!');

		len = strlen(sd->status.name) + 4;
	/*****************************************************************/

		// Check to determine if typed message is a command.
		if (iscommand[2] == '!') {

			sscanf(message,"%s",command);	// Gets current command, stores in char command[70].
			strtolower(command); 
		/*------------------------------------------------------------------------------------
			These commands can be used by all players
		--------------------------------------------------------------------------------------

			Command:       !unstuck
			Parameters:     None
			Purpose:        Frees you if/when you're stuck in a wall.
			------------------------------------------------------------------------*/
			if (strcmp(command,"!unstuck") == 0) {
				int tmpa = 0;
				int curx = sd->x;
				int cury = sd->y;
				int range = 0;
				if (map_data[sd->mapno].gat[sd->x+sd->y*map_data[sd->mapno].xs]==1 || map_data[sd->mapno].gat[sd->x+sd->y*map_data[sd->mapno].xs]==5) {
					do {
						srand((unsigned)time(NULL));
						range = tmpa + 1;
						sd->x = (rand()%range) - (range/2) + curx;
						sd->y = (rand()%range) - (range/2) + cury;
						tmpa++;
						if (sd->x == 0) sd->x = 1;
						if (sd->y == 0) sd->y = 1;
						if (sd->x < 0) sd->x = sd->x - (sd->x*2);
						if (sd->y < 0) sd->y = sd->y - (sd->y*2);
					}
					while ((map_data[sd->mapno].gat[sd->x+sd->y*map_data[sd->mapno].xs]==1 || map_data[sd->mapno].gat[sd->x+sd->y*map_data[sd->mapno].xs]==5) && (tmpa < 300));
					mmo_map_changemap(sd,sd->mapname,sd->x,sd->y,2);
					send_msg_p(fd, "!unstuck", "Unstuck!");
				}else{
					send_msg_p(fd, "!unstuck", "You're not stuck.");
				}
				return 1;
			}
			/*------------------------------------------------------------------------
			Command:       !help
			Parameters:     None
			Purpose:        Displays contents of help file.
			------------------------------------------------------------------------*/
			if (strcmp(command,"!help") == 0 || strcmp(command,"!h") == 0) {
				sscanf(message,"%s",command);
				i = 0;
				file = NULL; // init
				if (sd->account_id > 100000){
					i = 3;
				}else if (sd->account_id > 95000){
					i = 2;
				}else{
					i = 1;
				}
				while(i > 0)
				{
					if(i == 3)
						file = fopen("data/help/help_gm.txt","r");
					if(i == 2)
						file = fopen("data/help/help_subgm.txt","r");
					if(i == 1)
						file = fopen("data/help/help.txt","r");
					if(file!=NULL){
						send_msg_p(fd, "!help", "Help Commands");
						send_msg_p(fd, " ", " "); // blank line //

						while (fgets(moji,380,file) != NULL) {
							sprintf(temp, "%s", moji); // the string must be have 31 chars for align (in columns)
							strncpy(temp0, temp, 75);
							send_msg_p(fd, " ", temp0);
							strncpy(temp0, "", 75);
						}
					}
					i--;
				}
				fclose(file);
				return 1;
			}
			/*------------------------------------------------------------------------
			Command:       !save
			Parameters:     None
			Purpose:        Saves character's data manually.
			------------------------------------------------------------------------*/
			if (strcmp(command, "!save") == 0 || strcmp(command, "!sve") == 0) {

				strcpy(p->last_point.map, sd->mapname);
				p->last_point.x = sd->x;
				p->last_point.y = sd->y;
				mmo_char_save(sd);
				send_msg_p(fd, "!save", "Status and Inventory saved...");
				return 1;
			}
			/*------------------------------------------------------------------------
			Command:       !exp
			Parameters:     None
			Purpose:        Displays how much experience is needed
							to reach the next level.
			------------------------------------------------------------------------*/
			if (strcmp(command, "!exp") == 0) {

				if (p->class == 0) {								// Class 0
					next_job_exp = SkillExpData[0][p->job_level];
				} else if (p->class > 0 && p->class < 7) {			// Classes 1 -> 6
					next_job_exp = SkillExpData[1][p->job_level];
				} else if (p->class > 6 && p->class < 21) {			// Classes 7 -> 12
					next_job_exp = SkillExpData[2][p->job_level];
				}
				sprintf(temp, "EXP needed until next level: Base: [%ld] Job: [%ld]", ExpData[p->base_level] - p->base_exp, next_job_exp - p->job_exp);
				send_msg_p(fd, "!exp", temp);
				return 1;
			}
		/*------------------------------------------------------------------------------------
		These are the subGM commands, only for accounts with an account ID of 95000+
		------------------------------------------------------------------------------------*/
			if (sd->account_id > 95000) {
					/*------------------------------------------------------------------------
					Command:       !kill
					Parameters:     
					Purpose:        
					------------------------------------------------------------------------*/
					if ((strcmp(command, "!kill") == 0 || strcmp(command, "!kll") == 0 ||
						strcmp(command, "!suicide") == 0 || strcmp(command, "!sucide") == 0 ||
						strcmp(command, "!siucide") == 0 || strcmp(command, "!suicd") == 0 ||
						strcmp(command, "!die") == 0 || strcmp(command, "!dei") == 0) 
						&& // checks to make sure its not another command
						(strcmp(command,"mob") != 0 && strcmp(command,"mb") != 0)	) {

						num_scanf_args = sscanf(message, "%s %[^\n]", command, temp);
						if (sd->account_id > 100000){ // check to see if it should be handled by !doom instead
							if(strcmp(temp, "map") == 0 || strcmp(command, "map") == 0 || strcmp(temp, "all") == 0){
								strcpy(command, "!doom");
								z = 1;
							}else{
								cfd = find_char(temp);
								if (cfd == -1){
									z = 1;
								}
							}
						}
						if (num_scanf_args == 1 || sd->account_id < 100000) {
							strcpy(temp, p->name);
						}
						if(z < 1){
							cfd = find_char(temp);
							if (cfd == -1) {
								sprintf(temp0, "Could not find the player ('%s').", temp);
								send_msg_p(fd, command, temp0);
							} else {
								csd = session[cfd]->session_data;
								csd->status.hp = 0;
								csd->sitting = 1;
								WFIFOW(cfd, 0) = 0xb0; // R 00b0 <type>.w <val>.l: update values of various status (..0005:HP..)
								WFIFOW(cfd, 2) = 0005;
								WFIFOL(cfd, 4) = csd->status.hp;
								WFIFOSET(cfd, packet_len_table[0xb0]);
								WBUFW(buf, 0) = 0x80; // R 0080 <ID>.l <type>.B: type = 01 (character died)
								WBUFL(buf, 2) = csd->account_id;
								WBUFB(buf, 6) = 1;
								mmo_map_sendarea(cfd, buf, packet_len_table[0x80], 0);
								send_msg_p(fd, command, "The mark of the dead...");
							}
							return 1;
						}
					}
					/*------------------------------------------------------------------------
					Command:       !heal [hp] [sp]
					Parameters:     [hp] [sp]
					Purpose:        Heals specified HP/SP points, or all if not specified.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!heal") == 0 || strcmp(command,"!hael") == 0) {
						num_scanf_args = sscanf(message,"%s%d%d",command,&x,&y);
							if (x == 0 && y == 0) {
								WFIFOW(fd,0) = 0x13d;
								WFIFOW(fd,2) = 5;
								WFIFOW(fd,4) = p->max_hp - p->hp;
								WFIFOSET(fd,6);

								WFIFOW(fd,0) = 0x13d;
								WFIFOW(fd,2) = 7;
								WFIFOW(fd,4) = p->max_sp - p->sp;
								WFIFOSET(fd,6);
								p->hp = p->max_hp + 1;
								p->sp = p->max_sp + 1;
							}
							else if (x >= -10000 && x <= 10000 && y>= -10000 && y <= 10000) {
								p->hp += x;
								p->sp += y;
								WFIFOW(fd,0) = 0x13d;
								WFIFOW(fd,2) = 5;
								WFIFOW(fd,4) = x;
								WFIFOSET(fd,6);

								WFIFOW(fd,0) = 0x13d;
								WFIFOW(fd,2) = 7;
								WFIFOW(fd,4) = y;
								WFIFOSET(fd,6);
							}
							if (p->hp > p->max_hp || p->hp < 0) {
								p->hp = p->max_hp;
								WFIFOW(fd,0) = 0xb0;
								WFIFOW(fd,2) = 0005;
								WFIFOL(fd,4) = p->hp;
								WFIFOSET(fd,8);
							}
							if (p->sp > p->max_sp || p->sp < 0) {
								p->sp = p->max_sp;
								WFIFOW(fd,0) = 0xb0;
								WFIFOW(fd,2) = 0007;
								WFIFOL(fd,4) = p->sp;
								WFIFOSET(fd,8);
							}

							WFIFOW(fd,0) = 0xb0;
							WFIFOW(fd,2) = 0005;
							WFIFOL(fd,4) = p->hp;
							WFIFOSET(fd,8);
							WFIFOW(fd,0) = 0x148;
							WFIFOL(fd,2) = p->account_id;
							mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x148], 0);
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !shout <text>
					Parameters:     <text>, message displayed.
					Purpose:        Broadcasts a message to all players
									(prefixed with GM's character's name).
					------------------------------------------------------------------------*/
					if (strcmp(command,"!kami") == 0 || strcmp(command,"!announce") == 0 ||
						strcmp(command,"!anounce") == 0 || strcmp(command,"!shuot") == 0 ||
						strcmp(command,"!shout") == 0) {

						num_scanf_args = sscanf(message,"%s%[^\n]",command,temp0);

						if(num_scanf_args != 1){
							strcpy(moji,sd->status.name);
							strcat(moji," :");
							strcat(moji,temp0);
							x = strlen(message) + len;
							WFIFOW(fd,0) = 0x9a;
							WFIFOW(fd,2) = x;
							strcpy(WFIFOP(fd,4),moji);
							mmo_map_sendall(fd, WFIFOP(fd,0), WFIFOW(fd,2), 0);
							//printf("%s shouted.", temp0);
						}else{
							send_msg_p(fd, "Command Library", "Usage: !shout <text>");
						}
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !broadcast <text>
					Parameters:     <text>, message displayed.
					Purpose:        Broadcasts a message to all players (annon).
					------------------------------------------------------------------------*/
					if (strcmp(command,"!global") == 0 || strcmp(command,"!broadcast") == 0 ||
						strcmp(command,"!brodcast") == 0 || strcmp(command,"!bradcast") == 0 ||
						strcmp(command,"!broadcost") == 0 || strcmp(command,"!brodcost") == 0) {

						num_scanf_args = sscanf(message,"%s%[^\n]",command,temp0);

						if(num_scanf_args != 1){
							strcpy(moji,temp0);
							x = strlen(message) + len;
							WFIFOW(fd,0) = 0x9a;
							WFIFOW(fd,2) = x;
							strcpy(WFIFOP(fd,4),moji);
							mmo_map_sendall(fd, WFIFOP(fd,0), WFIFOW(fd,2), 0);
						}else{
							send_msg_p(fd, "Command Library", "Usage: !global <text>");
						}
						return 1; 
					}
					/*------------------------------------------------------------------------
					Command:       !kick <player>
					Parameters:     <player>, player's name.
					Purpose:        Boots a certain <player> off the server.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!kick") == 0 || strcmp(command,"!kik") == 0 ||
						strcmp(command,"!boot") == 0) {

						num_scanf_args = sscanf(message,"%s %[^\n]",command,temp);
						if(num_scanf_args != 1){
							cfd = find_char(temp);
							if (cfd == -1) {
								return 1;
							}
							else {
								struct map_session_data *sd=NULL;
								sd = session[cfd]->session_data;
								mmo_char_save(sd);
								if ((unsigned)cfd == char_fd) {
									char_fd = -1;
								}
								if (sd && sd->state.auth) {
									if (sd->chatID) {
										mmo_map_leavechat(cfd,(struct mmo_chat*)sd->chatID,sd->status.name);
									}
									del_block(&sd->block);
									WFIFOW(cfd,0) = 0x80;
									WFIFOL(cfd,2) = sd->account_id;
									WFIFOB(cfd,6) = 2;  //logout?
									mmo_map_sendarea(cfd,WFIFOP(cfd,0),packet_len_table[0x80], 1);
									map_data[sd->mapno].users--;
									if (char_fd > 0) {
										strncpy(sd->status.last_point.map,sd->mapname,16);
										sd->status.last_point.x = sd->x;
										sd->status.last_point.y = sd->y;

										WFIFOW(char_fd,0) = 0x2b01;
										WFIFOW(char_fd,2) = sizeof(sd->status)+12;
										WFIFOL(char_fd,4) = sd->account_id;
										WFIFOL(char_fd,8) = sd->char_id;
										memcpy(WFIFOP(char_fd,12),&sd->status,sizeof(sd->status));
										WFIFOSET(char_fd,WFIFOW(char_fd,2));
									}
								}
								close(cfd);
								delete_session(cfd);
								sprintf(temp1, "%s", "Player kicked");
							}
						}else{
							sprintf(temp1, "%s", "Usage: !kick <player>");
						}
						send_msg_p(fd, "!kick", temp1);
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:        !ban <player>
					Parameters:     <player>, player's name.
					Purpose:        Boots a certain <player> off the server and ban it.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!ban") == 0 || strcmp(command,"!block") == 0) {
						num_scanf_args = sscanf(message,"%s %[^\n]",command,temp);
						if(num_scanf_args != 1){
							cfd = find_char(temp);
							if (cfd == -1) {
								return 1;
							}
							else {
								struct map_session_data *sd=NULL;
								sd = session[cfd]->session_data;
								mmo_char_save(sd);
								if ((unsigned)cfd == char_fd) {
									char_fd = -1;
								}
								if (sd && sd->state.auth) {
									if (sd->chatID) {
										mmo_map_leavechat(cfd,(struct mmo_chat*)sd->chatID,sd->status.name);
									}
									del_block(&sd->block);
									WFIFOW(cfd,0) = 0x80;
									WFIFOB(cfd,2) = sd->account_id;
									WFIFOB(cfd,6) = 2;
									mmo_map_sendarea(cfd,WFIFOP(cfd,0),packet_len_table[0x80], 1);
									map_data[sd->mapno].users--;
									if (char_fd > 0) {
										memcpy(sd->status.last_point.map,sd->mapname,16);
										sd->status.last_point.x = sd->x;
										sd->status.last_point.y = sd->y;

										WFIFOW(char_fd,0) = 0x2b01;
										WFIFOW(char_fd,2) = sizeof(sd->status)+12;
										WFIFOL(char_fd,4) = sd->account_id;
										WFIFOL(char_fd,8) = sd->char_id;
										memcpy(WFIFOP(char_fd,12),&sd->status,sizeof(sd->status));
										WFIFOSET(char_fd,WFIFOW(char_fd,2));
									}
								}
								WFIFOW(char_fd, 0) = 0x30ff;
								WFIFOL(char_fd, 2) = sd->account_id;
								WFIFOSET(char_fd, 6);
								close(cfd);
								delete_session(cfd);
								sprintf(temp1, "%s", "Player kicked and banned");
							}
						}
						else {
							sprintf(temp1, "%s", "Usage: !ban <player>");
						}
						send_msg_p(fd, "!ban", temp1);
						return 1;
					}


					/*------------------------------------------------------------------------
					Command:       !nopain
					Parameters:     None.
					Purpose:        Makes Mobs ignore you.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!nopain") == 0 || strcmp(command,"!npain") == 0) {
						sscanf(message,"%s%d",command,&x);
						if (!sd->hidden) {
							sd->hidden = 1;
							sprintf(temp1,"%s", "You are unseen by monsters.");
						} else {
							sd->hidden = 0;
							sprintf(temp1,"%s", "You are seen by monsters.");
						}
						send_msg_p(fd, "!nopain", temp1);
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !killmob
					Parameters:     None
					Purpose:        Remove a monster close to you.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!killmob") == 0 || strcmp(command,"!kllmob") == 0 ||
						strcmp(command,"!killmb") == 0 || strcmp(command,"!kilmob") == 0) {
						sscanf(message,"%s",command);
						remove_monsters(sd->mapno,sd->x,sd->y,fd);
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !revive <player>
					Parameters:     <player>, player you wish to revive.
					Purpose:        Returns <player> to life.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!revive") == 0 || strcmp(command,"!rvive") == 0 ||
						strcmp(command,"!revve") == 0) {

						num_scanf_args = sscanf(message,"%s %[^\n]",command,temp);

						if (num_scanf_args == 1)
							strcpy(temp, p->name); // give GM's player name

						int cfd = find_char(temp);
						if (cfd != -1) {
							csd = session[cfd]->session_data;
							csd->status.hp = csd->status.max_hp;
							WFIFOW(cfd,0) = 0xb0;
							WFIFOW(cfd,2) = 0005;
							WFIFOL(cfd,4) = csd->status.hp;
							WFIFOSET(cfd,8);
							WFIFOW(cfd,0) = 0x148;
							WFIFOL(cfd,2) = csd->account_id;
							mmo_map_sendarea(cfd, WFIFOP(cfd,0), packet_len_table[0x148], 0);
						}
						return 1;
					}

					/*------------------------------------------------------------------------
					Command:       !msearch <id>/<name>
					Parameters:     <id>, The monster's ID.
									<name>, The monster's Name
					Purpose:        Searches the monster db, and returns
									the name (or id) of monster,with ID of <id>,
									or name of <name>.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!msearch") == 0 || strcmp(command,"!m_search") == 0){

						num_scanf_args = sscanf(message, "%s %s", command, temp);
						if (num_scanf_args != 1) {
							if (atoi(temp) >= 1 && atoi(temp) <= 2000) { // id given, search via #.
								x = atoi(temp);
								if (is_monster(x)) {
									sprintf(temp, "Monster: %i is a %s, level: %i.", x, mons_data[x].name, mons_data[x].lv);
								} else {
									sprintf(temp, "Id #%i isn't a monster.", x);
								}
									send_msg_p(fd, "!msearch", temp);
							}else{	// no id given, search using name.
								strtolower(temp);
								count = 0; // init
								strcpy(buf, ""); // init
								for (i = 1000; i < MAX_MONS; i++) {
									strcpy(temp0, mons_data[i].name);
									if (strstr(strtolower(temp0), temp) != NULL) {
										if (count == 0) {
											send_msg_p(fd, command, "List of the monsters:");
										}
										count++;
										sprintf(temp0, "%5d:%-25s", i, mons_data[i].real_name); // the string must be have 31 chars for align (in columns)
										strcat(buf, temp0);
										// work on 3 columns (max 94 chars per line: 31 * 3 = 93)
										if (count % 3 == 0) {
											send_msg_p(fd, " ", buf);
											strcpy(buf, ""); // initiliaze buf for new line
										}
									}
								}
								if (count == 0) {
									sprintf(temp0, "No monster has '%s' in its name, sorry.", temp);
									send_msg_p(fd, command, temp0);
								} else {
									if (count % 3 > 0) { // if last mon have not been displayed
										send_msg_p(fd, " ", buf);
									}
									if (count == 1) {
										sprintf(temp0, "One monster has '%s' in its name.", temp);
									} else {
										sprintf(temp0, "%i monsters have '%s' in their name.", count, temp);
									}
									send_msg_p(fd, " ", temp0);
								}
							}
						}else{
							send_msg_p(fd, "Command Library", "Usage: !msearch <id>/<name>");
						}
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !spawn <id|name> [num]
					Parameters:     <id|name>, The monster's ID or name. [num], Amounr to spawn.
					Purpose:        Spawns <number> monsters of <id>.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!spawn") == 0 || strcmp(command,"!spwn") == 0 ||
						strcmp(command,"!monster") == 0 || strcmp(command,"!mnster") == 0 ||
						strcmp(command,"!mnoster") == 0 || strcmp(command,"!monter") == 0 ||
						strcmp(command,"!spn") == 0 || strcmp(command,"!sawn") == 0 ||
						strcmp(command,"!monstr") == 0) {

						num_scanf_args = sscanf(message,"%s%s%d",command,temp,&y);
						if ((num_scanf_args == 2) || (y < 1)) {
							y = 1;
						}
						if(num_scanf_args != 1){
							if (atoi(temp) >= 1 && atoi(temp) <= 2000) { // id given, search via #.
								x = atoi(temp);
								if (is_monster(x)) {
									for (i=0;i<y;i++) {
										spawn_monster(x,sd->x,sd->y,sd->mapno,fd);
									}
								}
							}else{
								strtolower(temp);
								count = 0; // init
								strcpy(buf, ""); // init
								for (i = 1000; i < MAX_MONS; i++) {
									strcpy(temp0, mons_data[i].name);
									if (strstr(strtolower(temp0), temp) != NULL && count < 1) {
										x = i;
										if (is_monster(x)) {
											for (i=0;i<y;i++) {
												spawn_monster(x,sd->x,sd->y,sd->mapno,fd);
											}
										}
										count++;
									}
								}
								if (count == 0) {
									send_msg_p(fd, "Command Library", "Usage: !spawn <id|name> [num]");
								}
							}
						}else{
							send_msg_p(fd, "Command Library", "Usage: !spawn <id|name> [num]");
						}
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !warp <map> <x> <y>
					Parameters:     <map> <x> <y>
					Purpose:        Warps GM to the specified map at the specified coordinates.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!warp") == 0 || strcmp(command,"!wrp") == 0 ||
						strcmp(command,"!wap") == 0) {
						sscanf(message,"%s%s%d%d",command,temp,&x,&y);
						if (x>0 && x<325 && y>0 && y<325) {
							if (strstr(temp,".gat") == NULL) {
								strcat(temp,".gat");
							}
							for (i=0;map[i][0];i++) {
								if (strcmp(map[i],temp) == 0) {
									mmo_map_changemap(sd,temp,x,y,2);
									return 1;  
								}
							}
							return 1;  
						}else{
							send_msg_p(fd, "Command Library", "Usage: !warp <map> <x> <y>");
						}
						return 1; 
					}
					/*------------------------------------------------------------------------
					Command:       !jumpto <player>
					Parameters:     <player>, Player you wish to warp to.
					Purpose:        Warps the GM to the specified player.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!shift") == 0 ||
						strcmp(command, "!warp_to") == 0 || strcmp(command, "!warpto") == 0 ||
						strcmp(command, "!jump_to") == 0 || strcmp(command, "!jumpto") == 0) {
						sscanf(message,"%s %[^\n]",command,temp);

						int cfd = find_char(temp);
						if (cfd == -1) {
							sprintf(temp0, "Could not find the player ('%s').", temp);
							send_msg_p(fd,command, temp0);
							return 1; 
						}
						else {
							struct map_session_data *csd=NULL;
							csd = session[cfd]->session_data;
							mmo_map_changemap(sd,csd->mapname,csd->x,csd->y,2);
							return 1; 
						}
					}
					/*------------------------------------------------------------------------
					Command:       !hair color <color> [user]/!hair style <style> [user]
					Parameters:     <color>: Color of new hair
									<style>: Style of hair
									[user] : user who's hair to change

					Possible Values:	
					Purpose:        
					------------------------------------------------------------------------*/
					if (strcmp(command, "!hair") == 0 || strcmp(command, "!hiar") == 0 ) {

						num_scanf_args = sscanf(message, "%s %s %s %s", command, temp, temp0, temp1);
						if (num_scanf_args != 1) {
							if (num_scanf_args == 3 || sd->account_id <= 100000) {
								strcpy(temp1, p->name);
							}
							strtolower(temp);
							strtolower(temp0);
							strtolower(temp1);
							cfd = find_char(temp1);
							if (cfd == -1) {
								sprintf(temp0, "Could not find the player ('%s').", temp1);
								send_msg_p(fd,"!hair", temp0);
								return 1;
							}
							csd = session[cfd]->session_data;
							if(strcmp(temp, "color") == 0){						//change colour
								strtolower(temp0);
								if (strcmp(temp0, "blonde") == 0 || strcmp(temp0, "yellow") == 0){
									x = 1;
								}else if (strcmp(temp0, "purple") == 0 || strcmp(temp0, "violet") == 0){
									x = 2;
								}else if (strcmp(temp0, "brown") == 0){
									x = 3;
								}else if (strcmp(temp0, "green") == 0){
									x = 4;
								}else if (strcmp(temp0, "blue") == 0){
									x = 5;
								}else if (strcmp(temp0, "white") == 0){
									x = 6;
								}else if (strcmp(temp0, "black") == 0){
									x = 7;
								}else if (strcmp(temp0, "red") == 0){
									x = 0;
								}else if (atoi(temp0) >= 0 && atoi(temp0) <= 8) {
									x = atoi(temp0);
								}else{
									if (sd->account_id <= 100000) {
										send_msg_p(fd, "Command Library", "Usage: !hair color <0-8>");
									} else {
										send_msg_p(fd, "Command Library", "Usage: !hair color <0-8> [player]");
									}
									
									x = csd->status.hair_color;
								}
								csd->status.hair_color = x;
								

								// set hair colour
								WBUFW(buf, 0) = 0xc3; // R 00c3 <ID>.l <type>.B <val>.B: change looks. type = 01 Hair type.
								WBUFL(buf, 2) = csd->account_id;
								WBUFB(buf, 6) = 1;
								WBUFB(buf, 7) = csd->status.hair;
								mmo_map_sendarea(cfd, buf, packet_len_table[0xc3], 0);

								csd->status.hair_color = x;
								WBUFW(buf, 0) = 0xc3; // R 00c3 <ID>.l <type>.B <val>.B: change looks. type = 06 Hair color.
								WBUFL(buf, 2) = csd->account_id;
								WBUFB(buf, 6) = 6;
								WBUFB(buf, 7) = x;
								mmo_map_sendarea(cfd, buf, packet_len_table[0xc3], 0);

								WBUFW(buf, 0) = 0xc3; // R 00c3 <ID>.l <type>.B <val>.B: change looks. type = 07 Clothes color
								WBUFL(buf, 2) = csd->account_id;
								WBUFB(buf, 6) = 7;
								WBUFB(buf, 7) = csd->status.clothes_color;
								mmo_map_sendarea(cfd, buf, packet_len_table[0xc3], 0);

								send_msg_p(fd, "!hair", "Hair color changed.");
								
							}else if(strcmp(temp, "style") == 0){					//change style
								if (atoi(temp0) >= 0 && atoi(temp0) <= 18) {
									x = atoi(temp0);

									csd->status.hair = x;
									WBUFW(buf, 0) = 0xc3; // R 00c3 <ID>.l <type>.B <val>.B: change looks. type = 01 Hair type.
									WBUFL(buf, 2) = csd->account_id;
									WBUFB(buf, 6) = 1;
									WBUFB(buf, 7) = x;
									mmo_map_sendarea(cfd, buf, packet_len_table[0xc3], 0);

									WBUFW(buf, 0) = 0xc3; // R 00c3 <ID>.l <type>.B <val>.B: change looks. type = 06 Hair color.
									WBUFL(buf, 2) = csd->account_id;
									WBUFB(buf, 6) = 6;
									WBUFB(buf, 7) = csd->status.hair_color;
									mmo_map_sendarea(cfd, buf, packet_len_table[0xc3], 0);

									WBUFW(buf, 0) = 0xc3; // R 00c3 <ID>.l <type>.B <val>.B: change looks. type = 07 Clothes color
									WBUFL(buf, 2) = csd->account_id;
									WBUFB(buf, 6) = 7;
									WBUFB(buf, 7) = csd->status.clothes_color;
									mmo_map_sendarea(cfd, buf, packet_len_table[0xc3], 0);

									send_msg_p(fd, "!hair", "Hair style changed.");
								}else{
									if (sd->account_id <= 100000) {
										send_msg_p(fd, "Command Library", "Usage: !hair style <0-18>");
									} else {
										send_msg_p(fd, "Command Library", "Usage: !hair style <0-18> [player]");
									}
									
									return 1;
								}
							}
						}else{
							if (sd->account_id <= 100000) {
								send_msg_p(fd, "Command Library", "Usage: !hair color <1-8|color> / !hair style <1-18>");
							} else {
								send_msg_p(fd, "Command Library", "Usage: !hair color <1-8|color> [user] / !hair style <1-18> [user]");
							}
							
							return 1;
						}
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !sprite <0-18> <0-8> <0-4> [player]
					Parameters:     <0-18> Hair type,
									<0-8> hair color,
									<0-4> clothing color,
									[player] (optional), player to change.
					Purpose:        Changes sprite (in order) hair type, hair color
									and clothing color.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!model") == 0 || strcmp(command,"!mdel") == 0 ||
						strcmp(command,"!mdl") == 0 || strcmp(command,"!modl") == 0 ||
						strcmp(command,"!sprte") == 0 || strcmp(command,"!sprie") == 0 ||
						strcmp(command,"!srite") == 0 || strcmp(command,"!prite") == 0 ||
						strcmp(command,"!sprite") == 0) {

						num_scanf_args = sscanf(message, "%s %d %d %d %[^\n]",
												command, &x, &y, &z, temp);
						if (num_scanf_args > 3 &&
										x >= 0 && x < 18 &&
										y >= 0 && y < 9 &&
										z >= 0 && z < 5) {
							if (num_scanf_args == 4 || sd->account_id <= 100000) {
								strcpy(temp, p->name);
							}
							cfd = find_char(temp);
							if (cfd == -1) {
								sprintf(temp0, "Could not find the player ('%s').", temp);
								send_msg_p(fd,"!sprite", temp0);
							} else {
								csd = session[cfd]->session_data;
								csd->status.hair = x;
								WBUFW(buf, 0) = 0xc3; // R 00c3 <ID>.l <type>.B <val>.B: change looks. type = 01 Hair type.
								WBUFL(buf, 2) = csd->account_id;
								WBUFB(buf, 6) = 1;
								WBUFB(buf, 7) = x;
								mmo_map_sendarea(cfd, buf, packet_len_table[0xc3], 0);
								csd->status.hair_color = y;
								WBUFW(buf, 0) = 0xc3; // R 00c3 <ID>.l <type>.B <val>.B: change looks. type = 06 Hair color.
								WBUFL(buf, 2) = csd->account_id;
								WBUFB(buf, 6) = 6;
								WBUFB(buf, 7) = y;
								mmo_map_sendarea(cfd, buf, packet_len_table[0xc3], 0);
								csd->status.clothes_color = z;
								WBUFW(buf, 0) = 0xc3; // R 00c3 <ID>.l <type>.B <val>.B: change looks. type = 07 Clothes color
								WBUFL(buf, 2) = csd->account_id;
								WBUFB(buf, 6) = 7;
								WBUFB(buf, 7) = z;
								mmo_map_sendarea(cfd, buf, packet_len_table[0xc3], 0);
								send_msg_p(fd,"!sprite", "Sprite changed.");
							}
						} else {
							if (sd->account_id <= 100000) {
								send_msg_p(fd,"Command Library", "Usage: !sprite <0-18> <0-8> <0-4>");
							} else {
								send_msg_p(fd,"Command Library", "Usage: !sprite <0-18> <0-8> <0-4> [player]");
							}
						}
						return 1;
					}

					/*------------------------------------------------------------------------
					Command:       !who [gm|others|all|<text>]
					Parameters:     [gm|others|all|<text>], which type of uyser to display
									or a part of a player name to look up.
					Purpose:        Lists players who are currently online,
									the map they're on and their coordinates (x & y).
					------------------------------------------------------------------------*/
				if (strcmp(command, "!who") == 0) {
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
										send_msg(fd, "Players (except GM) currently online:");
									} else if (x == 2) { // asked for (sub-)GM players
										send_msg(fd, "(Sub-)GM currently online:");
									} else if (strcmp(temp, "all") == 0) { // asked for all players
										send_msg(fd, "People currently online:");
									} else { // asked for <text> in name of players
										sprintf(temp0, "People that have '%s' in their name:", temp);
										send_msg(fd, temp0);
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
								send_msg(fd, temp0);
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
							strcpy(temp0, "None player found."); // normally, impossible (the GM does the command)
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
					send_msg(fd, temp0);
					send_msg(fd, " "); // send an empty line to have nothing on the head of the player
					return 1; 
				} // end of !who command
				/*------------------------------------------------------------------------------------
				These are the GM commands, only for accounts with an account ID of 100000+
				------------------------------------------------------------------------------------*/
				if (sd->account_id > 100000) {
					/*------------------------------------------------------------------------
					Command:       !stat <stat to raise> <# to increase> [player]
					Parameters:     
					Purpose:        
					------------------------------------------------------------------------*/
					if (strcmp(command,"!stat") == 0 || strcmp(command,"!stt") == 0) {

						num_scanf_args = sscanf(message, "%s %s %d %[^\n]", command, temp, &x, temp0);

						if (num_scanf_args < 3) {
							send_msg_p(fd,"Command Library", "Usage: !stat <stat to raise> <# to increase> [player]");
							return 1;
						} else if (x <= 0) {
							x = 1;
						} else {
							x = x;
						}
						if(x > 0){
							if(strcmp(temp, "str")==0){
								y = SP_STR;
							}else if(strcmp(temp, "agi")==0){
								y = SP_AGI;
							}else if(strcmp(temp, "vit")==0){
								y = SP_VIT;
							}else if(strcmp(temp, "int")==0){
								y = SP_INT;
							}else if(strcmp(temp, "dex")==0){
								y = SP_DEX;
							}else if(strcmp(temp, "luk")==0){
								y = SP_LUK;
							}else{
								send_msg_p(fd,"Command Library", "Usage: !stat <stat to raise> [# to increase] [player]");
								return 1;
							}
							if (num_scanf_args == 3)
								strcpy(temp0, p->name); // give GM's player name

							cfd = find_char(temp0);
							csd = session[cfd]->session_data;
							if (cfd == -1){
								sprintf(temp1,"Couldn't find player (%s)",temp0);
								send_msg_p(fd, command, temp1);
								return 1;
							}
							for(i=0;i<x;i++){
								//check to see if stat can be raised more
								if(y == SP_STR && csd->status.str > 250){
									sprintf(temp1,"Cannot raise %s any higher.",temp);
									send_msg_p(fd, command, temp1);
									return 1;
								}else if(y == SP_AGI && csd->status.agi > 250){
									sprintf(temp1,"Cannot raise %s any higher.",temp);
									send_msg_p(fd, command, temp1);
									return 1;
								}else if(y == SP_VIT && csd->status.vit > 250){
									sprintf(temp1,"Cannot raise %s any higher.",temp);
									send_msg_p(fd, command, temp1);
									return 1;
								}else if(y == SP_INT && csd->status.int_ > 250){
									sprintf(temp1,"Cannot raise %s any higher.",temp);
									send_msg_p(fd, command, temp1);
									return 1;
								}else if(y == SP_DEX && csd->status.dex > 250){
									sprintf(temp1,"Cannot raise %s any higher.",temp);
									send_msg_p(fd, command, temp1);
									return 1;
								}else if(y == SP_LUK && csd->status.luk > 250){
									sprintf(temp1,"Cannot raise %s any higher.",temp);
									send_msg_p(fd, command, temp1);
									return 1;
								}else{ // checks ok update stat
									z = calc_need_status_point(csd, y);
									printf("Needed stat points for wanted stat: %d", z);
									if(z >= 0){
										csd->status.status_point += z;
										printf("Current stat points: %d", csd->status.status_point);
										mmo_map_update_param(fd,SP_STATUSPOINT,0);
										mmo_map_status_up(csd, y);
									}else{
										send_msg_p(fd, command, "error in calc_need_status_point, return -1");
									}
								}
							}
							return 1;

						}
						send_msg_p(fd,"Command Library", "Usage: !stat <stat to raise> [# to increase] [player]");
						return 1; 
					}
					/*------------------------------------------------------------------------
					Command:       !kill all | !doom <map|[mapname]>
					Parameters:     <map|[mapname]>
					Purpose:        kills all non-GM players on server, on GM's current map, or specified map
					------------------------------------------------------------------------*/
					if (strcmp(command, "!doom") == 0 || strcmp(command, "!dom") == 0 ||
						strcmp(command, "!doommap") == 0 || strcmp(command, "!doomap") == 0 ||
						strcmp(command, "!doom_map") == 0 || strcmp(command, "!kill") == 0) {

						num_scanf_args = sscanf(message, "%s %[^\n]", command, temp);
						send_msg_p(fd, command, "!doom command triggered"); // checks
						if(num_scanf_args == 2){
							if(strcmp(temp, "map") == 0 || strcmp(command, "map") == 0){	// kill all on GM's current map
								y = sd->mapno;
								x = 1;
							}else{	// kill all on specified map
								if (strstr(temp,".gat") == NULL) {
									strcat(temp,".gat");
								}
								for (i = 0; map[i][0]; i++) {
									if (strcmp(map[i],temp) == 0) {
										y = i;
									}
								}
								x = 2;
							}
						}else{
							y = 0;
							x = 0;
						}

						for (i = 5; i < FD_SETSIZE; i++) {
							if ((session[i] != NULL) && (session[i]->session_data != NULL)) {
								cfd = i;
								csd = session[cfd]->session_data;
								if(y > 0){
									if(csd->mapno == y){
										if (csd->account_id < 100000){
											csd->status.hp = 0;
											WFIFOW(cfd, 0) = 0xb0; // R 00b0 <type>.w <val>.l: update values of various status (..0005:HP..)
											WFIFOW(cfd, 2) = 0005;
											WFIFOL(cfd, 4) = csd->status.hp;
											WFIFOSET(cfd, packet_len_table[0xb0]);
											WBUFW(buf, 0) = 0x80; // R 0080 <ID>.l <type>.B: type = 01 (character died)
											WBUFL(buf, 2) = csd->account_id;
											WBUFB(buf, 6) = 1;
											mmo_map_sendarea(cfd, buf, packet_len_table[0x80], 0);
										}
										send_msg_p(fd, command, "Killing all on map"); // checks
									}
								}else{
									if (csd->account_id < 100000){
										csd->status.hp = 0;
										WFIFOW(cfd, 0) = 0xb0; // R 00b0 <type>.w <val>.l: update values of various status (..0005:HP..)
										WFIFOW(cfd, 2) = 0005;
										WFIFOL(cfd, 4) = csd->status.hp;
										WFIFOSET(cfd, packet_len_table[0xb0]);
										WBUFW(buf, 0) = 0x80; // R 0080 <ID>.l <type>.B: type = 01 (character died)
										WBUFL(buf, 2) = csd->account_id;
										WBUFB(buf, 6) = 1;
										mmo_map_sendarea(cfd, buf, packet_len_table[0x80], 0);
									}
									send_msg_p(fd, command, "Killing all on server."); // checks
								}
								send_msg_p(fd, command, "The mark of the dead...");
							}
						}
						
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !jobchange <0-12/20> [player]
					Parameters:     <0-12/20>, the number representing the
									job class you want to change to.
					Purpose:        Changes player's job class.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!jobchange") == 0 || strcmp(command,"!job_change") == 0 ||
						strcmp(command,"!jchange") == 0 || strcmp(command,"!j_change") == 0) {

						num_scanf_args = sscanf(message,"%s%d",command,&x);

						if ((num_scanf_args == 1 || x < 0) || x > MAX_CLASSES) { // if only command, no job, no player name
							if (MAX_CLASSES >= 24) { // super novice
								send_msg(fd, "Usage: !job_change <0-20> [player]");
								send_msg(fd, " 0 novice     5 merchant   10 blacksmith  15 monk      20 dancer");
								send_msg(fd, " 1 swordsman  6 thief      11 hunter      16 sage      21 crusader");
								send_msg(fd, " 2 mage       7 knight     12 priest      17 rogue     22 wedding");
								send_msg(fd, " 3 archer	    8 priest     13 knight      18 alchemist 23 super novice");
								send_msg(fd, " 4 acolyte    9 wizard     14 crusader    19 bard");
								send_msg(fd, " ");
							} else if (MAX_CLASSES == 23) { //wedding
								send_msg(fd, "Usage: !job_change <0-20> [player]");
								send_msg(fd, " 0 novice     5 merchant   10 blacksmith  15 monk      20 dancer");
								send_msg(fd, " 1 swordsman  6 thief      11 hunter      16 sage      21 crusader");
								send_msg(fd, " 2 mage       7 knight     12 priest      17 rogue     22 wedding");
								send_msg(fd, " 3 archer	    8 priest     13 knight      18 alchemist");
								send_msg(fd, " 4 acolyte    9 wizard     14 crusader    19 bard");
								send_msg(fd, " ");
							} else if (MAX_CLASSES >= 21) { // 2-2
								send_msg(fd, "Usage: !job_change <0-20> [player]");
								send_msg(fd, " 0 novice     5 merchant   10 blacksmith  15 monk      20 dancer");
								send_msg(fd, " 1 swordsman  6 thief      11 hunter      16 sage");
								send_msg(fd, " 2 mage       7 knight     12 priest      17 rogue");
								send_msg(fd, " 3 archer	    8 priest     13 knight      18 alchemist");
								send_msg(fd, " 4 acolyte    9 wizard     14 crusader    19 bard");
								send_msg(fd, " ");
							} else { // Only 1-1 and 2-1
								send_msg(fd, "Usage: !job_change <0-12> [player]");
								send_msg(fd, " 0 novice      3 archer     6 thief     9 wizard       12 assassin");
								send_msg(fd, " 1 swordsman   4 acolyte    7 knight   10 blacksmith");
								send_msg(fd, " 2 mage        5 merchant   8 priest   11 hunter");
								send_msg(fd, " ");
							}
						} else {
							if (num_scanf_args == 2)
								strcpy(temp, p->name);

							cfd = find_char(temp);

							if (cfd > 0) {
								csd = session[cfd]->session_data; // get structure of the player
								mmo_map_jobchange(cfd, x);
								send_msg(fd, "Job changed...");
							} else {
								sprintf(temp0, "Could not find the player ('%s').", temp);
								send_msg(fd, temp0);
							}
						}
						send_msg(fd, " ");
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !speed <1-140>
					Parameters:     <1-140>, Number representing new walking speed.
					Purpose:        Adjusts GM walking-speed (1 fastest, 140 normal speed).
					------------------------------------------------------------------------*/
					if (strcmp(command,"!speed") == 0) {
						num_scanf_args = sscanf(message,"%s%d",command,&x);
						if(num_scanf_args != 0){
							if (x>0 && x<=140) {
								char moji[200];
								sprintf(moji,"Speed was %i, changed to %i.",sd->speed,x);
								sd->speed = x;
								send_msg_p(fd,"!speed",moji);
							} else {
								send_msg_p(fd,"Command Library","Usage: !speed 1-140 (1 fastest, 140 normal speed)");
							}
						}else{
							send_msg_p(fd,"Command Library","Usage: !speed 1-140 (1 fastest, 140 normal speed)");
						}
						return 1;
					}

					/*------------------------------------------------------------------------
					Command:       !goto <0-12>/<cityname>
					Parameters:     <0-12>: Number representing the place to warp to.
									<cityname>: Name representing the place to warp to.

					Possible Values:	1 Prontera	7 Al de Baran
										2 Morroc	8 Lutie
										3 Geffen	9 Comodo
										4 Payon		10 Yuno
										5 Alberta	11 Newbie Zone
										6 Izlude	12 Orc dungeon
					Purpose:        Warps player to one of the predefined warp-spots.
					------------------------------------------------------------------------*/
					if (strcmp(command, "!go") == 0 || strcmp(command, "!goto") == 0 ||
						strcmp(command, "!gto") == 0) {

						num_scanf_args = sscanf(message, "%s %s", command, temp);
						if (num_scanf_args != 1) {
							if (atoi(temp) >= 1 && atoi(temp) <= 11) {
								x = atoi(temp);
							} else {
								if (strcmp(temp, "Prontera") == 0 || strcmp(temp, "Pront") == 0 ||
									strcmp(temp, "prontera") == 0 || strcmp(temp, "pront") == 0) {
									x = 1;
								} else if (strcmp(temp, "Morocc") == 0 || strcmp(temp, "Moroc") == 0 ||
										strcmp(temp, "Morrocc") == 0 || strcmp(temp, "Morroc") == 0 ||
										strcmp(temp, "morocc") == 0 || strcmp(temp, "moroc") == 0 ||
										strcmp(temp, "morrocc") == 0 || strcmp(temp, "morroc") == 0 ) {
									x = 2;
								} else if (strcmp(temp, "Geffen") == 0 || strcmp(temp, "Gefen") == 0 ||
										strcmp(temp, "Geff") == 0 || strcmp(temp, "Gef") == 0 ||
										strcmp(temp, "geffen") == 0 || strcmp(temp, "gefen") == 0 ||
										strcmp(temp, "geff") == 0 || strcmp(temp, "gef") == 0) {
									x = 3;
								} else if (strcmp(temp, "Payon") == 0 || strcmp(temp, "Pay") == 0 ||
										strcmp(temp, "payon") == 0 || strcmp(temp, "pay") == 0) {
									x = 4;
								} else if (strcmp(temp, "Alberta") == 0 || strcmp(temp, "alberta") == 0) {
									x = 5;
								} else if (strcmp(temp, "Izlude") == 0 || strcmp(temp, "izlude") == 0) {
									x = 6;
								} else if (strcmp(temp, "Aldebaran") == 0 || strcmp(temp, "Aldebar") == 0 ||
										strcmp(temp, "Al") == 0 || strcmp(temp, "aldebaran") == 0 ||
										strcmp(temp, "aldebar") == 0 || strcmp(temp, "al") == 0) {
									x = 7;
								} else if (strcmp(temp, "Lutie") == 0 || strcmp(temp, "Xmas") == 0 ||
										strcmp(temp, "lutie") == 0 || strcmp(temp, "xmas") == 0) {
									x = 8;
								} else if (strcmp(temp, "Comodo") == 0 || strcmp(temp, "Commodo") == 0 ||
										strcmp(temp, "comodo") == 0 || strcmp(temp, "commodo") == 0) {
									x = 9;
								} else if (strcmp(temp, "Juno") == 0 || strcmp(temp, "juno") == 0 ||
										strcmp(temp, "Yuno") == 0 || strcmp(temp, "yuno") == 0) {
									x = 10;
								} else if (strcmp(temp, "Newby") == 0 || strcmp(temp, "Newbies") == 0 ||
										strcmp(temp, "Newbie") == 0 || strcmp(temp, "Start") == 0 ||
										strcmp(temp, "newby") == 0 || strcmp(temp, "newbies") == 0 ||
										strcmp(temp, "newbie") == 0 || strcmp(temp, "start") == 0) {
									x = 11;
								} else if (strcmp(temp, "Orc") == 0 || strcmp(temp, "Orcs") == 0 ||
										strcmp(temp, "orc") == 0 || strcmp(temp, "orcs") == 0) {
									x = 12;
								}
							}
							if (x >= 1 && x <= 11) {
			/* Prontera */		if (x == 1) {
									mmo_map_changemap(sd, "prontera.gat", 156, 181, 2);
									send_msg(fd, "Welcome to Prontera.");
			/* Morroc */		} else if (x == 2) {
									mmo_map_changemap(sd, "morocc.gat", 156, 96, 2);
									send_msg(fd, "Welcome to Morroc.");
			/* Geffen */		} else if (x == 3) {
									mmo_map_changemap(sd, "geffen.gat", 119, 60, 2);
									send_msg(fd, "Welcome to Geffen.");
			/* Payon */			} else if (x == 4) {
									mmo_map_changemap(sd, "payon.gat", 89, 122, 2);
									send_msg(fd, "Welcome to Payon.");
			/* Alberta */		} else if (x == 5) {
									mmo_map_changemap(sd, "alberta.gat", 192, 147, 2);
									send_msg(fd, "Welcome to Alberta.");
			/* Izlude */		} else if (x == 6) {
									mmo_map_changemap(sd, "izlude.gat", 128, 114, 2);
									send_msg(fd, "Welcome to Izlude.");
			/* Al de Baran */	} else if (x == 7) {
									mmo_map_changemap(sd, "aldebaran.gat", 140, 131, 2);
									send_msg(fd, "Welcome to Al de Baran.");
			/* Lutie */			} else if (x == 8) {
									mmo_map_changemap(sd, "xmas.gat", 147, 134, 2);
									send_msg(fd, "Welcome to Lutie.");
			/* Comodo */		} else if (x == 9) {
									mmo_map_changemap(sd, "comodo.gat", 188, 161, 2);
									send_msg(fd, "Welcome to Comodo.");
			/* Yuno */			} else if (x == 10) {
									mmo_map_changemap(sd, "yuno.gat", 160, 168, 2);
									send_msg(fd, "Welcome to Yuno.");
			/* Newbie Zone */	} else if (x == 11) {
									mmo_map_changemap(sd, "new_5-1.gat", 57, 115, 2);
									send_msg(fd, "Welcome to the Newbie zone.");
			/* Orc dungeon */	} else if (x == 12) {
									mmo_map_changemap(sd, "gef_fild10.gat", 66, 335, 2);
									send_msg(fd, "Welcome to Orc dungeon.");
								}
							} else {
								send_msg(fd, "Usage: !go <1-12>|<cityname>");
								send_msg(fd, " 1 Prontera   4 Payon     7 AldeBaran   10 Newbie Zone");
								send_msg(fd, " 2 Morroc     5 Alberta   8 Lutie       11 Orc Dungeon");
								send_msg(fd, " 3 Geffen     6 Izlude    9 Comodo");
							}
						} else {
								send_msg(fd, "Usage: !go <1-12>|<cityname>");
								send_msg(fd, " 1 Prontera   4 Payon     7 AldeBaran   10 Newbie Zone");
								send_msg(fd, " 2 Morroc     5 Alberta   8 Lutie       11 Orc Dungeon");
								send_msg(fd, " 3 Geffen     6 Izlude    9 Comodo");
						}
						send_msg(fd," ");
						return 1;
					}

					/*------------------------------------------------------------------------
					Command:       !hide
					Parameters:     None
					Purpose:        Makes the GM hidden, or not.
					------------------------------------------------------------------------*/
					if (strcmp(command, "!hide") == 0 || strcmp(command, "!hde") == 0 ||
						strcmp(command, "!hie") == 0){

						num_scanf_args = sscanf(message, "%s %[^\n]", command, temp);
						if (!sd->hidden) {
							sd->hidden = 1;
							x = 64;
							sprintf(moji,"Unseen.");
						}else{
							sd->hidden = 0;
							x = 0;
							sprintf(moji,"Seen.");
						}
						WFIFOW(fd,0) = 0x0119;
						WFIFOL(fd,2) = sd->account_id;
						WFIFOW(fd,6) = 0;
						WFIFOW(fd,8) = 0;
						WFIFOW(fd,10) = x;
						mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x0119], 0);
						p->option_x = 0;
						p->option_y = 0;
						p->option_z = x;
						send_msg_p(fd, "!hide", moji);
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !status <param1> <param2> <param3>
					Parameters:     <param1>		<param2>		<param3>
									0 Normal		0 Normal		0 Normal
									1 Stoned		1 Poisoned		1 Sight
									2 Frozen		2 Death			2 Hiding
									3 Stunned		4 Silenced		4 Cloaking
									4 Slept			16 Dark-Sighted	8 Cart attachment
									6 Darkness						16 Falcon
																	32 Peco Peco riding
																	64 Vanish
					Purpose:        Adds/changes certain visual aspects around and on the GM's character.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!option") == 0 || strcmp(command,"!ption") == 0 ||
						strcmp(command,"!status") == 0 || strcmp(command,"!sttus") == 0 ||
						strcmp(command,"!stats") == 0 || strcmp(command,"!stts") == 0) {
						num_scanf_args = sscanf(message,"%s%d%d%d",command,&x,&y,&z);
						if(num_scanf_args != 1){
							WFIFOW(fd,0) = 0x0119;
							WFIFOL(fd,2) = sd->account_id;
							WFIFOW(fd,6) = x;
							WFIFOW(fd,8) = y;
							WFIFOW(fd,10) = z;
							mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x0119], 0);
							p->option_x = x;
							p->option_y = y;
							p->option_z = z;
							sprintf(moji,"Options set to %i %i %i.",x,y,z);
							send_msg_p(fd,"!status" , moji);
						}else{
							send_msg_p(fd,"Command Library", "Usage: !status <param1> <param2> <param3>");
						}
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !summon <player>
					Parameters:     <player>, Player to warp.
					Purpose:        Warps the specified player to the GM.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!call") == 0 || strcmp(command,"!recall") == 0 ||
						strcmp(command,"!summon") == 0 || strcmp(command,"!sumon") == 0 ||
						strcmp(command,"!smmon") == 0 || strcmp(command,"!smon") == 0) {
						sscanf(message,"%s %[^\n]",command,temp);
						int cfd = find_char(temp);
						if (cfd == -1) {
							send_msg_p(fd,"!summon" , "Usage: !summon <player>");
						}
						else {
							struct map_session_data *csd=NULL;
							csd = session[cfd]->session_data;
							mmo_map_changemap(csd,sd->mapname,sd->x,sd->y,2);
						}
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !baseup <lvl> [player]
					Parameters:     <lvl>, Number of levels you wish to increase
									[player], optional player to increase in lvl.
					Purpose:        Increases GM's character's base level. Max: 99.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!base_up") == 0 || strcmp(command,"!baseup") == 0 ||
						strcmp(command,"!b_up") == 0 || strcmp(command,"!bup") == 0) {

						num_scanf_args = sscanf(message, "%s %d %[^\n]", command, &x, temp);

						if (num_scanf_args == 1) {
							send_msg_p(fd,"Command Library", "Usage: !baseup <lvl> [player]");
						} else if (x > 0) {

							if (num_scanf_args == 2)
								strcpy(temp, p->name);

							cfd = find_char(temp); // search id of player

							if (cfd > 0) {
								if (mmo_map_level_up(cfd,x) == -1) {
									sprintf(temp0, "Cannot level up any higher.");
									send_msg_p(fd,"!baseup", temp0);
								}
							} else {
								sprintf(temp0, "Could not find the player ('%s').", temp);
								send_msg_p(fd,"!baseup", temp0);
							}
						}
						return 1; 
					}
					/*------------------------------------------------------------------------
					Command:       !giveskillp <amount> [player]
					Parameters:     <amount>: How many skill points you wish to give,
									[player]: Player awarded skill points.
					Purpose:        Give <amount> skill points to <player>.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!giveskillp") == 0 || strcmp(command,"!giveskp") == 0 ||
						strcmp(command,"!give_skillp") == 0 || strcmp(command,"!give_skp") == 0) {
						num_scanf_args = sscanf(message,"%s%d %[^\n]",command,&x,temp);

						if (num_scanf_args == 2)
							strcpy(temp, p->name);

						cfd = find_char(temp);
						if (cfd == -1) {
							send_msg_p(fd, "Command Library", "Usage: !giveskillp <amount> [player]");
						} else {
							struct map_session_data *csd=NULL;
							csd = session[cfd]->session_data;
							csd->status.skill_point += x;
							mmo_map_update_param(fd,SP_SKILLPOINT,0);
						}
						return 1;
					}

					/*------------------------------------------------------------------------
					Command:       !givestatp <amount> [player]
					Parameters:     <amount>: How many stat points you wish to give,
									[player]: Player awarded stat points.
					Purpose:        Give <amount> stat points to <player>.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!givestatp") == 0 || strcmp(command,"!givestp") == 0 ||
						strcmp(command,"!give_statp") == 0 || strcmp(command,"!give_stp") == 0) {
						num_scanf_args = sscanf(message,"%s%d %[^\n]",command,&x,temp);
						if (num_scanf_args == 2)
							strcpy(temp, p->name);

						cfd = find_char(temp);
						if (cfd == -1) {
							send_msg_p(fd, "Command Library", "Usage: !givestatp <amount> [player]");
						} else {
							csd = session[cfd]->session_data;
							csd->status.status_point += x;
							mmo_map_update_param(fd,SP_STATUSPOINT,0);
						}
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !givez <amount> [player]
					Parameters:     <amount>: How much zeny you wish to give,
									[player]: Player awarded zeny.
					Purpose:        Give <amount> zeny to <player>.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!zeny") == 0 || strcmp(command,"!givez") == 0 ||
						strcmp(command,"!give_z") == 0) {
						num_scanf_args = sscanf(message,"%s%d %[^\n]",command,&x,temp);
						if (num_scanf_args == 2)
							strcpy(temp, p->name);

						cfd = find_char(temp);
						if (cfd == -1) {
							send_msg_p(fd, "Command Library", "Usage: !givez <amount> [player]");
						} else {
							csd = session[cfd]->session_data;
							csd->status.zeny += x;
							mmo_map_update_param(fd,SP_ZENY,0);
						}
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !item <id> [quantity]
					Parameters:     <id>: Id of item to get,
									[quantity]: amount of item to get.
					Purpose:        Gives <number> items of <id>.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!item") == 0 || strcmp(command,"!iem") == 0 ||
						strcmp(command,"!itm") == 0) {
						x = 501, y = 1;
						sscanf(message,"%s%d%d",command,&x,&y);
						if (is_item(x) == 1) {
							memset(&tmp_item,0,sizeof(tmp_item));
							tmp_item.nameid = x;
							tmp_item.amount = y;
							tmp_item.identify = 1;
							mmo_map_get_item(fd, &tmp_item);
							return 1; 
						}else{
							send_msg_p(fd, "Command Library", "Usage: !item <id> [quantity]");
						}
						return 1;
					}

					/*------------------------------------------------------------------------
					Command:       !isearch <id>/<name>
					Parameters:     <id>, The item's ID.
									<name>, The item's Name
					Purpose:        Searches the item db, and returns
									the name (or id) of item,with ID of <id>,
									or name of <name>.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!isearch") == 0 || strcmp(command,"!i_search") == 0){

						num_scanf_args = sscanf(message, "%s %s", command, temp);
						if (num_scanf_args != 1) {
							if (atoi(temp) >= 1 && atoi(temp) <= 900000) { // id given, search via #.
								x = atoi(temp);
								if (is_item(x)) {
									sprintf(temp, "Item #%i is a %s.", x, itemdb[search_itemdb_index(x)].name);
									send_msg(fd, temp);
								} else {
									sprintf(temp, "Id #%i isn't an item.", x);
									send_msg(fd, temp);
								}
							}else{	// no id given, search using name.
								x = 0; // count the number of items that have <text>
								strtolower(temp); // in lowercase to prepare compare
								strcpy(temp1, ""); // initiliaze temp1 for 1st line
								for (i = 501; i < 10020; i++) { // item range, not the best way.. ideas?
									y = search_itemdb_index(i);
									strcpy(temp0, itemdb[y].name); // copy, otherwise str_lower modify source
									if (strstr(strtolower(temp0), temp) != NULL) {
										if (x == 0) {
											send_msg(fd, "List of the items:");
										}
										x++; // increase number of items found
										sprintf(temp0, "%5d:%-25s", i, itemdb[y].name); // the string must be have 31 chars for align (in columns)
										strcat(temp1, temp0);
										// work on 3 columns (max 94 chars per line: 31 * 3 = 93)
										if (x % 3 == 0) {
											send_msg(fd, temp1);
											strcpy(temp1, ""); // initiliaze temp1 for new line
										}
									}
								}
								if (x == 0) {
									sprintf(temp0, "No item has '%s' in its name, sorry.", temp);
									send_msg(fd, temp0);
								} else {
									if (x % 3 > 0) { // if last item have not been displayed
										send_msg(fd, temp1);
									}
									if (x == 1) {
										sprintf(temp0, "One item has '%s' in its name.", temp);
									} else {
										sprintf(temp0, "%i items have '%s' in their name.", x, temp);
									}
									send_msg(fd, temp0);
						}
							}
						}else{
							send_msg(fd, "Usage: !isearch <id>/<name>");
						}
						send_msg(fd, " ");
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !jobup <lvl> [player]
					Parameters:     <lvl>: How many levels to increase.
									[player]: Player who's job you want to change.
					Purpose:        Increases GM's character's job level.
									Maximum level is 50.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!jobup") == 0 || strcmp(command,"!job_up") == 0 ||
						strcmp(command,"!jup") == 0 || strcmp(command,"!j_up") == 0) {

						num_scanf_args = sscanf(message, "%s %d %[^\n]", command, &x, temp);

						if (num_scanf_args == 1) {
							send_msg_p(fd,"Command Library", "Usage: !baseup <lvl> [player]");
						} else if (x > 0) {

							if (num_scanf_args == 2)
								strcpy(temp, p->name);

							cfd = find_char(temp); // search id of player

							if (cfd > 0) {
								if (mmo_map_job_lv_up(cfd,x) == -1) {
									sprintf(temp0, "Cannot level up any higher.");
									send_msg_p(fd,"!jobup", temp0);
								}
							} else {
								sprintf(temp0, "Could not find the player ('%s').", temp);
								send_msg_p(fd,"!jobup", temp0);
							}
						}
						return 1; 
					}
					/*------------------------------------------------------------------------
					Command:       !pvp
					Parameters:     [on/off] otpional.
					Purpose:        Turns PvP setting ON or OFF in current map.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!pvp") == 0) {

						num_scanf_args = sscanf(message, "%s %s", command, temp);
						strtolower(temp);
						if(num_scanf_args != 1){
							if(strcmp(temp, "on") == 0)
								x = 0;
							else
								x = 1;
						} else {
							if(PVP_flag == 0)
								x = 0;
							else
								x = 1;
						}
						if( x >= 0 || x <= 1 ){
							if(x == 0){
								WFIFOW(fd,0) = 0x199;
								WFIFOW(fd,2) = 1;
								WFIFOSET(fd,4);
								WFIFOW(fd,0) = 0x19a;
								WFIFOL(fd,2) = sd->account_id;
								WFIFOL(fd,6) = sd->pvprank;
								WFIFOL(fd,10) = users_global;
								mmo_map_sendarea(fd, WFIFOP(fd,0), packet_len_table[0x19a], 0);
								send_msg_p(fd,"!pvp","PvP Mode: Enabled");
								PVP_flag = 1;
							}else{
								WFIFOW(fd,0) = 0x199;
								WFIFOW(fd,2) = 1;
								WFIFOSET(fd,4);
								send_msg_p(fd,"!pvp","PvP Mode: Disabled");
								PVP_flag = 0;
							}
						}
						return 1;
					}
					/*------------------------------------------------------------------------
					Command:       !warpchar <map> <x> <y> <player>
					Parameters:     <map>: Name of map to warp to.
									<x>: X coordinate to warp to.
									<y>: Y coordinate to warp to.
									<player>: Name of player to warp.
					Purpose:        Warps <player> to the specified <x> <y> coordinates
									of the specified <map>.
					------------------------------------------------------------------------*/
					if (strcmp(command,"!warpone") == 0 || strcmp(command,"!warp_one") == 0 ||
						strcmp(command,"!warpchar") == 0 || strcmp(command,"!warp_char") == 0 ||
						strcmp(command,"!w_char") == 0 || strcmp(command,"!wchar") == 0 ||
						strcmp(command,"!charwarp") == 0 || strcmp(command,"!char_warp") == 0) {
						sscanf(message,"%s%s%d%d %[^\n]",command,temp,&x,&y,temp0);
						if ((x > 0) && (x < 325) && (y > 0) && (y < 325)) {
							if (strstr(temp,".gat") == NULL) {
								strcat(temp,".gat");
							}
							for (i=0;map[i][0];i++) {
								if (strcmp(map[i],temp) == 0) {
									cfd = find_char(temp0);
									if (cfd == -1) {
										return 1; 
									} else {
										struct map_session_data *sd=NULL;
										sd = session[cfd]->session_data;
										mmo_map_changemap(sd,temp,x,y,2);
									}
									return 1;
								}
							}
							return 1;
						}else{
							send_msg_p(fd,"Command Library", "Usage: !warpchar <map> <x> <y> <player>");
						}
						return 1;
					}

				}
			}
		sprintf(temp, "(%s) Command not recognized. For help use !help.", command);
		send_msg_p(fd,"Command Library",temp);
		return 1;
	}
	return 0;
}
