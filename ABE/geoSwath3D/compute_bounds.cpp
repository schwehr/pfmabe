#include "geoSwath3D.hpp"

NV_BOOL compute_bounds (nvMapGL *map __attribute__ ((unused)), OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 *viewable_count, NV_F64_XYMBC *bounds)
{
  static NV_F64_XYMBC prev_bounds = {-999.0, -999.0, -999.0, -999.0, CHRTRNULL, -CHRTRNULL};


  //  Compute the Z range of the data.

  *viewable_count = 0;


  //  Compute the min and max values for defining the 3D space and optionally for coloring.

  for (NV_INT32 i = 0 ; i < data->count ;  i++)
    {
      //  Note that we NEVER check for slicing here because we want the range to be the full range when we are slicing.
      //  This option to check_bounds turns off slice check--\/

      if (!check_bounds (options, misc, data, i, NVTrue, NVFalse))
        {
          (*viewable_count)++;

          bounds->min_x = qMin (bounds->min_x, data->x[i]);
          bounds->max_x = qMax (bounds->max_x, data->x[i]);

          bounds->min_y = qMin (bounds->min_y, data->y[i]);
          bounds->max_y = qMax (bounds->max_y, data->y[i]);


          bounds->min_z = qMin (bounds->min_z, (NV_FLOAT64) -data->z[i]);
          bounds->max_z = qMax (bounds->max_z, (NV_FLOAT64) -data->z[i]);
        }
    }


  /*  For later.

  NV_FLOAT32 win_size = options->min_window_size;
  switch (options->iho_min_window)
    {
    case 0:
      win_size = options->min_window_size;
      break;

    case 1:
      win_size = 0.25 + 0.0075 * bounds->min_z;
      break;

    case 2:
      win_size = 0.5 + 0.013 * bounds->min_z;
      break;

    case 3:
      win_size = 1.0 + 0.023 * bounds->min_z;
      break;
    }
  map->setMinZExtents (win_size);
  */


  if (bounds->min_x != prev_bounds.min_x || bounds->min_y != prev_bounds.min_y || bounds->min_z != prev_bounds.min_z ||
      bounds->max_x != prev_bounds.max_x || bounds->max_y != prev_bounds.max_y || bounds->max_z != prev_bounds.max_z)
    {
      prev_bounds = *bounds;

      return (NVTrue);
    }

  return (NVFalse);
}
