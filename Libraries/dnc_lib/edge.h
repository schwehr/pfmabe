#ifndef _EDGE_H_
#define _EDGE_H_

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
  NV_INT32        id;
  NV_INT32        start_node;
  NV_INT32        end_node;
  TRIPLET         right_face_t;
  TRIPLET         left_face_t;
  TRIPLET         right_edge_t;
  TRIPLET         left_edge_t;
  NV_INT32        right_face_i;
  NV_INT32        left_face_i;
  NV_INT32        right_edge_i;
  NV_INT32        left_edge_i;
  NV_INT32        count;
  NV_F32_POS      *coordinate;
} EDGE;


  NV_INT32 open_edge (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_edge (NV_INT32 hnd);
  NV_BOOL read_edge (NV_INT32 hnd, NV_INT32 recnum, EDGE *data);
  void free_edge (EDGE *data);
  void dump_edge (EDGE data);


#ifdef  __cplusplus
}
#endif

#endif
