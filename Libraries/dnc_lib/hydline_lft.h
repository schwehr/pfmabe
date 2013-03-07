#ifndef __HYDLINE_LFT_H__
#define __HYDLINE_LFT_H__


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
  NV_CHAR              facc_code[6];
  NV_INT16             accuracy_category;
  NV_FLOAT32           contour_value;
  NV_INT16             tile_reference_identifier;
  NV_INT32             edge_primitive_key;
} HYDLINE_LFT;


  NV_INT32 open_hydline_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_hydline_lft (NV_INT32 hnd);
  NV_BOOL read_hydline_lft (NV_INT32 hnd, NV_INT32 recnum, HYDLINE_LFT *data);
  void dump_hydline_lft (HYDLINE_LFT data);


#ifdef  __cplusplus
}
#endif

#endif
