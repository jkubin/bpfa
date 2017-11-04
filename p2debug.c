/****
 * 2015/07/01 Josef Kubin
 *
 * $HeadURL: https://bx.cz/vyvoj/inmatutils/bpfa/p2debug.c $
 * $LastChangedDate: 2017-10-24 16:36:40 +0200 (Tue, 24 Oct 2017) $
 * $LastChangedBy: josef $
 * $Revision: 142 $
 */
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
#include <unistd.h>
#include <fcntl.h>
#include "coff.h"
#include "bpfa.h"

/* FIXME quadratic search complexity in symbol table !!! */

#define BASE_ADDR	0x400000

int p2_debug(struct bpfa *bpfa)
{

	const union symbol_table *symbol;
	int fd;
	struct coff_lineno *lineno;
	struct scnhdr *scn;
	unsigned int i;
	unsigned int j;
	unsigned int nscn;
	unsigned int nsyms;
	unsigned int scn_idx;

	nsyms = bpfa->coff->hdr.f_nsyms;
	nscn = bpfa->coff->hdr.f_nscns;

	for (i = 0, scn = bpfa->coff->scn; i < nscn; i++, scn++) {

		if (!(scn->s_flags & (STYP_TEXT | STYP_DATA_ROM)))
			continue;

		if (!scn->s_size)
			continue;

		if (/*scn->s_paddr >= (BASE_ADDR & 0x1ffff) && */scn->s_paddr < 0x20000) {

			scn->s_paddr += BASE_ADDR & 0xfffe0000;

			if (scn->s_lnnoptr) {

				lineno = (struct coff_lineno *)&((unsigned char *)bpfa->coff)[scn->s_lnnoptr];

				for (j = 0; j < scn->s_nlnno; j++, lineno++)
					lineno->l_paddr += BASE_ADDR & 0xfffe0000;
			}

			scn_idx = i + 1;

			//scn_idx = *scn - coff->scn;
			//scn_idx++;

			for (symbol = symbol_table, j = 0; j < nsyms; j++, symbol++) {

				if (symbol->entry.n_scnum == scn_idx)
					symbol->entry.n_value += BASE_ADDR & 0xfffe0000;

				if (symbol->entry.n_numaux) {

					symbol++;
					j++;
				}
			}

			continue;
		}

		if (scn->s_paddr >= BASE_ADDR && scn->s_paddr < (BASE_ADDR + 0x20000)) {

			scn->s_paddr &= 0x1ffff;

			if (scn->s_lnnoptr) {

				lineno = (struct coff_lineno *)&((unsigned char *)bpfa->coff)[scn->s_lnnoptr];

				for (j = 0; j < scn->s_nlnno; j++, lineno++)
					lineno->l_paddr &= 0x1ffff;
			}

			scn_idx = i + 1;

			//scn_idx = *scn - coff->scn;
			//scn_idx++;

			for (symbol = symbol_table, j = 0; j < nsyms; j++, symbol++) {

				if (symbol->entry.n_scnum == scn_idx)
					symbol->entry.n_value &= 0x1ffff;

				if (symbol->entry.n_numaux) {

					symbol++;
					j++;
				}
			}
		}
	}

	bpfa->coff->opt.proc_type = 0xb760;	// PIC18F67J60

	if ((fd = open(bpfa->p2_debug_file, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {

		perror(bpfa->p2_debug_file);

		return -1;
	}

	if (write(fd, bpfa->coff, bpfa->coff_size) != bpfa->coff_size) {

		perror(bpfa->p2_debug_file);

		return -1;
	}

	close(fd);

	return 0;
}
#endif

