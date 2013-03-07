#ifndef _RING_H_
#define _RING_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include "table_header.h"
#include "nvtypes.h"


typedef struct
{
  NV_INT32        id;
  NV_INT32        face_key;
  NV_INT32        edge_primitive_key;
} RING;


  NV_INT32 open_ring (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_ring (NV_INT32 hnd);
  NV_BOOL read_ring (NV_INT32 hnd, NV_INT32 recnum, RING *data);
  void dump_ring (RING data);


#ifdef  __cplusplus
}
#endif

#endif
