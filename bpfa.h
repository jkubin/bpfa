/* bpfa macros and declarations
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

#ifndef __BPFA_H
#define __BPFA_H

#define BPFA_VERSION_NUMBER		"0.1"


//#define PGMSIZE			0x20000
#define BPFA_PICRAM			0x1000
#define BPFA_CRC16_FLAG			0x8000

#define BPFA_GENERIC_MCU		"PIC18CXXX"

//#define BPFA_APPLIC_SUBOPT		0b00000000000000000000000000000001
#define BPFA_BINARY_OUTPUT		0b00000000000000000000000000000010
//#define BPFA_CODE_SCN_PRIMARY		0b00000000000000000000000000000100
#define BPFA_COLOR_OUTPUT		0b00000000000000000000000000001000
// #define BPFA_NEW_OBJECT_NAME		0b00000000000000000000000000010000
#define BPFA_PRINT_ALL			0b00000000000000000000000000100000
#define BPFA_PRINT_MCU_RAM		0b00000000000000000000000001000000
#define BPFA_PRINT_PATHS_UNIX		0b00000000000000000000000010000000
#define BPFA_PRINT_PATHS_WINDOWS	0b00000000000000000000000100000000
#define BPFA_PRINT_PROGRAM_MEMORY1	0b00000000000000000000001000000000
#define BPFA_PRINT_PROGRAM_MEMORY2	0b00000000000000000000010000000000
#define BPFA_PRINT_RAM_SECTIONS		0b00000000000000000000100000000000
#define BPFA_PRINT_RAM_SYMBOLS		0b00000000000000000001000000000000
#define BPFA_PRINT_ROM_SECTIONS		0b00000000000000000010000000000000
#define BPFA_PRINT_ROM_SYMBOLS		0b00000000000000000100000000000000
#define BPFA_PROCESS_MEMORY		0b00000000000000001000000000000000
#define BPFA_QUIET_OUTPUT		0b00000000000000010000000000000000
#define BPFA_SORT_BY_NAME		0b00000000000000100000000000000000
#define BPFA_UTC_TIME_PATCH		0b00000000000001000000000000000000
#define BPFA_VERBOSE_OUTPUT		0b00000000000010000000000000000000
#define BPFA_VERY_VERBOSE_OUTPUT	0b00000000000100000000000000000000
#define BPFA_LENGTH_SYMBOL		0b00000000001000000000000000000000
#define BPFA_LENGTH_SYM_TOTAL		0b00000000010000000000000000000000
#define BPFA_COMPUTE_CRC8		0b00000000100000000000000000000000
#define BPFA_COMPUTE_CRC16		0b00000001000000000000000000000000
#define BPFA_EXTENDED_MODE		0b00000010000000000000000000000000
#define BPFA_CRC_ARRAY_PATCH		0b00000100000000000000000000000000


struct id_proc
{
	unsigned int id;
	char name[16];
};

/* will be removed */
extern struct section_data
{
	unsigned int s_start;
	unsigned int s_end;
	unsigned int s_fuses;
	// unsigned int s_sect;
	char *name;
	char *bin_file;
	char *hex_file;
} boot, mcu, dflash, eth, fram;

struct symbol_item
{
	unsigned int symbol_index;
	unsigned int x_offset;
};

// extern char *boot_hex;
// extern char *memtype;
extern const char * bpfa_string_table;
extern const char * const bpfa_base_symbol_types[];
extern const char * const bpfa_derrived_symbol_types[];
extern const char bpfa_separator[];
extern const union symbol_table *bpfa_symbol_table;
extern const unsigned char crc8_table[];
extern const unsigned short crc16_ccitt[];
extern const unsigned short crc16_ibm[];
extern unsigned int bpfa_options;

struct bpfa
{

	// char *new_object_name;
	char *coff_file_name;
	char *crc_section;
	char *dasm_src_file;
	char *input_bin_name;
	char *map_file;
	char *ram_scn_file;
	char *ram_sym_file;
	char *rom_scn_file;
	char *rom_sym_file;
	char *section_name;
	char *symbol_name;
	struct coff_format *coff;
	struct scnhdr **ram_scn_by_addr;
	struct scnhdr **ram_scn_by_name;
	struct scnhdr **rom_scn_by_addr;
	struct scnhdr **rom_scn_by_name;
	unsigned char *bin_file_data;

	unsigned int coff_file_size;
	unsigned int ram_scn_count;
	unsigned int rom_scn_count;
	unsigned int usagemask;

	struct
	{

		char *aux_scn;
		char *data_scn;
		char *string_table;
		char *strtab_ptr;
		char *symbol;
		char *mcu_name;
		unsigned int proc_type;
		unsigned char *coff_data;

	} newobj;

	struct
	{
		unsigned int final_size_plus_crc;
		unsigned int final_size;

		unsigned int file_size_plus_crc;
		unsigned int file_size;

		unsigned int array_length;
		unsigned int array_item;

		unsigned int predicted_len;
	} dtlen;

	union
	{
		unsigned int length;
		unsigned int blocks;

		struct
		{
			unsigned :8;
			unsigned :8;
			unsigned :8;
			unsigned :4;
			unsigned bit_depth:2;
			unsigned compressed:1;
			unsigned ordered_by_page:1;
		};

	} aux0;

	struct
	{
		unsigned short x;
		unsigned short y;
	} aux1;

	int input_fd;

};

char *bpfa_part_name(unsigned int proc_type);
char *bpfa_storage_class(unsigned int);
int bpfa_cmp_sym_by_addr_and_name(const void *ptr1, const void *ptr2);
int bpfa_cmp_sym_by_name(const void *ptr1, const void *ptr2);
int bpfa_coff_time(struct bpfa *);
int bpfa_disassembly(void);
int bpfa_init_bin_input(struct bpfa *);
int bpfa_init_coff_file(struct bpfa *);
int bpfa_init_new_coff(struct bpfa *);
int bpfa_make_map_file(struct bpfa *);
int bpfa_make_new_coff(struct bpfa *);
int bpfa_memproc(void);
int bpfa_opt(struct bpfa *, int argc, char **argv);
int bpfa_patch_by_aux_section(struct bpfa *);
int bpfa_patch_compiled_array(struct bpfa *bpfa);
int bpfa_primcode(void);
int bpfa_print_paths(struct bpfa *);
int bpfa_print_sections(char *file, struct scnhdr **);
int bpfa_print_section(struct bpfa *);
int bpfa_print_symbols(struct bpfa *);
int bpfa_print_symbol(struct bpfa *);
int bpfa_sort_sections(struct bpfa *);
int bpfa_write_bin_file(struct bpfa *, struct section_data *);
int bpfa_write_hex_file(struct bpfa *, struct section_data *);
// int p2_debug(struct bpfa *);
struct scnhdr *bpfa_find_scn_by_name(struct bpfa *, char *);
struct scnhdr **bpfa_floor_address(struct bpfa *, unsigned int addr);
unsigned int bpfa_part_id(char *);
void bpfa_decode_scn_flags(char *ptr, unsigned int flags);
void bpfa_hexdump(unsigned int addr, unsigned int size, /*unsigned int flags, char *name, */unsigned char *raw);
void bpfa_mchp_list(void);
void bpfa_memreport(struct bpfa *);
void bpfa_print_coff_info(struct bpfa *);
void bpfa_print_hdr(struct bpfa *, FILE *fd);
void bpfa_print_ram(struct bpfa *);
// void decodeflags(FILE *fd, unsigned int flags);

#endif

