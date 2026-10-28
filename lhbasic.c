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

/* lhbasic.c - Definitions for the LHBASIC interpretor			*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lhtype.h"
#include "lhbasic.h"

static LHBASIC  lhbasic[] = {
  { "AREAD",	0xF180, 0xC684, 'N', },
  { "AND",	0xF150, 0xCD89, 'N', },
  { "ABS",	0xF170, 0xF597, 'N', },
  { "ATN",	0xF175, 0xF496, 'N', },
  { "ASN",	0xF173, 0xF49A, 'N', },
  { "ACS",	0xF174, 0xF492, 'N', },
  { "ASC",	0xF160, 0xD9DD, 'N', },
  { "ARUN",	0xF181, 0xC684, 'N', },
  { "BEEP",	0xF182, 0xE5C1, 'N', },
  { "CONT",	0xF183, 0xC8C7, 'N', },
  { "CURSOR",	0xF084, 0xE846, 'N', },
  { "CLEAR",	0xF187, 0xC85F, 'N', },
  { "CLS",	0xF088, 0xE865, 'N', },
  { "COS",	0xF17E, 0xF391, 'N', },
  { "CHR$",	0xF163, 0xD9B1, 'N', },
  { "CALL",	0xF18A, 0xC863, 'N', },
  { "DIM",	0xF18B, 0xC988, 'N', },
  { "DEGREE",	0xF18C, 0xC697, 'N', },
  { "DEG",	0xF165, 0xF531, 'N', },
  { "DMS",	0xF166, 0xF564, 'N', },
  { "DATA",	0xF18D, 0xC684, 'N', },
  { "END",	0xF18E, 0xC50D, 'N', },
  { "EXP",	0xF178, 0xF1CB, 'N', },
  { "ERROR",	0xF1B4, 0xCD89, 'N', },
  { "FOR",	0xF1A5, 0xC711, 'N', },
  { "GOTO",	0xF192, 0xC515, 'N', },
  { "GOSUB",	0xF194, 0xC64E, 'N', },
  { "GPRINT",	0xF09F, 0xE7AC, 'N', },
  { "GCURSOR",	0xF093, 0xE83E, 'N', },
  { "GRAD",	0xF186, 0xC6A8, 'N', },
  { "INPUT",	0xF091, 0xC8FA, 'N', },
  { "IF",	0xF196, 0xC5B4, 'N', },
  { "INT",	0xF171, 0xF5BE, 'N', },
  { "INKEY$",	0xF15C, 0xD9AA, 'N', },
  { "LIST",	0xF090, 0xC96E, 'N', },
  { "LOG",	0xF177, 0xF165, 'N', },
  { "LN",	0xF176, 0xF161, 'N', },
  { "LET",	0xF198, 0xC458, 'N', },
  { "LEN",	0xF164, 0xD9DD, 'N', },
  { "LEFT$",	0xF17A, 0xD9F3, 'N', },
  { "LOCK",	0xF1B5, 0xC968, 'N', },
  { "MEM",	0xF158, 0xDA5D, 'N', },
  { "MID$",	0xF17B, 0xD9F3, 'N', },
  { "NEXT",	0xF19A, 0xC705, 'N', },
  { "NOT",	0xF16D, 0x599E, 'N', },
  { "NEW",	0xF19B, 0xC80A, 'N', },
  { "ON",	0xF19C, 0xC5E0, 'N', },
  { "OR",	0xF151, 0xCD89, 'N', },
  { "OPN",	0xF19D, 0xE457, 'N', },
  { "OFF",	0xF19E, 0xCD89, 'N', },
  { "PRINT",	0xF097, 0xE4EB, 'N', },
  { "PI",	0xF15D, 0xF5B5, 'N', },
  { "PEEK#",	0xF16E, 0xD993, 'N', },
  { "PEEK",	0xF16F, 0xD993, 'N', },
  { "POKE#",	0xF1A0, 0xC778, 'N', },
  { "POKE",	0xF1A1, 0xC777, 'N', },
  { "POINT",	0xF168, 0xEECB, 'N', },
  { "PAUSE",	0xF1A2, 0xE6A5, 'N', },
  { "P    ",	0xF1A3, 0xCD89, 'N', },
  { "RUN",	0xF1A4, 0xC8B4, 'N', },
  { "RETURN",	0xF199, 0xC6AC, 'N', },
  { "READ",	0xF1A6, 0xC7B8, 'N', },
  { "RESTORE",	0xF1A7, 0xC7A2, 'N', },
  { "RND",	0xF17C, 0xF5DD, 'N', },
  { "RANDOM",	0xF1A8, 0xF641, 'N', },
  { "RIGHT$",	0xF172, 0xD9F3, 'N', },
  { "RADIAN",	0xF1AA, 0xC6A4, 'N', },
  { "REM",	0xF1AB, 0xC676, 'N', },
  { "STOP",	0xF1AC, 0xC4B6, 'N', },
  { "SQR",	0xF16B, 0xF0E9, 'N', },
  { "SIN",	0xF17D, 0xF3A2, 'N', },
  { "SGN",	0xF179, 0xF59D, 'N', },
  { "STR$",	0xF161, 0xD9CE, 'N', },
  { "STATUS",	0xF167, 0x5A44, 'N', },
  { "STEP",	0xF1AD, 0xCD89, 'N', },
  { "THEN",	0xF1AE, 0xCD89, 'N', },
  { "TAN",	0xF17F, 0xF39E, 'N', },
  { "TIME",	0xF15B, 0xDE82, 'N', },
  { "TRON",	0xF1AF, 0xC68C, 'N', },
  { "TROFF",	0xF1B0, 0xC693, 'N', },
  { "TO",	0xF1B1, 0xCD89, 'N', },
  { "USING",	0xF085, 0xC67C, 'N', },
  { "UNLOCK",	0xF1B6, 0xC96A, 'N', },
  { "VAL",	0xF162, 0xD9D7, 'N', },
  { "WAIT",	0xF1B3, 0xE86A, 'N', },
  { "COLOR",	0xF0B5, 0xB16A, 'N', },
  { "CSIZE",	0xE680, 0xB180, 'N', },
  { "GRAPH",	0xE681, 0xACD3, 'N', },
  { "GLCURSOR",	0xE682, 0xB191, 'N', },
  { "LCURSOR",	0xE683, 0xB201, 'N', },
  { "LF",	0xF0B6, 0xB1B4, 'N', },
  { "LINE",	0xF0B7, 0xB222, 'N', },
  { "LLIST",	0xF0B8, 0xB754, 'N', },
  { "LPRINT",	0xF0B9, 0xB2EC, 'N', },
  { "RLINE",	0xF0BA, 0xB224, 'N', },
  { "ROTATE",	0xE685, 0xB15A, 'N', },
  { "SORGN",	0xE684, 0xB153, 'N', },
  { "TAB",	0xF0BB, 0xB201, 'N', },
  { "TEST",	0xF0BC, 0xAB91, 'N', },
  { "TEXT",	0xE686, 0xACA6, 'N', },
  { "CHAIN",	0xF0B2, 0xBB6A, 'N', },
  { "CLOAD",	0xF089, 0xB8F9, 'N', },
  { "CSAVE",	0xF095, 0xB8A6, 'N', },
  { "MERGE",	0xF08F, 0xB994, 'N', },
  { "RMT",	0xE7A9, 0xBEF9, 'N', },
#ifdef CE158
  { "BREAK",	0xF0B3, 0xffff, 'N', },
  { "COM$",	0xE858, 0xffff, 'N', },
  { "CONSOLE",	0xF0B1, 0xffff, 'N', },
  { "DEV$",	0xE857, 0xffff, 'N', },
  { "DTE",	0xE884, 0xffff, 'N', },
  { "ERL",	0xF053, 0xffff, 'N', },
  { "ERN",	0xF052, 0xffff, 'N', },
  { "FEED",	0xF0B0, 0xffff, 'N', },
  { "INSTAT",	0xE859, 0xffff, 'N', },
  { "OUTSTAT",	0xE880, 0xffff, 'N', },
  { "PROTOCOL",	0xE881, 0xffff, 'N', },
  { "RINKEY$",	0xE85A, 0xffff, 'N', },
  { "SETCOM",	0xE882, 0xffff, 'N', },
  { "SETDEV",	0xE886, 0xffff, 'N', },
  { "SPACE$",	0xF061, 0xffff, 'N', },
  { "TERMINAL",	0xE883, 0xffff, 'N', },
  { "TRANSMIT",	0xE885, 0xffff, 'N', },
  { "ZONE",	0xF0B4, 0xffff, 'N', },
#endif
    { (PLHU8)0,	0x0000, 0x0000, '\0', },
};

// 2 Keyword tables by &1000, as the keyword table is aligned on &800
// &0000 is not possible, neither in &7000..&7FFF
// &C000..&FFFF is the standard BASIC ROM.
static LHBASIC lhuser[24 * 256] = {
    { (PLHU8)0,        0x0000, 0x0000, '\0', },
};

void lhBasicFlushAll( void )
{
  register PLHBASIC pbasic = lhuser;

  for( ; pbasic->code && pbasic < (lhuser + sizeof(lhuser)); pbasic++ )
    free(pbasic->basic);

  memset( lhuser, 0, sizeof(lhuser) );
}

LHU16 lhBasicRegister(PLHU8 keytab)
{
  register PLHBASIC pbasic = lhuser;
  LHU8 byte, bits;

  if(pbasic->basic) {
    fprintf(stderr,"LHBASIC user keyword already registered.\n");
    return(0);
  }
  while((byte = (*(keytab++) & 0x0f))) {
    if(!(pbasic->basic = (PLHU8)malloc(((unsigned)byte) + 1))) {
      fprintf(stderr,"Allocation failed for registering keyword.\n");
      return 0;
    }
    memcpy(pbasic->basic, keytab, byte);
    pbasic->basic[byte] = '\0';
    pbasic->code = (*(keytab + byte) << 8) |
                   (*(keytab + byte + 1) & 0xff);
    pbasic->jump = (*(keytab + byte + 2) << 8) |
                   (*(keytab + byte + 3) & 0xff);
    pbasic->bits = lhBasicConvertBits( (*(keytab + byte + 4)) & 0xE0 );
    keytab += (byte + 4);
    pbasic++;
  }
  return((unsigned)(pbasic - lhuser) / sizeof(LHBASIC)); 
}

LHU16 lhBasicRegisterOneKeyword( PLHU8 keyword, LHU16 code, LHU16 jump, LHU8 bits,
                                 void (*CB)( PLHU8, LHU16, LHU16, LHU8 ) )
{
  register PLHBASIC pbasic = lhuser;
  LHU8 byte;

  while(pbasic->code && pbasic < (lhuser + sizeof(lhuser)))
    pbasic++;

  if(pbasic == (lhuser + sizeof(lhuser)) || !code || !keyword)
    return 0;

  if(!(pbasic->basic = (PLHU8)malloc(strlen(keyword) + 1))) {
    fprintf(stderr,"Allocation failed for registering keyword.\n");
    return 0;
  }
  strcpy(pbasic->basic, keyword);
  pbasic->code = code;
  pbasic->jump = jump;
  pbasic->bits = bits;

  if( CB )
    CB( keyword, code, jump, bits );

  return(code);
}

PLHU8 lhBasicGetBasic(LHU16 code)
{
  register PLHBASIC pbasic = lhuser;

  while(pbasic->basic && (pbasic->code != code)) {
    pbasic++;
  }
  if(!pbasic->basic && ((pbasic = lhbasic)->basic)) {
    while(pbasic->basic && (pbasic->code != code)) {
      pbasic++;
    }
  }
  return(pbasic->basic);
}

LHU16 lhBasicGetCode(PLHU8 keyword)
{
  LHU16 jump;
  LHU8 bits;
  return(lhBasicGetCodeBitsAndJump(keyword, &bits, &jump));
}

LHU16 lhBasicGetCodeAndJump(PLHU8 keyword, LHU16 *jump)
{
  LHU8 bits;
  return(lhBasicGetCodeBitsAndJump(keyword, &bits, jump));
}

LHU16 lhBasicGetCodeBitsAndJump(PLHU8 keyword, LHU8 *bits, LHU16 *jump)
{
  register PLHBASIC pbasic = lhuser;

  while(pbasic->basic &&
        strcmp(keyword, pbasic->basic)) {
    pbasic++;
  }
  if(!pbasic->basic && ((pbasic = lhbasic)->basic)) {
    while(pbasic->basic &&
          strcmp(keyword, pbasic->basic)) {
      pbasic++;
    }
  }
  *jump = pbasic->jump;
  *bits = pbasic->bits;
  return(pbasic->code);
}

LHU16 lhBasicGetCodeClosest(PLHU8 keyword, PLHU8 *next)
{
  register PLHBASIC pbasic = lhuser;

  *next = keyword;
  while(pbasic->basic &&
        strncmp(keyword, pbasic->basic, strlen(pbasic->basic))) {
    pbasic++;
  }
  if(!pbasic->basic && ((pbasic = lhbasic)->basic)) {
    while(pbasic->basic &&
          strncmp(keyword, pbasic->basic, strlen(pbasic->basic))) {
      pbasic++;
    }
  }
  if( pbasic->basic )
    *next = keyword + strlen(pbasic->basic);
  return(pbasic->code);
}

LHU16 lhBasicGetCodeFromJump(LHU16 jump, LHU8 *bits)
{
  register PLHBASIC pbasic = lhbasic;

  while(pbasic->basic && (pbasic->jump != jump)) {
    pbasic++;
  }
  if(!pbasic->basic && ((pbasic = lhuser)->basic)) {
    while(pbasic->basic && (pbasic->jump != jump)) {
      pbasic++;
    }
  }
  *bits = pbasic->bits;
  return(pbasic->code);
}

LHU16 lhBasicGetNextCodeFromJump(LHU16 jump, LHU16 code, LHU8 *bits)
{
  register PLHBASIC pbasic = lhbasic;

  while(pbasic->basic && (pbasic->code != code)) {
    pbasic++;
  }
  if(pbasic->basic)
    pbasic++;
  while(pbasic->basic && (pbasic->jump != jump)) {
    pbasic++;
  }
  if(!pbasic->basic && ((pbasic = lhuser)->basic)) {
    while(pbasic->basic && (pbasic->code != code)) {
      pbasic++;
    }
    if(pbasic->basic)
      pbasic++;
    while(pbasic->basic && (pbasic->jump != jump)) {
      pbasic++;
    }
  }
  *bits = pbasic->bits;
  return(pbasic->code);
}

LHU8 lhBasicConvertBits( LHU8 bits )
{
  if( bits & 0x80 )
    {
    if( 0xC0 == bits )
      return 'N';
    else if( 0xA0 == bits )
      return 'P';
    else if( 0x80 == bits )
      return 'C';
    else
      return '?';
    }
  else
    {
    if( 'N' == bits )
      return 0xC0;
    else if( 'P' == bits )
      return 0xA0;
    else if( 'C' == bits )
      return 0x80;
    else
      return 0xE0;
    }
}

int lhBasicImportKeyword( char *keywfile, void (*CB)( PLHU8, LHU16, LHU16, LHU8 ) )
{
  LHU8 bits, *pbits;
  FILE *fksfilein;
  char string[256];
  LHU16 arghx, lasthx;

  if(!(fksfilein = fopen( keywfile, "r" )))
    {
    return -1;
    }
  if( 1 != fscanf( fksfilein, "%s", string ) || strcasecmp( string, LHBASIC_MAGIC ))
    {
    fclose( fksfilein );
    return -2;
    }
  for( ; ; )
    {
    if( 1 == fscanf(fksfilein, "%hx", &arghx) && (arghx & 0x0F) )
      {
      if( 3 == fscanf(fksfilein, "%s %hx %hx", string, &arghx, &lasthx) &&
          strlen( string ) && arghx)
        {
        bits = 'N';
        goto registerkeyword;
        }
    }
    else if( 4 == fscanf(fksfilein, "%s %hx %hx %c", string, &arghx, &lasthx, &bits) &&
            strlen(string) && arghx && (pbits = strchr( LHBASIC_BITS, toupper(bits))) )
      {
      bits = *pbits;
registerkeyword:
      if( *(string + strlen(string) - 1) == '\"' )
        *(string + strlen(string) - 1) = '\0';
      lhBasicRegisterOneKeyword( (*string == '\"' ? string + 1 : string),
                                 arghx, lasthx, bits, CB );
      }
    else
      break;
    }
  fclose(fksfilein);
  return 0;
}
