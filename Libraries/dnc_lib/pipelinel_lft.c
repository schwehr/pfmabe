#include "pipelinel_lft.h"


/********************************************************************

  Function:    swap_pipelinel_lft

  Purpose:     Byte swap a pipelinel_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The pipelinel_lft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_pipelinel_lft (PIPELINEL_LFT *data)
{
  swap_int (&data->id);
  swap_float (&data->depth_below_surface_level);
  swap_short (&data->existence_category);
  swap_float (&data->height_above_sea_bottom);
  swap_short (&data->location_category);
  swap_float (&data->overhead_clearance_category);
  swap_short (&data->over_water_obstruction);
  swap_short (&data->pipeline_type);
  swap_short (&data->product_category);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->edge_primitive_key);
}


/********************************************************************

  Function:    open_pipelinel_lft

  Purpose:     Open a pipelinel line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_pipelinel_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_pipelinel_lft

  Purpose:     Close a pipelinel line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_pipelinel_lft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_pipelinel_lft

  Purpose:     Retrieve a pipelinel_lft record from a pipelinel line
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the pipelinel_lft 
                                   record to be retrieved
               data           -    The returned pipelinel_lft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_pipelinel_lft (NV_INT32 hnd, NV_INT32 recnum, PIPELINEL_LFT *data)
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
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->depth_below_surface_level, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->existence_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->height_above_sea_bottom, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->location_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->overhead_clearance_category, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->over_water_obstruction, sizeof (NV_INT16), 1, th->fp);
  fread (&data->pipeline_type, sizeof (NV_INT16), 1, th->fp);
  fread (&data->product_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->edge_primitive_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_pipelinel_lft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_pipelinel_lft

  Purpose:     Print to stderr the contents of a pipelinel_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The pipelinel_lft record

  Returns:     N/A

********************************************************************/

void dump_pipelinel_lft (PIPELINEL_LFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Depth below surface level:  %f\n", data.depth_below_surface_level);
  fprintf (stdout, "Existence category:         %d\n", data.existence_category);
  fprintf (stdout, "Height above sea bottom:    %f\n", data.height_above_sea_bottom);
  fprintf (stdout, "Location category:          %d\n", data.location_category);
  fprintf (stdout, "Overhead clearance category:%f\n", data.overhead_clearance_category);
  fprintf (stdout, "Over water obstruction:     %d\n", data.over_water_obstruction);
  fprintf (stdout, "Pipeline type:              %d\n", data.pipeline_type);
  fprintf (stdout, "Product category:           %d\n", data.product_category);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Edge primitive key:         %d\n\n", data.edge_primitive_key);
}
