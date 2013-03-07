
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "pfmView.hpp"

/*!
  This function grabs the contour closest to the cursor and then highlights that contour level wherever it is displayed.
  The contour level will then be used for hand drawing contours to fill gaps in the data coverage.  This is only used
  if you have an interpolated surface instead of the normal average surface.
*/

void highlight_contour (MISC *misc, OPTIONS *options, NV_FLOAT64 lat, NV_FLOAT64 lon, nvMap *map)
{
  //  Save the MISP depth for drawing contours (depending on the type of MISP surface)

  NV_F64_COORD2 xy;
  BIN_RECORD bin;

  xy.y = lat;
  xy.x = lon;


  //  Try to find the highest layer with a valid value to grab the contour level.

  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      if (bin_inside_ptr (&misc->abe_share->open_args[pfm].head, xy))
	{
	  read_bin_record_xy (misc->pfm_handle[pfm], xy, &bin);


	  //  Check the validity.

	  if ((bin.validity & PFM_DATA) || (bin.validity & PFM_INTERPOLATED)) break;
	}
    }


  NV_FLOAT32 save_level = (NV_FLOAT32) (NINT (bin.avg_filtered_depth));


  //  Custom contour levels...

  if (misc->abe_share->cint == 0.0)
    {
      NV_FLOAT32 mindiff = 999999999.0;
      NV_INT32 j = 0;
      for (NV_INT32 i = 0 ; i < misc->abe_share->num_levels ; i++)
	{
	  NV_FLOAT32 diff = fabs (save_level - misc->abe_share->contour_levels[i]);
	  if (diff < mindiff)
	    {
	      diff = mindiff;
	      j = i;
	    }
	}

      misc->draw_contour_level = misc->abe_share->contour_levels[j];
    }


  //  Normal contour interval...

  else
    {
      NV_INT32 mult = NINT ((save_level * options->z_factor + options->z_offset) / misc->abe_share->cint);
      misc->draw_contour_level = misc->abe_share->cint * (NV_FLOAT32) mult;
    }


  //  Draw the current contours in highlight color so the "artist" can see where he can connect
  //  his/her drawn contour

  if (options->contour)
    {
      for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
	{
	  if (misc->abe_share->display_pfm[pfm])
	    {
	      //  Adjust bounds to nearest grid point

	      adjust_bounds (misc, pfm);


	      misc->hatchr_start_x = 0;
	      misc->hatchr_end_x = misc->displayed_area_width[pfm];
	      misc->hatchr_start_y = 0;
	      misc->hatchr_end_y = misc->displayed_area_height[pfm];


	      //  Allocate the needed arrays.

	      NV_INT32 size;
	      NV_FLOAT32 *ar = NULL;

	      size = misc->displayed_area_width[pfm] * misc->displayed_area_height[pfm];
	      ar = (NV_FLOAT32 *) calloc (size, sizeof (NV_FLOAT32));

	      if (ar == NULL)
		{
		  perror (pfmView::tr ("Allocating ar array in highlight_contour").toAscii ());
		  exit (-1);
		}

	      BIN_RECORD *current_record = (BIN_RECORD *) calloc (misc->displayed_area_width[pfm], sizeof (BIN_RECORD));

	      if (current_record == NULL)
		{
		  perror (pfmView::tr ("Allocating current_record array in highlight_contour").toAscii ());
		  exit (-1);
		}

	      misc->current_row = (NV_FLOAT32 *) calloc (misc->displayed_area_width[pfm], sizeof (NV_FLOAT32));
	      misc->current_flags = (NV_U_CHAR *) calloc (misc->displayed_area_width[pfm], sizeof (NV_CHAR));


	      //  If the calloc failed, error out.

	      if (misc->current_flags == NULL)
		{
		  perror (pfmView::tr (" Unable to allocate current_flags in highlight_contour.\n").toAscii ());
		  exit (-1);
		}


	      //  Read input data row by row and allot data to appropriate
	      //  subplot files until current row of subplots is complete.

	      for (NV_INT32 jj = misc->hatchr_start_y ; jj < misc->hatchr_end_y ; jj++)
		{
		  //  Read data from disk.

		  read_bin_row (misc->pfm_handle[pfm], misc->displayed_area_width[pfm], misc->displayed_area_row[pfm] + jj, 
				misc->displayed_area_column[pfm], current_record);

		  loadArrays (misc->abe_share->layer_type, misc->displayed_area_width[pfm], current_record, misc->current_row, NULL, 0,
                              misc->current_flags, options->highlight, options->h_count, misc->pfm_handle[pfm], misc->abe_share->open_args[pfm],
                              options->highlight_percent, misc->surface_val);


		  for (NV_INT32 i = 0 ; i < misc->displayed_area_width[pfm] ; i++)
		    {
		      ar[(jj - misc->hatchr_start_y) * misc->displayed_area_width[pfm] + i] = misc->current_row[i] *
			options->z_factor + options->z_offset;
		    }
		}
	      free (current_record);
	      free (misc->current_row);
	      free (misc->current_flags);

	      scribe (map, options, misc, pfm, ar, misc->draw_contour_level);

	      free (ar);
	    }
	}
    }
}
