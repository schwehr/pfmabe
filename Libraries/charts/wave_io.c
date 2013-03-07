#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "FileWave.h"

static NV_BOOL             swap, first = NVTrue;
static WAVE_HEADER_T       l_head;

/***************************************************************************\
*                                                                           *
*   Module Name:        lidar_get_string                                    *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       December 1994                                       *
*                                                                           *
*   Purpose:            Parses the input string for the : and returns       *
*                       everything to the right.                            *
*                                                                           *
*   Arguments:          *in     -   Input string                            *
*                       *out    -   Output string                           *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
*   Calling Routines:   envin                                               *
*                                                                           * 
\***************************************************************************/

void lidar_get_string (NV_CHAR *in, NV_CHAR *out)
{
  NV_INT32       i, start, length;

  start = 0;
  length = 0;
    
  strcpy (out, (NV_CHAR *) (strchr (in, ':') + 1));

  /*  Search for first non-blank character.   */
    
  for (i = 0 ; i < strlen (out) ; i++)
    {
      if (out[i] != ' ')
        {
          start = i;
          break;
        }
    }

  /*  Search for last non-blank character.    */
    
  for (i = strlen (out) ; i >= 0 ; i--)
    {
      if (out[i] != ' ' && out[i] != 0)
        {
          length = (i + 1) - start;
          break;
        }
    }

  strncpy (out, &out[start], length);
  out[length] = 0;
}



NV_INT32 wave_read_header (FILE *fp, WAVE_HEADER_T *head)
{
  NV_INT64    long_pos;
  NV_CHAR     varin[1024], info[1024];
  NV_INT32    i;
  NV_INT16    tmpi16;


  NV_CHAR *ngets (NV_CHAR *s, NV_INT32 size, FILE *stream);


  i = fseeko64 (fp, 0LL, SEEK_SET);


  /*  Read each entry.    */

  head->header_size = 0;
  while (ngets (varin, sizeof (varin), fp) != NULL)
    {
      if (!strcmp (varin, "EOF")) break;


      /*  Put everything to the right of the colon into 'info'.   */
            
      if (strchr (varin, ':') != NULL) strcpy (info, (NV_CHAR *) (strchr (varin, ':') + 1));


      /*  Check input for matching strings and load values if found.  */
            
      if (strstr (varin, "FileType:") != NULL) lidar_get_string (varin, head->file_type);

      if (strstr (varin, "EndianType:") != NULL) if (strstr (info, "Little")) head->endian = NVFalse;

      if (strstr (varin, "SoftwareVersionNumber:") != NULL) sscanf (info, "%f", &head->software_version);

      if (strstr (varin, "FileVersionNumber:") != NULL) sscanf (info, "%f", &head->file_version);


      if (strstr (varin, "HeaderSize:") != NULL) sscanf (info, "%d", &head->header_size);

      if (strstr (varin, "TextBlockSize:") != NULL) sscanf (info, "%d", &head->text_block_size);

      if (strstr (varin, "BinaryBlockSize:") != NULL) sscanf (info, "%d", &head->bin_block_size);

      if (strstr (varin, "HardwareBlockSize:") != NULL) sscanf (info, "%d", &head->hardware_block_size);

      if (strstr (varin, "HapsBlockSize:") != NULL) sscanf (info, "%d", &head->haps_block_size);

      if (strstr (varin, "OtherBlockSize:") != NULL) sscanf (info, "%d", &head->other_block_size);

      if (strstr (varin, "RecordSize:") != NULL) sscanf (info, "%hd", &head->record_size);

      if (strstr (varin, "ShotDataSize:") != NULL) sscanf (info, "%hd", &head->shot_data_size);

      if (strstr (varin, "WaveformSize:") != NULL) sscanf (info, "%hd", &head->wave_form_size);

      if (strstr (varin, "DeepWaveSize:") != NULL) sscanf (info, "%hd", &head->pmt_size);

      if (strstr (varin, "ShallowWaveSize:") != NULL) sscanf (info, "%hd", &head->apd_size);

      if (strstr (varin, "IRWaveSize:") != NULL) sscanf (info, "%hd", &head->ir_size);

      if (strstr (varin, "RamanWaveSize:") != NULL) sscanf (info, "%hd", &head->raman_size);

      if (strstr (varin, "ABSystemType:") != NULL)
        {
          sscanf (info, "%hd", &tmpi16);
          head->ab_system_type = (NV_BYTE) tmpi16;
        }

      if (strstr (varin, "ABSystemNumber:") != NULL)
        {
          sscanf (info, "%hd", &tmpi16);
          head->ab_system_number = (NV_BYTE) tmpi16;
        }

      if (strstr (varin, "SystemRepRate:") != NULL) sscanf (info, "%hd", &head->system_rep_rate);


      if (strstr (varin, "Project:") != NULL) lidar_get_string (varin, head->project);

      if (strstr (varin, "Mission:") != NULL) lidar_get_string (varin, head->mission);

      if (strstr (varin, "Dataset:") != NULL) lidar_get_string (varin, head->dataset);

      if (strstr (varin, "FlightlineNumber:") != NULL) lidar_get_string (varin, head->flightline_number);

      if (strstr (varin, "CodedFLNumber:") != NULL) sscanf (info, "%hd", &head->coded_fl_number);

      if (strstr (varin, "FlightDate:") != NULL) lidar_get_string (varin, head->flight_date);

      if (strstr (varin, "StartTime:") != NULL) lidar_get_string (varin, head->start_time);

      if (strstr (varin, "EndTime:") != NULL) lidar_get_string (varin, head->end_time);

      if (strstr (varin, "StartTimestamp:") != NULL) sscanf (info, NV_INT64_SPECIFIER, &head->start_timestamp);

      if (strstr (varin, "EndTimestamp:") != NULL) sscanf (info, NV_INT64_SPECIFIER, &head->end_timestamp);

      if (strstr (varin, "NumberShots:") != NULL) sscanf (info, "%d", &head->number_shots);

      if (strstr (varin, "DatasetCreateDate:") != NULL) lidar_get_string (varin, head->dataset_create_date);

      if (strstr (varin, "DatasetCreateTime:") != NULL) lidar_get_string (varin, head->dataset_create_time);

      long_pos = ftello64 (fp);
      if (head->header_size && long_pos >= head->header_size) break;
    }


  /*  Get the ac_zero_offset data.  */

  long_pos = head->text_block_size + head->bin_block_size + 572;
  fseeko64 (fp, long_pos, SEEK_SET);

  fread (&head->ac_zero_offset, sizeof (NV_U_INT16), 4, fp);


  /*  Move past the entire header block.  */

  fseeko64 (fp, head->header_size, SEEK_SET);

  return (0);
}


FILE *open_wave_file (NV_CHAR *path)
{
  FILE *fp;

  NV_INT32 big_endian ();


  swap = (NV_BOOL) big_endian ();
  first = NVTrue;

  if ((fp = fopen64 (path, "rb")) == NULL)
    {
      perror (path);
    }
  else
    {
      wave_read_header (fp, &l_head);
    }


  return (fp);
}


/*  Note that we're counting from 1 not 0.  Not my idea!  */

/*  RIDICULOUSLY IMPORTANT NOTE:  Make sure that you static "record" in the calling routine since we are allocating the
    memory for the waveforms here!  DOH!!!  */

NV_INT32 wave_read_record (FILE *fp, NV_INT32 num, WAVE_DATA_T *record)
{
  NV_INT32 ret;
  NV_INT64 long_pos;


  if (!num)
    {
      fprintf (stderr, "Zero is not a valid WAVE record number\n");
      fflush (stderr);
      return (0);
    }


  if (num != WAVE_NEXT_RECORD)
    {
      fseeko64 (fp, (NV_INT64) l_head.header_size + (NV_INT64) (num - 1) * (NV_INT64) l_head.record_size, SEEK_SET);
    }
  else
    {
      long_pos = ftello64 (fp);
      if (long_pos < l_head.header_size) fseeko64 (fp, (NV_INT64) l_head.header_size, SEEK_SET);
    }


  if (first)
    {
      record->shot_data = (NV_U_BYTE *) calloc (l_head.shot_data_size - sizeof (NV_INT64), sizeof (NV_U_BYTE));
      record->pmt = (NV_U_BYTE *) calloc (l_head.pmt_size, sizeof (NV_U_BYTE));
      record->apd = (NV_U_BYTE *) calloc (l_head.apd_size, sizeof (NV_U_BYTE));
      record->ir = (NV_U_BYTE *) calloc (l_head.ir_size, sizeof (NV_U_BYTE));
      record->raman = (NV_U_BYTE *) calloc (l_head.raman_size, sizeof (NV_U_BYTE));

      if (record->raman == NULL)
        {
          perror ("Allocating wave memory");
          exit (-1);
        }
      first = NVFalse;
    }


  /*  Read the timestamp.  */

  fread (&record->timestamp, sizeof (NV_INT64), 1, fp);

  if (swap) swap_NV_INT64 (&record->timestamp);


  /*  Read the shot data (Optech proprietary info that we don't care about).  */

  /*fseeko64 (fp, (NV_INT64) l_head.shot_data_size - (NV_INT64) sizeof (NV_INT64), SEEK_CUR);*/
  ret = fread (record->shot_data, l_head.shot_data_size - sizeof (NV_INT64), 1, fp);


  /*  Read the waveform data.  */

  ret = fread (record->pmt, l_head.pmt_size, 1, fp);
  ret = fread (record->apd, l_head.apd_size, 1, fp);
  ret = fread (record->ir, l_head.ir_size, 1, fp);
  ret = fread (record->raman, l_head.raman_size, 1, fp);


  return (ret);
}


void wave_dump_record (WAVE_DATA_T record)
{
  NV_INT32        i, j, start, end, year, day, hour, minute, month, mday;
  NV_FLOAT32      second;


  charts_cvtime (record.timestamp, &year, &day, &hour, &minute, &second);
  charts_jday2mday (year, day, &month, &mday);
  month++;

  printf ("*****************************\n");
  printf ("timestamp : ");
  printf (NV_INT64_SPECIFIER, record.timestamp);
  printf ("    %d-%02d-%02d (%d) %02d:%02d:%08.5f\n", year + 1900, month, mday, day, hour, minute, second);


  printf ("\n*****************  PMT waveform values  *****************\n");

  for (i = 0 ; i < l_head.pmt_size ; i += 10)
    {
      if (i >= l_head.pmt_size) break;

      start = i;
      end = MIN (i + 10, l_head.pmt_size);

      printf ("%04d-%04d : ", start, end - 1);

      for (j = start ; j < end ; j++) printf ("%05d ", record.pmt[j]);

      printf ("\n");
    }

  printf ("\n");

  printf ("\n*****************  APD waveform values  *****************\n");

  for (i = 0 ; i < l_head.apd_size ; i += 10)
    {
      if (i >= l_head.apd_size) break;

      start = i;
      end = MIN (i + 10, l_head.apd_size);

      printf ("%04d-%04d : ", start, end - 1);

      for (j = start ; j < end ; j++) printf ("%05d ", record.apd[j]);

      printf ("\n");
    }

  printf ("\n");

  printf ("\n*****************  IR waveform values  *****************\n");

  for (i = 0 ; i < l_head.ir_size ; i += 10)
    {
      if (i >= l_head.ir_size) break;

      start = i;
      end = MIN (i + 10, l_head.ir_size);

      printf ("%04d-%04d : ", start, end - 1);

      for (j = start ; j < end ; j++) printf ("%05d ", record.ir[j]);

      printf ("\n");
    }

  printf ("\n");

  printf ("\n*****************  RAMAN waveform values  *****************\n");

  for (i = 0 ; i < l_head.raman_size ; i += 10)
    {
      if (i >= l_head.raman_size) break;

      start = i;
      end = MIN (i + 10, l_head.raman_size);

      printf ("%04d-%04d : ", start, end - 1);

      for (j = start ; j < end ; j++) printf ("%05d ", record.raman[j]);

      printf ("\n");
    }

  printf ("\n");

  fflush (stdout);
}
