#include "tod2_dht.h"


/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    read_tod2_dht

  Purpose:     Read the database header table (it should always have
               just one record).

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               th             -    DNC table header structure

  Returns:     N/A

********************************************************************/

TOD2_DHT read_tod2_dht (NV_CHAR *path)
{
  NV_INT32 eof, num_recs, hnd, size;
  TOD2_DHT data;
  DNC_TABLE_HEADER *th;
  static NV_BOOL first = NVTrue;


  hnd = open_dnc_table_no_type_check (path, &th);


  /*  Find out the file size.  */

  fseek (th->fp, 0, SEEK_END);
  eof = ftell (th->fp);
  fseek (th->fp, th->header_length + 4, SEEK_SET);


  /*  Compute the number of records from the file size, header length, and the record length.  */

  num_recs = (eof - (th->header_length + sizeof (NV_INT32))) / th->reclen;


  if (num_recs > 1)
    {
      fprintf (stderr, "More than one record in the database header table - WTF, over?!?\n");
      exit (-1);
    }


  /*  Read the record.  */

  fread (&data.id, sizeof (NV_INT32), 1, th->fp);
  if (th->swap) swap_int (&data.id);

  trim_read (data.vpf_version, 10, th->fp);
  trim_read (data.database_name, 8, th->fp);
  trim_read (data.database_description, 100, th->fp);
  trim_read (data.media_standard, 20, th->fp);
  trim_read (data.originator, 50, th->fp);
  trim_read (data.addressee, 100, th->fp);
  trim_read (data.media_volumes, 4, th->fp);


  fread (&size, sizeof (NV_INT32), 1, th->fp);
  if (th->swap) swap_int (&size);

  if (!first)
    {
      free (data.seq_numbers);
      first = NVFalse;
    }

  data.seq_numbers = (NV_CHAR *) calloc (size, sizeof (NV_CHAR));
  if (data.seq_numbers == NULL)
    {
      perror ("Allocating seq_numbers memory in tod2_dht.c");
      exit (-1);
    }
  trim_read (data.seq_numbers, size, th->fp);


  trim_read (data.num_data_sets, 4, th->fp);
  fread (&data.security_class, 1, 1, th->fp);
  trim_read (data.downgrading, 3, th->fp);
  trim_read (data.downgrade_date, 20, th->fp);
  trim_read (data.releasability, 20, th->fp);
  fread (&data.transmittal_id, 1, 1, th->fp);
  trim_read (data.edition_number, 10, th->fp);
  trim_read (data.edition_date, 20, th->fp);


  fread (&size, sizeof (NV_INT32), 1, th->fp);
  if (th->swap) swap_int (&size);

  if (!first) free (data.declassification);
  data.declassification = (NV_CHAR *) calloc (size, sizeof (NV_CHAR));
  if (data.declassification == NULL)
    {
      perror ("Allocating declassification memory in tod2_dht.c");
      exit (-1);
    }
  trim_read (data.declassification, size, th->fp);


  fread (&size, sizeof (NV_INT32), 1, th->fp);
  if (th->swap) swap_int (&size);

  if (!first) free (data.class_just);
  data.class_just = (NV_CHAR *) calloc (size, sizeof (NV_CHAR));
  if (data.class_just == NULL)
    {
      perror ("Allocating class_just memory in tod2_dht.c");
      exit (-1);
    }
  trim_read (data.class_just, size, th->fp);


  close_dnc_table (hnd);


  return (data);
}



/********************************************************************

  Function:    dump_tod2_dht

  Purpose:     Print to stderr the contents of the database header table

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The database header table record.

  Returns:     N/A

********************************************************************/

void dump_tod2_dht (TOD2_DHT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "VPF version:                %s\n", data.vpf_version);
  fprintf (stdout, "Database name:              %s\n", data.database_name);
  fprintf (stdout, "Database description:       %s\n", data.database_description);
  fprintf (stdout, "Media standard:             %s\n", data.media_standard);
  fprintf (stdout, "Originator:                 %s\n", data.originator);
  fprintf (stdout, "Addressee:                  %s\n", data.addressee);
  fprintf (stdout, "Media volumes:              %1s\n", data.media_volumes);
  fprintf (stdout, "Sequence numbers:           %s\n", data.seq_numbers);
  fprintf (stdout, "Number of data sets:        %s\n", data.num_data_sets);
  fprintf (stdout, "Security classification:    %1c\n", data.security_class);
  fprintf (stdout, "Downgrading:                %s\n", data.downgrading);
  fprintf (stdout, "Downgrade date:             %s\n", data.downgrade_date);
  fprintf (stdout, "Releasability:              %s\n", data.releasability);
  fprintf (stdout, "Transmittal id:             %1c\n", data.transmittal_id);
  fprintf (stdout, "Edition number:             %s\n", data.edition_number);
  fprintf (stdout, "Edition date:               %s\n", data.edition_date);
  fprintf (stdout, "Declassification statement: %s\n", data.declassification);
  fprintf (stdout, "Justification of class:     %s\n", data.class_just);
}
