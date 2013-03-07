#ifndef __COASTL_LFT_H__
#define __COASTL_LFT_H__


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
  NV_INT16             shoreline_type_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             edge_primitive_key;
} COASTL_LFT;


  NV_INT32 open_coastl_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_coastl_lft (NV_INT32 hnd);
  NV_BOOL read_coastl_lft (NV_INT32 hnd, NV_INT32 recnum, COASTL_LFT *data);
  void dump_coastl_lft (COASTL_LFT data);


#ifdef  __cplusplus
}
#endif

#endif
