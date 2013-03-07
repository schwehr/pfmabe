
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




#include "pfmWDB.h"
#include "pfmWDB_version.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



static FILE *fp = NULL;
static FILE *lock_fp = NULL;
static FILE *bak_fp = NULL;
static NV_CHAR pfmwdb[512];
static NV_CHAR pfmwdbmap[512];
static NV_CHAR lock_name[512];
static NV_CHAR bak_name[512];
static PFMWDBMAP_HEADER l_header;
static NV_INT32 record_size, status_pos, records_pos;
static NV_BOOL update_header = NVFalse;
static NV_BOOL opened = NVFalse;


/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_open

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Open the PFMWDB.map file and read the header.

   - Arguments:          
                         - header         =  PFMWDBMAP_HEADER structure returned

   - Return Value:
                         - NV_BOOL        =  NVFalse on error, otherwise NVTrue

**********************************************************************************************/

NV_BOOL PFMWDBMAP_open (PFMWDBMAP_HEADER *header)
{
  NV_CHAR varin[8192], info[8192];


  /*  Just in case you forgot to close the file.  */

  if (opened)
    {
      memcpy (header, &l_header, sizeof (PFMWDBMAP_HEADER));
      return (NVTrue);
    }


  /*  Check for the PFMWDB environment variable  */

  if (getenv ("PFMWDB") == NULL)
    {
      fprintf (stderr, "The PFMWDB environment variable is not set.\nThis must point to the folder that contains the one-degree PFM files.");
      fflush (stderr);
      return (NVFalse);
    }


  strcpy (pfmwdb, getenv ("PFMWDB"));


  sprintf (pfmwdbmap, "%s%1cPFMWDB.map", pfmwdb, SEPARATOR);


  /*  Set the timezone to GMT.  */

#ifdef NVWIN3X
  #ifdef __MINGW64__
      putenv ("TZ=GMT");
      tzset ();
  #else
      _putenv ("TZ=GMT");
      _tzset ();
  #endif
#else
  putenv ("TZ=GMT");
  tzset ();
#endif


  /*  Open the file.  */

  if ((fp = fopen (pfmwdbmap, "rb+")) == NULL)
    {
      if ((fp = fopen (pfmwdbmap, "rb")) == NULL) return (NVFalse);
    }


  /*  We want to try to read the first line (version info) with an fread in case we mistakenly asked to load a binary file.
      If we try to use ngets to read a binary file and there are no line feeds in the first sizeof (varin) characters we
      would segfault since fgets doesn't check for overrun.  */

  if (!fread (varin, 128, 1, fp))
    {
      fprintf (stderr, "The header for the file %s has been corrupted!", pfmwdbmap);
      fflush (stderr);
      fclose (fp);
      fp = NULL;
      return (NVFalse);
    }


  /*  Check the version.  */

  if (!strstr (varin, "pfmWDBMap library") && !strstr (varin, "pfmWDB library"))
    {
      fprintf (stderr, "The header for the file %s has been corrupted!", pfmwdbmap);
      fflush (stderr);
      fclose (fp);
      fp = NULL;
      return (NVFalse);
    }


  /*  Rewind to the beginning of the file now that we know we have the right kind of file.  */

  fseek (fp, 0, SEEK_SET);


  /*  Read the version again (so we don't have to parse the string to find the line feed).  Note, we're using ngets instead
      of fgets since we really don't want the CR and/or LF in the strings.  */

  ngets (varin, sizeof (varin), fp);
  get_string (varin, info);
  strcpy (header->version, info);


  /*  Now parse the rest of the header.  */

  while (ngets (varin, sizeof (varin), fp))
    {
      if (strstr (varin, "[END OF HEADER]")) break;


      /*  Put everything to the right of the equals sign into 'info'.   */

      get_string (varin, info);

      if (strstr (varin, "[CREATION DATE/TIME]")) strcpy (header->creation_date_time, info);
      if (strstr (varin, "[MODIFICATION DATE/TIME]")) strcpy (header->modification_date_time, info);
      if (strstr (varin, "[CLASSIFICATION]")) strcpy (header->classification, info);
      if (strstr (varin, "[NUMBER OF RECORDS]")) sscanf (info, NV_INT64_SPECIFIER, &header->records);
      if (strstr (varin, "[NUMBER OF FILES]")) sscanf (info, "%d", &header->files);
      if (strstr (varin, "[HEADER SIZE]")) sscanf (info, "%d", &header->header_size);
      if (strstr (varin, "[STATUS BITS]")) sscanf (info, "%d", &header->status_bits);
      if (strstr (varin, "[RECORDS BITS]")) sscanf (info, "%d", &header->records_bits);
    }


  record_size = (header->status_bits + header->records_bits) / 8;
  if ((header->status_bits + header->records_bits) % 8) record_size++;

  status_pos = 0;
  records_pos = status_pos + header->status_bits;

  memcpy (&l_header, header, sizeof (PFMWDBMAP_HEADER));


  opened = NVTrue;


  return (NVTrue);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_close

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Close the PFMWDB.map file and write the header if it has been modified.

   - Arguments:          void

   - Return Value:       void

**********************************************************************************************/

void PFMWDBMAP_close ()
{
  if (update_header) PFMWDBMAP_write_header (&l_header);

  opened = NVFalse;

  fclose (fp);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_write_header

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Write the PFMWDB.map file header.

   - Arguments:
                         - header         =  PFMWDBMAP_HEADER structure to be written

   - Return Value:
                         - NV_BOOL        =  NVFalse on error, otherwise NVTrue

**********************************************************************************************/

NV_BOOL PFMWDBMAP_write_header (PFMWDBMAP_HEADER *header)
{
  NV_CHAR space = ' ';
  NV_U_BYTE zero = 0;
  NV_INT32 i, size;
  time_t systemtime;


  systemtime = time (&systemtime);


  //  If we haven't already opened the file we have to create and initialize the header.  */

  if (fp == NULL)
    {
      /*  Check for the PFMWDB environment variable  */

      if (getenv ("PFMWDB") == NULL)
        {
          fprintf (stderr, "The PFMWDB environment variable is not set.\nThis must point to the folder that contains the one-degree PFM files.");
          fflush (stderr);
          return (NVFalse);
        }


      strcpy (pfmwdb, getenv ("PFMWDB"));


      sprintf (pfmwdbmap, "%s%1cPFMWDB.map", pfmwdb, SEPARATOR);


      /*  Open the file.  */

      if ((fp = fopen (pfmwdbmap, "wb+")) == NULL) return (NVFalse);


      /*  Set the version.  */

      strcpy (header->version, PFMWDB_VERSION);


      /*  Set the header size.  */

      header->header_size = PFMWDBMAP_HEADER_SIZE;


      /*  Set the timezone to GMT.  */

#ifdef NVWIN3X
  #ifdef __MINGW64__
      putenv ("TZ=GMT");
      tzset ();
  #else
      _putenv ("TZ=GMT");
      _tzset ();
  #endif
#else
      putenv ("TZ=GMT");
      tzset ();
#endif


      /*  Save the creation time.  */

      strcpy (header->creation_date_time, asctime (localtime (&systemtime)));
      header->creation_date_time[strlen (header->creation_date_time) - 1] = 0;


      /*  Set the field sizes (40 bits covers over a trillion points).  */

      header->status_bits = PFMWDB_STATUS_BITS;
      header->records_bits = PFMWDB_RECORDS_BITS;

      record_size = (header->status_bits + header->records_bits) / 8;
      if ((header->status_bits + header->records_bits) % 8) record_size++;

      status_pos = 0;
      records_pos = status_pos + header->status_bits;


      /*  Zero the record and file counts.  */

      header->records = 0;
      header->files = 0;


      /*  Clear the classification.  */

      strcpy (header->classification, "");


      /*  Zero fill the entire file.  */

      size = 180 * 360 * record_size;

      fseek (fp, header->header_size, SEEK_SET);

      for (i = 0 ; i < size ; i++) fwrite (&zero, 1, 1, fp);
    }


  memcpy (&l_header, header, sizeof (PFMWDBMAP_HEADER));


  if (fseek (fp, 0, SEEK_SET) < 0) return (NVFalse);

  fprintf (fp, "[VERSION] = %s\n", header->version);


  fprintf (fp, "[HEADER SIZE] = %d\n", header->header_size);

  fprintf (fp, "[CREATION DATE/TIME] = %s\n", header->creation_date_time);
  fprintf (fp, "[MODIFICATION DATE/TIME] = %s", asctime (localtime (&systemtime)));

  fprintf (fp, "[SECURITY CLASSIFICATION] = %s\n", header->classification);

  fprintf (fp, "[NUMBER OF RECORDS] = "NV_INT64_SPECIFIER"\n", header->records);

  fprintf (fp, "[NUMBER OF FILES] = %d\n", header->files);

  fprintf (fp, "[STATUS BITS] = %d\n", header->status_bits);
  fprintf (fp, "[RECORDS BITS] = %d\n", header->records_bits);

  fprintf (fp, "[END OF HEADER]\n");


  /*  Space fill the rest.  */

  size = header->header_size - ftell (fp);


  for (i = 0 ; i < size ; i++) fwrite (&space, 1, 1, fp);


  fflush (fp);


  update_header = NVFalse;


  return (NVTrue);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_read_record

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Read a PFMWDB.map record.

   - Arguments:
                         - lat            =  Integer latitude degrees of desired one-degree cell
                         - lon            =  Integer longitude degrees of desired one-degree cell

   - Return Value:
                         - PFMWDBMAP_RECORD =  PFMWDBMAP_RECORD read for the desired cell

**********************************************************************************************/

PFMWDBMAP_RECORD PFMWDBMAP_read_record (NV_INT32 lat, NV_INT32 lon)
{
  NV_U_BYTE buffer[1024];
  PFMWDBMAP_RECORD record;
  NV_INT32 offset, status;

  offset = ((lat + 90) * 360 + (lon + 180)) * record_size + l_header.header_size;

  fseek (fp, offset, SEEK_SET);
  fread (buffer, record_size, 1, fp);


  status = bit_unpack (buffer, status_pos, l_header.status_bits);
  record.P = (NV_BOOL) ((status & PFMWDB_P_MASK) > PFMWDB_P_SHIFT);
  record.U = (NV_BOOL) ((status & PFMWDB_U_MASK) > PFMWDB_U_SHIFT);
  record.C = (NV_BOOL) ((status & PFMWDB_C_MASK) > PFMWDB_C_SHIFT);
  record.S = (NV_BOOL) ((status & PFMWDB_S_MASK) > PFMWDB_S_SHIFT);
  record.checked = (NV_BOOL) ((status & PFMWDB_CHECKED_MASK) > PFMWDB_CHECKED_SHIFT);
  record.verified = (NV_BOOL) ((status & PFMWDB_VERIFIED_MASK) > PFMWDB_VERIFIED_SHIFT);
  record.deleted = (NV_BOOL) ((status & PFMWDB_DELETED_MASK) > PFMWDB_DELETED_SHIFT);

  record.records = double_bit_unpack (buffer, records_pos, l_header.records_bits);

  return (record);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_write_record

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Write a PFMWDB.map record.

   - Arguments:
                         - lat            =  Integer latitude degrees of desired one-degree cell
                         - lon            =  Integer longitude degrees of desired one-degree cell
                         - PFMWDBMAP_RECORD =  PFMWDBMAP_RECORD to be written

   - Return Value:
                         - void

**********************************************************************************************/

void PFMWDBMAP_write_record (NV_INT32 lat, NV_INT32 lon, PFMWDBMAP_RECORD record)
{
  NV_U_BYTE buffer[1024];
  NV_INT32 offset, status;

  status = 0;
  if (record.P) status |= PFMWDB_P_MASK;
  if (record.U) status |= PFMWDB_U_MASK;
  if (record.C) status |= PFMWDB_C_MASK;
  if (record.S) status |= PFMWDB_S_MASK;
  if (record.checked) status |= PFMWDB_CHECKED_MASK;
  if (record.verified) status |= PFMWDB_VERIFIED_MASK;
  if (record.deleted) status |= PFMWDB_DELETED_MASK;
  bit_pack (buffer, status_pos, l_header.status_bits, status);

  double_bit_pack (buffer, records_pos, l_header.records_bits, record.records);

  offset = ((lat + 90) * 360 + (lon + 180)) * record_size + l_header.header_size;

  fseek (fp, offset, SEEK_SET);
  fwrite (buffer, record_size, 1, fp);
  fflush (fp);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_append_file

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Appends an input file name, data type, security key, and datum to the
                         PFMWDB.map file list.  Adds a "+" to the beginning of the file name
                         to indicate that it is not deleted.

   - Arguments:
                         - filename       =  Name of file (not fully qualified)
                         - type           =  PFM data type of file (e.g. PFM_GSF_DATA)
                         - security       =  Security key of file (see security.cfg)
                         - datum          =  Datum type of file (from PFMWDB/GSF vertical datums)

   - Return Value:
                         - void

**********************************************************************************************/

void PFMWDBMAP_append_file (const NV_CHAR *filename, NV_INT16 type, NV_INT16 security, NV_INT16 datum)
{
  NV_CHAR name[512];


  /*  Strip /PFMWDB:: from name if present.  */

  if (strstr (filename, "/PFMWDB::"))
    {
      strcpy (name, &filename[9]);
    }
  else
    {
      strcpy (name, filename);
    }


  fseek (fp, 0, SEEK_END);


  /*  Deletion flag (+/-) then the filename.  */

  fprintf (fp, "+ %05hd %05hd %05hd %s\n", type, security, datum, name);
  fflush (fp);


  l_header.files++;
  update_header = NVTrue;
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_read_file

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Reads the entire list file, one name at a time.

   - Arguments:
                         - filename       =  Name of file (not fully qualified)
                         - deleted        =  NVTrue if file is marked as PFM_DELETED
                         - type           =  PFM data type of file
                         - security       =  Security key of file (see security.cfg)
                         - datum          =  Datum type of file (from PFMWDB/GSF vertical datums)

   - Return Value:
                         - NV_BOOL        =  NVFalse on EOF, otherwise NVTrue

**********************************************************************************************/

NV_BOOL PFMWDBMAP_read_file (NV_CHAR *filename, NV_BOOL *deleted, NV_INT16 *type, NV_INT16 *security, NV_INT16 *datum)
{
  static NV_INT32 num = 0;
  NV_CHAR string[1024], dflag;


  /*  If we've hit the end, reset num and return NVFalse.  */

  if (num >= l_header.files)
    {
      num = 0;
      return (NVFalse);
    }


  /*  Starting over.  */

  if (!num) fseek (fp, l_header.header_size + 64800 * record_size, SEEK_SET);


  ngets (string, 512, fp);

  sscanf (string, "%1c %hd %hd %hd %s", &dflag, type, security, datum, filename);

  if (dflag == '+')
    {
      *deleted = NVFalse;
    }
  else
    {
      *deleted = NVTrue;
    }


  num++;

  return (NVTrue);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_mark_file

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Marks a file as deleted, changes security key and/or datum key.

   - Arguments:
                         - filename       =  Name of file (not fully qualified)
                         - delete         =  < 0 to delete, 0 for no change, > 0 to un-delete
                         - itype          =  Noty used at this time
                         - isecurity      =  Security key to set for file (-1 = no change)
                         - idatum         =  Datum type of file (0 = no change)

   - Return Value:
                         - NV_BOOL        =  NVFalse on error, otherwise NVTrue

**********************************************************************************************/

NV_BOOL PFMWDBMAP_mark_file (const NV_CHAR *filename, NV_INT16 idelete, NV_INT16 itype __attribute__ ((unused)), NV_INT16 isecurity, NV_INT16 idatum)
{
  NV_CHAR l_filename[512], realname[512], string[1024], dflag;
  NV_INT16 type, security, datum;
  NV_INT32 pos = 0, i;


  fseek (fp, l_header.header_size + 64800 * record_size, SEEK_SET);


  /*  Strip the "/PFMWDB::" off of the file name if it's there.  */

  if (strstr (filename, "PFMWDB::"))
    {
      sprintf (realname, &filename[9]);
    }
  else
    {
      strcpy (realname, filename);
    }


  /*  Loop through the list looking for a match.  */

  for (i = 0 ; i < l_header.files ; i++)
    {
      pos = ftell (fp);
      ngets (string, 512, fp);


      sscanf (string, "%1c %hd %hd %hd %s", &dflag, &type, &security, &datum, l_filename);

      if (!strcmp (l_filename, realname))
        {
          fseek (fp, pos, SEEK_SET);
          if (idelete < 0)
            {
              dflag = '-';
            }
          else if (idelete > 0)
            {
              dflag = '+';
            }
          else if (isecurity >= 0 || idatum)
            {
              if (idatum) datum = idatum;
              if (isecurity >= 0) security = isecurity;
            }

          fprintf (fp, "%1c %05hd %05hd %05hd", dflag, type, security, datum);
          fflush (fp);

          return (NVTrue);
        }
    }

  return (NVFalse);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_delete_file

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Changes the "+" mark at the beginning of a file name to "-" to
                         indicate that it has been marked as PFM_DELETED.  The filename may
                         include the "/PFMWDB::" prefix or not.

   - Arguments:
                         - filename       =  Name of file (not fully qualified)

   - Return Value:
                         - NV_BOOL        =  NVFalse on error, otherwise NVTrue

**********************************************************************************************/

NV_BOOL PFMWDBMAP_delete_file (const NV_CHAR *filename)
{
  return (PFMWDBMAP_mark_file (filename, -1, -1, -1, 0));
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_restore_file

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Changes the "-" mark at the beginning of a file name to "+" to
                         indicate that it has been unmarked as PFM_DELETED.  The filename may
                         include the "/PFMWDB::" prefix or not.

   - Arguments:
                         - filename       =  Name of file (not fully qualified)

   - Return Value:
                         - NV_BOOL        =  NVFalse on error, otherwise NVTrue

**********************************************************************************************/

NV_BOOL PFMWDBMAP_restore_file (const NV_CHAR *filename)
{
  return (PFMWDBMAP_mark_file (filename, 1, -1, -1, 0));
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_flag_file

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Writes security key and/or datum changes to the PFMWDB master file
                         entry for the selected file.  The filename may include the
                         "/PFMWDB::" prefix or not.

   - Arguments:
                         - filename       =  Name of file (not fully qualified)
                         - security       =  Security key to set for file (-1 = no change)
                         - datum          =  Datum type of file (0 = no change)

   - Return Value:
                         - NV_BOOL        =  NVFalse on error, otherwise NVTrue

**********************************************************************************************/

NV_BOOL PFMWDBMAP_flag_file (const NV_CHAR *filename, NV_INT16 security, NV_INT16 datum)
{
  return (PFMWDBMAP_mark_file (filename, 0, -1, security, datum));
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_file_cleanup

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Clears all "-" marked files, updates the file count, and truncates the
                         file length.

   - Arguments:
                         - void

   - Return Value:
                         - NV_BOOL        =  NVFalse on error, otherwise NVTrue

**********************************************************************************************/

NV_BOOL PFMWDBMAP_file_cleanup ()
{
  NV_CHAR l_filename[512], temp_file[512], string[1024], dflag;
  NV_INT16 type, security, datum;
  NV_INT32 i, count, stat, fd, pos;
  FILE *tfp;


  sprintf (temp_file, "%s%1ctemp_file", pfmwdb, SEPARATOR);

  if ((tfp = fopen (temp_file, "w+")) == NULL)
    {
      perror (temp_file);
      return (NVFalse);
    }


  /*  Move to where we put the file list.  */

  fseek (fp, l_header.header_size + 64800 * record_size, SEEK_SET);


  /*  Read the non-deleted file names and write them out to the temporary file.  */

  count = 0;
  while (ngets (string, 1024, fp) != NULL)
    {
      sscanf (string, "%1c %hd %hd %hd %s", &dflag, &type, &security, &datum, l_filename);

      if (dflag == '+')
        {
          fprintf (tfp, "+ %05hd %05hd %05hd %s\n", type, security, datum, l_filename);
          count++;
        }
    }


  /*  Rewind the temporary file.  */

  fseek (tfp, 0, SEEK_SET);
  fseek (fp, l_header.header_size + 64800 * record_size, SEEK_SET);


  /*  Read the temporary file list and replace the PFMWDB.map file list.  */

  for (i = 0 ; i < count ; i++)
    {
      ngets (string, 1024, tfp);
      fprintf (fp, "%s\n", string);
    }


  /*  Get the position at which to truncate the file.  */

  pos = ftell (fp);


  /*  Close and remove the temporary file.  */

  fclose (tfp);
  remove (temp_file);


  /*  Update the file count and write the header.  */

  l_header.files = count;
  PFMWDBMAP_write_header (&l_header);


  /*  Close the PFMWDB.map file so we can truncate it.  */

  fclose (fp);


  /*  Truncate the file to the new length.  */

  fd = open (pfmwdbmap, O_RDWR);

  if (fd < 0) return (NVFalse);


#ifdef NVWIN3X
  #ifdef __MINGW64__
  stat = chsize (fd, pos);
  #else
  stat = _chsize (fd, pos);
  #endif
#else
  stat = ftruncate (fd, pos);
#endif

  if (stat) return (NVFalse);

  close (fd);


  /*  Now open it back up with a normal FILE pointer.  */

  fp = fopen (pfmwdbmap, "rb+");


  return (NVTrue);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_recompute

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Recomputes the number of records in the entire PFMWDB and the overall
                         classification for the PFMWDB.

   - Arguments:          void

   - Return Value:       void

**********************************************************************************************/

void PFMWDBMAP_recompute ()
{
  NV_INT32 lat, lon;
  PFMWDBMAP_RECORD record;

  l_header.records = 0;
  strcpy (l_header.classification, "");

  NV_BOOL cls[4] = {0, 0, 0, 0};
  for (lat = -90 ; lat < 90 ; lat++)
    {
      for (lon = -180 ; lon < 180 ; lon++)
        {
          record = PFMWDBMAP_read_record (lat, lon);

          l_header.records += record.records;

          if (record.P) cls[0] = NVTrue;
          if (record.U) cls[1] = NVTrue;
          if (record.C) cls[2] = NVTrue;
          if (record.S) cls[3] = NVTrue;
        }
    }

  if (cls[0]) strcpy (l_header.classification, "*** UNCLASSIFIED PUBLIC RELEASE ***");
  if (cls[1]) strcpy (l_header.classification, "*** UNCLASSIFIED RESTRICTED ***");
  if (cls[2]) strcpy (l_header.classification, "*** CONFIDENTIAL ***");
  if (cls[3]) strcpy (l_header.classification, "*** SECRET ***");

  PFMWDBMAP_write_header (&l_header);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_is_read_only

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Determines if the PFMWDB should be treated as read only by the current
                         user.  This is determined by checking to see if the user has read only
                         access to the PFMWDB.map file.

   - Arguments:
                         - void

   - Return Value:
                         - NV_BOOL        =  NVTrue if PFMWDB.map file is read only for the
                                             current use, otherwise NVFalse

**********************************************************************************************/

NV_BOOL PFMWDB_is_read_only ()
{
  FILE *lfp;


  /*  Check for the PFMWDB environment variable  */

  if (getenv ("PFMWDB") == NULL)
    {
      fprintf (stderr, "The PFMWDB environment variable is not set.\nThis must point to the folder that contains the one-degree PFM files.");
      fflush (stderr);
      return (NVFalse);
    }


  strcpy (pfmwdb, getenv ("PFMWDB"));


  sprintf (pfmwdbmap, "%s%1cPFMWDB.map", pfmwdb, SEPARATOR);


  /*  Try to open the file for update.  */

  if ((lfp = fopen (pfmwdbmap, "rb+")) == NULL) return (NVTrue);


  fclose (lfp);

  return (NVFalse);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_open_lock_file

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Opens the lock/recovery file.

   - Arguments:
                         - void

   - Return Value:
                         - FILE           =  lock/recovery file pointer, NULL on error

**********************************************************************************************/

FILE *PFMWDB_open_lock_file ()
{
  FILE                *map_fp;
  NV_CHAR             map_name[512];
  NV_U_BYTE           *buffer;
  NV_INT32            ret;
  time_t              systemtime;


  /*  Check for the PFMWDB environment variable  */

  if (getenv ("PFMWDB") == NULL)
    {
      fprintf (stderr, "The PFMWDB environment variable is not set.\nThis must point to the folder that contains the one-degree PFM files.");
      fflush (stderr);
      return (NULL);
    }


  strcpy (pfmwdb, getenv ("PFMWDB"));


  sprintf (lock_name, "%s%1cpfmWDB_lock.rc", pfmwdb, SEPARATOR);
  if ((lock_fp = fopen (lock_name, "w")) == NULL)
    {
      perror (lock_name);
      return (NULL);
    }


  //  Make a copy of the PFMWDB.map file so we can roll it back if we abort.

  sprintf (pfmwdbmap, "%s%1cPFMWDB.map", pfmwdb, SEPARATOR);

  sprintf (map_name, "%s%1cPFMWDB.map", pfmwdb, SEPARATOR);
  if ((map_fp = fopen (map_name, "r")) == NULL)
    {
      perror (map_name);
      fclose (lock_fp);
      remove (lock_name);
      return (NULL);
    }

  sprintf (bak_name, "%s%1cPFMWDB.bak", pfmwdb, SEPARATOR);
  if ((bak_fp = fopen (bak_name, "w")) == NULL)
    {
      perror (bak_name);
      fclose (lock_fp);
      remove (lock_name);
      fclose (map_fp);
      return (NULL);
    }

  buffer = (NV_U_BYTE *) malloc (8192);
  if (buffer == NULL)
    {
      perror ("Allocating buffer memory in PFMWDB_open_lock_file");
      fclose (lock_fp);
      remove (lock_name);
      fclose (map_fp);
      fclose (bak_fp); 
      remove (bak_name);
      return (NULL);
    }

  while ((ret = fread (buffer, 1, 8192, map_fp))) fwrite (buffer, 1, ret, bak_fp);

  fclose (map_fp);
  fclose (bak_fp);
  free (buffer);


  /*  Add the startup time as a comment.  */

  systemtime = time (&systemtime);
  fprintf (lock_fp, "#%s\n", asctime (localtime (&systemtime)));
  fflush (lock_fp);


  /*  Add the backup file cleanup line to the lock file.  */

  fprintf (lock_fp, "mv %s %s\n", bak_name, map_name);
  fflush (lock_fp);


  return (lock_fp);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_close_lock_file

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Closes the lock/recovery file.

   - Arguments:          void

   - Return Value:       void

**********************************************************************************************/

void PFMWDB_close_lock_file ()
{
  fclose (lock_fp);
  remove (lock_name);
  remove (bak_name);
}



/*********************************************************************************************/
/*!

   - Module Name:        PFMWDBMAP_is_locked

   - Programmer(s):      Jan C. Depner

   - Date Written:       02/11/10

   - Purpose:            Determines if the PFMWDB is locked for loading/update.  This is
                         determined by checking to see if the lock/recovery file is present.

   - Arguments:
                         - void

   - Return Value:
                         - NV_BOOL        =  NVTrue if PFMWDB is locked for loading/update,
                                             otherwise NVFalse

*********************************************************************************************/

NV_BOOL PFMWDB_is_locked ()
{
  struct stat buf;


  /*  Check for the PFMWDB environment variable  */

  if (getenv ("PFMWDB") == NULL)
    {
      fprintf (stderr, "The PFMWDB environment variable is not set.\nThis must point to the folder that contains the one-degree PFM files.");
      fflush (stderr);
      return (NVFalse);
    }


  strcpy (pfmwdb, getenv ("PFMWDB"));


  sprintf (lock_name, "%s%1cpfmWDB_lock.rc", pfmwdb, SEPARATOR);


  /*  Check to see if the file exists.  */

  if (stat (lock_name, &buf)) return (NVFalse);

  return (NVTrue);
}
