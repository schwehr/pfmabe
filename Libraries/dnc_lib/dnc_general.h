#ifndef _DNC_GENERAL_H_
#define _DNC_GENERAL_H_

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


#include "nvtypes.h"


#define HARBOR 0
#define APPROACH 1
#define COASTAL 2
#define GENERAL 3
#define TOD2 4


typedef struct
{
  NV_U_BYTE       type;
  NV_INT32        field[3];
} TRIPLET;


  void trim_read (NV_CHAR *string, NV_INT32 length, FILE *fp);
  void swap_triplet (TRIPLET *triplet);
  void read_triplet (TRIPLET *triplet, FILE *fp);
  void set_library_type (NV_CHAR *library_path);
  NV_BYTE get_library_type ();
  NV_CHAR *get_library_name ();


#ifdef  __cplusplus
}
#endif

#endif
