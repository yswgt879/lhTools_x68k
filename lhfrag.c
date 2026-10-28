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

/* lhfile.c - Definitions for the file headers and structures		*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "lhtype.h"
#include "lhfrag.h"

struct s_lhfragmagic {
  PLHU8 str;
  LHU16 magic;
  LHU8  index;
};

static struct s_lhfragmagic lhfragmagic[] = {
  { "BASIC",	FRAG_BASIC,   FRAGIDX_BASIC, },
  { "RESERVE",	FRAG_RESERVE, FRAGIDX_RESERVE, },
  { "XREG",	FRAG_XREG,    FRAGIDX_XREG, },
  { "VAR",	FRAG_VAR,     FRAGIDX_VAR, },
  { "CODE",	FRAG_CODE,    FRAGIDX_CODE, },
  { "BYTE",	FRAG_BYTE,    FRAGIDX_BYTE, },
  { "WORD",	FRAG_WORD,    FRAGIDX_WORD, },
  { "LONG",	FRAG_LONG,    FRAGIDX_LONG, },
  { "TEXT",	FRAG_TEXT,    FRAGIDX_TEXT, },
  { "KEYWORD",	FRAG_KEYWORD, FRAGIDX_KEYWORD, },
  { "HOLE",	FRAG_HOLE,    FRAGIDX_HOLE, },
  { NULL,	0x0000 }
};

PLHU8 lhFragmentGetFragment(LHU16 magic)
{
  register struct s_lhfragmagic *pfragmagic = lhfragmagic;

  for( ; pfragmagic->magic && magic != pfragmagic->magic; pfragmagic++)
    ;
  return(pfragmagic->str);
}

LHU16 lhFragmentGetMagic(PLHU8 tstr)
{
  register struct s_lhfragmagic *pfragmagic = lhfragmagic;

  for( ; pfragmagic->magic && strcasecmp(pfragmagic->str, tstr); pfragmagic++)
    ;
  return(pfragmagic->magic);
}

PLHU8 lhFragmentGetFragmentFromIndex(LHU8 index)
{
  register struct s_lhfragmagic *pfragmagic = lhfragmagic;

  for( ; pfragmagic->index && index != pfragmagic->index; pfragmagic++)
    ;
  return(pfragmagic->str);
}

LHU8 lhFragmentGetIndex(PLHU8 tstr)
{
  register struct s_lhfragmagic *pfragmagic = lhfragmagic;

  for( ; pfragmagic->magic && strcasecmp(pfragmagic->str, tstr); pfragmagic++)
    ;
  return(pfragmagic->index);
}

