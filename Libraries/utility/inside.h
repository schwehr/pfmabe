
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef _INSIDE_H_
#define _INSIDE_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <math.h>
#include "nvtypes.h"
#include "area.h"


  NV_INT32 inside (NV_FLOAT64 *ax, NV_FLOAT64 *ay, NV_INT32 count, NV_FLOAT64 x, NV_FLOAT64 y);
  NV_INT32 inside_coord2 (NV_F64_COORD2 *xy, NV_INT32 count, NV_FLOAT64 x, NV_FLOAT64 y);
  NV_INT32 inside_area (AREA area, NV_FLOAT64 x, NV_FLOAT64 y);
  NV_BOOL inside_mbr (NV_F64_MBR *mbr, NV_FLOAT64 x, NV_FLOAT64 y);
  NV_BOOL inside_mbr_coord2 (NV_F64_MBR *mbr, NV_F64_COORD2 xy);
  NV_BOOL inside_mbr_coord3 (NV_F64_MBR *mbr, NV_F64_COORD3 xyz);
  NV_BOOL inside_xymbr (NV_F64_XYMBR *mbr, NV_FLOAT64 x, NV_FLOAT64 y);
  NV_BOOL inside_xymbr_coord2 (NV_F64_XYMBR *mbr, NV_F64_COORD2 xy);
  NV_BOOL inside_xymbr_coord3 (NV_F64_XYMBR *mbr, NV_F64_COORD3 xyz);


#ifdef  __cplusplus
}
#endif

#endif
