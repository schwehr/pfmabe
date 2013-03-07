#include "rmsMonitor.hpp"
#include "rmsMonitorHelp.hpp"


NV_FLOAT64 settings_version = 1.00;


rmsMonitor::rmsMonitor (NV_INT32 *argc, NV_CHAR **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;
  
  lock_track = NVFalse;


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);
  setFocus (Qt::ActiveWindowFocusReason);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/charts_monitor.png"));


  kill_switch = ANCILLARY_FORCE_EXIT;

  NV_INT32 option_index = 0;
  while (NVTrue) 
    {
      static struct option long_options[] = {{"shared_memory_key", required_argument, 0, 0},
                                             {"kill_switch", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "o", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              sscanf (optarg, "%d", &key);
              break;

            case 1:
              sscanf (optarg, "%d", &kill_switch);
              break;

            default:
              NV_CHAR tmp;
              sscanf (optarg, "%1c", &tmp);
              ac[option_index] = (NV_U_INT32) tmp;
              break;
            }
          break;
        }
    }


  if (!envin ())
    {
      //  Set the geometry from defaults since we didn't get any from the saved settings.

      this->resize (324, 400);
      this->move (0, 0);
    }


  setWindowTitle (QString (VERSION));


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


  //  Get the shared memory area.  If it doesn't exist, quit.  It should have already been created 
  //  by pfmView or geoSwath.

  QString skey;
  skey.sprintf ("%d_abe", key);

  abeShare = new QSharedMemory (skey);

  if (!abeShare->attach (QSharedMemory::ReadWrite))
    {
      fprintf (stderr, "\n\nError retrieving shared memory segment in %s.\n\n", argv[0]);
      exit (-1);
    }

  abe_share = (ABE_SHARE *) abeShare->data ();


  //  Open all the PFM structures that are being viewed in pfmEdit/pfmView

  num_pfms = abe_share->pfm_count;

  for (NV_INT32 pfm = 0 ; pfm < num_pfms ; pfm++)
    {
      open_args[pfm] = abe_share->open_args[pfm];

      if ((pfm_handle[pfm] = open_existing_pfm_file (&open_args[pfm])) < 0)
	{
	  QMessageBox::warning (this, tr ("Open PFM Structure"),
				tr ("The file ") + QDir::toNativeSeparators (QString (abe_share->open_args[pfm].list_path)) + 
				tr (" is not a PFM handle or list file or there was an error reading the file.") +
				tr ("  The error message returned was:\n\n") +
				QString (pfm_error_str (pfm_error)));
	}
    }


  QFrame *frame = new QFrame (this, 0);
  
  setCentralWidget (frame);

  QVBoxLayout *vBox = new QVBoxLayout ();
  vBox->setSpacing(0);
  frame->setLayout (vBox);

  listBox = new QTextEdit (this);
  listBox->setAutoFillBackground (TRUE);
  listBoxPalette.setColor (QPalette::Normal, QPalette::Base, bg_color);
  listBoxPalette.setColor (QPalette::Normal, QPalette::Window, bg_color);
  listBoxPalette.setColor (QPalette::Inactive, QPalette::Base, bg_color);
  listBoxPalette.setColor (QPalette::Inactive, QPalette::Window, bg_color);
  listBox->setPalette (listBoxPalette);
  listBox->setReadOnly (TRUE);


  vBox->addWidget (listBox);


  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);


  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), this, SLOT (slotQuit ()));

  QAction *filePrefsAction= new QAction (tr ("&Prefs"), this);
  filePrefsAction->setStatusTip (tr ("Set Preferences"));
  connect (filePrefsAction, SIGNAL (triggered ()), this, SLOT (slotPrefs ()));


  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (filePrefsAction);
  fileMenu->addSeparator ();
  fileMenu->addAction (fileQuitAction);


  //  Setup the help menu.  I like leaving the About Qt in since this is
  //  a really nice package - and it's open source.

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about rmsMonitor"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *aboutQtAct = new QAction (tr ("About&Qt"), this);
  aboutQtAct->setShortcut (tr ("Ctrl+Q"));
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QAction *whatThisAct = QWhatsThis::createAction (this);

  QMenu *helpMenu = menuBar ()->addMenu (tr ("&Help"));
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (aboutQtAct);
  helpMenu->addAction (whatThisAct);


  //  Setup the tracking timer

  QTimer *track = new QTimer (this);
  connect (track, SIGNAL (timeout ()), this, SLOT (trackCursor ()));
  track->start (10);
}



rmsMonitor::~rmsMonitor ()
{
}


void rmsMonitor::closeEvent (QCloseEvent * event __attribute__ ((unused)))
{
  slotQuit ();
}

  

void 
rmsMonitor::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  force_redraw = NVTrue;
}



void
rmsMonitor::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  Timer - timeout signal.  Very much like an X workproc.

void
rmsMonitor::trackCursor ()
{
  NV_INT32                year, day, mday, month, hour, minute, type = 0;
  NV_INT64                timestamp = 0;
  NV_FLOAT32              second;
  static ABE_SHARE        l_share;
  FILE                    *cfp = NULL, *rfp = NULL;
  HOF_HEADER_T            hof_header;
  TOF_HEADER_T            tof_header;
  HYDRO_OUTPUT_T          hof_record;
  TOPO_OUTPUT_T           tof_record;
  QString                 string, string2;
  NV_CHAR                 rms_file[512];
  RMS_OUTPUT_T            rms_record;


  //  We want to exit if we have locked the tracker to update our saved waveforms (in slotPlotWaves).

  if (lock_track) return;


  //  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.
  //  We also want to exit on the ANCILLARY_FORCE_EXIT key (from pfmEdit) or if our own personal kill signal
  //  has been placed in abe_share->key.

  if (abe_share->key == CHILD_PROCESS_FORCE_EXIT || abe_share->key == ANCILLARY_FORCE_EXIT || abe_share->key == kill_switch) slotQuit ();


  //  We can force an update using the same key as waveMonitor since they're looking at the same sahred memory area.

  if (abe_share->modcode == WAVEMONITOR_FORCE_REDRAW) force_redraw = NVTrue; 


  //  Check for HOF, TOF, no memory lock, record change, force_redraw.

  if (((abe_share->mwShare.multiType[0] == PFM_SHOALS_1K_DATA || abe_share->mwShare.multiType[0] == PFM_SHOALS_TOF_DATA ||
        abe_share->mwShare.multiType[0] == PFM_CHARTS_HOF_DATA) && abe_share->key < NV_INT32_MAX &&
       l_share.nearest_point != abe_share->nearest_point) || force_redraw)
    {
      force_redraw = NVFalse;


      abeShare->lock ();
      l_share = *abe_share;
      abeShare->unlock ();


      //  Open the CHARTS file and read the data.

      if (strstr (l_share.nearest_filename, ".hof"))
        {
          cfp = open_hof_file (l_share.nearest_filename);

          if (cfp == NULL)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + QString (strerror (errno));
              statusBar ()->showMessage (string);

              return;
            }
          type = PFM_CHARTS_HOF_DATA;
        }
      else if (strstr (l_share.nearest_filename, ".tof"))
        {
          cfp = open_tof_file (l_share.nearest_filename);


          if (cfp == NULL)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + QString (strerror (errno));
              statusBar ()->showMessage (string);
              
              return;
            }
          type = PFM_SHOALS_TOF_DATA;
        }


      //  Erase the data

      listBox->clear ();


      switch (type)
        {
        case PFM_SHOALS_1K_DATA:
        case PFM_CHARTS_HOF_DATA:
          hof_read_header (cfp, &hof_header);

          hof_read_record (cfp, l_share.mwShare.multiRecord[0], &hof_record);

          timestamp = hof_record.timestamp;

          fclose (cfp);
          break;


        case PFM_SHOALS_TOF_DATA:
          tof_read_header (cfp, &tof_header);

          tof_read_record (cfp, l_share.mwShare.multiRecord[0], &tof_record);

          timestamp = hof_record.timestamp;

          fclose (cfp);
          break;
        }



      if (!get_rms_file (l_share.nearest_filename, rms_file))
        {
          string.sprintf (tr ("<font color=\"#ff0000\"><b>Unable to find RMS file for</b> %s</font>").toAscii (), l_share.nearest_filename);
          listBox->insertHtml (string);
          string = tr ("<br><br><font color=\"#ff0000\"><b>This is probably because it wasn't moved to the <b>pos</b> directory by 3001 after running POSPac.</b></font>");
          listBox->insertHtml (string);
          return;
        }

      if ((rfp = open_rms_file (rms_file)) == NULL)
        {
          string.sprintf (tr ("Unable to open RMS file %s").toAscii (), rms_file);
          listBox->insertHtml (string);
          return;
        }

      if (!rms_find_record (rfp, &rms_record, timestamp))
        {
          string.sprintf (tr ("Unable to find RMS record for timestamp "NV_INT64_SPECIFIER).toAscii (), timestamp);
          listBox->insertHtml (string);
          return;
        }

      fclose (rfp);

      string.sprintf (tr ("<b>Time Stamp : </b>").toAscii ());
      string2.sprintf (NV_INT64_SPECIFIER, timestamp);
      listBox->insertHtml (string + string2 + "<br>");

      charts_cvtime (hof_record.timestamp, &year, &day, &hour, &minute, &second);
      charts_jday2mday (year, day, &month, &mday);
      month++;
      string.sprintf (tr ("<b>Date/Time : </b>%d-%02d-%02d (%03d) %02d:%02d:%05.2f<br>").toAscii (), year + 1900, month, mday, day, hour, minute, second);
      listBox->insertHtml (string);

      if (rms_record.north_pos_rms > stop_light[0][0])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[1].red (), stop_color[1].green (), stop_color[1].blue ());
          string += tr ("<b>North position RMS error : </b>%1<br>").arg (rms_record.north_pos_rms, 0, 'f', 4) + "</font>";
        }
      else if (rms_record.north_pos_rms > stop_light[0][1])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[2].red (), stop_color[2].green (), stop_color[2].blue ());
          string += tr ("<b>North position RMS error : </b>%1<br>").arg (rms_record.north_pos_rms, 0, 'f', 4) + "</font>";
        }
      else
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[0].red (), stop_color[0].green (), stop_color[0].blue ());
          string += tr ("<b>North position RMS error : </b>%1<br>").arg (rms_record.north_pos_rms, 0, 'f', 4) + "</font>";
        }
      listBox->insertHtml (string);


      if (rms_record.south_pos_rms > stop_light[0][0])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[1].red (), stop_color[1].green (), stop_color[1].blue ());
          string += tr ("<b>South position RMS error : </b>%1<br>").arg (rms_record.south_pos_rms, 0, 'f', 4) + "</font>";
        }
      else if (rms_record.south_pos_rms > stop_light[0][1])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[2].red (), stop_color[2].green (), stop_color[2].blue ());
          string += tr ("<b>South position RMS error : </b>%1<br>").arg (rms_record.south_pos_rms, 0, 'f', 4) + "</font>";
        }
      else
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[0].red (), stop_color[0].green (), stop_color[0].blue ());
          string += tr ("<b>South position RMS error : </b>%1<br>").arg (rms_record.south_pos_rms, 0, 'f', 4) + "</font>";
        }
      listBox->insertHtml (string);


      if (rms_record.down_pos_rms > stop_light[0][0])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[1].red (), stop_color[1].green (), stop_color[1].blue ());
          string += tr ("<b>Down position RMS error : </b>%1<br>").arg (rms_record.down_pos_rms, 0, 'f', 4) + "</font>";
        }
      else if (rms_record.down_pos_rms > stop_light[0][1])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[2].red (), stop_color[2].green (), stop_color[2].blue ());
          string += tr ("<b>Down position RMS error : </b>%1<br>").arg (rms_record.down_pos_rms, 0, 'f', 4) + "</font>";
        }
      else
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[0].red (), stop_color[0].green (), stop_color[0].blue ());
          string += tr ("<b>Down position RMS error : </b>%1<br>").arg (rms_record.down_pos_rms, 0, 'f', 4) + "</font>";
        }
      listBox->insertHtml (string);


      if (rms_record.north_vel_rms > stop_light[1][0])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[1].red (), stop_color[1].green (), stop_color[1].blue ());
          string += tr ("<b>North velocity RMS error : </b>%1<br>").arg (rms_record.north_vel_rms, 0, 'f', 4) + "</font>";
        }
      else if (rms_record.north_vel_rms > stop_light[1][1])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[2].red (), stop_color[2].green (), stop_color[2].blue ());
          string += tr ("<b>North velocity RMS error : </b>%1<br>").arg (rms_record.north_vel_rms, 0, 'f', 4) + "</font>";
        }
      else
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[0].red (), stop_color[0].green (), stop_color[0].blue ());
          string += tr ("<b>North velocity RMS error : </b>%1<br>").arg (rms_record.north_vel_rms, 0, 'f', 4) + "</font>";
        }
      listBox->insertHtml (string);


      if (rms_record.south_vel_rms > stop_light[1][0])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[1].red (), stop_color[1].green (), stop_color[1].blue ());
          string += tr ("<b>South velocity RMS error : </b>%1<br>").arg (rms_record.south_vel_rms, 0, 'f', 4) + "</font>";
        }
      else if (rms_record.south_vel_rms > stop_light[1][1])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[2].red (), stop_color[2].green (), stop_color[2].blue ());
          string += tr ("<b>South velocity RMS error : </b>%1<br>").arg (rms_record.south_vel_rms, 0, 'f', 4) + "</font>";
        }
      else
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[0].red (), stop_color[0].green (), stop_color[0].blue ());
          string += tr ("<b>South velocity RMS error : </b>%1<br>").arg (rms_record.south_vel_rms, 0, 'f', 4) + "</font>";
        }
      listBox->insertHtml (string);


      if (rms_record.down_vel_rms > stop_light[1][0])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[1].red (), stop_color[1].green (), stop_color[1].blue ());
          string += tr ("<b>Down velocity RMS error : </b>%1<br>").arg (rms_record.down_vel_rms, 0, 'f', 4) + "</font>";
        }
      else if (rms_record.down_vel_rms > stop_light[1][1])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[2].red (), stop_color[2].green (), stop_color[2].blue ());
          string += tr ("<b>Down velocity RMS error : </b>%1<br>").arg (rms_record.down_vel_rms, 0, 'f', 4) + "</font>";
        }
      else
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[0].red (), stop_color[0].green (), stop_color[0].blue ());
          string += tr ("<b>Down velocity RMS error : </b>%1<br>").arg (rms_record.down_vel_rms, 0, 'f', 4) + "</font>";
        }
      listBox->insertHtml (string);


      if (rms_record.roll_rms > stop_light[2][0])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[1].red (), stop_color[1].green (), stop_color[1].blue ());
          string += tr ("<b>Roll RMS error : </b>%1<br>").arg (rms_record.roll_rms * ARC_TO_DEG, 0, 'f', 9) + "</font>";
        }
      else if (rms_record.roll_rms > stop_light[2][1])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[2].red (), stop_color[2].green (), stop_color[2].blue ());
          string += tr ("<b>Roll RMS error : </b>%1<br>").arg (rms_record.roll_rms * ARC_TO_DEG, 0, 'f', 9) + "</font>";
        }
      else
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[0].red (), stop_color[0].green (), stop_color[0].blue ());
          string += tr ("<b>Roll RMS error : </b>%1<br>").arg (rms_record.roll_rms * ARC_TO_DEG, 0, 'f', 9) + "</font>";
        }
      listBox->insertHtml (string);


      if (rms_record.pitch_rms > stop_light[2][0])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[1].red (), stop_color[1].green (), stop_color[1].blue ());
          string += tr ("<b>Pitch RMS error : </b>%1<br>").arg (rms_record.pitch_rms * ARC_TO_DEG, 0, 'f', 9) + "</font>";
        }
      else if (rms_record.pitch_rms > stop_light[2][1])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[2].red (), stop_color[2].green (), stop_color[2].blue ());
          string += tr ("<b>Pitch RMS error : </b>%1<br>").arg (rms_record.pitch_rms * ARC_TO_DEG, 0, 'f', 9) + "</font>";
        }
      else
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[0].red (), stop_color[0].green (), stop_color[0].blue ());
          string += tr ("<b>Pitch RMS error : </b>%1<br>").arg (rms_record.pitch_rms * ARC_TO_DEG, 0, 'f', 9) + "</font>";
        }
      listBox->insertHtml (string);


      if (rms_record.heading_rms > stop_light[2][0])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[1].red (), stop_color[1].green (), stop_color[1].blue ());
          string += tr ("<b>Heading RMS error : </b>%1<br>").arg (rms_record.heading_rms * ARC_TO_DEG, 0, 'f', 9) + "</font>";
        }
      else if (rms_record.heading_rms > stop_light[2][1])
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[2].red (), stop_color[2].green (), stop_color[2].blue ());
          string += tr ("<b>Heading RMS error : </b>%1<br>").arg (rms_record.heading_rms * ARC_TO_DEG, 0, 'f', 9) + "</font>";
        }
      else
        {
          string.sprintf ("<font color=\"#%02d%02d%02d\">", stop_color[0].red (), stop_color[0].green (), stop_color[0].blue ());
          string += tr ("<b>Heading RMS error : </b>%1<br>").arg (rms_record.heading_rms * ARC_TO_DEG, 0, 'f', 9) + "</font>";
        }
      listBox->insertHtml (string);
    }
}



//  A bunch of slots.

void 
rmsMonitor::slotQuit ()
{
  //  Let the parent program know that we have died from something other than the kill switch from the parent.

  if (abe_share->key != kill_switch) abe_share->killed = kill_switch;


  envout ();


  //  Let go of the shared memory.

  abeShare->detach ();


  exit (0);
}



void
rmsMonitor::slotPrefs ()
{
  prefsD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  prefsD->setWindowTitle (tr ("rmsMonitor Preferences"));
  prefsD->setModal (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (prefsD);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QGroupBox *cbox = new QGroupBox (tr ("Colors"), this);
  QHBoxLayout *cboxLayout = new QHBoxLayout;
  cbox->setLayout (cboxLayout);
  QVBoxLayout *cboxLeftLayout = new QVBoxLayout;
  QVBoxLayout *cboxRightLayout = new QVBoxLayout;
  cboxLayout->addLayout (cboxLeftLayout);
  cboxLayout->addLayout (cboxRightLayout);


  bStoplightLowColor = new QPushButton (tr ("Low stoplight"), this);
  bStoplightLowColor->setToolTip (tr ("Change low stoplight color (default green)"));
  bStoplightLowColor->setWhatsThis (stoplightLowColorText);
  stoplightLowPalette = bStoplightLowColor->palette ();
  connect (bStoplightLowColor, SIGNAL (clicked ()), this, SLOT (slotStoplightLowColor ()));
  cboxLeftLayout->addWidget (bStoplightLowColor);


  bStoplightMidColor = new QPushButton (tr ("Mid stoplight"), this);
  bStoplightMidColor->setToolTip (tr ("Change mid stoplight color (default yellow)"));
  bStoplightMidColor->setWhatsThis (stoplightMidColorText);
  stoplightMidPalette = bStoplightMidColor->palette ();
  connect (bStoplightMidColor, SIGNAL (clicked ()), this, SLOT (slotStoplightMidColor ()));
  cboxLeftLayout->addWidget (bStoplightMidColor);


  bStoplightHighColor = new QPushButton (tr ("High stoplight"), this);
  bStoplightHighColor->setToolTip (tr ("Change high stoplight color (default red)"));
  bStoplightHighColor->setWhatsThis (stoplightHighColorText);
  stoplightHighPalette = bStoplightHighColor->palette ();
  connect (bStoplightHighColor, SIGNAL (clicked ()), this, SLOT (slotStoplightHighColor ()));
  cboxRightLayout->addWidget (bStoplightHighColor);


  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundColor->setToolTip (tr ("Change background color"));
  bBackgroundColor->setWhatsThis (backgroundColorText);
  backgroundPalette = bBackgroundColor->palette ();
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotbackgroundColor ()));
  cboxRightLayout->addWidget (bBackgroundColor);


  vbox->addWidget (cbox);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (prefsD);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  bRestoreDefaults = new QPushButton (tr ("Restore Defaults"), this);
  bRestoreDefaults->setToolTip (tr ("Restore all preferences to the default state"));
  bRestoreDefaults->setWhatsThis (restoreDefaultsText);
  connect (bRestoreDefaults, SIGNAL (clicked ()), this, SLOT (slotRestoreDefaults ()));
  actions->addWidget (bRestoreDefaults);

  QPushButton *closeButton = new QPushButton (tr ("Close"), prefsD);
  closeButton->setToolTip (tr ("Close the preferences dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClosePrefs ()));
  actions->addWidget (closeButton);


  setFields ();


  prefsD->show ();
}



void 
rmsMonitor::setFields ()
{
  NV_INT32 hue, sat, val;

  stop_color[0].getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      stoplightLowPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      stoplightLowPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      stoplightLowPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      stoplightLowPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  stoplightLowPalette.setColor (QPalette::Normal, QPalette::Button, stop_color[0]);
  stoplightLowPalette.setColor (QPalette::Inactive, QPalette::Button, stop_color[0]);
  bStoplightLowColor->setPalette (stoplightLowPalette);


  stop_color[1].getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      stoplightMidPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      stoplightMidPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      stoplightMidPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      stoplightMidPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  stoplightMidPalette.setColor (QPalette::Normal, QPalette::Button, stop_color[1]);
  stoplightMidPalette.setColor (QPalette::Inactive, QPalette::Button, stop_color[1]);
  bStoplightMidColor->setPalette (stoplightMidPalette);

  stop_color[2].getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      stoplightHighPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      stoplightHighPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      stoplightHighPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      stoplightHighPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  stoplightHighPalette.setColor (QPalette::Normal, QPalette::Button, stop_color[2]);
  stoplightHighPalette.setColor (QPalette::Inactive, QPalette::Button, stop_color[2]);
  bStoplightHighColor->setPalette (stoplightHighPalette);


  bg_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      backgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      backgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      backgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      backgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  backgroundPalette.setColor (QPalette::Normal, QPalette::Button, bg_color);
  backgroundPalette.setColor (QPalette::Inactive, QPalette::Button, bg_color);
  bBackgroundColor->setPalette (backgroundPalette);
}



void
rmsMonitor::slotClosePrefs ()
{
  prefsD->close ();
}



void
rmsMonitor::slotStoplightLowColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (stop_color[0], this, tr ("rmsMonitor Low Stoplight Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) stop_color[0] = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
rmsMonitor::slotStoplightMidColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (stop_color[1], this, tr ("rmsMonitor Mid Stoplight Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) stop_color[1] = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
rmsMonitor::slotStoplightHighColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (stop_color[2], this, tr ("rmsMonitor High Stoplight Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) stop_color[2] = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
rmsMonitor::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (bg_color, this, tr ("rmsMonitor Background Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) bg_color = clr;

  force_redraw = NVTrue;
}



void
rmsMonitor::slotRestoreDefaults ()
{
  stop_light[0][0] = 10.0;
  stop_light[0][1] = 30.0;
  stop_light[1][0] = 0.0;
  stop_light[1][1] = 0.0;
  stop_light[2][0] = 0.0;
  stop_light[2][1] = 0.0;

  stop_color[0] = QColor (0, 255, 0, 255);
  stop_color[1] = QColor (255, 255, 0, 255);
  stop_color[2] = QColor (255, 0, 0, 255);

  bg_color = QColor (200, 200, 200, 255);

  force_redraw = NVTrue;
}



void
rmsMonitor::about ()
{
  QMessageBox::about (this, VERSION,
                      "rmsMonitor - CHARTS RMS monitor."
                      "\n\nAuthor : Jan C. Depner (jan.depner@navy.mil)");
}


void
rmsMonitor::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



//  Get the users defaults.

NV_BOOL 
rmsMonitor::envin ()
{
  NV_FLOAT64 saved_version = 1.00;


  // Set Defaults so the if keys don't exist the parms are defined

  slotRestoreDefaults ();
  force_redraw = NVFalse;


  QSettings settings (tr ("navo.navy.mil"), tr ("rmsMonitor"));
  settings.beginGroup (tr ("rmsMonitor"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return (NVFalse);


  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      for (NV_INT32 j = 0 ; j < 2 ; j++)
        {
          QString str;
          str.sprintf (tr ("stoplight [%d][%d] value").toAscii (), i, j);
          stop_light[i][j] = settings.value (str, stop_light[i][j]).toDouble ();
        }
    }


  NV_INT32 red = settings.value (tr ("background color/red"), bg_color.red ()).toInt ();
  NV_INT32 green = settings.value (tr ("background color/green"), bg_color.green ()).toInt ();
  NV_INT32 blue = settings.value (tr ("background color/blue"), bg_color.blue ()).toInt ();
  NV_INT32 alpha = settings.value (tr ("background color/alpha"), bg_color.alpha ()).toInt ();
  bg_color.setRgb (red, green, blue, alpha);

  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      QString str;
      str.sprintf (tr ("stoplight [%d] color/red").toAscii (), i);
      red = settings.value (str, stop_color[i].red ()).toInt ();
      str.sprintf (tr ("stoplight [%d] color/green").toAscii (), i);
      green = settings.value (str, stop_color[i].green ()).toInt ();
      str.sprintf (tr ("stoplight [%d] color/blue").toAscii (), i);
      blue = settings.value (str, stop_color[i].blue ()).toInt ();
      str.sprintf (tr ("stoplight [%d] color/alpha").toAscii (), i);
      alpha = settings.value (str, stop_color[i].alpha ()).toInt ();
      stop_color[i].setRgb (red, green, blue, alpha);
    }

  this->restoreState (settings.value (tr ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  this->restoreGeometry (settings.value (tr ("main window geometry")).toByteArray ());

  settings.endGroup ();

  return (NVTrue);
}




//  Save the users defaults.

void
rmsMonitor::envout ()
{
  QSettings settings (tr ("navo.navy.mil"), tr ("rmsMonitor"));
  settings.beginGroup (tr ("rmsMonitor"));


  settings.setValue (tr ("settings version"), settings_version);

  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      for (NV_INT32 j = 0 ; j < 2 ; j++)
        {
          QString str;
          str.sprintf (tr ("stoplight [%d][%d] value").toAscii (), i, j);
          settings.setValue (str, stop_light[i][j]);
        }
    }


  settings.setValue (tr ("background color/red"), bg_color.red ());
  settings.setValue (tr ("background color/green"), bg_color.green ());
  settings.setValue (tr ("background color/blue"), bg_color.blue ());
  settings.setValue (tr ("background color/alpha"), bg_color.alpha ());


  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      QString str;
      str.sprintf (tr ("stoplight [%d] color/red").toAscii (), i);
      settings.setValue (str, stop_color[i].red ());
      str.sprintf (tr ("stoplight [%d] color/green").toAscii (), i);
      settings.setValue (str, stop_color[i].green ());
      str.sprintf (tr ("stoplight [%d] color/blue").toAscii (), i);
      settings.setValue (str, stop_color[i].blue ());
      str.sprintf (tr ("stoplight [%d] color/alpha").toAscii (), i);
      settings.setValue (str, stop_color[i].alpha ());
    }


  settings.setValue (tr ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.setValue (tr ("main window geometry"), this->saveGeometry ());

  settings.endGroup ();
}
