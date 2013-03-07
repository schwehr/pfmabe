
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



#include "pfmEdit3DDef.hpp"


/***************************************************************************/
/*!

  - Module Name:        overlayData

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1995

  - Purpose:            Draw flag locations in the GC and the pixmap.

  - Arguments:

  - Return Value:       void

****************************************************************************/

void overlayData (nvMapGL *map, OPTIONS *options, MISC *misc)
{
  //  Show the filter masked points if any are present.

  if (misc->filter_mask)
    {
      QColor mask = options->marker_color;
      mask.setAlpha (64);

      for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
        {
          //  If we are displaying and editing only a single line, only get those points that 
          //  are in that line.

          if (!misc->num_lines || check_line (misc, misc->data[i].line))
            {
              if (!check_bounds (options, misc, i, NVTrue, misc->slice))
                {
                  if (misc->data[i].fmask)
                    {
                      NV_INT32 pix_x, pix_y;
                      map->get2DCoords (misc->data[i].x, misc->data[i].y, -misc->data[i].z, &pix_x, &pix_y);

                      map->drawLine (pix_x - 3, pix_y - 3, pix_x + 3, pix_y - 3, mask, 2, Qt::SolidLine, NVFalse);
                      map->drawLine (pix_x + 3, pix_y - 3, pix_x + 3, pix_y + 3, mask, 2, Qt::SolidLine, NVFalse);
                      map->drawLine (pix_x + 3, pix_y + 3, pix_x - 3, pix_y + 3, mask, 2, Qt::SolidLine, NVFalse);
                      map->drawLine (pix_x - 3, pix_y + 3, pix_x - 3, pix_y - 3, mask, 2, Qt::SolidLine, NVFalse);
                    }
                }
            }
        }
      map->drawLine (0, 0, 0, 0, options->background_color, 0, Qt::SolidLine, NVTrue);
    }
}
