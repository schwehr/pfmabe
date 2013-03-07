
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



#include "smooth_contour.hpp"

#define MIN_ENDPOINT_SEPARATION 0.001
#define SPLINE_POINTS   0
#define SPLINE_ENDPOINT 1



/***************************************************************************/
/*!

  - Function Name:      interpolate

  - Programmer(s):      David B. Johanson

  - Date Written:       April 3, 1998

  - Purpose:            Given the number of points to interpolate, the
                        number of the current spline segment, and b_spline
                        coefficients, interpolate the spline.

  - Arguments:
                        - num_interp   = number of points to interpolate (in)
                        - segment      = current spline segment (in)
                        - coef_x       = x array of spline coefficients
                        - coef_y       = y array of spline coefficients
                        - contour_x    = contour x array (out)
                        - contour_y    = contour y array (out)
                        - end
                            - SPLINE_POINTS: start, middle points
                            - SPLINE_ENDPOINT: endpoint only

  - Return Value:
                        - void

****************************************************************************/

static void interpolate (NV_INT32 num_interp, NV_INT32 segment, NV_FLOAT64 *coef_x, NV_FLOAT64 *coef_y, NV_FLOAT64 *contour_x, NV_FLOAT64 *contour_y,
                         NV_INT32 interp_section)
{
  NV_FLOAT64   increment, t, t2, t3;
  NV_INT32     i, index;


  increment = 1.0 / (NV_FLOAT32) num_interp;

  if (interp_section == SPLINE_ENDPOINT)
    {
      t = t2 = t3 = 1.0;

      index = segment * num_interp + num_interp;
      contour_x[index] = coef_x[0] * t3 + coef_x[1] * t2 + coef_x[2] * t + coef_x[3];
      contour_y[index] = coef_y[0] * t3 + coef_y[1] * t2 + coef_y[2] * t + coef_y[3];
    }
  else
    {
      t = 0.0;

      for (i = 0; i < num_interp; i++)
        {
          t2 = t * t;
          t3 = t * t2;
        
          index = segment * num_interp + i;
          contour_x[index] = coef_x[0] * t3 + coef_x[1] * t2 + coef_x[2] * t + coef_x[3];
          contour_y[index] = coef_y[0] * t3 + coef_y[1] * t2 + coef_y[2] * t + coef_y[3];

          t += increment;
        }
    }
}



/***************************************************************************/
/*!

  - Function Name:      smooth_contour

  - Programmer(s):      David B. Johanson

  - Date Written:       April 3, 1998

  - Purpose:            Given x and y contour arrays, return the same
                        arrays populated with x and y values representing
                        a smoothed contour.  It is assumed that the contour
                        arrays will be large enough to hold the smoothed
                        data (they must have (n-1) * i + 1 elements, where
                        n is the original number of points and i is the
                        number of points to interpolate per spline segment.

  - Arguments:
                        - num_interp   = number of points to interpolate (in)
                        - num_pts      = number of points (in/out)
                        - contour_x    = contour x array (in/out)
                        - contour_y    = contour y array (in/out)

  - Return Value:
                        - void

****************************************************************************/

void smooth_contour (NV_INT32 num_interp, NV_INT32 *num_pts, NV_FLOAT32 *contour_x, NV_FLOAT32 *contour_y)
{
  NV_FLOAT64 *x64;
  NV_FLOAT64 *y64;

  if ((x64 = (NV_FLOAT64 *) malloc (*num_pts * sizeof (NV_FLOAT64))) == NULL)
    {
      perror ("Allocating x64 in smooth_contour.cpp");
      exit (-1);
    }

  if ((y64 = (NV_FLOAT64 *) malloc (*num_pts * sizeof (NV_FLOAT64))) == NULL)
    {
      perror ("Allocating y64 in smooth_contour.cpp");
      exit (-1);
    }

  for (NV_INT32 i = 0 ; i < *num_pts ; i++)
    {
      x64[i] = (NV_FLOAT64) contour_x[i];
      y64[i] = (NV_FLOAT64) contour_y[i];
    }


  smooth_contour (num_interp, num_pts, x64, y64);


  for (NV_INT32 i = 0 ; i < *num_pts ; i++)
    {
      contour_x[i] = (NV_FLOAT32) x64[i];
      contour_y[i] = (NV_FLOAT32) y64[i];
    }

  free (x64);
  free (y64);
}



void smooth_contour (NV_INT32 num_interp, NV_INT32 *num_pts, NV_FLOAT64 *contour_x, NV_FLOAT64 *contour_y)
{
  NV_FLOAT64  *raw_x, *raw_y;
  NV_FLOAT64  x[4], y[4];
  NV_FLOAT64  coef_x[4], coef_y[4];
  NV_FLOAT64  dx, dy, dist;
  NV_INT32    segment;
    
  void b_spline(NV_FLOAT64 *, NV_FLOAT64 *, NV_FLOAT64 *, NV_FLOAT64 *);


  // allocate memory and make copies of the original contour data

  if ((raw_x = (NV_FLOAT64 *) malloc ((*num_pts + 2) * sizeof (NV_FLOAT64))) == NULL)
    {
      perror ("Allocating raw_x in smooth_contour.cpp");
      perror (__FILE__);
      exit (-1);
    }

  if ((raw_y = (NV_FLOAT64 *) malloc ((*num_pts + 2) * sizeof (NV_FLOAT64))) == NULL)
    {
      perror ("Allocating raw_y in smooth_contour.cpp");
      exit (-1);
    }

  memcpy (raw_x + 1, contour_x, *num_pts * sizeof (NV_FLOAT64));
  memcpy (raw_y + 1, contour_y, *num_pts * sizeof (NV_FLOAT64));


  // obtain the distance between the endpoints

  dx = raw_x[*num_pts] - raw_x[1];
  dy = raw_y[*num_pts] - raw_y[1];
  dist = sqrt (pow (dx, 2) + pow (dy, 2));


  // if contour closes or nearly closes on itself (endpoints closer than
  // MIN_ENDPOINT_SEPARATION), use real data from the other end of the 
  // contour to populate the endpoints, else linearly extrapolate to obtain 
  // first and last points

  if (dist < MIN_ENDPOINT_SEPARATION)
    {
      raw_x[0] = raw_x[*num_pts-1];
      raw_y[0] = raw_y[*num_pts-1];
      raw_x[*num_pts+1] = raw_x[2];
      raw_y[*num_pts+1] = raw_y[2];
    }
  else
    {
      raw_x[0] = (raw_x[1] - raw_x[2]) + raw_x[1];
      raw_y[0] = (raw_y[1] - raw_y[2]) + raw_y[1];
      raw_x[*num_pts+1] = (raw_x[*num_pts] - raw_x[*num_pts-1]) + raw_x[*num_pts];
      raw_y[*num_pts+1] = (raw_y[*num_pts] - raw_y[*num_pts-1]) + raw_y[*num_pts];
    }


  // loop once for each spline segment, interpolate and fill contour arrays

  for (segment=0; segment < *num_pts - 1; segment++)
    {
      x[0] = raw_x[0 + segment];
      x[1] = raw_x[1 + segment];
      x[2] = raw_x[2 + segment];
      x[3] = raw_x[3 + segment];
      y[0] = raw_y[0 + segment];
      y[1] = raw_y[1 + segment];
      y[2] = raw_y[2 + segment];
      y[3] = raw_y[3 + segment];

      b_spline (x, y, coef_x, coef_y);
      interpolate (num_interp, segment, coef_x, coef_y, contour_x, contour_y, SPLINE_POINTS);
    }

  segment = *num_pts - 2;
  interpolate (num_interp, segment, coef_x, coef_y, contour_x, contour_y, SPLINE_ENDPOINT);


  //  set the new number of points in the contour arrays (number of
  //  interpolated points times the number of intervals

  *num_pts = (num_interp * (*num_pts - 1)) + 1;

  free(raw_x);
  free(raw_y);
}
