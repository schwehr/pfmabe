#include "table_header.h"
#include "variable_index.h"


/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


int big_endian ();


static DNC_TABLE_HEADER th[MAX_DNC_TABLES];


static NV_BOOL check_type = NVTrue;



/********************************************************************

  Function:    read_dnc_table_header

  Purpose:     Read the standard DNC table file header from any DNC
               table file.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               table_header   -    The returned table header

  Returns:     N/A

********************************************************************/

static void read_dnc_table_header (NV_INT32 hnd, DNC_TABLE_HEADER **table_header)
{
  NV_CHAR *text, order, *in, cut[MAX_DNC_TABLES];
  NV_INT32 i, num_col;


  fseek (th[hnd].fp, 0, SEEK_SET);
  fread (&th[hnd].header_length, 1, sizeof (NV_INT32), th[hnd].fp);

  fread (&order, 1, sizeof (NV_CHAR), th[hnd].fp);
  if ((big_endian () && toupper (order) == 'L') || (!big_endian () && toupper (order) == 'M'))
    {
      th[hnd].swap = NVTrue;
    }
  else
    {
      th[hnd].swap = NVFalse;
    }

  if (th[hnd].swap) swap_int (&th[hnd].header_length);


  text = (NV_CHAR *) calloc (1, th[hnd].header_length);

  if (text == NULL)
    {
      perror ("Allocating header text memory");
      exit (-1);
    }


  /*  Move back before the endian indicator.  */

  fseek (th[hnd].fp, 1, SEEK_CUR);


  /*  Read all of the ASCII data.  */

  fgets (text, th[hnd].header_length, th[hnd].fp);


  /*  Parse the header.  */

  /*  Skip the endian indicator.  */

  in = text;


  num_col = 0;


  /*  Description.  */

  i = 0;
  while (in[0] != ';') 
    {
      th[hnd].table_description[i++] = in[0];
      in++;
    }
  th[hnd].table_description[i++] = 0;
  in++;


  /*  Documentation file name  */

  i = 0;
  while (in[0] != ';') 
    {
      th[hnd].documentation_file_name[i++] = in[0];
      in++;
    }
  th[hnd].documentation_file_name[i++] = 0;
  in++;


  /*  Semi-colon ends the header section.  */

  while (in[0] != ';')
    {
      /*  Allocate storage for repeating header portion.  */

      th[hnd].rh = (DNC_REPEATING_HEADER *) realloc (th[hnd].rh, (num_col + 1) * sizeof (DNC_REPEATING_HEADER));
      if (th[hnd].rh == NULL)
        {
          perror ("Allocating repeating header memory");
          exit (-1);
        }


      /*  Column name  */

      i = 0;
      while (in[0] != '=') 
        {
          th[hnd].rh[num_col].column_name[i++] = in[0];
          in++;
        }
      th[hnd].rh[num_col].column_name[i++] = 0;
      in++;


      /*  Field type  */

      i = 0;
      while (in[0] != ',') 
        {
          th[hnd].rh[num_col].field_type = in[0];
          in++;
        }
      in++;


      /*  Number of elements  */

      i = 0;
      while (in[0] != ',') 
        {
          cut[i++] = in[0];
          in++;
        }
      cut[i++] = 0;
      in++;
      i = sscanf (cut, "%hd", &th[hnd].rh[num_col].number_of_elements);


      /*  Asterisk (*) indicates variable number of elements.  */

      if (!i) th[hnd].rh[num_col].number_of_elements = -1;


      /*  Key type  */

      i = 0;
      while (in[0] != ',') 
        {
          th[hnd].rh[num_col].key_type = in[0];
          in++;
        }
      in++;


      /*  Column description  */

      i = 0;
      while (in[0] != ',') 
        {
          th[hnd].rh[num_col].column_description[i++] = in[0];
          in++;
        }
      th[hnd].rh[num_col].column_description[i++] = 0;
      in++;


      /*  Value description table  */

      i = 0;
      while (in[0] != ',') 
        {
          th[hnd].rh[num_col].value_description_table[i++] = in[0];
          in++;
        }
      th[hnd].rh[num_col].value_description_table[i++] = 0;
      in++;


      /*  Thematic index  */

      i = 0;
      while (in[0] != ',') 
        {
          th[hnd].rh[num_col].thematic_index[i++] = in[0];
          in++;
        }
      th[hnd].rh[num_col].thematic_index[i++] = 0;
      in++;


      /*  Skip the colon ":"  */

      i = 0;
      while (in[0] != ':') in++;
      in++;

      num_col++;
    }

  th[hnd].num_col = num_col;


  /*  Figure out the record length based on the number of elements and the data types.  Note that
      if the number of elements is -1 then there are variable length records.  This is also the case
      if there are triplet records (type K) in the data.  Files with variable length records will always
      have an index file lookup table.  Type X is a null field.  I'm not sure what to do with the 
      date fields yet.  */

  th[hnd].reclen = 0;
  for (i = 0 ; i < num_col ; i++)
    {
      if (th[hnd].rh[i].number_of_elements == -1) th[hnd].reclen = -1;


      if (th[hnd].reclen >= 0)
        {
          switch (th[hnd].rh[i].field_type)
            {
            case 'I':
              th[hnd].reclen += (sizeof (NV_INT32) * th[hnd].rh[i].number_of_elements);
              break;

            case 'S':
              th[hnd].reclen += (sizeof (NV_INT16) * th[hnd].rh[i].number_of_elements);
              break;

            case 'F':
              th[hnd].reclen += (sizeof (NV_FLOAT32) * th[hnd].rh[i].number_of_elements);
              break;

            case 'R':
              th[hnd].reclen += (sizeof (NV_FLOAT64) * th[hnd].rh[i].number_of_elements);
              break;

            case 'T':
              th[hnd].reclen += (sizeof (NV_CHAR) * th[hnd].rh[i].number_of_elements);
              break;

            case 'C':
              th[hnd].reclen += (sizeof (NV_F32_POS) * th[hnd].rh[i].number_of_elements);
              break;

            case 'Z':
              th[hnd].reclen += (sizeof (NV_F32_POSDEP) * th[hnd].rh[i].number_of_elements);
              break;

            case 'B':
              th[hnd].reclen += (sizeof (NV_F64_POS) * th[hnd].rh[i].number_of_elements);
              break;

            case 'Y':
              th[hnd].reclen += (sizeof (NV_F64_POSDEP) * th[hnd].rh[i].number_of_elements);
              break;

            case 'D':
              th[hnd].reclen += (20 * th[hnd].rh[i].number_of_elements);
              break;

            case 'K':
              th[hnd].reclen = -1;
              break;

            case 'X':
              break;

            default:
              fprintf(stderr,"%s %d %d\n",__FILE__,__LINE__,i);
              fprintf(stderr,"%s %d Invalid data type %1c!\n", __FILE__, __LINE__, th[hnd].rh[num_col].field_type);
              exit (-1);
              break;
            }
        }
    }

  free (text);

  *table_header = &th[hnd];
}




/********************************************************************

  Function:    open_dnc_table

  Purpose:     Open a DNC table file, read the header, and then
               save the header in local memory for faster
               retrieval.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_dnc_table (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  static NV_BOOL first = NVTrue;
  NV_INT32 i, hnd;


  /*  The first time through we want to initialize the table header memory.  */

  if (first)
    {
      for (i = 0 ; i < MAX_DNC_TABLES ; i++) th[i].fp = NULL;
      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = MAX_DNC_TABLES;
  for (i = 0 ; i < MAX_DNC_TABLES ; i++)
    {
      if (th[i].fp == NULL)
        {
          hnd = i;
          break;
        }
    }


  if (hnd == MAX_DNC_TABLES)
    {
      fprintf (stderr, "\n\nToo many open tables!\nTerminating!\n\n");
      exit (-1);
    }


  /*  Read the header.  */

  if ((th[hnd].fp = fopen (path, "rb")) != NULL)
    {
      read_dnc_table_header (hnd, table_header);
    }
  else
    {
      hnd = -1;
    }


  /*  If this has variable length records open and read the index file.  */

  if (th[hnd].reclen < 0) 
    {
      i = open_dnc_variable_index (path, hnd);
      if (i < 0)
        {
          perror (path);
          exit (-1);
        }
    }


  /*  Get the library type so we'll know what values to read/skip in all of the records.  */

  if (check_type) th[hnd].library_type = get_library_type ();


  return (hnd);
}


/********************************************************************

  Function:    open_dnc_table_no_type_check

  Purpose:     Open a DNC table file, read the header, and then
               save the header in local memory for faster
               retrieval.  This version is for quick and dirty
               reading of DNC table headers without checking for
               the library type.  In reality it's just a hack to
               allow a header dumping utility.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        09/04/06

  Arguments:   path           -    The table file path
               table_header   -    DNC table header structure to
                                   be populated

  Returns:     NV_INT32       -    The table handle or -1 on error

********************************************************************/

NV_INT32 open_dnc_table_no_type_check (NV_CHAR *path, DNC_TABLE_HEADER **table_header)
{
  NV_INT32 ret;

  check_type = NVFalse;
  ret = open_dnc_table (path, table_header);
  check_type = NVTrue;

  return (ret);
}


/********************************************************************

  Function:    close_dnc_table

  Purpose:     Close a DNC table file and free any repeating header
               memory that was allocated when the header was read

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     N/A

********************************************************************/

void close_dnc_table (NV_INT32 hnd)
{
  free (th[hnd].rh);
  fclose (th[hnd].fp);
  th[hnd].fp = NULL;
  memset (&th[hnd], 0, sizeof (DNC_TABLE_HEADER));
}


/********************************************************************

  Function:    dump_dnc_table_header

  Purpose:     Print to stderr the contents of a DNC table header
               record.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   table_header   -    The table header record

  Returns:     N/A

********************************************************************/

void dump_dnc_table_header (DNC_TABLE_HEADER *table_header)
{
  NV_INT32 i;


  fprintf (stdout, "Header length:           %d\n", table_header->header_length);
  fprintf (stdout, "Byte swap:               %d\n", table_header->swap);
  fprintf (stdout, "Table description:       %s\n", table_header->table_description);
  fprintf (stdout, "Documentation file name: %s\n", table_header->documentation_file_name);
  fprintf (stdout, "Record length:           %d\n", table_header->reclen);
  fprintf (stdout, "Number of columns:       %d\n\n", table_header->num_col);

  for (i = 0 ; i < table_header->num_col ; i++)
    {
      fprintf (stdout, "Column number:           %d\n", i);
      fprintf (stdout, "Column name:             %s\n", table_header->rh[i].column_name);
      fprintf (stdout, "Field type:              %1c\n", table_header->rh[i].field_type);
      fprintf (stdout, "Number of elements:      %d\n", table_header->rh[i].number_of_elements);
      fprintf (stdout, "Key type:                %1c\n", table_header->rh[i].key_type);
      fprintf (stdout, "Column description:      %s\n", table_header->rh[i].column_description);
      fprintf (stdout, "Value description table: %s\n", table_header->rh[i].value_description_table);
      fprintf (stdout, "Thematic index:          %s\n\n", table_header->rh[i].thematic_index);
    }
}



/********************************************************************

  Function:    get_dnc_table_header

  Purpose:     Return a pointer to the local copy of the DNC table
               header associated with the open table handle.

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle

  Returns:     DNC_TABLE_HANDLE -  The table header record pointer

********************************************************************/

DNC_TABLE_HEADER *get_dnc_table_header (NV_INT32 hnd)
{
  return (&th[hnd]);
}
