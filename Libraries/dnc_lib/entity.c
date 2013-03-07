#include "entity.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    swap_entity

  Purpose:     Byte swap an entity record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The entity record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_entity (ENTITY *data)
{
  swap_int (&data->id);
  swap_int (&data->containing_face);
  swap_float (&data->coordinate.lon);
  swap_float (&data->coordinate.lat);
}


/********************************************************************

  Function:    open_entity

  Purpose:     Open an entity primitive table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_entity (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_entity

  Purpose:     Close an entity primitive table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_entity (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_entity

  Purpose:     Retrieve an entity record from an entity primitive table
               file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the entity 
                                   record to be retrieved
               data           -    The returned entity record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_entity (NV_INT32 hnd, NV_INT32 recnum, ENTITY *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);


  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  fread (&data->containing_face, sizeof (NV_INT32), 1, th->fp);
  fread (&data->coordinate.lon, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->coordinate.lat, sizeof (NV_FLOAT32), 1, th->fp);

  if (th->swap) swap_entity (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_entity

  Purpose:     Print to stderr the contents of an entity record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The entity record

  Returns:     N/A

********************************************************************/

void dump_entity (ENTITY data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "Containing face:            %d\n", data.containing_face);
  fprintf (stdout, "Coordinate longitude:       %f\n", data.coordinate.lon);
  fprintf (stdout, "Coordinate latitude:        %f\n", data.coordinate.lat);
}
