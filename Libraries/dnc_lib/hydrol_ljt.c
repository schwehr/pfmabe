#include "hydrol_ljt.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    swap_hydrol_ljt

  Purpose:     Byte swap a hydrol_ljt record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hydrol_ljt record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_hydrol_ljt (HYDROL_LJT *data)
{
  swap_int (&data->id);
  swap_int (&data->feature_key);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->edge_primitive_key);
}


/********************************************************************

  Function:    open_hydrol_ljt

  Purpose:     Open a hydrol line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_hydrol_ljt (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_hydrol_ljt

  Purpose:     Close a hydrol line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_hydrol_ljt (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_hydrol_ljt

  Purpose:     Retrieve a hydrol_ljt record from a hydrol line
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the hydrol_ljt 
                                   record to be retrieved
               data           -    The returned hydrol_ljt record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_hydrol_ljt (NV_INT32 hnd, NV_INT32 recnum, HYDROL_LJT *data)
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
  fread (&data->feature_key, sizeof (NV_INT32), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->edge_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_hydrol_ljt (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_hydrol_ljt

  Purpose:     Print to stderr the contents of a hydrol_ljt record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hydrol_ljt record

  Returns:     N/A

********************************************************************/

void dump_hydrol_ljt (HYDROL_LJT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "Feature key:                %d\n", data.feature_key);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Edge primitive key:         %d\n\n", data.edge_primitive_key);
}
