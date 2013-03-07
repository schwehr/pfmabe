
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



#include "pfmEditDef.hpp"


//!  Draws a semi-transparent grid of the bins over the data points when in plan view.

void bin_grid (MISC misc, OPTIONS options, nvMap *map)
{
  QColor color = options.contour_color;
  color.setAlpha (127);


  for (NV_INT32 i = 0 ; i < misc.abe_share->open_args[0].head.bin_height ; i++)
    {
      NV_FLOAT64 lat = misc.abe_share->open_args[0].head.mbr.min_y + (NV_FLOAT64) i *
	misc.abe_share->open_args[0].head.y_bin_size_degrees;

      if (lat >= misc.displayed_area.min_y && lat <= misc.displayed_area.max_y)
	{
	  map->drawLine (misc.displayed_area.min_x, lat, misc.displayed_area.max_x, lat, color, 2,
			 NVFalse, Qt::SolidLine);
	}
    }


  for (NV_INT32 i = 0 ; i < misc.abe_share->open_args[0].head.bin_width ; i++)
    {
      NV_FLOAT64 lon = misc.abe_share->open_args[0].head.mbr.min_x + (NV_FLOAT64) i *
	misc.abe_share->open_args[0].head.x_bin_size_degrees;

      if (lon >= misc.displayed_area.min_x && lon <= misc.displayed_area.max_x)
	{
	  map->drawLine (lon, misc.displayed_area.min_y, lon, misc.displayed_area.max_y, color, 2,
			 NVFalse, Qt::SolidLine);
	}
    }

  map->flush ();
}
