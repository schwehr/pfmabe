
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



#include "geo_distance.h"

/***************************************************************************/
/*!

  - Module Name:        geo_distance

  - Programmer(s):      Jan C. Depner

  - Date Written:       October 2007

  - Purpose:            Estimate geographic distance by breaking an area
                       into bins and using the actual longitudinal bin
                       sizes at each latitude bin boundary.

  - Method:             What we do is pre-compute the X bin sizes when we
                       call init_geo_distance.  Then we compute the X bin
                       size at each of the two positions and assume that
                       the X space is uniform having an X bin size of the
                       average of these two sizes.  This gives a
                       reasonable approximation of distance.  For example,
                       at 30 degrees north, with a 2 meter bin size,
                       computations of distances up to 10,000 meters yield
                       differences between this and the results from invgp
                       of less than 0.0001 meters.  At 60 degrees north we
                       still get differences of less than 0.0001 meters.
                       At 60 degrees north, with a bin size of 10 meters,
                       we get a difference of 0.06 meters when measuring a
                       distance of 50,000 meters.  This method of
                       computing distance is about 8 times faster than
                       using invgp.  A test run of 500,000,000 distance
                       computations using invgp took 12 minutes and 1
                       second while the same test took 1 minute and 28
                       seconds using geo_distance.

  - Arguments:
                       - lat0            =   latitude of first point
                       - lon0            =   longitude of first point
                       - lat1            =   latitude of second point
                       - lon1            =   longitude of first point
                       - *distance       =   distance between the points

  - Return Value:      NVTrue on success, NVFalse on failure

****************************************************************************/

static NV_FLOAT64               *geo_dist;
static NV_FLOAT64               *geo_post;
static NV_FLOAT64               l_min_x, l_min_y, l_max_x, l_max_y, x_bin_size_degrees, y_bin_size_degrees, l_bin_size_meters;
static NV_BOOL                  geo_dist_init;


void init_geo_distance (NV_FLOAT64 bin_size_meters, NV_FLOAT64 min_x, NV_FLOAT64 min_y, NV_FLOAT64 max_x, NV_FLOAT64 max_y)
{
  NV_INT32                i, height;
  NV_FLOAT64              az, x_dist[2], x[2], y[2], mid_lat;


  l_min_x = min_x;
  l_min_y = min_y;
  l_max_x = max_x;
  l_max_y = max_y;
  l_bin_size_meters = bin_size_meters;


  invgp (NV_A0, NV_B0, min_y, min_x, max_y, min_x, &x_dist[0], &az);

  height = NINT (x_dist[0] / bin_size_meters);

  y_bin_size_degrees = (max_y - min_y) / (NV_FLOAT64) height;

  mid_lat = min_y + (max_y - min_y) / 2.0;

  newgp (mid_lat, min_x, 90.0, bin_size_meters, &y[0], &x[0]);

  x_bin_size_degrees = x[0] - min_x;


  geo_dist = (NV_FLOAT64 *) calloc (height + 2, sizeof (NV_FLOAT64));
  if (geo_dist == NULL) 
    {
      perror ("Allocating geo_dist array in init_geo_distance");
      exit (-1);
    }

  geo_post = (NV_FLOAT64 *) calloc (height + 2, sizeof (NV_FLOAT64));
  if (geo_post == NULL)
    {
      perror ("Allocating geo_dist array in init_geo_distance");
      exit (-1);
    }


  /*  Get the incremental distances.  Go one extra to cover points on the upper boundary.  */

  for (i = 0 ; i <= height + 1 ; i++)
    {
      /*  Get the latitude "post" positions.  */

      geo_post[i] = min_y + (NV_FLOAT64) i * y_bin_size_degrees;


      /*  Compute the actual X bin size at this lat band.  */

      invgp (NV_A0, NV_B0, geo_post[i], min_x, geo_post[i], min_x + x_bin_size_degrees, &geo_dist[i], &az);
    }

  geo_dist_init = NVTrue;
}



NV_BOOL geo_distance (NV_FLOAT64 lat0, NV_FLOAT64 lon0, NV_FLOAT64 lat1, NV_FLOAT64 lon1, NV_FLOAT64 *distance)
{
  NV_FLOAT64              x_dist[2], x[2], y[2], x_bin_size, next_lat;
  NV_I32_COORD2           coord[2];


  if (!geo_dist_init) return (NVFalse);


  /*  Check the points.  */

  if (lon0 > l_max_x || lon0 < l_min_x || lat0 > l_max_y || lat0 < l_min_y || lon1 > l_max_x || lon1 < l_min_x || lat1 > l_max_y || lat1 < l_min_y)
    return (NVFalse);


  /*  Compute our own indices so we can deal with round-off.  */

  coord[0].x = (NV_INT32) ((NV_FLOAT64) (lon0 - l_min_x) / (NV_FLOAT64) x_bin_size_degrees + 0.05);
  coord[0].y = (NV_INT32) ((NV_FLOAT64) (lat0 - l_min_y) / (NV_FLOAT64) y_bin_size_degrees + 0.05);

  coord[1].x = (NV_INT32) ((NV_FLOAT64) (lon1 - l_min_x) / (NV_FLOAT64) x_bin_size_degrees + 0.05);
  coord[1].y = (NV_INT32) ((NV_FLOAT64) (lat1 - l_min_y) / (NV_FLOAT64) y_bin_size_degrees + 0.05);


  /*  Get the Y positions in "meters".  */

  y[0] = ((NV_FLOAT64) coord[0].y + (lat0 - geo_post[coord[0].y]) / y_bin_size_degrees) * l_bin_size_meters;
  y[1] = ((NV_FLOAT64) coord[1].y + (lat1 - geo_post[coord[1].y]) / y_bin_size_degrees) * l_bin_size_meters;


  /*  Get the X positions in "meters" adjusted for the change in Y.  Interpolating the value between the 
      posts on either side of the lat.  This is probably serious overkill but not too computationally
      taxing.  */

  next_lat = geo_post[coord[0].y] + y_bin_size_degrees;
  x_dist[0] = geo_dist[coord[0].y] + (geo_dist[coord[0].y + 1] - geo_dist[coord[0].y]) * ((lat0 - geo_post[coord[0].y]) / (next_lat - geo_post[coord[0].y]));

  next_lat = geo_post[coord[1].y] + y_bin_size_degrees;
  x_dist[1] = geo_dist[coord[1].y] + (geo_dist[coord[1].y + 1] - geo_dist[coord[1].y]) * ((lat1 - geo_post[coord[1].y]) / (next_lat - geo_post[coord[1].y]));


  x_bin_size = (x_dist[1] + x_dist[0]) / 2.0;


  x[0] = ((lon0 - l_min_x) / x_bin_size_degrees) * x_bin_size;
  x[1] = ((lon1 - l_min_x) / x_bin_size_degrees) * x_bin_size;


  /*  Damn, this looks familiar doesn't it?  I wonder what it is?  */

  *distance = sqrt ((x[1] - x[0]) * (x[1] - x[0]) + (y[1] - y[0]) * (y[1] - y[0]));


  return (NVTrue);
}



void clean_geo_distance ()
{
  if (!geo_dist_init) return;

  free (geo_dist);
  free (geo_post);

  geo_dist_init = NVFalse;
}

