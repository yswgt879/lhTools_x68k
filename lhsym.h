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

/* lhsym.h - Definitions for the symbols structures			*/

#ifndef __LHSYM_H__

#define __LHSYM_H__

#define LHSYM_MAGIC	".SYMBOLS:"

#define	SYM_NONE	0x0000
#define	SYM_CODE	0xC0DE
#define	SYM_BYTE	0xDA08
#define	SYM_WORD	0xDA10
#define	SYM_LONG	0xDA20
#define	SYM_TEXT	0xDADA
#define	SYM_RESERVE	0x56C5
#define	SYM_XREG	0x1234
#define	SYM_STRV	0xD080
#define	SYM_KEYTAB	0x55F0
#define	SYM_VAR		0x41E2

struct s_lhglobalsym {
  PLHU8       name;
  LHU16       value;
  LHU16       magic;
};

typedef struct s_lhglobalsym	LHGLOBALSYM;
typedef struct s_lhglobalsym *	PLHGLOBALSYM;

struct s_lhsym {
  struct s_lhsym *next;
  PLHU8          name;
  LHU16          value;
  LHU16          magic;
};

typedef struct s_lhsym	LHSYM;
typedef struct s_lhsym *PLHSYM;

/* Prototypes */
PLHSYM lhSymAlloc(PLHU8 name, LHU16 value, LHU16 magic);
PLHSYM lhSymAddSorted(PLHSYM sym, PLHU8 name, LHU16 value, LHU16 magic);
PLHSYM lhSymFlushAll( PLHSYM sym );
LHU16 lhSymGetValue(PLHSYM sym, PLHU8 name);
LHU16 lhSymGetMagic(PLHSYM sym, PLHU8 name);
PLHU8 lhSymGetName(PLHSYM sym, LHU16 value);
PLHU8 lhSymGetNextName(PLHSYM sym, LHU16 value, PLHU8 name);
PLHU8 lhSymGetNameOrAddr(PLHSYM sym, LHU16 value, PLHU8 *pname);
int lhSymImportSymbol( char *symfile, PLHSYM *psym, LHU16 magic );

#endif /* __LHSYM_H__ */

