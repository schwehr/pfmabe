
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

  - Module Name:        drawx

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1994

  - Purpose:            Draw an X or other figure in the GC and the pixmap.

  - Arguments:
                        - x       =   X pixel position
                        - y       =   Y pixel position
                        - color   =   QColor to draw the X

  - Return Value:
                        - void

****************************************************************************/

void drawx (nvMap *map, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 type, NV_BOOL flush)
{
  switch (type)
    {
    case FILTER_MASK:
      map->fillCircle (x, y, 10, 0.0, 360.0, color, flush);
      break;


      //  Special case for polygon points in feature polygons.

    case 99999:
      map->drawLine (x - 3, y - 3, x - 3, y + 3, color, 2, flush, Qt::SolidLine);
      map->drawLine (x - 3, y + 3, x + 3, y + 3, color, 2, flush, Qt::SolidLine);
      map->drawLine (x + 3, y + 3, x + 3, y - 3, color, 2, flush, Qt::SolidLine);
      map->drawLine (x + 3, y - 3, x - 3, y - 3, color, 2, flush, Qt::SolidLine);
      break;

    case HIGHLIGHTED:
      map->drawLine (x - 6, y, x - 3, y, color, 2, flush, Qt::SolidLine);
      map->drawLine (x + 3, y, x + 6, y, color, 2, flush, Qt::SolidLine);
      map->drawLine (x, y - 6, x, y - 3, color, 2, flush, Qt::SolidLine);
      map->drawLine (x, y + 3, x, y + 6, color, 2, flush, Qt::SolidLine);
      break;
    }
}
