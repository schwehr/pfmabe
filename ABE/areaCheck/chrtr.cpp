#include "areaCheck.hpp"


NV_BOOL chrtr (NV_INT32 file_number, nvMap *map, MISC *misc, OPTIONS *options)
{
  void hatchr (NV_INT32 k, NV_INT32 file_number, nvMap *map, OPTIONS *options, MISC *misc, NV_FLOAT32 *current_row, NV_FLOAT32 *next_row,
	       NV_INT32 start_row, NV_INT32 row, NV_INT32 cols, NV_FLOAT64 min_z, NV_FLOAT64 max_z, NV_F64_XYMBR mbr);


  CHRTR_HEADER        header;
  NV_INT32            k = CHRTR;


  //  Open the file.
        
  NV_INT32 hnd = open_chrtr (misc->overlays[k][file_number].filename, &header);

  if (hnd < 0) return (NVTrue);


  misc->overlays[k][file_number].x_bin_size_degrees = (NV_FLOAT64) header.grid_minutes / 60.0;
  misc->overlays[k][file_number].y_bin_size_degrees = (NV_FLOAT64) header.grid_minutes / 60.0;
  misc->overlays[k][file_number].mbr.min_x = (NV_FLOAT64) header.wlon;
  misc->overlays[k][file_number].mbr.min_y = (NV_FLOAT64) header.slat;
  misc->overlays[k][file_number].mbr.max_x = (NV_FLOAT64) header.elon;
  misc->overlays[k][file_number].mbr.max_y = (NV_FLOAT64) header.nlat;
  misc->overlays[k][file_number].bin_width = header.width;
  misc->overlays[k][file_number].bin_height = header.height;

  misc->overlays[k][file_number].center.x = header.wlon + (header.elon - header.wlon) / 2.0;
  if (header.wlon > header.elon) misc->overlays[k][file_number].center.x = header.wlon + 
    ((header.elon + 360.0) - header.wlon) / 2.0;
  misc->overlays[k][file_number].center.y = header.slat + (header.nlat - header.slat) / 2.0;


  //  Compute the x and y bin size in meters for sunshading

  NV_FLOAT64 az;

  invgp (NV_A0, NV_B0, misc->overlays[k][file_number].center.y, misc->overlays[k][file_number].center.x, 
	 misc->overlays[k][file_number].center.y + misc->overlays[k][file_number].y_bin_size_degrees,
	 misc->overlays[k][file_number].center.x, &misc->overlays[k][file_number].y_bin_size_meters, &az);

  invgp (NV_A0, NV_B0, misc->overlays[k][file_number].center.y, misc->overlays[k][file_number].center.x, 
	 misc->overlays[k][file_number].center.y, misc->overlays[k][file_number].center.x +
	 misc->overlays[k][file_number].x_bin_size_degrees, &misc->overlays[k][file_number].x_bin_size_meters, &az);


  //  Shorthand ;-)

  NV_FLOAT64 min_x = misc->displayed_area.min_x;
  NV_FLOAT64 min_y = misc->displayed_area.min_y;
  NV_FLOAT64 max_x = misc->displayed_area.max_x;
  NV_FLOAT64 max_y = misc->displayed_area.max_y;


  //  Set the bounds

  NV_F64_XYMBR mbr;
  NV_INT32 start_col, end_col, start_row, end_row;

  if (min_y < header.slat)
    {
      start_row = 0;
      mbr.min_y = header.slat;
    }
  else
    {
      start_row = (NV_INT32) ((min_y - header.slat) / misc->overlays[k][file_number].y_bin_size_degrees);
      mbr.min_y = min_y;
    }
  if (max_y > header.nlat)
    {
      end_row = misc->overlays[k][file_number].bin_height;
      mbr.max_y = header.nlat;
    }
  else
    {
      end_row = (NV_INT32) ((max_y - header.slat) / misc->overlays[k][file_number].y_bin_size_degrees);
      mbr.max_y = max_y;
    }

  if (min_x < header.wlon)
    {
      start_col = 0;
      mbr.min_x = header.wlon;
    }
  else
    {
      start_col = (NV_INT32) ((min_x - header.wlon) / misc->overlays[k][file_number].x_bin_size_degrees);
      mbr.min_x = min_x;
    }
  if (max_x > header.elon)
    {
      end_col = misc->overlays[k][file_number].bin_width;
      mbr.max_x = header.elon;
    }
  else
    {
      end_col = (NV_INT32) ((max_x - header.wlon) / misc->overlays[k][file_number].x_bin_size_degrees);
      mbr.max_x = max_x;
    }


  if (start_row < 0)
    {
      end_row = (NV_INT32) ((header.nlat - mbr.min_y) / misc->overlays[k][file_number].y_bin_size_degrees);
      start_row = 0;
    }

  if (end_row > misc->overlays[k][file_number].bin_height)
    {
      end_row = misc->overlays[k][file_number].bin_height;
      start_row = (NV_INT32) ((header.nlat - mbr.max_y) / misc->overlays[k][file_number].y_bin_size_degrees);
    }


  //  Number of rows and columns of the CHRTR file to draw.

  NV_INT32 rows = end_row - start_row;
  NV_INT32 cols = end_col - start_col;


  //  Allocate the needed arrays.

  NV_FLOAT32 *next_row = (NV_FLOAT32 *) calloc (cols + 1, sizeof (NV_FLOAT32));

  NV_FLOAT32 *current_row = (NV_FLOAT32 *) calloc (cols + 1, sizeof (NV_FLOAT32));

  if (current_row == NULL)
    {
      perror (areaCheck::tr ("Allocating current_row in chrtr.cpp").toAscii ());
      exit (-1);
    }


  //  Set the last column to CHRTRNULL so that hatchr won't freak.

  next_row[cols] = current_row[cols] = CHRTRNULL;


  //  Pre-read the data to get the min and max (since most disk systems cache this isn't much of a
  //  performance hit).

  NV_FLOAT32 min_z = CHRTRNULL;
  NV_FLOAT32 max_z = -CHRTRNULL;


  QProgressDialog prog (areaCheck::tr ("Loading..."), areaCheck::tr ("Cancel"), 0, rows, 0);

  prog.setWindowModality (Qt::WindowModal);
  prog.show ();

  qApp->processEvents();


  //  We only want to update the progress bar at about 20% increments.  This makes things go
  //  marginally faster.

  NV_INT32 prog_inc = rows / 5;
  if (!prog_inc) prog_inc = 1;


  for (NV_INT32 i = 0 ; i < rows ; i++)
    {
      if (!(i % prog_inc)) prog.setValue (i);

      qApp->processEvents();


      if (prog.wasCanceled ())
	{
	  free (current_row);
	  free (next_row);
	  close_chrtr (hnd);
	  return (NVFalse);
	}


      read_chrtr (hnd, start_row + i, start_col, cols, current_row);


      for (NV_INT32 j = 0 ; j < cols ; j++)
	{
	  if (current_row[j] < CHRTRNULL)
	    {
	      min_z = qMin (current_row[j], min_z);
	      max_z = qMax (current_row[j], max_z);
	    }
	}
    }

  prog.setValue (rows);


  //  Read input data row by row.

  for (NV_INT32 row = start_row ; row <= end_row ; row++)
    {
      //  Read data from disk.

      if (row == start_row)
	{
	  read_chrtr (hnd, row, start_col, cols, current_row);

	  memcpy (next_row, current_row, cols * sizeof (NV_FLOAT32));
	}
      else
	{
	  memcpy (current_row, next_row, cols * sizeof (NV_FLOAT32));
	  if (row < end_row) 
	    {
	      read_chrtr (hnd, row, start_col, cols, next_row);
	    }
	  else
	    {
	      read_chrtr (hnd, row, start_col, cols, current_row);
	    }
	}


      //  Sunshade.

      if (row < end_row)
	{
	  hatchr (k, file_number, map, options, misc, current_row, next_row, start_row, row, cols, min_z, max_z, mbr);
	}


      if (qApp->hasPendingEvents ())
	{
	  qApp->processEvents();
	  if (misc->drawing_canceled) break;
	}
    }


  //  Flush all of the saved fillRectangles from hatchr to screen

  map->flush ();


  //  Free allocated memory.

  free (next_row);
  free (current_row);


  close_chrtr (hnd);


  if (misc->drawing_canceled) return (NVFalse);


  return (NVTrue);
}
