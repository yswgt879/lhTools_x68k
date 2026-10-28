// Copyright 1992-2014 Christophe Gottheimer <cgh75015@gmail.com>
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
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lhtype.h"
#include "lh5801.h"
#include "lhsym.h"
#include "lhbasic.h"
#define FRAG_STRUCT   0x7374
#include "lhfrag.h"
#include "lhce158hdr.h"

#define LHASM_NONE           0
#define LHASM_BASIC          FRAGIDX_BASIC
#define LHASM_CODE           FRAGIDX_CODE
#define LHASM_TEXT           FRAGIDX_TEXT
#define LHASM_BYTE           FRAGIDX_BYTE
#define LHASM_WORD           FRAGIDX_WORD
#define LHASM_LONG           FRAGIDX_LONG
#define LHASM_VAR            FRAGIDX_VAR
#define LHASM_RESERVE        FRAGIDX_RESERVE
#define LHASM_XREG           FRAGIDX_XREG
#define LHASM_KYWD           FRAGIDX_KEYWORD
#define LHASM_HOLE           FRAGIDX_HOLE
#define LHASM_MACRO_DEF     (FRAGIDX_LAST + 1)
#define LHASM_STRUCT_DEF    (FRAGIDX_LAST + 2)
#define LHASM_DATA_DEF      (FRAGIDX_LAST + 3)
#define LHASM_DATAINIT_DEF  (FRAGIDX_LAST + 4)
#define LHASM_DATAEND_DEF   (FRAGIDX_LAST + 5)
#define LHASM_NEED_END      128
#define LHASM_SYMBOLS       (LHU8)-2
#define LHASM_END           (LHU8)-1

#define LHASM_PASS_1_ONLY          0x0001
#define LHASM_IMMEDIATE            0x0002
#define LHASM_ALL_LOCAL            0x0008
#define LHASM_DEBUG                0x0010
#define LHASM_VERBOSE              0x0020
#define LHASM_TRACE                0x0040
#define LHASM_NOCOMMENT            0x0100
#define LHASM_NOSYMBOL             0x0200
#define LHASM_NOLOCAL              0x0400
#define LHASM_OUTPUT_HEX           0x1000
#define LHASM_NOBINARY             0x2000
#define LHASM_KEYWORD_OLD         0x10000
#define LHASM_KEYWORD_EXPORT      0x20000
#define LHASM_JR_TO_JP           0x100000
#define LHASM_JP_DO_OPTIMIZE     0x200000
#define LHASM_LOW_WARNING       0x1000000
#define LHASM_WARNING_AS_ERROR  0x2000000
#define LHASM_FATAL_ERROR       0x4000000
// Reserved for lhasm runtime operation - Not setable by option
#define LHASM_OP_SYM_EXIST     0x80000000
#define LHASM_OP_HAS_KEYWORD   0x40000000

#define	LHASM_IF_NONE        0x00
#define	LHASM_IF_FALSE       0x10
#define	LHASM_IF_TRUE        0x11
#define	LHASM_IF_ELSE_FALSE  0x20
#define	LHASM_IF_ELSE_TRUE   0x21

#define	LHASM_NOP_NONE       0x00
#define	LHASM_NOP_TRUE       0x01
#define	LHASM_NOP_FALSE      0xFE

#define LHASM_DO_NONE        0x00
#define LHASM_DO_IF          0x01
#define LHASM_DO_ELSE        0x02
#define LHASM_DO_BEGIN       0x04
#define LHASM_DO_WHILE       0x08
#define LHASM_DO_FORCE_JR    0x80

extern const char const lhTools_vers[], lhTools_date[], lhTools_pltf[];

#define LHASM_MAX_ARGS       341

LHU8 asmmem[65536];
PLHU8 asmcurrent = asmmem, asmbase;
int asmorg = 0x000FFFFF, asmaddr = 0x000FFFFF;
int asmtop = 0x7FFFFFFF, asmbottom = -1;
LHU8 *kytab = NULL, *kyhash = NULL;
LHU8 kych = 0;
char *comment = NULL, *fragment = NULL;
PLHSYM asmsym = (PLHSYM)NULL;
PLHSYM resym = (PLHSYM)NULL;
#define LHASM_MAX_LOCAL     100
PLHSYM asmlocal[ LHASM_MAX_LOCAL ];
PLHSYM asmincsym = (PLHSYM)NULL;
PLHSYM asmkywsym = (PLHSYM)NULL;
LHU8 asmpass = 0;
int asmflags = 0;
unsigned int asmvers;

struct s_lhasmchar
  {
  char *str;
  char *alias;
  LHU8	val;
  };
struct s_lhasmchar asmchar[] =
{
  { "null",        NULL,           0x00, },
  { "soh",         NULL,           0x01, },
  { "stx",         NULL,           0x02, },
  { "etx",         NULL,           0x03, },
  { "eot",         NULL,           0x04, },
  { "enq",         NULL,           0x05, },
  { "ack",         NULL,           0x06, },
  { "bel",         NULL,           0x07, },
  { "bs",          NULL,           0x08, },
  { "tab",         NULL,           0x09, },
  { "lf",          NULL,           0x0a, },
  { "vt",          NULL,           0x0b, },
  { "ff",          NULL,           0x0c, },
  { "cr",          NULL,           0x0d, },
  { "so",          NULL,           0x0e, },
  { "si",          NULL,           0x0f, },
  { "dle",         NULL,           0x10, },
  { "dc1",         NULL,           0x11, },
  { "dc2",         NULL,           0x12, },
  { "dc3",         NULL,           0x13, },
  { "dc4",         NULL,           0x14, },
  { "nak",         NULL,           0x15, },
  { "syn",         NULL,           0x16, },
  { "etb",         NULL,           0x17, },
  { "can",         NULL,           0x18, },
  { "em",          NULL,           0x19, },
  { "sub",         NULL,           0x1a, },
  { "esc",         NULL,           0x1b, },
  { "fs",          NULL,           0x1c, },
  { "gs",          NULL,           0x1d, },
  { "rs",          NULL,           0x1e, },
  { "us",          NULL,           0x1f, },
  { "space",       NULL,           0x20, },
  { "exclam",      NULL,           0x21, },
  { "doublequote", "stringmark",   0x22, },
  { "sharp",       NULL,           0x23, },
  { "dollar",      NULL,           0x24, },
  { "percent",     NULL,           0x25, },
  { "ampersand",   "hexmark",      0x26, },
  { "quote",       "insert",       0x27, },
  { "openparenthesis",  NULL,      0x28, },
  { "closeparenthesis", NULL,      0x29, },
  { "star",        "multiply",     0x2a, },
  { "plus",        "add",          0x2b, },
  { "comma",       NULL,           0x2c, },
  { "minus",       "substract",    0x2d, },
  { "dot",         "period",       0x2e, },
  { "slash",       "divide",       0x2f, },
  { "zero",        NULL,           0x30, },
  { "one",         NULL,           0x31, },
  { "two",         NULL,           0x32, },
  { "three",       NULL,           0x33, },
  { "four",        NULL,           0x34, },
  { "five",        NULL,           0x35, },
  { "six",         NULL,           0x36, },
  { "seven",       NULL,           0x37, },
  { "eight",       NULL,           0x38, },
  { "nine",        NULL,           0x39, },
  { "colon",       NULL,           0x3a, },
  { "semicolon",   NULL,           0x3b, },
  { "less",        NULL,           0x3c, },
  { "equal",       NULL,           0x3d, },
  { "greater",     NULL,           0x3e, },
  { "question",    NULL,           0x3f, },
  { "at",          NULL,           0x40, },
  { "upper.a",     NULL,           0x41, },
  { "upper.b",     NULL,           0x42, },
  { "upper.c",     NULL,           0x43, },
  { "upper.d",     NULL,           0x44, },
  { "upper.e",     NULL,           0x45, },
  { "upper.f",     NULL,           0x46, },
  { "upper.g",     NULL,           0x47, },
  { "upper.h",     NULL,           0x48, },
  { "upper.i",     NULL,           0x49, },
  { "upper.j",     NULL,           0x4a, },
  { "upper.k",     NULL,           0x4b, },
  { "upper.l",     NULL,           0x4c, },
  { "upper.m",     NULL,           0x4d, },
  { "upper.n",     NULL,           0x4e, },
  { "upper.o",     NULL,           0x4f, },
  { "upper.p",     NULL,           0x50, },
  { "upper.q",     NULL,           0x51, },
  { "upper.r",     NULL,           0x52, },
  { "upper.s",     NULL,           0x53, },
  { "upper.t",     NULL,           0x54, },
  { "upper.u",     NULL,           0x55, },
  { "upper.v",     NULL,           0x56, },
  { "upper.w",     NULL,           0x57, },
  { "upper.x",     NULL,           0x58, },
  { "upper.y",     NULL,           0x59, },
  { "upper.z",     NULL,           0x5a, },
  { "openbracket", "squareroot",   0x5b, },
  { "backslash",   "yen",          0x5c, },
  { "closebracket","pi",           0x5d, },
  { "circumflex",  "power",        0x5e, },
  { "underscore",  NULL,           0x5f, },
  { "backquote",   NULL,           0x60, },
  { "lower.a",     NULL,           0x61, },
  { "lower.b",     NULL,           0x62, },
  { "lower.c",     NULL,           0x63, },
  { "lower.d",     NULL,           0x64, },
  { "lower.e",     NULL,           0x65, },
  { "lower.f",     NULL,           0x66, },
  { "lower.g",     NULL,           0x67, },
  { "lower.h",     NULL,           0x68, },
  { "lower.i",     NULL,           0x69, },
  { "lower.j",     NULL,           0x6a, },
  { "lower.k",     NULL,           0x6b, },
  { "lower.l",     NULL,           0x6c, },
  { "lower.m",     NULL,           0x6d, },
  { "lower.n",     NULL,           0x6e, },
  { "lower.o",     NULL,           0x6f, },
  { "lower.p",     NULL,           0x70, },
  { "lower.q",     NULL,           0x71, },
  { "lower.r",     NULL,           0x72, },
  { "lower.s",     NULL,           0x73, },
  { "lower.t",     NULL,           0x74, },
  { "lower.u",     NULL,           0x75, },
  { "lower.v",     NULL,           0x76, },
  { "lower.w",     NULL,           0x77, },
  { "lower.x",     NULL,           0x78, },
  { "lower.y",     NULL,           0x79, },
  { "lower.z",     NULL,           0x7a, },
  { "openbrace",   NULL,           0x7b, },
  { "verticalbar", NULL,           0x7c, },
  { "closebrace",  NULL,           0x7d, },
  { "tilde",       NULL,           0x7e, },
  { "delete",      "fullcursor",   0x7f, },
  { NULL,          NULL,           0x00, },
};

struct s_lhasmfrag
  {
  };

#define LHASM_MAX_MEMSECTION   8
struct s_lhasmmemsection
  {
  char *name;
  LHU16 base;
  LHU16 end;
#define LHASM_MEM_NONE         0x00
#define LHASM_MEM_RAM          0x01
#define LHASM_MEM_ROM          0x02
#define LHASM_MEM_SYS          0x04
#define LHASM_MEM_LM           0x08
#define LHASM_MEM_USERRAM      0x11
#define LHASM_MEM_USERROM      0x12
  LHU8  type;
  } asmmemsection[ LHASM_MAX_MEMSECTION ];
LHU8 asmsection = LHASM_MEM_NONE;

struct s_lhasmmemsection asm_pc1500[] = 
{
  { "RAM", 0x4000, 0x47FF, LHASM_MEM_RAM,  },
  { "SYS", 0x7000, 0x7FFF, LHASM_MEM_SYS,  },
  { "ROM", 0x8000, 0xFFFF, LHASM_MEM_ROM,  },
  {  NULL,      0,      0, LHASM_MEM_NONE, },
};

struct s_lhasmmemsection asm_pta4000_16[] = 
{
  { "RAM", 0x0000, 0x47FF, LHASM_MEM_RAM,  },
  { "SYS", 0x7000, 0x7FFF, LHASM_MEM_SYS,  },
  { "ROM", 0x8000, 0xFFFF, LHASM_MEM_ROM,  },
  {  NULL,      0,      0, LHASM_MEM_NONE, },
};

struct s_lhasmmemsection asm_pc1500a[] = 
{
  { "RAM", 0x4000, 0x57FF, LHASM_MEM_RAM,  },
  { "SYS", 0x7000, 0x7C00, LHASM_MEM_SYS,  },
  {  "LM", 0x7C01, 0x7FFF, LHASM_MEM_LM,  },
  { "ROM", 0x8000, 0xFFFF, LHASM_MEM_ROM,  },
  {  NULL,      0,      0, LHASM_MEM_NONE, },
};

struct s_lhasmmemsection asm_pc1560[] = 
{
  { "RAM", 0x0000, 0x7FFF, LHASM_MEM_RAM,  },
  { "ROM", 0x8000, 0xFFFF, LHASM_MEM_ROM,  },
  {  NULL,      0,      0, LHASM_MEM_NONE, },
};

struct s_lhasmmachine
  {
  char                     *name;
  struct s_lhasmmemsection *section;
  LHU16                     module;
  LHU16                     before;
  LHU16                     after;
  LHU16                     value;
  } asmmachine[] = 
{
#define LHASM_MACHINE_DEFAULT     0
  { "PC1500",        asm_pc1500,      0,      0,      0, 0x1500, },
  { "PC1500A",       asm_pc1500a,      0,      0,      0, 0x150A, },
  { "PTA4000+16", asm_pta4000_16, 0xEFFF,      0,      0, 0x4016, },
  { "PC1560",         asm_pc1560, 0xEFFF,      0,      0, 0x1560, },
  { "CE151",                NULL,      1,      0, 0x1000, 0xE151, },
  { "CE155",                NULL,      1, 0x0800, 0x1800, 0xE155, },
  { "CE159",                NULL,      1, 0x2000,      0, 0xE159, },
  { "CE161",                NULL,      1, 0x4000,      0, 0xE161, },
  { "CE163",                NULL,      1, 0x4000,      0, 0xE163, },
  {         NULL,           NULL,      1,      0,      0, 0xEFFF, },
};
struct s_lhasmmachine *asmpmachine = NULL;
struct s_lhasmmachine *asmpmodule = NULL;
struct s_lhasmmachine asmhwmachine;

struct s_lhasmctx
  {
  jmp_buf env;
  LHU8 state;
  LHU8 ifstate;
  LHU8 nopstate;
  LHU8 exportall;
  int iflinenum;
  int noplinenum;
  int srclinenum;
  int error;
  int included;
  PLHSYM *plocal;
  char *srcfile;
#define LHASM_MAX_CTX       10
  LHU16 start, end, length;
  int nndostack;
  } asmctx[LHASM_MAX_CTX];
int asmnnctx = 0;
struct s_lhasmctx *asmpctx = asmctx;

struct s_lhasminlin
  {
  LHU16 code;
  LHU16 linenum;
  LHU8  *plinebase;
  LHU8  *plineasm;
  LHU8  *pcurrent;
  } asminlin;
struct s_lhasminlin *asmpinlin = NULL;

char outfile[PATH_MAX];
char *frgfile = NULL;
FILE *frgfd = NULL;
char *keywfile = NULL;
FILE *keywfd = NULL;
char *symfile = NULL;
FILE *symfd = NULL;
char *macfile = NULL;
FILE *macfd = NULL;
char *logfile = NULL;
FILE *logfd = NULL;
char *resrcfile = NULL;
FILE *resrcfd = NULL;
char *resymfile = NULL;
FILE *resymfd = NULL;

struct s_lhasminc
  {
#ifdef _WIN32
  char name[PATH_MAX];
#else
  dev_t dev;
  ino_t ino;
#endif
#define LHASM_MAX_INCLUDE      100
  } asmincluded[ LHASM_MAX_INCLUDE ];

#define LHASM_MAX_INCLUDE_PATH  10
char *asmincpath[ LHASM_MAX_INCLUDE_PATH ];

#define LHASM_MAX_MACRO        100
struct s_lhasmmacro
  {
  char name[25];
  int  thisline;
  int  nnline;
#define LHASM_MAX_LINES_BY_MACRO   30
  char *lines[LHASM_MAX_LINES_BY_MACRO];
  int  nnarg;
#define LHASM_MAX_ARG_BY_MACRO     10
  char *arg[LHASM_MAX_ARG_BY_MACRO];
  } asmmacro[LHASM_MAX_MACRO];
struct s_lhasmmacro *asmrunmacro = NULL;
int asmnnmacro = 0;
int asmnnrun = 0;
#define LHASM_MAX_RUN          300
LHU16 asmrunlbl[10 * LHASM_MAX_RUN];
LHU16 asmlblmask[ LHASM_MAX_RUN ];

struct s_lhasmstruct;

struct s_lhasmfield
  {
  char name[25];
  short type;
  short array;
  short size;
  short element;
  short offset;
  LHU16 fragment;
  struct s_lhasmstruct *pstruct;
  };

#define LHASM_MAX_STRUCT        100
struct s_lhasmstruct
  {
  char name[25];
  int  size;
  int  nnfield;
#define LHASM_MAX_FIELDS_BY_STRUCT   30
  struct s_lhasmfield fields[LHASM_MAX_FIELDS_BY_STRUCT];
  } asmstruct[LHASM_MAX_STRUCT];
int asmnnstruct = 0;

#define LHASM_MAX_DATA        100
struct s_lhasmdata
  {
  char name[25];
  struct s_lhasmstruct *pstruct;
  LHU16 base;
  LHU16 end;
  LHU16 size;
  LHU16 array;
  LHU16 element;
  } asmdata[LHASM_MAX_DATA];
int asmnndata = 0;

struct s_lhasmbind
  {
  struct s_lhasmdata *pdata;
  struct s_lhasmstruct *pstruct;
  struct s_lhasmfield *pfield;
  LHU16 infield;
  LHU16 offset;
  LHU16 atbase;
  } asmbindR[ 4 ];

#define LHASM_MAX_VARS         100
LHU16 asmvars[ LHASM_MAX_VARS ]['z' - 'a' + 1];
int asmvarmask[ LHASM_MAX_VARS ];

struct s_lhasmsubs
  {
  struct s_lhasmsubs *next;
  char               *name;
  char               *body;
  };
struct s_lhasmsubs *asmsubs = NULL;

#define LHASM_MAX_JRJP        2000
LHU8 asmJRJP[LHASM_MAX_JRJP];
int asmnnJRJP = 0;
int asmJRJPpass = 0;
unsigned int asmJRJPloop = 0;
unsigned int asmJRJPmaxloop = ~0;

struct s_lhasmdostack
  {
  char *srcfile;
  int   srclinenum;
  PLHU8 current;
  int   jrjp;
  LHU16 jr;
  LHU8	cond;
  LHU8	state;
  };
#define LHASM_MAX_DO_STACK    100
struct s_lhasmdostack asmdostack[LHASM_MAX_DO_STACK];
int asmnndostack = 0;

struct tm asmtm;

#define LHASM_WRN( fmt, args... ) do { \
                                     if( asmflags & LHASM_WARNING_AS_ERROR ) \
                                       LHASM_ERR( fmt, ## args ); \
                                     fprintf( stderr, "%s line %u: --WARNING-- " fmt, \
                                              asmpctx->srcfile, asmpctx->srclinenum, ## args ); \
                                     if( logfile ) \
                                       fprintf( logfd, "%s line %u: --WARNING-- " fmt, \
                                              asmpctx->srcfile, asmpctx->srclinenum, ## args ); \
                                     } while( 0 )
#define LHASM_LOW( fmt, args... ) do { \
                                     if( asmflags & LHASM_LOW_WARNING ) \
                                       LHASM_WRN( fmt, ## args ); \
                                     } while( 0 )
#define LHASM_ERR( fmt, args... ) do { \
                                     if( asmflags & LHASM_FATAL_ERROR ) \
                                       LHASM_FTL( fmt, ## args ); \
                                     fprintf( stderr, "%s line %u: ***ERROR*** " fmt, \
                                              asmpctx->srcfile, asmpctx->srclinenum, ## args ); \
                                     if( logfile ) \
                                       fprintf( logfd, "%s line %u: ***ERROR*** " fmt, \
                                              asmpctx->srcfile, asmpctx->srclinenum, ## args ); \
                                     longjmp( asmpctx->env, 0xFFFF ); \
                                     } while( 0 )
#define LHASM_FTL( fmt, args... ) do { \
                                     fprintf( stderr, "%s line %u: !!!FATAL!!! " fmt, \
                                              asmpctx->srcfile, asmpctx->srclinenum, ## args ); \
                                     if( logfile ) \
                                       fprintf( logfd, "%s line %u: !!!FATAL!!! " fmt, \
                                              asmpctx->srcfile, asmpctx->srclinenum, ## args ); \
                                     exit( 255 ); \
                                     } while( 0 )

int lhAsmGetLHUValue( char *sym, char *str, LHU16 *v, int link, LHU16 defval, LHU16 off );

int lhAsmGetPC( void );

void usage(PLHU8 prog)
{
  fprintf(stderr,
          "Usage: %s [-h] [-v] [-d|-ddebug] [-dverbose]] [-i] [-E[E]] [-W]\n"
          "\t\t[-A argument=substitute...] [-D symbol=value...] [-a] [-na]\n"
          "\t\t[-T|-L logfile ] [-nc] [-ns] [-r resymfil] [-s resrcfile]\n"
          "\t\t[-F fragfile] [-K[K][E] keywordfile] [-M macfile] [-S symfile]\n"
          "\t\t[-m machine] [-O origin] [fragment] [-I includepath]\n"
          "\t\t[-Z[header=type]] [-Z[name=headername]] [-Z[entry=startup]]\n"
          "\t\t[-J[loop=n]] [-1] [-N|-no] [-x] [-o outfile] srcfile\n",
          prog);
  fprintf(stderr,"where:\n");
  fprintf(stderr,"\t-1\t\tDo assembler pass 1 only and stop\n");
  fprintf(stderr,"\t-a\t\tAll symbols treated as local except if .EXPORT: is specified\n");
  fprintf(stderr,"\t-d|-debug\tShow debug information\n");
  fprintf(stderr,"\t-dverbose\tEnable verbose mode\n");
  fprintf(stderr,"\t-h\t\tThis help\n");
  fprintf(stderr,"\t-i\t\tImmediate one pass only assembler. Read from stdin\n"
                 "\t\t\tIncompatible with -J\n");
  fprintf(stderr,"\t-m machine\tSelect the machine and modules\n");
  fprintf(stderr,"\t   -m pc1500\tRAM=&4000..&47FF ROM=&8000..&FFFF; This is the default\n");
  fprintf(stderr,"\t   -m pc1500A\tRAM=&4000..&57FF ROM=&8000..&FFFF LM=&7C01..7FFFF\n");
  fprintf(stderr,"\t   -m pta4000+16\tRAM=&0000..&47FF ROM=&8000..&FFFF\n");
  fprintf(stderr,"\t   -m pc1560\tRAM=&0000..&8000..&FFFF\n");
  fprintf(stderr,"\t   -m ce151\tAdd &1000 at end of RAM; This is CE-151\n");
  fprintf(stderr,"\t   -m ce155\tAdd &800 before RAM and &1800 at end of RAM; This is CE-155\n");
  fprintf(stderr,"\t   -m ce159\tAdd &1000 before RAM; This is CE-159\n");
  fprintf(stderr,"\t   -m ce161\tAdd &4000 before RAM; This is CE-161\n");
  fprintf(stderr,"\t   -m ce163\tAdd &4000 before RAM; This is CE-163, bank1 is not supported\n");
  fprintf(stderr,"\t   Only one -m ce... option may be given\n");
  fprintf(stderr,"\t-na\t\tDisable local symbols list into log file (with -T or -L)\n");
  fprintf(stderr,"\t-nc\t\tDisable comment copy into log file (with -T or -L)\n");
  fprintf(stderr,"\t-ns\t\tDisable symbols/variables list into log file (with -T or -L)\n");
  fprintf(stderr,"\t-o outfile\tOutput binary code into outfile (.bin)\n");
  fprintf(stderr,"\t-r resymfile\tInput file of addresses to be re-symbol'ed\n");
  fprintf(stderr,"\t-s srcfile\tOutput file (.asm) of the full source with re-symbols\n");
  fprintf(stderr,"\t-v\t\tShow version and exit\n");
  fprintf(stderr,"\t-x\t\tOutput hexadecimal dump instead of binary into outfile (.hex) \n");
  fprintf(stderr,"\t-A argument=substitute\tReplace argument by substitute when defining a symbol\n"
                 "\t\t\tSeveral -A may be given if several symbols need to be defined\n");
  fprintf(stderr,"\t-D symbol=value\tDefine the specified symbol to the given value\n"
                 "\t\t\tSeveral -D may be given if several symbols need to be defined\n");
  fprintf(stderr,"\t-E\t\tWarnings treated as errors\n");
  fprintf(stderr,"\t-EE\t\tErrors are fatal\n");
  fprintf(stderr,"\t-F fragfile\tOutput fragments to fragfile (.frag)\n");
  fprintf(stderr,"\t-I includepath\tAdd the <includepath> to the directories list where to search\n"
                 "\t\t\tthe files to include\n");
  fprintf(stderr,"\t-J\t\tReplace JR by JP if displacement > 255. Incompatible with -i\n");
  fprintf(stderr,"\t-Jloop=n\tLike -J, but run only n optimization loops\n");
  fprintf(stderr,"\t-K keywordfile\tOutput the BASIC keywords table to keywordfile (.keyw)\n");
  fprintf(stderr,"\t-KK keywordfile\tOutput the BASIC keywords table to keywordfile (.keyw)\n"
                 "\t\t\twith old format < 0.6.0\n");
  fprintf(stderr,"\t-K[K]E keywordfile\tExport declared BASIC keywords to keywordfile (.keyw)\n"
                 "\t\t\t(with old format < 0.6.0 if option is -KKE)\n");
  fprintf(stderr,"\t-L logfile\tOutput logs of the assembler processing into logfile (.log)\n"
                 "\t\t\tThis option is exclusive with the -T option\n");
  fprintf(stderr,"\t-M macfile\tOutput defined macros to macfile (.mac)\n");
  fprintf(stderr,"\t-N|-no\t\tDo not output generated binary code\n");
  fprintf(stderr,"\t-O origin\tSet origin address to specified value\n");
  fprintf(stderr,"\t-S symfile\tOutput declared symbols to symfile (.sym)\n");
  fprintf(stderr,"\t-T\t\tEnable trace mode output to stderr.\n"
                 "\t\t\tThis is exclusive with the -L log option\n");
  fprintf(stderr,"\t-W\t\tEnable low priority warnings\n");
  fprintf(stderr,"\tfragment\tSet original fragment\n");
  fprintf(stderr,"\t   -B\tBASIC fragment; This is the default\n");
  fprintf(stderr,"\t   -R\tRESERVE fragment\n");
  fprintf(stderr,"\t   -X\tXREG fragment\n");
  fprintf(stderr,"\t   -V\tdynamic VARiables fragment\n");
  fprintf(stderr,"\t   -c\tCODE fragment\n");
  fprintf(stderr,"\t   -b\tBYTE (8-bits) fragment\n");
  fprintf(stderr,"\t   -w\tWORD (16-bits) fragment\n");
  fprintf(stderr,"\t   -l\tLONG (32-bits) fragment\n");
  fprintf(stderr,"\t   -t\tTEXT fragment\n");
  fprintf(stderr,"\t   -k\tKEYWORD fragment\n");
  fprintf(stderr,"\t   -H\tHOLE fragment\n");
  fprintf(stderr,"\t-Z\t\tAdd a CE158 CSAVE header\n");
  fprintf(stderr,"\t-Zname=name\tSet <name> as CSAVE header file name\n");
  fprintf(stderr,"\t-Zentry=entry\tSet <entry> as CSAVE header startup routine\n");
  fprintf(stderr,"\t-Zheader=type\tSet <type> for CSAVE header magic\n");
  fprintf(stderr,"\t\twith <type>: CSAVE CSAVEM CSAVEr or PRINT\n");
  fprintf(stderr,"note:\n");
  fprintf(stderr,"\tIf srcfile has a .bas extension, the BASIC mode is assumed.\n");
  fprintf(stderr,"\tIf srcfile has a .asm, .as or .s extension, the CODE mode is assumed.\n");
  fprintf(stderr,"\tIf srcfile has a .hex or .x extension, the BYTE mode is assumed.\n");
  fprintf(stderr,"\tIf the -o outfile argument is not specified, <srcfile>.bin is used for output\n");
  fprintf(stderr,"\n");
  exit(1);
}

int lhAsmDeclareMachine( char *name, int sym, unsigned int *org )
{
  int imach;
  struct s_lhasmmachine *pmachine = NULL;
  char symna[80];

  for( imach = 0; imach < (sizeof( asmmachine ) / sizeof( asmmachine[0] )); imach++ )
    {
    if( asmmachine[ imach ].name && 0 == strcasecmp( name, asmmachine[ imach ].name ) )
      {
      pmachine = &asmmachine[ imach ];
      break;
      }
    }
  if( NULL == pmachine )
    return 0;
  if( 1 == asmpass )
    {
    if( asmpmachine && pmachine->section )
      LHASM_FTL( "Machine '%s' is already defined\n", asmpmachine->name );
    else if( asmpmodule && !pmachine->section )
      LHASM_FTL( "Module '%s' is already inserted\n", asmpmodule->name );
    }
  if( NULL == asmpmachine )
    {
    memcpy( (asmpmachine = &asmhwmachine),
            pmachine->section ? pmachine : &asmmachine[ LHASM_MACHINE_DEFAULT ],
            sizeof(asmhwmachine) );
    memcpy( asmmemsection, asmpmachine->section, sizeof(asmmemsection) );
    asmpmachine->section = asmmemsection;
    if( logfile )
      {
      fprintf( logfd, "%5u\t\t.%s\n", asmpctx->srclinenum, asmpmachine->name );
      }
    if( asmpmachine->module )
      for( imach = 0; imach < (sizeof( asmmachine ) / sizeof( asmmachine[0] )); imach++ )
        if( asmpmachine->module == asmmachine[ imach ].value )
          {
          asmpmodule = &asmmachine[ imach ];
          break;
          }
    for( imach = 0; asmpmachine->section[imach].type != LHASM_MEM_NONE; imach++ )
      if( asmpmachine->section[imach].type == LHASM_MEM_RAM )
        {
        *org = (asmpmachine->section[imach].base & 0xFF00) | 0xC5;
        break;
        }
    }
  if( !pmachine->section )
    {
    if( asmpmachine->module )
      LHASM_FTL( "No module to insert in machine '%s'\n", asmpmachine->name );
    asmpmodule = pmachine;
    if( logfile )
      {
      fprintf( logfd, "%5u\t\t.%s\n", asmpctx->srclinenum, asmpmodule->name );
      }
    for( imach = 0; asmpmachine->section[imach].type != LHASM_MEM_NONE; imach++ )
      if( asmpmachine->section[imach].type == LHASM_MEM_RAM )
        {
        if( (asmpmachine->section[imach].base - asmpmodule->before) < 0 ||
            (asmpmachine->section[imach].end + asmpmodule->after) > 0x8000 )
          LHASM_FTL( "Module '%s' is not compatible with machine '%s'\n",
                     asmpmodule->name, asmpmachine->name );
        asmpmachine->section[imach].base -= asmpmodule->before;
        asmpmachine->section[imach].end += asmpmodule->after;
        *org = (asmpmachine->section[imach].base & 0xFF00) | 0xC5;
        break;
      }
    }

  return 1;
}

void lhAsmSetOrigin( LHU16 org, char *str, LHU16 off )
{
  if( (asmorg == 0x0FFFFF || asmtop == 0x7FFFFFFF) && frgfile )
    {
    fclose( frgfd );
    unlink( frgfile );
    if( NULL == (frgfd = fopen( frgfile, "w" )) )
      {
      fprintf( stderr, "Unable to open frgfile %s\n", frgfile );
      exit( 1 );
      }
    fprintf( frgfd,  LHFRAGMENT_MAGIC "\t%04hX\n", org );
    if( fragment )
      fprintf( frgfd, "\t%s\t%04hX\n", fragment, org );
    }
  if( logfile )
    {
    fprintf( logfd, "%5u\n\t\t%s\t%04X\n", asmpctx->srclinenum, str, org );
    }
  if( asmtop == 0x7FFFFFFF || asmorg == 0x0FFFFF )
    asmpctx->start = asmpctx->end = org;
  asmorg = asmaddr = org;
  if( asmtop > asmorg )
    asmtop = asmorg;
  if( asmbottom < lhAsmGetPC() )
    asmbottom = lhAsmGetPC();
  asmcurrent = &asmmem[asmorg + off];
}

void lhAsmNewFragment( LHU8 fragment, PLHU8 f, PLHU8 comment, PLHU8 text )
{
  if( frgfile && asmorg != 0x0FFFFF )
    {
    fprintf( frgfd, "\t%s\t%04X\n", f, lhAsmGetPC() );
    if( comment )
      fprintf( frgfd, "\tCOMMENT\t%s\n", comment );
    }
  if( logfile && asmorg != 0x0FFFFF )
    {
    fprintf( logfd, "%5u\t\t.%s\t%04X", asmpctx->srclinenum, f, lhAsmGetPC() );
    if( text )
      fprintf( logfd, "\t; %s", text );
    fprintf( logfd, "\n" );
    }
}

int lhAsmGetPC( void )
{
  return asmaddr + (asmpinlin ? (LHU16)(asmpinlin->pcurrent - asmpinlin->plinebase) : 0);
}

void lhAsmFollowStruct( struct s_lhasmstruct *ps, PLHU8 comment, PLHU8 ptext )
{
  int ifi;
  struct s_lhasmfield *pf;
  char text[256];

  for( pf = ps->fields, ifi = 0; ifi < ps->nnfield; ifi++, pf++ )
    {
    if( pf->fragment != FRAG_STRUCT )
      {
      snprintf( text, sizeof(text), "%s%s\t{%s * #%u}", ptext, pf->name,
                ( FRAG_BYTE == pf->fragment ?
                    "byte" : (FRAG_WORD == pf->fragment ?
                                "word" : (FRAG_LONG == pf->fragment ?
                                            "long" : (FRAG_TEXT == pf->fragment ?
                                                        "text" : "????" )))),
                pf->array );
      lhAsmNewFragment( pf->fragment, lhFragmentGetFragment( pf->fragment ), comment, text );
      asmaddr += pf->size;
      }
    else
      {
      snprintf( text, sizeof(text), "%s%s\t{struct'%s * #%u}.",
                ptext, pf->name, pf->pstruct->name, pf->array );
      lhAsmFollowStruct( pf->pstruct, NULL, text );
      asmaddr += (pf->pstruct->size * (pf->array - 1));
      }
    comment = NULL;
    }
}

void lhAsmBuildData( struct s_lhasmdata *pdata )
{
  int iar;
  LHU16 saveaddr = asmaddr;
  LHU8 savestate = asmpctx->state;
  char *savefrag = fragment;
  LHU8 comment[256];
  LHU8 text[256];

  for( iar = 0; iar < pdata->array; iar++ ) {
    snprintf( comment, sizeof( comment ), "%s STRUCT %s ELEMENT %d",
              pdata->name, pdata->pstruct->name, iar );
    snprintf( text, sizeof( text ), "%s:#%d.",
              pdata->name, iar );
    lhAsmFollowStruct( pdata->pstruct, comment, text );
  }
  lhAsmNewFragment( savestate, savefrag, NULL, NULL );
  asmaddr = saveaddr;
}

LHU16 lhAsmGetSymValue( PLHSYM sym, PLHU8 symna )
{
  LHU16 v;
  int l;

  if( asmpctx->plocal )
    {
    char locsymna[80];

    snprintf( locsymna, sizeof(locsymna), "%03hu$$%s", asmpctx->included, symna );
    if( 0xFFFF != (v = lhSymGetValue( *asmpctx->plocal, locsymna )) )
      return v;
    }
  if( 0xFFFF != (v = lhSymGetValue( asmincsym, symna )) )
    return v;
  if( 0xFFFF != (v = lhSymGetValue( asmkywsym, symna )) )
    return v;
  if( 0 == strncmp( symna, "__THIS__$$._", 12 ) )
    {
    char locsymna[80];

    snprintf( locsymna, sizeof(locsymna), "%s%s", asmpctx->srcfile, symna + 8 );
    if( 0xFFFF != (v = lhSymGetValue( asmincsym, locsymna )) )
      return v;
    }
  if( 0 == strncmp( symna, "__MAIN__$$._", 12 ) )
    {
    char locsymna[80];

    snprintf( locsymna, sizeof(locsymna), "%s%s", asmctx[0].srcfile, symna + 8 );
    if( 0xFFFF != (v = lhSymGetValue( asmincsym, locsymna )) )
      return v;
    }
  if( 0 == strncmp( symna, "__VERSION__$$._", 15 ) )
    {
    symna += 15;
    if( 0 == strcmp( symna, "release" ) )
      return (asmvers >> 24) & 0xFF;
    else if( 0 == strcmp( symna, "major" ) )
      return (asmvers >> 16) & 0xFF;
    else if( 0 == strcmp( symna, "minor" ) )
      return (asmvers >> 8) & 0xFF;
    else if( 0 == strcmp( symna, "patch" ) )
      return (asmvers) & 0xFF;
    symna -= 15;
    }
  if( 0 == strncmp( symna, "__DATE__$$._", 12 ) )
    {
    symna += 12;
    if( 0 == strcmp( symna, "year" ) )
      return asmtm.tm_year + 1900;
    else if( 0 == strcmp( symna, "yy" ) )
      return (asmtm.tm_year % 100);
    else if( 0 == strcmp( symna, "month" ) )
      return asmtm.tm_mon + 1;
    else if( 0 == strcmp( symna, "day" ) )
      return asmtm.tm_mday;
    else if( 0 == strcmp( symna, "ymd" ) )
      return (((asmtm.tm_year + 1900) % 100) << 9) | ((asmtm.tm_mon + 1) << 5) | asmtm.tm_mday;
    symna -= 12;
    }
  if( 0 == strncmp( symna, "__TIME__$$._", 12 ) )
    {
    symna += 12;
    if( 0 == strcmp( symna, "hour" ) )
      return asmtm.tm_hour;
    else if( 0 == strcmp( symna, "minute" ) )
      return asmtm.tm_min;
    else if( 0 == strcmp( symna, "second" ) )
      return asmtm.tm_sec;
    else if( 0 == strcmp( symna, "hm" ) )
      return ((asmtm.tm_hour) << 6) | asmtm.tm_min;
    symna -= 12;
    }
  if( (l = strlen( symna )) && (l > 8) && (l < 24) &&
      (0 == strncmp( symna + (l - 8), "\\\\._", 4 )) )
    {
    LHU16 jump;
    LHU8 bits, basic[16];

    v = 0xFFFF;

    memset( basic, 0, sizeof(basic) );
    strncpy( basic, symna, l - 8 );

    v = lhBasicGetCodeBitsAndJump( basic, &bits, &jump );
    if( 0 == strcmp( symna + (l - 4), "code" ) && 0xFFFF != v )
      return v;
    else if( 0 == strcmp( symna + (l - 4), "jump" ) && 0xFFFF != v )
      return jump;
    else if( 0 == strcmp( symna + (l - 4), "bits" ) && 0xFFFF != v )
      return bits;
    }
  return lhSymGetValue( sym, symna );
}

PLHU8 lhAsmGetSymName( PLHSYM sym, LHU16 value )
{
  if( asmpctx->plocal )
    {
    PLHU8 thisname;

    if( (thisname = lhSymGetName( *asmpctx->plocal, value )) )
      return thisname;
    }
  return lhSymGetName( sym, value );
}

PLHSYM lhAsmAddSymSorted( PLHSYM sym, PLHU8 symna, LHU16 value, LHU16 magic, LHU8 exported )
{
  if( !exported && asmpctx->plocal )
    {
    char locsymna[80];

    snprintf( locsymna, sizeof(locsymna), "%03hu$$%s", asmpctx->included, symna );
    *asmpctx->plocal = lhSymAddSorted( *asmpctx->plocal, locsymna, value, magic );
    return asmsym;
    }
  return lhSymAddSorted( asmsym, symna, value, magic );
}

void lhAsmRegisterKeywordCB( PLHU8 basic, LHU16 code, LHU16 jump, LHU8 bits )
{
  char kywsymna[28];
  LHU16 v;

  if( logfile )
    fprintf( logfd, "\n%5u\t%04hX\t.DEFINE:\t\"%s\"\t= %04hX\t%c\n",
             asmpctx->srclinenum, lhAsmGetPC(), basic, code, bits );

  snprintf( kywsymna, sizeof(kywsymna), "%s\\\\._code", basic );
  if( asmpass == 1 )
    asmkywsym = lhSymAddSorted( asmkywsym, kywsymna, code, SYM_NONE );
  else
    {
    if( code != lhSymGetValue( asmkywsym, kywsymna ) )
      LHASM_FTL( "Assembler confused. CODE of '%s' differs between passes\n", basic );
    }
  if( logfile )
    fprintf( logfd, "\t\t;; %04X\t%s\n", code, kywsymna );

  snprintf( kywsymna, sizeof(kywsymna), "%s\\\\._jump", basic );
  v = lhAsmGetPC();
  if( asmpass == 1 )
    asmkywsym = lhSymAddSorted( asmkywsym, kywsymna, v, SYM_NONE );
  else
    {
    if( v != lhSymGetValue( asmkywsym, kywsymna ) )
      LHASM_FTL( "Assembler confused. JUMP of '%s' differs between passes\n", basic );
    }
  if( logfile )
    fprintf( logfd, "\t\t;; %04X\t%s\n", v, kywsymna );

  snprintf( kywsymna, sizeof(kywsymna), "%s\\\\._bits", basic );
  v = (bits << 8) | lhBasicConvertBits( bits );
  if( asmpass == 1 )
    asmkywsym = lhSymAddSorted( asmkywsym, kywsymna, v, SYM_NONE );
  else
    {
    if( v != lhSymGetValue( asmkywsym, kywsymna ) )
      LHASM_FTL( "Assembler confused. BITS of '%s' differs between passes\n", basic );
    }
  if( logfile )
    fprintf( logfd, "\t\t;; %04X\t%s\n", v, kywsymna );
}

PLHCNDS lhAsmFetchCnd( char *cnd )
{
  PLHCNDS plhc = lhcnds;
  char *pc = cnd;

  if( cnd && *cnd == '#' && 2 < strlen(cnd) )
    pc += 2;

  if( asmrunmacro && (pc && 0 == strncmp( pc, "__#", 3 )) )
    {
    LHU16 vs;
    if( 1 == sscanf( pc + 3, "%hu", &vs ) )
      if( vs < asmrunmacro->nnarg )
        pc = asmrunmacro->arg[vs];
    }

  for( ; plhc->cnds ; plhc++ )
    {
    if( strcasecmp( pc, plhc->cnds ) == 0 )
      {
      if( pc != cnd )
        {
        switch( cnd[1] )
          {
          case '!':
               return lhAsmFetchCnd( plhc->neg );
          }
        return NULL;
        }
      if( !(plhc->code & 0xF0) )
        return plhc;
      }
    }
  return NULL;
}

PLHREGS lhAsmFetchReg( char *reg )
{
  PLHREGS plhr = lhregs;
  char *pr = reg;

  if( reg && *reg == '#' && 2 < strlen(reg) )
    pr += 2;

  if( asmrunmacro && (pr && 0 == strncmp( pr, "__#", 3 )) )
    {
    LHU16 vs;
    if( 1 == sscanf( pr + 3, "%hu", &vs ) )
      if( vs < asmrunmacro->nnarg )
        pr = asmrunmacro->arg[vs];
    }

  for( ; plhr->regs ; plhr++ )
    {
    if( strcasecmp( pr, plhr->regs ) == 0 )
      {
      if( pr != reg )
        {
        switch( reg[1] )
          {
          case '<':
               return lhAsmFetchReg( plhr->regH );
          case '>':
               return lhAsmFetchReg( plhr->regL );
          case '^':
               return lhAsmFetchReg( plhr->regHL );
          case '*':
               return lhAsmFetchReg( plhr->regiHL );
          }
        return NULL;
        }
      return plhr;
      }
    }
  return NULL;
}

struct s_lhasmfield *lhAsmFetchField( struct s_lhasmstruct *ps,
                                      char *str, LHU16 *infield, LHU16 *offset )
{
  int ln, ifield, ioff = 0;
  char *f, *fn, *p;
  char stname[25], finame[25];
  struct s_lhasmfield *pf;
  LHU16 fa = 0;

  *infield = 0xFFFF;
  memset( finame, 0, sizeof(finame) );
  if( (f = str) )
    {
    fn = strchr( f + 1, '.' );
    strncpy( finame, f + 1, fn ? fn - f - 1: strlen(f + 1) );
    }
  if( (p = strchr( finame, ':' )) )
    {
    *(p++) = 0;
    if( !(lhAsmGetLHUValue( p, p, &fa, LINK_MEM_16, 0, 0 )) )
      {
      LHASM_ERR( "Badly formed word expression '%s'\n", p );
      return NULL;
      }
    }
  

redofield:
  for( pf = NULL, ifield = 0; f && ifield < ps->nnfield; ifield++ )
    {
    if( (0 == strcmp( finame, ps->fields[ifield].name )) && (pf = &(ps->fields[ifield])) )
      {
      if( 0xFFFF == *infield )
        *infield = ifield;
      if( (fa >= pf->array) )
        {
        LHASM_ERR( "Element %hu is outside array %hu for field %s in STRUCT %s\n",
                   fa, pf->array, pf->name, ps->name );
        return 0;
        }
      ioff += pf->offset + (fa * pf->element);
      fa = 0;
      if( pf->pstruct && fn )
        {
        ps = pf->pstruct;
        f = strchr( fn, '.' );
        fn = NULL;
        memset( finame, 0, sizeof(finame) );
        if( f )
          {
          fn = strchr( f + 1, '.' );
          strncpy( finame, f + 1, fn ? fn - (f + 1) : strlen(f + 1) );
          if( (p = strchr( finame, ':' )) )
            {
            *(p++) = 0;
            if( !(lhAsmGetLHUValue( p, p, &fa, LINK_MEM_16, 0, 0 )) )
              {
              LHASM_ERR( "Badly formed word expression '%s'\n", p );
              return NULL;
              }
            }
          goto redofield;
          }
        }
      else
        break;
      }
    }

  *offset = ioff;
  return pf;
}

struct s_lhasmstruct *lhAsmFetchStruct( char *str, char **field, struct s_lhasmdata **pdata, LHU16 *atbase )
{
  int l, istruct, idata;
  char *f, *p;
  char stname[25];
  struct s_lhasmstruct *ps;
  struct s_lhasmdata *pd;
  LHU16 a = 0;

  *field = f = strchr( str, '.' );
  l = strlen( str );
  memset( stname, 0, sizeof(stname) );
  if( f )
    {
    l = f - str;
    }
  strncpy( stname, str, l );
  if( (p = strchr( stname, ':' )) )
    {
    *(p++) = 0;
    if( !(lhAsmGetLHUValue( p, p, &a, LINK_MEM_16, 0, 0 )) )
      {
      LHASM_ERR( "Badly formed word expression '%s'\n", p );
      return NULL;
      }
    }
  
  for( pd = NULL, idata = 0; idata < asmnndata; idata++ )
    {
    if( (0 == strcmp( stname, asmdata[idata].name )) && (pd = &asmdata[idata]) )
      {
      if( a >= pd->array )
        {
        LHASM_ERR( "Element %hu is outside array %hu for DATA %s\n", a, pd->array, pd->name );
        return NULL;
        }
      *atbase = pd->base + (a * pd->element);
      break;
      }
    }
  *pdata = pd;

  if( pd )
    return pd->pstruct;

  *field = f = strchr( str, '.' );
  l = strlen( str );
  memset( stname, 0, sizeof(stname) );
  if( f )
    {
    l = f - str;
    }
  strncpy( stname, str, l );

  for( ps = NULL, istruct = 0; istruct < asmnnstruct; istruct++ )
    {
    if( (0 == strcmp( stname, asmstruct[istruct].name )) && (ps = &asmstruct[istruct]) )
      {
      break;
      }
    }
  return ps;
}

int lhAsmGetStructField( char *str, LHU16 *v )
{
  int of = 0, l, ln, istruct, ifield, ioff;
  char *s, *f, *fn;
  char stname[25], finame[25];
  struct s_lhasmdata *pd = NULL;
  struct s_lhasmstruct *ps = NULL;
  struct s_lhasmfield *pf = NULL;
  PLHREGS plhr;
  LHU16 e = 0xFFFF, fa = 0;

  if( 0 == strncasecmp( "sizeof'", str, l = 7 ) && (of = 1) )
    ;
  else if( 0 == strncasecmp( "offsetof'", str, l = 9 ) && (of = 2) )
    ;
  else if( 0 == strncasecmp( "typeof'", str, l = 7 ) && (of = 3) )
    ;
  else if( 0 == strncasecmp( "arrayof'", str, l = 8 ) && (of = 4) )
    ;
  else if( 0 == strncasecmp( "elementof'", str, l = 10 ) && (of = 5) )
    ;
  else if( 0 == strncasecmp( "addressof'", str, l = 10 ) && (of = 6) )
    ;
  else
    return 0;

  s = str + l;
  if( (plhr = lhAsmFetchReg( s )) )
    {
    if( !(ps = asmbindR[GET_LHREGS(plhr->code)].pstruct) )
      LHASM_ERR( "Register %s is not bound\n", s );
    pf = asmbindR[GET_LHREGS(plhr->code)].pfield;
    f = (pf ? pf->name : NULL);
    ioff = asmbindR[GET_LHREGS(plhr->code)].offset;
    goto dogetof;
    }

  fn = NULL;
  f = strchr( s, '.' );
  l = strlen( s );
  memset( stname, 0, sizeof(stname) );
  memset( finame, 0, sizeof(finame) );
  if( f )
    {
    char *p;
    fn = strchr( f + 1, '.' );
    strncpy( finame, f + 1, fn ? fn - f - 1: strlen(f + 1) );
    l = f - s;
    if( (p = strchr( finame, ':' )) )
    {
      *p = 0;
      if( !(lhAsmGetLHUValue( p + 1, p + 1, &fa, LINK_MEM_16, 0, 0 )) )
        {
        LHASM_ERR( "Badly formed word expression '%s'\n", (p + 1) );
        return 0;
        }
      }
    }
  strncpy( stname, s, l );
  {
  int idata;
  char daname[25];
  char *p;

  if( (p = strchr( stname, ':' )) )
    {
    memset( daname, 0, sizeof(daname) );
    strncpy( daname, stname, p - stname );
    if( !(lhAsmGetLHUValue( p + 1, p + 1, &e, LINK_MEM_16, 0, 0 )) )
      {
      LHASM_ERR( "Badly formed word expression '%s'\n", (p + 1) );
      return 0;
      }
    }
  else
    strcpy( daname, stname );
  for( pd = NULL, idata = 0; idata < asmnndata; idata++ )
    if( (0 == strcmp( daname, asmdata[idata].name )) )
      {
      strcpy( stname, (pd = &asmdata[idata])->pstruct->name );
      if( (0xFFFF != e) && (e >= pd->array) )
        {
        LHASM_ERR( "Element %hu is outside array %hu for DATA %s\n", e, pd->array, pd->name );
        return 0;
        }
      break;
      }
  }
  ioff = 0;
  for( ps = NULL, istruct = 0; istruct < asmnnstruct; istruct++ )
    {
    if( (0 == strcmp( stname, asmstruct[istruct].name )) && (ps = &asmstruct[istruct]) )
      {
redofield:
      for( pf = NULL, ifield = 0; f && ifield < ps->nnfield; ifield++ )
        {
        if( (0 == strcmp( finame, ps->fields[ifield].name )) && (pf = &ps->fields[ifield]) )
          {
          if( (fa >= pf->array) )
            {
            LHASM_ERR( "Element %hu is outside array %hu for field %s in DATA %s\n",
                       fa, pf->array, pf->name, pd->name );
            return 0;
            }
          ioff += pf->offset + (fa * pf->element);
          fa = 0;
          if( pf->pstruct && fn )
            {
            ps = pf->pstruct;
            f = strchr( fn, '.' );
            fn = NULL;
            memset( finame, 0, sizeof(finame) );
            if( f )
              {
              char *p;

              fn = strchr( f + 1, '.' );
              strncpy( finame, f + 1, fn ? fn - (f + 1) : strlen(f + 1) );
              if( (p = strchr( finame, ':' )) )
                {
                *p = 0;
                if( !(lhAsmGetLHUValue( p + 1, p + 1, &fa, LINK_MEM_16, 0, 0 )) )
                  {
                  LHASM_ERR( "Badly formed word expression '%s'\n", (p + 1) );
                  return 0;
                  }
                }
              goto redofield;
              }
            }
          else
            break;
          }
        }
      break;
      }
    }
  if( (istruct == asmnnstruct) || (f && ifield == ps->nnfield) )
    LHASM_FTL( "Unknown STRUCT or field %s\n", s );

dogetof:
  switch( of )
    {
    case 1: if( f )
              *v = pf->size;
            else if( pd && (0xFFFF == e) )
              *v = pd->size;
            else
              *v = ps->size;
            break;
    case 2: if( f || (0xFFFF != e) )
              {
              e = (0xFFFF == e ? 0 : e);
              if( pd )
                *v = (pd->element * e) + ioff;
              else
                *v = ioff; //ps->fields[ifield].offset;
              }
            else
              *v = 0;
            break;
    case 3: if( f )
              *v = pf->type;
            else
              *v = 1;
            break;
    case 4: if( f )
              *v = pf->array;
            else if( pd && (0xFFFF == e) )
              *v = pd->array;
            else
              *v = 1;
            break;
    case 5: if( f )
              *v = pf->element;
            else if( pd && (0xFFFF == e) )
              *v = pd->element;
            else
              *v = ps->size;
            break;
    case 6: if( pd )
              {
              e = (0xFFFF == e ? 0 : e);
              if( f )
                *v = pd->base + (pd->element * e) + ioff;
              else
                *v = pd->base + (pd->element * e);
              }
            else
              LHASM_FTL( "Unknown DATA %s\n", s );
            break;
    }
#if 0
  if( logfile )
    {
    fprintf( logfd, "%5u\t%04X\t\t\t;\n", asmpctx->srclinenum, lhAsmGetPC() );
    }
#endif
  return 1;
}

int lhAsmBCD( LHU16 hexval, LHU16 *val, int link )
{
  char bcdval[5];

  if( (LINK_MEM_8 == link && 100 > hexval) || (LINK_MEM_16 == link && 10000 > hexval) )
    {
    snprintf( bcdval, sizeof(bcdval), "%hu", hexval );
    return sscanf( bcdval, "%hx", val );
    }
  else
    return 0;
}

int lhAsmGetLHUValue( char *sym, char *str, LHU16 *v, int link, LHU16 defval, LHU16 off )
{
  char *fmt[7], *p2, *pt, pc = 0, *p3 = NULL;
  int   l;
  LHU16 max = 0xFFFF, vs, vv, rel = 0xFFFF, opval = 0, is = 0;
  char symna[80];

  if( !str || (str && *str == ';') )
    return 0;

  if( asmrunmacro && (str && 0 == strncmp( str, "__#", 3 )) )
    {
    if( 1 == sscanf( str + 3, "%hu", &vs ) )
      if( vs < asmrunmacro->nnarg )
        return lhAsmGetLHUValue( sym, asmrunmacro->arg[vs], v, link, defval, off );
    return 0;
    }

  if( asmrunmacro && (str && isdigit(str[0]) && str[1] == ':') )
    {
    LHU8 nc = str[0] - '0';
    *v = (asmlblmask[asmnnrun] & (1 << nc) ? asmrunlbl[(asmnnrun * 10) + nc] : defval);
    return 1;
    }

  if( lhAsmGetStructField( str, v ) )
    {
    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
    }

  if( 0 == strncasecmp( "CODEOF'\"", str, 8 ) &&
      '"' == str[strlen( str ) - 1] )
    {
    LHU16 sv;
    char basic[16];
    
    memset( basic, 0, sizeof(basic) );
    strncpy( basic, str + 8, strlen( str + 8) - 1);
    if( (sv = lhBasicGetCode( basic )) )
      {
      if( LINK_MEM_16 == link || LINK_iMEM_16 == link )
        *v = sv;
      else
        *v = sv & 0x00FF;
      return 1;
      }
    }
  else if( 0 == strncasecmp( "BCDOF'", str, 6 ) )
    {
    LHU16 sv;

    if( lhAsmGetLHUValue( sym, str + 6, &sv, link, defval, off ) )
      {
      if( LINK_MEM_16 == link || LINK_iMEM_16 == link )
        return lhAsmBCD( sv, v, LINK_MEM_16 );
      else
        return lhAsmBCD( sv, v, LINK_MEM_8 );
      }
    }
  else if( 0 == strncasecmp( "BCD16OF'", str, 8 ) )
    {
    LHU16 sv;

    if( lhAsmGetLHUValue( sym, str + 8, &sv, link, defval, off ) )
      {
      return lhAsmBCD( sv, v, LINK_MEM_16 );
      }
    }
  else if( 0 == strncasecmp( "BCD8OF'", str, 7 ) )
    {
    LHU16 sv;

    if( lhAsmGetLHUValue( sym, str + 7, &sv, link, defval, off ) )
      {
      return lhAsmBCD( sv, v, LINK_MEM_8 );
      }
    }
  else if( 0 == strncasecmp( "flags.'", str, 6 ) )
    {
    char *flags = "CIZVH";
    char *pf = strchr( flags, toupper( str[6] ) );

    if( !pf )
      {
      LHASM_ERR( "Unknown status flags %s\n", str[6] );
      return 0;
      }
    *v = ( 1 << ( pf - flags ) );
    return 1;
    }

  if( 0 == strncasecmp( "OPCODE'", str, 7 ) )
    {
    char opstr[512], *inst, *argv[3];
    PLHINST plhi = lhinst;
    char *plink;
    LHU16 opmatch = 0xFFFF;
    LHU8 ispage = 0, code, page = 0;
    int l, loop;
    PLHREGS plhr;
    PLHCNDS plhc;

    str += 7;
    if( asmrunmacro && ( 0 == strncmp( str, "__#", 3 )) )
      {
      if( 1 == sscanf( str + 3, "%hu", &vs ) )
        if( vs >= asmrunmacro->nnarg )
          return 0;
      str = asmrunmacro->arg[vs];
      }

      inst = strtok( strcpy( opstr, str ), ":" );
      if( !inst )
        return 0;
      argv[0] = strtok( NULL, ":" );
      argv[1] = strtok( NULL, ":" );
      argv[2] = strtok( NULL, ":" );

    l = strlen(inst);
    if( inst[l-1] == '#' )
      {
      ispage = SECOND_PAGE;
      l--;
      }
    for( ; plhi->inst ; plhi++ )
      {
      page = 0;
      plink = &(plhi->inst[l]);
      if( !(plink[0] == '%' || plink[0] == '!' || plink[0] == 0) )
        continue;
      if( strncasecmp( inst, plhi->inst, l ) == 0 )
        {
        if( plink[0] == 0 )
          {
          code = plhi->code;
          goto out;
          }
        else if( plink[0] == '!' && plink[1] == 0 )
          {
          page = SECOND_PAGE;
          code = plhi->code;
          goto out;
          }
        else if( plink[0] == '!' && plink[1] == '#' )
          {
          page = ispage;
          code = plhi->code;
          goto out;
          }
        else if( plink[0] == '!' && plink[1] != '%' )
          {
          page = SECOND_PAGE;
          plink ++;
          }
        else if( plink[0] == '!' && plink[1] == '%' )
          {
          page = ispage;
          plink += 2;
          }
        else
          plink++;
        code = plhi->code;
        loop = 0;
        if( 0xFFFF == opmatch )
          opmatch = (page << 8) | code;
        else if( 0xFFFE != opmatch )
          opmatch = 0xFFFE;

opcodereplay:
        if( argv[loop] )
          {
          if( (*plink == LINK_iREG_HL && (*argv[loop] != '(' && *argv[loop] != '#') ))
            continue;
          else if( (*plink == LINK_iMEM_8 || *plink == LINK_iMEM_16 || *plink == LINK_SBR) &&
              *argv[loop] !='(' )
            continue;
          switch( *plink )
            {
            case LINK_NONE:
                 goto out;
            case LINK_MEM_8:
                 if( 0 == strcmp( argv[loop], "&n" ) || 0 == strcmp( argv[loop], "_" ) )
                   {
                   goto nextplay;
                   }
                 continue;
            case LINK_MEM_16:
                 if( 0 == strcmp( argv[loop], "mm" ) || 0 == strcmp( argv[loop], "&mn" ) ||
                     0 == strcmp( argv[loop], "__" ) )
                   {
                   goto nextplay;
                   }
                 continue;
            case LINK_iMEM_8:
            case LINK_SBR:
                 if( 0 == strcmp( argv[loop], "(&n)" ) || 0 == strcmp( argv[loop], "(_)" ) )
                   {
                   goto nextplay;
                   }
                 continue;
            case LINK_iMEM_16:
                 if( 0 == strcmp( argv[loop], "(mm)" ) || 0 == strcmp( argv[loop], "(&mn)" ) ||
                     0 == strcmp( argv[loop], "(__)" ) )
                   {
                   goto nextplay;
                   }
                 continue;
            case LINK_REG_L:
                 if( 0 == strcmp( argv[loop], "rl" ) )
                   {
                   goto nextplay;
                   }
                 goto opregister;
            case LINK_REG_H:
                 if( 0 == strcmp( argv[loop], "rh" ) )
                   {
                   goto nextplay;
                   }
                 goto opregister;
            case LINK_REG_HL:
                 if( 0 == strcmp( argv[loop], "R" ) )
                   {
                   goto nextplay;
                   }
                 goto opregister;
            case LINK_iREG_HL:
                 if( 0 == strcmp( argv[loop], "(R)" ) )
                   {
                   goto nextplay;
                   }
opregister:
                 {
                 LHU16 br;

                 plhr = lhAsmFetchReg( argv[loop] );
                 if( !plhr || (plhr->link != *plink && (*plink != LINK_REG_HL || plhr->link != LINK_REG_BC)) )
                   continue;
                 code |= (plhr->code & plhi->amsk);
                 goto nextplay;
                 }
                 continue;
            case LINK_REG_A:
            case LINK_REG_F:
            case LINK_REG_T0:
            case LINK_REG_T1:
                 plhr = lhAsmFetchReg( argv[loop] );
                 if( !plhr || plhr->link != *plink )
                   continue;
                 goto nextplay;
            case LINK_REG_BC:
            case LINK_REG_SP:
            case LINK_REG_PC:
                 plhr = lhAsmFetchReg( argv[loop] );
                 if( !plhr || plhr->link != *plink )
                   continue;
                 goto nextplay;
            case LINK_JREL:
                 if( 0 == strcmp( argv[loop], "+d" ) )
                   {
                   goto nextplay;
                   }
                 else if( 0 == strcmp( argv[loop], "-d" ) )
                   {
                   code |= 0x10;
                   goto nextplay;
                   }
                 continue;
            case LINK_CNDS:
                 if( 0 == strcmp( argv[loop], "cc" ) )
                   {
                   goto nextplay;
                   }
                 plhc = lhAsmFetchCnd( argv[loop] );
                 if( !plhc )
                   continue;
                 if( (code & 0xC0) == 0xC0 && NOVERFLOW == plhc->code )
                   continue;
                 code |= plhc->code;
                 goto nextplay;
                 continue;
            case LINK_CD34:
            case LINK_KYWD:
                 continue;
            }
          }
        }
      }
    if( (NULL == argv[0]) && (0xFFFE > opmatch) )
      {
      page = (opmatch >> 8) & 0xFF;
      code = opmatch & 0xFF;
      goto out;
      }
    LHASM_ERR( "%s instruction '%s'!\n",
               (NULL == argv[0] && 0xFFFE == opmatch ? "Ambigous" : "Unknown"), inst );
    return 0;

nextplay:
    loop++;
    if( plink[1] == '%' )
      {
      plink += 2;
      goto opcodereplay;
      }

out:
    *v = (page << 8) | code;
    return 1;
    }

  if( *str != '(' && isdigit(*str) && ('+' == (pc = *(str + 1)) || '-' == pc) )
    {
    if( lhAsmGetLHUValue( sym ? sym + 2 : NULL, str + 2, &vs, link, defval, off ) )
      {
      *v = vs + ('+' == pc ? (*str - '0') : -(*str - '0'));
      return 1;
      }
    else
      return 0;
    }
  else if( *str == '(' && str[strlen(str) - 1] == ')' )
    {
    LHU8 _link;
    _link = (link == LINK_iMEM_16 ? LINK_MEM_16 :
               (link == LINK_iMEM_8 || link == LINK_SBR ? LINK_MEM_8 : link ) );
    memset( symna, 0, sizeof(symna) );
    str = strncpy( symna, str + 1, strlen(str + 1) - 1 );
    if( lhAsmGetLHUValue( sym ? str : NULL, str, &vs, _link, defval, off ) )
      {
      *v = vs;
      return 1;
      }
    else
      return 0;
    }
  else if( ((pt = str) && *str && (p2 = strchr( "+-*%<>!^~/'.{}[", *str )) && (pt = str + 1)) )
    {
    LHU16 _defval = defval;
    int _link = link;
    
    pc = (p2 ? *p2 : 0);
    switch( pc )
      {
      case '+':
      case '-':
           if( (link == LINK_JREL || link == LINK_iMEM_16 || link == LINK_MEM_16) )
             {
             _link = *(p2 + 1) == '(' ? LINK_iMEM_8 : LINK_MEM_8;
             _defval = 0;
             }
           break;
      case '[':
           if( (link == LINK_MEM_8 || link == LINK_MEM_16 || link == LINK_JREL) )
             {
             char *pn;
             char eval[80];

             memset( eval, 0, sizeof(eval) );
             if( (pn = strchr( pt, ']' )) && (p3 = strchr( "=+-*/&|^<>:?", *pt )) &&
                 *(pn + 1) && 
                 (*(pt + 1) == '\'' && strncpy( eval, pt + 2, pn - pt - 2 ) &&
                  (is = lhAsmGetLHUValue( eval, eval, &opval, LINK_MEM_16, 0, 0 ))) ||
                  (1 == sscanf( pt + 1, "%hx]", &opval)) )
               {
               pt = pn + 1;
               _link = LINK_MEM_16;
               }
             if( !is && '?' == *pt )
               pt = pn + 1;
             _defval = LINK_JREL == link ? lhAsmGetPC() : 0;
             }
           break;
      case '*':
           if( (link == LINK_MEM_8 || link == LINK_MEM_16) )
             {
             if( 0 == strncmp( pt, "_*", 2 ) && *(pt + 2) )
               {
               char *pb, *pe;
               if( (pb = strchr( pt, '[' )) && (pe = strchr( pb + 1, ']' )) && !(*(pe + 1)) )
                 {
                 memset( symna, 0, sizeof(symna) );
                 strncpy( symna, pb + 1, (pe - pb - 1));
                 rel = lhAsmGetSymValue( asmsym, symna );
                 if( 0xFFFF == rel )
                   {
                   if( asmpass == 1 && !(asmflags & LHASM_IMMEDIATE) )
                     {
                     rel = lhAsmGetPC();
                     }
                   else
                     return 0;
                   }
                 memset( symna, 0, sizeof(symna) );
                 pt = strncpy( symna, pt + 2, (pb - pt - 2) );
                 }
               else
                 pt += 2;
               off = -off + 1;
               }
             _defval = lhAsmGetPC() + off;
             }
           break;
      case '%':
           {
           LHU8 nv, nc;
           if( str && 4 <= strlen( str ) && 2 == sscanf( str+1, "%hhu%c", &nv, &nc ) &&
             nv < LHASM_MAX_VARS && isalpha(nc) )
             {
             nc = tolower(nc) - 'a';
             if( asmvarmask[nv] & (1 << nc) )
               {
               *v = asmvars[nv][nc];
               return 1;
               }
             LHASM_ERR( "Variable '%%%02hhu%c' is never defined.\n", nv, 'a' + nc );
             }
           return 0;
           }
           break;
      case '.':
           if( '.' == *(str+1) )
             *v = lhAsmGetPC();
           else
             *v = lhAsmGetPC() + off;
           return 1;
      case '<':
      case '>':
      case '{':
      case '}':
           _link = LINK_MEM_16;
      case '^':
           _defval = 0;
           break;
      case '~':
      case '/':
      case '\'':
      case '!':
           break;
      }
    if( lhAsmGetLHUValue( (rel != 0xFFFF ? pt : sym), pt, &vv, _link, _defval, off ) )
      {
      pc = (p2 ? *p2 : 0);
      switch( pc )
        {
        case '+':
             if( (link == LINK_JREL || link == LINK_iMEM_16 || link == LINK_MEM_16) && vv < 0x100 )
               {
               *v = lhAsmGetPC() + vv + off;
               return 1;
               }
             break;
        case '-':
             if( (link == LINK_JREL || link == LINK_iMEM_16 || link == LINK_MEM_16) && vv < 0x100 )
               {
               *v = lhAsmGetPC() - vv + off; // + 1;
               return 1;
               }
             break;
        case '*':
             if( link == LINK_MEM_8 )
               {
               if( rel != 0xFFFF )
                 *v = vv - rel - off;
               else
                 *v = vv - lhAsmGetPC() - off;
               return (*v < 0x100 ? 1 : 0);
               }
             else if( link == LINK_MEM_16 )
               {
               *v = vv - lhAsmGetPC() - off;
               return 1;
               }
             break;
        case '[':
             switch( p3 ? *p3 : 0 )
               {
               case '=':
                    *v = ((vv + ((1 << opval)-1)) & ~((1 << opval) - 1));
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '+':
                    *v = (vv + opval);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '-':
                    *v = (vv - opval);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '*':
                    *v = (vv * opval);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '/':
                    *v = (vv / opval);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '&':
                    *v = (vv & opval);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '|':
                    *v = (vv | opval);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '^':
                    *v = (vv ^ opval);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '<':
                    *v = (vv << opval);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '>':
                    *v = (vv >> opval);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case ':':
                    *v = (opval ? opval : vv);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               case '?':
                    *v = (is ? opval : vv);
                    return (link == LINK_MEM_8 ? (*v < 0x100 ? 1 : 0) : 1);
               }
             return 0;
             break;
        case '%':
             break;
        case '.':
             break;
        case '<':
             *v = (vv & 0xFF00) >> 8;
             return 1;
        case '>':
             *v = vv & 0xFF;
             return 1;
        case '{':
             *v = (vv << 1);
             return 1;
        case '}':
             *v = (vv >> 1);
             return 1;
        case '^':
             if( vv <= 0x10 )
               {
               *v = (vv ? 1 << (vv - 1) : 0);
               return 1;
               }
             break;
        case '\'':
             {
             LHU16 mask = 0x8000, bit = 16;

             while( mask && !(mask & vv) )
               {
               mask >>= 1;
               bit--;
               }
             *v = bit;
             return 1;
             }
             break;
        case '/':
             {
             LHU16 mask = 0x0001, bit = 1;

             while( mask && !(mask & vv) )
               {
               mask <<= 1;
               bit++;
               }
             *v = (vv ? bit : 0);
             return 1;
             }
             break;
        case '!':
             if( (link == LINK_MEM_8 || link == LINK_iMEM_8) )
               {
               *v = (vv ^ 0xFF) & 0xFF;
               return 1;
               }
             else if( (link == LINK_MEM_16 || link == LINK_iMEM_16) )
               {
               *v = (vv ^ 0xFFFF);
               return 1;
               }
             break;
        case '~':
             if( (link == LINK_MEM_8 || link == LINK_iMEM_8) )
               {
               *v = ((vv & 0xF0) >> 4) | ((vv & 0x0F) << 4);
               return 1;
               }
             else if( (link == LINK_MEM_16 || link == LINK_iMEM_16) )
               {
               *v = ((vv & 0xFF00) >> 8) | ((vv & 0x00FF) << 8);
               return 1;
               }
             break;
        }
      }
      return 0;
    }

replay:
  memset( fmt, 0, sizeof(fmt) );
  switch( link )
    {
    case LINK_MEM_8:
    case LINK_CD34:
         if( str && (0 == strncmp( str, "$:", 2 ) && str[2] ) )
           {
           int i = 0;

           for( i = 0; asmchar[i].str; i++ )
             if( 0 == strcasecmp( str + 2, asmchar[i].str ) ||
                 (asmchar[i].alias && 0 == strcasecmp( str + 2, asmchar[i].alias)) )
               {
               *v = asmchar[i].val;
               return 1;
               }
           LHASM_ERR( "Unknown character '%s'\n", str );
           return 0;
           }
         else if( str && (1 == sscanf( str, "$%c", symna ) || 1 == sscanf( str, "\\$%c", symna )) )
           {
           *v = ((LHU16)symna[0]) & 0xFF;
           return 1;
           }
         l = 2;
         fmt[0] = "%hx";
         fmt[1] = "&%hx";
         fmt[2] = "#%hu";
         fmt[3] = "@%ho";
         fmt[4] = "\\x%hx";
         fmt[5] = "\\u%hu";
         fmt[6] = "\\o%ho";
         max = 0xFF;
         break;
    case LINK_MEM_16:
    case LINK_JREL:
         l = 4;
         fmt[0] = "%hx";
         fmt[1] = "&%hx";
         fmt[2] = "#%hu";
         fmt[3] = "@%ho";
         fmt[4] = "\\X%hx";
         fmt[5] = "\\U%hu";
         fmt[6] = "\\O%ho";
         break;
    case LINK_iMEM_8:
    case LINK_SBR:
         l = 4;
         fmt[0] = "(%hx)";
         fmt[1] = "(&%hx)";
         fmt[2] = "(#%hu)";
         fmt[3] = "(@%ho)";
         fmt[4] = "(\\x%hx)";
         fmt[5] = "(\\u%hu)";
         fmt[6] = "(\\o%ho)";
         max = 0xFF;
         break;
    case LINK_iMEM_16:
         l = 6;
         fmt[0] = "(%hx)";
         fmt[1] = "(&%hx)";
         fmt[2] = "(#%hu)";
         fmt[3] = "(@%ho)";
         fmt[4] = "(\\X%hx)";
         fmt[5] = "(\\U%hu)";
         fmt[6] = "(\\O%hO)";
         break;
    default:
         return 0;
    }

  if( *fmt[0] == '(' && (str && *str != '(') )
    return 0;

  if( sym && (0 == strchr( "&#@\\", *str )) )
    {
    if( *((sym = str) + 1) == '%' )
      return( lhAsmGetLHUValue( NULL, str + 1, v, link, 0, off ) );
    if( str[0] == '(' && str[strlen(str) - 1] == ')' )
      {
      memset( symna, 0, sizeof(symna) );
      sym = strncpy( symna, str + 1, strlen(str + 1) - 1 );
      }
    if( isdigit(*sym) && ('+' == *(sym + 1) || '-' == *(sym + 1)) )
      return( lhAsmGetLHUValue( sym, sym, v, (link == LINK_iMEM_16 ? LINK_MEM_16 : LINK_MEM_8), 0, off ) );
    *v = vs = lhAsmGetSymValue( asmsym, sym );
    if( 0xFFFF == vs )
      {
      if( asmpass == 1 && !(asmflags & (LHASM_IMMEDIATE | LHASM_OP_SYM_EXIST)) )
        {
        *v = defval;
        return 1;
        }
      }
    else
      return 1;
    }

  if( !str || !(*str) )
    return 0;

  if( (strlen(str) == l && 1 == sscanf( str, *fmt, v ) && *v <= max ))
    return 1;

  for( l = 1; l < sizeof(fmt)/sizeof(char*) && fmt[l]; l++ )
    {
    if( 1 == sscanf( str, fmt[l], v ) && *v <= max )
      return 1;
    }
  if( link == LINK_MEM_16 )
    {
    link = LINK_MEM_8;
    goto replay;
    }
  return 0;
}

char *lhAsmEatSp( char *str )
{
  while( str && *str && (' ' == *str || '\t' == *str) )
    str++;
  return (str && *str ? str : NULL);
}

int lhAsmParse( char *buf, char **argv, int maxargc, LHU8 basic )
{
  char c;
  char *p, *e;
  int argc = 0;

  memset( argv, 0, sizeof(*argv)*maxargc );
  if( NULL ==( p = argv[argc] = lhAsmEatSp( strtok(buf, "\r\n" ) ) ))
    return 0;

  if( basic )
    {
    if( NULL != (argv[argc] = strtok( p, " \t\n")) )
      {
      p = lhAsmEatSp( argv[argc]+strlen(argv[argc])+1 );
      if( NULL != (argv[++argc] = p ) )
        {
        p = lhAsmEatSp( argv[argc]+strlen(argv[argc])+1 );
        argc++;
       }
      }
    return argc;
    }
  e = p + strlen( p );
  do
    {
    if( p )
      {
      //if( '"' == (*p) || '\'' == (*p) || '`' == (*p) || '(' == (*p) || '[' == (*p) )
      if( '"' == (*p) || '(' == (*p) )
        {
        c = (*p == '(' ? ')' : *p);
        argv[argc++] = p;
        p = strchr( p+1, c );
        if( p && (*(p+1)))
          {
          *(++p) = '\0';
          if( p >= e || NULL == (argv[argc] = strtok( p+1, basic ? " \t\n" : " ,\t\n" )) )
            break;
          }
        else
          {
          argv[argc] = NULL;
          break;
          }
        }
      else
        {
        if( p >= e || NULL == (argv[argc] = strtok( p, basic ? " \t\n" : " ,\t\n" )) )
          break;
        }
      }
    p = lhAsmEatSp( argv[argc]+strlen(argv[argc])+1 );
    }
  while( (++argc < maxargc) && p );
  if( (argc == maxargc) && p )
    LHASM_FTL( "Line is too long for parser\n" );
  return argc;
}

int lhAsmCheckIfCondition( char **argv, int argc, LHU8 *cond )
{
  int eq = 1, lt = 1, gt = 1, negate = 0;

  if( strcasecmp( argv[0], "NOT" ) == 0 && argc > 1 )
    {
    negate = 1;
    argv++;
    argc--;
    }
  if( strcasecmp( argv[0], "EXIST?" ) == 0 && argc > 1 )
    {
    *cond = (0xFFFF != lhAsmGetSymValue( asmsym, argv[1] ) ? 1 : 0);
    goto outCond;
    }
  else if( ((eq = strcasecmp( argv[0], "EQUAL?" )) == 0 ||
            (gt = strcasecmp( argv[0], "GREATER?" )) == 0 ||
            (lt = strcasecmp( argv[0], "LESS?" )) == 0) && argc > 2 )
    {
    LHU16 vL, vR;

    if( lhAsmGetLHUValue( argv[1], argv[1], &vL, LINK_MEM_16, lhAsmGetPC(), 0) &&
        lhAsmGetLHUValue( argv[2], argv[2], &vR, LINK_MEM_16, lhAsmGetPC(), 0) )
      {
      *cond = ((!eq ? vL == vR : (!lt ? vL < vR : vL > vR )) ? 1 : 0);
      goto outCond;
      }
    LHASM_ERR( "Badly formed expression %s %s %s\n", argv[0], argv[1], argv[2] );
    }
  else if( strcasecmp( argv[0], "KEYWORD?" ) == 0 && argc > 1 )
    {
    char keyw[16];
    int l = strlen( argv[1] );

    if( (l < 4) || '"' != *argv[1] || '"' != argv[1][l - 1] )
      LHASM_ERR( "Badly formed keyword %s\n", argv[1] );
    strncpy( keyw, argv[1] + 1, (l - 2) );
    keyw[l - 2] = '\0';
    *cond = !( 0 == lhBasicGetCode( keyw ) );
    goto outCond;
    }
  else if( strcasecmp( argv[0], "INCLUDED?" ) == 0 )
    {
    *cond = (asmpctx->included == 0 ? 0 : 1 );
    goto outCond;
    }
  else if( strcasecmp( argv[0], "MACHINE?" ) == 0 )
    {
    if( argc > 1 )
      *cond = (asmpmachine && 0 == strcasecmp( asmpmachine->name, argv[1] ) ? 1 : 0);
    else
      *cond = (asmpmachine ? 1 : 0);
    goto outCond;
    }
  else if( strcasecmp( argv[0], "MODULE?" ) == 0 )
    {
    if( argc > 1 )
      *cond = (asmpmodule &&
               0 == strcasecmp( asmpmodule->name ? asmpmodule->name : asmpmachine->name , argv[1] ) ? 1 : 0);
    else
      *cond = (asmpmodule ? 1 : 0);
    goto outCond;
    }
  else if( strcasecmp( argv[0], "ORIGIN?" ) == 0 )
    {
    *cond = !( asmtop == 0x7FFFFFFF || asmorg == 0x0FFFFF );
    goto outCond;
    }
  else if( strcasecmp( argv[0], "VERSION?" ) == 0 && argc > 1 )
    {
    char cvers[4];
    unsigned int nvers;

    memset( cvers, 0, sizeof(cvers) );
    sscanf( argv[1], "%hhu.%hhu.%hhup%hhu", &cvers[0], &cvers[1], &cvers[2], &cvers[3] );
    nvers = ((cvers[0] & 0xFF) << 24) | ((cvers[1] & 0xFF) << 16) | \
           ((cvers[2] & 0xFF) << 8) | (cvers[3] & 0xFF);
    *cond = ( asmvers >= nvers ? 1 : 0 );
    goto outCond;
    }
  else if( strcmp( argv[0], "0" ) == 0 || strcmp( argv[0], "1" ) == 0 )
    {
    *cond = ( '1' == *argv[0] );
    goto outCond;
    }
  else if( strcasecmp( argv[0], "SUBSTITUTE?" ) == 0 && argc > 1 )
    {
    struct s_lhasmsubs *psub;

    for( psub = asmsubs; psub ; psub = psub->next )
      if( strcmp( argv[1], psub->name ) == 0 )
        {
        *cond = 1;
        goto outCond;
        }
    *cond = 0;
    goto outCond;
    }
  else if( strcasecmp( argv[0], "STRUCT?" ) == 0 && argc > 1 )
    {
    struct s_lhasmdata *pd;
    struct s_lhasmstruct *ps;
    struct s_lhasmfield *pf;
    char *field;
    LHU16 v;
    LHU16 fieldoff, infield;

    ps = lhAsmFetchStruct( argv[1], &field, &pd, &v );
    if( ps && field )
      pf = lhAsmFetchField( ps, field, &infield, &fieldoff );
    *cond = ( ps ? (field ? (pf ? 1 : 0) : 1) : 0 );
    goto outCond;
    }
  else if( strcasecmp( argv[0], "PASS?" ) == 0 && argc > 1 )
    {
    LHU16 pass;

    if( 1 == sscanf( argv[1], "%hu", &pass ) && (pass == 1 || pass == 2) )
      {
      *cond = (pass == asmpass);
      goto outCond;
      }
    LHASM_ERR( "Expecting 1 or 2 for %s\n", argv[0] );
    }
  else
    LHASM_ERR( "Unknown .IF condition '%s'\n", argv[0] );
  return 0;

outCond:
  if( negate )
    {
    argv--;
    argc++;
    *cond = (*cond ? 0 : 1);
    }
  if( logfile )
    {
    fprintf( logfd, "%5u\t%04X\t%s\t\t.IF\t%s\t%s %s\n", asmpctx->srclinenum, lhAsmGetPC(),
             *cond ? "+TRUE+" : "/false/", argv[0], argc > 1 ? argv[1] : "", argc > 2 ? argv[2] : "" );
    }
  return 1;
}

int lhAsmPushDoStack( LHU8 docode, LHU16 addr, PLHU8 base, PLHU8 arg, PLHU8 code, PLHU8 *ppncode )
{
  PLHCNDS plhc = NULL;
  PLHU8 pncode = *ppncode;

  asmflags |= LHASM_JP_DO_OPTIMIZE;

  if( LHASM_MAX_DO_STACK == asmnndostack )
    LHASM_FTL( "IF/ELSE/ENDIF, BEGIN/WHILE/REPEAT, BEGIN/UNTIL stack exceeded\n" );

  if( arg )
    {
    plhc = lhAsmFetchCnd( arg );
    if( !plhc )
      {
      LHASM_ERR( "Unknown condition '%s'\n", arg );
      return 0;
      }
    }

  if( docode & (LHASM_DO_IF|LHASM_DO_WHILE) )
    *code = 0x81 | plhc->code;
  else if( docode & LHASM_DO_ELSE )
    *code = 0x8E;
  else
    goto nocode;

  if( docode & LHASM_DO_FORCE_JR )
    asmJRJP[asmnnJRJP] = 1;

  if( !asmJRJP[asmnnJRJP] )
    {
    if( *code == 0x8E )
      *code = 0xBA;
    else
      {
      *(pncode++) = 0x03;
      *(pncode++) = 0xBA;
      }
    *(pncode++) = 0x00;
    *(pncode++) = 0x00;
    }
  else
    {
    if( *code != 0x8E )
      *code ^= 2;
    *(pncode++) = 0x00;
    }
  asmnnJRJP++;
  if( asmnnJRJP == LHASM_MAX_JRJP )
    LHASM_FTL( "JR to JP database exceeded\n" );

  *ppncode = pncode;

nocode:
  asmdostack[asmnndostack].srcfile = asmpctx->srcfile;
  asmdostack[asmnndostack].srclinenum = asmpctx->srclinenum;
  asmdostack[asmnndostack].current = base;
  asmdostack[asmnndostack].jrjp = *code ? asmnnJRJP - 1 : -1;
  asmdostack[asmnndostack].jr = addr;
  asmdostack[asmnndostack].cond = plhc ? plhc->code : 0xff;
  asmdostack[asmnndostack++].state = docode;

  return 1;
}

int lhAsmPopDoStack( LHU8 docode, LHU16 addr, PLHU8 base, PLHU8 arg, PLHU8 code, PLHU8 *ppncode,
                     struct s_lhasmdostack **pop )
{
  PLHCNDS plhc = NULL;
  PLHU8 pncode = *ppncode;
  short d;

  if( asmpctx->nndostack == asmnndostack )
    {
    LHASM_ERR( "IF/ELSE/ENDIF, BEGIN/WHILE/REPEAT, BEGIN/UNTIL stack empty\n" );
    return 0;
    }

  *pop = &asmdostack[asmnndostack - 1];
  if( ((*pop)->state & LHASM_DO_IF) && (docode & LHASM_DO_ELSE))
    return 1;

  if( !(docode & (*pop)->state) )
    {
    LHASM_ERR( "%s expected\n",
               (LHASM_DO_IF|LHASM_DO_ELSE) & (*pop)->state ? "IF/ELSE" :
                  LHASM_DO_WHILE & (*pop)->state ? "BEGIN/WHILE" : "BEGIN" );
    return 0;
    }

  if( arg && *code == 0x91 )
    {
    plhc = lhAsmFetchCnd( arg );
    if( !plhc )
      {
      LHASM_ERR( "Unknown condition '%s'\n", arg );
      return 0;
      }
    *code |= (plhc->code ^ 0x02);
    }

  asmnndostack--;
  if( !*code )
    {
    d = addr - asmdostack[asmnndostack].jr;
    if( !asmJRJP[asmdostack[asmnndostack].jrjp] )
      {
#if 0
      if( asmpass != 1 && d <= 255 )
        asmJRJP[asmdostack[asmnndostack].jrjp] = asmJRJPpass = 1;
#endif
      d = 1;
      if( 0xff != asmdostack[asmnndostack].cond )
        d += 2;
      asmdostack[asmnndostack].current[d] = (addr >> 8);
      asmdostack[asmnndostack].current[d + 1] = (addr & 0xFF);
      }
    else
      {
      if( asmdostack[asmnndostack].state & LHASM_DO_WHILE )
        if( !asmJRJP[asmnnJRJP] )
          d++;
      if( d > 255 )
        goto outdis255;
      asmdostack[asmnndostack].current[1] = d;
      }
    if( logfile )
      {
      fprintf( logfd, "%5u\t%04hX\t",
               asmdostack[asmnndostack].srclinenum,
               asmdostack[asmnndostack].jr );
      if( asmJRJP[asmdostack[asmnndostack].jrjp] )
        fprintf( logfd, "%02hhX %02hhX\t",
                 asmdostack[asmnndostack].current[0],
                 asmdostack[asmnndostack].current[1] );
      else if (((*pop)->state & LHASM_DO_ELSE))
        fprintf( logfd, "%02hhX %02hhX %02hhX",
                 asmdostack[asmnndostack].current[0],
                 asmdostack[asmnndostack].current[1],
                 asmdostack[asmnndostack].current[2] );
      else
        fprintf( logfd, ".. .. %02hhX %02hhX %02hhX",
                 asmdostack[asmnndostack].current[2],
                 asmdostack[asmnndostack].current[3],
                 asmdostack[asmnndostack].current[4] );
      fprintf( logfd, " %4d:%s\n",
               asmnndostack,
               (*pop)->state & LHASM_DO_IF ? "if" :
               (*pop)->state & LHASM_DO_ELSE ? "else" :
               (*pop)->state & LHASM_DO_BEGIN ? "begin" :
               (*pop)->state & LHASM_DO_WHILE ? "while" :
               "endif" );
      }
    }

  if( *code )
    {
    d = addr - asmdostack[asmnndostack].jr + 2;
    if( !asmJRJP[asmnnJRJP] )
      {
#if 0
      if( asmpass != 1 && d <= 255 )
        asmJRJP[asmnnJRJP] = asmJRJPpass = 1;
#endif
      if( 0x9E == *code )
        {
        *code = 0xBA;
        }
      else
        {
        if( 0x88 == *code )
          {
          *code = 0x62; //DEC L
          *(pncode++) = 0x81;
          }
        else
          {
          *code ^= 2;
          *code &= 0xEF;
          }
        *(pncode++) = 3;
        *(pncode++) = 0xBA;
        }
      *(pncode++) = (asmdostack[asmnndostack].jr >> 8);
      *(pncode++) = (asmdostack[asmnndostack].jr & 0xFF);
      }
    else
      {
      if( d > 255 )
        goto outdis255;
      *(pncode++) = d;
      }
    asmnnJRJP++;
    if( asmnnJRJP == LHASM_MAX_JRJP )
      LHASM_FTL( "JR to JP database exceeded\n" );

    if( logfile )
      {
      fprintf( logfd, "%5u\t%04hX\t",
               asmdostack[asmnndostack].srclinenum,
               addr );
      if( asmJRJP[asmnnJRJP - 1] )
        fprintf( logfd, "%02hhX %02hhX\t", *code, *(pncode - 1) );
      else
        fprintf( logfd, "%s%02hhX %02hhX %02hhX",
                 (pncode - *ppncode) > 2 ? ".. .. " : "",
                 (pncode - *ppncode) > 2 ? *(pncode - 3) : *code,
                 *(pncode - 2), *(pncode - 1) );
      fprintf( logfd, " %4d:%s\n",
               asmnndostack,
               (*pop)->state & LHASM_DO_IF ? "if" :
               (*pop)->state & LHASM_DO_ELSE ? "else" :
               (*pop)->state & LHASM_DO_BEGIN ? "begin" :
               (*pop)->state & LHASM_DO_WHILE ? "while" :
               (*code == 0x9E | *code == 0xBA) ? "repeat" : "until" );
      }
    }

  *ppncode = pncode;

  return 1;

outdis255:
  LHASM_ERR( "Displacement > 255\n" );
  return 0;
}

int lhAsmOptimizeDoStack( LHU16 addr, PLHU8 base, struct s_lhasmdostack *pop2, struct s_lhasmdostack *pop1 )
{
  short d1, d2;

  if( pop1->state & LHASM_DO_IF )
    {
    if( pop2 )
      {
      d2 = addr - pop2->jr - 2;
      if( !asmJRJP[pop2->jrjp] )
        {
        if( asmpass != 1 && d2 <= 255 )
          asmJRJP[pop2->jrjp] = asmJRJPpass = 1;
        }
      else
        {
        pop2->current[1] = d2;
        if( !asmJRJP[pop1->jrjp] )
          {
          pop1->current[3] = (pop2->jr + 2) >> 8;
          pop1->current[4] = (pop2->jr + 2) & 0xFF;
          }
        }
      d1 = pop2->jr - pop1->jr - (asmJRJP[pop2->jrjp] ? 1 : 0);
      if( !asmJRJP[pop1->jrjp] )
        {
        if( asmpass != 1 && d1 <= 255 )
          asmJRJP[pop1->jrjp] = asmJRJPpass = 1;
        }
      else
        if( !asmJRJP[pop2->jrjp] )
          {
          pop1->current[1] -= 2;
          }
      }
    else
      {
      d1 = addr - pop1->jr - 2;
      if( !asmJRJP[pop1->jrjp] )
        {
        if( asmpass != 1 && d1 <= 255 )
          asmJRJP[pop1->jrjp] = asmJRJPpass = 1;
        }
      }
    }
  else if( pop2 && pop2->state & LHASM_DO_WHILE )
    {
    d1 = addr - pop1->jr;
    if( !asmJRJP[asmnnJRJP - 1] )
      {
      if( asmpass != 1 && d1 <= 255 )
        asmJRJP[asmnnJRJP - 1] = asmJRJPpass = 1;
      }
    d2 = addr - pop2->jr - (asmJRJP[asmnnJRJP - 1]);
    if( !asmJRJP[pop2->jrjp] )
      {
      if( asmpass != 1 && d2 <= 255 )
        asmJRJP[pop2->jrjp] = asmJRJPpass = 1;
      }
    }
  else
    {
    d1 = addr - pop1->jr;
    if( !asmJRJP[asmnnJRJP - 1] )
      {
      if( asmpass != 1 && d1 <= 255 )
        asmJRJP[asmnnJRJP - 1] = asmJRJPpass = 1;
      }
    }
}

int lhAsmStringify( char **argv, LHU8 *code, LHU8 **ppncode )
{
  char *strgfy = argv[0];
  char valueof[20];
  int loop;
  LHU8 *pncode;

  if( !argv || !(*argv[0]) )
    return 0;

  memset( valueof, 0, sizeof(valueof) );
  if( 0 == strncasecmp( strgfy, "VALUEOF'", 8 ) )
    {
    LHU16 sv;

    strgfy += 8;
    if( lhAsmGetLHUValue( strgfy, strgfy, &sv, LINK_MEM_16, 0, 0 ) )
      snprintf( strgfy = valueof, sizeof(valueof), 0x100 > sv ? "%02hx" : "%04hX", sv );
    else
      LHASM_ERR( "Badly formed expression %s\n", strgfy );
    }
  else if( 0 == strncasecmp( strgfy, "VALUE16OF'", 10 ) )
    {
    LHU16 sv;

    strgfy += 10;
    if( lhAsmGetLHUValue( strgfy, strgfy, &sv, LINK_MEM_16, 0, 0 ) )
      snprintf( strgfy = valueof, sizeof(valueof), "%04hX", sv );
    else
      LHASM_ERR( "Badly formed expression %s\n", strgfy );
    }
  else if( 0 == strncasecmp( strgfy, "VALUE8OF'", 9 ) )
    {
    LHU16 sv;

    strgfy += 9;
    if( lhAsmGetLHUValue( strgfy, strgfy, &sv, LINK_MEM_8, 0, 0 ) )
      snprintf( strgfy = valueof, sizeof(valueof), "%02hX", sv );
    else
      LHASM_ERR( "Badly formed expression %s\n", strgfy );
    }
  else if( 0 == strncasecmp( strgfy, "KEYWORDOF'", 10 ) )
    {
    LHU16 sv;

    strgfy += 10;
    if( lhAsmGetLHUValue( strgfy, strgfy, &sv, LINK_MEM_16, 0, 0 ) )
      {
      if( !(strgfy = lhBasicGetBasic( sv )) )
        LHASM_ERR( "No BASIC keyword for code %04hx\n", sv );
      }
    else
      LHASM_ERR( "Badly formed expression %s\n", strgfy );
    }
  else if( 0 == strncasecmp( strgfy, "KEYWORD3OF'", 11 ) )
    {
    LHU16 sv;

    strgfy += 11;
    if( lhAsmGetLHUValue( strgfy, strgfy, &sv, LINK_MEM_16, 0, 0 ) )
      {
      if( (strgfy = lhBasicGetBasic( sv )) )
        {
        strgfy = strncpy( valueof, strgfy, 3 );
        strcat( strgfy, "   " + strlen( strgfy ) );
        }
      else
        LHASM_ERR( "No BASIC keyword for code %04hx\n", sv );
      }
    else
      LHASM_ERR( "Badly formed expression %s\n", strgfy );
    }
  else if( 0 == strncasecmp( strgfy, "MONTHOF'", 8 ) )
    {
    LHU16 sv;
    static char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    strgfy += 8;
    if( lhAsmGetLHUValue( strgfy, strgfy, &sv, LINK_MEM_8, 0, 0 ) && ((0 < sv) && (12 >= sv)) )
      strgfy = month[sv - 1];
    else
      LHASM_ERR( "Badly formed expression %s\n", strgfy );
    }
  pncode = *ppncode;
  *code = strgfy[0];
  for( loop = 1; strgfy[loop]; loop++ )
    {
    *(pncode++) = strgfy[loop];
    }
  *ppncode = pncode;
  return 1;
}

LHU8 lhAsmFetchInst( char *inst, char **argv, LHU16 addr, PLHU8 *current )
{
  int l, loop = 0;
  PLHINST plhi = NULL;
  char *plink, link[2];
  LHU8 ispage = 0, code, page = 0, nn, ncode[514], *pncode, *base = *current, off;
  LHU16 v, cd34;
  short d;
  PLHREGS plhr;
  PLHCNDS plhc;
  LHU8 docode = 0;

  if( inst == 0 )
    return 0;

  if( argv[0] && 0 == strcasecmp( inst, "EXPAND" ) )
    {
    LHU16 x;

    if( lhAsmGetLHUValue( argv[0], argv[0], &x, LINK_MEM_16, 0, 0 ) )
      {
      if( logfile )
        fprintf( logfd, "%5u\t\t%s\t\tEXPAND\t%04hX\n",
                  asmpctx->srclinenum, x ? "+TRUE+" : "/false/", x );
      if( x )
        return lhAsmFetchInst( argv[1], &argv[2], addr, current );
      }
    else
      LHASM_ERR( "Badly formed expression %s\n", argv[0] );
    return 0;
    }

  if( 0 == strncmp( inst, "\\\"", 2 ) && (l = strlen(inst)) > 3 && inst[l - 1] == '"' )
    {
    pncode = ncode;
    code = inst[2];
    for( loop = 3; loop < (l - 1); loop++ )
      {
      *(pncode++) = inst[loop];
      }
    loop = 0;
    goto out;
    }

  if( strchr( "\\!&@+-*^'/~", *inst ) )
    {
    pncode = ncode;
    if( 1 == lhAsmGetLHUValue( inst, inst, &v, LINK_MEM_8, 0, 1 ) )
      {
      code = v;
      loop = 0;
      while( argv[loop] && strchr( "\\!&@+-*^'/~", *argv[loop] ) &&
             ( 1 == lhAsmGetLHUValue( argv[loop], argv[loop], &v, LINK_MEM_8, 0, loop + 2 ) ) )
        {
        *(pncode++) = v;
        loop++;
        }
      }
    else
      LHASM_ERR( "Invalid instruction %s\n", inst );
    goto out;
    }
  else if( 0 == strcasecmp( "BYTE", inst ) && argv[0] )
    {
    pncode = ncode;
    if( 1 == lhAsmGetLHUValue( argv[0], argv[0], &v, LINK_MEM_8, 0, 1 ) )
      {
      code = v;
      loop = 1;
      while( argv[loop] && *argv[loop] != ';' &&
             ( 1 == lhAsmGetLHUValue( argv[loop], argv[loop], &v, LINK_MEM_8, 0, loop + 2 ) ) )
        {
        *(pncode++) = v;
        loop++;
        }
      }
    goto out;
    }
  else if( 0 == strcasecmp( "WORD", inst ) && argv[0] )
    {
    pncode = ncode;
    if( 1 == lhAsmGetLHUValue( argv[0], argv[0], &v, LINK_MEM_16, 0, 1 ) )
      {
      code = (v & 0xFF00) >> 8;
      *(pncode++) = (v & 0x00FF);
      loop = 1;
      while( argv[loop] && *argv[loop] != ';' &&
             ( 1 == lhAsmGetLHUValue( argv[loop], argv[loop], &v, LINK_MEM_16, 0, loop + 2 ) ) )
        {
        *(pncode++) = (v & 0xFF00) >> 8;
        *(pncode++) = (v & 0x00FF);
        loop++;
        }
      }
    goto out;
    }
  else if( 0 == strcasecmp( "TEXT", inst ) && argv[0] &&
           (*argv[0] == '"' && (l = strlen(argv[0])) > 2 && argv[0][l - 1] == '"') )
    {
    pncode = ncode;
    code = argv[0][1];
    for( loop = 2; loop < (l - 1); loop++ )
      {
      *(pncode++) = argv[0][loop];
      }
    loop = 1;
    goto out;
    }
  else if( 0 == strcasecmp( "LENGTH", inst ) && argv[0] &&
           ((*argv[0] == '"' && (l = strlen(argv[0])) > 2 && argv[0][l - 1] == '"') ||
            (*argv[0] != ';' && (l = strlen(argv[0])))) )
    {
    pncode = ncode;
    loop = 1;
    code = l;
    goto out;
    }
  else if( 0 == strcasecmp( "STRINGIFY", inst ) && argv[0] )
    {
    pncode = ncode;
    if( !lhAsmStringify( argv, &code, &pncode ) )
      goto outnocode;

    loop = 1;
    goto out;
    }
  else if( 0 == strcasecmp( "BIND", inst ) && argv[1] )
    {
    LHU16 br, brs;
    struct s_lhasmdata *pd;
    struct s_lhasmstruct *ps;
    struct s_lhasmfield *pf = NULL;
    char *field;
    LHU16 fieldoff = 0, substoff = 0;
    LHU16 infield = 0;
    LHU16 atbase = 0xFFFF;
    PLHREGS plhrs;

    nn = 0;
    plhr = lhAsmFetchReg( argv[0] );
    if( !plhr || (plhr->link != LINK_REG_HL && plhr->link != LINK_REG_BC) )
      LHASM_ERR( "Register %s is illegal for BIND\n", argv[0] );
    br = GET_LHREGS( plhr->code );
    plhrs = lhAsmFetchReg( argv[1] );
    if( plhrs )
      {
      if( (plhrs->link != LINK_REG_HL && plhrs->link != LINK_REG_BC) )
        LHASM_ERR( "Source register %s is illegal for BIND\n", argv[1] );
      brs = GET_LHREGS( plhrs->code );
      if( !asmbindR[brs].pstruct )
        LHASM_ERR( "Register %s not bound\n", argv[1] );
      if( asmbindR[br].pstruct && (asmbindR[br].pstruct != asmbindR[brs].pstruct) )
        LHASM_ERR( "Destination register %s is bound. UNBUILD it\n", argv[0] );
      memcpy( &asmbindR[br], &asmbindR[brs], sizeof(struct s_lhasmbind) );
      if( asmbindR[brs].pdata )
        {
        nn++;
        pncode = ncode;
        if( !br )
          {
          page = 0xFD;
          code = 0x08 | (brs << 4);
          }
        else if( !brs )
          {
          page = 0xFD;
          code = 0x4A | (br << 4);
          }
        else
          {
          page = 0xFD;
          code = 0x88 | (brs << 4);
          *(pncode++) = 0xFD;
          *(pncode++) = 0x0A | (br << 4);
          }
        }
      goto endbind;
      }

    ps = lhAsmFetchStruct( argv[1], &field, &pd, &atbase );
    if( !ps )
      LHASM_ERR( "Unknown STRUCT %s for BIND\n", argv[1] );
    if( asmbindR[br].pdata && (pd != asmbindR[br].pdata) )
      LHASM_ERR( "BIND %s DATA differs. UNBIND it\n",
                 argv[0], asmbindR[br].pdata->name );
    if( asmbindR[br].pstruct && (ps != asmbindR[br].pstruct) )
      LHASM_ERR( "BIND %s STRUCT differs. UNBIND it\n",
                 argv[0], asmbindR[br].pstruct->name );
    if( field )
      {
      pf = lhAsmFetchField( ps, field, &infield, &fieldoff );
      if( !pf )
        LHASM_ERR( "Unknown field %s in STRUCT %s for BIND\n", argv[1], ps->name );
      }
    if( !asmbindR[br].pdata && pd )
      {
      asmbindR[br].atbase = atbase;
      atbase += fieldoff;
      pncode = ncode;
      code = 0x48 | (br << 4);
      *(pncode++) = (atbase >> 8) & 0xFF;
      *(pncode++) = 0x4A | (br << 4);
      *(pncode++) = atbase & 0xFF;
      nn++;
      }
    else if( asmbindR[br].pdata && pd )
      {
      LHU16 ataddr = asmbindR[br].atbase + asmbindR[br].offset;
      atbase += fieldoff;
      pncode = ncode;
      if( ((ataddr) & 0xFF00) != (atbase & 0xFF00) )
        {
        code = 0x48 | (br << 4);
        *(pncode++) = (atbase >> 8) & 0xFF;
        nn++;
        }
      if( ((ataddr) & 0x00FF) != (atbase & 0x00FF) )
        {
        if( pncode != ncode )
          *(pncode++) = 0x4A | (br << 4);
        else
          code = 0x4A | (br << 4);
        *(pncode++) = atbase & 0xFF;
        nn++;
        }
      asmbindR[br].atbase = (atbase - fieldoff);
      }
    else if( asmbindR[br].pstruct && fieldoff != asmbindR[br].offset )
      {
      pncode = ncode;
      if( fieldoff < asmbindR[br].offset )
        {
        if( (asmbindR[br].offset - fieldoff) < 9 )
          {
          code = 0x46 | (br << 4);
          for( nn = 1; nn < (asmbindR[br].offset - fieldoff); nn++ )
            *(pncode++) = 0x46 | (br << 4);
          }
        else
          {
          substoff = (asmbindR[br].offset - fieldoff);
          code = 0xFB;
          *(pncode++) = 0x04 | (br << 4);
          *(pncode++) = 0xB1;
          *(pncode++) = (substoff & 0xFF);
          *(pncode++) = 0x0A | (br << 4);
          *(pncode++) = 0x84 | (br << 4);
          if( fieldoff < 0x100 )
            {
            *(pncode++) = 0x30;
            }
          else
            {
            *(pncode++) = 0xB1;
            *(pncode++) = ((substoff >> 8) & 0xFF);
            }
          *(pncode++) = 0x08 | (br << 4);
          }
        }
      else if( 5 > (fieldoff - asmbindR[br].offset) )
        {
        code = 0x44 | (br << 4);
        for( nn = 1; nn < (fieldoff - asmbindR[br].offset); nn++ )
          *(pncode++) = 0x44 | (br << 4);
        }
      else
        {
        code = 0xB5;
        *(pncode++) = fieldoff;
        *(pncode++) = 0xFD;
        *(pncode++) = 0xCA | (br << 4);
        }
      }

endbinddata:
    asmbindR[br].pfield = pf;
    asmbindR[br].infield = infield;
    asmbindR[br].offset = fieldoff;
    if( !asmbindR[br].pstruct )
      fieldoff = 0;
    asmbindR[br].pstruct = ps;
    asmbindR[br].pdata = pd;

endbind:
    loop = 2;
    if( logfile )
      fprintf( logfd, "%5u\t%04hX\t\t\tBIND\t%s %s\n",
               asmpctx->srclinenum, lhAsmGetPC(), plhr->regs, argv[1] );
    if( (pd && nn) || fieldoff || substoff )
      goto out;
    goto outnocode;
    }
  else if( 0 == strcasecmp( "UNBIND", inst ) && argv[0] )
    {
    plhr = lhAsmFetchReg( argv[0] );
    if( !plhr || (plhr->link != LINK_REG_HL && plhr->link != LINK_REG_BC) )
      LHASM_ERR( "Register %s is illegal for UNBIND\n", argv[0] );
    memset( &asmbindR[GET_LHREGS( plhr->code )] , 0, sizeof(asmbindR[0]) );
    loop = 1;
    if( logfile )
      fprintf( logfd, "%5u\t%04hX\t\t\tUNBIND\t%s\n",
               asmpctx->srclinenum, lhAsmGetPC(), plhr->regs );
    goto outnocode;
    }
  else if( argv[0] && (0 == strcasecmp( "if", inst ) ||
           (0 == strcasecmp( "if!", inst ) && (docode |= LHASM_DO_FORCE_JR) )) )
    {
    docode |= LHASM_DO_IF;
    loop++;
    code = 0x81;
dopushjr:
    pncode = ncode;
    if( lhAsmPushDoStack( docode, addr, base, loop ? argv[0] : NULL, &code, &pncode ) )
      goto out;
    else
      goto outnocode;
    }
  else if( (0 == strcasecmp( "else", inst ) ||
           (0 == strcasecmp( "else!", inst ) && (docode |= LHASM_DO_FORCE_JR) )) )
    {
    docode |= LHASM_DO_ELSE;
    code = 0x8E;
    goto dopushjr;
    }
  else if( 0 == strcasecmp( "endif", inst ) )
    {
    struct s_lhasmdostack *popelse, *popif;
    //docode = (LHASM_DO_IF|LHASM_DO_ELSE);
    docode = LHASM_DO_ELSE;
    pncode = ncode;
    code = 0;
    if( lhAsmPopDoStack( docode, addr, base, NULL, &code, &pncode, &popelse ) &&
        popelse->state & LHASM_DO_ELSE )
      {
      docode = LHASM_DO_IF;
      lhAsmPopDoStack( docode,
                       asmJRJP[popelse->jrjp] ? popelse->jr : popelse->jr + 3,
                       popelse->current, NULL, &code, &pncode, &popif );
      }
    else
      {
      docode = LHASM_DO_IF;
      popif = popelse;
      popelse = NULL;
      lhAsmPopDoStack( docode,
                       asmJRJP[popif->jrjp] ? addr - 2 : addr,
                       base, NULL, &code, &pncode, &popif );
      }
    lhAsmOptimizeDoStack( addr, base, popelse, popif );
    goto outnocode;
    }
  else if( 0 == strcasecmp( "begin", inst ) )
    {
    docode = LHASM_DO_BEGIN;
    lhAsmPushDoStack( docode, addr, base, NULL, &code, &pncode );
    goto outnocode;
    }
  else if( argv[0] && (0 == strcasecmp( "while", inst ) ||
           (0 == strcasecmp( "while!", inst ) && (docode |= LHASM_DO_FORCE_JR) )) )
    {
    docode |= LHASM_DO_WHILE;
    loop++;
    goto dopushjr;
    }
  else if( 0 == strcasecmp( "repeat", inst ) )
    {
    struct s_lhasmdostack *popwhile, *popbegin;
    docode = LHASM_DO_WHILE;
    pncode = ncode;
    code = 0;
    if( !lhAsmPopDoStack( docode,
                          asmJRJP[asmnnJRJP - 1] ? addr: addr + 3,
                          base, NULL, &code, &pncode, &popwhile ) )
      goto outnocode;
    code = 0x9E;
dopopbegin:
    docode = LHASM_DO_BEGIN;
    lhAsmPopDoStack( docode, addr, base, loop ? argv[0] : NULL, &code, &pncode, &popbegin );
    lhAsmOptimizeDoStack( addr, base, (code == 0x9E | code == 0xBA) ? popwhile : NULL, popbegin );
    goto out;
    }
  else if( argv[0] && (0 == strcasecmp( "until", inst )) )
    {
    pncode = ncode;
    if( 0 == strcasecmp( argv[loop++], "DJC" ) )
      code = 0x88;
    else
      code = 0x91;
    goto dopopbegin;
    }

  l = strlen(inst);
  if( inst[l-1] == '#' )
    {
    ispage = SECOND_PAGE;
    l--;
    }
  link[0] = link[1] = 0x0;
  for( plhi = lhinst ; plhi->inst ; plhi++ )
    {
    page = 0;
    pncode = ncode;
    plink = &(plhi->inst[l]);
    if( !(plink[0] == '%' || plink[0] == '!' || plink[0] == 0) )
      continue;
    if( strncasecmp( inst, plhi->inst, l ) == 0 )
      {
      if( plink[0] == 0 )
        {
        code = plhi->code;
        goto out;
        }
      else if( plink[0] == '!' && plink[1] == 0 )
        {
        page = SECOND_PAGE;
        code = plhi->code;
        goto out;
        }
      else if( plink[0] == '!' && plink[1] == '#' )
        {
        page = ispage;
        code = plhi->code;
        goto out;
        }
      else if( plink[0] == '!' && plink[1] != '%' )
        {
        page = SECOND_PAGE;
        plink ++;
        }
      else if( plink[0] == '!' && plink[1] == '%' )
        {
        page = ispage;
        plink += 2;
        }
      else
        plink++;
      code = plhi->code;
      loop = 0;
      off = (page ? 1 : 0);

replay:
      off++;
      if( argv[loop] )
        {
        if( (*plink == LINK_iREG_HL && (*argv[loop] != '(' && *argv[loop] != '#') ))
          continue;
        else if( (*plink == LINK_iMEM_8 || *plink == LINK_iMEM_16 || *plink == LINK_SBR) &&
            *argv[loop] !='(' )
          continue;
        if( loop < 2 )
          link[loop] = *plink;
        switch( *plink )
          {
          case LINK_NONE:
               goto out;
          case LINK_MEM_8:
               if( lhAsmGetLHUValue( argv[loop], argv[loop], &v, *plink, 0, off ) )
                 {
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               continue;
          case LINK_MEM_16:
               if( lhAsmGetLHUValue( argv[loop], argv[loop], &v, *plink, addr, ++off ) )
                 {
                 *(pncode++) = (v & 0xFF00) >> 8;
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               continue;
          case LINK_iMEM_8:
               if( lhAsmGetLHUValue( argv[loop], argv[loop], &v, *plink, 0, off ) )
                 {
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               continue;
          case LINK_iMEM_16:
               if( lhAsmGetLHUValue( argv[loop], argv[loop], &v, *plink, addr, ++off ) )
                 {
                 *(pncode++) = (v & 0xFF00) >> 8;
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               continue;
          case LINK_REG_L:
          case LINK_REG_H:
          case LINK_REG_HL:
          case LINK_iREG_HL:
               {
               struct s_lhasmbind *pb;
               struct s_lhasmfield *pf;
               LHU16 br;

               plhr = lhAsmFetchReg( argv[loop] );
               if( !plhr || (plhr->link != *plink && (*plink != LINK_REG_HL || plhr->link != LINK_REG_BC)) )
                 continue;
               br = GET_LHREGS( plhr->code );
               pb = &asmbindR[br];
               if( !loop && plhi->bind == BIND_KILL && pb->pstruct )
                 LHASM_LOW( "Register %s is bound\n", plhr->regHL );
               else if( loop && pb->pstruct && *plink == LINK_REG_HL )
                 LHASM_LOW( "Destination register will not be bound. Use BIND\n" );
               else if( !loop && plhi->bind == BIND_UPDATE && pb->pstruct )
                 {
                 int ifi;

                 switch( code )
                   {
                   case 0x44:	// INC R
                   case 0x45:	// LDI (R)
                   case 0x41:	// STI (R)
                        pb->offset++;
                        if( pb->offset >= pb->pstruct->size )
                          LHASM_ERR( "Increment %s goes outside STRUCT %s\n",
                                     plhr->regHL, pb->pstruct->name );
                        for( ifi = 0, pf = &pb->pstruct->fields[ifi];
                             ifi < pb->pstruct->nnfield;
                             ifi++, pf++ )
                          if( (pf->offset <= pb->offset) &&
                              (pb->offset < (pf->offset + pf->size)) )
                            {
                            pb->pfield = pf;
                            break;
                            }
                        break;
                   case 0x46:	// DEC R
                   case 0x47:	// LDD (R)
                   case 0x43:	// STD (R)
                        pb->offset--;
                        if( pb->offset == 0xFFFF )
                          LHASM_ERR( "Decrement %s goes outside STRUCT %s\n",
                                     plhr->regHL, pb->pstruct->name );
                        for( ifi = 0, pf = &pb->pstruct->fields[ifi];
                             ifi < pb->pstruct->nnfield;
                             ifi++, pf++ )
                          if( (pf->offset <= pb->offset) &&
                              (pb->offset < (pf->offset + pf->size)) )
                            {
                            pb->pfield = pf;
                            break;
                            }
                        break;
                   }
                 }
               code |= (plhr->code & plhi->amsk);
               goto nextplay;
               }
          case LINK_REG_A:
          case LINK_REG_F:
          case LINK_REG_T0:
          case LINK_REG_T1:
               plhr = lhAsmFetchReg( argv[loop] );
               if( !plhr || plhr->link != *plink )
                 continue;
               goto nextplay;
          case LINK_REG_BC:
          case LINK_REG_SP:
          case LINK_REG_PC:
               plhr = lhAsmFetchReg( argv[loop] );
               if( !plhr || plhr->link != *plink )
                 continue;
               if( !loop && plhi->bind == BIND_KILL &&
                   *plink == LINK_REG_BC && asmbindR[ GET_LHREGS(plhr->code) ].pstruct )
                 LHASM_LOW( "Register %s is bound\n", plhr->regHL );
               else if( loop && *plink == LINK_REG_BC &&
                        asmbindR[ GET_LHREGS(plhr->code) ].pstruct )
                 LHASM_LOW( "Destination register will not be bind. Use BIND\n" );
               goto nextplay;
          case LINK_JREL:
               if( !lhAsmGetLHUValue( argv[loop], argv[loop], &v, *plink, addr, (off == 1 ? 2 : off) ) )
                 continue;
               d = (v - (pncode - ncode + addr + 2));
               if( (asmflags & LHASM_JR_TO_JP) && !asmJRJP[ asmnnJRJP ] )
                 {
                 if( (code & 0xE1) == 0x81 )
                   {
                   if( (asmpass != 1) && ((d < 0 ? -d : d) <= 255) )
                     asmJRJP[asmnnJRJP] = asmJRJPpass = 1;
                   code ^= 0x02;
                   *pncode++ = 3;
                   *pncode++ = 0xBA; // JP v
                   *pncode++ = (v >> 8) & 0xFF;
                   *pncode++ = (v & 0xFF);
                   goto nextplay;
                   }
                 else if( code == 0x8E ) 
                   {
                   if( (asmpass != 1) && ((d < 0 ? -d : d) <= 255) )
                     asmJRJP[asmnnJRJP] = asmJRJPpass = 1;
                   code = 0xBA;
                   *pncode++ = (v >> 8) & 0xFF;
                   *pncode++ = (v & 0xFF);
                   goto nextplay;
                   }
                 }
               if( d < 0 )
                 {
                 if( asmpass != 1 && (code & 0xC0 ) == 0xC0 )
                   LHASM_ERR( "SBR can not go backward\n" );
                 d = -d;
                 if( d > 0xFF )
                   LHASM_ERR( "Displacement > 255 bytes\n" );
                 if( code != 0x88 )
                   code |= 0x10;
                 }
               if( d > 0xFF )
                 LHASM_ERR( "Displacement > 255 bytes\n" );
               *pncode++ = (d & 0xFF);
               goto nextplay;
          case LINK_CNDS:
               plhc = lhAsmFetchCnd( argv[loop] );
               if( !plhc )
                 continue;
               if( (code & 0xC0) == 0xC0 && NOVERFLOW == plhc->code )
                 continue;
               code |= plhc->code;
               goto nextplay;
          case LINK_SBR:
               if( lhAsmGetLHUValue( NULL, argv[loop], &v, *plink, 0, off ) )
                 {
                 PLHINST plhs = lhsbr;

                 if( !loop && (code == 0xCD) && (v >= 0xC0) && !(v & 1) )
                   code = v;
                 else
                   *(pncode++) = (v & 0xFF);
                 while( plhs->inst )
                   {
                   if( plhs->code == v )
                     {
                     plink = plhs->inst - 1;
                     break;
                     }
                   plhs++;
                   }
                 goto nextplay;
                 }
               continue;
          case LINK_CD34:
               if( !lhAsmGetLHUValue( NULL, argv[loop], &cd34, *plink, 0, off ) )
                 continue;
               *(pncode++) = cd34;
               do
                 {
                 ++off;
                 if( argv[++loop] == NULL )
                   continue;
                 if( lhAsmGetLHUValue( argv[loop], argv[loop], &v, LINK_MEM_8, 0, off ) )
                   {
                   *(pncode++) = (v & 0xFF);
                   }
                 if( argv[++loop] == NULL )
                   continue;
                 ++off;
                 if( lhAsmGetLHUValue( argv[loop], argv[loop], &v, LINK_JREL, (pncode - ncode + addr + 2), off + 1) )
                   {
                   d = (v - (pncode - ncode + addr + 2));
                   if( d < 0 )
                     continue;
                   *(pncode++) = (d & 0xFF);
                   }
                 } while( cd34-- );
               loop++;
               goto out;
          case LINK_KYWD:
               {
               char s[5];

               if( (0 == strncmp( argv[loop], "$:", 2 ) && argv[loop][2] ) &&
                   lhAsmGetLHUValue( argv[loop], argv[loop], &v, LINK_MEM_8, 0, 0 ) )
                 {
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               else if( 2 == strlen( argv[loop]) && 1 == sscanf( argv[loop], "%hx", &v ) && v < 0x100 &&
                   2 == sprintf( s, "%02hX", v ) && strcasecmp( argv[loop], s ) == 0 )
                 {
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               else if( 0 == strncasecmp( argv[loop], "value8of'", 9 ) &&
                        lhAsmGetLHUValue( argv[loop] + 9, argv[loop] + 9, &v, LINK_MEM_8, 0, 0 ) )
                 {
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               else if( 4 == strlen( argv[loop]) && 1 == sscanf( argv[loop], "%hx", &v ) && v >= 0xE000 &&
                        4 == sprintf( s, "%04hX", v ) && strcasecmp( argv[loop], s ) == 0 )
                 {
                 off++;
                 *(pncode++) = (v & 0xFF00) >> 8;
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               else if( 0 == strncasecmp( argv[loop], "value16of'", 10 ) &&
                        lhAsmGetLHUValue( argv[loop] + 10, argv[loop] + 10, &v, LINK_MEM_16, 0, 0 ) &&
                        v >= 0xE000 )
                 {
                 off++;
                 *(pncode++) = (v & 0xFF00) >> 8;
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               else if( 0 == strncasecmp( argv[loop], "valueof'", 8 ) &&
                        lhAsmGetLHUValue( argv[loop] + 8, argv[loop] + 8, &v, LINK_MEM_16, 0, 0 ) )
                 {
                 if( v >= 0xE000 )
                   {
                   off++;
                   *(pncode++) = (v & 0xFF00) >> 8;
                   *(pncode++) = (v & 0xFF);
                   goto nextplay;
                   }
                 else if( v < 0x100 )
                   {
                   *(pncode++) = (v & 0xFF);
                   goto nextplay;
                   }
                 }
               else if( (0 == strncasecmp( argv[loop], "basic'", 6 ) &&
                        ((v = lhBasicGetCode( argv[loop] + 6)) || asmpass == 1 )) )
                 {
                 off++;
                 *(pncode++) = (v & 0xFF00) >> 8;
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               else if( (v = lhBasicGetCode( argv[loop])) || asmpass == 1 )
                 {
                 off++;
                 *(pncode++) = (v & 0xFF00) >> 8;
                 *(pncode++) = (v & 0xFF);
                 goto nextplay;
                 }
               }
               LHASM_ERR( "Invalid expression %s\n", argv[loop] );
               continue;
          }
        }
      }
    }
  LHASM_ERR( "Unknown instruction '%s'!\n", inst );
  return 0;

nextplay:
  loop++;
  if( plink[1] == '%' )
    {
    plink += 2;
    goto replay;
    }

out:
  if( (asmflags & LHASM_JR_TO_JP) && (((plhi->code & 0xE1) == 0x81) || (plhi->code == 0x8E)) )
    {
    asmnnJRJP++;
    if( asmnnJRJP == LHASM_MAX_JRJP )
      LHASM_FTL( "JR to JP database exceeded\n" );
    }
  if( page )
    *(base++) = page;
  *(base++) = code;
  for( nn = 0; &ncode[nn] < pncode; nn++ )
    *(base++) = ncode[nn];
outnocode:
  nn = base - *current;
  *current = base;
  if( argv[loop] && *argv[loop] != ';' )
    LHASM_WRN( "Argument %d; Trailing garbage not processed!\n", loop+2 );
  return nn;
}

LHU8 lhAsmFetchBasic( char *line, char **argv, LHU16 addr, PLHU8 *current )
{
  LHU8 nn, *ncode = *current, *pncode, *restart;
  LHU16 linenum, code, v;
  LHU8 symna[80];
  char ascii[5];
  static char evalstr[4096];
 
  if( !line || (!asmpinlin && (!argv || !argv[0])) )
    return 0;

  if( asmpinlin )
    {
    if( strcasecmp( line, "\\]end" ) == 0 )
      {
      pncode = *current;
      ncode = asmpinlin->plinebase;
      pncode = asmpinlin->pcurrent;
      asmpinlin->code = 0;
      asmpinlin->linenum = 0;
      asmpinlin->plinebase = 0;
      asmpinlin->pcurrent = 0;
      asmpinlin = NULL;
      }
    else
      {
      LHU8 inlinbuf[ 80 ], *inlin;
      pncode = *current;
      inlin = inlinbuf;
      ncode = asmpinlin->plinebase;
      pncode = asmpinlin->pcurrent;
      nn = lhAsmFetchInst( line, argv, addr, &inlin );
      inlin = inlinbuf;
      if( asmpinlin->code == 0x4450 )
        {
        while( nn-- )
          {
          if( pncode != asmpinlin->plineasm )
            *(pncode++) = ',';
          sprintf( pncode, "&%02hhX", *(inlin++) );
          pncode += 3;
          }
        }
      else
        {
        memcpy( pncode, inlinbuf, nn );
        pncode += nn;
        }
      asmpinlin->pcurrent = pncode;
      return 0;
      }
    }
  else if( 6 > strlen(line) && 1 == sscanf( line, "%hu", &linenum) && linenum && linenum < 0xFF00 )
    {
    pncode = ncode;
    *(pncode++) = (linenum & 0xFF00) >> 8;
    *(pncode++) = (linenum & 0xFF);
    *(pncode++) = 3; // At least 3 bytes: LINENUM (16-bits) and LINELEN (1 byte)
    if( 1 == asmpass )
      {
      snprintf( symna, sizeof(symna), "%s$$._addr:%05hu", asmpctx->srcfile, linenum );
      asmsym = lhAsmAddSymSorted( asmsym, symna, (LHU16)(pncode - asmmem), SYM_NONE, 1 );
      }
    }
  else
    LHASM_ERR( "Badly formed BASIC line '%s %s...'\n", line, (argv[0] ? argv[0] : "") );

    for( nn = 0; argv[nn] && (pncode - ncode) < 79; nn++ )
      {
      restart = argv[nn];
      while( restart && *restart )
        {
        if( *restart == '"' )
          {
          do
            {
            if( *restart == '\\' )
              {
              ascii[2] = 0;
              if( *(restart + 1) == '\\' )
                {
                *(pncode++) = '\\';
                restart += 2;
                }
              else if( strcasecmp( restart + 1, "asm[" ) == 0 )
                {
                if( asmpinlin )
                  LHASM_ERR( "asm[ inlining is already active starting line %u\n", asmpinlin->linenum );
                (asmpinlin = &asminlin)->code = 0x2424;
                asmpinlin->linenum = linenum;
                asmpinlin->plinebase = ncode;
                asmpinlin->plineasm = asmpinlin->pcurrent = pncode;
                return 0;
                }
              else if( *(restart + 1) && strchr( "ipsy", *(restart + 1) ) )
                {
                int n = 0;
                char c = *(restart);
                if( 0 == strncmp( "ins", (restart + 1), 3 ) )
                  {
                  c = 0x27;
                  n = 3;
                  }
                else if( 0 == strncmp( "sqr", (restart + 1), 3 ) )
                  {
                  c = 0x5b;
                  n = 3;
                  }
                else if( 0 == strncmp( "yen", (restart + 1), 3 ) )
                  {
                  c = 0x5c;
                  n = 3;
                  }
                else if( 0 == strncmp( "pi", (restart + 1), 2 ) )
                  {
                  c = 0x5d;
                  n = 2;
                  }
                *(pncode++) = c;
                restart += (1 + n);
                }
              else if( memcpy( ascii, restart+1, 2 ) && 1 == sscanf( ascii, "%hx", &v ) && v < 0x100 )
                {
                *(pncode++) = (v & 0xFF);
                restart += 3;
                }
              else
                *(pncode++) = *(restart++);
              }
            else
              *(pncode++) = *(restart++);
            }
          while( *restart && *restart != '"' );
          if( *restart == '"' )
            *(pncode++) = *(restart++);
          }
        else if( *restart == '\\' )
          {
          char basic[16];
          LHU8 *pp;

          if( *(restart+1) > 0x44 && *(restart + 5) == ' ' &&
                   memcpy( ascii, restart+1, 4 ) && 1 == sscanf( ascii, "%hx", &v ) )
            {
            *(pncode++) = (v & 0xFF00) >> 8;
            *(pncode++) = (v & 0xFF);
            restart += 6;
            }
          else if( *(restart + 1) == '[' && (pp = strchr( (restart + 4), ']' )) &&
                   memset( basic, 0, sizeof(basic) ) &&
                   strncpy( basic, restart + 2, (pp - restart - 2) ) &&
                   (code = lhBasicGetCode( basic )) )
            {
            *(pncode++) = (code & 0xFF00) >> 8;
            *(pncode++) = (code & 0xFF);
            restart = (pp + 1);
            }
          else if( strncasecmp( restart + 1, "get[", 4 ) == 0 &&
                   (pp = strrchr( (restart + 5), ']' )) &&
                   memset( symna, 0, sizeof(symna) ) &&
                   strncpy( symna, restart + 5, (pp - restart - 5) ) &&
                   (code = lhAsmGetSymValue( asmsym, symna )) )
            {
            snprintf( ascii, sizeof(ascii), "%04hX", code );
            *(pncode++) = '&';
            *(pncode++) = ascii[0];
            *(pncode++) = ascii[1];
            *(pncode++) = ascii[2];
            *(pncode++) = ascii[3];
            restart = (pp + 1);
            }
          else if( strncasecmp( restart + 1, "addr[", 5 ) == 0 &&
                   (pp = strchr( (restart + 6), ']' )) &&
                   1 == sscanf( restart + 6, "%u]", &v ) &&
                   snprintf( symna, sizeof(symna), "%s$$._addr:%05u", asmpctx->srcfile, v ) &&
                   (code = lhAsmGetSymValue( asmsym, symna )) )
            {
            snprintf( ascii, sizeof(ascii), "%04hX", code );
            *(pncode++) = '&';
            *(pncode++) = ascii[0];
            *(pncode++) = ascii[1];
            *(pncode++) = ascii[2];
            *(pncode++) = ascii[3];
            restart = (pp + 1);
            }
          else if( strncasecmp( restart + 1, "eval16[", 7 ) == 0 &&
                   (pp = strrchr( (restart + 8), ']' )) &&
                   memset( evalstr, 0, sizeof(evalstr) ) &&
                   strncpy( evalstr, restart + 8, (pp - restart - 8) ) &&
                   (lhAsmGetLHUValue( evalstr, evalstr, &v, LINK_MEM_16, 0, 0)) )
            {
            snprintf( ascii, sizeof(ascii), "%04hX", v );
            *(pncode++) = '&';
            *(pncode++) = ascii[0];
            *(pncode++) = ascii[1];
            *(pncode++) = ascii[2];
            *(pncode++) = ascii[3];
            restart = (pp + 1);
            }
          else if( strncasecmp( restart + 1, "eval8[", 6 ) == 0 &&
                   (pp = strrchr( (restart + 7), ']' )) &&
                   memset( evalstr, 0, sizeof(evalstr) ) &&
                   strncpy( evalstr, restart + 7, (pp - restart - 7) ) &&
                   (lhAsmGetLHUValue( evalstr, evalstr, &v, LINK_MEM_8, 0, 0)) && (v <= 0x100) )
            {
            snprintf( ascii, sizeof(ascii), "%02hX", v );
            *(pncode++) = '&';
            *(pncode++) = ascii[0];
            *(pncode++) = ascii[1];
            restart = (pp + 1);
            }
          else if( strcasecmp( restart + 1, "asm[" ) == 0 )
            {
            if( asmpinlin )
              LHASM_ERR( "asm[ inlining is already active starting line %u\n", asmpinlin->linenum );
            (asmpinlin = &asminlin)->code = 0x4450;
            asmpinlin->linenum = linenum;
            asmpinlin->plinebase = ncode;
            asmpinlin->plineasm = asmpinlin->pcurrent = pncode;
            return 0;
            }
          else if( memcpy( ascii, restart+1, 2 ) && 1 == sscanf( ascii, "%hx", &v ) && v < 0x100 )
            {
            *(pncode++) = (v & 0xFF);
            restart += 3;
            }
          else
            *(pncode++) = *(restart++);
          }
        else if( (code = lhBasicGetCodeClosest( restart, &restart )) )
          {
          *(pncode++) = (code & 0xFF00) >> 8;
          *(pncode++) = (code & 0xFF);
          if( code == 0xF1AB && *restart && *(++restart) ) // Handle the REM instruction
            {
            do
              {
              if( *restart == '\\' )
                {
                ascii[2] = 0;
                ascii[4] = 0;
                if( *(restart + 1) == '\\' )
                  {
                  *(pncode++) = '\\';
                  restart += 2;
                  }
                else if( strcasecmp( restart + 1, "asm[" ) == 0 )
                  {
                  code = 0xF1AB;
                  if( asmpinlin )
                    LHASM_ERR( "asm[ inlining is already active starting line %u\n", asmpinlin->linenum );
                  (asmpinlin = &asminlin)->code = code;
                  asmpinlin->linenum = linenum;
                  asmpinlin->plinebase = ncode;
                  asmpinlin->plineasm = asmpinlin->pcurrent = pncode;
                  return 0;
                  }
                else if( *(restart + 1) && strchr( "ipsy", *(restart + 1) ) )
                  {
                  int n = 0;
                  char c = *(restart);
                  if( 0 == strncmp( "ins", (restart + 1), 3 ) )
                    {
                    c = 0x27;
                    n = 3;
                    }
                  else if( 0 == strncmp( "sqr", (restart + 1), 3 ) )
                    {
                    c = 0x5b;
                    n = 3;
                    }
                  else if( 0 == strncmp( "yen", (restart + 1), 3 ) )
                    {
                    c = 0x5c;
                    n = 3;
                    }
                  else if( 0 == strncmp( "pi", (restart + 1), 2 ) )
                    {
                    c = 0x5d;
                    n = 2;
                    }
                  *(pncode++) = c;
                  restart += (1 + n);
                  }
                else if( *(restart+1) > 0x44 && *(restart + 5) == ' ' &&
                        memcpy( ascii, restart+1, 4 ) && 1 == sscanf( ascii, "%hx", &v ) )
                  {
                  *(pncode++) = (v & 0xFF00) >> 8;
                  *(pncode++) = (v & 0xFF);
                  restart += 6;
                  }
                else if( memcpy( ascii, restart+1, 2 ) && 1 == sscanf( ascii, "%hx", &v ) && v < 0x100 )
                  {
                  *(pncode++) = (v & 0xFF);
                  restart += 3;
                  }
                else
                  *(pncode++) = *(restart++);
                }
              else
                *(pncode++) = *(restart++);
              }
            while( *restart );
            break;
            }
          }
        else
          {
          *(pncode++) = *(restart++);
          }
        restart = lhAsmEatSp( restart );
        }
      }

update:
  if( !asmpinlin )
    {
    *(pncode++) = 0x0d;
    ncode[2] = (pncode - ncode - 3);
    }

out:
  *current = pncode;
  return pncode - ncode;
}

void lhAsmDoInclude( char *incfile )
{
  struct stat st;
  FILE *fd;
  char *arg[LHASM_MAX_ARGS];
  char  buffer[1024];
  int   narg, i, l, export;
  int iinc;
  LHU16 v, nn, thisdata;
  LHU8 fidx;
  char basic[16];
  int savestate;
  struct s_lhasmmacro *pmacro = NULL;
  struct s_lhasmstruct *pstruct = NULL;
  struct s_lhasmdata *pdata = NULL;
  int macronnline, structnnfield, datann;
  LHU16 resymva = 0xFFFF;
  char incpath[PATH_MAX];

  if( incfile == NULL )
    {
    fd = stdin;
    asmpctx->srcfile = "stdin";
    iinc = 0;
    }
  else
    {
    int fdst, ipinc = -2;
    char *d, *e;

    do
      {
      if( -2 == ipinc )
        snprintf( incpath, sizeof(incpath), "%s", incfile );
      else if( -1 == ipinc )
        {
        memset( incpath, 0, sizeof(incpath) );
        if( asmnnctx && (d = strrchr( e = asmctx[asmnnctx - 1].srcfile, '/' )) )
          {
          strcat( strncpy( incpath, e, d - e + 1 ), incfile );
          }
        else
          snprintf( incpath, sizeof(incpath), "%s", incfile );
        }
      else if( LHASM_MAX_INCLUDE_PATH == ipinc || NULL == asmincpath[ ipinc ] )
        LHASM_FTL( "Unable to stat file %s\n", incfile );
      else
        snprintf( incpath, sizeof(incpath), "%s/%s", asmincpath[ ipinc ], incfile );
#ifdef _WIN32
      if( 0 == access( incpath, R_OK ) )
        break;
#else
      if( 0 == stat( incpath, &st ) )
        break;
#endif
      }
    while( LHASM_MAX_INCLUDE_PATH > ++ipinc );

    for( iinc = 0; iinc < LHASM_MAX_INCLUDE; iinc++ )
      {
#ifdef _WIN32
      if( 0 == asmincluded[ iinc ].name[0] )
        {
        strcpy( asmincluded[ iinc ].name, incpath );
        break;
        }
      if( 0 == strcmp( incpath, asmincluded[ iinc ].name ) )
        {
        return 1;
        }
#else
      if( 0 == asmincluded[ iinc ].dev )
        {
        asmincluded[ iinc ].dev = st.st_dev;
        asmincluded[ iinc ].ino = st.st_ino;
        break;
        }
      if( asmincluded[ iinc ].dev == st.st_dev &&
          asmincluded[ iinc ].ino == st.st_ino )
        {
        return;
        }
#endif
      }
    if( LHASM_MAX_INCLUDE == iinc )
      LHASM_FTL( "Too many files included > %d\n", LHASM_MAX_INCLUDE );

    if( NULL == (fd = fopen( incpath, "r" )) )
      {
      LHASM_ERR( "No file %s\n", asmpctx->srcfile );
      }
    }
  asmpctx->srclinenum = 0;
  asmpctx->error = 0;
  asmpctx->plocal = ( (asmflags & LHASM_ALL_LOCAL) || asmlocal[ iinc ] ? &asmlocal[ iinc ] : NULL );
  asmpctx->included = iinc;
  asmpctx->start = asmaddr;
  asmpctx->end = asmaddr;
  asmpctx->length = 0x0;
  asmpctx->ifstate = LHASM_IF_NONE;
  asmpctx->nopstate = LHASM_NOP_NONE;
  asmpctx->exportall = 0;

  asmpctx->error |= setjmp(asmpctx->env);
  if( asmpctx->error )
    {
    if( logfile )
      {
      fprintf( logfd, "%5u\t\t\t    >>> ", asmpctx->srclinenum );
      for( l = 0; l < narg; l++ )
        fprintf( logfd, "%s%c", arg[l], l ? ' ' : '\t' );
      fprintf( logfd, " <<<\n" );
      }
    }

  if( !fd )
    return;

  if( asmrunmacro )
    goto freemacro;

  while( fgets( buffer, sizeof(buffer), fd ) )
    {
    if( asmaddr != resymva )
      {
      PLHU8 presym = lhSymGetName( resym, asmaddr );
      if( presym )
        {
        if( resrcfile && (2 == asmpass || (asmflags) & LHASM_IMMEDIATE) )
          fprintf( resrcfd, "%s:\n", presym );
        resymva = asmaddr;
        }
      }
    if( resrcfile && (2 == asmpass || (asmflags) & LHASM_IMMEDIATE) )
      fputs( buffer, resrcfd );

    asmpctx->srclinenum ++;

    asmbase = asmcurrent;
    nn = 0;

    if( asmpctx->ifstate )
      {
      char *p = lhAsmEatSp( strtok(buffer, "\r\n") );

      if( !p )
        continue;

      if( 0 == strcasecmp( p, ".ELSE" ) )
        {
        if( !(asmpctx->ifstate & 0x10) )
          LHASM_ERR( ".ELSE outside .IF block\n" );
        else if( (asmpctx->ifstate & 0x20) )
          LHASM_ERR( ".ELSE block already open in .IF at line %d\n", asmpctx->iflinenum );
        asmpctx->ifstate ^= (LHASM_IF_TRUE | LHASM_IF_ELSE_FALSE);
        goto ifinlog;
        }
      else if( 0 == strcasecmp( p, ".ENDIF" ) )
        {
        if( !asmpctx->ifstate )
          LHASM_ERR( ".ENDIF outside .IF block\n" );
        asmpctx->ifstate = LHASM_IF_NONE;
        goto ifinlog;
        }
      else if( 0 == strcasecmp( p, ".END" ) )
        goto end;
      if( !(asmpctx->ifstate & 0x1) )
        {
ifinlog:
        if( logfile )
          {
          fprintf( logfd, "%5u\t\t%s\t\t%s\n", asmpctx->srclinenum,
                    asmpctx->ifstate & 1 ? "+TRUE+" : "/false/", p );
          }
        continue;
        }
      }

    if( asmpctx->state == LHASM_MACRO_DEF )
      {
      char *p = lhAsmEatSp( strtok(buffer, "\r\n") );
      if( p && 0 == strcasecmp( p, ".ENDMACRO" ) )
        {
        if( logfile )
          {
          fprintf( logfd, "%5u\t\t\t\t}\n", asmpctx->srclinenum );
          fprintf( logfd, "%5u\t\t\t.ENDMACRO\t; %s\n", asmpctx->srclinenum, pmacro->name );
          }
        if( asmpass == 1 )
          {
          pmacro->thisline = 0;
          pmacro = NULL;
          macronnline = 0;
          }
        asmpctx->state = savestate;
        }
      else if( (pmacro && pmacro->nnline <= LHASM_MAX_LINES_BY_MACRO) && (p && *p != ';') )
        {
        if( asmpass == 1 )
          {
          if( pmacro->nnline == LHASM_MAX_LINES_BY_MACRO )
            LHASM_FTL( "Max lines reached for macro %s\n", pmacro->name );
          if( NULL == (pmacro->lines[pmacro->nnline] = malloc( strlen( p ) + 1 )) )
            LHASM_FTL( "Out of memory while defining macro %s\n", pmacro->name );
          strcpy( pmacro->lines[pmacro->nnline++], p );
          }
        macronnline++;
        if( logfile )
          fprintf( logfd, "%5u\t; %s:%2u\t\t%s\n", asmpctx->srclinenum, pmacro->name, macronnline, p );
        }
      continue;
      }
    else if( asmpctx->state == LHASM_STRUCT_DEF )
      {
      narg = lhAsmParse( buffer, arg, LHASM_MAX_ARGS, 0 );
      if( narg && 0 == strcasecmp( arg[0], ".ENDSTRUCT" ) )
        {
        if( logfile )
          {
          fprintf( logfd, "%5u\t\t\t\t}\n", asmpctx->srclinenum );
          fprintf( logfd, "%5u\t\t\t.ENDSTRUCT\t; %04X %s\n",
                   asmpctx->srclinenum, pstruct->size, pstruct->name );
          }
        if( asmpass == 1 )
          {
          pstruct = NULL;
          structnnfield = 0;
          }
        asmpctx->state = savestate;
        }
      else if( (pstruct && pstruct->nnfield <= LHASM_MAX_FIELDS_BY_STRUCT) && (narg && *arg[0] != ';') )
        {
        int nfarg;
        LHU16 field, frag, size = 1, array = 1;
        char *pf;
        struct s_lhasmstruct *ps = NULL;

        for( nfarg = 0; nfarg < narg && *arg[nfarg] != ';'; nfarg++ )
          ;
        if( nfarg < 2 )
          LHASM_FTL( "Bad field specified\n" );
        if( (0 == strcasecmp( arg[1], "byte" ) && (field = 1) && (frag = FRAG_BYTE)) ||
            (0 == strcasecmp( arg[1], "word" ) && (field = 2) && (frag = FRAG_WORD)) ||
            (0 == strcasecmp( arg[1], "long" ) && (field = 4) && (frag = FRAG_LONG)) ||
            (0 == strcasecmp( arg[1], "text" ) && (field = 1) && (frag = FRAG_TEXT)) ||
            (0 == strncasecmp( arg[1], "struct'", 7 ) && (field = 0xFFFF) && (frag = FRAG_STRUCT)) )
          {
          if( nfarg >= 3 )
            {
            int r;

            asmflags |= LHASM_OP_SYM_EXIST;
            r = lhAsmGetLHUValue( arg[2], arg[2], &v, LINK_MEM_16, 1, 0 );
            asmflags &= ~LHASM_OP_SYM_EXIST;
            if( r )
              array = v;
            else
              LHASM_ERR( "Badly formed parameter %s\n", arg[2] );
            }
          if( 0xFFFF == field )
            {
            int ist;

            pf = arg[1] + 7;
            for( ist = 0; ist < (asmnnstruct - 1); ist++ )
              if( (0 == strcmp( asmstruct[ist].name, pf )) && (ps = &asmstruct[ist]) )
                break;
            if( !ps )
              LHASM_FTL( "Unkown STRUCT %s\n", pf );
            size = ps->size;
            field = 1;
            }
          if( asmpass == 1 )
            {
            if( pstruct->nnfield == LHASM_MAX_FIELDS_BY_STRUCT )
              LHASM_FTL( "Max fields reached for struct %s\n", pstruct->name );
            pstruct->fields[pstruct->nnfield].offset = pstruct->size;
            pstruct->fields[pstruct->nnfield].type = field;
            pstruct->fields[pstruct->nnfield].element = size * field;
            pstruct->fields[pstruct->nnfield].array = array;
            pstruct->fields[pstruct->nnfield].pstruct = ps;
            pstruct->fields[pstruct->nnfield].fragment = frag;
            pstruct->size += pstruct->fields[pstruct->nnfield].size = array * size * field;
            strcpy( pstruct->fields[pstruct->nnfield++].name, arg[0] );
            }
          }
        structnnfield++;
        if( logfile )
          fprintf( logfd, "%5u\t; %s:%2u\t\t+%04X:%04X.%04X.%04X.%04X %s\t{%s%s * #%u}\n",
                   asmpctx->srclinenum, pstruct->name, structnnfield,
                   pstruct->fields[structnnfield - 1].offset,
                   pstruct->fields[structnnfield - 1].size,
                   pstruct->fields[structnnfield - 1].type,
                   pstruct->fields[structnnfield - 1].element,
                   pstruct->fields[structnnfield - 1].array,
                   pstruct->fields[structnnfield - 1].name,
                   (FRAG_STRUCT == pstruct->fields[structnnfield - 1].fragment ? "struct'" : ""),
                   ( FRAG_BYTE == (frag = pstruct->fields[structnnfield - 1].fragment) ?
                       "byte" :
                       (FRAG_WORD == frag ?
                          "word" :
                          (FRAG_LONG == frag ?
                             "long" :
                             (FRAG_TEXT == frag ?
                                "text" :
                                (FRAG_STRUCT == frag ?
                                   pstruct->fields[structnnfield - 1].pstruct->name :
                                   "????")))) ),
                   pstruct->fields[structnnfield - 1].array);
        }
      continue;
      }
    else if( asmpctx->state == LHASM_DATAINIT_DEF )
      {
      narg = lhAsmParse( buffer, arg, LHASM_MAX_ARGS, 0 );
      if( narg && 0 == strcasecmp( arg[0], ".ENDDATA" ) )
        {
        while( datann++ < pdata->size )
          {
          *(asmcurrent++) = thisdata;
          }
        nn = asmcurrent - asmbase;
        if( nn )
          arg[0] = "...";
        asmpctx->state = LHASM_DATAEND_DEF;
        }
      else
        {
        if( *arg[0] != ';' && datann >= pdata->size )
          LHASM_ERR( "Initialization outside STRUCT %s for DATA %s\n",
                     pdata->pstruct->name, pdata->name );
        for( i = 0; i < narg && datann < pdata->size && *arg[i] != ';'; i++, datann++ )
          {
          int ic, il;

          if( *arg[i] == '"' && arg[i][il = (strlen(arg[i]) - 1)] == '"' )
            {
            for( ic = 1 ; (ic < il) && (datann <pdata->size) ; ic++, datann++ )
              *(asmcurrent++) = thisdata = arg[i][ic];
            datann--;
            }
          else if( lhAsmGetLHUValue( arg[i], arg[i], &thisdata, LINK_MEM_8, 0, asmcurrent - asmbase + 1 ) )
            *(asmcurrent++) = thisdata;
          else
            LHASM_ERR( "Badly formed byte expression '%s'\n", arg[i] );
          }
        nn = asmcurrent - asmbase;
        }
      goto inlog;
      }

runmacro:
    if( asmrunmacro )
      strcpy( buffer, asmrunmacro->lines[asmrunmacro->thisline++] );

    narg = lhAsmParse( buffer, arg, LHASM_MAX_ARGS, (asmpctx->state == LHASM_BASIC && (!asmpinlin || strstr( buffer, "\\]end" ))) );

    for( i = 0; asmrunmacro && i < narg && *arg[i] != ';'; i++ )
      if( 0 == strncmp( arg[i], "__#", 3 ) &&
          1 == sscanf( arg[i] + 3, "%hu", &v ) && v < LHASM_MAX_ARG_BY_MACRO )
        arg[i] = asmrunmacro->arg[v];

    if( asmflags & LHASM_VERBOSE )
      {
      printf( "%3u+%s\t%5u",
              asmnnctx, "++++++++++" + 10 - asmnnctx, asmctx[asmnnctx].srclinenum );
      for( i = 0; i < narg; i++ )
        printf( "%c%s", i > 1 ? ' ' : '\t', arg[i] );
      printf( "\n" );
      }

    if( arg[0] == NULL || *arg[0] == 0 )
      goto freemacro;

    if( *arg[0] == ';' )
      {
      if( logfd && !(asmflags & LHASM_NOCOMMENT) )
        {
        fprintf( logfd, "%5u\t%04hX\t", asmpctx->srclinenum, lhAsmGetPC() );
        fprintf( logfd, "\t" );
        for( l = 0; l < narg; l++ )
          fprintf( logfd, "%s%c", arg[l], l ? ' ' : '\t' );
        fprintf(logfd, "\n");
        }
      goto freemacro;
      }

    if( asmrunmacro )
      goto nextmacro;

    if( (asmflags) & LHASM_IMMEDIATE )
      {
      if( 0 == strcmp( arg[0], ".!" ) )
        {
        fprintf( stderr, "%5u\t%04hX\t%c%s\n",
                 asmpctx->srclinenum, lhAsmGetPC(), fragment ? '.' : '\0', fragment ? fragment : "" );
        continue;
        }
#if 0
      else if( 0 == strcasecmp( arg[0], ".NOERROR" ) )
        {
        fprintf( stderr, "%5u\t%04hX\t; Ignore ERROR (-E)\n", asmpctx->srclinenum, lhAsmGetPC() );
        asmflags |= LHASM_IGNORE_ERROR;
        continue;
        }
#endif
      else if( 0 == strcasecmp( arg[0], ".NOOUTPUT" ) )
        {
        fprintf( stderr, "%5u\t%04hX\t; Do not output file (-N)\n", asmpctx->srclinenum, lhAsmGetPC() );
        asmflags |= LHASM_NOBINARY;
        continue;
        }
      }
    if( 0 == strcasecmp( arg[0], ".ORIGIN:" ) && arg[1] &&
        (0xFFFF != (v = lhAsmGetSymValue( asmsym, arg[1] )) || 
         (1 == sscanf( arg[1], "&%hx", &v )) || (1 == sscanf( arg[1], "\\x%hx", &v )) ||
         (1 == sscanf( arg[1], "#%hu", &v )) || (1 == sscanf( arg[1], "\\d%hu", &v )) ||
         (1 == sscanf( arg[1], "%hx", &v ))) )
      {
      if( asmtop != 0x7FFFFFFF && asmorg != 0x0FFFFF && (v < asmorg || v < lhAsmGetPC()) )
        LHASM_ERR( "Origin set is lower than current origin %04X or current address %04X\n",
                   asmorg, lhAsmGetPC() );
      lhAsmSetOrigin( v, arg[0], 0 );
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".ALIGN:" ) && arg[1] && 1 == sscanf( arg[1], "%hx", &v ) )
      {
      LHU16 oldorg = lhAsmGetPC();
      LHU16 offorg = 0;

      if( narg >= 3 )
        {
        if( 1 == sscanf( arg[2], "-%hx", &offorg ) )
          offorg = -offorg;
        else if( 1 != sscanf( arg[2], "+%hx", &offorg ) )
          LHASM_ERR( "Badly formed parameter '%s'\n", arg[2] );
        }

      if( asmtop == 0x7FFFFFFF || asmorg == 0x0FFFFF )
        LHASM_FTL( "Can not align. Origin not set\n", asmorg, lhAsmGetPC() );

      asmorg = asmaddr = (((lhAsmGetPC() + v - 1) & ~(v - 1)) + offorg) & 0xFFFF;
      if( asmorg < oldorg )
        {
        if( logfile )
          {
          fprintf( logfd, "%5u\n\t\t.ALIGN: Code overlaps. Skiping to next %04hX\n",
                   asmpctx->srclinenum, v );
          }
        asmorg = asmaddr = (((lhAsmGetPC() + (2*v) - 1) & ~(v - 1)) + offorg) & 0xFFFF;
        }
      if( logfile )
        {
        fprintf( logfd, "%5u\n\t\t%s\t%04X\n", asmpctx->srclinenum, arg[0], asmaddr );
        }
      for( ; oldorg < asmorg; oldorg++ )
        asmmem[oldorg] = 0x00;
      if( asmtop > asmorg )
        asmtop = asmorg;
      if( asmbottom < lhAsmGetPC() )
        asmbottom = lhAsmGetPC();
      asmcurrent = &asmmem[asmorg];
      if( asmpctx->start == 0xFFFF )
        asmpctx->start = asmpctx->end = asmorg;
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".COMMENT:" ) )
      {
      if( logfile )
        {
        fprintf( logfd, ";\t" );
        for( i = 1; i < narg; i++ )
          fprintf( logfd, "%s ", arg[i] );
        fprintf( logfd, "\n" );
        }
      if( frgfile && !comment )
        {
        fprintf( frgfd, comment = "\tCOMMENT\t" );
        for( i = 1; i < narg; i++ )
          fprintf( frgfd, "%s ", arg[i] );
        fprintf( frgfd, "\n" );
        }
      continue;
      }
    else if( ( '.' == *arg[0] ) && (fidx = lhFragmentGetIndex( arg[0] + 1 )) )
      { 
      asmpctx->state = fidx;
      fragment = lhFragmentGetFragmentFromIndex( fidx );
      comment = NULL;
      lhAsmNewFragment( fidx, fragment, NULL, NULL );
      if( FRAGIDX_KEYWORD == fidx )
        {
        if( (lhAsmGetPC() & 0x7FF) == 0x054 )
          {
          char kywna[80];

          asmmem[(lhAsmGetPC() & 0xF800)] = 0x55; 
          kytab = (LHU8 *)&asmmem[(lhAsmGetPC() & 0xF800) | 0x54];
          kyhash = (LHU8 *)&asmmem[(lhAsmGetPC() & 0xF800) | 0x20];
          memset( (void *)kyhash, kych = 0, sizeof(LHU16) * 26 );
          *(asmcurrent++) = 0xD0;
          asmaddr++;
          asmpctx->state = fidx;
          asmflags |= LHASM_OP_HAS_KEYWORD;

          snprintf( kywna, sizeof(kywna), "%s$$._keyword_start", asmpctx->srcfile );
          if( asmpass == 1 )
            asmincsym = lhSymAddSorted( asmincsym, kywna, lhAsmGetPC(), 0 );
          else
            {
            if( lhAsmGetPC() != lhSymGetValue( asmincsym, kywna ) )
              LHASM_FTL( "Assembler confused. KEYWORD_START from %s differs between passes\n",
                         asmpctx->srcfile );
            }
          if( logfile )
            fprintf( logfd, "\t\t;; %04X\t%s\n", lhAsmGetPC(), kywna );
          }
        else
          LHASM_ERR( "Badly address for keyword table: %04X\n", lhAsmGetPC() );
        }
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".DEFINE:" ) && narg >= 4 &&
             0 == strcmp( arg[2], "=" ) && strlen(arg[1]) < 17 )
      {
      LHU8 *pbits, bits;
      LHU16 code, jump;
      char *p;

      strncpy( memset( basic, 0, sizeof(basic) ), arg[1] + 1, strlen(arg[1] + 1) - 1);
      if( asmpass == 1 && lhBasicGetCode( basic ) )
        LHASM_ERR( "Keyword \"%s\" is already defined\n", basic );
      if( narg >= 5 )
        bits = toupper( *arg[4] );
      else
        {
        bits = 'N';
        LHASM_LOW( ".DEFINE: \"%s\" Old format detected, using %c for bits value\n", basic, bits );
        }

      jump = lhAsmGetPC();
      code = 0;
      if( 0 == strncasecmp( arg[3], "AUTO.", 5 ) &&
          (p = strchr( "IFV", arg[3][5] )) )
        {
        if( arg[3][6] && ('?' == arg[3][6] && (0 == sscanf( arg[3] + 6, "?%hx", &code ))) )
          LHASM_ERR( "Invalid keyword code expression %s\n", arg[3] );
        if( !code || lhBasicGetBasic( code ) )
          {
          if( asmpass == 1 )
            {
            LHU16 lastcode;
            switch( *p )
              {
              case 'I': code = 0xF080; lastcode = 0xF0FF; break;
              case 'F': code = 0xF060; lastcode = 0xF07F; break;
              case 'V': code = 0xF020; lastcode = 0xF05F; break;
              default:
                code = 0; lastcode = 0;
                LHASM_ERR( "Unknown AUTO.%c code\n", *p );
                break;
              }
            for( ; code <= lastcode; code++ )
              {
              if( NULL == lhBasicGetBasic( code ) )
                break;
              }
            if( code == lastcode )
              code = 0;
            }
          else
            code = lhBasicGetCode( basic );
          }
        }
      else if( 1 == sscanf( arg[3], "%hx", &code ) )
        ;
      else
        code = 0;
      if( (pbits = strchr( LHBASIC_BITS, bits )) && (code >= 0xE000) )
        {
        if( asmpass == 1 )
          lhBasicRegisterOneKeyword( basic, code, jump, *pbits, lhAsmRegisterKeywordCB );
        else
          lhAsmRegisterKeywordCB( basic, code, jump, *pbits );
        if( keywfile &&
            //(LHASM_KEYWORD_EXPORT == (asmflags & (LHASM_OP_HAS_KEYWORD | LHASM_KEYWORD_EXPORT))) )
            (LHASM_KEYWORD_EXPORT == (asmflags & (LHASM_KEYWORD_EXPORT))) )
          {
          if( asmflags & LHASM_KEYWORD_OLD )
            fprintf( keywfd, "\tD%X\t\"%s\"\t%04hX %04hX\n",
                             strlen(basic), basic, code, jump );
          else
            fprintf( keywfd, "\t\"%s\"\t%04hX %04hX %c\n",
                     basic, code, jump, *pbits );
          }
        }
      else
        LHASM_ERR( "Invalid keyword definition\n" );
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".SUBSTITUTE:" ) && narg >= 4 &&
             0 == strcmp( arg[2], "=" ) )
      {
      struct s_lhasmsubs *psub, *ptail;

      for( psub = asmsubs; psub; psub = psub->next )
        if( 0 == strcmp( psub->name, arg[1] ) )
          {
          if( 1 == asmpass && !(asmflags & (LHASM_JR_TO_JP|LHASM_JP_DO_OPTIMIZE)) )
            LHASM_ERR( "Substitute string '%s' is already defined.\n", arg[1] );
          else if( strcmp( psub->body, arg[3] ) )
             LHASM_FTL( "Assembler confused: Substitute string '%s' differs between passes\n",
                        arg[1] );
          }
      if( 1 == asmpass )
        {
        if( NULL == (psub = (struct s_lhasmsubs *)malloc( sizeof(struct s_lhasmsubs) )) ||
            NULL == (psub->name = malloc( strlen(arg[1]) + 1 )) ||
            NULL == (psub->body = malloc( strlen(arg[3]) + 1 )) )
          LHASM_FTL( "Out of memory while defining substitute string '%s'\n", arg[1] );
       strcpy( psub->name, arg[1] );
       strcpy( psub->body, arg[3] );
       psub->next = asmsubs;
       asmsubs = psub;
       }
     continue;
     }
    else if( narg > 3 &&
            (0 == strcasecmp( arg[0], ".FILLTO:" ) ||
             0 == strcasecmp( arg[0], ".FILLALIGN:" ) ||
             0 == strcasecmp( arg[0], ".FILL:" )) &&
            0 == strcasecmp( arg[2], "WITH" ) )
      {
      LHU16 ea, b[16], nb, ib;

      if(0xFFFF == (ea = lhAsmGetSymValue( asmsym, arg[1] )) && (1 != sscanf( arg[1], "%hx", &ea )) )
        LHASM_ERR( "Unknown size %s for %s\n", arg[1], arg[0] );
      for( nb = 0, ib = 3; ib < narg && ';' != *arg[ib] ; nb++, ib++ )
        if( 0 == lhAsmGetLHUValue( arg[ib], arg[ib], &b[nb], LINK_MEM_8, 0, 0 ) )
          LHASM_ERR( "Expected byte value %s\n", arg[ib] );
      if( 0 == strcasecmp( arg[0], ".FILL:" ) )
        ea = (lhAsmGetPC() + (ea * nb));
      else if( 0 == strcasecmp( arg[0], ".FILLALIGN:" ) )
        ea = (lhAsmGetPC() + (ea - 1)) & ~(ea - 1);
      for( nn = 0; (asmaddr + nn) < ea; nn++ )
        *(asmcurrent++) = b[(nn % nb)];
      goto inlog;
      }
    else if( (0 == strcasecmp( ".SKIP:", arg[0] ) ||
              0 == strcasecmp( ".SKIP", arg[0] )) && narg >= 2 )
      {
      if( 1 != sscanf( arg[1], "%hx", &v ) )
        LHASM_ERR( "Badly formed parameter '%s'\n", arg[1] );
      asmcurrent += v;
      asmaddr += v;
      goto inlog;
      }
    else if( 0 == strcasecmp( ".JUMPTO:", arg[0] ) && narg >= 2 )
      {
      if( !lhAsmGetLHUValue( arg[1], arg[1], &v, LINK_MEM_16, 0, 0 ) )
        LHASM_ERR( "Badly formed parameter '%s'\n", arg[1] );
      if( v < asmaddr )
        LHASM_FTL( ".JUMPTO goes backward: current %04hX jump to %04hX\n", asmaddr, v );
      asmcurrent += (v - asmaddr);
      asmaddr = v;
      goto inlog;
      }
    else if( (0 == (v = strcasecmp( arg[0], ".DATESTAMP" ))) ||
             (0 == strcasecmp( arg[0], ".TIMESTAMP" )) )
      {
      LHU16 b1, b2, b3;
      LHU8 tstamp = v;

      if( v )
        {
        b1 = asmtm.tm_hour;
        b2 = asmtm.tm_min;
        b3 = asmtm.tm_sec;
        }
      else
        {
        b1 = asmtm.tm_year % 100;
        b2 = asmtm.tm_mon + 1;
        b3 = asmtm.tm_mday;
        }
      nn = 0;
      asmbase = asmcurrent;
      if( arg[1] )
        {
        if( lhAsmGetLHUValue( arg[1], arg[1], &v, LINK_MEM_8, 0, 0 ) )
          {
          *(asmcurrent++) = v;
          nn++;
          }
        else
          LHASM_ERR( "Badly formed expression '%s'\n", arg[1] );
        }
      lhAsmBCD( b1, &b1, LINK_MEM_8 );
      *(asmcurrent++) = b1;
      lhAsmBCD( b2, &b2, LINK_MEM_8 );
      *(asmcurrent++) = b2;
      lhAsmBCD( b3, &b3, LINK_MEM_8 );
      *(asmcurrent++) = b3;
      nn += 3;
      if( logfd )
        fprintf( logfd, "%5u\t\t\t; %sSTAMP AT %04hX >> %2hx%c%02hx%c%02hx\n",
                 asmpctx->srclinenum, tstamp ? "TIME" : "DATE", lhAsmGetPC() + (nn > 3),
                 b1, tstamp ? ':' : '/', b2, tstamp ? ':' : '/', b3 );
      if( (asmpass == 2) || (asmflags & LHASM_IMMEDIATE) )
        printf( "%s %sSTAMP %2hx%c%02hx%c%02hx\n",
                *outfile ? outfile : (asmflags & LHASM_IMMEDIATE) ? "stdin.bin" : "(no output)",
                 tstamp ? "TIME" : "DATE",
                 b1, tstamp ? ':' : '/', b2, tstamp ? ':' : '/', b3 );

      goto inlog;
      }
    else if( 0 == strcasecmp( arg[0], ".CHECKSUM" ) )
      {
      LHU16 cks = 0, ncks, cksc = 0xFFFF, ckstart, ckend;
      char cksna[80];

      nn = 0;
      asmbase = asmcurrent;
      if( arg[1] ) {
        if( 0 == strcmp( arg[1], "()" ) )
          ;
        else if( ('+' == *arg[1]) && lhAsmGetLHUValue( arg[1] + 1, arg[1] + 1, &v, LINK_MEM_8, 0, 0 ) )
          {
          *(asmcurrent++) = v;
          nn++;
          }
        else if( lhAsmGetLHUValue( arg[1], arg[1], &v, LINK_MEM_8, 0, 0 ) )
          {
          cksc = v;
          }
        else
          LHASM_ERR( "Badly formed expression '%s'\n", arg[1] );
      }
      if( asmtop > asmorg )
        asmtop = asmorg;
      if( asmbottom < (lhAsmGetPC() + nn) )
        asmbottom = (lhAsmGetPC() + nn);
      ckstart = asmtop;
      ckend = asmbottom;
      if( narg >= 3 )
        {
        if( lhAsmGetLHUValue( arg[2], arg[2], &v, LINK_MEM_16, lhAsmGetPC(), 0 ) )
          ckstart = v;
        else
          if( asmpass == 2 )
            LHASM_ERR( "Badly formed expression '%s'\n", arg[2] );
        }
      if( narg >= 4 )
        {
        if( strcmp( arg[3], "." ) == 0 )
          ckend = (lhAsmGetPC() + nn);
        else if( lhAsmGetLHUValue( arg[3], arg[3], &v, LINK_MEM_16, (lhAsmGetPC() + nn), 0 ) &&
            v <= (lhAsmGetPC() + nn) )
          ckend = v;
        else
          if( asmpass == 2 )
            LHASM_ERR( "Badly formed expression '%s'\n", arg[3] );
        }
      for( ncks = ckstart; ncks < ckend; ncks++ )
        cks += asmmem[ncks];

      if( cksc != 0xFFFF )
        {
        *(asmcurrent++) = cksc;
        nn++;
        }
      *(asmcurrent++) = (cks & 0xFF00) >> 8;
      *(asmcurrent++) = cks & 0xFF;
      nn += 2;
      if( logfd )
        fprintf( logfd, "%5u\t\t\t; CHECKSUM AT %04hX (%04hX:%04hX) = %04hX\n",
                 asmpctx->srclinenum, lhAsmGetPC() + (nn > 2), ckstart, ckend, cks );
      if( (asmpass == 2) || (asmflags & LHASM_IMMEDIATE) )
        printf( "%s %5u bytes, checksum %04hX (%04hX:%04hX)\n",
                *outfile ? outfile : (asmflags & LHASM_IMMEDIATE) ? "stdin.bin" : "(no output)",
                ckend - ckstart, cks, ckstart, ckend );

      snprintf( cksna, sizeof(cksna), "%s$$._chksm", asmpctx[0].srcfile );
      if( asmpass == 2 )
        asmincsym = lhSymAddSorted( asmincsym, cksna, cks, 0 );
      else
        {
        if( 0xFFFF != lhSymGetValue( asmincsym, cksna ) )
          LHASM_ERR( "__MAIN__$$._chksm is already defined.\n" );
        }
      goto inlog;
      }
    else if( 0 == strcasecmp( arg[0], ".CHECKSUM:" ) )
      continue;
    else if( 0 == strcasecmp( arg[0], ".FULLCHECKSUM:" ) )
      continue;
    else if( 0 == strcasecmp( arg[0], ".DECLARE:" ) )
      continue;
    else if( 0 == strcasecmp( arg[0], ".EXPORTALL" ) )
      {
      asmpctx->exportall = 1;
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".LOCAL" ) )
      {
      char locsymna[80];

      snprintf( locsymna, sizeof( locsymna ), "   $$[.LOCAL] %03u %s",
                asmpctx->included, asmpctx->srcfile );

      if( asmpass == 1)
        {
        if( !(asmflags & LHASM_ALL_LOCAL) && asmpctx->plocal && asmpass == 1 )
          {
          LHASM_ERR( "Symbols are already set to local\n" );
          }

        asmpctx->plocal = &asmlocal[ asmpctx->included ];
        *asmpctx->plocal = lhSymAddSorted( *asmpctx->plocal,
                                           locsymna, (LHU16)asmpctx->included, SYM_NONE );
        }
      else
        {
        if( asmpctx->plocal != &asmlocal[ asmpctx->included ] ||
            strcmp( locsymna, (*asmpctx->plocal)->name ) ||
            (LHU16)asmpctx->included != (*asmpctx->plocal)->value )
          LHASM_FTL( "Assembler confused: Local symbol '%s' differs between passes\n",
                     locsymna );
        (*asmpctx->plocal)->value = 0xFFFF;
        }
      continue;
      }
    else if( 0 == strcasecmp( arg[0], LHSYM_MAGIC ) )
      {
      asmpctx->state = LHASM_SYMBOLS;
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".MACRO:" ) && narg >= 2 )
      {
      if( asmnnmacro == LHASM_MAX_MACRO )
        LHASM_ERR( "Macro table is full\n" );
      for( i = 0; i < asmnnmacro; i++ )
        if( 0 == strcmp( asmmacro[i].name, arg[1] ) )
          pmacro = &asmmacro[i];
      if( asmpass == 1 )
        {
	if( pmacro )
          LHASM_ERR( "Macro %s is already defined\n", arg[1] );
        pmacro = &asmmacro[asmnnmacro++];
        pmacro->nnline = 0;
        strcpy( pmacro->name, arg[1] );
        }
      else
        {
        if( !pmacro )
          LHASM_ERR( "Macro %s is never defined\n", arg[1] );
        }
      savestate = asmpctx->state;
      asmpctx->state = LHASM_MACRO_DEF;
      macronnline = 0;
      if( logfile )
        {
        fprintf( logfd, "%5u\t\t\t.MACRO:\t%s\n", asmpctx->srclinenum, arg[1] );
        fprintf( logfd, "%5u\t\t\t\t{\n", asmpctx->srclinenum );
        }
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".STRUCT:" ) && narg >= 2 )
      {
      if( asmnnstruct == LHASM_MAX_STRUCT )
        LHASM_ERR( "Structure table is full\n" );
      for( i = 0; i < asmnnstruct; i++ )
        if( 0 == strcmp( asmstruct[i].name, arg[1] ) )
          pstruct = &asmstruct[i];
      if( asmpass == 1 )
        {
	if( pstruct )
          LHASM_ERR( "Structure %s is already defined\n", arg[1] );
        pstruct = &asmstruct[asmnnstruct++];
        pstruct->size = pstruct->nnfield = 0;
        strcpy( pstruct->name, arg[1] );
        }
      else
        {
        if( !pstruct )
          LHASM_ERR( "Structure %s is never defined\n", arg[1] );
        }
      savestate = asmpctx->state;
      asmpctx->state = LHASM_STRUCT_DEF;
      structnnfield = 0;
      if( logfile )
        {
        fprintf( logfd, "%5u\t\t\t.STRUCT:\t%s\n", asmpctx->srclinenum, arg[1] );
        fprintf( logfd, "%5u\t\t\t\t{\n", asmpctx->srclinenum );
        }
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".DATA:" ) && narg >= 4 &&
             0 == strcasecmp( arg[2], "STRUCT" ) )
      {
      //struct s_lhasmdata *pdata;
      LHU16 array = 1;
      int initargc = 0;

      if( asmnndata == LHASM_MAX_DATA )
        LHASM_ERR( "Data area table is full\n" );
      for( pstruct = NULL, i = 0; i < asmnnstruct; i++ )
        if( 0 == strcmp( asmstruct[i].name, arg[3] ) )
          pstruct = &asmstruct[i];
      if( !pstruct )
        LHASM_ERR( "Structure %s is not defined\n", arg[3] );
      for( pdata = NULL, i = 0; i < asmnndata; i++ )
        if( 0 == strcmp( asmdata[i].name, arg[1] ) )
          pdata = &asmdata[i];
      if( narg >= 5 && strcasecmp( arg[4], "INIT" ) )
        {
        asmflags |= LHASM_OP_SYM_EXIST;
        if( lhAsmGetLHUValue( arg[4], arg[4], &v, LINK_MEM_16, 0, 0 ) )
          array = v;
        else
          LHASM_ERR( "Badly formed parameter %s\n", arg[4] );
        asmflags &= ~LHASM_OP_SYM_EXIST;
        initargc = 1;
        }
      if( narg >= (5 + initargc) && 0 == strcasecmp( arg[4 + initargc], "INIT" ) )
        {
        savestate = asmpctx->state;
        asmpctx->state = LHASM_DATAINIT_DEF;
        }
      if( asmtop == 0x7FFFFFFF || asmorg == 0x0FFFFF )
        lhAsmSetOrigin( asmaddr, ".ORIGIN:", 0 );
      if( asmpass == 1 )
       {
        if( pdata )
          LHASM_ERR( "Data area %s is already defined\n", arg[1] );
        pdata = &asmdata[asmnndata++];
        pdata->base = lhAsmGetPC();
        pdata->end = lhAsmGetPC() + pstruct->size;
        pdata->array = array;
        pdata->element = pstruct->size;
        pdata->size = pdata->array * pdata->element;
        pdata->pstruct = pstruct;
        strcpy( pdata->name, arg[1] );
        asmsym = lhAsmAddSymSorted( asmsym, pdata->name, asmaddr, SYM_NONE, (export|asmpctx->exportall));
        }
      else
        {
        if( !pdata )
          LHASM_ERR( "Data area %s is never defined\n", arg[1] );
        }
      if( logfile )
        {
        fprintf( logfd, "%5u\t\t\t.DATA:\t%s\n", asmpctx->srclinenum, arg[1] );
        fprintf( logfd, "%5u\t%04hX\t\t\t{\n", asmpctx->srclinenum, lhAsmGetPC() );
        }
      memset( asmcurrent, 0, pdata->size );
      lhAsmBuildData( pdata );
      if( asmpctx->state == LHASM_DATAINIT_DEF )
        {
        datann = 0;
        asmaddr -= nn;
        nn = 0;
        }
      else
        asmcurrent += (nn = pdata->size);
      if( logfile )
        {
        fprintf( logfd, "%5u\t%04hX\t\t\t}\t; %04X.%04X.%04X %s\n",
                 asmpctx->srclinenum, lhAsmGetPC(),
                 pdata->size, pdata->element, pdata->array, pdata->name );
        if( asmpctx->state == LHASM_DATAINIT_DEF )
          fprintf( logfd, "%5u\t\t\tINIT\n\t\t\t\t{\n", asmpctx->srclinenum );
        }
      if( asmpctx->state != LHASM_DATAINIT_DEF )
        pdata = NULL;
      goto inlog;
      }
    else if( 0 == strcasecmp( arg[0], ".INCLUDE:" ) && narg >= 2 )
      {
      if( asmnnctx < LHASM_MAX_CTX )
        {
        asmnnctx++;
        asmctx[asmnnctx].srcfile = arg[1];
        asmctx[asmnnctx].state = asmpctx->state;
        if( logfile )
          {
          fprintf( logfd, "%5u\t\t\t.INCLUDE:\t%s\n", asmpctx->srclinenum, asmctx[asmnnctx].srcfile );
          fprintf( logfd, "\n%5u\t; %-10s%3u\tEntering file %s\n",
                    asmpctx->srclinenum, "++++++++++" + 10 - asmnnctx, asmnnctx, asmctx[asmnnctx].srcfile );
          }
        asmpctx = &asmctx[asmnnctx];
        asmpctx->nndostack = asmnndostack;
        if( 0 == setjmp(asmpctx->env) )
          lhAsmDoInclude( arg[1] );
        asmctx[--asmnnctx].error |= asmpctx->error;
        asmpctx = &asmctx[asmnnctx];
        if( logfile )
          {
          fprintf( logfd, "\n%5u\t; %-10s%3u\tResuming file %s\n",
                    asmpctx->srclinenum, "++++++++++" + 10 - asmnnctx, asmnnctx, asmctx[asmnnctx].srcfile );
          }
        }
      else
        LHASM_ERR( "Too many files in stack\n" );
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".IMPORT:" ) && narg >= 2 )
      {
      if( 1 == asmpass )
        {
        int ret;

        if( -1 == (ret = lhSymImportSymbol( arg[1], &asmsym, SYM_NONE )) )
          LHASM_FTL( "Unable to import symbol file '%s'\n", arg[1] );
        else if( -2 == ret )
          {
          if( -1 == (ret = lhBasicImportKeyword( arg[1], lhAsmRegisterKeywordCB )) )
            LHASM_FTL( "Unable to import keyword file '%s'\n", arg[1] );
          else if( -2 == ret )
            {
            LHASM_FTL( "Imported file '%s' is not understood\n", arg[1] );
            }
          }
        }
      if( logfile )
        {
        fprintf( logfd, "%5u\t\t\t.IMPORT:\t%s\n", asmpctx->srclinenum, arg[1] );
        }
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".PRINT" ) ||
             0 == strcasecmp( arg[0], ".PRINT2" ) ||
             0 == strcasecmp( arg[0], ".DEBUG" ) ||
             0 == strcasecmp( arg[0], ".DEBUG2" ) )
      {
      if( logfile )
        fprintf( logfd, "%5u\t%04X\t",
                    asmpctx->srclinenum, lhAsmGetPC() );
      if( 0 == strcasecmp( arg[0], ".DEBUG" ) && !(asmflags & LHASM_DEBUG) )
        continue;
      if( 0 == strcasecmp( arg[0], ".DEBUG2" ) && !(asmflags & LHASM_DEBUG) && (2 != asmpass) )
        continue;
      if( 0 == strcasecmp( arg[0], ".PRINT2" ) && (2 != asmpass) )
        continue;
      asmflags |= LHASM_OP_SYM_EXIST;
      for( i = 1; i < narg && *arg[i] != ';'; i++ )
        if( *arg[i] == '"' )
          {
          fwrite( arg[i] + 1, strlen( arg[i] + 1 ) - 1, 1, stdout ); 
          if( logfile )
            fwrite( arg[i] + 1, strlen( arg[i] + 1 ) - 1, 1, logfd ); 
          }
        else if( 0 == strncasecmp( "value8of'", arg[i], 9 ) &&
                 lhAsmGetLHUValue( arg[i] + 9, arg[i] + 9, &v, LINK_MEM_8, 0, 0 ) )
          {
          fprintf( stdout, "%02X", v );
          if( logfile )
            fprintf( logfd, "%02X", v );
          }
        else if( lhAsmGetLHUValue( arg[i], arg[i], &v, LINK_MEM_16, 0, 0 ) )
          {
          fprintf( stdout, "%04X", v );
          if( logfile )
            fprintf( logfd, "%04X", v );
          }
        else
          {
          fprintf( stdout, "%s", arg[i] );
          if( logfile )
            fprintf( logfd, "%s", arg[i] );
          }
      fputc( '\n', stdout );
      fflush( stdout );
      if( logfile )
        {
        fputc( '\n', logfd );
        fflush( logfd );
        }
      asmflags &= ~LHASM_OP_SYM_EXIST;
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".FATAL" ) )
      {
      LHASM_FTL( "%s\n", arg[1] ? arg[1] : "" );
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".ERROR" ) )
      {
      LHASM_ERR( "%s\n", arg[1] ? arg[1] : "" );
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".WARNING" ) )
      {
      LHASM_WRN( "%s\n", arg[1] ? arg[1] : "" );
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".IF" ) )
      {
      LHU8 cond;

      if( asmpctx->ifstate )
        LHASM_ERR( ".IF block already open at line %d\n", asmpctx->iflinenum );
      if( lhAsmCheckIfCondition( &arg[1], narg - 1, &cond ) )
        {
        asmpctx->ifstate = cond ? LHASM_IF_TRUE : LHASM_IF_FALSE;
        asmpctx->iflinenum = asmpctx->srclinenum;
        }
      else
        LHASM_ERR( "Invalid condition for .IF\n" );
      continue;
      }
    else if( 2 <= narg && 0 == strcasecmp( arg[0], ".NOP" ) && 0 == strcasecmp( arg[1], "IF" ) )
      {
      LHU8 cond;

      if( asmpctx->nopstate )
        LHASM_ERR( ".NOP block already open at line %d\n", asmpctx->noplinenum );
      if( lhAsmCheckIfCondition( &arg[2], narg - 2, &cond ) )
        {
        asmpctx->nopstate = cond ? LHASM_NOP_TRUE : LHASM_NOP_FALSE;
        asmpctx->noplinenum = asmpctx->srclinenum;
        }
      else
        LHASM_ERR( "Invalid condition for .NOP\n" );
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".ENDNOP" ) )
      {
      if( logfile )
        fprintf( logfd, "%5u\t\t%s\t\t.ENDNOP\n",
                    asmpctx->srclinenum, asmpctx->nopstate & LHASM_NOP_TRUE ? "+TRUE+" : "/false/" );
      if( !asmpctx->nopstate )
        LHASM_ERR( ".NOP block not open\n" );
      asmpctx->nopstate = LHASM_NOP_NONE;
      continue;
      }
    else if( 0 == strcasecmp( arg[0], ".END" ) )
      {
end:
      if( asmpctx->ifstate )
        LHASM_FTL( ".IF block open at line %d not closed\n", asmpctx->iflinenum );
      if( asmpctx->nopstate )
        LHASM_FTL( ".NOP block open at line %d not closed\n", asmpctx->noplinenum );
      if( asmpctx->nndostack != asmnndostack )
        LHASM_FTL( "IF/ELSE/ENDIF BEGIN/WHILE/REPEAT BEGIN/UNTIL stack not empty\n" );
      if( asmpctx->state == LHASM_BASIC )
        {
#if 0
        *(asmcurrent++) = 0xFF;
        if( logfile )
          fprintf( logfd, "\t%04hX\t%02X            \t%s\n",
                   lhAsmGetPC(), *(asmcurrent - 1), "[END BASIC MARKER]" );
        asmaddr++;
#else
        if( logfile )
          fprintf( logfd, "\t%04hX\t              \t%s\n",
                   lhAsmGetPC(), "[END BASIC MARKER]" );
#endif
        }
      else if( logfile )
          fprintf( logfd, "%5u\t%04X\t.END\n",
                   asmpctx->srclinenum, lhAsmGetPC() );
      asmpctx->end = lhAsmGetPC();
      asmpctx->length = asmpctx->end - asmpctx->start;
      asmpctx->state = LHASM_END;
      if( asmpctx->length )
        {
        if( asmtop > asmorg )
          asmtop = asmorg;
        if( asmbottom < lhAsmGetPC() )
          asmbottom = lhAsmGetPC();
        }

      {
      char incna[80];

      snprintf( incna, sizeof(incna), "%s$$._start", asmpctx->srcfile );
      if( asmpass == 1 )
        asmincsym = lhSymAddSorted( asmincsym, incna, asmpctx->start, 0 );
      else
        {
        if( asmpctx->start != lhSymGetValue( asmincsym, incna ) )
          LHASM_FTL( "Assembler confused. START from %s differs between passes\n", asmpctx->srcfile );
        }
      if( logfile )
        fprintf( logfd, "\t\t;; %04X\t%s\n", asmpctx->start, incna );

      snprintf( incna, sizeof(incna), "%s$$._end", asmpctx->srcfile );
      if( asmpass == 1 )
        asmincsym = lhSymAddSorted( asmincsym, incna, asmpctx->end, 0 );
      else
        {
        if( asmpctx->end != lhSymGetValue( asmincsym, incna ) )
          LHASM_FTL( "Assembler confused. END from %s differs between passes\n", asmpctx->srcfile );
        }
      if( logfile )
        fprintf( logfd, "\t\t;; %04X\t%s\n", asmpctx->end, incna );

      snprintf( incna, sizeof(incna), "%s$$._length", asmpctx->srcfile );
      if( asmpass == 1 )
        asmincsym = lhSymAddSorted( asmincsym, incna, asmpctx->length, 0 );
      else
        {
        if( asmpctx->length != lhSymGetValue( asmincsym, incna ) )
          LHASM_FTL( "Assembler confused. LENGTH from %s differs between passes\n", asmpctx->srcfile );
        }
      if( logfile )
        fprintf( logfd, "\t\t;; %04X\t%s\n", asmpctx->length, incna );
      }
      {
      PLHSYM psym = asmsym;
      LHU8 nv, nc;

      if( logfile && !(asmflags & LHASM_NOSYMBOL) )
        {
        fprintf( logfd, "\n\t\t" LHSYM_MAGIC "\n");
        while(asmpctx->included == 0 && psym != (PLHSYM) NULL)
          {
          fprintf(logfd, "\t%04hX\t%s\n", psym->value, psym->name);
          psym = psym->next;
          }
        fprintf( logfd, "\n" );
        for( nv = 0; asmpctx->included == 0 && nv < LHASM_MAX_VARS; nv++ )
          for( nc = 0; asmvarmask[nv] && nc < 26; nc++ )
            if( asmvarmask[nv] & (1 << nc) )
              fprintf( logfd, "\t%04hX\t%%%02hhu%c\n", asmvars[nv][nc], nv, nc + 'a' );
        fprintf( logfd, "\n" );
        if( !(asmflags & LHASM_NOLOCAL) && asmpctx->plocal )
          {
          psym = *asmpctx->plocal;
          while(psym != (PLHSYM) NULL)
            {
            fprintf(logfd, "\t%04hX\t%s\n", psym->value, psym->name);
            psym = psym->next;
            }
          fprintf( logfd, "\n" );
          }
        psym = resym;
        while(asmpctx->included == 0 && psym != (PLHSYM) NULL)
          {
          fprintf(logfd, "\t%04hX\t%s\n", psym->value, psym->name);
          psym = psym->next;
          }
        fprintf( logfd, "\n" );
        }
      }

      if( !fd )
        return;

      continue;
      }
    else if( 0 == strcasecmp( ".RAM", arg[0] ) )
      {
      asmsection = LHASM_MEM_RAM;
      goto checksection;
      }
    else if( 0 == strcasecmp( ".ROM", arg[0] ) )
      {
      asmsection = LHASM_MEM_ROM;
      goto checksection;
      }
    else if( 0 == strcasecmp( ".SYS", arg[0] ) )
      {
      asmsection = LHASM_MEM_SYS;
      goto checksection;
      }
    else if( 0 == strcasecmp( ".LM", arg[0] ) )
      {
      asmsection = LHASM_MEM_LM;
checksection:
      if( asmpmachine )
        {
        int isec;

        for( isec = 0; isec < LHASM_MAX_MEMSECTION && asmpmachine->section[ isec ].type; isec++ )
          {
          if( asmpmachine->section[ isec ].base <= lhAsmGetPC() &&
              lhAsmGetPC() <= asmpmachine->section[ isec ].end )
            {
            if( asmpmachine->section[ isec ].type == asmsection )
              goto outsection;
            else
              LHASM_FTL( "Section mismatch at %04X. Expecting %s, section is %s\n",
                         lhAsmGetPC(), arg[0] + 1, asmpmachine->section[ isec ].name );
            }
          }
        LHASM_FTL( "No section for address %04X\n", lhAsmGetPC() );
        }
      else
        LHASM_FTL( "No section available because no machine defined\n" );
outsection:
      continue;
      }
    else if( 0 == strncasecmp( ".PC15", arg[0], 5 ) || \
             0 == strncasecmp( ".PTA", arg[0], 4 ) || \
             0 == strncasecmp( ".CE1", arg[0], 4 ) )
      {
      unsigned int org;

      if( 1 == lhAsmDeclareMachine( arg[0]+1, 0, &org ) )
        {
        v = org & 0xFFFF;
        if( asmtop == 0x7FFFFFFF || asmorg == 0x0FFFFF )
          lhAsmSetOrigin( v, arg[0], 0 );
        continue;
        }
      }

nextmacro:
    if( asmorg == 0x0FFFFF || lhAsmGetPC() == 0x0FFFFF )
      goto freemacro;

    if( asmrunmacro && isdigit(*arg[0]) && arg[0][1] == ':' && !arg[0][2] )
      {
      LHU8 nl = *arg[0] - '0';

      if( asmpass == 1 )
        {
        if( asmlblmask[asmnnrun] & (1 << nl) )
          LHASM_ERR( "Macro label '%c:' is already defined\n", nl );
        asmrunlbl[(asmnnrun * 10) + nl] = lhAsmGetPC();
        asmlblmask[asmnnrun] |= (1 << nl);
        }
      else
        {
        if( !(asmlblmask[asmnnrun] & (1 << nl)) )
          LHASM_ERR( "Macro label '%c:' is never defined\n", nl );
        if( lhAsmGetPC() != asmrunlbl[(asmnnrun * 10) + nl] )
          LHASM_FTL( "Assembler confused: Macro label '%c:' differs between passes\n", nl );
        }
      if( logfile )
        fprintf( logfd, "%5u\t%04X\t%hhu:\n"
                        "\t;\t#%s__%02u__#%hhu\t.EQU %04hX\n",
                 asmpctx->srclinenum, lhAsmGetPC(), nl,
                 asmrunmacro->name, asmnnrun, nl, asmrunlbl[(asmnnrun * 10) + nl] );
      goto freemacro;
      }
    else if( *arg[0] == '%' &&
        (arg[0][l = (strlen(arg[0]) - 1)] == ':' || (narg > 2 && 0 == strcasecmp( arg[1], ".EQU"))) )
      {
      LHU8 nv, nc, np = 1;

      if( arg[0] && 4 <= strlen( arg[0] ) && 2 == sscanf( arg[0]+1, "%hhu%c", &nv, &nc ) &&
        nv < LHASM_MAX_VARS && isalpha(nc) )
        {
        nc = tolower(nc) - 'a';
        if( narg > 2 && 0 == strcasecmp(arg[1], ".EQU") )
          {
          if( strcmp( arg[2], "." ) == 0 )
            v = lhAsmGetPC();
          else if( (0xFFFF == (v = lhAsmGetSymValue( asmsym, arg[2] ))) &&
                   0 == lhAsmGetLHUValue( NULL, arg[2], &v, LINK_MEM_16, 0, 0 ) )
            LHASM_ERR( "Badly formed parameter '%s'\n", arg[2] );
          np = 3;
          }
        else if( narg > 4 && 0 == strcasecmp(arg[1], ".ARRAYOF") )
          {
          LHU16 sz = 0, b, t;

          if( 0 == strcasecmp( arg[2], "BYTE" ) )
            sz = 1;
          else if( 0 == strcasecmp( arg[2], "WORD" ) )
            sz = 2;
          else if( 0 == strcasecmp( arg[2], "LONG" ) )
            sz = 4;
          else if( 1 != sscanf( arg[2], "%hx", &sz ) )
            LHASM_ERR( "Unknown element size '%s' for array\n", arg[2] );
          if(0xFFFF == (t = lhAsmGetSymValue( asmsym, arg[3] )) && (1 != sscanf( arg[3], "%hx", &t )) )
            LHASM_ERR( "Unknown array top address %s\n", arg[3] );
          if(0xFFFF == (b = lhAsmGetSymValue( asmsym, arg[4] )) && (1 != sscanf( arg[4], "%hx", &b )) )
            LHASM_ERR( "Unknown array base address %s\n", arg[4] );
          v = ((t < b) ? (b - t) : (t - b)) / sz;
          np = 5;
          }
        else
          v = lhAsmGetPC();
        if( arg[np] && *arg[np] != ';' )
          LHASM_ERR( "Unexpected arguments \"%s\"\n", arg[np]);

        asmvarmask[nv] |= (1 << nc);
        asmvars[nv][nc] = v;
        }
      if( logfile )
        fprintf( logfd, "%5u\t%04X\t%%%02hhu%c\t.EQU %04hX\n",
                 asmpctx->srclinenum, lhAsmGetPC(), nv, 'a' + nc, asmvars[nv][nc] );
      goto freemacro;
      }
    else if( ( (export = (0 == strcasecmp( ".EXPORT:", arg[0] ))) && *arg[1] != '.' &&
               (arg[1][l = (strlen(arg[1]) - 1)] == ':' ||
                (narg >= 4 && 0 == strcasecmp( arg[2], ".EQU"))) )
             ||
             ( *arg[0] != '.' &&
               (arg[0][l = (strlen(arg[0]) - 1)] == ':' ||
                (narg >= 3 && 0 == strcasecmp( arg[1], ".EQU"))) )
           )
      {
      char symna[80];

      strncpy( memset( symna, 0, sizeof(symna) ), arg[export], arg[export][l] == ':' ? l : l +  1);
      if( narg > (2+export) && 0 == strcasecmp(arg[1+export], ".EQU") )
        {
        char *pb, *pa = arg[2+export];

        if( strcmp( pa, "." ) == 0 )
          v = lhAsmGetPC();
        else if( (pb = strstr( pa, "__/" )) )
          {
          char substr[80], *pe;
          struct s_lhasmsubs *psub;

          if( (pe = strchr( (pb + 3), '/' )) )
            {
            memset( substr, 0, sizeof( substr ) );
            strncpy( substr, pb + 3, (int)(pe - pb - 3) );
            for( pa = NULL, psub = asmsubs; psub ; psub = psub->next )
              if( strcmp( substr, psub->name ) == 0 )
                break;
            if( psub )
              {
              memset( pa = substr, 0, sizeof( substr ) );
              strncpy( substr, arg[2+export], pb - arg[2+export] );
              strcat( substr, psub->body );
              strcat( substr, pe + 1 );
              }
            }
          if( !pa || ((0xFFFF == (v = lhAsmGetSymValue( asmsym, pa ))) &&
                      (0 == lhAsmGetLHUValue( NULL, pa, &v, LINK_MEM_16, 0, 0 )) || v == 0xFFFF) )
            LHASM_ERR( "Badly formed parameter '%s'\n", arg[2+export] );
          }
        else
          {
          if( !lhAsmGetLHUValue( NULL, pa, &v, LINK_MEM_16, 0, 0 ) &&
              (!lhAsmGetLHUValue( pa, pa, &v, LINK_MEM_16, 0xFFFF, 0xFFFF ) || v == 0xFFFF) )
            LHASM_ERR( "Badly formed parameter '%s'\n", pa );
          }
        }
      else if( narg+export > 4 && 0 == strcasecmp(arg[1+export], ".ARRAYOF") )
        {
        LHU16 sz = 0, b, t;

        if( 0 == strcasecmp( arg[2+export], "BYTE" ) )
          sz = 1;
        else if( 0 == strcasecmp( arg[2+export], "WORD" ) )
          sz = 2;
        else if( 0 == strcasecmp( arg[2+export], "LONG" ) )
          sz = 4;
        else if( 1 != sscanf( arg[2+export], "%hx", &sz ) )
          LHASM_ERR( "Unknown element size '%s' for array\n", arg[2+export] );
        if(0xFFFF == (t = lhAsmGetSymValue( asmsym, arg[3+export] )) &&
           (1 != sscanf( arg[3+export], "%hx", &t )) )
          LHASM_ERR( "Unknown array top address %s\n", arg[3+export] );
        if(0xFFFF == (b = lhAsmGetSymValue( asmsym, arg[4+export] )) &&
           (1 != sscanf( arg[4+export], "%hx", &b )) )
          LHASM_ERR( "Unknown array base address %s\n", arg[4+export] );
          v = ((t < b) ? (b - t) : (t - b)) / sz;
        }
      else if( arg[1+export] && *arg[1+export] != ';' )
        LHASM_ERR( "Unexpected arguments \"%s\"\n", arg[1+export]);
      else
        v = lhAsmGetPC();
      if( asmpass == 1 )
        {
        if( 0xFFFF != lhAsmGetSymValue(asmsym, symna) )
          LHASM_ERR( "Symbol '%s' is already defined.\n", symna );
        else
          {
          asmsym = lhAsmAddSymSorted( asmsym, symna, v, SYM_NONE, (export|asmpctx->exportall));
          }
        }
      else
        {
        LHU16 vv;

        if( 0xFFFF == (vv = lhAsmGetSymValue(asmsym, symna)) )
          LHASM_ERR( "Symbol '%s' is never defined.\n", symna );
        if( v != vv )
          LHASM_FTL( "Assembler confused: Symbol '%s' differs between passes\n",
                     symna );
        }
      if( logfile )
        fprintf( logfd, "%5u\t%04X\t%s:\t.EQU %04hX\n", asmpctx->srclinenum, lhAsmGetPC(), symna, v );
      goto freemacro;
      }
    if( !asmrunmacro )
      {
      for( i = 0; !asmrunmacro && i < asmnnmacro; i++ )
        {
        if( 0 == strcasecmp( asmmacro[i].name, arg[0] ) )
          {
          if( asmnnrun == LHASM_MAX_RUN )
            LHASM_ERR( "Macros run stack exceeded\n" );
          asmrunmacro = &asmmacro[i];
          asmrunmacro->thisline = 0;
          for( i = 0; i < LHASM_MAX_ARG_BY_MACRO && i <(narg - 1); i++ )
            strcpy( asmrunmacro->arg[i] = malloc( strlen( arg[i + 1] ) + 1 ), arg[i + 1]);
          asmrunmacro->nnarg = i;
          if( logfile )
            {
            fprintf( logfd, "%5u\t\t\t%s", asmpctx->srclinenum, asmrunmacro->name );
            for( i = 0; i < LHASM_MAX_ARG_BY_MACRO && i <(narg - 1); i++ )
              fprintf( logfd, "%c%s", i ? ' ' : '\t', asmrunmacro->arg[i] );
            fprintf( logfd, "\n\%5u\t\t\t\t{\n", asmpctx->srclinenum );
            }
          goto runmacro;
          }
        }
      }

    asmbase = asmcurrent;
    nn = 0;
    switch( asmpctx->state )
      {
      case LHASM_BASIC:
           nn = lhAsmFetchBasic( arg[0], (char**)&arg[1], lhAsmGetPC(), &asmcurrent );
           break;
      case LHASM_CODE:
           nn = lhAsmFetchInst( arg[0], (char**)&arg[1], lhAsmGetPC(), &asmcurrent );
           break;
      case LHASM_TEXT:
           for( i = 0; i < narg && *arg[i] != ';'; i++ )
             {
             int c;
             char ascii[3];
             l = strlen(arg[i]);
             ascii[2] = 0;
             if( *arg[i] == '"' && arg[i][l-1] == '"' )
               {
               l--;
               for( c = 1; c < l; c++ )
                 {
                 if( c < (l+5) && strncmp(&arg[i][c], "\\\\+80", 5) == 0 )
                   {
                   *(asmcurrent++) = arg[i][c] | 0x80;
                   c += 4;
                   }
                 else if( c < (l+1) && strncmp(&arg[i][c], "\\\\", 2) == 0 )
                   {
                   *(asmcurrent++) = arg[i][c];
                   c += 1;
                   }
                 else if( c < (l+4) && strncmp(&arg[i][c + 1], "\\+80", 4) == 0 )
                   {
                   *(asmcurrent++) = arg[i][c] | 0x80;
                   c += 4;
                   }
                 else if( c < (l+3) && strncmp(&arg[i][c], "\\x", 2) == 0 &&
                          1 == sscanf(memcpy(ascii, &arg[i][c + 2], 2), "%hx", &v) && v < 0x100 )
                   {
                   *(asmcurrent++) = v;
                   c += 3;
                   }
                 else if( c < (l+2) && arg[i][c] == '\\' &&
                          isxdigit( arg[i][c + 1] ) && isxdigit( arg[i][c + 2] ) &&
                          1 == sscanf(memcpy(ascii, &arg[i][c + 1], 2), "%hx", &v) && v < 0x100 )
                   {
                   *(asmcurrent++) = v;
                   c += 2;
                   }
                 else if( arg[i][c] == '\\' && arg[i][c + 1] && strchr( "ipsy", arg[i][c + 1] ) )
                   {
                   if( 0 == strncmp( "ins", &arg[i][c + 1], 3 ) )
                     {
                     v = 0x27;
                     c += 3;
                     }
                   else if( 0 == strncmp( "sqr", &arg[i][c + 1], 3 ) )
                     {
                     v = 0x5b;
                     c += 3;
                     }
                   else if( 0 == strncmp( "yen", &arg[i][c + 1], 3 ) )
                     {
                     v = 0x5c;
                     c += 3;
                     }
                   else if( 0 == strncmp( "pi", &arg[i][c + 1], 2 ) )
                     {
                     v = 0x5d;
                     c += 2;
                     }
                   else
                     {
                     v = arg[i][c];
                     }
                   *(asmcurrent++) = v;
                   }
                 else
                   *(asmcurrent++) = arg[i][c];
                 }
               }
             else
               {
               LHU8 *pcurrent = &asmcurrent[1];

               if( lhAsmStringify( &arg[i], &asmcurrent[0], &pcurrent ) )
                 asmcurrent = pcurrent;
               }
             }
           nn = asmcurrent - asmbase;
           break;
      case LHASM_BYTE:
           for( i = 0; i < narg && *arg[i] != ';'; i++ )
             {
             int ic, il;

             if( *arg[i] == '"' && arg[i][il = (strlen(arg[i]) - 1)] == '"' )
               for( ic = 1 ; ic < il ; ic++ )
                 *(asmcurrent++) = arg[i][ic];
             else if( lhAsmGetLHUValue( arg[i], arg[i], &v, LINK_MEM_8, 0, asmcurrent - asmbase + 1 ) )
               *(asmcurrent++) = v;
             else
               LHASM_ERR( "Badly formed byte expression '%s'\n", arg[i] );
             }
           nn = asmcurrent - asmbase;
           break;
      case LHASM_WORD:
           for( i = 0; i < narg && *arg[i] != ';'; i++ )
             if( lhAsmGetLHUValue( arg[i], arg[i], &v, LINK_MEM_16, lhAsmGetPC(), asmcurrent - asmbase + 2 ) )
               {
               *(asmcurrent++) = (v & 0xFF00) >> 8;
               *(asmcurrent++) = v & 0xFF;
               }
             else
               LHASM_ERR( "Badly formed word expression '%s'\n", arg[i] );
           nn = asmcurrent - asmbase;
           break;
      case LHASM_LONG:
           for( i = 0; i < narg && *arg[i] != ';'; i++ )
             {
             unsigned long lv;
             if( 8 == strlen( arg[i] ) && 1 == sscanf( arg[i], "%x", &lv) )
               {
               *(asmcurrent++) = (lv & 0xFF000000) >> 24;
               *(asmcurrent++) = (lv & 0xFF0000) >> 16;
               *(asmcurrent++) = (lv & 0xFF00) >> 8;
               *(asmcurrent++) = lv & 0xFF;
               }
             }
           nn = asmcurrent - asmbase;
           break;
      case LHASM_RESERVE:
           if( narg >= 2 && *arg[0] != ';' && *arg[1] != ';' )
             {
             LHU8 kr = 0xFF, lr, dr;
             if( 0 == strncmp( arg[0], "III.F", lr = 5 ) )
               kr = 0x08;
             else if( 0 == strncmp( arg[0], "II.F", lr = 4 ) )
               kr = 0x10;
             else if( 0 == strncmp( arg[0], "I.F", lr = 3 ) )
               kr = 0x00;
             if( kr != 0xFF &&
                 ( (((dr = arg[0][lr]) >= '1') && (dr <= '6')) ||
                   (((dr = arg[0][lr]) >= '!') && (dr <= '&')) ) &&
                   (0 == arg[0][lr + 1]) )
               {
               kr |= (dr & 0x07);
               *(asmcurrent++) = kr;
               for( i = 1; i < narg && *arg[i] != ';'; i++ )
                 {
                 int ic, il;

                 if( *arg[i] == '"' && arg[i][il = (strlen(arg[i]) - 1)] == '"' )
                   for( ic = 1 ; ic < il ; ic++ )
                     *(asmcurrent++) = arg[i][ic];
                 else if( (v = lhBasicGetCode( arg[i] )) )
                   {
                   *(asmcurrent++) = (v >> 8);
                   *(asmcurrent++) = (v & 0xFF);
                   }
                 else if( lhAsmGetLHUValue( arg[i], arg[i], &v, LINK_MEM_8, 0, asmcurrent - asmbase + 1 ) )
                   *(asmcurrent++) = v;
                 else
                   LHASM_ERR( "Badly formed byte expression '%s'\n", arg[i] );
                 }
               nn = asmcurrent - asmbase;
               }
             else
               LHASM_ERR( "Badly formed RESERVE key %s\n", arg[0] );
             }
           else
             LHASM_ERR( "Badly formed RESERVE expression\n" );
           break;
      case LHASM_VAR:
      case LHASM_XREG:
           nn = 0;
           break;
      case LHASM_KYWD:
           if( (narg >= 1 && '"' == *arg[0]) ||
               (narg >= 4 && 1 == sscanf( arg[0], "%hx", &v )) )
             {
             int l;
             LHU16 kycode, kyjump;
             LHU8 kybits;

             v = ('"' == *arg[0] ? 0 : 1);
             memset( basic, 0, sizeof(basic) );
             strncpy( basic, arg[v] + 1, l = (strlen(arg[v] + 1) - 1) );
             kycode = lhBasicGetCodeBitsAndJump(basic, &kybits, &kyjump);
             if( logfile )
               fprintf( logfd, "%5u\t%04hX\t" LHBASIC_MAGIC " \"%s\"\t%04X %04X %c\n",
                        asmpctx->srclinenum, lhAsmGetPC(), basic, kycode, kyjump, kybits );
             if( kycode || asmpass == 1 )
               {
               if( toupper(*basic) >= 'A' && toupper(*basic) <= 'Z' && kych != toupper(basic[0]) )
                 {
                 *(asmcurrent - 1) |= 0x10;
                 kych = toupper(basic[0]);
                 kyhash[ (kych - 'A') * 2 ] = ((lhAsmGetPC() + 1) & 0xFF00) >> 8;
                 kyhash[ (kych - 'A') * 2 + 1 ] = ((lhAsmGetPC() + 1) & 0xFF);
                 }
               else
                 *(asmcurrent - 1) &= 0xEF;
               *(asmcurrent - 1) = (*(asmcurrent - 1) & 0xF0) | l;
               memcpy( asmcurrent, basic, l);
               asmcurrent += l;
               *(asmcurrent++) = (kycode & 0xFF00) >> 8;
               *(asmcurrent++) = (kycode & 0xFF);
               *(asmcurrent++) = (kyjump & 0xFF00) >> 8;
               *(asmcurrent++) = (kyjump & 0xFF);
               *(asmcurrent++) = lhBasicConvertBits( kybits ) | 0x10;
               if( keywfile && !(asmflags & LHASM_KEYWORD_EXPORT) )
                 if( asmflags & LHASM_KEYWORD_OLD )
                   fprintf( keywfd, "\t%02X\t\"%s\"\t%04hX %04hX\n", *(asmbase - 1),
                                    basic, kycode, kyjump );
                 else
                   fprintf( keywfd, "\t\"%s\"\t%04hX %04hX %c\n",
                                    basic, kycode, kyjump, kybits );
               }
             }
           nn = asmcurrent - asmbase;
           break;
      case LHASM_HOLE:
           LHASM_FTL( "Nothing possible in .HOLE fragment\n" );
           break;
      case LHASM_SYMBOLS:
           continue;
      }
inlog:
    if( asmtop == 0x7FFFFFFF || asmorg == 0x0FFFFF )
      lhAsmSetOrigin( asmaddr, ".ORIGIN:", nn );
    if( asmpctx->nopstate & LHASM_NOP_TRUE )
      {
      for( i = 0; i < nn ; asmbase[i++] = 0x38 )
        ;
      }
    if( logfile && (nn || asmpctx->state == LHASM_BASIC) )
      {
      if( asmpctx->ifstate && !(asmpctx->ifstate & 0x1) )
        fprintf( logfd, "%5u\t/false/\t", asmpctx->srclinenum );
      else
        fprintf( logfd, "%5u\t%04hX\t", asmpctx->srclinenum, lhAsmGetPC() );
      for( i = 0; i < nn && i < 5; i++ )
        fprintf( logfd, "%02X ", asmbase[i] );
      for( ; i >= nn && i < 5; i++ )
        fprintf( logfd, "   " );
      fprintf( logfd, "\t" );
      for( l = 0; l < narg && (*arg[l] != ';' || !(asmflags & LHASM_NOCOMMENT)); l++ )
        fprintf( logfd, "%s%c", arg[l], l ? ' ' : '\t' );
      for( ; i < nn; i++ )
        {
        if( !(i % 5) )
          fprintf(logfd, "\n\t\t");
        fprintf( logfd, "%02X ", asmbase[i] );
        }
      fprintf(logfd, "\n");
      }
    asmaddr += nn;
    if( asmpctx->state == LHASM_DATAEND_DEF )
      {
      if( logfile )
        {
        fprintf( logfd, "%5u\t\t\t\t}\n", asmpctx->srclinenum );
        fprintf( logfd, "%5u\t\t\t.ENDDATA\t; %04X %s\n",
                 asmpctx->srclinenum, pdata->size, pdata->name );
        }
      pdata = NULL;
      datann = 0;
      asmpctx->state = savestate;
      continue;
      }

freemacro:
    if( asmrunmacro && asmrunmacro->thisline == asmrunmacro->nnline )
      {
      for( i = 0; i < asmrunmacro->nnarg; i++ )
        free( asmrunmacro->arg[i] );
      asmrunmacro->thisline = 0;
      asmrunmacro->nnarg = 0;
      asmrunmacro = NULL;
      asmnnrun++;
      if( logfile )
        fprintf( logfd, "%5u\t\t\t\t}\n", asmpctx->srclinenum );
      }

    if( asmrunmacro )
      goto runmacro;
    }

  if( asmpctx->ifstate )
    LHASM_FTL( ".IF block open at line %d not closed\n", asmpctx->iflinenum );
  if( asmpctx->nopstate )
    LHASM_FTL( ".NOP block open at line %d not closed\n", asmpctx->noplinenum );

  if( fd != stdin )
    fclose( fd );
  fd = NULL;

  if( asmpctx->state < (LHU8)LHASM_NEED_END )
    goto end;
}

int main(int argc, char **argv)
{
  int   argerr, iargc;
  char *argvc;
  int   l;
  unsigned int optorg = 0x0FFFFF, hworg = 0x0FFFFF;
  LHU16 cksm = 0;
  LHU8 optstate = LHASM_NONE;
  char *optfrag = NULL;
  char cvers[4];
  char *optmachine = NULL;
  char *optmodule = NULL;
  LHU8 zhdr[LHCE158HDR_LENGTH];
  LHU8 *zphdr = NULL, *zname = NULL, *zmagic = NULL;
  LHU16 zstart = 0xFFFF, zlength = 0xFFFF, zentry = 0xFFFF, zstate = LHASM_NONE;

  memset( asmincpath, 0, sizeof(asmincpath) );
  memset( outfile, 0, sizeof(outfile) );
  memset( cvers, 0, sizeof(cvers) );
  sscanf( lhTools_vers, "%hhu.%hhu.%hhup%hhu", &cvers[0], &cvers[1], &cvers[2], &cvers[3] );
  asmvers = ((cvers[0] & 0xFF) << 24) | ((cvers[1] & 0xFF) << 16) | \
            ((cvers[2] & 0xFF) << 8) | (cvers[3] & 0xFF);

  {
  time_t t = time(NULL);
  struct tm *tm = localtime( &t );
  memcpy( &asmtm, tm, sizeof(struct tm) );
  }

  for( argerr = 0, iargc = 1; !argerr && iargc < argc; iargc++ )
    {
    if(*(argvc = argv[iargc]) == '-')
      {
      switch(*(++argvc))
        {
        case 'h': usage(argv[0]);
                  break;
        case 'v': if( '#' == *(++argvc) )
                    {
                    printf( "%hhu.%hhu.%hhu", cvers[0], cvers[1], cvers[2] );
                    if( cvers[3] )
                      printf("p%hhu", cvers[3]);
                    printf("\n");
                    }
                  else
                    fprintf( stderr, "lhTools Version %s\n"
                                     "        Compiled %s\n"
                                     "        Platform %s\n"
                                     "Copyright 1992-2015 Christophe Gottheimer\n\n",
                                     lhTools_vers, lhTools_date, lhTools_pltf );
                  exit( 0 );
                  break;
        case 'd': if( 0 == *(++argvc) || 0 == strcmp( "debug", argvc ) )
                    asmflags |= LHASM_DEBUG;
                  else if( 0 == strcmp( "verbose", argvc ) )
                    asmflags |= LHASM_VERBOSE;
                  else
                    argerr++;
                  break;
        case 'a': asmflags |= LHASM_ALL_LOCAL;
                  break;
        case 'n': if( 'c' == *(++argvc) )
                    asmflags |= LHASM_NOCOMMENT;
                  else if( 's' == *argvc )
                    asmflags |= LHASM_NOSYMBOL;
                  else if( 'o' == *argvc )
                    asmflags |= LHASM_NOBINARY;
                  else if( 'a' == *argvc )
                    asmflags |= LHASM_NOLOCAL;
                  else
                    argerr++;
                  break;
        case 'i': asmflags |= LHASM_IMMEDIATE;
                  if( asmflags & LHASM_JR_TO_JP )
                    {
immediateJRJPerror:
                    fprintf( stderr, "Immediate assembler -i and 'JR to JP' feature -J are incomptible\n" );
                    exit( 1 );
                    }
                  asmpctx->srcfile = "(stdin)";
                  break;
        case '1': asmflags |= LHASM_PASS_1_ONLY;
                  break;
        case 'x': asmflags |= LHASM_OUTPUT_HEX;
                  break;
        case 'N': asmflags |= LHASM_NOBINARY;
                  break;
        case 'J': asmflags |= LHASM_JR_TO_JP;
                  if( asmflags & LHASM_IMMEDIATE )
                    goto immediateJRJPerror;
                  if( 0 == strncmp( argvc, "Jloop=", 6 ) )
                    if( 1 != sscanf( argvc + 6, "%u", &asmJRJPmaxloop ) )
                      {
                      fprintf( stderr, "Maximum loop for -Jloop= badly specified\n" );
                      exit( 1 );
                      }
                  break;
        case 'W': asmflags |= LHASM_LOW_WARNING;
                  break;
        case 'E': if( 'E' == argv[iargc][2] )
                    asmflags |= LHASM_FATAL_ERROR;
                  else
                    asmflags |= LHASM_WARNING_AS_ERROR;
                  break;
        case 'T': asmflags |= LHASM_TRACE;
                  argerr = (logfile != NULL);
                  break;
        case 'A': if(!(argerr = (++iargc == argc))) 
                    {
                    char substr[80];
                    char *p = strchr( argv[iargc], '=' );
                    struct s_lhasmsubs *psub, *ptail;

                    if( p )
                      {
                      strncpy( memset( substr, 0, sizeof(substr) ), argv[iargc], p - argv[iargc] );
                      for( psub = asmsubs; psub; psub = psub->next )
                        if( 0 == strcmp( psub->name, substr ) )
                          LHASM_FTL( "Substitute string '%s' is already defined.\n", substr );
                      if( NULL == (psub = (struct s_lhasmsubs *)malloc( sizeof(struct s_lhasmsubs) )) ||
                          NULL == (psub->name = malloc( strlen(substr) + 1 )) ||
                          NULL == (psub->body = malloc( strlen(p + 1) + 1 )) )
                        LHASM_FTL( "Out of memory while defining substitute string '%s'\n",
                                   substr );
                      strcpy( psub->name, substr );
                      strcpy( psub->body, p + 1);
                      psub->next = asmsubs;
                      asmsubs = psub;
                      }
                    }
                  break;
        case 'D': if(!(argerr = !(argv[iargc][2] && (argv[iargc]+=2))) || !(argerr = (++iargc == argc))) 
                    {
                    char symna[80];
                    char *p = strchr( argv[iargc], '=' );
                    LHU16 v;

                    if( p )
                      {
                      strncpy( memset( symna, 0, sizeof(symna) ), argv[iargc], p - argv[iargc] );
                      if( 0xFFFF != lhAsmGetSymValue(asmsym, symna) )
                        LHASM_FTL( "Symbol '%s' is already defined.\n", symna );
                      else
                        {
                        if( 1 != sscanf( p + 1, "%hx", &v ) )
                          LHASM_FTL( "Badly formed parameter '%s'\n", p );
                        asmsym = lhAsmAddSymSorted( asmsym, symna, v, SYM_NONE, 1 );
                        }
                      }
                    }
                  break;
        case 'I': if(!(argerr = (++iargc == argc)))
                    {
                    int iinc;

                    for( iinc = 0; iinc < LHASM_MAX_INCLUDE_PATH && asmincpath[ iinc ]; iinc++ )
                      if( 0 == strcmp( asmincpath[ iinc ], argv[iargc] ) )
                        break;
                    if( LHASM_MAX_INCLUDE_PATH == iinc )
                      LHASM_FTL( "Too many include path specified\n" );
                    if( !asmincpath[ iinc ] )
                      asmincpath[ iinc ] = argv[iargc];
                    }
                  break;
        case 'F': if(!(argerr = (++iargc == argc)))
                    frgfile = argv[iargc];
                  break;
        case 'K': if( 'K' == argvc[1] )
                    asmflags |= LHASM_KEYWORD_OLD;
                  if( 'E' == argvc[asmflags & LHASM_KEYWORD_OLD ? 2 : 1] )
                    asmflags |= LHASM_KEYWORD_EXPORT;
                  if(!(argerr = (++iargc == argc)))
                    keywfile = argv[iargc];
                  break;
        case 'M': if(!(argerr = (++iargc == argc)))
                    {
                    if( NULL == (macfd = fopen( macfile = argv[iargc], "w" )) )
                      {
                      fprintf( stderr, "Unable to open macfile %s\n", macfile );
                      exit( 1 );
                      }
                    }
                  break;
        case 'L': if(!(argerr = (++iargc == argc)) && !(asmflags & LHASM_TRACE))
                    logfile = argv[iargc];
                  break;
        case 'S': if(!(argerr = (++iargc == argc)))
                    {
                    if( NULL == (symfd = fopen( symfile = argv[iargc], "w" )) )
                      {
                      fprintf( stderr, "Unable to open symfile %s\n", symfile );
                      exit( 1 );
                      }
                    fprintf( symfd, LHSYM_MAGIC "\n" );
                    }
                  break;
        case 'r': if(!(argerr = (++iargc == argc)))
                    {
                    if( NULL == (resymfd = fopen( resymfile = argv[iargc], "r" )) )
                      {
                      fprintf( stderr, "Unable to open resymfile %s\n", resymfile );
                      exit( 1 );
                      }
                    }
                  break;
        case 's': if(!(argerr = (++iargc == argc)))
                    {
                    if( NULL == (resrcfd = fopen( resrcfile = argv[iargc], "w" )) )
                      {
                      fprintf( stderr, "Unable to open resrcfile %s\n", resrcfile );
                      exit( 1 );
                      }
                    }
                  break;
        case 'Z': zphdr = zhdr;
                  if( 0 == strncmp( argvc, "Zname=", 6 ) )
                    {
                    if( !(*(argvc + 6)) || zname )
                      {
                      fprintf( stderr, "Header name badly specified or already set\n" );
                      exit( 1 );
                      }
                    zname = (argvc + 6);
                    }
                  else if( 0 == strncmp( argvc, "Zentry=", 7 ) )
                    {
                    LHU16 entry;
                    if( (0xFFFF != zentry) || (1 != sscanf( argvc + 7, "%hx", &entry )) )
                      {
                      fprintf( stderr, "Header entry jump routine badly specified or already set\n" );
                      exit( 1 );
                      }
                    zentry = entry;
                    }
                  else if( 0 == strncmp( argvc, "Zheader=", 8 ) )
                    {
                    if( !(*(argvc + 6)) || zmagic )
                      {
                    badzmagic:
                      fprintf( stderr, "Header magic badly specified or already set\n" );
                      exit( 1 );
                      }
                    if( 0 == strcasecmp( argvc + 8, "CSAVE" ) )
                      zmagic = LHCE158HDR_CSAVE_MAGIC;
                    else if( 0 == strcasecmp( argvc + 8, "CSAVEM" ) )
                      zmagic = LHCE158HDR_CSAVEM_MAGIC;
                    else if( 0 == strcasecmp( argvc + 8, "CSAVEr" ) )
                      zmagic = LHCE158HDR_CSAVEr_MAGIC;
                    else if( 0 == strcasecmp( argvc + 8, "PRINT" ) )
                      zmagic = LHCE158HDR_PRINT_MAGIC;
                    else
                      goto badzmagic;
                    }
                  else if( *(argvc + 1) )
                    {
                    fprintf( stderr, "Header field unknown\n" );
                    exit( 1 );
                    }
                  break;
        case 'm': if(!(argerr = (++iargc == argc))) 
                    {
                    if( 'P' == toupper( *argv[iargc] ) )
                      optmachine = argv[iargc];
                    else
                      optmodule = argv[iargc];
                    }
                  break;
        case 'O': if(!(argerr = (++iargc == argc))) 
                    {
                    if( 1 != sscanf( argv[iargc], "%x", &optorg ) || (optorg & 0xFFFF0000) )
                        LHASM_FTL( "Badly formed parameter '%s'\n", argv[iargc] );
                    optorg &= 0xFFFF;
                    }
                  break;
        case 'B': optstate = LHASM_BASIC;
optfragget:
                  if( optfrag &&
                      strcmp( optfrag, lhFragmentGetFragmentFromIndex( optstate )) )
                    LHASM_FTL( "Fragment %s already specified\n", optfrag );
                  optfrag = lhFragmentGetFragmentFromIndex( optstate );
                  break;
        case 'R': optstate = LHASM_RESERVE;
                  goto optfragget;
                  break;
        case 'X': optstate = LHASM_XREG;
                  goto optfragget;
                  break;
        case 'V': optstate = LHASM_VAR;
                  goto optfragget;
                  break;
        case 'c': optstate = LHASM_CODE;
                  goto optfragget;
                  break;
        case 'b': optstate = LHASM_BYTE;
                  goto optfragget;
                  break;
        case 'w': optstate = LHASM_WORD;
                  goto optfragget;
                  break;
        case 'l': optstate = LHASM_LONG;
                  goto optfragget;
                  break;
        case 't': optstate = LHASM_TEXT;
                  goto optfragget;
                  break;
        case 'k': optstate = LHASM_KYWD;
                  goto optfragget;
                  break;
        case 'H': optstate = LHASM_HOLE;
                  goto optfragget;
                  break;
        case 'o': if(!(argerr = (++iargc == argc)))
                    strcpy( outfile, argv[iargc] );
                  break;
        default:  argerr = 1; break;
        }
      }
    else
      break;
    }
  if(argerr || (!(asmflags & LHASM_IMMEDIATE ) && iargc >= argc))
    usage(argv[0]);

  if( resymfile )
    {
    char symna[80];
    LHU16 symva;

    while( 1 == fscanf( resymfd, "%hx", &symva ) )
      {
      snprintf( symna, sizeof(symna), "tmp_%hx_%03hx", (symva & 0xF000) >> 12, (symva & 0x0FFF) );
      resym = lhSymAddSorted( resym, symna, symva, SYM_NONE );
      if( logfd )
        fprintf( logfd, "%s\t.EQU\t%04X\n", symna, symva );
      }
    fclose( resymfd );
    }

  memset( asmJRJP, 0, sizeof(asmJRJP) );

redoJRoptimize:
  asmJRJPloop++;
  memset( asmctx, 0, sizeof(asmctx) );
  memset( asmlocal, 0, sizeof(asmlocal) );
  memset( &asminlin, 0, sizeof(asminlin) );
  memset( asmmem, 0, sizeof(asmmem) );
  memset( asmvarmask, 0, sizeof(asmvarmask) );
  memset( asmmacro, 0, sizeof(asmmacro) );
  memset( asmrunlbl, 0, sizeof(asmrunlbl) );
  memset( asmstruct, 0, sizeof(asmstruct) );
  memset( asmmacro, 0, sizeof(asmmacro) );
  memset( asmdostack, 0, sizeof(asmdostack) );

  while( ++asmpass <= 2 )
    {
    memset( asmvarmask, 0, sizeof(asmvarmask) );
    zstate = asmpctx->state = optstate;
    asmaddr = asmorg = 0x0FFFFF;
    asmcurrent = &asmmem[asmorg];
    asmtop = 0x7FFFFFFF;
    asmbottom = -1;
    asmnnrun = 0;
    kytab = NULL;
    kyhash = NULL;
    comment = NULL;
    fragment = optfrag;
    asmpctx->srclinenum = 0;
    asmpctx->srcfile = argv[iargc];
    asmpmachine = asmpmodule = NULL;
    memset( &asmhwmachine, 0, sizeof(asmhwmachine) );
    asmsection = LHASM_MEM_NONE;
    memset( asmmemsection, 0, sizeof(asmmemsection) );
    memset( asmbindR, 0, sizeof(asmbindR) );
    asmnnJRJP = 0;
    asmpctx->nndostack = asmnndostack = 0;

    if( frgfile )
      {
      unlink( frgfile );
      if( NULL == (frgfd = fopen( frgfile, "w" )) )
        {
        fprintf( stderr, "Unable to open frgfile %s\n", frgfile );
        exit( 1 );
        }
      }
    if( keywfile )
      {
      if( NULL == (keywfd = fopen( keywfile, "w" )) )
        {
        fprintf( stderr, "Unable to open keywordfile %s\n", keywfile );
        exit( 1 );
        }
      fprintf( keywfd,  LHBASIC_MAGIC "\n" );
      }
    if( asmflags & LHASM_TRACE )
      {
      logfd = stderr;
      logfile = "(stderr)";
      }
    else if( logfile )
      {
      if( NULL == (logfd = fopen( logfile, "w" )) )
        {
        fprintf( stderr, "Unable to open logfile %s\n", logfile );
        exit( 1 );
        }
      }

    if( logfile )
      {
      int iiargc, larg;

      fprintf( logfd, ";\n"
                      ";\tlhTools Version %s\n"
                      ";\t        Compiled %s\n"
                      ";\t        Platform %s\n",
               lhTools_vers, lhTools_date, lhTools_pltf );
      fprintf( logfd, ";\n"
                      ";\tSource file: %s\n", asmpctx->srcfile );
      fprintf( logfd, ";\tlhasm options:\n"
                      ";\t\t" );
      for( larg = 0, iiargc = 1; iiargc < (argc - 1); iiargc++ )
        {
        larg += fprintf( logfd, "%s ", argv[iiargc] );
        if( larg <= 80 && iiargc < (argc - 2) && '-' != *argv[iiargc + 1] )
          {
          iiargc++;
          larg += fprintf( logfd, "%s ", argv[iiargc] );
          }
        if( larg > 80 )
          {
          fprintf( logfd, "\\\n"
                          ";\t\t" );
          larg = 0;
          }
        }
      fprintf( logfd, "\\\n"
                      ";\t\t%s\n\n", argv[iiargc] );
      }
    if( fragment == NULL )
      {
      if( asmflags & LHASM_IMMEDIATE ||
          (strlen( asmpctx->srcfile ) > 4 &&
           0 == strcasecmp( asmpctx->srcfile + strlen( asmpctx->srcfile ) - 4, ".bas")) )
        {
        zstate = asmpctx->state = LHASM_BASIC;
getfragment:
        fragment = lhFragmentGetFragmentFromIndex( asmpctx->state );
        }
      else if( (strlen( asmpctx->srcfile ) > 4 &&
                (0 == strcasecmp( asmpctx->srcfile + strlen( asmpctx->srcfile ) - 4, ".asm")) ||
                (0 == strcasecmp( asmpctx->srcfile + strlen( asmpctx->srcfile ) - 4, ".src"))) ||
               (strlen( asmpctx->srcfile ) > 3 &&
                (0 == strcasecmp( asmpctx->srcfile + strlen( asmpctx->srcfile ) - 3, ".as"))) ||
               (strlen( asmpctx->srcfile ) > 2 &&
                (0 == strcasecmp( asmpctx->srcfile + strlen( asmpctx->srcfile ) - 2, ".s"))) )
        {
        zstate = asmpctx->state = LHASM_CODE;
        goto getfragment;
        }
      else if( (strlen( asmpctx->srcfile ) > 4 &&
                (0 == strcasecmp( asmpctx->srcfile + strlen( asmpctx->srcfile ) - 4, ".hex"))) ||
               (strlen( asmpctx->srcfile ) > 2 &&
                (0 == strcasecmp( asmpctx->srcfile + strlen( asmpctx->srcfile ) - 2, ".x"))) )
        {
        zstate = asmpctx->state = LHASM_BYTE;
        goto getfragment;
        }
      else if( (strlen( asmpctx->srcfile ) > 4 &&
                (0 == strcasecmp( asmpctx->srcfile + strlen( asmpctx->srcfile ) - 4, ".rsv"))) )
        {
        zstate = asmpctx->state = LHASM_RESERVE;
        goto getfragment;
        }
      }

    if( optmachine && !lhAsmDeclareMachine( optmachine, 1, &hworg ) )
        LHASM_FTL( "Unknown machine '%s'\n", optmachine );
    if( optmodule && !lhAsmDeclareMachine( optmodule, 1, &hworg ) )
        LHASM_FTL( "Unknown module '%s'\n", optmodule );

    if( asmpctx->state )
      {
      if( optorg != 0x0FFFFF )
        {
        lhAsmSetOrigin( optorg, ".ORIGIN:", 0 );
        goto nofragdump;
        }
      else if( hworg != 0x0FFFFF )
        asmaddr = asmorg = hworg;
      else
        asmaddr = asmorg = 0x40C5;
      if( frgfile )
        {
        fprintf( frgfd, LHFRAGMENT_MAGIC "\t%04hX\n", asmorg );
        //fprintf( frgfd, "\t%s\t%04hX\n", fragment, asmorg );
        lhAsmNewFragment( asmpctx->state, fragment, NULL, NULL );
        }
nofragdump:
      asmcurrent = &asmmem[asmorg];
      }

    memset( asmincluded, 0, sizeof(asmincluded) );

    if( 0 == setjmp( asmpctx->env ) )
      {
      lhAsmDoInclude( asmflags & LHASM_IMMEDIATE ? NULL : asmpctx->srcfile );

      if( asmpctx->ifstate )
        LHASM_FTL( ".IF block open at line %d not closed\n", asmpctx->iflinenum );
      if( asmpctx->nopstate )
        LHASM_FTL( ".NOP block open at line %d not closed\n", asmpctx->noplinenum );
      if( asmpctx->nndostack != asmnndostack )
        LHASM_FTL( "IF/ELSE/ENDIF BEGIN/WHILE/REPEAT BEGIN/UNTIL stack not empty\n" );

      if( asmpctx->state == LHASM_BASIC )
        {
        *(asmcurrent++) = 0xFF;
        if( logfile )
          fprintf( logfd, "\t%04hX\t%02X            \t%s\n",
                   lhAsmGetPC(), *(asmcurrent - 1), "[END BASIC MARKER]");
        asmaddr++;
        }
      }

    if( frgfile )
      fclose( frgfd );
    if( keywfile )
      fclose( keywfd );
    if( logfile && logfd != stderr )
      fclose( logfd );
    logfd = keywfd = frgfd = NULL;

    if( asmpctx->error )
      break;

    if( asmflags & (LHASM_PASS_1_ONLY|LHASM_IMMEDIATE) )
      break;
    }

  if( (asmflags & (LHASM_JR_TO_JP|LHASM_JP_DO_OPTIMIZE)) && asmJRJPpass )
    {
    int ii, iii;

    asmsym = lhSymFlushAll( asmsym );
    for( ii = 0; ii < LHASM_MAX_LOCAL; ii++ )
      asmlocal[ ii ] = lhSymFlushAll( asmlocal[ ii ] );
    asmincsym = lhSymFlushAll( asmincsym );
    asmkywsym = lhSymFlushAll( asmkywsym );
    lhBasicFlushAll();
    for( ii = 0; ii < asmnnmacro; ii++ )
      for( iii = 0; iii < asmmacro[ii].nnline; iii++ )
        free( asmmacro[ii].lines[iii] );
    asmnnmacro = asmnnstruct = 0;
    asmpass = 0;
    asmJRJPpass = asmnnJRJP = 0;
    if( --asmJRJPmaxloop )
      goto redoJRoptimize;
    else if( asmflags & LHASM_JP_DO_OPTIMIZE )
      {
      asmJRJPloop = 1;
      goto redoJRoptimize;
      }
    else
      printf("JR TO JP optimization stopped after %u loops\n", asmJRJPloop );
    }

  if( symfile )
    {
    PLHSYM psym = asmsym;
    int ilocal;

    while(psym != (PLHSYM) NULL)
      {
      fprintf(symfd, "\t%04hX\t%s\n", psym->value, psym->name);
      psym = psym->next;
      }
    fprintf( symfd, "\n" );

    for( ilocal = 0; !(asmflags & LHASM_NOLOCAL) && ilocal < LHASM_MAX_LOCAL; ilocal++ )
      {
      psym = asmlocal[ ilocal ];
      while(psym != (PLHSYM) NULL)
        {
        if( *(psym->name) != ' ' )
          fprintf(symfd, "\t%04hX\t%s\n", psym->value, psym->name);
        psym = psym->next;
        }
      if( asmlocal[ ilocal ] )
        fprintf( symfd, "\n" );
      }
    psym = resym;
    while(psym != (PLHSYM) NULL)
      {
      fprintf(symfd, "\t%04hX\t%s\n", psym->value, psym->name);
      psym = psym->next;
      }
    fprintf( symfd, "\n" );
    fclose( symfd );
    }
  if( macfile )
    {
    int nnm, nnl;

    for( nnm = 0; nnm < asmnnmacro; nnm++ )
      {
      fprintf( macfd, ".MACRO: %s\n", asmmacro[nnm].name );
      for( nnl = 0; nnl < asmmacro[nnm].nnline; nnl++ )
        fprintf( macfd, "\t%s\n", asmmacro[nnm].lines[nnl] );
      fprintf( macfd, ".ENDMACRO\n\n" );
      }
    fclose( macfd );
    }
  if( asmpctx->error )
    {
    fprintf( stderr, "Aborting due to previous error...\n" );
    exit( 127 );
    }
  if( asmtop > asmorg )
    asmtop = asmorg;
  if( asmbottom < lhAsmGetPC() )
    asmbottom = lhAsmGetPC();

  for( l = asmtop;  l < asmbottom; l++ )
    cksm += asmmem[l];

  if( zphdr )
    {
    LHU16 zstart, zlength;
    char name[ LHCE158HDR_NAME_LENGTH ];

    printf( "Fixing CE158 CSAVE header ..." );
    if( !zname )
      {
      char *p, *ps;
      int  ll = strlen( asmpctx->srcfile );

      memset( name, 0, sizeof(name) );
      if( (ps = strrchr( asmpctx->srcfile, '/' )) )
        ps++;
      else
        ps = asmpctx->srcfile;
      if( (p = strrchr( ps, '.' )) )
        ll = p - ps;
      strncpy( zname = name, ps, ll > LHCE158HDR_NAME_LENGTH ? LHCE158HDR_NAME_LENGTH : ll);
      }
    zstart = lhAsmGetSymValue(asmsym, "__MAIN__$$._start");
    zlength = lhAsmGetSymValue(asmsym, "__MAIN__$$._length");
    if( 0xFFFF == zentry )
      zentry = lhAsmGetSymValue(asmsym, "STARTUP");
    if( 0xFFFF == zentry )
      zentry = lhAsmGetSymValue(asmsym, "__MAIN__$$._startup");
    if( zmagic )
      ;
    else if( LHASM_BASIC == zstate )
      zmagic = LHCE158HDR_CSAVE_MAGIC;
    else if( LHASM_RESERVE == zstate )
      zmagic = LHCE158HDR_CSAVEr_MAGIC;
    else
      zmagic = LHCE158HDR_CSAVEM_MAGIC;
    printf( " '%s'\n", zmagic );
    if( LHCE158HDR_LENGTH != lhCE158HdrBuild( zmagic, zphdr, zname, zstart, zlength, zentry ) )
      {
      fprintf( stderr, "Failed to build the CSAVE header\n" );
      zphdr = NULL;
      }
    }
  if( !(asmflags & (LHASM_NOBINARY | LHASM_PASS_1_ONLY)) )
    {
    FILE *fdw;

    if( *outfile == 0 )
      {
      char *p;

      if( (p = strrchr(strcpy( outfile, (asmflags & LHASM_IMMEDIATE ? "stdin" : asmpctx->srcfile) ), '.' )) )
        *p = 0;
      strcat( outfile, (asmflags & LHASM_OUTPUT_HEX ? ".hex" : ".bin") );
      }

    if( 0 > (fdw = fopen( outfile, (asmflags & LHASM_OUTPUT_HEX ? "w" :"wb") )) )
      {
      fprintf( stderr, "Unable to open outfile %s\n", outfile );
      exit( 2 );
      }

    for( l = asmtop;  l < asmbottom; l++ )
      cksm += asmmem[l];

    if( asmflags & LHASM_OUTPUT_HEX )
      {
      if( zphdr )
        {
        printf("Written CE158 CSAVE header\n");
        fprintf( fdw, " %02X\n", zphdr[0] );
        for( l = 1; l < 17; l++ )
          fprintf( fdw, " %02X", zphdr[l] );
        fprintf( fdw, "\n" );
        for( l = 17; l < 23; l++ )
          fprintf( fdw, " %02X", zphdr[l] );
        fprintf( fdw, "\n" );
        }
      printf("Written from %04X to %04X into %s Checksum %04X\n",
             (unsigned int)asmtop, (unsigned int)asmbottom, outfile, cksm);
      for( l = asmtop & 0xFFF0; l < asmtop; l++ )
        fprintf( fdw, "   " );
      for( l = asmtop; l < asmbottom; l++ )
        {
        fprintf( fdw, " %02X", asmmem[l] );
        if( 15 == (l % 16) )
          fprintf( fdw, "\n" );
        }
      fprintf( fdw, "\n" );
      }
    else
      {
      if( zphdr )
        {
        printf("Written CE158 CSAVE header\n");
        fwrite( zphdr, sizeof(zhdr), 1, fdw );
        }
      fwrite( &asmmem[asmtop], (unsigned int)asmbottom - (unsigned int)asmtop, 1, fdw );
      printf("Written %u bytes (%04hX:%04hX) to %s Checksum %04X\n",
             (unsigned int)asmbottom - (unsigned int)asmtop, asmtop, asmbottom, outfile, cksm);
      }
    fclose( fdw );
    }
  exit( 0 );
}
