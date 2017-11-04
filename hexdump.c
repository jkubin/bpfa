/* prints hexdump table
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
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "coff.h"
#include "bpfa.h"

void bpfa_hexdump(unsigned int addr, unsigned int size,
		/*unsigned int flags, char *name, */unsigned char *raw)
{

	unsigned char *txt_ptr;
	unsigned char chr;
	unsigned char txt_data[17];
	unsigned char unaligned;
	unsigned int i;
	unsigned int j;

	/*
	if (bpfa_options & BPFA_VERBOSE_OUTPUT) {
		puts(header);

		// end of section = addr + size - 1
		printf("%08X\t%08X\t", addr, size);

		print_scn_flags(flags);

		puts(name);
	}
	*/

	memset(txt_data, 0, sizeof(txt_data));

	printf( "------------------------------------------------------------+----------------+\n"
			"%08x  ", addr);

	unaligned = addr & 0xf;

	txt_ptr = txt_data;

	for (i = 0; i < size; i++, addr++, raw++, txt_ptr++) {

		if (!(addr & 0xf)) {

			if (unaligned) {
				unaligned = 0;

				for (j = i; j & 0xf; j++) {

					if ((j & 0x7) == 0x7)
						putchar(' ');

					printf("   ");
				}
			}

			if (*txt_data)
				printf("|%s|\n%08x  ", txt_data, addr);

			txt_ptr = txt_data;
		}

		chr = *raw;

		*txt_ptr = isprint(chr) ? chr : '.';

		printf("%02x ", chr);

		if (unaligned) {

			if ((i & 0x7) == 0x7)
				putchar(' ');
		} else {
			if ((addr & 0x7) == 0x7)
				putchar(' ');
		}
	}

	if (unaligned) 
		addr = i;

	for (; addr & 0xf; addr++) {

		if ((addr & 0x7) == 0x7)
			putchar(' ');

		printf("   ");
	}

	*txt_ptr = 0;

	printf("|%s|\n", txt_data);
}

