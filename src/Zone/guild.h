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

void mmo_guild_do_init(void);
int create_guild(struct map_session_data *sd, char *guildname);
int check_guild_exists(int guild_id);
int connected_members(int guild_id);
