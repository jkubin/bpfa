/* prints detailed symbol information (called from Vim)
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
#include "coff.h"
#include "bpfa.h"

static const unsigned char base_type_len[] = {
	0,	/* T_NULL */
	0,	/* T_VOID */
	1,	/* T_CHAR */
	2,	/* T_SHORT */
	2,	/* T_INT */
	4,	/* T_LONG */
	4,	/* T_FLOAT */
	4,	/* T_DOUBLE */
	0,	/* T_STRUCT */
	0,	/* T_UNION */
	0,	/* T_ENUM */
	0,	/* T_MOE */
	1,	/* T_UCHAR */
	2,	/* T_USHORT */
	2,	/* T_UINT */
	4,	/* T_ULONG */
	4,	/* T_LNGDBL */
	3,	/* T_SLONG */
	3,	/* T_USLONG */
};

int bpfa_print_symbol(struct bpfa *bpfa)
{

	char scn_attr[100];
	char short_name[9];
	const char *name;
	const union symbol_table *aux_symbol;
	const union symbol_table *symbol;
	struct coff_format *coff;
	struct scnhdr *scn;
	unsigned char arr_idx;
	unsigned char n_sclass;
	unsigned int found = 0;
	unsigned int i;
	unsigned int len;
	unsigned int n_type;
	unsigned int nsyms;
	unsigned int upper;
	unsigned int x_offset;

	short_name[8] = 0;

	coff = bpfa->coff;

	nsyms = coff->hdr.f_nsyms;

	for (symbol = bpfa_symbol_table, i = 0; i < nsyms; i++, symbol++) {

		if (
				symbol->entry.n_scnum &&
				symbol->entry.n_scnum < (unsigned short)N_DEBUG) {

			if (symbol->entry.sym_name.ptr.s_zeros) {

				*((long int *)short_name) = symbol->entry.sym_name.longname;

				name = short_name;
			} else {
				name = &bpfa_string_table[symbol->entry.sym_name.ptr.s_offset];
			}

			if (!strcmp(name, bpfa->symbol_name)) {

				scn = &coff->scn[symbol->entry.n_scnum - 1];

				if (scn->s_name.ptr.s_zeros) {

					*((long int *)short_name) = scn->s_name.longname;

					name = short_name;
				} else {
					name = &bpfa_string_table[scn->s_name.ptr.s_offset];
				}

				if (symbol->entry.n_numaux) {

					aux_symbol = symbol + 1;

					len = aux_symbol->var.x_size;

				} else {

					len = base_type_len[symbol->entry.n_type & 0x1f];
				}

				bpfa_decode_scn_flags(scn_attr, scn->s_flags);

				n_type = symbol->entry.n_type;

				upper = symbol->entry.n_value + len;

				//if ((!upper && len) || (upper && len))
					//upper--;

				printf(bpfa_separator);

				printf("0x%08x + 0x%02x (%d B) = 0x%08x\t%s %s %s %s %s",
						symbol->entry.n_value,
						len,
						len,
						upper,
						name,
						scn_attr,
						bpfa_storage_class(symbol->entry.n_sclass),
						bpfa_base_symbol_types[n_type & 0x1f],
						bpfa->symbol_name);

				/*
				printf("%08x  %s  %s  %s  %s",
						symbol->entry.n_value,
						scn_attr,
						bpfa_storage_class(symbol->entry.n_sclass),
						base_symbol_types[n_type & 0x1f],
						bpfa->symbol_name);
						*/

				for (n_type >>= 5, arr_idx = 0; n_type; n_type >>= 3, arr_idx++) {

					if ((n_type & 0x7) == 3)
						printf("[%d]", aux_symbol->arr.x_dimen[arr_idx]);
					else
						printf(bpfa_derrived_symbol_types[n_type & 0x7]);
				}



				printf("\t%s\n",
						&bpfa_string_table[x_offset]);

				if (scn->s_flags & STYP_DATA_ROM) {

					bpfa_hexdump(symbol->entry.n_value, len, // scn->s_flags, name,
							&(((unsigned char *)coff)[scn->s_scnptr + (symbol->entry.n_value - scn->s_paddr)]));
				}

				found = -1;
				// return 0;
			}
		}

		if (symbol->entry.n_numaux) {

			n_sclass = symbol->entry.n_sclass;
			symbol++;
			i++;

			if (n_sclass == C_FILE) {

				if (!symbol->file.x_flags && !symbol->file.x_incline)
					x_offset = symbol->file.x_offset;
			}
		}
	}

	if (!found) {

		fprintf(stderr, "%s not found\n", bpfa->symbol_name);

		return -1;
	}

	return 0;
}

