
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "read_coast.h"
#include "bit_pack.h"


/*  This lets me change file names without changing application code.  */

static NV_CHAR            files[COAST_TYPES][20] = {"coast50k.ccl", "gshhs_all.ccl", "gshhs_isle.ccl", "gshhs_lake.ccl", "gshhs_land.ccl",
                                                    "gshhs_pond.ccl", "wvsfull.dat", "wvs250k.dat", "wvs1.dat", "wvs3.dat", "wvs12.dat", "wvs43.dat"};

/*!

   - Module Name:        read_coast

   - Programmer(s):      Jan C. Depner (jan.depner@navy.mil)

   - Date Written:       July 2006


   - Purpose:   Reads a compressed world coastline file created with the build_coast program and returns Vector line 
                segments to the caller.  The file must be stored in the directory pointed to by the environment variable
                WVS_DIR.  Since all data written by build_coast used unsigned character buffers there are no "endian"
                issues to deal with.  The files are in the following format:


                - Version:
                    - 128 characters ASCII


                - Header:
                    - 180 X 360 groups of three, 32 bit integers stored as character buffers.  Each group consists of the
                      address of the block of segments for the associated one-degree cell, the total number of segments in
                      the cell, and the total number of vertices in the cell.  The order of the 180 X 360 cells goes from
                      west to east, south to north beginning at -90/-180.  That is, the first cell is -90/-180, the next is 
                      -90/-179, etc. until we reach -90/179 at which point we go to -89/-180 and so on.  We add 90 to all
                      latitudes and 180 to all longitudes so that we can work in positive numbers so, in essence we really
                      go from 0/0 to 89/359.


                - Cell records:
                      - Each cell consists of a number of segments.  Each segment in the cell consists of the following:
                            - 5 bits:        count bits
                            - 5 bits:        lon offset bits
                            - 5 bits:        lat offset bits
                            - count bits:    count of vertices in the segment
                            - 18 bits:       lon bias + 2**17
                            - 18 bits:       lat bias + 2**17
                            - 26 bits:       start lon (times 100000)
                            - 25 bits:       start lat (times 100000)
                            - count * (lon offset bits + lat offset bits):   lon and lat offsets (plus biases) from previous point


   - Some notes on the sizes - count bits is the number of bits needed to store the count of vertices in the
   current segment.  This should never exceed 32.  If it does we've got a problem.  Lat and lon offset
   bits is the number of bits needed to store the delta coded offsets between each lat and lon and the
   previous lat and lon (or the start lat and lon) in the segment.  This also should never exceed 32.  The
   lat and lon biases are values that we add to each lat and lon offset in order to make sure that we store
   all offsets as positive values (I hate messing with sign extension ;-)  We use 18 bits to store these
   because they should never exceed +-100000.  If they do then you have a line segment that is greater than
   one degree and that is probably bogus.  We add 2**17 (131071) to this number before we store it in order
   to ensure that it is always positive (stinkin' sign extension again ;-)  We store the start lat and lon
   in 25 and 26 bits respectively because they are stored as positive integers times 100000 (range
   0-17999999 and 0-35999999 respectively).  This gives us a resolution of about 1 meter at the equator.
   In the olden days (when dinosaurs roamed the earth) I would have stored the start lat and lon as offsets
   from the corner of the cell in order to save those few bits.  I would have also computed the number of
   bits needed to store all of the bit counts.  This is what I did in 1981 with the original CIA WDBII
   data and again in 1989 with the WVS data.  We used to have to worry about every bit back then.  Now I
   find it much easier to understand if I keep that kind of logistical nightmare to a minimum ;-)  The
   savings in storage are pretty minimal anyway.


   - Arguments:
                            - type      =  Coastline type:
                                  - COAST_50K     1:50,000 scale NGA coastline (not complete)
                                  - GSHHS_ALL     Cleaned up WVS major coastline
                                  - GSHHS_ISLE    Cleaned up WVS isles
                                  - GSHHS_LAKE    Cleaned up WVS lakes
                                  - GSHHS_LAND    Cleaned up WVS islands in lakes????
                                  - GSHHS_POND    Cleaned up WVS ponds
                                  - WVSFULL       Old compressed WVS full resolution
                                  - WVS250K       Old compressed WVS 1:250,000
                                  - WVS1          Old compressed WVS 1:1,000,000
                                  - WVS3          Old compressed WVS 1:3,000,000
                                  - WVS12         Old compressed WVS 1:12,000,000
                                  - WVS43         Old compressed WVS 1:43,000,000
                                  - WDB_COASTS    CIA World Data Bank coastline
                                  - WDB_RIVERS    CIA World Data Bank rivers
                                  - WDB_BOUNDS    CIA World Data Bank political boundaries
                            - lon       =   longitude degree of one-degree cell to read (-180 to 179)
                            - lat       =   latitude degree of one-degree cell to read (-90 to 89)
                            - **x       =   pointer to array of returned longitudes.  THE CALLER MUST FREE THE ARRAY AFTER USE!
                            - **y       =   pointer to array of returned latitudes.  THE CALLER MUST FREE THE ARRAY AFTER USE!

                            <pre>

                                   Example:

                                   NV_FLOAT64 *coast_x, *coast_y;

                                   while ((segCount = read_coast (j, i, &coast_x, &coast_y)))
                                     {
                                       Do something...

                                       free (coast_x);
                                       free (coast_y);
                                     }

                            </pre>

   - Return Value:  NV_INT32    -   -1 on error, 0 on end of data for the cell, otherwise number of vectors returned

*/

NV_INT32 read_coast (NV_INT32 type, NV_INT32 lon, NV_INT32 lat, NV_FLOAT64 **x, NV_FLOAT64 **y)
{
  static FILE               *fp = NULL;
  NV_CHAR                   fname[512];
  static NV_CHAR            version[128];
  static NV_INT32           l_lon, l_lat, prev_lon = -999, prev_lat = -999, num_vertices, seg_read = 0, num_segments = -1, address, prev_type = -1;
  NV_U_BYTE                 *buffer, head_buf[20];
  NV_INT32                  i, count_bits, lon_offset_bits, lat_offset_bits, segCount, size, offset, pos, save_add, bias_x, 
                            bias_y, max_bias;
  NV_FLOAT64                start_lon, start_lat;
  NV_BOOL                   dateline = NVFalse;


  dateline = NVFalse;

  if (lon >= 180.0)
    {
      dateline = NVTrue;
      lon -= 360.0;
    }


  l_lon = lon;
  l_lat = lat;

  l_lon += 180;
  l_lat += 90;

  if (seg_read == num_segments)
    {
      seg_read = 0;
      num_segments = -1;
      prev_lat = -999;
      prev_lon = -999;

      return (0);
    }


  if (type != prev_type)
    {
      if (type < 0 || type > WVS43)
        {
          fprintf (stderr, "Unknown file type %d\n", type);
          return (-1);
        }


      prev_type = type;


      /*  Use the environment variable WVS_DIR to get the         */
      /*  directory name.                                         */
      /*                                                          */
      /*  To set the variable in csh use :                        */
      /*                                                          */
      /*      setenv WVS_DIR /usr/wdbii                           */
      /*                                                          */
      /*  To set the variable in bash, sh, or ksh use :           */
      /*                                                          */
      /*      WVS_DIR=/usr/wdbii                                  */
      /*      export WVS_DIR                                      */


      if (getenv ("WVS_DIR") == NULL)
        {
          fprintf (stderr, "\n\nEnvironment variable WVS_DIR is not set\n\n");
          fflush (stderr);
          return (-1);
        }

      sprintf (fname, "%s%1c%s", getenv ("WVS_DIR"), (NV_CHAR) SEPARATOR, files[type]);


      if (fp) fclose (fp);


      if ((fp = fopen (fname, "rb")) == NULL)
        {
          perror (fname);
          return (-1);
        }

      fread (version, 128, 1, fp);
    }


  if (l_lon != prev_lon || l_lat != prev_lat)
    {
      offset = (l_lat * 360 + l_lon) * (3 * sizeof (NV_INT32)) + 128;
      fseek (fp, offset, SEEK_SET);

      fread (head_buf, 3 * sizeof (NV_INT32), 1, fp);


      i = 8 * sizeof (NV_INT32);

      pos = 0;
      address = bit_unpack (head_buf, pos, i); pos += i;

      if (!address) return (0);


      num_segments = bit_unpack (head_buf, pos, i); pos += i;
      num_vertices = bit_unpack (head_buf, pos, i);


      fseek (fp, address, SEEK_SET);

      seg_read = 0;
    }


  prev_lat = l_lat;
  prev_lon = l_lon;


  save_add = ftell (fp);
  fread (head_buf, 20, 1, fp);

  max_bias = (NV_INT32) (pow (2.0, 17.0) - 1.0);

  pos = 0;
  count_bits = bit_unpack (head_buf, pos, 5); pos += 5;
  lon_offset_bits = bit_unpack (head_buf, pos, 5); pos += 5;
  lat_offset_bits = bit_unpack (head_buf, pos, 5); pos += 5;
  segCount = bit_unpack (head_buf, pos, count_bits); pos += count_bits;
  bias_x = bit_unpack (head_buf, pos, 18) - max_bias; pos += 18;
  bias_y = bit_unpack (head_buf, pos, 18) - max_bias; pos += 18;
  start_lon = bit_unpack (head_buf, pos, 26); pos += 26;
  start_lat = bit_unpack (head_buf, pos, 25); pos += 25;
  size = 5 + 5 + 5 + count_bits + lon_offset_bits + lat_offset_bits + 18 + 18 + 26 + 25 + (segCount - 1) * (lon_offset_bits + lat_offset_bits);

  if (!segCount) return (0);

  size = size / 8 + 1;


  fseek (fp, save_add, SEEK_SET);

  buffer = (NV_U_BYTE *) calloc (1, size);

  if (buffer == NULL)
    {
      perror ("Allocating buffer");
      exit (-1);
    }

  fread (buffer, size, 1, fp);
  seg_read++;


  *x = (NV_FLOAT64 *) calloc (segCount, sizeof (NV_FLOAT64));

  if (*x == NULL)
    {
      perror ("Allocating X array memory in read_coast");
      exit (-1);
    }

  *y = (NV_FLOAT64 *) calloc (segCount, sizeof (NV_FLOAT64));

  if (*y == NULL)
    {
      perror ("Allocating Y array memory in read_coast");
      exit (-1);
    }


  (*x)[0] = (NV_FLOAT64) start_lon;
  (*y)[0] = (NV_FLOAT64) start_lat;

  for (i = 1 ; i < segCount ; i++)
    {
      (*x)[i] = ((*x)[i - 1] + (NV_FLOAT64) (bit_unpack (buffer, pos, lon_offset_bits)) - bias_x);
      pos += lon_offset_bits;
      (*y)[i] = ((*y)[i - 1] + (NV_FLOAT64) (bit_unpack (buffer, pos, lat_offset_bits)) - bias_y);
      pos += lat_offset_bits;
    }

  for (i = 0 ; i < segCount ; i++)
    {
      (*x)[i] = (*x)[i] / 100000.0L - 180.0L;
      if (dateline) (*x)[i] += 360.0;
      (*y)[i] = (*y)[i] / 100000.0L - 90.0L;
    }


  free (buffer);

  return (segCount);
}



/*!

   - Module Name:        check_coast

   - Programmer(s):      Jan C. Depner

   - Date Written:       July 2006


   - Purpose:   Check to see if the file "file" exists in the $WVS_DIR directory.

*/

NV_BOOL check_coast (NV_INT32 type)
{
  FILE                      *fp;
  NV_CHAR                   fname[512];


  /*  Use the environment variable WVS_DIR to get the         */
  /*  directory name.                                         */
  /*                                                          */
  /*  To set the variable in csh use :                        */
  /*                                                          */
  /*      setenv WVS_DIR /usr/wdbii                           */
  /*                                                          */
  /*  To set the variable in bash, sh, or ksh use :           */
  /*                                                          */
  /*      WVS_DIR=/usr/wdbii                                  */
  /*      export WVS_DIR                                      */


  if (type < 0 || type > WVS43)
    {
      fprintf (stderr, "Unknown file type %d\n", type);
      return (NVFalse);
    }


  if (getenv ("WVS_DIR") == NULL)
    {
      fprintf (stderr, "\n\nEnvironment variable WVS_DIR is not set\n\n");
      fflush (stderr);
      return (NVFalse);
    }
    
  sprintf (fname, "%s%1c%s", getenv ("WVS_DIR"), (NV_CHAR) SEPARATOR, files[type]);

  if ((fp = fopen (fname, "rb")) == NULL)
    {
      /*  We don't want to issue the warning if we can't find the coast50k.ccl file (type = 0) since it is DoD restricted and might not be
          loaded on all systems.  */

      if (type) perror (fname);

      return (NVFalse);
    }

  fclose (fp);
  return (NVTrue);
}

