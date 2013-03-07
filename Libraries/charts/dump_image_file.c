#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "nvtypes.h"
#include "nvdef.h"
#include "FileImage.h"

/*  dump_image_file  */


NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  FILE                *fp, *jpg_fp;
  NV_CHAR             fn[512], jpg_name[512];
  NV_U_BYTE           *image;
  NV_INT32            i, num_recs;
  NV_U_INT32          size;
  NV_INT64            image_time;
  IMAGE_HEADER_T      head;
  IMAGE_INDEX_T       image_index;


  if (argc < 2)
    {
      fprintf (stderr, "\n\nUsage: dump_image_file IMG_FILENAME\n\n");
      exit (-1);
    }

  if ((fp = open_image_file (argv[1])) == NULL)
    {
      perror (argv[1]);
      exit (-1);
    }


  image_read_header (fp, &head);

  num_recs = head.text.number_images;


  for (i = 0 ; i < num_recs ; i++)
    {
      image_get_metadata (fp, i + 1, &image_index);

      sprintf (jpg_name, "%04d_%d_%d_%lld.jpg", image_index.image_number, image_index.byte_offset, 
               image_index.image_size, image_index.timestamp);

      if (image_index.image_size)
        {
          image = image_read_record_recnum (fp, i + 1, &size, &image_time);


          if ((jpg_fp = fopen (jpg_name, "wb")) == NULL)
            {
              perror (jpg_name);
              exit (-1);
            }

          fwrite (image, size, 1, jpg_fp);

          fclose (jpg_fp);

          free (image);
        }
    }

  fclose (fp);

  return (0);
}
