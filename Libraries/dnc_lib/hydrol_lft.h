#ifndef __HYDROL_LFT_H__
#define __HYDROL_LFT_H__


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
  NV_INT16             hypsography_portrayal_category;
} HYDROL_LFT;


  NV_INT32 open_hydrol_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_hydrol_lft (NV_INT32 hnd);
  NV_BOOL read_hydrol_lft (NV_INT32 hnd, NV_INT32 recnum, HYDROL_LFT *data);
  void dump_hydrol_lft (HYDROL_LFT data);


#ifdef  __cplusplus
}
#endif

#endif
