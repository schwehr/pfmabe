
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

  - Module Name:        overlayData

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1995

  - Purpose:            Draw flag locations in the GC and the pixmap.

  - Arguments:

  - Return Value:       void

****************************************************************************/

void overlayData (nvMap *map, OPTIONS *options, MISC *misc)
{
  NV_INT32        x, y, z;
  NV_FLOAT64      dep;


  //  If we have a kill list from the filter we only want to mark the potential kill points (only if we are confirming the filter action).

  if (misc->filter_kill_count)
    {
      for (NV_INT32 i = 0 ; i < misc->filter_kill_count ; i++)
        {
          NV_INT32 j = misc->filter_kill_list[i];

          dep = misc->data[j].z;
          if (!misc->view)
            {
              map->map_to_screen (1, &misc->data[j].x, &misc->data[j].y, &dep, &x, &y, &z);
            }
          else
            {
              scale_view (map, misc, misc->data[j].x, misc->data[j].y, dep, &x, &y, &z);
            }

          drawx (map, x, y, options->contour_color, HIGHLIGHTED, NVFalse);
        }


      map->flush ();

      return;
    }


  //  If we have an av_dist list we only want to mark those points.

  if (misc->av_dist_count)
    {
      for (NV_INT32 i = 0 ; i < misc->av_dist_count ; i++)
        {
          NV_INT32 j = misc->av_dist_list[i];

          dep = misc->data[j].z;
          if (!misc->view)
            {
              map->map_to_screen (1, &misc->data[j].x, &misc->data[j].y, &dep, &x, &y, &z);
            }
          else
            {
              scale_view (map, misc, misc->data[j].x, misc->data[j].y, dep, &x, &y, &z);
            }


          //  Because the colors of the markers correlate to the Waveform viewer, white needs to be a color of a marker, therefore,
          //  I'm going with gray for a filter point for the AV Distance tool.  This still may conflict with a different shade of
          //  gray of a neighbor.

          drawx (map, x, y, Qt::gray, HIGHLIGHTED, NVFalse);
        }


      map->flush ();

      return;
    }


  //  Display the highlighted points.

  for (NV_INT32 i = 0 ; i < misc->highlight_count ; i++)
    {
      if (!check_bounds (map, options, misc, misc->highlight[i], NVTrue, misc->slice))
        {
          dep = misc->data[misc->highlight[i]].z;
          if (!misc->view)
            {
              map->map_to_screen (1, &misc->data[misc->highlight[i]].x, &misc->data[misc->highlight[i]].y, &dep, &x, &y, &z);
            }
          else
            {
              scale_view (map, misc, misc->data[misc->highlight[i]].x, misc->data[misc->highlight[i]].y, dep, &x, &y, &z);
            }
        }

      drawx (map, x, y, options->contour_color, HIGHLIGHTED, NVFalse);
    }
}
