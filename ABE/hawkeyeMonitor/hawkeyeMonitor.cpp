#include "hawkeyeMonitor.hpp"
#include "hawkeyeMonitorHelp.hpp"


NV_FLOAT64 settings_version = 1.00;


hawkeyeMonitor::hawkeyeMonitor (NV_INT32 *argc, NV_CHAR **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/hawkeye_monitor.png"));


#warning At present I do not know how to determine - waveform order, scale, offset, polarity, coarse delay, fine delay...

  //  Got to figure out what these scale factors and stuff mean.  Waiting for response from AHAB.

  scaleFactor[0] = scaleFactor[1] = scaleFactor[2] = scaleFactor[3] = 1.0;
  scaleFactor[4] = scaleFactor[5] = scaleFactor[6] = scaleFactor[7] = scaleFactor[8] = scaleFactor[9] = scaleFactor[10] = scaleFactor[11] = 1.0;


  /*  These came from an email from Xin Liu at AHAB...  I have no idea what they mean ;-)

  scaleFactor[4] = 1.0651;
  scaleFactor[5] = 0.9760;
  scaleFactor[6] = 0.7928;
  scaleFactor[7] = 1.1661;
  scaleFactor[8] = 0.6589;
  scaleFactor[9] = 0.9244;
  scaleFactor[10] = 1.3436;
  scaleFactor[11] = 1.0731;
  */

  polarity[0] = polarity[1] = polarity[5] = polarity[6] = polarity[7] = polarity[9] = polarity[10] = polarity[11] = -1.0;
  polarity[2] = polarity[3] = polarity[4] = polarity[8] = 1.0;
  offset[0] = offset[1] = offset[5] = offset[6] = offset[7] = offset[9] = offset[10] = offset[11] = 256.0;
  offset[2] = offset[3] = offset[4] = offset[8] = 0.0;
  shift[0] = shift[1] = shift[4] = shift[5] = shift[6] = shift[7] = shift[8] = shift[9] = shift[10] = shift[11] = 128;
  shift[2] = shift[3] = 32;


  kill_switch = ANCILLARY_FORCE_EXIT;
  for (NV_INT32 i = 0 ; i < 7 ; i++) ac[i] = -1;

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
  tools->setObjectName (tr ("hawkeyeMonitor main toolbar"));


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

  filError = NULL;
  lock_track = NVFalse;
  wave_read = NVFalse;


  //  Set the map values from the defaults

  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      mapdef[i].projection = NO_PROJECTION;
      mapdef[i].draw_width = width;
      mapdef[i].draw_height = height;
      mapdef[i].overlap_percent = 5;
      mapdef[i].grid_inc_x = 0.0;
      mapdef[i].grid_inc_y = 0.0;

      mapdef[i].coasts = NVFalse;
      mapdef[i].landmask = NVFalse;

      mapdef[i].border = 0;
      mapdef[i].coast_color = Qt::white;
      mapdef[i].grid_color = QColor (160, 160, 160, 127);
      mapdef[i].background_color = backgroundColor;


      mapdef[i].initial_bounds.min_x = 0;
      mapdef[i].initial_bounds.min_y = 0;
      mapdef[i].initial_bounds.max_x = 500;
      mapdef[i].initial_bounds.max_y = 300;


      //  Make the map.

      map[i] = new nvMap (this, &mapdef[i]);
      map[i]->setMinimumWidth (500);
      map[i]->setMinimumHeight (130);
      map[i]->setWhatsThis (mapText);
      connect (map[i], SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));

      map[i]->setCursor (Qt::ArrowCursor);
    }


  //  We only need one postRedrawSignal (the last one).

  connect (map[2], SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotPlotWaves (NVMAP_DEF)));


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Layouts, what fun!

  QGroupBox *hBox = new QGroupBox (frame);
  QHBoxLayout *hBoxLayout = new QHBoxLayout;
  hBox->setLayout (hBoxLayout);
  QVBoxLayout *vBoxL = new QVBoxLayout;
  QVBoxLayout *vBoxR = new QVBoxLayout;


  QGroupBox *IR = new QGroupBox (tr ("IR Channel"), this);
  IR->setWhatsThis (IRText);
  QHBoxLayout *IRLayout = new QHBoxLayout;
  IR->setLayout (IRLayout);

  IRLayout->addWidget (map[0]);


  QVBoxLayout *irvbox = new QVBoxLayout;
  irvbox->setAlignment (Qt::AlignTop);

  QButtonGroup *irGrp = new QButtonGroup (this);
  irGrp->setExclusive (FALSE);
  connect (irGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotIRDisplay (int)));


  bIRDisplay[0] = new QCheckBox (tr ("Parallel"), this);
  bIRDisplay[0]->setToolTip (tr ("Toggle display of parallel IR waveform"));
  bIRDisplay[0]->setWhatsThis (bIRDisplay[0]->toolTip ());
  bIRDisplayPalette[0] = bIRDisplay[0]->palette ();
  bIRDisplay[0]->setChecked (displayIR[0]);
  irGrp->addButton (bIRDisplay[0], 0);
  irvbox->addWidget (bIRDisplay[0]);


  bIRDisplay[1] = new QCheckBox (tr ("Ortho"), this);
  bIRDisplay[1]->setToolTip (tr ("Toggle display of ortho IR waveform"));
  bIRDisplay[1]->setWhatsThis (bIRDisplay[1]->toolTip ());
  bIRDisplayPalette[1] = bIRDisplay[1]->palette ();
  bIRDisplay[1]->setChecked (displayIR[1]);
  irGrp->addButton (bIRDisplay[1], 1);
  irvbox->addWidget (bIRDisplay[1]);


  for (NV_INT32 i = 0 ; i < 2 ; i++)
    {
      bIRDisplayPalette[i].setColor (QPalette::Normal, QPalette::Text, IRColor[i]);
      bIRDisplayPalette[i].setColor (QPalette::Inactive, QPalette::Text, IRColor[i]);
      bIRDisplay[i]->setPalette (bIRDisplayPalette[i]);
    }


  IRLayout->addLayout (irvbox, 1);


  vBoxL->addWidget (IR);


  QGroupBox *shallow = new QGroupBox (tr ("Shallow Channel"), this);
  shallow->setWhatsThis (shallowText);
  QHBoxLayout *shallowLayout = new QHBoxLayout;
  shallow->setLayout (shallowLayout);

  shallowLayout->addWidget (map[1]);


  QVBoxLayout *shallowvbox = new QVBoxLayout;
  shallowvbox->setAlignment (Qt::AlignTop);

  QButtonGroup *shallowGrp = new QButtonGroup (this);
  shallowGrp->setExclusive (FALSE);
  connect (shallowGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotShallowDisplay (int)));


  bShallowDisplay[0] = new QCheckBox (tr ("Pixel 0"), this);
  bShallowDisplay[0]->setToolTip (tr ("Toggle display of pixel 0 waveform"));
  bShallowDisplay[0]->setWhatsThis (bShallowDisplay[0]->toolTip ());
  bShallowDisplayPalette[0] = bShallowDisplay[0]->palette ();
  bShallowDisplay[0]->setChecked (displayShallow[0]);
  shallowGrp->addButton (bShallowDisplay[0], 0);
  shallowvbox->addWidget (bShallowDisplay[0]);

  bShallowDisplay[1] = new QCheckBox (tr ("Pixel 1"), this);
  bShallowDisplay[1]->setToolTip (tr ("Toggle display of pixel 1 waveform"));
  bShallowDisplay[1]->setWhatsThis (bShallowDisplay[1]->toolTip ());
  bShallowDisplayPalette[1] = bShallowDisplay[1]->palette ();
  bShallowDisplay[1]->setChecked (displayShallow[1]);
  shallowGrp->addButton (bShallowDisplay[1], 1);
  shallowvbox->addWidget (bShallowDisplay[1]);

  bShallowDisplay[2] = new QCheckBox (tr ("Pixel 2"), this);
  bShallowDisplay[2]->setToolTip (tr ("Toggle display of pixel 2 waveform"));
  bShallowDisplay[2]->setWhatsThis (bShallowDisplay[2]->toolTip ());
  bShallowDisplayPalette[2] = bShallowDisplay[2]->palette ();
  bShallowDisplay[2]->setChecked (displayShallow[2]);
  shallowGrp->addButton (bShallowDisplay[2], 2);
  shallowvbox->addWidget (bShallowDisplay[2]);

  bShallowDisplay[3] = new QCheckBox (tr ("Pixel 3"), this);
  bShallowDisplay[3]->setToolTip (tr ("Toggle display of pixel 3 waveform"));
  bShallowDisplay[3]->setWhatsThis (bShallowDisplay[3]->toolTip ());
  bShallowDisplayPalette[3] = bShallowDisplay[3]->palette ();
  bShallowDisplay[3]->setChecked (displayShallow[3]);
  shallowGrp->addButton (bShallowDisplay[3], 3);
  shallowvbox->addWidget (bShallowDisplay[3]);

  bShallowDisplay[4] = new QCheckBox (tr ("Composite"), this);
  bShallowDisplay[4]->setToolTip (tr ("Toggle display of composite waveform"));
  bShallowDisplay[4]->setWhatsThis (bShallowDisplay[4]->toolTip ());
  bShallowDisplayPalette[4] = bShallowDisplay[4]->palette ();
  bShallowDisplay[4]->setChecked (displayShallow[4]);
  shallowGrp->addButton (bShallowDisplay[4], 4);
  shallowvbox->addWidget (bShallowDisplay[4]);

  bShallowDisplay[5] = new QCheckBox (tr ("Monitor"), this);
  bShallowDisplay[5]->setToolTip (tr ("Toggle display of gain monitor waveform"));
  bShallowDisplay[5]->setWhatsThis (bShallowDisplay[5]->toolTip ());
  bShallowDisplayPalette[5] = bShallowDisplay[5]->palette ();
  bShallowDisplay[5]->setChecked (displayShallow[5]);
  shallowGrp->addButton (bShallowDisplay[5], 5);
  shallowvbox->addWidget (bShallowDisplay[5]);


  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      bShallowDisplayPalette[i].setColor (QPalette::Normal, QPalette::Text, shallowColor[i]);
      bShallowDisplayPalette[i].setColor (QPalette::Inactive, QPalette::Text, shallowColor[i]);
      bShallowDisplay[i]->setPalette (bShallowDisplayPalette[i]);
    }

  shallowLayout->addLayout (shallowvbox, 1);


  vBoxL->addWidget (shallow);


  QGroupBox *deep = new QGroupBox (tr ("Deep Channel"), this);
  deep->setWhatsThis (deepText);
  QHBoxLayout *deepLayout = new QHBoxLayout;
  deep->setLayout (deepLayout);

  deepLayout->addWidget (map[2]);


  QVBoxLayout *deepvbox = new QVBoxLayout;
  deepvbox->setAlignment (Qt::AlignTop);

  QButtonGroup *deepGrp = new QButtonGroup (this);
  deepGrp->setExclusive (FALSE);
  connect (deepGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotDeepDisplay (int)));


  bDeepDisplay[0] = new QCheckBox (tr ("Pixel 0"), this);
  bDeepDisplay[0]->setToolTip (tr ("Toggle display of pixel 0 waveform"));
  bDeepDisplay[0]->setWhatsThis (bDeepDisplay[0]->toolTip ());
  bDeepDisplayPalette[0] = bDeepDisplay[0]->palette ();
  bDeepDisplay[0]->setChecked (displayDeep[0]);
  deepGrp->addButton (bDeepDisplay[0], 0);
  deepvbox->addWidget (bDeepDisplay[0]);

  bDeepDisplay[1] = new QCheckBox (tr ("Pixel 1"), this);
  bDeepDisplay[1]->setToolTip (tr ("Toggle display of pixel 1 waveform"));
  bDeepDisplay[1]->setWhatsThis (bDeepDisplay[1]->toolTip ());
  bDeepDisplayPalette[1] = bDeepDisplay[1]->palette ();
  bDeepDisplay[1]->setChecked (displayDeep[1]);
  deepGrp->addButton (bDeepDisplay[1], 1);
  deepvbox->addWidget (bDeepDisplay[1]);

  bDeepDisplay[2] = new QCheckBox (tr ("Pixel 2"), this);
  bDeepDisplay[2]->setToolTip (tr ("Toggle display of pixel 2 waveform"));
  bDeepDisplay[2]->setWhatsThis (bDeepDisplay[2]->toolTip ());
  bDeepDisplayPalette[2] = bDeepDisplay[2]->palette ();
  bDeepDisplay[2]->setChecked (displayDeep[2]);
  deepGrp->addButton (bDeepDisplay[2], 2);
  deepvbox->addWidget (bDeepDisplay[2]);

  bDeepDisplay[3] = new QCheckBox (tr ("Pixel 3"), this);
  bDeepDisplay[3]->setToolTip (tr ("Toggle display of pixel 3 waveform"));
  bDeepDisplay[3]->setWhatsThis (bDeepDisplay[3]->toolTip ());
  bDeepDisplayPalette[3] = bDeepDisplay[3]->palette ();
  bDeepDisplay[3]->setChecked (displayDeep[3]);
  deepGrp->addButton (bDeepDisplay[3], 3);
  deepvbox->addWidget (bDeepDisplay[3]);

  bDeepDisplay[4] = new QCheckBox (tr ("Composite"), this);
  bDeepDisplay[4]->setToolTip (tr ("Toggle display of composite waveform"));
  bDeepDisplay[4]->setWhatsThis (bDeepDisplay[4]->toolTip ());
  bDeepDisplayPalette[4] = bDeepDisplay[4]->palette ();
  bDeepDisplay[4]->setChecked (displayDeep[4]);
  deepGrp->addButton (bDeepDisplay[4], 4);
  deepvbox->addWidget (bDeepDisplay[4]);

  bDeepDisplay[5] = new QCheckBox (tr ("Monitor"), this);
  bDeepDisplay[5]->setToolTip (tr ("Toggle display of gain monitor waveform"));
  bDeepDisplay[5]->setWhatsThis (bDeepDisplay[5]->toolTip ());
  bDeepDisplayPalette[5] = bDeepDisplay[5]->palette ();
  bDeepDisplay[5]->setChecked (displayDeep[5]);
  deepGrp->addButton (bDeepDisplay[5], 5);
  deepvbox->addWidget (bDeepDisplay[5]);


  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      bDeepDisplayPalette[i].setColor (QPalette::Normal, QPalette::Text, deepColor[i]);
      bDeepDisplayPalette[i].setColor (QPalette::Inactive, QPalette::Text, deepColor[i]);
      bDeepDisplay[i]->setPalette (bDeepDisplayPalette[i]);
    }


  deepLayout->addLayout (deepvbox);


  vBoxL->addWidget (deep, 1);


  QGroupBox *shot = new QGroupBox (tr ("Shot information"), this);
  shot->setWhatsThis (shotText);
  QVBoxLayout *shotLayout = new QVBoxLayout;
  shot->setLayout (shotLayout);

  QGroupBox *data = new QGroupBox (this);
  QVBoxLayout *dataLayout = new QVBoxLayout;
  data->setLayout (dataLayout);

  listBox = new QTextEdit (this);
  listBox->setMinimumWidth (300);
  listBox->setReadOnly (TRUE);
  dataLayout->addWidget (listBox);  

  shotLayout->addWidget (data);

  vBoxR->addWidget (shot);


  hBoxLayout->addLayout (vBoxL);
  hBoxLayout->addLayout (vBoxR);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.xpm"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


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
  aboutAct->setStatusTip (tr ("Information about hawkeyeMonitor"));
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


  map[0]->enableSignals ();
  map[1]->enableSignals ();
  map[2]->enableSignals ();


  QTimer *track = new QTimer (this);
  connect (track, SIGNAL (timeout ()), this, SLOT (trackCursor ()));
  track->start (10);
}



hawkeyeMonitor::~hawkeyeMonitor ()
{
}



void 
hawkeyeMonitor::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



void 
hawkeyeMonitor::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  force_redraw = NVTrue;
}



void
hawkeyeMonitor::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
hawkeyeMonitor::slotIRDisplay (int id)
{
  if (bIRDisplay[id]->isChecked ())
    {
      displayIR[id] = NVTrue;
    }
  else
    {
      displayIR[id] = NVFalse;
    }

  force_redraw = NVTrue;
}



void
hawkeyeMonitor::slotShallowDisplay (int id)
{
  if (bShallowDisplay[id]->isChecked ())
    {
      displayShallow[id] = NVTrue;
    }
  else
    {
      displayShallow[id] = NVFalse;
    }

  force_redraw = NVTrue;
}



void
hawkeyeMonitor::slotDeepDisplay (int id)
{
  if (bDeepDisplay[id]->isChecked ())
    {
      displayDeep[id] = NVTrue;
    }
  else
    {
      displayDeep[id] = NVFalse;
    }

  force_redraw = NVTrue;
}



void 
hawkeyeMonitor::leftMouse (NV_FLOAT64 x __attribute__ ((unused)), NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Placeholder
}



void 
hawkeyeMonitor::midMouse (NV_FLOAT64 x __attribute__ ((unused)), NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Placeholder
}



void 
hawkeyeMonitor::rightMouse (NV_FLOAT64 x __attribute__ ((unused)), NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Placeholder
}



//  Signal from the map class.

void 
hawkeyeMonitor::slotMousePress (QMouseEvent * e, NV_FLOAT64 x, NV_FLOAT64 y)
{
  if (e->button () == Qt::LeftButton) leftMouse (x, y);
  if (e->button () == Qt::MidButton) midMouse (x, y);
  if (e->button () == Qt::RightButton) rightMouse (x, y);
}



//  Signal from the map class.

void
hawkeyeMonitor::slotMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 x __attribute__ ((unused)),
                               NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Placeholder
}



//  Timer - timeout signal.  Very much like an X workproc.

void
hawkeyeMonitor::trackCursor ()
{
  NV_INT32                hawkeye_handle;
  HAWKEYE_META_HEADER     *meta_header;
  HAWKEYE_SHOT_DATA_HYDRO hawkeye_shot_data_hydro;
  static NV_U_INT32       prev_rec = -1;
  static ABE_SHARE        l_share;


  //  Since this is always a child process of something, we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.
  //  We also want to exit on the ANCILLARY_FORCE_EXIT key (from pfmEdit) or if our own personal kill signal
  //  has been placed in abe_share->key.

  if (abe_share->key == CHILD_PROCESS_FORCE_EXIT || abe_share->key == ANCILLARY_FORCE_EXIT ||
      abe_share->key == kill_switch) slotQuit ();


  //  We want to exit if we have locked the tracker to update our saved waveforms (in plotWaves).

  if (lock_track) return;


  //  Locking makes sure another process does not have memory locked.  It will block until it can lock it.
  //  At that point we copy the contents and then unlock it so other processes can continue.

  abeShare->lock ();


  //  Check for change of record and correct record type.

  NV_BOOL hit = NVFalse;
  if (prev_rec != abe_share->mwShare.multiRecord[0] && (abe_share->mwShare.multiType[0] == PFM_HAWKEYE_HYDRO_DATA))
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

      force_redraw = NVTrue;
    }


  if (abe_share->key == ac[1])
    {
      abe_share->key = 0;
      abe_share->modcode = NO_ACTION_REQUIRED;

      force_redraw = NVTrue;
    }


  if (abe_share->key == ac[2])
    {
      abe_share->key = 0;
      abe_share->modcode = NO_ACTION_REQUIRED;

      force_redraw = NVTrue;
    }


  if (abe_share->key == ac[3])
    {
      abe_share->key = 0;
      abe_share->modcode = NO_ACTION_REQUIRED;

      force_redraw = NVTrue;
    }


  if (abe_share->modcode == WAVEMONITOR_FORCE_REDRAW) force_redraw = NVTrue;


  abeShare->unlock ();


  //  Hit or force redraw from above.

  if (hit || force_redraw)
    {
      //  This will keep the postRedraw slot from doing anything until we're done loading the waveform.

      wave_read = NVFalse;
      force_redraw = NVFalse;
      hit = NVFalse;
      NV_BOOL err = NVTrue;

 
      //  Save for plotWaves.

      recnum = l_share.mwShare.multiRecord[0];
      strcpy (filename, l_share.nearest_filename);


      //  Set the range to 0 in case of error.

      for (NV_INT32 i = 0 ; i < hydroChannelID_nrOfChannels ; i++) bounds[i].range_y = 0;


      //  Open the Hawkeye file and read the data.

      if ((hawkeye_handle = hawkeye_open_file (filename, &meta_header, &contents_header, HAWKEYE_READONLY)))
        {
          statusBar ()->showMessage (tr ("Unable to open file!"));
        }
      else
        {
          available = contents_header->available;

          if (hawkeye_read_record (hawkeye_handle, recnum, &hawkeye_record))
            {
              statusBar ()->showMessage (tr ("Unable to read record!"));
            }
          else
            {
              if (hawkeye_read_shot_data_hydro (hawkeye_handle, recnum, &hawkeye_shot_data_hydro))
                {
                  NV_INT32 err = hawkeye_get_errno ();

                  if (err == HAWKEYE_SFM_EOF_ERROR)
                    {
                      statusBar ()->showMessage (tr ("Record not available in Shot File Map (.sfm) file"));
                    }
                  else
                    {
                      statusBar ()->showMessage (tr ("Error reading record in Shot File Map (.sfm) file"));
                    }
                }
              else
                {
                  for (NV_INT32 i = 0 ; i < hydroChannelID_nrOfChannels ; i++)
                    {
                      bounds[i].pulse_length = hawkeye_shot_data_hydro.channelInfo[i].pulseLength;
                      memcpy (wave_data[i].pulse, hawkeye_shot_data_hydro.wave_pulse[i], bounds[i].pulse_length);

                      bounds[i].reflex_length = hawkeye_shot_data_hydro.channelInfo[i].reflexLength;
                      memcpy (wave_data[i].reflex, hawkeye_shot_data_hydro.wave_reflex[i], bounds[i].reflex_length);

                      bounds[i].min_x = 0;
                      bounds[i].max_x = bounds[i].reflex_length;
                      bounds[i].min_y = 0;
                      bounds[i].max_y = 256;


                      //  Add 2% to the X axis.

                      bounds[i].range_x = bounds[i].max_x - bounds[i].min_x;
                      bounds[i].min_x = bounds[i].min_x - NINT (((NV_FLOAT32) bounds[i].range_x * 0.02 + 1));
                      bounds[i].max_x = bounds[i].max_x + NINT (((NV_FLOAT32) bounds[i].range_x * 0.02 + 1));
                      bounds[i].range_x = bounds[i].max_x - bounds[i].min_x;
                      bounds[i].range_y = bounds[i].max_y - bounds[i].min_y;
                    }

                  err = NVFalse;
                }
            }
        }


      hawkeye_close_file (hawkeye_handle);

      l_share.key = 0;

      abeShare->lock ();
      abe_share->key = 0;
      abe_share->modcode = return_type;
      abeShare->unlock ();


      if (!err)
        {
          lock_track = NVTrue;
          wave_read = NVTrue;
        }


      map[0]->redrawMapArea (NVTrue);
      map[1]->redrawMapArea (NVTrue);
      map[2]->redrawMapArea (NVTrue);
    }
}



//  Signal from the map class.

void 
hawkeyeMonitor::slotKeyPress (QKeyEvent *e)
{
  NV_CHAR key[20];
  strcpy (key, e->text ().toAscii ());

  if (key[0] == (NV_CHAR) ac[0])
    {
      force_redraw = NVTrue;
    }


  if (key[0] == (NV_CHAR) ac[1])
    {
      force_redraw = NVTrue;
    }


  if (key[0] == (NV_CHAR) ac[2])
    {
      force_redraw = NVTrue;
    }


  if (key[0] == (NV_CHAR) ac[3])
    {
      force_redraw = NVTrue;
    }
}



//  A bunch of slots.

void 
hawkeyeMonitor::slotQuit ()
{
  //  Let the parent program know that we have died from something other than the kill switch from the parent.

  if (abe_share->key != kill_switch) abe_share->killed = kill_switch;


  envout ();


  //  Let go of the shared memory.

  abeShare->detach ();


  exit (0);
}



void 
hawkeyeMonitor::setFields ()
{
  bGrp->button (pos_format)->setChecked (TRUE);


  NV_INT32 hue, sat, val;

  for (NV_INT32 i = 0 ; i < 2 ; i++)
    {
      IRColor[i].getHsv (&hue, &sat, &val);

      if (val < 128 || (hue > 200 && hue < 280))
	{
	  bIRPalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
	  bIRPalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
	}
      else
	{
	  bIRPalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
	  bIRPalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
	}
      bIRPalette[i].setColor (QPalette::Normal, QPalette::Button, IRColor[i]);
      bIRPalette[i].setColor (QPalette::Inactive, QPalette::Button, IRColor[i]);
      bIRColor[i]->setPalette (bIRPalette[i]);

      bIRDisplayPalette[i].setColor (QPalette::Normal, QPalette::Text, IRColor[i]);
      bIRDisplayPalette[i].setColor (QPalette::Inactive, QPalette::Text, IRColor[i]);
      bIRDisplay[i]->setPalette (bIRDisplayPalette[i]);
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      shallowColor[i].getHsv (&hue, &sat, &val);

      if (val < 128 || (hue > 200 && hue < 280))
	{
	  bShallowPalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
	  bShallowPalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
	}
      else
	{
	  bShallowPalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
	  bShallowPalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
	}
      bShallowPalette[i].setColor (QPalette::Normal, QPalette::Button, shallowColor[i]);
      bShallowPalette[i].setColor (QPalette::Inactive, QPalette::Button, shallowColor[i]);
      bShallowColor[i]->setPalette (bShallowPalette[i]);

      bShallowDisplayPalette[i].setColor (QPalette::Normal, QPalette::Text, shallowColor[i]);
      bShallowDisplayPalette[i].setColor (QPalette::Inactive, QPalette::Text, shallowColor[i]);
      bShallowDisplay[i]->setPalette (bShallowDisplayPalette[i]);
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      deepColor[i].getHsv (&hue, &sat, &val);

      if (val < 128 || (hue > 200 && hue < 280))
	{
	  bDeepPalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
	  bDeepPalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
	}
      else
	{
	  bDeepPalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
	  bDeepPalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
	}
      bDeepPalette[i].setColor (QPalette::Normal, QPalette::Button, deepColor[i]);
      bDeepPalette[i].setColor (QPalette::Inactive, QPalette::Button, deepColor[i]);
      bDeepColor[i]->setPalette (bDeepPalette[i]);

      bDeepDisplayPalette[i].setColor (QPalette::Normal, QPalette::Text, deepColor[i]);
      bDeepDisplayPalette[i].setColor (QPalette::Inactive, QPalette::Text, deepColor[i]);
      bDeepDisplay[i]->setPalette (bDeepDisplayPalette[i]);
    }

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
hawkeyeMonitor::slotPrefs ()
{
  prefsD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  prefsD->setWindowTitle (tr ("hawkeyeMonitor Preferences"));
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

  bGrp->button (pos_format)->setChecked (TRUE);


  QGroupBox *cbox = new QGroupBox (tr ("Colors"), this);
  QVBoxLayout *cboxLayout = new QVBoxLayout;
  cbox->setLayout (cboxLayout);
  QHBoxLayout *cboxTopLayout = new QHBoxLayout;
  QHBoxLayout *cboxMd1Layout = new QHBoxLayout;
  QHBoxLayout *cboxMd2Layout = new QHBoxLayout;
  QHBoxLayout *cboxBotLayout = new QHBoxLayout;
  cboxLayout->addLayout (cboxTopLayout);
  cboxLayout->addLayout (cboxMd1Layout);
  cboxLayout->addLayout (cboxMd2Layout);
  cboxLayout->addLayout (cboxBotLayout);


  QButtonGroup *bIRGrp = new QButtonGroup (this);
  connect (bIRGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotIRColor (int)));

  bIRColor[0] = new QPushButton (tr ("Parallel IR color"), this);
  bIRColor[0]->setToolTip (tr ("Change parallel IR color"));
  bIRColor[0]->setWhatsThis (bIRColor[0]->toolTip ());
  bIRPalette[0] = bIRColor[0]->palette ();
  bIRGrp->addButton (bIRColor[0], 0);
  cboxBotLayout->addWidget (bIRColor[0]);

  bIRColor[1] = new QPushButton (tr ("Ortho IR color"), this);
  bIRColor[1]->setToolTip (tr ("Change ortho IR color"));
  bIRColor[1]->setWhatsThis (bIRColor[1]->toolTip ());
  bIRPalette[1] = bIRColor[1]->palette ();
  bIRGrp->addButton (bIRColor[0], 1);
  cboxBotLayout->addWidget (bIRColor[1]);


  QButtonGroup *bShallowGrp = new QButtonGroup (this);
  connect (bShallowGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotShallowColor (int)));

  bShallowColor[0] = new QPushButton (tr ("Shallow pixel 0 color"), this);
  bShallowColor[0]->setToolTip (tr ("Change shallow pixel 0 color"));
  bShallowColor[0]->setWhatsThis (bShallowColor[0]->toolTip ());
  bShallowPalette[0] = bShallowColor[0]->palette ();
  bShallowGrp->addButton (bShallowColor[0], 0);
  cboxTopLayout->addWidget (bShallowColor[0]);

  bShallowColor[1] = new QPushButton (tr ("Shallow pixel 1 color"), this);
  bShallowColor[1]->setToolTip (tr ("Change shallow pixel 1 color"));
  bShallowColor[1]->setWhatsThis (bShallowColor[1]->toolTip ());
  bShallowPalette[1] = bShallowColor[1]->palette ();
  bShallowGrp->addButton (bShallowColor[1], 1);
  cboxTopLayout->addWidget (bShallowColor[1]);

  bShallowColor[2] = new QPushButton (tr ("Shallow pixel 2 color"), this);
  bShallowColor[2]->setToolTip (tr ("Change shallow pixel 2 color"));
  bShallowColor[2]->setWhatsThis (bShallowColor[2]->toolTip ());
  bShallowPalette[2] = bShallowColor[2]->palette ();
  bShallowGrp->addButton (bShallowColor[2], 2);
  cboxTopLayout->addWidget (bShallowColor[2]);

  bShallowColor[3] = new QPushButton (tr ("Shallow pixel 3 color"), this);
  bShallowColor[3]->setToolTip (tr ("Change shallow pixel 3 color"));
  bShallowColor[3]->setWhatsThis (bShallowColor[3]->toolTip ());
  bShallowPalette[3] = bShallowColor[3]->palette ();
  bShallowGrp->addButton (bShallowColor[3], 3);
  cboxTopLayout->addWidget (bShallowColor[3]);

  bShallowColor[4] = new QPushButton (tr ("Shallow composite color"), this);
  bShallowColor[4]->setToolTip (tr ("Change shallow composite color"));
  bShallowColor[4]->setWhatsThis (bShallowColor[4]->toolTip ());
  bShallowPalette[4] = bShallowColor[4]->palette ();
  bShallowGrp->addButton (bShallowColor[4], 4);
  cboxMd2Layout->addWidget (bShallowColor[4]);

  bShallowColor[5] = new QPushButton (tr ("Shallow monitor color"), this);
  bShallowColor[5]->setToolTip (tr ("Change shallow gain monitor color"));
  bShallowColor[5]->setWhatsThis (bShallowColor[5]->toolTip ());
  bShallowPalette[5] = bShallowColor[5]->palette ();
  bShallowGrp->addButton (bShallowColor[5], 5);
  cboxMd2Layout->addWidget (bShallowColor[5]);


  QButtonGroup *bDeepGrp = new QButtonGroup (this);
  connect (bDeepGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotDeepColor (int)));

  bDeepColor[0] = new QPushButton (tr ("Deep pixel 0 color"), this);
  bDeepColor[0]->setToolTip (tr ("Change deep pixel 0 color"));
  bDeepColor[0]->setWhatsThis (bDeepColor[0]->toolTip ());
  bDeepPalette[0] = bDeepColor[0]->palette ();
  bDeepGrp->addButton (bDeepColor[0], 0);
  cboxMd1Layout->addWidget (bDeepColor[0]);

  bDeepColor[1] = new QPushButton (tr ("Deep pixel 1 color"), this);
  bDeepColor[1]->setToolTip (tr ("Change deep pixel 1 color"));
  bDeepColor[1]->setWhatsThis (bDeepColor[1]->toolTip ());
  bDeepPalette[1] = bDeepColor[1]->palette ();
  bDeepGrp->addButton (bDeepColor[1], 1);
  cboxMd1Layout->addWidget (bDeepColor[1]);

  bDeepColor[2] = new QPushButton (tr ("Deep pixel 2 color"), this);
  bDeepColor[2]->setToolTip (tr ("Change deep pixel 2 color"));
  bDeepColor[2]->setWhatsThis (bDeepColor[2]->toolTip ());
  bDeepPalette[2] = bDeepColor[2]->palette ();
  bDeepGrp->addButton (bDeepColor[2], 2);
  cboxMd1Layout->addWidget (bDeepColor[2]);

  bDeepColor[3] = new QPushButton (tr ("Deep pixel 3 color"), this);
  bDeepColor[3]->setToolTip (tr ("Change deep pixel 3 color"));
  bDeepColor[3]->setWhatsThis (bDeepColor[3]->toolTip ());
  bDeepPalette[3] = bDeepColor[3]->palette ();
  bDeepGrp->addButton (bDeepColor[3], 3);
  cboxMd1Layout->addWidget (bDeepColor[3]);

  bDeepColor[4] = new QPushButton (tr ("Deep composite color"), this);
  bDeepColor[4]->setToolTip (tr ("Change deep composite color"));
  bDeepColor[4]->setWhatsThis (bDeepColor[4]->toolTip ());
  bDeepPalette[4] = bDeepColor[4]->palette ();
  bDeepGrp->addButton (bDeepColor[4], 4);
  cboxMd2Layout->addWidget (bDeepColor[4]);

  bDeepColor[5] = new QPushButton (tr ("Deep monitor color"), this);
  bDeepColor[5]->setToolTip (tr ("Change deep gain monitor color"));
  bDeepColor[5]->setWhatsThis (bDeepColor[5]->toolTip ());
  bDeepPalette[5] = bDeepColor[5]->palette ();
  bDeepGrp->addButton (bDeepColor[5], 5);
  cboxMd2Layout->addWidget (bDeepColor[5]);


  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundColor->setToolTip (tr ("Change display background color"));
  bBackgroundColor->setWhatsThis (bBackgroundColor->toolTip ());
  bBackgroundPalette = bBackgroundColor->palette ();
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotBackgroundColor ()));
  cboxBotLayout->addWidget (bBackgroundColor);


  vbox->addWidget (cbox, 1);


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
hawkeyeMonitor::slotPosClicked (int id)
{
  pos_format = id;
}



void
hawkeyeMonitor::slotClosePrefs ()
{
  prefsD->close ();
}



void
hawkeyeMonitor::slotIRColor (int id)
{
  QString title;

  if (id)
    {
      title = tr ("hawkeyeMonitor ortho IR Color");
    }
  else
    {
      title = tr ("hawkeyeMonitor parallel IR Color");
    }

  QColor clr;
  clr = QColorDialog::getColor (IRColor[id], this, title, QColorDialog::ShowAlphaChannel);
  if (clr.isValid ()) IRColor[id] = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
hawkeyeMonitor::slotShallowColor (int id)
{
  QString title;

  switch (id)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      title = tr ("hawkeyeMonitor shallow pixel %1 Color").arg (id);
      break;

    case 4:
      title = tr ("hawkeyeMonitor shallow composite Color");
      break;

    case 5:
      title = tr ("hawkeyeMonitor shallow gain monitor Color");
      break;
    }

  QColor clr;
  clr = QColorDialog::getColor (shallowColor[id], this, title, QColorDialog::ShowAlphaChannel);
  if (clr.isValid ()) shallowColor[id] = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
hawkeyeMonitor::slotDeepColor (int id)
{
  QString title;

  switch (id)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      title = tr ("hawkeyeMonitor deep pixel %1 Color").arg (id);
      break;

    case 4:
      title = tr ("hawkeyeMonitor deep composite Color");
      break;

    case 5:
      title = tr ("hawkeyeMonitor deep gain monitor Color");
      break;
    }

  QColor clr;
  clr = QColorDialog::getColor (deepColor[id], this, title, QColorDialog::ShowAlphaChannel);
  if (clr.isValid ()) deepColor[id] = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
hawkeyeMonitor::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (backgroundColor, this, tr ("hawkeyeMonitor Background Color"));

  if (clr.isValid ()) backgroundColor = clr;

  setFields ();

  force_redraw = NVTrue;
}



void 
hawkeyeMonitor::scaleWave (NV_INT32 x, NV_INT32 y, NV_INT32 *new_x, NV_INT32 *new_y, NV_INT32 type, NVMAP_DEF l_mapdef)
{
  *new_y = NINT (((NV_FLOAT32) (bounds[type].max_y - y) / (NV_FLOAT32) bounds[type].range_y) * (NV_FLOAT32) l_mapdef.draw_height);
  *new_x = NINT (((NV_FLOAT32) (x - bounds[type].min_x) / (NV_FLOAT32) bounds[type].range_x) * (NV_FLOAT32) l_mapdef.draw_width);
}



void 
hawkeyeMonitor::slotPlotWaves (NVMAP_DEF l_mapdef __attribute__ ((unused)))
{
  NV_INT32                src, pix_x[2], pix_y[2], composite[500], value;
  NVMAP_DEF               j_mapdef;


  void dumpRecord (QTextEdit *listBox, HAWKEYE_RECORD hawkeye_record, HAWKEYE_RECORD_AVAILABILITY available, NV_INT32 pos_format);


  //  If the trackCursor function is loading data we don't want to try to plot any.

  if (!wave_read) return;


  //  Because the trackCursor function may be changing the data while we're still plotting it we save it
  //  to this static structure.  lock_track (set in trackCursor) stops trackCursor from updating while we're
  //  trying to get an atomic snapshot of the data for the latest point.

  for (NV_INT32 j = 0 ; j < hydroChannelID_nrOfChannels ; j++) save_wave[j] = wave_data[j];
  save_hawkeye = hawkeye_record;
  strcpy (save_filename, filename);
  save_available = available;
  lock_track = NVFalse;


  //  Draw the IR waveforms.

  j_mapdef = map[0]->getMapdef ();

  for (NV_INT32 k = 0 ; k < 2 ; k++)
    {
      src = k;

      if (displayIR[k])
        {
          for (NV_INT32 i = 0 ; i < bounds[src].reflex_length ; i++)
            {
              NV_INT32 value = NINT (polarity[src] * (((NV_FLOAT64) save_wave[src].reflex[i]) - offset[src])) - shift[src];
              if (value < 0) value += 256;

              scaleWave (i, value, &pix_x[1], &pix_y[1], src, j_mapdef);

              if (i) map[0]->drawLine (pix_x[0], pix_y[0], pix_x[1], pix_y[1], IRColor[k], 1, NVFalse, Qt::SolidLine);

              pix_x[0] = pix_x[1];
              pix_y[0] = pix_y[1];
            }
        }
    }


  //  Draw the shallow waveforms.

  j_mapdef = map[1]->getMapdef ();

  for (NV_INT32 k = 0 ; k < 6 ; k++)
    {
      if (k < 4)
        {
          src = k + 4;
        }
      else if (k == 5)
        {
          src = 2;
        }
      else
        {
          //  Build the composite waveform.

          for (NV_INT32 i = 0 ; i < bounds[src].reflex_length ; i++)
            {
              NV_INT32 ndx = src - 4;
              composite[i] = NINT (((polarity[ndx] * (((NV_FLOAT64) save_wave[ndx].reflex[i] * scaleFactor[ndx]) - offset[ndx])) +
                                    (polarity[ndx + 1] * (((NV_FLOAT64) save_wave[ndx + 1].reflex[i] * scaleFactor[ndx + 1]) - offset[ndx + 1])) +
                                    (polarity[ndx + 2] * (((NV_FLOAT64) save_wave[ndx + 2].reflex[i] * scaleFactor[ndx + 2]) - offset[ndx + 2])) +
                                    (polarity[ndx + 3] * (((NV_FLOAT64) save_wave[ndx + 3].reflex[i] * scaleFactor[ndx + 3]) - offset[ndx + 3]))) / 4.0);
            }
        }

      if (displayShallow[k])
        {
          for (NV_INT32 i = 0 ; i < bounds[src].reflex_length ; i++)
            {
              if (k == 4)
                {
                  value = composite[i];
                }
              else
                {
                  value = NINT (polarity[src] * (((NV_FLOAT64) save_wave[src].reflex[i] * scaleFactor[src]) - offset[src])) - shift[src];
                  if (value < 0) value += 256;
                }
              scaleWave (i, value, &pix_x[1], &pix_y[1], src, j_mapdef);

              if (i) map[1]->drawLine (pix_x[0], pix_y[0], pix_x[1], pix_y[1], shallowColor[k], 1, NVFalse, Qt::SolidLine);

              pix_x[0] = pix_x[1];
              pix_y[0] = pix_y[1];
            }
        }
    }


  //  Draw the deep waveforms.

  j_mapdef = map[2]->getMapdef ();

  for (NV_INT32 k = 0 ; k < 6 ; k++)
    {
      if (k < 4)
        {
          src = k + 8;
        }
      else if (k == 5)
        {
          src = 3;
        }
      else
        {
          //  Build the composite waveform.

          for (NV_INT32 i = 0 ; i < bounds[src].reflex_length ; i++)
            {
              NV_INT32 ndx = src - 4;
              composite[i] = NINT (((polarity[ndx] * (((NV_FLOAT64) save_wave[ndx].reflex[i] * scaleFactor[ndx]) - offset[ndx])) +
                                    (polarity[ndx + 1] * (((NV_FLOAT64) save_wave[ndx + 1].reflex[i] * scaleFactor[ndx + 1]) - offset[ndx + 1])) +
                                    (polarity[ndx + 2] * (((NV_FLOAT64) save_wave[ndx + 2].reflex[i] * scaleFactor[ndx + 2]) - offset[ndx + 2])) +
                                    (polarity[ndx + 3] * (((NV_FLOAT64) save_wave[ndx + 3].reflex[i] * scaleFactor[ndx + 3]) - offset[ndx + 3]))) / 4.0);
            }
        }

      if (displayDeep[k])
        {
          for (NV_INT32 i = 0 ; i < bounds[src].reflex_length ; i++)
            {
              if (k == 4)
                {
                  value = composite[i];
                }
              else
                {
                  value = NINT (polarity[src] * (((NV_FLOAT64) save_wave[src].reflex[i] * scaleFactor[src]) - offset[src])) - shift[src];
                  if (value < 0) value += 256;
                }
              scaleWave (i, value, &pix_x[1], &pix_y[1], src, j_mapdef);

              if (i) map[2]->drawLine (pix_x[0], pix_y[0], pix_x[1], pix_y[1], deepColor[k], 1, NVFalse, Qt::SolidLine);

              pix_x[0] = pix_x[1];
              pix_y[0] = pix_y[1];
            }
        }
    }


  //  Now populate the shot information area.

  statusBar ()->showMessage (save_filename);


  //  Save the scroll bar position so we can maintain the view that we start with (in case the user wants to 
  //  see something other than the first 30 or so fields).

  NV_INT32 scrollValue = listBox->verticalScrollBar ()->value ();


  //  Fill the text edit box.

  dumpRecord (listBox, save_hawkeye, save_available, pos_format);


  //  Restore the view.

  listBox->verticalScrollBar ()->setValue (scrollValue);
}



void
hawkeyeMonitor::slotRestoreDefaults ()
{
  static NV_BOOL first = NVTrue;

  for (NV_INT32 i = 0 ; i < 2 ; i++) displayIR[i] = NVTrue;
  for (NV_INT32 i = 0 ; i < 6 ; i++) displayShallow[i] = NVTrue;
  for (NV_INT32 i = 0 ; i < 6 ; i++) displayDeep[i] = NVTrue;
  displayShallow[4] = NVFalse;
  displayDeep[4] = NVFalse;

  pos_format = 0;

  IRColor[0] = QColor (176, 0, 0);
  IRColor[1] = QColor (0, 176, 0);
  shallowColor[0] = QColor (176, 0, 0);
  shallowColor[1] = QColor (0, 176, 0);
  shallowColor[2] = QColor (0, 0, 176);
  shallowColor[3] = QColor (176, 0, 176);
  shallowColor[4] = QColor (0, 0, 0);
  shallowColor[5] = QColor (68, 204, 204);
  deepColor[0] = QColor (176, 0, 0);
  deepColor[1] = QColor (0, 176, 0);
  deepColor[2] = QColor (0, 0, 176);
  deepColor[3] = QColor (176, 0, 176);
  deepColor[4] = QColor (0, 0, 0);
  deepColor[5] = QColor (68, 204, 204);
  backgroundColor = QColor (255, 255, 255);

  width = 1024;
  height = 660;
  window_x = 0;
  window_y = 0;


  //  The first time restoreDefaults will be called from envin and the prefs dialog won't exist yet.

  if (!first) setFields ();
  first = NVFalse;

  force_redraw = NVTrue;
}



void
hawkeyeMonitor::about ()
{
  QMessageBox::about (this, VERSION,
                      "hawkeyeMonitor - CHARTS wave form monitor."
                      "\n\nAuthor : Jan C. Depner (jan.depner@navy.mil)");
}


void
hawkeyeMonitor::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}



void
hawkeyeMonitor::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



//  Get the users defaults.

void
hawkeyeMonitor::envin ()
{
  NV_FLOAT64 saved_version = 0.0;


  // Set Defaults so the if keys don't exist the parms are defined

  slotRestoreDefaults ();
  force_redraw = NVFalse;


  QSettings settings (tr ("navo.navy.mil"), tr ("hawkeyeMonitor"));
  settings.beginGroup (tr ("hawkeyeMonitor"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  pos_format = settings.value (tr ("position form"), pos_format).toInt ();

  width = settings.value (tr ("width"), width).toInt ();

  height = settings.value (tr ("height"), height).toInt ();

  window_x = settings.value (tr ("window x"), window_x).toInt ();

  window_y = settings.value (tr ("window y"), window_y).toInt ();

  for (NV_INT32 i = 0 ; i < 2 ; i++)
    {
      QString tag = tr ("Display IR %1").arg (i);
      displayIR[i] = settings.value (tag, displayIR[i]).toBool ();
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      QString tag = tr ("Display shallow %1").arg (i);
      displayShallow[i] = settings.value (tag, displayShallow[i]).toBool ();
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      QString tag = tr ("Display deep %1").arg (i);
      displayDeep[i] = settings.value (tag, displayDeep[i]).toBool ();
    }

  NV_INT32 red = settings.value (tr ("Background color/red"), backgroundColor.red ()).toInt ();
  NV_INT32 green = settings.value (tr ("Background color/green"), backgroundColor.green ()).toInt ();
  NV_INT32 blue = settings.value (tr ("Background color/blue"), backgroundColor.blue ()).toInt ();
  NV_INT32 alpha = settings.value (tr ("Background color/alpha"), backgroundColor.alpha ()).toInt ();
  backgroundColor.setRgb (red, green, blue, alpha);


  for (NV_INT32 i = 0 ; i < 2 ; i++)
    {
      QString tag = tr ("IR color %1/").arg (i);
      red = settings.value (tag + tr ("red"), IRColor[i].red ()).toInt ();
      green = settings.value (tag + tr ("green"), IRColor[i].green ()).toInt ();
      blue = settings.value (tag + tr ("blue"), IRColor[i].blue ()).toInt ();
      alpha = settings.value (tag + tr ("alpha"), IRColor[i].alpha ()).toInt ();
      IRColor[i].setRgb (red, green, blue, alpha);

      tag = tr ("Display IR %1").arg (i);
      settings.value (tag, displayIR[i]).toBool ();
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      QString tag = tr ("Shallow color %1/").arg (i);
      red = settings.value (tag + tr ("red"), shallowColor[i].red ()).toInt ();
      green = settings.value (tag + tr ("green"), shallowColor[i].green ()).toInt ();
      blue = settings.value (tag + tr ("blue"), shallowColor[i].blue ()).toInt ();
      alpha = settings.value (tag + tr ("alpha"), shallowColor[i].alpha ()).toInt ();
      shallowColor[i].setRgb (red, green, blue, alpha);

      tag = tr ("Display shallow %1").arg (i);
      settings.value (tag, displayShallow[i]).toBool ();
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      QString tag = tr ("Deep color %1/").arg (i);
      red = settings.value (tag + tr ("red"), deepColor[i].red ()).toInt ();
      green = settings.value (tag + tr ("green"), deepColor[i].green ()).toInt ();
      blue = settings.value (tag + tr ("blue"), deepColor[i].blue ()).toInt ();
      alpha = settings.value (tag + tr ("alpha"), deepColor[i].alpha ()).toInt ();
      deepColor[i].setRgb (red, green, blue, alpha);

      tag = tr ("Display deep %1").arg (i);
      settings.value (tag, displayDeep[i]).toBool ();
    }

  this->restoreState (settings.value (tr ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  settings.endGroup ();
}




//  Save the users defaults.

void
hawkeyeMonitor::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();

  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  QSettings settings (tr ("navo.navy.mil"), tr ("hawkeyeMonitor"));
  settings.beginGroup (tr ("hawkeyeMonitor"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("position form"), pos_format);

  settings.setValue (tr ("width"), width);

  settings.setValue (tr ("height"), height);

  settings.setValue (tr ("window x"), window_x);

  settings.setValue (tr ("window y"), window_y);


  for (NV_INT32 i = 0 ; i < 2 ; i++)
    {
      QString tag = tr ("Display IR %1").arg (i);
      settings.setValue (tag, displayIR[i]);
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      QString tag = tr ("Display shallow %1").arg (i);
      settings.setValue (tag, displayShallow[i]);
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      QString tag = tr ("Display deep %1").arg (i);
      settings.setValue (tag, displayDeep[i]);
    }

  settings.setValue (tr ("Background color/red"), backgroundColor.red ());
  settings.setValue (tr ("Background color/green"), backgroundColor.green ());
  settings.setValue (tr ("Background color/blue"), backgroundColor.blue ());
  settings.setValue (tr ("Background color/alpha"), backgroundColor.alpha ());


  for (NV_INT32 i = 0 ; i < 2 ; i++)
    {
      QString tag = tr ("IR color %1/").arg (i);
      settings.setValue (tag + tr ("red"), IRColor[i].red ());
      settings.setValue (tag + tr ("green"), IRColor[i].green ());
      settings.setValue (tag + tr ("blue"), IRColor[i].blue ());
      settings.setValue (tag + tr ("alpha"), IRColor[i].alpha ());

      tag = tr ("Display IR %1").arg (i);
      settings.setValue (tag, displayIR[i]);
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      QString tag = tr ("Shallow color %1/").arg (i);
      settings.setValue (tag + tr ("red"), shallowColor[i].red ());
      settings.setValue (tag + tr ("green"), shallowColor[i].green ());
      settings.setValue (tag + tr ("blue"), shallowColor[i].blue ());
      settings.setValue (tag + tr ("alpha"), shallowColor[i].alpha ());

      tag = tr ("Display shallow %1").arg (i);
      settings.setValue (tag, displayShallow[i]);
    }

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      QString tag = tr ("Deep color %1/").arg (i);
      settings.setValue (tag + tr ("red"), deepColor[i].red ());
      settings.setValue (tag + tr ("green"), deepColor[i].green ());
      settings.setValue (tag + tr ("blue"), deepColor[i].blue ());
      settings.setValue (tag + tr ("alpha"), deepColor[i].alpha ());

      tag = tr ("Display deep %1").arg (i);
      settings.setValue (tag, displayDeep[i]);
    }

  settings.setValue (tr ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}
