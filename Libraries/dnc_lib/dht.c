#include "dht.h"


/*****************************************************************************\

    This file contains public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/********************************************************************

  Function:    read_dht

  Purpose:     Read the database header table (it should always have
               just one record).

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   hnd            -    The table handle
               th             -    DNC table header structure

  Returns:     N/A

********************************************************************/

DHT read_dht (NV_CHAR *path)
{
  NV_INT32 eof, num_recs, hnd;
  DHT data;
  DNC_TABLE_HEADER *th;


  hnd = open_dnc_table_no_type_check (path, &th);
  if (hnd == -1)
    {
      perror (path);
      exit (-1);
    }


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
  fread (&data.media_volumes, 1, 1, th->fp);
  fread (&data.seq_numbers, 1, 1, th->fp);
  fread (&data.num_data_sets, 1, 1, th->fp);
  fread (&data.security_class, 1, 1, th->fp);
  trim_read (data.downgrading, 3, th->fp);
  trim_read (data.downgrade_date, 20, th->fp);
  trim_read (data.releasability, 20, th->fp);
  trim_read (data.other_std_name, 50, th->fp);
  trim_read (data.other_std_date, 20, th->fp);
  trim_read (data.other_std_ver, 10, th->fp);
  fread (&data.transmittal_id, 1, 1, th->fp);
  trim_read (data.edition_number, 10, th->fp);
  trim_read (data.edition_date, 20, th->fp);


  close_dnc_table (hnd);


  return (data);
}



/********************************************************************

  Function:    dump_dht

  Purpose:     Print to stderr the contents of the database header table

  Author:      Jan C. Depner (jan.depner@navy.mil)

  Date:        08/29/06

  Arguments:   data           -    The database header table record.

  Returns:     N/A

********************************************************************/

void dump_dht (DHT data)
{
  fprintf (stdout, "ID:                         %d\n", data.id);
  fprintf (stdout, "VPF version:                %s\n", data.vpf_version);
  fprintf (stdout, "Database name:              %s\n", data.database_name);
  fprintf (stdout, "Database description:       %s\n", data.database_description);
  fprintf (stdout, "Media standard:             %s\n", data.media_standard);
  fprintf (stdout, "Originator:                 %s\n", data.originator);
  fprintf (stdout, "Addressee:                  %s\n", data.addressee);
  fprintf (stdout, "Media volumes:              %1c\n", data.media_volumes);
  fprintf (stdout, "Sequence numbers:           %1c\n", data.seq_numbers);
  fprintf (stdout, "Number of data sets:        %1c\n", data.num_data_sets);
  fprintf (stdout, "Security classification:    %1c\n", data.security_class);
  fprintf (stdout, "Downgrading:                %s\n", data.downgrading);
  fprintf (stdout, "Downgrade date:             %s\n", data.downgrade_date);
  fprintf (stdout, "Releasability:              %s\n", data.releasability);
  fprintf (stdout, "Other standard name:        %s\n", data.other_std_name);
  fprintf (stdout, "Other standard date:        %s\n", data.other_std_date);
  fprintf (stdout, "Other standard version:     %s\n", data.other_std_ver);
  fprintf (stdout, "Transmittal id:             %1c\n", data.transmittal_id);
  fprintf (stdout, "Edition number:             %s\n", data.edition_number);
  fprintf (stdout, "Edition date:               %s\n", data.edition_date);
}
