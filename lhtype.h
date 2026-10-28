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

/* lhtype.h - Used type for LH 5801 emulator/assembler/unsassembler	*/

#ifndef __LHTYPE_H__

#define __LHTYPE_H__

typedef	unsigned char		LHU8;
typedef unsigned char *		PLHU8;
typedef unsigned short		LHU16;
typedef	unsigned short *	PLHU16;

struct s_lhregs {
  PLHU8 regs;
  LHU8  code;
  LHU8  link;
  PLHU8 regH;
  PLHU8 regL;
  PLHU8 regHL;
  PLHU8 regiHL;
};

typedef struct s_lhregs		LHREGS;
typedef struct s_lhregs *	PLHREGS;

struct s_lhcnds {
  PLHU8 cnds;
  LHU8  code;
  PLHU8 neg;
};

typedef struct s_lhcnds		LHCNDS;
typedef struct s_lhcnds *	PLHCNDS;

struct s_lhinst {
  PLHU8 inst;
  LHU8  code;
  LHU8  amsk;
  LHU8  umsk;
  LHU8  bind;
};

typedef struct s_lhinst		LHINST;
typedef struct s_lhinst *	PLHINST;

struct s_lhcode {
  PLHINST  inst;
  LHU16    op[2];
  LHU8     reg[2];
};

typedef struct s_lhcode		LHCODE;
typedef struct s_lhcode *	PLHCODE;

struct s_lhbasic {
  PLHU8	basic;
  LHU16 code;
  LHU16 jump;
  LHU8  bits;
};

typedef struct s_lhbasic	LHBASIC;
typedef struct s_lhbasic *	PLHBASIC;

#endif /* __LHTYPE_H__ */

