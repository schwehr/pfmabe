#include "coapoint_pft.h"


/********************************************************************

  Function:    swap_coapoint_pft

  Purpose:     Byte swap a coapoint_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The coapoint_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_coapoint_pft (COAPOINT_PFT *data)
{
  swap_int (&data->id);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    open_coapoint_pft

  Purpose:     Open a coapoint point feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_coapoint_pft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_coapoint_pft

  Purpose:     Close a coapoint point feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_coapoint_pft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_coapoint_pft

  Purpose:     Retrieve a coapoint_pft record from a coapoint point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the coapoint_pft 
                                   record to be retrieved
               data           -    The returned coapoint_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_coapoint_pft (NV_INT32 hnd, NV_INT32 recnum, COAPOINT_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  trim_read (data->name, 30, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_coapoint_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_coapoint_pft

  Purpose:     Print to stderr the contents of a coapoint_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The coapoint_pft record

  Returns:     N/A

********************************************************************/

void dump_coapoint_pft (COAPOINT_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}
