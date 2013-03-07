#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "nvutility.h"
#include "read_srtm_topo.h"


NV_BOOL thirty2topo (NV_FLOAT64 lat, NV_FLOAT64 lon, NV_INT16 *value, NV_INT16 *percent)
{
  static NV_BOOL     first = NVTrue;
  static NV_INT32    pct_offset = 1866240000;
  static FILE        *fpv, *fpp;
  NV_INT64           i, j, pos;


  if (first)
    {
      /*  Open it twice so we don't have to fseek/flush buffers when we get value vs percent.  */

      if ((fpv = fopen64 ("thirty.dat", "rb")) == NULL)
	{
	  perror ("thirty.dat");
	  exit (-1);
	}

      if ((fpp = fopen64 ("thirty.dat", "rb")) == NULL)
	{
	  perror ("thirty.dat");
	  exit (-1);
	}

      first = NVFalse;
    }


  if (lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 360.0) return (NVFalse);


  if (lon > 180.0) lon -= 360.0;


  i = (lat + 90.0) * 120.0;
  j = (lon + 180.0) * 120.0;


  pos = (i * 43200 + j) * sizeof (NV_INT16);

  fseeko64 (fpv, pos, SEEK_SET);

  fread (value, sizeof (NV_INT16), 1, fpv);

  pos += pct_offset;

  fseeko64 (fpp, pos, SEEK_SET);

  fread (percent, sizeof (NV_INT16), 1, fpp);


  return (NVTrue);
}

  
NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  FILE           *fp;
  NV_INT16       value, percent;
  NV_INT32       i, j;
  NV_FLOAT64     olat, olon, lat, lon;


  if (argc < 4)
    {
      fprintf (stderr, "Usage: %s OUTPUT_FILE LAT LON\n", argv[0]);
      exit (-1);
    }

  if ((fp = fopen (argv[1], "w")) == NULL)
    {
      perror (argv[1]);
      exit (-1);
    }

  sscanf (argv[2], "%lf", &olat);
  sscanf (argv[3], "%lf", &olon);

  for (i = 0 ; i < 120 ; i++)
    {
      lat = olat + (NV_FLOAT64) i * 0.0083333333 + 0.0041666666666;
      for (j = 0 ; j < 120 ; j++)
	{
	  lon = olon + (NV_FLOAT64) j * 0.0083333333 + 0.0041666666666;

	  if (!thirty2topo (lat, lon, &value, &percent))
	    {
	      fprintf (stderr, "\n\nWTF!?!?!?!?!\n\n");
	      exit (-1);
	    }

	  fprintf (fp, "%.9lf,%.9lf,%d\n", lat, lon, value);
	}
    }

  fclose (fp);

  return (0);
}
