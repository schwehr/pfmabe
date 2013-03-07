#include "geoSwath3D.hpp"

void get_nearest_kill_point (MISC *misc, POINT_DATA *data, NV_FLOAT64 lat, NV_FLOAT64 lon, NV_F64_COORD3 *hot)
{
  //  Return if we're not in kill point mode.

  if (!misc->filter_kill_count) return;


  NV_FLOAT64 min_dist = 9999999999.0;
  NV_INT32 kill_point = -1;

  for (NV_INT32 i = 0 ; i < misc->filter_kill_count ; i++)
    {
      NV_INT32 j = misc->filter_kill_list[i];


      NV_FLOAT64 dist = sqrt ((NV_FLOAT64) ((lat - data->y[j]) * (lat - data->y[j])) + (NV_FLOAT64) ((lon - data->x[j]) * (lon - data->x[j])));


      if (dist < min_dist)
        {
          min_dist = dist;
          hot->x = data->x[j];
          hot->y = data->y[j];
          hot->z = data->z[j];
          kill_point = j;
        }
    }


  misc->nearest_point = misc->nearest_stack.point[0] = kill_point;


  //  Force this point to not be replaced by compare_to_stack by setting it's distance to a negative.

  misc->nearest_stack.dist[0] = -999.0;


  return;
}
