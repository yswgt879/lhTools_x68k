// Copyright 1992-2012 Christophe Gottheimer <cgh75015@gmail.com>
//
// lhTools is free software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License version 2 as published by the Free
// Software Foundation.  Note that I am not granting permission to redistribute
// or modify lhTools under the terms of any later version of the General Public
// License.
//
// This program is distributed in the hope that it will be useful (or at
// least amusing), but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program (in the file "COPYING"); if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

/* lh5801.h - Definitions for the LH 5801 processor			*/

#ifndef __LH5801_H__

#define __LH5801_H__

/* Registers types */
#define	C		0x00
#define	E		0x10
#define	L		0x20
#define	N		0x30
#define	B		0x40
#define	D		0x50
#define	H		0x60
#define	M		0x70
#define	BC		0x80
#define	DE		0x90
#define	HL		0xa0
#define	MN		0xb0
#define	iBC		0x01
#define	iDE		0x11
#define	iHL		0x21
#define	iMN		0x31
#define	PC		0x88
#define	SP		0x84
#define	F		0x86
#define	A		0x8f
#define	T0		0xf0
#define	T1		0xf1

/* Conditions types */
#define	NCARRY		0x00
#define CARRY		0x02
#define	NHALFCARRY	0x04
#define	HALFCARRY	0x06
#define	NZERO		0x08
#define	ZERO		0x0a
#define NOVERFLOW	0x0c
#define	OVERFLOW	0x0e
#define	LESS		NCARRY	/* <  eq. nc */
#define GREATEREQUAL	CARRY	/* >= eq.  c */
#define	NOTEQUAL	NZERO	/* != eq. nz */
#define EQUAL		ZERO	/* == eq.  z */
#define	LESSEQUAL	0xf1	/* <= macro  */
#define GREATER		0xf2	/* >  macro  */

/* Instructions links */
#define	LINK_NONE	'\0'
#define	LINK_PAGE	'#'
#define	LINK_MEM_8	'n'
#define	LINK_MEM_16	'N'
#define	LINK_iMEM_8	'm'
#define	LINK_iMEM_16	'M'
#define	LINK_REG_L	'l'
#define	LINK_REG_H	'h'
#define	LINK_REG_HL	'R'
#define	LINK_iREG_HL	'P'
#define	LINK_REG_A	'a'
#define	LINK_REG_F	'f'
#define	LINK_REG_T0	'0'
#define LINK_REG_T1	'1'
#define	LINK_REG_BC	'B'
#define	LINK_REG_SP	'S'
#define	LINK_REG_PC	'C'
#define	LINK_JREL	'd'
#define	LINK_CNDS	'c'
#define LINK_SBR	's'
#define LINK_CD34	'.'
#define LINK_KYWD	'k'

/* Registers base */
#define	BASE_REG_A	 0
#define	BASE_REG_F	 1
#define	BASE_REG_T0	 2
#define	BASE_REG_T1	 3
#define	BASE_REG_L	 4
#define	BASE_REG_H	 8
#define	BASE_REG_HL	12
#define	BASE_REG_BC	12
#define	BASE_REG_SP	16
#define	BASE_REG_PC	17
#define	BASE_iREG_HL	18

/* Flags */
#define	F_CARRY		0x01
#define	F_INTERRUPT	0x02
#define	F_ZERO		0x04
#define	F_OVERFLOW	0x08
#define	F_HALFCARRY	0x10

/* Second page code */
#define	SECOND_PAGE	0xfd

/* Masks */
#define	MASK_NONE	0xff	/* registers coded with b5-b4		*/
#define	MASK_REGS	0xcf	/* registers coded with b5-b4		*/
#define	MASK_CNDS	0xf1	/* conditions coded with b3-b1		*/
#define	MASK_JREL	0xef	/* relative jumps coded with b4-b4	*/
#define	MASK_JRCN	0xe1	/* relative jumps with conditions coded with b4-b1	*/
#define	MASK_CALL	0xc1	/* vectorized call with b5-b1		*/

/* Bind register */
#define BIND_IGNORE	0
#define BIND_UPDATE	1
#define BIND_KILL	2

/* Miscallenous */
#define	GET_LHREGS(c)	(((c) >> 4) & 3)
#define	GET_LHCNDS(c)	(((c) >> 1) & 0x07)
#define	GET_LHU8(a)	(*(a) & 0x0000ff)
#define	GET_LHU16(a)	(((*(a) & 0x00ff) << 8) | ((*(a + 1)) & 0x00ff))

extern LHINST lhinst[];
extern LHINST lhsbr[];
extern LHREGS lhregs[];
extern LHCNDS lhcnds[];

#endif /* __LH5801_H__ */

