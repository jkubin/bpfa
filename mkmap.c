/* prints map file and symbol info as a shortcut in Vim
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
// #include <openssl/md5.h>
#include "coff.h"
#include "bpfa.h"

const char * const bpfa_base_symbol_types[] = {
	"null",
	"void",
	"char",
	"short",
	"int",
	"long",
	"float",
	"double",
	"struct",
	"union",
	"enum",
	"member_of_enum",
	"unsigned char",
	"unsigned short",
	"unsigned int",
	"unsigned long",
	"long_double",
	"short long",
	"unsigned short long",
};

const char * const bpfa_derrived_symbol_types[] = {
	"",
	" ram *",
	"()",
	"[]",
	" rom near *",
	" rom far *",
};

static void hdr_flags(char *ptr, unsigned short f_flags)
{
	*ptr = 0;

	if (f_flags & F_RELFLG)
		strcat(ptr, "F_RELFLG ");

	if (f_flags & F_EXEC)
		strcat(ptr, "F_EXEC ");

	if (f_flags & F_LNNO)
		strcat(ptr, "F_LNNO ");

	if (f_flags & F_ABSOLUTE)
		strcat(ptr, "F_ABSOLUTE ");

	if (f_flags & L_SYMS)
		strcat(ptr, "L_SYMS ");

	if (f_flags & F_EXTENDED18)
		strcat(ptr, "F_EXTENDED18 ");

	if (f_flags & F_GENERIC)
		strcat(ptr, "F_GENERIC ");
}

char *bpfa_storage_class(unsigned int cl)
{

	switch (cl) {
		case C_EFCN:
			return "end of function";
		case C_NULL:
			return "null";
		case C_AUTO:
			return "auto";
		case C_EXT:
			return "extern";
		case C_STAT:
			return "static";
		case C_REG:
			return "register";
		case C_EXTDEF:
			return "external definition";
		case C_LABEL:
			return "label";
		case C_ULABEL:
			return "undefined label";
		case C_MOS:
			return "member of structure";
		case C_ARG:
			return "function argument";
		case C_STRTAG:
			return "structure tag";
		case C_MOU:
			return "member of union";
		case C_UNTAG:
			return "union tag";
		case C_TPDEF:
			return "type definition";
		case C_USTATIC:
			return "undefined static";
		case C_ENTAG:
			return "enumeration tag";
		case C_MOE:
			return "member of enumeration";
		case C_REGPARM:
			return "register parameter";
		case C_FIELD:
			return "bit field";
		case C_AUTOARG:
			return "auto argument";
		case C_LASTENT:
			return "dummy entry (end of block)";
		case C_BLOCK:
			return ".bb or .eb";
		case C_FCN:
			return ".bf or .ef";
		case C_EOS:
			return "end of structure";
		case C_FILE:
			return "file name";
		case C_LINE:
			return "line number reformatted as symbol table entry";
		case C_ALIAS:
			return "duplicate tag";
		case C_HIDDEN:
			return "ext symbol in dmert public lib";
		case C_EOF:
			return "end of file";
		case C_LIST:
			return "absoulte listing on or off";
		case C_SECTION:
			return "section";
	}

	return "coff error";
}

int bpfa_cmp_sym_by_name(const void *ptr1, const void *ptr2)
{

	const char *name1;
	const char *name2;
	char short_name1[9];
	char short_name2[9];

	const union symbol_table *symbol1 =
		&bpfa_symbol_table[(*((struct symbol_item **)ptr1))->symbol_index];

	const union symbol_table *symbol2 =
		&bpfa_symbol_table[(*((struct symbol_item **)ptr2))->symbol_index];

	if (symbol1->entry.sym_name.ptr.s_zeros) {

		*((long int *)short_name1) = symbol1->entry.sym_name.longname;

		short_name1[8] = 0;

		name1 = short_name1;
	} else {
		name1 = &bpfa_string_table[symbol1->entry.sym_name.ptr.s_offset];
	}

	if (symbol2->entry.sym_name.ptr.s_zeros) {

		*((long int *)short_name2) = symbol2->entry.sym_name.longname;

		short_name2[8] = 0;

		name2 = short_name2;
	} else {
		name2 = &bpfa_string_table[symbol2->entry.sym_name.ptr.s_offset];
	}

	return strcmp(name1, name2);
}

int bpfa_cmp_sym_by_addr_and_name(const void *ptr1, const void *ptr2)
{

	const union symbol_table *item1 =
		&bpfa_symbol_table[(*((struct symbol_item **)ptr1))->symbol_index];

	const union symbol_table *item2 =
		&bpfa_symbol_table[(*((struct symbol_item **)ptr2))->symbol_index];

	if (item1->entry.n_value < item2->entry.n_value)
		return -1;

	if (item1->entry.n_value > item2->entry.n_value)
		return 1;

	return bpfa_cmp_sym_by_name(ptr1, ptr2);
}

#if 0
static int printsect(FILE *fd, unsigned int start, unsigned int stop, unsigned int mask, unsigned int flags)
{
	char bfr_flags[50], short_name[9], first_sect;
	struct scnhdr *scn;
	unsigned int i, n_type;
	unsigned short prev_section = -1;

	short_name[8] = 0;

	first_sect = -1;

	for (i = start; i < symbol_count; i++) {
		struct symbol_item symitem = *selected_sym[i];
		union symbol_table *symbol = &symbol_table[symitem.symbol_index];

		if (symbol->entry.n_value > stop) {

			if (!first_sect)
				fprintf(fd, "</section>\n");

			return i;	// FIXME dodelat stop podminku
		}

		scn = &coff->scn[symbol->entry.n_scnum - 1];

		if (scn->s_flags & flags) {
			char *name;

			if (prev_section != symbol->entry.n_scnum) {
				prev_section = symbol->entry.n_scnum;

				if (scn->s_name.ptr.s_zeros) {
					*((long int *)short_name) = scn->s_name.longname;
					name = short_name;
				} else {
					name = &bpfa_string_table[scn->s_name.ptr.s_offset];
				}

				bpfa_decode_scn_flags(bfr_flags, scn->s_flags);

				if (first_sect) {
					first_sect = 0;
				} else {
					fprintf(fd, "</section>\n");
				}

				fprintf(fd, "<section type=\"%s\" name=\"%s\" start=\"%08X\" end=\"%08X\" size=\"%d\">\n",
						bfr_flags,
						name,
						scn->s_paddr,
						scn->s_paddr + scn->s_size,
						scn->s_size);
			}

			if (symbol->entry.sym_name.ptr.s_zeros) {
				*((long int *)short_name) = symbol->entry.sym_name.longname;
				name = short_name;
			} else {
				name = &bpfa_string_table[symbol->entry.sym_name.ptr.s_offset];
			}

			n_type = symbol->entry.n_type;

			fprintf(fd, "%08X\t%s\t%s\t%s",
					symbol->entry.n_value & mask,
					bpfa_storage_class(symbol->entry.n_sclass),
					bpfa_base_symbol_types[n_type & 0x1f],
					name);

			for (n_type >>= 5; n_type; n_type >>= 3)
				fprintf(fd, bpfa_derrived_symbol_types[n_type & 0x7]);

			fprintf(fd, "\t%s\n", &bpfa_string_table[symitem.x_offset]);
		}
	}

	if (!first_sect)
		fprintf(fd, "</section>\n");

	return -1;
}
#endif

static void printdate(FILE *fd, time_t t)
{

	struct tm tm = *localtime(&t);

	tm.tm_year -= 100;
	tm.tm_mon++;

	fprintf(fd, "<date>2%03u%02u%02u-%02u:%02u:%02u</date>",
			tm.tm_year,
			tm.tm_mon,
			tm.tm_mday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec);
}

void bpfa_print_hdr(struct bpfa *bpfa, FILE *fd)
{

	// MD5_CTX c;
	// unsigned char i;
	// unsigned char md5[MD5_DIGEST_LENGTH];
	char bfr_flags[100];
	struct coff_format *coff;
	struct filehdr hdr;
	struct opthdr opt;

	coff = bpfa->coff;

	opt = coff->opt;
	hdr = coff->hdr;

	fprintf(fd, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<coff>\n");

	printdate(fd, time(NULL));



	hdr_flags(bfr_flags, coff->hdr.f_flags);

	// fprintf(fd, "<file><name>%s</name><size>%d</size><md5>",
	fprintf(fd, "<file><name>%s</name><size>%d</size>",
			bpfa->coff_file_name, bpfa->coff_file_size);

	/*
	MD5_Init(&c);
	MD5_Update(&c, (const void *)coff, bpfa->coff_file_size);
	MD5_Final(md5, &c);

	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
		fprintf(fd, "%02x", md5[i]);

	fprintf(fd, "</md5></file>\n<header magic=\"0x%x\" nscns=\"%d\" timdat=\"0x%x\" "
		*/
	fprintf(fd, "</file>\n<header magic=\"0x%x\" nscns=\"%d\" timdat=\"0x%x\" "
			"symptr=\"0x%x\" nsyms=\"%d\" opthdr=\"%d\" flags=\"0x%x\"><flags>%s</flags>",
			hdr.f_magic,
			hdr.f_nscns,
			hdr.f_timdat,
			hdr.f_symptr,
			hdr.f_nsyms,
			hdr.f_opthdr,
			hdr.f_flags,
			bfr_flags);

	printdate(fd, hdr.f_timdat);

	fprintf(fd, "</header>\n<opthdr magic=\"0x%x\" vstamp=\"0x%x\" proc_type=\"0x%x\" "
			"rom_width_bits=\"%d\" ram_width_bits=\"%d\"><mcu>%s</mcu></opthdr>\n",
			opt.opt_magic,
			opt.vstamp,
			opt.proc_type,
			opt.rom_width_bits,
			opt.ram_width_bits,
			bpfa_part_name(opt.proc_type));
}

int bpfa_make_map_file(struct bpfa *bpfa)
{

	// FILE *fd;
	char bfr_flags[50];
	char short_name[9];
	const char *name;
	const union symbol_table *symbol;
	struct coff_format *coff;
	struct symbol_item **ram_sym;
	struct symbol_item **rom_sym;
	struct symbol_item **selected_sym;
	struct symbol_item **sym_ptr;
	struct symbol_item *ptr;
	struct symbol_item *symbol_mtd;
	unsigned int f_nsyms;
	unsigned int i;
	unsigned int ram_sym_count;
	unsigned int rom_sym_count;
	unsigned int symbol_count;
	unsigned int x_offset;

	short_name[8] = 0;

	coff = bpfa->coff;

	f_nsyms = coff->hdr.f_nsyms;

	/* find interesting symbols */
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

	if ((selected_sym = malloc((symbol_count + 2) *
					sizeof(struct symbol_item *))) == NULL)
		return -1;

	if ((symbol_mtd = malloc(symbol_count *
					sizeof(struct symbol_item))) == NULL)
		return -1;

	/*
	   if ((fd = fopen(map_file, "wb")) == NULL) {

	   perror(map_file);

	   return -1;
	   }
	   */

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

				if (coff->scn[symbol->entry.n_scnum - 1].s_flags &
						(STYP_TEXT | STYP_DATA_ROM))
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
	sym_ptr = selected_sym;

	if (bpfa_options & BPFA_SORT_BY_NAME) {

		if (bpfa_options & BPFA_PRINT_RAM_SYMBOLS) {

			qsort(selected_sym, ram_sym_count, sizeof(struct symbol_item *),
					bpfa_cmp_sym_by_name);
		} else {
			qsort(rom_sym, rom_sym_count, sizeof(struct symbol_item *),
					bpfa_cmp_sym_by_name);
			sym_ptr = rom_sym;
		}
	} else {
		if (bpfa_options & BPFA_PRINT_RAM_SYMBOLS) {

			qsort(selected_sym, ram_sym_count, sizeof(struct symbol_item *),
					bpfa_cmp_sym_by_addr_and_name);
		} else {
			qsort(rom_sym, rom_sym_count, sizeof(struct symbol_item *),
					bpfa_cmp_sym_by_addr_and_name);
			sym_ptr = rom_sym;
		}
	}

	for (; (ptr = *sym_ptr); sym_ptr++) {

		unsigned int n_type;
		struct symbol_item symitem = *ptr;

		symbol = &bpfa_symbol_table[symitem.symbol_index];

		if (symbol->entry.sym_name.ptr.s_zeros) {

			*((long int *)short_name) = symbol->entry.sym_name.longname;

			name = short_name;
		} else {
			name = &bpfa_string_table[symbol->entry.sym_name.ptr.s_offset];
		}

		bpfa_decode_scn_flags(bfr_flags,
				coff->scn[symbol->entry.n_scnum - 1].s_flags);

		n_type = symbol->entry.n_type;

		printf("%08x\t%s\t%s\t%s\t%s",
				symbol->entry.n_value,
				bfr_flags,
				bpfa_storage_class(symbol->entry.n_sclass),
				bpfa_base_symbol_types[n_type & 0x1f],
				name);

		for (n_type >>= 5; n_type; n_type >>= 3)
			printf(bpfa_derrived_symbol_types[n_type & 0x7]);

		printf("\t%s\n", &bpfa_string_table[symitem.x_offset]);
	}



	// fprintf(fd, "</report>\n");
	//}

	free(symbol_mtd);
	free(selected_sym);

	/* fclose(fd); */

	return 0;
}

