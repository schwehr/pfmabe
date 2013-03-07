#include "char_vdt.h"


/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


static CHAR_VDT cv[MAX_DNC_TABLES];


/********************************************************************

  Function:    read_char_vdt

  Purpose:     Read a character value description table and save the 
               contents in the cv array for later query.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               th             -    DNC table header structure

  Returns:     N/A

********************************************************************/

static void read_char_vdt (NV_INT32 hnd, DNC_TABLE_HEADER *th)
{
  NV_INT32 i, eof;


  /*  Find out the file size.  */

  fseek (th->fp, 0, SEEK_END);
  eof = ftell (th->fp);
  fseek (th->fp, th->header_length + 4, SEEK_SET);


  /*  Compute the number of records from the file size, header length, and the record length.  */

  cv[hnd].num_recs = (eof - (th->header_length + sizeof (NV_INT32))) / th->reclen;


  /*  Allocate the local memory for the character descriptions.  */

  cv[hnd].rec = (CHAR_VDT_REC *) calloc (cv[hnd].num_recs, sizeof (CHAR_VDT_REC));
  if (cv[hnd].rec == NULL)
    {
      perror ("Allocating CHAR_VDT memory");
      exit (-1);
    }


  /*  Read the descriptions.  */

  for (i = 0 ; i < cv[hnd].num_recs ; i++)
    {
      fread (&cv[hnd].rec[i].id, sizeof (NV_INT32), 1, th->fp);
      if (th->swap) swap_int (&cv[hnd].rec[i].id);

      trim_read (cv[hnd].rec[i].table_name, 12, th->fp);

      trim_read (cv[hnd].rec[i].column_name, 10, th->fp);

      trim_read (cv[hnd].rec[i].attr_value, 5, th->fp);

      trim_read (cv[hnd].rec[i].description, 50, th->fp);
    }
}


/********************************************************************

  Function:    open_char_vdt

  Purpose:     Open a character value description table and then read
               and save the contents in the cv array for later query.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_char_vdt (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 hnd;


  hnd = open_dnc_table (path, table_header);

  if (hnd >= 0) read_char_vdt (hnd, *table_header);

  return (hnd);
}


/********************************************************************

  Function:    close_char_vdt

  Purpose:     Close a character value description table and then free 
               the saved cv array memory for that table.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_char_vdt (NV_INT32 hnd)
{
  close_dnc_table (hnd);


  /*  Free the memory.  */

  free (cv[hnd].rec);
}


/********************************************************************

  Function:    dump_char_vdt

  Purpose:     Print to stderr the header and all contents of a
               character value description table.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void dump_char_vdt (NV_INT32 hnd)
{
  NV_INT32 i;
  DNC_TABLE_HEADER *th;


  th = get_dnc_table_header (hnd);

  dump_dnc_table_header (th);


  fprintf (stdout, "Number of records:       %d\n\n", cv[hnd].num_recs);

  for (i = 0 ; i < cv[hnd].num_recs ; i++)
    {
      fprintf (stdout, "Record number:              %d\n", i);
      fprintf (stdout, "ID:                         %d\n", cv[hnd].rec[i].id);
      fprintf (stdout, "Feature table name:         %s\n", cv[hnd].rec[i].table_name);
      fprintf (stdout, "Column name:                %s\n", cv[hnd].rec[i].column_name);
      fprintf (stdout, "Attribute value:            %s\n", cv[hnd].rec[i].attr_value);
      fprintf (stdout, "Description:                %s\n\n", cv[hnd].rec[i].description);
    }
}


/********************************************************************

  Function:    get_char_vdt_description

  Purpose:     Retrieve a description from a character value
               description table given the table name, column name and
               the attribute value.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               table_name     -    Table name from a feature table
               attr_value     -    attribute value from a feature table
                                   (ex. BE010)
               description    -    Returned description

  Returns:     N/A

********************************************************************/

NV_BOOL get_char_vdt_description (NV_INT32 hnd, NV_CHAR *table_name, NV_CHAR *column_name, NV_CHAR *attr_value,
                                  NV_CHAR *description)
{
  NV_INT32 i;


  for (i = 0 ; i < cv[hnd].num_recs ; i++)
    {
      /*  Match both the table name and the attribute values since the descriptions can change with the table.  */

      if (!strcmp (table_name, cv[hnd].rec[i].table_name) && !strcmp (attr_value, cv[hnd].rec[i].attr_value))
        {
          strcpy (description, cv[hnd].rec[i].description);
          return (NVTrue);
        }
    }

  return (NVFalse);
}
