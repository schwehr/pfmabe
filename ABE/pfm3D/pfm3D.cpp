//  pfm3D class.

/***************************************************************************\

    Module :        pfm3D

    Programmer :    Jan C. Depner

    Date :          09/10/08

    Purpose :       Simple 3D PFM bin viewer.

    Caveats :       This application only runs from within pfmView.  That
                    is, it doesn't run standalone.

\***************************************************************************/

#include "pfm3D.hpp"
#include "pfm3DHelp.hpp"


void overlayFlag (nvMapGL *map, OPTIONS *options, MISC *misc);


pfm3D::pfm3D (int *argc, char **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;


  NV_BOOL envin (OPTIONS *options, QMainWindow *mainWindow);
  void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore);


  got_geotiff = NVFalse;
  NV_INT32 option_index = 0;
  NV_INT32 shmid = 0;
  popup_active = NVFalse;
  prefs_dialog = NULL;
  browserProc = NULL;


  strcpy (misc.GeoTIFF_name, " ");

  for (NV_INT32 layer = 0 ; layer < MAX_ABE_PFMS ; layer++) misc.pfm_handle[layer] = -1;


  while (NVTrue) 
    {
      static struct option long_options[] = {{"shared_memory_key", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "G:", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              sscanf (optarg, "%d", &shmid);
              break;
            }
          break;

        case 'G':
          strcpy (misc.GeoTIFF_name, optarg);
          got_geotiff = NVTrue;
          break;
        }
    }


  //  We must have the shared memory ID.

  if (!shmid)
    {
      QMessageBox::critical (this, tr ("pfm3D"), tr ("pfm3D can only be run from pfmView using shared memory."), QMessageBox::Close);
      exit (-1);
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
  //  have already been created by pfmView.  The key is the process ID of the parent
  //  plus "_abe".  The parent process ID is a required command line argument
  //  (--shared_memory_id).

  QString skey;
  skey.sprintf ("%d_abe", shmid);


  misc.abeShare = new QSharedMemory (skey);

  if (misc.abeShare->attach (QSharedMemory::ReadWrite)) misc.abe_share = (ABE_SHARE *) misc.abeShare->data ();



  options.position_form = misc.abe_share->position_form;
  options.z_factor = misc.abe_share->z_factor;
  options.z_offset = misc.abe_share->z_offset;


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set a few defaults for startup

  pfm_edit_active = NVFalse;
  pfmEdit3D_opened = NVFalse;
  cleared = NVFalse;
  delayClear = NVFalse;
  pfmEditMod = 0;
  mv_marker = -1;
  mv_tracker = -1;
  rb_rectangle = -1;
  rb_polygon = -1;
  active_window_id = getpid ();
  start_ctrl_x = -1;
  start_ctrl_y = -1;
  zoom_accum = 0;
  zoom_wait = NVFalse;
  polygon_flip = NVFalse;
  double_click = NVFalse;


  //  Set up the marker data list

  marker[0].x = -MARKER_W;
  marker[0].y = -MARKER_H;
  marker[1].x = MARKER_W;
  marker[1].y = -MARKER_H;
  marker[2].x = MARKER_W;
  marker[2].y = -MARKER_H;
  marker[3].x = MARKER_W;
  marker[3].y = MARKER_H;
  marker[4].x = MARKER_W;
  marker[4].y = MARKER_H;
  marker[5].x = -MARKER_W;
  marker[5].y = MARKER_H;
  marker[6].x = -MARKER_W;
  marker[6].y = MARKER_H;
  marker[7].x = -MARKER_W;
  marker[7].y = -MARKER_H;
  marker[8].x = -MARKER_W;
  marker[8].y = 0;
  marker[9].x = -(MARKER_W / 5);
  marker[9].y = 0;
  marker[10].x = (MARKER_W / 5);
  marker[10].y = 0;
  marker[11].x = MARKER_W;
  marker[11].y = 0;
  marker[12].x = 0;
  marker[12].y = -MARKER_H;
  marker[13].x = 0;
  marker[13].y = -(MARKER_W / 5);
  marker[14].x = 0;
  marker[14].y = (MARKER_W / 5);
  marker[15].x = 0;
  marker[15].y = MARKER_H;


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfm3D.xpm"));


  editRectCursor = QCursor (QPixmap (":/icons/edit_rect_cursor"), 1, 1);
  editPolyCursor = QCursor (QPixmap (":/icons/edit_poly_cursor"), 1, 1);
  rotateCursor = QCursor (QPixmap (":/icons/rotate_cursor.xpm"), 17, 17);
  zoomCursor = QCursor (QPixmap (":/icons/zoom_cursor.xpm"), 11, 11);


  this->setWindowTitle (QString (VERSION));


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  QToolBar *tools = addToolBar (tr ("Tools"));
  tools->setObjectName (tr ("pfm3D main toolbar"));


  //  Set all of the defaults

  set_defaults (&misc, &options, NVFalse);


  //  Get the user's defaults if available

  if (!envin (&options, this))
    {
      //  Set the geometry from defaults since we didn't get any from the saved settings.

      this->resize (misc.width, misc.height);
      this->move (misc.window_x, misc.window_y);
    }


  statusBar ()->setSizeGripEnabled (FALSE);
  statusBar ()->showMessage (VERSION);


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Set the map values from the defaults

  mapdef.draw_width = misc.width;
  mapdef.draw_height = misc.height;
  mapdef.zoom_percent = options.zoom_percent;
  mapdef.exaggeration = options.exaggeration;
  mapdef.min_z_extents = 1.0;
  mapdef.mode = NVMAPGL_BIN_MODE;
  mapdef.light_model = GL_LIGHT_MODEL_TWO_SIDE;
  mapdef.auto_scale = NVFalse;
  mapdef.projected = 0;

  mapdef.background_color = options.background_color;
  mapdef.scale_color = options.scale_color;
  mapdef.draw_scale = options.draw_scale;
  mapdef.initial_zx_rotation = 0.0;
  mapdef.initial_y_rotation = 0.0;


  //  Make the map.

  map = new nvMapGL (this, &mapdef, "pfm3D");
  map->setMinimumWidth (900);
  map->setWhatsThis (mapText);


  //  Connect to the signals from the map class.

  connect (map, SIGNAL (mousePressSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotMousePress (QMouseEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (mouseDoubleClickSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64)), this,
           SLOT (slotMouseDoubleClick (QMouseEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (mouseReleaseSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotMouseRelease (QMouseEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64, NVMAPGL_DEF)), this, 
           SLOT (slotMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64, NVMAPGL_DEF)));
  connect (map, SIGNAL (wheelSignal (QWheelEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotWheel (QWheelEvent *, NV_FLOAT64, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (map, SIGNAL (exaggerationChanged (NV_FLOAT32, NV_FLOAT32)), this, SLOT (slotExaggerationChanged (NV_FLOAT32, NV_FLOAT32)));


  exagBar = new QScrollBar (Qt::Vertical);
  exagBar->setTracking (TRUE);
  exagBar->setInvertedAppearance (TRUE);
  exagBar->setToolTip (tr ("Vertical exaggeration"));
  exagBar->setWhatsThis (exagBarText);
  exagPalette.setColor (QPalette::Normal, QPalette::Base, options.background_color);
  exagPalette.setColor (QPalette::Normal, QPalette::Window, options.background_color);
  exagPalette.setColor (QPalette::Inactive, QPalette::Base, options.background_color);
  exagPalette.setColor (QPalette::Inactive, QPalette::Window, options.background_color);
  exagBar->setPalette (exagPalette);
  exagBar->setMinimum (100);
  exagBar->setMaximum (10000);
  exagBar->setSingleStep (100);
  exagBar->setPageStep (500);
  exagBar->setValue (NINT (options.exaggeration * 100.0));
  connect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
  connect (exagBar, SIGNAL (sliderReleased ()), this, SLOT (slotExagReleased ()));


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (exagBar);
  hBox->addWidget (map, 1);
  vBox->addLayout (hBox, 1);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.xpm"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


  tools->addSeparator ();
  tools->addSeparator ();


  bGeoOpen = new QToolButton (this);
  bGeoOpen->setIcon (QIcon (":/icons/geotiffopen.xpm"));
  bGeoOpen->setToolTip (tr ("Open a GeoTIFF file"));
  bGeoOpen->setWhatsThis (geoOpenText);
  connect (bGeoOpen, SIGNAL (clicked ()), this, SLOT (slotOpenGeotiff ()));
  tools->addWidget (bGeoOpen);


  tools->addSeparator ();
  tools->addSeparator ();


  bReset = new QToolButton (this);
  bReset->setIcon (QIcon (":/icons/reset_view.xpm"));
  bReset->setToolTip (tr ("Reset view"));
  bReset->setWhatsThis (resetText);
  connect (bReset, SIGNAL (clicked ()), this, SLOT (slotReset ()));
  tools->addWidget (bReset);


  tools->addSeparator ();
  tools->addSeparator ();


  QButtonGroup *editModeGrp = new QButtonGroup (this);
  connect (editModeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotEditMode (int)));
  editModeGrp->setExclusive (TRUE);


  bEditRect = new QToolButton (this);
  bEditRect->setIcon (QIcon (":/icons/edit_rect.xpm"));
  bEditRect->setToolTip (tr ("Set rectangle edit mode"));
  bEditRect->setWhatsThis (editRectText);
  editModeGrp->addButton (bEditRect, RECT_EDIT_AREA);
  bEditRect->setCheckable (TRUE);
  tools->addWidget (bEditRect);

  bEditPoly = new QToolButton (this);
  bEditPoly->setIcon (QIcon (":/icons/edit_poly.xpm"));
  bEditPoly->setToolTip (tr ("Set polygon edit mode"));
  bEditPoly->setWhatsThis (editPolyText);
  editModeGrp->addButton (bEditPoly, POLY_EDIT_AREA);
  bEditPoly->setCheckable (TRUE);
  tools->addWidget (bEditPoly);


  tools->addSeparator ();
  tools->addSeparator ();


  bEditRect3D = new QToolButton (this);
  bEditRect3D->setIcon (QIcon (":/icons/edit_rect3D.xpm"));
  bEditRect3D->setToolTip (tr ("Set 3D rectangle edit mode"));
  bEditRect3D->setWhatsThis (editRectText);
  editModeGrp->addButton (bEditRect3D, RECT_EDIT_AREA_3D);
  bEditRect3D->setCheckable (TRUE);
  tools->addWidget (bEditRect3D);

  bEditPoly3D = new QToolButton (this);
  bEditPoly3D->setIcon (QIcon (":/icons/edit_poly3D.xpm"));
  bEditPoly3D->setToolTip (tr ("Set 3D polygon edit mode"));
  bEditPoly3D->setWhatsThis (editPolyText);
  editModeGrp->addButton (bEditPoly3D, POLY_EDIT_AREA_3D);
  bEditPoly3D->setCheckable (TRUE);
  tools->addWidget (bEditPoly3D);


  tools->addSeparator ();
  tools->addSeparator ();


  QMenu *featureMenu = new QMenu (this);

  QActionGroup *featureGrp = new QActionGroup (this);

  displayFeatureIcon[0] = QIcon (":/icons/display_no_feature.png");
  displayFeatureIcon[1] = QIcon (":/icons/display_all_feature.png");
  displayFeatureIcon[2] = QIcon (":/icons/display_unverified_feature.png");
  displayFeatureIcon[3] = QIcon (":/icons/display_verified_feature.png");
  displayFeature[0] = featureMenu->addAction (tr ("Don't display features"));
  displayFeature[1] = featureMenu->addAction (tr ("Display all features"));
  displayFeature[2] = featureMenu->addAction (tr ("Display unverified features"));
  displayFeature[3] = featureMenu->addAction (tr ("Display verified features"));
  for (NV_INT32 i = 0 ; i < 4 ; i++)
    {
      displayFeature[i]->setIcon (displayFeatureIcon[i]);
      featureGrp->addAction (displayFeature[i]);
    }

  bDisplayFeature = new QToolButton (this);
  bDisplayFeature->setToolTip (tr ("Select type of feature data to display"));
  bDisplayFeature->setWhatsThis (displayFeatureText);
  bDisplayFeature->setPopupMode (QToolButton::InstantPopup);
  bDisplayFeature->setMenu (featureMenu);
  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);
  connect (featureGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFeatureMenu (QAction *)));
  tools->addWidget (bDisplayFeature);


  bDisplayChildren = new QToolButton (this);
  bDisplayChildren->setIcon (QIcon (":/icons/displayfeaturechildren.xpm"));
  bDisplayChildren->setToolTip (tr ("Display feature group members"));
  bDisplayChildren->setWhatsThis (displayChildrenText);
  bDisplayChildren->setCheckable (TRUE);
  bDisplayChildren->setChecked (options.display_children);
  connect (bDisplayChildren, SIGNAL (clicked ()), this, SLOT (slotDisplayChildren ()));
  tools->addWidget (bDisplayChildren);

  bDisplayFeatureInfo = new QToolButton (this);
  bDisplayFeatureInfo->setIcon (QIcon (":/icons/displayfeatureinfo.xpm"));
  bDisplayFeatureInfo->setToolTip (tr ("Display feature information"));
  bDisplayFeatureInfo->setWhatsThis (displayFeatureInfoText);
  bDisplayFeatureInfo->setCheckable (TRUE);
  bDisplayFeatureInfo->setChecked (options.display_feature_info);
  connect (bDisplayFeatureInfo, SIGNAL (clicked ()), this, SLOT (slotDisplayFeatureInfo ()));
  tools->addWidget (bDisplayFeatureInfo);


  tools->addSeparator ();
  tools->addSeparator ();


  bGeotiff = new QToolButton (this);
  bGeotiff->setIcon (QIcon (":/icons/geo_off.xpm"));
  bGeotiff->setToolTip (tr ("Toggle GeoTIFF display"));
  bGeotiff->setWhatsThis (geoText);
  bGeotiff->setShortcut (options.buttonAccel[9]);
  connect (bGeotiff, SIGNAL (clicked ()), this, SLOT (slotGeotiff ()));
  tools->addWidget (bGeotiff);


  tools->addSeparator ();
  tools->addSeparator ();


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


  //  Set the edit function from the defaults

  misc.function = options.edit_mode;
  misc.save_function = misc.function;


  //  Right click popup menu

  popupMenu = new QMenu (map);

  popup0 = popupMenu->addAction (tr (""));
  connect (popup0, SIGNAL (triggered ()), this, SLOT (slotPopupMenu0 ()));
  popup1 = popupMenu->addAction (tr (""));
  connect (popup1, SIGNAL (triggered ()), this, SLOT (slotPopupMenu1 ()));
  popup2 = popupMenu->addAction (tr (""));
  connect (popup2, SIGNAL (triggered ()), this, SLOT (slotPopupMenu2 ()));
  popupMenu->addSeparator ();
  popup3 = popupMenu->addAction (tr ("Help"));
  connect (popup3, SIGNAL (triggered ()), this, SLOT (slotPopupHelp ()));
  popupMenu->show ();


  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), qApp, SLOT (closeAllWindows ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (fileQuitAction);


  //  Setup the help menu.

  QAction *exHelp = new QAction (tr ("&Extended Help"), this);
  exHelp->setShortcut (tr ("Ctrl+E"));
  exHelp->setStatusTip (tr ("Extended information about pfmView"));
  connect (exHelp, SIGNAL (triggered ()), this, SLOT (extendedHelp ()));

  QAction *whatsThisAct = QWhatsThis::createAction (this);

  QAction *hotHelp = new QAction (tr ("&GUI control"), this);
  hotHelp->setShortcut (tr ("Ctrl+G"));
  hotHelp->setStatusTip (tr ("Help on GUI control"));
  connect (hotHelp, SIGNAL (triggered ()), this, SLOT (slotHotkeyHelp ()));

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about pfm3D"));
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
  helpMenu->addAction (exHelp);
  helpMenu->addSeparator ();
  helpMenu->addAction (whatsThisAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (hotHelp);
  helpMenu->addSeparator ();
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgements);
  helpMenu->addAction (aboutQtAct);


  //  Try to open the feature file and read the features into memory.

  if (strcmp (misc.abe_share->open_args[0].target_path, "NONE"))
    {
      if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);
      misc.bfd_open = NVFalse;

      if ((misc.bfd_handle = binaryFeatureData_open_file (misc.abe_share->open_args[0].target_path, &misc.bfd_header, BFDATA_UPDATE)) >= 0)
        {
          if (binaryFeatureData_read_all_short_features (misc.bfd_handle, &misc.feature) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (this, tr ("pfm3D"), tr ("Unable to read feature records\nReason: ") + msg);
              binaryFeatureData_close_file (misc.bfd_handle);
            }
          else
            {
              misc.bfd_open = NVTrue;
            }
        }
    }


  //  Set the tracking timer to every 50 milliseconds.

  trackCursor = new QTimer (this);
  connect (trackCursor, SIGNAL (timeout ()), this, SLOT (slotTrackCursor ()));
  trackCursor->start (50);


  //  Set the scroll wheel zoom timer to every 250 milliseconds.

  zoomTimer = new QTimer (this);
  connect (zoomTimer, SIGNAL (timeout ()), this, SLOT (slotZoomTimer ()));
  zoomTimer->start (250);


  misc.GeoTIFF_open = NVFalse;
  misc.display_GeoTIFF = NVFalse;


  //  Set the GUI buttons and the cursor.

  setFunctionCursor (misc.function);
  setMainButtons (NVFalse);

  show ();
}



pfm3D::~pfm3D ()
{
}



void 
pfm3D::setMainButtons (NV_BOOL enabled)
{
  bGeoOpen->setEnabled (enabled);
  bReset->setEnabled (enabled);
  bEditRect->setEnabled (enabled);
  bEditPoly->setEnabled (enabled);
  bEditRect3D->setEnabled (enabled);
  bEditPoly3D->setEnabled (enabled);
  bPrefs->setEnabled (enabled);
  bDisplayFeature->setEnabled (enabled);


  //  No point in having a feature info button if we're not displaying features

  if (enabled && options.display_feature)
    {
      bDisplayChildren->setEnabled (enabled);
      bDisplayFeatureInfo->setEnabled (enabled);
    }
  else
    {
      bDisplayChildren->setEnabled (FALSE);
      bDisplayFeatureInfo->setEnabled (FALSE);
    }


  //  Only enable the GeoTIFF display button if we have opened a GeoTIFF file

  if (enabled && misc.GeoTIFF_open)
    {
      bGeotiff->setEnabled (enabled);
    }
  else
    {
      bGeotiff->setEnabled (FALSE);
    }


  switch (misc.function)
    {
    case RECT_EDIT_AREA:
      bEditRect->setChecked (TRUE);
      break;

    case POLY_EDIT_AREA:
      bEditPoly->setChecked (TRUE);
      break;

    case RECT_EDIT_AREA_3D:
      bEditRect3D->setChecked (TRUE);
      break;

    case POLY_EDIT_AREA_3D:
      bEditPoly3D->setChecked (TRUE);
      break;
    }
}



void 
pfm3D::initializeMaps (NV_BOOL reset)
{
  void paint_surface (pfm3D *parent, MISC *misc, nvMapGL *map, NV_BOOL reset);
  void paint_otf_surface (pfm3D *parent, MISC *misc, nvMapGL *map, NV_BOOL reset);


  static NV_BOOL first = NVTrue;


  setMainButtons (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  misc.drawing = NVTrue;


  map->enableSignals ();


  //  Clear out any overlays.

  map->resetMap ();


  //  Build and display the 3D surface.

  if (misc.abe_share->otf_width)
    {
      paint_otf_surface (this, &misc, map, reset);
    }
  else
    {
      paint_surface (this, &misc, map, reset);
    }


  //  Overlay the features if requested.

  overlayFlag (map, &options, &misc);


  //  Let the parent that shelled us know that we're up and running.

  if (misc.abe_share->key == WAIT_FOR_START) misc.abe_share->key = 0;


  misc.drawing = NVFalse;


  setMainButtons (NVTrue);


  if (first)
    {
      map->rotateZX (30.0);
      map->flush ();
      first = NVFalse;
    }


  qApp->restoreOverrideCursor ();
}



void 
pfm3D::slotOpenGeotiff ()
{
  static QDir dir;
  QStringList files, filters;
  QString file;

  QFileDialog *fd = new QFileDialog (this, tr ("pfm3D Open GeoTIFF"));
  fd->setViewMode (QFileDialog::List);


  QString mosaic = "";
  if (!misc.GeoTIFF_open)
    {
      for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
        {
          if (strcmp (misc.abe_share->open_args[pfm].image_path, "NONE"))
            {
              mosaic = QString (misc.abe_share->open_args[pfm].image_path);
              dir = QFileInfo (mosaic).dir ();
              break;
            }
        }
    }


  fd->setDirectory (dir);
  filters << tr ("GeoTIFF (*.tif)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("GeoTIFF (*.tif)"));
  if (!mosaic.isEmpty ()) fd->selectFile (mosaic);

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      if (!file.isEmpty())
        {
          //  Check the file to make sure it is in the area

          GDALDataset        *poDataset;
          NV_FLOAT64         adfGeoTransform[6];
          NV_FLOAT64         GeoTIFF_wlon, GeoTIFF_nlat, GeoTIFF_lon_step, 
                             GeoTIFF_lat_step, GeoTIFF_elon, GeoTIFF_slat;
          NV_INT32           width, height;


          GDALAllRegister ();


          NV_CHAR path[512];
          strcpy (path, file.toAscii ());

          poDataset = (GDALDataset *) GDALOpen (path, GA_ReadOnly);
          if (poDataset != NULL)
            {
              if (poDataset->GetProjectionRef ()  != NULL)
                {
                  QString projRef = QString (poDataset->GetProjectionRef ());

                  if (projRef.contains ("GEOGCS"))
                    {
                      if (poDataset->GetGeoTransform (adfGeoTransform) == CE_None)
                        {
                          GeoTIFF_lon_step = adfGeoTransform[1];
                          GeoTIFF_lat_step = -adfGeoTransform[5];


                          width = poDataset->GetRasterXSize ();
                          height = poDataset->GetRasterYSize ();


                          GeoTIFF_wlon = adfGeoTransform[0];
                          GeoTIFF_nlat = adfGeoTransform[3];


                          GeoTIFF_slat = GeoTIFF_nlat - height * GeoTIFF_lat_step;
                          GeoTIFF_elon = GeoTIFF_wlon + width * GeoTIFF_lon_step;


                          if (GeoTIFF_nlat < misc.abe_share->viewer_displayed_area.min_y || 
                              GeoTIFF_slat > misc.abe_share->viewer_displayed_area.max_y ||
                              GeoTIFF_elon < misc.abe_share->viewer_displayed_area.min_x || 
                              GeoTIFF_wlon > misc.abe_share->viewer_displayed_area.max_x)
                            {
                              QMessageBox::warning (this, tr ("Open GeoTIFF"), 
                                                    tr ("No part of this GeoTIFF file falls within the displayed area."));
                              return;
                            }
                        }
                      else
                        {
                          QMessageBox::warning (this, tr ("Open GeoTIFF"), 
                                                tr ("This program only handles unprojected GeoTIFF files."));
                          return;
                        }
                    }
                  else
                    {
                      QMessageBox::warning (this, tr ("Open GeoTIFF"), 
                                            tr ("This program only handles geographic GeoTIFF files."));
                      return;
                    }
                }
              else
                {
                  QMessageBox::warning (this, tr ("Open GeoTIFF"), 
                                        tr ("This file has no datum/projection information."));
                  return;
                }
            }
          else
            {
              QMessageBox::warning (this, tr ("Open GeoTIFF"), 
                                    tr ("Unable to open file."));
              return;
            }

          delete poDataset;

          misc.GeoTIFF_open = NVTrue;
          bGeotiff->setEnabled (TRUE);

          strcpy (misc.GeoTIFF_name, file.toAscii ());
          misc.GeoTIFF_init = NVTrue;

          bGeotiff->setEnabled (TRUE);
          bGeotiff->setIcon (QIcon (":/icons/geo_decal.xpm"));
          misc.display_GeoTIFF = 1;

          initializeMaps (NVFalse);

          dir = fd->directory ().path ();
        }
    }
}



void 
pfm3D::slotPrefs ()
{
  if (prefs_dialog) prefs_dialog->close ();
  prefs_dialog = new Prefs (this, &options, &misc);

  connect (prefs_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotPrefDataChanged ()));
}



//  Changed some of the preferences

void 
pfm3D::slotPrefDataChanged ()
{
  bDisplayFeature->setChecked (options.display_feature);
  bDisplayChildren->setChecked (options.display_children);
  bDisplayFeatureInfo->setChecked (options.display_feature_info);

  map->setZoomPercent (options.zoom_percent);
  
  map->setBackgroundColor (options.background_color);
  map->setScaleColor (options.scale_color);
  map->enableScale (options.draw_scale);

  initializeMaps (NVFalse);
}



void 
pfm3D::discardMovableObjects ()
{
  map->closeMovingList (&mv_marker);
  map->closeMovingList (&mv_tracker);
}



void 
pfm3D::leftMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon __attribute__ ((unused)),
                  NV_FLOAT64 lat __attribute__ ((unused)), NV_FLOAT64 z __attribute__ ((unused)))
{
  //  If the popup menu is up discard this mouse press

  if (popup_active)
    {
      popup_active = NVFalse;
      return;
    }


  QString file, string;
  static QDir dir = QDir (".");
  NV_INT32 *px, *py;


  //  Actions based on the active function

  switch (misc.function)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, mouse_x, mouse_y, &px, &py);


          NV_FLOAT64 mz;


          //  Lock it down and tell pfmView to edit.

          misc.abeShare->lock ();
          {
            for (NV_INT32 i = 0 ; i < 4 ; i++)
              {
                map->get3DCoords (px[i], py[i], &misc.abe_share->polygon_x[i], &misc.abe_share->polygon_y[i], &mz);
              }

            misc.abe_share->polygon_count = 4;

            if (misc.function == RECT_EDIT_AREA_3D)
              {
                delayClear = NVTrue;

                misc.abe_share->key = PFMVIEW_FORCE_EDIT_3D;
              }
            else
              {
                misc.abe_share->key = PFMVIEW_FORCE_EDIT;
              }
          }
          misc.abeShare->unlock ();


          map->discardRubberbandRectangle (&rb_rectangle);
          setFunctionCursor (misc.function);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, mouse_x, mouse_y, options.edit_color, 2, Qt::SolidLine);
        }
      break;

    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          map->vertexRubberbandPolygon (rb_polygon, mouse_x, mouse_y);
        }
      else
        {
          map->anchorRubberbandPolygon (&rb_polygon, mouse_x, mouse_y, options.edit_color, LINE_WIDTH, NVTrue, Qt::SolidLine);
        }
      break;
    }
}



void 
pfm3D::midMouse (NV_INT32 mouse_x __attribute__ ((unused)), NV_INT32 mouse_y __attribute__ ((unused)),
                 NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)),
                 NV_FLOAT64 z __attribute__ ((unused)))
{
  //  Actions based on the active function

  switch (misc.function)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      map->discardRubberbandRectangle (&rb_rectangle);
      break;

    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      map->discardRubberbandPolygon (&rb_polygon);
      break;
    }
}



void 
pfm3D::slotMouseDoubleClick (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon __attribute__ ((unused)),
                             NV_FLOAT64 lat __attribute__ ((unused)), NV_FLOAT64 z __attribute__ ((unused)))
{
  //  Flip the double_click flag.  The right-click menu sets this to NVTrue so it will flip to NVFalse.
  //  Left-click sets it to NVFalse so it will filp to NVTrue;

  double_click = !double_click;


  //  Actions based on the active function

  switch (misc.function)
    {
    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      NV_INT32 count;
      NV_INT32 *px, *py, mouse_x, mouse_y;


      //  If we double clicked we already added the point so just get what we've already got.

      if (double_click)
        {
          map->getRubberbandPolygon (rb_polygon, &count, &px, &py);
          count--;
        }


      //  If we used the menu we need to store the last point.

      else
        {
          mouse_x = menu_cursor_x;
          mouse_y = menu_cursor_y;

          map->closeRubberbandPolygon (rb_polygon, mouse_x, mouse_y, &count, &px, &py);
        }


      NV_FLOAT64 mz;


      //  Lock it down and tell pfmView to edit.

      misc.abeShare->lock ();
      {
        for (NV_INT32 i = 0 ; i < count ; i++)
          {
            map->get3DCoords (px[i], py[i], &misc.abe_share->polygon_x[i], &misc.abe_share->polygon_y[i], &mz);
          }

        misc.abe_share->polygon_count = count;

        if (misc.function == POLY_EDIT_AREA_3D)
          {
            delayClear = NVTrue;

            misc.abe_share->key = PFMVIEW_FORCE_EDIT_3D;
          }
        else
          {
            misc.abe_share->key = PFMVIEW_FORCE_EDIT;
          }
      }
      misc.abeShare->unlock ();


      map->discardRubberbandPolygon (&rb_polygon);
      setFunctionCursor (misc.function);
      break;

    default:
      break;
    }

  double_click = NVFalse;
}



void 
pfm3D::rightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat,
                   NV_FLOAT64 z __attribute__ ((unused)))
{
  QString tmp;

  menu_cursor_lon = lon;
  menu_cursor_lat = lat;
  menu_cursor_x = mouse_x;
  menu_cursor_y = mouse_y;


  QPoint pos (mouse_x, mouse_y);


  //  Popups need global positioning

  QPoint global_pos = map->mapToGlobal (pos);

  popup0->setVisible (TRUE);
  popup1->setVisible (TRUE);
  popup2->setVisible (TRUE);

  switch (misc.function)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      popup0->setText (tr ("Close rectangle and edit area"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      popup0->setText (tr ("Close polygon and edit area"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case ROTATE:
      popup0->setVisible (FALSE);
      popup1->setVisible (FALSE);
      popup2->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case ZOOM:
      start_ctrl_y = mouse_y;
      break;
    }
}



void 
pfm3D::slotPopupMenu0 ()
{
  popup_active = NVFalse;

  switch (misc.function)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      leftMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat, menu_cursor_z);
      break;

    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:

      //  Fake out slotMouseDoubleClick so that it won't decrement the point count.

      double_click = NVTrue;

      slotMouseDoubleClick (NULL, menu_cursor_lon, menu_cursor_lat, menu_cursor_z);
      break;
    }
}



void 
pfm3D::slotPopupMenu1 ()
{
  popup_active = NVFalse;
}



void 
pfm3D::slotPopupMenu2 ()
{
  popup_active = NVFalse;
}



void 
pfm3D::slotPopupHelp ()
{
  switch (misc.function)
    {
    case ROTATE:
    case ZOOM:
      QWhatsThis::showText (QCursor::pos ( ), mapText, map);
      break;
    }
  midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat, menu_cursor_z);
}



//  Signal from the map class.

void 
pfm3D::slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z)
{
  if (e->button () == Qt::LeftButton)
    {
      //  Check for the control key modifier.  If it's set, we want to rotate the image.

      if (e->modifiers () == Qt::ControlModifier)
        {
          //  Get the cursor position at this time.

          start_ctrl_x = e->x ();
          start_ctrl_y = e->y ();

          misc.save_function = misc.function;
          misc.function = ROTATE;
          setFunctionCursor (misc.function);
        }
      else
        {
          if (!pfm_edit_active) leftMouse (e->x (), e->y (), lon, lat, z);
        }
    }

  if (e->button () == Qt::MidButton)
    {
      //  Check for the control key modifier.  If it's set, we want to move to center (in slotMouseRelease).

      if (e->modifiers () == Qt::ControlModifier)
        {
          //  Get the cursor position at this time.

          start_ctrl_x = e->x ();
          start_ctrl_y = e->y ();
          mid_lat = lat;
          mid_lon = lon;
          mid_z = z;
        }
    }

  if (e->button () == Qt::RightButton)
    {
      //  Check for the control key modifier.  If it's set, we want to zoom based on cursor movement.

      if (e->modifiers () == Qt::ControlModifier)
        {
          start_ctrl_y = e->y ();

          misc.save_function = misc.function;
          misc.function = ZOOM;
          setFunctionCursor (misc.function);
        }
      else
        {
          if (!pfm_edit_active) rightMouse (e->x (), e->y (), lon, lat, z);
        }
    }
}



//  Mouse press signal prior to signals being enabled from the map class.

void 
pfm3D::slotPreliminaryMousePress (QMouseEvent *e)
{
  QPoint pos = QPoint (e->x (), e->y ());

  QWhatsThis::showText (pos, mapText, map);
}



//  Signal from the map class.

void 
pfm3D::slotMouseRelease (QMouseEvent * e, NV_FLOAT64 lon __attribute__ ((unused)),
                         NV_FLOAT64 lat __attribute__ ((unused)), NV_FLOAT64 z __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton)
    {
      popup_active = NVFalse;
      switch (misc.function)
        {
        case ROTATE:
          misc.function = misc.save_function;
          setFunctionCursor (misc.function);
          map->force_full_res ();
          break;
        }
    }

  if (e->button () == Qt::MidButton)
    {
      //  Check for the control key modifier.  If it's set, we want to center on the cursor.

      if (e->modifiers () == Qt::ControlModifier)
        {
          //if (abs (start_ctrl_x - e->x ()) < 5 && abs (start_ctrl_y - e->y ()) < 5)
            {
              map->setMapCenter (mid_lon, mid_lat, mid_z);
            }
        }
      else
        {
          if (!pfm_edit_active) midMouse (start_ctrl_x, start_ctrl_y, mid_lon, mid_lat, mid_z);
        }

      misc.function = misc.save_function;
      setFunctionCursor (misc.function);

      popup_active = NVFalse;
    }

  if (e->button () == Qt::RightButton)
    {
      if (misc.function == ZOOM)
        {
          misc.function = misc.save_function;
          setFunctionCursor (misc.function);
          map->force_full_res ();
        }
    }
}



//  Mouse wheel signal from the map class.

void
pfm3D::slotWheel (QWheelEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)),
                  NV_FLOAT64 z __attribute__ ((unused)))
{
  static NV_INT32 prev_numSteps = 0;

  NV_INT32 numDegrees = e->delta () / 8;
  NV_INT32 numSteps = numDegrees / 15;


  //  Only zoom if we are holding down the Ctrl key.

  if (e->modifiers () == Qt::ControlModifier)
    {
      //  Only do this if we're not waiting after a reversal of wheel direction.

      if (!zoom_wait)
        {
          zoom_accum += numSteps;


          //  If we reversed direction we want to wait for 1 second before using anymore mouse events.

          if ((numSteps < 0 && prev_numSteps > 0) || (numSteps > 0 && prev_numSteps < 0))
            {
              zoom_accum = 0;
              zoom_wait = NVTrue;

              QTimer::singleShot (1000, this, SLOT (slotZoomWait ()));
            }

          prev_numSteps = numSteps;

          if (!zoom_accum)
            {
              map->force_full_res ();
              prev_numSteps = 0;
            }
        }
    }
}



//  Signal from the map class.

void
pfm3D::slotMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z, NVMAPGL_DEF l_mapdef)
{
  NV_CHAR              ltstring[25], lnstring[25], hem;
  QString              string, geo_string, exag_string;
  NV_FLOAT64           deg, min, sec;
  NV_INT32             ix, iy;
  NV_F64_COORD3        xy, prev_xy = {-1.0, -1.0, -1.0};


  //  Let other ABE programs know which window we're in.

  misc.abe_share->active_window_id = active_window_id;


  //  If pfmEdit3D *was* up and we had cleared memory we need to reload it  This check is done in both
  //  slotMouseMove () and slotTrackCursor ().

  if (pfmEdit3D_opened && misc.abe_share->key == PFMEDIT3D_CLOSED && cleared)
    {
      pfmEdit3D_opened = NVFalse;
      cleared = NVFalse;
      delayClear = NVFalse;
      initializeMaps (NVFalse);
      return;
    }


  //  If we just started pfmEdit3D and it hasn't told us to clear memory yet (and we're still in this window)
  //  go ahead and clear it.

  if (delayClear && !cleared)
    {
      pfmEdit3D_opened = NVTrue;
      cleared = NVTrue;
      map->clearData (NVFalse);
      return;
    }


  if (delayClear) return;


  //  If pfmEdit3D is up and we have cleared memory we need to reload it since we've obviously moved back 
  //  into this window.  Once we move out of this window it will be cleared again by slotTrackCursor ().

  if (pfmEdit3D_opened && cleared)
    {
      cleared = NVFalse;
      initializeMaps (NVFalse);
      return;
    }


  //  Get rid of the tracking cursor from slotTrackCursor ().  But only if it already exists, otherwise we
  //  will be creating a new one (which we don't want to do).

  if (mv_tracker >= 0) map->closeMovingList (&mv_tracker);


  xy.x = lon;
  xy.y = lat;
  xy.z = z;
  ix = e->x ();
  iy = e->y ();


  //  Track the cursor position for other ABE programs

  misc.abe_share->cursor_position.y = lat;
  misc.abe_share->cursor_position.x = lon;

  if (lat >= misc.abe_share->viewer_displayed_area.min_y && lat <= misc.abe_share->viewer_displayed_area.max_y && 
      lon >= misc.abe_share->viewer_displayed_area.min_x && lon <= misc.abe_share->viewer_displayed_area.max_x)
    {
      //  Try to find the highest layer with a valid value to display in the status bar.

      NV_F64_COORD2 xy;
      xy.y = lat;
      xy.x = lon;
      NV_INT32 hit = -1;
      BIN_RECORD bin;


      for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
        {
          if (bin_inside_ptr (&misc.abe_share->open_args[pfm].head, xy))
            {
              read_bin_record_xy (misc.pfm_handle[pfm], xy, &bin);


              //  Check the validity.

              if ((bin.validity & PFM_DATA) || 
                  ((bin.validity & PFM_INTERPOLATED) && misc.abe_share->layer_type == AVERAGE_FILTERED_DEPTH) ||
                  (misc.abe_share->layer_type > MAX_FILTERED_DEPTH && bin.num_soundings))
                {
                  hit = pfm;
                  break;
                }
            }
        }


      strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));

      geo_string.sprintf ("Lat: %s  Lon: %s", ltstring, lnstring);

      exag_string.sprintf ("Z Exaggeration: %.3f", options.exaggeration);


      if (hit != -1)
        {
          NV_FLOAT64 z = -999999.0;
          read_bin_record_xy (misc.pfm_handle[hit], xy, &bin);

          switch (misc.abe_share->layer_type)
            {
            case AVERAGE_FILTERED_DEPTH:
            default:
              if (bin.validity & (PFM_DATA | PFM_INTERPOLATED)) z = bin.avg_filtered_depth * options.z_factor + options.z_offset;
              break;

            case MIN_FILTERED_DEPTH:
              if (bin.validity & PFM_DATA) z = bin.min_filtered_depth * options.z_factor + options.z_offset;
              break;

            case MAX_FILTERED_DEPTH:
              if (bin.validity & PFM_DATA) z = bin.max_filtered_depth * options.z_factor + options.z_offset;
              break;

            case AVERAGE_DEPTH:
              if (bin.num_soundings) z = bin.avg_depth * options.z_factor + options.z_offset;
              break;

            case MIN_DEPTH:
              if (bin.num_soundings) z = bin.min_depth * options.z_factor + options.z_offset;
              break;

            case MAX_DEPTH:
              if (bin.num_soundings) z = bin.max_depth * options.z_factor + options.z_offset;
              break;
            }


          NV_INT32 pix_x, pix_y;
          NV_FLOAT64 x = bin.xy.x - misc.abe_share->open_args[hit].head.x_bin_size_degrees / 2.0;
          NV_FLOAT64 y = bin.xy.y - misc.abe_share->open_args[hit].head.y_bin_size_degrees / 2.0;
          map->get2DCoords (x, y, -z, &pix_x, &pix_y);

          map->setMovingList (&mv_marker, marker, 16, pix_x, pix_y, 0.0, 2, options.tracker_color);
        }



      statusBar ()->showMessage (geo_string + "      " + exag_string);
    }


  //  Actions based on the active function

  switch (misc.function)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      map->dragRubberbandRectangle (rb_rectangle, ix, iy);
      break;


    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      if (map->rubberbandPolygonIsActive (rb_polygon)) map->dragRubberbandPolygon (rb_polygon, ix, iy);
      break;


    case ROTATE:
      if (start_ctrl_x != ix || start_ctrl_y != iy)
        {
          NV_INT32 diff_x = ix - start_ctrl_x;


          if (diff_x)
            {
              //  Flip the sign if we are above the center point looking at the top of the surface or below
              //  the center point looking at the bottom.  This allows the Y rotation from the mouse click/drag
              //  to be more intuitive.

              NV_FLOAT64 zxrot = map->getZXRotation ();
              NV_INT32 half = l_mapdef.draw_height / 2;


              //  If we're within 15 degrees of flat rotate normally for the front side.

              if ((zxrot < 15.0 && zxrot > -15.0) || (zxrot > 345.0) || (zxrot < -345.0))
                {
                  //  Don't do anything.
                }


              //  Reverse rotate reverse for the back side.

              else if ((zxrot > -195.0 && zxrot < -165.0) || (zxrot < 195.0 && zxrot > 165.0))
                {
                  diff_x = -diff_x;
                }


              //  Otherwise, check to see which side is up.

              else
                {
                  if ((iy < half && ((zxrot > 0.0 && zxrot < 180.0) || (zxrot < -180.0))) ||
                      (iy > half && ((zxrot < 0.0 && zxrot > -180.0) || (zxrot > 180.0)))) diff_x = -diff_x;
                }

              map->rotateY ((NV_FLOAT64) diff_x / 5.0);
            }

          NV_INT32 diff_y = iy - start_ctrl_y;

          if (diff_y) map->rotateZX ((NV_FLOAT64) diff_y / 5.0);


          start_ctrl_x = ix;
          start_ctrl_y = iy;
        }
      break;

    case ZOOM:
      if (start_ctrl_y != xy.y)
        {
          NV_INT32 diff_y = iy - start_ctrl_y;


          if (diff_y < -5)
            {
              map->zoomInPercent ();
              start_ctrl_y = iy;
            }
          else if (diff_y > 5)
            {
              map->zoomOutPercent ();
              start_ctrl_y = iy;
            }
        }
      break;

    default:
      break;
    }


  //  Set the previous cursor.
             
  prev_xy = xy;
}



//  Timer - timeout signal.  Very much like an X workproc.  This tracks the cursor in the associated
//  programs (pfmEdit and pfmView) not in this window.  This is active whenever the mouse leaves this
//  window.

void
pfm3D::slotTrackCursor ()
{
  NV_CHAR               ltstring[25], lnstring[25];
  QString               geo_string;
  QString               string;
  static NVMAP_DEF      mpdf;
  static NV_F64_COORD2  prev_xy;


  //  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key

  if (misc.abe_share->key == CHILD_PROCESS_FORCE_EXIT) slotQuit ();


  if (misc.drawing) return;


  //  If we just started pfmEdit3D and it hasn't told us to clear memory yet go ahead and clear it.

  if (delayClear && !cleared)
    {
      pfmEdit3D_opened = NVTrue;
      cleared = NVTrue;
      map->clearData (NVFalse);
      return;
    }


  //  If we're not in this window, erase the marker and check for the 3D editor being up.

  if (misc.abe_share->active_window_id != active_window_id)
    {
      if (mv_marker >= 0) map->closeMovingList (&mv_marker);


      //  We must have changed windows so we can allow this window to reload if we re-enter it.
      //  Assuming we just started pfmEdit3D.

      delayClear = NVFalse;


      if (pfmEdit3D_opened && !cleared)
        {
          cleared = NVTrue;
          map->clearData (NVFalse);
        }

      if (!pfmEdit3D_opened && misc.abe_share->key == PFMEDIT3D_OPENED)
        {
          misc.abe_share->key = NO_ACTION_REQUIRED;

          pfmEdit3D_opened = NVTrue;
          cleared = NVTrue;

          map->clearData (NVFalse);
        }


      //  If pfmEdit3D *was* up and we had cleared memory we need to reload it.  This check is done in both
      //  slotMouseMove () and slotTrackCursor ().

      if (pfmEdit3D_opened && misc.abe_share->key == PFMEDIT3D_CLOSED && cleared)
        {
          pfmEdit3D_opened = NVFalse;
          cleared = NVFalse;

          initializeMaps (NVFalse);

          return;
        }
    }


  //  If we have changed the view in pfmView, we need to reload the data.

  if (misc.abe_share->key == PFM3D_FORCE_RELOAD || misc.abe_share->key == PFM_LAYERS_CHANGED)
    {
      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED; 

      initializeMaps (NVFalse);

      pfmEdit3D_opened = NVFalse;
      cleared = NVFalse;

      return;
    }


  /*
      this->setWindowTitle (QString ("pfm3D : ") + QString (misc.abe_share->open_args[0].image_path));

      NV_CHAR retval[512];
      strcpy (retval, get_geotiff (misc.abe_share->open_args[0].image_path, &misc));

      misc.displayed_area = misc.geotiff_area;
      map->resetBounds (misc.geotiff_area);

      redrawMap ();
    }
  */


  //  If we have changed the feature file contents in another program, we need to draw the GeoTIFF.

  if (misc.abe_share->key == FEATURE_FILE_MODIFIED)
    {
      //  Try to open the feature file and read the features into memory.

      if (strcmp (misc.abe_share->open_args[0].target_path, "NONE"))
        {
          if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);
          misc.bfd_open = NVFalse;

          if ((misc.bfd_handle = binaryFeatureData_open_file (misc.abe_share->open_args[0].target_path, &misc.bfd_header, BFDATA_UPDATE)) >= 0)
            {
              if (binaryFeatureData_read_all_short_features (misc.bfd_handle, &misc.feature) < 0)
                {
                  QString msg = QString (binaryFeatureData_strerror ());
                  QMessageBox::warning (this, tr ("pfm3D"), tr ("Unable to read feature records\nReason: ") + msg);
                  binaryFeatureData_close_file (misc.bfd_handle);
                }
              else
                {
                  misc.bfd_open = NVTrue;
                }
            }
        }


      if (options.display_feature) initializeMaps (NVFalse);


      //  Wait 2 seconds so that all associated programs will see the modified flag.

#ifdef NVWIN3X
      Sleep (2000);
#else
      sleep (2);
#endif
      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED; 
    }


  NV_F64_COORD2 xy;
  xy.y = misc.abe_share->cursor_position.y;
  xy.x = misc.abe_share->cursor_position.x;


  if (misc.abe_share->active_window_id != active_window_id && xy.y >= misc.abe_share->viewer_displayed_area.min_y &&
      xy.y <= misc.abe_share->viewer_displayed_area.max_y && xy.x >= misc.abe_share->viewer_displayed_area.min_x &&
      xy.x <= misc.abe_share->viewer_displayed_area.max_x && xy.y != prev_xy.y && xy.x != prev_xy.x)
    {
      //  Try to find the highest layer with a valid value to display in the status bar.

      NV_INT32 hit = -1;
      BIN_RECORD bin;


      for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
        {
          if (bin_inside_ptr (&misc.abe_share->open_args[pfm].head, xy))
            {
              read_bin_record_xy (misc.pfm_handle[pfm], xy, &bin);


              //  Check the validity.

              if ((bin.validity & PFM_DATA) || 
                  ((bin.validity & PFM_INTERPOLATED) && misc.abe_share->layer_type == AVERAGE_FILTERED_DEPTH) ||
                  (misc.abe_share->layer_type > MAX_FILTERED_DEPTH && bin.num_soundings))
                {
                  hit = pfm;
                  break;
                }
            }
        }


      NV_FLOAT64 deg, min, sec;
      NV_CHAR    hem;

      strcpy (ltstring, fixpos (xy.y, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (xy.x, &deg, &min, &sec, &hem, POS_LON, options.position_form));

      geo_string.sprintf ("Lat: %s  Lon: %s", ltstring, lnstring);

      QString lbl;
      lbl.sprintf (tr ("Z Exaggeration: %5.3f").toAscii (), (NV_FLOAT32) exagBar->value () / 100.0);
      statusBar ()->showMessage (geo_string + "      " + lbl);


      if (hit != -1)
        {
          read_bin_record_xy (misc.pfm_handle[hit], xy, &bin);

          NV_FLOAT64 z = 0.0;
          switch (misc.abe_share->layer_type)
            {
            case AVERAGE_FILTERED_DEPTH:
            default:
              if (bin.validity & (PFM_DATA | PFM_INTERPOLATED)) z = -bin.avg_filtered_depth;
              break;

            case MIN_FILTERED_DEPTH:
              if (bin.validity & PFM_DATA) z = -bin.min_filtered_depth;
              break;

            case MAX_FILTERED_DEPTH:
              if (bin.validity & PFM_DATA) z = -bin.max_filtered_depth;
              break;

            case AVERAGE_DEPTH:
              if (bin.num_soundings) z = -bin.avg_depth;
              break;

            case MIN_DEPTH:
              if (bin.num_soundings) z = -bin.min_depth;
              break;

            case MAX_DEPTH:
              if (bin.num_soundings) z = -bin.max_depth;
              break;
            }

          NV_INT32 pix_x, pix_y;
          map->get2DCoords (xy.x, xy.y, z, &pix_x, &pix_y);

          map->setMovingList (&mv_tracker, marker, 16, pix_x, pix_y, 0.0, 2, options.tracker_color);
        }
    }
  prev_xy = xy;
}



//  slotZoomTimer - timeout signal.  Mouse wheel zoom timer.

void
pfm3D::slotZoomTimer ()
{
  if (zoom_accum)
    {
      NV_INT32 percent = abs (zoom_accum) * mapdef.zoom_percent;

      if (zoom_accum > 0)
        {
          map->zoomInPercent (percent);
        }
      else
        {
          map->zoomOutPercent (percent);
        }
    }
}



//  slotZoomWait - timeout signal.

void
pfm3D::slotZoomWait ()
{
  zoom_wait = NVFalse;
}



//  Resize signal from the map class.

void
pfm3D::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  //initializeMaps ();
}



void 
pfm3D::slotExagTriggered (int action)
{
  if (!misc.drawing)
    {
      switch (action)
        {
        case QAbstractSlider::SliderSingleStepAdd:
          if (options.exaggeration >= 1.0)
            {
              options.exaggeration -= 1.0;

              if (options.exaggeration < 1.0) options.exaggeration = 1.0;

              map->setExaggeration (options.exaggeration);

              initializeMaps (NVFalse);
            }
          break;

        case QAbstractSlider::SliderPageStepAdd:
          if (options.exaggeration >= 5.0)
            {
              options.exaggeration -= 5.0;

              if (options.exaggeration < 1.0) options.exaggeration = 1.0;

              map->setExaggeration (options.exaggeration);

              initializeMaps (NVFalse);
            }
          break;

        case QAbstractSlider::SliderSingleStepSub:
          options.exaggeration += 1.0;
          if (options.exaggeration > 100.0) options.exaggeration = 100.0;
          map->setExaggeration (options.exaggeration);

          initializeMaps (NVFalse);
          break;

        case QAbstractSlider::SliderPageStepSub:
          options.exaggeration += 5.0;
          if (options.exaggeration > 100.0) options.exaggeration = 100.0;
          map->setExaggeration (options.exaggeration);

          initializeMaps (NVFalse);
          break;

        case QAbstractSlider::SliderMove:
          QString lbl;
          lbl.sprintf (tr ("Z Exaggeration: %5.3f").toAscii (), (NV_FLOAT32) exagBar->value () / 100.0);
          statusBar ()->showMessage (lbl);
          break;
        }
    }
}



void 
pfm3D::slotExagReleased ()
{
  options.exaggeration = (NV_FLOAT32) exagBar->value () / 100.0;
  map->setExaggeration (options.exaggeration);

  initializeMaps (NVFalse);
}



void 
pfm3D::slotExaggerationChanged (NV_FLOAT32 exaggeration, NV_FLOAT32 apparent_exaggeration)
{
  if (exaggeration < 1.0)
    {
      exagBar->setEnabled (FALSE);
    }
  else
    {
      exagBar->setEnabled (TRUE);
    }

  QString lbl;
  lbl.sprintf (tr ("Z Exaggeration: %5.3f").toAscii (), apparent_exaggeration);
  statusBar ()->showMessage (lbl);

  options.exaggeration = exaggeration;

  disconnect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
  exagBar->setValue (NINT (options.exaggeration * 100.0));
  connect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
}



//  Using the keys to move around.

void
pfm3D::keyPressEvent (QKeyEvent *e)
{
  QString lbl;


  if (!misc.drawing && !pfm_edit_active)
    {
      switch (e->key ())
        {
        case Qt::Key_Left:
          map->rotateY (-5.0);
          return;
          break;

        case Qt::Key_Up:
          map->rotateZX (-5.0);
          return;
          break;

        case Qt::Key_Right:
          map->rotateY (5.0);
          return;
          break;

        case Qt::Key_Down:
          map->rotateZX (5.0);
          return;
          break;

        case Qt::Key_PageUp:
          slotExagTriggered (QAbstractSlider::SliderSingleStepSub);
          return;
          break;

        case Qt::Key_PageDown:
          slotExagTriggered (QAbstractSlider::SliderSingleStepAdd);
          return;
          break;

        default:
          return;
        }
      e->accept ();
    }
}



void 
pfm3D::slotClose (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



void 
pfm3D::slotQuit ()
{
  NV_BOOL envout (OPTIONS *options, QMainWindow *mainWindow);


  options.edit_mode = misc.save_function;


  envout (&options, this);


  //  Get rid of the shared memory.

  misc.abeShare->detach ();


  //  Close the BFD file if opened

  if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);


  clean_exit (0);
}



void 
pfm3D::clean_exit (NV_INT32 ret)
{
  //  Have to close the GL widget or it stays on screen in VirtualBox

  map->close ();

  exit (ret);
}



void
pfm3D::slotGeotiff ()
{
  setMainButtons (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  if (!misc.display_GeoTIFF)
    {
      misc.display_GeoTIFF = 1;
      map->setTextureType (misc.display_GeoTIFF);
      map->enableTexture (NVTrue);
      bGeotiff->setIcon (QIcon (":/icons/geo_decal.xpm"));
    }
  else if (misc.display_GeoTIFF > 0)
    {
      misc.display_GeoTIFF = -1;
      map->setTextureType (misc.display_GeoTIFF);
      map->enableTexture (NVTrue);
      bGeotiff->setIcon (QIcon (":/icons/geo_modulate.xpm"));
    }
  else
    {
      misc.display_GeoTIFF = 0;
      map->enableTexture (NVFalse);
      bGeotiff->setIcon (QIcon (":/icons/geo_off.xpm"));
    }
  qApp->processEvents ();

  setMainButtons (NVTrue);


  qApp->restoreOverrideCursor ();


  initializeMaps (NVFalse);
}



void
pfm3D::slotReset ()
{
  map->setMapCenter (misc.map_center_x, misc.map_center_y, misc.map_center_z);


  map->resetPOV ();
}



void
pfm3D::slotEditMode (int id)
{
  QString msc;


  switch (id)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      misc.function = id;
      misc.save_function = misc.function;
      break;
    }
  setFunctionCursor (misc.function);
}



void
pfm3D::slotFeatureMenu (QAction *action)
{
  for (NV_INT32 i = 0 ; i < 4 ; i++)
    {
      if (action == displayFeature[i])
        {
          options.display_feature = i;
          break;
        }
    }

  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);

  if (options.display_feature)
    {
      bDisplayChildren->setEnabled (TRUE);
      bDisplayFeatureInfo->setEnabled (TRUE);

      overlayFlag (map, &options, &misc);
    }
  else
    {
      bDisplayChildren->setEnabled (FALSE);
      bDisplayFeatureInfo->setEnabled (FALSE);

      initializeMaps (NVFalse);
    }
}



void
pfm3D::slotDisplayChildren ()
{
  if (bDisplayChildren->isChecked ())
    {
      options.display_children = NVTrue;

      overlayFlag (map, &options, &misc);
    }
  else
    {
      options.display_children = NVFalse;

      initializeMaps (NVFalse);
    }
}



void
pfm3D::slotDisplayFeatureInfo ()
{
  if (bDisplayFeatureInfo->isChecked ())
    {
      options.display_feature_info = NVTrue;

      overlayFlag (map, &options, &misc);
    }
  else
    {
      options.display_feature_info = NVFalse;

      initializeMaps (NVFalse);
    }
}



void 
pfm3D::setFunctionCursor (NV_INT32 function)
{
  misc.function = function;


  discardMovableObjects ();


  switch (function)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      map->setCursor (editRectCursor);
      break;

    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      map->setCursor (editPolyCursor);
      break;

    case ROTATE:
      map->setCursor (rotateCursor);
      break;

    case ZOOM:
      map->setCursor (zoomCursor);
      break;
      /*
    case DRAG:
      map->setCursor (Qt::SizeAllCursor);
      break;
      */
    }
}



void 
pfm3D::slotBrowserError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfm3D help browser"), tr ("Unable to start the browser process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfm3D help browser"), tr ("The browser process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfm3D help browser"), tr ("The browser process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfm3D help browser"), tr ("There was a write error to the browser process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfm3D help browser"), tr ("There was a read error from the browser process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfm3D help browser"), tr ("The browser process died with an unknown error!"));
      break;
    }
}



void
pfm3D::extendedHelp ()
{
  if (getenv ("PFM_ABE") == NULL)
    {
      QMessageBox::warning (this, tr ("pfm3D extended help"), 
                            tr ("The PFM_ABE environment variable is not set.\n") +
                            tr ("This must point to the folder that contains the bin folder containing the PFM_ABE programs."));
      return;
    }



  QString html_help_file;
  QString dir (getenv ("PFM_ABE"));
  html_help_file = dir + SEPARATOR + "doc" + SEPARATOR + "PFM" + SEPARATOR + "Area_Based_Editor.html";


  QFileInfo hf (html_help_file);

  if (!hf.exists () || !hf.isReadable ())
    {
      QMessageBox::warning (this, tr ("pfm3D extended help"), 
                            tr ("Can't find the Area Based Editor documentation file: ") + html_help_file);
      return;
    }


  QFileInfo hbi (misc.help_browser);

  if (hbi.isRelative ())
    {
      NV_CHAR hb[512];
      strcpy (hb, misc.help_browser.toAscii ());

      if (find_startup_name (hb) == NULL)
        {
          QMessageBox::warning (this, tr ("pfm3D extended help"), 
                                tr ("Unable to find the specified browser: ") + misc.help_browser);
          return;
        }

      hbi = QString (find_startup_name (hb));
    }


  if (!hbi.exists () || !hbi.isExecutable ())
    {
      QMessageBox::warning (this, tr ("pfm3D extended help"), 
                            tr ("Unable to use the specified browser: ") + misc.help_browser);
      return;
    }


  QString arg;
  QStringList arguments;

  arguments.clear ();


  if (browserProc && browserProc->state () == QProcess::Running)
    {
      disconnect (browserProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotBrowserError (QProcess::ProcessError)));

      browserProc->kill ();
      delete (browserProc);
    }

  browserProc = new QProcess (this);

#ifdef NVWIN3X
  arg = "file://" + html_help_file + misc.html_help_tag;
  arguments += arg;
#else
  arg = "file:" + html_help_file + misc.html_help_tag;
  arguments += arg;
#endif

  connect (browserProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotBrowserError (QProcess::ProcessError)));

  browserProc->start (misc.help_browser, arguments);
}



void
pfm3D::slotHotkeyHelp ()
{
  hotkeyHelp *hk = new hotkeyHelp (this, &options, &misc);
  hk->show ();
}


void
pfm3D::about ()
{
  QMessageBox::about (this, VERSION,
                      tr ("pfm3D - 3D PFM viewer.") + 
                      tr ("\n\nAuthor : Jan C. Depner (depnerj@navo.navy.mil)"));
}


void
pfm3D::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}



void
pfm3D::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



//  This triggers a paintevent in the QGLWidget on Windoze.  I have no idea why.

void 
pfm3D::focusInEvent (QFocusEvent *e __attribute__ ((unused)))
{
  map->enableWindozePaintEvent (NVFalse);
}
