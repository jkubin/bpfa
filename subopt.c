/* ???
 * Copyright (C) 2017 Josef Kubin
 *
 * This file is part of bpfa.
 *
 * bpfa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bpfa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#if 0

#include "opt.h"

struct applic_suboption app_sopt;

int applic_subopts(char *subopts)
{

	char *value;

	while (*subopts) {

		switch (getsubopt(&subopts, app_subopts, &value)) {

			case APP_LOW_ADDR:
				app_sopt.lo = strtol(value, NULL, 0);
				break;
			case APP_HIGH_ADDR:
				app_sopt.hi = strtol(value, NULL, 0);
				break;
			case APP_MASK:
				app_sopt.mask = strtol(value, NULL, 0);
				break;
			case APP_OBJECT_FILE:
				app_sopt.obj = value;
				break;
			case APP_BIN_FILE:
				app_sopt.bin = value;
				break;
			default:
				fprintf(stderr, "Unknown suboption %s\n", value);
				return -1;
		}
	}

	//if (app_sopt.lo >= app_sopt.hi)
		//return -1;

	if (!app_sopt.hi && !app_sopt.mask)
		return -1;

	if (app_sopt.hi && app_sopt.mask)
		return -1;

	if (!app_sopt.obj && !app_sopt.bin)
		return -1;

	return 0;
}
#endif

