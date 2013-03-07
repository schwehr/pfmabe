#include "lat.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    swap_lat

  Purpose:     Byte swap a lat record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The lat record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_lat (LAT *data)
{
  swap_int (&data->id);
  swap_float (&data->sw_corner_lon);
  swap_float (&data->sw_corner_lat);
  swap_float (&data->ne_corner_lon);
  swap_float (&data->ne_corner_lat);
}


/********************************************************************

  Function:    open_lat

  Purpose:     Open a soundp point feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_lat (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table_no_type_check (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_lat

  Purpose:     Close a soundp point feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_lat (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_lat

  Purpose:     Retrieve a lat record from a soundp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the lat 
                                   record to be retrieved
               data           -    The returned lat record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_lat (NV_INT32 hnd, NV_INT32 recnum, LAT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->library_name, 8, th->fp);
  fread (&data->sw_corner_lon, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->sw_corner_lat, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->ne_corner_lon, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->ne_corner_lat, sizeof (NV_FLOAT32), 1, th->fp);

  if (th->swap) swap_lat (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_lat

  Purpose:     Print to stderr the contents of a lat record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The lat record

  Returns:     N/A

********************************************************************/

void dump_lat (LAT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "Library name:               %s\n", data.library_name);
  fprintf (stdout, "SW corner longitude:        %f\n", data.sw_corner_lon);
  fprintf (stdout, "SW corner latitude:         %f\n", data.sw_corner_lat);
  fprintf (stdout, "NE corner longitude:        %f\n", data.ne_corner_lon);
  fprintf (stdout, "NE corner latitude:         %f\n", data.ne_corner_lat);
}
