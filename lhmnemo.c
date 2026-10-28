// Copyright 1992-2013 Christophe Gottheimer <cgh75015@gmail.com>
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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lhtype.h"
#include "lh5801.h"
#include "lhsym.h"
#include "lhbasic.h"
#include "lhunasm.h"

int main( int argc, char **argv )
{
  int page, i, j, l, index;
  PLHINST inst;
  char *pinst, *pass;
  char strinst[10], *str;
  LHU16 reg, cnd, code, link;

  for( page = 0; page < 2; page++ )
    {
    printf( "\n   " );
    for( i = 0; i < 16; i++ )
      printf( "|  %02X  ", i*16 );
    printf( "\n" );
    for( j = 0; j < 16; j++ )
      {
      for( l = 0; l < 2; l++ )
      {
      if( l )
        printf( "   " );
      else
        printf( " %02X", j );
      for( i = 0; i < 16; i++ )
        {
        code = (i*16 + j);
        if( NULL == (inst = lhUnasmFetch( page * 0xFD, code, lhinst )) )
          strcpy( strinst, "     " );
        else
          {
          pinst = inst->inst;
          link = lhUnasmDoLink( pinst );
          str = strinst;
          while(*pinst >= '0' && *pinst <= 'Z')
            *str++ = *pinst++;
          *str = '\0';
          reg = GET_LHREGS(code);
          cnd = GET_LHCNDS(code);
          if( l )
            memset( strinst, 0, sizeof(strinst) );
          for(index = 0, pass="\0,"; l && index < 2; index++, pass++)
            {
            if( ((link >> (8 - index * 8)) & 0xff) != LINK_NONE )
              strncat( strinst, pass, 1 );
            switch((link >> (8 - index * 8)) & 0xff)
              {
              case LINK_NONE:
                break;
              case LINK_MEM_8:
                strcat( strinst, "n" );
                break;
              case LINK_MEM_16:
                strcat( strinst, "mn" );
                break;
              case LINK_SBR:
              case LINK_iMEM_8:
                strcat( strinst, "(n)" );
                break;
              case LINK_iMEM_16:
                strcat( strinst, "(mn)" );
                break;
              case LINK_KYWD:
                strcat( strinst, "k" );
                break;
              case LINK_REG_L:
                strcat( strinst, lhregs[reg + BASE_REG_L].regs );
                break;
              case LINK_REG_H:
                strcat( strinst, lhregs[reg + BASE_REG_H].regs );
                break;
              case LINK_REG_BC:
                strcat( strinst, lhregs[BASE_REG_BC].regs );
                break;
              case LINK_REG_HL:
                strcat( strinst, lhregs[reg + BASE_REG_HL].regs );
                break;
              case LINK_iREG_HL:
                strcat( strinst, lhregs[reg + BASE_iREG_HL].regs );
                break;
              case LINK_REG_SP:
                strcat( strinst, lhregs[BASE_REG_SP].regs );
                break;
              case LINK_REG_PC:
                strcat( strinst, lhregs[BASE_REG_PC].regs );
                break;
              case LINK_REG_A:
                strcat( strinst, lhregs[BASE_REG_A].regs );
                break;
              case LINK_REG_F:
                strcat( strinst, lhregs[BASE_REG_F].regs );
                break;
              case LINK_REG_T0:
                strcat( strinst, lhregs[BASE_REG_T0].regs );
                break;
              case LINK_REG_T1:
                strcat( strinst, lhregs[BASE_REG_T1].regs );
                break;
              case LINK_CNDS:
                strcat( strinst, lhcnds[cnd].cnds );
                break;
              case LINK_JREL:
                strcat( strinst, ((code & 0xf0) == 0x90 || code == 0x88) ? "-d" : "+d" );
                break;
              }
            }
          }
        printf( "|%-6s", strinst );
        }
      printf( "\n" );
      }
      }
    }
}

