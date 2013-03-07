#ifndef __DANGERA_AFT_H__
#define __DANGERA_AFT_H__


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
  NV_INT16             date;
  NV_INT16             existence_category;
  NV_INT16             hydrographic_depth_height_info;
  NV_FLOAT32           hydrographic_depth;
  NV_INT16             value;
  NV_INT16             vertical_reference_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             face_primitive_key;
} DANGERA_AFT;


  NV_INT32 open_dangera_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_dangera_aft (NV_INT32 hnd);
  NV_BOOL read_dangera_aft (NV_INT32 hnd, NV_INT32 recnum, DANGERA_AFT *data);
  void dump_dangera_aft (DANGERA_AFT data);


#ifdef  __cplusplus
}
#endif

#endif
