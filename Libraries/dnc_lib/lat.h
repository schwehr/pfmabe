#ifndef __LAT_H__
#define __LAT_H__


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
  NV_CHAR              library_name[9];
  NV_FLOAT32           sw_corner_lon;
  NV_FLOAT32           sw_corner_lat;
  NV_FLOAT32           ne_corner_lon;
  NV_FLOAT32           ne_corner_lat;
} LAT;


  NV_INT32 open_lat (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_lat (NV_INT32 hnd);
  NV_BOOL read_lat (NV_INT32 hnd, NV_INT32 recnum, LAT *data);
  void dump_lat (LAT data);


#ifdef  __cplusplus
}
#endif

#endif
