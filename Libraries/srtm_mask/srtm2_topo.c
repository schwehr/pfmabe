/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/***************************************************************************\
*                                                                           *
*   Module Name:        srtm2_topo                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 2007                                       *
*                                                                           *
*   Purpose:            Reads the SRTM2 DTED files (*.dt2) and creates      *
*                       world (or as much as is covered) compressed         *
*                       topoographic elevation (.cte) file.                 *
*                                                                           *
*   Method:             Note that we are reading both the srt2 and srt2f_1  *
*                       files.  The lower of the two vertical accuracy      *
*                       values is declared the winner.  If the winner hits  *
*                       a -32767 value we will use the loser value if it's  *
*                       not also -32767.  In this case we will store the    *
*                       loser's vertical accuracy (higher value) in the     *
*                       address map area.                                   *
*                                                                           *
*   Caveats:            This program must be run from the directory         *
*                       directly above the srt2 and srt2f_1 directories.    *
*                       There must be an srtm2 directory in the directory   *
*                       pointed to by the SRTM_DATA environment variable.   *
*                                                                           *
*   Arguments:          argv[1]         -   input NGA block number          *
*                                                                           *
\***************************************************************************/


/*

    Description of the compressed topographic elevation (.cte) file format (look Ma, no endians!)



    Header - 16384 bytes, ASCII

    [HEADER SIZE] = 16384
    [CREATION DATE] = 
    [VERSION] = 
    [ZLIB VERSION] =
    .
    .
    .
    Other stuff...
    .
    .
    .
    [END OF HEADER]


    One-degree map - 64800 * 44 bits
        36 bits - double precision integer address of block, stored as characters.
        8 bits  - vertical accuracy of block in meters

        Records start at 90S,180W and proceed west to east then south to north (that is, the second record
        is for 90S,179W and the 361st record is for 89S,180W).
        Address contains 0 if all water, 2 if undefined, or address.


    Data -

        3 bits  - 0 for 1 by 1 second blocks; 1 for 1 by 2 second (lat by lon) blocks
        30 bits - size of the zlib level 9 compressed block (SB)
        31 bits - size of the uncompressed block
        SB bytes - data

        Inside the compressed block the data has already been delta coded and bit packed.  The format of the
        data stored in the compressed block is as follows:

            16 bits - signed short, starting value, stored as characters
            16 bits - signed short, bias value, stored as characters
            4 bits  - number of bits used to store delta coding offsets (NB)
            NB bits - first offset
            NB bits - offset from first
            NB bits - offset from second
            .
            .
            .
            NB bits - last offset (may be the 12,960,000th, 1,440,000th, or 14,400th offset depending on 
            resolution)

            Undefined values (-32768) will be stored as (int) pow (2.0L, NB) - 1 and will not be used in the
            delta chain.
            
            The deltas are computed by subtracting the previous valid value from the current value.  The data
            is traversed west to east for one row, then east to west for the next row, then west to east, etc.
            I call this the snake dance (named after the lines at Disney World ;-)  After the delta is computed
            the bias is added to it.
            
            
        The compression is compliments of the ZLIB compression library which can be found at 
        http://www.zlib.net/.  Many thanks to Jean-loup Gailly, Mark Adler, and all others associated with
        that effort.

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
#include "dted.h"
#include "srtm2_topo_version.h"


#define HEADER_SIZE             16384
#define MAP_BYTES               (64800 * 44) / 8


NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  NV_INT32       i, j, k, m, shift_lat, shift_lon, n, hit_land, hit_water,
                 pos, prev_value, first, ndx, total_bits, lat_count, lon_count, vacc[2], finalvacc, status, block;
  uLong          total_bytes;
  uLongf         out_bytes;
  FILE           *fp[2], *block_fp, *ofp;
  NV_INT16       **row, *delta, bias, start_val = 0, last_val = 0, diff, min_diff, max_diff, num_bits, null_val, value, 
                 winner = 0, loser = 0;
  NV_CHAR        ofile[512], lathem, lonhem, header_block[HEADER_SIZE], dir[512], block_file[512], srt_file[2][512];
  NV_INT64       lpos;
  NV_U_INT32     mpos;
  NV_U_BYTE      water[4], head[8], *in_buf, *out_buf, *address_map, byte, block_map[64800];
  time_t         t;
  struct tm      *cur_tm;
  NV_BOOL        both = NVFalse;

  UHL            uhl[2];
  DSI            dsi[2];
  ACC            acc[2];
  DTED_DATA      dted_data[2];



  printf ("\n\n%s\n\n", VERSION);


  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s SRTM2_BLOCK_NUMBER\n\n", argv[0]);
      exit (-1);
    }


  sscanf (argv[1], "%d", &block);


  /*  Open and read the block map file.  */

  if (getenv ("SRTM_DATA") == NULL)
    {
      fprintf (stderr, "\n\nYou must set the SRTM_DATA environment variable to point to the location of srtm2 directory\n\n");
      exit (-1);
    }

  strcpy (dir, getenv ("SRTM_DATA"));


  /*  Open and read the block map file.  */

  sprintf (block_file, "%s%1csrtm2%1csrtm2_block_map.dat", dir, SEPARATOR, SEPARATOR);

  if ((block_fp = fopen (block_file, "rb+")) == NULL)
    {
      if ((block_fp = fopen (block_file, "wb+")) == NULL)
        {
          perror (block_file);
          exit (-1);
        }


      /*  This must be the first time so initialize it.  */

      byte = 0;
      for (i = 0 ; i < 180 ; i++)
        {
          for (j = 0 ; j < 360 ; j++)
            {
              fwrite (&byte, 1, 1, block_fp);
            }
        }
    }

  fseek (block_fp, 0LL, SEEK_SET);
  fread (block_map, sizeof (block_map), 1, block_fp);


  /*  Hmmm, had problems with statically allocated arrays of this size.  */

  delta = (NV_INT16 *) calloc (3600 * 3600, sizeof (NV_INT16));
  if (delta == NULL)
    {
      perror ("Allocating delta");
      exit (-1);
    }


  address_map = (NV_U_BYTE *) calloc (MAP_BYTES, sizeof (NV_U_BYTE));
  if (address_map == NULL)
    {
      perror ("Allocating address_map");
      exit (-1);
    }


  row = (NV_INT16 **) calloc (3601, sizeof (NV_INT16 *));
  if (row == NULL)
    {
      perror ("Allocating row");
      exit (-1);
    }

  for (i = 0 ; i < 3601 ; i++)
    {
      row[i] = (NV_INT16 *) calloc (3601, sizeof (NV_INT16));
      if (row[i] == NULL)
        {
          perror ("Allocating row[i]");
          exit (-1);
        }
    }


  /*  Set the water flag for the map area.  */

  srtm_bit_pack (water, 0, 32, 0);


  /*  Open the output file.  */

  sprintf (ofile, "%s%1csrtm2%1csrtm2_block_%03d.cte", dir, SEPARATOR, SEPARATOR, block);

  if ((ofp = fopen64 (ofile, "wb")) == NULL)
    {
      perror (ofile);
      exit (-1);
    }


  /*  Write the (minimalist) ASCII header.  */


  memset (header_block, 0, HEADER_SIZE);

  t = time (&t);
  cur_tm = gmtime (&t);

  sprintf (header_block, "[HEADER SIZE] = %d\n", HEADER_SIZE);
  sprintf (&header_block[strlen (header_block)], "[VERSION] = %s\n", VERSION);
  sprintf (&header_block[strlen (header_block)], "[ZLIB VERSION] = %s\n", zlibVersion ());
  sprintf (&header_block[strlen (header_block)], "[CREATION DATE] = %s", asctime (cur_tm));
  sprintf (&header_block[strlen (header_block)], "[TITLE] = SRTM Digital Terrain Elevation Data Level 2\n");
  sprintf (&header_block[strlen (header_block)], "[ORGANISATION] = National Geospatial-Intelligence Agency\n");
  sprintf (&header_block[strlen (header_block)], "[SRTM2 BLOCK NUMBER] = %03d\n", block);
  sprintf (&header_block[strlen (header_block)], "[REFERENCE SYSTEM] = WGS84\n");
  sprintf (&header_block[strlen (header_block)], "[CLASSIFICATION] = UNCLASSIFIED RESTRICTED\n");
  sprintf (&header_block[strlen (header_block)], "[HANDLING DESCRIPTION] = DISTRIBUTION LIMITED TO DOD AND DOD CONTRACTORS ONLY\n");
  sprintf (&header_block[strlen (header_block)], "[END OF HEADER]\n");


  fwrite (header_block, HEADER_SIZE, 1, ofp);


  /*  Set the default for all map addresses to 2 (undefined).  Also, write the blank map area out to the
      file to save the space.  */

  for (i = 0 ; i < 180 ; i++)
    {
      for (j = 0 ; j < 360 ; j++)
        {
          mpos = (i * 360 + j) * 44;
          srtm_double_bit_pack (address_map, mpos, 36, 2);

          mpos += 36;
          srtm_bit_pack (address_map, mpos, 8, 0);
        }
    }
  fseeko64 (ofp, (NV_INT64) HEADER_SIZE, SEEK_SET);
  fwrite (address_map, MAP_BYTES, 1, ofp);


  /*  Loop through the entire range of longitudes.  */

  for (j = -180 ; j < 180 ; j++)
    {
      lonhem = 'e';
      if (j < 0) lonhem = 'w';


      /*  Shift into the 0 to 360 world.  */

      shift_lon = j + 180;


      /*  Loop through the entire range of available latitudes.  */

      for (i = -56 ; i < 60 ; i++)
        {
          lathem = 'n';
          if (i < 0) lathem = 's';


          /*  Shift into the 0 to 180 world.  */

          shift_lat = i + 90;


          /*  Build the srt2 file name.  */

          sprintf (srt_file[0], "srt2/srt%03d/dted/%1c%03d/%1c%02d.dt2", block, lonhem, abs (j), lathem, abs (i));


          /*  Build the srt2f file name.  */

          sprintf (srt_file[1], "srt2f_1/srt2f%03d/dted/%1c%03d/%1c%02d.dt2", block, lonhem, abs (j), lathem, abs (i));


          /*  If we can open either of the files, read it.  */

          lat_count = 0;
          lon_count = 0;
          vacc[0] = vacc[1] = 256;
          fp[0] = fp[1] = NULL;

          fp[0] = fopen (srt_file[0], "rb");
          fp[1] = fopen (srt_file[1], "rb");

          both = NVFalse;
          if (fp[0] != NULL && fp[1] != NULL) both = NVTrue;


          if (fp[0] != NULL || fp[1] != NULL)
            {
              prev_value = -1;
              pos = 0;

              for (m = 0 ; m < 2 ; m++)
                {
                  if (fp[m] != NULL)
                    {
                      read_uhl (fp[m], &uhl[m]);
                      read_dsi (fp[m], &dsi[m]);
                      read_acc (fp[m], &acc[m]);

                      lat_count = uhl[m].num_lat_points;
                      lon_count = uhl[m].num_lon_lines;

                      sscanf (uhl[m].vertical_accuracy, "%d", &vacc[m]);


                      if (vacc[m] > 255)
                        {
                          fprintf (stderr, "Vertical accuracy is too large to store, setting to 255\n");
                          vacc[m] = 255;
                        }
                    }
                }


              winner = 0;
              loser = 1;
              if (vacc[1] < vacc[0])
                {
                  loser = 0;
                  winner = 1;
                }

              finalvacc = winner;


              /*  Flipping this stuff around so it gets stored like the original SRTM data even though it's
                  in DTED format coming in.  */

              for (m = 0 ; m < lon_count - 1 ; m++)
                {
                  if (fp[0] != NULL && (status = read_dted_data (fp[0], lat_count, m, &dted_data[0])))
                    {
                      fprintf (stderr,"READ ERROR %d\n", status);
                      fflush (stderr);
                    }

                  if (fp[1] != NULL && (status = read_dted_data (fp[1], lat_count, m, &dted_data[1])))
                    {
                      fprintf (stderr,"READ ERROR %d\n", status);
                      fflush (stderr);
                    }

                  for (k = 0 ; k < lat_count ; k++)
                    {
                      value = dted_data[winner].elev[(lat_count - 1) - k];

                      if (both && value <= -32767 && dted_data[loser].elev[(lat_count - 1) - k] > -32767)
                        {
                          value = dted_data[loser].elev[(lat_count - 1) - k];
                          finalvacc = loser;
                        }

                      row[k][m] = value;
                    }
                }


              /*  Set the block map value to the block number - 200 (so it'll fit in a byte).  */

              block_map[shift_lat * 360 + shift_lon] = block - 200;


              first = 1;
              ndx = 0;
              min_diff = 32767;
              max_diff = -32768;
              hit_land = 0;
              hit_water = 0;


              /*  Loop through the cell.  */

              for (k = 0 ; k < lat_count - 1 ; k++)
                {

                  /*  Snake dance test.  */

                  if (!(k % 2))
                    {
                      /*  West to east.  */

                      for (m = 0 ; m < lon_count - 1 ; m++)
                        {
                          if (row[k][m] != -32767)
                            {
                              /*  Set the "hit" flags.  */

                              if (row[k][m])
                                {
                                  hit_land = 1;
                                }
                              else
                                {
                                  hit_water = 1;
                                }


                              /*  First time through set the last_val to start_val.  */

                              if (first)
                                {
                                  start_val = row[k][m];
                                  last_val = start_val;
                                  first = 0;
                                }
                              diff = row[k][m] - last_val;
                              if (diff > max_diff) max_diff = diff;
                              if (diff < min_diff) min_diff = diff;
                              last_val = row[k][m];

                              if (abs (diff) > 32767)
                                {
                                  fprintf (stderr, "We've exceeded 16 bits in size for a delta - something is terribly wrong!\n");
                                  exit (-1);
                                }

                              delta[ndx++] = diff;
                            }
                          else
                            {
                              /*  -32767 means land but no defined elevation.  */

                              hit_land = 1;
                              delta[ndx++] = -32768;
                            }
                        }
                    }
                  else
                    {

                      /*  East to west.  */

                      for (m = lon_count - 2 ; m >= 0 ; m--)
                        {
                          if (row[k][m] != -32767)
                            {
                              /*  Set the "hit" flags.  */

                              if (row[k][m])
                                {
                                  hit_land = 1;
                                }
                              else
                                {
                                  hit_water = 1;
                                }


                              /*  First time through set the last_val to start_val.  */

                              if (first)
                                {
                                  start_val = row[k][m];
                                  last_val = start_val;
                                  first = 0;
                                }
                              diff = row[k][m] - last_val;
                              if (diff > max_diff) max_diff = diff;
                              if (diff < min_diff) min_diff = diff;
                              last_val = row[k][m];

                              if (abs (diff) > 32767)
                                {
                                  fprintf (stderr, "We've exceeded 16 bits in size for a delta - something is terribly wrong!\n");
                                  exit (-1);
                                }

                              delta[ndx++] = diff;
                            }
                          else
                            {
                              /*  From what I've seen, -32767 means land but no defined elevation.  */

                              hit_land = 1;
                              delta[ndx++] = -32768;
                            }
                        }
                    }
                }


              /*  All water cell.  */

              if (!hit_land && hit_water)
                {
                  mpos = (shift_lat * 360 + shift_lon) * 44;
                  srtm_double_bit_pack (address_map, mpos, 44, 0);
                }


              /*  Land and water or all land cell.  */

              else
                {
                  /*  The bias is the negative of the minimum difference.  We bias so we don't have to play with
                      sign extension in the bit unpacking.  */

                  bias = -min_diff;


                  /*  Add two to the max difference to allow room for the null value.  */

                  max_diff += (bias + 2);


                  /*  Compute the number of bits needed to store a delta.  */

                  num_bits = NINT (log10 ((NV_FLOAT64) max_diff) / LOG2 + 0.5L);
                  null_val = NINT (pow (2.0L, (NV_FLOAT64) num_bits)) - 1;


                  /*  Compute the total bytes needed to store the block.  */

                  total_bits = 16 + 16 + 4 + lon_count * lat_count * num_bits;
                  total_bytes = total_bits / 8;
                  if (total_bits % 8) total_bytes++;


                  /*  Allocate the uncompressed memory block.  */

                  in_buf = (NV_U_BYTE *) calloc (total_bytes, 1);
                  if (in_buf == NULL)
                    {
                      perror ("Allocating in_buf");
                      exit (-1);
                    }


                  /*  Allocate the compressed memory block.  */

                  out_bytes = total_bytes + NINT ((NV_FLOAT32) total_bytes * 0.10) + 12;
                  out_buf = (NV_U_BYTE *) calloc (out_bytes, 1);
                  if (out_buf == NULL)
                    {
                      perror ("Allocating out_buf");
                      exit (-1);
                    }


                  /*  Pack the internal header.  */

                  pos = 0;
                  srtm_bit_pack (in_buf, pos, 16, start_val); pos += 16;
                  srtm_bit_pack (in_buf, pos, 16, bias); pos += 16;
                  srtm_bit_pack (in_buf, pos, 4, num_bits); pos += 4;


                  /*  Pack the deltas.  */

                  for (k = 0 ; k < lon_count * lat_count ; k++)
                    {
                      if (delta[k] == -32768)
                        {
                          delta[k] = null_val;
                        }
                      else
                        {
                          delta[k] += bias;
                        }
                      srtm_bit_pack (in_buf, pos, num_bits, delta[k]); pos += num_bits;
                    }


                  /*  Compress at maximum level.  */

                  n = compress2 (out_buf, &out_bytes, in_buf, total_bytes, 9);
                  if (n)
                    {
                      fprintf (stderr, "Error %d compressing record\n", n);
                      exit (-1);
                    }
                  free (in_buf);


                  /*  Pack the header.  */

                  pos = 0;
                  if (lon_count == 3601)
                    {
                      srtm_bit_pack (head, pos, 3, 0); pos += 3;
                    }
                  else
                    {
                      srtm_bit_pack (head, pos, 3, 1); pos += 3;
                    }
                  srtm_bit_pack (head, pos, 30, out_bytes); pos += 30;
                  srtm_bit_pack (head, pos, 31, total_bytes);


                  /*  Get the address where we're going to write the compressed block.  */

                  fseeko64 (ofp, 0LL, SEEK_END);
                  lpos = ftello64 (ofp);


                  /*  Write the header to the file.  */

                  fwrite (head, 8, 1, ofp);


                  /*  Write the buffer to the file.  */

                  fwrite (out_buf, out_bytes, 1, ofp);


                  free (out_buf);


                  /*  Save the address of the block (and vertical accuracy) to the map.  */

                  mpos = (shift_lat * 360 + shift_lon) * 44;
                  srtm_double_bit_pack (address_map, mpos, 36, lpos);

                  mpos += 36;
                  srtm_bit_pack (address_map, mpos, 8, vacc[finalvacc]);
                }

              if (fp[0] != NULL) fclose (fp[0]);
              if (fp[1] != NULL) fclose (fp[1]);


              fprintf (stderr, "block: %d   lon: %03d   Lat: %03d       Vacc[%01d]: %d       \r", block, j, i, finalvacc, vacc[finalvacc]);
              if (finalvacc == loser) fprintf (stderr, "Used loser vacc for lat %d, lon %d                                               \n", i, j);
              fflush (stderr);
            }
        }
    }


  /*  Write the map.  */

  fseeko64 (ofp, (NV_INT64) HEADER_SIZE, SEEK_SET);
  fwrite (address_map, MAP_BYTES, 1, ofp);

  fclose (ofp);


  /*  Write the block map.  */

  fseek (block_fp, 0, SEEK_SET);
  fwrite (block_map, sizeof (block_map), 1, block_fp);

  fclose (block_fp);


  fprintf (stderr, "\nBlock %d complete                               \n\n", block);
  fflush (stderr);


  return (0);
}
