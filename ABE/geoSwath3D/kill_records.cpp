#include "geoSwath3D.hpp"

/***************************************************************************\
*                                                                           *
*   Module Name:        kill_records                                        *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 2009                                       *
*                                                                           *
*   Purpose:            Invalidates records (pings) that are contained      *
*                       within a rectangle or polygon.  Unlike set_area,    *
*                       this function determines which records (pings) are  *
*                       contained within the rectangle or polygon and then  *
*                       invalidates all visible records (pings) in the      *
*                       displayed data.  Even those outside of the rectangle*
*                       or polygon.                                         *
*                                                                           *
*   Arguments:          map             -   nvmap object to draw on         *
*                       options         -   OPTIONS structure               *
*                       misc            -   MISC structure                  *
*                       data            -   point data used to define bounds*
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
\***************************************************************************/

void kill_records (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 *rb, NV_INT32 x, NV_INT32 y)
{
  NV_INT32        count = 0, record_count = 0, *px, *py, min_x = 0, max_x = 0, min_y = 0, max_y = 0;
  NV_U_INT32      *records = NULL;
  NV_FLOAT64      *mx = NULL, *my = NULL;
  NV_BOOL         scan = NVFalse, hit = NVFalse;


  //  First, get the area to be scanned based on the rectangle or polygon.

  switch (options->function)
    {
    case DELETE_RECTANGLE:
      if (map->rubberbandRectangleIsActive (*rb))
        {
          map->closeRubberbandRectangle (*rb, x, y, &px, &py);

          map->discardRubberbandRectangle (rb);
        }
      count = 4;


      //  Get the minimum bounding rectangle (X and Y in pixels).

      min_y = 9999999;
      max_y = -1;
      min_x = 9999999;
      max_x = -1;
      for (NV_INT32 j = 0 ; j < count ; j++)
        {
          min_y = qMin (min_y, py[j]);
          max_y = qMax (max_y, py[j]);
          min_x = qMin (min_x, px[j]);
          max_x = qMax (max_x, px[j]);
        }

      break;

    case DELETE_POLYGON:

      if (map->rubberbandPolygonIsActive (*rb))
        {
          map->closeRubberbandPolygon (*rb, x, y, &count, &px, &py);


          //  If it's a polygon we have to convert to NV_FLOAT64 so that the "inside" function will work.

          mx = (NV_FLOAT64 *) malloc (count * sizeof (NV_FLOAT64));

          if (mx == NULL)
            {
              perror ("Allocating mx array in set_area");
              exit (-1);
            }

          my = (NV_FLOAT64 *) malloc (count * sizeof (NV_FLOAT64));

          if (my == NULL)
            {
              perror ("Allocating my array in set_area");
              exit (-1);
            }

          for (NV_INT32 i = 0 ; i < count ; i++)
            {
              mx[i] = (NV_FLOAT64) px[i];
              my[i] = (NV_FLOAT64) py[i];
            }

          map->discardRubberbandPolygon (rb);
        }
      break;
    }



  //  Second, scan the area for record numbers and save them in the records array (record_count).

  NV_INT32 cx = -1, cy = -1;

  for (NV_INT32 i = 0 ; i < data->count ; i++)
    {
      //  Check against the displayed minimum bounding rectangle.  This is all that is needed for 
      //  a rectangle and it cuts down on the computations for a polygon.

      if (!check_bounds (options, misc, data, i, NVFalse, misc->slice))
        {
          //  Convert the X, Y, and Z value to a projected pixel position

          map->get2DCoords (data->x[i], data->y[i], -data->z[i], &cx, &cy);


          scan = NVFalse;

          switch (options->function)
            {
            case DELETE_RECTANGLE:
              if (cx >= min_x && cx <= max_x && cy >= min_y && cy <= max_y) scan = NVTrue;
              break;

            case DELETE_POLYGON:
              if (inside_polygon2 (mx, my, count, (NV_FLOAT64) cx, (NV_FLOAT64) cy)) scan = NVTrue;
              break;
            }


          if (scan)
            {
              hit = NVFalse;

              for (NV_INT32 k = 0 ; k < record_count ; k++)
                {
                  if (data->rec[i] == records[k])
                    {
                      hit = NVTrue;
                      break;
                    }
                }

              if (!hit)
                {
                  records = (NV_U_INT32 *) realloc (records, (record_count + 1) * sizeof (NV_U_INT32));

                  if (records == NULL)
                    {
                      perror ("Allocating records memory in kill_records.cpp");
                      exit (-1);
                    }

                  records[record_count] = data->rec[i];

                  record_count++;
                }
            }
        }
    }


  if (mx != NULL)
    {
      free (mx);
      free (my);
    }


  //  Third, cycle through all of the visible data and invalidate subrecords of records that showed up in the scanned area.

  if (record_count)
    {
      for (NV_INT32 i = 0 ; i < data->count ; i++)
        {
          for (NV_INT32 j = 0 ; j < record_count ; j++)
            {
              if (data->rec[i] == records[j])
                {
                  //  Save the undo information.

                  store_undo (misc, options->undo_levels, data->val[i], i);


                  data->val[i] |= PFM_MANUALLY_INVAL;
                  break;
                }
            }
        }

      free (records);
    }
}
