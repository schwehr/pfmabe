
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
#include <cmath>


/***************************************************************************/
/*!

  - Module Name:        scale_view

  - Programmer(s):      Jan C. Depner

  - Date Written:       January 1996

  - Purpose:            Projects points in the selected view display.

  - Arguments:
                        - x               =   x value
                        - y               =   y value
                        - z               =   z value
                        - x_ret           =   x pixel value returned
                        - y_ret           =   y pixel value returned

  - Return Value:
                        - void

****************************************************************************/

void scale_view (nvMap *map, MISC *misc, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT32 z, NV_INT32 *x_ret, NV_INT32 *y_ret, NV_INT32 *z_ret)
{
    NV_FLOAT64      dummy, ox, xyz_x, xyz_y;


    map->map_to_screen (1, &x, &y, &dummy, &ox, &xyz_y, &dummy);

    xyz_x = ox * misc->cos_array[misc->rotate_angle] - xyz_y * misc->sin_array[misc->rotate_angle];

    *x_ret = misc->map_border + NINT (((NV_FLOAT32) (xyz_x + misc->rotate_offset) - misc->map_border) * misc->rotate_scale);
    *y_ret = NINT (misc->map_border + ((z - misc->view_min) / (misc->view_max - misc->view_min)) * (misc->draw_area_height - (misc->map_border * 2)));

    xyz_x = ox * misc->cos_array[misc->ortho_angle] - xyz_y * misc->sin_array[misc->ortho_angle];

    *z_ret = misc->map_border + NINT (((NV_FLOAT32) (xyz_x + misc->ortho_offset) - misc->map_border) * misc->ortho_scale);
}


/***************************************************************************/
/*!

  - Module Name:        unscale_view

  - Programmer(s):      Jan C. Depner

  - Date Written:       January 1996

  - Purpose:            Un-projects points in the selected view display.

  - Arguments:
                        - y               =   y pixel
                        - y_ret           =   z value returned

  - Return Value:
                        - void

****************************************************************************/

void unscale_view (MISC *misc, NV_INT32 y, NV_FLOAT64 *y_ret)
{
    *y_ret = misc->view_min + (y / (NV_FLOAT32) (misc->draw_area_height)) * (misc->view_max - misc->view_min);
}
