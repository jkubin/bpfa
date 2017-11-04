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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <linux/limits.h>
//#include <features.h>
#include "bpfa.h"
#include "coff.h"



struct coff_lineno **dasmprobe(union symbol_table *);

static union symbol_table * find_symbol(void)
{

	char *ptr;
	union symbol_table *parent_symbol;
	union symbol_table *symbol;
	unsigned int i;

	/*
#define MAX_PATH_LEN	1000
	char path[MAX_PATH_LEN + 1];

	if (strlen(dasm_src_file) > MAX_PATH_LEN) {
		fprintf(stderr, "exceeded max path length\n");

		return NULL;
	}

	strcpy(path, (const void *)dasm_src_file);

	for (ptr = path; (ptr = strchr(ptr, '/')); *ptr++ = '\\');

	*/

	for (ptr = dasm_src_file; (ptr = strchr(ptr, '/')); *ptr++ = '\\');

	for (symbol = symbol_table, i = 0; i < coff->hdr.f_nsyms; i++, symbol++) {

		if (symbol->entry.n_numaux) {

			parent_symbol = symbol;

			symbol++;
			i++;

			if (parent_symbol->entry.n_sclass == C_FILE) {

				if (!symbol->file.x_flags && !symbol->file.x_incline) {

					if (!strcmp(&bpfa_string_table[symbol->file.x_offset + 3], dasm_src_file))
						return parent_symbol;
				}
			}
		}
	}

	fprintf(stderr, "%s not found\n", dasm_src_file);

	return NULL;
}

/*
 * NOT implemented yet
 * this is only a stub
 */

int bpfa_disassembly(void)
{

	// union symbol_table *symbol;
	// unsigned int i;
	// struct coff_lineno ** lineptr;
	FILE *fd;
	char *src, *ptr;
	struct stat src_file;
	union symbol_table *file_symbol;
	unsigned int lines;

	if (!(file_symbol = find_symbol()))
		return -1;

	for (ptr = dasm_src_file; (ptr = strchr(ptr, '\\')); *ptr++ = '/');

	if ((fd = fopen(dasm_src_file, "rb")) == NULL) {

		perror(dasm_src_file);

		return -1;
	}

	if (fstat(fileno(fd), &src_file) == -1) {

		perror(dasm_src_file);

		return -1;
	}

	if (!src_file.st_size)
		return 0;

	if ((src = malloc(src_file.st_size + 1)) == NULL) {

		perror(dasm_src_file);

		return -1;
	}

	if (src_file.st_size != (unsigned int)fread(src, 1, src_file.st_size, fd)) {

		perror(dasm_src_file);

		return -1;
	}

	fclose(fd);

	/*
	 * for a special case, when the file does not end with LF
	 */
	if (src[src_file.st_size - 1] == '\n')
		src[src_file.st_size - 1] = 0;

	src[src_file.st_size] = 0;

	/* remove M$ teletype garbage */
	for (ptr = src; (ptr = strchr(ptr, '\r')); *ptr = ' ', ptr++);

	for (lines = 1, ptr = src; (ptr = strchr(ptr, '\n')); *ptr++ = 0, lines++);

	// lineptr = dasmprobe(file_symbol);



	free(src);

	return 0;
}

#endif

