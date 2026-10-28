// Copyright 1992-2012 Christophe Gottheimer <cgh75015_gmail.com>
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

/* lhsym.c - Symbols manager 						*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lhtype.h"
#include "lhsym.h"

static LHGLOBALSYM lhgsym[] = {
  { "BEEP1",		0xE669, SYM_CODE },
  { "BEEP",		0xE66F, SYM_CODE },
  { "INKEY",		0xE42C, SYM_CODE },
  { "ISBREAK",		0xE451, SYM_CODE },
  { "GETKEY",		0xE24A, SYM_CODE },
  { "HEXDEC",		0xED95, SYM_CODE },
  { "CLS",		0xE771, SYM_CODE },
  { "PRINTA",		0xED5B, SYM_CODE },
  { "GDISPA",		0xEDEB, SYM_CODE },
  { "GPRINTA",		0xEDF6, SYM_CODE },
  { "POSTOADR",		0xEE22, SYM_CODE },
  { "COLTOA",		0xEECE, SYM_CODE },
#if 0
  { "&CDE6",		0xFFFF, SYM_CODE },
  { "&CE38",		0xFFFF, SYM_CODE },
  { "&CF27",		0xFFFF, SYM_CODE },
  { "&CFCC",		0xFFFF, SYM_CODE },
  { "&CFF7",		0xFFFF, SYM_CODE },
  { "&D00D",		0xFFFF, SYM_CODE },
  { "&D02B",		0xFFFF, SYM_CODE },
  { "&D048",		0xFFFF, SYM_CODE },
  { "&D091",		0xFFFF, SYM_CODE },
  { "&D0AA",		0xFFFF, SYM_CODE },
  { "&D0B0",		0xFFFF, SYM_CODE },
  { "&D2E0",		0xFFFF, SYM_CODE },
  { "&D3C7",		0xFFFF, SYM_CODE },
  { "&D3D5",		0xFFFF, SYM_CODE },
  { "&D407",		0xFFFF, SYM_CODE },
  { "&D9E7",		0xFFFF, SYM_CODE },
  { "&DA6C",		0xFFFF, SYM_CODE },
  { "&DB95",		0xFFFF, SYM_CODE },
  { "&DBB3",		0xFFFF, SYM_CODE },
  { "&DBCA",		0xFFFF, SYM_CODE },
  { "&DE99",		0xFFFF, SYM_CODE },
  { "&DEC7",		0xFFFF, SYM_CODE },
  { "&DEFC",		0xFFFF, SYM_CODE },
  { "&DFA1",		0xFFFF, SYM_CODE },
  { "&DFB4",		0xFFFF, SYM_CODE },
  { "&E5B4",		0xFFFF, SYM_CODE },
  { "&ECAE",		0xFFFF, SYM_CODE },
  { "&ECFA",		0xFFFF, SYM_CODE },
  { "&ED7D",		0xFFFF, SYM_CODE },
  { "&EE88",		0xFFFF, SYM_CODE },
  { "&EE99",		0xFFFF, SYM_CODE },
  { "&EEB0",		0xFFFF, SYM_CODE },
  { "&F019",		0xFFFF, SYM_CODE },
  { "&F082",		0xFFFF, SYM_CODE },
  { "&F0E9",		0xFFFF, SYM_CODE },
  { "&F707",		0xFFFF, SYM_CODE },
  { "&F711",		0xFFFF, SYM_CODE },
  { "&F737",		0xFFFF, SYM_CODE },
  { "&F73F",		0xFFFF, SYM_CODE },
  { "&F743",		0xFFFF, SYM_CODE },
  { "&F75B",		0xFFFF, SYM_CODE },
  { "&F763",		0xFFFF, SYM_CODE },
  { "&F7BB",		0xFFFF, SYM_CODE },
  { "&F957",		0xFFFF, SYM_CODE },
  { "&FA58",		0xFFFF, SYM_CODE },
#endif
  { "HIGHLCDFLAG",	0x764E, SYM_BYTE },
  { "LOWLCDFLAG",	0x764F, SYM_BYTE },
  { "STARTRAM",		0x7863, SYM_BYTE },
  { "ENDRAM",		0x7864, SYM_BYTE },
  { "STARTBASIC",	0x7865, SYM_WORD },
  { "ENDBASIC",		0x7867, SYM_WORD },
  { "MERGEBASIC",	0x7869, SYM_WORD },
  { "ERRORNUM",		0x789B, SYM_BYTE },
  { "ERRORLINE",	0x78B4, SYM_WORD },
  { "CONCATCHAR",	0xD925, SYM_CODE },
  { "CHR",		0xD9B1, SYM_CODE },
  { "STR",		0xD9CF, SYM_CODE },
  { "VAL",		0xD9D7, SYM_CODE },
  { "LEN",		0xD9DD, SYM_CODE },
  { "ASC",		0xD9DD, SYM_CODE },
  { "RIGHT",		0xD9F3, SYM_CODE },
  { "MID",		0xD9F3, SYM_CODE },
  { "LEFT",		0xD9F3, SYM_CODE },
  { "SUBSTRACT",	0xEFB6, SYM_CODE },
  { "ADD",		0xEFBA, SYM_CODE },
  { "MULTIPLY",		0xF01A, SYM_CODE },
  { "DIVIDE",		0xF084, SYM_CODE },
  { "LN",		0xF161, SYM_CODE },
  { "LOG",		0xF165, SYM_CODE },
  { "EXP",		0xF1CB, SYM_CODE },
  { "10N",		0xF1D4, SYM_CODE },
  { "COS",		0xF391, SYM_CODE },
  { "TAN",		0xF39E, SYM_CODE },
  { "SIN",		0xF3A2, SYM_CODE },
  { "ACS",		0xF492, SYM_CODE },
  { "ATN",		0xF496, SYM_CODE },
  { "ASN",		0xF49A, SYM_CODE },
  { "DEG",		0xF531, SYM_CODE },
  { "DMS",		0xF564, SYM_CODE },
  { "ABS",		0xF597, SYM_CODE },
  { "SGN",		0xF59D, SYM_CODE },
  { "INT",		0xF5BE, SYM_CODE },
  { "POW",		0xF89C, SYM_CODE },
  { "NUMCMP",		0xD0D2, SYM_CODE },
  { "STRCMP",		0xD0F9, SYM_CODE },
  { "LINESEARCH",	0xD2EA, SYM_CODE },
  { "VARSEARCH",	0xD461, SYM_CODE },
  { "KEYSCAN1",		0xE42C, SYM_CODE },
  { "KEYSCAN2",		0xE243, SYM_CODE },
  { "AUTOPOWOFF",	0xE33F, SYM_CODE },
  { "PRGMDISP",		0xE8CA, SYM_CODE },
  { "GRAPHDISP",	0xEDEF, SYM_CODE },
  { "ONECHARDISP",	0xED57, SYM_CODE },
  { "NCHARDISP",	0xED3B, SYM_CODE },
  { "CURMOVONECHAR",	0xED4D, SYM_CODE },
  { "CURMOVNCHAR",	0xED00, SYM_CODE },
  { "_A",		0x7900, SYM_XREG },
  { "_B",		0x7908, SYM_XREG },
  { "_C",		0x7910, SYM_XREG },
  { "_D",		0x7918, SYM_XREG },
  { "_E",		0x7920, SYM_XREG },
  { "_F",		0x7928, SYM_XREG },
  { "_G",		0x7930, SYM_XREG },
  { "_H",		0x7938, SYM_XREG },
  { "_I",		0x7940, SYM_XREG },
  { "_J",		0x7948, SYM_XREG },
  { "_K",		0x7950, SYM_XREG },
  { "_L",		0x7958, SYM_XREG },
  { "_M",		0x7960, SYM_XREG },
  { "_N",		0x7968, SYM_XREG },
  { "_O",		0x7970, SYM_XREG },
  { "_P",		0x7978, SYM_XREG },
  { "_Q",		0x7980, SYM_XREG },
  { "_R",		0x7988, SYM_XREG },
  { "_S",		0x7990, SYM_XREG },
  { "_T",		0x7998, SYM_XREG },
  { "_U",		0x79A0, SYM_XREG },
  { "_V",		0x79A8, SYM_XREG },
  { "_W",		0x79B0, SYM_XREG },
  { "_X",		0x79B8, SYM_XREG },
  { "_Y",		0x79C0, SYM_XREG },
  { "_Z",		0x79C8, SYM_XREG },
  { "A$",		0x78C0, SYM_STRV },
  { "B$",		0x78D0, SYM_STRV },
  { "C$",		0x78E0, SYM_STRV },
  { "D$",		0x78F0, SYM_STRV },
  { "E$",		0x7650, SYM_STRV },
  { "F$",		0x7660, SYM_STRV },
  { "G$",		0x7670, SYM_STRV },
  { "H$",		0x7680, SYM_STRV },
  { "I$",		0x7690, SYM_STRV },
  { "J$",		0x76A0, SYM_STRV },
  { "K$",		0x76B0, SYM_STRV },
  { "L$",		0x76C0, SYM_STRV },
  { "M$",		0x76D0, SYM_STRV },
  { "N$",		0x76E0, SYM_STRV },
  { "O$",		0x76F0, SYM_STRV },
  { "P$",		0x7750, SYM_STRV },
  { "Q$",		0x7760, SYM_STRV },
  { "R$",		0x7770, SYM_STRV },
  { "S$",		0x7780, SYM_STRV },
  { "T$",		0x7790, SYM_STRV },
  { "U$",		0x77A0, SYM_STRV },
  { "V$",		0x77B0, SYM_STRV },
  { "W$",		0x77C0, SYM_STRV },
  { "X$",		0x77D0, SYM_STRV },
  { "Y$",		0x77E0, SYM_STRV },
  { "Z$",		0x77F0, SYM_STRV },
  { "XREG",		0x7A00, SYM_XREG },
  { "ZREG",		0x7A08, SYM_XREG },
  { "YREG",		0x7A10, SYM_XREG },
  { "UREG",		0x7A18, SYM_XREG },
  { "VREG",		0x7A20, SYM_XREG },
  { "WREG",		0x7A28, SYM_XREG },
  { "SREG",		0x7A30, SYM_XREG },
  { "BASICSTACKTOP",	0x7A38, SYM_BYTE },
  { "BASICSTACKBOTTOM",	0x7AFF, SYM_BYTE },
  { "SYSTEMSTACKEND",	0x7800, SYM_BYTE },
  { "SYSTEMSTACKBASE",	0x784F, SYM_BYTE },
  { "STRINGBUF",	0x7B10, SYM_TEXT },
  { "OUTPUTBUF",	0x7B60, SYM_BYTE },
  { "INPUTBUF",		0x7BB0, SYM_BYTE },
  { (PLHU8)0,		0x0000, SYM_NONE }
};

PLHSYM lhSymAlloc(PLHU8 name, LHU16 value, LHU16 magic)
{
  PLHSYM psym;

  psym = (PLHSYM) malloc(sizeof(LHSYM) + strlen( name ) + 1);
  psym->name = strcpy( (char *)(&psym[1]), name );
  psym->value = value;
  psym->magic = magic;
  psym->next = (PLHSYM)NULL;
  return(psym);
}

PLHSYM lhSymFlushAll( PLHSYM sym )
{
  PLHSYM ts;

  while( sym )
    {
    ts = sym;
    sym = sym->next;
    free( ts );
    }
  return sym;
}

PLHSYM lhSymAddSorted(PLHSYM sym, PLHU8 name, LHU16 value, LHU16 magic)
{
  register PLHSYM psym, osym;
  PLHSYM tsym;
  int  cmp;

  for(osym = (PLHSYM)NULL, psym = sym;
      psym != (PLHSYM)NULL;
      osym = psym, psym = psym->next) {
    if((cmp = strcmp(psym->name, name)) > 0) {
      break;
    }
    else if(cmp == 0 && psym->magic == magic) {
      fprintf(stderr, "Warning: symbol %s is redefined\n", name);
      psym->value = value;
      return(sym);
    }
  }

  tsym = lhSymAlloc(name, value, magic);
  tsym->next = psym;
  if(osym != (PLHSYM)NULL) {
    osym->next = tsym;
  }
  else {
    sym = tsym;
  }
  return(sym);
}

LHU16 lhSymGetValue(PLHSYM sym, PLHU8 name)
{
  register PLHGLOBALSYM gsym = lhgsym;

  while(sym != (PLHSYM) NULL) {
    if(strcmp(sym->name, name) == 0)
      return(sym->value);
    sym = sym->next;
  }
  
  while(gsym->name != (PLHU8) NULL) {
    if(strcmp(gsym->name, name) == 0)
      return(gsym->value);
    gsym++;
  }
  
  return(0xffff);
}

LHU16 lhSymGetMagic(PLHSYM sym, PLHU8 name)
{
  register PLHGLOBALSYM gsym = lhgsym;

  while(sym != (PLHSYM) NULL) {
    if(strcmp(sym->name, name) == 0)
      return(sym->magic);
    sym = sym->next;
  }
  
  while(gsym->name != (PLHU8) NULL) {
    if(strcmp(gsym->name, name) == 0)
      return(gsym->magic);
    gsym++;
  }
  
  return(SYM_NONE);
}

PLHU8 lhSymGetName(PLHSYM sym, LHU16 value)
{
  register PLHGLOBALSYM gsym = lhgsym;

  while(sym != (PLHSYM) NULL) {
    if(sym->value == value)
      return(sym->name);
    sym = sym->next;
  }
  
  while(gsym->name != (PLHU8) NULL) {
    if(gsym->value == value)
      return(gsym->name);
    gsym++;
  }
  
  return((PLHU8) NULL);
}

PLHU8 lhSymGetNextName(PLHSYM sym, LHU16 value, PLHU8 name)
{
  register PLHGLOBALSYM gsym = lhgsym;

  while(sym != (PLHSYM) NULL) {
    if((sym->value == value) && (0 == strcmp(sym->name, name))) {
      sym = sym->next;
      break;
    }
    sym = sym->next;
  }
  
  while(sym != (PLHSYM) NULL) {
    if(sym->value == value)
      return(sym->name);
    sym = sym->next;
  }

  while(gsym->name != (PLHU8) NULL) {
    if(gsym->value == value)
      return(gsym->name);
    gsym++;
  }
  
  return((PLHU8) NULL);
}

PLHU8 lhSymGetNameOrAddr(PLHSYM sym, LHU16 value, PLHU8 *pname)
{
  PLHU8 name;

  if((name = lhSymGetName( sym, value)) == (PLHU8) NULL)
    {
    static LHU8 symAddr[7];

    sprintf(name = symAddr, "%04hX", value);
    if( pname )
      *pname = NULL;
  }
  else if( pname )
    *pname = name;
  return(name);
}

int lhSymImportSymbol( char *symfile, PLHSYM *psym, LHU16 magic )
{
  FILE *fksfilein;
  char string[256];
  LHU16 arghx;
  int nnnsym = 0;

  if(!(fksfilein = fopen(symfile, "r"))) {
    return -1;
  }
  if(fscanf(fksfilein, "%s", string) != 1 || strcasecmp(string, LHSYM_MAGIC)) {
    fclose(fksfilein);
    return -2;
  }
  while(fscanf(fksfilein, "%hx %s", &arghx, string) == 2) {
    *psym = lhSymAddSorted(*psym, string, arghx, magic);
    nnnsym++;
  }
  fclose(fksfilein);

  return nnnsym;
}
