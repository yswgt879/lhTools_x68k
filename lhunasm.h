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

/* */

#ifndef __LHUNASM_H__

#define __LHUNASM_H__

LHU16 lhUnasmJump(LHU16 code, LHU16 jump, LHU16 base);
unsigned lhUnasmDoLink(PLHU8 mnemo);
LHU16 lhUnasmPage(LHU16 page, PLHU8 inst);
PLHINST lhUnasmFetch(LHU16 page, LHU16 code, PLHINST lhtab);
PLHU8 lhUnasm(PLHU16 base, PLHU8 address, PLHU8 string, PLHSYM *sym, char disjr,
              char upass, LHU16 top, LHU16 bottom);

#endif /* __LHUNASM_H__ */

