
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

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



/***************************************************************************/
/*!

  - Programmer(s):      Dominic Avery, Jan C. Depner

  - Date Written:       April 1992

  - Module Name:        interpolate

  - Purpose:            Performs bicubic spline interpolation for an input
                        data array.

  - Inputs:
                        - interval        =   desired interpolation interval
                        - length_x        =   length of the input x array
                        - start_xinterp   =   starting position in x of the
                                              output array
                        - end_xinterp     =   ending position in x of the
                                              output array
                        - length_xinterp  =   length of the output array
                        - x               =   input x data
                        - y               =   input y data
                        - x_interp        =   interpolated x output data
                        - y_interp        =   interpolated y output data

  - Outputs:            None

  - Calling Routines:   misps

  - Glossary:
                        - num_segments    =   number of 50 point segments
                        - count           =   counter
                        - length2         =   length of x in intervals
                        - endloop         =   end of loop flag
                        - index           =   utility integer
                        - seg_points      =   number of points in the 50
                                              point segments
                        - flag1           =   utility flag
                        - flag2           =   utility flag
                        - last_x          =   end of segment
                        - first_x         =   beginning of segment
                        - spline_length   =   length of data sent to spline
                        - current_x       =   position of data sent to spline
                        - new_last_x      =   last_x for next segment
                        - new_first_x     =   first_x for next segment
                        - ater            =
                        - x_pos           =   x value for spline
                        - y_pos           =   y value from spline

  - Method:             Breaks up input data into 50 point pieces prior
                        to sending it to the bicubic spline function.

****************************************************************************/

#include <stdio.h>
#include <cmath>
#include "nvtypes.h"

void interpolate (NV_FLOAT32 interval, NV_INT32 length_x, NV_FLOAT32 start_xinterp, NV_FLOAT32 end_xinterp, 
                  NV_INT32 *length_xinterp, NV_FLOAT32 *x, NV_FLOAT32 *y, NV_FLOAT32 *x_interp, 
                  NV_FLOAT32 *y_interp)
{
  NV_INT32   length2, new_last_x, new_first_x;
  NV_FLOAT32 y_pos;   


  void spline (NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, NV_FLOAT32, NV_FLOAT32 *, NV_FLOAT32 *);


  //  Compute length of interpolated data.

  *length_xinterp = (NV_INT32) (fabs ((NV_FLOAT64) ((end_xinterp - start_xinterp) / interval)) + 1.0);


  //  Compute number of segments.
    
  NV_INT32 num_segments = length_x / 50;
  NV_INT32 seg_points = num_segments * 50;
  NV_BOOL flag1 = NVFalse;
  NV_BOOL flag2 = NVFalse;
  NV_INT32 last_x = 48;
  if (seg_points == 0) last_x = length_x - 1;
  if ((length_x - seg_points) >= 2) flag2 = NVTrue;


  //  Overlap interpolation intervals by 2 input points so spline routine is dimensioned to max of 53 points.
    
  NV_INT32 count = 1;
  NV_INT32 first_x = 0;
  NV_INT32 spline_length = last_x + 1;
  if (seg_points == 0) spline_length--;
  NV_INT32 current_x = 0;
  NV_INT32 endloop = 0;

  while (!endloop)
    {
      endloop = 1;


      //  Break up the data into segments.
        
      length2 = (NV_INT32) (fabs ((NV_FLOAT64) ((*(x + last_x) - start_xinterp) / interval)) + 1.0);
      if (last_x == (length_x - 1)) length2 = *length_xinterp;
      NV_FLOAT32 ater = 9999.999;


      //  Figure the length of the segment and whether it is the last or not.

      for (NV_INT32 index = first_x; index <= length2 - 1; index++)
        {
          NV_FLOAT32 x_pos = (NV_FLOAT32) index * interval + start_xinterp;

            if ((x_pos > *(x + last_x)) && (last_x != (length_x - 1)))
            {
              length2 = 0;
              count++;
              if (seg_points != 0)
                {
                  if (count <= num_segments)
                    {
                      new_last_x = count * 50 - 1;
                      new_first_x = last_x - 1;
                      current_x = new_first_x;
                      last_x += 50;
                      if (flag1) last_x = length_x - 1;
                      first_x = length2;
                      spline_length = new_last_x - new_first_x;
                      endloop = 0;
                      break;
                    }
                  if (flag2)
                    {
                      flag1 = NVTrue;
                      flag2 = NVFalse;
                      new_last_x = length_x - 1;
                      new_first_x = last_x - 1;
                      current_x = new_first_x;
                      last_x += 50;
                      if (flag1) last_x = length_x - 1;
                      first_x = length2;
                      spline_length = new_last_x - new_first_x;
                      endloop = 0;
                      break;
                    }
                }
              *length_xinterp = length2;
              return;
            }
            if (x_pos > end_xinterp)
              {
                *length_xinterp = index - 1;
                return;
              }


            //  Spline it.

            spline ((x + current_x), (y + current_x), spline_length, x_pos, &y_pos, &ater);

            *(x_interp + index) = x_pos;
            *(y_interp + index) = y_pos;
        }

      if (endloop)
        {
          count++;
          if (seg_points != 0)
            {
              if (count <= num_segments)
                {
                  new_last_x = count * 50 - 1;
                  new_first_x = last_x - 1;
                  current_x = new_first_x;
                  last_x += 50;
                  if (flag1) last_x = length_x - 1;
                  first_x = length2;
                  spline_length = new_last_x - new_first_x;
                  endloop = 0;
                }
              else if (flag2)
                {
                  flag1 = NVTrue;
                  flag2 = NVFalse;
                  new_last_x = length_x - 1;
                  new_first_x = last_x - 1;
                  current_x = new_first_x;
                  last_x += 50;
                  if (flag1) last_x = length_x - 1;
                  first_x = length2;
                  spline_length = new_last_x - new_first_x;
                  endloop = 0;
                }
            }
        }
    }
  *length_xinterp = length2;

  return;
}
