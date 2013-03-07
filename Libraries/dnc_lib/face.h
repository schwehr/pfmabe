#ifndef _FACE_H_
#define _FACE_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include "table_header.h"
#include "nvtypes.h"


typedef struct
{
  NV_INT32        id;
  NV_INT32        ring_primitive_key;
} FACE;


  NV_INT32 open_face (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_face (NV_INT32 hnd);
  NV_BOOL read_face (NV_INT32 hnd, NV_INT32 recnum, FACE *data);
  void dump_face (FACE data);


#ifdef  __cplusplus
}
#endif

#endif
