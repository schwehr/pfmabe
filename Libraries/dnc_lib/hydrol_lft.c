#include "hydrol_lft.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    swap_hydrol_lft

  Purpose:     Byte swap a hydrol_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hydrol_lft record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_hydrol_lft (HYDROL_LFT *data)
{
  swap_int (&data->id);
  swap_short (&data->accuracy_category);
  swap_float (&data->contour_value);
  swap_short (&data->hypsography_portrayal_category);
}


/********************************************************************

  Function:    open_hydrol_lft

  Purpose:     Open a hydrol line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_hydrol_lft (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_hydrol_lft

  Purpose:     Close a hydrol line feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_hydrol_lft (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_hydrol_lft

  Purpose:     Retrieve a hydrol_lft record from a hydrol line
               feature table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the hydrol_lft 
                                   record to be retrieved
               data           -    The returned hydrol_lft record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_hydrol_lft (NV_INT32 hnd, NV_INT32 recnum, HYDROL_LFT *data)
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
  fread (&data->accuracy_category, sizeof (NV_INT16), 1, th->fp);
  fread (&data->contour_value, sizeof (NV_FLOAT32), 1, th->fp);
  fread (&data->hypsography_portrayal_category, sizeof (NV_INT16), 1, th->fp);

  if (th->swap) swap_hydrol_lft (data);

  return (NVTrue);
}


/********************************************************************

  Function:    dump_hydrol_lft

  Purpose:     Print to stderr the contents of a hydrol_lft record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The hydrol_lft record

  Returns:     N/A

********************************************************************/

void dump_hydrol_lft (HYDROL_LFT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "FACC code:                  %s\n", data.facc_code);
  fprintf (stdout, "Accuracy category:          %d\n", data.accuracy_category);
  fprintf (stdout, "Contour value:              %f\n", data.contour_value);
  fprintf (stdout, "Hypsography portrayal cat.: %d\n", data.hypsography_portrayal_category);
}
