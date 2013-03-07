#include "tunnell_lft.h"


/********************************************************************

  Function:    swap_tunnell_lft

  Purpose:     Byte swap a tunnell_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The tunnell_lft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_tunnell_lft (TUNNELL_LFT *data)
{
  swap_int (&data->id);
  swap_float (&data->hydrographic_depth);
  swap_short (&data->transportation_use_category);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->edge_primitive_key);
}


/********************************************************************

  Function:    open_tunnell_lft

  Purpose:     Open a tunnell line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_tunnell_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_tunnell_lft

  Purpose:     Close a tunnell line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_tunnell_lft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_tunnell_lft

  Purpose:     Retrieve a tunnell_lft record from a tunnell line
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the tunnell_lft 
                                   record to be retrieved
               data           -    The returned tunnell_lft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_tunnell_lft (NV_INT32 hnd, NV_INT32 recnum, TUNNELL_LFT *data)
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
  fread (&data->hydrographic_depth, sizeof (NV_FLOAT32), 1, th->fp);
  trim_read (data->name, 30, th->fp);
  fread (&data->transportation_use_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->edge_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_tunnell_lft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_tunnell_lft

  Purpose:     Print to stderr the contents of a tunnell_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The tunnell_lft record

  Returns:     N/A

********************************************************************/

void dump_tunnell_lft (TUNNELL_LFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Hydrographic depth:         %f\n", data.hydrographic_depth);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Transportation use category:%d\n", data.transportation_use_category);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Edge primitive key:         %d\n\n", data.edge_primitive_key);
}
