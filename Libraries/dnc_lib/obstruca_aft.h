#ifndef __OBSTRUCA_AFT_H__
#define __OBSTRUCA_AFT_H__


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
  NV_INT16             tile_reference_identifier;
  NV_INT32             face_primitive_key;
} OBSTRUCA_AFT;


  NV_INT32 open_obstruca_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_obstruca_aft (NV_INT32 hnd);
  NV_BOOL read_obstruca_aft (NV_INT32 hnd, NV_INT32 recnum, OBSTRUCA_AFT *data);
  void dump_obstruca_aft (OBSTRUCA_AFT data);


#ifdef  __cplusplus
}
#endif

#endif
