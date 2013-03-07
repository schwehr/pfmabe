
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

  - Module Name:        get_bounds

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1995

  - Purpose:            Computes the boundaries of the area based on the
                        data in memory.

  - Return Value:       0 if no non-null points found, else count           *

****************************************************************************/

NV_INT32 get_bounds (OPTIONS *options, MISC *misc)
{
  //  Define the total display area.

  misc->displayed_area.min_x = 999999999.0;
  misc->displayed_area.max_x = -999999999.0;
  misc->displayed_area.min_y = 999999999.0;
  misc->displayed_area.max_y = -999999999.0;
  NV_INT32 count = 0;


  //  Loop through the data in memory looking for min and max.
    
  for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
    {
      //  Only use data for PFM files that are being displayed.  Do not use null data.  Do not use masked data.
      //  Do not use invalid data unless the display_invalid flag is set.

      if (misc->abe_share->display_pfm[misc->data[i].pfm] && !(misc->data[i].val & PFM_DELETED) && !misc->data[i].mask &&
          (options->display_man_invalid || (!(misc->data[i].val & PFM_MANUALLY_INVAL))) && 
          (options->display_flt_invalid || (!(misc->data[i].val & PFM_FILTER_INVAL))))
        {
          misc->displayed_area.min_x = qMin (misc->displayed_area.min_x, misc->data[i].x);
          misc->displayed_area.min_y = qMin (misc->displayed_area.min_y, misc->data[i].y);
          misc->displayed_area.max_x = qMax (misc->displayed_area.max_x, misc->data[i].x);
          misc->displayed_area.max_y = qMax (misc->displayed_area.max_y, misc->data[i].y);
          count ++;
        }
    }


  //  If no valid data points were found, use any nav points for the bounds.
        
  if (misc->displayed_area.min_x == 999999999.0)
    {
      misc->displayed_area.min_x = 999999999.0;
      misc->displayed_area.max_x = -999999999.0;
      misc->displayed_area.min_y = 999999999.0;
      misc->displayed_area.max_y = -999999999.0;
      count = 0;

      for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
        {
          misc->displayed_area.min_x = qMin (misc->displayed_area.min_x, misc->data[i].x);
          misc->displayed_area.min_y = qMin (misc->displayed_area.min_y, misc->data[i].y);
          misc->displayed_area.max_x = qMax (misc->displayed_area.max_x, misc->data[i].x);
          misc->displayed_area.max_y = qMax (misc->displayed_area.max_y, misc->data[i].y);
          count ++;
        }
    }


  if (count)
    {
      //  Add 2% to the window size.
        
      NV_FLOAT64 width_2 = (misc->displayed_area.max_x - misc->displayed_area.min_x) * 0.02;
      NV_FLOAT64 height_2 = (misc->displayed_area.max_y - misc->displayed_area.min_y) * 0.02;
      misc->displayed_area.min_y -= height_2;
      misc->displayed_area.max_y += height_2;
      misc->displayed_area.min_x -= width_2;
      misc->displayed_area.max_x += width_2;
    }


  //  Saving the bounds to the minimum bounding cube.  At the moment we only use the Z part of this which is computed in
  //  pfmEdit.cpp but this makes it match pfmEdit3D a bit more closely.

  misc->bounds.min_x = misc->displayed_area.min_x;
  misc->bounds.max_x = misc->displayed_area.max_x;
  misc->bounds.min_y = misc->displayed_area.min_y;
  misc->bounds.max_y = misc->displayed_area.max_y;


  //  Save the displayed area for other ABE programs

  misc->abe_share->displayed_area = misc->displayed_area;


  return (count);
}
