#ifndef __TABLE_HEADER_H__
#define __TABLE_HEADER_H__


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


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "nvtypes.h"
#include "nvdef.h"
#include "swap_bytes.h"
#include "basename.h"
#include "dnc_general.h"


#define MAX_DNC_TABLES 512


typedef struct
{
  NV_CHAR              column_name[17];
  NV_CHAR              field_type;
  NV_INT16             number_of_elements;
  NV_CHAR              key_type;
  NV_CHAR              column_description[81];
  NV_CHAR              value_description_table[13];
  NV_CHAR              thematic_index[13];
} DNC_REPEATING_HEADER;


typedef struct
{
  FILE                 *fp;
  NV_BYTE              library_type;
  NV_INT32             header_length;
  NV_BOOL              swap;
  NV_CHAR              table_description[81];
  NV_CHAR              documentation_file_name[13];
  NV_INT32             num_col;
  NV_INT32             reclen;
  DNC_REPEATING_HEADER *rh;
} DNC_TABLE_HEADER;


NV_INT32 open_dnc_table (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
NV_INT32 open_dnc_table_no_type_check (NV_CHAR *path, DNC_TABLE_HEADER **table_header);
void close_dnc_table (NV_INT32 hnd);
void dump_dnc_table_header (DNC_TABLE_HEADER *table_header);
DNC_TABLE_HEADER *get_dnc_table_header (NV_INT32 hnd);


#ifdef  __cplusplus
}
#endif

#endif
