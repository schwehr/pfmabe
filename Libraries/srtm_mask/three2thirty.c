#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "nvutility.h"

NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  FILE           *fp;
  NV_U_CHAR      *degree = NULL;
  NV_INT32       i, j, m, n, p, q, wsize, percent = 0, old_percent = -1, count;
  NV_FLOAT64     winc = 0.00083333333333, lat, lon;


  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s OUTPUT_FILE\n", argv[0]);
      exit (-1);
    }

  if ((fp = fopen (argv[1], "w")) == NULL)
    {
      perror (argv[1]);
      exit (-1);
    }


  for (i = -60 ; i < 60 ; i++)
    {
      for (j = -180 ; j < 180 ; j++)
	{
	  wsize = read_srtm_mask_one_degree (i, j, &degree, 3);

	  if (wsize == 1200)
	    {
	      for (m = 0 ; m < 1200 ; m += 10)
		{
		  for (n = 0 ; n < 1200 ; n += 10)
		    {
		      count = 0;

		      for (p = 0 ; p < 10 ; p++)
			{
			  lat = (NV_FLOAT64) (i + 1) - (NV_FLOAT64) (m + p + 1) * winc;

			  for (q = 0 ; q < 10 ; q++)
			    {
			      lon = (NV_FLOAT64) (j) + (NV_FLOAT64) ((n - 10) + q + 1) * winc;

			      if (wsize == 1 || degree[(m + p) * wsize + n + q]) count++;
			    }
			}

		      if (count && count != 100) fprintf (fp, "%.9f,%.9f,%d\n", lat, lon, count);
		    }
		}
	    }
	}

      percent = (NV_INT32) (((NV_FLOAT32) (i + 90) / 180.0) * 100.0);
      if (percent != old_percent)
	{
	  fprintf (stderr, "%%%d processed\r", percent);
	  old_percent = percent;
	}
    }


  cleanup_srtm_mask ();


  fclose (fp);

  return (0);
}
