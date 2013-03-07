#include "waveformMonitor.hpp"
#include "waveformMonitorHelp.hpp"


NV_FLOAT64 settings_version = 2.01;


waveformMonitor::waveformMonitor (NV_INT32 *argc, NV_CHAR **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;


  filError = NULL;
  lock_track = NVFalse;


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/waveform_monitor.xpm"));


  kill_switch = ANCILLARY_FORCE_EXIT;

  NV_INT32 option_index = 0;
  while (NVTrue) 
    {
      static struct option long_options[] = {{"actionkey00", required_argument, 0, 0},
                                             {"actionkey01", required_argument, 0, 0},
                                             {"actionkey02", required_argument, 0, 0},
                                             {"actionkey03", required_argument, 0, 0},
                                             {"shared_memory_key", required_argument, 0, 0},
                                             {"kill_switch", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "o", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          //  The parent ID argument

          switch (option_index)
            {
            case 4:
              sscanf (optarg, "%d", &key);
              break;

            case 5:
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


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  QToolBar *tools = addToolBar (tr ("Tools"));
  tools->setObjectName (tr ("waveformMonitor main toolbar"));


  envin ();


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


  //  Set the window size and location from the defaults

  this->resize (width, height);
  this->move (window_x, window_y);


  //  Set all of the default values.

  wave_read = 0;


  //  Set the map values from the defaults

  mapdef.projection = NO_PROJECTION;
  mapdef.draw_width = width;
  mapdef.draw_height = height;
  mapdef.overlap_percent = 5;
  mapdef.grid_inc_x = 0.0;
  mapdef.grid_inc_y = 50.0;

  mapdef.coasts = NVFalse;
  mapdef.landmask = NVFalse;

  mapdef.border = 0;
  mapdef.coast_color = Qt::white;
  mapdef.grid_color = QColor (160, 160, 160, 127);
  mapdef.background_color = backgroundColor;


  mapdef.initial_bounds.min_x = 0;
  mapdef.initial_bounds.min_y = 0;
  mapdef.initial_bounds.max_x = 300;
  mapdef.initial_bounds.max_y = 500;


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Make the map.

  map = new nvMap (this, &mapdef);
  map->setWhatsThis (mapText);


  //  Connect to the signals from the map class.

  connect (map, SIGNAL (mousePressSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotMousePress (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (map, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotPlotWaves (NVMAP_DEF)));


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  vBox->addWidget (map);


  for (NV_INT32 i = 0 ; i < 5 ; i++)
    {
      statusBar[i] = new QStatusBar (frame);
      statusBar[i]->setSizeGripEnabled (FALSE);
      statusBar[i]->show ();
      vBox->addWidget (statusBar[i]);
    }


  nameLabel = new QLabel ("                                        ", this);
  nameLabel->setAlignment (Qt::AlignLeft);
  nameLabel->setMinimumSize (nameLabel->sizeHint ());
  nameLabel->setWhatsThis (nameLabelText);
  nameLabel->setToolTip (tr ("File name"));

  dateLabel = new QLabel ("0000-00-00 (000) 00:00:00.00", this);
  dateLabel->setAlignment (Qt::AlignCenter);
  dateLabel->setMinimumSize (dateLabel->sizeHint ());
  dateLabel->setWhatsThis (dateLabelText);
  dateLabel->setToolTip (tr ("Date and time"));

  recordLabel = new QLabel ("00000000", this);
  recordLabel->setAlignment (Qt::AlignCenter);
  recordLabel->setMinimumSize (recordLabel->sizeHint ());
  recordLabel->setWhatsThis (recordLabelText);
  recordLabel->setToolTip (tr ("Record number"));

  dataType = new QLabel ("DGPS", this);
  dataType->setAlignment (Qt::AlignCenter);
  dataType->setMinimumSize (dataType->sizeHint ());
  dataType->setWhatsThis (dataTypeText);
  dataType->setToolTip (tr ("Data type"));

  correctDepth = new QLabel ("0000.00", this);
  correctDepth->setAlignment (Qt::AlignCenter);
  correctDepth->setMinimumSize (correctDepth->sizeHint ());
  correctDepth->setWhatsThis (correctDepthText);
  correctDepth->setToolTip (tr ("Tide/datum corrected depth"));

  secDepth = new QLabel ("0000.00", this);
  secDepth->setAlignment (Qt::AlignCenter);
  secDepth->setMinimumSize (secDepth->sizeHint ());
  secDepth->setWhatsThis (secDepthText);
  secDepth->setToolTip (tr ("Tide/datum corrected second depth"));

  reportedDepth = new QLabel ("0000.00", this);
  reportedDepth->setAlignment (Qt::AlignCenter);
  reportedDepth->setMinimumSize (reportedDepth->sizeHint ());
  reportedDepth->setWhatsThis (reportedDepthText);
  reportedDepth->setToolTip (tr ("Uncorrected depth"));

  tideLabel = new QLabel ("0000.00", this);
  tideLabel->setAlignment (Qt::AlignCenter);
  tideLabel->setMinimumSize (tideLabel->sizeHint ());
  tideLabel->setWhatsThis (tideLabelText);
  tideLabel->setToolTip (tr ("Tide value"));

  waveHeight = new QLabel ("000.00", this);
  waveHeight->setAlignment (Qt::AlignCenter);
  waveHeight->setMinimumSize (waveHeight->sizeHint ());
  waveHeight->setWhatsThis (waveHeightText);
  waveHeight->setToolTip (tr ("Wave height"));

  abdcLabel = new QLabel ("00", this);
  abdcLabel->setAlignment (Qt::AlignCenter);
  abdcLabel->setMinimumSize (abdcLabel->sizeHint ());
  abdcLabel->setWhatsThis (abdcLabelText);
  abdcLabel->setToolTip (tr ("Abbreviated depth confidence"));

  sabdcLabel = new QLabel ("N/A", this);
  sabdcLabel->setAlignment (Qt::AlignCenter);
  sabdcLabel->setMinimumSize (sabdcLabel->sizeHint ());
  sabdcLabel->setWhatsThis (sabdcLabelText);
  sabdcLabel->setToolTip (tr ("Second abbreviated depth confidence"));

  botBinLabel = new QLabel ("APD 000", this);
  botBinLabel->setAlignment (Qt::AlignCenter);
  botBinLabel->setMinimumSize (botBinLabel->sizeHint ());
  botBinLabel->setWhatsThis (botBinLabelText);
  botBinLabel->setToolTip (tr ("Bottom channel and bin used"));

  secBotBinLabel = new QLabel ("PMT 000", this);
  secBotBinLabel->setAlignment (Qt::AlignCenter);
  secBotBinLabel->setMinimumSize (secBotBinLabel->sizeHint ());
  secBotBinLabel->setWhatsThis (secBotBinLabelText);
  secBotBinLabel->setToolTip (tr ("Second bottom channel and bin used"));

  sfcBinLabel = new QLabel ("RAMAN 000", this);
  sfcBinLabel->setAlignment (Qt::AlignCenter);
  sfcBinLabel->setMinimumSize (sfcBinLabel->sizeHint ());
  sfcBinLabel->setWhatsThis (sfcBinLabelText);
  sfcBinLabel->setToolTip (tr ("Surface channel and bin used"));

  fullConf = new QLabel ("000000", this);
  fullConf->setAlignment (Qt::AlignCenter);
  fullConf->setMinimumSize (fullConf->sizeHint ());
  fullConf->setWhatsThis (fullConfText);
  fullConf->setToolTip (tr ("Full confidence value"));

  secFullConf = new QLabel ("000000", this);
  secFullConf->setAlignment (Qt::AlignCenter);
  secFullConf->setMinimumSize (secFullConf->sizeHint ());
  secFullConf->setWhatsThis (secFullConfText);
  secFullConf->setToolTip (tr ("Second full confidence value"));

  bfomThresh = new QLabel ("00", this);
  bfomThresh->setAlignment (Qt::AlignCenter);
  bfomThresh->setMinimumSize (bfomThresh->sizeHint ());
  bfomThresh->setWhatsThis (bfomThreshText);
  bfomThresh->setToolTip (tr ("Bottom figure of merit threshold * 10"));

  secBfomThresh = new QLabel ("00", this);
  secBfomThresh->setAlignment (Qt::AlignCenter);
  secBfomThresh->setMinimumSize (secBfomThresh->sizeHint ());
  secBfomThresh->setWhatsThis (secBfomThreshText);
  secBfomThresh->setToolTip (tr ("Second bottom figure of merit threshold * 10"));

  sigStrength = new QLabel ("0000.0", this);
  sigStrength->setAlignment (Qt::AlignCenter);
  sigStrength->setMinimumSize (sigStrength->sizeHint ());
  sigStrength->setWhatsThis (sigStrengthText);
  sigStrength->setToolTip (tr ("Shallow bottom signal strength"));

  secSigStrength = new QLabel ("0000.0", this);
  secSigStrength->setAlignment (Qt::AlignCenter);
  secSigStrength->setMinimumSize (secSigStrength->sizeHint ());
  secSigStrength->setWhatsThis (secSigStrengthText);
  secSigStrength->setToolTip (tr ("Deep bottom signal strength"));


  statusBar[0]->addWidget (nameLabel, 1);
  statusBar[1]->addWidget (dateLabel);
  statusBar[1]->addWidget (dataType);
  statusBar[1]->addWidget (recordLabel);
  statusBar[2]->addWidget (correctDepth);
  statusBar[2]->addWidget (secDepth);
  statusBar[2]->addWidget (reportedDepth);
  statusBar[2]->addWidget (tideLabel);
  statusBar[2]->addWidget (waveHeight);
  statusBar[3]->addWidget (abdcLabel);
  statusBar[3]->addWidget (sabdcLabel);
  statusBar[3]->addWidget (botBinLabel);
  statusBar[3]->addWidget (secBotBinLabel);
  statusBar[3]->addWidget (sfcBinLabel);
  statusBar[4]->addWidget (fullConf);
  statusBar[4]->addWidget (secFullConf);
  statusBar[4]->addWidget (bfomThresh);
  statusBar[4]->addWidget (secBfomThresh);
  statusBar[4]->addWidget (sigStrength);
  statusBar[4]->addWidget (secSigStrength);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.xpm"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


  bMode = new QToolButton (this);
  if (wave_line_mode)
    {
      bMode->setIcon (QIcon (":/icons/mode_line.xpm"));
    }
  else
    {
      bMode->setIcon (QIcon (":/icons/mode_dot.xpm"));
    }
  bMode->setToolTip (tr ("Wave drawing mode toggle"));
  bMode->setWhatsThis (modeText);
  bMode->setCheckable (TRUE);
  bMode->setChecked (wave_line_mode);
  connect (bMode, SIGNAL (toggled (bool)), this, SLOT (slotMode (bool)));
  tools->addWidget (bMode);


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.xpm"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  tools->addWidget (bPrefs);


  tools->addSeparator ();
  tools->addSeparator ();


  QAction *bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  tools->addAction (bHelp);



  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), this, SLOT (slotQuit ()));


  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (fileQuitAction);


  //  Setup the help menu.

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about waveformMonitor"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgements = new QAction (tr ("A&cknowledgements"), this);
  acknowledgements->setShortcut (tr ("Ctrl+c"));
  acknowledgements->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgements, SIGNAL (triggered ()), this, SLOT (slotAcknowledgements ()));

  QAction *aboutQtAct = new QAction (tr ("About&Qt"), this);
  aboutQtAct->setShortcut (tr ("Ctrl+Q"));
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("&Help"));
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgements);
  helpMenu->addAction (aboutQtAct);


  map->setCursor (Qt::ArrowCursor);

  map->enableSignals ();


  QTimer *track = new QTimer (this);
  connect (track, SIGNAL (timeout ()), this, SLOT (trackCursor ()));
  track->start (10);
}



waveformMonitor::~waveformMonitor ()
{
}



void 
waveformMonitor::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



void 
waveformMonitor::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  force_redraw = NVTrue;
}



void
waveformMonitor::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
waveformMonitor::leftMouse (NV_FLOAT64 x __attribute__ ((unused)), NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Placeholder
}



void 
waveformMonitor::midMouse (NV_FLOAT64 x __attribute__ ((unused)), NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Placeholder
}



void 
waveformMonitor::rightMouse (NV_FLOAT64 x __attribute__ ((unused)), NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Placeholder
}



//  Signal from the map class.

void 
waveformMonitor::slotMousePress (QMouseEvent * e, NV_FLOAT64 x, NV_FLOAT64 y)
{
  if (e->button () == Qt::LeftButton) leftMouse (x, y);
  if (e->button () == Qt::MidButton) midMouse (x, y);
  if (e->button () == Qt::RightButton) rightMouse (x, y);
}



//  Signal from the map class.

void
waveformMonitor::slotMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 x __attribute__ ((unused)),
                            NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Placeholder
}



//  Timer - timeout signal.  Very much like an X workproc.

void
waveformMonitor::trackCursor ()
{
  NV_INT32                year, day, mday, month, hour, minute;
  NV_INT32                wlf_handle, **wlf_waveforms, return_type = 0;
  NV_FLOAT32              second;
  static NV_U_INT32       prev_rec = -1;
  static ABE_SHARE        l_share;
  static NV_BOOL          first = NVTrue;
  FILE                    *hfp = NULL, *wfp = NULL;


  //  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.
  //  We also want to exit on the ANCILLARY_FORCE_EXIT key (from pfmEdit) or if our own personal kill signal
  //  has been placed in abe_share->key.

  if (abe_share->key == CHILD_PROCESS_FORCE_EXIT || abe_share->key == ANCILLARY_FORCE_EXIT ||
      abe_share->key == kill_switch) slotQuit ();


  //  We want to exit if we have locked the tracker to update our saved waveforms (in slotPlotWaves).

  if (lock_track) return;


  //  Locking makes sure another process does not have memory locked.  It will block until it can lock it.
  //  At that point we copy the contents and then unlock it so other processes can continue.

  abeShare->lock ();


  //  Check for change of record and correct record type.

  NV_BOOL hit = NVFalse;
  if (prev_rec != abe_share->mwShare.multiRecord[0] && (abe_share->mwShare.multiType[0] == PFM_SHOALS_1K_DATA ||
                                                        abe_share->mwShare.multiType[0] == PFM_CHARTS_HOF_DATA ||
                                                        abe_share->mwShare.multiType[0] == PFM_WLF_DATA))
    {
      l_share = *abe_share;
      prev_rec = l_share.mwShare.multiRecord[0];
      hit = NVTrue;
      return_type = abe_share->mwShare.multiType[0];
    }


  //  Check for action keys.

  if (abe_share->key == ac[0])
    {
      abe_share->key = 0;
      abe_share->modcode = NO_ACTION_REQUIRED;

      if (display[APD])
        {
          display[APD] = NVFalse;
        }
      else
        {
          display[APD] = NVTrue;
        }

      force_redraw = NVTrue;
    }


  if (abe_share->key == ac[1])
    {
      abe_share->key = 0;
      abe_share->modcode = NO_ACTION_REQUIRED;

      if (display[PMT])
        {
          display[PMT] = NVFalse;
        }
      else
        {
          display[PMT] = NVTrue;
        }

      force_redraw = NVTrue;
    }


  if (abe_share->key == ac[2])
    {
      abe_share->key = 0;
      abe_share->modcode = NO_ACTION_REQUIRED;

      if (display[IR])
        {
          display[IR] = NVFalse;
        }
      else
        {
          display[IR] = NVTrue;
        }

      force_redraw = NVTrue;
    }


  if (abe_share->key == ac[3])
    {
      abe_share->key = 0;
      abe_share->modcode = NO_ACTION_REQUIRED;

      if (display[RAMAN])
        {
          display[RAMAN] = NVFalse;
        }
      else
        {
          display[RAMAN] = NVTrue;
        }

      force_redraw = NVTrue;
    }


  if (abe_share->modcode == WAVEMONITOR_FORCE_REDRAW) force_redraw = NVTrue;


  abeShare->unlock ();


  //  Hit or force redraw from above.

  if (hit || force_redraw)
    {
      force_redraw = NVFalse;

 
      //  Save for slotPlotWaves.

      recnum = l_share.mwShare.multiRecord[0];
      strcpy (filename, l_share.nearest_filename);


      //  Open the HOF or WLF file and read the data.

      NV_CHAR string[512];

      if (l_share.mwShare.multiType[0] == PFM_SHOALS_1K_DATA || l_share.mwShare.multiType[0] == PFM_CHARTS_HOF_DATA)
        {
          strcpy (string, l_share.nearest_filename);
          db_name.sprintf ("%s", pfm_basename (l_share.nearest_filename));


          strcpy (string, l_share.nearest_filename);
          sprintf (&string[strlen (string) - 4], ".inh");
          strcpy (wave_path, string);


          hfp = open_hof_file (l_share.nearest_filename);

          if (hfp == NULL)
            {
              if (filError) filError->close ();
              filError = new QMessageBox (QMessageBox::Warning, tr ("Open HOF file"), tr ("Error opening ") + 
                                          QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                                          QString (strerror (errno)), QMessageBox::NoButton, this, 
                                          (Qt::WindowFlags) Qt::WA_DeleteOnClose);
              filError->show ();
              return;
            }


          wfp = open_wave_file (wave_path);

          if (wfp == NULL)
            {
              if (filError) filError->close ();
              filError = new QMessageBox (QMessageBox::Warning, tr ("Open INH file"), tr ("Error opening ") + 
                                          QDir::toNativeSeparators (QString (wave_path)) + " : " + 
                                          QString (strerror (errno)), QMessageBox::NoButton, this, 
                                          (Qt::WindowFlags) Qt::WA_DeleteOnClose);
              filError->show ();

              fclose (hfp);
              return;
            }


          wave_read_header (wfp, &wave_header);


          hof_read_record (hfp, l_share.mwShare.multiRecord[0], &hof_record);
          wave_read = wave_read_record (wfp, l_share.mwShare.multiRecord[0], &wave_data);


          fclose (hfp);
          fclose (wfp);


          bounds[PMT].ac_zero_offset = wave_header.ac_zero_offset[PMT];
          bounds[APD].ac_zero_offset = wave_header.ac_zero_offset[APD];
          bounds[IR].ac_zero_offset = wave_header.ac_zero_offset[IR];
          bounds[RAMAN].ac_zero_offset = wave_header.ac_zero_offset[RAMAN];

          bounds[PMT].length = wave_header.pmt_size - 1;
          bounds[APD].length = wave_header.apd_size - 1;
          bounds[IR].length = wave_header.ir_size - 1;
          bounds[RAMAN].length = wave_header.raman_size - 1;


          for (NV_INT32 j = 0 ; j < bounds[PMT].length ; j++) wave.data[PMT][j] = wave_data.pmt[j + 1];
          for (NV_INT32 j = 0 ; j < bounds[APD].length ; j++) wave.data[APD][j] = wave_data.apd[j + 1];
          for (NV_INT32 j = 0 ; j < bounds[IR].length ; j++) wave.data[IR][j] = wave_data.ir[j + 1];
          for (NV_INT32 j = 0 ; j < bounds[RAMAN].length ; j++) wave.data[RAMAN][j] = wave_data.raman[j + 1];


          if (hof_record.sec_depth == -998.0) 
            {
              secondary = NVFalse;
            }
          else
            {
              secondary = NVTrue;
            }


          tide = 0.0;
          if (!hof_record.data_type && hof_record.correct_depth != -998.0)
            tide = -hof_record.correct_depth - hof_record.reported_depth;


          charts_cvtime (hof_record.timestamp, &year, &day, &hour, &minute, &second);
          charts_jday2mday (year, day, &month, &mday);
          month++;


          date_time.sprintf ("%d-%02d-%02d (%03d) %02d:%02d:%05.2f", year + 1900, month, mday, day, hour, minute, second);


          //  Set the min and max values.

          bounds[PMT].min_y = 0;
          bounds[PMT].max_y = 256;
          bounds[PMT].min_x = 0;
          bounds[PMT].max_x = bounds[PMT].length;

          bounds[APD].min_y = 0;
          bounds[APD].max_y = 256;

          bounds[IR].min_y = 0;
          bounds[IR].max_y = 256;

          bounds[RAMAN].min_y = 0;
          bounds[RAMAN].max_y = 256;


          //  Add 5% to the X axis.

          bounds[PMT].range_x = bounds[PMT].max_x - bounds[PMT].min_x;
          bounds[PMT].min_x = bounds[PMT].min_x - NINT (((NV_FLOAT32) bounds[PMT].range_x * 0.05 + 1));
          bounds[PMT].max_x = bounds[PMT].max_x + NINT (((NV_FLOAT32) bounds[PMT].range_x * 0.05 + 1));
          bounds[PMT].range_x = bounds[PMT].max_x - bounds[PMT].min_x;
          bounds[PMT].range_y = bounds[PMT].max_y - bounds[PMT].min_y;

          bounds[APD].range_y = bounds[APD].max_y - bounds[APD].min_y;

          bounds[IR].range_y = bounds[IR].max_y - bounds[IR].min_y;

          bounds[RAMAN].range_y = bounds[RAMAN].max_y - bounds[RAMAN].min_y;

          wave.wlf_record = NVFalse;
        }
      else if (l_share.mwShare.multiType[0] == PFM_WLF_DATA)
        {
          strcpy (string, l_share.nearest_filename);

          if ((wlf_handle = wlf_open_file (l_share.nearest_filename, &wlf_header, WLF_READONLY)) < 0)
            {
              if (filError) filError->close ();
              filError = new QMessageBox (QMessageBox::Warning, tr ("waveformMonitor"), QString (wlf_strerror ()),
                                          QMessageBox::NoButton, this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
              filError->show ();
              return;
            }


          //  If we have no waveforms (like for a topo file), quietly leave.

          if (!wlf_header.number_of_waveforms)
            {
              wlf_close_file (wlf_handle);
              return;
            }


          if (wlf_read_record (wlf_handle, l_share.mwShare.multiRecord[0], &wlf_record, NVTrue, &wlf_waveforms) < 0)
            {
              if (filError) filError->close ();
              filError = new QMessageBox (QMessageBox::Warning, tr ("waveformMonitor"), QString (wlf_strerror ()),
                                          QMessageBox::NoButton, this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
              filError->show ();

              wlf_close_file (wlf_handle);
              return;
            }
          wave_read = 1;

          bounds[PMT].length = wlf_header.waveform_count[PMT];
          bounds[APD].length = wlf_header.waveform_count[APD];
          bounds[IR].length = wlf_header.waveform_count[IR];
          bounds[RAMAN].length = wlf_header.waveform_count[RAMAN];


          for (NV_INT32 j = 0 ; j < bounds[PMT].length ; j++) wave.data[PMT][j] = wlf_waveforms[PMT][j];
          for (NV_INT32 j = 0 ; j < bounds[APD].length ; j++) wave.data[APD][j] = wlf_waveforms[APD][j];
          for (NV_INT32 j = 0 ; j < bounds[IR].length ; j++) wave.data[IR][j] = wlf_waveforms[IR][j];
          for (NV_INT32 j = 0 ; j < bounds[RAMAN].length ; j++) wave.data[RAMAN][j] = wlf_waveforms[RAMAN][j];


          wlf_close_file (wlf_handle);


          secondary = NVFalse;


          tide = wlf_record.z_offset;


          wlf_cvtime (wlf_record.tv_sec, wlf_record.tv_nsec, &year, &day, &hour, &minute, &second);
          wlf_jday2mday (year, day, &month, &mday);
          month++;


          date_time.sprintf ("%d-%02d-%02d (%03d) %02d:%02d:%05.2f", year + 1900, month, mday, day, hour, 
                                minute, second);


          //  Set the min and max values.

          bounds[PMT].min_y = 0;
          bounds[PMT].max_y = wlf_header.max_waveform[PMT];
          bounds[PMT].min_x = 0;
          bounds[PMT].max_x = bounds[PMT].length;

          bounds[APD].min_y = 0;
          bounds[APD].max_y = wlf_header.max_waveform[PMT];

          bounds[IR].min_y = 0;
          bounds[IR].max_y = wlf_header.max_waveform[PMT];

          bounds[RAMAN].min_y = 0;
          bounds[RAMAN].max_y = wlf_header.max_waveform[PMT];


          //  Add 5% to the X axis.

          bounds[PMT].range_x = bounds[PMT].max_x - bounds[PMT].min_x;
          bounds[PMT].min_x = bounds[PMT].min_x - NINT (((NV_FLOAT32) bounds[PMT].range_x * 0.05 + 1));
          bounds[PMT].max_x = bounds[PMT].max_x + NINT (((NV_FLOAT32) bounds[PMT].range_x * 0.05 + 1));
          bounds[PMT].range_x = bounds[PMT].max_x - bounds[PMT].min_x;
          bounds[PMT].range_y = bounds[PMT].max_y - bounds[PMT].min_y;

          bounds[APD].range_y = bounds[APD].max_y - bounds[APD].min_y;

          bounds[IR].range_y = bounds[IR].max_y - bounds[IR].min_y;

          bounds[RAMAN].range_y = bounds[RAMAN].max_y - bounds[RAMAN].min_y;


          wave.wlf_record = NVTrue;
        }
      else
        {
          return;
        }


      l_share.key = 0;
      abe_share->key = 0;
      abe_share->modcode = return_type;


      map->redrawMapArea (NVTrue);
    }


  //  Display the startup info message the first time through.

  if (first)
    {
      QString startupMessageText = 
        tr ("The following action keys are available in ") + parentName + tr (":\n\n") +
        QString (ac[0]) + tr (" = Toggle display of the APD waveform\n") + 
        QString (ac[1]) + tr (" = Toggle display of the PMT waveform\n") + 
        QString (ac[2]) + tr (" = Toggle display of the IR waveform\n") + 
        QString (ac[3]) + tr (" = Toggle display of the Raman waveform\n\n") + 
        tr ("You can change these key values in the ") + parentName + "\n" +
        tr ("Preferences->Ancillary Programs window\n\n\n") + 
        tr ("You can turn off this startup message in the\n") + 
        tr ("waveformMonitor Preferences window.");

      if (startup_message) QMessageBox::information (this, tr ("waveformMonitor Startup Message"), startupMessageText);

      first = NVFalse;
    }
}



//  Signal from the map class.

void 
waveformMonitor::slotKeyPress (QKeyEvent *e)
{
  NV_CHAR key[20];
  strcpy (key, e->text ().toAscii ());

  if (key[0] == (NV_CHAR) ac[0])
    {
      if (display[APD])
        {
          display[APD] = NVFalse;
        }
      else
        {
          display[APD] = NVTrue;
        }

      force_redraw = NVTrue;
    }


  if (key[0] == (NV_CHAR) ac[1])
    {
      if (display[PMT])
        {
          display[PMT] = NVFalse;
        }
      else
        {
          display[PMT] = NVTrue;
        }

      force_redraw = NVTrue;
    }


  if (key[0] == (NV_CHAR) ac[2])
    {
      if (display[IR])
        {
          display[IR] = NVFalse;
        }
      else
        {
          display[IR] = NVTrue;
        }

      force_redraw = NVTrue;
    }


  if (key[0] == (NV_CHAR) ac[3])
    {
      if (display[RAMAN])
        {
          display[RAMAN] = NVFalse;
        }
      else
        {
          display[RAMAN] = NVTrue;
        }

      force_redraw = NVTrue;
    }
}



//  A bunch of slots.

void 
waveformMonitor::slotQuit ()
{
  //  Let the parent program know that we have died from something other than the kill switch from the parent.

  if (abe_share->key != kill_switch) abe_share->killed = kill_switch;


  envout ();


  //  Let go of the shared memory.

  abeShare->detach ();


  exit (0);
}



void 
waveformMonitor::slotMode (bool on)
{
  wave_line_mode = on;

  if (on)
    {
      bMode->setIcon (QIcon (":/icons/mode_line.xpm"));
    }
  else
    {
      bMode->setIcon (QIcon (":/icons/mode_dot.xpm"));
    }

  force_redraw = NVTrue;
}



void 
waveformMonitor::setFields ()
{
  if (pos_format == "hdms") bGrp->button (0)->setChecked (TRUE);
  if (pos_format == "hdm") bGrp->button (1)->setChecked (TRUE);
  if (pos_format == "hd") bGrp->button (2)->setChecked (TRUE);
  if (pos_format == "dms") bGrp->button (3)->setChecked (TRUE);
  if (pos_format == "dm") bGrp->button (4)->setChecked (TRUE);
  if (pos_format == "d") bGrp->button (5)->setChecked (TRUE);


  if (startup_message)
    {
      sMessage->setCheckState (Qt::Checked);
    }
  else
    {
      sMessage->setCheckState (Qt::Unchecked);
    }
      

  NV_INT32 hue, sat, val;

  for (NV_INT32 i = 0 ; i < 4 ; i++)
    {
      waveColor[i].getHsv (&hue, &sat, &val);
      if (val < 128)
	{
	  bWavePalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
	  bWavePalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
	}
      else
	{
	  bWavePalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
	  bWavePalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
	}
      bWavePalette[i].setColor (QPalette::Normal, QPalette::Button, waveColor[i]);
      bWavePalette[i].setColor (QPalette::Inactive, QPalette::Button, waveColor[i]);
      bWaveColor[i]->setPalette (bWavePalette[i]);
      acZeroColor[i] = waveColor[i];
      acZeroColor[i].setAlpha (128);
    }

  surfaceColor.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bSurfacePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bSurfacePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bSurfacePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bSurfacePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bSurfacePalette.setColor (QPalette::Normal, QPalette::Button, surfaceColor);
  bSurfacePalette.setColor (QPalette::Inactive, QPalette::Button, surfaceColor);
  bSurfaceColor->setPalette (bSurfacePalette);


  primaryColor.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bPrimaryPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bPrimaryPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bPrimaryPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bPrimaryPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bPrimaryPalette.setColor (QPalette::Normal, QPalette::Button, primaryColor);
  bPrimaryPalette.setColor (QPalette::Inactive, QPalette::Button, primaryColor);
  bPrimaryColor->setPalette (bPrimaryPalette);


  secondaryColor.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bSecondaryPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bSecondaryPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bSecondaryPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bSecondaryPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bSecondaryPalette.setColor (QPalette::Normal, QPalette::Button, secondaryColor);
  bSecondaryPalette.setColor (QPalette::Inactive, QPalette::Button, secondaryColor);
  bSecondaryColor->setPalette (bSecondaryPalette);


  backgroundColor.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bBackgroundPalette.setColor (QPalette::Normal, QPalette::Button, backgroundColor);
  bBackgroundPalette.setColor (QPalette::Inactive, QPalette::Button, backgroundColor);
  bBackgroundColor->setPalette (bBackgroundPalette);
}



void
waveformMonitor::slotPrefs ()
{
  prefsD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  prefsD->setWindowTitle (tr ("waveformMonitor Preferences"));
  prefsD->setModal (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (prefsD);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QGroupBox *fbox = new QGroupBox (tr ("Position Format"), prefsD);
  fbox->setWhatsThis (bGrpText);

  QRadioButton *hdms = new QRadioButton (tr ("Hemisphere Degrees Minutes Seconds.decimal"));
  QRadioButton *hdm_ = new QRadioButton (tr ("Hemisphere Degrees Minutes.decimal"));
  QRadioButton *hd__ = new QRadioButton (tr ("Hemisphere Degrees.decimal"));
  QRadioButton *sdms = new QRadioButton (tr ("+/-Degrees Minutes Seconds.decimal"));
  QRadioButton *sdm_ = new QRadioButton (tr ("+/-Degrees Minutes.decimal"));
  QRadioButton *sd__ = new QRadioButton (tr ("+/-Degrees.decimal"));

  bGrp = new QButtonGroup (prefsD);
  bGrp->setExclusive (TRUE);
  connect (bGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotPosClicked (int)));

  bGrp->addButton (hdms, 0);
  bGrp->addButton (hdm_, 1);
  bGrp->addButton (hd__, 2);
  bGrp->addButton (sdms, 3);
  bGrp->addButton (sdm_, 4);
  bGrp->addButton (sd__, 5);

  QHBoxLayout *fboxSplit = new QHBoxLayout;
  QVBoxLayout *fboxLeft = new QVBoxLayout;
  QVBoxLayout *fboxRight = new QVBoxLayout;
  fboxSplit->addLayout (fboxLeft);
  fboxSplit->addLayout (fboxRight);
  fboxLeft->addWidget (hdms);
  fboxLeft->addWidget (hdm_);
  fboxLeft->addWidget (hd__);
  fboxRight->addWidget (sdms);
  fboxRight->addWidget (sdm_);
  fboxRight->addWidget (sd__);
  fbox->setLayout (fboxSplit);

  vbox->addWidget (fbox, 1);

  if (pos_format == "hdms") bGrp->button (0)->setChecked (TRUE);
  if (pos_format == "hdm") bGrp->button (1)->setChecked (TRUE);
  if (pos_format == "hd") bGrp->button (2)->setChecked (TRUE);
  if (pos_format == "dms") bGrp->button (3)->setChecked (TRUE);
  if (pos_format == "dm") bGrp->button (4)->setChecked (TRUE);
  if (pos_format == "d") bGrp->button (5)->setChecked (TRUE);


  QGroupBox *cbox = new QGroupBox (tr ("Colors"), this);
  QVBoxLayout *cboxLayout = new QVBoxLayout;
  cbox->setLayout (cboxLayout);
  QHBoxLayout *cboxTopLayout = new QHBoxLayout;
  QHBoxLayout *cboxBottomLayout = new QHBoxLayout;
  cboxLayout->addLayout (cboxTopLayout);
  cboxLayout->addLayout (cboxBottomLayout);


  QButtonGroup *bGrp = new QButtonGroup (this);
  connect (bGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotWaveColor (int)));


  bWaveColor[PMT] = new QPushButton (tr ("PMT"), this);
  bWaveColor[PMT]->setToolTip (tr ("Change PMT color"));
  bWaveColor[PMT]->setWhatsThis (bWaveColor[PMT]->toolTip ());
  bWavePalette[PMT] = bWaveColor[PMT]->palette ();
  cboxTopLayout->addWidget (bWaveColor[PMT]);


  bWaveColor[APD] = new QPushButton (tr ("APD"), this);
  bWaveColor[APD]->setToolTip (tr ("Change APD color"));
  bWaveColor[APD]->setWhatsThis (bWaveColor[APD]->toolTip ());
  bWavePalette[APD] = bWaveColor[APD]->palette ();
  cboxTopLayout->addWidget (bWaveColor[APD]);


  bWaveColor[IR] = new QPushButton (tr ("IR"), this);
  bWaveColor[IR]->setToolTip (tr ("Change IR color"));
  bWaveColor[IR]->setWhatsThis (bWaveColor[IR]->toolTip ());
  bWavePalette[IR] = bWaveColor[IR]->palette ();
  cboxTopLayout->addWidget (bWaveColor[IR]);


  bWaveColor[RAMAN] = new QPushButton (tr ("Raman"), this);
  bWaveColor[RAMAN]->setToolTip (tr ("Change Raman color"));
  bWaveColor[RAMAN]->setWhatsThis (bWaveColor[RAMAN]->toolTip ());
  bWavePalette[RAMAN] = bWaveColor[RAMAN]->palette ();
  cboxTopLayout->addWidget (bWaveColor[RAMAN]);


  bSurfaceColor = new QPushButton (tr ("Surface"), this);
  bSurfaceColor->setToolTip (tr ("Change surface return marker color"));
  bSurfaceColor->setWhatsThis (bSurfaceColor->toolTip ());
  bSurfacePalette = bSurfaceColor->palette ();
  connect (bSurfaceColor, SIGNAL (clicked ()), this, SLOT (slotSurfaceColor ()));
  cboxBottomLayout->addWidget (bSurfaceColor);


  bPrimaryColor = new QPushButton (tr ("Primary"), this);
  bPrimaryColor->setToolTip (tr ("Change primary return marker color"));
  bPrimaryColor->setWhatsThis (bPrimaryColor->toolTip ());
  bPrimaryPalette = bPrimaryColor->palette ();
  connect (bPrimaryColor, SIGNAL (clicked ()), this, SLOT (slotPrimaryColor ()));
  cboxBottomLayout->addWidget (bPrimaryColor);


  bSecondaryColor = new QPushButton (tr ("Secondary"), this);
  bSecondaryColor->setToolTip (tr ("Change secondary return marker color"));
  bSecondaryColor->setWhatsThis (bSecondaryColor->toolTip ());
  bSecondaryPalette = bSecondaryColor->palette ();
  connect (bSecondaryColor, SIGNAL (clicked ()), this, SLOT (slotSecondaryColor ()));
  cboxBottomLayout->addWidget (bSecondaryColor);


  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundColor->setToolTip (tr ("Change display background color"));
  bBackgroundColor->setWhatsThis (bBackgroundColor->toolTip ());
  bBackgroundPalette = bBackgroundColor->palette ();
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotBackgroundColor ()));
  cboxBottomLayout->addWidget (bBackgroundColor);


  vbox->addWidget (cbox, 1);


  QGroupBox *mBox = new QGroupBox (tr ("Display startup message"), this);
  QHBoxLayout *mBoxLayout = new QHBoxLayout;
  mBox->setLayout (mBoxLayout);
  sMessage = new QCheckBox (mBox);
  sMessage->setToolTip (tr ("Toggle display of startup message when program starts"));
  mBoxLayout->addWidget (sMessage);


  vbox->addWidget (mBox, 1);


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
waveformMonitor::slotPosClicked (int id)
{
  switch (id)
    {
    case 0:
    default:
      pos_format = "hdms";
      break;

    case 1:
      pos_format = "hdm";
      break;

    case 2:
      pos_format = "hd";
      break;

    case 3:
      pos_format = "dms";
      break;

    case 4:
      pos_format = "dm";
      break;

    case 5:
      pos_format = "d";
      break;
    }
}



void
waveformMonitor::slotClosePrefs ()
{
  if (sMessage->checkState () == Qt::Checked)
    {
      startup_message = NVTrue;
    }
  else
    {
      startup_message = NVFalse;
    }

  prefsD->close ();
}



void
waveformMonitor::slotWaveColor (int id)
{
  QString title;
  switch (id)
    {
    case 0:
      title = tr ("waveformMonitor PMT Color");
      break;

    case 1:
      title = tr ("waveformMonitor APD Color");
      break;

    case 2:
      title = tr ("waveformMonitor IR Color");
      break;

    case 3:
      title = tr ("waveformMonitor Raman Color");
      break;
    }

  QColor clr; 
  clr = QColorDialog::getColor (waveColor[id], this, title, QColorDialog::ShowAlphaChannel);
  if (clr.isValid ()) waveColor[id] = clr;

  acZeroColor[id] = waveColor[id];
  acZeroColor[id].setAlpha (128);

  setFields ();

  force_redraw = NVTrue;
}



void
waveformMonitor::slotSurfaceColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (surfaceColor, this, tr ("waveformMonitor Surface Marker Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) surfaceColor = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
waveformMonitor::slotPrimaryColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (primaryColor, this, tr ("waveformMonitor Primary Marker Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) primaryColor = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
waveformMonitor::slotSecondaryColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (secondaryColor, this, tr ("waveformMonitor Secondary Marker Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) secondaryColor = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
waveformMonitor::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (backgroundColor, this, tr ("waveformMonitor Background Color"));

  if (clr.isValid ()) backgroundColor = clr;

  setFields ();

  force_redraw = NVTrue;
}



void 
waveformMonitor::scaleWave (NV_INT32 x, NV_INT32 y, NV_INT32 *new_x, NV_INT32 *new_y, NV_INT32 type, NVMAP_DEF l_mapdef)
{
  *new_y = NINT (((NV_FLOAT32) (x - bounds[PMT].min_x) / (NV_FLOAT32) bounds[PMT].range_x) * (NV_FLOAT32) l_mapdef.draw_height);

  switch (type)
    {
    case RAMAN:
      *new_x = NINT (((NV_FLOAT32) (y - bounds[RAMAN].min_y) / (NV_FLOAT32) bounds[RAMAN].range_y) * (NV_FLOAT32) l_mapdef.draw_width);
      break;

    case IR:
      *new_x = NINT (((NV_FLOAT32) (y - bounds[IR].min_y) / (NV_FLOAT32) bounds[IR].range_y) * (NV_FLOAT32) l_mapdef.draw_width);
      break;

    case PMT:
      *new_x = NINT (((NV_FLOAT32) (y - bounds[PMT].min_y) / (NV_FLOAT32) bounds[PMT].range_y) * (NV_FLOAT32) l_mapdef.draw_width);
      break;

    case APD:
      *new_x = NINT (((NV_FLOAT32) (y - bounds[APD].min_y) / (NV_FLOAT32) bounds[APD].range_y) * (NV_FLOAT32) l_mapdef.draw_width);
      break;
    }
}



void 
waveformMonitor::slotPlotWaves (NVMAP_DEF l_mapdef)
{
  static NV_INT32         prev_type = -1, save_rec;
  static WAVE             save_wave;
  static HYDRO_OUTPUT_T   save_hof;
  static QString          save_name;
  static NV_BOOL          save_z_offset_present;
  static WLF_RECORD       save_wlf;
  NV_INT32                pix_x[2], pix_y[2];
  QString                 stat;


  if (!wave_read) return;


  //  Because the trackCursor function may be changing the data while we're still plotting it we save it
  //  to this static structure.  lock_track stops trackCursor from updating while we're trying to get an
  //  atomic snapshot of the data for the latest point.

  lock_track = NVTrue;
  save_wave = wave;
  save_rec = recnum;

  //  If we unset wlf_record, this is a HOF file.

  if (!save_wave.wlf_record)
    {
      save_hof = hof_record;
      save_name = db_name;
    }
  else
    {
      save_z_offset_present = wlf_header.opt.z_offset_present;
      save_wlf = wlf_record;
    }
  lock_track = NVFalse;


  //  Draw the waveforms.

  for (NV_INT32 j = 3 ; j >= 0 ; j--)
    {
      if (display[j]) 
	{
	  //  Mark the ac_zero offset for each waveform (only for HOF files).

	  if (!save_wave.wlf_record)
	    {
	      Qt::PenStyle pen = Qt::DashLine;

	      switch (j)
		{
		case PMT:
		  pen = Qt::DashLine;
		  break;

		case APD:
		  pen = Qt::DashDotLine;
		  break;

		case IR:
		  pen = Qt::DashDotDotLine;
		  break;

		case RAMAN:
		  pen = Qt::DotLine;
		  break;
		}

	      scaleWave (0, bounds[j].ac_zero_offset, &pix_x[0], &pix_y[0], j, l_mapdef);
	      scaleWave (bounds[j].length, bounds[j].ac_zero_offset, &pix_x[1], &pix_y[1], j, l_mapdef);
	      map->drawLine (pix_x[0], pix_y[0] - 15, pix_x[1], pix_y[1] + 15, acZeroColor[j], 2, NVFalse, pen);
	    }

	  scaleWave (1, save_wave.data[j][0], &pix_x[0], &pix_y[0], j, l_mapdef);

	  for (NV_INT32 i = 1 ; i < bounds[j].length ; i++)
	    {
	      scaleWave (i, save_wave.data[j][i], &pix_x[1], &pix_y[1], j, l_mapdef);

	      if (wave_line_mode)
		{
		  map->drawLine (pix_x[0], pix_y[0], pix_x[1], pix_y[1], waveColor[j], 2, NVFalse, Qt::SolidLine);
		}
	      else
		{
		  map->fillRectangle (pix_x[0], pix_y[0], SPOT_SIZE, SPOT_SIZE, waveColor[j], NVFalse);
		}
	      pix_x[0] = pix_x[1];
	      pix_y[0] = pix_y[1];
	    }
	}
    }


  record.sprintf ("%d", save_rec);


  nameLabel->setText (save_name);
  dateLabel->setText (date_time);
  recordLabel->setText (record);


  //  If we unset wlf_record, this is a HOF file.

  if (!save_wave.wlf_record)
    {
      if (save_hof.sfc_channel_used == IR)
        {
          if (display[IR]) 
            {
              scaleWave (save_hof.sfc_bin_ir, save_wave.data[IR][save_hof.sfc_bin_ir], &pix_x[0], &pix_y[0], IR, l_mapdef);
              drawX (pix_x[0], pix_y[0], 10, 2, surfaceColor);
            }
          sfc_bin.sprintf ("IR %d", save_hof.sfc_bin_ir);
        }

      if (save_hof.sfc_channel_used == RAMAN && display[RAMAN])
        {
          if (display[RAMAN]) 
            {
              scaleWave (save_hof.sfc_bin_ram, save_wave.data[RAMAN][save_hof.sfc_bin_ram], &pix_x[0], &pix_y[0], RAMAN,
                         l_mapdef);
              drawX (pix_x[0], pix_y[0], 10, 2, surfaceColor);
            }
          sfc_bin.sprintf ("RAMAN %d", save_hof.sfc_bin_ram);
        }


      if (secondary)
        {
          if (save_hof.sec_bot_chan == PMT)
            {
              if (display[PMT]) 
                {
                  scaleWave (save_hof.bot_bin_second, save_wave.data[PMT][save_hof.bot_bin_second], &pix_x[0], &pix_y[0], PMT,
                             l_mapdef);
                  drawX (pix_x[0], pix_y[0], 10, 2, secondaryColor);
                }
              sec_bot_bin.sprintf ("PMT %d", save_hof.bot_bin_second);
            }
          else
            {
              if (display[APD]) 
                {
                  scaleWave (save_hof.bot_bin_second, save_wave.data[APD][save_hof.bot_bin_second], &pix_x[0], &pix_y[0], APD, l_mapdef);
                  drawX (pix_x[0], pix_y[0], 10, 2, secondaryColor);
                }
              sec_bot_bin.sprintf ("APD %d", save_hof.bot_bin_second);
            }
        }
      else
        {
          sec_bot_bin = "N/A";
        }


      if (save_hof.ab_dep_conf == 72)
        {
          if (display[APD]) 
            {
              scaleWave (save_hof.sfc_bin_apd, save_wave.data[APD][save_hof.sfc_bin_apd], &pix_x[0], &pix_y[0], APD, l_mapdef);
              drawX (pix_x[0], pix_y[0], 10, 2, primaryColor);
            }
          bot_bin.sprintf ("SL %d", save_hof.sfc_bin_apd);
          sfc_bin.sprintf ("APD %d", save_hof.sfc_bin_apd);
        }
      else if (save_hof.ab_dep_conf == 74)
        {
          //  No marker for shallow water algorithm.

          bot_bin.sprintf ("SWA %d", save_hof.sfc_bin_apd);
        }
      else if (save_hof.bot_channel == PMT)
        {
          if (display[PMT]) 
            {
              scaleWave (save_hof.bot_bin_first, save_wave.data[PMT][save_hof.bot_bin_first], &pix_x[0], &pix_y[0], PMT, l_mapdef);
              drawX (pix_x[0], pix_y[0], 10, 2, primaryColor);
            }
          bot_bin.sprintf ("PMT %d", save_hof.bot_bin_first);
        }
      else
        {
          if (display[APD]) 
            {
              scaleWave (save_hof.bot_bin_first, save_wave.data[APD][save_hof.bot_bin_first], &pix_x[0], &pix_y[0], APD, l_mapdef);
              drawX (pix_x[0], pix_y[0], 10, 2, primaryColor);
            }
          bot_bin.sprintf ("APD %d", save_hof.bot_bin_first);
        }


      if (prev_type != save_hof.data_type)
        {
          tideLabel->setToolTip ("");
          correctDepth->setToolTip ("");
          secDepth->setToolTip ("");
        }

      if (save_hof.data_type)
        {
          mode = "KGPS";
          first.sprintf ("%.2f", -save_hof.correct_depth);
          rep_depth.sprintf ("%.2f", save_hof.kgps_res_elev);
          level.sprintf ("%.3f", save_hof.kgps_datum);
          if (prev_type != save_hof.data_type)
            {
              tideLabel->setToolTip (tr ("Datum value"));
              correctDepth->setToolTip (tr ("Datum corrected depth"));
              secDepth->setToolTip (tr ("Datum corrected second depth"));
            }
          abdc = save_hof.kgps_abd_conf;
        }
      else
        {
          mode = "DGPS";
          first.sprintf ("%.2f", -save_hof.correct_depth);
          rep_depth.sprintf ("%.2f", save_hof.reported_depth);
          level.sprintf ("%.3f", -tide);
          if (prev_type != save_hof.data_type)
            {
              tideLabel->setToolTip (tr ("Tide value"));
              correctDepth->setToolTip (tr ("Tide corrected depth"));
              secDepth->setToolTip (tr ("Tide corrected second depth"));
            }
          abdc = save_hof.abdc;
        }
      prev_type = save_hof.data_type;

      if (secondary)
        {
          sec_depth.sprintf ("%.2f", -save_hof.correct_sec_depth);
          slat.sprintf ("%f", save_hof.sec_latitude);
          slon.sprintf ("%f", save_hof.sec_longitude);
          if (save_hof.data_type)
            {
              sabdc.sprintf ("%d", save_hof.kgps_sec_abd_conf);
            }
          else
            {
              sabdc.sprintf ("%d", save_hof.sec_abdc);
            }
          ssig.sprintf ("%.1f", save_hof.sec_bot_conf);
          s_full_conf.sprintf ("%d", save_hof.sec_depth_conf);
        }
      else
        {
          sec_depth = "N/A";
          sabdc = "N/A";
          ssig = "N/A";
          s_full_conf = "N/A";
        }

      wave_height.sprintf ("%.2f", save_hof.wave_height);

      f_full_conf.sprintf ("%d", save_hof.depth_conf);

      calc_bfom.sprintf ("%d", save_hof.calc_bfom_thresh_times10[0]);
      sec_calc_bfom.sprintf ("%d", save_hof.calc_bfom_thresh_times10[1]);


      fsig.sprintf ("%.1f", save_hof.bot_conf);


      //  Set the status bar labels

      correctDepth->setText (first);
      reportedDepth->setText (rep_depth);
      tideLabel->setText (level);
      dataType->setText (mode);
      abdcLabel->setNum (abdc);
      secFullConf->setText (s_full_conf);
      secDepth->setText (sec_depth);
      sabdcLabel->setText (sabdc);
      waveHeight->setText (wave_height);
      fullConf->setText (f_full_conf);
      bfomThresh->setText (calc_bfom);
      secBfomThresh->setText (sec_calc_bfom);
      botBinLabel->setText (bot_bin);
      secBotBinLabel->setText (sec_bot_bin);
      sfcBinLabel->setText (sfc_bin);
      sigStrength->setText (fsig);
      secSigStrength->setText (ssig);


      if (save_hof.status & AU_STATUS_DELETED_BIT) 
        {
          stat = "Deleted";
        }
      else if (save_hof.status & AU_STATUS_KEPT_BIT)
        {
          stat = "Kept";
        }
      else if (save_hof.status & AU_STATUS_SWAPPED_BIT) 
        {
          stat = "Swapped";
        }
      else
        {
          stat = " ";
        }
    }
  else
    {
      if (save_wlf.waveform == PMT)
        {
          if (display[PMT]) 
            {
              scaleWave (save_wlf.waveform_point, save_wave.data[PMT][save_wlf.waveform_point], &pix_x[0], &pix_y[0], PMT, l_mapdef);
              drawX (pix_x[0], pix_y[0], 10, 2, primaryColor);
            }
          bot_bin.sprintf ("PMT %d", save_wlf.waveform_point);
        }
      else
        {
          if (!save_wlf.waveform_point)
            {
              if (display[APD]) 
                {
                  scaleWave (save_wlf.waveform_point, save_wave.data[APD][save_wlf.waveform_point], &pix_x[0], &pix_y[0], APD, l_mapdef);
                  drawX (pix_x[0], pix_y[0], 10, 2, primaryColor);
                }
              bot_bin.sprintf ("APD %d", save_wlf.waveform_point);
            }
          else
            {
              bot_bin.sprintf ("SWA");
            }
        }


      first.sprintf ("%.2f", save_wlf.z);

      if (save_z_offset_present)
        {
          rep_depth.sprintf ("%.2f", save_wlf.z - save_wlf.z_offset);
          level.sprintf ("%.3f", save_wlf.z_offset);
        }
      else
        {
          rep_depth = "";
          level = "";
        }

      botBinLabel->setText (bot_bin);


      //  Set the status bar labels

      correctDepth->setText (first);
      reportedDepth->setText (rep_depth);
      tideLabel->setText (level);
    }


  NV_INT32 w = l_mapdef.draw_width, h = l_mapdef.draw_height;

  map->drawText ("APD", w - 60, h - 70, waveColor[APD], NVTrue);
  map->drawText ("PMT", w - 60, h - 55, waveColor[PMT], NVTrue);
  map->drawText ("IR", w - 60, h - 40, waveColor[IR], NVTrue);
  map->drawText ("RAMAN", w - 60, h - 25, waveColor[RAMAN], NVTrue);
  map->drawText (stat, w - 60, h - 10, surfaceColor, NVTrue);
}



void 
waveformMonitor::drawX (NV_INT32 x, NV_INT32 y, NV_INT32 size, NV_INT32 width, QColor color)
{
  NV_INT32 hs = size / 2;

  map->drawLine (x - hs, y + hs, x + hs, y - hs, color, width, NVTrue, Qt::SolidLine);
  map->drawLine (x + hs, y + hs, x - hs, y - hs, color, width, NVTrue, Qt::SolidLine);
}



void
waveformMonitor::slotRestoreDefaults ()
{
  static NV_BOOL first = NVTrue;

  pos_format = "hdms";
  width = WAVE_X_SIZE;
  height = WAVE_Y_SIZE;
  window_x = 0;
  window_y = 0;
  waveColor[APD] = Qt::yellow;
  waveColor[PMT] = Qt::white;
  waveColor[IR] = Qt::red;
  waveColor[RAMAN] = Qt::cyan;
  surfaceColor = Qt::yellow;
  primaryColor = Qt::green;
  secondaryColor = Qt::red;
  backgroundColor = Qt::black;
  display[APD] = NVTrue;
  display[PMT] = NVTrue;
  display[IR] = NVTrue;
  display[RAMAN] = NVTrue;
  startup_message = NVTrue;


  //  The first time will be called from envin and the prefs dialog won't exist yet.

  if (!first) setFields ();
  first = NVFalse;

  force_redraw = NVTrue;
}



void
waveformMonitor::about ()
{
  QMessageBox::about (this, VERSION,
                      "waveformMonitor - CHARTS wave form monitor."
                      "\n\nAuthor : Jan C. Depner (depnerj@navo.navy.mil)");
}


void
waveformMonitor::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}



void
waveformMonitor::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



//  Get the users defaults.

void
waveformMonitor::envin ()
{
  NV_FLOAT64 saved_version = 0.0;


  // Set Defaults so the if keys don't exist the parms are defined

  slotRestoreDefaults ();
  force_redraw = NVFalse;


  QSettings settings (tr ("navo.navy.mil"), tr ("waveformMonitor"));
  settings.beginGroup (tr ("waveformMonitor"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  pos_format = settings.value (tr ("position form"), pos_format).toString ();

  width = settings.value (tr ("width"), width).toInt ();

  height = settings.value (tr ("height"), height).toInt ();

  window_x = settings.value (tr ("window x"), window_x).toInt ();

  window_y = settings.value (tr ("window y"), window_y).toInt ();


  startup_message = settings.value (tr ("Display Startup Message"), startup_message).toBool ();


  display[APD] = settings.value (tr ("Display APD"), display[APD]).toBool ();
  display[PMT] = settings.value (tr ("Display PMT"), display[PMT]).toBool ();
  display[IR] = settings.value (tr ("Display IR"), display[IR]).toBool ();
  display[RAMAN] = settings.value (tr ("Display Raman"), display[RAMAN]).toBool ();

  wave_line_mode = settings.value (tr ("Wave line mode flag"), wave_line_mode).toBool ();


  NV_INT32 red = settings.value (tr ("APD color/red"), waveColor[APD].red ()).toInt ();
  NV_INT32 green = settings.value (tr ("APD color/green"), waveColor[APD].green ()).toInt ();
  NV_INT32 blue = settings.value (tr ("APD color/blue"), waveColor[APD].blue ()).toInt ();
  NV_INT32 alpha = settings.value (tr ("APD color/alpha"), waveColor[APD].alpha ()).toInt ();
  waveColor[APD].setRgb (red, green, blue, alpha);

  red = settings.value (tr ("PMT color/red"), waveColor[PMT].red ()).toInt ();
  green = settings.value (tr ("PMT color/green"), waveColor[PMT].green ()).toInt ();
  blue = settings.value (tr ("PMT color/blue"), waveColor[PMT].blue ()).toInt ();
  alpha = settings.value (tr ("PMT color/alpha"), waveColor[PMT].alpha ()).toInt ();
  waveColor[PMT].setRgb (red, green, blue, alpha);

  red = settings.value (tr ("IR color/red"), waveColor[IR].red ()).toInt ();
  green = settings.value (tr ("IR color/green"), waveColor[IR].green ()).toInt ();
  blue = settings.value (tr ("IR color/blue"), waveColor[IR].blue ()).toInt ();
  alpha = settings.value (tr ("IR color/alpha"), waveColor[IR].alpha ()).toInt ();
  waveColor[IR].setRgb (red, green, blue, alpha);

  red = settings.value (tr ("Raman color/red"), waveColor[RAMAN].red ()).toInt ();
  green = settings.value (tr ("Raman color/green"), waveColor[RAMAN].green ()).toInt (); 
  blue = settings.value (tr ("Raman color/blue"), waveColor[RAMAN].blue ()).toInt ();
  alpha = settings.value (tr ("Raman color/alpha"), waveColor[RAMAN].alpha ()).toInt ();
  waveColor[RAMAN].setRgb (red, green, blue, alpha);

  red = settings.value (tr ("Surface color/red"), surfaceColor.red ()).toInt ();
  green = settings.value (tr ("Surface color/green"), surfaceColor.green ()).toInt ();
  blue = settings.value (tr ("Surface color/blue"), surfaceColor.blue ()).toInt ();
  alpha = settings.value (tr ("Surface color/alpha"), surfaceColor.alpha ()).toInt ();
  surfaceColor.setRgb (red, green, blue, alpha);

  red = settings.value (tr ("First color/red"), primaryColor.red ()).toInt ();
  green = settings.value (tr ("First color/green"), primaryColor.green ()).toInt ();
  blue = settings.value (tr ("First color/blue"), primaryColor.blue ()).toInt ();
  alpha = settings.value (tr ("First color/alpha"), primaryColor.alpha ()).toInt ();
  primaryColor.setRgb (red, green, blue, alpha);

  red = settings.value (tr ("Second color/red"), secondaryColor.red ()).toInt ();
  green = settings.value (tr ("Second color/green"), secondaryColor.green ()).toInt ();
  blue = settings.value (tr ("Second color/blue"), secondaryColor.blue ()).toInt ();
  alpha = settings.value (tr ("Second color/alpha"), secondaryColor.alpha ()).toInt ();
  secondaryColor.setRgb (red, green, blue, alpha);

  red = settings.value (tr ("Background color/red"), backgroundColor.red ()).toInt ();
  green = settings.value (tr ("Background color/green"), backgroundColor.green ()).toInt ();
  blue = settings.value (tr ("Background color/blue"), backgroundColor.blue ()).toInt ();
  alpha = settings.value (tr ("Background color/alpha"), backgroundColor.alpha ()).toInt ();
  backgroundColor.setRgb (red, green, blue, alpha);


  //  Set the ac zero line colors.

  for (NV_INT32 i = 0 ; i < 4 ; i++)
    {
      acZeroColor[i] = waveColor[i];
      acZeroColor[i].setAlpha (128);
    }

  this->restoreState (settings.value (tr ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  settings.endGroup ();
}




//  Save the users defaults.

void
waveformMonitor::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();

  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  QSettings settings (tr ("navo.navy.mil"), tr ("waveformMonitor"));
  settings.beginGroup (tr ("waveformMonitor"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("position form"), pos_format);

  settings.setValue (tr ("width"), width);

  settings.setValue (tr ("height"), height);

  settings.setValue (tr ("window x"), window_x);

  settings.setValue (tr ("window y"), window_y);


  settings.setValue (tr ("Display Startup Message"), startup_message);


  settings.setValue (tr ("Display APD"), display[APD]);
  settings.setValue (tr ("Display PMT"), display[PMT]);
  settings.setValue (tr ("Display IR"), display[IR]);
  settings.setValue (tr ("Display Raman"), display[RAMAN]);

  settings.setValue (tr ("Wave line mode flag"), wave_line_mode);


  settings.setValue (tr ("PMT color/red"), waveColor[PMT].red ());
  settings.setValue (tr ("PMT color/green"), waveColor[PMT].green ());
  settings.setValue (tr ("PMT color/blue"), waveColor[PMT].blue ());
  settings.setValue (tr ("PMT color/alpha"), waveColor[PMT].alpha ());

  settings.setValue (tr ("APD color/red"), waveColor[APD].red ());
  settings.setValue (tr ("APD color/green"), waveColor[APD].green ());
  settings.setValue (tr ("APD color/blue"), waveColor[APD].blue ());
  settings.setValue (tr ("APD color/alpha"), waveColor[APD].alpha ());

  settings.setValue (tr ("IR color/red"), waveColor[IR].red ());
  settings.setValue (tr ("IR color/green"), waveColor[IR].green ());
  settings.setValue (tr ("IR color/blue"), waveColor[IR].blue ());
  settings.setValue (tr ("IR color/alpha"), waveColor[IR].alpha ());

  settings.setValue (tr ("Raman color/red"), waveColor[RAMAN].red ());
  settings.setValue (tr ("Raman color/green"), waveColor[RAMAN].green ());
  settings.setValue (tr ("Raman color/blue"), waveColor[RAMAN].blue ());
  settings.setValue (tr ("Raman color/alpha"), waveColor[RAMAN].alpha ());

  settings.setValue (tr ("Surface color/red"), surfaceColor.red ());
  settings.setValue (tr ("Surface color/green"), surfaceColor.green ());
  settings.setValue (tr ("Surface color/blue"), surfaceColor.blue ());
  settings.setValue (tr ("Surface color/alpha"), surfaceColor.alpha ());

  settings.setValue (tr ("First color/red"), primaryColor.red ());
  settings.setValue (tr ("First color/green"), primaryColor.green ());
  settings.setValue (tr ("First color/blue"), primaryColor.blue ());
  settings.setValue (tr ("First color/alpha"), primaryColor.alpha ());

  settings.setValue (tr ("Second color/red"), secondaryColor.red ());
  settings.setValue (tr ("Second color/green"), secondaryColor.green ());
  settings.setValue (tr ("Second color/blue"), secondaryColor.blue ());
  settings.setValue (tr ("Second color/alpha"), secondaryColor.alpha ());

  settings.setValue (tr ("Background color/red"), backgroundColor.red ());
  settings.setValue (tr ("Background color/green"), backgroundColor.green ());
  settings.setValue (tr ("Background color/blue"), backgroundColor.blue ());
  settings.setValue (tr ("Background color/alpha"), backgroundColor.alpha ());


  settings.setValue (tr ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}
