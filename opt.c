/* unfinished bunch of options for bpfa
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
#include <getopt.h>
#include <ctype.h>
#include "opt.h"
#include "bpfa.h"


static const char version[] =

BPFA_VERSION_NUMBER

;

static const struct option long_options[] = {

	{"all", no_argument, NULL, 'a'},
	{"all-symbols", no_argument, NULL, BPFA_OPT_ALL_SYMBOLS},
	{"aux-name", required_argument, NULL, 'A'},
	{"boot", required_argument, NULL, 'B'},
	{"color-ram", no_argument, NULL, 'c'},
	{"crc-array-patch", no_argument, NULL, BPFA_OPT_CRC_ARRAY_PATCH},
	{"crc", required_argument, NULL, 'C'},
	{"flash-bin", required_argument, NULL, 'd'},
	{"flash-hex", required_argument, NULL, 'D'},
	{"fram-bin", required_argument, NULL, 'f'},
	{"fram-hex", required_argument, NULL, 'F'},
	{"help", no_argument, NULL, 'h'},
	{"input", required_argument, NULL, 'I'},
	{"length-sym", no_argument, NULL, BPFA_OPT_LENGTH_SYMBOL},
	{"map", required_argument, NULL, BPFA_OPT_SUMMARY_MAP},
	{"mchp", no_argument, NULL, BPFA_OPT_MCHP_PART_LIST},
	//{"mcu2-bin", required_argument, NULL, 'e'},
	{"mcu-hex", required_argument, NULL, 'M'},
	{"mcu", required_argument, NULL, BPFA_OPT_PART_NAME},
	{"memory-usage", required_argument, NULL, 'u'},
	//{"mixed", required_argument, NULL, 'S'},
	{"name", required_argument, NULL, 'N'},
	// {"new-symbol", required_argument, NULL, 'N'},
	//{"output", required_argument, NULL, 'o'},
	//{"p2-debug", required_argument, NULL, 'g'},
	{"part", required_argument, NULL, 'p'},
	//{"primary-code", required_argument, NULL, 'C'},
	{"program-memory1", no_argument, NULL, BPFA_OPT_PROGRAM_MEMORY1},
	{"program-memory2", no_argument, NULL, BPFA_OPT_PROGRAM_MEMORY2},
	{"quiet", no_argument, NULL, 'q'},
	{"ram-alloc", no_argument, NULL, 'i'},
	{"ram-scn-file", required_argument, NULL, BPFA_OPT_RAM_SECTIONS_FILE},
	{"ram-sections", no_argument, NULL, 'L'},
	{"ram-symbols", no_argument, NULL, BPFA_OPT_RAM_SYMBOLS},
	{"ram-sym-file", required_argument, NULL, BPFA_OPT_RAM_SYMBOLS_FILE},
	{"rom-scn-file", required_argument, NULL, BPFA_OPT_ROM_SECTIONS_FILE},
	{"rom-sections", no_argument, NULL, 'l'},
	{"rom-symbols", no_argument, NULL, BPFA_OPT_ROM_SYMBOLS},
	{"rom-sym-file", required_argument, NULL, BPFA_OPT_ROM_SYMBOLS_FILE},
	{"section", required_argument, NULL, 'j'},
	{"sort-by-name", no_argument, NULL, 'n'},
	{"symbol", required_argument, NULL, 's'},
	//{"symbols", no_argument, NULL, 'P'},
	{"unix-paths", no_argument, NULL, 'w'},
	{"utc-time", no_argument, NULL, 't'},
	{"version", no_argument, NULL, 'V'},
	{"windows-paths", no_argument, NULL, 'W'},

	{0, 0, 0, 0},
};



static const char short_options[] =

"a"
"A:"
"B:"
"c"
"C:"
"d:"
"D:"
"e"
//"E:"
"f:"
"F:"
//"g:"
"h"
"i"
"I:"
"j:"
"l"
"L"
"m:"
"M:"
"n"
"N:"
//"o:"
"p:"
"P:"
"q"
"s:"
//"S:"
"t"
"u:"
"v"
"V"
"w"
"W"

;



static const char usage[] =

"Usage: bpfa [options] file.cof\n"
"options:\n"

"  -a, --all                 print ALL (including zero sized)\n"
//"  -A, --applic lo=NUM,hi=NUM,obj=FILE,bin=FILE,hex=FILE\n""      Make application code\n"
"  -A, --aux-name NAME       add auxiliary section name\n"
"      --all-symbols         print ROM + RAM symbols\n"
"  -B, --boot-hex FILE       initial bootloader\n"
"  -c, --color-ram           color terminal output\n"
"  -C, --crc NAME            auxiliary CRC section to patch other section(s) (CRC8 0x8c or CRC16 0xa001)\n"
//"  -C, --primary-code lo=LO,hi=HI,o=FILE\n""       Set range LO,HI for debugging\n"	// byl bych si schopen vygenerovat obj se zasunutou aplikaci
"      --crc-array-patch     patch a simple COFF with a 2D array inside (CRC8 or CRC16 patch)\n"
//"      --data-length         add symbol with data length\n"
"  -d, --flash-bin FILE      write DataFlash bin file\n"
"  -D, --flash-hex FILE      write DataFlash hex file\n"
//"  -E eth.hex,      Write ETH hex file\n"
//"  -e, --mcu2-bin FILE     write binary file\n"		// <--- zrusit, udelat obecny prepinac
"  -f, --fram-bin FILE       write FRAM bin file\n"
"  -F, --fram-hex FILE       write FRAM hex file\n"
//"  -G config2.bin,  Write ETH config\n"
//"  -g config.bin,   Write MCU config\n"
//"  -g, --p2-debug FILE       write debug file for mcu2\n"
"  -h, --help                print this message and exit\n"
"  -I, --input FILE          input (binary) file for conversion to COFF\n"
"  -i, --ram-alloc           print SRAM allocation info\n"
"  -j, --section NAME        print section content as a raw data\n"
//"  -k,              Do not patch file\n"
"      --length-sym          add length symbol for binary object\n"
"  -L, --ram-sections        print RAM sections\n"
"  -l, --rom-sections        print ROM sections\n"
"      --map FILE            write address map file\n"
"      --mchp                print Microchip part list\n"
"      --mcu PICxxxx         same as --part\n"
"  -m, --mcu-bin FILE        write MCU bin file\n"
"  -M, --mcu-hex FILE        write MCU hex file\n"
"  -N  --name NAME           symbol name for binary object\n"
"  -n, --sort-by-name        sort by name\n"
//"  -o, --output FILE         write an output file\n"
"  -p, --part PICxxxx        part (or processor) case insensitive name\n"
"      --program-memory1     print mcu1 flash 0x00000 - 0x1ffff\n"
"      --program-memory2     print mcu2 flash 0x00000 - 0x1ffff\n"
"  -P, --section-hex NAME    print section as hex\n"
"  -q, --quiet               quiet output\n"
"      --ram-scn-file FILE   write RAM sections to a file\n"
"      --ram-symbols         print RAM symbols to stdout\n"
"      --ram-sym-file FILE   write RAM symbols to a file\n"
"      --rom-scn-file FILE   write ROM sections to a file\n"
"      --rom-symbols         print ROM symbols to stdout\n"
"      --rom-sym-file FILE   write ROM symbols to a file\n"
//"  -S, --mixed path/file.c,  print mixed disassembly listing of the file.c\n"
"  -s, --symbol NAME         symbol name\n"
"  -t, --utc-time            patch a symbol \"utc_time\" (4B) from the coff header time (f_timdat)\n"
"  -u, --memory-usage MASK   print memory usage bargraphs\n"
"  -v                        increase verbosity\n"
"  -V, --version             print version and exit\n"
"  -w, --unix-paths          print UNIX/file/paths from string table\n"
"  -W, --windows-paths       print Windows\\file\\paths from string table\n"
//"  -x mixed.c,      print mixed disassembly listing (depends on -S) to mixed.c\n"

;





unsigned int bpfa_options;

int bpfa_opt(struct bpfa *bpfa, int argc, char **argv)
{

	int opt;

	while ((opt = getopt_long(argc, argv, short_options,
					long_options, (int *)0)) != EOF) {
		switch (opt) {

			case 'A':
				bpfa->newobj.aux_scn = optarg;
				break;
			case 'a':
				bpfa_options |= BPFA_PRINT_ALL;
				break;
			case BPFA_OPT_ALL_SYMBOLS:
				bpfa_options |= BPFA_PRINT_RAM_SYMBOLS | BPFA_PRINT_ROM_SYMBOLS;
				break;
			case 'B':
				boot.hex_file = optarg;
				break;
			case 'c':
				bpfa_options |= BPFA_COLOR_OUTPUT;
				break;
			case 'C':
				bpfa->crc_section = optarg;
				break;
			case BPFA_OPT_CRC_ARRAY_PATCH:
				bpfa_options |= BPFA_CRC_ARRAY_PATCH;
				break;
			case 'd':
				dflash.bin_file = optarg;
				break;
			case 'D':
				dflash.hex_file = optarg;
				break;
			case 'e':
				bpfa_options |= BPFA_EXTENDED_MODE;
				break;

			case 'f':
				fram.bin_file = optarg;
				break;
			case 'F':
				fram.hex_file = optarg;
				break;
			case BPFA_OPT_PROGRAM_MEMORY1:
				bpfa_options |= BPFA_PRINT_PROGRAM_MEMORY1;
				break;
			case BPFA_OPT_PROGRAM_MEMORY2:
				bpfa_options |= BPFA_PRINT_PROGRAM_MEMORY2;
				break;
				/*
			case 'g':
				// bpfa_options |= BPFA_CODE_SCN_PRIMARY;
				bpfa->p2_debug_file = optarg;
				break;
				*/
			case 'h':
				fprintf(stdout, usage);
				exit(EXIT_SUCCESS);
				break;
			case 'I':
				bpfa->input_bin_name = optarg;
				break;
			case 'i':
				bpfa_options |= BPFA_PRINT_MCU_RAM;
				break;
			case 'j':
				bpfa_options |= BPFA_BINARY_OUTPUT;
				bpfa->section_name = optarg;
				break;
			case BPFA_OPT_LENGTH_SYMBOL:
				bpfa_options |= BPFA_LENGTH_SYMBOL;
				break;
			case 'L':
				bpfa_options |= BPFA_PRINT_RAM_SECTIONS;
				break;
			case BPFA_OPT_RAM_SECTIONS_FILE:
				bpfa_options |= BPFA_PRINT_RAM_SECTIONS;
				bpfa->ram_scn_file = optarg;
				break;
			case 'l':
				bpfa_options |= BPFA_PRINT_ROM_SECTIONS;
				break;
			case BPFA_OPT_MCHP_PART_LIST:
				bpfa_mchp_list();
				exit(EXIT_SUCCESS);
				break;
			case BPFA_OPT_ROM_SECTIONS_FILE:
				bpfa_options |= BPFA_PRINT_ROM_SECTIONS;
				bpfa->rom_scn_file = optarg;
				break;
			case 'm':
				mcu.bin_file = optarg;
				break;
			case 'M':
				mcu.hex_file = optarg;
				break;
			case 'N':
				bpfa->newobj.symbol = optarg;
				break;
			case 'n':
				bpfa_options |= BPFA_SORT_BY_NAME;
				break;
				/*
			case 'o':
				bpfa->output_file = optarg;
				break;
				*/
			case 'p':
			case BPFA_OPT_PART_NAME:

				bpfa->newobj.mcu_name = BPFA_GENERIC_MCU;

				if (optarg) {

					char *ptr = optarg;

					bpfa->newobj.mcu_name = optarg;

					while ((*ptr = toupper(*ptr)))
						ptr++;
				}

				if ((bpfa->newobj.proc_type =
						 bpfa_part_id(bpfa->newobj.mcu_name)) == 0)
					return -1;

				break;
			case 'q':
				bpfa_options |= BPFA_QUIET_OUTPUT;
				break;
			case BPFA_OPT_RAM_SYMBOLS:
				bpfa_options |= BPFA_PRINT_RAM_SYMBOLS;
				break;
			case BPFA_OPT_RAM_SYMBOLS_FILE:
				bpfa_options |= BPFA_PRINT_RAM_SYMBOLS;
				bpfa->ram_sym_file = optarg;
				break;
			case BPFA_OPT_ROM_SYMBOLS:
				bpfa_options |= BPFA_PRINT_ROM_SYMBOLS;
				break;
			case BPFA_OPT_ROM_SYMBOLS_FILE:
				bpfa_options |= BPFA_PRINT_ROM_SYMBOLS;
				bpfa->rom_sym_file = optarg;
				break;
			case 'S':
				bpfa->dasm_src_file = optarg;
				break;
			case 's':
				bpfa->symbol_name = optarg;
				break;
			case 't':
				bpfa_options |= BPFA_UTC_TIME_PATCH;
				break;
			case BPFA_OPT_SUMMARY_MAP:
				bpfa->map_file = optarg;
				break;
			case 'u':
				bpfa->usagemask = atoi(optarg);
				break;
			case 'v':
				bpfa_options |= BPFA_VERBOSE_OUTPUT;
				break;
			case 'V':
				puts(version);
				exit(EXIT_SUCCESS);
				break;
			case 'W':
				bpfa_options |= BPFA_PRINT_PATHS_WINDOWS;
				break;
			case 'w':
				bpfa_options |= BPFA_PRINT_PATHS_UNIX;
				break;
				/*
				case 'x':
					dasm_mix_file = optarg;
					break;
					*/
			default:
				fprintf(stderr, "Unknown option %s\n", optarg);
				fprintf(stdout, usage);
				return -1;
		}
	}

	if (bpfa->input_bin_name) {

		if (!bpfa->newobj.mcu_name) {

			fprintf(stdout, "error: missing part name -p PIC...\n");

			return -1;
		}

		if (!bpfa->newobj.symbol) {

			fprintf(stdout, "error: missing symbol name -N foo\n");

			return -1;
		}
	}

	if (optind > argc) {

		fprintf(stdout, usage);

		return -1;
	}

	bpfa->coff_file_name = argv[optind];

	return 0;
};

