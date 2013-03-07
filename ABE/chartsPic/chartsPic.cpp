#include "chartsPic.hpp"
#include "chartsPicHelp.hpp"


static NV_FLOAT64 settings_version = 2.0;


chartsPic::chartsPic (NV_INT32 *argc, NV_CHAR **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;


  filError = NULL;
  stickpin_count = 0;
  active_window_id = getpid ();


  QResource::registerResource ("/icons.rcc");


  stickPin = new QPixmap (":/icons/stickpin.xpm");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Make the "marker" cursor painter path.

  marker = QPainterPath ();

  marker.moveTo (0, 0);
  marker.lineTo (30, 0);
  marker.lineTo (30, 20);
  marker.lineTo (0, 20);
  marker.lineTo (0, 0);

  marker.moveTo (0, 10);
  marker.lineTo (12, 10);

  marker.moveTo (30, 10);
  marker.lineTo (18, 10);

  marker.moveTo (15, 0);
  marker.lineTo (15, 6);

  marker.moveTo (15, 20);
  marker.lineTo (15, 14);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/charts_pic.png"));


  stickpinCursor = QCursor (QPixmap (":/icons/stickpin_cursor.xpm"), 9, 31);


  NV_INT32 option_index = 0;
  abe_share = NULL;
  kill_switch = ANCILLARY_FORCE_EXIT;

  while (NVTrue) 
    {
      static struct option long_options[] = {{"actionkey00", required_argument, 0, 0},
                                             {"actionkey01", required_argument, 0, 0},
                                             {"shared_memory_key", required_argument, 0, 0},
                                             {"kill_switch", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "o", long_options, &option_index);
      if (c == -1) break;


      QString skey;


      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 2:

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


              //  Get the ABE shared memory area.

              sscanf (optarg, "%d", &key);

              skey.sprintf ("%d_abe", key);

              abeShare = new QSharedMemory (skey);

              if (abeShare->attach (QSharedMemory::ReadWrite)) abe_share = (ABE_SHARE *) abeShare->data ();
              break;

            case 3:

              //  Get the kill switch.

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


  //  Make sure we got the shared memory (this program can only run as a shell from a parent).

  if (abe_share == NULL)
    {
      fprintf (stderr, "Unable to attach shared memory!\nTerminating!\n");
      exit (-1);
    }


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  QToolBar *tools = addToolBar (tr ("Tools"));
  tools->setObjectName (tr ("chartsPic main toolbar"));


  envin ();


  setWindowTitle (QString (VERSION));


  //  Set the window size and location from the defaults

  this->resize (width, height);
  this->move (window_x, window_y);


  //  Camera system biases at present (we only know systems 2 and 3 and they change ;-).

  x_fov[0] = 42.0 / NV_RAD_TO_DEG;
  x_fov[1] = 42.0 / NV_RAD_TO_DEG;
  x_fov[2] = 42.0 / NV_RAD_TO_DEG;
  y_fov[0] = 28.0 / NV_RAD_TO_DEG;
  y_fov[1] = 28.0 / NV_RAD_TO_DEG;
  y_fov[2] = 28.0 / NV_RAD_TO_DEG;
  roll_bias[0] = -0.2 / NV_RAD_TO_DEG;
  roll_bias[1] = -0.7 / NV_RAD_TO_DEG;
  roll_bias[2] = -0.2 / NV_RAD_TO_DEG;
  pitch_bias[0] = 11.3 / NV_RAD_TO_DEG;
  pitch_bias[1] = 10.72 / NV_RAD_TO_DEG;
  pitch_bias[2] = 11.3 / NV_RAD_TO_DEG;

  a0 = 6378137.0;
  b0 = 6356752.314245;


  //  Set the tracking timer to every 100 milliseconds.

  trackCursor = new QTimer (this);
  connect (trackCursor, SIGNAL (timeout ()), this, SLOT (slotTrackCursor ()));
  trackCursor->start (100);


  statusBar ()->setSizeGripEnabled (FALSE);
  statusBar ()->showMessage (VERSION);


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Make the pic.

  picdef.transMode = Qt::FastTransformation;
  pic = new nvPic (this, &picdef);
  pic->setWhatsThis (picText);


  strcpy (prev_file, "NEW");


  //  Connect to the signals from the pic class.

  connect (pic, SIGNAL (mousePressSignal (QMouseEvent *, NV_INT32, NV_INT32)), this, 
           SLOT (slotMousePress (QMouseEvent *, NV_INT32, NV_INT32)));
  connect (pic, SIGNAL (mouseReleaseSignal (QMouseEvent *, NV_INT32, NV_INT32)), this,
           SLOT (slotMouseRelease (QMouseEvent *, NV_INT32, NV_INT32)));
  connect (pic, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_INT32, NV_INT32)), this,
           SLOT (slotMouseMove (QMouseEvent *, NV_INT32, NV_INT32)));
  connect (pic, SIGNAL (postRedrawSignal (NVPIC_DEF)), this, SLOT (slotPostRedraw (NVPIC_DEF)));
  connect (pic, SIGNAL (keyPressSignal (QKeyEvent *)), this, SLOT (slotKeyPress (QKeyEvent *)));


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  vBox->addWidget (pic);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.xpm"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


  bSave = new QToolButton (this);
  bSave->setIcon (QIcon (":/icons/fileopen.xpm"));
  bSave->setToolTip (tr ("Save original size picture"));
  bSave->setWhatsThis (saveText);
  connect (bSave, SIGNAL (clicked ()), this, SLOT (slotSave ()));
  tools->addWidget (bSave);


  bScaled = new QToolButton (this);
  bScaled->setIcon (QIcon (":/icons/scaled.xpm"));
  bScaled->setToolTip (tr ("Save scaled picture"));
  bScaled->setWhatsThis (scaledText);
  connect (bScaled, SIGNAL (clicked ()), this, SLOT (slotScaled ()));
  tools->addWidget (bScaled);


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.xpm"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  tools->addWidget (bPrefs);


  bRotate = new QToolButton (this);
  bRotate->setIcon (QIcon (":/icons/rotate.xpm"));
  bRotate->setToolTip (tr ("Rotate picture to <b>north up</b>"));
  bRotate->setWhatsThis (rotateText);
  bRotate->setCheckable (TRUE);
  bRotate->setChecked (rotation);
  connect (bRotate, SIGNAL (clicked ()), this, SLOT (slotRotate ()));
  tools->addWidget (bRotate);


  tools->addSeparator ();
  tools->addSeparator ();


  QToolButton *bBrighten = new QToolButton (this);
  bBrighten->setIcon (QIcon (":/icons/brighten.xpm"));
  bBrighten->setToolTip (tr ("Brighten the picture"));
  bBrighten->setWhatsThis (brightenText);
  connect (bBrighten, SIGNAL (clicked ()), this, SLOT (slotBrighten ()));
  tools->addWidget (bBrighten);


  QToolButton *bDarken = new QToolButton (this);
  bDarken->setIcon (QIcon (":/icons/darken.xpm"));
  bDarken->setToolTip (tr ("Darken the picture"));
  bDarken->setWhatsThis (darkenText);
  connect (bDarken, SIGNAL (clicked ()), this, SLOT (slotDarken ()));
  tools->addWidget (bDarken);


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

  QAction *fileSaveAction = new QAction (tr ("&Save As"), this);
  fileSaveAction->setShortcut (tr ("Ctrl+S"));
  fileSaveAction->setStatusTip (tr ("Save file"));
  connect (fileSaveAction, SIGNAL (triggered ()), this, SLOT (slotSave ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (fileQuitAction);
  fileMenu->addSeparator ();
  fileMenu->addAction (fileSaveAction);


  //  Setup the help menu.

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about chartsPic"));
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


  pic->setCursor (stickpinCursor);

  pic->enableSignals ();
}



chartsPic::~chartsPic ()
{
}



void 
chartsPic::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



void 
chartsPic::leftMouse (QMouseEvent * e, NV_INT32 x __attribute__ ((unused)), NV_INT32 y __attribute__ ((unused)))
{
  stickpin[stickpin_count].x = e->x ();
  stickpin[stickpin_count].y = e->y ();

  stickpin_count++;

  pic->redrawPic ();
}



void 
chartsPic::midMouse (QMouseEvent * e __attribute__ ((unused)), NV_INT32 x __attribute__ ((unused)),
                     NV_INT32 y __attribute__ ((unused)))
{
  //  Placeholder
}



void 
chartsPic::rightMouse (QMouseEvent * e __attribute__ ((unused)), NV_INT32 x __attribute__ ((unused)),
                       NV_INT32 y __attribute__ ((unused)))
{
  //  Placeholder
}



//  Signal from the pic class.

void 
chartsPic::slotMousePress (QMouseEvent * e, NV_INT32 x, NV_INT32 y)
{
  if (e->button () == Qt::LeftButton) leftMouse (e, x, y);
  if (e->button () == Qt::MidButton) midMouse (e, x, y);
  if (e->button () == Qt::RightButton) rightMouse (e, x, y);
}



//  Signal from the pic class.

void 
chartsPic::slotMouseRelease (QMouseEvent * e, NV_INT32 x __attribute__ ((unused)), NV_INT32 y __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton);
  if (e->button () == Qt::MidButton);
  if (e->button () == Qt::RightButton);
}



//  Signal from the pic class.

void
chartsPic::slotMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_INT32 x, NV_INT32 y)
{
  //  Let other ABE programs know which window we're in.

  abe_share->active_window_id = active_window_id;


  NV_CHAR ltstring[25], lnstring[25];


  sprintf (ltstring, "%d", y);
  sprintf (lnstring, "%d", x);

  QString string;
  string.sprintf ("X: %s  Y: %s   pixel of the original image", lnstring, ltstring);


  statusBar ()->showMessage (string);


  pic->dragRubberbandRectangle (x, y);
}



//  Timer - timeout signal.  Very much like an X workproc.  This tracks the cursor in the associated
//  programs (pfmEdit and geoSwath) not in this window.

void
chartsPic::slotTrackCursor ()
{
  NV_INT32           type = 0, wlf = 0;
  NV_CHAR            ltstring[25], lnstring[25], img_file[512];
  NV_INT64           timestamp = 0;
  NV_BOOL            wlf_attitude = NVFalse;
  FILE               *cfp = NULL, *ifp, *pfp;
  WLF_HEADER         wlf_header;
  HOF_HEADER_T       hof_header;
  TOF_HEADER_T       tof_header;
  HYDRO_OUTPUT_T     hof_record;
  TOPO_OUTPUT_T      tof_record;
  WLF_RECORD         wlf_record;
  POS_OUTPUT_T       pos_record;

  static QString     string;
  static NV_BOOL     no_nav = NVTrue;
  static NV_INT32    prev_pic_x = -1, prev_pic_y = -1, system_num = 2, x_box_size, y_box_size;
  static NV_U_INT32  prev_rec = NV_U_INT32_MAX;
  static NV_BOOL     first = NVTrue, startup = NVTrue;
  static NV_FLOAT64  lat, lon, alt, new_lat, new_lon, dist, az, x_pix_size, y_pix_size, point_lat, point_lon;
  static NV_INT64    prev_time = -1;
  static NVPIC_DEF   pcdf;
  static NV_U_CHAR   *image = NULL;
  static NV_U_INT32  size = 0;


  //  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.
  //  We also want to exit on the ANCILLARY_FORCE_EXIT key (from pfmEdit) or if our own personal kill signal
  //  has been placeed in abe_share->key.

  if (abe_share->key == CHILD_PROCESS_FORCE_EXIT || abe_share->key == ANCILLARY_FORCE_EXIT ||
      abe_share->key == kill_switch) slotQuit ();


  //  If the cursor is in this window we don't want to try to track the cursor elsewhere.

  if (abe_share->active_window_id == active_window_id) return;


  //  Locking makes sure another process does not have memory locked.  It will block until it can lock it.
  //  At that point we copy the contents and then unlock it so other processes can continue.

  abeShare->lock ();


  //  Check for change of record and correct record type.

  NV_BOOL hit = NVFalse;
  if (prev_rec != abe_share->mwShare.multiRecord[0] && 
      (abe_share->mwShare.multiType[0] == PFM_SHOALS_TOF_DATA || abe_share->mwShare.multiType[0] == PFM_SHOALS_1K_DATA ||
       abe_share->mwShare.multiType[0] == PFM_WLF_DATA || abe_share->mwShare.multiType[0] == PFM_CHARTS_HOF_DATA))
    {
      l_share = *abe_share;
      prev_rec = l_share.mwShare.multiRecord[0];
      hit = NVTrue;
    }


  //  Toggle the picture rotation.

  if (abe_share->key == ac[0])
    {
      abe_share->key = 0;
      abe_share->modcode = NO_ACTION_REQUIRED;
      if (rotation)
        {
          rotation = NVFalse;
          strcpy (prev_file, "NEW");

          pic->RotatePixmap (0.0);
        }
      else
        {
          rotation = NVTrue;

          pic->RotatePixmap ((NV_FLOAT64) heading);
        }
      bRotate->setChecked (rotation);

      pic->redrawPic ();
    }


  //  Save snippet file

  if (abe_share->key == ac[1])
    {
      abe_share->key = 0;

      if (!strstr (abe_share->snippet_file_name, "NONE"))
        {
          abe_share->heading = heading;

          snippet_file_name.sprintf ("%s", abe_share->snippet_file_name);

          pic->SaveFile (snippet_file_name, NVTrue);


          string.sprintf ("Saved: %s", abe_share->snippet_file_name);

          statusBar ()->showMessage (string);
        }


      //  pfmEdit (actionKey.cpp) is looking for this modcode.

      abe_share->modcode = CHARTSPIC_SAVED_PICTURE;
    }


  abeShare->unlock ();


  //  Don't do anything unless we're HOF, TOF, or WLF and the record has changed.

  if (hit)
    {
      wlf_attitude = NVFalse;

      if (strstr (l_share.nearest_filename, ".hof"))
        {
          cfp = open_hof_file (l_share.nearest_filename);

          if (cfp == NULL)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                QString (strerror (errno));
              statusBar ()->showMessage (string);

              return;
            }
          type = 0;
        }
      else if (strstr (l_share.nearest_filename, ".tof"))
        {
          cfp = open_tof_file (l_share.nearest_filename);


          if (cfp == NULL)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                QString (strerror (errno));
              statusBar ()->showMessage (string);

              return;
            }
          type = 1;
        }
      else if (strstr (l_share.nearest_filename, ".wlf") || strstr (l_share.nearest_filename, ".wtf") ||
               strstr (l_share.nearest_filename, ".whf"))
        {
          wlf = wlf_open_file (l_share.nearest_filename, &wlf_header, WLF_READONLY);


          if (wlf < 0)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                QString (wlf_strerror ());
              statusBar ()->showMessage (string);

              return;
            }
          type = 2;

          wlf_attitude = (wlf_header.opt.sensor_position_present && wlf_header.opt.sensor_attitude_present);
        }


      switch (type)
        {
        case 0:
          hof_read_header (cfp, &hof_header);

          system_num = hof_header.text.ab_system_number - 1;

          hof_read_record (cfp, l_share.mwShare.multiRecord[0], &hof_record);


          timestamp = hof_record.timestamp;
          alt = hof_record.altitude;
          point_lat = hof_record.latitude;
          point_lon = hof_record.longitude;
          fclose (cfp);
          break;

        case 1:
          tof_read_header (cfp, &tof_header);

          system_num = tof_header.text.ab_system_number - 1;

          tof_read_record (cfp, l_share.mwShare.multiRecord[0], &tof_record);
          timestamp = tof_record.timestamp;
          point_lat = tof_record.latitude_last;
          point_lon = tof_record.longitude_last;
          fclose (cfp);
          break;

        case 2:
          sscanf (strstr (wlf_header.system, "System "), "%d", &system_num);
          system_num--;

          wlf_read_record (wlf, l_share.mwShare.multiRecord[0], &wlf_record, NVFalse, NULL);

          wlf_close_file (wlf);


          timestamp = wlf_record.tv_sec * 1000000 + wlf_record.tv_nsec / 1000000000;
          point_lat = wlf_record.y;
          point_lon = wlf_record.x;


          //  We need to fake out the Optech library so that it will find the POS or SBET file.  The names should be identical 
          //  with the exception of the extension.

          if (!wlf_attitude) strcpy (&l_share.nearest_filename[strlen (l_share.nearest_filename) - 4], ".hof");

          break;
        }


      if (first || strcmp (prev_file, l_share.nearest_filename) || timestamp / 1000000 != prev_time)
        {
          first = NVFalse;
          strcpy (prev_file, l_share.nearest_filename);

          strcpy (img_file, l_share.nearest_filename);
          strcpy (&img_file[strlen (img_file) - 4], ".img");

          ifp = open_image_file (img_file);
          if (ifp == NULL)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (img_file)) + " : " + 
                QString (strerror (errno));
              statusBar ()->showMessage (string);

              return;
            }


          image = image_read_record (ifp, timestamp, &size, &image_time);
          fclose (ifp);

          if (image == NULL)
            {
              string = tr ("Error reading image");
              statusBar ()->showMessage (string);

              return;
            }


          //  Load the picture.

          if (!pic->OpenData (image, size))
            {
              string = tr ("Error converting image");
              statusBar ()->showMessage (string);
              free (image);

              return;
            }

          free (image);


          //  Zero the stickpin count since we've changed pictures.

          stickpin_count = 0;


          pcdf = pic->getPicdef ();


          prev_time = timestamp / 1000000;


          //  If this is a WLF file and the sensor position and attitude are stored in the record, use them.  Otherwise
          //  try to find the associated POS or SBET file.


          if (wlf_attitude)
            {
              lat = wlf_record.sensor_y;
              lon = wlf_record.sensor_x;


              alt = wlf_record.sensor_z;


              //  Find the center of the image.

              heading = wlf_record.sensor_heading;

              dist = tan (wlf_record.sensor_pitch + pitch_bias[system_num]) * alt;

              newgp (lat, lon, heading, dist, &new_lat, &new_lon);


              //  Add 270 (same as -90).

              az = fmod (heading + 270.0, 360.0);

              dist = tan (wlf_record.sensor_roll + roll_bias[system_num]) * alt;

              newgp (new_lat, new_lon, az, dist, &lat, &lon);

              no_nav = NVFalse;
            }
          else
            {
              if (get_pos_file (l_share.nearest_filename, pos_file))
                {
                  if ((pfp = open_pos_file (pos_file)) != NULL)
                    {
                      pos_find_record (pfp, &pos_record, image_time);

                      lat = pos_record.latitude * NV_RAD_TO_DEG;
                      lon = pos_record.longitude * NV_RAD_TO_DEG;


                      //  Having to use height above ellipsoid for .tof and .wlf data.

                      if (type) alt = pos_record.altitude;

                      fclose (pfp);


                      //  Find the center of the image.

                      heading = (pos_record.platform_heading - pos_record.wander_angle) * NV_RAD_TO_DEG;

                      dist = tan (pos_record.pitch + pitch_bias[system_num]) * alt;

                      newgp (lat, lon, heading, dist, &new_lat, &new_lon);


                      //  Add 270 (same as -90).

                      az = fmod (heading + 270.0, 360.0);

                      dist = tan (pos_record.roll + roll_bias[system_num]) * alt;

                      newgp (new_lat, new_lon, az, dist, &lat, &lon);

                      no_nav = NVFalse;
                    }
                  else
                    {
                      no_nav = NVTrue;
                    }
                }
              else
                {
                  no_nav = NVTrue;
                }
            }


          if (!no_nav)
            {
              //  Save the center lat/lon in case we want to save the picture.

              center_lat = lat;
              center_lon = lon;


              //  This just sets the rotation.

              if (rotation)
                {
                  pic->RotatePixmap (heading);
                  pic->redrawPic ();
                }
              else
                {
                  pic->RotatePixmap (0.0);
                }


              //  Estimating pixel size.  This isn't quite right since the picture isn't
              //  taken straight down.  The difference per pixel in X can be as much as 
              //  ~2 cm at an elevation of 400 meters (37 meters difference between the
              //  width at the forward edge and directly under the plane).  I think we can 
              //  live with this for the time being since we only really want the box so
              //  we can see what the features are.

              y_pix_size = (2.0 * tan (y_fov[system_num] / 2.0) * alt) / (NV_FLOAT64) pcdf.height;
              x_pix_size = (2.0 * tan (x_fov[system_num] / 2.0) * alt) / (NV_FLOAT64) pcdf.width;

              x_box_size = NINT (3.0 / x_pix_size);
              y_box_size = NINT (3.0 / y_pix_size);


              prev_pic_x = -1;
              prev_pic_y = -1;
            }
        }
    }
  else
    {
      point_lat = abe_share->cursor_position.y;
      point_lon = abe_share->cursor_position.x;
    }


  //  Did we get a wlf sensor position or a pos or sbet file?

  if (no_nav)
    {
      heading = 0.0;
      pic->RotatePixmap (0.0);
      string = tr ("Cannot find nav file for this picture!");
    }
  else
    {
      //  Convert lat and lon into a cursor position in the picture.

      invgp (a0, b0, lat, lon, point_lat, point_lon, &dist, &az);

      NV_FLOAT64 diff = az - heading;

      if (diff < 0.0) diff += 360.0;


      //  Convert to radians

      diff /= NV_RAD_TO_DEG;

      NV_FLOAT64 x_dist = sin (diff) * dist;
      NV_FLOAT64 y_dist = cos (diff) * dist;


      NV_INT32 x = pcdf.width / 2 + NINT (x_dist / x_pix_size);
      NV_INT32 y = pcdf.height / 2 - NINT (y_dist / y_pix_size);


      if (prev_pic_x != x || prev_pic_y != y)
        {
          NV_FLOAT64 deg, min, sec;
          NV_CHAR    hem;

          strcpy (ltstring, fixpos (point_lat, &deg, &min, &sec, &hem, POS_LAT, pos_format));
          strcpy (lnstring, fixpos (point_lon, &deg, &min, &sec, &hem, POS_LON, pos_format));


          //  If we don't have a frozen cursor in pfmEdit...

          if (hit)
            {
              if (wlf_attitude)
                {
                  string.sprintf ("Lat: %s   Lon: %s   Alt: %.2f   Nav source: %s", ltstring, lnstring, alt, 
                                  gen_basename (l_share.nearest_filename));
                }
              else
                {
                  string.sprintf ("Lat: %s   Lon: %s   Alt: %.2f   Nav source: %s", ltstring, lnstring, alt, 
                                  gen_basename (pos_file));
                }
            }
          else
            {
              string.sprintf ("Lat: %s   Lon: %s   approximate cursor position from other ABE application", ltstring, lnstring);
            }


          QBrush b1;
          if (rotation)
            {
              pic->setMovingPath (marker, x, y, 2, Qt::red, b1, NVFalse, heading);
            }
          else
            {
              pic->setMovingPath (marker, x, y, 2, Qt::red, b1, NVFalse, 0.0);
            }

          prev_pic_x = x;
          prev_pic_y = y;
        }
    }
  statusBar ()->showMessage (string);


  //  Display the startup info message the first time through.

  if (startup)
    {
      QString key1;
      if (parentName == "geoSwath")
        {
          key1 = "\n\n";
        }
      else
        {
          key1 = QString (ac[1]) + tr (" = Save the picture \"as is\" to a snippet file\n\n");
        }

      QString startupMessageText = 
        tr ("The following action keys are available in ") + parentName + tr (":\n\n") +
        QString (ac[0]) + tr (" = Rotate the picture to \"north up\"\n") + 
        key1 +
        tr ("You can change these key values in the ") + parentName + "\n" +
        tr ("Preferences->Ancillary Programs window.\n\n\n") +
        tr ("You can turn off this startup message in the\n") + 
        tr ("chartsPic Preferences window.");

      if (startup_message) QMessageBox::information (this, tr ("chartsPic Startup Message"), startupMessageText);

      startup = NVFalse;
    }
}



//  Signal from the pic class.

void 
chartsPic::slotPostRedraw (NVPIC_DEF pic_picdef)
{
  picdef = pic_picdef;


  //  Draw the stickpin(s).

  NV_INT32 x, y;

  for (NV_INT32 i = 0 ; i < stickpin_count ; i++)
    {
      x = stickpin[i].x - 11;
      y = stickpin[i].y - 28;

      
      pic->pixmapPaint (x, y, stickPin, 0, 0, 32, 32);
    }


  pic->setCursor (stickpinCursor);


  //  Let the caller know that we're up and running.

  abeShare->lock ();

  if (abe_share->key == WAIT_FOR_START) abe_share->key = 0;

  abeShare->unlock ();
}



void 
chartsPic::slotKeyPress (QKeyEvent *e)
{
  NV_CHAR key[20];
  strcpy (key, e->text ().toAscii ());

  if (key[0] == (NV_CHAR) ac[0])
    {
      if (rotation)
        {
          rotation = NVFalse;
          strcpy (prev_file, "NEW");

          pic->RotatePixmap (0.0);
        }
      else
        {
          rotation = NVTrue;

          pic->RotatePixmap ((NV_FLOAT64) heading);
        }
      bRotate->setChecked (rotation);

      pic->redrawPic ();
    }

  // ac[1] only works from pfmEdit    if (key[0] == (NV_CHAR) ac[1]) 
}



void
chartsPic::redrawPic ()
{
  pic->redrawPic ();
}



//  A bunch of slots.

void 
chartsPic::slotQuit ()
{
  //  Let the parent program know that we have died from something other than the kill switch from the parent.

  if (abe_share->key != kill_switch) abe_share->killed = kill_switch;


  //  Make sure we haven't locked memory.

  abeShare->unlock ();

  envout ();


  //  Let go of the shared memory.

  abeShare->detach ();


  exit (0);
}



void 
chartsPic::slotBrighten ()
{
  pic->brighten (10);
}



void 
chartsPic::slotDarken ()
{
  pic->brighten (-10);
}



void 
chartsPic::picSave (NV_BOOL scaled)
{
  //  Convert the image time to something reasonable

  NV_INT32 year, jday, hour, minute;
  NV_FLOAT32 second;

  charts_cvtime (image_time, &year, &jday, &hour, &minute, &second);


  //  Convert the lat and lon to something reasonable

  NV_CHAR ltstring[50], lnstring[50];
  NV_FLOAT64 deg, min, sec;
  NV_CHAR    hem;

  strcpy (ltstring, fixpos (center_lat, &deg, &min, &sec, &hem, POS_LAT, pos_format));
  for (NV_INT32 i = 0 ; i < (NV_INT32) strlen (ltstring) ; i++) if (ltstring[i] == ' ') ltstring[i] = '_';
  strcpy (lnstring, fixpos (center_lon, &deg, &min, &sec, &hem, POS_LON, pos_format));
  for (NV_INT32 i = 0 ; i < (NV_INT32) strlen (lnstring) ; i++) if (lnstring[i] == ' ') lnstring[i] = '_';


  //  Set the default file name.

  QString name;
  name.sprintf ("P%04d%03d%02d%02d%05.2f__%s__%s.jpg", year + 1900, jday, hour, minute, second, ltstring, lnstring);


  QDir dir = QDir (save_directory);

  QFileDialog *fd = new QFileDialog (this, tr ("chartsPic Save As"));
  fd->setViewMode (QFileDialog::List);
  fd->setDirectory (dir);

  QStringList filters;
  filters << tr ("JPEG (*.jpg)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::AnyFile);
  fd->selectFilter (tr ("JPEG (*.jpg)"));
  fd->selectFile (name);

  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      save_file_name = files.at (0);

      if (!save_file_name.endsWith (".jpg")) save_file_name.append (".jpg");

      save_directory = fd->directory ().absolutePath ();

      pic->SaveFile (save_file_name, scaled);
    }
}



void 
chartsPic::slotSave ()
{
  picSave (NVFalse);
}



void 
chartsPic::slotScaled ()
{
  picSave (NVTrue);
}



void
chartsPic::slotPrefs ()
{
  prefsD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  prefsD->setWindowTitle (tr ("chartsPic Preferences"));
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


  QGroupBox *mBox = new QGroupBox (tr ("Display startup message"), this);
  QHBoxLayout *mBoxLayout = new QHBoxLayout;
  mBox->setLayout (mBoxLayout);
  sMessage = new QCheckBox (mBox);
  sMessage->setToolTip (tr ("Toggle display of startup message when program starts"));
  mBoxLayout->addWidget (sMessage);
  if (startup_message)
    {
      sMessage->setCheckState (Qt::Checked);
    }
  else
    {
      sMessage->setCheckState (Qt::Unchecked);
    }


  vbox->addWidget (mBox, 1);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (prefsD);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), prefsD);
  closeButton->setToolTip (tr ("Close the preferences dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClosePrefs ()));
  actions->addWidget (closeButton);

  prefsD->show ();
}


void
chartsPic::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
chartsPic::slotPosClicked (int id)
{
  pos_format = id;
}



void
chartsPic::slotClosePrefs ()
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
chartsPic::slotRotate ()
{
  if (bRotate->isChecked ())
    {
      rotation = NVTrue;

      pic->RotatePixmap ((NV_FLOAT64) heading);
    }
  else
    {
      rotation = NVFalse;
      strcpy (prev_file, "NEW");

      pic->RotatePixmap (0.0);
    }

  pic->redrawPic ();
}



void
chartsPic::about ()
{
  QMessageBox::about (this, VERSION,
                      tr ("chartsPic - CHARTS down-looking picture viewer.") + 
                      tr ("\n\nAuthor : Jan C. Depner (depnerj@navo.navy.mil)"));
}


void
chartsPic::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}


void
chartsPic::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}


//  Get the users defaults.

void
chartsPic::envin ()
{
  NV_FLOAT64 saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  pos_format = POS_HDMS;

  save_directory = ".";


  window_x = 0;
  window_y = 0;
  width = PIC_X_SIZE;
  height = PIC_Y_SIZE;
  rotation = NVFalse;
  startup_message = NVTrue;


  QSettings settings (tr ("navo.navy.mil"), tr ("chartsPic"));
  settings.beginGroup (tr ("chartsPic"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  pos_format = settings.value (tr ("position form"), pos_format).toInt ();

  rotation = settings.value (tr ("rotate image"), rotation).toBool ();

  save_directory = settings.value (tr ("save directory"), save_directory).toString ();

  width = settings.value (tr ("width"), width).toInt ();

  height = settings.value (tr ("height"), height).toInt ();

  window_x = settings.value (tr ("window x"), window_x).toInt ();

  window_y = settings.value (tr ("window y"), window_y).toInt ();


  startup_message = settings.value (tr ("Display Startup Message"), startup_message).toBool ();


  this->restoreState (settings.value (tr ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  settings.endGroup ();
}




//  Save the users defaults.

void
chartsPic::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();
  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  QSettings settings (tr ("navo.navy.mil"), tr ("chartsPic"));
  settings.beginGroup (tr ("chartsPic"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("position form"), pos_format);

  settings.setValue (tr ("rotate image"), rotation);

  settings.setValue (tr ("save directory"), save_directory);

  settings.setValue (tr ("width"), width);

  settings.setValue (tr ("height"), height);

  settings.setValue (tr ("window x"), window_x);

  settings.setValue (tr ("window y"), window_y);


  settings.setValue (tr ("Display Startup Message"), startup_message);


  settings.setValue (tr ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}
