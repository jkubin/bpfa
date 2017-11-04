/* prints the most interesting COFF file information as an XML output
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

static const char * const base_symbol_type[] = {
	"T_NULL",
	"T_VOID",
	"T_CHAR",
	"T_SHORT",
	"T_INT",
	"T_LONG",
	"T_FLOAT",
	"T_DOUBLE",
	"T_STRUCT",
	"T_UNION",
	"T_ENUM",
	"T_MOE",
	"T_UCHAR",
	"T_USHORT",
	"T_UINT",
	"T_ULONG",
	"T_LNGDBL",
	"T_SLONG",
	"T_USLONG",
};

static const char * const derrived_symbol_type[] = {
	" DT_NON",
	" DT_RAMPTR",
	" DT_FCN",
	" DT_ARY",
	" DT_ROMPTR",
	" DT_FARROMPTR",
};

static char *sclass(unsigned char code)
{

	switch (code) {

		case C_EFCN: return "C_EFCN";
		case C_NULL: return "C_NULL";
		case C_AUTO: return "C_AUTO";
		case C_EXT: return "C_EXT";
		case C_STAT: return "C_STAT";
		case C_REG: return "C_REG";
		case C_EXTDEF: return "C_EXTDEF";
		case C_LABEL: return "C_LABEL";
		case C_ULABEL: return "C_ULABEL";
		case C_MOS: return "C_MOS";
		case C_ARG: return "C_ARG";
		case C_STRTAG: return "C_STRTAG";
		case C_MOU: return "C_MOU";
		case C_UNTAG: return "C_UNTAG";
		case C_TPDEF: return "C_TPDEF";
		case C_USTATIC: return "C_USTATIC";
		case C_ENTAG: return "C_ENTAG";
		case C_MOE: return "C_MOE";
		case C_REGPARM: return "C_REGPARM";
		case C_FIELD: return "C_FIELD";
		case C_AUTOARG: return "C_AUTOARG";
		case C_LASTENT: return "C_LASTENT";
		case C_BLOCK: return "C_BLOCK";
		case C_FCN: return "C_FCN";
		case C_EOS: return "C_EOS";
		case C_FILE: return "C_FILE";
		case C_LINE: return "C_LINE";
		case C_ALIAS: return "C_ALIAS";
		case C_HIDDEN: return "C_HIDDEN";
		case C_EOF: return "C_EOF";
		case C_LIST: return "C_LIST";
		case C_SECTION: return "C_SECTION";
	}

	return "unknown";
}

static const char *reloc_type[] = {
	"-",
	"RELOCT_CALL",
	"RELOCT_GOTO",
	"RELOCT_HIGH",
	"RELOCT_LOW",
	"RELOCT_P",
	"RELOCT_BANKSEL",
	"RELOCT_PAGESEL",
	"RELOCT_ALL",
	"RELOCT_IBANKSEL ",
	"RELOCT_F",
	"RELOCT_TRIS",
	"RELOCT_MOVLR",
	"RELOCT_MOVLB",
	"RELOCT_GOTO2 or RELOCT_CALL2",
	"RELOCT_FF1",
	"RELOCT_FF2",
	"RELOCT_LFSR1",
	"RELOCT_LFSR2",
	"RELOCT_BRA or RELOCT_RCALL",
	"RELOCT_CONDBRA",
	"RELOCT_UPPER",
	"RELOCT_ACCESS",
	"RELOCT_PAGESEL_WREG",
	"RELOCT_PAGESEL_BITS",
	"RELOCT_SCNSZ_LOW",
	"RELOCT_SCNSZ_HIGH",
	"RELOCT_SCNSZ_UPPER ",
	"RELOCT_SCNEND_LOW",
	"RELOCT_SCNEND_HIGH",
	"RELOCT_SCNEND_UPPER",
	"RELOCT_SCNEND_LFSR1",
	"RELOCT_SCNEND_LFSR2",
	"RELOCT_TRIS_4BIT",
	"RELOCT_PAGESEL_MOVLP",
};

void bpfa_print_coff_info(struct bpfa *bpfa)
{

	//unsigned short *instr;
	char bfr_flags[50];
	char short_name[9];
	const char *name;
	const union symbol_table *symbol;
	struct coff_format *coff;
	struct coff_lineno *lineno;
	struct reloc_entry *reloc;
	struct scnhdr *scn;
	unsigned int addr;
	unsigned int i;
	unsigned int j;
	unsigned int total;

	bpfa_print_hdr(bpfa, stdout);

	coff = bpfa->coff;

	scn = coff->scn;

	total = coff->hdr.f_nscns;

	short_name[8] = 0;

	for (i = 0; i < total; i++, scn++) {

		if (scn->s_name.ptr.s_zeros) {

			*((long int *)short_name) = scn->s_name.longname;

			name = short_name;
		} else {
			name = &bpfa_string_table[scn->s_name.ptr.s_offset];
		}

		addr = (unsigned long)scn - (unsigned long)coff;

		bpfa_decode_scn_flags(bfr_flags, scn->s_flags);

		printf("<section type=\"%s\" ptr=\"0x%x\" idx=\"%d\" s_name=\"%016lx\" str=\"%s\" "
				"s_paddr=\"0x%x\" s_vaddr=\"0x%x\" s_size=\"%d\" s_scnptr=\"0x%x\" "
				"s_relptr=\"0x%x\" s_lnnoptr=\"0x%x\" s_nreloc=\"%d\" s_nlnno=\"%d\">",
				bfr_flags,
				addr,
				i,
				scn->s_name.longname,
				name,
				scn->s_paddr,
				scn->s_vaddr,
				scn->s_size,
				scn->s_scnptr,
				scn->s_relptr,
				scn->s_lnnoptr,
				scn->s_nreloc,
				scn->s_nlnno);

		if (scn->s_lnnoptr || scn->s_relptr) {

			puts("");

			if (scn->s_relptr) {

				reloc = (struct reloc_entry *)&((unsigned char *)coff)[scn->s_relptr];

				for (j = 0; j < scn->s_nreloc; j++, reloc++) {

					addr = (unsigned long)reloc - (unsigned long)coff;

					printf("<reloc ptr=\"0x%x\" r_vaddr=\"0x%x\" r_symndx=\"%d\" "
							"r_offset=\"%d\" r_type=\"%s\" />\n",
							addr,
							reloc->r_vaddr,
							reloc->r_symndx,
							reloc->r_offset,
							reloc_type[reloc->r_type]);
				}
			}

			if (scn->s_lnnoptr) {

				lineno = (struct coff_lineno *)&((unsigned char *)coff)[scn->s_lnnoptr];

				for (j = 0; j < scn->s_nlnno; j++, lineno++) {

					addr = (unsigned long)lineno - (unsigned long)coff;

					//instr = *((unsigned short *)&(((unsigned char *)coff)[scn->s_scnptr + (lineno->l_paddr - scn->s_paddr)]));

					printf("<lineno ptr=\"0x%x\" l_srcndx=\"%d\" l_lnno=\"%d\" "
							"l_paddr=\"0x%x\" raw=\"%04x\" l_flags=\"%d\" l_fcnndx=\"%d\" />\n",
							addr,
							lineno->l_srcndx,
							lineno->l_lnno,
							lineno->l_paddr,
							*((unsigned short *)&(((unsigned char *)coff)[scn->s_scnptr + (lineno->l_paddr - scn->s_paddr)])),
							//*instr,
							lineno->l_flags,
							lineno->l_fcnndx);
				}
			}
		}

		puts("</section>");
	}

	symbol = bpfa_symbol_table;
	total = coff->hdr.f_nsyms;

	for (i = 0; i < total; i++, symbol++) {

		unsigned char derrived_type;
		unsigned char n_sclass;
		unsigned int n_type;

		if (symbol->entry.sym_name.ptr.s_zeros) {

			*((long int *)short_name) = symbol->entry.sym_name.longname;

			name = short_name;
		} else {
			name = &bpfa_string_table[symbol->entry.sym_name.ptr.s_offset];
		}

		addr = (unsigned long)symbol - (unsigned long)coff;

		printf("<symbol ptr=\"0x%x\" idx=\"%d\" n_name=\"%016lx\" "
				"str=\"%s\" n_value=\"0x%x\" n_scnum=\"%d\" n_type=\"",
				addr,
				i,
				symbol->entry.sym_name.longname,
				name,
				symbol->entry.n_value,
				symbol->entry.n_scnum);

		n_type = symbol->entry.n_type;

		printf(base_symbol_type[n_type & 0x1f]);

		for (n_type >>= 5; n_type; n_type >>= 3)
			printf(derrived_symbol_type[n_type & 0x7]);

		n_type = symbol->entry.n_type;

		printf("\" n_sclass=\"%s\" n_numaux=\"%d\">",
				sclass(n_sclass = symbol->entry.n_sclass),
				symbol->entry.n_numaux);

		if (symbol->entry.n_numaux) {

			symbol++;
			i++;

			addr = (unsigned long)symbol - (unsigned long)coff;

			printf("\n<aux ptr=\"0x%x\" idx=\"%d\" ", addr, i);

			switch (n_sclass) {
				case C_STAT:
				case C_EXT:
					switch (n_type & 0x1F) {
						case T_STRUCT:
						case T_UNION:
						case T_ENUM:
							printf("x_tagndx=\"%d\" x_size=\"%d\" ",
									symbol->var.x_tagndx,
									symbol->var.x_size);
							break;
					}
					derrived_type = (n_type >> 5) & 0x7;
					switch (derrived_type) {
						case DT_FCN:
							printf("x_tagndx=\"%d\" x_size=\"%d\" x_lnnoptr=\"0x%x\" "
									"x_endndx=\"%d\" x_actscnum=\"%d\" ",
									symbol->fcn.x_tagndx,
									symbol->fcn.x_size,
									symbol->fcn.x_lnnoptr,
									symbol->fcn.x_endndx,
									symbol->fcn.x_actscnum);
							break;
						case DT_ARY:
							printf("x_tagndx=\"%d\" x_lnno=\"%d\" x_size=\"%d\" "
									"x_dim0=\"%d\" x_dim1=\"%d\" x_dim2=\"%d\" x_dim3=\"%d\" ",
									symbol->arr.x_tagndx,
									symbol->arr.x_lnno,
									symbol->arr.x_size,
									symbol->arr.x_dimen[0],
									symbol->arr.x_dimen[1],
									symbol->arr.x_dimen[2],
									symbol->arr.x_dimen[3]);
							break;
					}
					break;
				case C_BLOCK:
				case C_FCN:
					printf("x_lnno=\"%d\" x_endndx=\"%d\" ",
							symbol->bobf.x_lnno,
							symbol->bobf.x_endndx);
					break;
					/*
					printf("x_tagndx=\"%d\" x_size=\"%d\" x_lnnoptr=\"0x%X\" x_endndx=\"%d\" x_actscnum=\"%d\" ",
							symbol->fcn.x_tagndx,
							symbol->fcn.x_size,
							symbol->fcn.x_lnnoptr,
							symbol->fcn.x_endndx,
							symbol->fcn.x_actscnum);
					break;
					*/
				case C_ENTAG:
				case C_STRTAG:
				case C_UNTAG:
					printf("x_size=\"%d\" x_endndx=\"%d\" ",
							symbol->tag.x_size,
							symbol->tag.x_endndx);
					break;
				case C_EOS:
					printf("x_tagndx=\"%d\" x_size=\"%d\" ",
							symbol->eos.x_tagndx,
							symbol->eos.x_size);
					break;
				case C_MOS:
				case C_MOU:
					printf("x_tagndx=\"%d\" x_size=\"%d\" ",
							symbol->var.x_tagndx,
							symbol->var.x_size);
					break;
				case C_FILE:
					printf("str=\"%s\" x_offset=\"0x%x\" x_incline=\"%d\" x_flags=\"%d\" ",
							&bpfa_string_table[symbol->file.x_offset],
							symbol->file.x_offset,
							symbol->file.x_incline,
							symbol->file.x_flags);
					break;
				case C_FIELD:
					printf("x_size=\"%d\" ",
							symbol->field.x_size);
					break;
				case C_SECTION:
					printf("x_scnlen=\"%d\" x_nreloc=\"%d\" x_nlinno=\"%d\" ",
							symbol->scn.x_scnlen,
							symbol->scn.x_nreloc,
							symbol->scn.x_nlinno);
					break;
			}

			printf("raw=\"");

			for (j = 0; j < sizeof(union symbol_table); j++)
				printf("%02x", ((unsigned char *)symbol)[j]);

			printf("\" />");
		}

		puts("</symbol>");
	}

	puts("</coff>");
}

