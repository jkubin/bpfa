/* sorts sections, finds a section by name or address
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

static int find_name(const void *name, const void *ptr)
{

	const char *name2;
	char short_name2[9];
	struct scnhdr *scn;

	scn = *(struct scnhdr **)ptr;

	if (scn->s_name.ptr.s_zeros) {

		*((long int *)short_name2) = scn->s_name.longname;

		short_name2[8] = 0;

		name2 = short_name2;
	} else {
		name2 = &bpfa_string_table[scn->s_name.ptr.s_offset];
	}

	return strcmp(name, name2);
}

static int cmp_by_name(const void *ptr1, const void *ptr2)
{

	const char *name1;
	const char *name2;
	char short_name1[9];
	char short_name2[9];
	struct scnhdr *scn1;
	struct scnhdr *scn2;

	scn1 = *(struct scnhdr **)ptr1;

	if (scn1->s_name.ptr.s_zeros) {

		*((long int *)short_name1) = scn1->s_name.longname;

		short_name1[8] = 0;

		name1 = short_name1;
	} else {
		name1 = &bpfa_string_table[scn1->s_name.ptr.s_offset];
	}

	scn2 = *(struct scnhdr **)ptr2;

	if (scn2->s_name.ptr.s_zeros) {

		*((long int *)short_name2) = scn2->s_name.longname;

		short_name2[8] = 0;

		name2 = short_name2;
	} else {
		name2 = &bpfa_string_table[scn2->s_name.ptr.s_offset];
	}

	return strcmp(name1, name2);
}

static int cmp_by_address_and_name(const void *scn1, const void *scn2)
{

	unsigned int paddr1 = (*(struct scnhdr **)scn1)->s_paddr;
	unsigned int paddr2 = (*(struct scnhdr **)scn2)->s_paddr;

	if (paddr1 < paddr2)
		return -1;

	if (paddr1 > paddr2)
		return 1;

	return cmp_by_name(scn1, scn2);
}

static int cmp_by_address(unsigned int key, const void *scn2)
{

	unsigned int paddr = (*(struct scnhdr **)scn2)->s_paddr;

	if (key < paddr)
		return -1;

	if (key > paddr)
		return 1;

	return 0;
}

/*
struct scnhdr **find_rom_addr(unsigned int addr)
{
	struct scnhdr **scn;

	if (!(scn = bsearch(&addr, bpfa->rom_scn_by_addr, bpfa->rom_scn_count, sizeof(struct scnhdr *),
	cmp_by_address))) {

		if (bpfa_options & BPFA_VERBOSE_OUTPUT)
			fprintf(stderr, "error: 0x%x not found\n", addr);
	}

	return scn;
}
*/

int bpfa_sort_sections(struct bpfa *bpfa)
{

	struct coff_format *coff;
	struct scnhdr **ram_scn;
	struct scnhdr *scn;
	unsigned int f_nscns;
	unsigned int i;
	unsigned int length;

	coff = bpfa->coff;

	f_nscns = coff->hdr.f_nscns;

	length = (f_nscns + 2) * sizeof(struct scnhdr *);

	if ((bpfa->ram_scn_by_addr = malloc(length)) == NULL) {

		perror("out of memory");

		return -1;
	}

	ram_scn = bpfa->ram_scn_by_addr;

	bpfa->rom_scn_by_addr = &bpfa->ram_scn_by_addr[f_nscns + 1];

	*bpfa->rom_scn_by_addr-- = NULL;
	*ram_scn = NULL;

	scn = coff->scn;

	for (i = 0; i < f_nscns; i++, scn++) {

		if (scn->s_flags & (STYP_TEXT | STYP_DATA_ROM))
			*bpfa->rom_scn_by_addr-- = scn;
		else
			*ram_scn++ = scn;
	}

	bpfa->rom_scn_by_addr++;
	*ram_scn = NULL;

	bpfa->ram_scn_count = ram_scn - bpfa->ram_scn_by_addr;
	bpfa->rom_scn_count = f_nscns - bpfa->ram_scn_count;

	qsort(bpfa->ram_scn_by_addr, bpfa->ram_scn_count,
			sizeof(struct scnhdr *), cmp_by_address_and_name);

	qsort(bpfa->rom_scn_by_addr, bpfa->rom_scn_count,
			sizeof(struct scnhdr *), cmp_by_address_and_name);

	/* init sections by name */

	if ((bpfa->ram_scn_by_name = malloc(length)) == NULL) {

		perror("out of memory");

		return -1;
	}

	bpfa->rom_scn_by_name = &bpfa->ram_scn_by_name[bpfa->ram_scn_count + 1];

	memcpy(bpfa->ram_scn_by_name, bpfa->ram_scn_by_addr, length);

	qsort(bpfa->ram_scn_by_name, bpfa->ram_scn_count,
			sizeof(struct scnhdr *), cmp_by_name);

	qsort(bpfa->rom_scn_by_name, bpfa->rom_scn_count,
			sizeof(struct scnhdr *), cmp_by_name);

	return 0;
}

/*
int findmetadata(char *name, struct scn_metadata *scnmtd)
{
	struct scnhdr **scn, *ptr, key;

	if (!bpfa->ram_scn_by_name) {
		if (init_scn_by_name())
			return -1;
	}

	strcpy(search_string, name);

	key.s_name.ptr.s_zeros = 0;
	key.s_name.ptr.s_offset = search_string - bpfa_string_table;

	ptr = &key;

	scn = bsearch(&ptr, bpfa->rom_scn_by_name, bpfa->rom_scn_count,
	sizeof(struct scnhdr *), cmp_by_name);

	if (scn) {
		scnmtd->hdr = *scn;
		scnmtd->mtd = NULL;

		if ((*scn)->s_scnptr)
			scnmtd->mtd = (struct meta_data *)&((unsigned char *)coff)[(*scn)->s_scnptr];

		return 0;
	}

	if (bpfa_options & BPFA_VERBOSE_OUTPUT)
		fprintf(stderr, "%s not found\n", name);

	return -1;
}
*/

struct scnhdr **bpfa_floor_address(struct bpfa *bpfa, unsigned int key)
{

	int comparison;
	size_t idx;
	size_t l;
	size_t u;
	struct scnhdr **lst;
	struct scnhdr **rom_scn_by_addr;

	rom_scn_by_addr = bpfa->rom_scn_by_addr;

	l = 0;
	u = bpfa->rom_scn_count;

	while (l < u) {

		idx = (l + u) / 2;

		lst = (struct scnhdr **) (((const char *)rom_scn_by_addr) +
				(idx * sizeof(struct scnhdr *)));

		comparison = cmp_by_address(key, lst);

		if (comparison < 0)
			u = idx;
		else if (comparison > 0)
			l = idx + 1;
		else
			return lst;
	}

	l--;

	return (struct scnhdr **) (((const char *)rom_scn_by_addr) +
			(l * sizeof(struct scnhdr *)));
}

struct scnhdr *bpfa_find_scn_by_name(struct bpfa *bpfa, char *name)
{

	struct scnhdr **scn;

	scn = bsearch(name, bpfa->rom_scn_by_name, bpfa->rom_scn_count,
			sizeof(struct scnhdr *), find_name);

	if (scn)
		return *scn;

	fprintf(stderr, "error: %s not found\n", name);

	return NULL;
}

