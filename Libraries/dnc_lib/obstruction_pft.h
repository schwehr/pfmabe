#ifndef __OBSTRUCTION_PFT_H__
#define __OBSTRUCTION_PFT_H__


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
  NV_INT16             date;
  NV_INT16             existence_category;
  NV_INT16             hydrographic_depth_height_info;
  NV_FLOAT32           hydrographic_depth;
  NV_INT16             value;
  NV_INT16             vertical_reference_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} DANGERP_PFT;

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
  NV_INT16             location_category;
  NV_INT16             material_composition_category;
  NV_CHAR              name[31];
  NV_INT16             seafloor_feature_category;
  NV_INT16             severity;
  NV_CHAR              text[256];
  NV_INT16             value;
  NV_INT16             vertical_reference_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} HAZARDP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT16             light_characteristic_category;
  NV_CHAR              name[31];
  NV_INT16             nav_system_type;
  NV_INT16             sound_signal_type;
  NV_INT16             usage;
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} LOADINGP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} OBSTRUCP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_FLOAT32           hydrographic_drying_height;
  NV_INT16             hydrographic_depth_height_info;
  NV_FLOAT32           hydrographic_depth;
  NV_INT16             location_category;
  NV_CHAR              name[31];
  NV_INT16             vertical_reference_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} RUINSP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_CHAR              berth_identifier[26];
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} BERTHP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT16             traffic_direction[4];
  NV_CHAR              name[30];
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} CALLINP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT16             certainty;
  NV_INT16             existence_category;
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} LANDINGP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} MOORINGP_PFT;

typedef struct
{
  NV_INT32             id;
  NV_CHAR              facc_code[6];
  NV_INT16             accuracy_category;
  NV_INT16             elevation_accuracy;
  NV_INT32             highest_z_value;
  NV_INT16             tile_reference_identifier;
  NV_INT32             entity_node_key;
} RELPOINT_PFT;



  NV_INT32 open_obstruction_pft (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_obstruction_pft (NV_INT32 hnd);


  NV_BOOL read_dangerp_pft (NV_INT32 hnd, NV_INT32 recnum, DANGERP_PFT *data);
  void dump_dangerp_pft (DANGERP_PFT data);
  NV_BOOL read_hazardp_pft (NV_INT32 hnd, NV_INT32 recnum, HAZARDP_PFT *data);
  void dump_hazardp_pft (HAZARDP_PFT data);
  NV_BOOL read_obstrucp_pft (NV_INT32 hnd, NV_INT32 recnum, OBSTRUCP_PFT *data);
  void dump_obstrucp_pft (OBSTRUCP_PFT data);
  NV_BOOL read_loadingp_pft (NV_INT32 hnd, NV_INT32 recnum, LOADINGP_PFT *data);
  void dump_loadingp_pft (LOADINGP_PFT data);
  NV_BOOL read_ruinsp_pft (NV_INT32 hnd, NV_INT32 recnum, RUINSP_PFT *data);
  void dump_ruinsp_pft (RUINSP_PFT data);
  NV_BOOL read_berthp_pft (NV_INT32 hnd, NV_INT32 recnum, BERTHP_PFT *data);
  void dump_berthp_pft (BERTHP_PFT data);
  NV_BOOL read_callinp_pft (NV_INT32 hnd, NV_INT32 recnum, CALLINP_PFT *data);
  void dump_callinp_pft (CALLINP_PFT data);
  NV_BOOL read_landingp_pft (NV_INT32 hnd, NV_INT32 recnum, LANDINGP_PFT *data);
  void dump_landingp_pft (LANDINGP_PFT data);
  NV_BOOL read_mooringp_pft (NV_INT32 hnd, NV_INT32 recnum, MOORINGP_PFT *data);
  void dump_mooringp_pft (MOORINGP_PFT data);
  NV_BOOL read_relpoint_pft (NV_INT32 hnd, NV_INT32 recnum, RELPOINT_PFT *data);
  void dump_relpoint_pft (RELPOINT_PFT data);


#ifdef  __cplusplus
}
#endif

#endif
