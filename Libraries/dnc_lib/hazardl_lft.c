#include "hazardl_lft.h"


/********************************************************************

  Function:    swap_hazardl_lft

  Purpose:     Byte swap a hazardl_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hazardl_lft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_hazardl_lft (HAZARDL_LFT *data)
{
  swap_int (&data->id);
  swap_short (&data->existence_category);
  swap_short (&data->location_category);
  swap_short (&data->usage);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->edge_primitive_key);
}


/********************************************************************

  Function:    open_hazardl_lft

  Purpose:     Open a hazardl line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_hazardl_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_hazardl_lft

  Purpose:     Close a hazardl line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_hazardl_lft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_hazardl_lft

  Purpose:     Retrieve a hazardl_lft record from a hazardl line
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the hazardl_lft 
                                   record to be retrieved
               data           -    The returned hazardl_lft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_hazardl_lft (NV_INT32 hnd, NV_INT32 recnum, HAZARDL_LFT *data)
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
  fread (&data->existence_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->location_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->usage, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->edge_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_hazardl_lft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_hazardl_lft

  Purpose:     Print to stderr the contents of a hazardl_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hazardl_lft record

  Returns:     N/A

********************************************************************/

void dump_hazardl_lft (HAZARDL_LFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Existence category:         %d\n", data.existence_category);
  fprintf (stdout, "Location category:          %d\n", data.location_category);
  fprintf (stdout, "Usage:                      %d\n", data.usage);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Edge primitive key:         %d\n\n", data.edge_primitive_key);
}
