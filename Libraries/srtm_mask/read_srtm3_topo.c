
/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


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



#define HEADER_SIZE      16384
#define MAP_BYTES        (64800 * 36) / 8


static NV_BOOL           no_file = NVFalse, first = NVTrue;
static NV_INT32          prev_size = -1;
static FILE              *fp = NULL;
static NV_INT16          *box = NULL;
static NV_U_BYTE         *map = NULL, block_map[64800];





/***************************************************************************\
*                                                                           *
*   Module Name:        check_srtm3_topo                                    *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 2007                                       *
*                                                                           *
*   Purpose:            Checks for availability of the                      *
*                       SRTM3 topo data file.                               *
*                                                                           *
*   Arguments:          None                                                *
*                                                                           *
*   Returns:            NVTrue or NVFalse                                   *
*                                                                           *
\***************************************************************************/


NV_BOOL check_srtm3_topo ()
{
  NV_CHAR    dir[512], file[512];
  FILE       *block_fp;

  if (getenv ("SRTM_DATA") == NULL)
    {
      no_file = NVTrue;
      return (NVFalse);
    }

  strcpy (dir, getenv ("SRTM_DATA"));


  /*  Check the block map file.  */

  sprintf (file, "%s%1csrtm3%1csrtm3_block_map.dat", dir, SEPARATOR, SEPARATOR);

  if ((block_fp = fopen (file, "rb")) == NULL)
    {
      no_file = NVTrue;
      return (NVFalse);
    }
  fclose (block_fp);


  return (NVTrue);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        read_srtm3_topo_one_degree                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 2006                                        *
*                                                                           *
*   Purpose:            Reads the SRTM compressed topographic elevation     *
*                       file (*.cte) and returns a one-degree single        *
*                       dimensioned array containing the elevations in the  *
*                       same format as the srtm3 data files.  The           *
*                       width/height of the array is returned.              *
*                                                                           *
*   Arguments:          lat             -   degree of latitude, S negative  *
*                       lon             -   degree of longitude, W negative *
*                       array           -   topo array                      *
*                                                                           *
*   Returns:            0 for all water cell, 2 for undefined cell, or      *
*                       1200 (width/height of array, it's square).          *
*                                                                           *
*   Caveats:            The array is one dimensional so the user/caller     *
*                       must index into the array accordingly.  The data is *
*                       stored in order from the northwest corner of the    *
*                       cell, west to east, then north to south so the last *
*                       point in the returned array is the southeast        *
*                       corner.  See pointlat and pointlon in the following *
*                       example code:                                       *
*                                                                           *
*                                                                           *
*                       #include "read_srtm3_topo.h"                        *
*                                                                           *
*                       NV_INT16           *array;                          *
*                       NV_INT32           size;                            *
*                       NV_FLOAT64         inc, pointlat, pointlon;         *
*                       NV_INT32           lat, lon;                        *
*                                                                           *
*                       if (lat < 0) lat--;                                 *
*                       if (lon < 0) lon--;                                 *
*                                                                           *
*                       size = read_srtm3_topo_one_degree (lat, lon,        *
*                           &array);                                        *
*                                                                           *
*                       if (size != 32767 && size > 2)                      *
*                         {                                                 *
*                           inc = 1.0L / size;                              *
*                           for (i = 0 ; i < size ; i++)                    *
*                             {                                             *
*                               pointlat = (lat + 1.0L) - (i + 1) * inc;    *
*                               for (j = 0 ; j < size ; j++)                *
*                                 {                                         *
*                                   pointlon = lon + j * inc;               *
*                                   //DO SOMETHING WITH array[i * size + j] *
*                                 }                                         *
*                             }                                             *
*                         }                                                 *
*                                                                           *
*                       IMPORTANT NOTE: The latitude and longitude used to  *
*                       retrieve a one-degree cell are the lat and lon of   *
*                       the southwest corner of the cell.  Since the data   *
*                       was originally stored north to south you need to    *
*                       add one to the latitude to get the proper position  *
*                       (note the pointlat computation above).  This also   *
*                       means that you should be very careful about         *
*                       hemisphere when retrieving.  If you want the cell   *
*                       containing 19.88564N,156.3265W you need to request  *
*                       the cell 19,-157 not 19,-156.  The same holds true  *
*                       in the southern hemisphere.  If you need the cell   *
*                       holding 19.88564S,156.3265W you need to request     *
*                       -20,-157 not -19,157.                               *
*                                                                           *
*                                                                           *
*                       You should also call cleanup_srtm3_topo after you   *
*                       are finished using the database in order to close   *
*                       the open file and free the associated memory.       *
*                                                                           *
\***************************************************************************/


NV_INT32 read_srtm3_topo_one_degree (NV_INT32 lat, NV_INT32 lon, NV_INT16 **array)
{
  static NV_CHAR   dir[512], file[512], version[128], zversion[128];
  static NV_INT32  header_size, prev_lat = -999, prev_lon = -999, prev_block = -1;
  FILE             *block_fp;
  NV_CHAR          varin[1024], info[1024], header_block[HEADER_SIZE];
  NV_CHAR          dir_name[6][40] = {"Africa", "Australia", "Eurasia", "Islands", "North_America", "South_America"};
  NV_U_BYTE        *buf, *bit_box = NULL, head[4];
  NV_INT32         i, j, shift_lat, shift_lon, resolution, pos, size = 0, status, ndx, block;
  NV_INT64         address;
  uLong            csize;
  uLongf           bsize;
  NV_INT16         start_val, bias, null_val, num_bits, temp, last_val;


  if (no_file) return (-1);


  /*  First time through, open the file and read the header.    */

  if (first)
    {
      if (getenv ("SRTM_DATA") == NULL)
        {
          fprintf (stderr, "Unable to find SRTM_DATA environment variable\n");
          fflush (stderr);
          no_file = NVTrue;
          return (-1);
        }

      strcpy (dir, getenv ("SRTM_DATA"));

      sprintf (file, "%s%1csrtm3%1csrtm3_block_map.dat", dir, SEPARATOR, SEPARATOR);


      if ((block_fp = fopen (file, "rb")) == NULL)
        {
          perror (file);
          no_file = NVTrue;
          return (-1);
        }


      fseek (block_fp, 0, SEEK_SET);
      fread (block_map, sizeof (block_map), 1, block_fp);

      fclose (block_fp);

      first = NVFalse;
    }


  /*  If we're working in the real 0-360 world (where 0 to 0 is 0 to 360) we want to turn longitudes greater
      than 180 into negatives before we switch to the bogus 0-360 world (where -180 to 180 is 0 to 180).  */

  if (lon >= 180) lon -= 360;


  /*  Shift into a 0 to 180 by 0 to 360 world.  */

  shift_lat = lat + 90;
  shift_lon = lon + 180;


  block = shift_lat * 360 + shift_lon;


  if (block_map[block] != prev_block)
    {
      /*  If the block_map value is 0 then no data was loaded for this cell.  */

      if (!block_map[block]) return (2);


      prev_block = block;
      sprintf (file, "%s%1csrtm3%1c%s.cte", dir, SEPARATOR, SEPARATOR, dir_name[block_map[block] - 1]);


      /*  Note that we are returning undefined if we can't find the file.  This is to allow the use of 
          a subset of the area block files without requiring all of the files to be present.  */

      if (fp) fclose (fp);
      if ((fp = fopen64 (file, "rb")) == NULL) return (2);


      /*  Allocate the map memory.  */

      if (map) free (map);
      map = (NV_U_BYTE *) calloc (MAP_BYTES, sizeof (NV_U_BYTE));
      if (map == NULL)
        {
          perror ("Allocating map memory");
          exit (-1);
        }


      /*  Read the header block.  */

      fread (header_block, HEADER_SIZE, 1, fp);


      ndx = 0;
      while (header_block[ndx] != 0)
        {
          for (i = 0 ; i < 1024 ; i++)
            {
              if (header_block[ndx] == '\n') break;
              varin[i] = header_block[ndx];
              ndx++;
            }

          varin[i] = 0;

          if (strstr (varin, "[END OF HEADER]")) break;


          /*  Put everything to the right of the equals sign in 'info'.   */

          if (strchr (varin, '=') != NULL) strcpy (info, (strchr (varin, '=') + 1));

          if (strstr (varin, "[VERSION]")) strcpy (version, info);

          if (strstr (varin, "[ZLIB VERSION]"))
            {
              strcpy (zversion, info);

              sscanf (zversion, "%d.", &i);
              sscanf (zlibVersion (), "%d.", &j);

              if (i != j)
                {
                  fprintf (stderr, "\n\nZlib library version (%s) is not compatible with version used to build SRTM file (%s)\n\n",
                           zlibVersion (), zversion);
                  exit (-1);
                }
            }

          if (strstr (varin, "[HEADER SIZE]")) sscanf (info, "%d", &header_size);

          ndx++;
        }


      if (header_size != HEADER_SIZE)
        {
          fprintf (stderr, "Header sizes do not match, WTF, over!\n");
          exit (-1);
        }


      /*  Move past the end of the header and read the map.  */

      fseeko64 (fp, (NV_INT64) header_size, SEEK_SET);
      fread (map, MAP_BYTES, 1, fp);

      first = NVFalse;
    }


  /*  Only read the data if we have changed one-degree cells.  */

  if (prev_lat != shift_lat || prev_lon != shift_lon)
    {
      /*  Unpack the address from the map.  */

      address = srtm_double_bit_unpack (map, (shift_lat * 360 + shift_lon) * 36, 36);

      prev_lat = shift_lat;
      prev_lon = shift_lon;


      /*  If the address is 0 (water) or 2 (undefined), return the address.  */

      if (address < header_size) return ((NV_INT32) address);


      /*  Move to the address and read/unpack the header.  */

      fseeko64 (fp, address, SEEK_SET);
      fread (head, 8, 1, fp);

      pos = 0;
      resolution = (NV_INT32) srtm_bit_unpack (head, pos, 3); pos += 3;
      csize = (uLong) srtm_bit_unpack (head, pos, 30); pos += 30;
      bsize = (uLongf) srtm_bit_unpack (head, pos, 31);


      size = 1200;


      /*  We have to set an approximate size for unpacking (see the ZLIB documentation).  */

      bsize += NINT ((NV_FLOAT32) bsize * 0.10) + 12;


      /*  Allocate the uncompressed memory.  */

      bit_box = (NV_U_BYTE *) calloc (bsize, sizeof (NV_U_BYTE));
      if (bit_box == NULL)
        {
          perror ("Allocating bit_box memory in read_srtm3_topo");
          exit (-1);
        }


      /*  Allocate the compressed memory.  */

      buf = (NV_U_BYTE *) calloc (csize, sizeof (NV_U_BYTE));
      if (buf == NULL)
        {
          perror ("Allocating buf memory");
          exit (-1);
        }


      /*  Read the compressed data.  */

      fread (buf, csize, 1, fp);


      /*  Uncompress the data.  */

      status = uncompress (bit_box, &bsize, buf, csize);
      if (status)
        {
          fprintf (stderr, "Error %d uncompressing record\n", status);
          fprintf (stderr, "%d %d "NV_INT64_SPECIFIER"\n", lat, lon, address);
          exit (-1);
        }

      free (buf);


      /*  Unpack the internal header.  */

      pos = 0;
      start_val = srtm_bit_unpack (bit_box, pos, 16); pos += 16;
      bias = srtm_bit_unpack (bit_box, pos, 16); pos += 16;
      num_bits = srtm_bit_unpack (bit_box, pos, 4); pos += 4;
      null_val = NINT (pow (2.0L, (NV_FLOAT64) num_bits)) - 1;


      /*  Allocate the cell memory.  */

      if (box != NULL) free (box);

      box = (NV_INT16 *) calloc (size * size, sizeof (NV_INT16));
      if (box == NULL)
        {
          perror ("Allocating box memory in read_srtm3_topo");
          exit (-1);
        }


      /*  Uncompress the data (delta coded snake dance).  */

      last_val = start_val;
      for (i = 0 ; i < size ; i++)
        {
          if (!(i % 2))
            {
              for (j = 0 ; j < size ; j++)
                {
                  temp = srtm_bit_unpack (bit_box, pos, num_bits); pos += num_bits;

                  if (temp < null_val)
                    {
                      box[i * size + j] = last_val + temp - bias;
                      last_val = box[i * size + j];
                    }
                  else
                    {
                      box[i * size + j] = -32768;
                    }
                }
            }
          else
            {
              for (j = size - 1 ; j >= 0 ; j--)
                {
                  temp = srtm_bit_unpack (bit_box, pos, num_bits); pos += num_bits;

                  if (temp < null_val)
                    {
                      box[i * size + j] = last_val + temp - bias;
                      last_val = box[i * size + j];
                    }
                  else
                    {
                      box[i * size + j] = -32768;
                    }
                }
            }
        }


      free (bit_box);

      prev_size = size;
    }


  /*  We didn't change cells so set the same size as last time.  */

  else
    {
      size = prev_size;
    }


  *array = box;


  return (size);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        read_srtm3_topo                                     *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       September 2006                                      *
*                                                                           *
*   Purpose:            Reads the SRTM compressed topographic elevation     *
*                       file (*.cte) and returns the elevation value.  If   *
*                       the value is undefined at that point it will return *
*                       -32768.  For water it will return 0.                *
*                                                                           *
*   Arguments:          lat             -   latitude degrees, S negative    *
*                       lon             -   longitude degrees, W negative   *
*                                                                           *
*   Returns:            0 = water, -32768 undefined, elevation, 32767 on    *
*                       error                                               *
*                                                                           *
\***************************************************************************/


NV_INT16 read_srtm3_topo (NV_FLOAT64 lat, NV_FLOAT64 lon)
{
  static NV_INT16    *array;
  static NV_INT32    prev_ilat = -999, prev_ilon = -999, size = 0;
  static NV_FLOAT64  inc = 0.0;
  NV_INT32           ilat, ilon, lat_index, lon_index;


  if (no_file) return (32767);


  /*  If we're working in the real 0-360 world (where 0 to 0 is 0 to 360) we want to turn longitudes greater
      than 180 into negatives before we switch to the bogus 0-360 world (where -180 to 180 is 0 to 180).  */

  if (lon >= 180.0) lon -= 360.0;
  if (lon < 0.0) lon -= 1.0;  
  if (lat < 0.0) lat -= 1.0;


  ilat = (NV_INT32) lat;
  ilon = (NV_INT32) lon;


  /*  No point in calling the function if we didn't change cells.  */

  if (ilat != prev_ilat || ilon != prev_ilon) 
    {
      prev_ilat = ilat;
      prev_ilon = ilon;


      size = read_srtm3_topo_one_degree (ilat, ilon, &array);

      if (size < 0)
        {
          no_file = NVTrue;
          return (32767);
        }

      if (size == 0) return (0);

      inc = 1.0L / (NV_FLOAT64) size;
    }

  if (size == 0) return (0);
  if (size == 2) return (-32768);


  /*  Get the cell index.  */

  lon += 180.0;
  ilon = (NV_INT32) lon;

  lat += 90.0;
  ilat = (NV_INT32) lat;

  lat_index = (NV_INT32) ((((NV_FLOAT64) ilat + 1.0L) - lat) / inc) + 1;
  lon_index = (NV_INT32) ((lon - (NV_FLOAT64) ilon) / inc);

  return (array[lat_index * size + lon_index]);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        cleanup_srtm3_topo                                  *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 2006                                        *
*                                                                           *
*   Purpose:            Closes the open srtm3 topo file, frees memory, and  *
*                       sets the first flag back to NVTrue.                 *
*                                                                           *
*   Arguments:          None                                                *
*                                                                           *
*   Returns:            Nada                                                *
*                                                                           *
\***************************************************************************/


void cleanup_srtm3_topo ()
{
  if (fp) fclose (fp);
  if (box != NULL) free (box);
  if (map != NULL) free (map);
  box = NULL;
  map = NULL;
  fp = NULL;
  first = NVTrue;
  prev_size = -1;
  no_file = NVFalse;
}
