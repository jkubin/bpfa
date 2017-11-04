/* prints sorted list of RAM/ROM symbols
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
#include "coff.h"
#include "bpfa.h"

static void print_sym(struct bpfa *bpfa, FILE *fd, struct symbol_item **symlst)
{

	char bfr_flags[100];
	char short_name[9];
	const char *name;
	struct coff_format *coff;
	struct scnhdr *scn;
	struct symbol_item *ptr;

	coff = bpfa->coff;

	short_name[8] = 0;

	for (; (ptr = *symlst); symlst++) {

		const union symbol_table *symbol;
		struct symbol_item sym_item;
		unsigned char arr_idx;
		unsigned int n_type;

		sym_item = *ptr;

		symbol = &bpfa_symbol_table[sym_item.symbol_index];

		if (symbol->entry.sym_name.ptr.s_zeros) {

			*((long int *)short_name) =
				symbol->entry.sym_name.longname;

			name = short_name;
		} else {
			name = &bpfa_string_table[symbol->entry.sym_name.ptr.s_offset];
		}

		scn = &coff->scn[symbol->entry.n_scnum - 1];

		bpfa_decode_scn_flags(bfr_flags, scn->s_flags);

		n_type = symbol->entry.n_type;

		fprintf(fd, "%08x\t%s\t%s\t%s\t%s",
				symbol->entry.n_value,
				bfr_flags,
				bpfa_storage_class(symbol->entry.n_sclass),
				bpfa_base_symbol_types[n_type & 0x1f],
				name);

		if (symbol->entry.n_numaux)
			symbol++;

		for (n_type >>= 5, arr_idx = 0; n_type; n_type >>= 3, arr_idx++) {

			if ((n_type & 0x7) == 3)
				fprintf(fd, "[%d]", symbol->arr.x_dimen[arr_idx]);
			else
				fprintf(fd, bpfa_derrived_symbol_types[n_type & 0x7]);
		}

		if (scn->s_name.ptr.s_zeros) {

			*((long int *)short_name) = scn->s_name.longname;

			name = short_name;
		} else {
			name = &bpfa_string_table[scn->s_name.ptr.s_offset];
		}

		fprintf(fd, "\t\t%s\t\t%s\n",
				&bpfa_string_table[sym_item.x_offset],
				name);
	}
}



int bpfa_print_symbols(struct bpfa *bpfa)
{

	FILE *fd1 = stdout;
	FILE *fd2 = stdout;
	const union symbol_table *symbol;
	struct coff_format *coff;
	struct symbol_item **ram_sym;
	struct symbol_item **rom_sym;
	struct symbol_item **selected_sym;
	struct symbol_item *ptr;
	struct symbol_item *symbol_mtd;
	unsigned int f_nsyms;
	unsigned int i;
	unsigned int ram_sym_count;
	unsigned int rom_sym_count;
	unsigned int symbol_count;
	unsigned int x_offset;

	coff = bpfa->coff;

	f_nsyms = coff->hdr.f_nsyms;

	/* find symbols with certain attributes */
	for (symbol = bpfa_symbol_table, symbol_count = 0, i = 0;
			i < f_nsyms; i++, symbol++) {

		if (
				symbol->entry.n_scnum &&
				symbol->entry.n_scnum < (unsigned short)N_DEBUG) {

			if (
					symbol->entry.n_sclass == C_EXT ||
					symbol->entry.n_sclass == C_STAT ||
					symbol->entry.n_sclass == C_LABEL)
				symbol_count++;
		}

		if (symbol->entry.n_numaux) {

			symbol++;
			i++;
		}
	}

	if ((selected_sym = malloc((symbol_count + 2) * sizeof(struct symbol_item *))) == NULL)
		return -1;

	if ((symbol_mtd = malloc(symbol_count * sizeof(struct symbol_item))) == NULL)
		return -1;

	ram_sym = selected_sym;
	rom_sym = &selected_sym[symbol_count + 1];
	*rom_sym-- = NULL;
	*ram_sym = NULL;

	for (symbol = bpfa_symbol_table, ptr = symbol_mtd, i = 0;
			i < f_nsyms; i++, symbol++) {

		unsigned char n_sclass;

		if (
				symbol->entry.n_scnum &&
				symbol->entry.n_scnum < (unsigned short)N_DEBUG) {

			if (
					symbol->entry.n_sclass == C_EXT ||
					symbol->entry.n_sclass == C_STAT ||
					symbol->entry.n_sclass == C_LABEL) {

				ptr->symbol_index = i;
				ptr->x_offset = x_offset;

				if (coff->scn[symbol->entry.n_scnum - 1].s_flags
						& (STYP_TEXT | STYP_DATA_ROM))
					*rom_sym-- = ptr;
				else
					*ram_sym++ = ptr;

				ptr++;
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

	rom_sym++;
	*ram_sym = NULL;

	ram_sym_count = ram_sym - selected_sym;
	rom_sym_count = symbol_count - ram_sym_count;

	if (bpfa->ram_sym_file) {

		if ((fd1 = fopen(bpfa->ram_sym_file, "wb")) == NULL) {

			perror(bpfa->ram_sym_file);

			return -1;
		}
	}

	if (bpfa->rom_sym_file) {

		if ((fd2 = fopen(bpfa->rom_sym_file, "wb")) == NULL) {

			perror(bpfa->rom_sym_file);

			return -1;
		}
	}

	if (bpfa_options & BPFA_SORT_BY_NAME) {

		if (bpfa_options & BPFA_PRINT_RAM_SYMBOLS) {

			qsort(selected_sym, ram_sym_count, sizeof(struct symbol_item *),
					bpfa_cmp_sym_by_name);

			print_sym(bpfa, fd1, selected_sym);
		}

		if (bpfa_options & BPFA_PRINT_ROM_SYMBOLS) {

			qsort(rom_sym, rom_sym_count, sizeof(struct symbol_item *),
					bpfa_cmp_sym_by_name);

			print_sym(bpfa, fd2, rom_sym);
		}

	} else {
		if (bpfa_options & BPFA_PRINT_RAM_SYMBOLS) {

			qsort(selected_sym, ram_sym_count, sizeof(struct symbol_item *),
					bpfa_cmp_sym_by_addr_and_name);

			print_sym(bpfa, fd1, selected_sym);
		}

		if (bpfa_options & BPFA_PRINT_ROM_SYMBOLS) {

			qsort(rom_sym, rom_sym_count, sizeof(struct symbol_item *),
					bpfa_cmp_sym_by_addr_and_name);

			print_sym(bpfa, fd2, rom_sym);
		}
	}

	if (fd1 != stdout)
		fclose(fd1);

	if (fd2 != stdout)
		fclose(fd2);

	free(symbol_mtd);
	free(selected_sym);

	return 0;
}

