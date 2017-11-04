/* will be removed
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

#include <stdio.h>
#include "bpfa.h"

struct section_data boot = {
	.s_start = 0x00000000,
	.s_end   = 0x00003fff,
	.s_fuses = 0x0001fff0,
	//.s_sect  = 0xfff40000,
	.name    = "BOOT",
};

struct section_data mcu = {
	.s_start = 0x00000000,
	.s_end   = 0x0001ffff,
	//.s_sect  = 0xfffe0000,
	.name    = "MCU",
};

struct section_data dflash = {
	.s_start = 0xff000000,
	.s_end   = 0xffffffff,
	//.s_sect  = 0xff000000,
	.name    = "DFLASH",
};

struct section_data eth = {
	.s_start = 0x00400000,
	.s_end   = 0x0041ffff,
	//.s_sect  = 0xff800000,
	.name    = "ETH",
};

struct section_data fram = {
	.s_start = 0xfe000000,
	.s_end   = 0xfeffffff,
	//.s_sect  = 0xff000000,
	.name    = "FRAM",
};

