#include "hofReturnKill.hpp"
#include "version.hpp"


/***************************************************************************\
*                                                                           *
*   Module Name:        hofReturnKill                                       *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       December 05, 2007                                   *
*                                                                           *
*   Purpose:            Invalidate HOF returns in a portion of a PFM based  *
*                       on the slope and amplitude of the associated        *
*                       waveforms.                                          *
*                                                                           *
*   Caveats:            This program is not meant to be run from the        *
*                       command line.  It should only be run as a QProcess  *
*                       from pfmEdit.                                       *
*                                                                           *
\***************************************************************************/

NV_INT32 main (NV_INT32 argc, NV_CHAR **argv)
{
  hofReturnKill *hr;

  hr = new hofReturnKill (argc, argv);
}


void hofReturnKill::usage ()
{
  fprintf (stderr, "\nUsage: hofReturnKill [-s] --shared_file SHARED_FILE\n");
  fprintf (stderr, "\nWhere:\n\n");
  fprintf (stderr, "\t-s = invalidate Shoreline Depth Swapped (abdc = 72) or \n");
  fprintf (stderr, "\t\tShallow Water Algorithm (abdc = 74) processed data.\n");
  fprintf (stderr, "\tSHARED_FILE = shared file name from pfmEdit or pfmEdit3D\n\n");
  fprintf (stderr, "This program is not meant to be run from the command line.  It should only be\n");
  fprintf (stderr, "run as a QProcess from pfmEdit or pfmEdit3D.\n\n");
  fflush (stderr);
}


hofReturnKill::hofReturnKill (NV_INT32 argc, NV_CHAR **argv)
{
  NV_CHAR            wave_file[512], shared_file[512], tmp_file[512], string[1024];
  FILE               *fp = NULL, *wfp = NULL, *shfp = NULL, *shfp2 = NULL;
  HYDRO_OUTPUT_T     hof_record;
  WAVE_HEADER_T      wave_header;
  static WAVE_DATA_T wave_data;
  NV_CHAR            c;
  extern char        *optarg;
  NV_INT32           rec_num, pmt_run_req = 0, apd_run_req = 0, point, count;
  NV_FLOAT32         slope_req = 0.75;
  NV_BOOL            new_file = NVTrue, kill_shallow = NVFalse;
  SHARED_FILE_STRUCT data;


  NV_BOOL pmt_return_filter (NV_INT32 rec, NV_INT32 sub_rec, HYDRO_OUTPUT_T *hof_record, NV_INT32 pmt_run_req, NV_FLOAT32 slope_req, WAVE_DATA_T *wave_data);
  NV_BOOL apd_return_filter (NV_INT32 rec, NV_INT32 sub_rec, HYDRO_OUTPUT_T *hof_record, NV_INT32 apd_run_req, NV_FLOAT32 slope_req, WAVE_DATA_T *wave_data);


  if (argc < 2)
    {
      usage ();
      exit (-1);
    }


  NV_INT32 option_index = 0;
  NV_INT32 key = 0;
  while (NVTrue) 
    {
      static struct option long_options[] = {{"shared_file", required_argument, 0, 0},
					     {"shared_memory_key", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      c = (NV_CHAR) getopt_long (argc, argv, "s", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              strcpy (shared_file, optarg);
              break;

            case 1:
	      sscanf (optarg, "%d", &key);
              break;
            }

          break;

        case 's':
          kill_shallow = NVTrue;
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


  //  Get the shared memory area.  If it doesn't exist, quit.  It should
  //  have already been created by pfmEdit.  The key is the process ID of the bin viewer (pfmView)
  //  plus _abe.

  if (!key)
    {
      fprintf (stderr, "\n\n--shared_memory_key option not specified on command line.  Terminating!\n\n");
      exit (-1);
    }

  QString skey;
  skey.sprintf ("%d_abe", key);

  abeShare = new QSharedMemory (skey);

  if (!abeShare->attach (QSharedMemory::ReadWrite))
    {
      fprintf (stderr, "\n\nError retrieving shared memory segment in %s.\n\n", argv[0]);
      exit (-1);
    }

  abe_share = (ABE_SHARE *) abeShare->data ();


  if ((shfp = fopen (shared_file, "r")) == NULL)
    {
      perror (shared_file);
      return;
    }

  sprintf (tmp_file, "%s_2", shared_file);
  if ((shfp2 = fopen (tmp_file, "w")) == NULL)
    {
      perror (tmp_file);
      return;
    }


  new_file = NVTrue;
  fp = NULL;
  wfp = NULL;
  NV_BOOL mod = NVFalse;
  while (ngets (string, sizeof (string), shfp) != NULL)
    {
      if (strstr (string, "EOF")) break;

      if (strstr (string, "EOD"))
        {
          new_file = NVTrue;
        }
      else
        {
          if (new_file)
            {
              if (fp) fclose (fp);
              if (wfp) fclose (wfp);

              if ((fp = open_hof_file (string)) == NULL)
                {
                  perror (string);
                  return;
                }


              strcpy (wave_file, string);
              sprintf (&wave_file[strlen (wave_file) - 4], ".inh");


              if ((wfp = open_wave_file (wave_file)) == NULL) 
                {
                  perror (wave_file);
                  return;
                }

              wave_read_header (wfp, &wave_header);

              new_file = NVFalse;
            }
          else
            {
              NV_INT32 tmpi;

              sscanf (string, "%d %d %lf %lf %f %f %f %d %hd %hd %d %d %d %d", &point, &count, &data.x, &data.y, 
                      &data.z, &data.herr, &data.verr, &data.val, &data.pfm, &data.file, &data.line, &data.rec, &data.sub, &tmpi);
              data.exflag = (NV_BOOL) tmpi;

              rec_num = data.rec;

              hof_read_record (fp, rec_num, &hof_record);


              apd_run_req = hof_record.calc_bot_run_required[0];
              pmt_run_req = hof_record.calc_bot_run_required[1];


              killed = NVFalse;


              if (kill_shallow)
                {
                  if (hof_record.abdc == 72 || hof_record.abdc == 74)
                    {
                      killed = NVTrue;
                      data.val = PFM_FILTER_INVAL;
                      mod = NVTrue;
                    }
                }
              else
                {
                  if (wfp != NULL)
                    {
                      //  Read the corresponding wave data.

                      wave_read_record (wfp, rec_num, &wave_data);


                      //  Check to see if the sub_record we're looking for is PMT.

                      if ((data.sub == 0 && hof_record.bot_channel == PMT) || (data.sub == 1 && hof_record.sec_bot_chan == PMT))
                        {
                          killed = pmt_return_filter (data.rec, data.sub, &hof_record, pmt_run_req, slope_req, &wave_data);
                          if (killed)
                            {
                              data.val = PFM_MANUALLY_INVAL;
                              mod = NVTrue;
                            }
                        }


                      //  Check to see if the sub_record we're looking for is APD.

                      if ((data.sub == 0 && hof_record.bot_channel == APD) || (data.sub == 1 && hof_record.sec_bot_chan == APD))
                        {
                          killed = apd_return_filter (data.rec, data.sub, &hof_record, apd_run_req, slope_req, &wave_data);
                          if (killed)
                            {
                              data.val = PFM_MANUALLY_INVAL;
                              mod = NVTrue;
                            }
                        }
                    }
                }

              fprintf (shfp2, "%d %d %0.9f %0.9f %0.3f %0.3f %0.3f %d %d %d %d %d %d %d %d\n", point, count, data.x, data.y,
                       data.z, data.herr, data.verr, data.val, data.pfm, data.file, data.line, data.rec, data.sub, data.exflag, killed);
            }
        }
    }

  fprintf (shfp2, "EOF\n");
  fclose (shfp);
  fclose (shfp2);
  remove (shared_file);
  rename (tmp_file, shared_file);


  //  Lock shared memory while we're modifying things.

  abeShare->lock ();


  if (mod)
    {
      abe_share->modcode = PFM_CHARTS_HOF_DATA;
    }
  else
    {
      abe_share->modcode = NO_ACTION_REQUIRED;
    }


  abeShare->unlock ();
  abeShare->detach ();
}


hofReturnKill::~hofReturnKill ()
{
}
