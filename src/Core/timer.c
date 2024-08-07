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

#include "core.h"
#include "timer.h"

int timer_data_max = 0;
int timer_data_num = 0;

unsigned int gettick(void)
{
	struct timeval st;

	gettimeofday(&st, NULL);
	return st.tv_sec * 1000 + st.tv_usec / 1000;
}

int do_timer(register unsigned int tick)
{
	int i, nextmin = 1000;

	for (i = 1; i < timer_data_num; i++) {
		if (timer_data[i]) {
			if ((int)(timer_data[i]->tick - tick) < nextmin)
				nextmin = (int)(timer_data[i]->tick - tick);

			if ((int)(timer_data[i]->tick - tick) <= 0) {
				if (timer_data[i]->func) {
					timer_data[i]->func(i, tick, timer_data[i]->data, timer_data[i]->data_);
					if (timer_data[i]) {
						switch (timer_data[i]->type) {
						case TIMER_ONCE_AUTODEL:
							if (delete_timer(i, timer_data[i]->func))
								debug_output("do_timer: Imposible to delete timer: %d class: TIMER_ONCE_AUTODEL.\n", i);

							break;
						case TIMER_INTERVAL:
							timer_data[i]->tick += timer_data[i]->interval;
							break;
						}
					}
				}
			}
		}
	}
	if (nextmin < 10)
		nextmin = 10;

	return nextmin;
}

int add_timer(unsigned int tick, void (*func)(int, unsigned int, int, int), int data, int data_)
{
	int i;
	struct TimerData *td = malloc(sizeof(struct TimerData));
	if (td == NULL) {
		debug_output("add_timer: Cannot allocate memory in 'td'.\n");
		exit(1);
	}
	if (func == NULL) {
		debug_output("add_timer: Invalid timer function pointer.\n");
		exit(1);
	}
	for (i = 1; i < timer_data_num; i++)
		if (!timer_data[i])
			break;


	if (i >= timer_data_num && i >= timer_data_max) {
		if (timer_data_max == 0) {
			timer_data_max = 256;
			timer_data = malloc(sizeof(struct TimerData) * timer_data_max);
			timer_data[0] = NULL;
		}
		else {
			timer_data_max += 256;
			timer_data = realloc(timer_data, sizeof(struct TimerData) * timer_data_max);
		}
	}
	timer_data[i] = td;
	timer_data[i]->id = i;
	timer_data[i]->tick = tick;
	timer_data[i]->func = func;
	timer_data[i]->data = data;
	timer_data[i]->data_ = data_;
	timer_data[i]->type = TIMER_ONCE_AUTODEL;
	timer_data[i]->interval = 0;
	if (i >= timer_data_num)
		timer_data_num = i + 1;

	return i;
}

int delete_timer(int id, void (*func)(int, unsigned int, int, int))
{
	if (id <= 0 || id > timer_data_num || !timer_data[id]) {
		debug_output("delete_timer: Timer to delete is not valid.\n");
		return 1;
	}
	if (timer_data[id]->func != func) {
		debug_output("delete_timer: Timer function pointer is not valid.\n");
		return 2;
	}
	free(timer_data[id]);
	timer_data[id] = NULL;
	return 0;
}
