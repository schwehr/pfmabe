#ifndef __NAVAID_PFT_H__
#define __NAVAID_PFT_H__


/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


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
  NV_INT32             broadcast_frequency[2];
  NV_INT16             buoy_beacon_type;
  NV_INT16             color_code;
  NV_CHAR              character[11];
  NV_INT16             elevation;
  NV_INT16             nominal_light_range;
  NV_CHAR              multiple_light_ranges[11];
  NV_CHAR              name[31];
  NV_INT16             nav_system_type[2];
  NV_INT16             operating_range[2];
  NV_FLOAT32           light_period;
  NV_INT16             radar_reflector_attr;
  NV_INT16             structure_shape_category;
  NV_INT16             sound_signal_type;
  NV_INT16             topmark_characteristic;
  NV_CHAR              text[256];
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} BUOYBCNP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT16             accuracy_category;
  NV_INT32             broadcast_frequency[2];
  NV_INT16             color_code;
  NV_CHAR              character[11];
  NV_INT16             elevation;
  NV_INT16             hydrographic_light_type;
  NV_INT16             IALA_aid_category;
  NV_INT16             nominal_light_range;
  NV_CHAR              multiple_light_ranges[11];
  NV_CHAR              name[31];
  NV_INT16             nav_system_type[2];
  NV_FLOAT32           light_period;
  NV_INT16             radar_reflector_attr;
  NV_INT16             sound_signal_type;
  NV_INT16             topmark_characteristic;
  NV_CHAR              text[256];
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} LIGHTSP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT16             usage;
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} MARKERP_PFT;



  NV_INT32 open_navaid_pft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_navaid_pft (NV_INT32 hnd);


  NV_BOOL read_buoybcnp_pft (NV_INT32 hnd, NV_INT32 recnum, BUOYBCNP_PFT *data);
  void dump_buoybcnp_pft (BUOYBCNP_PFT data);
  NV_BOOL read_lightsp_pft (NV_INT32 hnd, NV_INT32 recnum, LIGHTSP_PFT *data);
  void dump_lightsp_pft (LIGHTSP_PFT data);
  NV_BOOL read_markerp_pft (NV_INT32 hnd, NV_INT32 recnum, MARKERP_PFT *data);
  void dump_markerp_pft (MARKERP_PFT data);


#ifdef  __cplusplus
}
#endif

#endif
