/* patch compiled struct array with a 1B or 2B crc unsigned int member
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
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "coff.h"
#include "bpfa.h"

static char err_chr[] =

"error: not a NULL character 0x%02x found\n"

;

static int patch_compiled_array(struct bpfa *bpfa)
{

	struct coff_format *coff;
	union symbol_table *aux;
	union symbol_table *sym_crc;
	union symbol_table *symbol;
	unsigned char *ptr;
	unsigned char crc8;
	unsigned int i;
	unsigned int j;
	unsigned int x;
	unsigned int y;

	union
	{
		unsigned short val;
		struct
		{
			unsigned char lo;
			unsigned char hi;
		};
	} crc16;

	coff = bpfa->coff;

	symbol = (union symbol_table *)bpfa_symbol_table;

	for (i = 0; i < coff->hdr.f_nsyms; i++, symbol++) {

		if (symbol->entry.n_type == (T_STRUCT | (DT_ARY << 5))) {

			if (symbol->entry.n_sclass != C_EXT) {

				fprintf(stderr, "error: not an external symbol\n");

				return -1;
			}

			sym_crc = symbol - 3;

			if (strcmp(sym_crc->entry.sym_name.name, "crc")) {

				fprintf(stderr, "error: struct member crc not found\n");

				return -1;
			}

			aux = symbol + 1;

			if (
					aux->arr.x_dimen[1] ||
					aux->arr.x_dimen[2] ||
					aux->arr.x_dimen[3]) {

				fprintf(stderr, "error: one dimensional array expected\n");

				return -1;
			}

			ptr = &((unsigned char *)coff)[
				coff->scn[symbol->entry.n_scnum - 1].s_scnptr
			];

			x = aux->arr.x_size / aux->arr.x_dimen[0] - 1;
			y = aux->arr.x_dimen[0];

			if (sym_crc->entry.n_type == T_UCHAR) {

				while (y--) {
					crc8 = 0xff;

					for (j = 0; j < x; j++, ptr++)
						crc8 = crc8_table[*ptr ^ crc8];

					if (*ptr == 0) {
						*ptr++ = crc8;
					} else {
						fprintf(stderr, err_chr, *ptr);
						return -1;
					}
				}

				return 0;

			} else if (sym_crc->entry.n_type == T_UINT) {
				x--;

				while (y--) {

					crc16.val = 0xffff;

					for (j = 0; j < x; j++, ptr++)
						crc16.val = crc16_ibm[crc16.lo ^ *ptr] ^ crc16.hi;

					if (*ptr == 0) {
						*ptr++ = crc16.lo;
					} else {
						fprintf(stderr, err_chr, *ptr);
						return -1;
					}

					if (*ptr == 0) {
						*ptr++ = crc16.hi;
					} else {
						fprintf(stderr, err_chr, *ptr);
						return -1;
					}
				}

				return 0;

			} else {

				fprintf(stderr, "error: unknown crc member size (unsigned char or int expected)\n");

				return -1;
			}
		}

		if (symbol->entry.n_numaux) {
			symbol++;
			i++;
		}
	}

	fprintf(stderr, "error: data array not found\n");

	return -1;
}

int bpfa_patch_compiled_array(struct bpfa *bpfa)
{

	int retval = 0;

	if (patch_compiled_array(bpfa))
		retval = -1;

	close(bpfa->input_fd);

	return retval;
}

