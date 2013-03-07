#include "coastl_lft.h"


/********************************************************************

  Function:    swap_coastl_lft

  Purpose:     Byte swap a coastl_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The coastl_lft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_coastl_lft (COASTL_LFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_short (&data->shoreline_type_category);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->edge_primitive_key);
}


/********************************************************************

  Function:    open_coastl_lft

  Purpose:     Open a coastl line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_coastl_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_coastl_lft

  Purpose:     Close a coastl line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_coastl_lft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_coastl_lft

  Purpose:     Retrieve a coastl_lft record from a coastl line
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the coastl_lft 
                                   record to be retrieved
               data           -    The returned coastl_lft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_coastl_lft (NV_INT32 hnd, NV_INT32 recnum, COASTL_LFT *data)
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
  fread (&data->shoreline_type_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->edge_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_coastl_lft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_coastl_lft

  Purpose:     Print to stderr the contents of a coastl_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The coastl_lft record

  Returns:     N/A

********************************************************************/

void dump_coastl_lft (COASTL_LFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Accuracy category:          %d\n", data.accuracy_category);
  fprintf (stdout, "Shoreline type category:    %d\n", data.shoreline_type_category);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Edge primitive key:         %d\n\n", data.edge_primitive_key);
}
