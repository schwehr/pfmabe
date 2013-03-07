
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

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "pfmLoadM.hpp"

#include "load_file.hpp"
#include "unisips_io.h"
#include "newgp.h"
#include "invgp.h"


// ******************************************************************
// The variables following this are global within this module only. 
// ******************************************************************


static NV_INT16         file_number[MAX_PFM_FILES];
static NV_INT32         recnum;
static NV_INT32         pfm_file_count;
static NV_INT16         line_count[MAX_PFM_FILES];
static NV_CHAR          line_time[2000][18];
static NV_INT16         next_line[MAX_PFM_FILES];
static NV_FLOAT64       prev_heading[MAX_PFM_FILES];
static NV_INT32         total_out_of_limits;
static NV_INT32         out_count[MAX_PFM_FILES];
static NV_FLOAT32       attr[NUM_ATTR];
static NV_CHAR          path[1024];
static NV_CHAR          native_path[1024];
static QString          nativePath;
static NV_BOOL          lidar_null;


static QProgressBar *prog;


void getGSFAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, gsfSwathBathyPing mb_ping, NV_INT32 beam);
void getHOFAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, HYDRO_OUTPUT_T hof);
void getTOFAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, TOPO_OUTPUT_T tof);
void getWLFAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, WLF_RECORD wlf);
void getCZMILAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, CZMIL_CXY_Data czmil);
void getBAGAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, BAG_RECORD bag);
void getHAWKEYEAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, HAWKEYE_RECORD hawkeye);


static void indexProgress (int state, int percent)
{
  static NV_INT32 old_percent = -1;
  static NV_BOOL second = NVFalse;

  if (state == 1) second = NVFalse;

  if (!second && state == 2) 
    {
      prog->reset ();
      second = NVTrue;
    }
      

  //  We don't want to eat up all of the free time on the system with a progress bar.

  if (percent - old_percent >= 5 || old_percent > percent)
    {
      prog->setValue (percent);
      old_percent = percent;
    }

  qApp->processEvents ();
}



/********************************************************************
 *
 * Function Name :Do_PFM_Processing 
 *
 * Description : Once the data to be put into the PFM file has been 
 *               processed, then this is called to do the job.
 *
 * Inputs :NV_INT32   BeamNo  - Sub-record number (beam number for swath sonars)
 *         NV_FLOAT64 depth   - Depth to fill grid
 *         NV_F64_COORD2 nxy  - NAVO structure with position of sounding.
 *         NV_U_INT32 flags   - flags associated with the sounding.
 *         NV_FLOAT64 heading - heading used to test for line change (GSF only)
 *         time_t tv_sec      - POSIX time from epoch in seconds (for line name)
 *         long tv_nsec       - POSIX nanoseconds of second (for line name)
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

void Do_PFM_Processing (NV_INT32 beam, NV_FLOAT64 depth, NV_F64_COORD2 nxy, NV_FLOAT32 herr, NV_FLOAT32 verr, NV_U_INT32 flags, 
                        NV_FLOAT64 heading, NV_FLOAT32 *l_attr, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[],
                        NV_INT32 percent, time_t tv_sec, long tv_nsec)
{
  NV_INT32            j, k, status;
  DEPTH_RECORD        depth_record;
  NV_FLOAT64          diff;
  static NV_INT32     old_percent = -1;


  //  Loop over PFM files we have to fill. 

  for (j = 0 ; j < pfm_file_count ; j++) 
    {
      //  Dateline check.

      if (pfm_def[j].dateline && nxy.x < 0.0) nxy.x += 360.0;


      if (bin_inside_ptr (&pfm_def[j].open_args.head, nxy))
        {
          compute_index_ptr (nxy, &depth_record.coord, &pfm_def[j].open_args.head);


          depth_record.xyz.x = nxy.x;
          depth_record.xyz.y = nxy.y;
          depth_record.xyz.z = depth;

          depth_record.vertical_error = verr;
          depth_record.horizontal_error = herr;

          for (k = 0 ; k < NUM_ATTR ; k++) depth_record.attr[k] = l_attr[k];


          //  Note that we may be setting the reference flag for this data.

          depth_record.validity = flags | (load_parms[j].flags.ref * PFM_REFERENCE);

          depth_record.beam_number = beam;
          depth_record.ping_number = recnum;


          //  A change of over 120 degrees in heading from the last saved heading is treated as a line change.  This only works
          //  for GSF data.

          diff = fabs (heading - prev_heading[j]);
          if (line_count[j] < 0 || ((diff > 120.0) && (diff < 345.0)))
            {
              line_count[j]++;

              NV_INT32 year, jday, hour, minute;
              NV_FLOAT32 second;
              cvtime (tv_sec, tv_nsec, &year, &jday, &hour, &minute, &second);

              sprintf (line_time[line_count[j]], "%d-%03d-%02d:%02d:%02d", year + 1900, jday, hour, minute, NINT (second));

              prev_heading[j] = heading;
            }


          depth_record.line_number = line_count[j] + next_line[j];
          depth_record.file_number = file_number[j];


          //  If the depth is outside of the min and max depths, set it to the null depth (defined in the PFM API as
          //  max_depth + 1.0) and set the PFM_FILTER_INVAL and the PFM_MODIFIED bit that is used by the PFM API to signify that 
          //  the data must be saved to the input file after editing.  Note that these points will not be visible in the PFM editor.

          if (depth_record.xyz.z > pfm_def[j].max_depth || depth_record.xyz.z <= pfm_def[j].min_depth)
            {
              depth_record.xyz.z = pfm_def[j].max_depth + 1.0;


              //  Caveat - If we're loading HOF LIDAR null data and this is a null point we don't want to set this to invalid or modified 
              //  so that GCS can do reprocessing.

              if (!lidar_null) depth_record.validity |= (PFM_FILTER_INVAL | PFM_MODIFIED);

              total_out_of_limits++;
            }


          //  Turn off selected soundings in input data.

          depth_record.validity &= ~PFM_SELECTED_SOUNDING;


          //  Load the point.

          if ((status = add_cached_depth_record (pfm_def[j].hnd, &depth_record)))
            //if ((status = add_depth_record_index (pfm_def[j].hnd, &depth_record)))
            {
              if (status == WRITE_BIN_RECORD_DATA_READ_ERROR)
                {
                  fprintf (stderr, "%s\n", pfm_error_str (status));
                  fprintf (stderr, "%d %d %f %f %f %f\n\n", depth_record.coord.y, depth_record.coord.x,
                           pfm_def[j].open_args.head.mbr.min_y, pfm_def[j].open_args.head.mbr.max_y,
                           pfm_def[j].open_args.head.mbr.min_x, pfm_def[j].open_args.head.mbr.max_x);
                  fflush (stderr);
                }
              else
                {
                  pfm_error_exit (status);
                }
            }


          //  Set the add_map value to show that we have new data in this bin.

          pfm_def[j].add_map[depth_record.coord.y * pfm_def[j].open_args.head.bin_width + depth_record.coord.x] = 1;


          out_count[j]++;
        }
    }


  //  We don't want to eat up all of the free time on the system with a progress bar.

  if (percent - old_percent >= 5 || old_percent > percent)
    {
      prog->setValue (percent);
      old_percent = percent;
    }

  qApp->processEvents ();
}



/********************************************************************
 *
 * Function Name :  GSF_PFM_Processing
 *
 * Description :    This is called when GSF records are to be
 *               output to the PFM file. All GSF specific massaging is done
 *               and then passed to Do_PFM_Processing.
 *
 * Inputs : gsf_records - gsf data to be processed.
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 ********************************************************************/

static void GSF_PFM_Processing (gsfRecords * gsf_records, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[],
                                NV_INT32 percent)
{
  NV_INT32            i;
  NV_U_INT32          flags;
  NV_FLOAT64          lateral, ang1, ang2, lat, lon;
  NV_FLOAT32          dep;
  NV_F64_COORD2       xy2;
  NV_FLOAT32          herr, verr;
  NV_F64_COORD2       nxy[1024];


  void gsf_to_pfm_flags (NV_U_INT32 *pfm_flags, NV_U_CHAR gsf_flags);


  //  Set and then increment the record number.  

  lat = gsf_records->mb_ping.latitude;
  lon = gsf_records->mb_ping.longitude;
  ang1 = gsf_records->mb_ping.heading + 90.0;
  ang2 = gsf_records->mb_ping.heading;


  //  Make sure position and ping is valid.   

  if ((lat <= 90.0) && (lon <= 180.0) && !(gsf_records->mb_ping.ping_flags & GSF_IGNORE_PING))
    {
      //  Check for 1 beam multibeam.

      if (gsf_records->mb_ping.number_beams == 1)
        {
          herr = load_parms[0].pfm_global.horizontal_error;
          verr = load_parms[0].pfm_global.vertical_error;

          gsf_to_pfm_flags (&flags, gsf_records->mb_ping.beam_flags[0]);


          nxy[0].x = lon;
          nxy[0].y = lat;


          if (load_parms[0].flags.nom && gsf_records->mb_ping.nominal_depth != NULL)
            {
              dep = gsf_records->mb_ping.nominal_depth[0];
            }
          else
            {
              dep = gsf_records->mb_ping.depth[0];
            }


          if (load_parms[0].pfm_global.attribute_count) getGSFAttributes (attr, load_parms[0].pfm_global, gsf_records->mb_ping, 0);

          Do_PFM_Processing (1, dep, nxy[0], herr, verr, flags, ang2, attr, load_parms, pfm_def, percent, gsf_records->mb_ping.ping_time.tv_sec, 
                             gsf_records->mb_ping.ping_time.tv_nsec);
        }
      else
        {
          for (i = 0 ; i < gsf_records->mb_ping.number_beams ; i++) 
            {
              herr = 0.0;
              verr = 0.0;

              if (load_parms[0].flags.nom && gsf_records->mb_ping.nominal_depth != NULL)
                {
                  dep = gsf_records->mb_ping.nominal_depth[i];
                }
              else
                {
                  dep = gsf_records->mb_ping.depth[i];
                }

              if (dep != 0.0 && gsf_records->mb_ping.beam_flags != NULL && !(check_flag (gsf_records->mb_ping.beam_flags[i], HMPS_IGNORE_NULL_BEAM))) 
                {
                  //  Adjust for cross track position.  

                  lateral = gsf_records->mb_ping.across_track[i];
                  newgp (lat, lon, ang1, lateral, &nxy[i].y, &nxy[i].x);


                  //  if the along track array is present then use it 
                
                  if (gsf_records->mb_ping.along_track != (NV_FLOAT64 *) NULL) 
                    {
                      xy2.y = nxy[i].y;
                      xy2.x = nxy[i].x;
                      lateral = gsf_records->mb_ping.along_track[i];

                      newgp (xy2.y, xy2.x, ang2, lateral, &nxy[i].y, &nxy[i].x);
                    }


                  //  If the horizontal and vertical errors arrays are present then use them 

                  if (gsf_records->mb_ping.horizontal_error != (NV_FLOAT64 *) NULL) 
                    {
                      herr = (NV_FLOAT32) gsf_records->mb_ping.horizontal_error[i];
                    }
                  else
                    {
                      herr = load_parms[0].pfm_global.horizontal_error;
                    }

                  if (gsf_records->mb_ping.vertical_error != (NV_FLOAT64 *) NULL) 
                    {
                      verr = (NV_FLOAT32) gsf_records->mb_ping.vertical_error[i];
                    }
                  else
                    {
                      verr = load_parms[0].pfm_global.vertical_error;
                    }


                  gsf_to_pfm_flags (&flags, gsf_records->mb_ping.beam_flags[i]);


                  if (load_parms[0].pfm_global.attribute_count) getGSFAttributes (attr, load_parms[0].pfm_global, gsf_records->mb_ping, i);

                  Do_PFM_Processing (i + 1, dep, nxy[i], herr, verr, flags, ang2, attr, load_parms, pfm_def, percent, gsf_records->mb_ping.ping_time.tv_sec, 
                                     gsf_records->mb_ping.ping_time.tv_nsec);
                }
            }
        }
    }
}


NV_BOOL check_gsf_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  NV_INT32          gsf_handle;
  extern int        gsfError;


  prog = preCheck;


  //  Open the GSF file indexed so the library will generate another index file if needed.

  gsf_register_progress_callback (indexProgress);

  if (gsfOpen (path, GSF_READONLY_INDEX, &gsf_handle)) 
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      gsfPrintError (errfp);
      if (gsfError != GSF_FOPEN_ERROR) gsfClose (gsf_handle);
      return (NVFalse);
    }
  gsfClose (gsf_handle);

  prog->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_gsf_file
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_gsf_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                        PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  gsfDataID         gsf_data_id;
  gsfRecords        gsf_records;
  NV_INT32          i, j, gsf_handle, percent, records;
  time_t            sec;
  long              nsec;
  extern int        gsfError;
  NV_CHAR           temp[1024], line[1024], *ptr;
  NV_BOOL           nom_warning;


  nom_warning = NVFalse;


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;


  //  Open the GSF file indexed so we can get the number of records.

  if (gsfOpen (path, GSF_READONLY_INDEX, &gsf_handle)) 
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      gsfPrintError (errfp);
      if (gsfError != GSF_FOPEN_ERROR) gsfClose (gsf_handle);
      return (0);
    }


  //  Get the number of records in the file.

  records = gsfIndexTime (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, -1, &sec, &nsec);


  //  Get the next available line number for this file so we can generate our own lines from GSF data.

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
    }


  memset (&gsf_records, 0, sizeof (gsfRecords));
  memset (&gsf_data_id, 0, sizeof (gsfDataID));


  //  Read all of the data from this file.  

  for (i = 0 ; i < records ; i++)
    {
      gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
      gsf_data_id.record_number = i + 1;
      if (gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_records, NULL, 0) < 0) 
        {
          if (gsfError == GSF_READ_TO_END_OF_FILE) break;
          fprintf (errfp, "Error reading record - %s\n", gsfStringError ());
          fprintf (errfp,"Record %d skipped\n", recnum);
        }
      else
        {
          //  NOTE: GSF records start at 1 not 0.

          recnum = i + 1;

          percent = gsfPercent (gsf_handle);


          //  Check for nominal depth present.

          if (!nom_warning && load_parms[0].flags.nom && gsf_records.mb_ping.nominal_depth == NULL)
            {
              fprintf (errfp, "\nFor file %s\n", path);
              fprintf (errfp, "nominal depth requested, true depth used.\n");
              nom_warning = NVTrue;
            }


          GSF_PFM_Processing (&gsf_records, load_parms, pfm_def, percent);
        }
    }

  prog->setValue (100);


  gsfClose (gsf_handle);


  //  If we actually wrote any data, write the input file name to the list file.   

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      if (out_count[i]) 
        {
          //  May be MSYS on Windoze so we have to check for either separator.

          if (strrchr (path, '/'))
            {
              ptr = strrchr (path, '/');
            }
          else if (strrchr (path, '\\'))
            {
              ptr = strrchr (path, '\\');
            }
          else
            {
              ptr = NULL;
            }


          if (ptr == NULL) 
            {
              strcpy (temp, path);
            }
          else
            {
              strcpy (temp, (ptr + 1));
            }

          for (j = 0 ; j <= line_count[i] ; j++)
            {
              sprintf (line, "%s-%s", temp, line_time[j]);
              write_line_file (pfm_def[i].hnd, line);
            }


          //  Convert to native separators so that we get nice file names on Windows.

          nativePath = QDir::toNativeSeparators (QString (path));
          strcpy (native_path, nativePath.toAscii ());

          write_list_file (pfm_def[i].hnd, native_path, PFM_GSF_DATA);
          count[i] += out_count[i];
        }
    }

  return (total_out_of_limits);
}



NV_BOOL check_hof_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  FILE               *hof_fp;
  HOF_HEADER_T       head;


  //  Open the SHOALS .hof file.

  if ((hof_fp = open_hof_file (path)) != NULL)
    {
      hof_read_header (hof_fp, &head);

      if (!strstr (head.text.file_type, "Hydrographic Output File"))
        {
          fprintf (errfp, "\n\nFile %s is not a .hof file.\n", path);
          fclose (hof_fp);
          return (NVFalse);
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open the SHOALS .hof file.\n");
      fprintf (errfp, "%s : %s\n", path, strerror (errno));
      return (NVFalse);
    }


  fclose (hof_fp);

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_hof_file
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_hof_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                        PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam;
  FILE               *hof_fp;
  HYDRO_OUTPUT_T     hof;
  HOF_HEADER_T       head;
  NV_INT32           i, percent;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth;
  NV_U_INT32         flags;
  NV_FLOAT32         eof;
  NV_CHAR            line[256];
  NV_INT32           data_type = PFM_CHARTS_HOF_DATA;


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;


  //  HOF records start at 1.

  recnum = 1;


  //  Open the HOF files and read the data.

  if ((hof_fp = open_hof_file (path)) != NULL)
    {
      fseek (hof_fp, 0, SEEK_END);
      eof = ftell (hof_fp);
      fseek (hof_fp, 0, SEEK_SET);


      hof_read_header (hof_fp, &head);


      //  Read all of the data from this file.  

      while (hof_read_record (hof_fp, HOF_NEXT_RECORD, &hof))
        {
          //  Get the start time for the line name.

          if (recnum == 1)
            {
              NV_INT32 year, jday, hour, minute;
              NV_FLOAT32 second;
              time_t tv_sec = hof.timestamp / 1000000;
              long tv_nsec = (hof.timestamp % 1000000) * 1000;
              cvtime (tv_sec, tv_nsec, &year, &jday, &hour, &minute, &second);

              sprintf (line, "Line %s %d-%03d-%02d:%02d:%02d", head.text.flightline_number, year + 1900, jday, hour, minute, NINT (second));
            }


          percent = (NV_INT32) roundf (((NV_FLOAT32) (ftell (hof_fp)) / eof) * 100.0);


          //  Check to see if we want to load HOF data in the old PFM_SHOALS_1K_DATA form.

          if (load_parms[0].flags.old)
            {
              data_type = PFM_SHOALS_1K_DATA;

              beam = 0;

              xy.y = hof.latitude;
              xy.x = hof.longitude;


              lidar_null = NVFalse;


              //  We'll allow 0 ABDC if we're loading NULL points.

              if (hof.abdc || load_parms[0].flags.hof)
                {
                  //  HOF uses the lower three bits of the status field for status thusly :

                  //  bit 0 = deleted    (1) 
                  //  bit 1 = kept       (2) 
                  //  bit 2 = swapped    (4)


                  flags = 0;
                  if (hof.status & AU_STATUS_DELETED_BIT) flags = PFM_FILTER_INVAL;


                  //  If the absolute value of the abdc is below 70 we mark it as filter invalid.  If the value is
                  //  below 70 (like a -93) but the absolute value is above 70 we will mark it as manually invalid.

                  if (hof.abdc < 70)
                    {
                      if (abs (hof.abdc) < 70)
                        {
                          flags = PFM_FILTER_INVAL;
                        }
                      else
                        {
                          flags = PFM_MANUALLY_INVAL;
                        }
                    }


                  //  Invalidate HOF land flags.

                  if (load_parms[0].flags.lnd && hof.abdc == 70) flags = (PFM_FILTER_INVAL | PFM_MODIFIED);


                  if (hof.suspect_status & SUSPECT_STATUS_SUSPECT_BIT) flags |= PFM_SUSPECT;
                  if (hof.suspect_status & SUSPECT_STATUS_FEATURE_BIT) flags |= PFM_SELECTED_FEATURE;


                  //  Maybe in the future ;-)

                  //  HOF classification status :
                  //  bit 0 = classified        (1=classified; 0=not classified)
                  //  bit 1 = non-bare_earth1   (0x02);   
                  //  bit 2 = non-bare_earth2   (0x04);
                  //  bit 3 = water             (0x08);   

                  //  if ((hof.classification_status & 0x01) && (!(hof.classification_status & 0x0e))) flags |= PFM_USER_01;


                  if (load_parms[0].flags.lid)
                    {
                      //  Flag "shallow water processed" or "shoreline depth swapped" HOF data (these two are mututally exclusive in practice).

                      if (abs (hof.abdc) == 74 || abs (hof.abdc) == 72) flags |= PFM_USER_01;


                      //  Flag "APD" data

                      if (hof.bot_channel) flags |= PFM_USER_02;


                      //  Flag "land" data

                      if (abs (hof.abdc) == 70) flags |= PFM_USER_03;
                    }


                  if (hof.correct_depth <= -998.0)
                    {
                      if (load_parms[0].flags.hof)
                        {
                          //  We're setting the null (-998.0) depths to -999999.0 to put them outside any sane bounds.
                          //  This will cause them to be set to the null depth for whatever PFM they fall in.  We
                          //  want to keep them so that the Optech GCS software can reprocess them if possible.

                          lidar_null = NVTrue;

                          if (load_parms[0].pfm_global.attribute_count) getHOFAttributes (attr, load_parms[0].pfm_global, hof);

                          depth = -999999.0;

                          Do_PFM_Processing (beam, depth, xy, 0.0, 0.0, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                        }
                    }
                  else
                    {
                      hof_get_uncertainty (hof, &herr, &verr, hof.correct_depth, hof.abdc);

                      if (load_parms[0].pfm_global.attribute_count) getHOFAttributes (attr, load_parms[0].pfm_global, hof);


                      if (load_parms[0].flags.lid && hof.correct_sec_depth > -998.0) flags |= PFM_USER_04;


                      depth = -hof.correct_depth;

                      Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                    }
                }
            }
          else
            {
              data_type = PFM_CHARTS_HOF_DATA;

              lidar_null = NVFalse;


              if (hof.abdc)
                {
                  //  HOF uses the lower three bits of the status field for status thusly :
                  //
                  //  bit 0 = deleted    (1) 
                  //  bit 1 = kept       (2) 
                  //  bit 2 = swapped    (4)


                  flags = 0;
                  if (hof.status & AU_STATUS_DELETED_BIT) flags = PFM_FILTER_INVAL;


                  //  If the absolute value of the abdc is below 70 we mark it as filter invalid.  If the value is
                  //  below 70 (like a -93) but the absolute value is above 70 we will mark it as manually invalid.

                  if (hof.abdc < 70)
                    {
                      if (abs (hof.abdc) < 70)
                        {
                          flags = PFM_FILTER_INVAL;
                        }
                      else
                        {
                          flags = PFM_MANUALLY_INVAL;
                        }
                    }


                  //  Invalidate HOF land flags.

                  if (load_parms[0].flags.lnd && hof.abdc == 70) flags = (PFM_FILTER_INVAL | PFM_MODIFIED);

                  if (hof.suspect_status & SUSPECT_STATUS_SUSPECT_BIT) flags |= PFM_SUSPECT;
                  if (hof.suspect_status & SUSPECT_STATUS_FEATURE_BIT) flags |= PFM_SELECTED_FEATURE;


                  if (load_parms[0].flags.lid)
                    {
                      //  Flag "shallow water processed" or "shoreline depth swapped" HOF data (these two are mututally exclusive in practice).

                      if (abs (hof.abdc) == 74 || abs (hof.abdc) == 72) flags |= PFM_USER_01;


                      //  Flag "APD" data

                      if (hof.bot_channel) flags |= PFM_USER_02;


                      //  Flag "land" data

                      if (abs (hof.abdc) == 70) flags |= PFM_USER_03;
                    }


                  //  The same attributes will be loaded for primary and secondary returns.

                  if (load_parms[0].pfm_global.attribute_count) getHOFAttributes (attr, load_parms[0].pfm_global, hof);


                  beam = 0;
                  xy.y = hof.latitude;
                  xy.x = hof.longitude;


                  if (hof.correct_depth <= -998.0)
                    {
                      if (load_parms[0].flags.hof)
                        {
                          //  We're setting the null (-998.0) depths to -999999.0 to put them outside any sane bounds.  This will cause
                          //  them to be set to the null depth for whatever PFM they fall in.  We want to keep them just to see coverage.

                          lidar_null = NVTrue;

                          depth = -999999.0;

                          Do_PFM_Processing (beam, depth, xy, 0.0, 0.0, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                        }
                    }
                  else
                    {
                      hof_get_uncertainty (hof, &herr, &verr, hof.correct_depth, hof.abdc);

                      if (load_parms[0].flags.lid && hof.correct_sec_depth > -998.0) flags |= PFM_USER_04;

                      depth = -hof.correct_depth;

                      Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                    }


                  if (hof.correct_sec_depth > -998.0)
                    {
                      flags = 0;


                      //  If the absolute value of the abdc is below 70 we mark it as filter invalid.  If the value is
                      //  below 70 (like a -93) but the absolute value is above 70 we will mark it as manually invalid.

                      if (hof.sec_abdc < 70)
                        {
                          if (abs (hof.sec_abdc) < 70)
                            {
                              flags = PFM_FILTER_INVAL;
                            }
                          else
                            {
                              flags = PFM_MANUALLY_INVAL;
                            }
                        }


                      //  Invalidate HOF land flags.

                      if (load_parms[0].flags.lnd && hof.sec_abdc == 70) flags = (PFM_FILTER_INVAL | PFM_MODIFIED);


                      //  Invalidate HOF secondary returns.

                      if (load_parms[0].flags.sec) flags = (PFM_FILTER_INVAL | PFM_MODIFIED);


                      if (load_parms[0].flags.lid)
                        {
                          //  Flag "shallow water processed" or "shoreline depth swapped" HOF data (these two are mututally exclusive in practice).

                          if (abs (hof.sec_abdc) == 74 || abs (hof.sec_abdc) == 72) flags |= PFM_USER_01;


                          //  Flag "APD" data

                          if (hof.sec_bot_chan) flags |= PFM_USER_02;


                          //  Flag "land" data

                          if (abs (hof.sec_abdc) == 70) flags |= PFM_USER_03;
                        }


                      hof_get_uncertainty (hof, &herr, &verr, hof.correct_sec_depth, hof.sec_abdc);

                      if (load_parms[0].flags.lid) flags |= PFM_USER_04;


                      beam = 1;
                      xy.y = hof.sec_latitude;
                      xy.x = hof.sec_longitude;
                      depth = -hof.correct_sec_depth;

                      Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                    }
                }
            }
          recnum++;
        }

      prog->setValue (100);


      fclose (hof_fp);


      //  If we actually wrote any data, write the input file name to the list file.   

      for (i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              write_list_file (pfm_def[i].hnd, native_path, data_type);
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      return (0);
    }


  //  We don't return out of range values for HOF data since we load the -998.0 (NBR) values as null depths.

  total_out_of_limits = 0;
  return (total_out_of_limits);
}



NV_BOOL check_tof_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  FILE               *tof_fp;
  TOF_HEADER_T       head;


  //  Open the SHOALS .tof file.

  if ((tof_fp = open_tof_file (path)) != NULL)
    {
      tof_read_header (tof_fp, &head);

      if (!strstr (head.text.file_type, "Topographic Output File"))
        {
          fprintf (errfp, "\n\nFile %s is not a .tof file.\n", path);
          fclose (tof_fp);
          return (NVFalse);
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open the SHOALS .tof file.\n");
      fprintf (errfp, "%s : %s\n", path, strerror (errno));
      return (NVFalse);
    }


  fclose (tof_fp);

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_tof_file
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_tof_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                        PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam;
  FILE               *tof_fp;
  TOPO_OUTPUT_T      tof;
  TOF_HEADER_T       head;
  NV_INT32           i, percent;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth;
  NV_U_INT32         flags;
  NV_FLOAT32         eof;
  NV_CHAR            line[256];


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;


  //  TOF records now start at 1 as per IVS so we'll be compatible with Optech GCS and Fledermaus.

  recnum = 1;


  //  Open the TOF files and read the data.

  if ((tof_fp = open_tof_file (path)) != NULL)
    {
      fseek (tof_fp, 0, SEEK_END);
      eof = ftell (tof_fp);
      fseek (tof_fp, 0, SEEK_SET);


      tof_read_header (tof_fp, &head);


      //  Check for files older than 10/07/2011.  At that point we changed the way we load TOF data into PFM
      //  so that we load first returns even if the second return is bad.

      NV_BOOL ver_dep_flag = NVFalse;
      NV_INT64 hd_start_time;
      sscanf (head.text.start_time, NV_INT64_SPECIFIER, &hd_start_time);
      if (hd_start_time < 1317945600000000LL) ver_dep_flag = NVTrue;


      //  Read all of the data from this file.  

      while (tof_read_record (tof_fp, TOF_NEXT_RECORD, &tof))
        {
          //  Get the start time for the line name.

          if (recnum == 1)
            {
              NV_INT32 year, jday, hour, minute;
              NV_FLOAT32 second;
              time_t tv_sec = tof.timestamp / 1000000;
              long tv_nsec = (tof.timestamp % 1000000) * 1000;
              cvtime (tv_sec, tv_nsec, &year, &jday, &hour, &minute, &second);

              sprintf (line, "Line %s %d-%03d-%02d:%02d:%02d", head.text.flightline_number, year + 1900, jday, hour, minute, NINT (second));
            }


          herr = load_parms[0].pfm_global.horizontal_error;
          verr = load_parms[0].pfm_global.vertical_error;


          percent = (NV_INT32) roundf (((NV_FLOAT32) (ftell (tof_fp)) / eof) * 100.0);


          if (load_parms[0].pfm_global.attribute_count) getTOFAttributes (attr, load_parms[0].pfm_global, tof);


          //  If the difference between the first and last returns is less than 5 cm we will not load the first 
          //  return and will give it the last return validity on unload.

          if (tof.elevation_first > -998.0 && fabs ((NV_FLOAT64) (tof.elevation_last - tof.elevation_first)) > 0.05)
            {
              //  We don't load first returns if the last return was bad in files prior to 10/07/2011.

              if (ver_dep_flag && tof.elevation_last > -998.0)
                {
                  beam = 0;

                  flags = 0;
                  if (load_parms[0].flags.tof) flags = PFM_REFERENCE;

                  if (tof.conf_first < 50) flags = PFM_FILTER_INVAL;

                  xy.y = tof.latitude_first;
                  xy.x = tof.longitude_first;

                  depth = -tof.elevation_first;

                  Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                }
            }


          if (tof.elevation_last > -998.0) 
            {
              //  TOF uses the lower two bits of the status field for status thusly :
              //
              //  bit 0 = first deleted    (1) 
              //  bit 1 = second deleted   (2)

              beam = 1;

              flags = 0;

              if (tof.conf_last < 50) flags = PFM_FILTER_INVAL;


              xy.y = tof.latitude_last;
              xy.x = tof.longitude_last;

              depth = -tof.elevation_last;

              Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
            }

          recnum++;
        }

      prog->setValue (100);


      fclose (tof_fp);


      //  If we actually wrote any data, write the input file name to the list file.   

      for (i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              write_list_file (pfm_def[i].hnd, native_path, PFM_SHOALS_TOF_DATA);
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      return (0);
    }

  return (total_out_of_limits);
}



NV_BOOL check_wlf_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  NV_INT32           wlf_handle;
  WLF_HEADER         wlf_header;


  //  Open the WLF file.

  if ((wlf_handle = wlf_open_file (path, &wlf_header, WLF_READONLY)) < 0)
    {
      fprintf (errfp, wlf_strerror ());
      return (NVFalse);
    }


  wlf_close_file (wlf_handle);

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_wlf_file
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_wlf_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                        PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam;
  NV_INT32           wlf_handle;
  WLF_RECORD         wlf;
  WLF_HEADER         head;
  NV_INT32           i, percent;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth;
  NV_U_INT32         flags;
  NV_FLOAT32         eof;
  NV_CHAR            line[256];


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;


  //  Starting at the correct record number.

  recnum = 0;


  //  Open the WLF files and read the data.

  if ((wlf_handle = wlf_open_file (path, &head, WLF_READONLY)) >= 0)
    {
      eof = head.number_of_records;


      //  Read all of the data from this file.  

      for (i = 0 ; i < eof ; i++)
        {
          if (wlf_read_record (wlf_handle, i, &wlf, NVFalse, NULL) >= 0)
            {
              //  Get the start time for the line name.

              if (recnum == 1)
                {
                  NV_INT32 year, jday, hour, minute;
                  NV_FLOAT32 second;
                  cvtime (wlf.tv_sec, wlf.tv_nsec, &year, &jday, &hour, &minute, &second);

                  sprintf (line, "Line %s %d-%03d-%02d:%02d:%02d", head.flight_id, year + 1900, jday, hour, minute, NINT (second));
                }


              percent = NINT (((NV_FLOAT32) i / eof) * 100.0);

              if (wlf.z > -998.0)
                {
                  xy.y = wlf.y;
                  xy.x = wlf.x;


                  flags = 0;
                  if (wlf.status & WLF_MANUALLY_INVAL) flags = PFM_MANUALLY_INVAL;
                  if (wlf.status & WLF_FILTER_INVAL) flags = PFM_FILTER_INVAL;
                  if (wlf.status & WLF_SUSPECT) flags |= PFM_SUSPECT;
                  if (wlf.status & WLF_SELECTED_SOUNDING) flags |= PFM_SELECTED_SOUNDING;
                  if (wlf.status & WLF_SELECTED_FEATURE) flags |= PFM_SELECTED_FEATURE;
                  if (wlf.status & WLF_DESIGNATED_SOUNDING) flags |= PFM_DESIGNATED_SOUNDING;

                  beam = wlf.return_number;

                  herr = wlf.horizontal_uncertainty;
                  verr = wlf.vertical_uncertainty;

                  if (load_parms[0].pfm_global.attribute_count) getWLFAttributes (attr, load_parms[0].pfm_global, wlf);

                  lidar_null = NVFalse;
                  depth = -wlf.z;

                  Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                }
            }
          else
            {
              fprintf (errfp, "%s\n", wlf_strerror ());
            }
          recnum++;
        }

      prog->setValue (100);


      wlf_close_file (wlf_handle);


      //  If we actually wrote any data, write the input file name to the list file.   

      for (i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              write_list_file (pfm_def[i].hnd, native_path, PFM_WLF_DATA);
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, wlf_strerror ());
      return (0);
    }


  //  We don't return out of range values for HOF data since we load the -998.0 (NBR) values as null depths.

  total_out_of_limits = 0;
  return (total_out_of_limits);
}



NV_BOOL check_hawkeye_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  NV_INT32                hawkeye_handle;
  HAWKEYE_META_HEADER     *hawkeye_meta_header;
  HAWKEYE_CONTENTS_HEADER *hawkeye_contents_header;


  //  Open the HAWKEYE file.

  if ((hawkeye_handle = hawkeye_open_file (path, &hawkeye_meta_header, &hawkeye_contents_header, HAWKEYE_READONLY)) < 0)
    {
      fprintf (errfp, hawkeye_strerror ());
      return (NVFalse);
    }


  //  Check for the required fields.

  if (hawkeye_contents_header->data_type != HAWKEYE_HYDRO_BIN_DATA)
    {
      if (!hawkeye_contents_header->available.Timestamp || !hawkeye_contents_header->available.Point_Latitude ||
          !hawkeye_contents_header->available.Point_Longitude || !hawkeye_contents_header->available.Point_Altitude ||
          !hawkeye_contents_header->available.Manual_Output_Screening_Flags || !hawkeye_contents_header->available.Return_Number)
        {
          fprintf (errfp, "Missing required point record fields in HAWKEYE file %s\n", path);
          hawkeye_close_file (hawkeye_handle);
          return (NVFalse);
        }
    }
  else
    {
      if (!hawkeye_contents_header->available.Timestamp || !hawkeye_contents_header->available.Point_Latitude ||
          !hawkeye_contents_header->available.Point_Longitude || !hawkeye_contents_header->available.Depth ||
          !hawkeye_contents_header->available.Depth_Confidence || !hawkeye_contents_header->available.Manual_Output_Screening_Flags ||
          !hawkeye_contents_header->available.Return_Number)
        {
          fprintf (errfp, "Missing required point record fields in HAWKEYE file %s\n", path);
          hawkeye_close_file (hawkeye_handle);
          return (NVFalse);
        }
    }


  hawkeye_close_file (hawkeye_handle);

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_hawkeye_file
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_hawkeye_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                            PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32                    beam;
  NV_INT32                    hawkeye_handle;
  HAWKEYE_RECORD              hawkeye;
  HAWKEYE_META_HEADER         *meta_header;
  HAWKEYE_CONTENTS_HEADER     *contents_header;
  NV_INT32                    i, percent, data_type;
  NV_F64_COORD2               xy;
  NV_FLOAT32                  herr, verr;
  NV_FLOAT64                  depth;
  NV_U_INT32                  flags;
  NV_FLOAT32                  eof;
  NV_CHAR                     line[256], *ptr;


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;


  //  Starting at the correct record number.

  recnum = 0;


  //  Open the HAWKEYE files and read the data.

  if ((hawkeye_handle = hawkeye_open_file (path, &meta_header, &contents_header, HAWKEYE_READONLY)) >= 0)
    {
      data_type = contents_header->data_type;

      eof = contents_header->NbrOfPointRecords;


      //  Read all of the data from this file.  

      for (i = 0 ; i < (NV_INT32) eof ; i++)
        {
          if (!hawkeye_read_record (hawkeye_handle, i, &hawkeye))
            {
              percent = NINT (((NV_FLOAT32) i / eof) * 100.0);

              xy.y = hawkeye.Point_Latitude;
              xy.x = hawkeye.Point_Longitude;
              beam = hawkeye.Return_Number;
              herr = load_parms[0].pfm_global.horizontal_error;
              verr = load_parms[0].pfm_global.vertical_error;
              if (load_parms[0].pfm_global.attribute_count) getHAWKEYEAttributes (attr, load_parms[0].pfm_global, hawkeye);
              lidar_null = NVFalse;
              flags = 0;

              if (data_type == HAWKEYE_HYDRO_BIN_DATA)
                {
                  if (hawkeye.Depth < 99.0)
                    {
                      if (hawkeye.Depth_Confidence <= 0.0)
                        {
                          flags = PFM_FILTER_INVAL;
                        }
                      else if (hawkeye.Manual_Output_Screening_Flags & HAWKEYE_INVALID)
                        {
                          flags = PFM_MANUALLY_INVAL;
                        }

                      depth = hawkeye.Depth;

                      Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                    }
                }
              else
                {
                  if (hawkeye.Manual_Output_Screening_Flags & HAWKEYE_INVALID) flags = PFM_MANUALLY_INVAL;

                  depth = -hawkeye.Point_Altitude;

                  Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                }
            }
          else
            {
              fprintf (errfp, "%s\n", hawkeye_strerror ());
            }
          recnum++;
        }

      prog->setValue (100);


      hawkeye_close_file (hawkeye_handle);


      //  If we actually wrote any data, write the input file name to the list file.   

      for (i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              //  May be MSYS on Windoze so we have to check for either separator.

              if (strrchr (path, '/'))
                {
                  ptr = strrchr (path, '/');
                }
              else if (strrchr (path, '\\'))
                {
                  ptr = strrchr (path, '\\');
                }
              else
                {
                  ptr = NULL;
                }


              if (ptr == NULL) 
                {
                  strcpy (line, path);
                }
              else
                {
                  strcpy (line, (ptr + 1));
                }

              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              if (data_type == HAWKEYE_HYDRO_BIN_DATA)
                {
                  write_list_file (pfm_def[i].hnd, native_path, PFM_HAWKEYE_HYDRO_DATA);
                }
              else
                {
                  write_list_file (pfm_def[i].hnd, native_path, PFM_HAWKEYE_TOPO_DATA);
                }
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, hawkeye_strerror ());
      return (0);
    }


  //  We don't return out of range values for HOF data since we load the -998.0 (NBR) values as null depths.

  total_out_of_limits = 0;
  return (total_out_of_limits);
}



NV_BOOL check_unisips_depth_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  FILE               *uni_fp;
  NV_INT32           swap;
  UNISIPS_HEADER     unisips_header;


  //  Open the UNISIPS file.

  uni_fp = fopen (path, "rb");


  // Read UNISIPS header information

  if (!read_unisips_header (uni_fp, &unisips_header, &swap))
    {
      fprintf (errfp, "\n\nUnable to open the UNISIPS file.\n");
      fprintf (errfp, "%s : %s\n", path, strerror (errno));
      return (NVFalse);
    }

  fclose (uni_fp);

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_unisips_depth_file
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_unisips_depth_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, 
                                  FILE *errfp, PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  FILE               *uni_fp;
  UNISIPS_HEADER     unisips_header;
  UNISIPS_REC_HDR    unisips_rec_hdr;
  UNISIPS_REC_SUBHDR unisips_rec_subhdr;
  NV_INT32           swap, row, i, percent, height, width;
  NV_CHAR            line[512], *ptr;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth;
  NV_FLOAT32         tide;
  NV_U_INT32         flags;


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;

  recnum = 0;


  //  Open the UNISIPS file and read the data.

  if ((uni_fp = fopen (path, "r")) != NULL)
    {
      //  Read UNISIPS header information

      if (read_unisips_header (uni_fp, &unisips_header, &swap))
        {
          height = unisips_header.rows;
          width = unisips_header.cols;


          //  Loop thru data reading record header to find needed information...

          for (row = 0 ; row < unisips_header.rows ; row++)
            {
              herr = load_parms[0].pfm_global.horizontal_error;
              verr = load_parms[0].pfm_global.vertical_error;


              percent = (NV_INT32) roundf (((NV_FLOAT32) row / (NV_FLOAT32) unisips_header.rows) * 100.0);

              read_unisips_rec_hdr (uni_fp, &unisips_header, &unisips_rec_hdr, &unisips_rec_subhdr, row, swap);


              //  Don't consider if the centerbeam depth is 0

              if (unisips_rec_hdr.cb_depth != 0)
                {
                  //  This is about the only way we have to deal with the junk in what used to be pad data in unisips record
                  //  headers.  The assumption is that, since we don't use these 4 bits, if they have something set they're
                  //  probably wrong.  The possibility still exists that all of the other bits got chewed but not these.

                  if (unisips_rec_hdr.tide_v & 0x003c) unisips_rec_hdr.tide_v = 0;


                  flags = unisips_rec_hdr.tide_v & 0x0003;


                  //  Combine the 'unisips_rec_hdr.cbdepth' and 'unisips_rec_hdr.tow_depth' value

                  depth = unisips_rec_hdr.cb_depth + unisips_rec_hdr.tow_depth;


                  tide = unisips_rec_hdr.tide_v >> 6;

                  if (tide) depth -= ((NV_FLOAT32) (tide - 511) / 10.0);


                  xy.x = unisips_rec_hdr.lon;
                  xy.y = unisips_rec_hdr.lat;

                  Do_PFM_Processing (0, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                }

              recnum++;
            }

          prog->setValue (100);

          fclose (uni_fp);
        }


      //  If we actually wrote any data, write the input file name to the list file.   

      for (i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              //  May be MSYS on Windoze so we have to check for either separator.

              if (strrchr (path, '/'))
                {
                  ptr = strrchr (path, '/');
                }
              else if (strrchr (path, '\\'))
                {
                  ptr = strrchr (path, '\\');
                }
              else
                {
                  ptr = NULL;
                }


              if (ptr == NULL) 
                {
                  strcpy (line, path);
                }
              else
                {
                  strcpy (line, (ptr + 1));
                }

              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              write_list_file (pfm_def[i].hnd, native_path, PFM_UNISIPS_DEPTH_DATA);
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      return (0);
    }
    
  return (total_out_of_limits);
}



NV_BOOL check_hypack_xyz_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  FILE               *xyz_fp;


  //  Open the hypack file.

  if ((xyz_fp = fopen (path, "r")) == NULL)
    {
      fprintf (errfp, "\n\nUnable to open the ASCII file.\n");
      fprintf (errfp, "%s : %s\n", path, strerror (errno));
      return (NVFalse);
    }

  fclose (xyz_fp);

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_hypack_xyz_file
 *
 * Description : Reads a Hypack xyz file in the following format:
 *
 *                DDD-MM-SS.SSSs,DDD-MM-SS.SSSs,DD.DDD
 *
 *               Example:
 *
 *                013-27-18.771N,144-37-41.383E,53.230
 *
 *               Last field is not fixed size so we can only read these
 *               not update (like I care).
 *
 *               Or it will read files derived from Hypack RAW files that
 *               are in the following format:
 *
 * YEAR DAY HH:MM:SS.SSSS   DD.ddddddddd  DDD.ddddddddd   depth   depth   depth
 * 2001 017 03:25:10.0620   13.430910402  144.660955271    7.06    8.77    8.02
 *
 *                All fields that start with a # are comments.  Third depth is
 *                tide corrected.
 *
 *               Or it will read YXZ files in the following formats:
 *
 * [signed] lat degrees decimal  [signed] lon degrees decimal  depth
 *
 *               or
 *
 * [signed] lat degrees decimal,[signed] lon degrees decimal,depth
 *
 *               Example:
 *
 *                21.000278 -157.619722 223
 *
 *               or
 *
 *                21.000278,-157.619722,223
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_hypack_xyz_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                               PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam, eof, latdeg, londeg, latmin, lonmin, i, percent, year, day, hour, minute;
  NV_FLOAT32         second, dep, dep2;
  FILE               *xyz_fp;
  NV_CHAR            string[256], cut[50], line[256], *ptr;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth, latsec, lonsec;
  NV_U_INT32         flags;


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;

  recnum = 0;


  //  Open the Hypack xyz files and read the data.

  if ((xyz_fp = fopen (path, "r")) != NULL)
    {
      fseek (xyz_fp, 0, SEEK_END);
      eof = ftell (xyz_fp);
      fseek (xyz_fp, 0, SEEK_SET);


      //  Read all of the data from this file.  

      while (fgets (string, sizeof (string), xyz_fp) != NULL)
        {
          herr = load_parms[0].pfm_global.horizontal_error;
          verr = load_parms[0].pfm_global.vertical_error;


          percent = (NV_INT32) roundf (((NV_FLOAT32) (ftell (xyz_fp)) / eof) * 100.0);

          if (string[0] != '#')
            {
              if (strchr (string, ':'))
                {
                  sscanf (string, "%d %d %d:%d:%f %lf %lf %f %f %lf", &year, &day, &hour, &minute, &second, &xy.y, 
                          &xy.x, &dep, &dep2, &depth);
                }
              else if (string[3] == '-' && string[6] == '-')
                {
                  strncpy (cut, &string[1], 13);
                  cut[13] = 0;
                  sscanf (cut, "%03d-%02d-%lf", &latdeg, &latmin, &latsec);

                  strncpy (cut, &string[15], 13);
                  cut[13] = 0;
                  sscanf (cut, "%03d-%02d-%lf", &londeg, &lonmin, &lonsec);

                  strcpy (cut, &string[30]);
                  sscanf (cut, "%lf", &depth);


                  xy.y = (NV_FLOAT64) latdeg + (NV_FLOAT64) latmin / 60.0 + latsec / 3600.0;
                  if (strchr (string, 'S') || strchr (string, 's')) xy.y = -xy.y;
                  xy.x = (NV_FLOAT64) londeg + (NV_FLOAT64) lonmin / 60.0 + lonsec / 3600.0;
                  if (strchr (string, 'W') || strchr (string, 'w')) xy.x = -xy.x;
                }
              else
                {
                  if (strchr (string, ','))
                    {
                      sscanf (string, "%lf,%lf,%lf", &xy.y, &xy.x, &depth);
                    }
                  else
                    {
                      sscanf (string, "%lf %lf %lf", &xy.y, &xy.x, &depth);
                    }
                }


              beam = 0;

              flags = 0;


              Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);

              recnum++;
            }
        }

      prog->setValue (100);


      fclose (xyz_fp);


      //  If we actually wrote any data, write the input file name to the list file.   

      for (i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              //  May be MSYS on Windoze so we have to check for either separator.

              if (strrchr (path, '/'))
                {
                  ptr = strrchr (path, '/');
                }
              else if (strrchr (path, '\\'))
                {
                  ptr = strrchr (path, '\\');
                }
              else
                {
                  ptr = NULL;
                }

          
              if (ptr == NULL) 
                {
                  strcpy (line, path);
                }
              else
                {
                  strcpy (line, (ptr + 1));
                }

              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              write_list_file (pfm_def[i].hnd, native_path, PFM_NAVO_ASCII_DATA);
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      return (0);
    }

  return (total_out_of_limits);
}



NV_BOOL check_ivs_xyz_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  FILE               *xyz_fp;


  //  Open the IVS file.

  if ((xyz_fp = fopen (path, "r")) == NULL)
    {
      fprintf (errfp, "\n\nUnable to open the ASCII file.\n");
      fprintf (errfp, "%s : %s\n", path, strerror (errno));
      return (NVFalse);
    }

  fclose (xyz_fp);

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_ivs_xyz_file
 *
 * Description : Reads an IVS xyz file in the following format:
 *
 *                LON LAT DEPTH
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_ivs_xyz_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                            PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam, eof, i, percent;
  FILE               *xyz_fp;
  NV_CHAR            string[256], line[256], *ptr;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth;
  NV_U_INT32         flags;


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;

  recnum = 0;


  //  Open the IVS xyz files and read the data.

  if ((xyz_fp = fopen (path, "r")) != NULL)
    {
      fseek (xyz_fp, 0, SEEK_END);
      eof = ftell (xyz_fp);
      fseek (xyz_fp, 0, SEEK_SET);


      //  Read all of the data from this file.  

      while (fgets (string, sizeof (string), xyz_fp) != NULL)
        {
          herr = load_parms[0].pfm_global.horizontal_error;
          verr = load_parms[0].pfm_global.vertical_error;


          percent = (NV_INT32) roundf (((NV_FLOAT32) (ftell (xyz_fp)) / eof) * 100.0);

          if (string[0] != '#')
            {
              sscanf (string, "%lf %lf %lf", &xy.x, &xy.y, &depth);

              beam = 0;

              flags = 0;


              Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);

              recnum++;
            }
        }

      fileInput->setValue (100);

      fclose (xyz_fp);


      //  If we actually wrote any data, write the input file name to the list file.   

      for (i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              //  May be MSYS on Windoze so we have to check for either separator.

              if (strrchr (path, '/'))
                {
                  ptr = strrchr (path, '/');
                }
              else if (strrchr (path, '\\'))
                {
                  ptr = strrchr (path, '\\');
                }
              else
                {
                  ptr = NULL;
                }


              if (ptr == NULL) 
                {
                  strcpy (line, path);
                }
              else
                {
                  strcpy (line, (ptr + 1));
                }

              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              write_list_file (pfm_def[i].hnd, native_path, PFM_NAVO_ASCII_DATA);
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      return (0);
    }

  return (total_out_of_limits);
}



NV_BOOL check_llz_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  NV_INT32 llz_hnd;
  LLZ_HEADER llz_header;


  //  Open the llz file.

  if ((llz_hnd = open_llz (path, &llz_header)) < 0)
    {
      fprintf (errfp, "\n\nUnable to open the LLZ file.\n");
      fprintf (errfp, "%s : %s\n", path, strerror (errno));
      return (NVFalse);
    }

  if (!strstr (llz_header.version, "llz library"))
    {
      fprintf (errfp, "\n\nLLZ file version corrupt or %s is not an LLZ file.\n", path);
      return (NVFalse);
    }      

  close_llz (llz_hnd);

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_llz_file
 *
 * Description : Reads a NAVO LLZ file.  See llz.h for format.
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_llz_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                        PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam, i, percent, llz_hnd;
  NV_U_INT32         flags;
  LLZ_HEADER         llz_header;
  LLZ_REC            llz;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_CHAR            line[256], *ptr;
  NV_FLOAT64         depth;


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;
  recnum = 0;


  //  Open the LLZ file and read the data.

  if ((llz_hnd = open_llz (path, &llz_header)) >= 0)
    {
      //  Read all of the data from this file.  

      for (i = 0 ; i < llz_header.number_of_records ; i++)
        {
          herr = load_parms[0].pfm_global.horizontal_error;
          verr = load_parms[0].pfm_global.vertical_error;


          percent = (NV_INT32) (((NV_FLOAT32) i / (NV_FLOAT32) llz_header.number_of_records) * 100.0);

          read_llz (llz_hnd, i, &llz);
          
          xy.y = llz.xy.lat;
          xy.x = llz.xy.lon;
          depth = llz.depth;
          flags = llz.status;

          beam = 0;

          recnum = i;

          if (load_parms[0].pfm_global.attribute_count)
            {
              NV_INT32 index = load_parms[0].pfm_global.time_attribute_num - 1;
              if (load_parms[0].pfm_global.time_attribute_num)
                {
                  //  Special case!  LLZ files may not have time so we load the null time value.

                  if (llz.tv_sec == 0 && llz.tv_nsec == 0)
                    {
                      attr[index] = PFMWDB_NULL_TIME;
                    }
                  else
                    {
                      attr[index] = (NV_FLOAT32) (llz.tv_sec / 60);
                    }
                }
            }

          Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
        }

      prog->setValue (100);


      close_llz (llz_hnd);


      //  If we actually wrote any data, write the input file name to the list file.   

      for (i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              //  May be MSYS on Windoze so we have to check for either separator.

              if (strrchr (path, '/'))
                {
                  ptr = strrchr (path, '/');
                }
              else if (strrchr (path, '\\'))
                {
                  ptr = strrchr (path, '\\');
                }
              else
                {
                  ptr = NULL;
                }


              if (ptr == NULL) 
                {
                  strcpy (line, path);
                }
              else
                {
                  strcpy (line, (ptr + 1));
                }

              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              write_list_file (pfm_def[i].hnd, native_path, PFM_NAVO_LLZ_DATA);
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      return (0);
    }

  return (total_out_of_limits);
}



NV_BOOL check_czmil_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  NV_INT32           czmil_hnd;
  CZMIL_CXY_Header   czmil_cxy_header;


  //  Open the CZMIL file.

  if ((czmil_hnd = czmil_open_file (path, &czmil_cxy_header, CZMIL_READONLY, NVFalse)) != CZMIL_SUCCESS)
    {
      fprintf (errfp, "\n\nUnable to open the CZMIL file.\n");
      fprintf (errfp, "%s : %s\n", path, czmil_strerror ());
      return (NVFalse);
    }

  czmil_close_file (czmil_hnd);

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_czmil_file
 *
 * Description : Reads a CZMIL file.
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_czmil_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                          PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam, percent, czmil_hnd;
  NV_U_INT32         flags;
  CZMIL_CXY_Header   czmil_cxy_header;
  CZMIL_CXY_Data     czmil;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_CHAR            line[256], *ptr;
  NV_FLOAT64         depth;


  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  

  for (NV_INT32 i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;
  recnum = 0;


  //  Open the CZMIL file and read the data.

  if ((czmil_hnd = czmil_open_file (path, &czmil_cxy_header, CZMIL_READONLY, NVFalse)) != CZMIL_SUCCESS)
    {
      fprintf (errfp, "\n\nUnable to open the CZMIL file.\n");
      fprintf (errfp, "%s : %s\n", path, czmil_strerror ());
      return (0);
    }


  //  Read all of the data from this file.  

  for (NV_U_INT32 i = 0 ; i < czmil_cxy_header.number_of_records ; i++)
    {
      herr = load_parms[0].pfm_global.horizontal_error;
      verr = load_parms[0].pfm_global.vertical_error;


      percent = (NV_INT32) (((NV_FLOAT32) i / (NV_FLOAT32) czmil_cxy_header.number_of_records) * 100.0);


      lidar_null = NVFalse;


      flags = 0;
      if (czmil_read_cxy_record (czmil_hnd, i, &czmil) != CZMIL_SUCCESS) break;

      if (load_parms[0].pfm_global.attribute_count) getCZMILAttributes (attr, load_parms[0].pfm_global, czmil);


      beam = 0;
      recnum = i;
      xy.y = czmil.channel[CZMIL_DEEP_CHANNEL].latitude;
      xy.x = czmil.channel[CZMIL_DEEP_CHANNEL].longitude;

      if (czmil.channel[CZMIL_DEEP_CHANNEL].return_elevation[0] == czmil_cxy_header.null_z_value)
        {
          if (load_parms[0].flags.hof)
            {
              //  We're setting the null depths to -999999.0 to put them outside any sane bounds.  This will cause
              //  them to be set to the null depth for whatever PFM they fall in.  We want to keep them just to see coverage.

              lidar_null = NVTrue;

              depth = -999999.0;

              Do_PFM_Processing (beam, depth, xy, 0.0, 0.0, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
            }
        }
      else
        {
          depth = -czmil.channel[CZMIL_DEEP_CHANNEL].return_elevation[0];

          Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
        }
    }

  prog->setValue (100);


  czmil_close_file (czmil_hnd);


  //  If we actually wrote any data, write the input file name to the list file.   

  for (NV_INT32 i = 0 ; i < pfm_file_count ; i++) 
    {
      if (out_count[i]) 
        {
          //  May be MSYS on Windoze so we have to check for either separator.

          if (strrchr (path, '/'))
            {
              ptr = strrchr (path, '/');
            }
          else if (strrchr (path, '\\'))
            {
              ptr = strrchr (path, '\\');
            }
          else
            {
              ptr = NULL;
            }


          if (ptr == NULL) 
            {
              strcpy (line, path);
            }
          else
            {
              strcpy (line, (ptr + 1));
            }

          write_line_file (pfm_def[i].hnd, line);


          //  Convert to native separators so that we get nice file names on Windows.

          nativePath = QDir::toNativeSeparators (QString (path));
          strcpy (native_path, nativePath.toAscii ());

          write_list_file (pfm_def[i].hnd, native_path, PFM_CZMIL_DATA);
          count[i] += out_count[i];
        }
    }

  return (total_out_of_limits);
}



/********************************************************************
 *
 * Function Name : load_srtm_file
 *
 * Description : Reads the compressed Shuttle Radar Topography Mission
 *               (SRTM) data and loads it as PFM_SRTM_DATA and,
 *               optionally, flagged as reference data.
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_srtm_file (NV_INT32 pfm_fc, NV_INT32 *count, QProgressBar *fileInput,  
                        PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam, i, percent;
  NV_U_INT32         flags, type = 0;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth, min_x, min_y, max_x, max_y;
  NV_CHAR            line[256], *ptr;
  QString            string;


  
  type = 0;
  string = "/Best_available_resolution_SRTM_data";

  if (load_parms[0].flags.srtm1)
    {
      type = 1;
      string = "/1_second_SRTM_data";
    }
  else if (load_parms[0].flags.srtm3)
    {
      type = 2;
      string = "/3_second_SRTM_data";
    }
  else if (load_parms[0].flags.srtm30)
    {
      type = 3;
      string ="/30_second_SRTM_data";
    }


  if (load_parms[0].flags.srtmr) string += "__Reference";
  if (type <= 1 && !load_parms[0].flags.srtme) string += "__SRTM2";


  strcpy (path, string.toAscii ());


  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;



  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  We also need to get the min and max boundaries for all
  //  of the PFM areas.

  min_x = 999.0;
  min_y = 999.0;
  max_x = -999.0;
  max_y = -999.0;
  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);

      if (pfm_def[i].open_args.head.mbr.min_x < min_x) min_x = pfm_def[i].open_args.head.mbr.min_x;
      if (pfm_def[i].open_args.head.mbr.max_x > max_x) max_x = pfm_def[i].open_args.head.mbr.max_x;
      if (pfm_def[i].open_args.head.mbr.min_y < min_y) min_y = pfm_def[i].open_args.head.mbr.min_y;
      if (pfm_def[i].open_args.head.mbr.max_y > max_y) max_y = pfm_def[i].open_args.head.mbr.max_y;
    }

  percent = 0;
  recnum = 0;


  NV_INT32 start_lat = (NV_INT32) min_y;
  NV_INT32 start_lon = (NV_INT32) min_x;
  NV_INT32 end_lat = (NV_INT32) max_y + 1;
  NV_INT32 end_lon = (NV_INT32) max_x + 1;

  if (min_y < 0.0)
    {
      start_lat--;
      if (max_y < 0.0) end_lat--;
    }

  if (min_x < 0.0)
    {
      start_lon--;
      if (max_x < 0.0) end_lon--;
    }

  NV_INT32 range_y = end_lat - start_lat;
  NV_INT32 range_x = end_lon - start_lon;
  NV_INT32 total_cells = range_y * range_x;

  NV_INT16 *array;
  NV_FLOAT64 hinc, winc;
  NV_INT32 wsize = -1, hsize = -1, incount = 0;


  for (NV_INT32 lat = start_lat ; lat < end_lat ; lat++)
    {
      for (NV_INT32 lon = start_lon ; lon < end_lon ; lon++)
        {
          switch (type)
            {
            case 0:
              wsize = read_srtm1_topo_one_degree (lat, lon, &array);
              hsize = wsize;


              //  Try the SRTM2 1 second data if the SRTM1 data area was undefined.

              if (wsize == 2 && !load_parms[0].flags.srtme)
                {
                  wsize = read_srtm2_topo_one_degree (lat, lon, &array);
                  hsize = wsize;
                }
              break;

            case 1:
              set_exclude_srtm2_data (load_parms[0].flags.srtme);
              wsize = read_srtm_topo_one_degree (lat, lon, &array);
              hsize = wsize;
              break;

            case 2:
              wsize = read_srtm3_topo_one_degree (lat, lon, &array);
              hsize = wsize;
              break;

            case 3:
              wsize = read_srtm30_topo_one_degree (lat, lon, &array);
              hsize = wsize;
              break;
            }


          //  Check for the SRTM2 1 by 2 second data (returns a wsize of 1800).

          if (wsize == 1800) hsize = 3600;


          if (wsize > 2)
            {
              winc = 1.0L / (NV_FLOAT64) wsize;
              hinc = 1.0L / (NV_FLOAT64) hsize;

              for (NV_INT32 i = 0 ; i < hsize ; i++)
                {
                  xy.y = (lat + 1.0L) - (i + 1) * hinc;
                  for (NV_INT32 j = 0 ; j < wsize ; j++)
                    {
                      //  We don't load zeros from SRTM since that is the only way they marked water surface.
                      //  This leaves a ring around Death Valley ;-)

                      if (array[i * wsize + j] && array[i * wsize + j] > -32768)
                        {
                          xy.x = lon + j * winc;
                          depth = (NV_FLOAT64) (-array[i * wsize + j]);
                          flags = 0;
                          if (load_parms[0].flags.srtmr) flags = PFM_REFERENCE;
                          beam = 0;
                          incount++;


                          //  Record number is meaningless for SRTM data and can exceed the PFM limits.

                          recnum = 0;
                          herr = 0.0;
                          verr = 0.0;
                          Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                        }
                    }
                  percent = (NV_INT32) (((NV_FLOAT32) ((lat - start_lat) * (NV_FLOAT32) range_x +
                                                       (NV_FLOAT32) (lon - start_lon)) / (NV_FLOAT32) total_cells) * 100.0) +
                    (NV_INT32) (((NV_FLOAT32) i / (NV_FLOAT32) hsize) * 100.0 * (1.0 / total_cells));
                }
            }
        }
    }

  prog->setValue (100);


  switch (type)
    {
    case 0:
      cleanup_srtm_topo ();
      break;

    case 1:
      cleanup_srtm1_topo ();
      if (!load_parms[0].flags.srtme) cleanup_srtm2_topo ();
      break;

    case 2:
      cleanup_srtm3_topo ();
      break;

    case 3:
      cleanup_srtm30_topo ();
      break;
    }


  //  If we actually wrote any data, write the input file name to the list file.   

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      if (out_count[i]) 
        {
          //  May be MSYS on Windoze so we have to check for either separator.

          if (strrchr (path, '/'))
            {
              ptr = strrchr (path, '/');
            }
          else if (strrchr (path, '\\'))
            {
              ptr = strrchr (path, '\\');
            }
          else
            {
              ptr = NULL;
            }


          if (ptr == NULL) 
            {
              strcpy (line, path);
            }
          else
            {
              strcpy (line, (ptr + 1));
            }

          write_line_file (pfm_def[i].hnd, line);


          //  Convert to native separators so that we get nice file names on Windows.

          nativePath = QDir::toNativeSeparators (QString (path));
          strcpy (native_path, nativePath.toAscii ());

          write_list_file (pfm_def[i].hnd, native_path, PFM_NAVO_ASCII_DATA);
          count[i] += out_count[i];
        }
    }

  return (0);
}



NV_BOOL check_dted_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  FILE           *fp;
  UHL            uhl;


  //  Open the DTED file.

  if ((fp = fopen (path, "rb")) == NULL)
    {
      fprintf (errfp, "\n\nUnable to open the DTED file.\n");
      fprintf (errfp, "%s : %s\n", path, strerror (errno));
      return (NVFalse);
    }


  if (read_uhl (fp, &uhl) < 0)
    {
      fprintf (errfp, "\n\n%s is not a DTED file.\n", path);
      return (NVFalse);
    }


  fclose (fp);


  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_dted_file
 *
 * Description : Reads NGA DTED format files.
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_dted_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                         PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam, i, j, percent, total_points, status;
  FILE               *fp;
  UHL                uhl;
  NV_U_INT32         flags;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth, lat_inc, lon_inc;
  NV_CHAR            line[256], *ptr;
  DTED_DATA          dted_data;


  
  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  We also need to get the min and max boundaries for all
  //  of the PFM areas.

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;
  recnum = 0;


  //  Open the DTED file and read the data.

  if ((fp = fopen (path, "rb")) != NULL)
    {
      //  Read the UHL record.

      read_uhl (fp, &uhl);

      lon_inc = uhl.lon_int / 3600.0;
      lat_inc = uhl.lat_int / 3600.0;


      //  Read all of the data from this file.  

      total_points = uhl.num_lat_points * uhl.num_lon_lines;

      for (i = 0 ; i < uhl.num_lon_lines ; i++)
        {
          xy.x = uhl.ll_lon + (NV_FLOAT64) i * lon_inc;


          if ((status = read_dted_data (fp, uhl.num_lat_points, i, &dted_data))) 
            {
              fprintf (stderr, "READ ERROR %d\n", status);
              fflush (stderr);
              return (0);
            }


          for (j = 0 ; j < uhl.num_lat_points ; j++)
            {
              xy.y = uhl.ll_lat + (NV_FLOAT64) j * lat_inc;


              percent = (NV_INT32) ((((NV_FLOAT32) i * uhl.num_lon_lines) / (NV_FLOAT32) total_points) * 100.0);


              depth = -dted_data.elev[j];


              //  Loading undefined values as an imposible depth so that we can get rid of bad -32767 values.
 
              if (dted_data.elev[j] <= -32767) depth = 11999.0;


              flags = 0;

              recnum = i;
              beam = j;

              herr = 0.0;
              verr = 0.0;
              Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
            }
        }

      prog->setValue (100);


      fclose (fp);


      //  If we actually wrote any data, write the input file name to the list file.   

      for (i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              //  May be MSYS on Windoze so we have to check for either separator.

              if (strrchr (path, '/'))
                {
                  ptr = strrchr (path, '/');
                }
              else if (strrchr (path, '\\'))
                {
                  ptr = strrchr (path, '\\');
                }
              else
                {
                  ptr = NULL;
                }


              if (ptr == NULL) 
                {
                  strcpy (line, path);
                }
              else
                {
                  strcpy (line, (ptr + 1));
                }

              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              write_list_file (pfm_def[i].hnd, native_path, PFM_DTED_DATA);
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      return (0);
    }

  return (total_out_of_limits);
}



NV_BOOL check_chrtr_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  NV_INT32       chrtr_handle;
  CHRTR_HEADER   chrtr_header;
  CHRTR2_HEADER  chrtr2_header;


  //  Since we're phasing out the use of the old CHRTR format we're reusing the PFM_CHRTR_DATA flag for the new CHRTR2 
  //  format.  There will be a transitional period where both must be supported.  Nobody uses CHRTR or CHRTR2 except 
  //  NAVO so I doubt if this will cause a problem.  JCD 01/17/11

  if (strstr (path, ".ch2"))
    {
      if ((chrtr_handle = chrtr2_open_file (path, &chrtr2_header, CHRTR2_READONLY)) < 0)
        {
          fprintf (errfp, "\n\nUnable to open the CHRTR2 file.\n");
          fprintf (errfp, "%s : %s\n", path, chrtr2_strerror ());
          return (NVFalse);
        }

      chrtr2_close_file (chrtr_handle);
    }
  else
    {
      if ((chrtr_handle = open_chrtr (path, &chrtr_header)) < 0)
        {
          fprintf (errfp, "\n\nUnable to open the CHRTR file.\n");
          fprintf (errfp, "%s : %s\n", path, strerror (errno));
          return (NVFalse);
        }

      close_chrtr (chrtr_handle);
    }

  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_chrtr_file
 *
 * Description : Reads NAVO CHRTR format files.
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_chrtr_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                          PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  NV_INT32           beam, i, j, percent, total_points;
  NV_INT32           chrtr_handle;
  CHRTR_HEADER       chrtr_header;
  NV_FLOAT32         *chrtr_data;
  CHRTR2_HEADER      chrtr2_header;
  CHRTR2_RECORD      *chrtr2_data;
  NV_U_INT32         flags;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth;
  NV_CHAR            line[256], *ptr;


  
  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  We also need to get the min and max boundaries for all
  //  of the PFM areas.

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;
  recnum = 0;
  chrtr_data = NULL;


  //  Since we're phasing out the use of the old CHRTR format we're reusing the PFM_CHRTR_DATA flag for the new CHRTR2 
  //  format.  There will be a transitional period where both must be supported.  Nobody uses CHRTR or CHRTR2 except 
  //  NAVO so I doubt if this will cause a problem.  JCD 01/17/11

  if (strstr (path, ".ch2"))
    {
      if ((chrtr_handle = chrtr2_open_file (path, &chrtr2_header, CHRTR2_READONLY)) >= 0)
        {
          chrtr2_data = (CHRTR2_RECORD *) malloc (chrtr2_header.width * sizeof (CHRTR2_RECORD));
          if (chrtr2_data == NULL)
            {
              perror ("Allocating memory for CHRTR2 input array in load_file");
              exit (-1);
            }


          NV_BOOL huncert = NVFalse;
          NV_BOOL vuncert = NVFalse;
          NV_BOOL vstd = NVFalse;
          if (chrtr2_header.horizontal_uncertainty_scale != 0.0) huncert = NVTrue;
          if (chrtr2_header.vertical_uncertainty_scale != 0.0) vuncert = NVTrue;
          if (chrtr2_header.uncertainty_scale != 0.0) vstd = NVTrue;

          NV_FLOAT64 half_x = chrtr2_header.lon_grid_size_degrees * 0.5;
          NV_FLOAT64 half_y = chrtr2_header.lat_grid_size_degrees * 0.5;


          //  Read all of the data from this file.  

          total_points = chrtr2_header.width * chrtr2_header.height;

          for (i = 0 ; i < chrtr2_header.height ; i++)
            {
              //  Compute the latitude of the center of the bin.

              NV_F64_COORD2 xy;
              xy.y = chrtr2_header.mbr.slat + i * chrtr2_header.lat_grid_size_degrees + half_y;

              if (!chrtr2_read_row (chrtr_handle, i, 0, chrtr2_header.width, chrtr2_data))
                {
                  for (j = 0 ; j < chrtr2_header.width ; j++)
                    {
                      //  Compute the longitude of the center of the bin.

                      xy.x = chrtr2_header.mbr.wlon + j * chrtr2_header.lon_grid_size_degrees + half_x;


                      //  Only load "real" data.

                      if (chrtr2_data[j].status & (CHRTR2_REAL | CHRTR2_DIGITIZED_CONTOUR))
                        {
                          depth = chrtr2_data[j].z;
                          flags = 0;

                          recnum = i;
                          beam = j;

                          herr = 0.0;
                          verr = 0.0;

                          if (huncert) herr = chrtr2_data[j].horizontal_uncertainty;
                          if (vuncert)
                            {
                              verr = chrtr2_data[j].vertical_uncertainty;
                            }
                          else if (vstd)
                            {
                              verr = chrtr2_data[j].uncertainty;
                            }

                          percent = (NV_INT32) ((((NV_FLOAT32) (i * chrtr2_header.width + j)) / (NV_FLOAT32) total_points) * 100.0);

                          Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                        }
                    }
                }
            }

          prog->setValue (100);


          if (chrtr2_data) free (chrtr2_data);
          chrtr2_close_file (chrtr_handle);
        }
      else
        {
          fprintf (errfp, "\n\nUnable to open file %s\n", path);
          return (0);
        }
    }
  else
    {
      //  Open the CHRTR file and read the data.

      if ((chrtr_handle = open_chrtr (path, &chrtr_header)) >= 0)
        {
          chrtr_data = (NV_FLOAT32 *) malloc (chrtr_header.width * sizeof (NV_FLOAT32));
          if (chrtr_data == NULL)
            {
              perror ("Allocating memory for CHRTR input array in load_file");
              exit (-1);
            }


          //  Read all of the data from this file.  

          NV_FLOAT64 grid_degrees = chrtr_header.grid_minutes / 60.0;
          total_points = chrtr_header.width * chrtr_header.height;

          for (i = 0 ; i < chrtr_header.height ; i++)
            {
              xy.y = chrtr_header.slat + (NV_FLOAT64) i * grid_degrees;

              if (read_chrtr (chrtr_handle, i, 0, chrtr_header.width, chrtr_data))
                {
                  for (j = 0 ; j < chrtr_header.width ; j++)
                    {
                      xy.x = chrtr_header.wlon + (NV_FLOAT64) j * grid_degrees;


                      //  Only load "real" data.

                      if (bit_test (chrtr_data[j], 0))
                        {
                          depth = chrtr_data[j];
                          flags = 0;

                          recnum = i;
                          beam = j;

                          herr = 0.0;
                          verr = 0.0;

                          percent = (NV_INT32) ((((NV_FLOAT32) (i * chrtr_header.width + j)) / (NV_FLOAT32) total_points) * 100.0);

                          Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                        }
                    }
                }
            }

          prog->setValue (100);


          if (chrtr_data) free (chrtr_data);
          close_chrtr (chrtr_handle);
        }
      else
        {
          fprintf (errfp, "\n\nUnable to open file %s\n", path);
          return (0);
        }
    }


  //  If we actually wrote any data, write the input file name to the list file.   

  for (i = 0 ; i < pfm_file_count ; i++) 
    {
      if (out_count[i]) 
        {
          //  May be MSYS on Windoze so we have to check for either separator.

          if (strrchr (path, '/'))
            {
              ptr = strrchr (path, '/');
            }
          else if (strrchr (path, '\\'))
            {
              ptr = strrchr (path, '\\');
            }
          else
            {
              ptr = NULL;
            }


          if (ptr == NULL) 
            {
              strcpy (line, path);
            }
          else
            {
              strcpy (line, (ptr + 1));
            }

          write_line_file (pfm_def[i].hnd, line);


          //  Convert to native separators so that we get nice file names on Windows.

          nativePath = QDir::toNativeSeparators (QString (path));
          strcpy (native_path, nativePath.toAscii ());

          write_list_file (pfm_def[i].hnd, native_path, PFM_CHRTR_DATA);
          count[i] += out_count[i];
        }
    }

  return (total_out_of_limits);
}



NV_BOOL check_bag_file (NV_CHAR *path, QProgressBar *preCheck, FILE *errfp)
{
  bagHandle                    bagHnd;
  bagError                     bagErr;
  IDENTIFICATION_INFO          identification_info;
  MD_LEGAL_CONSTRAINTS         md_legal_constraints;
  MD_SECURITY_CONSTRAINTS      md_security_constraints;
  DATA_QUALITY_INFO            data_quality_info;
  SPATIAL_REPRESENTATION_INFO  spatial_representation_info;
  REFERENCE_SYSTEM_INFO        reference_system_info;
  RESPONSIBLE_PARTY            contact;
  NV_U_CHAR                    *xmlString;


  //  Open the BAG file.

  if ((bagErr = bagFileOpen (&bagHnd, BAG_OPEN_READONLY, (u8 *) path)) != BAG_SUCCESS)
    {
      fprintf (errfp, "\n\nUnable to open the BAG file %s.\n", path);

      u8 *errstr;

      if (bagGetErrorString (bagErr, &errstr) == BAG_SUCCESS) fprintf (errfp, "%s: %s\n", path, errstr);

      return (NVFalse);
    }

  bagReadXMLStream  (bagHnd);
  xmlString = bagGetDataPointer (bagHnd)->metadata;

  GetAllStructures (xmlString, &identification_info, &md_legal_constraints, &md_security_constraints, &data_quality_info, &spatial_representation_info,
                    &reference_system_info, &contact);

  if (strcmp (reference_system_info.projection, "Geodetic"))
    {
      bagFileClose (bagHnd);
      fprintf (errfp, "\n\nBAG file %s not Geodetic projection.\n", path);
      return (NVFalse);
    }


  bagFileClose (bagHnd);


  preCheck->setValue (100);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : load_bag_file
 *
 * Description : Reads NGA BAG format files.
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

NV_INT32 load_bag_file (NV_INT32 pfm_fc, NV_INT32 *count, QString file, QProgressBar *fileInput, FILE *errfp, 
                         PFM_LOAD_PARAMETERS load_parms[], PFM_DEFINITION pfm_def[])
{
  bagHandle          bagHnd;
  bagError           bagErr;
  BAG_RECORD         bag;
  NV_INT32           beam, percent;
  NV_U_INT32         flags;
  NV_F64_COORD2      xy;
  NV_FLOAT32         herr, verr;
  NV_FLOAT64         depth;
  NV_CHAR            line[256], *ptr;


  
  strcpy (path, file.toAscii ());
  prog = fileInput;


  //  Zero count on number of soundings per file. 

  memset (out_count, 0, MAX_PFM_FILES * sizeof (NV_INT32));
  pfm_file_count = pfm_fc;
  total_out_of_limits = 0;


  //  Clear the attributes.

  if (load_parms[0].pfm_global.attribute_count) for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) attr[i] = 0.0;


  //  Just so that we don't mistakenly think this might be a HOF LIDAR NULL.

  lidar_null = NVFalse;


  //  Get the next available file number so that we can put it into the index file.  If we don't read any data from this file that
  //  is in the defined area we won't put the filename into the list file.  We also need to get the min and max boundaries for all
  //  of the PFM areas.

  for (NV_INT32 i = 0 ; i < pfm_file_count ; i++) 
    {
      line_count[i] = -1; 
      next_line[i] = get_next_line_number (pfm_def[i].hnd);
      file_number[i] = get_next_list_file_number (pfm_def[i].hnd);
    }

  percent = 0;
  recnum = 0;


  //  Open the BAG file and read the data.

  if ((bagErr = bagFileOpen (&bagHnd, BAG_OPEN_READONLY, (u8 *) path)) == BAG_SUCCESS)
    {
      NV_INT32 data_cols = bagGetDataPointer (bagHnd)->def.ncols;
      NV_INT32 data_rows = bagGetDataPointer (bagHnd)->def.nrows;
      NV_FLOAT64 x_bin_size_degrees = bagGetDataPointer (bagHnd)->def.nodeSpacingX;
      NV_FLOAT64 y_bin_size_degrees = bagGetDataPointer (bagHnd)->def.nodeSpacingY;
      NV_FLOAT64 half_y = y_bin_size_degrees / 2.0;
      NV_FLOAT64 half_x = x_bin_size_degrees / 2.0;
      NV_FLOAT64 sw_corner_y = bagGetDataPointer (bagHnd)->def.swCornerY;
      NV_FLOAT64 sw_corner_x = bagGetDataPointer (bagHnd)->def.swCornerX;

      NV_FLOAT32 *data = (NV_FLOAT32 *) malloc (data_cols * sizeof (NV_FLOAT32));
      if (data == NULL) 
        {
          perror ("Allocating data memory in load_bag_file");
          exit (-1);
        }

      NV_FLOAT32 *uncert = NULL;
      uncert = (NV_FLOAT32 *) malloc (data_cols * sizeof (NV_FLOAT32));
      if (uncert == NULL) 
        {
          perror ("Allocating uncertainty memory in load_bag_file");
          exit (-1);
        }


      NV_INT32 total_points = data_rows * data_cols;

      for (NV_INT32 i = 0 ; i < data_rows ; i++)
        {
          bagErr = bagReadRow (bagHnd, i, 0, data_cols - 1, Elevation, (void *) data);

          bagErr = bagReadRow (bagHnd, i, 0, data_cols - 1, Uncertainty, (void *) uncert);


          xy.y = sw_corner_y + (NV_FLOAT64) i * y_bin_size_degrees + half_y;


          for (NV_INT32 j = 0 ; j < data_cols ; j++)
            {
              if (data[j] < NULL_ELEVATION)
                {
                  xy.x = sw_corner_x + (NV_FLOAT64) j * x_bin_size_degrees + half_x;

                  if (uncert[j] < NULL_UNCERTAINTY)
                    {
                      bag.uncert = uncert[j];
                    }
                  else
                    {
                      bag.uncert = 1001.0;
                    }

                  bag.elev = -data[j];

                  if (load_parms[0].pfm_global.attribute_count) getBAGAttributes (attr, load_parms[0].pfm_global, bag);

                  flags = 0;

                  recnum = i;
                  beam = j;

                  herr = 0.0;
                  verr = 0.0;

                  depth = bag.elev;

                  percent = (NV_INT32) ((((NV_FLOAT32) (i * data_cols + j)) / (NV_FLOAT32) total_points) * 100.0);

                  Do_PFM_Processing (beam, depth, xy, herr, verr, flags, 0.0, attr, load_parms, pfm_def, percent, 0, 0);
                }
            }
        }


      prog->setValue (100);


      bagFileClose (bagHnd);


      //  If we actually wrote any data, write the input file name to the list file.   

      for (NV_INT32 i = 0 ; i < pfm_file_count ; i++) 
        {
          if (out_count[i]) 
            {
              //  May be MSYS on Windoze so we have to check for either separator.

              if (strrchr (path, '/'))
                {
                  ptr = strrchr (path, '/');
                }
              else if (strrchr (path, '\\'))
                {
                  ptr = strrchr (path, '\\');
                }
              else
                {
                  ptr = NULL;
                }


              if (ptr == NULL) 
                {
                  strcpy (line, path);
                }
              else
                {
                  strcpy (line, (ptr + 1));
                }

              write_line_file (pfm_def[i].hnd, line);


              //  Convert to native separators so that we get nice file names on Windows.

              nativePath = QDir::toNativeSeparators (QString (path));
              strcpy (native_path, nativePath.toAscii ());

              write_list_file (pfm_def[i].hnd, native_path, PFM_BAG_DATA);
              count[i] += out_count[i];
            }
        }
    }
  else
    {
      fprintf (errfp, "\n\nUnable to open file %s\n", path);
      return (0);
    }

  return (total_out_of_limits);
}
