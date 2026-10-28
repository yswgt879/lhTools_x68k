// Copyright 1992-2015 Christophe Gottheimer <cgh75015@gmail.com>
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

/* lhcom.c - Communication manager over RS232C				*/

#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <ctype.h>
#ifndef _WIN32
#include <sys/wait.h>
#ifdef __linux__
#include <poll.h>
#include <termios.h>
#endif
#endif

#include "lhtype.h"
#include "lh5801.h"
#include "lhsym.h"
#include "lhbasic.h"
#define FRAG_NONE                 0
#include "lhfrag.h"
#include "lhce158hdr.h"

extern const char const lhTools_vers[], lhTools_date[], lhTools_pltf[];

PLHSYM pokesym;

#define LHPOKE_HEX       0x1
#define LHPOKE_NICERHEX  0x2
#define LHPOKE_AAPPEND   0x4
#define LHPOKE_ZHEADER   0x8
unsigned int pokeflags = 0;

#define FPRINTF_STDERR( fmt, args... ) do { \
                                         fprintf( stderr, fmt, ## args ); \
                                       } while( 0 )

#define FPUTS( str, fd )               do { \
                                         fputs( str, fd ); \
                                       } while( 0 )

#define FPUTC( c, fd )                 do { \
                                         fputc( c, fd ); \
                                       } while( 0 )

#define FPRINTF( fd, fmt, args... )    do { \
                                         fprintf( fd, fmt, ## args ); \
                                       } while( 0 )

#define FFLUSH( fd )                   do { \
                                         fflush( fd ); \
                                       } while( 0 )

void usage(PLHU8 prog)
{
  fprintf(stderr,
          "Usage: %s [-h] [-v] [-x] [-xx] [-Z] [-O origin] [-A[A] appendline]\n"
          "\t\t[-B byteperline] [-L linenum] [-I lineincr] [-V varable]\n"
          "\t\t[-S symfile] [-o basfile] binfile\n",
          prog);
  fprintf(stderr,"where:\n");
  fprintf(stderr,"\t-h\t\tThis help\n");
  fprintf(stderr,"\t-x\t\tValues in POKE are in hexadecimal\n");
  fprintf(stderr,"\t-xx\t\tValues in POKE are in hexadecimal aligned\n");
  fprintf(stderr,"\t-A appendline\tAppend <appendline> on the first line\n");
  fprintf(stderr,"\t-AA appendline\tAppend a new line after the first with <appendline>\n");
  fprintf(stderr,"\t-B byteperline\tWrite <byteperline> bytes on each line. Default 10\n");
  fprintf(stderr,"\t-I lineincr\tUse <lineincr> as line number increment. Default 10\n");
  fprintf(stderr,"\t-L linenum\tUse <linenum> as first line number. Default 10\n");
  fprintf(stderr,"\t-O address\tUse <address> as origin base address. Default &40C5\n");
  fprintf(stderr,"\t-S symfile\tRead symbols from <symfile>\n");
  fprintf(stderr,"\t-V variable\tUse <variable> as base address. Default A\n");
  fprintf(stderr,"\t-Z\t\tExpect and use a CE158 CSAVE header\n");
  fprintf(stderr,"\t-o outfile\tOutput BASIC code into basfile (.bas)\n");
  fprintf(stderr,"\n");
  exit(1);
}

int main( int argc, char **argv )
{
  int             iargc, argerr;
  char           *argvc;
  char            cvers[4];
  char           *symfile = NULL, *basfile = NULL, *binfile;
  char            baspath[PATH_MAX];
  char           *append = NULL;
  FILE           *binfd;
  LHU8           *inmem; 
  LHU8            basline[200];
  LHU16           line = 10, incr = 10, bpl = 10;
  LHU16           start = 0xFFFF;
  LHU8            var[2] = { 'A', 0, };
  struct stat     st;
  int             inlength, ibyte;
  LHU16           zstart, zlength, zentry;
  LHU8            zmagic[LHCE158HDR_MAGIC_LENGTH+1], zname[LHCE158HDR_NAME_LENGTH+1];

  memset( cvers, 0, sizeof(cvers) );
  sscanf( lhTools_vers, "%hhu.%hhu.%hhup%hhu", &cvers[0], &cvers[1], &cvers[2], &cvers[3] );

  for( argerr = 0, iargc = 1; !argerr && iargc < argc; iargc++ ) {
    if(*(argvc = argv[iargc]) == '-') {
      switch(*(++argvc)) {
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
        case 'x': pokeflags |= LHPOKE_HEX;
                  if('x' == *(argvc+1))
                    pokeflags |= LHPOKE_NICERHEX;
                  break;
        case 'S': if(!(argerr = (++iargc == argc || symfile)))
                    symfile = argv[iargc];
                  break;
        case 'A': if('A' == *(argvc+1))
                    pokeflags |= LHPOKE_AAPPEND;
                  if(!(argerr = (++iargc == argc || append))) {
                    append = argv[iargc];
                  }
                  break;
        case 'B': if(!(argerr = (++iargc == argc)) ) {
                    if(1 != sscanf(argv[iargc], "%hu", &bpl ) ||
                       (((bpl < 1) || (bpl > 16)))) {
                      FPRINTF_STDERR( "Bad byte-per-line value\n" );
                      exit( 2 );
                    }
                  }
                  break;
        case 'I': if(!(argerr = (++iargc == argc)) ) {
                    if(1 != sscanf(argv[iargc], "%hu", &incr )) {
                      FPRINTF_STDERR( "Bad line increment\n" );
                      exit( 2 );
                    }
                  }
                  break;
        case 'L': if(!(argerr = (++iargc == argc)) ) {
                    if(1 != sscanf(argv[iargc], "%hu", &line )) {
                      FPRINTF_STDERR( "Bad line number\n" );
                      exit( 2 );
                    }
                  }
                  break;
        case 'V': if(!(argerr = (++iargc == argc)) ) {
                    if(1 != sscanf(argv[iargc], "%c", &var[0] ) ||
                       !isupper( var[0] ) || strcmp( argv[iargc], var )) {
                      FPRINTF_STDERR( "Bad variable specified\n" );
                      exit( 2 );
                    }
                  }
                  break;
        case 'O': if(!(argerr = (++iargc == argc)) ) {
                    if(1 != sscanf(argv[iargc], "%hx", &start )) {
                      FPRINTF_STDERR( "Bad origin value\n" );
                      exit( 2 );
                    }
                  }
                  break;
        case 'Z': pokeflags |= LHPOKE_ZHEADER;
                  break;
        case 'o': if(!(argerr = (++iargc == argc)) ) {
                    basfile = argv[iargc];
                  }
                  break;
        default:  argerr = 1; break;
      }
    }
    else
      break;
  }

  if(argerr || (iargc >= argc))
    usage(argv[0]);

  binfile = argv[iargc];

  if( symfile ) {
    int ret;
    LHU16 sstart;

    if( 0 > (ret = lhSymImportSymbol( symfile, &pokesym, SYM_NONE )) ) {
      if( -1 == ret )
        fprintf(stderr, "Cannot open symbols file %s\n", symfile);
      else
        fprintf(stderr, "Hmmm... Not a symbols file %s\n", symfile);
      exit( 2 );
    }
    if( (0xFFFF == start) &&
        (0xFFFF != (sstart = lhSymGetValue( pokesym, "STARTUP" ))) ) {
      start = sstart;
    }
  }

  if( (0 > stat( binfile, &st )) ||
      (NULL == (binfd = fopen( binfile, "rb" ))) ) {
      fprintf( stderr, "Failed to open binary file '%'\n", binfile );
      exit( 3 );
  }
  inlength = st.st_size;

  if( NULL == (inmem = (PLHU8)malloc( inlength )) ) {
    fprintf( stderr, "Unable to allocate memory\n" );
    exit( 127 );
  }

  if( 1 != fread( inmem, inlength, 1, binfd ) ) {
    fprintf( stderr, "Error while reading binary file\n" );
    exit( 4 );
  }

  fclose(binfd);

  if( pokeflags & LHPOKE_ZHEADER ) {
    if( LHCE158HDR_LENGTH == lhCE158HdrExplode( zmagic, inmem, zname,
                                                &zstart, &zlength, &zentry ) ) {
      inmem += LHCE158HDR_LENGTH;
      inlength -= LHCE158HDR_LENGTH;
      if( 0xFFFF == start ) {
        start = zstart;
      }
    }
    else {
      fprintf( stderr, "No CE158 header specified. Abort !\n" );
      exit( 99 );
    }
  }

  if( NULL == basfile ) {
    char *p;

    snprintf( basfile = baspath, sizeof(baspath), "%s", binfile );
    if( (p = strrchr( baspath, '.' )) )
      *p = 0;
    else
      p = &baspath[strlen(baspath)];
    snprintf( p, sizeof(baspath) - (p - baspath), ".bas" );
  }

  if( NULL == (binfd = fopen( basfile, "w" )) ) {
      fprintf( stderr, "Failed to create basfile '%'\n", basfile );
      exit( 5 );
  }

  if( 0xFFFF == start ) {
    fprintf( binfd, "%u %c=197+256*PEEK &7863", line, var[0] );
  }
  else {
    fprintf( binfd, "%u %c=", line, var[0] );
    fprintf( binfd, (pokeflags & LHPOKE_NICERHEX ? "&%04X" :
                       (pokeflags & LHPOKE_HEX ? "&%X" : "%u")), start );
  }
  if( append ) {
    if( pokeflags & LHPOKE_AAPPEND ) {
      line += incr;
      fprintf( binfd, "\n%u %s", line, append );
    }
    else
      fprintf( binfd, ":%s", append );
  }
  fputc( '\n', binfd );
  fflush( binfd );
  line += incr;

  for( ibyte = 0; ibyte < inlength; ibyte++ ) {
    if( (ibyte % bpl) == 0 ) {
      fprintf( binfd, "%u POKE %c+", line, var[0] );
      fprintf( binfd, (pokeflags & LHPOKE_NICERHEX ? "&%04X" :
                         (pokeflags & LHPOKE_HEX ? "&%X" : "%u")), ibyte );
      line += incr;
    }
    fprintf( binfd, (pokeflags & LHPOKE_NICERHEX ? ",&%02X" :
                       (pokeflags & LHPOKE_HEX ? ",&%X" : ",%u")), inmem[ibyte] );
    if( ((ibyte % bpl) == (bpl -1)) || (ibyte == (inlength - 1)) ) {
      fputc( '\n', binfd );
      fflush( binfd );
    }
  }
  fclose( binfd );

  exit( 0 );
}
