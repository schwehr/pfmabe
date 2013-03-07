#include "geoSwath3DDef.hpp"
#include <cmath>


static NV_FLOAT64 edit_x_range, edit_y_range;


/***************************************************************************\
*                                                                           *
*   Module Name:        pseudo_dist_from_viewer                             *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 2008                                       *
*                                                                           *
*   Purpose:            Compute a pseudo distance from the viewer to be     *
*                       used when slicing the data.                         *
*                                                                           *
*   Arguments:          x               -   x value                         *
*                       y               -   y value                         *
*                       z               -   z value                         *
*                                                                           *
*   Return Value:       Integer pseudo distance                             *
*                                                                           *
\***************************************************************************/

NV_INT32 pseudo_dist_from_viewer (MISC *misc, POINT_DATA *data, NV_FLOAT64 x, NV_FLOAT64 y)
{
  NV_FLOAT64 xyz_y = ((y - data->bounds.min_y) / edit_y_range) * misc->draw_area_height;
  NV_FLOAT64 xyz_x = ((x - data->bounds.min_x) / edit_x_range) * misc->draw_area_width;


  xyz_x = xyz_x * misc->cos_array[misc->ortho_angle] - xyz_y * misc->sin_array[misc->ortho_angle];

  return (NINT (xyz_x * (NV_FLOAT64) misc->ortho_scale));
}



/***************************************************************************\
*                                                                           *
*   Module Name:        compute_ortho_values                                *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 2008                                       *
*                                                                           *
*   Purpose:            Compute the ortho scale and values for the pseudo   *
*                       distance function.                                  *
*                                                                           *
\***************************************************************************/

void compute_ortho_values (nvMapGL *map, MISC *misc, OPTIONS *options, POINT_DATA *data, QScrollBar *sliceBar, NV_BOOL set_to_min)
{
  //  Turn off slice prior to checking bounds.

  NV_BOOL save_slice = misc->slice;
  misc->slice = NVFalse;


  //  Turn off depth testing in the map widget so that we can see through the data outside the slice
  //  from any direction.

  map->setDepthTest (NVFalse);


  //  Save some computation when calling pseudo_dist_from_viewer.

  edit_x_range = data->bounds.max_x - data->bounds.min_x;
  edit_y_range = data->bounds.max_y - data->bounds.min_y;


  //  I have to add 90 degrees here because of the way I'm rotating Y in OpenGL as opposed to
  //  the way I was rotating in the 2D editor.

  NV_FLOAT64 angle = fmod (map->getYRotation () + 90.0 + 360.0, 360.0);

  misc->rotate_angle = NINT (angle * 10.0);


  //  Phony pixel values (as if we were viewing from the side).

  NV_INT32 rxmin_value = 0;
  NV_INT32 rymin_value = misc->draw_area_height;
  NV_INT32 rxmax_value = misc->draw_area_width;
  NV_INT32 rymax_value = 0;


  //  Compute orthogonal values for slicing.  This is the min and max pseudo distance from viewer for the
  //  four corner points (used to compute the ortho_scale).

  misc->ortho_angle = (misc->rotate_angle + 1800) % 3600;

  NV_FLOAT32 sin_val = misc->sin_array[misc->ortho_angle];
  NV_FLOAT32 cos_val = misc->cos_array[misc->ortho_angle];


  NV_FLOAT64 rotate_x_value[4], rotate_y_value[4];
            
  rotate_x_value[0] = rxmin_value * cos_val - rymin_value * sin_val;
  rotate_y_value[0] = rxmin_value * sin_val + rymin_value * cos_val;

  rotate_x_value[1] = rxmax_value * cos_val - rymax_value * sin_val;
  rotate_y_value[1] = rxmax_value * sin_val + rymax_value * cos_val;

  rotate_x_value[2] = rxmin_value * cos_val - rymax_value * sin_val;
  rotate_y_value[2] = rxmin_value * sin_val + rymax_value * cos_val;

  rotate_x_value[3] = rxmax_value * cos_val - rymin_value * sin_val;
  rotate_y_value[3] = rxmax_value * sin_val + rymin_value * cos_val;

  NV_INT32 min_x = NINT (rotate_x_value[0]);
  NV_INT32 max_x = NINT (rotate_x_value[0]);
  for (NV_INT32 i = 1 ; i < 4 ; i++)
    {
      min_x = qMin (min_x, NINT (rotate_x_value[i]));
      max_x = qMax (max_x, NINT (rotate_x_value[i]));
    }
  misc->ortho_scale = (NV_FLOAT32) (misc->draw_area_width) / (NV_FLOAT32) (max_x - min_x);


  //  Compute the ortho_min and ortho_max based on actual data points (because there probably aren't points at the
  //  corners.

  misc->ortho_min = 9999;
  misc->ortho_max = -9999;
  NV_INT32 min_point = 0, max_point = 0;

  for (NV_INT32 i = 0 ; i < data->count ; i++)
    {
      if (!check_bounds (options, misc, data, i, NVTrue, misc->slice))
        {
          NV_INT32 z = pseudo_dist_from_viewer (misc, data, data->x[i], data->y[i]);


          //  Note the flipping of max_point and min_point.  This is because Z is inverted (for us hydrographers) in OpenGL.

          if (z < misc->ortho_min)
            {
              misc->ortho_min = z;
              max_point = i;
            }

          if (z > misc->ortho_max)
            {
              misc->ortho_max = z;
              min_point = i;
            }
        }
    }


  //  Compute the distance from the front of the view to the back of the view to get the slice size in meters.

  NV_FLOAT64 dist, az;
  invgp (NV_A0, NV_B0, data->y[min_point], data->x[min_point], data->y[max_point], data->x[max_point], &dist, &az);
  misc->slice_meters = dist * ((NV_FLOAT32) options->slice_percent / 100.0);


  misc->ortho_range = misc->ortho_max - misc->ortho_min;

  misc->slice_size = NINT (misc->ortho_range * ((NV_FLOAT32) options->slice_percent / 100.0));

  sliceBar->setMinimum (misc->ortho_min);
  sliceBar->setMaximum (misc->ortho_max);
  sliceBar->setSingleStep (misc->slice_size);
  sliceBar->setPageStep (misc->slice_size);
  if (set_to_min)
    {
      misc->slice_min = misc->ortho_min;
      misc->slice_max = misc->slice_min + misc->slice_size;
      sliceBar->setValue (misc->ortho_min);
    }

  misc->slice = save_slice;

  if (!misc->slice) map->setDepthTest (NVTrue);
}
