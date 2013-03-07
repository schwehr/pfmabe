
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



#include "pfmEditDef.hpp"
#include <cmath>


/***************************************************************************/
/*!

  - Module Name:        compute_ortho_values

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 2008

  - Purpose:            Compute the ortho scale and values for the pseudo
                        distance function.  This is for the pseudo-3D views
                        (i.e. not plan view).

****************************************************************************/

void compute_ortho_values (nvMap *map, MISC *misc, OPTIONS *options, QScrollBar *sliceBar, NV_BOOL set_to_min)
{
  //  Get the area of the screen that is being used for map display.  We'll use this for
  //  the area to plot our "rotated" points.

  NV_FLOAT64 dummy;
  NV_INT32 rxmin_pixel, rymin_pixel, z;
  map->map_to_screen (1, &misc->displayed_area.min_x, &misc->displayed_area.min_y, &dummy, &rxmin_pixel, &rymin_pixel, &z);

  NV_INT32 rxmax_pixel, rymax_pixel;
  map->map_to_screen (1, &misc->displayed_area.max_x, &misc->displayed_area.max_y, &dummy, &rxmax_pixel, &rymax_pixel, &z);

  NV_FLOAT32 sin_val = misc->sin_array[misc->rotate_angle];
  NV_FLOAT32 cos_val = misc->cos_array[misc->rotate_angle];

  NV_FLOAT32 rotate_x_pixel[4], rotate_y_pixel[4];
  rotate_x_pixel[0] = rxmin_pixel * cos_val - rymin_pixel * sin_val;
  rotate_y_pixel[0] = rxmin_pixel * sin_val + rymin_pixel * cos_val;

  rotate_x_pixel[1] = rxmax_pixel * cos_val - rymax_pixel * sin_val;
  rotate_y_pixel[1] = rxmax_pixel * sin_val + rymax_pixel * cos_val;

  rotate_x_pixel[2] = rxmin_pixel * cos_val - rymax_pixel * sin_val;
  rotate_y_pixel[2] = rxmin_pixel * sin_val + rymax_pixel * cos_val;

  rotate_x_pixel[3] = rxmax_pixel * cos_val - rymin_pixel * sin_val;
  rotate_y_pixel[3] = rxmax_pixel * sin_val + rymin_pixel * cos_val;

  NV_INT32 min_x = NINT (rotate_x_pixel[0]);
  NV_INT32 max_x = NINT (rotate_x_pixel[0]);
  for (NV_INT32 i = 1 ; i < 4 ; i++)
    {
      min_x = qMin (min_x, NINT (rotate_x_pixel[i]));
      max_x = qMax (max_x, NINT (rotate_x_pixel[i]));
    }


  //  Add 1% to the size.

  NV_FLOAT32 range = (NV_FLOAT32) (max_x - min_x);
  min_x -= NINT ((range * 0.1) / 2.0);
  max_x += NINT ((range * 0.1) / 2.0);


  misc->rotate_offset = misc->map_border - min_x;
  misc->rotate_scale = (NV_FLOAT32) (misc->draw_area_width) / (NV_FLOAT32) (max_x - min_x);


  //  Compute orthogonal values for slicing.

  misc->ortho_angle = (misc->rotate_angle + 900) % 3600;

  sin_val = misc->sin_array[misc->ortho_angle];
  cos_val = misc->cos_array[misc->ortho_angle];

  rotate_x_pixel[0] = rxmin_pixel * cos_val - rymin_pixel * sin_val;
  rotate_y_pixel[0] = rxmin_pixel * sin_val + rymin_pixel * cos_val;

  rotate_x_pixel[1] = rxmax_pixel * cos_val - rymax_pixel * sin_val;
  rotate_y_pixel[1] = rxmax_pixel * sin_val + rymax_pixel * cos_val;

  rotate_x_pixel[2] = rxmin_pixel * cos_val - rymax_pixel * sin_val;
  rotate_y_pixel[2] = rxmin_pixel * sin_val + rymax_pixel * cos_val;

  rotate_x_pixel[3] = rxmax_pixel * cos_val - rymin_pixel * sin_val;
  rotate_y_pixel[3] = rxmax_pixel * sin_val + rymin_pixel * cos_val;

  min_x = NINT (rotate_x_pixel[0]);
  max_x = NINT (rotate_x_pixel[0]);
  for (NV_INT32 i = 1 ; i < 4 ; i++)
    {
      min_x = qMin (min_x, NINT (rotate_x_pixel[i]));
      max_x = qMax (max_x, NINT (rotate_x_pixel[i]));
    }

  misc->ortho_offset = misc->map_border - min_x;
  misc->ortho_scale = (NV_FLOAT32) (misc->draw_area_width) / (NV_FLOAT32) (max_x - min_x);


  //  Compute the ortho_min and ortho_max based on actual data points (because there probably aren't points at the
  //  corners.

  misc->ortho_min = 9999;
  misc->ortho_max = -9999;
  NV_INT32 min_point = 0, max_point = 0;

  for (NV_INT32 i = 0 ; i < misc->last_drawn_index ; i++)
    {
      //  Note that the slice check is turned off in the check_bounds call.

      if (!check_bounds (map, options, misc, i, NVTrue, NVFalse))
        {
          NV_INT32 z, pix_x, pix_y;
          scale_view (map, misc, misc->data[i].x, misc->data[i].y, misc->data[i].z, &pix_x, &pix_y, &z);

          if (z < misc->ortho_min)
            {
              misc->ortho_min = z;
              min_point = i;
            }

          if (z > misc->ortho_max)
            {
              misc->ortho_max = z;
              max_point = i;
            }
        }
    }


  //  Compute the distance from the front of the view to the back of the view to get the slice size in meters.

  NV_FLOAT64 dist, az;
  invgp (NV_A0, NV_B0, misc->data[min_point].y, misc->data[min_point].x, misc->data[max_point].y, misc->data[max_point].x, &dist, &az);
  misc->slice_meters = dist * ((NV_FLOAT32) options->slice_percent / 100.0);


  misc->ortho_range = misc->ortho_max - misc->ortho_min;

  misc->slice_size = NINT (misc->ortho_range * ((NV_FLOAT32) options->slice_percent / 100.0));

  sliceBar->setMinimum (misc->ortho_min);
  sliceBar->setMaximum (misc->ortho_max);
  sliceBar->setSingleStep (misc->slice_size);
  sliceBar->setPageStep (misc->slice_size);
  if (set_to_min) sliceBar->setValue (misc->ortho_min);
}
