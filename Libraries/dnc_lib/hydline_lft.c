#include "hydline_lft.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    swap_hydline_lft

  Purpose:     Byte swap a hydline_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hydline_lft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_hydline_lft (HYDLINE_LFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_float (&data->contour_value);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->edge_primitive_key);
}


/********************************************************************

  Function:    open_hydline_lft

  Purpose:     Open a hydline line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_hydline_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_hydline_lft

  Purpose:     Close a hydline line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_hydline_lft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_hydline_lft

  Purpose:     Retrieve a hydline_lft record from a hydline line
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the hydline_lft 
                                   record to be retrieved
               data           -    The returned hydline_lft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_hydline_lft (NV_INT32 hnd, NV_INT32 recnum, HYDLINE_LFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  if (recnum >= 0)
    {
      pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
      fseek (th->fp, pos, SEEK_SET);
    }

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->accuracy_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->contour_value, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->edge_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_hydline_lft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_hydline_lft

  Purpose:     Print to stderr the contents of a hydline_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hydline_lft record

  Returns:     N/A

********************************************************************/

void dump_hydline_lft (HYDLINE_LFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Accuracy category:          %d\n", data.accuracy_category);
  fprintf (stdout, "Contour value:              %f\n", data.contour_value);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Edge primitive key:         %d\n\n", data.edge_primitive_key);
}
