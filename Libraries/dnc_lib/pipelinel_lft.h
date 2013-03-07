#ifndef __PIPELINEL_LFT_H__
#define __PIPELINEL_LFT_H__


#ifdef  __cplusplus
extern "C" {
#endif


#include "table_header.h"
#include "nvtypes.h"


typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_FLOAT32           depth_below_surface_level;
  NV_INT16             existence_category;
  NV_FLOAT32           height_above_sea_bottom;
  NV_INT16             location_category;
  NV_FLOAT32           overhead_clearance_category;
  NV_INT16             over_water_obstruction;
  NV_INT16             pipeline_type;
  NV_INT16             product_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             edge_primitive_key;
} PIPELINEL_LFT;


  NV_INT32 open_pipelinel_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_pipelinel_lft (NV_INT32 hnd);
  NV_BOOL read_pipelinel_lft (NV_INT32 hnd, NV_INT32 recnum, PIPELINEL_LFT *data);
  void dump_pipelinel_lft (PIPELINEL_LFT data);


#ifdef  __cplusplus
}
#endif

#endif
