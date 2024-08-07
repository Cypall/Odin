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
#include "mmo.h"
#include "map_core.h"
#include "element_damage.h"

int element_db[NB_LEVELS][NB_ELEMENTS][NB_ELEMENTS];

void ele_init()
{
	int i, j;
	int level;
	char line[2048];
	char filename[255];
	int tmp_ele[NB_ELEMENTS];
	FILE *fp;

	for (level = 0; level < NB_LEVELS; level++) {
		i = 0;
		sprintf(filename, "data/tables/element_damage/element_damage%d.txt", level+1);
		fp = fopen(filename, "r");
		if(!fp) {
			printf("%s not found\n", filename);
			continue;
		}
		while (fgets(line, 2047, fp)) {
			if ((line[0] == '/') && (line[1] == '/'))
				continue;

			if (line[0] == 0)
				continue;

			if (sscanf(line,"%i,%i,%i,%i,%i,%i,%i,%i,%i,%i",
				   &tmp_ele[0], &tmp_ele[1], &tmp_ele[2], &tmp_ele[3], &tmp_ele[4],
				   &tmp_ele[5], &tmp_ele[6], &tmp_ele[7], &tmp_ele[8], &tmp_ele[9]) != 10)
				printf("%s LINE %i : Incorrect Format\n",filename,i+1);

			for (j = 0; j < NB_ELEMENTS; j++)
				element_db[level][i][j] = tmp_ele[j];

			if (i == 9)
				break;

			i++;
		}
		fclose(fp);
	}
	return;
}

double get_ele_attack_factor(int attack_ele, int target_ele)
{
	double ret;
	int target_level = (target_ele / 10) / 2;

	if (attack_ele < 0)
		attack_ele = 0;

	else
		attack_ele = attack_ele % 10;

	if (target_ele < 0)
		target_ele = 0;

	else
		target_ele = target_ele % 10;

	if (target_level > 4)
		target_level = 4;

	else if (target_level < 1)
		target_level = 1;

	ret = element_db[target_level-1][target_ele][attack_ele] / 100.0;
	return ret;
}
