#include "navaid_pft.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    open_navaid_pft

  Purpose:     Open a navaid point feature table file.  This includes
               buoybcnp.pft, lightsp.pft, and markerp.pft files.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_navaid_pft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_navaid_pft

  Purpose:     Close a navaid point feature table file.  This includes
               buoybcnp.pft, lightsp.pft, and markerp.pft files.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_navaid_pft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/**************************************************/
/*              buoybcnp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_buoybcnp_pft

  Purpose:     Byte swap a buoybcnp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The buoybcnp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_buoybcnp_pft (BUOYBCNP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_int (&data->broadcast_frequency[0]);
  swap_int (&data->broadcast_frequency[1]);
  swap_short (&data->buoy_beacon_type);
  swap_short (&data->color_code);
  swap_short (&data->elevation);
  swap_short (&data->nominal_light_range);
  swap_short (&data->nav_system_type[0]);
  swap_short (&data->nav_system_type[1]);
  swap_short (&data->operating_range[0]);
  swap_short (&data->operating_range[1]);
  swap_float (&data->light_period);
  swap_short (&data->radar_reflector_attr);
  swap_short (&data->structure_shape_category);
  swap_short (&data->sound_signal_type);
  swap_short (&data->topmark_characteristic);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_buoybcnp_pft

  Purpose:     Retrieve a buoybcnp_pft record from a buoybcnp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the buoybcnp_pft 
                                   record to be retrieved
               data           -    The returned buoybcnp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_buoybcnp_pft (NV_INT32 hnd, NV_INT32 recnum, BUOYBCNP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);


  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->accuracy_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->broadcast_frequency[0], sizeof (NV_INT32), 1, th->fp);
  fread (&data->broadcast_frequency[1], sizeof (NV_INT32), 1, th->fp);
  fread (&data->buoy_beacon_type, sizeof (NV_INT16), 1, th->fp);


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL)
    {
      fread (&data->color_code, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->color_code = 0;
    }

  trim_read (data->character, 10, th->fp);


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL)
    {
      fread (&data->elevation, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->elevation = 0;
    }

  fread (&data->nominal_light_range, sizeof (NV_INT16), 1, th->fp);
  trim_read (data->multiple_light_ranges, 10,  th->fp);
  trim_read (data->name, 30, th->fp);
  fread (&data->nav_system_type[0], sizeof (NV_INT16), 1, th->fp);
  fread (&data->nav_system_type[1], sizeof (NV_INT16), 1, th->fp);
  fread (&data->operating_range[0], sizeof (NV_INT16), 1, th->fp);
  fread (&data->operating_range[1], sizeof (NV_INT16), 1, th->fp);
  fread (&data->light_period, sizeof (NV_FLOAT32), 1, th->fp);


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL)
    {
      fread (&data->radar_reflector_attr, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->radar_reflector_attr = 0;
    }

  fread (&data->structure_shape_category, sizeof (NV_INT16), 1, th->fp);


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL)
    {
      fread (&data->sound_signal_type, sizeof (NV_INT16), 1, th->fp);
      fread (&data->topmark_characteristic, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->sound_signal_type = 0;
      data->topmark_characteristic = 0;
    }
  trim_read (data->text, 255, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_buoybcnp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_buoybcnp_pft

  Purpose:     Print to stderr the contents of a buoybcnp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The buoybcnp_pft record

  Returns:     N/A

********************************************************************/

void dump_buoybcnp_pft (BUOYBCNP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Accuracy category:          %d\n", data.accuracy_category);
  fprintf (stdout, "Broadcast frequency 1:      %d\n", data.broadcast_frequency[0]);
  fprintf (stdout, "Broadcast frequency 2:      %d\n", data.broadcast_frequency[1]);
  fprintf (stdout, "Buoy/Beacon type:           %d\n", data.buoy_beacon_type);
  fprintf (stdout, "Color code:                 %d\n", data.color_code);
  fprintf (stdout, "Character of light:         %s\n", data.character);
  fprintf (stdout, "Elevation of light:         %d\n", data.elevation);
  fprintf (stdout, "Nominal light range:        %d\n", data.nominal_light_range);
  fprintf (stdout, "Multiple light ranges:      %s\n", data.multiple_light_ranges);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Nav system type 1:          %d\n", data.nav_system_type[0]);
  fprintf (stdout, "Nav system type 2:          %d\n", data.nav_system_type[1]);
  fprintf (stdout, "Operating range 1:          %d\n", data.operating_range[0]);
  fprintf (stdout, "Operating range 2:          %d\n", data.operating_range[1]);
  fprintf (stdout, "Light period:               %f\n", data.light_period);
  fprintf (stdout, "Radar reflector attribute:  %d\n", data.radar_reflector_attr);
  fprintf (stdout, "Structure shape category:   %d\n", data.structure_shape_category);
  fprintf (stdout, "Sound signal type:          %d\n", data.sound_signal_type);
  fprintf (stdout, "Topmark characteristic:     %d\n", data.topmark_characteristic);
  fprintf (stdout, "Text:                       %s\n", data.text);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              lightsp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_lightsp_pft

  Purpose:     Byte swap a lightsp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The lightsp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_lightsp_pft (LIGHTSP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_int (&data->broadcast_frequency[0]);
  swap_int (&data->broadcast_frequency[1]);
  swap_short (&data->color_code);
  swap_short (&data->elevation);
  swap_short (&data->hydrographic_light_type);
  swap_short (&data->IALA_aid_category);
  swap_short (&data->nominal_light_range);
  swap_short (&data->nav_system_type[0]);
  swap_short (&data->nav_system_type[1]);
  swap_float (&data->light_period);
  swap_short (&data->radar_reflector_attr);
  swap_short (&data->sound_signal_type);
  swap_short (&data->topmark_characteristic);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_lightsp_pft

  Purpose:     Retrieve a lightsp_pft record from a lightsp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the lightsp_pft 
                                   record to be retrieved
               data           -    The returned lightsp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_lightsp_pft (NV_INT32 hnd, NV_INT32 recnum, LIGHTSP_PFT *data)
{
  DNC_TABLE_HEADER *th;
  NV_INT32 pos;


  th = get_dnc_table_header (hnd);

  pos = recnum * th->reclen + th->header_length + sizeof (NV_INT32);
  fseek (th->fp, pos, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  trim_read (data->facc_code, 5, th->fp);
  fread (&data->accuracy_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->broadcast_frequency[0], sizeof (NV_INT32), 1, th->fp);
  fread (&data->broadcast_frequency[1], sizeof (NV_INT32), 1, th->fp);


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL)
    {
      fread (&data->color_code, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->color_code = 0;
    }

  trim_read (data->character, 10, th->fp);


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL)
    {
      fread (&data->elevation, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->elevation = 0;
    }

  fread (&data->hydrographic_light_type, sizeof (NV_INT16), 1, th->fp);


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL)
    {
      fread (&data->IALA_aid_category, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->IALA_aid_category = 0;
    }

  fread (&data->nominal_light_range, sizeof (NV_INT16), 1, th->fp);
  trim_read (data->multiple_light_ranges, 10, th->fp);
  trim_read (data->name, 30, th->fp);
  fread (&data->nav_system_type[0], sizeof (NV_INT16), 1, th->fp);
  fread (&data->nav_system_type[1], sizeof (NV_INT16), 1, th->fp);
  fread (&data->light_period, sizeof (NV_FLOAT32), 1, th->fp);


  /*  Not stored for GENERAL libraries.  */

  if (th->library_type != GENERAL)
    {
      fread (&data->radar_reflector_attr, sizeof (NV_INT16), 1, th->fp);
      fread (&data->sound_signal_type, sizeof (NV_INT16), 1, th->fp);
      fread (&data->topmark_characteristic, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->radar_reflector_attr = 0;
      data->sound_signal_type = 0;
      data->topmark_characteristic = 0;
    }

  trim_read (data->text, 255, th->fp);
  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_lightsp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_lightsp_pft

  Purpose:     Print to stderr the contents of a lightsp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The lightsp_pft record

  Returns:     N/A

********************************************************************/

void dump_lightsp_pft (LIGHTSP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Accuracy category:          %d\n", data.accuracy_category);
  fprintf (stdout, "Broadcast frequency 1:      %d\n", data.broadcast_frequency[0]);
  fprintf (stdout, "Broadcast frequency 2:      %d\n", data.broadcast_frequency[1]);
  fprintf (stdout, "Color code:                 %d\n", data.color_code);
  fprintf (stdout, "Character of light:         %s\n", data.character);
  fprintf (stdout, "Elevation of light:         %d\n", data.elevation);
  fprintf (stdout, "Hydrographic light type:    %d\n", data.hydrographic_light_type);
  fprintf (stdout, "IALA aid category:          %d\n", data.IALA_aid_category);
  fprintf (stdout, "Nominal light range:        %d\n", data.nominal_light_range);
  fprintf (stdout, "Multiple light ranges:      %s\n", data.multiple_light_ranges);
  fprintf (stdout, "Name:                       %s\n", data.name);
  fprintf (stdout, "Nav system type 1:          %d\n", data.nav_system_type[0]);
  fprintf (stdout, "Nav system type 2:          %d\n", data.nav_system_type[1]);
  fprintf (stdout, "Light period:               %f\n", data.light_period);
  fprintf (stdout, "Radar reflector attribute:  %d\n", data.radar_reflector_attr);
  fprintf (stdout, "Sound signal type:          %d\n", data.sound_signal_type);
  fprintf (stdout, "Topmark characteristic:     %d\n", data.topmark_characteristic);
  fprintf (stdout, "Text:                       %s\n", data.text);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}


/**************************************************/
/*              markerp.pft                      */
/**************************************************/


/********************************************************************

  Function:    swap_markerp_pft

  Purpose:     Byte swap a markerp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The markerp_pft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_markerp_pft (MARKERP_PFT *data)
{
  swap_int (&data->id);
  swap_short (&data->usage);
  swap_short (&data->tile_reference_identifier);
  swap_int (&data->entity_node_key);
}


/********************************************************************

  Function:    read_markerp_pft

  Purpose:     Retrieve a markerp_pft record from a markerp point
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the markerp_pft 
                                   record to be retrieved
               data           -    The returned markerp_pft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_markerp_pft (NV_INT32 hnd, NV_INT32 recnum, MARKERP_PFT *data)
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
      fread (&data->usage, sizeof (NV_INT16), 1, th->fp);
    }
  else
    {
      data->usage = 0;
    }

  fread (&data->tile_reference_identifier, sizeof (NV_INT16), 1, th->fp);
  fread (&data->entity_node_key, sizeof (NV_INT32), 1, th->fp);

  if (th->swap) swap_markerp_pft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_markerp_pft

  Purpose:     Print to stderr the contents of a markerp_pft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The markerp_pft record

  Returns:     N/A

********************************************************************/

void dump_markerp_pft (MARKERP_PFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Usage:                      %d\n", data.usage);
  fprintf (stdout, "Tile reference identifier:  %d\n", data.tile_reference_identifier);
  fprintf (stdout, "Entity node key:            %d\n\n", data.entity_node_key);
}
