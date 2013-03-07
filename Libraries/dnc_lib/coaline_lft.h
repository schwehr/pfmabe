#ifndef __COALINE_LFT_H__
#define __COALINE_LFT_H__


#ifdef  __cplusplus
extern "C" {
#endif


#include "table_header.h"
#include "nvtypes.h"


typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT32             edge_primitive_key;
} COALINE_LFT;


  NV_INT32 open_coaline_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_coaline_lft (NV_INT32 hnd);
  NV_BOOL read_coaline_lft (NV_INT32 hnd, NV_INT32 recnum, COALINE_LFT *data);
  void dump_coaline_lft (COALINE_LFT data);


#ifdef  __cplusplus
}
#endif

#endif
