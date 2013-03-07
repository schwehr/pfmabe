#include "ring.h"


/********************************************************************

  Function:    swap_ring

  Purpose:     Byte swap a ring record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The ring record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_ring (RING *data)
{
  swap_int (&data->id);
  swap_int (&data->face_key);
  swap_int (&data->edge_primitive_key);
}


/********************************************************************

  Function:    open_ring

  Purpose:     Open a ring primitive table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_ring (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_ring

  Purpose:     Close a ring primitive table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_ring (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_ring

  Purpose:     Retrieve a ring record from a ring primitive table
               file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the ring 
                                   record to be retrieved
               data           -    The returned ring record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_ring (NV_INT32 hnd, NV_INT32 recnum, RING *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  fread (&data->face_key, sizeof (NV_INT32), 1, th->fp);
  fread (&data->edge_primitive_key, sizeof (NV_INT32), 1, th->fp);


  if (th->swap) swap_ring (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_ring

  Purpose:     Print to stderr the contents of a ring record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The ring record

  Returns:     N/A

********************************************************************/

void dump_ring (RING data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "Face key:                   %d\n", data.face_key);
  fprintf (stdout, "Edge primitive key:         %d\n", data.edge_primitive_key);
}
