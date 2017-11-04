/* declaration of the Common Object File Format (COFF) used by Microchip
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

/*
 * some parts were taken from gputils (GNU PIC utils)
 * thank you!
 *
 * Copyright (C) 2001, 2002, 2003, 2004, 2005
 * Craig Franklin
 */

#ifndef __COFF_H
#define __COFF_H

/* Common Object File Format (COFF) of Microchip */
struct __attribute__ ((__packed__)) coff_format
{
	/* file header */
	struct __attribute__ ((__packed__)) filehdr
	{
		unsigned short f_magic;		/* magic number */
		unsigned short f_nscns;		/* number of sections */
		unsigned int f_timdat;		/* time and date stamp */
		unsigned int f_symptr;		/* file ptr to symtab */
		unsigned int f_nsyms;		/* number symtab entries */
		unsigned short f_opthdr;	/* size of opt hdr */
		unsigned short f_flags;		/* flags */
	} hdr;

	/* optional file header */
	struct __attribute__ ((__packed__)) opthdr
	{
		unsigned short opt_magic;
		unsigned int vstamp;		/* version of the compiler assembler */
		unsigned int proc_type;
		unsigned int rom_width_bits;
		unsigned int ram_width_bits;
	} opt;

	/* section headers */
	struct __attribute__ ((__packed__)) scnhdr
	{
		union __attribute__ ((__packed__)) sname
		{
			char name[8];		/* section name if less then 9 characters */
			unsigned long int longname;
			struct
			{
				unsigned int s_zeros;		/* first four characters are 0 */
				unsigned int s_offset;		/* pointer to the string table */
			} ptr;
		} s_name;
		unsigned int s_paddr;		/* physical address */
		unsigned int s_vaddr;		/* virtual address */
		unsigned int s_size;		/* section size */
		unsigned int s_scnptr;		/* file ptr to raw data */
		unsigned int s_relptr;		/* file ptr to relocation */
		unsigned int s_lnnoptr;		/* file ptr to line numbers */
		unsigned short s_nreloc;	/* number reloc entries */
		unsigned short s_nlnno;		/* number line number entries */
		unsigned int s_flags;		/* flags */
	} scn[];
};

/* relocation entry */
struct __attribute__ ((__packed__)) reloc_entry
{
	unsigned int r_vaddr;	/* entry relative virtual address */
	unsigned int r_symndx;	/* index into symbol table */
	short r_offset;			/* offset to be added to address of symbol 'r_symndx' */
	unsigned short r_type;	/* relocation type */
};

/* linenumber entry */
struct __attribute__ ((__packed__)) coff_lineno
{
	unsigned int l_srcndx;	/* symbol table index of associated source file */
	unsigned short l_lnno;	/* line number */
	unsigned int l_paddr;	/* address of code for this lineno */
	unsigned short l_flags;	/* bit flags for the line number */
	unsigned int l_fcnndx;	/* symbol table index of associated function, if there is one */
};

union symbol_table
{
	/* symbol table entry */
	struct __attribute__ ((__packed__)) syment
	{
		union sname sym_name;		/* less than 8 chr */
		unsigned int n_value;		/* symbol value */
		unsigned short n_scnum;		/* section number */
		unsigned int n_type;		/* base type and derived type */
		unsigned char n_sclass;		/* storage class */
		unsigned char n_numaux;		/* number of auxiliary symbols */
	} entry;

	/* auxiliary symbol table entry for a source file */
	struct __attribute__ ((__packed__)) aux_file
	{
		unsigned int x_offset;		/* String table offset for filename */
		unsigned int x_incline;		/* Line number at which this file was included, 0->not included */
		unsigned char x_flags;		/* Bit flags for the .file entry. (X_FILE_DEBUG_ONLY) */
		char _unused[11];
	} file;

	/* auxiliary symbol table entry for a section */
	struct __attribute__ ((__packed__)) aux_scn
	{
		unsigned int x_scnlen;		/* Section Length */
		unsigned short x_nreloc;	/* Number of relocation entries */
		unsigned short x_nlinno;	/* Number of line numbers */
		char _unused[12];
	} scn;

	/* auxiliary symbol table entry for the tagname of a struct/union/enum */
	struct __attribute__ ((__packed__)) aux_tag
	{
		char _unused[6];
		unsigned short x_size;		/* Size of struct/union/enum */
		char _unused2[4];
		unsigned int x_endndx;		/* Symbol index of next entry beyond this struct/union/enum */
		char _unused3[4];
	} tag;

	/* auxiliary symbol table entry for an end of struct/union/enum */
	struct __attribute__ ((__packed__)) aux_eos
	{
		unsigned int x_tagndx;		/* Symbol index of struct/union/enum tag */
		char _unused[2];
		unsigned short x_size;		/* Size of struct/union/enum */
		char _unused2[12];
	} eos;

	/* auxiliary symbol table entry for a function name */
	struct __attribute__ ((__packed__)) aux_fcn
	{
		unsigned int x_tagndx;		/* The symbol table index of the structure or union tag name associated with the return value type, if the return value base type is structure or union. */
		unsigned int x_size;		/* */
		unsigned int x_lnnoptr;		/* File pointer to line numbers for this function */
		unsigned int x_endndx;		/* Symbol Index of next entry beyond this function */
		short x_actscnum;			/* size of static activation record to allocate */
		char _unused[2];
	} fcn;

	/* auxiliary symbol table entry for function call references */
	struct __attribute__ ((__packed__)) aux_fcn_calls
	{
		/* Symbol index of the called function. If call of a higher order function, set to AUX_FCN_CALLS_HIGHERORDER. */
		unsigned int x_calleendx;		/* Symbol table entry of callee - 1 */
		unsigned int x_is_interrupt;	/* 0 not, 1 low, 2 high */
		char _unused[12];
	} fcn_calls;

	/* auxiliary symbol table entry for an array */
	struct __attribute__ ((__packed__)) aux_arr
	{
		unsigned int x_tagndx;		/* Unused??  Tag Index */
		unsigned short x_lnno;		/* Unused??  Line number declaration */
		unsigned short x_size;		/* Size of array */
		unsigned short x_dimen[4];	/* Size of first four dimensions */
		char _unused[4];
	} arr;

	/* auxiliary symbol table entry for the beginning of a block or function */
	struct __attribute__ ((__packed__)) aux_bobf
	{
		char _unused[4];
		unsigned short x_lnno;		/* C Source line number of the beginning, relative to start enclosing scope */
		char _unused2[6];
		unsigned int x_endndx;		/* Symbol Index of next entry past this block/func */
		char _unused3[4];
	} bobf;

	/* auxiliary symbol table entry for the end of a block or function */
	struct __attribute__ ((__packed__)) aux_eobf
	{
		char _unused[4];
		unsigned short x_lnno;		/* C Source line number of the end, relative to start of block/func */
		char _unused2[14];
	} eobf;

	/* auxiliary symbol table entry for a variable of type struct/union/enum */
	struct __attribute__ ((__packed__)) aux_var
	{
		unsigned int x_tagndx;		/* Symbol Index of struct/union/enum tagname */
		char _unused[2];
		unsigned short x_size;		/* Size of the struct/union/enum */
		char _unused2[12];
	} var;

	/* auxiliary entry for a bit field */
	struct __attribute__ ((__packed__)) aux_field
	{
		char _unused[6];
		unsigned short x_size;
		char _unused2[12];
	} field;
};

/* magic number of a Microchip COFF file */
#define MICROCHIP_MAGIC		0x1240
#define COMPILER_VERSION	0x00008728

/* file header flags */
#define F_RELFLG			0x0001	/* relocation info has been stripped */
#define F_EXEC				0x0002	/* file is executable - has no unresolved external symbols */
#define F_LNNO				0x0004	/* line numbers have been stripped */
#define F_ABSOLUTE			0x0010	/* the MPASM assembler object file is from absolute (as opposed to relocatable) assembly code */
#define L_SYMS				0x0080	/* local symbols have been stripped */
#define F_EXTENDED18			0x4000	/* the COFF file produced utilizing the Extended mode */
#define F_GENERIC			0x8000	/* processor independent file for a core */

/* section header flags */
#define STYP_TEXT			0x00020	/* Section contains executable code */
#define STYP_DATA			0x00040	/* Section contains initialized data */
#define STYP_BSS			0x00080	/* Section contains uninitialized data */
#define STYP_DATA_ROM			0x00100	/* Section contains initialized data for ROM */
#define STYP_ABS			0x01000	/* Section is absolute */
#define STYP_SHARED			0x02000	/* Section is shared across banks */
#define STYP_OVERLAY			0x04000	/* Section is overlaid with other sections of the same name from different objects modules */
#define STYP_ACCESS			0x08000	/* Section is available using access bit */
#define STYP_ACTREC			0x10000	/* Section contains the activation record for a function */
#define STYP_RELOC			0x20000	/* Section has been relocated. This is a temporary flag used by the linker */
#define STYP_BPACK			0x40000	/* Section is byte packed on 16bit devices */

/* relocation types */
#define RELOCT_CALL			0x01	/* relocation for the CALL instruction (first word only on 18cxx) */
#define RELOCT_GOTO			0x02	/* relocation for the GOTO instruction (first word only on 18cxx) */
#define RELOCT_HIGH			0x03	/* relocation for the second 8 bits of an address */
#define RELOCT_LOW			0x04	/* relocation for the low order 8 bits of an address */
#define RELOCT_P			0x05	/* relocation for the 5 bits of address for the P operand of a 17cxx MOVFP or MOVPF instruction */
#define RELOCT_BANKSEL			0x06	/* relocation to generate the appropriate instruction to bank switch for a symbol */
#define RELOCT_PAGESEL			0x07	/* relocation to generate the appropriate instruction to page switch for a symbol */
#define RELOCT_ALL			0x08	/* FIXME */
#define RELOCT_IBANKSEL 		0x09	/* FIXME */
#define RELOCT_F			0x0a	/* relocation for the 8 bits of address for the F operand of a 17cxx MOVFP or MOVPF instruction */
#define RELOCT_TRIS			0x0b	/* FIXME */
#define RELOCT_MOVLR			0x0c	/* relocation for the MOVLR bank 17cxx banking instruction */
#define RELOCT_MOVLB			0x0d	/* relocation for the MOVLB 17cxx and 18cxx banking instruction */
#define RELOCT_GOTO2			0x0e	/* relocation for the second word of an 18cxx goto instruction */
#define RELOCT_CALL2			RELOCT_GOTO2	/* relocation for the second word of an 18cxx call instruction */
#define RELOCT_FF1			0x0f	/* relocation for the source register of the 18cxx MOVFF instruction */
#define RELOCT_FF2			0x10	/* relocation for the destination register of the 18cxx MOVFF instruction */
#define RELOCT_LFSR1			0x11	/* relocation for the first word of the 18cxx LFSR instruction */
#define RELOCT_LFSR2			0x12	/* relocation for the second word of the 18cxx LFSR instruction */
#define RELOCT_BRA			0x13	/* relocation for the 18cxx BRA instruction */
#define RELOCT_RCALL			RELOCT_BRA	/* relocation for the 18cxx RCALL instruction */
#define RELOCT_CONDBRA			0x14	/* relocation for the 18cxx relative conditional branch instructions */
#define RELOCT_UPPER			0x15	/* relocation for the highest order 8 bits of a 24-bit address */
#define RELOCT_ACCESS			0x16	/* relocation for the 18cxx access bit */
#define RELOCT_PAGESEL_WREG		0x17	/* relocation for selecting the correct page using WREG as scratch */
#define RELOCT_PAGESEL_BITS		0x18	/* relocation for selecting the correct page using bit set/clear instructions */
/* relocation for the size of a section */
#define RELOCT_SCNSZ_LOW		0x19
#define RELOCT_SCNSZ_HIGH		0x1a
#define RELOCT_SCNSZ_UPPER 		0x1b
/* relocation for the address of the end of a section */
#define RELOCT_SCNEND_LOW		0x1c
#define RELOCT_SCNEND_HIGH		0x1d
#define RELOCT_SCNEND_UPPER		0x1e
/* relocation for the address of the end of a section on LFSR */
#define RELOCT_SCNEND_LFSR1		0x1f
#define RELOCT_SCNEND_LFSR2		0x20
#define RELOCT_TRIS_4BIT		0x21
/* relocation for selecting the correct page using pic14 enhanced MOVLP instruction */
#define RELOCT_PAGESEL_MOVLP		0x22

/* symbol section numbers */
#define N_DEBUG			-2	/* A debugging symbol */
#define N_ABS			-1	/* An absolute symbol (n_value is a constant, not an address) */
#define N_UNDEF			0	/* An undefined (extern) symbol */
#define N_SCNUM			1	/* */

/* base symbol types */
#define T_NULL			0x00	/* null */
#define T_VOID			0x01	/* void */
#define T_CHAR			0x02	/* character */
#define T_SHORT			0x03	/* short integer */
#define T_INT			0x04	/* integer */
#define T_LONG			0x05	/* long integer */
#define T_FLOAT			0x06	/* floating point */
#define T_DOUBLE		0x07	/* double length floating point */
#define T_STRUCT		0x08	/* structure */
#define T_UNION			0x09	/* union */
#define T_ENUM			0x0a	/* enumeration */
#define T_MOE			0x0b	/* member of enumeration */
#define T_UCHAR			0x0c	/* unsigned character */
#define T_USHORT		0x0d	/* unsigned short */
#define T_UINT			0x0e	/* unsigned integer */
#define T_ULONG			0x0f	/* unsigned long */
#define T_LNGDBL		0x10	/* long double floating point */
#define T_SLONG			0x11	/* short long */
#define T_USLONG		0x12	/* unsigned short long */

/* derived types */
#define DT_NON			0x00	/* no derived type */
#define DT_RAMPTR		0x01	/* pointer to data memory */
#define DT_FCN			0x02	/* function */
#define DT_ARY			0x03	/* array */
#define DT_ROMPTR		0x04	/* pointer to program memory */
#define DT_FARROMPTR		0x05	/* far (24 bit) pointer to program memory */

/* storage classes */
#define C_NULL			0x00	/* null */
#define C_AUTO			0x01	/* automatic variable */
#define C_EXT			0x02	/* external symbol */
#define C_STAT			0x03	/* static */
#define C_REG			0x04	/* register variable */
#define C_EXTDEF		0x05	/* external definition */
#define C_LABEL			0x06	/* label */
#define C_ULABEL		0x07	/* undefined label */
#define C_MOS			0x08	/* member of structure */
#define C_ARG			0x09	/* function argument */
#define C_STRTAG		0x0a	/* structure tag */
#define C_MOU			0x0b	/* member of union */
#define C_UNTAG			0x0c	/* union tag */
#define C_TPDEF			0x0d	/* type definition */
#define C_USTATIC		0x0e	/* undefined static */
#define C_ENTAG			0x0f	/* enumeration tag */
#define C_MOE			0x10	/* member of enumeration */
#define C_REGPARM		0x11	/* register parameter */
#define C_FIELD			0x12	/* bit field */
#define C_AUTOARG		0x13	/* auto argument */
#define C_LASTENT		0x14	/* dummy entry (end of block) */
#define C_BLOCK			0x64	/* ".bb" or ".eb" */
#define C_FCN			0x65	/* ".bf" or ".ef" */
#define C_EOS			0x66	/* end of structure */
#define C_FILE			0x67	/* file name */
#define C_LINE			0x68	/* line number reformatted as symbol table entry */
#define C_ALIAS			0x69	/* duplicate tag */
#define C_HIDDEN		0x6a	/* ext symbol in dmert public lib */
#define C_EOF			0x6b	/* end of file */
#define C_LIST			0x6c	/* absoulte listing on or off */
#define C_SECTION		0x6d	/* section */
#define C_EFCN			0xff	/* physical end of function */

#define X_FILE_DEBUG_ONLY	0x01	/* This .file entry was included for debugging purposes only. */
#define LINENO_HASFCN		0x01	/* set if l_fcnndx is valid */
#define AUX_FCN_CALLS_HIGHERORDER	((unsigned int)-1)	/* struct aux_fcn_calls {unsigned long x_calleendx} */

#endif

