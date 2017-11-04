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

#ifndef __OPT_H
#define __OPT_H

struct bpfa_applic_suboption
{
	unsigned int lo;
	unsigned int hi;
	unsigned int mask;
	char *obj;
	char *bin;
};

enum
{

	BPFA_APP_LOW_ADDR = 0,

	BPFA_APP_BIN_FILE,
	BPFA_APP_HIGH_ADDR,
	BPFA_APP_MASK,
	BPFA_APP_OBJECT_FILE,
	BPFA_APP_THE_END,

};

enum
{

	BPFA_OPT_ALL_SYMBOLS = 128,

	BPFA_OPT_CRC_ARRAY_PATCH,
	BPFA_OPT_LENGTH_SYMBOL,
	BPFA_OPT_MCHP_PART_LIST,
	// BPFA_OPT_NEW_OBJECT_NAME,
	BPFA_OPT_PART_NAME,
	BPFA_OPT_PROGRAM_MEMORY1,
	BPFA_OPT_PROGRAM_MEMORY2,
	BPFA_OPT_RAM_SECTIONS_FILE,
	BPFA_OPT_RAM_SYMBOLS,
	BPFA_OPT_RAM_SYMBOLS_FILE,
	BPFA_OPT_ROM_SECTIONS_FILE,
	BPFA_OPT_ROM_SYMBOLS,
	BPFA_OPT_ROM_SYMBOLS_FILE,
	BPFA_OPT_SUMMARY_MAP,

};

// extern char * const app_subopts[];
// extern const char short_options[];
// extern const char usage[];
// extern const char version[];
// extern const struct option long_options[];
// extern struct applic_suboption app_sopt;

// int applic_subopts(char *subopts);


#endif

