#include "edge.h"
#include "variable_index.h"

/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    swap_edge

  Purpose:     Byte swap an edge record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The edge record to be swapped

  Returns:     N/A

********************************************************************/

static void swap_edge (EDGE *data)
{
  NV_INT32 i;


  swap_int (&data->id);
  swap_int (&data->start_node);
  swap_int (&data->end_node);


  /*  Sometimes it's an int, sometimes it's a triplet, more brain damage.  */

  swap_triplet (&data->right_face_t);
  swap_int (&data->right_face_i);
  swap_triplet (&data->left_face_t);
  swap_int (&data->left_face_i);
  swap_triplet (&data->right_edge_t);
  swap_int (&data->right_edge_i);
  swap_triplet (&data->left_edge_t);
  swap_int (&data->left_edge_i);


  for (i = 0 ; i < data->count ; i++)
    {
      swap_float (&data->coordinate[i].lon);
      swap_float (&data->coordinate[i].lat);
    }
}


/********************************************************************

  Function:    open_edge

  Purpose:     Open an edge primitive table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_edge (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_edge

  Purpose:     Close an edge primitive table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_edge (NV_INT32 hnd)
{
  close_dnc_table (hnd);
}


/********************************************************************

  Function:    read_edge

  Purpose:     Retrieve an edge record from an edge primitive table
               file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               recnum         -    The record number of the edge 
                                   record to be retrieved
               data           -    The returned edge record

  Returns:     NV_BOOL        -    NVFalse on error, else NVTrue

********************************************************************/

NV_BOOL read_edge (NV_INT32 hnd, NV_INT32 recnum, EDGE *data)
{
  DNC_TABLE_HEADER *th;
  INDEX_ADDRESS pos;
  NV_INT32 i;


  th = get_dnc_table_header (hnd);


  /*  I have no idea why this happens, but it does!  */

  if (recnum > 10000000) return (NVFalse);


  pos = get_dnc_variable_index_address (hnd, recnum);
  if (pos.offset < 0) return (NVFalse);

  fseek (th->fp, pos.offset, SEEK_SET);

  if ((fread (&data->id, sizeof (NV_INT32), 1, th->fp)) == 0) return (NVFalse);
  fread (&data->start_node, sizeof (NV_INT32), 1, th->fp);
  fread (&data->end_node, sizeof (NV_INT32), 1, th->fp);


  /*  Sometimes it's an int, sometimes it's a triplet, more brain damage.  */

  if (th->rh[3].field_type == 'K')
    {
      read_triplet (&data->right_face_t, th->fp);
    }
  else
    {
      fread (&data->right_face_i, sizeof (NV_INT32), 1, th->fp);
    }

  if (th->rh[4].field_type == 'K')
    {
      read_triplet (&data->left_face_t, th->fp);
    }
  else
    {
      fread (&data->left_face_i, sizeof (NV_INT32), 1, th->fp);
    }

  if (th->rh[5].field_type == 'K')
    {
      read_triplet (&data->right_edge_t, th->fp);
    }
  else
    {
      fread (&data->right_edge_i, sizeof (NV_INT32), 1, th->fp);
    }

  if (th->rh[6].field_type == 'K')
    {
      read_triplet (&data->left_edge_t, th->fp);
    }
  else
    {
      fread (&data->left_edge_i, sizeof (NV_INT32), 1, th->fp);
    }


  /*  This 4 byte integer count value is not documented in the DNC documentation nor does it show up in
      the edg header.  */

  fread (&data->count, sizeof (NV_INT32), 1, th->fp);
  if (th->swap) swap_int (&data->count);


  /*  Sanity check!  */

  if (data->count < 0 || data->count > 100000) return (NVFalse);


  data->coordinate = (NV_F32_POS *) calloc (data->count, sizeof (NV_F32_POS));
  if (data->coordinate == NULL)
    {
      perror ("Allocating coordinate memory in edge.c");
      exit (-1);
    }

  for (i = 0 ; i < data->count ; i++)
    {
      fread (&data->coordinate[i].lon, sizeof (NV_FLOAT32), 1, th->fp);
      fread (&data->coordinate[i].lat, sizeof (NV_FLOAT32), 1, th->fp);
    }

  if (th->swap) swap_edge (data);


  /*  More sanity checks - if we have points outside the normal lat/lon range something is wrong.  */

  for (i = 0 ; i < data->count ; i++)
    {
      if (data->coordinate[i].lon < -180.0 || data->coordinate[i].lon > 180.0 ||
	  data->coordinate[i].lat < -90.0 || data->coordinate[i].lat > 90.0)
	{
	  free (data->coordinate);
	  return (NVFalse);
	}
    }


  return (NVTrue);
}


/********************************************************************

  Function:    free_edge

  Purpose:     Free the edge coordinate data that was allocated on the
               last read_edge call.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The edge data record

  Returns:     N/A

********************************************************************/

void free_edge (EDGE *data)
{
  free (data->coordinate);
}


/********************************************************************

  Function:    dump_edge

  Purpose:     Print to stderr the contents of an edge record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The edge record

  Returns:     N/A

********************************************************************/

void dump_edge (EDGE data)
{
  NV_INT32 i;


  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "Start node:                 %d\n", data.start_node);
  fprintf (stdout, "End node:                   %d\n", data.end_node);

  fprintf (stdout, "Right face triplet:         %x %d %d %d\n", data.right_face_t.type, data.right_face_t.field[0],
           data.right_face_t.field[1], data.right_face_t.field[2]);
  fprintf (stdout, "Left face triplet:          %x %d %d %d\n", data.left_face_t.type, data.left_face_t.field[0],
           data.left_face_t.field[1], data.left_face_t.field[2]);
  fprintf (stdout, "Right edge triplet:         %x %d %d %d\n", data.right_edge_t.type, data.right_edge_t.field[0],
           data.right_edge_t.field[1], data.right_edge_t.field[2]);
  fprintf (stdout, "Left edge triplet:          %x %d %d %d\n", data.left_edge_t.type, data.left_edge_t.field[0],
           data.left_edge_t.field[1], data.left_edge_t.field[2]);
  fprintf (stdout, "Right face integer:         %d\n", data.right_face_i);
  fprintf (stdout, "Left face integer:          %d\n", data.left_face_i);
  fprintf (stdout, "Right edge integer:         %d\n", data.right_edge_i);
  fprintf (stdout, "Left edge integer:          %d\n", data.left_edge_i);


  for (i = 0 ; i < data.count ; i++)
    {
      fprintf (stdout, "Coordinate longitude:       %f\n", data.coordinate[i].lon);
      fprintf (stdout, "Coordinate latitude:        %f\n", data.coordinate[i].lat);
    }
}
