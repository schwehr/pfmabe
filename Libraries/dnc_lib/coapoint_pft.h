#ifndef __COAPOINT_PFT_H__
#define __COAPOINT_PFT_H__


#ifdef  __cplusplus
extern "C" {
#endif


#include "table_header.h"
#include "nvtypes.h"


typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_CHAR              name[31];
  NV_INT32             entity_node_key;
} COAPOINT_PFT;


  NV_INT32 open_coapoint_pft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_coapoint_pft (NV_INT32 hnd);
  NV_BOOL read_coapoint_pft (NV_INT32 hnd, NV_INT32 recnum, COAPOINT_PFT *data);
  void dump_coapoint_pft (COAPOINT_PFT data);


#ifdef  __cplusplus
}
#endif

#endif
