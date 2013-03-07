
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

//!  Filter based on the attribute that we are coloring by (for example, horizontal uncertainty).

void attrFilter (OPTIONS *options, MISC *misc)
{
  NV_BOOL inside = NVFalse;
  NV_FLOAT32 minval = options->attr_filter_range[options->color_index][0];
  NV_FLOAT32 maxval = options->attr_filter_range[options->color_index][1];


  //  Determine if we are invalidating inside or outside of the range.

  if (minval > maxval) inside = NVTrue;


  for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
    {
      if (!(misc->data[i].val & (PFM_DELETED | PFM_REFERENCE | PFM_INVAL)))
        {
          NV_BOOL filter_it = NVFalse;

          if (!check_bounds (options, misc, misc->data[i].x, misc->data[i].y, misc->data[i].z, NVFalse, misc->data[i].mask, misc->data[i].pfm,
                             NVFalse, misc->slice))
            {
              //  Note that we don't filter on depth or line number.

              switch (options->color_index)
                {
                case 2:
                  if (inside)
                    {
                      if (misc->data[i].herr >= maxval && misc->data[i].herr <= minval) filter_it = NVTrue;
                    }
                  else
                    {
                      if (misc->data[i].herr < minval || misc->data[i].herr > maxval) filter_it = NVTrue;
                    }
                  break;

                case 3:
                  if (inside)
                    {
                      if (misc->data[i].verr >= maxval && misc->data[i].verr <= minval) filter_it = NVTrue;
                    }
                  else
                    {
                      if (misc->data[i].verr < minval || misc->data[i].verr > maxval) filter_it = NVTrue;
                    }
                  break;

                default:
                  NV_INT32 ndx = options->color_index - PRE_ATTR;

                  if (inside)
                    {
                      if (misc->data[i].attr[ndx] >= maxval && misc->data[i].attr[ndx] <= minval) filter_it = NVTrue;
                    }
                  else
                    {
                      if (misc->data[i].attr[ndx] < minval || misc->data[i].attr[ndx] > maxval) filter_it = NVTrue;
                    }
                  break;
                }


              if (filter_it)
                {
                  misc->filter_kill_list = (NV_INT32 *) realloc (misc->filter_kill_list, (misc->filter_kill_count + 1) * sizeof (NV_INT32));

                  if (misc->filter_kill_list == NULL)
                    {
                      perror ("Allocating misc->filter_kill_list memory in attrFilter.cpp");
                      exit (-1);
                    }

                  misc->filter_kill_list[misc->filter_kill_count] = i;
                  misc->filter_kill_count++;
                }
            }
        }
    }
}
