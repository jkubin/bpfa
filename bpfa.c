/* bpfa is a binary utility for Microchip COFF used for small 8bit MCU
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
#include <unistd.h>
#include "bpfa.h"

static struct bpfa bpfa;



int main(int argc, char *argv[])
{


	if (bpfa_opt(&bpfa, argc, argv)) {

		exit(EXIT_FAILURE);
	}

	if (bpfa.input_bin_name) {

		if (bpfa_init_bin_input(&bpfa)) {

			exit(EXIT_FAILURE);
		}

		if (bpfa_init_new_coff(&bpfa)) {

			exit(EXIT_FAILURE);
		}

		if (bpfa_make_new_coff(&bpfa)) {

			exit(EXIT_FAILURE);
		}

		exit(EXIT_SUCCESS);
	}

	if (bpfa_init_coff_file(&bpfa)) {

		exit(EXIT_FAILURE);
	}

	if (argc == 2) {

		bpfa_print_coff_info(&bpfa);

		exit(EXIT_SUCCESS);
	}

	if (bpfa_options & BPFA_CRC_ARRAY_PATCH) {

		if (bpfa_patch_compiled_array(&bpfa)) {

			exit(EXIT_FAILURE);
		}

		exit(EXIT_SUCCESS);
	}



	if (bpfa_options & (BPFA_PRINT_PATHS_UNIX | BPFA_PRINT_PATHS_WINDOWS)) {

		if (bpfa_print_paths(&bpfa))
			exit(EXIT_FAILURE);

		exit(EXIT_SUCCESS);
	}

	if (bpfa.symbol_name) {

		if (bpfa_print_symbol(&bpfa))
			exit(EXIT_FAILURE);

		exit(EXIT_SUCCESS);
	}

	if (bpfa_sort_sections(&bpfa)) {

		exit(EXIT_FAILURE);
	}

	if (bpfa.crc_section) {

		if (bpfa_patch_by_aux_section(&bpfa))
			exit(EXIT_FAILURE);
	}

	if (bpfa_options & (BPFA_PRINT_ROM_SYMBOLS | BPFA_PRINT_RAM_SYMBOLS)) {

		if (bpfa_print_symbols(&bpfa))
			exit(EXIT_FAILURE);
	}

	if (bpfa.map_file) {

		if (bpfa_make_map_file(&bpfa))
			exit(EXIT_FAILURE);
	}

	if (bpfa.section_name) {

		if (bpfa_print_section(&bpfa))
			exit(EXIT_FAILURE);

		exit(EXIT_SUCCESS);
	}

	if (bpfa_options & BPFA_PRINT_ROM_SECTIONS) {

		if (bpfa_options & BPFA_SORT_BY_NAME) {

			if (bpfa_print_sections(bpfa.rom_scn_file, bpfa.rom_scn_by_name))
				exit(EXIT_FAILURE);
		} else {
			if (bpfa_print_sections(bpfa.rom_scn_file, bpfa.rom_scn_by_addr))
				exit(EXIT_FAILURE);
		}
	}

	if (bpfa_options & BPFA_PRINT_RAM_SECTIONS) {

		if (bpfa_options & BPFA_SORT_BY_NAME) {

			if (bpfa_print_sections(bpfa.ram_scn_file, bpfa.ram_scn_by_name))
				exit(EXIT_FAILURE);
		} else {
			if (bpfa_print_sections(bpfa.ram_scn_file, bpfa.ram_scn_by_addr))
				exit(EXIT_FAILURE);
		}
	}

	if (bpfa_options & BPFA_PRINT_MCU_RAM) {

		bpfa_print_ram(&bpfa);

		exit(EXIT_SUCCESS);
	}



	if (mcu.bin_file) {

		if (bpfa_write_bin_file(&bpfa, &mcu))
			exit(EXIT_FAILURE);
	}

	if (mcu.hex_file) {

		if (bpfa_write_hex_file(&bpfa, &mcu))
			exit(EXIT_FAILURE);
	}

	if (boot.hex_file) {

		if (bpfa_write_hex_file(&bpfa, &boot))
			exit(EXIT_FAILURE);
	}

	if (fram.bin_file) {

		if (bpfa_write_bin_file(&bpfa, &fram))
			exit(EXIT_FAILURE);
	}

	if (fram.hex_file) {

		if (bpfa_write_hex_file(&bpfa, &fram))
			exit(EXIT_FAILURE);
	}

	if (dflash.bin_file) {

		if (bpfa_write_bin_file(&bpfa, &dflash))
			exit(EXIT_FAILURE);
	}

	if (dflash.hex_file) {

		if (bpfa_write_hex_file(&bpfa, &dflash))
			exit(EXIT_FAILURE);
	}

	if (eth.bin_file) {

		if (bpfa_write_bin_file(&bpfa, &eth))
			exit(EXIT_FAILURE);
	}

	if (eth.hex_file) {

		if (bpfa_write_hex_file(&bpfa, &eth))
			exit(EXIT_FAILURE);
	}

	/*
	   if (output_file) {
	   if (mk_obj())
	   exit(EXIT_FAILURE);
	   }
	   */

	if (bpfa.usagemask) {

		bpfa_memreport(&bpfa);
	};

	free(bpfa.ram_scn_by_addr);
	free(bpfa.ram_scn_by_name);

	close(bpfa.input_fd);

	exit(EXIT_SUCCESS);
}

