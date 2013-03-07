#include "dangera_aft.h"


/********************************************************************

  Function:    swap_dangera_aft

  Purpose:     Byte swap a dangera_aft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The dangera_aft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_dangera_aft (DANGERA_AFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_short (&data->date);
  swap_short (&data->existence_category);
  swap_short (&data->hydrographic_depth_height_info);
  swap_float (&data->hydrographic_depth);
  swap_short (&data->value);
  swap_short (&data->vertical_reference_category);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->face_primitive_key);
}


/********************************************************************

  Function:    open_dangera_aft

  Purpose:     Open a danger area feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_dangera_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_dangera_aft

  Purpose:     Close a danger area feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_dangera_aft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_dangera_aft

  Purpose:     Retrieve a dangera_aft record from a danger area
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the dangera_aft 
                                   record to be retrieved
               data           -    The returned dangera_aft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_dangera_aft (NV_INT32 hnd, NV_INT32 recnum, DANGERA_AFT *data)
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
  fread (&data->hydrographic_depth_height_info, sizeof (NV_INT16), 1, th->fp);
  fread (&data->hydrographic_depth, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->value, sizeof (NV_INT16), 1, th->fp);
  fread (&data->vertical_reference_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->face_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_dangera_aft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_dangera_aft

  Purpose:     Print to stderr the contents of a dangera_aft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The dangera_aft record

  Returns:     N/A

********************************************************************/

void dump_dangera_aft (DANGERA_AFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Accuracy category:          %d\n", data.accuracy_category);
  fprintf (stdout, "Date:                       %d\n", data.date);
  fprintf (stdout, "Existence category:         %d\n", data.existence_category);
  fprintf (stdout, "Hydrographic dep/hgt info:  %d\n", data.hydrographic_depth_height_info);
  fprintf (stdout, "Hydrographic depth:         %f\n", data.hydrographic_depth);
  fprintf (stdout, "Value:                      %d\n", data.value);
  fprintf (stdout, "Vertical ref. category:     %d\n", data.vertical_reference_category);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Face primitive key:         %d\n\n", data.face_primitive_key);
}
