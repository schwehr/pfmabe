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
  NV_INT32            i, num_recs;
  NV_U_INT32          prev_size = -1;
  NV_INT64            prev_time = -1;
  IMAGE_HEADER_T      head;
  IMAGE_INDEX_T       image_index;


  if (argc < 2)
    {
      fprintf (stderr, "\n\nUsage: check_image_file IMG_FILENAME\n\n");
      exit (-1);
    }

  if ((fp = open_image_file (argv[1])) == NULL)
    {
      perror (argv[1]);
      exit (-1);
    }


  fprintf (stderr, "%s\n", argv[1]);


  image_read_header (fp, &head);

  num_recs = head.text.number_images;


  for (i = 0 ; i < num_recs ; i++)
    {
      image_get_metadata (fp, i + 1, &image_index);

      if (i)
        {
          if (image_index.timestamp - prev_time == 2000000)
            fprintf (stderr, "2 second gap following record %d\n", i);

          if (image_index.image_size == prev_size)
            fprintf (stderr, "Duplicate images at records %d and %d - %d %d\n", i, i + 1, image_index.image_size, prev_size);
        }

      prev_time = image_index.timestamp;
      prev_size = image_index.image_size;
    }

  fclose (fp);

  return (0);
}
