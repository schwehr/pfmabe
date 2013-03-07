#include "soundp_pft.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    swap_soundp_pft

  Purpose:     Byte swap a soundp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The soundp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_soundp_pft (SOUNDP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_short (&data->date);
  swap_short (&data->existence_category);
  swap_float (&data->hydrographic_drying_height);
  swap_float (&data->hydrographic_depth);
  swap_short (&data->sounding_category);
  swap_short (&data->sounding_velocity);
  swap_short (&data->value);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    open_soundp_pft

  Purpose:     Open a soundp point feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_soundp_pft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_soundp_pft

  Purpose:     Close a soundp point feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_soundp_pft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_soundp_pft

  Purpose:     Retrieve a soundp_pft record from a soundp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the soundp_pft 
                                   record to be retrieved
               data           -    The returned soundp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_soundp_pft (NV_INT32 hnd, NV_INT32 recnum, SOUNDP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->accuracy_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->date, sizeof (NV_INT16), 1, th->fp);
  fread (&data->existence_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->hydrographic_drying_height, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->hydrographic_depth, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->sounding_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->sounding_velocity, sizeof (NV_INT16), 1, th->fp);
  fread (&data->value, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_soundp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_soundp_pft

  Purpose:     Print to stderr the contents of a soundp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The soundp_pft record

  Returns:     N/A

********************************************************************/

void dump_soundp_pft (SOUNDP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Accuracy category:          %d\n", data.accuracy_category);
  fprintf (stdout, "Date:                       %d\n", data.date);
  fprintf (stdout, "Existence category:         %d\n", data.existence_category);
  fprintf (stdout, "Hydrographic drying height: %f\n", data.hydrographic_drying_height);
  fprintf (stdout, "Hydrographic depth:         %f\n", data.hydrographic_depth);
  fprintf (stdout, "Sounding category:          %d\n", data.sounding_category);
  fprintf (stdout, "Sounding velocity:          %d\n", data.sounding_velocity);
  fprintf (stdout, "Value:                      %d\n", data.value);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}
