/* (ugly hack) creates linkable COFF file from an input (binary) file
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
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "coff.h"
#include "bpfa.h"
#include "coff_strings.h"

#define write_section_name(s)		_write_name(bpfa, s, &scn->s_name)
#define write_section_name_prefix(s)	_write_string_tab(bpfa, s, &scn->s_name)
//#define write_symbol_name(s)		_write_name(bpfa, s, &symbol->entry.sym_name)

static void _write_string_tab(struct bpfa *bpfa, char *str, union sname *snm)
{
	snm->ptr.s_offset = bpfa->newobj.strtab_ptr - bpfa->newobj.string_table;
	strcpy(bpfa->newobj.strtab_ptr, str);
	bpfa->newobj.strtab_ptr += strlen(str);
	strcpy(bpfa->newobj.strtab_ptr, bpfa->coff_file_name);
	bpfa->newobj.strtab_ptr += strlen(bpfa->coff_file_name);
	bpfa->newobj.strtab_ptr++;
}

static void _write_name(struct bpfa *bpfa, char *str, union sname *snm)
{

	unsigned int length = strlen(str);

	/*
	 * I do not know why, but the mcc18.exe sends
	 * an eight-character name to the string table
	 *
	 * I have found experimentally that in
	 * the section name there are max 8 characters,
	 * but in the symbol name is 7 characters
	 *
	 * section < 9
	 * symbol < 8
	 */

	if (length > 8) {

		snm->ptr.s_offset = bpfa->newobj.strtab_ptr - bpfa->newobj.string_table;
		strcpy(bpfa->newobj.strtab_ptr, str);
		bpfa->newobj.strtab_ptr += length;
		bpfa->newobj.strtab_ptr++;

		return;
	}

	/*
	 * if the length is just 8 characters, there may be a problem
	 * strcpy copies a name including a NULL character
	 * therefore, do not fill the following symbol member
	 * because it is tampered by NULL from the string
	 */

	strcpy(snm->name, str);
}

static unsigned int bpfa_heading_name(struct bpfa *bpfa, char *str)
{

	unsigned int offset = bpfa->newobj.strtab_ptr - bpfa->newobj.string_table;

	strcpy(bpfa->newobj.strtab_ptr, str);
	bpfa->newobj.strtab_ptr += strlen(str);

	return offset;
}

static void bpfa_append_suffix(struct bpfa *bpfa, char *str)
{
	//unsigned int offset = bpfa_heading_name(bpfa, str);
	bpfa_heading_name(bpfa, str);

	bpfa->newobj.strtab_ptr++;
}

static unsigned char bpfa_declare_auxsym_num(struct bpfa *bpfa)
{

	unsigned char counter = 0;

	if (bpfa->newobj.aux_scn) {

		counter++;	// ..._aux0
		counter++;	// ..._aux1
	}

	if (bpfa_options & BPFA_LENGTH_SYMBOL) {

		counter++;	// ..._len
		counter++;	// ..._len_hi
	}

	if (bpfa_options & BPFA_LENGTH_SYM_TOTAL) {

		counter++;	// ..._lent
		counter++;	// ..._lent_hi
	}

	return counter;
}

static unsigned char bpfa_define_auxsym(struct bpfa *bpfa, union symbol_table *symbol, unsigned int aux_scnum)
{

	union symbol_table *fst_aux_sym;

	fst_aux_sym = symbol;

	if (bpfa->newobj.aux_scn) {

		symbol->entry.sym_name.ptr.s_offset =
			bpfa_heading_name(bpfa, bpfa->newobj.symbol);

		/*
		switch (zyx.bit_depth) {
			case 1:
				bpfa_heading_name(bpfa, COLORMAP_BW);
				break;
			case 2:
				bpfa_heading_name(bpfa, GRAY4);
				break;
			case 3:
				bpfa_heading_name(bpfa, GRAY8);
				break;
			case 4:
				bpfa_heading_name(bpfa, GRAY16);
				break;
		}
		*/

		bpfa_append_suffix(bpfa, AUX0);

		symbol->entry.n_value = *((int *)&bpfa->aux0);
		symbol->entry.n_scnum = aux_scnum;
		symbol->entry.n_type = T_CHAR;
		symbol->entry.n_sclass = C_STAT;
		symbol++;

		symbol->entry.sym_name.ptr.s_offset =
			bpfa_heading_name(bpfa, bpfa->newobj.symbol);

		bpfa_append_suffix(bpfa, AUX1);

		symbol->entry.n_value = *((int *)&bpfa->aux1);
		symbol->entry.n_scnum = aux_scnum;
		symbol->entry.n_type = T_CHAR;
		symbol->entry.n_sclass = C_STAT;
		symbol++;
	}

	if (bpfa_options & BPFA_LENGTH_SYMBOL) {

		symbol->entry.sym_name.ptr.s_offset =
			bpfa_heading_name(bpfa, bpfa->newobj.symbol);
		bpfa_append_suffix(bpfa, SCN_LEN);
		symbol->entry.n_value = bpfa->dtlen.final_size;
		symbol->entry.n_scnum = aux_scnum;
		symbol->entry.n_type = T_CHAR;
		symbol->entry.n_sclass = C_STAT;
		symbol++;

		symbol->entry.sym_name.ptr.s_offset =
			bpfa_heading_name(bpfa, bpfa->newobj.symbol);
		bpfa_append_suffix(bpfa, SCN_LEN_HI);
		symbol->entry.n_value = bpfa->dtlen.final_size >> 24;
		symbol->entry.n_scnum = aux_scnum;
		symbol->entry.n_type = T_CHAR;
		symbol->entry.n_sclass = C_STAT;
		symbol++;
	}

	if (bpfa_options & BPFA_LENGTH_SYM_TOTAL) {

		symbol->entry.sym_name.ptr.s_offset =
			bpfa_heading_name(bpfa, bpfa->newobj.symbol);
		bpfa_append_suffix(bpfa, SCN_LEN_CRC);
		symbol->entry.n_value = bpfa->dtlen.final_size_plus_crc;
		symbol->entry.n_scnum = aux_scnum;
		symbol->entry.n_type = T_CHAR;
		symbol->entry.n_sclass = C_STAT;
		symbol++;

		symbol->entry.sym_name.ptr.s_offset =
			bpfa_heading_name(bpfa, bpfa->newobj.symbol);
		bpfa_append_suffix(bpfa, SCN_LEN_HI_CRC);
		symbol->entry.n_value = bpfa->dtlen.final_size_plus_crc >> 24;
		symbol->entry.n_scnum = aux_scnum;
		symbol->entry.n_type = T_CHAR;
		symbol->entry.n_sclass = C_STAT;
		symbol++;
	}

	return symbol - fst_aux_sym;
}

int bpfa_init_bin_input(struct bpfa *bpfa)
{

	int fd;
	struct stat file_info;

	if ((fd = open(bpfa->input_bin_name, O_RDONLY)) == -1) {
		perror(bpfa->input_bin_name);

		return -1;
	}

	if (fstat(fd, &file_info) == -1) {
		perror(bpfa->input_bin_name);

		return -1;
	}

	bpfa->dtlen.file_size_plus_crc =
		bpfa->dtlen.final_size =
		bpfa->dtlen.file_size = file_info.st_size;

	if (bpfa_options & (BPFA_COMPUTE_CRC8 | BPFA_COMPUTE_CRC16)) {

		if (bpfa->dtlen.file_size < bpfa->dtlen.array_item) {

			fprintf(stderr, "wrong CRC length\n");

			return -1;
		}

		if (!bpfa->dtlen.array_item)
			bpfa->dtlen.array_item = bpfa->dtlen.file_size;

		if (bpfa->dtlen.file_size % bpfa->dtlen.array_item) {

			fprintf(stderr, "array mod error\n");

			return -1;
		}

		bpfa->dtlen.array_length = bpfa->dtlen.file_size / bpfa->dtlen.array_item;
	}

	if (bpfa->aux1.x) {

		if (bpfa->dtlen.file_size != (unsigned int)(bpfa->aux1.x * bpfa->aux1.y)) {

			fprintf(stderr, "no-rectangular X Y data\n");

			return -1;
		}
	}

	if ((bpfa->bin_file_data = mmap((caddr_t)0,
					bpfa->dtlen.file_size,
					PROT_READ,
					MAP_SHARED, fd, 0)) ==
			(unsigned char *)-1) {

		perror(bpfa->input_bin_name);

		return -1;
	}

	return 0;
}

int bpfa_init_new_coff(struct bpfa *bpfa)
{

	struct coff_format *coff;
	struct scnhdr *scn;

	/*
	 * the predicted length MUST have to be greater than the resulting length
	 * (the resulting length does not have to be exact, but a little bit greater)
	 */

	bpfa->dtlen.predicted_len =
		// header and 5 basic section headers
		(unsigned long)&((struct coff_format *)0)->scn[5] +
		sizeof(struct scnhdr) +		// bpfa->newobj.data_scn
		sizeof(struct scnhdr) +		// MATH_DATA
		sizeof(struct scnhdr) + 	// bpfa->newobj.aux_scn
		sizeof(struct syment) * 5 +	// basic symtabs (.tmpdata; .udata_...; .idata_...; .romdata_...; .code_...)
		sizeof(struct syment) +		// ..._aux
		sizeof(struct syment) +		// ..._len
		sizeof(struct syment) +		// ..._hilen

		4 +	// size of string table sizeof(int)
		1 +	// data_sect NULL
		1 +	// bpfa->newobj.symbol NULL
		1 +	// bpfa->newobj.aux_scn NULL

		// mandatory strings
		sizeof(CODE) +
		sizeof(IDATA) +
		sizeof(MATH_DATA) +
		sizeof(ROMDATA) +
		sizeof(TMP_DATA) +
		sizeof(UDATA) +
		sizeof(WINDOWS_PATH) +

		// optional strings
		sizeof(AUX0) +	// _aux
		sizeof(AUX1) +	// _aux

		// maxlen(_shade16,_shade8,_shade4,_bw)
		sizeof(GRAY16) +	// name_shade16; name_bw; ...
		sizeof(GRAY16) +	// name_shade16_aux; name_bw_aux; ...
		sizeof(CRC16) +		// (name_shade16_raw_crc16_aux)
		sizeof(SCN_LEN) +	// symbol with length
		sizeof(SCN_LEN_HI) +	// symbol with length that did not fit in the lower three bytes (rom pointer has 3B size)
		sizeof(SCN_LEN_CRC) +	// symbol with length including CRC
		sizeof(SCN_LEN_HI_CRC) +	// symbol with length including CRC, that did not fit in the lower three bytes
		0;

	// .udata_ + bpfa->coff_file_name, .idata_ + bpfa->coff_file_name,
	// .romdata_ + bpfa->coff_file_name, .code_ + bpfa->coff_file_name;
	// (NULL is contained v sizeof(PREFIX))
	bpfa->dtlen.predicted_len += strlen(bpfa->coff_file_name) * 4;

	if (bpfa->newobj.data_scn)
		bpfa->dtlen.predicted_len += strlen(bpfa->newobj.data_scn);	// 1x NULL is already added

	bpfa->dtlen.predicted_len += strlen(bpfa->input_bin_name);		// 1x NULL is already added in WINDOWS_PATH

	// "name", "name_aux0", "name_aux1", "name_len",
	// "name_len_hi", "name_len_total", "name_len_hi_total"
	bpfa->dtlen.predicted_len += strlen(bpfa->newobj.symbol) * 7;

	if (bpfa->newobj.aux_scn)
		bpfa->dtlen.predicted_len += strlen(bpfa->newobj.aux_scn);	// 1x NULL is already added

	if (bpfa_options & BPFA_COMPUTE_CRC8)
		bpfa->dtlen.file_size_plus_crc += bpfa->dtlen.array_length;
	else if (bpfa_options & BPFA_COMPUTE_CRC16)
		bpfa->dtlen.file_size_plus_crc += bpfa->dtlen.array_length * 2;

	bpfa->dtlen.predicted_len += bpfa->dtlen.file_size_plus_crc;

	if ((coff = calloc(1, bpfa->dtlen.predicted_len)) == NULL) {

		perror("calloc");

		return -1;
	}

	bpfa->coff = coff;

	scn = &coff->scn[5];

	// MATH_DATA
	if (!(bpfa_options & BPFA_EXTENDED_MODE))
		scn++;

	if (bpfa->newobj.data_scn)
		scn++;

	if (bpfa->newobj.aux_scn)
		scn++;

	coff->hdr.f_nscns = scn - coff->scn;

	bpfa->newobj.coff_data = (unsigned char *)scn;

	// CRC patch for binary data will not be computed
	// (I have proper implementation, but not implemented here yet)

	bpfa->aux0.length |=
		bpfa->dtlen.final_size_plus_crc =
		bpfa->dtlen.final_size =
		bpfa->dtlen.file_size;

	memcpy(bpfa->newobj.coff_data, bpfa->bin_file_data,
			bpfa->dtlen.final_size);

	return 0;
}

int bpfa_make_new_coff(struct bpfa *bpfa)
{

	int fd;
	struct coff_format *coff;
	struct scnhdr *data_scn;
	struct scnhdr *scn;
	union symbol_table *symbol;
	union symbol_table *symbol_table;
	unsigned int aux_scnum;

	coff = bpfa->coff;

	symbol =
		symbol_table =
		(union symbol_table *)&((unsigned char *)bpfa->newobj.coff_data)[
		bpfa->dtlen.final_size_plus_crc
	];

	// add 5 mandatory symbols (.file + aux_sym; .eof; name + aux_sym)
	symbol += 5;

	if (bpfa->newobj.aux_scn)
		symbol += bpfa_declare_auxsym_num(bpfa);

	bpfa->newobj.string_table =
		bpfa->newobj.strtab_ptr =
		(char *)symbol;

	// skip length
	bpfa->newobj.strtab_ptr += 4;

	////////////////////////////////////////////////////////////////////////////////
	// struct filehdr
	coff->hdr.f_magic = MICROCHIP_MAGIC;
	coff->hdr.f_timdat = time(NULL);
	coff->hdr.f_symptr = (unsigned long)&((struct coff_format *)0)->scn[coff->hdr.f_nscns] +
		bpfa->dtlen.final_size_plus_crc;
	coff->hdr.f_nsyms = symbol - symbol_table;
	coff->hdr.f_opthdr = sizeof(struct opthdr);

	if (bpfa_options & BPFA_EXTENDED_MODE)
		coff->hdr.f_flags = F_EXTENDED18;

	////////////////////////////////////////////////////////////////////////////////
	// struct opthdr
	coff->opt.vstamp = COMPILER_VERSION;
	coff->opt.proc_type = bpfa->newobj.proc_type;
	coff->opt.rom_width_bits = 8;
	coff->opt.ram_width_bits = 8;

	scn = coff->scn;

	if (!(bpfa_options & BPFA_EXTENDED_MODE)) {
		////////////////////////////////////////////////////////////////////////////////
		// MATH_DATA
		write_section_name(MATH_DATA);
		scn->s_flags = STYP_BSS | STYP_ACCESS;
		scn++;
	}

	////////////////////////////////////////////////////////////////////////////////
	// .tmpdata
	//scn->s_name.ptr.s_offset = bpfa->newobj.strtab_ptr - bpfa->newobj.string_table;
	//strcpy(bpfa->newobj.strtab_ptr, TMP_DATA);
	//bpfa->newobj.strtab_ptr += sizeof(TMP_DATA);
	write_section_name(TMP_DATA);

	if (bpfa_options & BPFA_EXTENDED_MODE)
		scn->s_flags = STYP_BSS | STYP_OVERLAY;
	else
		scn->s_flags = STYP_BSS | STYP_OVERLAY | STYP_ACCESS;

	scn++;

	////////////////////////////////////////////////////////////////////////////////
	// .udata_???
	write_section_name_prefix(UDATA);
	scn->s_flags = STYP_BSS;
	scn++;

	////////////////////////////////////////////////////////////////////////////////
	// .idata_???
	write_section_name_prefix(IDATA);
	scn->s_flags = STYP_DATA;
	scn++;

	////////////////////////////////////////////////////////////////////////////////
	// .romdata_???
	write_section_name_prefix(ROMDATA);
	scn->s_flags = STYP_DATA_ROM;
	data_scn = scn;
	scn++;

	////////////////////////////////////////////////////////////////////////////////
	// .code_???
	write_section_name_prefix(CODE);
	scn->s_flags = STYP_TEXT;
	scn++;

	////////////////////////////////////////////////////////////////////////////////
	// romdata section
	if (bpfa->newobj.data_scn) {
		write_section_name(bpfa->newobj.data_scn);
		scn->s_flags = STYP_DATA_ROM;
		data_scn = scn;
		scn++;
	}

	if (bpfa->newobj.aux_scn) {
		write_section_name(bpfa->newobj.aux_scn);
		scn->s_flags = STYP_BSS | STYP_OVERLAY;

		/*
		 * the section must have a non-zero size
		 * otherwise the linker incorrectly assigns it
		 * to the previous section (type romdata)
		 */
		scn->s_size++;

		scn++;

		aux_scnum = scn - coff->scn;
	}

	data_scn->s_scnptr = (char *)scn - (char *)coff;
	data_scn->s_size = bpfa->dtlen.final_size_plus_crc;
	data_scn++;		// scn + 1

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	symbol = symbol_table;

	////////////////////////////////////////////////////////////////////////////////
	// .file
	strcpy(symbol->entry.sym_name.name, ".file");
	symbol->entry.n_scnum = N_DEBUG;
	symbol->entry.n_sclass = C_FILE;
	symbol->entry.n_numaux++;

	symbol++;

	////////////////////////////////////////////////////////////////////////////////
	// bpfa->input_bin_name to bpfa->newobj.strtab_ptr
	symbol->file.x_offset = bpfa_heading_name(bpfa, WINDOWS_PATH);
	bpfa_append_suffix(bpfa, bpfa->input_bin_name);

	symbol++;

	////////////////////////////////////////////////////////////////////////////////
	// bpfa->newobj.symbol

//#ifdef APPEND_IMAGE_DEPTH_TO_NAME
	symbol->entry.sym_name.ptr.s_offset =
		bpfa_heading_name(bpfa, bpfa->newobj.symbol);

	/*
	   switch (zyx.bit_depth) {
	   case 1:
	   bpfa_heading_name(bpfa, COLORMAP_BW);
	   break;
	   case 2:
	   bpfa_heading_name(bpfa, GRAY4);
	   break;
	   case 3:
	   bpfa_heading_name(bpfa, GRAY8);
	   break;
	   case 4:
	   bpfa_heading_name(bpfa, GRAY16);
	   break;
	   }
	   */

	if (bpfa_options & BPFA_COMPUTE_CRC8)
		bpfa_heading_name(bpfa, CRC8);

	if (bpfa_options & BPFA_COMPUTE_CRC16)
		bpfa_heading_name(bpfa, CRC16);

	bpfa_append_suffix(bpfa, "");

//#else
//	write_symbol_name(bpfa->newobj.symbol);
//#endif

	symbol->entry.n_scnum = data_scn - coff->scn;

	symbol->entry.n_type = (DT_ARY << 5) | T_UCHAR;

	symbol->entry.n_sclass = C_EXT;
	symbol->entry.n_numaux++;

	symbol++;

	symbol->arr.x_size = bpfa->dtlen.final_size;

	*((unsigned short *)symbol->arr.x_dimen) = bpfa->dtlen.final_size;

	symbol++;

	////////////////////////////////////////////////////////////////////////////////
	// _aux, _len, ...
	if (bpfa->newobj.aux_scn)
		symbol += bpfa_define_auxsym(bpfa, symbol, aux_scnum);

	////////////////////////////////////////////////////////////////////////////////
	// .eof
	strcpy(symbol->entry.sym_name.name, ".eof");
	symbol->entry.n_scnum = N_DEBUG;
	symbol->entry.n_sclass = C_EOF;
	symbol++;

	////////////////////////////////////////////////////////////////////////////////
	// write final bpfa->newobj.string_table length
	*((unsigned int *)bpfa->newobj.string_table) =
		bpfa->newobj.strtab_ptr - bpfa->newobj.string_table;

	////////////////////////////////////////////////////////////////////////////////
	// write coff image to bpfa->coff_file_name
	if ((fd = open(bpfa->coff_file_name,
					O_WRONLY | O_CREAT | O_TRUNC,
					0666)) == -1) {

		perror(bpfa->coff_file_name);

		return -1;
	}

	//#define DEBUG_OBJECT_IMAGE

#ifdef DEBUG_OBJECT_IMAGE
	write(fd, coff, bpfa->dtlen.predicted_len);
#else
	write(fd, coff, bpfa->newobj.strtab_ptr - (char *)coff);
#endif

	close(fd);

	return 0;
}

