#include "geoSwath3D.hpp"

/***************************************************************************\
*                                                                           *
*   Module Name:        get_bounds                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 1995                                       *
*                                                                           *
*   Purpose:            Computes the boundaries of the section (page)       *
*                       based on the data in memory.                        *
*                                                                           *
*   Arguments:          None                                                *
*                                                                           *
*   Return Value:       0 if no non-null points found, else count           *
*                                                                           *
\***************************************************************************/

NV_INT32 get_bounds (POINT_DATA *data, MISC *misc)
{
  //  Define the total display area.

  data->bounds.min_x = 999999999.0;
  data->bounds.max_x = -999999999.0;
  data->bounds.min_y = 999999999.0;
  data->bounds.max_y = -999999999.0;

  NV_FLOAT64 sum_x = 0.0, sum_y = 0.0;
  NV_INT32 count = 0, sum_count = 0;


  //  Loop through the data in memory looking for min and max.
    
  for (NV_INT32 i = 0 ; i < data->count ; i++)
    {
      //  Always use invalid data for the X and Y extents otherwise you can't turn it back on when you need it.

      data->bounds.min_x = qMin (data->x[i], data->bounds.min_x);
      data->bounds.max_x = qMax (data->x[i], data->bounds.max_x);
      data->bounds.min_y = qMin (data->y[i], data->bounds.min_y);
      data->bounds.max_y = qMax (data->y[i], data->bounds.max_y);

      switch (misc->data_type)
        {
        case PFM_GSF_DATA:
          if (i && data->rec[i] == data->rec[i - 1] && abs (data->sub[i] - data->sub[i - 1]) < 3)
            {
              sum_x += fabs (data->x[i] - data->x[i - 1]);
              sum_y += fabs (data->y[i] - data->y[i - 1]);
              sum_count++;
            }
          break;

        case PFM_CHARTS_HOF_DATA:
        case PFM_SHOALS_TOF_DATA:
        case PFM_WLF_DATA:
        case PFM_HAWKEYE_HYDRO_DATA:
        case PFM_HAWKEYE_TOPO_DATA:
          if (i && abs (data->rec[i] - data->rec[i - 1]) < 3)
            {
              sum_x += fabs (data->x[i] - data->x[i - 1]);
              sum_y += fabs (data->y[i] - data->y[i - 1]);
              sum_count++;
            }
          break;
        }

      count ++;
    }


  if (count)
    {
      /*  Add 5% to the window size.  */
        
      NV_FLOAT64 width_5 = (data->bounds.max_x - data->bounds.min_x) * 0.05;
      NV_FLOAT64 height_5 = (data->bounds.max_y - data->bounds.min_y) * 0.05;
      data->bounds.min_y -= height_5;
      data->bounds.max_y += height_5;
      data->bounds.min_x -= width_5;
      data->bounds.max_x += width_5;


      //  The grid size is based on 2 times the average distance between points that are within one or two points of each other.

      data->x_grid_size = (sum_x / (NV_FLOAT64) sum_count) * 2.0;
      data->y_grid_size = (sum_y / (NV_FLOAT64) sum_count) * 2.0;
    }

  return (count);
}
