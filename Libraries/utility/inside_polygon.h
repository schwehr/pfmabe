
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



#ifndef _INSIDE_POLYGON_H_
#define _INSIDE_POLYGON_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include "nvtypes.h"
#include "line_intersection.h"


  NV_INT32 inside_polygon (NV_F64_COORD2 *poly, NV_INT32 npol, NV_FLOAT64 x, NV_FLOAT64 y);
  NV_INT32 inside_polygon2 (NV_FLOAT64 *poly_x, NV_FLOAT64 *poly_y, NV_INT32 npol, NV_FLOAT64 x, NV_FLOAT64 y);
  NV_INT32 inside_polygon3 (NV_INT32 *xs, NV_INT32 *ys, NV_INT32 count, NV_INT32 x, NV_INT32 y);
  NV_BOOL polygon_collision (NV_F64_COORD2 *poly1, NV_INT32 npol1, NV_F64_COORD2 *poly2, NV_INT32 npol2);
  NV_BOOL polygon_collision2 (NV_FLOAT64 *poly1x, NV_FLOAT64 *poly1y, NV_INT32 npol1, NV_FLOAT64 *poly2x, NV_FLOAT64 *poly2y, NV_INT32 npol2);


#ifdef  __cplusplus
}
#endif

#endif
