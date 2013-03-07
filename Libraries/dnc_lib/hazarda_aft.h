#ifndef __HAZARDA_AFT_H__
#define __HAZARDA_AFT_H__


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
  NV_INT16             certainty;
  NV_INT16             date;
  NV_INT16             existence_category;
  NV_INT16             hydrographic_depth_height_info;
  NV_FLOAT32           hydrographic_depth;
  NV_INT16             location_category;
  NV_INT16             sea_floor_feature_category;
  NV_INT16             severity;
  NV_CHAR              text[256];
  NV_INT16             value;
  NV_INT16             vertical_reference_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             face_primitive_key;
} HAZARDA_AFT;


  NV_INT32 open_hazarda_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_hazarda_aft (NV_INT32 hnd);
  NV_BOOL read_hazarda_aft (NV_INT32 hnd, NV_INT32 recnum, HAZARDA_AFT *data);
  void dump_hazarda_aft (HAZARDA_AFT data);


#ifdef  __cplusplus
}
#endif

#endif
