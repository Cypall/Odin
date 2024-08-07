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

void grfio_init(void);
int grfio_add(char*);
void* grfio_read(char*);
void* grfio_reads(char*,int*);
int grfio_size(char*);

typedef struct {
	void *data;
	int size;
	int fd;
} GrfMappedFile;

GrfMappedFile *grfio_readm(char *name);
void grfio_freem(GrfMappedFile *file);

char *grfio_setdatafile(const char *str);
char *grfio_setadatafile(const char *str);
char *grfio_setsdatafile(const char *str);
