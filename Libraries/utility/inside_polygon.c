
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



#include "inside_polygon.h"

/***************************************************************************/
/*!

  - Module Name:        inside_polygon

  - Programmer(s):      Found this on the www

  - Date Written:       Unknown

  - Purpose:            Determines if a data point is inside or outside of
                        a polygon

  - Arguments:
  - Return Value:
                        - 1: point is inside polygon
                        - 0: point is outside of polygon

  - Method:             This approach is MUCH faster than the approach used by
                        "inside" of summing the angles between the point and
                        each vertex of the polygon.

****************************************************************************/

NV_INT32 inside_polygon (NV_F64_COORD2 *poly, NV_INT32 npol, NV_FLOAT64 x, NV_FLOAT64 y)
{
  NV_INT32 i;
  NV_INT32 j;
  NV_INT32 c = 0;
    
  for (i = 0, j = npol - 1 ; i < npol ; j = i++) 
    {
      if ((((poly[i].y <= y) && (y < poly[j].y)) || ((poly[j].y <= y) && (y < poly[i].y))) && 
          (x < (poly[j].x - poly[i].x) * (y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x))
        {
            c = !c;
        }
    }
  return c;
}

NV_INT32 inside_polygon2 (NV_FLOAT64 *poly_x, NV_FLOAT64 *poly_y, NV_INT32 npol, NV_FLOAT64 x, NV_FLOAT64 y)
{
  NV_INT32 i;
  NV_INT32 j;
  NV_INT32 c = 0;
    
  for (i = 0, j = npol - 1 ; i < npol ; j = i++) 
    {
      if ((((poly_y[i] <= y) && (y < poly_y[j])) || ((poly_y[j] <= y) && (y < poly_y[i]))) && 
          (x < (poly_x[j] - poly_x[i]) * (y - poly_y[i]) / (poly_y[j] - poly_y[i]) + poly_x[i]))
        {
            c = !c;
        }
    }
  return c;
}


NV_INT32 inside_polygon3 (NV_INT32 *xs, NV_INT32 *ys, NV_INT32 count, NV_INT32 x, NV_INT32 y)
{
  NV_INT32 i, j, c = 0;

  for (i = 0, j = count - 1 ; i < count ; j = i++) 
    {
      if ((((ys[i] <= y) && (y < ys[j])) || ((ys[j] <= y) && (y < ys[i]))) && ((NV_FLOAT64) x < (xs[j] - xs[i]) * (y - ys[i]) /
                                                                               (NV_FLOAT64) (ys[j] - ys[i]) + xs[i])) 
        c = !c;
    }

  return c;
}


/***************************************************************************/
/*!

  - Module Name:        polygon_collision

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 11, 2009

  - Purpose:            Determines if two polygons intersect

  - Arguments:
  - Return Value:       NVTrue if they intersect, otherwise NVFalse

****************************************************************************/

NV_BOOL polygon_collision (NV_F64_COORD2 *poly1, NV_INT32 npol1, NV_F64_COORD2 *poly2, NV_INT32 npol2)
{
  NV_INT32 i, j;
  NV_FLOAT64 x, y;


  /*  Easy check first.  If any vertex of one polygon lies inside of the other then they intersect.  */

  for (i = 0 ; i < npol1 ; i++)
    {
      /*  First check to see if this vertex of polygon 1 lies inside polygon 2.  */

      if (inside_polygon (poly2, npol2, poly1[i].x, poly1[i].y)) return (NVTrue);


      for (j = 0 ; j < npol2 ; j++)
        {
          /*  Check to see if this vertex of polygon 2 lies inside polygon 1.  */

          if (inside_polygon (poly1, npol1, poly2[j].x, poly2[j].y)) return (NVTrue);
        }
    }


  /*  If we got here then it's time to do the hard check.  If any two lines intersect then there is a collision.  */

  for (i = 0 ; i < npol1 ; i++)
    {
      for (j = 0 ; j < npol2 ; j++)
        {
          if (i)
            {
              if (j)
                {
                  if (line_intersection (poly1[i - 1].x, poly1[i - 1].y, poly1[i].x, poly1[i].y,
                                         poly2[j - 1].x, poly2[j - 1].y, poly2[j].x, poly2[j].y, &x, &y) == 2) return (NVTrue);
                }
              else
                {
                  /*  If j is 0 then we have to check the last to first point line segment.  */

                  if (line_intersection (poly1[i - 1].x, poly1[i - 1].y, poly1[i].x, poly1[i].y,
                                         poly2[npol2 - 1].x, poly2[npol2 - 1].y, poly2[j].x, poly2[j].y, &x, &y) == 2) return (NVTrue);
                }
            }


          /*  If i is 0 then we have to check the last to first point line segment.  */

          else
            {
              if (j)
                {
                  if (line_intersection (poly1[npol1 - 1].x, poly1[npol1 - 1].y, poly1[i].x, poly1[i].y,
                                         poly2[j - 1].x, poly2[j - 1].y, poly2[j].x, poly2[j].y, &x, &y) == 2) return (NVTrue);
                }
              else
                {
                  /*  If j is 0 then we have to check the last to first point line segment.  */

                  if (line_intersection (poly1[npol1 - 1].x, poly1[npol1 - 1].y, poly1[i].x, poly1[i].y,
                                         poly2[npol2 - 1].x, poly2[npol2 - 1].y, poly2[j].x, poly2[j].y, &x, &y) == 2) return (NVTrue);
                }
            }
        }
    }

  return (NVFalse);
}



NV_BOOL polygon_collision2 (NV_FLOAT64 *poly1x, NV_FLOAT64 *poly1y, NV_INT32 npol1, NV_FLOAT64 *poly2x, NV_FLOAT64 *poly2y, NV_INT32 npol2)
{
  NV_INT32 i, j;
  NV_FLOAT64 x, y;


  /*  Easy check first.  If any vertex of one polygon lies inside of the other then they intersect.  */

  for (i = 0 ; i < npol1 ; i++)
    {
      /*  First check to see if this vertex of polygon 1 lies inside polygon 2.  */

      if (inside_polygon2 (poly2x, poly2y, npol2, poly1x[i], poly1y[i])) return (NVTrue);


      for (j = 0 ; j < npol2 ; j++)
        {
          /*  Check to see if this vertex of polygon 2 lies inside polygon 1.  */

          if (inside_polygon2 (poly1x, poly1y, npol1, poly2x[j], poly2y[j])) return (NVTrue);
        }
    }


  /*  If we got here then it's time to do the hard check.  If any two lines intersect then there is a collision.  */

  for (i = 0 ; i < npol1 ; i++)
    {
      for (j = 0 ; j < npol2 ; j++)
        {
          if (i)
            {
              if (j)
                {
                  if (line_intersection (poly1x[i - 1], poly1y[i - 1], poly1x[i], poly1y[i],
                                         poly2x[j - 1], poly2y[j - 1], poly2x[j], poly2y[j], &x, &y) == 2) return (NVTrue);
                }
              else
                {
                  /*  If j is 0 then we have to check the last to first point line segment.  */

                  if (line_intersection (poly1x[i - 1], poly1y[i - 1], poly1x[i], poly1y[i],
                                         poly2x[npol2 - 1], poly2y[npol2 - 1], poly2x[j], poly2y[j], &x, &y) == 2) return (NVTrue);
                }
            }


          /*  If i is 0 then we have to check the last to first point line segment.  */

          else
            {
              if (j)
                {
                  if (line_intersection (poly1x[npol1 - 1], poly1y[npol1 - 1], poly1x[i], poly1y[i],
                                         poly2x[j - 1], poly2y[j - 1], poly2x[j], poly2y[j], &x, &y) == 2) return (NVTrue);
                }
              else
                {
                  /*  If j is 0 then we have to check the last to first point line segment.  */

                  if (line_intersection (poly1x[npol1 - 1], poly1y[npol1 - 1], poly1x[i], poly1y[i],
                                         poly2x[npol2 - 1], poly2y[npol2 - 1], poly2x[j], poly2y[j], &x, &y) == 2) return (NVTrue);
                }
            }
        }
    }

  return (NVFalse);
}
