
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



#include "polygon_intersection.h"
#include "line_intersection.h"
#include "inside_polygon.h"


/*!

  - These functions just tell you whether the polygons intersect, not what the intersection is.  This is a brute force method and I'm sure there
    are better implementations but I really don't need the speed here.
*/

NV_BOOL polygon_intersection (NV_F64_COORD2 *poly1, NV_INT32 poly1_count, NV_F64_COORD2 *poly2, NV_INT32 poly2_count)
{
  NV_INT32             i, j;
  NV_FLOAT64           x, y;


  /*  Check each point of polygon 1 to see if any lie inside polygon 2.  */

  for (i = 0 ; i < poly1_count ; i++)
    {
      if (inside_polygon (poly2, poly2_count, poly1[i].x, poly1[i].y)) return (NVTrue);
    }


  /*  Check each point of polygon 2 to see if any lie inside polygon 1.  */

  for (i = 0 ; i < poly2_count ; i++)
    {
      if (inside_polygon (poly1, poly1_count, poly2[i].x, poly2[i].y)) return (NVTrue);
    }


  /*  Now we have to check for intersection of any of the lines of each polygon.  */

  for (i = 1 ; i < poly1_count ; i++)
    {
      for (j = 1 ; j < poly2_count ; j++)
        {
          if (line_intersection (poly1[i - 1].x, poly1[i - 1].y, poly1[i].x, poly1[i].y, poly2[j - 1].x, poly2[j - 1].y, poly2[j].x, poly2[j].y, &x, &y) == 2)
            return (NVTrue);
        }
    }


  /*  Now we check the polygon 2 closing line segment against the polygon 1 segments (without the closing segment).  */

  for (i = 1 ; i < poly1_count ; i++)
    {
      if (line_intersection (poly1[i - 1].x, poly1[i - 1].y, poly1[i].x, poly1[i].y, poly2[poly2_count - 1].x, poly2[poly2_count - 1].y,
                             poly2[0].x, poly2[0].y, &x, &y) == 2) return (NVTrue);
    }
      

  /*  Now we check the polygon 1 closing line segment against the polygon 2 segments (without the closing segment).  */

  for (i = 1 ; i < poly2_count ; i++)
    {
      if (line_intersection (poly2[i - 1].x, poly2[i - 1].y, poly2[i].x, poly2[i].y, poly1[poly1_count - 1].x, poly1[poly1_count - 1].y,
                             poly1[0].x, poly1[0].y, &x, &y) == 2) return (NVTrue);
    }
      

  /*  Finally, we check the closing segments against each other.  */

  if (line_intersection (poly2[poly2_count - 1].x, poly2[poly2_count - 1].y, poly2[0].x, poly2[0].y, poly1[poly1_count - 1].x, poly1[poly1_count - 1].y,
                         poly1[0].x, poly1[0].y, &x, &y) == 2) return (NVTrue);


  /*  If we got here there is no intersection.  */

  return (NVFalse);
}


NV_BOOL polygon_intersection2 (NV_FLOAT64 *poly1_x, NV_FLOAT64 *poly1_y, NV_INT32 poly1_count, NV_FLOAT64 *poly2_x, NV_FLOAT64 *poly2_y,
                               NV_INT32 poly2_count)
{
  NV_INT32             i, j;
  NV_FLOAT64           x, y;


  /*  Check each point of polygon 1 to see if any lie inside polygon 2.  */

  for (i = 0 ; i < poly1_count ; i++)
    {
      if (inside_polygon2 (poly2_x, poly2_y, poly2_count, poly1_x[i], poly1_y[i])) return (NVTrue);
    }


  /*  Check each point of polygon 2 to see if any lie inside polygon 1.  */

  for (i = 0 ; i < poly2_count ; i++)
    {
      if (inside_polygon2 (poly1_x, poly1_y, poly1_count, poly2_x[i], poly2_y[i])) return (NVTrue);
    }


  /*  Now we have to check for intersection of any of the lines of each polygon.  */

  for (i = 1 ; i < poly1_count ; i++)
    {
      for (j = 1 ; j < poly2_count ; j++)
        {
          if (line_intersection (poly1_x[i - 1], poly1_y[i - 1], poly1_x[i], poly1_y[i], poly2_x[j - 1], poly2_y[j - 1], poly2_x[j], poly2_y[j], &x, &y) == 2)
            return (NVTrue);
        }
    }


  /*  Now we check the polygon 2 closing line segment against the polygon 1 segments (without the closing segment).  */

  for (i = 1 ; i < poly1_count ; i++)
    {
      if (line_intersection (poly1_x[i - 1], poly1_y[i - 1], poly1_x[i], poly1_y[i], poly2_x[poly2_count - 1], poly2_y[poly2_count - 1],
                             poly2_x[0], poly2_y[0], &x, &y) == 2) return (NVTrue);
    }
      

  /*  Now we check the polygon 1 closing line segment against the polygon 2 segments (without the closing segment).  */

  for (i = 1 ; i < poly2_count ; i++)
    {
      if (line_intersection (poly2_x[i - 1], poly2_y[i - 1], poly2_x[i], poly2_y[i], poly1_x[poly1_count - 1], poly1_y[poly1_count - 1],
                             poly1_x[0], poly1_y[0], &x, &y) == 2) return (NVTrue);
    }
      

  /*  Finally, we check the closing segments against each other.  */

  if (line_intersection (poly2_x[poly2_count - 1], poly2_y[poly2_count - 1], poly2_x[0], poly2_y[0], poly1_x[poly1_count - 1], poly1_y[poly1_count - 1],
                         poly1_x[0], poly1_y[0], &x, &y) == 2) return (NVTrue);


  /*  If we got here there is no intersection.  */

  return (NVFalse);
}



NV_BOOL polygon_intersection3 (NV_F64_XYMBR mbr, NV_F64_COORD2 *poly, NV_INT32 poly_count)
{
  NV_F64_COORD2        poly2[4];

  poly2[0].x = mbr.min_x;
  poly2[0].y = mbr.min_y;
  poly2[1].x = mbr.min_x;
  poly2[1].y = mbr.max_y;
  poly2[2].x = mbr.max_x;
  poly2[2].y = mbr.max_y;
  poly2[3].x = mbr.max_x;
  poly2[3].y = mbr.min_y;


  return (polygon_intersection (poly, poly_count, poly2, 4));
}



NV_BOOL polygon_intersection4 (NV_F64_XYMBR mbr, NV_FLOAT64 *poly_x, NV_FLOAT64 *poly_y, NV_INT32 poly_count)
{
  NV_FLOAT64           poly2_x[4], poly2_y[4];

  poly2_x[0] = mbr.min_x;
  poly2_y[0] = mbr.min_y;
  poly2_x[1] = mbr.min_x;
  poly2_y[1] = mbr.max_y;
  poly2_x[2] = mbr.max_x;
  poly2_y[2] = mbr.max_y;
  poly2_x[3] = mbr.max_x;
  poly2_y[3] = mbr.min_y;


  return (polygon_intersection2 (poly_x, poly_y, poly_count, poly2_x, poly2_y, 4));
}



NV_BOOL mbr_intersection (NV_F64_XYMBR mbr1, NV_F64_XYMBR mbr2)
{
  NV_F64_COORD2        poly1[4], poly2[4];

  poly1[0].x = mbr1.min_x;
  poly1[0].y = mbr1.min_y;
  poly1[1].x = mbr1.min_x;
  poly1[1].y = mbr1.max_y;
  poly1[2].x = mbr1.max_x;
  poly1[2].y = mbr1.max_y;
  poly1[3].x = mbr1.max_x;
  poly1[3].y = mbr1.min_y;


  poly2[0].x = mbr2.min_x;
  poly2[0].y = mbr2.min_y;
  poly2[1].x = mbr2.min_x;
  poly2[1].y = mbr2.max_y;
  poly2[2].x = mbr2.max_x;
  poly2[2].y = mbr2.max_y;
  poly2[3].x = mbr2.max_x;
  poly2[3].y = mbr2.min_y;


  return (polygon_intersection (poly1, 4, poly2, 4));
}
