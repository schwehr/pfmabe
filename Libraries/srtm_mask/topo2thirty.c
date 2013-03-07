#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "nvutility.h"
#include "read_srtm_topo.h"


NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  FILE           *fp;
  NV_INT16       *array, *val[43200], *pct[43200];
  NV_INT32       i, j, m, n, p, q, wsize, count, land_count, iinc, mindex, nindex;
  NV_INT64       valpos = 0, pctpos = 1866240000;
  NV_FLOAT64     winc, sum, lat = 0.0, lon = 0.0;


  /*  Stupid compiler won't let me declare arrays of [43200] by [120] so I have to malloc them.  */

  for (i = 0 ; i < 43200 ; i++) 
    {
      val[i] = (NV_INT16 *) malloc (120 * sizeof (NV_INT16));

      if (val[i] == NULL)
	{
	  perror ("Allocating val array");
	  exit (-1);
	}


      pct[i] = (NV_INT16 *) malloc (120 * sizeof (NV_INT16));

      if (pct[i] == NULL)
	{
	  perror ("Allocating pct array");
	  exit (-1);
	}
    }


  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s OUTPUT_FILE\n", argv[0]);
      exit (-1);
    }

  if ((fp = fopen64 (argv[1], "w")) == NULL)
    {
      perror (argv[1]);
      exit (-1);
    }


  set_exclude_srtm2_data (NVTrue);


  for (i = -90 ; i < 90 ; i++)
    {
      for (j = -180 ; j < 180 ; j++)
	{
	  wsize = read_srtm_topo_one_degree (i, j, &array);

	  fprintf (stdout, "%02d %03d - %03d    \r", i, j, wsize);
	  fflush (stdout);

	  if ((wsize == 120 && (i < -55 || i > 55)) || wsize == 1200 || wsize == 3600)
	    {
	      winc = 1.0L / wsize;
	      iinc = wsize / 120;

	      for (m = 0, mindex = 0 ; m < wsize ; m += iinc, mindex++)
		{
		  for (n = 0, nindex = 0 ; n < wsize ; n += iinc, nindex++)
		    {
		      count = 0;
		      land_count = 0;
		      sum = 0.0;

		      for (p = 0 ; p < iinc ; p++)
			{
			  lat = (NV_FLOAT64) (i + 1) - (NV_FLOAT64) (m + p + 1) * winc;

			  for (q = 0 ; q < iinc ; q++)
			    {
			      lon = (NV_FLOAT64) (j) + (NV_FLOAT64) ((n - iinc) + q + 1) * winc;

			      if (array[(m + p) * wsize + n + q]) land_count++;

			      if (array[(m + p) * wsize + n + q] != -32768) sum += array[(m + p) * wsize + n + q];
			      count++;
			    }
			}

		      val[(j + 180) * 120 + nindex][mindex] = NINT (sum / (NV_FLOAT64) count);
		      pct[(j + 180) * 120 + nindex][mindex] = NINT (((NV_FLOAT64) land_count / (NV_FLOAT64) count) * 100.0);
		    }
		}
	    }
	  else
	    {
	      if (wsize != 2 && wsize != 0 && wsize != 120)
		{
		  fprintf (stderr, "\n\n%d - WTF?!?!?!?!?!?\n\n", wsize);
		  exit (-1);
		}


	      for (m = 0 ; m < 120 ; m++)
		{
		  for (n = 0 ; n < 120 ; n++)
		    {
		      val[(j + 180) * 120 + n][m] = 0;
		      pct[(j + 180) * 120 + n][m] = 0;
		    }
		}
	    }
	}


      /*  Write out the value and percentage arrays as single points instead of one-degree squares.  */

      fseeko64 (fp, valpos, SEEK_SET);

      for (m = 119 ; m >= 0 ; m--)
	{
	  for (n = 0 ; n < 43200 ; n++)
	    {
	      fwrite (&val[n][m], sizeof (NV_INT16), 1, fp);
	    }
	}
      valpos = ftello64 (fp);


      fseeko64 (fp, pctpos, SEEK_SET);

      for (m = 119 ; m >= 0 ; m--)
	{
	  for (n = 0 ; n < 43200 ; n++)
	    {
	      fwrite (&pct[n][m], sizeof (NV_INT16), 1, fp);
	    }
	}
      pctpos = ftello64 (fp);
    }


  cleanup_srtm_topo ();


  fclose (fp);

  return (0);
}
