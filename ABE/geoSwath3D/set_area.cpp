#include "geoSwath3D.hpp"

/***************************************************************************\
*                                                                           *
*   Module Name:        set_area                                            *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 1995                                       *
*                                                                           *
*   Purpose:            Invalidates, restores, in a rectangle or polygon.   *
*                                                                           *
*   Arguments:          x_bounds        -   array of x vertices (in pixels) *
*                       y_bounds        -   array of y vertices (in pixels) *
*                       num_vertices    -   number of vertices              *
*                       rect_flag       -   1 = rectangle, 0 = polygon      *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
*   Calling Routines:   ButtonReleaseEH                                     *
*                                                                           * 
\***************************************************************************/

void set_area (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 *x_bounds, NV_INT32 *y_bounds,
               NV_INT32 num_vertices, NV_INT32 rect_flag)
{
  NV_FLOAT64 *mx = NULL, *my = NULL;
  NV_BOOL undo_on = NVFalse;


  //  If it's a polygon we have to convert to NV_FLOAT64 so that the "inside" function will work.

  if (!rect_flag)
    {
      mx = (NV_FLOAT64 *) malloc (num_vertices * sizeof (NV_FLOAT64));

      if (mx == NULL)
        {
          perror ("Allocating mx array in set_area");
          exit (-1);
        }

      my = (NV_FLOAT64 *) malloc (num_vertices * sizeof (NV_FLOAT64));

      if (my == NULL)
        {
          perror ("Allocating my array in set_area");
          exit (-1);
        }

      for (NV_INT32 i = 0 ; i < num_vertices ; i++)
        {
          mx[i] = (NV_FLOAT64) x_bounds[i];
          my[i] = (NV_FLOAT64) y_bounds[i];
        }
    }


  //  Get the minimum bounding rectangle (X and Y in pixels).

  NV_INT32 min_y = 9999999;
  NV_INT32 max_y = -1;
  NV_INT32 min_x = 9999999;
  NV_INT32 max_x = -1;
  for (NV_INT32 j = 0 ; j < num_vertices ; j++)
    {
      min_y = qMin (min_y, y_bounds[j]);
      max_y = qMax (max_y, y_bounds[j]);
      min_x = qMin (min_x, x_bounds[j]);
      max_x = qMax (max_x, x_bounds[j]);
    }


  NV_INT32 px = -1, py = -1;


  //  CLEAR_POLYGON is a special case because we have to search the highlight list instead of all of the points.

  if (options->function == CLEAR_POLYGON)
    {
      for (NV_INT32 i = 0 ; i < misc->highlight_count ; i++)
        {
          NV_INT32 j = misc->highlight[i];


          //  Convert the X, Y, and Z value to a projected pixel position

          map->get2DCoords (data->x[j], data->y[j], -data->z[j], &px, &py);


          //  Mark the points that fall inside the polygon.

          if (inside_polygon2 (mx, my, num_vertices, (NV_FLOAT64) px, (NV_FLOAT64) py)) misc->highlight[i] = -1;
        }

      free (mx);
      free (my);


      //  Now reorder and pack the highlight array.

      NV_INT32 new_count = 0;
      for (NV_INT32 i = 0 ; i < misc->highlight_count ; i++)
        {
          if (misc->highlight[i] >= 0)
            {
              misc->highlight[new_count] = misc->highlight[i];
              new_count++;
            }
        }

        misc->highlight_count = new_count;

        misc->highlight = (NV_INT32 *) realloc (misc->highlight, (misc->highlight_count) * sizeof (NV_INT32));
        if (misc->highlight == NULL)
          {
            perror ("Allocating misc->highlight in set_area.cpp");
            exit (-1);
          }

        return;
    }


  //  Whether we're making an undo block.

  switch (options->function)
    {
    case RESTORE_RECTANGLE:
    case DELETE_RECTANGLE:
    case RESTORE_POLYGON:
    case DELETE_POLYGON:
      undo_on = NVTrue;
      break;

    default:
      undo_on = NVFalse;
    }


  for (NV_INT32 i = 0 ; i < data->count ; i++)
    {
      //  Check against the displayed minimum bounding rectangle.  This is all that is needed for 
      //  a rectangle and it cuts down on the computations for a polygon.

      if (!check_bounds (options, misc, data, i, NVFalse, misc->slice))
        {
          //  Convert the X, Y, and Z value to a projected pixel position

          map->get2DCoords (data->x[i], data->y[i], -data->z[i], &px, &py);


          //  Rectangle.

          if (rect_flag)
            {
              if (px >= min_x && px <= max_x && py >= min_y && py <= max_y)
                {
                  switch (options->function)
                    {
                    case RESTORE_RECTANGLE:
                      store_undo (misc, options->undo_levels, data->val[i], i);
                      data->val[i] &= ~PFM_MANUALLY_INVAL;
                      data->val[i] &= ~PFM_FILTER_INVAL;
                      break;

                    case DELETE_RECTANGLE:
                      store_undo (misc, options->undo_levels, data->val[i], i);
                      data->val[i] |= PFM_MANUALLY_INVAL;
                      break;

                    case MASK_INSIDE_RECTANGLE:
                      data->mask[i] = NVTrue;
                      break;

                    case RECT_FILTER_MASK:
                      data->fmask[i] = NVTrue;
                      break;
                    }
                }
              else
                {
                  switch (options->function)
                    {
                    case MASK_OUTSIDE_RECTANGLE:
                      data->mask[i] = NVTrue;
                      break;
                    }
                }
            }


          //  Polygon.
                    
          else
            {
              if (inside_polygon2 (mx, my, num_vertices, (NV_FLOAT64) px, (NV_FLOAT64) py))
                {
                  switch (options->function)
                    {
                    case RESTORE_POLYGON:
                      store_undo (misc, options->undo_levels, data->val[i], i);
                      data->val[i] &= ~PFM_MANUALLY_INVAL;
                      data->val[i] &= ~PFM_FILTER_INVAL;
                      break;

                    case DELETE_POLYGON:
                      store_undo (misc, options->undo_levels, data->val[i], i);
                      data->val[i] |= PFM_MANUALLY_INVAL;
                      break;

                    case MASK_INSIDE_POLYGON:
                      data->mask[i] = NVTrue;
                      break;

                    case POLY_FILTER_MASK:
                      data->fmask[i] = NVTrue;
                      break;

                    case HIGHLIGHT_POLYGON:
                      misc->highlight = (NV_INT32 *) realloc (misc->highlight, (misc->highlight_count + 1) * sizeof (NV_INT32));
                      if (misc->highlight == NULL)
                        {
                          perror ("Allocating misc->highlight in set_area.cpp");
                          exit (-1);
                        }

                      misc->highlight[misc->highlight_count] = i;
                      misc->highlight_count++;
                      break;
                    }
                }
              else
                {
                  switch (options->function)
                    {
                    case MASK_OUTSIDE_POLYGON:
                      data->mask[i] = NVTrue;
                      break;
                    }
                }
            }
        }
    }

  if (!rect_flag)
    {
      free (mx);
      free (my);
    }


  //  Close the undo block

  if (undo_on) end_undo_block (misc);
}
