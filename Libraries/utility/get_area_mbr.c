
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



#include "get_area_mbr.h"


/*!  Get an NV_F64_XYMBR from the supplied area file (path) and return in polygon X and Y arrays.  Area file may be .ARE, .are, or .afs.  */

NV_BOOL get_area_mbr (const NV_CHAR *path, NV_INT32 *polygon_count, NV_FLOAT64 *polygon_x, NV_FLOAT64 *polygon_y, NV_F64_XYMBR *mbr)
{
  NV_CHAR                 string[256], tmp[256], lat_hemi, lon_hemi;
  FILE                    *area_fp;
  NV_INT32                i, lat_deg, lat_min, lon_deg, lon_min;
  NV_FLOAT32              lat_sec, lon_sec;
  NV_BOOL                 west = NVFalse, east = NVFalse;


  if ((area_fp = fopen (path, "r")) == NULL)
    {
      perror (path);
      return (NVFalse);
    }


  *polygon_count = 0;


  /*  ISS-60 format area file.  */

  if (!strcmp (&path[strlen (path) - 4], ".ARE"))
    {
      while (fgets (string, sizeof (string), area_fp) != NULL)
        {
          if (!strncmp (string, "POINT=", 6))
            {
              strtok (string, ";");
              strcpy (tmp, strtok (NULL, ";"));
              posfix (tmp, &polygon_y[*polygon_count], POS_LAT);
              strcpy (tmp, strtok (NULL, ";"));
              posfix (tmp, &polygon_x[*polygon_count], POS_LON);

              (*polygon_count)++;
            }
        }
    }


  /*  Polygon list format area file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".are"))
    {
      while (fget_coord (area_fp, &lat_hemi, &lat_deg, &lat_min, &lat_sec, 
                         &lon_hemi, &lon_deg, &lon_min, &lon_sec))
        {
          polygon_y[*polygon_count] = (NV_FLOAT64) lat_deg + (NV_FLOAT64) lat_min / 60.0 + 
            (NV_FLOAT64) lat_sec / 3600.0;
          if (lat_hemi == 'S') polygon_y[*polygon_count] = -polygon_y[*polygon_count];

          polygon_x[*polygon_count] = (NV_FLOAT64) lon_deg + (NV_FLOAT64) lon_min / 60.0 +
            (NV_FLOAT64) lon_sec / 3600.0;
          if (lon_hemi == 'W') polygon_x[*polygon_count] = -polygon_x[*polygon_count];

          (*polygon_count)++;
        }
    }


  /*  Army Corps area file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".afs"))
    {
      while (fgets (string, sizeof (string), area_fp) != NULL)
        {
	  if (strchr (string, ','))
	    {
	      sscanf (string, "%lf,%lf", &polygon_x[*polygon_count], &polygon_y[*polygon_count]);
	    }
	  else
	    {
	      sscanf (string, "%lf %lf", &polygon_x[*polygon_count], &polygon_y[*polygon_count]);
	    }

          (*polygon_count)++;
        }
    }
  else
    {
      fclose (area_fp);
      return (NVFalse);
    }


  fclose (area_fp);


  /*  Check for dateline crossing.  If you're making an area that goes more than half way around the earth
      you're on your own!  */

  for (i = 0 ; i < *polygon_count ; i++)
    {
      if (polygon_x[i] < -90.0) west = NVTrue;
      if (polygon_x[i] > 90.0) east = NVTrue;
    }


  mbr->min_y = 99999999999.0;
  mbr->min_x = 99999999999.0;
  mbr->max_y = -99999999999.0;
  mbr->max_x = -99999999999.0;
    
  for (i = 0 ; i < *polygon_count ; i++)
    {
      if (east && west && polygon_x[i] < 0.0) polygon_x[i] += 360.0;

      if (polygon_y[i] < mbr->min_y) mbr->min_y = polygon_y[i];
      if (polygon_y[i] > mbr->max_y) mbr->max_y = polygon_y[i];
      if (polygon_x[i] < mbr->min_x) mbr->min_x = polygon_x[i];
      if (polygon_x[i] > mbr->max_x) mbr->max_x = polygon_x[i];
    }

  return (NVTrue);
}



/*!  Get an NV_F64_XYMBR from the supplied area file (path) and return in polygon NV_F64_COORD2 array.  Area file may be .ARE, .are, or .afs.  */

NV_BOOL get_area_mbr2 (const NV_CHAR *path, NV_INT32 *polygon_count, NV_F64_COORD2 *polygon, NV_F64_XYMBR *mbr)
{
  NV_CHAR                 string[256], tmp[256], lat_hemi, lon_hemi;
  FILE                    *area_fp;
  NV_INT32                i, lat_deg, lat_min, lon_deg, lon_min;
  NV_FLOAT32              lat_sec, lon_sec;
  NV_BOOL                 west = NVFalse, east = NVFalse;


  if ((area_fp = fopen (path, "r")) == NULL)
    {
      perror (path);
      return (NVFalse);
    }
    

  *polygon_count = 0;


  /*  ISS-60 format area file.  */

  if (!strcmp (&path[strlen (path) - 4], ".ARE"))
    {
      while (fgets (string, sizeof (string), area_fp) != NULL)
        {
          if (!strncmp (string, "POINT=", 6))
            {
              strtok (string, ";");
              strcpy (tmp, strtok (NULL, ";"));
              posfix (tmp, &polygon[*polygon_count].y, POS_LAT);
              strcpy (tmp, strtok (NULL, ";"));
              posfix (tmp, &polygon[*polygon_count].x, POS_LON);

              (*polygon_count)++;
            }
        }
    }


  /*  Polygon list format area file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".are"))
    {
      while (fget_coord (area_fp, &lat_hemi, &lat_deg, &lat_min, &lat_sec, 
                         &lon_hemi, &lon_deg, &lon_min, &lon_sec))
        {
          polygon[*polygon_count].y = (NV_FLOAT64) lat_deg + (NV_FLOAT64) lat_min / 60.0 + 
            (NV_FLOAT64) lat_sec / 3600.0;
          if (lat_hemi == 'S') polygon[*polygon_count].y = -polygon[*polygon_count].y;

          polygon[*polygon_count].x = (NV_FLOAT64) lon_deg + (NV_FLOAT64) lon_min / 60.0 +
            (NV_FLOAT64) lon_sec / 3600.0;
          if (lon_hemi == 'W') polygon[*polygon_count].x = -polygon[*polygon_count].x;

          (*polygon_count)++;
        }
    }


  /*  Army Corps area file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".afs"))
    {
      while (fgets (string, sizeof (string), area_fp) != NULL)
        {
	  if (strchr (string, ','))
	    {
	      sscanf (string, "%lf,%lf", &polygon[*polygon_count].x, &polygon[*polygon_count].y);
	    }
	  else
	    {
	      sscanf (string, "%lf %lf", &polygon[*polygon_count].x, &polygon[*polygon_count].y);
	    }

          (*polygon_count)++;
        }
    }
  else
    {
      fclose (area_fp);
      return (NVFalse);
    }


  fclose (area_fp);


  /*  Check for dateline crossing.  If you're making an area that goes more than half way around the earth
      you're on your own!  */

  for (i = 0 ; i < *polygon_count ; i++)
    {
      if (polygon[i].x < -90.0) west = NVTrue;
      if (polygon[i].x > 90.0) east = NVTrue;
    }


  mbr->min_y = 99999999999.0;
  mbr->min_x = 99999999999.0;
  mbr->max_y = -99999999999.0;
  mbr->max_x = -99999999999.0;
    
  for (i = 0 ; i < *polygon_count ; i++)
    {
      if (east && west && polygon[i].x < 0.0) polygon[i].x += 360.0;

      if (polygon[i].y < mbr->min_y) mbr->min_y = polygon[i].y;
      if (polygon[i].y > mbr->max_y) mbr->max_y = polygon[i].y;
      if (polygon[i].x < mbr->min_x) mbr->min_x = polygon[i].x;
      if (polygon[i].x > mbr->max_x) mbr->max_x = polygon[i].x;
    }

  return (NVTrue);
}



/*  Get an NV_F64_MBR (i.e. lat/lon) instead of an NV_F64_XYMBR.  */

NV_BOOL get_area_ll_mbr (const NV_CHAR *path, NV_INT32 *polygon_count, NV_FLOAT64 *polygon_x, NV_FLOAT64 *polygon_y, NV_F64_MBR *mbr)
{
  NV_F64_XYMBR l_mbr;
  NV_BOOL ret;

  ret = get_area_mbr (path, polygon_count, polygon_x, polygon_y, &l_mbr);

  if (ret)
    {
      mbr->slat = l_mbr.min_y;
      mbr->nlat = l_mbr.max_y;
      mbr->wlon = l_mbr.min_x;
      mbr->elon = l_mbr.max_x;
    }

  return (ret);
}



/*!  Get an NV_F64_MBR from the supplied area file (path) and return in polygon NV_F64_COORD2 array.  Area file may be .ARE, .are, or .afs.  */

NV_BOOL get_area_ll_mbr2 (const NV_CHAR *path, NV_INT32 *polygon_count, NV_F64_COORD2 *polygon, NV_F64_MBR *mbr)
{
  NV_F64_XYMBR l_mbr;
  NV_BOOL ret;

  ret = get_area_mbr2 (path, polygon_count, polygon, &l_mbr);

  if (ret)
    {
      mbr->slat = l_mbr.min_y;
      mbr->nlat = l_mbr.max_y;
      mbr->wlon = l_mbr.min_x;
      mbr->elon = l_mbr.max_x;
    }

  return (ret);
}



/*!  Determine if a polygon is a rectangle.  */

NV_BOOL polygon_is_rectangle (NV_INT32 polygon_count, NV_FLOAT64 *polygon_x, NV_FLOAT64 *polygon_y)
{
  NV_INT32          j, k, lat_count = 0, lon_count = 0;
  NV_FLOAT64        *unique_lat, *unique_lon;
  NV_BOOL           unique;


  unique_lat = (NV_FLOAT64 *) malloc (polygon_count * sizeof (NV_FLOAT64));
  if (unique_lat == NULL)
    {
      perror ("Allocating unique_lat in get_area_mbr.c");
      exit (-1);
    }

  unique_lon = (NV_FLOAT64 *) malloc (polygon_count * sizeof (NV_FLOAT64));
  if (unique_lon == NULL)
    {
      perror ("Allocating unique_lon in get_area_mbr.c");
      exit (-1);
    }


  for (j = 0 ; j < polygon_count ; j++)
    {
      unique = NVTrue;
      for (k = 0 ; k < lat_count ; k++)
        {
          if (polygon_y[j] == unique_lat[k])
            {
              unique = NVFalse;
              break;
            }
        }

      if (unique)
        {
          unique_lat[lat_count] = polygon_y[j];
          lat_count++;
        }


      unique = NVTrue;
      for (k = 0 ; k < lon_count ; k++)
        {
          if (polygon_x[j] == unique_lon[k])
            {
              unique = NVFalse;
              break;
            }
        }

      if (unique)
        {
          unique_lon[lon_count] = polygon_x[j];
          lon_count++;
        }
    }

  free (unique_lat);
  free (unique_lon);


  //  If there are only two unique latitudes and two unique longitudes then this is a rectangle.

  if (lat_count == 2 && lon_count == 2) return (NVTrue);


  return (NVFalse);
}



/*!  Determine if a polygon is a rectangle (NV_F64_COORD2).  */

NV_BOOL polygon_is_rectangle2 (NV_INT32 polygon_count, NV_F64_COORD2 *polygon)
{
  NV_FLOAT64       *polygon_x, *polygon_y;
  NV_INT32         i;
  NV_BOOL          ret;


  polygon_x = (NV_FLOAT64 *) malloc (polygon_count * sizeof (NV_FLOAT64));
  if (polygon_x == NULL)
    {
      perror ("Allocating polygon_x in get_area_mbr.c");
      exit (-1);
    }

  polygon_y = (NV_FLOAT64 *) malloc (polygon_count * sizeof (NV_FLOAT64));
  if (polygon_y == NULL)
    {
      perror ("Allocating polygon_y in get_area_mbr.c");
      exit (-1);
    }


  for (i = 0 ; i < polygon_count ; i++)
    {
      polygon_x[i] = polygon[i].x;
      polygon_y[i] = polygon[i].y;
    }


  ret = polygon_is_rectangle (polygon_count, polygon_x, polygon_y);


  free (polygon_x);
  free (polygon_y);


  return (ret);
}
