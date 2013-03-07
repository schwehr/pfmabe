#ifndef _ENTITY_H_
#define _ENTITY_H_

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
  NV_INT32        containing_face;
  NV_F32_POS      coordinate;
} ENTITY;


  NV_INT32 open_entity (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_entity (NV_INT32 hnd);
  NV_BOOL read_entity (NV_INT32 hnd, NV_INT32 recnum, ENTITY *data);
  void dump_entity (ENTITY data);


#ifdef  __cplusplus
}
#endif

#endif
