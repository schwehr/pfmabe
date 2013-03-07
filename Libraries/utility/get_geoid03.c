
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "nvtypes.h"
#include "swap_bytes.h"


static NV_FLOAT32 *array = NULL;


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



/*!  Gets orthometric correction for specified lat/lon from geoid03 bin files.  */

NV_FLOAT32 get_geoid03 (NV_FLOAT64 lat, NV_FLOAT64 lon)
{
  static NV_INT32 first = 1, swap[14], prev_file = -99, rows[14], cols[14];
  static NV_FLOAT64 lat_bounds[14][2], lon_bounds[14][2], short_lat[14][2], short_lon[14][2], lat_space[14], lon_space[14];
  static NV_CHAR dirfil[14][512], wvsdir[256];
  static NV_CHAR file[14][20] = {"g2003u01.bin", "g2003u02.bin", "g2003u03.bin", "g2003u04.bin",
                                 "g2003u05.bin", "g2003u06.bin", "g2003u07b.bin", "g2003u08.bin",
                                 "g2003a01.bin", "g2003a02.bin", "g2003a03.bin", "g2003a04.bin",
                                 "g2003h01.bin", "g2003p01.bin"};

  NV_INT32 i, j, row, col, endian, current_file, ll_ndx, ul_ndx, ur_ndx, lr_ndx, size;
  NV_FLOAT32 ll_height, ul_height, ur_height, lr_height, l_diff, r_diff, lr_diff, l_height, r_height, height;
  NV_FLOAT64 lat_grid, lon_grid;
  FILE *fp;



  /*  The first time through we need to get the file names and the bounds of all of the files.  */

  if (first)
    {
      /*  Use the environment variable WVS_DIR to get the directory name.   */

      if (getenv ("WVS_DIR") == NULL)
        {
          fprintf (stderr, "\n\nEnvironment variable WVS_DIR is not set\n\n");
          fflush (stderr);
          return (-999.0);
        }
    
      strcpy (wvsdir, getenv ("WVS_DIR"));
      if (wvsdir[0] == 0)
        {
          fprintf (stderr, "\n\nEnvironment variable WVS_DIR is not set\n\n");
          fflush (stderr);
          return (-999.0);
        }


      for (i = 0 ; i < 14 ; i++)
        {
          sprintf (dirfil[i], "%s%1c%s", wvsdir, (NV_CHAR) SEPARATOR, file[i]);

          if ((fp = fopen (dirfil[i], "rb")) == NULL)
            {
              perror (dirfil[i]);
              rows[i] = 0;
            }
          else
            {
              fread (&lat_bounds[i][0], sizeof (NV_FLOAT64), 1, fp);
              fread (&lon_bounds[i][0], sizeof (NV_FLOAT64), 1, fp);
              fread (&lat_space[i], sizeof (NV_FLOAT64), 1, fp);
              fread (&lon_space[i], sizeof (NV_FLOAT64), 1, fp);
              fread (&rows[i], sizeof (NV_INT32), 1, fp);
              fread (&cols[i], sizeof (NV_INT32), 1, fp);
              fread (&endian, sizeof (NV_INT32), 1, fp);


              if ((big_endian () && endian) || (!big_endian () && !endian))
                {
                  swap[i] = 1;

                  swap_double (&lat_bounds[i][0]);
                  swap_double (&lon_bounds[i][0]);
                  swap_double (&lat_space[i]);
                  swap_double (&lon_space[i]);
                  swap_int (&rows[i]);
                  swap_int (&cols[i]);
                }
              else
                {
                  swap[i] = 0;
                }


              lat_bounds[i][1] = lat_bounds[i][0] + lat_space[i] * rows[i];

              lon_bounds[i][1] = lon_bounds[i][0] + lon_space[i] * cols[i];

	      short_lat[i][0] = lat_bounds[i][0] + 0.1;
	      short_lat[i][1] = lat_bounds[i][1] - 0.1;
	      short_lon[i][0] = lon_bounds[i][0] + 0.1;
	      short_lon[i][1] = lon_bounds[i][1] - 0.1;

              fclose (fp);
            }
        }
      first = 0;
    }


  /*  Switch to 0-360 world if needed  */

  if (lon < 0.0) lon += 360.0;


  /*  Figure out which file we need.  We're using short bounds so that we don't have to worry about edge
      conditions.  */

  current_file = -1;
  for (i = 0 ; i < 14 ; i++)
    {
      if (rows[i])
        {
          if (lat > short_lat[i][0] && lat < short_lat[i][1] && lon > short_lon[i][0] && lon < short_lon[i][1])
            {
              current_file = i;
              break;
            }
        }
    }


  /*  If our point isn't in any of the files, return a "bad" value  */

  if (current_file == -1) return (-999.0);


  /*  If we've changed files, read the new one into memory  */

  if (current_file != prev_file)
    {
      if (array) free (array);

      size = rows[current_file] * cols[current_file] * sizeof (NV_FLOAT32);

      array = (NV_FLOAT32 *) calloc (1, size);

      if (array == NULL)
        {
          perror ("Allocating geoid array memory in get_geoid03");
          exit (-1);
        }


      if ((fp = fopen (dirfil[current_file], "rb")) == NULL)
        {
          perror (dirfil[current_file]);
          exit (-1);
        }


      /*  Skip the header  */

      fseek (fp, 44, SEEK_SET);


      /*  Read the whole stinkin' file as one big block.  */

      fread (array, size, 1, fp);


      /*  Swap if needed  */

      if (swap[current_file])
        {
          for (i = 0 ; i < rows[current_file] ; i++)
            {
              for (j = 0 ; j < cols[current_file] ; j++)
                {
                  swap_float (&array[i * cols[current_file] + j]);
                }
            }
        }

      fclose (fp);

      prev_file = current_file;
    }


  /*  Finally we get down to cases...  */

  row = (NV_INT32) ((lat - lat_bounds[current_file][0]) / lat_space[current_file]);
  col = (NV_INT32) ((lon - lon_bounds[current_file][0]) / lon_space[current_file]);

  lat_grid = lat_bounds[current_file][0] + row * lat_space[current_file];
  lon_grid = lon_bounds[current_file][0] + col * lon_space[current_file];

  ll_ndx = row * cols[current_file] + col;
  ul_ndx = (row + 1) * cols[current_file] + col;
  ur_ndx = ul_ndx + 1;
  lr_ndx = ll_ndx + 1;

  ll_height = array[ll_ndx];
  ul_height = array[ul_ndx];
  ur_height = array[ur_ndx];
  lr_height = array[lr_ndx];


  /*  Interpolate top to bottom on left and right sides  */

  l_diff = ul_height - ll_height;
  r_diff = ur_height - lr_height;

  l_height = ll_height + ((lat - lat_grid) / lat_space[current_file]) * l_diff;
  r_height = lr_height + ((lat - lat_grid) / lat_space[current_file]) * r_diff;


  /*  Interpolate right to left at lat  */

  lr_diff = r_height - l_height;

  height = l_height + ((lon - lon_grid) / lon_space[current_file]) * lr_diff;

  return (height);
}


void free_geoid03 ()
{
  if (array) free (array);
}



#undef TEST_MAIN


/*  Test main  */

#ifdef TEST_MAIN

NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  NV_FLOAT32 value;
  NV_FLOAT64 lat, lon;


  if (argc < 3)
    {
      fprintf (stderr, "\nUsage: %s LAT LON\n\n", argv[0]);
      exit (-1);
    }


  sscanf (argv[1], "%lf", &lat);
  sscanf (argv[2], "%lf", &lon);


  value = get_geoid03 (lat, lon);

  fprintf (stderr,"%f\n", value);
  fflush (stderr);


  free_geoid03 ();

  return (0);
}


#endif


#undef TEST2_MAIN


/*  Test2 main  */

#ifdef TEST2_MAIN

NV_INT32 main (NV_INT32 argc, NV_CHAR *argv[])
{
  NV_INT32 rows, cols;
  NV_FLOAT64 lat_bounds[2], lon_bounds[2], lat_space, lon_space;
  NV_CHAR dirfil[512], wvsdir[256];
  NV_U_BYTE zero = 0;
  NV_INT32 i, j, endian, swap = 0, size;
  NV_FLOAT32 grid_min, wlon, elon, slat, nlat, tmp;
  FILE *fp, *ofp;
  NV_BOOL hit = NVFalse;
  static NV_CHAR file[14][20] = {"g2003u01.bin", "g2003u02.bin", "g2003u03.bin", "g2003u04.bin",
                                 "g2003u05.bin", "g2003u06.bin", "g2003u07b.bin", "g2003u08.bin",
                                 "g2003a01.bin", "g2003a02.bin", "g2003a03.bin", "g2003a04.bin",
                                 "g2003h01.bin", "g2003p01.bin"};


  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s GEOID03_FILE_NAME\n", argv[0]);
      fprintf (stderr, "Where GEOID03_FILE_NAME is one of the following:\n");
      fprintf (stderr, "g2003u01.bin, g2003u02.bin, g2003u03.bin, g2003u04.bin,\n");
      fprintf (stderr, "g2003u05.bin, g2003u06.bin, g2003u07b.bin, g2003u08.bin,\n");
      fprintf (stderr, "g2003a01.bin, g2003a02.bin, g2003a03.bin, g2003a04.bin,\n");
      fprintf (stderr, "g2003h01.bin, or g2003p01.bin\n\n");
      exit (-1);
    }


  for (i = 0 ; i < 14 ; i++)
    {
      if (!strcmp (argv[1], file[i]))
	{
	  hit = NVTrue;
	  break;
	}
    }


  if (!hit)
    {
      fprintf (stderr, "Usage: %s GEOID03_FILE_NAME\n", argv[0]);
      fprintf (stderr, "Where GEOID03_FILE_NAME is one of the following:\n");
      fprintf (stderr, "g2003u01.bin, g2003u02.bin, g2003u03.bin, g2003u04.bin,\n");
      fprintf (stderr, "g2003u05.bin, g2003u06.bin, g2003u07b.bin, g2003u08.bin,\n");
      fprintf (stderr, "g2003a01.bin, g2003a02.bin, g2003a03.bin, g2003a04.bin,\n");
      fprintf (stderr, "g2003h01.bin, or g2003p01.bin\n\n");
      exit (-1);
    }


  /*  Use the environment variable WVS_DIR to get the directory name.   */

  if (getenv ("WVS_DIR") == NULL)
    {
      fprintf (stderr, "\n\nEnvironment variable WVS_DIR is not set\n\n");
      fflush (stderr);
      exit (-1);
    }
    
  strcpy (wvsdir, getenv ("WVS_DIR"));
  if (wvsdir[0] == 0)
    {
      fprintf (stderr, "\n\nEnvironment variable WVS_DIR is not set\n\n");
      fflush (stderr);
      exit (-1);
    }


  sprintf (dirfil, "%s%1c%s", wvsdir, (NV_CHAR) SEPARATOR, argv[1]);

  if ((fp = fopen (dirfil, "rb")) == NULL)
    {
      perror (dirfil);
      exit (-1);
    }

  fread (&lat_bounds[0], sizeof (NV_FLOAT64), 1, fp);
  fread (&lon_bounds[0], sizeof (NV_FLOAT64), 1, fp);
  fread (&lat_space, sizeof (NV_FLOAT64), 1, fp);
  fread (&lon_space, sizeof (NV_FLOAT64), 1, fp);
  fread (&rows, sizeof (NV_INT32), 1, fp);
  fread (&cols, sizeof (NV_INT32), 1, fp);
  fread (&endian, sizeof (NV_INT32), 1, fp);


  if ((big_endian () && endian) || (!big_endian () && !endian))
    {
      swap = 1;

      swap_double (&lat_bounds[0]);
      swap_double (&lon_bounds[0]);
      swap_double (&lat_space);
      swap_double (&lon_space);
      swap_int (&rows);
      swap_int (&cols);
    }
  else
    {
      swap = 0;
    }


  lat_bounds[1] = lat_bounds[0] + lat_space * rows;

  lon_bounds[1] = lon_bounds[0] + lon_space * cols;


  size = rows * cols * sizeof (NV_FLOAT32);

  array = (NV_FLOAT32 *) calloc (1, size);

  if (array == NULL)
    {
      perror ("Allocating geoid array memory in get_geoid03");
      exit (-1);
    }


  /*  Read the whole stinkin' file as one big block.  */

  fread (array, size, 1, fp);


  /*  Swap if needed  */

  if (swap)
    {
      for (i = 0 ; i < rows ; i++)
	{
	  for (j = 0 ; j < cols ; j++)
	    {
	      swap_float (&array[i * cols + j]);
	    }
	}
    }

  fclose (fp);


  /*  Write a CHRTR header to the file.  */

  sprintf (dirfil, "%s.fin", argv[1]);

  if ((ofp = fopen (dirfil, "wb")) == NULL)
    {
      perror (dirfil);
      exit (-1);
    }


  wlon = lon_bounds[0] - 360.0;
  elon = lon_bounds[1] - 360.0;
  slat = lat_bounds[0];
  nlat = lat_bounds[1];

  grid_min = lat_space * 60.0;
  fwrite (&wlon, sizeof (NV_FLOAT32), 1, ofp);
  fwrite (&elon, sizeof (NV_FLOAT32), 1, ofp);
  fwrite (&slat, sizeof (NV_FLOAT32), 1, ofp);
  fwrite (&nlat, sizeof (NV_FLOAT32), 1, ofp);
  fwrite (&grid_min, sizeof (NV_FLOAT32), 1, ofp);
  fwrite (&cols, sizeof (NV_INT32), 1, ofp);
  fwrite (&rows, sizeof (NV_INT32), 1, ofp);
  i = 0x00010203;
  fwrite (&i, sizeof (NV_INT32), 1, ofp);

  j = ftell (ofp);

  fprintf(stderr,"%f %f %f %f %f %f %d %d %d %f %d\n",wlon,elon,slat,nlat,lat_space,lon_space,rows,cols,swap,grid_min,j);
  for (i = j ; i < cols * sizeof (NV_FLOAT32) ; i++)
    {
      fwrite (&zero, 1, 1, ofp);
    }


  /*  Finally we get down to cases...  */

  for (i = 0 ; i < rows ; i++)
    {
      for (j = 0 ; j < cols ; j++)
	{
	  tmp = -array[i * cols + j];
	  /*fprintf(stderr,"%d %d %f %d\n",i, j, array[i * cols + j],ftell (ofp));*/
	  fwrite (&tmp, 1, sizeof (NV_FLOAT32), ofp);
	}
    }

  fclose (ofp);

  return (0);
}

#endif
