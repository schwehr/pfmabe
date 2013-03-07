
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



#include "pfmEdit3D.hpp"


/***************************************************************************/
/*!

  - Module Name:        set_area

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1995

  - Purpose:            Invalidates, restores, sets, unsets reference data
                        in a rectangle or polygon.

  - Arguments:
                        - map             =   nvmap object to draw on
                        - options         =   OPTIONS structure
                        - misc            =   MISC structure
                        - data            =   point data used to define bounds
                        - x_bounds        =   array of x vertices
                        - y_bounds        =   array of y vertices
                        - z_bounds        =   array of z vertices (east & south views)
                        - num_vertices    =   number of vertices
                        - rect_flag       =   1 for rectangle, 0 for polygon

  - Return Value:
                        - void

****************************************************************************/

void set_area (nvMapGL *map, OPTIONS *options, MISC *misc, NV_INT32 *x_bounds, NV_INT32 *y_bounds, NV_INT32 num_vertices, NV_INT32 rect_flag)
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

          map->get2DCoords (misc->data[j].x, misc->data[j].y, -misc->data[j].z, &px, &py);


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

  NV_BOOL check_invalid = NVTrue;
  switch (options->function)
    {
    case RESTORE_RECTANGLE:
    case RESTORE_POLYGON:
      undo_on = NVTrue;
      check_invalid = NVFalse;
      break;

    case DELETE_RECTANGLE:
    case UNSET_REFERENCE:
    case SET_REFERENCE:
    case DELETE_POLYGON:
      undo_on = NVTrue;
      break;

    default:
      undo_on = NVFalse;
    }


  for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
    {
      //  Check for single line display.

      if (!misc->num_lines || check_line (misc, misc->data[i].line))
        {
          //  Check against the displayed minimum bounding rectangle.  This is all that is needed for 
          //  a rectangle and it cuts down on the computations for a polygon.   Also, DO NOT 
          //  allow changes to null value status.

          if (!check_bounds (options, misc, i, check_invalid, misc->slice) && misc->data[i].z < misc->null_val[misc->data[i].pfm])
            {

              //  Convert the X, Y, and Z value to a projected pixel position

              map->get2DCoords (misc->data[i].x, misc->data[i].y, -misc->data[i].z, &px, &py);


              //  Rectangle.

              if (rect_flag)
                {
                  if (px >= min_x && px <= max_x && py >= min_y && py <= max_y)
                    {
                      switch (options->function)
                        {
                        case RESTORE_RECTANGLE:
                          store_undo (misc, options->undo_levels, misc->data[i].val, i);
                          misc->data[i].val &= ~PFM_MANUALLY_INVAL;
                          misc->data[i].val &= ~PFM_FILTER_INVAL;
                          break;

                        case DELETE_RECTANGLE:
                          store_undo (misc, options->undo_levels, misc->data[i].val, i);
                          misc->data[i].val |= PFM_MANUALLY_INVAL;
                          break;

                        case UNSET_REFERENCE:
                          store_undo (misc, options->undo_levels, misc->data[i].val, i);
                          misc->data[i].val &= ~PFM_REFERENCE;
                          break;

                        case SET_REFERENCE:
                          store_undo (misc, options->undo_levels, misc->data[i].val, i);
                          misc->data[i].val |= PFM_REFERENCE;
                          break;

                        case MASK_INSIDE_RECTANGLE:
                          misc->data[i].mask = NVTrue;
                          break;

                        case RECT_FILTER_MASK:
                          misc->data[i].fmask = NVTrue;
                          break;
                        }
                    }
                  else
                    {
                      switch (options->function)
                        {
                        case MASK_OUTSIDE_RECTANGLE:
                          misc->data[i].mask = NVTrue;
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
                          store_undo (misc, options->undo_levels, misc->data[i].val, i);
                          misc->data[i].val &= ~PFM_MANUALLY_INVAL;
                          misc->data[i].val &= ~PFM_FILTER_INVAL;
                          break;

                        case DELETE_POLYGON:
                          store_undo (misc, options->undo_levels, misc->data[i].val, i);
                          misc->data[i].val |= PFM_MANUALLY_INVAL;
                          break;

                        case UNSET_REFERENCE:
                          store_undo (misc, options->undo_levels, misc->data[i].val, i);
                          misc->data[i].val &= ~PFM_REFERENCE;
                          break;

                        case SET_REFERENCE:
                          store_undo (misc, options->undo_levels, misc->data[i].val, i);
                          misc->data[i].val |= PFM_REFERENCE;
                          break;

                        case MASK_INSIDE_POLYGON:
                          misc->data[i].mask = NVTrue;
                          break;

                        case POLY_FILTER_MASK:
                          misc->data[i].fmask = NVTrue;
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
                          misc->data[i].mask = NVTrue;
                          break;
                        }
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
