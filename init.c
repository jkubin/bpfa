/* initialization of the COFF (or maybe later ELF) input file
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "bpfa.h"
#include "coff.h"

const char *bpfa_string_table;
const union symbol_table *bpfa_symbol_table;

int bpfa_init_coff_file(struct bpfa *bpfa)
{

	struct coff_format *coff;
	struct stat stat;

	if ((bpfa->input_fd = open(bpfa->coff_file_name, O_RDWR)) == -1) {

		perror(bpfa->coff_file_name);

		return -1;
	}

	if (fstat(bpfa->input_fd, &stat) == -1) {

		perror(bpfa->coff_file_name);

		close(bpfa->input_fd);

		return -1;
	}

	if ((coff = mmap(0, stat.st_size,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					bpfa->input_fd, 0)) == MAP_FAILED) {

		perror(bpfa->coff_file_name);

		close(bpfa->input_fd);

		return -1;
	}

	bpfa->coff = coff;

	if (coff->hdr.f_magic != MICROCHIP_MAGIC) {

		fprintf(stderr, "error: unknown file format (wrong Microchip magic number)\n");

		close(bpfa->input_fd);

		return -1;
	}

	bpfa->coff_file_size = stat.st_size;

	bpfa_string_table = &((const char *)coff)[
		coff->hdr.f_symptr + coff->hdr.f_nsyms * sizeof(struct syment)
	];

	bpfa_symbol_table = (const union symbol_table *)&((unsigned char *)coff)[
		coff->hdr.f_symptr
	];

	return 0;
}

