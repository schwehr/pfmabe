#include "face.h"


/********************************************************************

  Function:    swap_face

  Purpose:     Byte swap a face record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The face record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_face (FACE *data)
{
  swap_int (&data->id);
  swap_int (&data->ring_primitive_key);
}


/********************************************************************

  Function:    open_face

  Purpose:     Open a face primitive table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_face (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_face

  Purpose:     Close a face primitive table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_face (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_face

  Purpose:     Retrieve a face record from a face primitive table
               file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the face 
                                   record to be retrieved
               data           -    The returned face record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_face (NV_INT32 hnd, NV_INT32 recnum, FACE *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  fread (&data->ring_primitive_key, sizeof (NV_INT32), 1, th->fp);


  if (th->swap) swap_face (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_face

  Purpose:     Print to stderr the contents of a face record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The face record

  Returns:     N/A

********************************************************************/

void dump_face (FACE data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "Ring primitive key:         %d\n", data.ring_primitive_key);
}
