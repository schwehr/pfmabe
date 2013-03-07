
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



#include "line_intersection.h"

/************************************************************************/
/*!

  - Module Name:    line_intersection

  - Programmer:     Jan C. Depner

  - Date Written:   September 2006

  - Purpose:        Determines if two lines intersect.  If so it
                    returns the intersection point.

  - Inputs:
                    - x1            =  starting x coordinate line 1
                    - y1            =  starting y coordinate line 1
                    - x2            =  ending x coordinate line 1
                    - y2            =  ending y coordinate line 1
                    - x3            =  starting x coordinate line 2
                    - y3            =  starting y coordinate line 2
                    - x4            =  ending x coordinate line 2
                    - y4            =  ending y coordinate line 2
                    - x             =  intersection x coordinate
                    - y             =  intersection y coordinate

  - Outputs:
                    - 0 = no intersection
                    - 1 = lines intersect but the segments don't
                    - 2 = segments intersect



  - Referenced from: http://en.wikipedia.org/wiki/Line_segment_intersection


  - Derived from: http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
                  Intersection point of two lines (2 dimensions)
                  Paul Bourke, April 1989



  - This note describes the technique and algorithm for determining the intersection point of two lines
    (or line segments) in 2 dimensions.

  <pre>
                             P3        P2
                              \        /
                        line b \      /
                                \    /
                                 \  /
                                  \/
                                  /\
                                 /  \
                                /    \
                        line a /      \
                              /        \
                             P1        P4


    The equations of the lines are

    Pa = P1 + ua ( P2 - P1 )

    Pb = P3 + ub ( P4 - P3 )

    Solving for the point where Pa = Pb gives the following two equations in two unknowns (ua and ub) 
    x1 + ua (x2 - x1) = x3 + ub (x4 - x3)

    and
    y1 + ua (y2 - y1) = y3 + ub (y4 - y3)

    Solving gives the following expressions for ua and ub

    Substituting either of these into the corresponding equation for the line gives the intersection point.
    For example the intersection point (x,y) is
    x = x1 + ua (x2 - x1)

    y = y1 + ua (y2 - y1)


    Notes:

    * The denominators for the equations for ua and ub are the same.

    * If the denominator for the equations for ua and ub is 0 then the two lines are parallel.

    * If the denominator and numerator for the equations for ua and ub are 0 then the two lines are coincident.

    * The equations apply to lines, if the intersection of line segments is required then it is only necessary to test
      if ua and ub lie between 0 and 1. Whichever one lies within that range then the corresponding line segment
      contains the intersection point. If both lie within the range of 0 to 1 then the intersection point is within
      both line segments. 

</pre>
 ************************************************************************/


NV_INT32 line_intersection (NV_FLOAT64 x1, NV_FLOAT64 y1, NV_FLOAT64 x2, NV_FLOAT64 y2,
                            NV_FLOAT64 x3, NV_FLOAT64 y3, NV_FLOAT64 x4, NV_FLOAT64 y4,
                            NV_FLOAT64 *x, NV_FLOAT64 *y)
{
  NV_FLOAT64 denominator, ua, ub;


  denominator = ((y4 -y3) * (x2 - x1) - (x4 -x3) * (y2 - y1));

  if (denominator == 0.0) return (0);


  ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denominator;
  ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denominator;

  *x = x1 + ua * (x2 - x1);
  *y = y1 + ua * (y2 - y1);

  if (ua >= 0.0 && ua <= 1.0 && ub >= 0.0 && ub <= 1.0) return (2);


  return (1);
}



/***********************************************************************/
/*!

  - Module Name:    clip

  - Programmer:     Jan C. Depner

  - Date Written:   September 2006

  - Purpose:        Clips the input line segment to the input
                    minimum bounding rectangle.

  - Inputs:
                    - x1            =  starting x coordinate
                    - y1            =  starting y coordinate
                    - x2            =  ending x coordinate
                    - y2            =  ending y coordinate
                    - mbr           =  minimum bounding rectangle

  - Outputs:
                    - 0 = no part of line in MBR
                    - 1 = both ends are in MBR
                    - 2 = first point clipped
                    - 3 = last point clipped
                    - 4 = both points clipped

 ************************************************************************/

NV_INT32 clip (NV_FLOAT64 *x1, NV_FLOAT64 *y1, NV_FLOAT64 *x2, NV_FLOAT64 *y2, NV_F64_XYMBR mbr)
{
  NV_FLOAT64 x[2], y[2];
  NV_INT32 hits, retval;


  /*  No point in doing the math if the whole line is in the rectangle.  */

  if (*x1 >= mbr.min_x && *x1 <= mbr.max_x && *x2 >= mbr.min_x && *x2 <= mbr.max_x &&
      *y1 >= mbr.min_y && *y1 <= mbr.max_y && *y2 >= mbr.min_y && *y2 <= mbr.max_y) return (1);


  /*  No point in doing the math if no part of the line can be in the rectangle.  */

  if ((*x1 > mbr.max_x && *x2 > mbr.max_x) || (*x1 < mbr.min_x && *x2 < mbr.min_x) ||
      (*y1 > mbr.max_y && *y2 > mbr.max_y) || (*y1 < mbr.min_y && *y2 < mbr.min_y)) return (0);


  /*  Since this is a rectangle we can only intersect two lines at most.  */

  hits = 0;

  if (line_intersection (*x1, *y1, *x2, *y2, mbr.min_x, mbr.min_y, mbr.min_x, mbr.max_y, &x[hits], &y[hits]) == 2) hits++;


  if (line_intersection (*x1, *y1, *x2, *y2, mbr.min_x, mbr.max_y, mbr.max_x, mbr.max_y, &x[hits], &y[hits]) == 2) hits++;


  if (hits < 2)
    {
      if (line_intersection (*x1, *y1, *x2, *y2, mbr.max_x, mbr.max_y, mbr.max_x, mbr.min_y, &x[hits], &y[hits]) == 2) hits++;

      if (hits < 2)
        {
          if (line_intersection (*x1, *y1, *x2, *y2, mbr.max_x, mbr.min_y, mbr.min_x, mbr.min_y, &x[hits], &y[hits]) == 2) hits++;
        }
    }


  retval = 0;


  /*  If we only intersected one boundary, figure out which end needs to be clipped.  */

  if (hits == 1)
    {
      /*  Start point is inside.  */

      if (*x1 >= mbr.min_x && *x1 <= mbr.max_x && *y1 >= mbr.min_y && *y1 <= mbr.max_y)
        {
          *x2 = x[0];
          *y2 = y[0];

          retval = 3;
        }


      /*  End point is inside.  */

      else
        {
          *x1 = x[0];
          *y1 = y[0];

          retval = 2;
        }
    }


  /*  We intersected two sides.  Both start and end points must be outside the mbr.  */

  if (hits == 2)
    {
      *x1 = x[0];
      *y1 = y[0];
      *x2 = x[1];
      *y2 = y[1];

      retval = 4;
    }


  return (retval);
}



/************************************************************************/
/*!

  - Module Name:    clip_lat_lon

  - Programmer:     Jan C. Depner

  - Date Written:   September 2010

  - Purpose:        Calls clip (above) but handles an NV_F64_MBR
                    instead of an NV_F64_XYMBR as the MBR argument.

************************************************************************/

NV_INT32 clip_lat_lon (NV_FLOAT64 *x1, NV_FLOAT64 *y1, NV_FLOAT64 *x2, NV_FLOAT64 *y2, NV_F64_MBR mbr)
{
  NV_F64_XYMBR xymbr;

  xymbr.min_y = mbr.slat;
  xymbr.max_y = mbr.nlat;
  xymbr.min_x = mbr.wlon;
  xymbr.max_x = mbr.elon;

  return (clip (x1, y1, x2, y2, xymbr));
}
