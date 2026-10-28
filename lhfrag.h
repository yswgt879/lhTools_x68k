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

/* lhfile.h - Definitions for the file headers and structures		*/

#ifndef __LHFRAGMENT_H__

#define __LHFRAGMENT_H__

#define LHFRAGMENT_MAGIC	".FRAGMENTS:"

/* Fragment type magic number	*/
#define	FRAG_BASIC	0x7865
#define	FRAG_RESERVE	0x56C5
#define	FRAG_XREG	0x1234
#define	FRAG_VAR	0x41E2
#define	FRAG_CODE	0xC0DE
#define	FRAG_BYTE	0xDA08
#define	FRAG_WORD	0xDA10
#define	FRAG_LONG	0xDA20
#define	FRAG_TEXT	0x2424
#define	FRAG_KEYWORD	0x55F0
#define	FRAG_HOLE	0xFFFF

#define	FRAGIDX_BASIC	     1
#define	FRAGIDX_RESERVE	     2
#define	FRAGIDX_XREG	     3
#define	FRAGIDX_VAR	     4
#define	FRAGIDX_CODE	     5
#define	FRAGIDX_BYTE	     6
#define	FRAGIDX_WORD	     7
#define	FRAGIDX_LONG	     8
#define	FRAGIDX_TEXT	     9
#define	FRAGIDX_KEYWORD	    10
#define	FRAGIDX_HOLE	    11
#define	FRAGIDX_LAST	    (FRAGIDX_HOLE+1)

struct s_lhfrag {
  LHU16 magic;   /* fragment type MAGIC number	*/
  LHU16 chksum;	/* checksum of this fragment	*/
  LHU16 origin;	/* code origin of this fragment	*/
  LHU16 length;	/* code length of this fragment	*/
  PLHU8 comment;
};

typedef	struct s_lhfrag		LHFRAG;
typedef	struct s_lhfrag *	PLHFRAG;

PLHU8 lhFragmentGetFragment(LHU16 magic);
LHU16 lhFragmentGetMagic(PLHU8 tstr);
PLHU8  lhFragmentGetFragmentFromIndex(LHU8 index);
LHU8  lhFragmentGetIndex(PLHU8 tstr);

#endif /* __LHFRAGMENT_H__ */

