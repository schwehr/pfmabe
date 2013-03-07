#include "geoSwath3DDef.hpp"

/***************************************************************************\
*                                                                           *
*   Module Name:        overlay_flag                                        *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 1995                                       *
*                                                                           *
*   Purpose:            Draw flag locations in the GC and the pixmap.       *
*                                                                           *
*   Arguments:          None                                                *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
*   Calling Routines:   utility routine                                     *
*                                                                           * 
\***************************************************************************/

void overlayFlag (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data)
{
  NV_FLOAT64          wlon, elon, slat, nlat;
  QColor              tmp;



  slat = misc->abe_share->edit_area.min_y;
  nlat = misc->abe_share->edit_area.max_y;
  wlon = misc->abe_share->edit_area.min_x;
  elon = misc->abe_share->edit_area.max_x;


  //  Clear the marker points.

  map->clearMarkerPoints ();


  //  We can only display one group of markers (it gets too confusing to look at otherwise).  These are the filter kill points.

  if (misc->filter_kill_count)
    {
      //  Turn off highlighted points.

      if (misc->highlight_count)
        {
          free (misc->highlight);
          misc->highlight = NULL;
          misc->highlight_count = 0;
        }


      for (NV_INT32 i = 0 ; i < misc->filter_kill_count ; i++)
        {
          NV_INT32 j = misc->filter_kill_list[i];

          if (!check_bounds (options, misc, data, j, NVTrue, misc->slice))
            map->setMarkerPoints (data->x[j], data->y[j], -data->z[j], options->edit_color, 0.01, NVFalse);
        }
      map->setMarkerPoints (0.0, 0.0, 0.0, Qt::black, 0, NVTrue);
    }


  //  These are the data type or PFM_USER flagged points.

  else if (options->flag_index)
    {
      //  Turn off highlighted points.

      if (misc->highlight_count)
        {
          free (misc->highlight);
          misc->highlight = NULL;
          misc->highlight_count = 0;
        }


      NV_U_INT32 check = 0;


      switch (options->flag_index)
        {
        case 1:
          check = PFM_SUSPECT;
          break;

        case 2:
          check = PFM_INVAL;
          break;
        }

      for (NV_INT32 i = 0 ; i < data->count ; i++)
        {
          if (!check_bounds (options, misc, data, i, NVTrue, misc->slice))
            {
              if (data->val[i] & check)
                map->setMarkerPoints (data->x[i], data->y[i], -data->z[i], options->edit_color, 0.01, NVFalse);
            }
        }
      map->setMarkerPoints (0.0, 0.0, 0.0, Qt::black, 0, NVTrue);
    }


  //  These are highlight points

  else
    {
      for (NV_INT32 i = 0 ; i < misc->highlight_count ; i++)
        {
          NV_INT32 j = misc->highlight[i];

          if (!check_bounds (options, misc, data, j, NVTrue, misc->slice))
            map->setMarkerPoints (data->x[j], data->y[j], -data->z[j], options->edit_color, 0.01, NVFalse);
        }
      map->setMarkerPoints (0.0, 0.0, 0.0, Qt::black, 0, NVTrue);
    }
}
