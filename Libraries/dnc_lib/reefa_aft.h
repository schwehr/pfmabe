#ifndef __REEFA_AFT_H__
#define __REEFA_AFT_H__


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
  NV_FLOAT32           hydrographic_drying_height;
  NV_INT16             hydrographic_depth_height_info;
  NV_FLOAT32           hydrographic_depth;
  NV_INT16             material_composition_category;
  NV_CHAR              name[31];
  NV_INT16             severity;
  NV_INT16             value;
  NV_INT16             vertical_reference_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             face_primitive_key;
} REEFA_AFT;


  NV_INT32 open_reefa_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_reefa_aft (NV_INT32 hnd);
  NV_BOOL read_reefa_aft (NV_INT32 hnd, NV_INT32 recnum, REEFA_AFT *data);
  void dump_reefa_aft (REEFA_AFT data);


#ifdef  __cplusplus
}
#endif

#endif
