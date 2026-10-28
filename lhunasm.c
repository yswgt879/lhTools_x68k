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

/* lhunasm.c - LH 5801 lhUnasmassembler					*/

#include <stdio.h>

#include "lhtype.h"
#include "lh5801.h"
#include "lhsym.h"
#include "lhunasm.h"
#include "lhbasic.h"

static LHU16 lhUnasmMask[] = {
  0xff,		/* no mask			*/
  MASK_REGS,	/* registers masked b5-b4	*/
  MASK_CNDS,	/* conditions masked b3-b1	*/
  MASK_JREL,	/* relative jump masked b4-b1	*/
  MASK_CALL	/* 1-byte vectorized call b5-b1	*/
};

LHU16 lhUnasmJump(LHU16 code, LHU16 jump, LHU16 base)
{
  return(base + (((code & 0xf0) == 0x90) || (code == 0x88) ? -jump : jump));
}

unsigned lhUnasmDoLink(PLHU8 mnemo)
{
  unsigned link;

  while(*mnemo >= 'A' && *mnemo <= 'Z')
    mnemo++;
  if(*mnemo == '\0')
    return(0);
  if(*mnemo == '!' && *(mnemo+1) == '\0' )
    return(0);
  if(*(mnemo++) == '!') {
    if(*mnemo == '%')
      mnemo++;
  }
  link = *(mnemo++) << 16;
  if(*(mnemo++) == '\0')
    return(link);
  else {
    link |= (*(mnemo++) << 8);
  if(*(mnemo++) == '\0')
    return(link);
  else
    return(link | (*mnemo));
  }
}

LHU16 lhUnasmPage(LHU16 page, PLHU8 inst)
{
  while(*inst) {
    if(*inst == '!') {
      if( *(inst + 1) == '%' )
        return( 1 );
      return(page == SECOND_PAGE);
    }
    if(*inst == '%') {
      return(page != SECOND_PAGE);
    }
    inst++;
  }
  return !page;
}

PLHINST lhUnasmFetch(LHU16 page, LHU16 code, PLHINST lhtab)
{
  register PLHINST pinst = lhtab;

  while(pinst->inst) {
    if(pinst->code == (code & pinst->umsk)) {
      if(lhUnasmPage(page, pinst->inst))
        return(pinst);
    }
    pinst++;
  }
  return((PLHINST) 0);
}

PLHU8 lhUnasmDoSymbol( PLHSYM *sym, LHU16 address, char upass, LHU16 top, LHU16 bottom )
{
  PLHU8   pname, pnm;
  LHU8    symna[12];

  pnm = lhSymGetNameOrAddr(*sym, address, &pname);
  if( upass == 1 && !pname && (top <= address && address <= bottom) )
    {
    snprintf(symna, sizeof(symna), "lbl_%hx_%03hx", (address & 0xF000) >> 12, address & 0xFFF);
    *sym = lhSymAddSorted(*sym, symna, address, 0);
    return "";
    }
  return pnm;
}

PLHU8 lhUnasm(PLHU16 base, PLHU8 address, PLHU8 string, PLHSYM *sym, char disjr,
              char upass, LHU16 top, LHU16 bottom)
{
  LHU16   code, page = 0, cnd, reg, index, bytes = 0;
  LHU16   cd34 = 0;
  LHU8    pass;
  PLHINST mnemo;
  PLHU8   pinst;
  PLHU8   passchr = "\t,,,,", pagechr = "";
  unsigned link;

  code = GET_LHU8(address++);
  bytes++;
  if(code == SECOND_PAGE) {
    code = GET_LHU8(address++);
    page = SECOND_PAGE;
    pagechr = "#";
    bytes++;
  }
  if((mnemo = lhUnasmFetch(page, code, lhinst)) != (PLHINST) 0)
    link = lhUnasmDoLink(mnemo->inst);
  if(mnemo != (PLHINST) 0) {
    pinst = mnemo->inst;
    while(*pinst >= '0' && *pinst <= 'Z')
      *string++ = *pinst++;
      *string = '\0';
    reg = GET_LHREGS(code);
    cnd = GET_LHCNDS(code);
    for(index = 0; index < 3; index++) {
      while(*string)
        string++;
      if(cd34 > 0)
        {
        index = 0;
        link = (cd34 % 2 ? LINK_JREL : LINK_MEM_8) << 16;
        cd34--;
        }
      else
        pass = *(passchr++);
      switch((link >> (16 - index * 8)) & 0xff) {
        case LINK_NONE:
          break;
        case LINK_PAGE:
          sprintf(string, "%s", pagechr);
          break;
        case LINK_MEM_8:
          sprintf(string, "%c%02X", pass, GET_LHU8(address++));
          bytes++;
          break;
        case LINK_MEM_16:
          sprintf(string, "%c%s", pass,
                  lhUnasmDoSymbol(sym, GET_LHU16(address), upass, top, bottom));
          address += 2;
          bytes += 2;
          break;
        case LINK_iMEM_8:
          code = GET_LHU8(address++);
          bytes++;
          sprintf(string, "%s%c(%02X)", pagechr, pass, code);
          break;
        case LINK_SBR:
          code = GET_LHU8(address++);
          bytes++;
          sprintf(string, "%s%c(%02X)", pagechr, pass, code);
          mnemo = lhUnasmFetch(0, code, lhsbr);
          link = (mnemo != (PLHINST)0 ?
                  lhUnasmDoLink(mnemo->inst) : LINK_NONE);
          index=-1;
          break;
        case LINK_iMEM_16:
          sprintf(string, "%s%c(%s)", pagechr, pass,
                  lhUnasmDoSymbol(sym, GET_LHU16(address), upass, top, bottom));
          address += 2;
          bytes += 2;
          break;
        case LINK_KYWD:
          bytes++;
          if(GET_LHU8(address) < 0xe0)
            sprintf(string,"%c%02X", pass, GET_LHU8(address++));
          else {
            PLHU8 pbasic = lhBasicGetBasic(GET_LHU16(address));
            if(pbasic)
              sprintf(string,"%c%s", pass, pbasic);
            else
              sprintf(string,"%c%04X", pass, GET_LHU16(address));
            address += 2;
            bytes++;
          }
          break;
        case LINK_CD34:
          cd34 = ((GET_LHU8(address) + 1) * 2);
          sprintf(string, "%c%02X", pass, GET_LHU8(address++));
          bytes++;
          index = 0;
          break;
        case LINK_REG_L:
          sprintf(string, "%c%s", pass, lhregs[reg + BASE_REG_L].regs);
          break;
        case LINK_REG_H:
          sprintf(string, "%c%s", pass, lhregs[reg + BASE_REG_H].regs);
          break;
        case LINK_REG_BC:
          sprintf(string, "%c%s", pass, lhregs[BASE_REG_BC].regs);
          break;
        case LINK_REG_HL:
          sprintf(string, "%c%s", pass, lhregs[reg + BASE_REG_HL].regs);
          break;
        case LINK_iREG_HL:
          sprintf(string, "%s%c%s", pagechr, pass, lhregs[reg + BASE_iREG_HL].regs);
          break;
        case LINK_REG_SP:
          sprintf(string, "%c%s", pass, lhregs[BASE_REG_SP].regs);
          break;
        case LINK_REG_PC:
          sprintf(string, "%c%s", pass, lhregs[BASE_REG_PC].regs);
          break;
        case LINK_REG_A:
          sprintf(string, "%c%s", pass, lhregs[BASE_REG_A].regs);
          break;
        case LINK_REG_F:
          sprintf(string, "%c%s", pass, lhregs[BASE_REG_F].regs);
          break;
        case LINK_REG_T0:
          sprintf(string, "%c%s", pass, lhregs[BASE_REG_T0].regs);
          break;
        case LINK_REG_T1:
          sprintf(string, "%c%s", pass, lhregs[BASE_REG_T1].regs);
          break;
        case LINK_CNDS:
          sprintf(string, "%c%s", pass, lhcnds[cnd].cnds);
          break;
        case LINK_JREL:
          bytes++;
          if(disjr)
            sprintf(string, "%c%c%02X", pass,
                    ((code & 0xf0) == 0x90 || code == 0x88) ? '-' : '+',
                    GET_LHU8(address++));
          else
            sprintf(string, "%c%s", pass,
                    lhUnasmDoSymbol(sym,
                                    lhUnasmJump(code,
                                             GET_LHU8(address++),
                                             *base + bytes), upass, top, bottom));
          break;
      }
    } 
  }
  else {
    if(page == SECOND_PAGE)
      sprintf(string, "\\xFD,\\x%02X", code);
    else
      sprintf(string, "\\x%02X", code);
  }
  *base += bytes++;
  return(address);
}

