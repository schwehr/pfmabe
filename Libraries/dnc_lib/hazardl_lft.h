#ifndef __HAZARDL_LFT_H__
#define __HAZARDL_LFT_H__


#ifdef  __cplusplus
extern "C" {
#endif


#include "table_header.h"
#include "nvtypes.h"


typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT16             existence_category;
  NV_INT16             location_category;
  NV_INT16             usage;
  NV_INT16             tile_reference_identifier;
  NV_INT32             edge_primitive_key;
} HAZARDL_LFT;


  NV_INT32 open_hazardl_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_hazardl_lft (NV_INT32 hnd);
  NV_BOOL read_hazardl_lft (NV_INT32 hnd, NV_INT32 recnum, HAZARDL_LFT *data);
  void dump_hazardl_lft (HAZARDL_LFT data);


#ifdef  __cplusplus
}
#endif

#endif
