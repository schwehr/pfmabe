#include "geoSwath3DDef.hpp"

/***************************************************************************\
*                                                                           *
*   Module Name:        overlayData                                         *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       August 2009                                         *
*                                                                           *
*   Purpose:            Draw data marker locations in 2D over the 3D data.  *
*                                                                           *
*   Arguments:          None                                                *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
*   Calling Routines:   utility routine                                     *
*                                                                           * 
\***************************************************************************/

void overlayData (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data)
{
  //  Show the filter masked points if any are present.

  if (misc->filter_mask)
    {
      QColor mask = options->edit_color;
      mask.setAlpha (64);

      for (NV_INT32 i = 0 ; i < data->count ; i++)
        {
          if (!check_bounds (options, misc, data, i, NVTrue, misc->slice))
            {
              if (data->fmask[i])
                {
                  NV_INT32 pix_x, pix_y;
                  map->get2DCoords (data->x[i], data->y[i], -data->z[i], &pix_x, &pix_y);

                  map->drawLine (pix_x - 3, pix_y - 3, pix_x + 3, pix_y - 3, mask, 2, Qt::SolidLine, NVFalse);
                  map->drawLine (pix_x + 3, pix_y - 3, pix_x + 3, pix_y + 3, mask, 2, Qt::SolidLine, NVFalse);
                  map->drawLine (pix_x + 3, pix_y + 3, pix_x - 3, pix_y + 3, mask, 2, Qt::SolidLine, NVFalse);
                  map->drawLine (pix_x - 3, pix_y + 3, pix_x - 3, pix_y - 3, mask, 2, Qt::SolidLine, NVFalse);
                }
            }
        }
      map->drawLine (0, 0, 0, 0, options->background_color, 0, Qt::SolidLine, NVTrue);
    }
}
