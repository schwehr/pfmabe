
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "llz.h"
#include "swap_bytes.h"
#include "llz_version.h"


typedef struct
{
  FILE          *fp;
  NV_BOOL       time_flag;            /*!<  This is duplicated in header due to the need to support 1.0 files.  */
  NV_U_BYTE     depth_units;          /*!<  This is duplicated in header due to the need to support 1.0 files.  */
  NV_BOOL       swap;
  NV_BOOL       at_end;
  NV_BOOL       size_changed;
  NV_BOOL       modified;
  NV_BOOL       created;
  NV_BOOL       write;
  LLZ_HEADER    header;
  NV_INT16      major_version;
} INTERNAL_LLZ_HEADER;

typedef struct
{
  NV_INT32      tv_sec;
  NV_INT32      tv_nsec;
  NV_INT32      lat;
  NV_INT32      lon;
  NV_INT32      dep;
  NV_U_INT16    stat;
} INTERNAL_LLZ;

static INTERNAL_LLZ_HEADER llzh[MAX_LLZ_FILES];
static NV_BOOL first;
static NV_INT32 llz_recnum[MAX_LLZ_FILES];


NV_INT32 big_endian ();



/********************************************************************/
/*!

 - Function:    swap_internal_llz

 - Purpose:     Byte swap an internal llz record.

 - Author:      Jan C. Depner (jan.depner@navy.mil)

 - Date:        08/31/06

 - Arguments:   data           -    The internal format llz record to be swapped

 - Returns:     N/A

********************************************************************/

static void swap_internal_llz (INTERNAL_LLZ *data)
{
  swap_int (&data->tv_sec);
  swap_int (&data->tv_nsec);
  swap_int (&data->lat);
  swap_int (&data->lon);
  swap_int (&data->dep);
  swap_int ((NV_INT32 *) &data->stat);
}


/********************************************************************/
/*!

 - Function:    write_llz_header

 - Purpose:     Write the llz_header to the llz file.

 - Author:      Jan C. Depner (jan.depner@navy.mil)

 - Date:        08/31/06

 - Arguments:   hnd            =    The llz file handle

 - Returns:     N/A

********************************************************************/

static void write_llz_header (NV_INT32 hnd)
{
  NV_U_BYTE zero = 0;
  NV_INT32 i, size ;


  fseeko64 (llzh[hnd].fp, 0L, SEEK_SET);


  fprintf (llzh[hnd].fp, "[VERSION] = %s\n", LLZ_VERSION);

  if (llzh[hnd].time_flag)
    {
      fprintf (llzh[hnd].fp, "[TIME FLAG] = 1\n");
    }
  else
    {
      fprintf (llzh[hnd].fp, "[TIME FLAG] = 0\n");
    }

  switch (llzh[hnd].depth_units)
    {
    case 0:
    default:
      fprintf (llzh[hnd].fp, "[DEPTH UNITS] = METERS\n");
      break;

    case 1:
      fprintf (llzh[hnd].fp, "[DEPTH UNITS] = FEET\n");
      break;

    case 2:
      fprintf (llzh[hnd].fp, "[DEPTH UNITS] = FATHOMS\n");
      break;

    case 3:
      fprintf (llzh[hnd].fp, "[DEPTH UNITS] = CUBITS\n");
      break;

    case 4:
      fprintf (llzh[hnd].fp, "[DEPTH UNITS] = WILLETTS\n");
      break;
    }


  if (big_endian ())
    {
      fprintf (llzh[hnd].fp, "[ENDIAN] = BIG\n");
    }
  else
    {
      fprintf (llzh[hnd].fp, "[ENDIAN] = LITTLE\n");
    }

  fprintf (llzh[hnd].fp, "[CLASSIFICATION] = %s\n", llzh[hnd].header.classification);
  fprintf (llzh[hnd].fp, "[DISTRIBUTION] = %s\n", llzh[hnd].header.distribution);
  fprintf (llzh[hnd].fp, "[DECLASSIFICATION] = %s\n", llzh[hnd].header.declassification);
  fprintf (llzh[hnd].fp, "[CLASSIFICATION JUSTIFICATION] = %s\n", llzh[hnd].header.class_just);
  fprintf (llzh[hnd].fp, "[DOWNGRADE] = %s\n", llzh[hnd].header.downgrade);
  fprintf (llzh[hnd].fp, "[SOURCE] = %s\n", llzh[hnd].header.source);
  fprintf (llzh[hnd].fp, "[COMMENTS] = %s\n", llzh[hnd].header.comments);


  /*  The next two don't need \n since the linefeed is included from the asctime function.  */

  fprintf (llzh[hnd].fp, "[CREATION DATE] = %s", llzh[hnd].header.creation_date);
  fprintf (llzh[hnd].fp, "[LAST MODIFIED DATE] = %s", llzh[hnd].header.modified_date);

  fprintf (llzh[hnd].fp, "[NUMBER OF RECORDS] = %d\n", llzh[hnd].header.number_of_records);


  fprintf (llzh[hnd].fp, "[END OF HEADER]\n");


  size = LLZ_HEADER_SIZE - ftell (llzh[hnd].fp);

  for (i = 0 ; i < size ; i++) fwrite (&zero, 1, 1, llzh[hnd].fp);
}



/********************************************************************/
/*!

 - Function:    create_llz

 - Purpose:     Create an llz file.

 - Author:      Jan C. Depner (jan.depner@navy.mil)

 - Date:        08/31/06

 - Arguments:
                - path           =    The llz file path
                - llz_header     =    LLZ_HEADER structure to
                                      be written to the file
                                      Note: leave version, number_of_records,
                                      creation_date, and modified_date
                                      empty.  Don't forget to set time_flag
                                      and depth_units.

 - Returns:
                - The file handle or -1 on error

********************************************************************/

NV_INT32 create_llz (const NV_CHAR *path, LLZ_HEADER llz_header)
{
  NV_INT32 i, hnd;


  /*  The first time through we want to initialize the llz handle array.  */

  if (first)
    {
      for (i = 0 ; i < MAX_LLZ_FILES ; i++) 
        {
          memset (&llzh[i], 0, sizeof (INTERNAL_LLZ_HEADER));
          llzh[i].fp = NULL;
        }
      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = MAX_LLZ_FILES;
  for (i = 0 ; i < MAX_LLZ_FILES ; i++)
    {
      if (llzh[i].fp == NULL)
        {
          hnd = i;
          llz_recnum[hnd] = 0;
          break;
        }
    }


  if (hnd == MAX_LLZ_FILES)
    {
      fprintf (stderr, "\n\nToo many open llz files!\nTerminating!\n\n");
      exit (-1);
    }


  /*  Check for programs built with version 1.0 of the library (just in case
      time_flag or depth units has a weird value).  */

  if (llz_header.time_flag != 1) llz_header.time_flag = 0;
  if (llz_header.depth_units > 4) llz_header.depth_units = 0;


  llzh[hnd].time_flag = llz_header.time_flag;
  llzh[hnd].depth_units = llz_header.depth_units;


  /*  Open the file and write the header.  */

  if ((llzh[hnd].fp = fopen64 (path, "wb")) != NULL)
    {
      llzh[hnd].header = llz_header;

      write_llz_header (hnd);
    }
  else
    {
      hnd = -1;
    }


  llzh[hnd].at_end = NVTrue;
  llzh[hnd].size_changed = NVTrue;
  llzh[hnd].modified = NVTrue;
  llzh[hnd].created = NVTrue;
  llzh[hnd].write = NVTrue;
  llzh[hnd].header.number_of_records = 0;

  return (hnd);
}


/********************************************************************/
/*!

 - Function:    open_llz

 - Purpose:     Open an llz file.

 - Author:      Jan C. Depner (jan.depner@navy.mil)

 - Date:        08/31/06

 - Arguments:
                - path           =    The llz file path
                - llz_header     =    LLZ_HEADER structure to be populated

 - Returns:
                - The file handle or -1 on error

********************************************************************/

NV_INT32 open_llz (const NV_CHAR *path, LLZ_HEADER *llz_header)
{
  NV_INT32 i, hnd, tf;
  NV_FLOAT32 tmpf;
  NV_CHAR varin[1024], info[1024];


  /*  The first time through we want to initialize the llz handle array.  */

  if (first)
    {
      for (i = 0 ; i < MAX_LLZ_FILES ; i++) 
        {
          memset (&llzh[i], 0, sizeof (INTERNAL_LLZ_HEADER));
          llzh[i].fp = NULL;
          llz_recnum[i] = 0;
        }
      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = MAX_LLZ_FILES;
  for (i = 0 ; i < MAX_LLZ_FILES ; i++)
    {
      if (llzh[i].fp == NULL)
        {
          hnd = i;
          llz_recnum[hnd] = 0;
          break;
        }
    }


  if (hnd == MAX_LLZ_FILES)
    {
      fprintf (stderr, "\n\nToo many open llz files!\n\n");
      return (-1);
    }


  /*  Open the file and read the header.  */

  tf = 0;
  llzh[hnd].depth_units = 0;
  if ((llzh[hnd].fp = fopen64 (path, "rb+")) != NULL || (llzh[hnd].fp = fopen64 (path, "rb")) != NULL)
    {
      /*  We want to try to read the first line (version info) with an fread in case we mistakenly asked to
          load a binary file.  If we try to use ngets to read a binary file and there are no line feeds in 
          the first sizeof (varin) characters we would segfault.  */

      if (!fread (varin, 128, 1, llzh[hnd].fp)) return (-1);


      /*  Check for the version.  */

      if (!strstr (varin, "llz library V")) return (-1);


      /*  Rewind to the beginning of the file.  Yes, we'll read the version again but it doesn't matter.  */

      fseeko64 (llzh[hnd].fp, 0L, SEEK_SET);


      while (fgets (varin, sizeof (varin), llzh[hnd].fp))
        {
          if (strstr (varin, "[END OF HEADER]")) break;


          /*  Put everything to the right of the equals sign in 'info'.   */

          if (strchr (varin, '=') != NULL) strcpy (info, (strchr (varin, '=') + 1));

          if (strstr (varin, "[VERSION]"))
            {
              strcpy (llzh[hnd].header.version, info);
              sscanf (llzh[hnd].header.version, "%f", &tmpf);
              llzh[hnd].major_version = (NV_INT16) tmpf;
            }

          if (strstr (varin, "[TIME FLAG]")) sscanf (info, "%d", &tf);

          if (strstr (varin, "[DEPTH UNITS]")) sscanf (info, "%c", &llzh[hnd].depth_units);

          if (strstr (varin, "[ENDIAN]"))
            {
              if (big_endian ())
                {
                  if (strstr (info, "LITTLE"))
                    {
                      llzh[hnd].swap = NVTrue;
                    }
                  else
                    {
                      llzh[hnd].swap = NVFalse;
                    }
                }
              else
                {
                  if (strstr (info, "BIG"))
                    {
                      llzh[hnd].swap = NVTrue;
                    }
                  else
                    {
                      llzh[hnd].swap = NVFalse;
                    }
                }
            }

          if (strstr (varin, "[CLASSIFICATION]")) strcpy (llzh[hnd].header.classification, info);

          if (strstr (varin, "[DISTRIBUTION]")) strcpy (llzh[hnd].header.distribution, info);

          if (strstr (varin, "[DECLASSIFICATION]")) strcpy (llzh[hnd].header.declassification, info);

          if (strstr (varin, "[CLASSIFICATION JUSTIFICATION]")) strcpy (llzh[hnd].header.class_just, info);

          if (strstr (varin, "[DOWNGRADE]")) strcpy (llzh[hnd].header.downgrade, info);

          if (strstr (varin, "[SOURCE]")) strcpy (llzh[hnd].header.source, info);

          if (strstr (varin, "[COMMENTS]")) strcpy (llzh[hnd].header.comments, info);

          if (strstr (varin, "[NUMBER OF RECORDS]")) sscanf (info, "%d", &llzh[hnd].header.number_of_records);

          if (strstr (varin, "[CREATION DATE]")) strcpy (llzh[hnd].header.creation_date, info);

          if (strstr (varin, "[LAST MODIFIED DATE]")) strcpy (llzh[hnd].header.modified_date, info);
        }

      llzh[hnd].time_flag = llzh[hnd].header.time_flag = (NV_BOOL) tf;
      llzh[hnd].depth_units = llzh[hnd].header.depth_units;
      llzh[hnd].at_end = NVFalse;
      llzh[hnd].size_changed = NVFalse;
      llzh[hnd].modified = NVFalse;
      llzh[hnd].created = NVFalse;
      llzh[hnd].write = NVFalse;


      *llz_header = llzh[hnd].header;
    }
  else
    {
      hnd = -1;
    }


  return (hnd);
}


/********************************************************************/
/*!

 - Function:    close_llz

 - Purpose:     Close an llz file.

 - Author:      Jan C. Depner (jan.depner@navy.mil)

 - Date:        08/31/06

 - Arguments:   hnd            =    The file handle

 - Returns:     void

********************************************************************/

void close_llz (NV_INT32 hnd)
{
  time_t t;
  struct tm *cur_tm;


  if (llzh[hnd].modified)
    {
      t = time (&t);
      cur_tm = gmtime (&t);
      strcpy (llzh[hnd].header.modified_date, asctime (cur_tm));
    }

  if (llzh[hnd].created)
    {
      t = time (&t);
      cur_tm = gmtime (&t);
      strcpy (llzh[hnd].header.creation_date, asctime (cur_tm));
    }

  if (llzh[hnd].size_changed || llzh[hnd].created || llzh[hnd].modified) write_llz_header (hnd);

  fclose (llzh[hnd].fp);
  llzh[hnd].fp = NULL;
  memset (&llzh[hnd], 0, sizeof (INTERNAL_LLZ_HEADER));
  llzh[hnd].fp = NULL;
}


/********************************************************************/
/*!

 - Function:    read_llz

 - Purpose:     Retrieve an llz record from an llz file.

 - Author:      Jan C. Depner (jan.depner@navy.mil)

 - Date:        08/31/06

 - Arguments:
                - hnd            =    The file handle
                - recnum         =    The record number of the llz
                                      record to be retrieved or
                                      LLZ_NEXT_RECORD (-1)
                - data           =    The returned llz record

 - Returns:
                - NVFalse on error or end of file
                - NVTrue

********************************************************************/

NV_BOOL read_llz (NV_INT32 hnd, NV_INT32 recnum, LLZ_REC *data)
{
  NV_INT64 pos;
  NV_INT32 tmpi;
  INTERNAL_LLZ llz;


  /*  Flush the buffer if the last thing we did was a write operation.  */

  if (llzh[hnd].write) fflush (llzh[hnd].fp);


  /*  Set recnum for LLZ_NEXT_RECORD  */

  if (recnum < 0) 
    {
      if (llz_recnum[hnd] == llzh[hnd].header.number_of_records) return (NVFalse);
      recnum = llz_recnum[hnd];
    }


  if (llzh[hnd].time_flag)
    {
      pos = (NV_INT64) recnum * (5L * (NV_INT64) sizeof (NV_INT32) + (NV_INT64) sizeof (NV_INT16)) + (NV_INT64) LLZ_HEADER_SIZE;
      fseeko64 (llzh[hnd].fp, pos, SEEK_SET);

      if ((fread (&llz.tv_sec, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fread (&llz.tv_nsec, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fread (&llz.lat, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fread (&llz.lon, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fread (&llz.dep, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fread (&llz.stat, sizeof (NV_U_INT16), 1, llzh[hnd].fp)) == 0) return (NVFalse);
    }
  else
    {
      /*  Version 1.00 files.  */

      if (llzh[hnd].major_version < 2)
        {
          pos = (NV_INT64) recnum * 4L * (NV_INT64) sizeof (NV_INT32) + (NV_INT64) LLZ_HEADER_SIZE;
          fseeko64 (llzh[hnd].fp, pos, SEEK_SET);

          llz.tv_sec = 0;
          llz.tv_nsec = 0;
          if ((fread (&llz.lat, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fread (&llz.lon, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fread (&llz.dep, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fread (&tmpi, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);

          if (llzh[hnd].swap)
            {
              swap_int (&tmpi);
              llz.stat = (NV_U_INT16) tmpi;
              swap_int ((NV_INT32 *) &llz.stat);
            }
          else
            {
              llz.stat = (NV_U_INT16) tmpi;
            }
        }
      else
        {
          pos = (NV_INT64) recnum * (3L * (NV_INT64) sizeof (NV_INT32) + (NV_INT64) sizeof (NV_INT16)) + (NV_INT64) LLZ_HEADER_SIZE;
          fseeko64 (llzh[hnd].fp, pos, SEEK_SET);

          llz.tv_sec = 0;
          llz.tv_nsec = 0;
          if ((fread (&llz.lat, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fread (&llz.lon, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fread (&llz.dep, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fread (&llz.stat, sizeof (NV_U_INT16), 1, llzh[hnd].fp)) == 0) return (NVFalse);
        }
    }


  /*  Set the next record number.  */

  llz_recnum[hnd]++;


  if (llzh[hnd].swap) swap_internal_llz (&llz);

  data->tv_sec = llz.tv_sec;
  data->tv_nsec = llz.tv_nsec;
  data->xy.lat = (NV_FLOAT64) llz.lat / 10000000.0L;
  data->xy.lon = (NV_FLOAT64) llz.lon / 10000000.0L;
  data->depth = (NV_FLOAT32) llz.dep / 10000.0L;
  data->status = (NV_U_INT32) llz.stat;


  llzh[hnd].at_end = NVFalse;
  llzh[hnd].write = NVFalse;


  return (NVTrue);
}


/********************************************************************/
/*!

 - Function:    append_llz

 - Purpose:     Store an llz record on the end of an llz file and update
                the number_of_records.

 - Author:      Jan C. Depner (jan.depner@navy.mil)

 - Date:        08/31/06

 - Arguments:
                - hnd            =    The file handle
                - data           =    The llz record

 - Returns:
                - NVFalse on error
                - NVTrue

********************************************************************/

NV_BOOL append_llz (NV_INT32 hnd, LLZ_REC data)
{
  INTERNAL_LLZ llz;


  /*  Flush the buffer if the last thing we did was a read operation.  */

  if (!llzh[hnd].write) fflush (llzh[hnd].fp);


  if (!llzh[hnd].at_end) fseeko64 (llzh[hnd].fp, 0L, SEEK_END);


  llz.tv_sec = data.tv_sec;
  llz.tv_nsec = data.tv_nsec;
  llz.lat = NINT (data.xy.lat * 10000000.0L);
  llz.lon = NINT (data.xy.lon * 10000000.0L);
  llz.dep = NINT (data.depth * 10000.0L);
  llz.stat = (NV_U_INT16) data.status;


  /*  Swap it if the file was originally swapped.  */

  if (llzh[hnd].swap) swap_internal_llz (&llz);


  if (llzh[hnd].time_flag)
    {
      if ((fwrite (&llz.tv_sec, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.tv_nsec, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.lat, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.lon, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.dep, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse); 
      if ((fwrite (&llz.stat, sizeof (NV_U_INT16), 1, llzh[hnd].fp)) == 0) return (NVFalse);
    }
  else
    {
      if ((fwrite (&llz.lat, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.lon, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.dep, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);


      /*  Version 1.00 files.  */

      if (llzh[hnd].major_version < 2)
        {
          if ((fwrite (&llz.stat, sizeof (NV_U_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
        }
      else
        {
          if ((fwrite (&llz.stat, sizeof (NV_U_INT16), 1, llzh[hnd].fp)) == 0) return (NVFalse);
        }
    }


  llzh[hnd].header.number_of_records++;
  llzh[hnd].size_changed = NVTrue;
  llzh[hnd].modified = NVTrue;
  llzh[hnd].write = NVTrue;
  llzh[hnd].at_end = NVTrue;

  return (NVTrue);
}


/********************************************************************/
/*!

 - Function:    update_llz

 - Purpose:     Store an llz record at the recnum record location in
                an llz file.

 - Author:      Jan C. Depner (jan.depner@navy.mil)

 - Date:        08/31/06

 - Arguments:
                - hnd            =    The file handle
                - recnum         =    The record number
                - data           =    The llz record

 - Returns:
                - NVFalse on error
                - NVTrue

********************************************************************/

NV_BOOL update_llz (NV_INT32 hnd, NV_INT32 recnum, LLZ_REC data)
{
  NV_INT64 pos;
  INTERNAL_LLZ llz;


  if (recnum > llzh[hnd].header.number_of_records - 1) return (NVFalse);


  /*  Flush the buffer if the last thing we did was a read operation.  */

  if (!llzh[hnd].write) fflush (llzh[hnd].fp);


  llz.tv_sec = data.tv_sec;
  llz.tv_nsec = data.tv_nsec;
  llz.lat = NINT (data.xy.lat * 10000000.0L);
  llz.lon = NINT (data.xy.lon * 10000000.0L);
  llz.dep = NINT (data.depth * 10000.0L);
  llz.stat = (NV_U_INT16) data.status;


  /*  Swap it if the file was originally swapped.  */

  if (llzh[hnd].swap) swap_internal_llz (&llz);


  if (llzh[hnd].time_flag)
    {
      pos = (NV_INT64) recnum * (5L * (NV_INT64) sizeof (NV_INT32) + (NV_INT64) sizeof (NV_INT16)) + (NV_INT64) LLZ_HEADER_SIZE;
      fseeko64 (llzh[hnd].fp, pos, SEEK_SET);

      if ((fwrite (&llz.tv_sec, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.tv_nsec, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.lat, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.lon, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.dep, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
      if ((fwrite (&llz.stat, sizeof (NV_U_INT16), 1, llzh[hnd].fp)) == 0) return (NVFalse);
    }
  else
    {
      /*  Version 1.00 files.  */

      if (llzh[hnd].major_version < 2)
        {
          pos = (NV_INT64) recnum * 4L * (NV_INT64) sizeof (NV_INT32) + (NV_INT64) LLZ_HEADER_SIZE;
          fseeko64 (llzh[hnd].fp, pos, SEEK_SET);

          llz.tv_sec = 0;
          llz.tv_nsec = 0;
          if ((fwrite (&llz.lat, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fwrite (&llz.lon, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fwrite (&llz.dep, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fwrite (&llz.stat, sizeof (NV_U_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
        }
      else
        {
          pos = (NV_INT64) recnum * (3L * (NV_INT64) sizeof (NV_INT32) + (NV_INT64) sizeof (NV_INT16)) + (NV_INT64) LLZ_HEADER_SIZE;
          fseeko64 (llzh[hnd].fp, pos, SEEK_SET);

          llz.tv_sec = 0;
          llz.tv_nsec = 0;
          if ((fwrite (&llz.lat, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fwrite (&llz.lon, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fwrite (&llz.dep, sizeof (NV_INT32), 1, llzh[hnd].fp)) == 0) return (NVFalse);
          if ((fwrite (&llz.stat, sizeof (NV_U_INT16), 1, llzh[hnd].fp)) == 0) return (NVFalse);
        }
    }


  llzh[hnd].modified = NVTrue;
  llzh[hnd].write = NVTrue;


  return (NVTrue);
}


/********************************************************************/
/*!

 - Function:    ftell_llz

 - Purpose:     Returns the location of the pointer within the LLZ file.

 - Author:      Jan C. Depner (jan.depner@navy.mil)

 - Date:        06/12/08

 - Arguments:   hnd            =    The file handle

 - Returns:     NV_INT32       =    location

********************************************************************/

NV_INT32 ftell_llz (NV_INT32 hnd)
{
  return (ftell (llzh[hnd].fp));
}
