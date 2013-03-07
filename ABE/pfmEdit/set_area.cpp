
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



#include "pfmEdit.hpp"


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

void set_area (nvMap *map, OPTIONS *options, MISC *misc, NV_FLOAT64 *x_bounds, NV_FLOAT64 *y_bounds, NV_FLOAT64 *z_bounds,
               NV_INT32 num_vertices, NV_INT32 rect_flag)
{
  NV_INT32        i, j, x_value, dummy;
  NV_FLOAT64      min_x, max_x, min_y, max_y;
  NV_BOOL         undo_on = NVFalse;


  //  Whether we're making an undo block.

  NV_BOOL check_invalid = NVTrue;
  switch (options->function)
    {
    case RESTORE_RECTANGLE:
    case RESTORE_POLYGON:
      check_invalid = NVFalse;
      undo_on = NVTrue;
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


  //  View switch.

  if (!misc->view)
    {
      //  Get the minimum bounding rectangle (X and Y).

      min_y = y_bounds[0];
      max_y = y_bounds[0];
      min_x = x_bounds[0];
      max_x = x_bounds[0];
      for (j = 0 ; j < num_vertices ; j++)
        {
          min_y = qMin (min_y , y_bounds[j]);
          max_y = qMax (max_y , y_bounds[j]);
          min_x = qMin (min_x , x_bounds[j]);
          max_x = qMax (max_x , x_bounds[j]);
        }


      //  CLEAR_POLYGON is a special case because we have to search the highlight list instead of all of the points.

      if (options->function == CLEAR_POLYGON)
        {
          for (NV_INT32 i = 0 ; i < misc->highlight_count ; i++)
            {
              NV_INT32 j = misc->highlight[i];


              //  Mark the points that fall inside the polygon.

              if (inside_polygon2 (x_bounds, y_bounds, num_vertices, misc->data[j].x, misc->data[j].y)) misc->highlight[i] = -1;
            }


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


      for (i = 0 ; i < misc->last_drawn_index ; i++)
        {
          //  Check for single line display.

          if (!misc->num_lines || check_line (misc, misc->data[i].line))
            {

              //  Check against the minimum bounding rectangle.  This is all that is needed for 
              //  a rectangle and it cuts down on the computations for a polygon.   Also, DO NOT 
              //  allow changes to null value status.

              if (!check_bounds (map, options, misc, i, check_invalid, misc->slice) && misc->data[i].z < misc->null_val[misc->data[i].pfm])
                {
                  //  Rectangle.
                    
                  if (rect_flag)
                    {
                      if (misc->data[i].x >= min_x && misc->data[i].x <= max_x && misc->data[i].y >= min_y && misc->data[i].y <= max_y)
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
                      if (inside_polygon2 (x_bounds, y_bounds, num_vertices, misc->data[i].x, misc->data[i].y))
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
    }
  else
    {
      //  Get the minimum bounding rectangle (X and Z).

      min_y = z_bounds[0];
      max_y = z_bounds[0];
      min_x = x_bounds[0];
      max_x = x_bounds[0];
      for (j = 0 ; j < num_vertices ; j++)
        {
          min_y = qMin (min_y , z_bounds[j]);
          max_y = qMax (max_y , z_bounds[j]);
          min_x = qMin (min_x , x_bounds[j]);
          max_x = qMax (max_x , x_bounds[j]);
        }


      //  CLEAR_POLYGON is a special case because we have to search the highlight list instead of all of the points.

      if (options->function == CLEAR_POLYGON)
        {
          for (NV_INT32 i = 0 ; i < misc->highlight_count ; i++)
            {
              NV_INT32 j = misc->highlight[i];


              //  Rotate the points to check them since we can't unrotate from the x and z pixel values.
                
              scale_view (map, misc, misc->data[j].x, misc->data[j].y, misc->data[j].z, &x_value, &dummy, &dummy);


              //  Mark the points that fall inside the polygon.

              if (inside_polygon2 (x_bounds, z_bounds, num_vertices, (NV_FLOAT64) x_value, (NV_FLOAT64) misc->data[j].z)) misc->highlight[i] = -1;
            }


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


      for (i = 0 ; i < misc->last_drawn_index ; i++)
        {

          //  Check for single line display.

          if (!misc->num_lines || check_line (misc, misc->data[i].line))
            {
              //  Rotate the points to check them since we can't unrotate from the x and z pixel values.
                
              scale_view (map, misc, misc->data[i].x, misc->data[i].y, misc->data[i].z, &x_value, &dummy, &dummy);


              //  Check against the rectangular or polygonal area that is displayed.

              if (!check_bounds (map, options, misc, i, check_invalid, misc->slice) && misc->data[i].z < misc->null_val[misc->data[i].pfm])
                {
                  //  Rectangle.
                        
                  if (rect_flag)
                    {
                      if (x_value >= min_x && x_value <= max_x && misc->data[i].z >= min_y && misc->data[i].z <= max_y)
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
                      if (inside_polygon2 (x_bounds, z_bounds, num_vertices, (NV_FLOAT64) x_value, (NV_FLOAT64) misc->data[i].z))
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
    }


  //  Close the undo block

  if (undo_on) end_undo_block (misc);
}
