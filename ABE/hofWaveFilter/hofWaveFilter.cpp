#include "hofWaveFilter.hpp"
#include "version.hpp"


/***************************************************************************\
*                                                                           *
*   Module Name:        hofWaveFilter                                       *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       April 21, 2011                                      *
*                                                                           *
*   Purpose:            Invalidate HOF first returns in a portion of a PFM  *
*                       based on the slope and amplitude of the associated  *
*                       waveforms.                                          *
*                                                                           *
*   Caveats:            This program is not meant to be run from the        *
*                       command line.  It should only be run as a QProcess  *
*                       from pfmEdit.                                       *
*                                                                           *
\***************************************************************************/

NV_INT32 main (NV_INT32 argc, NV_CHAR **argv)
{
  hofWaveFilter *hr;

  hr = new hofWaveFilter (argc, argv);
}


/*  This is the PFM/file/rec sort function for qsort.  */

static NV_INT32 compare_pfm_file_numbers (const void *a, const void *b)
{
    SORT_REC *sa = (SORT_REC *) (a);
    SORT_REC *sb = (SORT_REC *) (b);


    //  If the lines aren't equal we sort on line.

    if (sa->pfm_file != sb->pfm_file) return (sa->pfm_file < sb->pfm_file ? 0 : 1);


    //  Otherwise we sort on the original record number.

    return (sa->orig_rec < sb->orig_rec ? 0 : 1);
}



void hofWaveFilter::usage ()
{
  fprintf (stderr, "\nUsage: hofWaveFilter --shared_memory_key SHARED_MEMORY_KEY\n");
  fprintf (stderr, "This program is not meant to be run from the command line.  It should only be\n");
  fprintf (stderr, "run as a QProcess from pfmEdit or pfmEdit3D.\n\n");
  fflush (stderr);
}


hofWaveFilter::hofWaveFilter (NV_INT32 argc, NV_CHAR **argv)
{
  NV_CHAR            wave_file[512], string[1024];
  FILE               *fp = NULL, *wfp = NULL;
  HYDRO_OUTPUT_T     hof_record;
  WAVE_HEADER_T      wave_header;
  WAVE_DATA_T        wave_rec;
  NV_CHAR            c;
  extern char        *optarg;
  NV_INT32           pmt_run_req = 0, apd_run_req = 0, pmt_ac_zero_offset = 0, apd_ac_zero_offset = 0;
  NV_FLOAT32         slope_req = 0.50;


  NV_BOOL pmt_return_filter (NV_INT32 rec, NV_INT32 sub_rec, HYDRO_OUTPUT_T *hof_record, NV_INT32 pmt_run_req, NV_FLOAT32 slope_req, NV_INT32 ac_zero_offset,
                             NV_INT32 ac_off_req, WAVE_DATA_T *wave_rec);
  NV_BOOL apd_return_filter (NV_INT32 rec, NV_INT32 sub_rec, HYDRO_OUTPUT_T *hof_record, NV_INT32 apd_run_req, NV_FLOAT32 slope_req, NV_INT32 ac_zero_offset,
                             NV_INT32 ac_off_req, WAVE_DATA_T *wave_rec);
  NV_BOOL waveform_check (MISC *misc, WAVE_DATA *wave_data, NV_INT32 recnum);


  if (argc < 2)
    {
      usage ();
      exit (-1);
    }


  NV_INT32 option_index = 0;
  NV_INT32 key = 0;
  while (NVTrue) 
    {
      static struct option long_options[] = {{"shared_memory_key", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      c = (NV_CHAR) getopt_long (argc, argv, "s", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
	      sscanf (optarg, "%d", &key);
              break;
            }

          break;

        case 's':
          break;

        default:
          usage ();
          exit (-1);
          break;
        }
    }


  /******************************************* IMPORTANT NOTE ABOUT SHARED MEMORY **************************************** \

      This is a little note about the use of shared memory within the Area-Based Editor (ABE) programs.  If you read
      the Qt documentation (or anyone else's documentation) about the use of shared memory they will say "Dear [insert
      name of omnipotent being of your choice here], whatever you do, always lock shared memory when you use it!".
      The reason they say this is that access to shared memory is not atomic.  That is, reading shared memory and then
      writing to it is not a single operation.  An example of why this might be important - two programs are running,
      the first checks a value in shared memory, sees that it is a zero.  The second program checks the same location
      and sees that it is a zero.  These two programs have different actions they must perform depending on the value
      of that particular location in shared memory.  Now the first program writes a one to that location which was
      supposed to tell the second program to do something but the second program thinks it's a zero.  The second program
      doesn't do what it's supposed to do and it writes a two to that location.  The two will tell the first program 
      to do something.  Obviously this could be a problem.  In real life, this almost never occurs.  Also, if you write
      your program properly you can make sure this doesn't happen.  In ABE we almost never lock shared memory because
      something much worse than two programs getting out of sync can occur.  If we start a program and it locks shared
      memory and then dies, all the other programs will be locked up.  When you look through the ABE code you'll see
      that we very rarely lock shared memory, and then only for very short periods of time.  This is by design.

  \******************************************* IMPORTANT NOTE ABOUT SHARED MEMORY ****************************************/


  //  Get the shared memory area.  If it doesn't exist, quit.  It should have already been created by pfmView and passed from
  //  pfmEdit(3D).  The key is the process ID of the bin viewer (pfmView) plus _abe.

  if (!key)
    {
      fprintf (stderr, "\n\n--shared_memory_key option not specified on command line.  Terminating!\n\n");
      exit (-1);
    }

  QString skey;
  skey.sprintf ("%d_abe", key);

  misc.abeShare = new QSharedMemory (skey);

  if (!misc.abeShare->attach (QSharedMemory::ReadWrite))
    {
      fprintf (stderr, "\n\nError retrieving shared memory segment in %s.\n\n", argv[0]);
      exit (-1);
    }

  misc.abe_share = (ABE_SHARE *) misc.abeShare->data ();


  //  Get the point cloud shared memory area.  If it doesn't exist, quit.  It should have already been created by pfmEdit(3D).
  //  The key is the process ID of pfmEdit(3D) plus _abe_pfmEdit.

  QString dskey;
  dskey.sprintf ("%d_abe_pfmEdit", misc.abe_share->ppid);

  misc.dataShare = new QSharedMemory (dskey);

  if (!misc.dataShare->attach (QSharedMemory::ReadWrite))
    {
      fprintf (stderr, "\n\nError retrieving point cloud shared memory segment in %s.\n\n", argv[0]);
      exit (-1);
    }

  misc.data = (POINT_CLOUD *) misc.dataShare->data ();


  //  Lock the shared memory so that pfmEdit(3D) can't do anything until we're done.

  misc.dataShare->lock ();


  wave_data = (WAVE_DATA *) malloc (misc.abe_share->point_cloud_count * sizeof (WAVE_DATA));
  if (wave_data == NULL)
    {
      perror ("Allocating wave_data in hofWaveFilter.cpp");
      misc.dataShare->unlock ();
      exit (-1);
    }

  SORT_REC *sa = (SORT_REC *) malloc (misc.abe_share->point_cloud_count * sizeof (SORT_REC));
  if (sa == NULL)
    {
      perror ("Allocating sort array in hofWaveFilter.cpp");
      misc.dataShare->unlock ();
      exit (-1);
    }


  //  Open the PFM files and compute the average bin size.

  NV_FLOAT64 bin_size_meters = 0.0;

  for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
    {
      if ((misc.pfm_handle[pfm] = open_existing_pfm_file (&misc.abe_share->open_args[pfm])) < 0)
        {
          fprintf (stderr, "The file %s is not a PFM handle or list file or there was an error reading the file.\n", misc.abe_share->open_args[pfm].list_path);
          fprintf (stderr, "The error message returned was: %s\n", pfm_error_str (pfm_error));
          misc.dataShare->unlock ();
          exit (-1);
        }
      bin_size_meters += misc.abe_share->open_args[pfm].head.bin_size_xy;
    }

  bin_size_meters /= (NV_FLOAT64) misc.abe_share->pfm_count;


  init_geo_distance (bin_size_meters, misc.abe_share->edit_area.min_x, misc.abe_share->edit_area.min_y, misc.abe_share->edit_area.max_x,
                     misc.abe_share->edit_area.max_y);


  //  Stuff the record pointers into the sort array.

  for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
    {
      sa[i].pfm_file = misc.data[i].pfm * PFM_MAX_FILES + misc.data[i].file;
      sa[i].orig_rec = misc.data[i].rec;
      sa[i].rec = i;
    }


  //  Sort the records so we can read from each file in order.

  qsort (sa, misc.abe_share->point_cloud_count, sizeof (SORT_REC), compare_pfm_file_numbers);


  /*  Beginning of possible multithreading (to be used in a batch version of this program).

  //  We need to find 3 break points to set up the possibility of 4 threads.  First, find the pfm_file change location nearest to the center of
  //  the sorted point cloud array.

  NV_INT32 break_point[3];
  break_point[1] = misc.abe_share->point_cloud_count / 2;
  NV_INT32 after = 0, before = 0;

  for (NV_INT32 i = break_point[1] ; i < misc.abe_share->point_cloud_count - 1 ; i++)
    {
      if (sa[i + 1].pfm_file != sa[i].pfm_file)
        {
          after = i + 1;
          break;
        }
    }

  for (NV_INT32 i = break_point[1] ; i > 0 ; i--)
    {
      if (sa[i].pfm_file != sa[i - 1].pfm_file)
        {
          before = i;
          break;
        }
    }

  if (after - break_point[1] < break_point[1] - before)
    {
      break_point[1] = after;
    }
  else
    {
      break_point[1] = before;
    }

  fprintf (stderr,"%s %d %d %d %d %d %d %d\n",__FILE__,__LINE__,before,after,misc.abe_share->point_cloud_count,misc.data[before].file,misc.data[after].file,break_point[1]);

  //  Find the file change location nearest to the mid-point of the section prior to break_point[1] determined above.

  break_point[0] = break_point[1] / 2;
  before = after = 0;

  for (NV_INT32 i = break_point[0] ; i < break_point[1] - 1 ; i++)
    {
      if (sa[i + 1].pfm_file != sa[i].pfm_file)
        {
          after = i + 1;
          break;
        }
    }


  for (NV_INT32 i = break_point[0] ; i > 0 ; i--)
    {
      if (sa[i].pfm_file != sa[i - 1].pfm_file)
        {
          before = i;
          break;
        }
    }

  if (after - break_point[0] < break_point[0] - before)
    {
      break_point[0] = after;
    }
  else
    {
      break_point[0] = before;
    }

  fprintf (stderr,"%s %d %d %d %d %d %d %d\n",__FILE__,__LINE__,before,after,misc.abe_share->point_cloud_count,misc.data[before].file,misc.data[after].file,break_point[0]);

  //  Find the file change location nearest to the mid-point of the section after break_point[1] determined above.

  break_point[2] = break_point[1] + (misc.abe_share->point_cloud_count - break_point[1]) / 2;
  before = after = 0;

  for (NV_INT32 i = break_point[2] ; i < misc.abe_share->point_cloud_count - 1 ; i++)
    {
      if (sa[i + 1].pfm_file != sa[i].pfm_file)
        {
          after = i + 1;
          break;
        }
    }


  for (NV_INT32 i = break_point[2] ; i > break_point[1] ; i--)
    {
      if (sa[i].pfm_file != sa[i - 1].pfm_file)
        {
          before = i;
          break;
        }
    }

  if (after - break_point[2] < break_point[2] - before)
    {
      break_point[2] = after;
    }
  else
    {
      break_point[2] = before;
    }

  fprintf (stderr,"%s %d %d %d %d %d %d %d\n",__FILE__,__LINE__,before,after,misc.abe_share->point_cloud_count,misc.data[before].file,misc.data[after].file,break_point[2]);
  */


  //  Do the low slope filter on all the data points.

  fp = NULL;
  wfp = NULL;
  NV_INT32 prev_pfm_file = -999;

  for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
    {
      //  This is the misc.data record number from the pfm/file/rec sorted array.

      NV_INT32 ndx = sa[i].rec;


      //  Only on PFM_HOF_CHARTS_DATA.

      if (misc.data[ndx].type == PFM_CHARTS_HOF_DATA)
        {
          //  Since we sorted by PFM file combined with the file number, we only have to close and open a new file when the pfm_file number changes.

          if (sa[i].pfm_file != prev_pfm_file)
            {
              //  If previous HOF and/or INH (wave) files were open, close them.

              if (fp) fclose (fp);
              if (wfp) fclose (wfp);
              fp = NULL;
              wfp = NULL;


              //  Get the HOF file name from the PFM list (.ctl) file.

              NV_INT16 type;
              read_list_file (misc.pfm_handle[misc.data[ndx].pfm], misc.data[ndx].file, string, &type);


              //  Open the HOF file.

              if ((fp = open_hof_file (string)) == NULL)
                {
                  perror (string);

                  misc.dataShare->unlock ();
                  misc.dataShare->detach ();
                  misc.abeShare->detach ();

                  exit (-1);
                }


              //  Construct the INH file name

              strcpy (wave_file, string);
              sprintf (&wave_file[strlen (wave_file) - 4], ".inh");


              //  Open the INH file

              if ((wfp = open_wave_file (wave_file)) == NULL) 
                {
                  perror (wave_file);

                  misc.dataShare->unlock ();
                  misc.dataShare->detach ();
                  misc.abeShare->detach ();

                  exit (-1);
                }


              //  Read the INH header

              wave_read_header (wfp, &wave_header);

              pmt_ac_zero_offset = wave_header.ac_zero_offset[PMT];
              apd_ac_zero_offset = wave_header.ac_zero_offset[APD];


              //  We're assuming that the waveform sizes are constant.  This error should never happen.

              if (wave_header.apd_size != HWF_APD_SIZE || wave_header.pmt_size != HWF_PMT_SIZE)
                {
                  fprintf (stderr, "Bad APD (%d) or PMT (%d) array length in file %s\n", wave_header.apd_size, wave_header.pmt_size, wave_file);

                  misc.dataShare->unlock ();
                  misc.dataShare->detach ();
                  misc.abeShare->detach ();

                  exit (-1);
                }


              //  Save the previous pfm_file number so we'll know when to open a new file.

              prev_pfm_file = sa[i].pfm_file;
            }


          //  We want to store X and Y as meters from the lower left corner of the total MBR so that we can do our
          //  distance calculations more quickly.

          geo_distance (misc.abe_share->edit_area.min_y, misc.abe_share->edit_area.min_x, misc.abe_share->edit_area.min_y, misc.data[ndx].x, &wave_data[ndx].mx);
          geo_distance (misc.abe_share->edit_area.min_y, misc.abe_share->edit_area.min_x, misc.data[ndx].y, misc.abe_share->edit_area.min_x, &wave_data[ndx].my);


          //  Set all of the check flags to NVTrue.  We'll unset them as we go along.

          wave_data[ndx].check = NVTrue;


          //  No point in checking already invalid data.

          if (misc.data[ndx].val & PFM_INVAL)
            {
              wave_data[ndx].check = NVFalse;
            }
          else
            {
              //  Read the current HOF record.

              hof_read_record (fp, misc.data[ndx].rec, &hof_record);


              //  No point in checking Shallow Water Algorithm, Shoreline Depth Swapped data, or land.  We still have to load the wave form data though.

              if ((misc.data[ndx].sub == 0 && (hof_record.abdc == 72 || hof_record.abdc == 74 || hof_record.abdc == 70)) ||
                  (misc.data[ndx].sub == 1 && (hof_record.sec_abdc == 72 || hof_record.sec_abdc == 74 || hof_record.sec_abdc == 70)))
                wave_data[ndx].check = NVFalse;


              apd_run_req = hof_record.calc_bot_run_required[0];
              pmt_run_req = hof_record.calc_bot_run_required[1];


              wave_data[ndx].bot_bin_first = hof_record.bot_bin_first;
              wave_data[ndx].bot_bin_second = hof_record.bot_bin_second;

              if (wfp)
                {
                  //  Read the corresponding wave data.

                  wave_read_record (wfp, misc.data[ndx].rec, &wave_rec);


                  //  Copy the waveform data to our internal arrays.

                  memcpy (wave_data[ndx].apd, wave_rec.apd, HWF_APD_SIZE);
                  memcpy (wave_data[ndx].pmt, wave_rec.pmt, HWF_PMT_SIZE);


                  //  Check to see if the sub_record we're looking for is PMT (0).

                  if ((misc.data[ndx].sub == 0 && hof_record.bot_channel == PMT) || (misc.data[ndx].sub == 1 && hof_record.sec_bot_chan == PMT))
                    {
                      if (pmt_return_filter (misc.data[ndx].rec, misc.data[ndx].sub, &hof_record, pmt_run_req, slope_req, pmt_ac_zero_offset,
                                             misc.abe_share->filterShare.pmt_ac_zero_offset_required, &wave_rec)) misc.data[ndx].exflag = NVTrue;
                    }


                  //  Check to see if the sub_record we're looking for is APD (1).

                  if ((misc.data[ndx].sub == 0 && hof_record.bot_channel == APD) || (misc.data[ndx].sub == 1 && hof_record.sec_bot_chan == APD))
                    {
                      if (apd_return_filter (misc.data[ndx].rec, misc.data[ndx].sub, &hof_record, apd_run_req, slope_req, apd_ac_zero_offset, 
                                             misc.abe_share->filterShare.apd_ac_zero_offset_required, &wave_rec)) misc.data[ndx].exflag = NVTrue;
                    }
                }
            }
        }
    }


  for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++) close_pfm_file (misc.pfm_handle[pfm]);

  if (fp) fclose (fp);
  if (wfp) fclose (wfp);


  free (sa);


  //  Now we need to build an array of bins (twice the size of the search radius) so that we can efficiently perform the dreaded
  //  Hockey Puck of Confidence (TM) proximity valid point search.

  NV_FLOAT64 search_bin_size_meters = misc.abe_share->filterShare.search_radius * 2.0;
  NV_FLOAT64 width_meters, height_meters;


  geo_distance (misc.abe_share->edit_area.min_y, misc.abe_share->edit_area.min_x, misc.abe_share->edit_area.max_y, misc.abe_share->edit_area.min_x,
                &height_meters);
  geo_distance (misc.abe_share->edit_area.min_y, misc.abe_share->edit_area.min_x, misc.abe_share->edit_area.min_y, misc.abe_share->edit_area.max_x,
                &width_meters);


  NV_INT32 rows = (NV_INT32) (height_meters / search_bin_size_meters) + 1;
  NV_INT32 cols = (NV_INT32) (width_meters / search_bin_size_meters) + 1;

  BIN_DATA **bin_data = (BIN_DATA **) calloc (rows, sizeof (BIN_DATA *));
  if (bin_data == NULL)
    {
      perror ("Allocating bin_data in hofWaveFilter.cpp");
      misc.dataShare->unlock ();
      exit (-1);
    }

  for (NV_INT32 i = 0 ; i < rows ; i++)
    {
      bin_data[i] = (BIN_DATA *) calloc (cols, sizeof (BIN_DATA));
      if (bin_data[i] == NULL)
        {
          perror ("Allocating bin_data[i] in hofWaveFilter.cpp");
          misc.dataShare->unlock ();
          exit (-1);
        }
    }


  //  Now let's load the record pointers into the bin array.

  for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
    {
      NV_INT32 row = (NV_INT32) (wave_data[i].my / search_bin_size_meters);
      NV_INT32 col = (NV_INT32) (wave_data[i].mx / search_bin_size_meters);

      bin_data[row][col].data = (NV_INT32 *) realloc (bin_data[row][col].data, (bin_data[row][col].count + 1) * sizeof (NV_INT32));
      if (bin_data[row][col].data == NULL)
        {
          perror ("Allocating bin_data[row][col].data in hofWaveFilter.cpp");
          misc.dataShare->unlock ();
          exit (-1);
        }

      bin_data[row][col].data[bin_data[row][col].count] = i;
      bin_data[row][col].count++;
    }


  //  Determine which points need to have their waveforms evaluated.  This uses the dreaded Hockey Puck of Confidence (TM).  We only want
  //  to search in one bin around the current bin.  This means we'll search 9 total bins and that should give us enough nearby data for
  //  any point in the center bin.

  for (NV_INT32 i = 0 ; i < rows ; i++)
    {
      //  Compute the start and end Y bins for the 9 bin block.

      NV_INT32 start_y = qMax (0, i - 1);
      NV_INT32 end_y = qMin (rows - 1, i + 1);

      for (NV_INT32 j = 0 ; j < cols ; j++)
        {
          //  No point in checking if we have no data points in the bin.

          if (bin_data[i][j].count)
            {
              //  Compute the start and end X bins for the 9 bin block.

              NV_INT32 start_x = qMax (0, j - 1);
              NV_INT32 end_x = qMin (cols - 1, j + 1);


              //  Loop through the current bin checking against all points in any of the 9 bins.

              for (NV_INT32 k = 0 ; k < bin_data[i][j].count ; k++)
                {
                  NV_INT32 ndx = bin_data[i][j].data[k];


                  //  If we've already determined that this point doesn't need to be checked we can move on.

                  if (wave_data[ndx].check)
                    {
                      NV_BOOL only_one_line = NVTrue;


                      //  Y bin block loop.

                      for (NV_INT32 m = start_y ; m <= end_y ; m++)
                        {
                          //  X bin block loop.

                          for (NV_INT32 n = start_x ; n <= end_x ; n++)
                            {
                              //  No point in checking empty bins.

                              if (bin_data[m][n].count)
                                {
                                  //  Loop though all points in the bin.

                                  for (NV_INT32 p = 0 ; p < bin_data[m][n].count ; p++)
                                    {
                                      NV_INT32 indx = bin_data[m][n].data[p];


                                      //  Don't check against itself and don't check against invalid data.

                                      if ((m != i || n != j || ndx != indx) && !(misc.data[indx].val & PFM_INVAL) && !misc.data[indx].exflag)
                                        {
                                          //  If the points are in the same line we don't check them.

                                          if (misc.data[ndx].line != misc.data[indx].line)
                                            {
                                              if (misc.data[ndx].rec == 528045 && misc.data[indx].rec == 553904) fprintf (stderr,"%s %d %d\n",__FILE__,__LINE__,misc.data[ndx].exflag);

                                              //  Simple check for exceeding distance in X or Y direction (prior to a radius check).

                                              NV_FLOAT64 diff_x = fabs (wave_data[ndx].mx - wave_data[indx].mx);
                                              NV_FLOAT64 diff_y = fabs (wave_data[ndx].my - wave_data[indx].my);

                                              NV_FLOAT64 dist = misc.abe_share->filterShare.search_radius + misc.data[ndx].herr + misc.data[indx].herr;

                                              if (diff_x <= dist && diff_y <= dist)
                                                {
                                              if (misc.data[ndx].rec == 528045 && misc.data[indx].rec == 553904) DPRINT
                                                  //  Next check the distance.  If we're within this distance, the point is valid, and it's from a different file
                                                  //  we don't need to check either of these points.

                                                  if (sqrt (diff_x * diff_x + diff_y * diff_y) <= dist)
                                                    {
                                              if (misc.data[ndx].rec == 528045 && misc.data[indx].rec == 553904) DPRINT
                                                      only_one_line = NVFalse;


                                                      //  Finally we check the Z difference.

                                                      if (fabs (misc.data[ndx].z - misc.data[indx].z) < ((misc.data[ndx].verr + misc.data[indx].verr) / 2.0))
                                                        {
                                              if (misc.data[ndx].rec == 528045 && misc.data[indx].rec == 553904) DPRINT
                                                          wave_data[ndx].check = wave_data[indx].check = NVFalse;
                                                          break;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }


                      //  If there was only data from a single line within the radius we're not going to try to filter this point.
                      //  That is a job for the analyst.

                      if (only_one_line) wave_data[ndx].check = NVFalse;
                    }
                }
            }
        }
    }


  //  Now let's do the waveform check on those points that need it.  We only want to search in one bin around the current bin.  This means
  //  we'll search 9 total bins and that should give us enough nearby data for any point in the center bin.

  for (NV_INT32 i = 0 ; i < rows ; i++)
    {
      //  Compute the start and end Y bins for the 9 bin block.

      NV_INT32 start_y = qMax (0, i - 1);
      NV_INT32 end_y = qMin (rows - 1, i + 1);

      for (NV_INT32 j = 0 ; j < cols ; j++)
        {
          //  No point in checking if we have no data points in the bin.

          if (bin_data[i][j].count)
            {
              //  Compute the start and end X bins for the 9 bin block.

              NV_INT32 start_x = qMax (0, j - 1);
              NV_INT32 end_x = qMin (cols - 1, j + 1);


              //  Loop through the current bin checking against all points in any of the 9 bins.

              for (NV_INT32 k = 0 ; k < bin_data[i][j].count ; k++)
                {
                  NV_INT32 ndx = bin_data[i][j].data[k];


                  //  If we've already determined that this point doesn't need to be checked we can move on.

                      if (misc.data[ndx].rec == 528045) DPRINT
                  if (wave_data[ndx].check)
                    {
                      //  Y bin block loop.

                      for (NV_INT32 m = start_y ; m <= end_y ; m++)
                        {
                          //  X bin block loop.

                          for (NV_INT32 n = start_x ; n <= end_x ; n++)
                            {
                              //  No point in checking empty bins.

                              if (bin_data[m][n].count)
                                {
                                  //  Loop though all points in the bin.

                                  for (NV_INT32 p = 0 ; p < bin_data[m][n].count ; p++)
                                    {
                                      NV_INT32 indx = bin_data[m][n].data[p];


                                      //  Don't check against itself and don't check against invalid data.

                                      if ((m != i || n != j || ndx != indx) && !(misc.data[indx].val & PFM_INVAL) && !misc.data[indx].exflag)
                                        {
                                          //  If the points are in the same line we don't check them.

                                          if (misc.data[ndx].line != misc.data[indx].line)
                                            {
                                              //  Simple check for exceeding distance in X or Y direction (prior to a radius check).

                                              NV_FLOAT64 diff_x = fabs (wave_data[ndx].mx - wave_data[indx].mx);
                                              NV_FLOAT64 diff_y = fabs (wave_data[ndx].my - wave_data[indx].my);

                                              NV_FLOAT64 dist = misc.abe_share->filterShare.search_radius + misc.data[ndx].herr + misc.data[indx].herr;

                                              if (diff_x <= dist && diff_y <= dist)
                                                {
                                                  //  Next check the distance.  If we're within this distance, the point is valid, and it's from a different file
                                                  //  we don't need to check either of these points.

                                                  if (sqrt (diff_x * diff_x + diff_y * diff_y) <= dist)
                                                    {
                                                      if ((misc.points = (NV_INT32 *) realloc (misc.points, (misc.point_count + 1) * sizeof (NV_INT32))) == NULL)
                                                        {
                                                          perror ("Allocating points memory in hofWaveFilter.cpp");
                                                          misc.dataShare->unlock ();
                                                          exit (-1);
                                                        }

                                                      misc.points[misc.point_count] = indx;
                                                      misc.point_count++;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }


                      //  Now we have to look at the waveforms for all of the points in the search radius.

                      if (waveform_check (&misc, wave_data, ndx))
                        {
                          //  No supporting waveforms.

                          misc.data[ndx].exflag = NVTrue;
                        }


                      //  Free the point memory to prepare for the next point.

                      if (misc.point_count)
                        {
                          free (misc.points);
                          misc.points = NULL;
                          misc.point_count = 0;
                        }
                    }
                }
            }
        }
    }


  //  Free all of the memory we allocated.

  for (NV_INT32 i = 0 ; i < rows ; i++)
    {
      for (NV_INT32 j = 0 ; j < cols ; j++)
        {
          if (bin_data[i][j].count) free (bin_data[i][j].data);
        }
      free (bin_data[i]);
    }
  free (bin_data);

  free (wave_data);


  //  Lock shared memory while we're modifying things.

  misc.abeShare->lock ();

  misc.abe_share->modcode = PFM_CHARTS_HOF_DATA;

  misc.abeShare->unlock ();


  //  Unlock the data shared memory area.

  misc.dataShare->unlock ();


  //  Detach shared memory.

  misc.dataShare->detach ();
  misc.abeShare->detach ();
}


hofWaveFilter::~hofWaveFilter ()
{
}
