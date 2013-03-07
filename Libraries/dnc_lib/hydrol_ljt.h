#ifndef __HYDROL_LJT_H__
#define __HYDROL_LJT_H__


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
  NV_INT32             feature_key;
  NV_INT16             tile_reference_identifier;
  NV_INT32             edge_primitive_key;
} HYDROL_LJT;


  NV_INT32 open_hydrol_ljt (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_hydrol_ljt (NV_INT32 hnd);
  NV_BOOL read_hydrol_ljt (NV_INT32 hnd, NV_INT32 recnum, HYDROL_LJT *data);
  void dump_hydrol_ljt (HYDROL_LJT data);


#ifdef  __cplusplus
}
#endif

#endif
