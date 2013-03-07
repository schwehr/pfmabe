
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


//!  Returns (in the hot argument) the nearest point to the cursor (at xy) that is marked to be killed.

void get_nearest_kill_point (MISC *misc, NV_FLOAT64 lat, NV_FLOAT64 lon, NV_F64_COORD3 *hot)
{
  NV_FLOAT64 min_dist = 9999999999.0;
  NV_INT32 kill_point = -1;

  for (NV_INT32 i = 0 ; i < misc->filter_kill_count ; i++)
    {
      NV_INT32 j = misc->filter_kill_list[i];


      NV_FLOAT64 dist = sqrt ((NV_FLOAT64) ((lat - misc->data[j].y) * (lat - misc->data[j].y)) +
                              (NV_FLOAT64) ((lon - misc->data[j].x) * (lon - misc->data[j].x)));

      if (dist < min_dist)
        {
          min_dist = dist;
          hot->x = misc->data[j].x;
          hot->y = misc->data[j].y;
          hot->z = misc->data[j].z;
          kill_point = j;
        }
    }


  misc->nearest_point = misc->nearest_stack.point[0] = kill_point;


  //  Force this point to not be replaced by compare_to_stack by setting it's distance to a negative.

  misc->nearest_stack.dist[0] = -999.0;


  return;
}
