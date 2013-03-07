#include "areaCheck.hpp"
#include "libshp/shapefil.h"


#define PLOT_INCREMENT  400


void overlayData (nvMap *map, MISC *misc, OPTIONS *options, NV_BOOL raster)
{
  FILE          *fp;
  NV_CHAR       string[256], tmp[256], latstring[50], lonstring[50], valstring[50], lat_hemi = 'N', lon_hemi = 'E', flg;
  NV_FLOAT64    lat[4], lon[4], dep, minBounds[4], maxBounds[4], sum_x, sum_y;
  NV_INT32      lat_deg = 0, lat_min = 0, lon_deg = 0, lon_min = 0, file, ping, x, y, z, type, numShapes, numParts, 
                clipped = 0, count, plot_count = 0;;
  NV_FLOAT32    lat_sec = 0.0, lon_sec = 0.0, min_val, max_val, val;
  NV_BOOL       first = NVFalse, minmax = NVFalse, nathan_hale = NVFalse;
  QString       depth, flightline;
  SHPHandle     shpHandle;
  SHPObject     *shape = NULL;


  void geotiff (NV_INT32 file_number, nvMap *map, MISC *misc, OPTIONS *options);
  void chrtr (NV_INT32 file_number, nvMap *map, MISC *misc, OPTIONS *options);
  void chrtr2 (NV_INT32 file_number, nvMap *map, MISC *misc, OPTIONS *options);
  void bag (NV_INT32 file_number, nvMap *map, MISC *misc, OPTIONS *options);


  //  If we're drawing more than 5 degrees plot a Nathan Hale at the location so we can find the 
  //  damn thing.  (Nathan Hale - I only regret that I have but one .:asterisk:. for my country).

  nathan_hale = NVFalse;
  if (misc->displayed_area.max_x - misc->displayed_area.min_x > 5.0 ||
      misc->displayed_area.max_y - misc->displayed_area.min_y > 5.0) nathan_hale = NVTrue;


  //  If raster is set we are plotting CHRTR, GeoTIFF, and BAG files prior to drawing coastlines.

  if (raster)
    {
      for (NV_INT32 k = GEOTIFF ; k < GEOTIFF + 3 ; k++)
        {
          for (NV_INT32 i = 0 ; i < misc->num_overlays[k] ; i++)
            {
              if (misc->overlays[k][i].active)
                {
                  //  If we're drawing more than 5 degrees plot an asterisk (Nathan Hale) at the location so we can find the 
                  //  damn thing.

                  if (nathan_hale)
                    {
                      NV_FLOAT64 dep;
                      NV_INT32 x, y, z;
                      map->map_to_screen (1, &misc->overlays[k][i].center.x, &misc->overlays[k][i].center.y, &dep, &x, &y, &z);
                      map->drawLine (x - 3, y - 3, x + 3, y + 3, misc->overlays[k][i].color, 2, NVFalse, Qt::SolidLine);
                      map->drawLine (x - 3, y + 3, x + 3, y - 3, misc->overlays[k][i].color, 2, NVFalse, Qt::SolidLine);
                      map->drawLine (x - 6, y, x + 6, y, misc->overlays[k][i].color, 2, NVFalse, Qt::SolidLine);
                      map->drawLine (x, y - 6, x, y + 6, misc->overlays[k][i].color, 2, NVFalse, Qt::SolidLine);
                    }


                  //  Only draw the file if we're displaying less than 10 degrees.

                  if (misc->displayed_area.max_x - misc->displayed_area.min_x < 10.0 && 
                      misc->displayed_area.max_y - misc->displayed_area.min_y < 10.0)
                    {
                      switch (k)
                        {
                        case GEOTIFF:
                          geotiff (i, map, misc, options);
                          break;

                        case CHRTR:
                          if (misc->overlays[k][i].chrtr2)
                            {
                              chrtr2 (i, map, misc, options);
                            }
                          else
                            {
                              chrtr (i, map, misc, options);
                            }
                          break;

                        case BAG:
                          bag (i, map, misc, options);
                          break;
                        }
                    }
                }
            }


          //  Check for drawing canceled.

          qApp->processEvents ();
          if (misc->drawing_canceled) break;
        }


      if (misc->drawing_canceled) return;
    }
  else
    {
      //  Plot the other stuff (non-raster).

      for (NV_INT32 k = 0 ; k < NUM_TYPES ; k++)
        {
          if (k != GEOTIFF && k != CHRTR && k != BAG)
            {
              for (NV_INT32 i = 0 ; i < misc->num_overlays[k] ; i++)
                {
                  if (misc->overlays[k][i].active)
                    {
                      sum_x = 0.0;
                      sum_y = 0.0;
                      count = 0;


                      //  This is sort of a waste for .shp, .llz, or .xml files but it does check whether the file
                      //  exists.  We'll close it later anyway.

                      if ((fp = fopen (misc->overlays[k][i].filename, "r")) != NULL)
                        {
                          switch (k)
                            {

                              //  ESRI Polygon shape files.  Note: We always read these from the file, they don't get saved

                            case SHAPE:

                              fclose (fp);


                              //  Open shape file

                              shpHandle = SHPOpen (misc->overlays[k][i].filename, "rb");

                              if (shpHandle != NULL)
                                {
                                  //  Get shape file header info

                                  SHPGetInfo (shpHandle, &numShapes, &type, minBounds, maxBounds);


                                  //  Read all shapes

                                  misc->overlays[k][i].count = 0;
                                  for (NV_INT32 n = 0 ; n < numShapes ; n++)
                                    {
                                      shape = SHPReadObject (shpHandle, n);


                                      //  Get all vertices

                                      if (shape->nVertices >= 2)
                                        {
                                          misc->overlays[k][i].count = 0;
                                          numParts = 1;
                                          for (NV_INT32 j = 0 ; j < shape->nVertices ; j++)
                                            {
                                              if (!j || (numParts < shape->nParts && shape->panPartStart[numParts] == j))
                                                {
                                                  if (j)
                                                    {
                                                      //  Clip polygon to the minimum bounding rectangle

                                                      for (NV_INT32 m = 0 ; m < misc->overlays[k][i].count ; m++)
                                                        {
                                                          if (misc->overlays[k][i].x[m] < misc->displayed_area.min_x)
                                                            misc->overlays[k][i].x[m] = misc->displayed_area.min_x;
                                                          if (misc->overlays[k][i].x[m] > misc->displayed_area.max_x)
                                                            misc->overlays[k][i].x[m] = misc->displayed_area.max_x;
                                                          if (misc->overlays[k][i].y[m] < misc->displayed_area.min_y)
                                                            misc->overlays[k][i].y[m] = misc->displayed_area.min_y;
                                                          if (misc->overlays[k][i].y[m] > misc->displayed_area.max_y)
                                                            misc->overlays[k][i].y[m] = misc->displayed_area.max_y;
                                                        }

                                                      map->fillPolygon (misc->overlays[k][i].count, misc->overlays[k][i].x, misc->overlays[k][i].y,
                                                                        misc->overlays[k][i].color, NVFalse);
                                                    }

                                                  misc->overlays[k][i].count = 0;

                                                  numParts++;
                                                }


                                              lon[1] = shape->padfX[j];
                                              lat[1] = shape->padfY[j];

                                              misc->overlays[k][i].x = (NV_FLOAT64 *) realloc (misc->overlays[k][i].x, 
                                                                                               sizeof (NV_FLOAT64) * 
                                                                                               (misc->overlays[k][i].count + 1));
                                              misc->overlays[k][i].y = (NV_FLOAT64 *) realloc (misc->overlays[k][i].y,
                                                                                               sizeof (NV_FLOAT64) * 
                                                                                               (misc->overlays[k][i].count + 1));
                                              if (misc->overlays[k][i].y == NULL) 
                                                {
                                                  perror (areaCheck::tr ("Allocating vertex memory").toAscii ());
                                                  exit (-1);
                                                }

                                              misc->overlays[k][i].x[misc->overlays[k][i].count] = lon[1];
                                              misc->overlays[k][i].y[misc->overlays[k][i].count] = lat[1];
                                              misc->overlays[k][i].count++;

                                              sum_x += lon[1];
                                              sum_y += lat[1];
                                              count++;
                                            }


                                          //  Clip polygon to the minimum bounding rectangle

                                          for (NV_INT32 m = 0 ; m < misc->overlays[k][i].count ; m++)
                                            {
                                              if (misc->overlays[k][i].x[m] < misc->displayed_area.min_x)
                                                misc->overlays[k][i].x[m] = misc->displayed_area.min_x;
                                              if (misc->overlays[k][i].x[m] > misc->displayed_area.max_x)
                                                misc->overlays[k][i].x[m] = misc->displayed_area.max_x;
                                              if (misc->overlays[k][i].y[m] < misc->displayed_area.min_y)
                                                misc->overlays[k][i].y[m] = misc->displayed_area.min_y;
                                              if (misc->overlays[k][i].y[m] > misc->displayed_area.max_y)
                                                misc->overlays[k][i].y[m] = misc->displayed_area.max_y;
                                            }

                                          map->fillPolygon (misc->overlays[k][i].count, misc->overlays[k][i].x, misc->overlays[k][i].y,
                                                            misc->overlays[k][i].color, NVTrue);
                                        }
                                      SHPDestroyObject (shape);


                                      //  Check for drawing canceled.

                                      qApp->processEvents ();
                                      if (misc->drawing_canceled) break;
                                    }
                                  SHPClose (shpHandle);
                                }

                              if (misc->overlays[k][i].x != NULL)
                                {
                                  free (misc->overlays[k][i].x);
                                  misc->overlays[k][i].x = NULL;
                                  free (misc->overlays[k][i].y);
                                  misc->overlays[k][i].y = NULL;
                                }

                              break;


                              //  ISS60 (*.ARE), generic (*.are), and Army Corps (*.afs) area files plus ISS60 (*.zne) and
                              //  generic (*.tdz) zone files

                            case ISS60_AREA:
                            case ISS60_ZONE:
                            case GENERIC_AREA:
                            case GENERIC_ZONE:
                            case ACE_AREA:
                              if (!misc->overlays[k][i].count)
                                {
                                  //  ISS60

                                  if (k == ISS60_AREA || k == ISS60_ZONE)
                                    {
                                      while (fgets (string, sizeof (string), fp) != NULL)
                                        {
                                          if (!strncmp (string, "POINT=", 6))
                                            {
                                              strtok (string, ";");
                                              strcpy (tmp, strtok (NULL, ";"));
                                              posfix (tmp, &lat[1], POS_LAT);
                                              strcpy (tmp, strtok (NULL, ";"));
                                              posfix (tmp, &lon[1], POS_LON);

                                              misc->overlays[k][i].x = (NV_FLOAT64 *) realloc (misc->overlays[k][i].x, sizeof (NV_FLOAT64) * 
                                                                                               (misc->overlays[k][i].count + 1));
                                              misc->overlays[k][i].y = (NV_FLOAT64 *) realloc (misc->overlays[k][i].y, sizeof (NV_FLOAT64) * 
                                                                                               (misc->overlays[k][i].count + 1));
                                              if (misc->overlays[k][i].y == NULL) 
                                                {
                                                  perror (areaCheck::tr ("Allocating vertex memory").toAscii ());
                                                  exit (-1);
                                                }


                                              sum_x += lon[1];
                                              sum_y += lat[1];
                                              count++;


                                              misc->overlays[k][i].x[misc->overlays[k][i].count] = lon[1];
                                              misc->overlays[k][i].y[misc->overlays[k][i].count] = lat[1];
                                              misc->overlays[k][i].count++;
                                            }
                                        }
                                    }


                                  //  Generic

                                  else if (k == GENERIC_AREA || k == GENERIC_ZONE)
                                    {
                                      while (fget_coord (fp, &lat_hemi, &lat_deg, &lat_min, &lat_sec, &lon_hemi, &lon_deg, &lon_min,
                                                         &lon_sec))
                                        {
                                          lat[1] = (NV_FLOAT64) lat_deg + (NV_FLOAT64) lat_min / 60.0 + (NV_FLOAT64) lat_sec / 3600.0;
                                          if (lat_hemi == 'S') lat[1] = -lat[1];

                                          lon[1] = (NV_FLOAT64) lon_deg + (NV_FLOAT64) lon_min / 60.0 + (NV_FLOAT64) lon_sec / 3600.0;
                                          if (lon_hemi == 'W') lon[1] = -lon[1];


                                          misc->overlays[k][i].x = (NV_FLOAT64 *) realloc (misc->overlays[k][i].x, sizeof (NV_FLOAT64) * 
                                                                                           (misc->overlays[k][i].count + 1));
                                          misc->overlays[k][i].y = (NV_FLOAT64 *) realloc (misc->overlays[k][i].y, sizeof (NV_FLOAT64) * 
                                                                                           (misc->overlays[k][i].count + 1));
                                          if (misc->overlays[k][i].y == NULL) 
                                            {
                                              perror (areaCheck::tr ("Allocating vertex memory").toAscii ());
                                              exit (-1);
                                            }


                                          sum_x += lon[1];
                                          sum_y += lat[1];
                                          count++;


                                          misc->overlays[k][i].x[misc->overlays[k][i].count] = lon[1];
                                          misc->overlays[k][i].y[misc->overlays[k][i].count] = lat[1];
                                          misc->overlays[k][i].count++;
                                        }
                                    }


                                  //  Army Corps

                                  else
                                    {
                                      while (fgets (string, sizeof (string), fp) != NULL)
                                        {
                                          if (strchr (string, ','))
                                            {
                                              sscanf (string, "%lf,%lf", &lon[1], &lat[1]);
                                            }
                                          else
                                            {
                                              sscanf (string, "%lf %lf", &lon[1], &lat[1]);
                                            }

                                          misc->overlays[k][i].x = (NV_FLOAT64 *) realloc (misc->overlays[k][i].x, sizeof (NV_FLOAT64) * 
                                                                                           (misc->overlays[k][i].count + 1));
                                          misc->overlays[k][i].y = (NV_FLOAT64 *) realloc (misc->overlays[k][i].y, sizeof (NV_FLOAT64) * 
                                                                                           (misc->overlays[k][i].count + 1));
                                          if (misc->overlays[k][i].y == NULL) 
                                            {
                                              perror (areaCheck::tr ("Allocating vertex memory").toAscii ());
                                              exit (-1);
                                            }


                                          sum_x += lon[1];
                                          sum_y += lat[1];
                                          count++;


                                          misc->overlays[k][i].x[misc->overlays[k][i].count] = lon[1];
                                          misc->overlays[k][i].y[misc->overlays[k][i].count] = lat[1];
                                          misc->overlays[k][i].count++;
                                        }
                                    }


                                  //  Close the polygon/rectangle

                                  if (misc->overlays[k][i].x[misc->overlays[k][i].count - 1] != misc->overlays[k][i].x[0] ||
                                      misc->overlays[k][i].y[misc->overlays[k][i].count - 1] != misc->overlays[k][i].y[0])
                                    {
                                      misc->overlays[k][i].x = (NV_FLOAT64 *) realloc (misc->overlays[k][i].x, sizeof (NV_FLOAT64) * 
                                                                                       (misc->overlays[k][i].count + 1));
                                      misc->overlays[k][i].y = (NV_FLOAT64 *) realloc (misc->overlays[k][i].y, sizeof (NV_FLOAT64) * 
                                                                                       (misc->overlays[k][i].count + 1));
                                      if (misc->overlays[k][i].y == NULL) 
                                        {
                                          perror (areaCheck::tr ("Allocating vertex memory").toAscii ());
                                          exit (-1);
                                        }

                                      misc->overlays[k][i].x[misc->overlays[k][i].count] = misc->overlays[k][i].x[0];
                                      misc->overlays[k][i].y[misc->overlays[k][i].count] = misc->overlays[k][i].y[0];
                                      misc->overlays[k][i].count++;
                                    }
                                }
                              else
                                {
                                  count = misc->overlays[k][i].count;
                                  sum_x = 0.0;
                                  sum_y = 0.0;
                                  for (NV_INT32 j = 0 ; j < count ; j++)
                                    {
                                      sum_x += misc->overlays[k][i].x[j];
                                      sum_y += misc->overlays[k][i].y[j];
                                    }
                                }


                              //  Clip the polygon line segments to the minimum bounding rectangle

                              for (NV_INT32 j = 1 ; j < misc->overlays[k][i].count ; j++)
                                {
                                  NV_FLOAT64 x[2], y[2];

                                  x[0] = misc->overlays[k][i].x[j - 1];
                                  y[0] = misc->overlays[k][i].y[j - 1];
                                  x[1] = misc->overlays[k][i].x[j];
                                  y[1] = misc->overlays[k][i].y[j];

                                  if (clip (&x[0], &y[0], &x[1], &y[1], misc->displayed_area))
                                    map->drawLine (x[0], y[0], x[1], y[1], misc->overlays[k][i].color, 2, NVFalse, Qt::SolidLine);
                                }

                              fclose (fp);

                              break;


                              //  Decimated trackline (*.trk) files

                            case TRACK:

                              plot_count = 0;
                              while (fgets (string, sizeof (string), fp) != NULL)
                                {
                                  if (!strncmp (string, "FILE", 4) || !strncmp (string, "LEVEL", 5) || !strncmp (string, "MINMAX", 6)) 
                                    {
                                      first = NVTrue;
                                      if (!strncmp (string, "MINMAX", 6))
                                        {
                                          sscanf (string, "MINMAX %f %f", &min_val, &max_val);
                                          minmax = NVTrue;
                                        }
                                      else
                                        {
                                          minmax = NVFalse;
                                        }
                                    }
                                  else
                                    {
                                      sscanf (string, "%1c,%d,%d,%lf,%lf,%f", &flg, &file, &ping, &lat[1], &lon[1], &val);

                                      if (flg == '-') first = NVTrue;

                                      if (!first) 
                                        {
                                          if ((clipped = clip (&lon[0], &lat[0], &lon[1], &lat[1], misc->displayed_area)))
                                            {
                                              if (minmax)
                                                {
                                                  if (options->stoplight)
                                                    {
                                                      if (val < options->minstop)
                                                        {
                                                          map->drawLine (lon[0], lat[0], lon[1], lat[1], Qt::green, 2, NVFalse,
                                                                         Qt::SolidLine);
                                                        }
                                                      else if (val > options->maxstop)
                                                        {
                                                          map->drawLine (lon[0], lat[0], lon[1], lat[1], Qt::red, 2, NVFalse,
                                                                         Qt::SolidLine);
                                                        }
                                                      else
                                                        {
                                                          map->drawLine (lon[0], lat[0], lon[1], lat[1], Qt::yellow, 2, NVFalse,
                                                                         Qt::SolidLine);
                                                        }
                                                    }
                                                  else
                                                    {
                                                      NV_INT32 c_index = NUMSHADES - NINT ((val - min_val) / (max_val - min_val) * 
                                                                                           (NV_FLOAT32) (NUMSHADES - 1));

                                                      map->drawLine (lon[0], lat[0], lon[1], lat[1], misc->color_array[c_index][0], 2, 
                                                                     NVFalse, Qt::SolidLine);
                                                    }
                                                }
                                              else
                                                {
                                                  map->drawLine (lon[0], lat[0], lon[1], lat[1], misc->overlays[k][i].color, 2, NVFalse,
                                                                 Qt::SolidLine);
                                                }
                                            }
                                        }
                                      plot_count++;
                                      if (plot_count >= PLOT_INCREMENT)
                                        {
                                          plot_count = 0;
                                          map->flush ();
                                        }


                                      sum_x += lon[1];
                                      sum_y += lat[1];
                                      count++;


                                      lon[0] = lon[1];
                                      lat[0] = lat[1];

                                      first = NVFalse;


                                      //  If the last point went out of the area we want to start a new line.

                                      if (clipped == 3 || clipped == 4) 
                                        {
                                          clipped = 0;
                                          first = NVTrue;
                                        }
                                    }


                                  //  Check for drawing canceled.

                                  qApp->processEvents ();
                                  if (misc->drawing_canceled) break;
                                }

                              fclose (fp);

                              break;


                              //  Optech flightline files

                            case OPTECH_FLT:

                              NV_BOOL start_flt;
                              NV_CHAR id[2];

                              start_flt = NVFalse;

                              while (fgets (string, sizeof (string), fp) != NULL)
                                {
                                  if (!start_flt)
                                    {
                                      if (!strncmp (string, "[FLT_SECTION]", 13)) start_flt = NVTrue;
                                    }
                                  else
                                    {
                                      //  Break out when we hit the next section.

                                      if (string[0] == '[') break;


                                      if (strstr (string, "flightline_number:"))
                                        {
                                          NV_BOOL tagged = NVFalse;

                                          flightline = QString (string).remove ("flightline_number: ").remove ("\n");

                                          fgets (string, sizeof (string), fp);
                                          sscanf (string, "%s %lf %lf", id, &lat[0], &lon[0]);

                                          if (lon[0] >= misc->displayed_area.min_x && lon[0] <= misc->displayed_area.max_x &&
                                              lat[0] >= misc->displayed_area.min_y && lat[0] <= misc->displayed_area.max_y && !nathan_hale)
                                            {
                                              map->drawText (flightline, lon[0], lat[0], misc->overlays[k][i].color, NVFalse);
                                              tagged = NVTrue;

                                              map->fillCircle (lon[0], lat[0], 8, 0.0, 360.0, Qt::red, NVFalse);
                                            }

                                          sum_x += lon[0];
                                          sum_y += lat[0];
                                          count++;

                                          fgets (string, sizeof (string), fp);
                                          sscanf (string, "%s %lf %lf", id, &lat[1], &lon[1]);

                                          if (lon[1] >= misc->displayed_area.min_x && lon[1] <= misc->displayed_area.max_x &&
                                              lat[1] >= misc->displayed_area.min_y && lat[1] <= misc->displayed_area.max_y && !nathan_hale)
                                            {
                                              if (!tagged)
                                                {
                                                  map->drawText (flightline, lon[1], lat[1], misc->overlays[k][i].color, NVFalse);
                                                  tagged = NVTrue;
                                                }

                                              map->fillCircle (lon[1], lat[1], 8, 0.0, 360.0, Qt::red, NVFalse);
                                            }

                                          if ((clipped = clip (&lon[0], &lat[0], &lon[1], &lat[1], misc->displayed_area)))
                                            {
                                              if (!nathan_hale) map->drawLine (lon[0], lat[0], lon[1], lat[1], misc->overlays[k][i].color,
                                                                               2, NVFalse, Qt::SolidLine);
                                            }

                                          sum_x += lon[1];
                                          sum_y += lat[1];
                                          count++;

                                          lat[0] = lat[1];
                                          lon[0] = lon[1];
                                          fgets (string, sizeof (string), fp);
                                          sscanf (string, "%s %lf %lf", id, &lat[1], &lon[1]);

                                          if (lon[1] >= misc->displayed_area.min_x && lon[1] <= misc->displayed_area.max_x &&
                                              lat[1] >= misc->displayed_area.min_y && lat[1] <= misc->displayed_area.max_y && !nathan_hale)
                                            {
                                              if (!tagged)
                                                {
                                                  map->drawText (flightline, lon[1], lat[1], misc->overlays[k][i].color, NVFalse);
                                                  tagged = NVTrue;
                                                }

                                              map->fillCircle (lon[1], lat[1], 8, 0.0, 360.0, Qt::red, NVFalse);
                                            }

                                          if ((clipped = clip (&lon[0], &lat[0], &lon[1], &lat[1], misc->displayed_area)))
                                            {
                                              if (!nathan_hale) map->drawLine (lon[0], lat[0], lon[1], lat[1], misc->overlays[k][i].color,
                                                                               2, NVFalse, Qt::SolidLine);
                                            }

                                          sum_x += lon[1];
                                          sum_y += lat[1];
                                          count++;

                                          lat[0] = lat[1];
                                          lon[0] = lon[1];
                                          fgets (string, sizeof (string), fp);
                                          sscanf (string, "%s %lf %lf", id, &lat[1], &lon[1]);

                                          if (lon[1] >= misc->displayed_area.min_x && lon[1] <= misc->displayed_area.max_x &&
                                              lat[1] >= misc->displayed_area.min_y && lat[1] <= misc->displayed_area.max_y && !nathan_hale)
                                            {
                                              if (!tagged) map->drawText (flightline, lon[1], lat[1], misc->overlays[k][i].color, NVFalse);

                                              map->fillCircle (lon[1], lat[1], 8, 0.0, 360.0, Qt::red, NVFalse);
                                            }

                                          if ((clipped = clip (&lon[0], &lat[0], &lon[1], &lat[1], misc->displayed_area)))
                                            {
                                              if (!nathan_hale) map->drawLine (lon[0], lat[0], lon[1], lat[1], misc->overlays[k][i].color,
                                                                               2, NVFalse, Qt::SolidLine);
                                            }

                                          sum_x += lon[1];
                                          sum_y += lat[1];
                                          count++;
                                        }
                                    }


                                  //  Check for drawing canceled.

                                  qApp->processEvents ();
                                  if (misc->drawing_canceled) break;
                                }

                              fclose (fp);

                              break;


                              //  Generic yxz data.

                            case YXZ:

                              plot_count = 0;

                              while (fgets (string, sizeof (string), fp) != NULL)
                                {
                                  //  Yes, I know that strtok destroys the input string.  I don't really care in this case.

                                  if (strchr (string, ','))
                                    {
                                      strcpy (latstring, strtok (string, ","));
                                      strcpy (lonstring, strtok (NULL, ","));
                                      strcpy (valstring, strtok (NULL, ","));
                                    }
                                  else
                                    {
                                      strcpy (latstring, strtok (string, " "));
                                      strcpy (lonstring, strtok (NULL, " "));
                                      strcpy (valstring, strtok (NULL, " "));
                                    }

                                  posfix (latstring, &lat[0], POS_LAT);
                                  posfix (lonstring, &lon[0], POS_LON);
                                  sscanf (valstring, "%lf", &dep);

                                  if (lon[0] >= misc->displayed_area.min_x && lon[0] <= misc->displayed_area.max_x &&
                                      lat[0] >= misc->displayed_area.min_y && lat[0] <= misc->displayed_area.max_y)
                                    {
                                      depth.sprintf ("%0.1f", dep);

                                      map->drawLine (x - 4, y, x + 4, y, misc->overlays[k][i].color, 1, NVFalse, Qt::SolidLine);
                                      map->drawLine (x, y - 4, x, y + 4, misc->overlays[k][i].color, 1, NVFalse, Qt::SolidLine);
                                      if (misc->displayed_area.max_x - misc->displayed_area.min_x < 2.0 ||
                                          misc->displayed_area.max_y - misc->displayed_area.min_y < 2.0)
                                        map->drawText (depth, lon[0], lat[0], misc->overlays[k][i].color, NVFalse);
                                    }
                                  plot_count++;
                                  if (plot_count >= PLOT_INCREMENT)
                                    {
                                      plot_count = 0;
                                      map->flush ();
                                    }


                                  sum_x += lon[0];
                                  sum_y += lat[0];
                                  count++;


                                  //  Check for drawing canceled.

                                  qApp->processEvents ();
                                  if (misc->drawing_canceled) break;
                                }
                              fclose (fp);

                              break;


                              //  LLZ data.

                            case LLZ_DATA:

                              fclose (fp);

                              plot_count = 0;

                              LLZ_HEADER llz_header;
                              LLZ_REC llz;
                              NV_INT32 llz_hnd;

                              if ((llz_hnd = open_llz (misc->overlays[k][i].filename, &llz_header)) >= 0)
                                { 
                                  for (NV_INT32 j = 0 ; j < llz_header.number_of_records ; j++)
                                    {
                                      read_llz (llz_hnd, j, &llz);

                                      if (llz.xy.lon >= misc->displayed_area.min_x && llz.xy.lon <= misc->displayed_area.max_x &&
                                          llz.xy.lat >= misc->displayed_area.min_y && llz.xy.lat <= misc->displayed_area.max_y)
                                        {
                                          if (misc->displayed_area.max_x - misc->displayed_area.min_x < 1.0 ||
                                              misc->displayed_area.max_y - misc->displayed_area.min_y < 1.0)
                                            {
                                              depth.sprintf ("%0.1f", llz.depth);

                                              map->drawText (depth, llz.xy.lon, llz.xy.lat, misc->overlays[k][i].color, NVFalse);
                                            }
                                          else
                                            {
                                              dep = (NV_FLOAT64) llz.depth;
                                              map->map_to_screen (1, &llz.xy.lon, &llz.xy.lat, &dep, &x, &y, &z);
                                              map->drawLine (x - 4, y, x + 4, y, misc->overlays[k][i].color, 1, NVFalse, Qt::SolidLine);
                                              map->drawLine (x, y - 4, x, y + 4, misc->overlays[k][i].color, 1, NVFalse, Qt::SolidLine);
                                            }
                                        }
                                      plot_count++;
                                      if (plot_count >= PLOT_INCREMENT)
                                        {
                                          plot_count = 0;
                                          map->flush ();
                                        }

                                      sum_x += llz.xy.lon;
                                      sum_y += llz.xy.lat;
                                      count++;


                                      //  Check for drawing canceled.

                                      qApp->processEvents ();
                                      if (misc->drawing_canceled) break;
                                    }
                                }
                              break;


                              //  Feature (*.bfd) files

                            case FEATURE:

                              fclose (fp);

                              plot_count = 0;

                              NV_INT32 bfd_handle;
                              BFDATA_HEADER bfd_header;

                              if ((bfd_handle = binaryFeatureData_open_file (misc->overlays[k][i].filename, &bfd_header, BFDATA_READONLY)) >= 0)
                                {
                                  BFDATA_SHORT_FEATURE *feature = NULL;
                                  binaryFeatureData_read_all_short_features (bfd_handle, &feature);

                                  for (NV_U_INT32 f = 0 ; f < bfd_header.number_of_records ; f++)
                                    {
                                      if (feature[f].confidence_level)
                                        {
                                          lat[0] = feature[f].latitude;
                                          lon[0] = feature[f].longitude;


                                          sum_x += lon[0];
                                          sum_y += lat[0];
                                          count++;

                                          if (lon[0] >= misc->displayed_area.min_x && lon[0] <= misc->displayed_area.max_x &&
                                              lat[0] >= misc->displayed_area.min_y && lat[0] <= misc->displayed_area.max_y)
                                            {
                                              if (options->display_children || !feature[f].parent_record)
                                                {
                                                  map->map_to_screen (1, &lon[0], &lat[0], &dep, &x, &y, &z);
                                                  if (feature[f].parent_record)
                                                    {
                                                      map->drawCircle (x, y, 6, 0.0, 360.0, 2, misc->overlays[k][i].color, Qt::SolidLine, NVFalse);
                                                    }
                                                  else
                                                    {
                                                      map->drawCircle (x, y, 12, 0.0, 360.0, 2, misc->overlays[k][i].color, Qt::SolidLine, NVFalse);
                                                    }

                                                  if (options->display_feature_info)
                                                    {
                                                      QString string0 (feature[f].description);

                                                      QString string1 (feature[f].remarks);

                                                      QString new_string;
                                                      if (string1.isEmpty ())
                                                        {
                                                          if (!string0.isEmpty ()) new_string = string0;
                                                        }
                                                      else
                                                        {
                                                          if (string0.isEmpty ())
                                                            {
                                                              new_string = string1;
                                                            }
                                                          else
                                                            {
                                                              new_string = string0 + " :: " + string1;
                                                            }
                                                        }

                                                      map->drawText (new_string, x + 6, y + 3, misc->overlays[k][i].color, NVFalse);
                                                    }

                                                  if (options->display_feature_poly && feature[f].poly_count)
                                                    {
                                                      BFDATA_POLYGON bfd_polygon;

                                                      if (binaryFeatureData_read_polygon (bfd_handle, f, &bfd_polygon) >= 0)
                                                        {
                                                          //  Draw a line from the center of the associated feature to the first polygon
                                                          //  point that is in the displayed area so we'll know which feature goes with
                                                          //  which polygon.

                                                          for (NV_U_INT32 j = 0 ; j < feature[f].poly_count ; j++)
                                                            {
                                                              if (bfd_polygon.longitude[j] >= misc->displayed_area.min_x &&
                                                                  bfd_polygon.longitude[j] <= misc->displayed_area.max_x &&
                                                                  bfd_polygon.latitude[j] >= misc->displayed_area.min_y &&
                                                                  bfd_polygon.latitude[j] <= misc->displayed_area.max_y)
                                                                {
                                                                  map->drawLine (lon[0], lat[0], bfd_polygon.longitude[j],
                                                                                 bfd_polygon.latitude[j], misc->overlays[k][i].color, 2,
                                                                                 NVFalse, Qt::DotLine);
                                                                  break;
                                                                }
                                                            }


                                                          NV_FLOAT64 nx[2], ny[2];


                                                          //  Clip the lines.

                                                          for (NV_U_INT32 j = 0 ; j < feature[f].poly_count ; j++)
                                                            {
                                                              //  Don't close polylines (fature[f].poly_type 0), only polygons.

                                                              if (!feature[f].poly_type && j == (feature[f].poly_count - 1)) break;

                                                              NV_U_INT32 m = (j + 1) % feature[f].poly_count;


                                                              //  Don't clip the original points, clip a copy.

                                                              nx[0] = bfd_polygon.longitude[j];
                                                              ny[0] = bfd_polygon.latitude[j];
                                                              nx[1] = bfd_polygon.longitude[m];
                                                              ny[1] = bfd_polygon.latitude[m];


                                                              if (clip (&nx[0], &ny[0], &nx[1], &ny[1], misc->displayed_area))
                                                                {
                                                                  map->drawLine (nx[0], ny[0], nx[1], ny[1], misc->overlays[k][i].color, 2,
                                                                                 NVFalse, Qt::SolidLine);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                      plot_count++;
                                      if (plot_count >= PLOT_INCREMENT)
                                        {
                                          plot_count = 0;
                                          map->flush ();
                                        }


                                      //  Check for drawing canceled.

                                      qApp->processEvents ();
                                      if (misc->drawing_canceled) break;
                                    }
                                  binaryFeatureData_close_file (bfd_handle);
                                }
                              break;
                            }
                        }


                      //  Check for drawing canceled.

                      qApp->processEvents ();
                      if (misc->drawing_canceled) break;


                      //  Compute the center if the data is a bunch of disparate points.

                      if (count)
                        { 
                          misc->overlays[k][i].center.x = sum_x / (NV_FLOAT64) count;
                          misc->overlays[k][i].center.y = sum_y / (NV_FLOAT64) count;
                        }


                      //  If we're drawing more than 5 degrees plot an asterisk (Nathan Hale) at the location so we can find the 
                      //  damn thing.  What?  You didn't get the Nathan Hale reference?  He had but one asterisk for his country.

                      if (nathan_hale)
                        {
                          NV_FLOAT64 dep;
                          NV_INT32 x, y, z;
                          map->map_to_screen (1, &misc->overlays[k][i].center.x, &misc->overlays[k][i].center.y, &dep, &x, &y, &z);
                          map->drawLine (x - 3, y - 3, x + 3, y + 3, misc->overlays[k][i].color, 2, NVFalse, Qt::SolidLine);
                          map->drawLine (x - 3, y + 3, x + 3, y - 3, misc->overlays[k][i].color, 2, NVFalse, Qt::SolidLine);
                          map->drawLine (x - 6, y, x + 6, y, misc->overlays[k][i].color, 2, NVFalse, Qt::SolidLine);
                          map->drawLine (x, y - 6, x, y + 6, misc->overlays[k][i].color, 2, NVFalse, Qt::SolidLine);
                        }
                    }
                }
            }
        }
    }

  map->flush ();
}
