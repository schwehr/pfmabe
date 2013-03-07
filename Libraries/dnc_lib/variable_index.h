#ifndef __VARIABLE_INDEX_H__
#define __VARIABLE_INDEX_H__


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


typedef struct
{
  NV_INT32             offset;
  NV_INT32             size;
} INDEX_ADDRESS;


typedef struct
{
  NV_INT32             num_recs;
  NV_INT32             header_length;
  INDEX_ADDRESS        *address;
} DNC_VARIABLE_INDEX;


  NV_INT32 open_dnc_variable_index (NV_CHAR *table_path, NV_INT32 table_hnd);
  void close_dnc_variable_index (NV_INT32 table_hnd);
  void dump_dnc_variable_index (NV_INT32 table_hnd);
  INDEX_ADDRESS get_dnc_variable_index_address (NV_INT32 table_hnd, NV_INT32 rec_num);


#ifdef  __cplusplus
}
#endif

#endif
