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

#ifndef _TIMER_H_
#define _TIMER_H_
#define TIMER_ONCE_AUTODEL 0
#define TIMER_INTERVAL 1

struct TimerData
{
	int id;
	unsigned int tick;
	int type;
	int interval;
	void (*func)(int, unsigned int, int, int);
	int data;
	int data_;
}**timer_data;

unsigned int gettick(void);
int do_timer(register unsigned int);
int add_timer(unsigned int, void (*func)(int, unsigned int, int, int), int, int);
int delete_timer(int, void (*func)(int, unsigned int, int, int));
#endif
