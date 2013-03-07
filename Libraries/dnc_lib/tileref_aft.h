#ifndef __TILEREF_AFT_H__
#define __TILEREF_AFT_H__


/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


#ifdef  __cplusplus
extern "C" {
#endif


#include "table_header.h"
#include "nvtypes.h"


typedef struct
{
  NV_INT32             id;
  NV_CHAR              tile_name[9];
  NV_INT32             face_primitive_key;
} TILEREF_AFT;


  NV_INT32 open_tileref_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_tileref_aft (NV_INT32 hnd);
  NV_BOOL read_tileref_aft (NV_INT32 hnd, NV_INT32 recnum, TILEREF_AFT *data);
  void dump_tileref_aft (TILEREF_AFT data);


#ifdef  __cplusplus
}
#endif

#endif
