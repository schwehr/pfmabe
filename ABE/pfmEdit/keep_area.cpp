
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

  - Module Name:        keep_area

  - Programmer(s):      Rebecca Martinolich

  - Date Written:       October 1998

  - Purpose:            Invalidates data outside a polygon.

  - Arguments:
                        - x_bounds        =   array of x vertices
                        - y_bounds        =   array of y vertices
                        - z_bounds        =   array of z vertices (east &
                                              south views)
                        - num_vertices    =   number of vertices

  - Return Value:
                        - void

****************************************************************************/

void keep_area (nvMap *map, OPTIONS *options, MISC *misc, NV_FLOAT64 *x_bounds, NV_FLOAT64 *y_bounds, NV_FLOAT64 *z_bounds, NV_INT32 num_vertices)
{
  NV_INT32            x_value, dummy;


  //  View switch.

  if (!misc->view)
    {
      for (NV_INT32 i = 0 ; i < misc->last_drawn_index ; i++)
        {
          //  Check for single line display.

          if (!misc->num_lines || check_line (misc, misc->data[i].line))
            {
              //  Check against the rectangular or polygonal area that is displayed.

              if (!check_bounds (map, options, misc, i, NVFalse, misc->slice))
                {
                  //  Now check the point against the polygon.

                  if (!inside_polygon2 (x_bounds, y_bounds, num_vertices, misc->data[i].x, misc->data[i].y))
                    {
                      //  Save the undo information.

                      store_undo (misc, options->undo_levels, misc->data[i].val, i);


                      misc->data[i].val |= PFM_MANUALLY_INVAL;
                    }
                }
            }
        }
    }
  else
    {
      for (NV_INT32 i = 0 ; i < misc->last_drawn_index ; i++)
        {
          //  Check for single line display.

          if (!misc->num_lines || check_line (misc, misc->data[i].line))
            {
              //  Rotate the points to check them since we can't unrotate from the x and z pixel values.
                
              scale_view (map, misc, misc->data[i].x, misc->data[i].y, misc->data[i].z, &x_value, &dummy, &dummy);


              //  Check against the rectangular or polygonal area that is displayed.

              if (!check_bounds (map, options, misc, i, NVFalse, misc->slice))
                {
                  //  Now check the point against the polygon.

                  if (!inside_polygon2 (x_bounds, z_bounds, num_vertices, (NV_FLOAT64) x_value, (NV_FLOAT64) misc->data[i].z))
                    {
                      //  Save the undo information.

                      store_undo (misc, options->undo_levels, misc->data[i].val, i);


                      misc->data[i].val |= PFM_MANUALLY_INVAL;
                    }
                }
            }
        }
    }


  //  Close the undo block.

  end_undo_block (misc);
}
