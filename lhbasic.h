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

/* lhbasic.h - Definitions for the LHBASIC interpretor			*/

#ifndef __LHBASIC_H__

#define __LHBASIC_H__

#define LHBASIC_MAGIC	".KEYWORD:"

#define LHBASIC_BITS	"NPC?"

#define BASIC_BITS_NORMAL	'N'	// bit7,6 := &Cx
#define BASIC_BITS_PROGRAMMABLE	'P'	// bit7,5 := &Ax
#define BASIC_BITS_COMMAND	'C'	// bit7   := &8x
#define BASIC_BITS_OTHER	'?'

extern void lhBasicFlushAll( void );
extern LHU16 lhBasicRegister(PLHU8 keytab);
extern LHU16 lhBasicRegisterOneKeyword( PLHU8 keyword, LHU16 code, LHU16 jump, LHU8 bits, 
                                        void (*CB)( PLHU8, LHU16, LHU16, LHU8 ) );
extern PLHU8 lhBasicGetBasic(LHU16 code);
extern LHU16 lhBasicGetCode(PLHU8 keyword);
extern LHU16 lhBasicGetCodeAndJump(PLHU8 keyword, LHU16 *jump);
extern LHU16 lhBasicGetCodeBitsAndJump(PLHU8 keyword, LHU8 *bits, LHU16 *jump);
extern LHU16 lhBasicGetCodeClosest(PLHU8 keyword, PLHU8 *next);
extern LHU16 lhBasicGetCodeFromJump(LHU16 jump, LHU8 *bits);
extern LHU16 lhBasicGetNextCodeFromJump(LHU16 jump, LHU16 code, LHU8 *bits);
extern LHU8 lhBasicConvertBits( LHU8 bits );
extern int lhBasicImportKeyword( char *keywfile,
                                 void (*CB)( PLHU8, LHU16, LHU16, LHU8 ) );

#endif /* __LHBASIC_H__ */

