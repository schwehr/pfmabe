
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



#include "pfmEditShell.hpp"
#include "version.hpp"


/***************************************************************************\
*                                                                           *
*   Module Name:        pfmEditShell                                        *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       December 15, 2010                                   *
*                                                                           *
*   Purpose:            Simple shell program to allow CZMIL (or other       *
*                       programs) to shell pfmEdit(3D) without having to    *
*                       run pfmView.                                        *
*                                                                           *
\***************************************************************************/

NV_INT32 main (NV_INT32 argc, NV_CHAR **argv)
{
  QApplication a (argc, argv);

  pfmEditShell *pes;

  pes = new pfmEditShell (&argc, argv);

  return a.exec ();
}


void pfmEditShell::usage ()
{
  fprintf (stderr, "\nUsage: pfmEditShell OPTIONS PFM_FILENAME\n");
  fprintf (stderr, "\nOPTIONS:\n\n");
  fprintf (stderr, "\t--share = A numeric value to be used to label shared memory for\n");
  fprintf (stderr, "\t\tthis process, it's parent process, and it's child processes.\n");
  fprintf (stderr, "\t\tIf the value is negative it means that shared memory will\n");
  fprintf (stderr, "\t\tbe destroyed and all child processes will be killed.\n");
  fprintf (stderr, "\t\tIf --share is not provided, the process ID will be used.\n");
  fprintf (stderr, "\t\tHere's an example of how this could be used.  A parent program\n");
  fprintf (stderr, "\t\tstarts this program with a --share value of 1000.  This program\n");
  fprintf (stderr, "\t\tcan run, exit, and be restarted by the parent any number of\n");
  fprintf (stderr, "\t\ttimes.  When the parent program starts this program with a\n");
  fprintf (stderr, "\t\t--share value of \"-1000\" all child processes of this program\n");
  fprintf (stderr, "\t\twill be terminated, shared memory will be released, and this\n");
  fprintf (stderr, "\t\tprogram will be terminated.\n");
  fprintf (stderr, "\t--min_hsv_color = HSV color wheel value (0-315) for minimum data\n");
  fprintf (stderr, "\t--max_hsv_color = HSV color wheel value (0-315) for maximum data\n");
  fprintf (stderr, "\t--min_hsv_value = Value below which minimum color will be used\n");
  fprintf (stderr, "\t--max_hsv_value = Value above which maximum color will be used\n");
  fprintf (stderr, "\t--force_auto_unload forces pfmEdit(3D) to unload edits on save/exit.\n");
  fprintf (stderr, "\t--2D runs pfmEdit instead of pfmEdit3D.\n");
  fprintf (stderr, "\t--area_file = Area file name to be used to define area to edit\n");
  fprintf (stderr, "\t--nsew = North, south, east, west boundaries used to define area\n");
  fprintf (stderr, "\t\tto edit.  Argument should be in quotes (\").  Fields must be\n");
  fprintf (stderr, "\t\tcomma separated degrees (ex. \"27.5,26.5,88.534,87.999\")\n\n");
  fprintf (stderr, "\tNote that --area_file and --nsew are mutually exclusive and that\n");
  fprintf (stderr, "\tone of them is required as is PFM_FILENAME.\n\n");
  fflush (stderr);
  exit (-1);
}


pfmEditShell::pfmEditShell (NV_INT32 *argc, NV_CHAR **argv)
{
  NV_CHAR            area_file[512];
  NV_BOOL            area_defined = NVFalse, kill_all;
  NV_F64_XYMBR       command_line_mbr = {999.0, 999.0, 999.0, 999.0};


  NV_BOOL envin (OPTIONS *options, MISC *misc);
  void set_defaults (MISC *misc, OPTIONS *options);


  if (*argc < 3) usage ();


  //  Set all of the normal defaults in case there are no user defaults

  set_defaults (&misc, &options);


  //  Get the user's defaults if available

  envin (&options, &misc);


  extern char *optarg;
  extern int optind;
  NV_INT32 option_index = 0;

  nsew = NVFalse;
  threeD_edit = NVTrue;
  force_auto_unload = NVFalse;
  kill_all = NVFalse;

  while (NVTrue) 
    {
      static struct option long_options[] = {{"min_hsv_color", required_argument, 0, 0},
                                             {"max_hsv_color", required_argument, 0, 0},
                                             {"min_hsv_value", required_argument, 0, 0},
                                             {"max_hsv_value", required_argument, 0, 0},
                                             {"area_file", required_argument, 0, 0},
                                             {"nsew", required_argument, 0, 0},
                                             {"force_auto_unload", no_argument, 0, 0},
                                             {"2D", no_argument, 0, 0},
                                             {"share", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              sscanf (optarg, "%hd", &options.min_hsv_color);
              break;

            case 1:
              sscanf (optarg, "%hd", &options.max_hsv_color);
              break;

            case 2:
              sscanf (optarg, "%f", &options.min_hsv_value);
              options.min_hsv_locked = NVTrue;
              break;

            case 3:
              sscanf (optarg, "%f", &options.max_hsv_value);
              options.max_hsv_locked = NVTrue;
              break;

            case 4:
              area_defined = NVTrue;

              strcpy (area_file, optarg);
              get_area_mbr (optarg, &misc.poly_count, misc.polygon_x, misc.polygon_y, &command_line_mbr);
              break;

            case 5:
              area_defined = nsew = NVTrue;

              sscanf (optarg, "%lf,%lf,%lf,%lf", &command_line_mbr.max_y, &command_line_mbr.min_y, &command_line_mbr.max_x, &command_line_mbr.min_x);


              //  Check the bounds for the correct order (NSEW)

              NV_FLOAT64 tmp_f64;
              if (command_line_mbr.max_y < command_line_mbr.min_y)
                {
                  tmp_f64 = command_line_mbr.min_y;
                  command_line_mbr.min_y = command_line_mbr.max_y;
                  command_line_mbr.max_y = tmp_f64;
                }
              if (command_line_mbr.max_x < command_line_mbr.min_x)
                {
                  tmp_f64 = command_line_mbr.min_x;
                  command_line_mbr.min_x = command_line_mbr.max_x;
                  command_line_mbr.max_x = tmp_f64;
                }
              break;

            case 6:
              force_auto_unload = NVTrue;
              break;

            case 7:
              threeD_edit = NVFalse;
              break;

            case 8:
              sscanf (optarg, "%d", &misc.process_id);
              break;
            }
          break;
        }
    }


  //  Check the min and max colors and flip them if needed.

  if (options.max_hsv_color > 315) options.max_hsv_color = 315;
  if (options.min_hsv_color > 315) options.min_hsv_color = 0;

  if (options.max_hsv_color < options.min_hsv_color)
    {
      NV_U_INT16 tmpu16 = options.max_hsv_color;
      options.max_hsv_color = options.min_hsv_color;
      options.min_hsv_color = tmpu16;
    }


  // Make sure we got the mandatory file name argument.

  if (optind >= *argc || !area_defined) usage ();


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


  //  Get the ABE shared memory area.  If it exists, use it.  The key is the --share value passed on the command line
  //  (or the process ID if the --share argument isn't used) plus "_abe".  If the value is a negative we're going to
  //  kill everything MWAH HA HA HA!!!

  if (misc.process_id < 0)
    {
      kill_all = NVTrue;
      misc.process_id = -misc.process_id;
    }


  QString skey;
  skey.sprintf ("%d_abe", misc.process_id);

  misc.abeShare = new QSharedMemory (skey);

  if (!misc.abeShare->create (sizeof (ABE_SHARE), QSharedMemory::ReadWrite)) misc.abeShare->attach (QSharedMemory::ReadWrite);

  misc.abe_share = (ABE_SHARE *) misc.abeShare->data ();


  //  If the process ID supplied with the --share argument was negative, kill them all.

  if (kill_all)
    {
      misc.abe_share->key = CHILD_PROCESS_FORCE_EXIT;


      //  Wait 2 seconds so that all associated programs will see the modified flag.

#ifdef NVWIN3X
      usleep (2000000);
#else
      sleep (2);
#endif

      slotQuit (0);
    }


  //  Clear the shared memory area and set the ppid key.

  memset (misc.abe_share, 0, sizeof (ABE_SHARE));
  misc.abe_share->ppid = misc.process_id;


  //  Place the HSV color info, that may have come in on the command line, into shared memory.

  misc.abe_share->min_hsv_color = options.min_hsv_color;
  misc.abe_share->max_hsv_color = options.max_hsv_color;
  misc.abe_share->min_hsv_value = options.min_hsv_value;
  misc.abe_share->max_hsv_value = options.max_hsv_value;
  misc.abe_share->min_hsv_locked = options.min_hsv_locked;
  misc.abe_share->max_hsv_locked = options.max_hsv_locked;


  //  Put the PFM file name into shared memory and open the file.

  misc.abe_share->pfm_count = 1;
  misc.abe_share->open_args[0].checkpoint = 0;
  strcpy (misc.abe_share->open_args[0].list_path, argv[optind]);

  pfm_handle = open_existing_pfm_file (&misc.abe_share->open_args[0]);

  if (pfm_handle < 0)
    {
      fprintf (stderr, "The file %s is not a PFM structure or there was an error reading the file.\nThe error message returned was:%s\n\n",
               misc.abe_share->open_args[0].list_path, pfm_error_str (pfm_error));
      usage ();
    }

  if (misc.abe_share->open_args[0].head.mbr.max_x > 180.0 && misc.abe_share->open_args[0].head.mbr.min_x < 180.0) misc.dateline = NVTrue;

  misc.abe_share->display_pfm[0] = NVTrue;


  //  Set a couple of things that pfmEdit(3D) will need to know.

  misc.abe_share->settings_changed = NVFalse;
  misc.abe_share->zoom_requested = NVFalse;
  misc.abe_share->position_form = options.position_form;
  misc.abe_share->smoothing_factor = options.smoothing_factor;
  misc.abe_share->z_factor = options.z_factor;
  misc.abe_share->z_offset = options.z_offset;


  //  Before the first pfmEdit(3D), attribute viewer running is guaranteed to be false

  misc.abe_share->avShare.avRunning = NVFalse;


  //  Move stuff from options to share.

  misc.abe_share->layer_type = options.layer_type;
  misc.abe_share->cint = options.cint;
  misc.abe_share->num_levels = options.num_levels;
  for (NV_INT32 i = 0 ; i < options.num_levels ; i++) misc.abe_share->contour_levels[i] = options.contour_levels[i];
  strcpy (misc.abe_share->feature_search_string, options.feature_search_string.toAscii ());


  //  Adjust the command line arguments to the bounds of the just opened PFM.

  command_line_mbr.max_x = qMin (command_line_mbr.max_x, misc.abe_share->open_args[0].head.mbr.max_x);
  command_line_mbr.max_y = qMin (command_line_mbr.max_y, misc.abe_share->open_args[0].head.mbr.max_y);
  command_line_mbr.min_x = qMax (command_line_mbr.min_x, misc.abe_share->open_args[0].head.mbr.min_x);
  command_line_mbr.min_y = qMax (command_line_mbr.min_y, misc.abe_share->open_args[0].head.mbr.min_y);


  //  If we just entered north, south, east, west on the command line...

  if (nsew)
    {
      NV_FLOAT64 mx[4], my[4];

      mx[0] = command_line_mbr.min_x;
      my[0] = command_line_mbr.min_y;
      mx[1] = command_line_mbr.min_x;
      my[1] = command_line_mbr.max_y;
      mx[2] = command_line_mbr.max_x;
      my[2] = command_line_mbr.max_y;
      mx[3] = command_line_mbr.max_x;
      my[3] = command_line_mbr.min_y;

      editCommand (mx, my, 4);
    }
  else
    {
      editCommand (misc.polygon_x, misc.polygon_y, misc.poly_count);
    }
}


pfmEditShell::~pfmEditShell ()
{
}



//  If we errored out of the pfmEdit(3D) process...

void 
pfmEditShell::slotEditError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      fprintf (stderr, "Unable to start the edit process!\n");
      break;

    case QProcess::Crashed:
      fprintf (stderr, "The edit process crashed!\n");
      break;

    case QProcess::Timedout:
      fprintf (stderr, "The edit process timed out!\n");
      break;

    case QProcess::WriteError:
      fprintf (stderr, "There was a write error to the edit process!\n");
      break;

    case QProcess::ReadError:
      fprintf (stderr, "There was a read error from the edit process!\n");
      break;

    case QProcess::UnknownError:
      fprintf (stderr, "The edit process died with an unknown error!\n");
      break;
    }

  slotQuit (1);
}



//  This is the return from the edit QProcess (when finished normally)

void 
pfmEditShell::slotEditDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  if (pfmEditMod) pfmViewMod = NVTrue;

  if (misc.abe_share->settings_changed)
    {
      misc.abe_share->settings_changed = NVFalse;
      options.smoothing_factor = misc.abe_share->smoothing_factor;
      options.z_factor = misc.abe_share->z_factor;
      options.z_offset = misc.abe_share->z_offset;
      options.position_form = misc.abe_share->position_form;
      options.min_hsv_color = misc.abe_share->min_hsv_color;
      options.max_hsv_color = misc.abe_share->max_hsv_color;
      options.min_hsv_value = misc.abe_share->min_hsv_value;
      options.max_hsv_value = misc.abe_share->max_hsv_value;
      options.min_hsv_locked = misc.abe_share->min_hsv_locked;
      options.max_hsv_locked = misc.abe_share->max_hsv_locked;
    }

  slotQuit (0);
}



//  This is the stderr read return from the edit QProcess.  Hopefully you won't see anything here.

void 
pfmEditShell::slotEditReadyReadStandardError ()
{
  QByteArray response = editProc->readAllStandardError ();
  NV_CHAR *res = response.data ();


  //  Let's not print out the "QProcess: Destroyed while process is still running" messages that come from
  //  killing ancillary programs in the editor.

  if (!strstr (res, "Destroyed while"))
    {
      fprintf (stderr, "%s %d %s\n", __FILE__, __LINE__, res);
      fflush (stderr);
    }
}



//  This is the stdout read return from the edit QProcess
//  We need to look at the value returned on exit to see if anything changed.
//  pfmEdit(3D) will print out a 0 for no changes, a 1 for PFM structure changes, or a 2 for feature changes.
//  The second number is the filter mask flag which is irrelevant for this program.

void 
pfmEditShell::slotEditReadyReadStandardOutput ()
{
  QByteArray response = editProc->readAllStandardOutput ();
  NV_CHAR *res = response.data ();


  //  Only those messages that begin with "Edit return status:" are valid.  The rest may be error messages.

  if (!strncmp (res, "Edit return status:", 19))
    {
      sscanf (res, "Edit return status:%d,%d", &pfmEditMod, &pfmEditFilt);
    }
  else
    {
      //  Let's not print out the "QProcess: Destroyed while process is still running" messages that come from
      //  killing ancillary programs in the editor.

      if (!strstr (res, "Destroyed while"))
        {
          fprintf (stdout, "%s %d %s\n", __FILE__, __LINE__, res);
          fflush (stdout);
        }
    }
}



//  Kick off the edit QProcess

void 
pfmEditShell::editCommand (NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count)
{
  pfmEditMod = 0;
  pfmEditFilt = 0;


  //  Compute the minimum bounding rectangle for the edit area.

  NV_FLOAT64 min_x = 999.0;
  NV_FLOAT64 max_x = -999.0;
  NV_FLOAT64 min_y = 999.0;
  NV_FLOAT64 max_y = -999.0;
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

      misc.abe_share->polygon_x[i] = mx[i];
      misc.abe_share->polygon_y[i] = my[i];
      if (mx[i] < min_x) min_x = mx[i];
      if (mx[i] > max_x) max_x = mx[i];
      if (my[i] < min_y) min_y = my[i];
      if (my[i] > max_y) max_y = my[i];
    }


  misc.abe_share->edit_area.min_x = min_x;
  misc.abe_share->edit_area.max_x = max_x;
  misc.abe_share->edit_area.min_y = min_y;
  misc.abe_share->edit_area.max_y = max_y;


  //  For rectangles we pass a count of zero to tell pfmEdit(3D) that it's a rectangle.

  if (nsew)
    {
      misc.abe_share->polygon_count = 0;
    }
  else
    {
      misc.abe_share->polygon_count = count;
    }


  editProc = new QProcess (this);


  QStringList arguments;
  QString arg;


  //  Always add the shared memory ID (the process ID).

  arg.sprintf ("--shared_memory_key=%d", misc.abe_share->ppid);
  arguments += arg;


  //  Check for the force auto unload argument

  if (force_auto_unload)
    {
      arg.sprintf ("--force_auto_unload");
      arguments += arg;
    }


  connect (editProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotEditDone (int, QProcess::ExitStatus)));
  connect (editProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotEditReadyReadStandardError ()));
  connect (editProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotEditReadyReadStandardOutput ()));
  connect (editProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotEditError (QProcess::ProcessError)));


  if (threeD_edit)
    {
      editProc->start (QString (options.edit_name_3D), arguments);
    }
  else
    {
      editProc->start (QString (options.edit_name), arguments);
    }
}



void 
pfmEditShell::slotQuit (NV_INT32 status)
{
  //  Detach from shared memory.

  misc.abeShare->detach ();


  //  Close the PFM file.

  close_pfm_file (pfm_handle);


  //  Output the mod status.

  if (pfmViewMod)
    {
      fprintf (stdout, "View return status:1\n");
    }
  else
    {
      fprintf (stdout, "View return status:0\n");
    }

  exit (status);
}
