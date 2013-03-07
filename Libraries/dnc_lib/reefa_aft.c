#include "reefa_aft.h"


/********************************************************************

  Function:    swap_reefa_aft

  Purpose:     Byte swap a reefa_aft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The reefa_aft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_reefa_aft (REEFA_AFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_short (&data->certainty);
  swap_short (&data->date);
  swap_short (&data->existence_category);
  swap_float (&data->hydrographic_drying_height);
  swap_short (&data->hydrographic_depth_height_info);
  swap_float (&data->hydrographic_depth);
  swap_short (&data->material_composition_category);
  swap_short (&data->severity);
  swap_short (&data->value);
  swap_short (&data->vertical_reference_category);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->face_primitive_key);
}


/********************************************************************

  Function:    open_reefa_aft

  Purpose:     Open a reef area feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_reefa_aft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_reefa_aft

  Purpose:     Close a reef area feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_reefa_aft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_reefa_aft

  Purpose:     Retrieve a reefa_aft record from a reef area
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the reefa_aft 
                                   record to be retrieved
               data           -    The returned reefa_aft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_reefa_aft (NV_INT32 hnd, NV_INT32 recnum, REEFA_AFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->accuracy_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->certainty, sizeof (NV_INT16), 1, th->fp);
  fread (&data->date, sizeof (NV_INT16), 1, th->fp);
  fread (&data->existence_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->hydrographic_drying_height, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->hydrographic_depth_height_info, sizeof (NV_INT16), 1, th->fp);
  fread (&data->hydrographic_depth, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->material_composition_category, sizeof (NV_INT16), 1, th->fp);
  trim_read (data->name, 30, th->fp);
  fread (&data->severity, sizeof (NV_INT16), 1, th->fp);
  fread (&data->value, sizeof (NV_INT16), 1, th->fp);
  fread (&data->vertical_reference_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->face_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_reefa_aft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_reefa_aft

  Purpose:     Print to stderr the contents of a reefa_aft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The reefa_aft record

  Returns:     N/A

********************************************************************/

void dump_reefa_aft (REEFA_AFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Accuracy category:          %d\n", data.accuracy_category);
  fprintf (stdout, "Certainty:                  %d\n", data.certainty);
  fprintf (stdout, "Date:                       %d\n", data.date);
  fprintf (stdout, "Existence category:         %d\n", data.existence_category);
  fprintf (stdout, "Hydrographic drying height: %f\n", data.hydrographic_drying_height);
  fprintf (stdout, "Hydrographic dep/hgt info:  %d\n", data.hydrographic_depth_height_info);
  fprintf (stdout, "Hydrographic depth:         %f\n", data.hydrographic_depth);
  fprintf (stdout, "Material composition cat.:  %d\n", data.material_composition_category);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Severity:                   %d\n", data.severity);
  fprintf (stdout, "Value:                      %d\n", data.value);
  fprintf (stdout, "Vertical ref. category:     %d\n", data.vertical_reference_category);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Face primitive key:         %d\n\n", data.face_primitive_key);
}
