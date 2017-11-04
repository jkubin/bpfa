/* basic set of strings to create a new COFF file
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

#ifndef __COFF_STRINGS_H
#define __COFF_STRINGS_H

//#define LCD_SUFFIX		"_lcd"
//#define AUX_RLE		"_rle"	<--- obcas nelze kompresi pouzit, coz by zpusobilo chybu

#define AUX0			"_aux0"
#define AUX1			"_aux1"
#define CODE			".code_"
#define COLORMAP_BW		"_bw"
#define CRC16			"_crc16"
#define CRC8			"_crc8"
#define GRAY16			"_gray16"
#define GRAY4			"_gray4"
#define GRAY8			"_gray8"
#define IDATA			".idata_"
#define MATH_DATA		"MATH_DATA"
#define ROMDATA			".romdata_"
#define SCN_LEN_CRC		"_len_crc"
#define SCN_LEN_HI_CRC		"_len_hi_crc"
#define SCN_LEN_HI		"_len_hi"
#define SCN_LEN			"_len"
#define TMP_DATA		".tmpdata"
#define UDATA			".udata_"
#define WINDOWS_PATH		"X:\\"

//#define APPEND_IMAGE_DEPTH_TO_NAME

#endif

