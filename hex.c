/* writes well known Intel HEX file format commonly used for microcontrollers
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
#include "coff.h"

int bpfa_write_hex_file(struct bpfa *bpfa, struct section_data *hex)
{

	FILE *fd;
	struct coff_format *coff;
	struct scnhdr **lst;
	struct scnhdr *floor;
	struct scnhdr *scn;
	unsigned short lineaddr;
	unsigned short prevsegment = 0xffff;

	lst = bpfa_floor_address(bpfa, hex->s_start);

	floor = *lst;

	if (floor->s_paddr != hex->s_start) {

		fprintf(stderr, "error: %s not found\n", hex->name);

		return -1;
	}

	if ((fd = fopen(hex->hex_file, "wb")) == NULL) {

		perror(hex->hex_file);

		return -1;
	}

	for (coff = bpfa->coff; (scn = *lst); lst++) {

		unsigned char *raw;
		unsigned char csum;
		unsigned char i;
		unsigned char len;
		unsigned int size;

		union
		{
			unsigned int all;
			struct
			{
				unsigned short low;
				unsigned short high;
			};
			struct
			{
				unsigned char lo;
				unsigned char hi;
				unsigned char up;
				unsigned char ms;
			};
		} addr;

		if (scn->s_paddr > hex->s_end) {

			if (!hex->s_fuses)
				break;

			scn = *bpfa_floor_address(bpfa, hex->s_fuses);

			if (scn->s_paddr != hex->s_fuses) {

				fprintf(stderr, "warning: %s fuses at address 0x%08x not found\n",
						hex->name, hex->s_fuses);

				break;
			}

			hex->s_fuses = 0;
		}

		if (!scn->s_size)
			continue;

		addr.all = scn->s_paddr - floor->s_paddr;
		size = scn->s_size;

		raw = &((unsigned char *)coff)[scn->s_scnptr];

		do {
			if (prevsegment != addr.high) {

				prevsegment = addr.high;

				csum = 6;
				csum += addr.up;
				csum += addr.ms;

				*((char *)&csum) *= -1;

				/* segment adresss */
				fprintf(fd, ":02000004%04X%02X\n", addr.high, csum);
			}

			lineaddr = addr.low;

			addr.all++;
			len = 1;
			size--;

			if (size) {

				csum = addr.lo;
				*((char *)&csum) *= -1;
				csum &= 0xf;

				if (csum > size)
					csum = size;

				size -= csum;
				len += csum;
				addr.all += csum;
			}

			/* line */
			fprintf(fd, ":%02X%04X00", len, lineaddr);

			csum = len;
			csum += ((unsigned char *)&lineaddr)[0];
			csum += ((unsigned char *)&lineaddr)[1];

			for (i = 0; i < len; i++, raw++) {

				fprintf(fd, "%02X", *raw);
				csum += *raw;
			}

			*((char *)&csum) *= -1;

			fprintf(fd, "%02X\n", csum);

		} while (size);
	}

	/* end of hex */
	fprintf(fd, ":00000001FF\n");

	fclose(fd);

	return 0;
}

