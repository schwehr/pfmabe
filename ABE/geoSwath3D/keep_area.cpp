#include "geoSwath3D.hpp"

/***************************************************************************\
*                                                                           *
*   Module Name:        keep_area                                           *
*                                                                           *
*   Programmer(s):      Rebecca Martinolich                                 *
*                                                                           *
*   Date Written:       October 1998                                        *
*                                                                           *
*   Purpose:            Invalidates data outside a polygon.                 *
*                                                                           *
*   Arguments:          x_bounds        -   array of x vertices             *
*                       y_bounds        -   array of y vertices             *
*                       num_vertices    -   number of vertices              *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
*   Calling Routines:   ButtonReleaseEH                                     *
*                                                                           * 
\***************************************************************************/

void keep_area (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 *x_bounds, NV_INT32 *y_bounds,
                NV_INT32 num_vertices)
{

  //  We have to convert to NV_FLOAT64 so that the "inside" function will work.

  NV_FLOAT64 *mx = (NV_FLOAT64 *) malloc (num_vertices * sizeof (NV_FLOAT64));

  if (mx == NULL)
    {
      perror ("Allocating mx array in keep_area");
      exit (-1);
    }

  NV_FLOAT64 *my = (NV_FLOAT64 *) malloc (num_vertices * sizeof (NV_FLOAT64));

  if (my == NULL)
    {
      perror ("Allocating my array in keep_area");
      exit (-1);
    }

  for (NV_INT32 i = 0 ; i < num_vertices ; i++)
    {
      mx[i] = (NV_FLOAT64) x_bounds[i];
      my[i] = (NV_FLOAT64) y_bounds[i];
    }


  NV_INT32 px = -1, py = -1;

  for (NV_INT32 i = 0 ; i < data->count ; i++)
    {
      //  Check against the rectangular or polygonal area that is displayed.

      if (!check_bounds (options, misc, data, i, NVFalse, misc->slice))
        {
          //  Convert the X, Y, and Z value to a projected pixel position

          map->get2DCoords (data->x[i], data->y[i], -data->z[i], &px, &py);


          //  Now check the point against the polygon.

          if (!inside_polygon2 (mx, my, num_vertices, (NV_FLOAT64) px, (NV_FLOAT64) py))
            {
              //  Save the undo information.

              store_undo (misc, options->undo_levels, data->val[i], i);


              data->val[i] |= PFM_MANUALLY_INVAL;
            }
        }
    }

  free (mx);
  free (my);


  //  Close the undo block.

  end_undo_block (misc);
}
