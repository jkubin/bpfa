/* patches section(s) by AUX_CRC section in COFF file
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
#include "coff.h"
#include "bpfa.h"

/*
 * the following structure declaration is the same as
 * one structure record from AUX_CRC section
 */



struct aux_crc_data
{
	unsigned int offset;
	unsigned short length;
	unsigned short size;	// size | 0x8000 = crc16
};



#if 0
static int crc8_patch(struct aux_crc_data metadata, unsigned char *raw, unsigned char *limit)
{
	unsigned short i, j;

	// size of an array item - minus CRC space
	metadata.size--;

	for (i = 0; i < metadata.length; i++) {

		unsigned char crc = 0xff;

		for (j = 0; j < metadata.size; j++, raw++)
			crc = crc8_table[*raw ^ crc];

		if (limit <= raw)
			return -1;

		// save CRC and skip to a next array item
		*raw++ = crc;
	}

	return 0;
}

// simple block of an inicialized data
static int blkcrc(struct aux_crc_data metadata, unsigned char *raw, unsigned char *limit)
{
	unsigned short i;
	unsigned char crc = 0xff;

	for (i = 0; i < metadata.size; i++, raw++)
		crc = crc8_table[*raw ^ crc];

	if (limit <= raw)
		return -1;

	*raw = crc;

	return 0;
}

// make doubly linked XOR list
static int dllcrc(struct aux_crc_data metadata, unsigned char *raw, unsigned char *limit)
{
	unsigned char crc, val;
	unsigned short i, num_of_items;

	num_of_items = metadata.size - 1;

	for (i = 0; i < num_of_items; i++) {
		val = ((i + 1) % num_of_items) ^ ((num_of_items - 1 + i) % num_of_items);

		if (limit <= raw)
			return -1;

		*raw++ = val;

		crc = crc8_table[val ^ 0xff];

		if (limit <= raw)
			return -1;

		*raw++ = crc;
	}

	i--;

	if (limit <= raw)
		return -1;

	*raw++ = i;

	crc = crc8_table[i ^ 0xff];

	if (limit <= raw)
		return -1;

	*raw = crc;

	return 0;
}

// make a simple list
static int lstcrc(struct aux_crc_data metadata, unsigned char *raw, unsigned char *limit)
{

	unsigned char crc;
	unsigned short i;

	for (i = 0; i < metadata.size; i++) {
		if (limit <= raw)
			return -1;

		*raw++ = i;

		crc = crc8_table[i ^ 0xff];

		if (limit <= raw)
			return -1;

		*raw++ = crc;
	}

	return 0;
}

static int (* patch_item[])(struct aux_crc_data, unsigned char *, unsigned char *) = {
	arrcrc,
	blkcrc,
	dllcrc,
	lstcrc,
};
#endif



static const char crc_err[] =

"error: 0x%08lx CRC address contains 0x%02x; symbol 0x%08x\n"

;

int bpfa_patch_by_aux_section(struct bpfa *bpfa)
{

	struct aux_crc_data *crc_ptr;
	struct aux_crc_data crc_data;
	struct scnhdr *scn;
	unsigned char *base;
	unsigned char *raw;
	unsigned char patch;
	unsigned int i;
	unsigned int item;
	unsigned int j;
	unsigned int k;
	unsigned int max_addr_crc;
	unsigned int max_addr_scn;
	unsigned int n;
	unsigned int offset;
	unsigned int size;
	unsigned long addr;

	union
	{
		unsigned short all;
		struct
		{
			unsigned char lo;
			unsigned char hi;
		};
	} crc;

	if (!(scn = bpfa_find_scn_by_name(bpfa, bpfa->crc_section)))
		return -1;

	/*
	if (!scn->s_scnptr) {

		fprintf(stderr, "error: empty crc section\n");

		return NULL;
	}
	*/

	n = scn->s_size;
	crc_ptr = (struct aux_crc_data *)&((char *)bpfa->coff)[scn->s_scnptr];

	if (n & 0x7) {

		fprintf(stderr, "error: wrong CRC data alignment\n");

		return -1;
	}

	n >>= 3;

	for (i = 0; i < n; i++, crc_ptr++) {

		crc_data = *crc_ptr;

		/* remove CRC16 flag from aux CRC record */
		size = item = crc_data.size & ~BPFA_CRC16_FLAG;

		size *= crc_data.length;

		scn = *bpfa_floor_address(bpfa, crc_data.offset);

		max_addr_scn = scn->s_paddr + scn->s_size;
		max_addr_crc = crc_data.offset + size;

		if (max_addr_crc > max_addr_scn) {

			const char *name;
			char short_name[9];

			if (scn->s_name.ptr.s_zeros) {

				*((long int *)short_name) = scn->s_name.longname;

				short_name[8] = 0;

				name = short_name;
			} else {
				name = &bpfa_string_table[scn->s_name.ptr.s_offset];
			}



			fprintf(stderr, "error: CRC address 0x%08x is out of ‘%s’ 0x%08x - 0x%08x  (%d B)\n",
					max_addr_crc - 1,
					name,
					scn->s_paddr,
					max_addr_scn - 1,
					scn->s_size);

			return -1;
		}

		offset = crc_data.offset - scn->s_paddr;

		base = &((unsigned char *)bpfa->coff)[scn->s_scnptr];

		raw = &((unsigned char *)bpfa->coff)[scn->s_scnptr + offset];

		/* size of an array item - minus CRC space */
		item--;
		if (crc_data.size & BPFA_CRC16_FLAG) {
			item--;

			for (j = 0; j < crc_data.length; j++) {

				crc.all = 0xffff;

				for (k = 0; k < item; k++, raw++)
					crc.all = crc16_ibm[crc.lo ^ *raw] ^ crc.hi;

				addr = (raw - base) + scn->s_paddr;

				if (bpfa_options & BPFA_VERBOSE_OUTPUT) {

					fprintf(stderr, "CRC16  0x%08lx\n", addr);
				}

				patch = *raw;

				if (patch) {

					fprintf(stderr, crc_err, addr, patch, crc_data.offset);

					return -1;
				}

				*raw++ = crc.lo;

				patch = *raw;

				if (patch) {

					fprintf(stderr, crc_err, addr, patch, crc_data.offset);

					return -1;
				}

				*raw++ = crc.hi;
			}

		} else {

			for (j = 0; j < crc_data.length; j++) {

				crc.all = 0xffff;

				for (k = 0; k < item; k++, raw++)
					crc.lo = crc8_table[*raw ^ crc.lo];

				addr = (raw - base) + scn->s_paddr;

				if (bpfa_options & BPFA_VERBOSE_OUTPUT) {

					fprintf(stderr, "CRC8   0x%08lx\n", addr);
				}

				patch = *raw;

				if (patch) {

					fprintf(stderr, crc_err, addr, patch, crc_data.offset);

					return -1;
				}

				*raw++ = crc.lo;
			}
		}
	}

	return 0;
}

