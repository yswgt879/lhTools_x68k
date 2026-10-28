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

/* lhce158hdr.h - Definitions for the CSAVE headers for the CE158	*/

#ifndef __LHCE158HDR_H__

#define __LHCE158HDR_H__

#define LHCE158HDR_LENGTH              27
#define LHCE158HDR_START              0x1
#define LHCE158HDR_CSAVE_MAGIC     "@COM"
#define LHCE158HDR_CSAVEr_MAGIC    "ACOM"
#define LHCE158HDR_CSAVEM_MAGIC    "BCOM"
#define LHCE158HDR_PRINT_MAGIC     "HCOM"
#define LHCE158HDR_MAGIC_OFFSET         1
#define LHCE158HDR_MAGIC_LENGTH         4
#define LHCE158HDR_NAME_OFFSET          5
#define LHCE158HDR_NAME_LENGTH         16
#define LHCE158HDR_START_OFFSET        21
#define LHCE158HDR_START_LENGTH         2
#define LHCE158HDR_LENGTH_OFFSET       23
#define LHCE158HDR_LENGTH_LENGTH        2
#define LHCE158HDR_ENTRY_OFFSET        25
#define LHCE158HDR_ENTRY_LENGTH         2

int lhCE158HdrExplode( PLHU8 magic, PLHU8 hdr, PLHU8 name, LHU16 *start, LHU16 *length, LHU16 *entry );
int lhCE158HdrBuild( PLHU8 magic, PLHU8 hdr, PLHU8 name, LHU16 start, LHU16 length, LHU16 entry );

#endif /* __LHCE158HDR_H__ */

