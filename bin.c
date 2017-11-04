/* writes raw data from a selected section to a file
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
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "bpfa.h"
#include "coff.h"

int bpfa_write_bin_file(struct bpfa *bpfa, struct section_data *bin)
{

	char *raw;
	int fd;
	struct coff_format *coff;
	struct scnhdr **lst;
	struct scnhdr *scn;
	unsigned long total_size;

	lst = bpfa_floor_address(bpfa, bin->s_start);

	if ((*lst)->s_paddr != bin->s_start) {

		fprintf(stderr, "error: %s data not found\n", bin->name);

		return -1;
	}

	scn = *bpfa_floor_address(bpfa, bin->s_end);

	/* ceil file size to modulo 256 */
	total_size = scn->s_paddr - bin->s_start;
	total_size += scn->s_size + 0xff;
	total_size &= 0xffffff00;

	/* note: "O_WRONLY" mode is not sufficient when mmaping */
	if ((fd = open(bin->bin_file, O_RDWR | O_CREAT | O_TRUNC,
					(mode_t)0666)) == -1) {

		perror(bin->bin_file);

		return -1;
	}

	/* stretch the file size to the size of the (mmapped) array of char */
	if (lseek(fd, total_size - 1, SEEK_SET) == -1) {

		perror("error: lseek");

		close(fd);

		return -1;
	}

	/*
	 * Something needs to be written at the end of the file
	 * to have the file actually have the new size.
	 */
	if (write(fd, "", 1) == -1) {

		perror("error: write last val");

		close(fd);

		return -1;
	}

	/* now the file is ready to be mmapped */
	if ((raw = mmap(0, total_size, PROT_READ | PROT_WRITE,
					MAP_SHARED, fd, 0)) == MAP_FAILED) {

		perror("error: mmap");

		close(fd);

		return -1;
	}

	memset(raw, 0xff, total_size);

	for (coff = bpfa->coff; (scn = *lst); lst++) {

		char *rawptr;
		char *scnptr;

		if (scn->s_paddr > bin->s_end)
			break;

		if (!scn->s_size)
			continue;

		scnptr = &((char *)coff)[scn->s_scnptr];

		rawptr = &((char *)raw)[scn->s_paddr - bin->s_start];

		memcpy(rawptr, scnptr, scn->s_size);
	}

	/* write it to disk now */
	if (msync(raw, total_size, MS_SYNC) == -1) {

		perror("error: msync");

		close(fd);

		return -1;
	}

	/* free the mmapped memory */
	if (munmap(raw, total_size) == -1) {

		perror("error: munmap");

		close(fd);

		return -1;
	}

	close(fd);

	return 0;
}

