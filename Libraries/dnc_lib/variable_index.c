#include "variable_index.h"


/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


static DNC_VARIABLE_INDEX vi[MAX_DNC_TABLES];
static FILE *fp;


/********************************************************************

  Function:    read_dnc_variable_index

  Purpose:     Read the variable index file and save it's contents to
               local memory so that we can quickly retireve them when
               we need to read a file with variable length records.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   table_hnd      -    The table handle for the table that
                                   this index is associated with.  For
                                   example, if the index file is the edx
                                   file then the associated table would
                                   be the edg file
               th             -    The table header

  Returns:     N/A

********************************************************************/

static void read_dnc_variable_index (NV_INT32 table_hnd, DNC_TABLE_HEADER th)
{
  NV_INT32 i;


  fseek (fp, 0, SEEK_SET);

  fread (&vi[table_hnd].num_recs, 1, sizeof (NV_INT32), fp);
  fread (&vi[table_hnd].header_length, 1, sizeof (NV_INT32), fp);

  if (th.swap)
    {
      swap_int (&vi[table_hnd].num_recs);
      swap_int (&vi[table_hnd].header_length);
    }

  vi[table_hnd].address = (INDEX_ADDRESS *) calloc (vi[table_hnd].num_recs, sizeof (INDEX_ADDRESS));
  if (vi[table_hnd].address == NULL)
    {
      perror ("Allocating variable index address memory");
      exit (-1);
    }

  for (i = 0 ; i < vi[table_hnd].num_recs ; i++)
    {
      fread (&vi[table_hnd].address[i].offset, 1, sizeof (NV_INT32), fp);
      fread (&vi[table_hnd].address[i].size, 1, sizeof (NV_INT32), fp);
      if (th.swap)
        {
          swap_int (&vi[table_hnd].address[i].offset);
          swap_int (&vi[table_hnd].address[i].size);
        }
    }
}




/********************************************************************

  Function:    open_dnc_variable_index

  Purpose:     Open a DNC vriable index file and read all records
               into local memory for faster retrieval.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   table_path     -    The path to the associated table
                                   file that contains variable length
                                   records.  For example, an "edg"
                                   file will have an "edx" variable
                                   index file.
               table_hnd      -    The handle for the associated table
                                   file.

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_dnc_variable_index (NV_CHAR *table_path, NV_INT32 table_hnd)
{
  NV_INT32 hnd;
  DNC_TABLE_HEADER *th;
  NV_CHAR vi_path[512];


  strcpy (vi_path, table_path);
  vi_path[strlen (vi_path) - 1] = 'x';


  th = get_dnc_table_header (table_hnd);


  hnd = table_hnd;
  if ((fp = fopen (vi_path, "rb")) != NULL)
    {
      read_dnc_variable_index (hnd, *th);
    }
  else
    {
      hnd = -1;
    }

  fclose (fp);

  return (hnd);
}


/********************************************************************

  Function:    close_dnc_variable_index

  Purpose:     Close a variable index file and frees the memory
               that was allocated when the file was read.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_dnc_variable_index (NV_INT32 table_hnd)
{
  free (vi[table_hnd].address);
  memset (&vi[table_hnd], 0, sizeof (DNC_VARIABLE_INDEX));
}


/********************************************************************

  Function:    dump_dnc_variable_index

  Purpose:     Print to stderr all of the contents of a variable
               index file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   table_hnd      -    The handle for the associated table
                                   file.

  Returns:     N/A

********************************************************************/

void dump_dnc_variable_index (NV_INT32 table_hnd)
{
  NV_INT32 i;


  fprintf (stdout, "Number of records:       %d\n", vi[table_hnd].num_recs);
  fprintf (stdout, "Header length:           %d\n", vi[table_hnd].header_length);

  for (i = 0 ; i < vi[table_hnd].num_recs ; i++)
    {
      fprintf (stdout, "Record number: %d    Offset: %d    Size: %d\n", i, vi[table_hnd].address[i].offset,
               vi[table_hnd].address[i].size);
    }
}



/********************************************************************

  Function:    get_dnc_variable_index

  Purpose:     Return an INDEX_ADDRESS record from the variable
               index file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   table_hnd      -    The handle for the associated table
                                   file.
               rec_num        -    The record number of the INDEX_ADDRESS
                                   record to be retrieved
  Returns:     INDEX_ADDRESS  -    The index record or a dummy index record
                                   with a -1 offset and 0 size on error

********************************************************************/

INDEX_ADDRESS get_dnc_variable_index_address (NV_INT32 table_hnd, NV_INT32 rec_num)
{
  INDEX_ADDRESS dummy = {-1, 0};

  if (!vi[table_hnd].num_recs) return (dummy);

  return (vi[table_hnd].address[rec_num]);
}
