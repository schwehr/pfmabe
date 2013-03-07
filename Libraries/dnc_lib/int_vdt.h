#ifndef __INT_VDT_H__
#define __INT_VDT_H__


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
  NV_CHAR              table_name[13];
  NV_CHAR              column_name[11];
  NV_INT16             attr_value;
  NV_CHAR              description[51];
} INT_VDT_REC;

typedef struct
{
  NV_INT32             num_recs;
  INT_VDT_REC          *rec;
} INT_VDT;


  NV_INT32 open_int_vdt (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_int_vdt (NV_INT32 hnd);
  void dump_int_vdt (NV_INT32 hnd);
  NV_BOOL get_int_vdt_description (NV_INT32 hnd, NV_CHAR *table_name, NV_CHAR *column_name, NV_INT16 attr_value,
                                   NV_CHAR *description);


#ifdef  __cplusplus
}
#endif

#endif
