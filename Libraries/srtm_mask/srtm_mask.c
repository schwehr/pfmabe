
/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/***************************************************************************\
*                                                                           *
*   Module Name:        srtm_mask                                           *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 2007                                       *
*                                                                           *
*   Purpose:            Reads the compressed SRTM topo files (.cte) and     *
*                       creates world (or as much as is covered) land mask. *
*                                                                           *
*   Arguments:          argv[1]         -   output file name                *
*                                                                           *
*   Caveats:            You must have all of the compressed SRTM topo files *
*                       in order to run this properly.  The SRTM2 limdis    *
*                       file is not required but will give you an almost    *
*                       worldwide 1 second land mask.  To the best of my    *
*                       knowledge the land mask will not be limited         *
*                       distribution even if you use the SRTM2 data.  Set   *
*                       SRTM_DATA to point to the location of the .cte      *
*                       files.                                              *
*                                                                           *
\***************************************************************************/


/*

    Description of the compressed land mask (.clm) file format (look Ma, no endians!)



    Header - 16384 bytes, ASCII

    [HEADER SIZE] = 16384
    [CREATION DATE] = 
    [VERSION] = 
    [ZLIB VERSION] =
    [END OF HEADER]


    One-degree map - 64800 * 4 bytes, binary, stored as characters.
    
        Records start at 90S,180W and proceed west to east then south to north (that is, the second record
        is for 90S,179W and the 361st record is for 89S,180W).
        Record contains 0 if all water, 1 if all land, 2 if undefined, or address if both land and water.


    Data - 1's and 0's (woo hoo)

        3 bits  - resolution, 0 = one second mask, 1 = 3 second mask, 2 = 30 second mask, 3 = 1 (lon) by
                  2 (lat) second mask, others TBD
        29 bits - size of the zlib level 9 compressed block (SB)
        SB bytes - data

        The data is stored as a series of single bits for water (0) and land (1).  Each bit represents a 
        one second, three second, or thirty second cell in the block.  The block is a one-degree square.
        It will be 3600 X 3600, 1200 X 1200, 120 X 120, or 60 X 60 depending on the resolution.  It is
        ordered in the same fashion as the srtm3 data, that is, west to east starting in the northwest
        corner and moving southward.  The compression is compliments of the ZLIB compression library which
        can be found at http://www.zlib.net/.  Many thanks to Jean-loup Gailly, Mark Adler, and all others
        associated with that effort.


    For some unknown reason the majority of the N60 files in srtm3 are totally hosed up so I'm going to mark
    them as undefined (2).  Also, N59E170, N59W167, and N59W166 seem to be missing so these are forced to
    undefined as well.  GTOPO30 (SRTM30) data will be used for these cells.

*/



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


#include "srtm_bit_pack.h"
#include "read_srtm_topo.h"
#include "srtm_mask_version.h"


#define HEADER_SIZE             16384


NV_INT32 main (NV_INT32 argc, char *argv[])
{
  NV_INT32       i, j, k, m, shift_lat, shift_lon, n, hit_land, hit_water, wsize = 0, hsize, pos, min_res = 0;
  uLong          in_size;
  uLongf         out_size;
  NV_INT16       *array = NULL;
  FILE           *ofp;
  NV_CHAR        ofile[512];
  NV_U_BYTE      zero = 0, mapbuf[4], water[4], land[4], *in_buf, *out_buf;
  time_t         t;
  struct tm      *cur_tm;


  printf ("\n\n%s\n\n", VERSION);


  if (argc < 3)
    {
      fprintf (stderr, "Usage: %s MIN_RESOLUTION OUTPUT_FILENAME\n\n", argv[0]);
      fprintf (stderr, "Where MIN_RESOLUTION is 1, 3, or 30\n");
      exit (-1);
    }


  /*  Set the water and land flags for the map area.  */

  srtm_bit_pack (water, 0, 32, 0);
  srtm_bit_pack (land, 0, 32, 1);


  /*  Open the output file.  */

  strcpy (ofile, argv[2]);
  if (strcmp (&ofile[strlen (ofile) - 4], ".clm")) strcat (ofile, ".clm");

  if ((ofp = fopen (ofile, "wb")) == NULL)
    {
      perror (ofile);
      exit (-1);
    }


  sscanf (argv[1], "%d", &min_res);

  if (min_res != 1 && min_res != 3 && min_res != 30)
    {
      fprintf (stderr, "Invalid resolution %d, use 1, 3, or 30\n", min_res);
      exit (-1);
    }


  /*  Write the (minimalist) ASCII header.  */

  t = time (&t);
  cur_tm = gmtime (&t);

  fprintf (ofp, "[HEADER SIZE] = %d\n", HEADER_SIZE);
  fprintf (ofp, "[VERSION] = %s\n", VERSION);
  fprintf (ofp, "[ZLIB VERSION] = %s\n", zlibVersion ());
  fprintf (ofp, "[CREATION DATE] = %s", asctime (cur_tm));
  fprintf (ofp, "[END OF HEADER]\n");


  /*  Zero out the remainder of the header.  */

  j = ftell (ofp);
  for (i = j ; i < HEADER_SIZE ; i++) fwrite (&zero, 1, 1, ofp);


  /*  Set the default for all map addresses to 2 (undefined).  */

  for (i = -90 ; i < 90 ; i++)
    {
      for (j = -180 ; j < 180 ; j++)
        {
          srtm_bit_pack (mapbuf, 0, 32, 2);
          fwrite (mapbuf, 4, 1, ofp);
        }
    }


  /*  Loop through the entire world, lat then lon.  */

  for (i = -90 ; i < 90 ; i++)
    {
      shift_lat = i + 90;

      for (j = -180 ; j < 180 ; j++)
        {
          hit_land = 0;
          hit_water = 0;
          pos = 0;


          shift_lon = j + 180;


          /*  Use srtm2 for the one second data since I edited the screwups out of the dted files for that set.
              Only use srtm30 outside of 50S and 50N as there are some really screwy things in the 30 second data.  */

          switch (min_res)
            {
            case 1:
              wsize = read_srtm2_topo_one_degree (i, j, &array);
              if ((wsize == -1 || wsize == 2) && (i < -55 || i > 55)) wsize = read_srtm30_topo_one_degree (i, j, &array);
              break;

            case 3:
              wsize = read_srtm3_topo_one_degree (i, j, &array);
              if ((wsize == -1 || wsize == 2) && (i < -55 || i > 55)) wsize = read_srtm30_topo_one_degree (i, j, &array);
              break;

            case 30:
              wsize = read_srtm30_topo_one_degree (i, j, &array);
              break;
            }


          if (wsize < 0)
            {
              fprintf (stderr, "Problem opening/reading SRTM files!\n");
              exit (-1);
            }


          fprintf (stderr,"Latitude:%d  Longitude:%d   Size:%d\n", i, j, wsize);


          /*  All water (2 means we're inside 50N/50S and there wasn't a 1 or 3 second file).  */

          if (wsize == 0 || wsize == 2)
            {
              fseek (ofp, HEADER_SIZE + (shift_lat * 360 + shift_lon) * 4, SEEK_SET);
              fwrite (water, 4, 1, ofp);
            }
          else
            {
              hsize = wsize;
              if (wsize == 1800) hsize = 3600;

              in_size = (hsize * wsize) / 8 + 1;
              in_buf = (NV_U_BYTE *) calloc (in_size, sizeof (NV_U_BYTE));

              if (in_buf == NULL)
                {
                  perror ("Allocating in_buf");
                  exit (-1);
                }

              out_size = in_size + in_size * 0.10 + 100;
              out_buf = (NV_U_BYTE *) calloc (out_size, sizeof (NV_U_BYTE));

              if (out_buf == NULL)
                {
                  perror ("Allocating out_buf");
                  exit (-1);
                }


              /*  -32767 values are usually land of undefined elevation but not always.  
                  Some is just garbage (over water).  */

              for (m = 0 ; m < hsize ; m++)
                {
                  for (k = 0 ; k < wsize ; k++)
                    {
                      if (array[m * wsize + k])
                        {
                          hit_land = 1;
                          srtm_bit_pack (in_buf, pos, 1, 1);
                        }
                      else
                        {
                          hit_water = 1;
                          srtm_bit_pack (in_buf, pos, 1, 0);
                        }
                      pos++;
                    }
                }


              /*  All land cell.  */

              if (hit_land && !hit_water)
                {
                  fseek (ofp, HEADER_SIZE + (shift_lat * 360 + shift_lon) * 4, SEEK_SET);
                  fwrite (land, 4, 1, ofp);
                }
              else
                {
                  if ((!hit_water && !hit_land))
                    {
                      fprintf (stderr, "Hit land: %d   Hit water; %d    WTF, over!\n", hit_land, hit_water);
                      exit (-1);
                    }


                  /*  Save the address of where we're going to write the compressed block.  */

                  fseek (ofp, 0, SEEK_END);
                  pos = ftell (ofp);


                  /*  Compress at maximum level.  */

                  n = compress2 (out_buf, &out_size, in_buf, in_size, 9);


                  /*  Write the type of data and size of the buffer to the file.  */

                  switch (wsize)
                    {
                    case 120:
                      srtm_bit_pack (mapbuf, 0, 3, 2);
                      break;

                    case 1200:
                      srtm_bit_pack (mapbuf, 0, 3, 1);
                      break;

                    case 1800:
                      srtm_bit_pack (mapbuf, 0, 3, 3);
                      break;

                    case 3600:
                      srtm_bit_pack (mapbuf, 0, 3, 0);
                      break;

                    default:
                      fprintf (stderr, "WTF!  Strange resolution : %d\n", wsize);
                      exit (-1);
                    }


                  srtm_bit_pack (mapbuf, 3, 29, out_size);
                  fwrite (mapbuf, 4, 1, ofp);


                  /*  Write the buffer to the file.  */

                  fwrite (out_buf, out_size, 1, ofp);


                  /*  Write the address of the block to the map.  */

                  fseek (ofp, HEADER_SIZE + (shift_lat * 360 + shift_lon) * 4, SEEK_SET);
                  srtm_bit_pack (mapbuf, 0, 32, pos);
                  fwrite (mapbuf, 4, 1, ofp);
                }

              free (in_buf);
              free (out_buf);
            }
        }
    }


  fclose (ofp);


  fprintf (stderr, "100%% processed      \n\n");
  fflush (stderr);


  return (0);
}
