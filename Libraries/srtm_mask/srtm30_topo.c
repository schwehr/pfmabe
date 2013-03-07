
/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


/***************************************************************************\
*                                                                           *
*   Module Name:        srtm30_topo                                         *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 2006                                        *
*                                                                           *
*   Purpose:            Reads the uncompressed SRTM30 files (*.hgt) and     *
*                       creates world (or as much as is covered)            *
*                       compressed topoographic elevation (.cte) file.      *
*                                                                           *
*   Arguments:          argv[1]         -   output file name                *
*                                                                           *
*   Caveats:            ALL of the SRTM3 hgt files must be in the directory *
*                       that you are running from (CWD).                    *
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


    For some unknown reason the majority of the N60 files in srtm3 are totally hosed up so I'm going to mark
    them as undefined (2).  Also, N59E170, N59W167, and N59W166 seem to be missing so these are forced to
    undefined as well.

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
#include "srtm30_topo_version.h"


#define HEADER_SIZE             16384
#define MAP_BYTES               (64800 * 36) / 8


/*  A function to determine if your system is big or litle endian.  Returns a 0 if it's little endian or a
    3 if it's big endian.  */

static NV_INT32 big_endian ()
{
  union
  {
    NV_INT32        word;
    NV_U_BYTE       byte[4];
  } a;

  a.word = 0x00010203;
  return ((NV_INT32) a.byte[3]);
}


NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  NV_INT32       i, j, k, m, shift_lat, shift_lon, percent = 0, old_percent = -1, n, hit_land = 0, hit_water = 0, first, ndx, 
                 pos, swap = 1, prev_value, rows, cols, nodata, lat_offset, lon_offset, start_lon, total_bits;
  uLong          total_bytes;
  uLongf         out_bytes;
  FILE           *ofp;
  NV_INT16       *frow, row[120][120], delta[14400], bias, start_val = 0, last_val = 0, diff, min_diff = 0, max_diff = 0, num_bits;
  NV_FLOAT64     xdim = 0.0, ydim = 0.0;
  FILE           *fp;
  NV_CHAR        string[512], ofile[512], ltstring[20], lonhem, header_block[HEADER_SIZE];
  NV_INT64       lpos;
  NV_U_BYTE      head[8], water[4], *in_buf, *out_buf, map[MAP_BYTES];
  time_t         t;
  struct tm      *cur_tm;


  printf ("\n\n%s\n\n", VERSION);


  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s OUTPUT_FILENAME\n\n", argv[0]);
      exit (-1);
    }


  /*  Are we on a big endian machine?  */

  if (big_endian ()) swap = 0;


  /*  Set the water flag for the map area.  */

  srtm_bit_pack (water, 0, 32, 0);


  /*  Open the output file.  */

  strcpy (ofile, argv[1]);
  if (strcmp (&ofile[strlen (ofile) - 4], ".cte")) strcat (ofile, ".cte");


  if ((ofp = fopen (ofile, "wb+")) == NULL)
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
  fseek (ofp, HEADER_SIZE, SEEK_SET);
  fwrite (map, MAP_BYTES, 1, ofp);


  /*  Loop over the entire world, south to north.  */

  for (i = -90 ; i < 90 ; i++)
    {
      shift_lat = i + 90;


      /*  Figure out what lat band we're using.  */

      if (i < -60)
        {
          sprintf (ltstring, "S60");
          lat_offset = 30 - shift_lat - 1;
        }
      else if (i < -10)
        {
          sprintf (ltstring, "S10");
          lat_offset = 80 - shift_lat - 1;
        }
      else if (i < 40)
        {
          sprintf (ltstring, "N40");
          lat_offset = 130 - shift_lat - 1;
        }
      else
        {
          sprintf (ltstring, "N90");
          lat_offset = 180 - shift_lat - 1;
        }


      /*  Loop over the entire world, west to east.  */

      for (j = -180 ; j < 180 ; j++)
        {
          shift_lon = j + 180;


          /*  Figure out what lon band we're using.  */

          if (i < -60)
            {
              start_lon = shift_lon / 60;
              start_lon *= 60;
            }
          else
            {
              start_lon = shift_lon / 40;
              start_lon *= 40;
            }

          if (start_lon < 180)
            {
              lonhem = 'W';
            }
          else
            {
              lonhem = 'E';
            }

          lon_offset = (shift_lon - start_lon) * 120;


          /*  Build the file name.  */

          sprintf (string, "%1c%03d%s.HDR", lonhem, abs (start_lon - 180), ltstring);


          /*  If we can open the header file, read it.  */

          if ((fp = fopen (string, "rb")) != NULL)
            {
              while (fgets (string, sizeof (string), fp) != NULL)
                {
                  if (strstr (string, "NROWS")) sscanf (string, "NROWS %d", &rows);
                  if (strstr (string, "NCOLS")) sscanf (string, "NCOLS %d", &cols);
                  if (strstr (string, "NODATA")) sscanf (string, "NODATA %d", &nodata);
                  if (strstr (string, "XDIM")) sscanf (string, "XDIM %lf", &xdim);
                  if (strstr (string, "YDIM")) sscanf (string, "YDIM %lf", &ydim);
                }
              fclose (fp);


              /*  We opened the .HDR file so we should be able to open the .DEM file.  */

              sprintf (string, "%1c%03d%s.DEM", lonhem, abs (start_lon - 180), ltstring);

              if ((fp = fopen (string, "rb")) == NULL)
                {
                  perror (string);
                  exit (-1);
                }


              frow = (NV_INT16 *) calloc (cols, sizeof (NV_INT16));

              if (frow == NULL)
                {
                  perror ("Allocating frow");
                  exit (-1);
                }


              /*  Find the cell we need.  */

              fseek (fp, lat_offset * 120 * cols * sizeof (NV_INT16), SEEK_SET);


              /*  Loop through the cell.  */

              for (k = 0 ; k < 120 ; k++)
                {

                  /*  Read a row.  */

                  fread (frow, cols * sizeof (NV_INT16), 1, fp);


                  /*  Loop through the row.  */

                  for (m = lon_offset ; m < lon_offset + 120 ; m++)
                    {
                      /*  If our system is little endian we need to swap the bytes.  */

                      if (swap)
                        {
#ifdef __GNUC__
                          swab (&frow[m], &frow[m], 2);
#else
                          _swab ((NV_CHAR *) &frow[m], (NV_CHAR *) &frow[m], 2);
#endif
                        }

                      row[k][m - lon_offset] = frow[m];
                    }
                }
              free (frow);
              fclose (fp);



              first = 1;
              ndx = 0;
              min_diff = 32767;
              max_diff = -32768;
              hit_land = 0;
              hit_water = 0;
              prev_value = -1;
              pos = 0;


              /*  Computing delta snake dance.  */


              /*  Loop through the cell.  */

              for (k = 0 ; k < 120 ; k++)
                {
                  /*  Snake dance test.  */

                  if (!(k % 2))
                    {

                      /*  West to east.  */

                      for (m = 0 ; m < 120 ; m++)
                        {
                          /*  From what I've seen, "nodata" means water.  */

                          if (row[k][m] != nodata)
                            {
                              hit_land = 1;
                            }
                          else
                            {
                              hit_water = 1;
                              row[k][m] = 0;
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
                    }
                  else
                    {

                      /*  East to west.  */

                      for (m = 119 ; m >= 0 ; m--)
                        {
                          /*  From what I've seen, "nodata" means water.  */

                          if (row[k][m] != nodata)
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


              /*  Add two to the max difference to allow room for the null value (doesn't exist for srtm30).  */

              max_diff += (bias + 2);


              /*  Compute the number of bits needed to store a delta.  */

              num_bits = NINT (log10 ((NV_FLOAT64) max_diff) / LOG2 + 0.5L);


              /*  Compute the total bytes needed to store the block.  */

              total_bits = 16 + 16 + 4 + 1440000 * num_bits;
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

              for (k = 0 ; k < 14400 ; k++)
                {
                  delta[k] += bias;

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
              srtm_bit_pack (head, pos, 3, 2); pos += 3;
              srtm_bit_pack (head, pos, 30, out_bytes); pos += 30;
              srtm_bit_pack (head, pos, 31, total_bytes);


              /*  Get the address where we're going to write the compressed block.  */

              fseek (ofp, 0, SEEK_END);
              lpos = ftell (ofp);


              /*  Write the header to the file.  */

              fwrite (head, 8, 1, ofp);


              /*  Write the buffer to the file.  */

              fwrite (out_buf, out_bytes, 1, ofp);
              free (out_buf);


              /*  Save the address of the block to the map.  */

              srtm_double_bit_pack (map, (shift_lat * 360 + shift_lon) * 36, 36, lpos);
            }
        }

      percent = (NV_INT32) (((NV_FLOAT32) (i + 90) / 180.0) * 100.0);
      if (percent != old_percent)
        {
          fprintf (stderr, "%03d%% processed\r", percent);
          fflush (stderr);
          old_percent = percent;
        }
    }


  /*  Write the map.  */

  fseek (ofp, HEADER_SIZE, SEEK_SET);
  fwrite (map, MAP_BYTES, 1, ofp);


  fclose (ofp);


  fprintf (stderr, "100%% processed      \n\n");
  fflush (stderr);


  return (0);
}
