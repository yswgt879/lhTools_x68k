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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lhtype.h"
#include "lhfrag.h"
#include "lhsym.h"
#include "lhunasm.h"
#include "lhbasic.h"
#include "lhce158hdr.h"

extern const char const lhTools_vers[], lhTools_date[], lhTools_pltf[];

struct s_lhlistfrag {
  struct s_lhlistfrag *next;
  LHFRAG               frag;
};

typedef struct s_lhlistfrag  LHLISTFRAG;
typedef struct s_lhlistfrag *PLHLISTFRAG;

void usage(PLHU8 prog)
{
  fprintf(stderr,
          "Usage: %s [-h] [-v] [{-s [-inline]|-d}] [-a] [-g]\n"
          "\t\t[-D:<dis>] [-C[=[start:]addr]] [-Z]\n",
          prog);
  fprintf(stderr, "\t\t[-F infile] [-K infile] [-S infile]"
          " [-O addr] [fragment, ...]\n");
  fprintf(stderr,"\t\t[-o outfile] infile\n");
#if 0
          "\t\t[-o outfile] [infile, ...]\n", prog);
#endif
  fprintf(stderr,"where:\n");
  fprintf(stderr,"\t-a\t\tBYTE fragments are printed in HEX and ASCII\n");
  fprintf(stderr,"\t-d\t\tProduce listing file; This is the default\n");
  fprintf(stderr,"\t-g\t\tUse graphical character for &27 &5B &5C &5D and &7F\n");
  fprintf(stderr,"\t-h\t\tThis help\n");
  fprintf(stderr,"\t-o outfile\tWrite dump or source to outfile, else use stdout\n");
  fprintf(stderr,"\t-s\t\tProduce source file; exclusive with -d\n");
  fprintf(stderr,"\t-inline\t\tProduce \\asm[ .. \\]end directive if -s is active\n");
  fprintf(stderr,"\t-v\t\tShow version and exit\n");
  fprintf(stderr,"\t-C\t\tCompute full CHECKSUM\n");
  fprintf(stderr,"\t-C=addr\t\tCompute CHECKSUM to addr-1 and compare to addr\n");
  fprintf(stderr,"\t-C=start:addr\tCompute CHECKSUM from start to addr-1 and compare to addr\n");
  fprintf(stderr,"\t-D:<inst>\tIn BASIC fragment <inst> are disassembled\n");
  fprintf(stderr,"\t\twhere <inst> is DATA, POKE, REM, VAR\n");
  fprintf(stderr,"\t\tREM and VAR are disassembled only when code is found\n");
  fprintf(stderr,"\t-F fragfile\tRead fragment description from <fragfile>\n");
  fprintf(stderr,"\t-K kywfile\tRead keyword from <kywfile>\n");
  fprintf(stderr,"\t-O addr\t\tOrigin address, else start at 0000\n");
  fprintf(stderr,"\t-S symfile\tRead symbols from <symfile>\n");
  fprintf(stderr,"\t-Z\t\tExpect and use a CE158 CSAVE header\n");
#if 0
  fprintf(stderr,"\tinfile, ...\tRead binary from infile, else use stdin\n");
#endif
  fprintf(stderr,"with fragment:\n");
  fprintf(stderr,"\t-B [addr]\tBASIC fragment; This is the default\n");
  fprintf(stderr,"\t-R [addr]\tRESERVE fragment\n");
  fprintf(stderr,"\t-X [addr]\tXREG fragment\n");
  fprintf(stderr,"\t-V [addr]\tdynamic VARiables fragment\n");
  fprintf(stderr,"\t-c [addr]\tCODE fragment\n");
  fprintf(stderr,"\t-b [addr]\tBYTE (8-bits) fragment\n");
  fprintf(stderr,"\t-w [addr]\tWORD (16-bits) fragment\n");
  fprintf(stderr,"\t-l [addr]\tLONG (32-bits) fragment\n");
  fprintf(stderr,"\t-t [addr]\tTEXT fragment\n");
  fprintf(stderr,"\t-k [addr]\tKEYWORD fragment\n");
  fprintf(stderr,"\t-H [addr]\tHOLE fragment\n");
  fprintf(stderr,"\n");
  exit(1);
}

void outdigit(char **xrp, LHU16 xr[], int ndigit)
{
  int byte;

  for(byte = 0; byte < ndigit; byte++, (*xrp)++) {
    (*xrp)[0] = ((*(xr++) & 0x0f) | 0x30);
  } 
}

void outxreg(char *addr, FILE* fileout)
{
  int byte;

  if((addr[4] & 0xff) == 0xB2)
  fprintf(fileout, "BINARY:%11hd",
            (short)((addr[5] & 0xff) << 8 | addr[6] & 0xff));
  else if((addr[4] & 0xff) == 0xD0)
    fprintf(fileout, "ADDR: %02hX%02hX LEN: %2hu",
            addr[5] & 0xff, addr[6] & 0xff, addr[7] & 0xff);
  else {
    int xrE, xrN; LHU16 xr[10];
    char xrbuf[17], *xrp = xrbuf;
    memset(xrbuf, 0, sizeof(xrbuf));
    for(byte = 0; byte < 10; byte += 2) {
      xr[byte] = ((addr[(byte >> 1) + 2]) >> 4) & 0x0f;
      xr[byte + 1] = (addr[(byte >> 1) + 2]) & 0x0f;
    }
    for(xrN = 10; byte > 0 && !xr[--byte]; xrN--)
      ;
    xrE = ((byte = *(addr) & 0xff) > 0x7f ? 256 - byte : byte);
    *(xrp++) = (*(addr + 1) & 0x80 ? '-' : ' ');
    if((xrE + xrN) <= 10 || (xrE <= 10)) {
      if((*(addr) & 0xff) > 0x7f) {
        *(xrp++) = '0';
        *(xrp++) = '.';
        for(byte = 0; byte < xrE - 1; byte++)
          *(xrp++) = '0';
        outdigit(&xrp, xr, xrN);
      }
      else {
        outdigit(&xrp, xr, xrE + 1);
        if(xrN > xrE) {
          *(xrp++) = '.';
          outdigit(&xrp, xr + xrE + 1, xrN - xrE - 1);
        }
      }
    }
    else {
      outdigit(&xrp, xr, 1);
      *(xrp++) = '.';
      outdigit(&xrp, xr + 1, xrN - 1);
      *(xrp++) = 'E';
      *(xrp++) = (*(addr) & 0x80 ? '-' : '+');
      sprintf(xrp, "%d", xrE);
    }
    fprintf(fileout, "%18s", xrbuf);
  }
}

int main(argc, argv)
  int  argc;
  char *argv[];
{
  int    source = 0 /*-d=0,-s=1*/, checksum = 0; /*-C*/
  int    ascgraph = 0 /*-g*/, ascbyte = 0 /*-a*/;
  LHU16   basdis, disrem = 0, dispoke =0 , disdata = 0, disvar = 0, iscode, inlinbas = 0;
  char   *fragfile = NULL, *keywfile = NULL, *symfile = NULL;
  char   *outfile = NULL;
  FILE   *filein, *fileout, *fksfilein, *filenull;
  PLHU8 addr, memory, stopaddr, inlinstart, oldaddr, chksumaddr = 0, chksumstart = 0;
  LHU8  string[1024];
  int    rch, byte;
  LHU16   base = 0x40C5, top, bottom, fullchksum = 0, chksum = 0, chksumfrom = 0, chksumbase = 0, rlen = 0;
  PLHFRAG  frag;
  PLHFRAG  keywordfrag = NULL;
  PLHLISTFRAG glistf = NULL, tlistf, plistf = NULL;
  PLHSYM   sym = (PLHSYM) NULL;
  PLHU8 name;
  LHU16   nnnsym = 0;
  LHU16   basline, baslen, basi, bascodelen, basdq;
  PLHU8 basname, bascode;
  char basjr, upass = 0;
  LHU8 varnambuf[80], *var[26], *varcode[26];
  int varcodelen[26], varpush;
  int start = 0;
  int iargc, argerr;
  char *argvc;
  LHU16 argmagic = 0, arghx, lasthx = 0;
  long inlength, memlen;
  struct stat st;
  char cvers[4];
  LHU8 zhdr[ LHCE158HDR_LENGTH ], *pzhdr = NULL, zname[ LHCE158HDR_NAME_LENGTH+1 ];
  LHU8 zmagic[ LHCE158HDR_MAGIC_LENGTH + 1 ];
  LHU16 zentry, zstart, zlength;

  memset( cvers, 0, sizeof(cvers) );
  sscanf( lhTools_vers, "%hhu.%hhu.%hhup%hhu", &cvers[0], &cvers[1], &cvers[2], &cvers[3] );
  
  top = bottom = base;

  for( argerr = 0, iargc = 1; !argerr && iargc < argc; iargc++ ) {
    if(*(argvc = argv[iargc]) == '-') {
      switch(*(++argvc)) {
        case 'h': usage(argv[0]);
                  break;
        case 'v': if( '#' == *(++argvc) )
                    {
                    printf( "%hhu.%hhu.%hhu", cvers[0], cvers[1], cvers[2] );
                    if( cvers[3] )
                      printf( "p%hhu", cvers[3] );
                    printf( "\n" );
                    }
                  else
                    fprintf( stderr, "lhTools Version %s\n"
                                     "        Compiled %s\n"
                                     "        Platform %s\n"
                                     "Copyright 1992-2015 Christophe Gottheimer\n\n",
                                     lhTools_vers, lhTools_date, lhTools_pltf );
                  exit( 0 );
                  break;
        case 'd': 
        case 's': upass = source = (*argvc == 's' ? 1 : 0);
                  break;
        case 'a': ascbyte = 1;
                  break;
        case 'g': ascgraph = 1;
                  break;
        case 'i': if( !(argerr = strcmp( argvc, "inline" )) )
                    inlinbas = 1;
                  break;
        case 'D': if(*(++argvc) == ':') {
                    if(!strcmp("DATA", ++argvc))
                      disdata = lhBasicGetCode(argvc);
                    else if(!strcmp("POKE", argvc))
                      dispoke = lhBasicGetCode(argvc);
                    else if(!strcmp("REM", argvc))
                      disrem = lhBasicGetCode(argvc);
                    else if(!strcmp("VAR", argvc))
                      disvar = 0x2424;
                    else {
                      fprintf(stderr,"Unknown -D:%s\n", argvc);
                      exit(1);
                    }
                  }
                  else
                    argerr = 1;
                  break;
        case 'C': checksum = 1;
                  if(*(++argvc) == '=') {
                    if(strchr((++argvc), ':')) {
                      if(sscanf(argvc, "%hx:%hx", &chksumfrom, &chksumbase) == 2) {
                        checksum = 6;
                      }
                    }
                    else if(sscanf(argvc, "%hx", &chksumbase) == 1) {
                      checksum = 2;
                    }
                    if(checksum == 1) {
                      fprintf(stderr,"Invalid CHECKSUM address.\n");
                      exit(1);
                    }
                  }
                  break;
        case 'O': if(!(argerr = (++iargc == argc)))
                    if(sscanf(argv[iargc], "%hx", &base) != 1) {
                      fprintf(stderr,"Invalid ORIGIN address.\n");
                      exit(1);
                    }
                  top = bottom = lasthx = (unsigned)base;
                  break;
        case 'B': if(!argmagic) argmagic = FRAG_BASIC;
        case 'R': if(!argmagic) argmagic = FRAG_RESERVE;
        case 'X': if(!argmagic) argmagic = FRAG_XREG;
        case 'V': if(!argmagic) argmagic = FRAG_VAR;
        case 'c': if(!argmagic) argmagic = FRAG_CODE;
        case 'b': if(!argmagic) argmagic = FRAG_BYTE;
        case 'w': if(!argmagic) argmagic = FRAG_WORD;
        case 'l': if(!argmagic) argmagic = FRAG_LONG;
        case 't': if(!argmagic) argmagic = FRAG_TEXT;
        case 'k': if(!argmagic) argmagic = FRAG_KEYWORD;
        case 'H': if(!argmagic) argmagic = FRAG_HOLE;
                  if(!(argerr = (++iargc == argc)))
                    if(sscanf(argv[iargc], "%hx", &arghx) != 1) {
                      fprintf(stderr,
                              "Invalid address for fragment -%c.\n", *argvc);
                      exit(1);
                    }
                  if(lasthx > arghx) {
                    fprintf(stderr,
                           "Previous address > current address.\n");
                    exit(1);
                  }
                  if(!(tlistf = (PLHLISTFRAG)malloc(sizeof(LHLISTFRAG)))) {
                    fprintf(stderr,
                            "Unable to allocate LHFRAGMENT descriptor.\n");
                    exit(2);
                  }
                  tlistf->frag.magic = argmagic;
                  tlistf->frag.chksum = 0;
                  tlistf->frag.origin = arghx;
                  tlistf->frag.length = 0;
                  tlistf->frag.comment = NULL;
                  tlistf->next = NULL;
                  if(glistf == NULL)
                    glistf = tlistf;
                  else
                    plistf->next = tlistf;
                  plistf = tlistf;
                  argmagic = 0;
                  break;
        case 'o': if(!(argerr = (++iargc == argc || outfile)))
                    outfile = argv[iargc];
                  break;
        case 'F': if(!(argerr = (++iargc == argc || fragfile)))
                    fragfile = argv[iargc];
                  break;
        case 'K': if(!(argerr = (++iargc == argc || keywfile)))
                    keywfile = argv[iargc];
                  break;
        case 'S': if(!(argerr = (++iargc == argc || symfile)))
                    symfile = argv[iargc];
                  break;
        case 'Z': pzhdr = zhdr;
                  break;
        default:
          argerr = 1; break;
      }
    }
    else
      break;
  }

  if(argerr || iargc >= argc) usage(argv[0]);
  if(fragfile) {
    if(glistf) {
      fprintf(stderr,
              "LHFRAGMENT are already specified.\n");
      exit(2);
    }
    if(!(fksfilein = fopen(fragfile, "r"))) {
      fprintf(stderr, "Cannot open fragments file %s\n", fragfile);
      exit(2);
    }
    if(fscanf(fksfilein, "%s %hx", string, &base) != 2 ||
              strcasecmp(string, LHFRAGMENT_MAGIC)) {
      fprintf(stderr, "Hmmm... Not a fragment file %s\n", fragfile);
      fclose(fksfilein);
      exit(2);
    }
    top = bottom = lasthx = (unsigned)base;
    if(fscanf(fksfilein, "%s", string) == 1) {
      while(fscanf(fksfilein, "%hxs", &arghx) == 1) {
        if(!(argmagic = lhFragmentGetMagic(string))) {
          fprintf(stderr,
                  "Unknown fragment type \"%s\".\n", string);
          fclose(fksfilein);
          exit(1);
        }
        if(lasthx > arghx) {
          fprintf(stderr,
                  "Previous address > current address.\n");
          fclose(fksfilein);
          exit(1);
        }
        if(!(tlistf = (PLHLISTFRAG)malloc(sizeof(LHLISTFRAG)))) {
          fprintf(stderr,
                  "Unable to allocate LHFRAGMENT descriptor.\n");
          fclose(fksfilein);
          exit(2);
        }
        tlistf->frag.magic = argmagic;
        tlistf->frag.chksum = 0;
        tlistf->frag.origin = arghx;
        tlistf->frag.length = 0;
        tlistf->frag.comment = NULL;
        tlistf->next = NULL;
        if(glistf == NULL)
          glistf = tlistf;
        else
          plistf->next = tlistf;
        plistf = tlistf;
        if(fscanf(fksfilein,"%s",string) == 1 &&
           strcmp(string, "COMMENT") == 0 &&
           fgets(string, sizeof(string), fksfilein) == (char*)string) {
          if((byte = strlen(string)) > 1) {
            string[--byte]='\0';
            if(!(tlistf->frag.comment = (PLHU8)malloc(byte + 1))) {
              fprintf(stderr,
                      "Unable to allocate COMMENT for LHFRAGMENT.\n");
              fclose(fksfilein);
              exit(2);
            }
            strcpy(tlistf->frag.comment, string);
          }
          fscanf(fksfilein,"%s",string);
        }
      }
    }
    fclose(fksfilein);
  }
  if(glistf == NULL) {
    if(!(glistf = (PLHLISTFRAG)malloc(sizeof(LHLISTFRAG)))) {
      fprintf(stderr,
              "Unable to allocate LHFRAGMENT descriptor.\n");
      exit(2);
    }
    glistf->frag.magic = FRAG_BASIC;
    glistf->frag.chksum = 0;
    glistf->frag.origin = base;
    glistf->frag.length = 0;
    glistf->frag.comment = NULL;
    glistf->next = NULL;
  }

  base = glistf->frag.origin;

  if(keywfile) {
    int ret;

    if( 0 > (ret = lhBasicImportKeyword( keywfile, NULL )) )
      {
      if( -1 == ret )
        fprintf(stderr, "Cannot open keyword file %s\n", keywfile);
      else
        fprintf(stderr, "Hmmm... Not a keyword file %s\n", keywfile);
      exit( 2 );
      }
  }

  if(stat(argv[iargc], &st)) {
    fprintf(stderr, "Cannot stat binary file %s\n", argv[iargc]);
    exit(2);
  }

  memlen = inlength = st.st_size;
  if((filein = fopen(argv[iargc], "rb")) == (FILE *)NULL) {
    fprintf(stderr, "Cannot open binary file %s\n", argv[iargc]);
    exit(2);
  }

  if( pzhdr ) {
    memset( zmagic, 0, sizeof(zmagic) );
    memset( zname, 0, sizeof(zname) );
    if( 1 != fread( zhdr, sizeof(zhdr), 1, filein ) ) {
      fprintf(stderr, "Cannot read CSAVE header\n");
      exit(2);
    }
    if( LHCE158HDR_LENGTH != lhCE158HdrExplode( zmagic, pzhdr, zname, &zstart, &zlength, &zentry ) ) {
    badzmagic:
      fprintf(stderr, "Does not seem to be a CSAVE header... Skip it!\n");
      pzhdr = NULL;
    }
    else {
      if( 0 == memcmp( zmagic, LHCE158HDR_CSAVE_MAGIC, LHCE158HDR_MAGIC_LENGTH ) )
        glistf->frag.magic = FRAG_BASIC;
      else if( 0 == memcmp( zmagic, LHCE158HDR_CSAVEM_MAGIC, LHCE158HDR_MAGIC_LENGTH ) )
        glistf->frag.magic = FRAG_CODE;
      else if( 0 == memcmp( zmagic, LHCE158HDR_CSAVEr_MAGIC, LHCE158HDR_MAGIC_LENGTH ) )
        glistf->frag.magic = FRAG_RESERVE;
      else if( 0 == memcmp( zmagic, LHCE158HDR_PRINT_MAGIC, LHCE158HDR_MAGIC_LENGTH ) )
        glistf->frag.magic = FRAG_BYTE;
      else
        goto badzmagic;
      memlen = inlength = zlength;
      printf( "CSAVE Header OK.\n"
              "\tHEADER\t\"%s\"\n"
              "\tNAME\t\"%s\"\n"
              "\tSTART\t %04hX\n\tLENGTH\t%5hu\n\tENTRY\t %04hX\n\n",
              zmagic, zname, zstart, zlength, zentry );
      if( zentry != 0xFFFF )
        sym = lhSymAddSorted( sym, "__MAIN__$$._startup", zentry, SYM_NONE );
      glistf->frag.origin = zstart;
    }
  }

  for(plistf = glistf, rlen = inlength; plistf; plistf = plistf->next) {
    if(plistf->next) {
      plistf->frag.length = plistf->next->frag.origin - plistf->frag.origin;
      if(plistf->frag.length > rlen) {
        fprintf(stderr,
                "LHFRAGMENT goes outside.\n");
        exit(2);
      }
      rlen -= plistf->frag.length; 
    }
    else
      plistf->frag.length = rlen;
    if(plistf->frag.magic == FRAG_KEYWORD)
      keywordfrag = &(plistf->frag);
  }

  if(outfile == NULL)
    fileout = stdout;
  else {
    if((fileout = fopen(outfile, "w")) == (FILE *)NULL) {
      fprintf(stderr, "Cannot open assembler file %s\n", outfile);
      fclose(filein);
      exit(2);
    }
  }

  if(symfile) {
    int ret;

    if( 0 > (ret = lhSymImportSymbol( symfile, &sym, SYM_NONE )) )
      {
      if( -1 == ret )
        fprintf(stderr, "Cannot open symbols file %s\n", symfile);
      else
        fprintf(stderr, "Hmmm... Not a symbols file %s\n", symfile);
      fclose(filein);
      fclose(fileout);
      exit( 2 );
      }
    nnnsym = ret;
    fprintf(stderr, "%5hu symbol(s) read\n", nnnsym);
  }

  memory = (char *)malloc(inlength);

  if(fread(memory, inlength, 1, filein) != 1) {
    fputs("Length error while reading code\n", stderr);
    fclose(filein);
    fclose(fileout);
    exit(2);
  }
  
  if(keywordfrag) {
    lhBasicRegister(memory + keywordfrag->origin - base);
  }

  addr = memory;
  if(checksum & 4)
    chksumstart = addr + chksumfrom - base;
  else
    {
    chksumstart = addr;
    chksumfrom = base;
    }
  if(checksum & 2)
    chksumaddr = addr + chksumbase - base;
  else
    chksumaddr = addr + inlength;
  if((chksumbase - chksumfrom) > inlength) {
    fprintf(stderr, "CHECKSUM is outside binary: %04hX > %04X\n",
            chksumbase, inlength);
    exit(2);
  }
  while(inlength--) {
    if(chksumstart <= addr && addr < chksumaddr) {
      chksum += (*addr) & 0xff;
    }
    fullchksum += (*(addr++)) & 0xff;
  }

dosource:
  bottom = top + (LHU16)memlen;
  if( upass == 1 )
    {
    filenull = fileout;
    if( NULL == (fileout = fopen( "/dev/null", "w" )) )
      {
      fprintf(stderr, "Cannot open null-assembler file %s\n", outfile);
      fclose(filein);
      fclose(filenull);
      exit(2);
      }
    }
  else if( upass == 2 )
    {
    int iiargc, larg;

    fclose( fileout );
    fileout = filenull;

    fprintf( fileout, ";\n"
                      ";\tlhTools Version %s\n"
                      ";\t        Compiled %s\n"
                      ";\t        Platform %s\n",
             lhTools_vers, lhTools_date, lhTools_pltf );
    fprintf( fileout, ";\n"
                      ";\tBinary file: %s\n", argv[iargc] );
    fprintf( fileout, ";\tlhdump options:\n"
                      ";\t\t" );
    for( larg = 0, iiargc = 1; iiargc < (argc - 1); iiargc++ )
      {
      larg += fprintf( fileout, "%s ", argv[iiargc] );
      if( larg <= 80 && iiargc < (argc - 2) && '-' != *argv[iiargc + 1] )
        {
        iiargc++;
        larg += fprintf( fileout, "%s ", argv[iiargc] );
        }
      if( larg > 80 )
        {
        fprintf( fileout, "\\\n"
                          ";\t\t" );
        larg = 0;
        }
      }
    fprintf( fileout, "\\\n"
                      ";\t\t%s\n\n", argv[iiargc] );
    }
  stopaddr = memory;
  for(plistf = glistf; plistf; plistf = plistf->next) {
    base = (frag = &(plistf->frag))->origin;
    addr = stopaddr;
    stopaddr = addr + frag->length;

    if(source) {
      if(plistf == glistf)
        fprintf(fileout,"\t.ORIGIN: %04hX\n",frag->origin);
      fprintf(fileout, "\t.%s\n", lhFragmentGetFragment(frag->magic));
      fprintf(fileout, "\t; %04hX %04hX (length %04hX)\n",
              frag->origin, frag->origin + frag->length, frag->length);
    }
    if(frag->comment)
      fprintf(fileout, "\t%s %s\n",
              (source ? ".COMMENT:" : ";"), frag->comment);

    while(addr < stopaddr) {
      fflush(stdout);
      if(frag->magic != FRAG_BASIC) {
        LHU8 bits;
        if((basi = lhBasicGetCodeFromJump(base, &bits)) &&
          (basname = lhBasicGetBasic(basi))) {
          fprintf(fileout, "\n");
          while( basi && basname ) {
            if(source)
              fprintf(fileout,"\t.DEFINE: \"%s\"\t= %04hX %c\n",
                      basname, basi, bits);
            else
              fprintf(fileout,"%04hX\t[%04hX],%c   \"%s\"\n",
                      base, basi, bits, basname);
            if(basi = lhBasicGetNextCodeFromJump(base, basi, &bits))
              basname = lhBasicGetBasic(basi);
          }
        }
        if((name = lhSymGetName(sym, base)) != (PLHU8) NULL)
          fprintf(fileout, "%s%s:\n%s",
                  (source ? "" : "\n"), name, (source ? "\t" : ""));
        if(!source)
          fprintf(fileout, "%04hX\t", base);
        else
          fprintf(fileout, "%s\t", (name ? "\0" : "\t"), base);
      }
      switch(frag->magic) {
        case FRAG_CODE:
             oldaddr = addr;
             addr = lhUnasm(&base, addr, string, &sym, 0, upass, top, bottom);
             if(!source) {
               for(byte = 0; oldaddr < addr && byte < 5; oldaddr++, byte++)
                 fprintf(fileout,"%02X ", (unsigned int)(*oldaddr) & 0xff);
               while(byte++ < 6)
                 fprintf(fileout,"   ");
             }
             fprintf(fileout, "%s", string);
             if(!source)
               for(byte = 0; oldaddr < addr; oldaddr++, byte++) {
                 if((byte % 5) == 0)
                   fprintf(fileout,"\n\t");
                 fprintf(fileout,"%02X ", (unsigned int)(*oldaddr) & 0xff);
               }
             fprintf(fileout,"\n");
             break;
        case FRAG_BYTE:
             for(byte = 0; addr < stopaddr && byte < 16; byte++, base++) {
               fprintf(fileout,"%02X ", (unsigned)(*(addr++)) & 0xff);
               if(!ascbyte && byte == 7) fputs("  ",fileout);
             }
             if(ascbyte) {
               fprintf(fileout, "%*s  ", (16 - byte) * 3, "");
               for(addr -= byte, byte = 0; addr < stopaddr && byte < 16; byte++, addr++) {
                 fputc((*(addr) & 0x7f) < 0x20 ? '.' : (*(addr) & 0x7f), fileout);
               }
             }
             fprintf(fileout,"\n");
             break;
        case FRAG_WORD:
             for(byte = 0; addr < stopaddr && byte < 16; byte++, base++) {
               fprintf(fileout,
                       "%02X", (unsigned)(*(addr++)) & 0xff);
               if(byte & 1) fputc(' ', fileout);
               if(byte == 7) fputs("  ",fileout);
             }
             if(byte & 1) fputs("??", fileout);
             fprintf(fileout,"\n");
             break;
        case FRAG_LONG:
             for(byte = 0; addr < stopaddr && byte < 16; byte++, base++) {
               fprintf(fileout,
                       "%02X", (unsigned)(*(addr++)) & 0xff);
               if((byte & 3) == 3)fputc(' ', fileout);
               if(byte == 7) fputs("  ",fileout);
             }
             for(byte &= 3; byte & 3; byte++)
               fputs("??", fileout);
             fprintf(fileout,"\n");
             break;
        case FRAG_TEXT:
             fprintf(fileout, "\"");
             for(byte = 0; addr < stopaddr && byte < 32; byte++, base++) {
               if((rch = (*(addr++)) & 0xff) > 0x1f && rch < 0x80 && rch != 0x22 ) {
                 fputc(rch, fileout);
                 if((LHU8)rch == '\\') fputc('\\', fileout);
                 if((LHU8)rch == '"') fputs("\\22", fileout);
               }
               else {
                 if(rch > 0x9f && rch != 0xa2) {
                   fprintf(fileout,"%c\\+80", (LHU8)rch & 0x7f);
                   byte += 4;
                 }
                 else {
                   fprintf(fileout,"\\%02X", rch);
                   byte += 2;
                 }
               }
             }
             fprintf(fileout, "\"\n");
             break;
        case FRAG_KEYWORD:
             if(addr < stopaddr) {
               byte = (*(addr++)) & 0xff;
               base++;
               fprintf(fileout, "%02X \"", byte);
             }
             for(rch = (byte &= 0x0f) ; addr < stopaddr && byte > 0; byte--, base++) {
               fputc((*(addr++)) & 0xff, fileout);
             }
             fprintf(fileout, "\"%*s ", (12 - rch), "");
             for(byte = 0; addr < stopaddr && byte < 4; byte++, base++) {
               fprintf(fileout,
                       "%02X", (unsigned)(*(addr++)) & 0xff);
               if(byte & 1) fputc(' ', fileout);
             }
             if(byte & 1) fputs("??", fileout);
             fprintf(fileout,"\n");
             break;
        case FRAG_BASIC:
             basi = frag->length;
             for( ; addr < stopaddr; ) {
               if(basi < 2) {
                 byte = (*addr) & 0xff;
                 basi--;
                 fprintf(fileout,"%u??\n", byte);
                 break;
               }
               bascode = NULL;
               basdis = 0;
               inlinstart = 0;
               basline = ((*addr & 0xff) << 8) | (*(addr + 1) & 0xff);
               addr += 2;
               basi -= 2;
               if( basline > 0xfeff )
                 {
                 break;
                 }
               fprintf(fileout, "%u ", basline);
               if(basi < 1) {
                 fprintf(fileout,"??\n");
                 break;
               }
               baslen = *(addr++) & 0xff;
               basi--;
               memset((var[0] = &varnambuf[0]), 0, sizeof(varnambuf));
               memset(varcode, 0, sizeof(varcode));
               memset(varcodelen, 0, sizeof(varcodelen));
               varpush = 0;
               basdq = 0;
               for(start = 1; baslen > 1; start++) {
                 if(basi < 1) {
                   fprintf(fileout,"??\n");
                   break;
                 }
                 byte = *(addr++) & 0xff;
                 if(byte == ':' && !basdq) {
                   start = 0;
                 }
                 basi--;
                 baslen--;
                 if(byte == '"')
                   basdq = !basdq;
                 if(!basdq && disvar && start > 0) {
                   if(start == 1 && isupper(byte))
                     var[varpush][0] = byte;
                   else if(start == 2 &&
                           (isupper(byte) || isdigit(byte) || byte == '$'))
                     var[varpush][1] = byte;
                   else if(start == 3 && byte == '$')
                     var[varpush][2] = byte;
                   else
                     start = 0;
                   if(byte == '$' && (start == 2 || start == 3)) {
                     int ivar = 0, tvar = 0;
                     int code = 0;
                     if(addr[tvar] == '(')
                       for(tvar = 0;
                           tvar < baslen - 1 &&
                           (var[varpush][tvar++ + start] = addr[tvar]) != ')';
                          )
                         ;
                     var[varpush][tvar + start] = '\0';
                     if((tvar < baslen - 3) &&
                        (addr[tvar] == '=' && addr[tvar + 1] == '"')) {
                       for(ivar = tvar + 2;
                           ivar < baslen - 1 && addr[ivar] != '"';
                           ivar++)
                         if((addr[ivar] & 0xff) > 0x7f ||
                            (addr[ivar] & 0xff) < 0x20) {
                           code = 1;
                         }
                     }
                     if(code && (ivar - tvar) > 2) {
                       varcode[varpush] = addr + tvar + 2;
                       varcodelen[varpush] = ivar - tvar - 2;
                       varpush++;
                       if(varpush == sizeof(varcode)/sizeof(int)) {
                         fprintf(stderr,
                                 "Internal variables disassembled stack exceed!");
                         exit(2);
                       }
                       var[varpush] = &(var[varpush - 1][tvar + start + 1]);
                     }
                     else
                       memset(var[varpush], 0, start + tvar);
                     start = 0;
                   }
                 }
                 if((basdis || varpush) && (source & inlinbas) && !inlinstart) {
                   if((dispoke == basdis) && (byte == ',')) {
                     inlinstart = addr;
                     fputc( byte, fileout);
                   }
                   else if((disdata == basdis || basdis == disrem)) {
                     inlinstart = addr-1;
                   }
                   else if((disvar && varpush) && (basdq && byte == '"')) {
                     inlinstart = addr;
                     fputc( byte, fileout);
                   }
                 }
                 if(!basdq && byte >= 0xe0) {
                   if(basi < 2) {
                     fprintf(fileout,"%02X??\n", byte);
                     break;
                   }
                   byte = (byte << 8 | (*(addr++)) & 0xff);
                   basi--;
                   baslen--;
                   if(!basdis &&
                      (byte == disdata || byte == dispoke || byte == disrem)) {
                     basdis = byte;
                     bascode = addr;
                     bascodelen = baslen;
                     iscode = 0;
                   }
                   if((basname = lhBasicGetBasic((LHU16)byte)) != NULL)
                     fprintf(fileout, "%s ", basname);
                   else
                     {
                     fprintf(fileout, "\\%04X ", byte);
                     iscode = 1;
                     }
                 }
                 else {
                   if( inlinstart ) {
                     if(byte < 0x20 || byte > 0x7f) {
                       iscode = 1;
                     }
                   }
                   else if(byte < 0x20 || byte > 0x7f) {
                     fprintf(fileout, "\\%02X", byte);
                     iscode = 1;
                   }
                   else if(byte == 0x27) {
                     if(ascgraph)
                       fputc( 0xf0, fileout);
                     else
                       fputs("\\ins", fileout);
                   }
                   else if(byte == 0x5b) {
                     if(ascgraph)
                       fputc( 0xfb, fileout);
                     else
                       fputs("\\sqr", fileout);
                   }
                   else if(byte == 0x5c) {
                     if(ascgraph)
                       fputc( 0x9d, fileout);
                     else
                       fputs("\\yen", fileout);
                   }
                   else if(byte == 0x5d) {
                     if(ascgraph)
                       fputc( 0xe3, fileout);
                     else
                       fputs("\\pi", fileout);
                   }
                   else if(byte == 0x60) {
                     fputs("\\60", fileout);
                   }
                   else if(byte == 0x7f) {
                     if(ascgraph)
                       fputc( 0xdb, fileout);
                     else
                       fputs("\\7F", fileout);
                   }
                   else {
                     fputc(byte, fileout);
                   }
                 }
                 if(!inlinstart && (*(addr) == 0x0d && (baslen == 1)))
                   fputc('\n', fileout);
                 if(!(source & inlinbas) && (baslen > 1))
                   continue;
                 if(((baslen == 1) || (!basdq && (byte == ':' || (byte == '"' && (basdis && basdis != disrem))))))
                   ;
                 else
                   continue;
                 if((bascode && basdis && (basdis != disrem || iscode)) ||
                    (disvar && varpush)) {
                   LHU8 basbuf[80], *pbasbuf;
                   PLHU8 basaddr = addr - 1;
                   int varpop = 0;
                   /*int isbase = 0;*/
                   iscode = 1;
                   basjr = 1;
                   if(basdis && basdis == dispoke) {
                     if((*bascode == '&' &&
                        sscanf(bascode, "&%hX", &base) == 1) ||
                        sscanf(bascode, "%hu", &base) == 1) {
                       basjr = 0;
                       /*isbase = 1;*/
                     }
                     while(bascode < basaddr && (byte = *(bascode++)) != ',') {
                       if(byte != '&' && !isxdigit(byte)) {
                         /*isbase = 0;*/
                         basjr = 1;
                       }
                     }
                   }
                   if(basdis && (basdis == dispoke || basdis == disdata)) {
                     pbasbuf = basbuf;
                     while(iscode && bascode < basaddr) {
                       if((*bascode == '&' &&
                          sscanf(bascode, "&%X", &byte) == 1) ||
                          sscanf(bascode, "%u", &byte) == 1) {
                         *(pbasbuf++) = byte;
                         while(bascode < basaddr &&
                              ((byte = *(bascode++)) != ',')) {
                           if(byte != '&' && !isxdigit(byte)) {
                             iscode = 0;
                           }
                         }
                       }
                       else {
                         iscode = 0;
                         basdis = 0;
                       }
                     }
                     bascode = basbuf;
                     basaddr = pbasbuf;
                     /*basjr = 0;*/
                     if(basjr) {
                       base = frag->origin;
                       /*basjr = 1;*/
                     }
                   }
                   else {
                     /*basaddr = addr - 1;*/
                     /*base = frag->origin + (basaddr - bascode);*/
                     base = frag->origin + (bascode - memory);
                     basjr = 0;
                     /*isbase = 1;*/
                   }
                   if((!iscode || (!basdis && !varpush)) && inlinstart) {
flush_inlinstart:
                     while(inlinstart < addr) {
                       byte = *(inlinstart++);
                       if(byte < 0x20 || byte > 0x7f) {
                         fprintf(fileout, "\\%02X", byte);
                       }
                       else if(byte == 0x27) {
                         if(ascgraph)
                           fputc( 0xf0, fileout);
                         else
                           fputs("\\ins", fileout);
                       }
                       else if(byte == 0x5b) {
                         if(ascgraph)
                           fputc( 0xfb, fileout);
                         else
                           fputs("\\sqr", fileout);
                       }
                       else if(byte == 0x5c) {
                         if(ascgraph)
                           fputc( 0x9d, fileout);
                         else
                           fputs("\\yen", fileout);
                       }
                       else if(byte == 0x5d) {
                         if(ascgraph)
                           fputc( 0xe3, fileout);
                         else
                           fputs("\\pi", fileout);
                       }
                       else if(byte == 0x60) {
                         fputs("\\60", fileout);
                       }
                       else if(byte == 0x7f) {
                         if(ascgraph)
                           fputc( 0xdb, fileout);
                         else
                           fputs("\\7F", fileout);
                       }
                       else {
                         fputc(byte, fileout);
                       }
                     }
                   }
                   while(iscode && (basdis || (disvar && (varpop < varpush)))) {
                     if(source)
                       fprintf(fileout,
                               (inlinbas ? (basdis ?"\t\\asm[\n" : "\\asm[\n")
                                         : "\t\t.CODE: [%s]\n"),
                                         (basdis ? lhBasicGetBasic(basdis)
                                                 : var[varpop]));
                     if(!basdis && disvar) {
                       bascode = varcode[varpop];
                       basaddr = bascode + varcodelen[varpop];
                       if((basjr =
                             (base = lhSymGetValue(sym,
                                                  var[varpop++])) == 0xffff))
                         base = 0;
                     }
                     if(source & inlinbas) {
                       basjr = 1;
                       base = 0;
                     }
                     while(bascode < basaddr) {
                       oldaddr = bascode;
                       if(source)
                         fputs("\t\t", fileout);
                       else if(!basjr || basdis == disrem)
                         fprintf(fileout, "\t; %04hX\t", base);
                       else if(!basdis && disvar && varpop <= varpush)
                         fprintf(fileout, "\t; %04hX\t", base);
                       else
                         fprintf(fileout,
                                 "\t; %4s+%04X\t",
                                 lhBasicGetBasic(basdis), base - frag->origin);
                       bascode = lhUnasm(&base, bascode, string, &sym, basjr, upass, top, bottom);
                       if(!source) {
                         for(byte = 0;
                             oldaddr < bascode && byte < 5;
                             oldaddr++, byte++)
                           fprintf(fileout, "%02X ",
                                   (unsigned int)(*oldaddr) & 0xff);
                         while(byte++ < 6)
                           fprintf(fileout,"   ");
                       }
                       fputs(string, fileout);
                       if(!source) {
                         for(byte = 0; oldaddr < bascode; oldaddr++, byte++) {
                           if((byte % 5) == 0)
                             fprintf(fileout,"\n\t");
                           fprintf(fileout, "%02X ",
                                   (unsigned int)(*oldaddr) & 0xff);
                         }
                       }
                       fprintf(fileout,"\n");
                     }
                     fprintf(fileout,
                             (inlinbas & source) ? (varpush ? "\t\\]end \"" : "\t\\]end ") :
                                (source ? "\t\t.END: [%s]\n" : "\t;\n"),
                             (basdis ? lhBasicGetBasic(basdis)
                                     : var[varpop - 1]));
                     basdis = 0;
                   }
                   if((source & inlinbas) && (baslen == 1))
                     fputc('\n', fileout);
                 }
                 else if( basdis && (basdis == disrem) && inlinstart )
                   goto flush_inlinstart;
                 bascode = NULL;
                 basdis = 0;
                 inlinstart = 0;
                 memset((var[0] = &varnambuf[0]), 0, sizeof(varnambuf));
                 memset(varcode, 0, sizeof(varcode));
                 memset(varcodelen, 0, sizeof(varcodelen));
                 varpush = 0;
                 basdq = 0;
                 iscode = 0;
               }
               if(*(addr++) != 0x0d || baslen != 1)
                 fprintf(stderr, "\nIncorrect BASIC line %u.\n", basline);
             }
             break;
        case FRAG_RESERVE:
             if((byte = *(addr) & 0xff) && byte < 0x18) {
               fprintf(fileout, "%3s.F%hx\t",
                       (char*)("III"+ (2 - (((byte >> 4) & 1) | ((byte >> 2) & 2)))), byte & 7);
               for(base++, addr++;
                   addr < stopaddr && (byte = *(addr) & 0xff) > 0x17;
                   base++, addr++) {
                 if(byte >= 0xe0 && addr < stopaddr - 1) {
                   byte = (byte << 8) | (*(addr + 1) & 0xff);
                   if(basname = lhBasicGetBasic((LHU16)byte))
                     fprintf(fileout, "%s ", basname);
                   else
                     fprintf(fileout, "\\%04X ", byte);
                   addr++;
                   base++;
                 }
                 else {
                   fputc(byte, fileout);
                 }
               }
             }
             else
               for(byte = 0; addr < stopaddr && byte < 16; byte++, base++) {
                 fprintf(fileout,"%02X ", (unsigned)(*(addr++)) & 0xff);
                 if(byte == 7) fputs("  ",fileout);
               }
             fputc('\n', fileout);
             break;
        case FRAG_VAR:
             if(stopaddr - addr >= 7) {
               char varnam[3];
               unsigned vartyp, vardim1, vardim2, varlen, varsiz;
               unsigned d1, d2, l;
               varnam[0] = *(addr++) & 0xff;
               varnam[1] = ((((vartyp = *(addr++)) & 0x5f) & 0x50) == 0x10 ?
                                 (vartyp & 0x5f | 0x20) : vartyp & 0x5f);
               varnam[2] = '\0';
               fprintf(fileout, "%s%s%s",
                       (source ? ".DECLARE: " : ""),
                       (vartyp & 0x80 ? "DIM " : ""), varnam);
               if(vartyp & 0x20) fputc('$', fileout);
               vardim1 = *(addr + 2) & 0xff;
               vardim2 = *(addr + 3) & 0xff;
               if(vartyp & 0x80) {
                 fprintf(fileout, "(%u,%u)", vardim1, vardim2);
               }
               varlen = (*(addr + 4) & 0xff) & 0x7f;
               if(vartyp & 0x20) {
                 fprintf(fileout, "*%u", varlen);
               }
               fputc('\n', fileout);
               if((varsiz = (((*addr) & 0xff) << 8 | (*(addr + 1)) & 0xff) - 3) !=
                  ((vardim1 + 1) * (vardim2 + 1) * varlen))
                 fprintf(fileout, "Variable size mismatch: %u != %u\n", varsiz,
                         ((vardim1 + 1) * (vardim2 + 1) * varlen));
               addr += 5;
               base += 7;
               for(d1 = 0; d1 <= vardim1; d1++) {
                 for(d2 = 0; d2 <= vardim2; d2++) {
                   if(source)
                     fputs("\t\t", fileout);
                   else
                     fprintf(fileout, "%04hX\t  ", base);
                   fprintf(fileout,"%s", varnam);
                   if(vartyp & 0x20) fputc('$', fileout);
                   if((vartyp & 0x80) == 0x80) {
                     fprintf(fileout,"(%u,%u)", d1, d2);
                   }
                   fputs("\t= ", fileout);
                   if((stopaddr - addr) >= varlen) {
                     if(vartyp & 0x20) {
                       fputc('"', fileout);
                       for(l = 0; l < varlen; l++) {
                         if((byte = *(addr + l) & 0xff)) {
                           if(byte > 0x1f && byte < 0x80) {
                             if((LHU8)byte == '\\') fputc('\\', fileout);
                             fputc(byte, fileout);
                           }
                           else {
                             if(byte > 0x9f) {
                               fprintf(fileout,"%c\\+80", (LHU8)byte & 0x7f);
                             }
                             else {
                               fprintf(fileout,"\\%02X", byte);
                             }
                           }
                         }
                       }
                       fputc('"', fileout);
                     }
                     else
                       outxreg(addr, fileout);
                     addr += varlen;
                     base += varlen;
                     fputc('\n', fileout);
                   }
                   else {
                     if(vartyp & 0x20) {
                       fputs("<INCOMPLETE STRING> [ ", fileout);
                       for(byte = 0;
                           addr < stopaddr && byte < varlen;
                           byte++, base++)
                         fprintf(fileout, "%02X ", (*(addr++)) & 0xff);
                     }
                     else {
                       fputs("<INCOMPLETE  XREG>  [ ", fileout);
                       for(byte = 0;
                           addr < stopaddr && byte < varlen;
                           byte++, base++)
                         fprintf(fileout, "%02X ", (*(addr++)) & 0xff);
                     }
                     for(; byte < varlen; byte++)
                       fputs("?? ", fileout);
                     fputs("]\n", fileout);
                   }
                 }
               }
               fputc('\n', fileout);
             }
             else {
               fputs("<INCOMPLETE   VAR>  [ ", fileout);
               for(byte = 0; addr < stopaddr && byte < 8; byte++, base++)
                 fprintf(fileout, "%02X ", (*(addr++)) & 0xff);
               fprintf(fileout, "%s]\n",
                       &("?? ?? ?? ?? ?? ?? ?? "[byte * 3]));
             }
             break;
        case FRAG_XREG:
             if(stopaddr - addr >= 8) {
               base += 8;
               outxreg(addr, fileout);
               fputs("  [", fileout);
               for(byte = 0; byte < 8; byte++, addr++)
                 fprintf(fileout, " %02hX", *addr & 0xff);
               fputs(" ]\n", fileout);
             }
             else {
               fputs("<INCOMPLETE  XREG>  [ ", fileout);
               for(byte = 0; addr < stopaddr && byte < 8; byte++, base++)
                 fprintf(fileout, "%02X ", (*(addr++)) & 0xff);
               fprintf(fileout, "%s]\n",
                       &("?? ?? ?? ?? ?? ?? ?? ?? "[byte * 3]));
             }
             break;
        case FRAG_HOLE:
             fprintf(fileout, ".SKIP\t%04X\n", (stopaddr - addr));
             base += (stopaddr - addr);
             addr = stopaddr;
             break;
        default:
             break;
      }
    }
    fputc('\n', fileout);
  }

  if((name = lhSymGetName(sym, base)) != (PLHU8) NULL)
    fprintf(fileout, "%s%s:\n",
            (source ? "" : "\n"), name);

  if(source)
    fprintf(fileout,"\t.END\n\t" LHSYM_MAGIC "\n");

  if( upass != 1 )
    while(sym != (PLHSYM) NULL) {
      if(source)
        fprintf(fileout,"\t%04hX\t%s\n", sym->value, sym->name);
      sym = sym->next;
    }

  if(source)
    fprintf(fileout,"\n");
  if(checksum & 2)
    fprintf(fileout,"%sCHECKSUM: %04hX:%04hX = %04hX [%02X%02X]\n",
            (source ? "\t." : ""), chksumfrom, chksumbase, chksum,
            (*(chksumaddr) & 0xff),
            (*(chksumaddr + 1) & 0xff));
  if(checksum)
    fprintf(fileout,"%sFULLCHECKSUM: %04hX\n",
            (source ? "\t." : ""), fullchksum);

  if( upass == 1)
    {
    rewind( fileout );
    upass = 2;
    goto dosource;
    }

  free(memory);

  fclose(filein);
  fclose(fileout);
  exit(0);
}

