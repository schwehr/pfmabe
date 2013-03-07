#ifndef __TUNNELL_LFT_H__
#define __TUNNELL_LFT_H__


#ifdef  __cplusplus
extern "C" {
#endif


#include "table_header.h"
#include "nvtypes.h"


typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_FLOAT32           hydrographic_depth;
  NV_CHAR              name[31];
  NV_INT16             transportation_use_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             edge_primitive_key;
} TUNNELL_LFT;


  NV_INT32 open_tunnell_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_tunnell_lft (NV_INT32 hnd);
  NV_BOOL read_tunnell_lft (NV_INT32 hnd, NV_INT32 recnum, TUNNELL_LFT *data);
  void dump_tunnell_lft (TUNNELL_LFT data);


#ifdef  __cplusplus
}
#endif

#endif
