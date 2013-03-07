
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



#include "bagViewer.hpp"


void overlayFlag (nvMapGL *map, OPTIONS *options, MISC *misc)
{
  map->clearFeaturePoints ();


  //  Display features.

  if (options->display_tracking_list)
    {
      for (NV_U_INT32 i = 0 ; i < misc->tracking_list_len ; i++)
        {
          //  Go through here if we want to display feature info

          if (options->display_tracking_info)
            {
              QString string0 (misc->tracking_list[i].description);

              if (!string0.isEmpty ()) 
                map->drawText (string0, misc->tracking_list[i].x, misc->tracking_list[i].y, misc->tracking_list[i].depth, 0.0, 10,
                               options->feature_info_color, NVFalse);
            }

          map->setFeaturePoints (misc->tracking_list[i].x, misc->tracking_list[i].y, misc->tracking_list[i].depth, options->feature_color,
                                 options->feature_size, 7, 7, NVFalse);

          if (qApp->hasPendingEvents ())
            {
              qApp->processEvents();
              if (misc->drawing_canceled) break;
            }
        }

      map->setFeaturePoints (0.0, 0.0, 0.0, Qt::black, 0.00, 0, 0, NVTrue);
    }
  else
    {
      map->flush ();
    }
}
