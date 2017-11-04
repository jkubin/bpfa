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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "coff.h"
#include "bpfa.h"

static const char header[] =

"address\tsize\tflags\tname\n"

;

const char bpfa_separator[] =

"------------------------------------------------------------------------------\n"

;

void bpfa_decode_scn_flags(char *ptr, unsigned int flags)
{
	*ptr = 0;

	if (flags & STYP_TEXT)
		strcat(ptr, "code");

	if (flags & STYP_DATA)
		strcat(ptr, "idata");

	if (flags & STYP_BSS)
		strcat(ptr, "udata");

	if (flags & STYP_DATA_ROM)
		strcat(ptr, "romdata");

	if (flags & STYP_ABS)
		strcat(ptr, " abs");

	if (flags & STYP_SHARED)
		strcat(ptr, " shared");

	if (flags & STYP_OVERLAY)
		strcat(ptr, " overlay");

	if (flags & STYP_ACCESS)
		strcat(ptr, " access");

	if (flags & STYP_ACTREC)
		strcat(ptr, "actrec");

	if (flags & STYP_RELOC)
		strcat(ptr, " reloc");

	if (flags & STYP_BPACK)
		strcat(ptr, " bpack");
}

int bpfa_print_sections(char *file, struct scnhdr **lst)
{

	FILE *fd = stdout;
	char bfr_flags[100];
	char short_name[9];
	const char *name;
	struct scnhdr *scn;

	short_name[8] = 0;

	if (file) {

		if ((fd = fopen(file, "wb")) == NULL) {

			perror(file);

			return -1;
		}
	}

	if (bpfa_options & BPFA_VERBOSE_OUTPUT)
		fprintf(fd, header);

	for (; (scn = *lst); lst++) {

		if (!(bpfa_options & BPFA_PRINT_ALL))
			if (!scn->s_size)
				continue;

		if (scn->s_name.ptr.s_zeros) {

			*((long int *)short_name) = scn->s_name.longname;

			name = short_name;
		} else {
			name = &bpfa_string_table[scn->s_name.ptr.s_offset];
		}

		bpfa_decode_scn_flags(bfr_flags, scn->s_flags);

		fprintf(fd, "%08x\t%08x\t%s\t%s\n",
				scn->s_paddr,
				scn->s_size,
				bfr_flags,
				name);
	}

	if (fd != stdout)
		fclose(fd);

	return 0;
}

void bpfa_print_ram(struct bpfa *bpfa)
{

	struct scnhdr **lst;
	struct scnhdr *scn;
	unsigned char *ramptr;
	unsigned char chr;
	unsigned char color = 0;
	unsigned char ram[BPFA_PICRAM];
	unsigned int i;

	memset(ram, 0, BPFA_PICRAM);

	for (lst = bpfa->ram_scn_by_addr; (scn = *lst); lst++) {

		if (scn->s_paddr >= BPFA_PICRAM)
			break;

		if (!scn->s_size)
			continue;

		ramptr = ram;
		ramptr += scn->s_paddr;

		for (i = 0; i < scn->s_size; i++, ramptr++)
			*ramptr = color | 0xf8;

		color++;
	}

	//if (bpfa_options & BPFA_VERBOSE_OUTPUT) {
	//	puts("sram allocation");
	//	puts(separ);
	//}

	color = 0;

	puts("0123456789abcdef");

	ramptr = ram;

	for (i = 0; i < BPFA_PICRAM; i++, ramptr++) {

		chr = *ramptr;

		if (bpfa_options & BPFA_COLOR_OUTPUT) {

			if (chr) {
				if (color != chr) {

					color = chr;
					printf("\e[1;3%xm", color & 0x7);
					/* printf("\e[7;1;3%Xm", color & 0x7); */
				}
			} else {
				if (color) {

					color = 0;
					printf("\e[m");
					/* printf("\e[7;30m"); */
				}
			}
		}

		if (chr)
			putchar('x');
		else
			putchar('.');

		if (!((i + 1) & 0xf)) {

			if (bpfa_options & BPFA_COLOR_OUTPUT) {

				if ((i & 0xff) == 0x0f)
					printf("\e[m \e[7;1;30m0x%03x\e[m\n", i & 0xfff0);
				else
					printf("\e[m 0x%03x\n", i & 0xfff0);

				color = 0;

			} else {
				printf(" 0x%03x\n", i & 0xfff0);
			}
		}
	}
}

int bpfa_print_section(struct bpfa *bpfa)
{

	char scn_attr[50];
	struct scnhdr *scn;
	unsigned char *raw;
	unsigned int upper;

	if (!(scn = bpfa_find_scn_by_name(bpfa, bpfa->section_name)))
		return -1;

	if (!scn->s_scnptr) {

		fprintf(stderr, "error: empty section\n");

		return -1;
	}

	raw = &((unsigned char *)bpfa->coff)[scn->s_scnptr];

	if (bpfa_options & BPFA_BINARY_OUTPUT) {

		if (scn->s_size != fwrite(raw, 1, scn->s_size,  stdout)) {

			perror(bpfa->section_name);

			return -1;
		}

		return 0;
	}

	upper = scn->s_paddr + scn->s_size;

	if ((!upper && scn->s_size) || (upper && scn->s_size))
		upper--;

	bpfa_decode_scn_flags(scn_attr, scn->s_flags);

	printf(bpfa_separator);

	printf("%s  %s  (0x%08x - 0x%08x  %d B)\n",
			scn_attr,
			bpfa->section_name,
			scn->s_paddr,
			upper,
			scn->s_size);

	bpfa_hexdump(scn->s_paddr, scn->s_size, /*scn->s_flags, bpfa->section_name, */raw);

	return 0;
}

static void bar(const char *title, unsigned int size, unsigned int total)
{

	float percent = size;
	unsigned char csize;
	unsigned char odd;
	unsigned char scale[51];

	percent /= total;
	percent *= 100;

	csize = percent + 0.5;
	odd = csize & 1;
	csize /= 2;

	scale[sizeof(scale) - 1] = 0;
	memset(scale, ' ', sizeof(scale) - 1);
	memset(scale, '=', csize);

	if (odd)
		scale[csize] = '-';

	printf("%-6s%5.1f %%[%s] %d/%d\n",
			title, percent, scale, size, total);
}





#define USAGE_RAM		0b00000001
#define USAGE_PGM		0b00000010
#define USAGE_RAM_APP		0b00000100
#define USAGE_PGM_APP		0b00001000
#define USAGE_SRAM1		0b00010000
#define USAGE_SRAM2		0b00100000
#define USAGE_FRAM		0b01000000
#define USAGE_DFLASH		0b10000000

void bpfa_memreport(struct bpfa *bpfa)
{


	unsigned int usagemask;

	usagemask = bpfa->usagemask;

	if (usagemask & USAGE_RAM)
		bar("RAM", 94, 100);

	if (usagemask & USAGE_PGM)
		bar("PGM", 73, 100);

	if (usagemask & USAGE_RAM_APP)
		bar("aplRAM", 33, 100);

	if (usagemask & USAGE_PGM_APP)
		bar("aplPGM", 89, 100);

	if (usagemask & USAGE_SRAM1)
		bar("SRAM1", 83, 100);

	if (usagemask & USAGE_SRAM2)
		bar("SRAM2", 57, 100);

	if (usagemask & USAGE_FRAM)
		bar("FRAM", 25, 100);

	if (usagemask & USAGE_DFLASH)
		bar("DFlash", 5, 100);
}

