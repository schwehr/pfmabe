#ifndef __SOUNDP_PFT_H__
#define __SOUNDP_PFT_H__


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
  NV_INT16             date;
  NV_INT16             existence_category;
  NV_FLOAT32           hydrographic_drying_height;
  NV_FLOAT32           hydrographic_depth;
  NV_INT16             sounding_category;
  NV_INT16             sounding_velocity;
  NV_INT16             value;
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} SOUNDP_PFT;


  NV_INT32 open_soundp_pft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_soundp_pft (NV_INT32 hnd);
  NV_BOOL read_soundp_pft (NV_INT32 hnd, NV_INT32 recnum, SOUNDP_PFT *data);
  void dump_soundp_pft (SOUNDP_PFT data);


#ifdef  __cplusplus
}
#endif

#endif
