#include "pfm_charts_image.h"

NV_BOOL misp (NV_INT32 pfm_handle, NV_FLOAT64 bin_size_meters, NV_CHAR *dem_file, NV_FLOAT64 min_ne_x,
              NV_FLOAT64 min_ne_y, NV_FLOAT64 max_ne_x, NV_FLOAT64 max_ne_y, 
              NV_INT32 pfm_start_x, NV_INT32 pfm_start_y, NV_INT32 pfm_width,
              NV_INT32 pfm_height, NV_FLOAT64 *zero_x, NV_FLOAT64 *zero_y, NV_INT32 zero_count,
              LIST_NUM *list, NV_FLOAT32 datum_offset, NV_BOOL flip, NV_BOOL srtm, projPJ pj_utm, projPJ pj_latlon, NV_INT32 zone)
{
  NV_BOOL             first = NVTrue, no_datum_shift = NVFalse;
  TOPO_OUTPUT_T       tof;
  HYDRO_OUTPUT_T      hof;
  NV_F64_COORD3       xyz;
  NV_I32_COORD2       coord;
  NV_FLOAT64          x, y;
  BIN_RECORD          bin;
  DEPTH_RECORD        *depth;
  NV_INT32            i, j, k, recnum, gridcols, gridrows, percent = 0, old_percent = -1, minnex, minney, maxnex,
                      maxney;
  NV_FLOAT32          *array;
  NV_F64_XYMBR        new_mbr;
  NV_INT16            srtm_data;
 

  OGRSpatialReferenceH ref;
  GDALDatasetH        df;
  char                *wkt = NULL;
  GDALRasterBandH     bd;
  NV_FLOAT64          trans[6];
  GDALDriverH         gt;
  NV_CHAR             **gdal_options = NULL;


  minnex = (NV_INT32) (min_ne_x - (NV_FLOAT64) FILTER * bin_size_meters);
  minney = (NV_INT32) (min_ne_y - (NV_FLOAT64) FILTER * bin_size_meters);

  min_ne_x = (NV_FLOAT64) minnex;
  min_ne_y = (NV_FLOAT64) minney;

  maxnex = (NV_INT32) (max_ne_x + (NV_FLOAT64) FILTER * bin_size_meters + 1.0);
  maxney = (NV_INT32) (max_ne_y + (NV_FLOAT64) FILTER * bin_size_meters + 1.0);

  gridcols = (NV_INT32) ((maxnex - minnex) / bin_size_meters) + 1;
  gridrows = (NV_INT32) ((maxney - minney) / bin_size_meters) + 1;

  max_ne_x = min_ne_x + gridcols * bin_size_meters;
  max_ne_y = min_ne_y + gridrows * bin_size_meters;


  /*  We're going to let MISP handle everything in zero based units of the bin size.  That is, we subtract off the
      west lon from longitudes then divide by the grid size in the X direction.  We do the same with the latitude using
      the south latitude.  This will give us values that range from 0.0 to gridcols in longitude and 0.0 to gridrows
      in latitude.  */

  new_mbr.min_x = 0.0;
  new_mbr.min_y = 0.0;
  new_mbr.max_x = (NV_FLOAT64) gridcols;
  new_mbr.max_y = (NV_FLOAT64) gridrows;


  /*  Initialize the MISP engine.  */

  misp_init (1.0, 1.0, 0.05, 4, 20.0, 20, 999999.0, -999999.0, -2, new_mbr);


  fprintf (stderr, "\n\nReading data for DEM\n");
  fflush (stderr);


  /*  double loop over  height & width of area  */

  for (i = pfm_start_y ; i < pfm_start_y + pfm_height ; i++)
    {
      coord.y = i;
      for (j = pfm_start_x ; j < pfm_start_x + pfm_width ; j++)
        {
          coord.x = j;

          read_bin_record_index (pfm_handle, coord, &bin);


          /*  Force zero value in optional "zero" area.  */

          if (zero_count && inside (zero_x, zero_y, zero_count, bin.xy.x, bin.xy.y))
            {
              x = bin.xy.x * NV_DEG_TO_RAD;
              y = bin.xy.y * NV_DEG_TO_RAD;
              pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

              xyz.x = (x - min_ne_x) / bin_size_meters;
              xyz.y = (y - min_ne_y) / bin_size_meters;

              xyz.z = 0.0;

              misp_load (xyz);
            }
          else
            {
              /*  We'll take anything that has data, valid or otherwise, since the pictures will be OK even if the data isn't. */

              if (bin.num_soundings)
                {
                  /*  Get file numbers and min and max record numbers in file so we can figure out which   */
                  /*  images to retrieve.  */

                  if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                    {
                      for (k = 0 ; k < recnum ; k++)
                        {
                          if (!(depth[k].validity & PFM_DELETED))
                            {

                              /*  The first time through we have to find out if the data has been corrected to local datum.
                                  This is for later use if we wish to add SRTM data to the DEM.*/

                              if (first && srtm && !(depth[k].validity & PFM_INVAL))
                                {
                                  NV_INT16 type;
                                  NV_CHAR path[512];
                                  FILE *data_fp;
 
                                  read_list_file (pfm_handle, depth[k].file_number, path, &type);

                                  if (type == PFM_SHOALS_TOF_DATA)
                                    {
                                      if ((data_fp = open_tof_file (path)) == NULL)
                                        {
                                          perror (path);
                                          exit (-1);
                                        }

                                      tof_read_record (data_fp, depth[k].ping_number, &tof);

                                      if (tof.result_elevation_last == tof.elevation_last) no_datum_shift = NVTrue;
                                    }
                                  else
                                    {
                                      if ((data_fp = open_hof_file (path)) == NULL)
                                        {
                                          perror (path);
                                          exit (-1);
                                        }


                                      hof_read_record (data_fp, depth[k].ping_number, &hof);

                                      if (hof.kgps_datum == 0.0) no_datum_shift = NVTrue;
                                    }

                                  fclose (data_fp);


                                  first = NVFalse;
                                }


                              NV_INT32 m = depth[k].file_number;

                              if (m > 10000)
                                {
                                  fprintf (stderr, "\n\nFile number out of bounds - %d\n\n", depth[k].file_number);
                                  exit (-1);
                                }
                              list[m].hit = NVTrue;
                              if (depth[k].ping_number < list[m].start) list[m].start = depth[k].ping_number;
                              if (depth[k].ping_number > list[m].end) list[m].end = depth[k].ping_number;


                              if (!(depth[k].validity & (PFM_INVAL | PFM_REFERENCE)))
                                {
                                  x = depth[k].xyz.x * NV_DEG_TO_RAD;
                                  y = depth[k].xyz.y * NV_DEG_TO_RAD;
                                  pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                                  xyz.x = (x - min_ne_x) / bin_size_meters;
                                  xyz.y = (y - min_ne_y) / bin_size_meters;

                                  xyz.z = depth[k].xyz.z;

                                  if (flip) xyz.z = -xyz.z;


                                  /*  Zero out water areas.  */

                                  if (flip)
                                    {
                                      xyz.z -= datum_offset;

                                      if (xyz.z < 0.0) xyz.z = 0.0;
                                    }
                                  else
                                    {
                                      xyz.z += datum_offset;

                                      if (xyz.z > 0.0) xyz.z = 0.0;
                                    }

                                  misp_load (xyz);
                                }
                            }
                        }

                      free (depth);
                    }
                }
              else
                {
                  if (srtm)
                    {
                      srtm_data = read_srtm_topo (bin.xy.y, bin.xy.x);

                      if (srtm_data > -32768)
                        {
                          xyz.z = (NV_FLOAT64) srtm_data;


                          /*  This should switch the SRTM data back to ellipsoid heights.  */
                          if (no_datum_shift)
                            {
                              NV_FLOAT32 adjust = get_egm08 (bin.xy.y, bin.xy.x);


                              /* Adjusting the SRTM up or down depending upon the flip option  */

                              if (flip)
                                {
                                  xyz.z -= (NV_FLOAT64) adjust;
                                }
                              else
                                {
                                  xyz.z += (NV_FLOAT64) adjust;
                                }
                            }
                          
                          xyz.z -= datum_offset;

                          if (xyz.z < 0.0) xyz.z = 0.0;

                          x = bin.xy.x * NV_DEG_TO_RAD;
                          y = bin.xy.y * NV_DEG_TO_RAD;
                          pj_transform (pj_latlon, pj_utm, 1, 1, &x, &y, NULL);

                          xyz.x = (x - min_ne_x) / bin_size_meters;
                          xyz.y = (y - min_ne_y) / bin_size_meters;

                          misp_load (xyz);
                        }
                    }
                }
            }
        }

      if (percent != old_percent)
        {
          percent = NINT (((NV_FLOAT32) (i - pfm_start_y) / (NV_FLOAT32) pfm_height) * 100.0);
          fprintf (stderr, "%03d%% read\r", percent);
          fflush (stderr);
          old_percent = percent;
        }
    }

  fprintf (stderr, "100%% read\n");


  fprintf (stderr, "\n\nGenerating DEM\n\n");
  fflush (stderr);


  if (misp_proc ()) return (NVTrue);


  /*  Allocating one more than gridcols due to constraints of old chrtr (see comments in misp_funcs.c)  */

  array = (NV_FLOAT32 *) malloc ((gridcols + 1) * sizeof (NV_FLOAT32));

  if (array == NULL)
    {
      perror ("Allocating array");
      exit (-1);
    }


  gdal_options = CSLSetNameValue (gdal_options, "COMPRESS", "NONE");


  /*  Set up the output GeoTIFF file.  */

  GDALAllRegister ();

  gt = GDALGetDriverByName ("GTiff");
  if (gt == NULL)
    {
      fprintf (stderr, "Could not get GTiff driver\n");
      exit (-1);
    }

  df = GDALCreate (gt, dem_file, gridcols, gridrows, 1, GDT_Float32, gdal_options);
  if (df == NULL)
    {
      fprintf (stderr, "Could not create %s\n", dem_file);
      exit (-1);
    }

  trans[0] = min_ne_x;
  trans[1] = bin_size_meters;
  trans[2] = 0.0;
  trans[3] = max_ne_y;
  trans[4] = 0.0;
  trans[5] = -bin_size_meters;
  GDALSetGeoTransform (df, trans);
  ref = OSRNewSpatialReference (NULL);
  OSRSetUTM (ref, zone, TRUE);
  OSRSetWellKnownGeogCS (ref, "WGS84");
  OSRExportToWkt (ref, &wkt);
  GDALSetProjection (df, wkt);
  CPLFree (wkt);
  bd = GDALGetRasterBand (df, 1);


  fprintf (stderr, "\n\nRetrieving DEM\n");
  fflush (stderr);

  old_percent = -1;

  for (i = 0 ; i < gridrows ; i++)
    {
      if (!misp_rtrv (array)) break;

      GDALRasterIO (bd, GF_Write, 0, (gridrows - 1) - i, gridcols, 1, array, gridcols, 1, GDT_Float32, 0, 0);

      if (percent != old_percent)
        {
          percent = NINT (((NV_FLOAT32) i / (NV_FLOAT32) pfm_height) * 100.0);
          fprintf (stderr, "%03d%% retrieved\r", percent);
          fflush (stderr);
          old_percent = percent;
        }
    }
  fprintf (stderr, "100%% retrieved\n");
  fflush (stderr);


  GDALClose (df);


  free (array);

  return (NVFalse);
}
