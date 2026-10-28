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

#ifndef __linux__
#ifndef __APPLE__
// X68000（クロスコンパイル環境）ではシリアル通信の実装をスキップする
#include <stdio.h>
int main(void) {
    printf("lhcom is not supported on this platform.\n");
    return 0;
}
#define REAL_MAIN_DISABLED
#endif
#endif

#ifndef REAL_MAIN_DISABLED
// --- ここから下に元のソースコードが続く ---

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
// #include <poll.h>
#include <termios.h>
#else // !_WIN32
#include "windows.h"
#endif

#include "lhtype.h"
#include "lh5801.h"
#include "lhsym.h"
#include "lhbasic.h"
#define FRAG_NONE                 0
#include "lhfrag.h"
#include "lhce158hdr.h"

extern const char const lhTools_vers[], lhTools_date[], lhTools_pltf[];

PLHSYM comsym = (PLHSYM)NULL;

#define LHCOM_TTY_NAME_LENGTH     32

struct {
  char tty[LHCOM_TTY_NAME_LENGTH];
  unsigned int speed;
  LHU8 bits, parity, stop;
} comserial;

#define LHCOM_SEND         0x01
#define LHCOM_RECEIVE      0x02
#define LHCOM_DEBUG        0x10
#define LHCOM_VERBOSE      0x20
unsigned int comflags = 0;

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
          "Usage: %s [-h] [-v] [-d|-ddebug] [-dverbose]] [-m interface]\n"
          "\t\t[-Y {[line][=speed,size,parity,stopb]}]\n"
          "\t\t[-Z[header=type]] [-Z[start=addr]] [-Z[name=headername]] [-Z[entry=addr]]\n"
          "\t\t[-S symfile] {-r|-s} binfile\n",
          prog);
  fprintf(stderr,"where:\n");
  fprintf(stderr,"\t-d|-debug\tShow debug information\n");
  fprintf(stderr,"\t-dverbose\tEnable verbose mode\n");
  fprintf(stderr,"\t-h\t\tThis help\n");
  fprintf(stderr,"\t-m interface\tSelect the interface type\n");
  fprintf(stderr,"\t   -m ce158\tUse the CE158 serial interface setting and discipline\n");
  fprintf(stderr,"\t   Only one -m ce... option may be given\n");
  fprintf(stderr,"\t-F fragfile\tRead fragments from <fragfile>\n");
  fprintf(stderr,"\t-S symfile\tRead symbols from <symfile>\n");
  fprintf(stderr,"\t-Y line\t\tUse <line> as serial device\n");
  fprintf(stderr,"\t-Y =speed,size,parity,stopb\tSet the serial settings\n");
  fprintf(stderr,"\t\twith <speed>  : 75 100 110 200 300 600 1200 or 2400\n");
  fprintf(stderr,"\t\twith <size>   : 5 6 7 or 8\n");
  fprintf(stderr,"\t\twith <parity> : N E or O\n");
  fprintf(stderr,"\t\twith <stopb>  : 1 or 2\n");
  fprintf(stderr,"\t-Z\t\tAdd a CE158 CSAVE header\n");
  fprintf(stderr,"\t-Zname=name\tSet <name> as CSAVE header file name\n");
  fprintf(stderr,"\t-Zstart=addr\tSet <addr> as CSAVE header start base address\n");
  fprintf(stderr,"\t-Zentry=addr\tSet <addr> as CSAVE header startup routine\n");
  fprintf(stderr,"\t-Zheader=type\tSet <type> for CSAVE header magic\n");
  fprintf(stderr,"\t\twith <type>   : CSAVE CSAVEM CSAVEr or PRINT\n");
  fprintf(stderr,"\t-r\t\tReceive data from a PC-1500; exclusive with -s\n");
  fprintf(stderr,"\t-s\t\tSend data to a PC-1500; exclusive with -r\n");
  fprintf(stderr,"\n");
  exit(1);
}

#ifndef _WIN32
int lhComSetSerial( char *tty, unsigned speed, LHU8 bits, LHU8 parity, LHU8 stop )
{
  struct stat st;
  struct termios tios;
  tcflag_t tf = 0;
  int fdtty;

  if( NULL != tty ) {
    if( (0 > lstat( tty, &st )) || (0 > access( tty, R_OK|W_OK )) ) {
      FPRINTF_STDERR( "Unable to access to serial '%s'\n", tty );
      return -1;
    }
  }
  switch(speed) {
    case   50 : tf = B50; break;
    case   75 : tf = B75; break;
    case  110 : tf = B110; break;
    case  200 : tf = B200; break;
    case  300 : tf = B300; break;
    case  600 : tf = B600; break;
    case 1200 : tf = B1200; break;
    case 2400 : tf = B2400; break;
    default:
      FPRINTF_STDERR( "Speed %u is not supported\n", speed );
      return -1;
  }
  switch( bits ) {
    case '5': tf |= CS5; break;
    case '6': tf |= CS6; break;
    case '7': tf |= CS7; break;
    case '8': tf |= CS8; break;
    default:
      FPRINTF_STDERR( "CSIZE %c not supported\n", bits );
      return -1;
  }
  switch( toupper(parity) ) {
    case 'N': break;
    case 'O': tf |= PARODD;
    case 'E': tf |= PARENB; break;
    default: 
      FPRINTF_STDERR( "CSIZE %c not supported\n", bits );
      return -1;
  }
  if( '1' == stop )
    ;
  else if( '2' == stop )
    tf |= CSTOPB;
  else {
    FPRINTF_STDERR( "Only 1 or 2 stop bits supported\n" );
    return -1;
  }
  if( 0 >= (fdtty = open( tty ? tty : comserial.tty, O_RDWR )) ) {
    FPRINTF_STDERR( "Unable to open serial '%s'\n", tty ? tty : comserial.tty );
    return -1;
  }
  tcgetattr( fdtty, &tios );
  cfmakeraw( &tios );
  tios.c_cflag &= ~(CSIZE|CSTOPB|CBAUD|(PARENB|PARODD));
  tios.c_cflag |= tf;
  tcsetattr( fdtty, TCSANOW, &tios );
  close( fdtty );
  comserial.speed = speed;
  comserial.bits = bits;
  comserial.parity = parity;
  comserial.stop = stop;
  if( tty )
    snprintf( comserial.tty, sizeof(comserial.tty), "%s", tty );
  if( comflags & LHCOM_VERBOSE )
    fprintf( stdout, "Serial: '%s' Parameters: %u,%c,%c,%c\n",
             comserial.tty,
             comserial.speed, comserial.bits, comserial.parity, comserial.stop );
  return 0;
}
#else // !_WIN32
int lhComSetSerial( char *tty, unsigned speed, LHU8 bits, LHU8 parity, LHU8 stop )
{
  HANDLE fdtty;
  DCB ttyparam = {0};
                
  fdtty = CreateFile( tty ? tty : comserial.tty,
                      GENERIC_READ|GENERIC_WRITE, 0, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if( INVALID_HANDLE_VALUE == fdtty ) {
    FPRINTF_STDERR( "Unable to open serial port %s\n", tty ? tty : comserial.tty );
    return -1;
  }

  ttyparam.DCBlength = sizeof(ttyparam);
  if( 0 == GetCommState( fdtty, &ttyparam ) ) {
    CloseHandle( fdtty );
    FPRINTF_STDERR( "Unable to get serial port settings\n" );
    return -1;
  }
        
  switch(speed) {
    case  110 : ttyparam.BaudRate = CBR_110; break;
    case  300 : ttyparam.BaudRate = CBR_300; break;
    case  600 : ttyparam.BaudRate = CBR_600; break;
    case 1200 : ttyparam.BaudRate = CBR_1200; break;
    case 2400 : ttyparam.BaudRate = CBR_2400; break;
    default:
      CloseHandle( fdtty );
      FPRINTF_STDERR( "Speed %u is not supported\n", speed );
      return -1;
  }
  switch( bits ) {
    case '5': ttyparam.ByteSize = 5; break;
    case '6': ttyparam.ByteSize = 6; break;
    case '7': ttyparam.ByteSize = 7; break;
    case '8': ttyparam.ByteSize = 8; break;
    default:
      CloseHandle( fdtty );
      FPRINTF_STDERR( "CSIZE %c not supported\n", bits );
      return -1;
  }
  switch( toupper(parity) ) {
    case 'N': ttyparam.Parity = NOPARITY; break;
    case 'O': ttyparam.Parity = ODDPARITY; break;
    case 'E': ttyparam.Parity = EVENPARITY; break;
    default: 
      CloseHandle( fdtty );
      FPRINTF_STDERR( "CSIZE %c not supported\n", bits );
      return -1;
  }
  if( '1' == stop )
    ttyparam.StopBits = ONESTOPBIT;
  else if( '2' == stop )
    ttyparam.StopBits = TWOSTOPBITS;
  else {
    CloseHandle( fdtty );
    FPRINTF_STDERR( "Only 1 or 2 stop bits supported\n" );
    return -1;
  }
  if( 0 == SetCommState( fdtty, &ttyparam ) ) {
    CloseHandle( fdtty );
    FPRINTF_STDERR( "Unable to set serial port settings\n");
    return -1;
  }

  CloseHandle( fdtty );
  comserial.speed = speed;
  comserial.bits = bits;
  comserial.parity = parity;
  comserial.stop = stop;
  if( tty )
    snprintf( comserial.tty, sizeof(comserial.tty), "%s", tty );
  if( comflags & LHCOM_VERBOSE )
    fprintf( stdout, "Serial: '%s' Parameters: %u,%c,%c,%c\n",
             comserial.tty,
             comserial.speed, comserial.bits, comserial.parity, comserial.stop );
  return 0;
}
#endif

void lhComSend( PLHU8 hdr, PLHU8 mem, LHU16 length )
{
  LHU8 rmagic[LHCE158HDR_MAGIC_LENGTH+1];
  LHU8 rname[LHCE158HDR_NAME_LENGTH+1];
  LHU16 rstart, rentry, rlength;
#ifndef _WIN32
  int fdtty;
#else // !_WIN32
  HANDLE fdtty;
  DWORD wrbyte;
#endif

  memset( rmagic, 0, sizeof(rmagic) );
  memset( rname, 0, sizeof(rname) );
  lhCE158HdrExplode( rmagic, hdr, rname, &rstart, &rlength, &rentry );
  if( comflags & LHCOM_VERBOSE )
    fprintf( stdout,
             "Sending CE158 header:\n"
             "\tMagic:%s Name:\"%s\" Start:%04hX Length:%5u Entry:%04hX\n",
             rmagic, rname, rstart, rlength, rentry );
#ifndef _WIN32
  if( 0 > (fdtty = open( comserial.tty, O_WRONLY )) ) {
    fprintf(stderr, "Error while opening serial '%s'\n", comserial.tty );
    return;
  }
  write( fdtty, hdr, LHCE158HDR_LENGTH );
  write( fdtty, mem, length );
  close( fdtty );
#else // !_WIN32
  fdtty = CreateFile( comserial.tty,
                      GENERIC_READ|GENERIC_WRITE, 0, NULL,
                      OPEN_EXISTING, 0, NULL );
  if( INVALID_HANDLE_VALUE == fdtty ) {
    FPRINTF_STDERR( "Unable to open serial port %s\n", comserial.tty );
    return;
  }
  if( 0 == WriteFile( fdtty, hdr, LHCE158HDR_LENGTH, &wrbyte, NULL ) ) {
    FPRINTF_STDERR( "Error while sending header\n");
  }
  if( 0 == WriteFile( fdtty, mem, length, &wrbyte, NULL ) ) {
    FPRINTF_STDERR( "Error while sending binary data\n");
  }
  CloseHandle( fdtty );
#endif

  if( comflags & LHCOM_VERBOSE ) {
    int i;

    fprintf( stdout, "HEADER:" );
    for( i = 0; i < LHCE158HDR_LENGTH; i++ ) {
      fprintf( stdout, " %02hhX", hdr[i] );
      if( 16 == i ) {
        fprintf( stdout, "\n      :" );
      }
    }
    fprintf( stdout, "\nBINARY:" );
    for( i = 0; i < length; i++ ) {
      if( (i & 0xF) == 0x0 )
        fprintf( stdout, "\n +%04hX:", i );
      fprintf( stdout, " %02hhX", mem[i] );
    }
    fprintf( stdout, "\n" );
  }
}

void lhComReceive( PLHU8 hdr, PLHU8 *mem, PLHU16 length )
{
  LHU16 end;
  LHU8 rmagic[LHCE158HDR_MAGIC_LENGTH+1];
  LHU8 rname[LHCE158HDR_NAME_LENGTH+1];
  LHU16 rstart, rentry;
  PLHU8 pmem;
#ifndef _WIN32
  int fdtty;
#else // !_WIN32
  HANDLE fdtty;
  DWORD rdbyte;
#endif

  memset( rmagic, 0, sizeof(rmagic) );
  memset( rname, 0, sizeof(rname) );
  *length = end = 0;
  *mem = NULL;

#ifndef _WIN32
  if( 0 > (fdtty = open( comserial.tty, O_RDONLY )) ) {
    FPRINTF_STDERR( "Error while opening serial '%s'\n", comserial.tty );
    return;
  }
#else // !_WIN32
  fdtty = CreateFile( comserial.tty,
                      GENERIC_READ|GENERIC_WRITE, 0, NULL,
                      OPEN_EXISTING, 0, NULL );
  if( INVALID_HANDLE_VALUE == fdtty ) {
    FPRINTF_STDERR( "Unable to open serial port %s\n", comserial.tty );
    return;
  }
#endif

  if( comflags & LHCOM_DEBUG )
    fprintf( stdout, "Waiting for header..." );
  do {
#ifndef _WIN32
    read( fdtty, &hdr[end], 1 );
#else // !_WIN32
    if( 0 == ReadFile( fdtty, &hdr[end], 1, &rdbyte, NULL ) ) {
      FPRINTF_STDERR( "Error while reading header\n");
    }
#endif
    end++;
  } while( end < LHCE158HDR_LENGTH );
  if( comflags & LHCOM_DEBUG )
    fprintf( stdout, "\n");

  if( 0 == lhCE158HdrExplode( rmagic, hdr, rname, &rstart, &end, &rentry ) ) {
    fprintf( stderr, "Unexpected magic received %c%c%c%c\n",
                    rmagic[0],rmagic[1],rmagic[2],rmagic[3] );
#ifndef _WIN32
    close( fdtty );
#else // !_WIN32
    CloseHandle( fdtty );
#endif
    if( comflags & LHCOM_DEBUG ) {
      int i;

      fprintf( stdout, "HEADER:" );
      for( i = 0; i < LHCE158HDR_LENGTH; i++ ) {
        fprintf( stdout, " %02hhX", hdr[i] );
        if( 16 == i ) {
          fprintf( stdout, "\n      :" );
        }
      }
    }
    return;
  }
  if( comflags & LHCOM_VERBOSE )
    fprintf( stdout,
             "Receiving CE158 header:\n"
             "\tMagic:%s Name:\"%s\" Start:%04hX Length:%5u Entry:%04hX\n",
             rmagic, rname, rstart, end, rentry );
  pmem = (PLHU8)malloc(*length = end);
  while( end-- ) {
#ifndef _WIN32
    read( fdtty, pmem++, 1 );
#else // !_WIN32
    if( 0 == ReadFile( fdtty, pmem++, 1, &rdbyte, NULL ) ) {
      FPRINTF_STDERR( "Error while reading data\n");
    }
#endif
  }
#ifndef _WIN32
  close( fdtty );
#else // !_WIN32
  CloseHandle( fdtty );
#endif

  *mem = pmem -= (end = *length);
  if( comflags & LHCOM_VERBOSE ) {
    int i;

    fprintf( stdout, "HEADER:" );
    for( i = 0; i < LHCE158HDR_LENGTH; i++ ) {
      fprintf( stdout, " %02hhX", hdr[i] );
      if( 16 == i ) {
        fprintf( stdout, "\n      :" );
      }
    }
    fprintf( stdout, "\nBINARY:" );
    for( i = 0; i < end; i++ ) {
      if( (i & 0xF) == 0x0 )
        fprintf( stdout, "\n +%04hX:", i );
      fprintf( stdout, " %02hhX", pmem[i] );
    }
    fprintf( stdout, "\n" );
  }
}

int main( int argc, char **argv )
{
  int             iargc, argerr;
  char           *argvc;
  char            cvers[4];
  char           *symfile = NULL, *binfile;
  FILE           *binfd;
  LHU8            string[1024];
  PLHU8           name;
  int             start = 0x40C5;
  struct stat     st;
  LHU8            zhdr[LHCE158HDR_LENGTH];
  LHU8           *zphdr = NULL, *zname = NULL, *zmagic = NULL;
  LHU16           zstart = 0xFFFF, zlength = 0xFFFF, zentry = 0xFFFF, zstate = FRAG_NONE;
  LHU16           entry = 0xFFFF, xit = 0xFFFF, optentry = 0xFFFF, optxit = 0xFFFF;
  unsigned        inlength = 0;

  memset( cvers, 0, sizeof(cvers) );
  sscanf( lhTools_vers, "%hhu.%hhu.%hhup%hhu", &cvers[0], &cvers[1], &cvers[2], &cvers[3] );

#ifndef _WIN32
  lhComSetSerial( "/dev/ttyS0", 300, '8', 'N', '1' );
#else // !_WIN32
  lhComSetSerial( "\\\\.\\COM1", 300, '8', 'N', '1' );
#endif

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
        case 'd': if( 0 == *(++argvc) || 0 == strcmp( "debug", argvc ) )
                    comflags |= LHCOM_DEBUG;
                  else if( 0 == strcmp( "verbose", argvc ) )
                    comflags |= LHCOM_VERBOSE;
                  else
                    argerr++;
                  break;
        case 'r': comflags |= LHCOM_RECEIVE;
                  break;
        case 's': comflags |= LHCOM_SEND;
                  break;
        case 'S': if(!(argerr = (++iargc == argc || symfile)))
                    symfile = argv[iargc];
                  break;
        case 'Y': if(!(argerr = (++iargc == argc))) {
                    unsigned int speed = comserial.speed;
                    LHU8 bits = comserial.bits;
                    LHU8 parity = comserial.parity;
                    LHU8 stop = comserial.stop;
                    char tty[sizeof(comserial.tty)];
                    char *p1, *p2;
                    int l;
                    memset( tty, 0, sizeof(tty) );
                    p2 = strchr( (p1 = argv[iargc]), '=' );
                    if( p1 != p2 ) {
                      l = p2 ? (p2 - p1) : strlen(p1);
                      strncpy( tty, p1, l );
                    }
                    if( p2 &&
                        (4 != sscanf( p2+1, "%u,%c,%c,%c", &speed, &bits, &parity, &stop )) ) {
                      FPRINTF_STDERR( "Bad syntax for serial line settings\n" );
                      exit( 2 );
                    }
                    lhComSetSerial( *tty ? tty : NULL, speed, bits, parity, stop );
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
                  else if( 0 == strncmp( argvc, "Zstart=", 7 ) )
                    {
                    LHU16 start;
                    if( (0xFFFF != zentry) || (1 != sscanf( argvc + 7, "%hx", &start )) )
                      {
                      fprintf( stderr, "Header start address badly specified or already set\n" );
                      exit( 1 );
                      }
                    zstart = start;
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
        default:  argerr = 1; break;
      }
    }
    else
      break;
  }

  if(argerr || (iargc >= argc))
    usage(argv[0]);

  if( (comflags & (LHCOM_SEND|LHCOM_RECEIVE)) == (LHCOM_SEND|LHCOM_RECEIVE) ) {
    FPRINTF_STDERR( "Receive -r and sending -s are exclusive\n" );
    exit( 2 );
  }
  if( (comflags & (LHCOM_SEND|LHCOM_RECEIVE)) == 0 ) {
    FPRINTF_STDERR( "Specify one of receive -r or sending -s actions\n" );
    exit( 2 );
  }

  binfile = argv[iargc];

  if( comflags & LHCOM_SEND ) {
    int slength;
    PLHU8 szhdr = NULL, smem;
    LHU8 rzmagic[LHCE158HDR_MAGIC_LENGTH+1], rzname[LHCE158HDR_NAME_LENGTH+1];
    LHU16 rzstart, rzlength, rzentry;

    if( symfile ) {
      int ret;

      if( 0 > (ret = lhSymImportSymbol( symfile, &comsym, SYM_NONE )) ) {
        if( -1 == ret )
          fprintf(stderr, "Cannot open symbols file %s\n", symfile);
        else
          fprintf(stderr, "Hmmm... Not a symbols file %s\n", symfile);
        exit( 2 );
      }
    }

    if( (0 > stat( binfile, &st )) ||
        (NULL == (binfd = fopen( binfile, "rb" ))) ) {
      fprintf( stderr, "Failed to open binary file '%'\n", binfile );
      exit( 3 );
    }
    inlength = st.st_size;

    if( zphdr  ) {
      char name[ LHCE158HDR_NAME_LENGTH+1 ];

      if( comflags & LHCOM_VERBOSE )
        fprintf( stdout, "Fixing CE158 CSAVE header from command line..." );
      if( !zname )
        {
        char *p, *ps;
        int  ll = strlen( binfile );

        memset( name, 0, sizeof(name) );
        if( (ps = strrchr( binfile, '/' )) )
          ps++;
        else
          ps = binfile;
        if( (p = strrchr( ps, '.' )) )
          ll = p - ps;
        strncpy( zname = name, ps,
                 ll > LHCE158HDR_NAME_LENGTH ? LHCE158HDR_NAME_LENGTH : ll);
        }
      if( 0xFFFF == zentry )
        zentry = lhSymGetValue(comsym, "STARTUP");
      zlength = inlength;
      if( zmagic )
        ;
      else if( FRAG_NONE == zstate || FRAG_BASIC == zstate )
        zmagic = LHCE158HDR_CSAVE_MAGIC;
      else if( FRAG_RESERVE == zstate )
        zmagic = LHCE158HDR_CSAVEr_MAGIC;
      else
        zmagic = LHCE158HDR_CSAVEM_MAGIC;
      if( comflags & LHCOM_VERBOSE )
        fprintf( stdout, " '%s'\n", zmagic );
      if( LHCE158HDR_LENGTH != lhCE158HdrBuild( zmagic, zphdr, zname,
                                                zstart, zlength, zentry ) ) {
        fprintf( stderr, "Failed to build the CSAVE header\n" );
        zphdr = NULL;
      }
    }

    memset( rzmagic, 0, sizeof(rzmagic) );
    memset( rzname, 0, sizeof(rzname) );

    slength = st.st_size;
    if( NULL == (szhdr = smem = (PLHU8)malloc( slength )) ) {
      fprintf( stderr, "Unable to allocate memory\n" );
      exit( 127 );
    }

    if( 1 != fread( smem, slength, 1, binfd ) ) {
      fprintf( stderr, "Error while reading binary file\n" );
      exit( 4 );
    }

    if( LHCE158HDR_LENGTH == lhCE158HdrExplode( rzmagic, smem, rzname,
                                                &rzstart, &rzlength, &rzentry ) ) {
      smem += LHCE158HDR_LENGTH;
      slength -= LHCE158HDR_LENGTH;
    }
    else if( zphdr ) {
      szhdr = zphdr;
    }
    else {
      fprintf( stderr, "No CE158 header specified. Abort !\n" );
      exit( 99 );
    }
    lhComSend( szhdr, smem, slength );
  }
  else {
    LHU16 rlength;
    PLHU8 rmem;
    lhComReceive( zhdr, &rmem, &rlength );

    if( !rmem ) {
      fprintf( stderr, "Error while receiving. Abort !\n" );
      exit( 99 );
    }
    if( (NULL == (binfd = fopen( binfile, "wb" ))) ) {
      fprintf( stderr, "Failed to open binary file '%'\n", binfile );
      exit( 3 );
    }
    if( zphdr )
      fwrite( zhdr, LHCE158HDR_LENGTH, 1, binfd );
    fwrite( rmem, rlength, 1, binfd );
    fclose(binfd);
  }

  exit( 0 );
}

#endif // REAL_MAIN_DISABLED
