
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



#include "inside.h"


#define	M_PI		3.14159265358979323846	/* pi */
#define	M_PI_2		1.57079632679489661923	/* pi/2 */




/***************************************************************************/
/*!

  - Module Name:        angle

  - Programmer(s):      Unknown

  - Date Written:       Unknown

  - Modified:           Jan C. Depner, ported to C.

  - Date Modified:      August 1992

  - Purpose:            Computes the angle between the three geographic
                        points specified.

  - Arguments:
                        - x0       =   x position of point 0
                        - y0       =   y position of point 0
                        - x1       =   x position of point 1
                        - y1       =   y position of point 1
                        - x2       =   x position of point 2
                        - y2       =   y position of point 2

  - Return Value:       NV_FLOAT64 =   angle

  - Method:             Computes the cross and dot products of the two
                        vectors.  The cross product divided by the dot
                        product gives the tangent of the angle.  Then the
                        arc tangent is taken to get the angle.

****************************************************************************/

static NV_FLOAT64 angle (NV_FLOAT64 x0, NV_FLOAT64 y0, NV_FLOAT64 x1, NV_FLOAT64 y1, 
                         NV_FLOAT64 x2, NV_FLOAT64 y2)
{
  NV_FLOAT64          xx1, yy1, xx2, yy2, cross, dot, local_angle;

    xx1 = x1 - x0;
    yy1 = y1 - y0;
    xx2 = x2 - x0;
    yy2 = y2 - y0;
    cross = xx1 * yy2 - xx2 * yy1;
    dot = xx1 * xx2 + yy1 * yy2;

    if (fabs (dot) <= 1.0e-30)
    {
        local_angle = M_PI_2;
    }
    else
    {
        local_angle = atan (fabs (cross / dot));
        if (dot < 0.0) local_angle = M_PI - local_angle;
    }

    if (cross < 0.0) local_angle = -local_angle;

    return (local_angle);
}



/***************************************************************************/
/*!

  - Module Name:        inside

  - Programmer(s):      Unknown

  - Date Written:       Unknown

  - Modified:           Jan C. Depner, ported to C.

  - Date Modified:      August 1992

  - Purpose:            Checks a point to see if it falls within the
                        specified polygon.

  - Arguments:
                        - ax       =   x points
                        - ay       =   y points
                        - count    =   number of points
                        - x        =   x position of point
                        - y        =   y position of point

  - Return Value:       NV_INT32   =   1 if inside, 0 if not

  - Method:             Sums all of the angles between the point and each
                        pair of polygon vertices.  If the sum is greater
                        than PI, the point is inside the polygon.

  - Restrictions:       The vertices of the polygon must be in order around
                        the polygon.

****************************************************************************/

NV_INT32 inside (NV_FLOAT64 *ax, NV_FLOAT64 *ay, NV_INT32 count, NV_FLOAT64 x, NV_FLOAT64 y)
{
  NV_FLOAT64          angle_sum;
  NV_INT32            i;


  /*  There have to be at least three points in the polygon.          */

  if (count > 2)
    {
      angle_sum = 0.0;

      for (i = 1 ; i < count ; i++)
        {
          angle_sum += angle (x, y, ax[i], ay[i], ax[i - 1], ay[i - 1]);
        }
        
      angle_sum += angle (x, y, ax[0], ay[0], ax[count - 1], ay[count - 1]);

      if (fabs (angle_sum) > M_PI) return (1);
    }
    
  return (0);
}


/*!  A version of inside using an NV_F64_COORD2 array as input.  */

NV_INT32 inside_coord2 (NV_F64_COORD2 *xy, NV_INT32 count, NV_FLOAT64 x, NV_FLOAT64 y)
{
  NV_FLOAT64          angle_sum;
  NV_INT32            i;


  /*  There have to be at least three points in the polygon.          */

  if (count > 2)
    {
      angle_sum = 0.0;

      for (i = 1 ; i < count ; i++)
        {
          angle_sum += angle (x, y, xy[i].x, xy[i].y, xy[i - 1].x, xy[i - 1].y);
        }
        
      angle_sum += angle (x, y, xy[0].x, xy[0].y, xy[count - 1].x, xy[count - 1].y);

      if (fabs (angle_sum) > M_PI) return (1);
    }
    
  return (0);
}


/*!  A version of inside using an AREA structure as input.  */

NV_INT32 inside_area (AREA area, NV_FLOAT64 x, NV_FLOAT64 y)
{
    NV_FLOAT64          angle_sum;
    NV_INT32            i;


    /*  There have to be at least three points in the polygon.          */

    if (area.points > 2)
    {
        angle_sum = 0.0;

        for (i = 1 ; i < area.points ; i++)
        {
            angle_sum += angle (x, y, area.lon[i], area.lat[i], area.lon[i - 1], area.lat[i - 1]);
        }
        
        angle_sum += angle (x, y, area.lon[0], area.lat[0], area.lon[area.points - 1], area.lat[area.points - 1]);

        if (fabs (angle_sum) > M_PI) return (1);
    }
    
    return (0);
}



/*!  Checking for point inside a minimum bounding rectangle (MBR).  */

NV_BOOL inside_mbr (NV_F64_MBR *mbr, NV_FLOAT64 x, NV_FLOAT64 y)
{
  if (x < mbr->wlon || x > mbr->elon || y < mbr->slat || y > mbr->nlat) return (NVFalse);

  return (NVTrue);
}



NV_BOOL inside_mbr_coord2 (NV_F64_MBR *mbr, NV_F64_COORD2 xy)
{
  if (xy.x < mbr->wlon || xy.x > mbr->elon || xy.y < mbr->slat || xy.y > mbr->nlat) return (NVFalse);

  return (NVTrue);
}



NV_BOOL inside_mbr_coord3 (NV_F64_MBR *mbr, NV_F64_COORD3 xyz)
{
  if (xyz.x < mbr->wlon || xyz.x > mbr->elon || xyz.y < mbr->slat || xyz.y > mbr->nlat) return (NVFalse);

  return (NVTrue);
}



NV_BOOL inside_xymbr (NV_F64_XYMBR *mbr, NV_FLOAT64 x, NV_FLOAT64 y)
{
  if (x < mbr->min_x || x > mbr->max_x || y < mbr->min_y || y > mbr->max_y) return (NVFalse);

  return (NVTrue);
}



NV_BOOL inside_xymbr_coord2 (NV_F64_XYMBR *mbr, NV_F64_COORD2 xy)
{
  if (xy.x < mbr->min_x || xy.x > mbr->max_x || xy.y < mbr->min_y || xy.y > mbr->max_y) return (NVFalse);

  return (NVTrue);
}



NV_BOOL inside_xymbr_coord3 (NV_F64_XYMBR *mbr, NV_F64_COORD3 xyz)
{
  if (xyz.x < mbr->min_x || xyz.x > mbr->max_x || xyz.y < mbr->min_y || xyz.y > mbr->max_y) return (NVFalse);

  return (NVTrue);
}
