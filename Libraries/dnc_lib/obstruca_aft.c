#include "obstruca_aft.h"


/********************************************************************

  Function:    swap_obstruca_aft

  Purpose:     Byte swap a obstruca_aft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The obstruca_aft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_obstruca_aft (OBSTRUCA_AFT *data)
{
  swap_int (&data->id);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->face_primitive_key);
}


/********************************************************************

  Function:    open_obstruca_aft

  Purpose:     Open a obstruc area feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_obstruca_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_obstruca_aft

  Purpose:     Close a obstruc area feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_obstruca_aft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_obstruca_aft

  Purpose:     Retrieve a obstruca_aft record from a obstruc area
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the obstruca_aft 
                                   record to be retrieved
               data           -    The returned obstruca_aft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_obstruca_aft (NV_INT32 hnd, NV_INT32 recnum, OBSTRUCA_AFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  trim_read (data->name, 30, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->face_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_obstruca_aft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_obstruca_aft

  Purpose:     Print to stderr the contents of a obstruca_aft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The obstruca_aft record

  Returns:     N/A

********************************************************************/

void dump_obstruca_aft (OBSTRUCA_AFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Face primitive key:         %d\n\n", data.face_primitive_key);
}
