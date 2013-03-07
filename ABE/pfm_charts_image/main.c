#include "pfm_charts_image.h"

#include "version.h"


/*****************************************************************************

    Program:    pfm_charts_image

    Purpose:    Extracts image files and plane attitude/position for use in
                building photo mosaics in Ray Seyfarth's mosaic program.

    Programmer: Jan C. Depner

    Date:       10/26/04

*****************************************************************************/


OPTIONS                options;
NV_INT32               zone = 0;
projPJ                 utm_projection = NULL, latlon_projection = NULL;


void usage ()
{
  fprintf (stderr, 
           "\nUsage: pfm_charts_image PFM_FILE AREA_FILE CAMERA_FILE -d DATUM_OFFSET -g DEM_GRID_SIZE -c CELL_SIZE [options]\n");


  fprintf (stderr, "\nWhere:\n\n");
  fprintf (stderr, "\tPFM_FILE = PFM file (required)\n\n");
  fprintf (stderr, "\tAREA_FILE = Area file (required)\n\n");
  fprintf (stderr, "\t\tThe area file names must have a .ARE extension\n");
  fprintf (stderr, "\t\tfor ISS60 type area files, a .are extension for generic area files, or\n");
  fprintf (stderr, "\t\ta .afs extension for Army Corps area files.\n\n");
  fprintf (stderr, "\tCAMERA_FILE = Optech format system parameter file (*.txt, required)\n\n"); 
  fprintf (stderr, "\t-d DATUM_OFFSET = offset in meters from ellipsoid to PFM datum\n");
  fprintf (stderr, "\t\tEither this or the -e option is required (value is added to data)\n");
  fprintf (stderr, "\t-g DEM_GRID_SIZE = DEM grid size in meters (required)\n");
  fprintf (stderr, "\t-c CELL_SIZE = Output GeoTIFF cell (pixel) size in meters (required)\n\n\n");
  fprintf (stderr, 
           "[options] = [-l LOWER -u UPPER] [-s] [-e] [-m] [-H] [-T] [-x] [-z ZERO_FILE] [-i INTERVAL] [-f] [-r] [-n]\n\n");
  fprintf (stderr, "\t-l LOWER = lower value of heading direction limits (0 - 360 degrees [requires -u]).\n");
  fprintf (stderr, "\t-u UPPER = upper value of heading direction limits (0 - 360 degrees [requires -l]).\n");
  fprintf (stderr, "\t-U = output file is UTM projected GeoTIFF (default is geographic GeoTIFF).\n");
  fprintf (stderr, "\t-x = exclude too light or too dark pictures.\n");
  fprintf (stderr, "\t-n = use ImageMagick's 'convert -normalize' to normalize the colors (Linux only).\n");
  fprintf (stderr, "\t-s = skip data that uses a .pos file (instead of SBET) to position the plane.\n");
  fprintf (stderr, "\t-e = use EGM08 value for DATUM_OFFSET.\n");
  fprintf (stderr, "\t-m = include Shuttle Radar Topography Mission (SRTM) data in empty DEM cells.\n");
  fprintf (stderr, "\t-H = use only images associated with HOF files.\n");
  fprintf (stderr, "\t-T = use only images associated with TOF files.\n");
  fprintf (stderr, "\t-z = set all data input to the DEM computation in the ZERO_FILE (area file) to zero.\n");
  fprintf (stderr, "\t-i = only output pictures at INTERVAL spacing (-i 2 = output every other picture)\n");
  fprintf (stderr, "\t-f = flip sign on Z value (change depth to elevation or elevation to depth).\n");
  fprintf (stderr, "\t-r = remove the intermediate image directory after creating the mosaic.\n\n\n");
  fflush (stderr);
}



void get_camera_parameters (NV_CHAR *path, OPTIONS *options)
{
  FILE *fp;
  NV_CHAR string[128];
  NV_INT32 ms;


  if ((fp = fopen (path, "r")) == NULL)
    {
      perror (path);
      exit (-1);
    }


  while (fgets (string, sizeof (string), fp) != NULL)
    {
      if (strstr (string, "camera_boresight_roll:")) 
        sscanf (string, "camera_boresight_roll: %lf", &options->roll_bias);

      if (strstr (string, "camera_boresight_pitch:"))
        sscanf (string, "camera_boresight_pitch: %lf", &options->pitch_bias);

      if (strstr (string, "camera_boresight_heading:"))
        sscanf (string, "camera_boresight_heading: %lf", &options->heading_bias);

      if (strstr (string, "pixel_size:"))
        sscanf (string, "pixel_size: %lf", &options->pixel_size);

      if (strstr (string, "focal_length:"))
        sscanf (string, "focal_length: %lf", &options->focal_length);

      if (strstr (string, "principal_point_offsets:"))
        sscanf (string, "principal_point_offsets: %lf, %lf", &options->column_offset, &options->row_offset);

      if (strstr (string, "camera_trig_delay:"))
        {
          sscanf (string, "camera_trig_delay: %d", &ms);
          options->time_offset = (NV_FLOAT64) ms / 1000.0;
        }
    }

  fprintf (stderr, "\n\nCamera parameters:\n\n");
  fprintf (stderr, "\tcamera_boresight_roll:    %f\n", options->roll_bias);
  fprintf (stderr, "\tcamera_boresight_pitch:   %f\n", options->pitch_bias);
  fprintf (stderr, "\tcamera_boresight_heading: %f\n", options->heading_bias);
  fprintf (stderr, "\tpixel_size:               %f\n", options->pixel_size);
  fprintf (stderr, "\tfocal_length:             %f\n", options->focal_length);
  fprintf (stderr, "\tprincipal_point_offsets:  %f, %f\n", options->column_offset, options->row_offset);
  fprintf (stderr, "\tcamera_trig_delay:        %f\n\n\n", options->time_offset);
  fflush (stderr);

  fclose (fp);
}



NV_INT32 get_images (NV_CHAR *path, NV_INT32 type, char *img_dir, NV_FLOAT64 *polygon_x, NV_FLOAT64 *polygon_y,
                     NV_INT32 polygon_count, OPTIONS options, NV_INT32 start, NV_INT32 end, NV_INT32 total, 
                     FILE *txt_fp, NV_CHAR *areafile)
{
  FILE                   *data_fp, *jpg_fp, *img_fp;
  POS_OUTPUT_T           pos;
  NV_INT64               new_stamp, image_time, data_timestamp, offset;
  NV_INT32               i, img_rec_num, gicount, interval_count;
  NV_U_INT32             size;
  TOPO_OUTPUT_T          tof;
  static HYDRO_OUTPUT_T  hof;
  static NV_INT32        count = 0, percent = 0, old_percent = -1, total_count = 0, bad_count = 0;
  static NV_INT64        prev_stamp = -1;
  NV_CHAR                img_file[512], pos_file[512], temp[512], jpg_file[512], norm_file[512], command[1024];
  static NV_CHAR         prev_img_file[512] = {""};
  static FILE            *pos_fp = NULL;
  NV_U_CHAR              *image;
  IMAGE_INDEX_T          image_index;
  static NV_FLOAT32      prev_local_datum = 99999.0, local_datum = 99999.0;
  NV_FLOAT32             heading;
  NV_FLOAT64             lat_degs, lon_degs, x, y, p, r, h, cos_p, cos_r, cos_hc, sin_p, sin_r, sin_hc, c11, c21, 
                         c31, c32, c33, omega, phi, kappa;
  NV_BOOL                skip_pos = NVFalse, good_rec = NVTrue;


  strcpy (img_file, path);
  strcpy (&img_file[strlen (img_file) - 4], ".img");

  if ((img_fp = open_image_file (img_file)) == NULL) return (0);


  if (type == PFM_SHOALS_TOF_DATA)
    {
      if ((data_fp = open_tof_file (path)) == NULL)
	{
	  perror (path);
	  exit (-1);
	}
    }
  else
    {
      if ((data_fp = open_hof_file (path)) == NULL)
	{
	  perror (path);
	  exit (-1);
	}
    }


  gicount = 0;
  interval_count = 0;
  offset = NINT (options.time_offset * 1000000.0);


  for (i = start ; i <= end ; i++)
    {
      /*  Find the record based on the timestamp from the hof or tof file.  */

      if (type == PFM_SHOALS_TOF_DATA)
	{
	  tof_read_record (data_fp, i, &tof);
	  data_timestamp = tof.timestamp;


          if (options.datum_offset == 0.0)
            {
              if (tof.elevation_last > -997.0)
                {
                  local_datum = tof.result_elevation_last - tof.elevation_last;
                }
              else
                {
                  if (prev_local_datum < 99999.0)
                    {
                      local_datum = prev_local_datum;
                    }
                  else
                    {
                      continue;
                    }
                }
            }
          else
            {
              local_datum = options.datum_offset;
            }
	}
      else
	{
	  hof_read_record (data_fp, i, &hof);
	  data_timestamp = hof.timestamp;


          if (options.datum_offset == 0.0)
            {
              if (hof.correct_depth > -997.0)
                {
                  if (hof.data_type)
                    {
                      local_datum = hof.kgps_datum;
                    }
                  else
                    {
                      fprintf (stderr,"\n\nI don't know how to deal with DGPS data - file %s\n\n", path);
                      exit (-1);
                    }
                }
              else
                {
                  if (prev_local_datum < 99999.0)
                    {
                      local_datum = prev_local_datum;
                    }
                  else
                    {
                      continue;
                    }
                }
            }
          else
            {
              local_datum = options.datum_offset;
            }
	}

      if (local_datum != 0.0) prev_local_datum = local_datum;


      if ((img_rec_num = image_find_record (img_fp, data_timestamp)) != 0)
        {
	  image_get_metadata (img_fp, img_rec_num, &image_index);


          /*  Don't get the same image twice.  */

          if (image_index.timestamp != prev_stamp) 
            {
              /*  If the image file has changed, find, open, and read the sbet (reprocessed) or pos file information.  */

              if (strcmp (img_file, prev_img_file))
                {
                  strcpy (prev_img_file, img_file);

                  if (pos_fp != NULL)
		    {
		      fclose (pos_fp);
		      pos_fp = NULL;
		    }

                  if (!get_pos_file (img_file, pos_file))
                    {
                      fprintf (stderr, "Unable to find pos/sbet file for image file %s\n", img_file);
                      fflush (stderr);
                      continue;
                    }


                  /*  Check to see if we want to skip .pos files.  */

                  if (options.pos && (strstr (pos_file, "sbet") || strstr (pos_file, "SBET")))
                    {
                      skip_pos = NVFalse;


                      if ((pos_fp = open_pos_file (pos_file)) == NULL)
                        {
                          fprintf (stderr, "Unable to open pos/sbet file for image file %s\n", pos_file);
                          fflush (stderr);
                          continue;
                        }
                    }
                  else
                    {
                      skip_pos = NVTrue;
                    }
                }


              if (skip_pos) continue;


              /*  Get the attitude data for this image.  */

              new_stamp = pos_find_record (pos_fp, &pos, image_index.timestamp + offset);

              good_rec = NVTrue;
              if (!new_stamp) 
                {
                  fprintf (stderr, "\n\nUnable to get timestamp ");
                  fprintf (stderr, NV_INT64_SPECIFIER, image_index.timestamp + offset);
                  fprintf (stderr, " for pos/sbet file %s\n", pos_file);
                  fprintf (stderr, 
                           "This usually indicates that the above pos/sbet file is FUBAR or the name is incorrect!\n");
                  fprintf (stderr, 
                           "Make sure the file name conforms to the naming convention (_YYMMDD_NNNN.out or .pos) and\n");
                  fprintf (stderr, 
                           "check the start and end times of this file (dump_pos) against the data in the HOF/TOF/IMG files.\n\n\n");

                  bad_count++;
                  good_rec = NVFalse;

                  if (bad_count > 100) exit (-1);
                }


              lat_degs = pos.latitude * RAD_TO_DEG;
              lon_degs = pos.longitude * RAD_TO_DEG;


              /*  Make sure we're inside the area we specified.  */

              if (good_rec && inside (polygon_x, polygon_y, polygon_count, lon_degs, lat_degs))
                {
                  /*  Read the actual image.  */

                  image = image_read_record (img_fp, image_index.timestamp, &size, &image_time);


                  /*  If the size is less than 150000 this image is either too bright or too dark.  */

                  if (!options.exclude || size >= 150000)
                    {
                      heading = fmod ((pos.platform_heading - pos.wander_angle) * RAD_TO_DEG, 360.0);
                      if (heading < -180.0) heading += 360.0;
                      if (heading > 180.0) heading -=360.0;


                      /*  Check for direction limits.  */

                      NV_INT32 norm_heading = (NV_INT32) heading + 360;
                      if (options.high_limit <= 360) norm_heading %= 360;

                      if (norm_heading >= options.low_limit && norm_heading <= options.high_limit)
                        {
                          /*  Check for interval.  */

                          if (!(interval_count % options.interval) || i == end)
                            {
                              /*  Open the jpg file.  */

                              strcpy (temp, img_file);
                              strcpy (jpg_file, pfm_basename (temp));
                              sprintf (&jpg_file[strlen (jpg_file) - 4], "__%s_%02d_%03d.jpg", pfm_basename (areafile),
                                       gicount, NINT (heading));
                              sprintf (temp, "%s%1c%s", img_dir, (NV_CHAR) SEPARATOR, jpg_file);

                              if ((jpg_fp = fopen (temp, "wb")) == NULL) 
                                {
                                  free (image);
                                  continue;
                                }


                              /*  Write the image to the jpg file.  */

                              fwrite (image, size, 1, jpg_fp);

                              fclose (jpg_fp);


                              if (options.normalize)
                                {
                                  sprintf (norm_file, "%s.norm.tmp", temp);
                                  sprintf (command, "convert %s -normalize %s", temp, norm_file);

                                  if (system (command))
                                    {
                                      perror (command);
                                    }
                                  else
                                    {
                                      rename (norm_file, temp);
                                    }
                                }


                              /*  Convert to omega, phi, kappa.  */

                              r = (pos.roll * RAD_TO_DEG + options.roll_bias) * DEG_TO_RAD;
                              p = (pos.pitch * RAD_TO_DEG + options.pitch_bias) * DEG_TO_RAD;
                              h = (heading + options.heading_bias) * DEG_TO_RAD;

			      /* Calculate the convergence (true north to grid north) correction to be added to the
			       heading value.
			       This calculation is from http://www.ordnancesurvey.co.uk/
			       Ellipsoid axis dimensions ( a (NV_A0) & b (NV_B0) ) in meters and central meridian
			       scale factor ( f0 or 0.9996 )
			      */

			      NV_FLOAT64 f0  = 0.9996L;
			      NV_FLOAT64 af0 = NV_A0 * f0;
			      NV_FLOAT64 bf0 = NV_B0 * f0;
			      NV_FLOAT64 e2  = ((af0 * af0) - (bf0 * bf0)) / (af0 * af0);

			      NV_FLOAT64 xiii = sin (pos.latitude);
			      NV_FLOAT64 xiii2 = cos (pos.latitude);
			      NV_FLOAT64 xiii3 = tan (pos.latitude);

			      NV_FLOAT64 nu = af0 / sqrt (1.0L - (e2 * xiii * xiii));
			      NV_FLOAT64 rho = af0 * (1.0 - e2) / pow (1.0 - (e2 * xiii * xiii), 1.5);
			      NV_FLOAT64 eta2 = (nu / rho) - 1.0L;
			      NV_FLOAT64 p2 = pos.longitude - (((NV_FLOAT64) (zone - 31) * 6.0L + 3.0L) * NV_DEG_TO_RAD);


			      /* Compute convergence */

			      NV_FLOAT64 xiv = ((-xiii * xiii2 * xiii2) / 3.0L) * (1.0L + (3.0L * eta2) + (2.0L * eta2 * eta2));
			      NV_FLOAT64 xv = ((-xiii * pow (xiii2, 4.0L)) / 15.0L) * (2.0L - (xiii3 * xiii3));

			      NV_FLOAT64 lat_long_to_c_rad = (p2 * -xiii) + (pow (p2, 3.0L) * xiv) + (pow (p2, 5.0L) * xv);


			      /*  Add convergence to heading.*/

			      NV_FLOAT64 hc = h + lat_long_to_c_rad;


                              /*  Convert lat/lon to northing/easting because MOSAIC wants it that way.  */

                              x = lon_degs * NV_DEG_TO_RAD;
                              y = lat_degs * NV_DEG_TO_RAD;
                              pj_transform (latlon_projection, utm_projection, 1, 1, &x, &y, NULL);


                              cos_p = cos (p);
                              sin_p = sin (p);
                              cos_r = cos (r);
                              sin_r = sin (r);
                              cos_hc = cos (hc);
                              sin_hc = sin (hc);

                              c11 = cos_r * cos_hc;
                              c21 = sin_p * sin_r * cos_hc + cos_p * sin_hc;
                              c31 = cos_p * sin_r * cos_hc - sin_p * sin_hc;
                              c32 = -cos_p * sin_r * sin_hc - sin_p * cos_hc;
                              c33 = cos_p * cos_r;

                              phi = asin (c31) * RAD_TO_DEG;
                              omega = atan (-c32 / c33) * RAD_TO_DEG;
                              kappa = atan2 (-c21, c11) * RAD_TO_DEG;
			      
                              fprintf (txt_fp, "%d %s%1c%s %0.9f %0.9f %f %f %f %f %d\n", total_count, img_dir, (NV_CHAR) SEPARATOR, jpg_file, 
                                       x, y, pos.altitude - local_datum, omega, phi, kappa, zone);


                              gicount++;
                              total_count++;
                            }
                        }
                      interval_count++;
                    }
                  free (image);
                }
            }
          prev_stamp = image_index.timestamp;
        }


      count++;
      percent = ((NV_FLOAT32) count / (NV_FLOAT32) total) * 100.0;
      if (percent != old_percent)
        {
          old_percent = percent;
          fprintf (stderr, "%03d%% processed            \r", percent);
          fflush (stderr);
        }
    }


  fclose (img_fp);
  fclose (data_fp);


  return (gicount);
}


NV_INT32 main (NV_INT32 argc, char **argv)
{
  FILE                *txt_fp, *pars_fp;
  NV_INT32            i, polygon_count = 0, x_start, y_start, width, zero_count = 0, 
                      height, icount = 0, total;
  NV_INT16            type;
  LIST_NUM            list[10000];
  NV_CHAR             pfm_file[512], dem_file[512], areafile[512], camera_file[512], dir[512], path[512],
                      txt_file[512], zero_file[512], pars_file[512], mosaic_file[512], command[1024];
  NV_CHAR             c;
  NV_FLOAT64          polygon_x[200], polygon_y[200], cen_x, cen_y, min_ne_x, min_ne_y, 
                      max_ne_x, max_ne_y, zero_x[200], zero_y[200];
  NV_F64_XYMBR        zero_mbr;
  extern char         *optarg;
  extern int          optind;
  NV_BOOL             datum_input = NVFalse, grid_input = NVFalse, cell_input = NVFalse, restricted = NVFalse;


  NV_BOOL misp (NV_INT32 pfm_handle, NV_FLOAT64 bin_size_meters, NV_CHAR *dem_file, NV_FLOAT64 min_ne_x,
                NV_FLOAT64 min_ne_y, NV_FLOAT64 max_ne_x, NV_FLOAT64 max_ne_y, 
                NV_INT32 pfm_start_x, NV_INT32 pfm_start_y, NV_INT32 pfm_width,
                NV_INT32 pfm_height, NV_FLOAT64 *zero_x, NV_FLOAT64 *zero_y, NV_INT32 zero_count,
                LIST_NUM *list, NV_FLOAT32 datum_offset, NV_BOOL flip, NV_BOOL srtm, projPJ utm_projection, projPJ latlon_projection, NV_INT32 zone);


  /*  Set defaults.  */

  options.type = 0;
  options.roll_bias = 0.0;
  options.pitch_bias = 0.0;
  options.heading_bias = 0.0;
  options.datum_offset = 0.0;
  options.grid_size = 1.0;
  options.time_offset = 0.1;
  options.interval = 1;
  options.low_limit = 0;
  options.high_limit = 360;
  options.remove = NVFalse;
  options.flip = NVFalse;
  options.exclude = NVFalse;
  options.normalize = NVFalse;
  options.egm = NVFalse;
  options.srtm = NVFalse;
  options.utm = NVFalse;
  options.pos = NVFalse;
  options.cell_size = 0.2;
  options.focal_length = 16.065;
  options.pixel_size = 7.40;
  options.column_offset = 30.11;
  options.row_offset = -11.68;


  printf ("\n\n %s \n\n\n", VERSION);


  /*  Set all hits to false.  */

  for (i = 0 ; i < 10000 ; i++)
    {
      list[i].hit = NVFalse;
      list[i].start = NV_U_INT32_MAX;
      list[i].end = 0;
    }


  strcpy (zero_file, "NONE");


  while ((c = getopt (argc, argv, "d:i:g:z:l:u:c:UxmnsHTefr")) != EOF)
    {
      switch (c)
        {
        case 'r':
          options.remove = NVTrue;
          break;

        case 'c':
          sscanf (optarg, "%lf", &options.cell_size);
          cell_input = NVTrue;
          break;

        case 'd':
          sscanf (optarg, "%lf", &options.datum_offset);
          datum_input = NVTrue;
          break;

        case 'i':
          sscanf (optarg, "%d", &options.interval);
          break;

        case 'g':
          sscanf (optarg, "%lf", &options.grid_size);
          grid_input = NVTrue;
          break;

        case 'l':
          sscanf (optarg, "%d", &options.low_limit);
          break;

        case 'u':
          sscanf (optarg, "%d", &options.high_limit);
          break;

        case 'z':
          strcpy (zero_file, optarg);
          break;

        case 'U':
          options.utm = NVTrue;
          break;

        case 'x':
          options.exclude = NVTrue;
          break;

        case 'n':
          options.normalize = NVTrue;
          break;

        case 's':
          options.pos = NVTrue;
          break;

        case 'H':
          options.type = PFM_CHARTS_HOF_DATA;
          break;

        case 'T':
          options.type = PFM_SHOALS_TOF_DATA;
          break;

        case 'e':
          options.egm = NVTrue;
          break;

        case 'm':
          options.srtm = NVTrue;
          break;

        case 'f':
          options.flip = NVTrue;
          break;

        default:
          usage ();
          exit (-1);
          break;
        }
    }


  /*  If we don't have SRTM 1, 2, or 3 turn off the option if it's on.  */

  if (options.srtm && !check_srtm1_topo () && !check_srtm2_topo () && !check_srtm3_topo ())
    {
      fprintf (stderr, "\n\nSRTM data is not available, turning off SRTM option.\n\n");
      options.srtm = NVFalse;
    }


#ifndef NVLinux
  options.normalize = NVFalse;
#endif


  /* Make sure we got the mandatory file name arguments.  */

  if (optind >= argc || (!options.egm && !datum_input) || !grid_input || !cell_input)
    {
      usage ();
      exit (-1);
    }


  strcpy (pfm_file, argv[optind]);
  strcpy (areafile, argv[optind + 1]);
  strcpy (camera_file, argv[optind + 2]);
 

  get_area_mbr (areafile, &polygon_count, polygon_x, polygon_y, &options.mbr);


  get_camera_parameters (camera_file, &options);

 
  /*  We're doing this in UTM for now because MOSAIC wants it.  Later we should be able to change back.  */

  cen_x = options.mbr.min_x + (options.mbr.max_x - options.mbr.min_x) / 2.0;
  cen_y = options.mbr.min_y + (options.mbr.max_y - options.mbr.min_y) / 2.0;

  NV_CHAR string[60];
  if (options.mbr.max_y < 0.0)
    {
      sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +south +lon_0=%.9f", cen_x);
    }
  else
    {
      sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +lon_0=%.9f", cen_x);
    }

  if (!(utm_projection = pj_init_plus (string)))
    {
      fprintf (stderr, "Error initializing UTM projection\n");
      exit (-1);
    }

  if (!(latlon_projection = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
    {
      fprintf (stderr, "Error initializing latlon projection\n");
      exit (-1);
    }

  zone = (NV_INT32) (31.0 + cen_x / 6.0);
  if (zone >= 61) zone = 60;	
  if (zone <= 0) zone = 1;


  fprintf (stderr, "UTM zone %d\n", zone);

  min_ne_x = options.mbr.min_x * NV_DEG_TO_RAD;
  min_ne_y = options.mbr.min_y * NV_DEG_TO_RAD;
  pj_transform (latlon_projection, utm_projection, 1, 1, &min_ne_x, &min_ne_y, NULL);

  fprintf (stderr, "Min northing and easting : %.2f %.2f\n", min_ne_y, min_ne_x);

  max_ne_x = options.mbr.max_x * NV_DEG_TO_RAD;
  max_ne_y = options.mbr.max_y * NV_DEG_TO_RAD;
  pj_transform (latlon_projection, utm_projection, 1, 1, &max_ne_x, &max_ne_y, NULL);

  fprintf (stderr, "Max northing and easting : %.2f %.2f\n\n\n", max_ne_y, max_ne_x);
  fflush (stderr);


  if (strcmp (zero_file, "NONE")) get_area_mbr (zero_file, &zero_count, zero_x, zero_y, &zero_mbr);


  /*  Make the image directory.  */

  sprintf (dir, "%s__%s__image_files", pfm_file, pfm_basename (areafile));
#ifdef NVWIN3X
 #ifdef __MINGW64__
  if (mkdir (dir))
 #else
  if (_mkdir (dir))
 #endif
#else
  if (mkdir (dir, 00777))
#endif
    {
      if (errno != 17)
        {
          perror (dir);
          exit (-1);
        }
    }

  strcpy (options.open_args.list_path, argv[optind]);

  options.open_args.checkpoint = 0;
  options.pfm_handle = open_existing_pfm_file (&options.open_args);

  if (options.pfm_handle < 0) pfm_error_exit (pfm_error);


  x_start = 0;
  y_start = 0;
  width = options.open_args.head.bin_width;
  height = options.open_args.head.bin_height;

  if (options.mbr.min_y > options.open_args.head.mbr.max_y || options.mbr.max_y < options.open_args.head.mbr.min_y ||
      options.mbr.min_x > options.open_args.head.mbr.max_x || options.mbr.max_x < options.open_args.head.mbr.min_x)
    {
      fprintf (stderr, "\n\nSpecified area is completely outside of the PFM bounds!\n\n");
      exit (-1);
    }


  /*  Match to nearest cell.  */

  x_start = NINT ((options.mbr.min_x - options.open_args.head.mbr.min_x) / options.open_args.head.x_bin_size_degrees);
  y_start = NINT ((options.mbr.min_y - options.open_args.head.mbr.min_y) / options.open_args.head.y_bin_size_degrees);
  width = NINT ((options.mbr.max_x - options.mbr.min_x) / options.open_args.head.x_bin_size_degrees);
  height = NINT ((options.mbr.max_y - options.mbr.min_y) / options.open_args.head.y_bin_size_degrees);


  /*  Adjust to PFM bounds if necessary.  */

  if (x_start < 0) x_start = 0;
  if (y_start < 0) y_start = 0;
  if (x_start + width > options.open_args.head.bin_width) width = options.open_args.head.bin_width - x_start;
  if (y_start + height > options.open_args.head.bin_height) height = options.open_args.head.bin_height - y_start;


  /*  Redefine bounds.  */

  options.mbr.min_x = options.open_args.head.mbr.min_x + x_start * options.open_args.head.x_bin_size_degrees;
  options.mbr.min_y = options.open_args.head.mbr.min_y + y_start * options.open_args.head.y_bin_size_degrees;
  options.mbr.max_x = options.mbr.min_x + width * options.open_args.head.x_bin_size_degrees;
  options.mbr.max_y = options.mbr.min_y + height * options.open_args.head.y_bin_size_degrees;


  /*  If we want to use EGM08, get the value at the center of the MBR.  */

  if (options.egm)
    {
      options.datum_offset = get_egm08 (cen_y, cen_x);
      if (options.datum_offset > 999998.0) 
        {
          fprintf (stderr, "\n\nUnable to get EGM08 ellipsoid to datum offset!\n\n");
          exit (-1);
        }
    }


  fprintf (stderr, "Data parameters:\n\n");
  fprintf (stderr, "\tData type (0-both, %d-HOF, %d-TOF): %d\n", PFM_CHARTS_HOF_DATA, PFM_SHOALS_TOF_DATA, options.type);
  fprintf (stderr, "\tDatum offset:                       %f\n", options.datum_offset);
  fprintf (stderr, "\tDEM grid size(m):                   %f\n", options.grid_size);
  fprintf (stderr, "\tImage interval:                     %d\n", options.interval);
  fprintf (stderr, "\tLower direction limit:              %d\n", options.low_limit);
  fprintf (stderr, "\tUpper direction limit:              %d\n", options.high_limit);
  fprintf (stderr, "\tRemove image directory:             %d\n", options.remove);
  fprintf (stderr, "\tInvert Z values:                    %d\n", options.flip);
  fprintf (stderr, "\tExclude too dark or light images:   %d\n", options.exclude);
  fprintf (stderr, "\tNormalize colors:                   %d\n", options.normalize);
  fprintf (stderr, "\tUse SRTM data:                      %d\n", options.srtm);
  fprintf (stderr, "\tOutput UTM GeoTIFF:                 %d\n", options.utm);
  fprintf (stderr, "\tDo not use .pos files (only SBET):  %d\n", options.pos);
  fprintf (stderr, "\tGeoTIFF cell (pixel) size:          %f\n\n\n", options.cell_size);


  /*  Create the dem tif file name.  */

  if (options.srtm && check_srtm2_topo ()) restricted = NVTrue;


  if (restricted)
    {
      sprintf (dem_file, "%s/%s_DOD_restricted_if_outside_US__dem.tif", dir, pfm_basename (areafile));
    }
  else
    {
      sprintf (dem_file, "%s/%s_dem.tif", dir, pfm_basename (areafile));
    }


  fprintf (stderr, "Building DEM tif file %s\n\n", dem_file);
  fflush (stderr);


  if (misp (options.pfm_handle, options.grid_size, dem_file, min_ne_x, min_ne_y, max_ne_x, max_ne_y, x_start, y_start, width, height,
            zero_x, zero_y, zero_count, list, options.datum_offset, options.flip, options.srtm, utm_projection, latlon_projection, zone))
    {
      fprintf (stderr, "\n\nError, no input points found running misp_proc!\nUnable to create DEM!\n");
      fflush (stderr);
      return (-1);
    }


  if (restricted)
    {
      if (check_srtm2_restricted_data_read ())
        {
          sprintf (path, "%s/%s_DOD_restricted__dem.tif", dir, pfm_basename (areafile));
        }
      else
        {
          sprintf (path, "%s/%s_dem.tif", dir, pfm_basename (areafile));
        }

      rename (dem_file, path);
      strcpy (dem_file, path);
    }


  total = 0;
  for (i = 0 ; i < 10000 ; i++)
    {
      if (list[i].hit) total += ((list[i].end - list[i].start) + 1);
    }


  /*  Open the pos.dat file.  */

  sprintf (txt_file, "%s%1c%s_pos.dat", dir, (NV_CHAR) SEPARATOR, pfm_basename (areafile));
  if ((txt_fp = fopen (txt_file, "w")) == NULL) 
    {
      perror (txt_file);
      exit (-1);
    }


  fprintf (stderr, "\n\nExtracting images\n\n");
  fflush (stderr);


  for (i = 0 ; i < 10000 ; i++)
    {
      if (list[i].hit)
        {
          read_list_file (options.pfm_handle, (NV_INT16) i, path, &type);


          /*  Check for input type limiting.  */

          if (!options.type || options.type == type || (options.type == PFM_CHARTS_HOF_DATA && type == PFM_SHOALS_1K_DATA))
            {
              icount += get_images (path, type, dir, polygon_x, polygon_y, polygon_count, options, list[i].start, 
                                    list[i].end, total, txt_fp, areafile);
            }
        }
    }

  fprintf (stderr, "Extracted %d images\n\n", icount);
  fflush (stderr);

  close_pfm_file (options.pfm_handle);
  fclose (txt_fp);



  /*  Generate the mosaic.  */

  sprintf (pars_file, "%s__%s", pfm_file, pfm_basename (areafile));
  strcpy (strstr (pars_file, ".are"), ".pars");

  if (options.utm)
    {
      sprintf (mosaic_file, "%s__%s", pfm_file, pfm_basename (areafile));
      strcpy (strstr (mosaic_file, ".are"), "_utm.tif");
    }
  else
    {
      sprintf (mosaic_file, "%s__%s", pfm_file, pfm_basename (areafile));
      strcpy (strstr (mosaic_file, ".are"), ".tif");
    }


  if ((pars_fp = fopen (pars_file, "w")) == NULL)
    {
      perror (pars_file);
      exit (-1);
    }


  /*  Fill the mosaic parameter file.  */

  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "images_file:         %s\n", txt_file);
  fprintf (pars_fp, "dem_file:            %s\n", dem_file);
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "readers:             20\n");
  fprintf (pars_fp, "mappers:             8\n");
  fprintf (pars_fp, "writers:             1\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "block_x:             512\n");
  fprintf (pars_fp, "block_y:             512\n");
  fprintf (pars_fp, "slack:               50\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   easting_left, easting_right, northing_top and northern_bottom define\n");
  fprintf (pars_fp, "#   the geographic region to process\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "easting_left:        %.3f\n", min_ne_x);
  fprintf (pars_fp, "easting_right:       %.3f\n", max_ne_x);
  fprintf (pars_fp, "northing_top:        %.3f\n", max_ne_y);
  fprintf (pars_fp, "northing_bottom:     %.3f\n", min_ne_y);
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   longitude_left, longitude_right, latitude_bottom, and latitude_top define\n");
  fprintf (pars_fp, "#   the geographic region to process for geographic output\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "longitude_left:      %.3f\n", options.mbr.min_x);
  fprintf (pars_fp, "longitude_right:     %.3f\n", options.mbr.max_x);
  fprintf (pars_fp, "latitude_top:        %.3f\n", options.mbr.max_y);
  fprintf (pars_fp, "latitude_bottom:     %.3f\n", options.mbr.min_y);
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   utm_zone defines which zone is in use\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "utm_zone:            %d\n", zone);
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   output_projection is either utm for UTM projection or blank for geographic\n");
  fprintf (pars_fp, "#\n");
  if (options.utm)
    {
      fprintf (pars_fp, "output_projection:            utm\n");
    }
  else
    {
      fprintf (pars_fp, "output_projection:               \n");
    }
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   output_cell_size defines the output cell size\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "output_cell_size:    %.2f\n", options.cell_size);
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   input_cell_size defines the input cell size\n");
  fprintf (pars_fp, "#   input_rows and input_columns define the imput image size.\n");
  fprintf (pars_fp, "#   These are used in the approximation of the maximum distance.\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "input_cell_size:     0.2\n");
  fprintf (pars_fp, "input_rows:          1200\n");
  fprintf (pars_fp, "input_columns:       1600\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   Sea level needs to be defined\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "sea_level:           0.0\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   focal_length is the focal length of the camera in millimeters\n");
  fprintf (pars_fp, "#   pixel_size is the camera's pixel size in microns\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "focal_length:        %.3f\n", options.focal_length);
  fprintf (pars_fp, "pixel_size:          %.2f\n", options.pixel_size);
  fprintf (pars_fp, "column_offset:       %.2f\n", options.column_offset);
  fprintf (pars_fp, "row_offset:          %.2f\n", options.row_offset);
  fprintf (pars_fp, "camera_roll:         0.0\n");
  fprintf (pars_fp, "camera_pitch:        0.0\n");
  fprintf (pars_fp, "camera_yaw:          0.0\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   The output will be a GeoTiff file\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "output_file:         %s\n", mosaic_file);
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "#   Overviews will be defined by nearest neighbor sampling\n");
  fprintf (pars_fp, "#   0 means no overview; 1 means sampled at 2; 2 means sampled at 2 and 4;\n");
  fprintf (pars_fp, "#   3 means sampled at 2, 4 and 8.\n");
  fprintf (pars_fp, "#   The maximum number of overview levels in set to 5.\n");
  fprintf (pars_fp, "#\n");
  fprintf (pars_fp, "overviews:           0\n");

  fclose (pars_fp);


  fprintf (stderr, "\n\nRunning Ray Seyfarth's MOSAIC program\n\n");
  fflush (stderr);


  sprintf (command, "mosaic  %s", pars_file);

  if (system (command))
    {
      perror (command);
      exit (-1);
    }


  /*  Remove the image directory if we requested it.  */

  if (options.remove) 
    {
      sprintf (command, "rm -rf %s", dir);
      system (command);
    }


  return (0);
}
