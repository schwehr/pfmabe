#ifndef __COAAREA_AFT_H__
#define __COAAREA_AFT_H__


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
  NV_INT32             face_primitive_key;
} COAAREA_AFT;


  NV_INT32 open_coaarea_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_coaarea_aft (NV_INT32 hnd);
  NV_BOOL read_coaarea_aft (NV_INT32 hnd, NV_INT32 recnum, COAAREA_AFT *data);
  void dump_coaarea_aft (COAAREA_AFT data);


#ifdef  __cplusplus
}
#endif

#endif
