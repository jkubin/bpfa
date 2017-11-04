/* prints paths of used files from string table
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
#include <string.h>
#include "bpfa.h"
#include "coff.h"



static int cmp_by_name(const void *ptr1, const void *ptr2)
{
	return strcmp(*((char **)ptr1), *((char **)ptr2));
}

int bpfa_print_paths(struct bpfa *bpfa)
{

	const char **paths;
	const char **ptr;
	const char *chr;
	const char *path;
	const char *prev_path;
	const union symbol_table *parent_sym;
	const union symbol_table *symbol;
	unsigned int i;
	unsigned int npaths;
	unsigned int nsyms;

	nsyms = bpfa->coff->hdr.f_nsyms;

	for (symbol = bpfa_symbol_table, npaths = 0, i = 0; i < nsyms; i++, symbol++) {

		if (symbol->entry.n_numaux) {

			if (symbol->entry.n_sclass == C_FILE)
				npaths++;

			symbol++;
			i++;
		}
	}

	//printf("idx: %d; next: %d %s\n", i, i + parent_sym->entry.n_value, &bpfa_string_table[symbol->file.x_offset]);
	//printf("idx: %d; next: %d %s\n", i, i + n_value, &bpfa_string_table[symbol->file.x_offset]);

	if ((paths = malloc((npaths + 1) * sizeof(char *))) == NULL) {

		perror("malloc");

		return -1;
	}

	for (symbol = bpfa_symbol_table,
			ptr = paths, i = 0; i < nsyms; i++, symbol++) {

		if (symbol->entry.n_numaux) {

			parent_sym = symbol;

			symbol++;
			i++;

			if (parent_sym->entry.n_sclass == C_FILE) {

				*ptr = &bpfa_string_table[symbol->file.x_offset];

				/*
				 * cut off a windoze letter with colon
				 * (e.g. C:/path/to/hell.c ---> /path/to/hell.c)
				 *
				if (bpfa_options & BPFA_PRINT_PATHS_UNIX) {
					path = *ptr;

					if (*(path + 1) == ':')
						*ptr += 3;
				}
				*/

				if (bpfa_options & BPFA_PRINT_ALL) {

					ptr++;
				} else {
					if (!symbol->file.x_flags && !symbol->file.x_incline)
						ptr++;
				}
			}
		}
	}

	*ptr = NULL;

	qsort(paths, ptr - paths, sizeof(char *), cmp_by_name);

	for (ptr = paths, prev_path = NULL; (path = *ptr); ptr++) {

		if (prev_path != path) {

			prev_path = path;

			if (bpfa_options & BPFA_PRINT_PATHS_UNIX) {

				for (chr = path; *chr; chr++) {

					if (*chr == '\\') {
						putchar('/');
					} else {
						putchar(*chr);
					}
				}

				putchar('\n');
			} else {
				puts(path);
			}
		}
	}

	free(paths);

	return 0;
}

