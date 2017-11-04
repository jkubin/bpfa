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

#if 0
#include <stdio.h>
#include "bpfa.h"
#include "coff.h"

struct coff_lineno **dasmprobe(union symbol_table *file_symbol)
{

	char *name;
	char short_name[9];
	struct scnhdr *scn;
	union symbol_table *last_symbol;
	union symbol_table *parent_sym;
	union symbol_table *symbol;
	unsigned int i;

	short_name[8] = 0;

	last_symbol = &symbol_table[coff->hdr.f_nsyms - 1];

	for (i = 1, symbol = file_symbol + 2;; i++, symbol++) {

		if (symbol->entry.n_scnum && symbol->entry.n_scnum < (unsigned short)N_DEBUG) {

			if (symbol->entry.sym_name.ptr.s_zeros) {

				*((long int *)short_name) = symbol->entry.sym_name.longname;

				name = short_name;
			} else {
				name = &bpfa_string_table[symbol->entry.sym_name.ptr.s_offset];
			}

			scn = &coff->scn[symbol->entry.n_scnum - 1];

			if (
					symbol->entry.n_sclass == C_EXT ||
					symbol->entry.n_sclass == C_STAT ||
					symbol->entry.n_sclass == C_LABEL) {

				if (scn->s_flags & STYP_TEXT) {

					unsigned int line = (symbol->entry.n_value - scn->s_paddr) * (sizeof(struct coff_lineno) / 2);

					struct coff_lineno *lineno = (struct coff_lineno *)&((unsigned char *)coff)[scn->s_lnnoptr + line];

					printf("%s %X code %d\n", name, symbol->entry.n_value, lineno->l_lnno);


				}

			} /* else if (symbol->entry.n_sclass == C_LABEL) {

				if (scn->s_flags & STYP_TEXT)
					printf("%s %X label\n", name, symbol->entry.n_value);
			}
			*/
		}

		if (symbol->entry.n_numaux) {

			parent_sym = symbol;

			symbol++;
			i++;

			if (
					parent_sym->entry.n_sclass == C_FILE &&
					!symbol->file.x_flags &&
					!symbol->file.x_incline) {
				break;
			}
		}

		if (symbol == last_symbol)
			break;
	}

	return NULL;
}
#endif

