#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "FileImage.h"

static NV_BOOL swap = NVFalse;
static IMAGE_HEADER_T l_head;
static IMAGE_INDEX_T *records = NULL;
static NV_INT32 old = 0;


static void swap_image_header (IMAGE_INFO_T *info)
{
  swap_NV_INT32 (&info->ImageHeaderSize);
  swap_NV_INT32 (&info->ImageTextHeaderSize);
  swap_NV_INT32 (&info->IndexSize);
  swap_NV_INT32 (&info->ImageIndexEntrySize);
  swap_NV_INT32 (&info->number_images);

  swap_NV_INT64 (&info->start_timestamp);
  swap_NV_INT64 (&info->end_timestamp);

  swap_NV_INT16 ((NV_INT16 *) &info->flightline);
}


NV_INT32 image_read_header (FILE *fp, IMAGE_HEADER_T *head)
{
  NV_INT32    ret;
  NV_INT64    long_pos;
  NV_CHAR     varin[1024], info[1024];

  NV_CHAR *ngets (NV_CHAR *s, NV_INT32 size, FILE *stream);
  NV_INT32 big_endian ();


  swap = NVFalse;


  /*  Check for the new format file.  If the first four characters are 
      "File" it's the new format, otherwise read the binary header.  */

  fseeko64 (fp, 0LL, SEEK_SET);
  ngets (varin, sizeof (varin), fp);

  if (!strncmp (varin, "File", 4))
    {
      fseeko64 (fp, 0LL, SEEK_SET);


      /*  Read each entry.    */

      head->text.header_size = 0;
      while (ngets (varin, sizeof (varin), fp) != NULL)
        {
          if (!strcmp (varin, "EOF")) break;


          /*  Put everything to the right of the colon into 'info'.   */
            
          if (strchr (varin, ':') != NULL) strcpy (info, (NV_CHAR *) (strchr (varin, ':') + 1));


          /*  Check input for matching strings and load values if found.  */
            
          if (strstr (varin, "FileType:") != NULL) lidar_get_string (varin, head->text.file_type);

          if (strstr (varin, "EndianType:") != NULL)
            {
              if (strstr (info, "Little"))
                {
                  head->text.endian = NVFalse;
                  if (big_endian ()) swap = NVTrue;
                }
              else
                {
                  head->text.endian = NVTrue;
                  if (!big_endian ()) swap = NVTrue;
                }
            }


          if (strstr (varin, "SoftwareVersionNumber:") != NULL) sscanf (info, "%f", &head->text.software_version);

          if (strstr (varin, "FileVersionNumber:") != NULL) sscanf (info, "%f", &head->text.file_version);


          if (strstr (varin, "Downloaded By:") != NULL) lidar_get_string (varin, head->text.UserName);


          if (strstr (varin, "HeaderSize:") != NULL) sscanf (info, "%d", &head->text.header_size);

          if (strstr (varin, "TextBlockSize:") != NULL) sscanf (info, "%d", &head->text.text_block_size);

          if (strstr (varin, "BinaryBlockSize:") != NULL) sscanf (info, "%d", &head->text.bin_block_size);


          if (strstr (varin, "Project:") != NULL) lidar_get_string (varin, head->text.project);

          if (strstr (varin, "Mission:") != NULL) lidar_get_string (varin, head->text.mission);

          if (strstr (varin, "Dataset:") != NULL) lidar_get_string (varin, head->text.dataset);

          if (strstr (varin, "FlightlineNumber:") != NULL) lidar_get_string (varin, head->text.flightline_number);

          if (strstr (varin, "CodedFLNumber:") != NULL) sscanf (info, "%hd", &head->text.coded_fl_number);

          if (strstr (varin, "FlightDate:") != NULL) lidar_get_string (varin, head->text.flight_date);

          if (strstr (varin, "StartTime:") != NULL) lidar_get_string (varin, head->text.start_time);

          if (strstr (varin, "EndTime:") != NULL) lidar_get_string (varin, head->text.end_time);

          if (strstr (varin, "StartTimestamp:") != NULL) sscanf (info, NV_INT64_SPECIFIER, &head->text.start_timestamp);

          if (strstr (varin, "EndTimestamp:") != NULL) sscanf (info, NV_INT64_SPECIFIER, &head->text.end_timestamp);

          if (strstr (varin, "NumberImages:") != NULL) sscanf (info, "%d", &head->text.number_images);

          if (strstr (varin, "IndexRecordSize:") != NULL) sscanf (info, "%d", &head->text.record_size);

          if (strstr (varin, "IndexBlockSize:") != NULL) sscanf (info, "%d", &head->text.block_size);

          if (strstr (varin, "FileCreateDate:") != NULL) lidar_get_string (varin, head->text.dataset_create_date);

          if (strstr (varin, "FileCreateTime:") != NULL) lidar_get_string (varin, head->text.dataset_create_time);

          long_pos = ftello64 (fp);
          if (head->text.header_size && long_pos >= head->text.header_size) break;
        }


      /*  Make sure we're past the header.  */

      fseeko64 (fp, (NV_INT64) head->text.header_size, SEEK_SET);

      ret = 0;
    }
  else
    {
      /*  Early versions were screwed up.  */

      fseeko64 (fp, (NV_INT64) IMAGE_HEAD_SIZE, SEEK_SET);

      fread (&head->info, sizeof (IMAGE_INFO_T), 1, fp);


      /*  Swap the INFO block if needed.  */

      if (swap) swap_image_header (&head->info);


      head->text.start_timestamp = head->info.start_timestamp;
      head->text.end_timestamp = head->info.end_timestamp;
      head->text.number_images = head->info.number_images;

      ret = 1;
    }


  head->text.data_size = l_head.text.data_size;


  return (ret);
}



FILE *open_image_file (NV_CHAR *path)
{
  FILE                  *fp;
  NV_INT32              i;
  OLD_IMAGE_INDEX_T     old_record;


  NV_INT32 big_endian ();


  swap = (NV_BOOL) big_endian ();


  l_head.text.data_size = 0;


  /*  Brute force!  Load the index into memory, it ain't big anyway.  */

  if ((fp = fopen64 (path, "rb")) == NULL)
    {
      perror (path);
    }
  else
    {
      old = image_read_header (fp, &l_head);

      if (records) free (records);

      records = (IMAGE_INDEX_T *) calloc (l_head.text.number_images, sizeof (IMAGE_INDEX_T));

      if (records == NULL)
        {
          perror ("Allocating image index");
          exit (-1);
        }

      if (old)
        {
          for (i = 0 ; i < l_head.text.number_images ; i++)
            {
              fread (&old_record, sizeof (OLD_IMAGE_INDEX_T), 1, fp);
                
              if (swap)
                {
                  swap_NV_INT64 (&old_record.timestamp);
                  swap_NV_INT64 (&old_record.byte_offset);

                  swap_NV_INT32 (&old_record.image_size);
                  swap_NV_INT32 (&old_record.image_number);
                }
              records[i].timestamp = old_record.timestamp;
              records[i].byte_offset = old_record.byte_offset;
              records[i].image_size = old_record.image_size;
              records[i].image_number = old_record.image_number;

              l_head.text.data_size += records[i].image_size;
            }
        }
      else
        {
          for (i = 0 ; i < l_head.text.number_images ; i++)
            {
              fread (&records[i], sizeof (IMAGE_INDEX_T), 1, fp);

              if (swap)
                {
                  swap_NV_INT64 (&records[i].timestamp);
                  swap_NV_INT64 (&records[i].byte_offset);

                  swap_NV_INT32 (&records[i].image_size);
                  swap_NV_INT32 (&records[i].image_number);
                }

              l_head.text.data_size += records[i].image_size;
            }
        }
    }

  return (fp);
}


/*  Note that we're counting from 1 not 0.  Not my idea!  */

NV_INT32 image_get_metadata (FILE *fp, NV_INT32 rec_num, IMAGE_INDEX_T *image_index)
{
  NV_INT32     real_num;


  real_num = rec_num - 1;

  if (real_num < 0 || real_num > l_head.text.number_images) return (-1);

  image_index->timestamp = records[real_num].timestamp;
  image_index->byte_offset = records[real_num].byte_offset;
  image_index->image_size = records[real_num].image_size;
  image_index->image_number = records[real_num].image_number;

  return (0);
}


/*  Returns the nearest record number to "timestamp".  Note that we're counting
    from 1 not 0.  Not my idea!  */

NV_INT32 image_find_record (FILE *fp, NV_INT64 timestamp)
{
  NV_INT32        i, j;


  if (timestamp < l_head.text.start_timestamp || timestamp > l_head.text.end_timestamp) return (0);

  j = -1;
  for (i = 0 ; i < l_head.text.number_images ; i++)
    {
      if (timestamp <= records[i].timestamp)
        {
          if (!i)
            {
              j = i;
            }
          else
            {
              if ((timestamp - records[i - 1].timestamp) < (records[i].timestamp - timestamp))
                {
                  j = i - 1;
                }
              else
                {
                  j = i;
                }
            }
          break;
        }
    }

  return (j + 1);
}



/*  This function tries to find the record based on the timestamp.  Returns NULL on failure or the 
    image if it succeeds.  You must free the image in the calling program.  */

NV_U_CHAR *image_read_record (FILE *fp, NV_INT64 timestamp, NV_U_INT32 *size, NV_INT64 *image_time)
{
  NV_U_CHAR       *image;
  NV_INT32        i, j;


  if (timestamp < l_head.text.start_timestamp || timestamp > l_head.text.end_timestamp) return (NULL);

  j = -1;
  for (i = 0 ; i < l_head.text.number_images ; i++)
    {
      if (timestamp <= records[i].timestamp)
        {
          if (!i)
            {
              j = i;
            }
          else
            {
              if ((timestamp - records[i - 1].timestamp) < (records[i].timestamp - timestamp))
                {
                  j = i - 1;
                }
              else
                {
                  j = i;
                }
            }
          break;
        }
    }


  /*  Make sure we got an image.  */

  if (j == -1 || records[j].image_size == 0) return ((NV_U_CHAR *) NULL);


  *image_time = records[j].timestamp;


  image = (NV_U_CHAR *) malloc (records[j].image_size);
  if (image == NULL)
    {
      perror ("Allocating image memory");
      exit (-1);
    }


  fseeko64 (fp, records[j].byte_offset, SEEK_SET);
  fread (image, records[j].image_size, 1, fp);

  *size = records[j].image_size;

  return (image);
}


/*  This call reads the image at record "recnum".  Returns NULL on failure or the 
    image if it succeeds.  You must free the image in the calling program.  */

NV_U_CHAR *image_read_record_recnum (FILE *fp, NV_INT32 recnum, NV_U_INT32 *size, NV_INT64 *image_time)
{
  NV_U_CHAR       *image;
  NV_INT32        j;


  if (recnum < 1 || recnum > l_head.text.number_images) return (NULL);

  j = recnum - 1;


  if (records[j].image_size == 0) return (NULL);


  *image_time = records[j].timestamp;

  image = (NV_U_CHAR *) malloc (records[j].image_size);
  if (image == NULL)
    {
      perror ("Allocating image memory");
      exit (-1);
    }


  fseeko64 (fp, records[j].byte_offset, SEEK_SET);
  fread (image, records[j].image_size, 1, fp);

  *size = records[j].image_size;

  return (image);
}


/*  Given the .hof or .tof filename this function will write a .jpg file
    from the .img file of the same name.  The .jpg will be the image nearest
    to the timestamp entered and the filename will be in the argument
    'path'.  The file must be in the same directory as the .hof or .tof
    file.  This returns the timestamp of the picture or 0 (failed).  */

NV_INT64 dump_image (NV_CHAR *file, NV_INT64 timestamp, NV_CHAR *path)
{
  NV_U_CHAR    *image;
  NV_CHAR      img_file[512];
  NV_U_INT32   size;
  FILE         *lfp, *dfp;
  NV_INT64     ret = 0;


  strcpy (img_file, file);
  strcpy (&img_file[strlen (img_file) - 4], ".img");

  if ((lfp = open_image_file (img_file)) != NULL)
    {
      image = image_read_record (lfp, timestamp, &size, &ret);

      if (size)
        {
          if (image)
            {
              if ((dfp = fopen64 (path, "wb")) != NULL)
                {
                  fwrite (image, size, 1, dfp);

                  fclose (dfp);

                  free (image);
                }
            }

          fclose (lfp);
        }
    }

  return (ret);
}
