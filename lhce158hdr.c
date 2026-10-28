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

/* CSAVE header for the CE158 utilities					*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lhtype.h"
#include "lhce158hdr.h"

int lhCE158hdrCheckMagic( PLHU8 magic )
{
   return( (0 == memcmp( magic, LHCE158HDR_CSAVE_MAGIC, LHCE158HDR_MAGIC_LENGTH ) ||
            0 == memcmp( magic, LHCE158HDR_CSAVEr_MAGIC, LHCE158HDR_MAGIC_LENGTH ) ||
            0 == memcmp( magic, LHCE158HDR_CSAVEM_MAGIC, LHCE158HDR_MAGIC_LENGTH ) ||
            0 == memcmp( magic, LHCE158HDR_PRINT_MAGIC, LHCE158HDR_MAGIC_LENGTH )) ? 1 : 0 );
}

int lhCE158HdrExplode( PLHU8 magic, PLHU8 hdr, PLHU8 name, LHU16 *start, LHU16 *length, LHU16 *entry )
{
  int ret = 0;

  if( lhCE158hdrCheckMagic( &hdr[LHCE158HDR_MAGIC_OFFSET] ) )
    {
    if( LHCE158HDR_START == hdr[0] )
      {
      memset( name, 0, LHCE158HDR_NAME_LENGTH );
      strncpy( magic, &hdr[LHCE158HDR_MAGIC_OFFSET], LHCE158HDR_MAGIC_LENGTH );
      strncpy( name, &hdr[LHCE158HDR_NAME_OFFSET], LHCE158HDR_NAME_LENGTH );
      *start = ((hdr[LHCE158HDR_START_OFFSET] << 8) | hdr[LHCE158HDR_START_OFFSET+1]);
      *length = ((hdr[LHCE158HDR_LENGTH_OFFSET] << 8) | hdr[LHCE158HDR_LENGTH_OFFSET+1]) + 1;
      *entry = ((hdr[LHCE158HDR_ENTRY_OFFSET] << 8) | hdr[LHCE158HDR_ENTRY_OFFSET+1]);
      ret = LHCE158HDR_LENGTH;
      }
    }
  return ret;
}

int lhCE158HdrBuild( PLHU8 magic, PLHU8 hdr, PLHU8 name, LHU16 start, LHU16 length, LHU16 entry )
{
  int ret = 0;

  if( lhCE158hdrCheckMagic( magic ) )
    {
    length --;
    hdr[0] = LHCE158HDR_START;
    memset( &hdr[LHCE158HDR_NAME_OFFSET], 0, LHCE158HDR_NAME_LENGTH );
    memcpy( &hdr[LHCE158HDR_MAGIC_OFFSET], magic, LHCE158HDR_MAGIC_LENGTH );
    strncpy( &hdr[LHCE158HDR_NAME_OFFSET], name, LHCE158HDR_NAME_LENGTH );
    hdr[LHCE158HDR_START_OFFSET] = (start >> 8) & 0xFF;
    hdr[LHCE158HDR_START_OFFSET+1] = start & 0xFF;
    hdr[LHCE158HDR_LENGTH_OFFSET] = (length >> 8) & 0xFF;
    hdr[LHCE158HDR_LENGTH_OFFSET+1] = length & 0xFF;
    hdr[LHCE158HDR_ENTRY_OFFSET] = (entry >> 8) & 0xFF;
    hdr[LHCE158HDR_ENTRY_OFFSET+1] = entry & 0xFF;
    ret = LHCE158HDR_LENGTH;
    }
  return ret;
}

