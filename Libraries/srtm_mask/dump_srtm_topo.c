/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/***************************************************************************\
*                                                                           *
*   Module Name:        dump_srtm_topo                                      *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 2006                                       *
*                                                                           *
*   Purpose:            Reads a one-degree cell from the compressed SRTM    *
*                       files (*.cte) and dumps an ASCII YXZ file of the    *
*                       best available resolution data.                     *
*                                                                           *
*   Arguments:          argv[1]         -   southwest latitude of cell      *
*                       argv[2]         -   southwest longitude of cell     *
*                       argv[3]         -   output file name                *
*                                                                           *
\***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <zlib.h>


#include "nvtypes.h"
#include "nvdef.h"


#include "dump_srtm_topo_version.h"
#include "read_srtm_topo.h"


NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  NV_INT16       *array;
  NV_INT32       i, j, wsize, hsize, lat, lon, percent = 0, old_percent = -1;
  NV_FLOAT64     winc, hinc, pointlat, pointlon;
  FILE           *fp;



  printf ("\n\n%s\n\n", VERSION);



  if (argc < 3)
    {
      fprintf (stderr, "Usage: %s LAT LON OUTPUT_FILE\n\n", argv[0]);
      fprintf (stderr, "\tWhere:\n\n");
      fprintf (stderr, "\tLAT = latitude degrees of southwest corner of one-degree cell (south negative)\n");
      fprintf (stderr, "\tLON = longitude degrees of southwest corner of one-degree cell (west negative)\n");
      fprintf (stderr, "\tOUTPUT_FILE = output file name\n\n");
      exit (-1);
    }


  if ((fp = fopen (argv[3], "w")) == NULL)
    {
      perror (argv[3]);
      exit (-1);
    }


  sscanf (argv[1], "%d", &lat);
  sscanf (argv[2], "%d", &lon);


  wsize = read_srtm2_topo_one_degree (lat, lon, &array);

  hsize = wsize;
  if (wsize == 1800) hsize = 3600;

  if (wsize > 2)
    {
      winc = 1.0L / wsize;
      hinc = 1.0L / hsize;

      for (i = 0 ; i < hsize ; i++)
        {
          pointlat = ((NV_FLOAT64) lat + 1.0L) - (NV_FLOAT64) (i + 1) * hinc;
          for (j = 0 ; j < wsize ; j++)
            {
              pointlon = (NV_FLOAT64) lon + (NV_FLOAT64) j * winc;

              fprintf (fp, "%0.9f,%0.9f,%0.2f\n", pointlat, pointlon, (NV_FLOAT32) -array[i * wsize + j]);
            }

          percent = (NV_INT32) (((NV_FLOAT32) i / (NV_FLOAT32) hsize) * 100.0);
          if (percent != old_percent)
            {
              fprintf (stderr, "%03d%% processed\r", percent);
              fflush (stderr);
              old_percent = percent;
            }
        }
    }
  else
    {
      /*  Force a 30 second map of zero values.  */

      hsize = wsize = 120;
      winc = 1.0L / wsize;
      hinc = 1.0L / hsize;

      for (i = 0 ; i < hsize ; i++)
        {
          pointlat = ((NV_FLOAT64) lat + 1.0L) - (NV_FLOAT64) (i + 1) * hinc;
          for (j = 0 ; j < wsize ; j++)
            {
              pointlon = (NV_FLOAT64) lon + (NV_FLOAT64) j * winc;

              fprintf (fp, "%0.9f,%0.9f,0.00\n", pointlat, pointlon);
            }

          percent = (NV_INT32) (((NV_FLOAT32) i / (NV_FLOAT32) hsize) * 100.0);
          if (percent != old_percent)
            {
              fprintf (stderr, "%03d%% processed\r", percent);
              fflush (stderr);
              old_percent = percent;
            }
        }
    }


  fprintf (stderr, "100%% processed\n");
  fflush (stderr);
  fclose (fp);


  if (wsize > 2) cleanup_srtm_topo ();


  return (0);
}
