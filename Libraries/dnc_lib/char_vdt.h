#ifndef __CHAR_VDT_H__
#define __CHAR_VDT_H__


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
  NV_CHAR              attr_value[6];
  NV_CHAR              description[51];
} CHAR_VDT_REC;

typedef struct
{
  NV_INT32             num_recs;
  CHAR_VDT_REC         *rec;
} CHAR_VDT;


  NV_INT32 open_char_vdt (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
  void close_char_vdt (NV_INT32 hnd);
  void dump_char_vdt (NV_INT32 hnd);
  NV_BOOL get_char_vdt_description (NV_INT32 hnd, NV_CHAR *table_name, NV_CHAR *column_name, NV_CHAR *attr_value,
                                    NV_CHAR *description);


#ifdef  __cplusplus
}
#endif

#endif
