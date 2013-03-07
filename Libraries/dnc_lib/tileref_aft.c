#include "tileref_aft.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    swap_tileref_aft

  Purpose:     Byte swap a tileref_aft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The tileref_aft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_tileref_aft (TILEREF_AFT *data)
{
  swap_int (&data->id);
  swap_int (&data->face_primitive_key);
}


/********************************************************************

  Function:    open_tileref_aft

  Purpose:     Open a tileref area feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_tileref_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_tileref_aft

  Purpose:     Close a tileref area feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_tileref_aft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_tileref_aft

  Purpose:     Retrieve a tileref_aft record from a tileref area
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the tileref_aft 
                                   record to be retrieved
               data           -    The returned tileref_aft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_tileref_aft (NV_INT32 hnd, NV_INT32 recnum, TILEREF_AFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;
  NV_INT32 i;


  th = get_dnc_table_header (hnd);

  if (recnum >= 0)
    {
      pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
      fseek (th->fp, pos, SEEK_SET);
    }

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->tile_name, 8, th->fp);


  /*  Convert to lower case.  */

  for (i = 0 ; i < strlen (data->tile_name) ; i++) data->tile_name[i] = tolower (data->tile_name[i]);


  fread (&data->face_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_tileref_aft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_tileref_aft

  Purpose:     Print to stderr the contents of a tileref_aft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The tileref_aft record

  Returns:     N/A

********************************************************************/

void dump_tileref_aft (TILEREF_AFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "Tile name:                  %s\n", data.tile_name);
  fprintf (stdout, "Face primitive key:         %d\n\n", data.face_primitive_key);
}
