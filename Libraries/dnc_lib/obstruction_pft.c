#include "obstruction_pft.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    open_obstruction_pft

  Purpose:     Open an obstruction point feature table file.  This
               includes dangerp.pft, hazardp.pft, loadingp.pft,
               obstrucp.pft, ruinsp.pft, berthp.pft, callinp.pft,
               landingp.pft, mooringp.pft, and relpoint.pft files.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_obstruction_pft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_obstruction_pft

  Purpose:     Close an obstruction point feature table file.  This
               includes dangerp.pft, hazardp.pft, loadingp.pft,
               obstrucp.pft, ruinsp.pft, berthp.pft, callinp.pft,
               landingp.pft, mooringp.pft, and relpoint.pft files.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_obstruction_pft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/**************************************************/
/*              dangerp.pft                       */
/**************************************************/


/********************************************************************

  Function:    swap_dangerp_pft

  Purpose:     Byte swap a dangerp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The dangerp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_dangerp_pft (DANGERP_PFT *data)
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
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_dangerp_pft

  Purpose:     Retrieve a dangerp_pft record from a dangerp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the dangerp_pft 
                                   record to be retrieved
               data           -    The returned dangerp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_dangerp_pft (NV_INT32 hnd, NV_INT32 recnum, DANGERP_PFT *data)
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


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL) 
    {
      fread (&data->vertical_reference_category, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->vertical_reference_category = 0;
    }

  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_dangerp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_dangerp_pft

  Purpose:     Print to stderr the contents of a dangerp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The dangerp_pft record

  Returns:     N/A

********************************************************************/

void dump_dangerp_pft (DANGERP_PFT data)
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
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              hazardp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_hazardp_pft

  Purpose:     Byte swap a hazardp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hazardp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_hazardp_pft (HAZARDP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_short (&data->certainty);
  swap_short (&data->date);
  swap_short (&data->existence_category);
  swap_float (&data->hydrographic_drying_height);
  swap_short (&data->hydrographic_depth_height_info);
  swap_float (&data->hydrographic_depth);
  swap_short (&data->location_category);
  swap_short (&data->material_composition_category);
  swap_short (&data->seafloor_feature_category);
  swap_short (&data->severity);
  swap_short (&data->value);
  swap_short (&data->vertical_reference_category);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_hazardp_pft

  Purpose:     Retrieve a hazardp_pft record from a hazardp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the hazardp_pft 
                                   record to be retrieved
               data           -    The returned hazardp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_hazardp_pft (NV_INT32 hnd, NV_INT32 recnum, HAZARDP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->accuracy_category, sizeof (NV_INT16), 1, th->fp);


  /*  Not stored for COASTAL or GENERAL libraries.  */

  if (th->library_type < COASTAL)
    {
      fread (&data->certainty, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->certainty = 0;
    }

  fread (&data->date, sizeof (NV_INT16), 1, th->fp);
  fread (&data->existence_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->hydrographic_drying_height, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->hydrographic_depth_height_info, sizeof (NV_INT16), 1, th->fp);
  fread (&data->hydrographic_depth, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->location_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->material_composition_category, sizeof (NV_INT16), 1, th->fp);
  trim_read (data->name, 30, th->fp);
  fread (&data->seafloor_feature_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->severity, sizeof (NV_INT16), 1, th->fp);
  trim_read (data->text, 255, th->fp);
  fread (&data->value, sizeof (NV_INT16), 1, th->fp);
  fread (&data->vertical_reference_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_hazardp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_hazardp_pft

  Purpose:     Print to stderr the contents of a hazardp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hazardp_pft record

  Returns:     N/A

********************************************************************/

void dump_hazardp_pft (HAZARDP_PFT data)
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
  fprintf (stdout, "Location category:          %d\n", data.location_category);
  fprintf (stdout, "Material composition cat.:  %d\n", data.material_composition_category);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Seafloor feature category:  %d\n", data.seafloor_feature_category);
  fprintf (stdout, "Severity:                   %d\n", data.severity);
  fprintf (stdout, "Text:                       %s\n", data.text);
  fprintf (stdout, "Value:                      %d\n", data.value);
  fprintf (stdout, "Vertical ref. category:     %d\n", data.vertical_reference_category);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              loadingp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_loadingp_pft

  Purpose:     Byte swap a loadingp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The loadingp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_loadingp_pft (LOADINGP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->light_characteristic_category);
  swap_short (&data->nav_system_type);
  swap_short (&data->sound_signal_type);
  swap_short (&data->usage);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_loadingp_pft

  Purpose:     Retrieve a loadingp_pft record from a loadingp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the loadingp_pft 
                                   record to be retrieved
               data           -    The returned loadingp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_loadingp_pft (NV_INT32 hnd, NV_INT32 recnum, LOADINGP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->light_characteristic_category, sizeof (NV_INT16), 1, th->fp);
  trim_read (data->name, 30, th->fp);
  fread (&data->nav_system_type, sizeof (NV_INT16), 1, th->fp);


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL)
    {
      fread (&data->sound_signal_type, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->sound_signal_type = 0;
    }

  fread (&data->usage, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_loadingp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_loadingp_pft

  Purpose:     Print to stderr the contents of a loadingp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The loadingp_pft record

  Returns:     N/A

********************************************************************/

void dump_loadingp_pft (LOADINGP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Light characteristic cat.:  %d\n", data.light_characteristic_category);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Nav system type:            %d\n", data.nav_system_type);
  fprintf (stdout, "Sound signal type:          %d\n", data.sound_signal_type);
  fprintf (stdout, "Usage:                      %d\n", data.usage);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              obstrucp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_obstrucp_pft

  Purpose:     Byte swap an obstrucp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The obstrucp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_obstrucp_pft (OBSTRUCP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_obstrucp_pft

  Purpose:     Retrieve an obstrucp_pft record from an obstrucp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the obstrucp_pft 
                                   record to be retrieved
               data           -    The returned obstrucp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_obstrucp_pft (NV_INT32 hnd, NV_INT32 recnum, OBSTRUCP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_obstrucp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_obstrucp_pft

  Purpose:     Print to stderr the contents of an obstrucp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The obstrucp_pft record

  Returns:     N/A

********************************************************************/

void dump_obstrucp_pft (OBSTRUCP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              ruinsp.pft                        */
/**************************************************/


/********************************************************************

  Function:    swap_ruinsp_pft

  Purpose:     Byte swap a ruinsp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The ruinsp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_ruinsp_pft (RUINSP_PFT *data)
{
  swap_int (&data->id);
  swap_float (&data->hydrographic_drying_height);
  swap_short (&data->hydrographic_depth_height_info);
  swap_float (&data->hydrographic_depth);
  swap_short (&data->location_category);
  swap_short (&data->vertical_reference_category);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_ruinsp_pft

  Purpose:     Retrieve a ruinsp_pft record from a ruinsp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the ruinsp_pft 
                                   record to be retrieved
               data           -    The returned ruinsp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_ruinsp_pft (NV_INT32 hnd, NV_INT32 recnum, RUINSP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);


  /*  Only stored for HARBOR libraries.  */

  if (th->library_type == HARBOR)
    {
      fread (&data->hydrographic_drying_height, sizeof (NV_FLOAT32), 1, th->fp);
    }
  else
    {
      data->hydrographic_drying_height = 0;
    }

  fread (&data->hydrographic_depth_height_info, sizeof (NV_INT16), 1, th->fp);
  fread (&data->hydrographic_depth, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->location_category, sizeof (NV_INT16), 1, th->fp);


  /*  Only stored for HARBOR libraries.  */

  if (th->library_type == HARBOR)
    {
      trim_read (data->name, 30, th->fp);
    }
  else
    {
      strcpy (data->name, "");
    }

  fread (&data->vertical_reference_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_ruinsp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_ruinsp_pft

  Purpose:     Print to stderr the contents of a ruinsp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The ruinsp_pft record

  Returns:     N/A

********************************************************************/

void dump_ruinsp_pft (RUINSP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Hydrographic drying height: %f\n", data.hydrographic_drying_height);
  fprintf (stdout, "Hydrographic dep/hgt info:  %d\n", data.hydrographic_depth_height_info);
  fprintf (stdout, "Hydrographic depth:         %f\n", data.hydrographic_depth);
  fprintf (stdout, "Location category:          %d\n", data.location_category);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Vertical ref. category:     %d\n", data.vertical_reference_category);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              berthp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_berthp_pft

  Purpose:     Byte swap a berthp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The berthp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_berthp_pft (BERTHP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_berthp_pft

  Purpose:     Retrieve a berthp_pft record from a berthp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the berthp_pft 
                                   record to be retrieved
               data           -    The returned berthp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_berthp_pft (NV_INT32 hnd, NV_INT32 recnum, BERTHP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  trim_read (data->berth_identifier, 25, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_berthp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_berthp_pft

  Purpose:     Print to stderr the contents of a berthp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The berthp_pft record

  Returns:     N/A

********************************************************************/

void dump_berthp_pft (BERTHP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Berth identifier:           %s\n", data.berth_identifier);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              callinp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_callinp_pft

  Purpose:     Byte swap a callinp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The callinp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_callinp_pft (CALLINP_PFT *data)
{
  NV_INT32 i;


  swap_int (&data->id);
  for (i = 0 ; i < 4 ; i++) swap_short (&data->traffic_direction[i]);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_callinp_pft

  Purpose:     Retrieve a callinp_pft record from a callinp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the callinp_pft 
                                   record to be retrieved
               data           -    The returned callinp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_callinp_pft (NV_INT32 hnd, NV_INT32 recnum, CALLINP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos, i;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  for (i = 0 ; i < 4 ; i++) fread (&data->traffic_direction[i], sizeof (NV_INT16), 1, th->fp);
  trim_read (data->name, 30, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_callinp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_callinp_pft

  Purpose:     Print to stderr the contents of a callinp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The callinp_pft record

  Returns:     N/A

********************************************************************/

void dump_callinp_pft (CALLINP_PFT data)
{
  NV_INT32 i;


  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  for (i = 0 ; i < 4 ; i++) fprintf (stdout, "Traffic direction:          %d\n", data.traffic_direction[i]);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              landingp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_landingp_pft

  Purpose:     Byte swap a landingp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The landingp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_landingp_pft (LANDINGP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->certainty);
  swap_short (&data->existence_category);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_landingp_pft

  Purpose:     Retrieve a landingp_pft record from a landingp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the landingp_pft 
                                   record to be retrieved
               data           -    The returned landingp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_landingp_pft (NV_INT32 hnd, NV_INT32 recnum, LANDINGP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->certainty, sizeof (NV_INT16), 1, th->fp);
  fread (&data->existence_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_landingp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_landingp_pft

  Purpose:     Print to stderr the contents of a landingp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The landingp_pft record

  Returns:     N/A

********************************************************************/

void dump_landingp_pft (LANDINGP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Certainty:                  %d\n", data.certainty);
  fprintf (stdout, "Existence category:         %d\n", data.existence_category);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              mooringp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_mooringp_pft

  Purpose:     Byte swap a mooringp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The mooringp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_mooringp_pft (MOORINGP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_mooringp_pft

  Purpose:     Retrieve a mooringp_pft record from a mooringp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the mooringp_pft 
                                   record to be retrieved
               data           -    The returned mooringp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_mooringp_pft (NV_INT32 hnd, NV_INT32 recnum, MOORINGP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_mooringp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_mooringp_pft

  Purpose:     Print to stderr the contents of a mooringp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The mooringp_pft record

  Returns:     N/A

********************************************************************/

void dump_mooringp_pft (MOORINGP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}



/**************************************************/
/*              relpoint.pft                       */
/**************************************************/


/********************************************************************

  Function:    swap_relpoint_pft

  Purpose:     Byte swap a relpoint_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The relpoint_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_relpoint_pft (RELPOINT_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_short (&data->elevation_accuracy);
  swap_int (&data->highest_z_value);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_relpoint_pft

  Purpose:     Retrieve a relpoint_pft record from a relpoint point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the relpoint_pft 
                                   record to be retrieved
               data           -    The returned relpoint_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_relpoint_pft (NV_INT32 hnd, NV_INT32 recnum, RELPOINT_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);


  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);


  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->accuracy_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->elevation_accuracy, sizeof (NV_INT16), 1, th->fp);
  fread (&data->highest_z_value, sizeof (NV_INT32), 1, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_relpoint_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_relpoint_pft

  Purpose:     Print to stderr the contents of a relpoint_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The relpoint_pft record

  Returns:     N/A

********************************************************************/

void dump_relpoint_pft (RELPOINT_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Accuracy category:          %d\n", data.accuracy_category);
  fprintf (stdout, "Elevation accuracy:         %d\n", data.elevation_accuracy);
  fprintf (stdout, "Highest Z value:            %d\n", data.highest_z_value);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}
