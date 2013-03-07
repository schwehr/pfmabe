/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/***************************************************************************\
*                                                                           *
*   Module Name:        srtm1_topo                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 2006                                        *
*                                                                           *
*   Purpose:            Reads the uncompressed SRTM1 files (*.hgt) and      *
*                       creates world (or as much as is covered)            *
*                       compressed topoographic elevation (.cte) file.      *
*                                                                           *
*   Arguments:          argv[1]         -   Region number                   *
*                                                                           *
*   Caveats:            This program must be run from the directory         *
*                       directly above the Region_0N directories.           *
*                       There must be an srtm1 directory in the directory   *
*                       pointed to by the SRTM_DATA environment variable.   *
*                                                                           *
\***************************************************************************/


/*

    Description of the compressed topographic elevation (.cte) file format (look Ma, no endians!)



    Header - 16384 bytes, ASCII

    [HEADER SIZE] = 16384
    [CREATION DATE] = 
    [VERSION] = 
    [ZLIB VERSION] =
    [END OF HEADER]


    One-degree map - 64800 * 36 bits, double precision integer, stored as characters.
    
        Records start at 90S,180W and proceed west to east then south to north (that is, the second record
        is for 90S,179W and the 361st record is for 89S,180W).
        Record contains 0 if all water, 2 if undefined, or address.


    Data -

        3 bits  - Not used (originally planned for resolution in mixed files)
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
#include "srtm1_topo_version.h"


#define HEADER_SIZE             16384
#define MAP_BYTES               (64800 * 36) / 8


/*  A function to determine if your system is big or litle endian.  Returns a 0 if it's little endian or a
    3 if it's big endian.  */

static NV_INT32 big_endian ()
{
  union
  {
    NV_INT32        word;
    NV_U_BYTE   byte[4];
  } a;

  a.word = 0x00010203;
  return ((NV_INT32) a.byte[3]);
}


NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  NV_INT32       i, j, k, m, shift_lat, shift_lon, n, hit_land, hit_water,
                 pos, swap = 1, prev_value, first, ndx, total_bits, block;
  uLong          total_bytes;
  uLongf         out_bytes;
  FILE           *fp, *block_fp, *ofp;
  NV_INT16       **row, *delta, bias, start_val = 0, last_val = 0, diff, min_diff, max_diff, num_bits, null_val;
  NV_CHAR        string[512], ofile[512], lathem, lonhem, header_block[HEADER_SIZE], dir[512], block_file[512];
  NV_INT64       lpos;
  NV_U_BYTE      water[4], head[8], *in_buf, *out_buf, map[MAP_BYTES], byte, block_map[64800];
  time_t         t;
  struct tm      *cur_tm;


  printf ("\n\n%s\n\n", VERSION);


  sscanf (argv[1], "%d", &block);

  if (argc < 2 || block < 1 || block > 7)
    {
      fprintf (stderr, "Usage: %s REGION_NUMBER\n", argv[0]);
      fprintf (stderr, "Where REGION_NUMBER is a number between 1 and 7\n\n");
      exit (-1);
    }


  /*  Open and read the block map file.  */

  if (getenv ("SRTM_DATA") == NULL)
    {
      fprintf (stderr, "\n\nYou must set the SRTM_DATA environment variable to point to the location of srtm1 directory\n\n");
      exit (-1);
    }

  strcpy (dir, getenv ("SRTM_DATA"));


  /*  Open and read the block map file.  */

  sprintf (block_file, "%s%1csrtm1%1csrtm1_block_map.dat", dir, SEPARATOR, SEPARATOR);

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

  fseek (block_fp, 0, SEEK_SET);
  fread (block_map, sizeof (block_map), 1, block_fp);


  /*  Hmmm, had problems with statically allocated arrays of this size.  */

  delta = (NV_INT16 *) calloc (3600 * 3600, sizeof (NV_INT16));
  if (delta == NULL)
    {
      perror ("Allocating delta");
      exit (-1);
    }


  row = (NV_INT16 **) calloc (3600, sizeof (NV_INT16 *));
  if (row == NULL)
    {
      perror ("Allocating row");
      exit (-1);
    }

  for (i = 0 ; i < 3600 ; i++)
    {
      row[i] = (NV_INT16 *) calloc (3601, sizeof (NV_INT16));
      if (row[i] == NULL)
        {
          perror ("Allocating row[i]");
          exit (-1);
        }
    }


  /*  Are we on a big endian machine?  */

  if (big_endian ()) swap = 0;;


  /*  Set the water flag for the map area.  */

  srtm_bit_pack (water, 0, 32, 0);


  /*  Open the output file.  */

  sprintf (ofile, "%s%1csrtm1%1cRegion_0%1d.cte", dir, SEPARATOR, SEPARATOR, block);

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
  sprintf (&header_block[strlen (header_block)], "[END OF HEADER]\n");


  fwrite (header_block, HEADER_SIZE, 1, ofp);


  /*  Set the default for all map addresses to 2 (undefined).  Also, write the blank map area out to the
      file to save the space.  */

  for (i = 0 ; i < 180 ; i++)
    {
      for (j = 0 ; j < 360 ; j++)
        {
          srtm_double_bit_pack (map, (i * 360 + j) * 36, 36, 2);
        }
    }
  fseeko64 (ofp, (NV_INT64) HEADER_SIZE, SEEK_SET);
  fwrite (map, MAP_BYTES, 1, ofp);


  /*  Loop through -90 to 90 lat.  */

  for (i = -90 ; i < 90 ; i++)
    {
      lathem = 'N';
      if (i < 0) lathem = 'S';


      /*  Shift into the 0 to 180 world.  */

      shift_lat = i + 90;


      /*  Loop through the entire range of longitudes.  */

      for (j = -180 ; j < 180 ; j++)
        {
          lonhem = 'E';
          if (j < 0) lonhem = 'W';


          /*  Shift into the 0 to 360 world.  */

          shift_lon = j + 180;


          /*  Build the file name.  */

          sprintf (string, "Region_0%1d%1c%1c%02d%1c%03d.hgt", block, SEPARATOR, lathem, abs (i), lonhem, abs (j));


          /*  If we can open the file, read it.  */

          if ((fp = fopen (string, "rb")) != NULL)
            {
              prev_value = -1;
              pos = 0;


              /*  Note that we're only going to 3600 not 3601 because we don't need the redundant data.  */

              for (k = 0 ; k < 3600 ; k++)
                {
                  /*  Read one row (all 3601).  */

                  fread (row[k], 3601 * sizeof (NV_INT16), 1, fp);

                  for (m = 0 ; m < 3600 ; m++)
                    {
                      /*  If we're on a little endian system we need to swap the bytes.  */

                      if (swap)
                        {
#ifdef __GNUC__
                          swab (&row[k][m], &row[k][m], 2);
#else
                          _swab ((NV_CHAR *) &row[k][m], (NV_CHAR *) &row[k][m], 2);
#endif
                        }
                    }
                }


              first = 1;
              ndx = 0;
              min_diff = 32767;
              max_diff = -32768;
              hit_land = 0;
              hit_water = 0;


              /*  Loop through the cell.  */

              for (k = 0 ; k < 3600 ; k++)
                {

                  /*  Snake dance test.  */

                  if (!(k % 2))
                    {
                      /*  West to east.  */

                      for (m = 0 ; m < 3600 ; m++)
                        {
                          if (row[k][m] != -32768)
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
                              delta[ndx++] = diff;
                            }
                          else
                            {
                              /*  From what I've seen, -32768 means land but no defined elevation.  */

                              hit_land = 1;
                              delta[ndx++] = -32768;
                            }
                        }
                    }
                  else
                    {

                      /*  East to west.  */

                      for (m = 3599 ; m >= 0 ; m--)
                        {
                          if (row[k][m] != -32768)
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
                              delta[ndx++] = diff;
                            }
                          else
                            {
                              /*  From what I've seen, -32768 means land but no defined elevation.  */

                              hit_land = 1;
                              delta[ndx++] = -32768;
                            }
                        }
                    }
                }


              /*  All water cell.  */

              if (!hit_land && hit_water)
                {
                  srtm_double_bit_pack (map, (shift_lat * 360 + shift_lon) * 36, 36, 0);
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

                  total_bits = 16 + 16 + 4 + 3600 * 3600 * num_bits;
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

                  for (k = 0 ; k < 3600 * 3600 ; k++)
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
                  srtm_bit_pack (head, pos, 3, 0); pos += 3;
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


                  /*  Save the address of the block to the map.  */

                  srtm_double_bit_pack (map, (shift_lat * 360 + shift_lon) * 36, 36, lpos);
                }

              fclose (fp);


              /*  Set the block map value to the region number (1-7).  */

              block_map[shift_lat * 360 + shift_lon] = block;


              fprintf (stderr, "block: Region_0%1d   lon: %03d   lat: %03d       \r", block, j, i);
              fflush (stderr);
            }
        }
    }


  /*  Write the map.  */

  fseeko64 (ofp, (NV_INT64) HEADER_SIZE, SEEK_SET);
  fwrite (map, MAP_BYTES, 1, ofp);


  fclose (ofp);


  /*  Write the block map.  */

  fseek (block_fp, 0, SEEK_SET);
  fwrite (block_map, sizeof (block_map), 1, block_fp);

  fclose (block_fp);


  fprintf (stderr, "Region_0%1d processing complete             \n\n", block);
  fflush (stderr);


  return (0);
}
