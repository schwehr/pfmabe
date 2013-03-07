//  areaCheck class.

/***************************************************************************\

    Module :        areaCheck

    Programmer :    Jan C. Depner

    Date :          06/22/05

    Purpose :       Simple area file viewer/editor.

    Caveats :       We can start this up with a couple of command line options.
                    Either "file" or "empty_file" and a file name.  These are
                    used to allow other programs to shell areaCheck as a single
                    file editor/creator.  Some of the buttons will be desensitized
                    if either of these options are used.

\***************************************************************************/

#include "areaCheck.hpp"
#include "areaCheckHelp.hpp"


static NV_BOOL popup_active;


areaCheck::areaCheck (int *argc, char **argv, QWidget *parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;


  NV_BOOL envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);
  void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore);


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Override the HDF5 version check so that we can read BAGs created with an older version of HDF5.

  putenv ("HDF5_DISABLE_VERSION_CHECK=2");


  //  Initialize some important variables.

  misc.function = ZOOM_IN_AREA;
  prev_area = -1;
  cur_area = -1;
  prev_vertex = -1;
  cur_vertex = -1;
  prev_line[0] = -1;
  cur_line[0] = -1;
  cur_type = -1;
  prev_type = -1;
  area_moving = NVFalse;
  area_copying = NVFalse;
  vertex_moving = NVFalse;
  line_moving = NVFalse;
  new_overlay.count = 0;
  new_overlay.x = NULL;
  new_overlay.y = NULL;
  no_save = NVFalse;
  input_file = NVFalse;
  empty_file = NVFalse;
  any_file = NVFalse;
  popup_active = NVFalse;
  double_click = NVFalse;
  rb_rectangle = -1;
  rb_polygon = -1;
  mv_polygon = -1;
  mv_marker = -1;
  mv_tracker = -1;
  active_window_id = getpid ();
  abe_register_group = -1;
  need_redraw = NVFalse;
  for (NV_INT32 k = 0 ; k < NUM_TYPES ; k++) displayFile_dialog[k] = NULL;


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

  setWindowIcon (QIcon (":/icons/pfm_abe.xpm"));


  //  Define the cursors

  zoomCursor = QCursor (QPixmap (":/icons/zoom_cursor.xpm"), 11, 11);
  addVertexCursor = QCursor (QPixmap (":/icons/add_vertex_cursor.xpm"), 3, 13);
  moveVertexCursor = QCursor (QPixmap (":/icons/move_vertex_cursor.xpm"), 15, 26);
  deleteVertexCursor = QCursor (QPixmap (":/icons/delete_vertex_cursor.xpm"), 2, 2);
  editVertexCursor = QCursor (QPixmap (":/icons/edit_vertex_cursor.xpm"), 11, 26);
  copyAreaCursor = QCursor (QPixmap (":/icons/copy_area_cursor.xpm"), 1, 2);
  moveAreaCursor = QCursor (QPixmap (":/icons/move_area_cursor.xpm"), 15, 26);
  editRectCursor = QCursor (QPixmap (":/icons/edit_rect_cursor.xpm"), 1, 1);
  editPolyCursor = QCursor (QPixmap (":/icons/edit_poly_cursor.xpm"), 1, 1);


  //  Get command line arguments

  NV_INT32 option_index = 0;
  while (NVTrue) 
    {
      static struct option long_options[] = {{"file", required_argument, 0, 0},
                                             {"empty_file", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "s", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:
          NV_CHAR tmp[512];
          sscanf (optarg, "%s", tmp);

          switch (option_index)
            {
            case 0:
              fileName = QString (tmp);
              input_file = NVTrue;
              break;

            case 1:
              fileName = QString (tmp);
              empty_file = NVTrue;
              break;
            }
        }
    }


  //  Set all of the defaults

  set_defaults (&misc, &options, NVFalse);


  //  This is so we can drag and drop files on the Desktop icon.

  if (*argc > 1 && !input_file && !empty_file)
    {
      fileName = QString (argv[1]);
      any_file = NVTrue;
    }


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  tools = new QToolBar (tr ("Tools"));
  addToolBar (Qt::TopToolBarArea, tools);
  tools->setObjectName (tr ("areaCheck main toolbar"));


  //  Set up the tool bar buttons

  QButtonGroup *exitGrp = new QButtonGroup (this);
  connect (exitGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotExit (int)));


  bExitSave = new QToolButton (this);
  bExitSave->setIcon (QIcon (":/icons/exit_save.xpm"));
  bExitSave->setToolTip (tr ("Save all changes and exit"));
  bExitSave->setWhatsThis (exitSaveText);
  exitGrp->addButton (bExitSave, 0);
  tools->addWidget (bExitSave);


  bExitNoSave = new QToolButton (this);
  bExitNoSave->setIcon (QIcon (":/icons/exit_no_save.xpm"));
  bExitNoSave->setToolTip (tr ("Exit without saving changes"));
  bExitNoSave->setWhatsThis (exitNoSaveText);
  exitGrp->addButton (bExitNoSave, 1);
  tools->addWidget (bExitNoSave);


  tools->addSeparator ();


  bFile = new QToolButton (this);
  bFile->setIcon (QIcon (":/icons/fileopen.xpm"));
  bFile->setToolTip (tr ("Open input files"));
  bFile->setWhatsThis (fileText);
  connect (bFile, SIGNAL (clicked ()), this, SLOT (slotOpenFiles ()));
  tools->addWidget (bFile);


  tools->addSeparator ();


  bZoomIn = new QToolButton (this);
  bZoomIn->setIcon (QIcon (":/icons/icon_zoomin.xpm"));
  bZoomIn->setToolTip (tr ("Zoom in"));
  bZoomIn->setWhatsThis (zoomInText);
  connect (bZoomIn, SIGNAL (clicked ()), this, SLOT (slotZoomIn ()));
  tools->addWidget (bZoomIn);


  bZoomOut = new QToolButton (this);
  bZoomOut->setIcon (QIcon (":/icons/icon_zoomout.xpm"));
  bZoomOut->setToolTip (tr ("Zoom out"));
  bZoomOut->setWhatsThis (zoomOutText);
  connect (bZoomOut, SIGNAL (clicked ()), this, SLOT (slotZoomOut ()));
  tools->addWidget (bZoomOut);


  tools->addSeparator ();


  bRect = new QToolButton (this);
  bRect->setIcon (QIcon (":/icons/edit_rect.xpm"));
  bRect->setToolTip (tr ("Define rectangular area"));
  bRect->setWhatsThis (rectText);
  connect (bRect, SIGNAL (clicked ()), this, SLOT (slotDefineRectArea ()));
  tools->addWidget (bRect);


  bPoly = new QToolButton (this);
  bPoly->setIcon (QIcon (":/icons/edit_poly.xpm"));
  bPoly->setToolTip (tr ("Define polygonal area"));
  bPoly->setWhatsThis (polyText);
  connect (bPoly, SIGNAL (clicked ()), this, SLOT (slotDefinePolyArea ()));
  tools->addWidget (bPoly);


  tools->addSeparator ();


  bAddVertex = new QToolButton (this);
  bAddVertex->setIcon (QIcon (":/icons/add_vertex.xpm"));
  bAddVertex->setToolTip (tr ("Add vertex"));
  bAddVertex->setWhatsThis (addVertexText);
  connect (bAddVertex, SIGNAL (clicked ()), this, SLOT (slotAddVertex ()));
  tools->addWidget (bAddVertex);


  bDeleteVertex = new QToolButton (this);
  bDeleteVertex->setIcon (QIcon (":/icons/delete_vertex.xpm"));
  bDeleteVertex->setToolTip (tr ("Delete vertex"));
  bDeleteVertex->setWhatsThis (deleteVertexText);
  connect (bDeleteVertex, SIGNAL (clicked ()), this, SLOT (slotDeleteVertex ()));
  tools->addWidget (bDeleteVertex);


  bMoveVertex = new QToolButton (this);
  bMoveVertex->setIcon (QIcon (":/icons/move_vertex.xpm"));
  bMoveVertex->setToolTip (tr ("Move vertex"));
  bMoveVertex->setWhatsThis (moveVertexText);
  connect (bMoveVertex, SIGNAL (clicked ()), this, SLOT (slotMoveVertex ()));
  tools->addWidget (bMoveVertex);


  bEditVertex = new QToolButton (this);
  bEditVertex->setIcon (QIcon (":/icons/edit_vertex.xpm"));
  bEditVertex->setToolTip (tr ("Edit vertex"));
  bEditVertex->setWhatsThis (editVertexText);
  connect (bEditVertex, SIGNAL (clicked ()), this, SLOT (slotEditVertex ()));
  tools->addWidget (bEditVertex);


  tools->addSeparator ();


  bMoveArea = new QToolButton (this);
  bMoveArea->setIcon (QIcon (":/icons/move_area.xpm"));
  bMoveArea->setToolTip (tr ("Move area"));
  bMoveArea->setWhatsThis (moveAreaText);
  connect (bMoveArea, SIGNAL (clicked ()), this, SLOT (slotMoveArea ()));
  tools->addWidget (bMoveArea);


  bCopyArea = new QToolButton (this);
  bCopyArea->setIcon (QIcon (":/icons/copy_area.xpm"));
  bCopyArea->setToolTip (tr ("Copy area"));
  bCopyArea->setWhatsThis (copyAreaText);
  connect (bCopyArea, SIGNAL (clicked ()), this, SLOT (slotCopyArea ()));
  tools->addWidget (bCopyArea);


  tools->addSeparator ();


  bCoast = new QToolButton (this);
  bCoast->setIcon (QIcon (":/icons/coast.xpm"));
  bCoast->setToolTip (tr ("Toggle coastline on and off"));
  bCoast->setWhatsThis (coastText);
  bCoast->setCheckable (TRUE);
  tools->addWidget (bCoast);


  bMask = new QToolButton (this);
  bMask->setIcon (QIcon (":/icons/landmask.xpm"));
  bMask->setToolTip (tr ("Toggle land mask on and off"));
  bMask->setWhatsThis (maskText);
  bMask->setCheckable (TRUE);
  tools->addWidget (bMask);


  tools->addSeparator ();


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.xpm"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  tools->addWidget (bPrefs);



  tools->addSeparator ();
  tools->addSeparator ();


  bLink = new QToolButton (this);
  bLink->setIcon (QIcon (":/icons/unlink.xpm"));
  bLink->setToolTip (tr ("Link to other ABE applications"));
  bLink->setWhatsThis (linkText);
  connect (bLink, SIGNAL (clicked ()), this, SLOT (slotLink ()));
  tools->addWidget (bLink);


  tools->addSeparator ();
  tools->addSeparator ();


  bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  tools->addAction (bHelp);


  //  This is the "dataTypes" toolbar.

  dataTypes = new QToolBar (tr ("Data Types"));
  addToolBar (Qt::LeftToolBarArea, dataTypes);
  dataTypes->setObjectName (tr ("areaCheck dataTypes toolbar"));


  //  DataTypes toolbar

  bStop = new QToolButton (this);
  bStop->setIcon (QIcon (":/icons/stop.xpm"));
  bStop->setToolTip (tr ("Stop drawing"));
  bStop->setWhatsThis (stopText);
  connect (bStop, SIGNAL (clicked ()), this, SLOT (slotStop ()));
  bStop->setEnabled (FALSE);
  dataTypes->addWidget (bStop);


  dataTypes->addSeparator ();
  dataTypes->addSeparator ();


  QButtonGroup *typeGrp = new QButtonGroup (this);
  connect (typeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotType (int)));


  bISS60Area = new QToolButton (this);
  bISS60Area->setIcon (QIcon (":/icons/display_iss60_area.xpm"));
  bISS60Area->setToolTip (tr ("Toggle display of ISS60 area files"));
  misc.overlayButton[ISS60_AREA] = bISS60Area;
  typeGrp->addButton (bISS60Area, ISS60_AREA);
  dataTypes->addWidget (bISS60Area);


  bGenArea = new QToolButton (this);
  bGenArea->setIcon (QIcon (":/icons/display_gen_area.xpm"));
  bGenArea->setToolTip (tr ("Toggle display of generic area files"));
  misc.overlayButton[GENERIC_AREA] = bGenArea;
  typeGrp->addButton (bGenArea, GENERIC_AREA);
  dataTypes->addWidget (bGenArea);


  bACEArea = new QToolButton (this);
  bACEArea->setIcon (QIcon (":/icons/display_ace_area.xpm"));
  bACEArea->setToolTip (tr ("Toggle display of ACE area files"));
  misc.overlayButton[ACE_AREA] = bACEArea;
  typeGrp->addButton (bACEArea, ACE_AREA);
  dataTypes->addWidget (bACEArea);


  bISS60Zone = new QToolButton (this);
  bISS60Zone->setIcon (QIcon (":/icons/display_iss60_zone.xpm"));
  bISS60Zone->setToolTip (tr ("Toggle display of ISS60 zone files"));
  misc.overlayButton[ISS60_ZONE] = bISS60Zone;
  typeGrp->addButton (bISS60Zone, ISS60_ZONE);
  dataTypes->addWidget (bISS60Zone);


  bGenZone = new QToolButton (this);
  bGenZone->setIcon (QIcon (":/icons/display_gen_zone.xpm"));
  bGenZone->setToolTip (tr ("Toggle display of generic zone files"));
  misc.overlayButton[GENERIC_ZONE] = bGenZone;
  typeGrp->addButton (bGenZone, GENERIC_ZONE);
  dataTypes->addWidget (bGenZone);


  dataTypes->addSeparator ();
  dataTypes->addSeparator ();


  bGeotiff = new QToolButton (this);
  bGeotiff->setIcon (QIcon (":/icons/displaygeotiff.xpm"));
  bGeotiff->setToolTip (tr ("Toggle display of geoTIFF files"));
  typeGrp->addButton (bGeotiff, GEOTIFF);
  misc.overlayButton[GEOTIFF] = bGeotiff;
  dataTypes->addWidget (bGeotiff);


  bChrtr = new QToolButton (this);
  bChrtr->setIcon (QIcon (":/icons/displaychrtr.xpm"));
  bChrtr->setToolTip (tr ("Toggle display of chrtr files"));
  misc.overlayButton[CHRTR] = bChrtr;
  typeGrp->addButton (bChrtr, CHRTR);
  dataTypes->addWidget (bChrtr);


  bBag = new QToolButton (this);
  bBag->setIcon (QIcon (":/icons/displaybag.xpm"));
  bBag->setToolTip (tr ("Toggle display of BAG files"));
  typeGrp->addButton (bBag, BAG);
  misc.overlayButton[BAG] = bBag;
  dataTypes->addWidget (bBag);


  bShape = new QToolButton (this);
  bShape->setIcon (QIcon (":/icons/displayshape.xpm"));
  bShape->setToolTip (tr ("Toggle display of shape files"));
  misc.overlayButton[SHAPE] = bShape;
  typeGrp->addButton (bShape, SHAPE);
  dataTypes->addWidget (bShape);


  bTrack = new QToolButton (this);
  bTrack->setIcon (QIcon (":/icons/displaytrack.xpm"));
  bTrack->setToolTip (tr ("Toggle display of track (.trk) files"));
  misc.overlayButton[TRACK] = bTrack;
  typeGrp->addButton (bTrack, TRACK);
  dataTypes->addWidget (bTrack);


  bFlight = new QToolButton (this);
  bFlight->setIcon (QIcon (":/icons/displayflight.xpm"));
  bFlight->setToolTip (tr ("Toggle display of Optech flight (.txt) files"));
  misc.overlayButton[OPTECH_FLT] = bFlight;
  typeGrp->addButton (bFlight, OPTECH_FLT);
  dataTypes->addWidget (bFlight);


  bYXZ = new QToolButton (this);
  bYXZ->setIcon (QIcon (":/icons/displayyxz.xpm"));
  bYXZ->setToolTip (tr ("Toggle display of YXZ files"));
  misc.overlayButton[YXZ] = bYXZ;
  typeGrp->addButton (bYXZ, YXZ);
  dataTypes->addWidget (bYXZ);


  bLLZ = new QToolButton (this);
  bLLZ->setIcon (QIcon (":/icons/displayllz.xpm"));
  bLLZ->setToolTip (tr ("Toggle display of LLZ files"));
  misc.overlayButton[LLZ_DATA] = bLLZ;
  typeGrp->addButton (bLLZ, LLZ_DATA);
  dataTypes->addWidget (bLLZ);


  dataTypes->addSeparator ();
  dataTypes->addSeparator ();


  bFeature = new QToolButton (this);
  bFeature->setIcon (QIcon (":/icons/displayfeature.xpm"));
  bFeature->setToolTip (tr ("Toggle display of feature files"));
  misc.overlayButton[FEATURE] = bFeature;
  typeGrp->addButton (bFeature, FEATURE);
  dataTypes->addWidget (bFeature);


  bChildren = new QToolButton (this);
  bChildren->setIcon (QIcon (":/icons/displayfeaturechildren.xpm"));
  bChildren->setToolTip (tr ("Toggle display of sub-features (grouped features)"));
  bChildren->setCheckable (TRUE);
  dataTypes->addWidget (bChildren);


  bFeatureInfo = new QToolButton (this);
  bFeatureInfo->setIcon (QIcon (":/icons/displayfeatureinfo.xpm"));
  bFeatureInfo->setToolTip (tr ("Toggle display of feature information"));
  bFeatureInfo->setCheckable (TRUE);
  dataTypes->addWidget (bFeatureInfo);


  bFeaturePoly = new QToolButton (this);
  bFeaturePoly->setIcon (QIcon (":/icons/displayfeaturepoly.xpm"));
  bFeaturePoly->setToolTip (tr ("Toggle display of feature polygons"));
  bFeaturePoly->setCheckable (TRUE);
  dataTypes->addWidget (bFeaturePoly);


  //  Get the user's defaults if available

  if (!envin (&options, &misc, this))
    {
      //  Set the geometry from defaults since we didn't get any from the saved settings.

      this->resize (misc.width, misc.height);
      this->move (misc.window_x, misc.window_y);
    }


  //  Define the surface colors.  Doing this here speeds up drawing.

  NV_FLOAT32 hue_inc = 315.0 / (NV_FLOAT32) (NUMHUES + 1);

  for (NV_INT32 i = 0 ; i < NUMHUES ; i++)
    {
      for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
	{
	  misc.color_array[i][j].setHsv ((NV_INT32) (((NUMHUES + 1) - i) * hue_inc), 255, j, options.color[CHRTR].alpha ());
	}
    }


  //  Set the sun shading value based on the defaults

  options.sunopts.sun = sun_unv (options.sunopts.azimuth, options.sunopts.elevation);


  //  Set the map values from the defaults

  mapdef.projection = CYLINDRICAL_EQUIDISTANT;
  mapdef.draw_width = misc.width;
  mapdef.draw_height = misc.height;
  mapdef.overlap_percent = 5;
  mapdef.grid_inc_x = 0.0;
  mapdef.grid_inc_y = 0.0;

  if (options.coast)
    {
      mapdef.coasts = NVMAP_AUTO_COAST;
    }
  else
    {
      mapdef.coasts = NVMAP_NO_COAST;
    }

  mapdef.coast_thickness = 1;
  mapdef.landmask = options.mask;
  mapdef.landmask_color = options.mask_color;

  mapdef.border = 5;
  mapdef.coast_color = options.coast_color;
  mapdef.background_color = options.background_color;


  mapdef.initial_bounds.min_x = -180.0;
  mapdef.initial_bounds.min_y = -90.0;
  mapdef.initial_bounds.max_x = 180.0;
  mapdef.initial_bounds.max_y = 90.0;


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Make the map.

  map = new nvMap (this, &mapdef);

  map->setWhatsThis (mapText);


  //  Connect to the signals from the map class.

  connect (map, SIGNAL (mousePressSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotMousePress (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (mouseDoubleClickSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this,
           SLOT (slotMouseDoubleClick (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (mouseReleaseSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotMouseRelease (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (map, SIGNAL (preRedrawSignal (NVMAP_DEF)), this, SLOT (slotPreRedraw (NVMAP_DEF)));
  connect (map, SIGNAL (midRedrawSignal (NVMAP_DEF)), this, SLOT (slotMidRedraw (NVMAP_DEF)));
  connect (map, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotPostRedraw (NVMAP_DEF)));


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (map);
  vBox->addLayout (hBox);


  //  Set the toolbar button states based on saved options.

  bCoast->setChecked (options.coast);
  connect (bCoast, SIGNAL (clicked ()), this, SLOT (slotCoast ()));

  bMask->setChecked (options.mask);
  connect (bMask, SIGNAL (clicked ()), this, SLOT (slotMask ()));

  bChildren->setChecked (options.display_children);
  connect (bChildren, SIGNAL (clicked ()), this, SLOT (slotChildren ()));

  bFeatureInfo->setChecked (options.display_feature_info);
  connect (bFeatureInfo, SIGNAL (clicked ()), this, SLOT (slotFeatureInfo ()));

  bFeaturePoly->setChecked (options.display_feature_poly);
  connect (bFeaturePoly, SIGNAL (clicked ()), this, SLOT (slotFeaturePoly ()));


  setMainButtons (NVFalse);



  //  Right click popup menu

  popupMenu = new QMenu (map);

  popup0 = popupMenu->addAction (tr ("Close rectangle and zoom in"));
  connect (popup0, SIGNAL (triggered ()), this, SLOT (slotPopupMenu0 ()));
  popup1 = popupMenu->addAction (tr ("Discard rectangle"));
  connect (popup1, SIGNAL (triggered ()), this, SLOT (slotPopupMenu1 ()));
  popupMenu->addSeparator ();
  popup2 = popupMenu->addAction (tr ("Help"));
  connect (popup2, SIGNAL (triggered ()), this, SLOT (slotPopupHelp ()));



  //  Setup the file menu.

  QAction *openFilesAct = new QAction (tr ("&Open input files"), this);
  openFilesAct->setShortcut (tr ("Ctrl+O"));
  openFilesAct->setStatusTip (tr ("Open one or more input files"));
  connect (openFilesAct, SIGNAL (triggered ()), this, SLOT (slotOpenFiles ()));

  QAction *exitSaveAct = new QAction (tr ("&Save and exit"), this);
  exitSaveAct->setShortcut (tr ("Ctrl+S"));
  exitSaveAct->setStatusTip (tr ("Save changes and exit"));
  connect (exitSaveAct, SIGNAL (triggered ()), this, SLOT (slotExitSave ()));

  QAction *exitNoSaveAct = new QAction (tr ("&Exit without saving"), this);
  exitNoSaveAct->setShortcut (tr ("Ctrl+E"));
  exitNoSaveAct->setStatusTip (tr ("Exit without saving changes"));
  connect (exitNoSaveAct, SIGNAL (triggered ()), this, SLOT (slotExitNoSave ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (openFilesAct);
  fileMenu->addSeparator ();
  fileMenu->addAction (exitSaveAct);
  fileMenu->addAction (exitNoSaveAct);



  //  Setup the help menu.

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about areaCheck"));
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


  //  Set up the status bar.  I'm not using the default one due to positioning issues.

  mapStatus = new QStatusBar ();
  mapStatus->setSizeGripEnabled (FALSE);
  mapStatus->show ();
  vBox->setStretchFactor (hBox, 1);
  vBox->addWidget (mapStatus);

  geoLabel = new QLabel (" N 00 00 00.00  W 000 00 00.00 ", this);
  geoLabel->setAlignment (Qt::AlignCenter);
  geoLabel->setMinimumSize (geoLabel->sizeHint ());
  geoLabel->setToolTip (tr ("Latitude and Longitude"));

  chrtrLabel = new QLabel (" 00000.00 ", this);
  chrtrLabel->setAlignment (Qt::AlignCenter);
  chrtrLabel->setMinimumSize (chrtrLabel->sizeHint ());
  chrtrLabel->setToolTip (tr ("CHRTR/BAG value"));

  modeLabel = new QLabel (tr (" Define rectangular area "), this);
  modeLabel->setAlignment (Qt::AlignCenter);
  modeLabel->setMinimumSize (modeLabel->sizeHint ());
  modeLabel->setToolTip (tr ("Operational mode"));
  modeLabel->setAutoFillBackground (TRUE);
  modePalette = modeLabel->palette ();


  miscLabel = new QLabel (this);
  miscLabel->setAlignment (Qt::AlignCenter);
  miscLabel->setMinimumSize (miscLabel->sizeHint ());

  mapStatus->addWidget (geoLabel);
  mapStatus->addWidget (chrtrLabel);
  mapStatus->addWidget (modeLabel);
  mapStatus->addWidget (miscLabel, 1);
  mapStatus->setSizeGripEnabled (FALSE);
  mapStatus->setWhatsThis (statusBarText);


  //  Set the tracking timer to every 50 milliseconds.

  trackCursor = new QTimer (this);
  connect (trackCursor, SIGNAL (timeout ()), this, SLOT (slotTrackCursor ()));
  trackCursor->start (25);


  setFunctionCursor (misc.function);


  //  If we came in with an input file or an empty input file we want to desensitize the open file and the copy area button.

  if (input_file || empty_file)
    {
      bFile->setEnabled (FALSE);
      bCopyArea->setEnabled (FALSE);
    }


  //  If we came in with an input file we want to desensitize the rect and poly buttons.

  if (input_file)
    {
      bRect->setEnabled (FALSE);
      bPoly->setEnabled (FALSE);
    }


  //  If we had an input file on the command line...

  if (input_file || any_file)
    {
      //  Figure out what kind of file it is...

      if (fileName.endsWith (".trk"))
        {
          addOverlay (TRACK, fileName);
        }
      else if (fileName.endsWith (".shp") || fileName.endsWith (".SHP"))
        {
          addOverlay (SHAPE, fileName);
        }
      else if (fileName.endsWith (".ARE"))
        {
          addOverlay (ISS60_AREA, fileName);
        }
      else if (fileName.endsWith (".are"))
        {
          addOverlay (GENERIC_AREA, fileName);
        }
      else if (fileName.endsWith (".afs"))
        {
          addOverlay (ACE_AREA, fileName);
        }
      else if (fileName.endsWith (".zne") || fileName.endsWith (".ZNE"))
        {
          addOverlay (ISS60_AREA, fileName);
        }
      else if (fileName.endsWith (".tdz"))
        {
          addOverlay (GENERIC_AREA, fileName);
        }
      else if (fileName.endsWith (".tif"))
        {
          addOverlay (GEOTIFF, fileName);
        }
      else if (fileName.endsWith (".fin") || fileName.endsWith (".ch2"))
        {
          addOverlay (CHRTR, fileName);
        }
      else if (fileName.endsWith (".bag"))
        {
          addOverlay (BAG, fileName);
        }
      else if (fileName.endsWith (".bfd"))
        {
          addOverlay (FEATURE, fileName);
        }
      else if (fileName.endsWith (".yxz"))
        {
          addOverlay (YXZ, fileName);
        }
      else if (fileName.endsWith (".llz"))
        {
          addOverlay (LLZ_DATA, fileName);
        }
      else if (fileName.endsWith (".txt") && (QFileInfo (fileName).baseName ().mid (2, 2) == "MP" ||
					      QFileInfo (fileName).baseName ().mid (2, 2) == "MD"))
        {
          addOverlay (OPTECH_FLT, fileName);
        }
      else
	{
	  QMessageBox::warning (this, tr ("Unknown file type"), tr ("This file had an unknown type:\n") + fileName);
	}
    }
}



areaCheck::~areaCheck ()
{
}



//  This function allocates memory for, and populates, the overlay structure.

void 
areaCheck::addOverlay (NV_INT32 k, QString filename)
{
  misc.overlays[k] = (OVERLAY *) realloc (misc.overlays[k], sizeof (OVERLAY) * (misc.num_overlays[k] + 1));
  if (misc.overlays[k] == NULL) 
    {
      perror (tr ("Allocating overlay memory").toAscii ());
      exit (-1);
    }


  //  Check for CHRTR2 subtype.

  if (filename.endsWith (".ch2"))
    {
      misc.overlays[k][misc.num_overlays[k]].chrtr2 = NVTrue;
    }
  else
    {
      misc.overlays[k][misc.num_overlays[k]].chrtr2 = NVFalse;
    }

  misc.overlays[k][misc.num_overlays[k]].count = 0;
  misc.overlays[k][misc.num_overlays[k]].x = NULL;
  misc.overlays[k][misc.num_overlays[k]].y = NULL;
  misc.overlays[k][misc.num_overlays[k]].modified = NVFalse;
  misc.overlays[k][misc.num_overlays[k]].active = NVTrue;
  misc.overlays[k][misc.num_overlays[k]].color = options.color[k];


  strcpy (misc.overlays[k][misc.num_overlays[k]].filename, filename.toAscii ());
  misc.num_overlays[k]++;
}



//  This function is where we link to other (non-parent or child) ABE programs.

void 
areaCheck::slotLink ()
{
  //  If we're already linked, unlink.

  if (misc.linked)
    {
      //  Unregister this application.

      unregisterABE (misc.abeRegister, misc.abe_register, abe_register_group, active_window_id);


      abe_register_group = -1;


      bLink->setToolTip (tr ("Connect to other ABE applications"));
      bLink->setWhatsThis (linkText);
      bLink->setIcon (QIcon (":/icons/unlink.xpm"));


      //  Unset the link flag.

      misc.linked = NVFalse;


      //  Remove the group id from the title

      QString title;
      title.sprintf ("%s", VERSION);
      this->setWindowTitle (title);


      //  Reset the main tool bar buttons

      setMainButtons (NVTrue);
    }
  else
    {
      //  Get the ABE_REGISTER shared memory area.

      registerABE *registerABEDialog = new registerABE (this, "areaCheck", active_window_id, NULL, &misc.abeRegister,
							&misc.abe_register);


      //  Connect to the other program (or create a group if none exist).

      connect (registerABEDialog, SIGNAL (keySetSignal (NV_INT32)), this, SLOT (slotRegisterABEKeySet (NV_INT32)));
    }
}



//  Callback from registering with a group.

void 
areaCheck::slotRegisterABEKeySet (NV_INT32 key)
{
  if (key >= 0)
    {
      abe_register_group = key;


      QString title;
      title.sprintf ("areaCheck (ABE group %02d)", key);
      this->setWindowTitle (title);


      misc.linked = NVTrue;


      bLink->setToolTip (tr ("Disconnect from other ABE applications"));
      bLink->setWhatsThis (unlinkText);
      bLink->setIcon (QIcon (":/icons/link.xpm"));
    }
}



//  Set all of the toolbar button states (either enabled or not)

void 
areaCheck::setMainButtons (NV_BOOL enabled)
{
  //  Get the map definition from the map object

  NVMAP_DEF l_mapdef = map->getMapdef ();
  if (enabled && l_mapdef.zoom_level >= 1)
    {
      bZoomOut->setEnabled (enabled);
    }
  else
    {
      bZoomOut->setEnabled (FALSE);
    }

  bZoomIn->setEnabled (enabled);
  bPrefs->setEnabled (enabled);
  bExitSave->setEnabled (enabled);
  bExitNoSave->setEnabled (enabled);
  bFile->setEnabled (enabled);
  bRect->setEnabled (enabled);
  bPoly->setEnabled (enabled);
  bCoast->setEnabled (enabled);
  bMask->setEnabled (enabled);
  bLink->setEnabled (enabled);


  NV_BOOL mod_overlays = NVFalse;

  for (NV_INT32 k = 0 ; k < MOD_TYPES ; k++)
    {
      if (misc.num_overlays[k])
        {
          mod_overlays = NVTrue;
          break;
        }
    }

  if (!enabled || (enabled && !mod_overlays))
    {
      bMoveArea->setEnabled (FALSE);
      bCopyArea->setEnabled (FALSE);
      bMoveVertex->setEnabled (FALSE);
      bDeleteVertex->setEnabled (FALSE);
      bEditVertex->setEnabled (FALSE);
      bAddVertex->setEnabled (FALSE);
    }
  else
    {
      bMoveArea->setEnabled (TRUE);
      bCopyArea->setEnabled (TRUE);
      bMoveVertex->setEnabled (TRUE);
      bDeleteVertex->setEnabled (TRUE);
      bEditVertex->setEnabled (TRUE);
      bAddVertex->setEnabled (TRUE);
    }


  //  The stop button is only enabled during drawing

  bStop->setEnabled (!enabled);


  for (NV_INT32 k = 0 ; k < NUM_TYPES ; k++)
    {
      if (enabled && misc.num_overlays[k])
        {
          misc.overlayButton[k]->setEnabled (TRUE);
        }
      else
        {
          misc.overlayButton[k]->setEnabled (FALSE);
        }
    }


  NV_BOOL features = NVFalse;
  for (NV_INT32 i = 0 ; i < misc.num_overlays[FEATURE] ; i++)
    {
      if (misc.overlays[FEATURE][i].active)
        {
          features = NVTrue;
          break;
        }
    }


  //  No point in enabling these if we're not displaying feature files.

  if (enabled && features)
    {
      bChildren->setEnabled (TRUE);
      bFeatureInfo->setEnabled (TRUE);
      bFeaturePoly->setEnabled (TRUE);
    }
  else
    {
      bChildren->setEnabled (FALSE);
      bFeatureInfo->setEnabled (FALSE);
      bFeaturePoly->setEnabled (FALSE);
    }
}



//  Clear all of the movable objects

void 
areaCheck::discardMovableObjects ()
{
  map->closeMovingPath (&mv_marker);
  map->closeMovingPath (&mv_polygon);
  map->closeMovingPath (&mv_tracker);
  map->discardRubberbandRectangle (&rb_rectangle);
  map->discardRubberbandPolygon (&rb_polygon);
}



//  Left mouse press function.  Called from the map's mouse press callback.

void 
areaCheck::leftMouse (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  QString string;
  NV_INT32 count, *px, *py;
  NV_FLOAT64 *mx, *my;
  NV_F64_XYMBR bounds;


  //  If the popup menu is up discard this mouse press

  if (popup_active)
    {
      popup_active = NVFalse;
      return;
    }


  //  Actions based on the active function.  If there is an active rectangle or polygon this will be the second
  //  mouse press and we want to perform the operation.  If there isn't a rectangle or polygon present then this
  //  is the first mouse press and we want to start the rectangle or polygon.  In other cases we set a flag to
  //  indicate that we have started the operation.

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);

          bounds.min_x = 999999.0;
          bounds.min_y = 999999.0;
          bounds.max_x = -999999.0;
          bounds.max_y = -999999.0;
          for (NV_INT32 i = 0 ; i < 4 ; i++)
            {
              if (mx[i] < bounds.min_x) bounds.min_x = mx[i];
              if (my[i] < bounds.min_y) bounds.min_y = my[i];
              if (mx[i] > bounds.max_x) bounds.max_x = mx[i];
              if (my[i] > bounds.max_y) bounds.max_y = my[i];
            }

	  discardMovableObjects ();
          map->zoomIn (bounds);
          bZoomOut->setEnabled (TRUE);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, Qt::black, 2, Qt::SolidLine);
        }
      break;


    case DEFINE_RECT_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);
          count = 4;

          output_file (mx, my, count);

          redrawMap ();
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, Qt::black, 2, Qt::SolidLine);
        }
      break;


    case DEFINE_POLY_AREA:
      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          map->vertexRubberbandPolygon (rb_polygon, lon, lat);
        }
      else
        {
          map->anchorRubberbandPolygon (&rb_polygon, lon, lat, Qt::black, 2, NVTrue, Qt::SolidLine);
        }
      break;


    case MOVE_AREA:
      if (area_moving)
        {
          for (NV_INT32 i = 0 ; i < new_overlay.count ; i++)
            {
              misc.overlays[cur_type][cur_area].x[i] = new_overlay.x[i];
              misc.overlays[cur_type][cur_area].y[i] = new_overlay.y[i];
            }
          misc.overlays[cur_type][cur_area].modified = NVTrue;

          area_moving = NVFalse;
          map->closeMovingPolygon (&mv_polygon);
          redrawMap ();
        }
      else
        {
          area_moving = NVTrue;
        }
      break;


    case COPY_AREA:
      if (area_copying)
        {
          output_file (new_overlay.x, new_overlay.y, new_overlay.count);

          area_copying = NVFalse;
          map->closeMovingPolygon (&mv_polygon);
        }
      else
        {
          area_copying = NVTrue;
        }
      break;


    case MOVE_VERTEX:
      if (vertex_moving)
        {
          for (NV_INT32 j = 0 ; j < new_overlay.count - 2 ; j++)
            {
              misc.overlays[cur_type][cur_area].x[j] = new_overlay.x[j];
              misc.overlays[cur_type][cur_area].y[j] = new_overlay.y[j];
            }

          misc.overlays[cur_type][cur_area].x[new_overlay.count - 2] = lon;
          misc.overlays[cur_type][cur_area].y[new_overlay.count - 2] = lat;
          misc.overlays[cur_type][cur_area].x[new_overlay.count - 1] = misc.overlays[cur_type][cur_area].x[0];
          misc.overlays[cur_type][cur_area].y[new_overlay.count - 1] = misc.overlays[cur_type][cur_area].y[0];


          misc.overlays[cur_type][cur_area].modified = NVTrue;

          vertex_moving = NVFalse;

          map->discardRubberbandPolygon (&rb_polygon);

          redrawMap ();
        }
      else
        {
          vertex_moving = NVTrue;
          miscLabel->setText (tr ("Left click to place, middle click to abort"));
          map->setCursor (Qt::CrossCursor);


          new_overlay.count = misc.overlays[cur_type][cur_area].count;


          //  Make a temporary polygon with the vertices reordered based on the selected vertex

          new_overlay.x = (NV_FLOAT64 *) realloc (new_overlay.x, sizeof (NV_FLOAT64) * new_overlay.count);
          new_overlay.y = (NV_FLOAT64 *) realloc (new_overlay.y, sizeof (NV_FLOAT64) * new_overlay.count);
              
          if (new_overlay.y == NULL)
            {
              perror (tr ("Allocating move vertex memory").toAscii ());
              exit (-1);
            }

          for (NV_INT32 j = 0 ; j < new_overlay.count ; j++)
            {
              new_overlay.x[j] = misc.overlays[cur_type][cur_area].x[(cur_vertex + j + 1) % (new_overlay.count - 1)];
              new_overlay.y[j] = misc.overlays[cur_type][cur_area].y[(cur_vertex + j + 1) % (new_overlay.count - 1)];
            }


          map->anchorRubberbandPolygon (&rb_polygon, new_overlay.x[0], new_overlay.y[0], Qt::black, 2, NVTrue, Qt::SolidLine);


          //  We use new_overlay.count - 2 since the last point is being redefined.

          for (NV_INT32 j = 1 ; j < new_overlay.count - 2 ; j++)
            {
              map->vertexRubberbandPolygon (rb_polygon, new_overlay.x[j], new_overlay.y[j]);
            }
        }
      break;


    case DELETE_VERTEX:

      //  Since we are carrying an ending point in our polygons, a triangle will have 4 points.  We don't want to
      //  delete a point from a triangle.

      if (misc.overlays[cur_type][cur_area].count > 4)
        {
          NV_FLOAT64 *tmpx = (NV_FLOAT64 *) malloc (sizeof (NV_FLOAT64) * misc.overlays[cur_type][cur_area].count - 1);
          NV_FLOAT64 *tmpy = (NV_FLOAT64 *) malloc (sizeof (NV_FLOAT64) * misc.overlays[cur_type][cur_area].count - 1);

          if (tmpy == NULL)
            {
              perror (tr ("Allocating delete vertex memory").toAscii ());
              exit (-1);
            }


	  //  Remove the selected vertex and reallocate the memory for the object.

          NV_INT32 j = 0;
          for (NV_INT32 i = 0 ; i < misc.overlays[cur_type][cur_area].count ; i++)
            {
              if (cur_vertex == 0 || cur_vertex == (misc.overlays[cur_type][cur_area].count - 1))
                {
                  if (i != 0 && i != (misc.overlays[cur_type][cur_area].count - 1)) 
                    {
                      tmpx[j] = misc.overlays[cur_type][cur_area].x[i];
                      tmpy[j] = misc.overlays[cur_type][cur_area].y[i];
                      j++;
                    }
                }
              else
                {
                  if (cur_vertex != i) 
                    {
                      tmpx[j] = misc.overlays[cur_type][cur_area].x[i];
                      tmpy[j] = misc.overlays[cur_type][cur_area].y[i];
                      j++;
                    }
                }
            }
          tmpx[j] = tmpx[0];
          tmpy[j] = tmpy[0];

          misc.overlays[cur_type][cur_area].count--;
          misc.overlays[cur_type][cur_area].x = (NV_FLOAT64 *) realloc (misc.overlays[cur_type][cur_area].x, sizeof (NV_FLOAT64) * 
                                                                        misc.overlays[cur_type][cur_area].count);
          misc.overlays[cur_type][cur_area].y = (NV_FLOAT64 *) realloc (misc.overlays[cur_type][cur_area].y, sizeof (NV_FLOAT64) * 
                                                                        misc.overlays[cur_type][cur_area].count);

          for (NV_INT32 i = 0 ; i < misc.overlays[cur_type][cur_area].count ; i++)
            {
              misc.overlays[cur_type][cur_area].x[i] = tmpx[i];
              misc.overlays[cur_type][cur_area].y[i] = tmpy[i];
            }
          misc.overlays[cur_type][cur_area].modified = NVTrue;

          free (tmpx);
          free (tmpy);
        }

      map->closeMovingPath (&mv_marker);
      redrawMap ();
      break;


    case ADD_VERTEX:
      if (line_moving)
        {
          NV_FLOAT64 *tmpx = (NV_FLOAT64 *) malloc (sizeof (NV_FLOAT64) * (misc.overlays[cur_type][cur_area].count + 1));
          NV_FLOAT64 *tmpy = (NV_FLOAT64 *) malloc (sizeof (NV_FLOAT64) * (misc.overlays[cur_type][cur_area].count + 1));

          if (tmpy == NULL)
            {
              perror (tr ("Allocating add vertex memory").toAscii ());
              exit (-1);
            }


	  //  Increment the count and reallocate the memory for the object.

          NV_INT32 j = 0;
          for (NV_INT32 i = 0 ; i < misc.overlays[cur_type][cur_area].count ; i++)
            {
              tmpx[j] = misc.overlays[cur_type][cur_area].x[i];
              tmpy[j] = misc.overlays[cur_type][cur_area].y[i];
              j++;


              if (i == cur_line[0]) 
                {
                  tmpx[j] = lon;
                  tmpy[j] = lat;
                  j++;
                }
            }

          tmpx[j - 1] = tmpx[0];
          tmpy[j - 1] = tmpy[0];

          misc.overlays[cur_type][cur_area].count++;
          misc.overlays[cur_type][cur_area].x = (NV_FLOAT64 *) realloc (misc.overlays[cur_type][cur_area].x, sizeof (NV_FLOAT64) * 
                                                                        misc.overlays[cur_type][cur_area].count);
          misc.overlays[cur_type][cur_area].y = (NV_FLOAT64 *) realloc (misc.overlays[cur_type][cur_area].y, sizeof (NV_FLOAT64) * 
                                                                        misc.overlays[cur_type][cur_area].count);

          for (NV_INT32 i = 0 ; i < misc.overlays[cur_type][cur_area].count ; i++)
            {
              misc.overlays[cur_type][cur_area].x[i] = tmpx[i];
              misc.overlays[cur_type][cur_area].y[i] = tmpy[i];
            }
          misc.overlays[cur_type][cur_area].modified = NVTrue;

          free (tmpx);
          free (tmpy);

          line_moving = NVFalse;
          map->closeMovingPath (&mv_marker);
          redrawMap ();
        }
      else
        {
          line_moving = NVTrue;
          miscLabel->setText (tr ("Left click to place, middle click to abort"));
          map->setCursor (Qt::CrossCursor);
        }
      break;


    case EDIT_VERTEX:
      map->closeMovingPath (&mv_marker);


      //  Set up and display the edit vertex dialog.

      editVertexD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
      editVertexD->setModal (FALSE);
      editVertexD->setWindowTitle (tr ("Vertex"));


      QVBoxLayout *vbox = new QVBoxLayout (editVertexD);
      vbox->setMargin (5);
      vbox->setSpacing (5);

      NV_FLOAT64 deg, min, sec;
      NV_CHAR ltstring[25], lnstring[25], hem;

      strcpy (ltstring, fixpos (misc.overlays[cur_type][cur_area].y[cur_vertex], &deg, &min, &sec, &hem, POS_LAT, 
                                options.position_form));
      strcpy (lnstring, fixpos (misc.overlays[cur_type][cur_area].x[cur_vertex], &deg, &min, &sec, &hem, POS_LON, 
                                options.position_form));

      latEdit = new QLineEdit (editVertexD);
      latEdit->setToolTip (tr ("Edit latitude"));
      latEdit->setText (QString (ltstring));

      vbox->addWidget (latEdit, 0, 0);


      lonEdit = new QLineEdit (editVertexD);
      lonEdit->setToolTip (tr ("Edit longitude"));
      lonEdit->setText (QString (lnstring));

      vbox->addWidget (lonEdit, 1, 0);


      QHBoxLayout *actions = new QHBoxLayout (0);
      vbox->addLayout (actions);

      QPushButton *applyButton = new QPushButton (tr ("Apply"), editVertexD);
      applyButton->setToolTip (tr ("Apply vertex changes"));
      connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApplyVertex ()));
      actions->addWidget (applyButton, 1);

      actions->addStretch (10);

      QPushButton *closeButton = new QPushButton (tr ("Cancel"), editVertexD);
      closeButton->setToolTip (tr ("Discard vertex changes"));
      connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCancelVertex ()));
      actions->addWidget (closeButton, 1);


      editVertexD->show ();
      break;
    }
}



//  Middle mouse click.  Called from the map mouse press callback.  Middle mouse press is usually a discard operation.

void 
areaCheck::midMouse (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon __attribute__ ((unused)),
                     NV_FLOAT64 lat __attribute__ ((unused)))
{
  //  Actions based on the active function.

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
    case DEFINE_RECT_AREA:
      map->discardRubberbandRectangle (&rb_rectangle);
      break;

    case DEFINE_POLY_AREA:
      map->discardRubberbandPolygon (&rb_polygon);
      break;

    case MOVE_AREA:
      area_moving = NVFalse;
      map->closeMovingPolygon (&mv_polygon);
      break;

    case COPY_AREA:
      area_copying = NVFalse;
      map->closeMovingPolygon (&mv_polygon);
      break;

    case MOVE_VERTEX:
      vertex_moving = NVFalse;
      map->closeMovingPath (&mv_marker);
      break;

    case ADD_VERTEX:
      line_moving = NVFalse;
      map->closeMovingPath (&mv_marker);
      break;
    }
}



//  Double left mouse click.  Called from the map mouse double click callback.  This usually means that we
//  want to finalize an operation.

void 
areaCheck::slotMouseDoubleClick (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  NV_INT32 count, *px, *py;
  NV_FLOAT64 *mx, *my;


  //  Flip the double_click flag.  The right-click menu sets this to NVTrue so it will flip to NVFalse.
  //  Left-click sets it to NVFalse so it will filp to NVTrue;

  double_click = !double_click;


  //  Actions based on the active function

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
      break;

    case DEFINE_RECT_AREA:
      break;

    case DEFINE_POLY_AREA:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      if (double_click) count--;

      output_file (mx, my, count);

      redrawMap ();
      break;

    case MOVE_AREA:
      break;

    case COPY_AREA:
      break;

    case MOVE_VERTEX:
      break;

    case ADD_VERTEX:
      break;
    }
}



//  Right mouse press.  Called from the map mouse press callback.  This pops up the right click menu based on the active function.

void 
areaCheck::rightMouse (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  menu_cursor_lon = lon;
  menu_cursor_lat = lat;
  menu_mouse_event = e;

  QPoint pos (e->x (), e->y ());


  //  Popups need global positioning

  QPoint global_pos = map->mapToGlobal (pos);


  switch (misc.function)
    {
    case ZOOM_IN_AREA:
      popup0->setText (tr ("Close rectangle and zoom in"));
      popup1->setText (tr ("Discard rectangle"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DEFINE_RECT_AREA:
      popup0->setText (tr ("Close rectangle and save to file"));
      popup1->setText (tr ("Discard rectangle"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DEFINE_POLY_AREA:
      popup0->setText (tr ("Close polygon and save to file"));
      popup1->setText (tr ("Discard polygon"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case MOVE_AREA:
      popup0->setText (tr ("Move area to current location"));
      popup1->setText (tr ("Discard move"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case COPY_AREA:
      popup0->setText (tr ("Copy area to current location"));
      popup1->setText (tr ("Discard copy"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case MOVE_VERTEX:
      popup0->setText (tr ("Move vertex to current location"));
      popup1->setText (tr ("Discard move"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case ADD_VERTEX:
      popup0->setText (tr ("Add vertex at current location"));
      popup1->setText (tr ("Discard add"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DELETE_VERTEX:
      popup0->setText (tr ("Delete vertex at current location"));
      popup1->setText (tr ("Discard delete"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case EDIT_VERTEX:
      popup0->setText (tr ("Edit vertex at current location"));
      popup1->setText (tr ("Discard edit"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;
    }
}



//  User selected the first entry in the right click popup menu.

void 
areaCheck::slotPopupMenu0 ()
{
  popup_active = NVFalse;

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
    case DEFINE_RECT_AREA:
    case MOVE_AREA:
    case COPY_AREA:
    case MOVE_VERTEX:
    case ADD_VERTEX:
    case DELETE_VERTEX:
    case EDIT_VERTEX:
      leftMouse (menu_mouse_event, menu_cursor_lon, menu_cursor_lat);
      break;

    case DEFINE_POLY_AREA:

      //  Fake out slotMouseDoubleClick so that it won't decrement the point count.

      double_click = NVTrue;

      slotMouseDoubleClick (menu_mouse_event, menu_cursor_lon, menu_cursor_lat);
      break;
    }
}



//  User selected the second entry in the right click popup menu.

void 
areaCheck::slotPopupMenu1 ()
{
  popup_active = NVFalse;

  midMouse (menu_mouse_event, menu_cursor_lon, menu_cursor_lat);
}



//  User selected the help entry in the right click popup menu.

void 
areaCheck::slotPopupHelp ()
{
  switch (misc.function)
    {
    case ZOOM_IN_AREA:
      QWhatsThis::showText (QCursor::pos (), zoomInText, map);
      break;

    case DEFINE_RECT_AREA:
      QWhatsThis::showText (QCursor::pos (), rectText, map);
      break;

    case MOVE_AREA:
      QWhatsThis::showText (QCursor::pos (), moveAreaText, map);
      break;

    case COPY_AREA:
      QWhatsThis::showText (QCursor::pos (), copyAreaText, map);
      break;

    case MOVE_VERTEX:
      QWhatsThis::showText (QCursor::pos (), moveVertexText, map);
      break;

    case DELETE_VERTEX:
      QWhatsThis::showText (QCursor::pos (), deleteVertexText, map);
      break;

    case EDIT_VERTEX:
      QWhatsThis::showText (QCursor::pos (), editVertexText, map);
      break;

    case ADD_VERTEX:
      QWhatsThis::showText (QCursor::pos (), addVertexText, map);
      break;

    case DEFINE_POLY_AREA:
      QWhatsThis::showText (QCursor::pos (), polyText, map);
      break;
    }
  midMouse (menu_mouse_event, menu_cursor_lon, menu_cursor_lat);
}



//  Timer - timeout signal.  Very much like an X workproc.  This tracks the cursor in associated
//  programs not in this window.  This is active whenever the mouse leaves this window.

void
areaCheck::slotTrackCursor ()
{
  NV_CHAR            ltstring[25], lnstring[25];
  QString            geo_string;
  QString            string;
  static NV_INT32    prev_linked_command_num = 0;


  //  If we received a bunch of resize signals (see slotResize) and we're not currently drawing
  //  the map, we need to redraw it for (possibly) the final time.  This keeps us from redrawing
  //  repeatedly when someone resizes the window and has "Display content in resizing windows" turned
  //  on.

  if (!misc.drawing && need_redraw)
    {
      redrawMap ();
      need_redraw = NVFalse;
    }


  //  If we're drawing or we're not linked to another program there's no need to track the cursor.

  if (misc.drawing || !misc.linked) return;


  //  Get the cursor position from the linked program (where the mouse cursor is).

  NV_FLOAT64 lat = misc.abe_register->data[abe_register_group].cursor_position.y;
  NV_FLOAT64 lon = misc.abe_register->data[abe_register_group].cursor_position.x;


  //  Check for a command, make sure it's a new command (we haven't already done it), and make sure
  //  it's not one that we issued.

  if (misc.abe_register->command[abe_register_group].number != prev_linked_command_num && 
      misc.abe_register->command[abe_register_group].window_id != active_window_id)
    {
      prev_linked_command_num = misc.abe_register->command[abe_register_group].number;

      switch (misc.abe_register->command[abe_register_group].id)
	{
	case ZOOM_TO_MBR_COMMAND:

	  discardMovableObjects ();
	  map->zoomIn (misc.abe_register->command[abe_register_group].mbr);
	  bZoomOut->setEnabled (TRUE);

	  break;
	}
    }


  //  If the mouse cursor is not in this window and it has changed position since last time we want to update the status bar
  //  and draw the marker to indicate the cursor's geographic location in the linked program.

  if (misc.abe_register->data[abe_register_group].active_window_id != active_window_id && lat >= misc.displayed_area.min_y && 
      lat <= misc.displayed_area.max_y && lon >= misc.displayed_area.min_x && lon <= misc.displayed_area.max_x)
    {
      NV_FLOAT64 deg, min, sec;
      NV_CHAR    hem;


      strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));

      geo_string.sprintf ("Lat: %s  Lon: %s", ltstring, lnstring);

      geoLabel->setText (geo_string);


      QBrush b1;
      map->setMovingPath (&mv_tracker, marker, lon, lat, 2, Qt::black, b1, NVFalse, Qt::SolidLine);
    }
}



//  Mouse press signal from the map class.

void 
areaCheck::slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  if (!misc.drawing)
    {
      if (e->button () == Qt::LeftButton) leftMouse (e, lon, lat);
      if (e->button () == Qt::MidButton) midMouse (e, lon, lat);
      if (e->button () == Qt::RightButton) rightMouse (e, lon, lat);
    }
}



//  Mouse release signal from the map class.

void 
areaCheck::slotMouseRelease (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)),
                             NV_FLOAT64 lat __attribute__ ((unused)))
{
  if (!misc.drawing)
    {
      if (e->button () == Qt::LeftButton) popup_active = NVFalse;
      if (e->button () == Qt::MidButton) popup_active = NVFalse;
      if (e->button () == Qt::RightButton);
    }
}



//  Mouse move signal from the map class.

void
areaCheck::slotMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  NV_CHAR         ltstring[25], lnstring[25], hem;
  QString         string, geo_string, num_string, avg_string, min_string, max_string;
  NV_FLOAT64      deg, min, sec;
  NV_I32_COORD2   ixy;
  static NV_FLOAT64 start_x, start_y;
  static NV_BOOL  first = NVTrue;


  ixy.x = e->x ();
  ixy.y = e->y ();


  //  Get rid of the tracking cursor from slotTrackCursor () since, if we got a move signal, we're obviously in
  //  this window.  We only want to kill it if it already exists, otherwise we will be creating a new one (which
  //  we don't want to do).

  if (mv_tracker >= 0) map->closeMovingPath (&mv_tracker);


  //  Track the cursor position for other ABE programs (only if we're linked).

  if (misc.linked)
    {
      //  Let other ABE programs know which window the mouse cursor is in.

      misc.abe_register->data[abe_register_group].active_window_id = active_window_id;


      misc.abe_register->data[abe_register_group].cursor_position.y = lat;
      misc.abe_register->data[abe_register_group].cursor_position.x = lon;
    }



  //  Compute the northing and easting values

  if (lat >= misc.displayed_area.min_y && lat <= misc.displayed_area.max_y && 
      lon >= misc.displayed_area.min_x && lon <= misc.displayed_area.max_x)
    {
      strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));

      geo_string.sprintf (" %s  %s ", ltstring, lnstring);


      geoLabel->setText (geo_string);
    }


  //  Check to see if we're over a CHRTR file.  If so, grab the value and display it.

  QString chrtrString = (" ");

  for (NV_INT32 i = 0 ; i < misc.num_overlays[CHRTR] ; i++)
    {
      if (misc.overlays[CHRTR][i].active)
        {
          if (lat >= misc.overlays[CHRTR][i].mbr.min_y && lat <= misc.overlays[CHRTR][i].mbr.max_y &&
              lon >= misc.overlays[CHRTR][i].mbr.min_x && lon <= misc.overlays[CHRTR][i].mbr.max_x)
            {
              NV_INT32 hnd;

              if (misc.overlays[CHRTR][i].chrtr2)
                {
                  CHRTR2_HEADER header;
                  CHRTR2_RECORD record;

                  if ((hnd = chrtr2_open_file (misc.overlays[CHRTR][i].filename, &header, CHRTR2_READONLY)) >= 0)
                    {
                      if (!chrtr2_read_record_lat_lon (hnd, lat, lon, &record)) chrtrString.sprintf ("%0.2f", record.z);

                      chrtr2_close_file (hnd);

                      chrtrLabel->setText (chrtrString);
                    }
                }
              else
                {
                  CHRTR_HEADER header;
                  NV_FLOAT32 value;

                  if ((hnd = open_chrtr (misc.overlays[CHRTR][i].filename, &header)) >= 0)
                    {
                      if (get_chrtr_value (hnd, lat, lon, &value)) chrtrString.sprintf ("%0.2f", value);
		      
                      close_chrtr (hnd);

                      chrtrLabel->setText (chrtrString);
                    }
                }
            }
        }
    }



  //  Check to see if we're over a BAG file.  If so, grab the value and display it.

  QString bagString = (" ");

  for (NV_INT32 i = 0 ; i < misc.num_overlays[BAG] ; i++)
    {
      if (misc.overlays[BAG][i].active)
        {
          if (lat >= misc.overlays[BAG][i].mbr.min_y && lat <= misc.overlays[BAG][i].mbr.max_y &&
              lon >= misc.overlays[BAG][i].mbr.min_x && lon <= misc.overlays[BAG][i].mbr.max_x)
            {
              bagHandle bagHandle;
              NV_FLOAT32 value;

              if (bagFileOpen (&bagHandle, BAG_OPEN_READONLY, (u8 *) misc.overlays[BAG][i].filename) == BAG_SUCCESS)
                {
                  NV_INT32 x = qMin (NINT ((lon - misc.overlays[BAG][i].mbr.min_x) / 
                                           misc.overlays[BAG][i].x_bin_size_degrees), misc.overlays[BAG][i].bin_width - 1);
                  NV_INT32 y = qMin (NINT ((lat - misc.overlays[BAG][i].mbr.min_y) / 
                                           misc.overlays[BAG][i].y_bin_size_degrees), misc.overlays[BAG][i].bin_height - 1);

                  if (bagReadNode (bagHandle, y, x, Elevation, (void *) &value) == BAG_SUCCESS) bagString.sprintf ("%0.2f", -value);

                  bagFileClose (bagHandle);

                  chrtrLabel->setText (bagString);
                }
            }
        }
    }



  NV_FLOAT64 anc_lat, anc_lon, az, dist;


  //  Actions based on the active function.  These are usually drag operations for rectangles, lines, or polygons.
  //  Move and copy area actually move an entire object.

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
    case DEFINE_RECT_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->dragRubberbandRectangle (rb_rectangle, lon, lat);

          map->getRubberbandRectangleAnchor (rb_rectangle, &anc_lon, &anc_lat);

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, lon, &dist, &az);


          string.sprintf (tr (" Distance from anchor (m): %.2f").toAscii (), dist);
          miscLabel->setText (string);
        }
      break;

    case DEFINE_POLY_AREA:
      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          map->dragRubberbandPolygon (rb_polygon, lon, lat);

          map->getRubberbandPolygonCurrentAnchor (rb_polygon, &anc_lon, &anc_lat);

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, lon, &dist, &az);

 
          string.sprintf (tr (" Distance from anchor (m): %.2f").toAscii (), dist);
          miscLabel->setText (string);
        }
      break;

    case MOVE_AREA:
    case COPY_AREA:
      if (area_moving || area_copying)
        {
          NV_FLOAT64 diff_x = lon - start_x;
          NV_FLOAT64 diff_y = lat - start_y;

          for (NV_INT32 j = 0 ; j < new_overlay.count ; j++)
            {
              new_overlay.x[j] += diff_x;
              new_overlay.y[j] += diff_y;
            }

          
          map->setMovingPolygon (&mv_polygon, new_overlay.count, new_overlay.x, new_overlay.y, Qt::black, 2, NVFalse, Qt::SolidLine);

          start_x = lon;
          start_y = lat;
        }
      else
        {
          highlight_nearest_area (lat, lon);
          if (cur_area != -1 && cur_vertex != -1)
            {
              start_x = lon;
              start_y = lat;

              new_overlay.x = (NV_FLOAT64 *) realloc (new_overlay.x, sizeof (NV_FLOAT64) * misc.overlays[cur_type][cur_area].count);
              new_overlay.y = (NV_FLOAT64 *) realloc (new_overlay.y, sizeof (NV_FLOAT64) * misc.overlays[cur_type][cur_area].count);
              
              if (new_overlay.y == NULL)
                {
                  perror (tr ("Allocating move area memory").toAscii ());
                  exit (-1);
                }
              new_overlay.count = misc.overlays[cur_type][cur_area].count;
              strcpy (new_overlay.filename, misc.overlays[cur_type][cur_area].filename);

              for (NV_INT32 j = 0 ; j < new_overlay.count ; j++)
                {
                  new_overlay.x[j] = misc.overlays[cur_type][cur_area].x[j];
                  new_overlay.y[j] = misc.overlays[cur_type][cur_area].y[j];
                }
            }
        }

      string = QFileInfo (misc.overlays[cur_type][cur_area].filename).fileName ();
      miscLabel->setText (string);
  
      break;

    case MOVE_VERTEX:
      if (vertex_moving)
        {
          map->dragRubberbandPolygon (rb_polygon, lon, lat);
        }
      else
        {
          highlight_nearest_vertex (lat, lon);
        }

      start_x = lon;
      start_y = lat;
      break;

    case DELETE_VERTEX:
    case EDIT_VERTEX:
      if (vertex_moving)
        {
          map->setMovingPath (&mv_marker, marker, lon, lat, 2, Qt::black, marker_brush, NVFalse, Qt::SolidLine);
        }
      else
        {
          highlight_nearest_vertex (lat, lon);
        }
      start_x = lon;
      start_y = lat;

      string = QFileInfo (misc.overlays[cur_type][cur_area].filename).fileName ();
      miscLabel->setText (string);

      break;

    case ADD_VERTEX:
      if (line_moving)
        {
          map->setMovingPath (&mv_marker, marker, lon, lat, 2, Qt::black, marker_brush, NVFalse, Qt::SolidLine);
        }
      else
        {
          highlight_nearest_line (lat, lon);
          first = NVTrue;
        }

      start_x = lon;
      start_y = lat;

      string = QFileInfo (misc.overlays[cur_type][cur_area].filename).fileName ();
      miscLabel->setText (string);

      break;
    }
}



//  Resize signal from the map class.

void
areaCheck::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  if (!misc.drawing && !need_redraw)
    {
      redrawMap ();
    }
  else
    {
      //  If the windowing system is doing "Display content in resizing windows" we'll get about a million
      //  resize callbacks all stacked up.  What we want to do is kick off the first one and then let the
      //  trackCursor function finish up when we're no longer drawing the first one.

      need_redraw = NVTrue;
    }
}



//  Slot for signal from map class (prior to drawing coastline)

void 
areaCheck::slotPreRedraw (NVMAP_DEF l_mapdef)
{
  misc.drawing = NVTrue;


  //  Turn off all the toolbar buttons (except the Stop button) and set the wait cursor.

  setMainButtons (NVFalse);
  QApplication::setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();

  mapdef = l_mapdef;


  //  Set the new boundaries.

  misc.displayed_area.min_y = mapdef.bounds[mapdef.zoom_level].min_y;
  misc.displayed_area.min_x = mapdef.bounds[mapdef.zoom_level].min_x;
  misc.displayed_area.max_y = mapdef.bounds[mapdef.zoom_level].max_y;
  misc.displayed_area.max_x = mapdef.bounds[mapdef.zoom_level].max_x;


  //  Deal with dateline crossing.

  if (misc.displayed_area.min_x > misc.displayed_area.max_x)
    {
      //  Only adjust the lons if the displayed area does not cross the dateline.
            
      if ((misc.displayed_area.max_x > 0.0 && misc.displayed_area.min_x > 0.0) || 
          (misc.displayed_area.max_x < 0.0 && misc.displayed_area.min_x < 0.0))
        {
          NV_FLOAT64 temp = misc.displayed_area.min_x;
          misc.displayed_area.min_x = misc.displayed_area.max_x;
          misc.displayed_area.max_x = temp;
        }
    }


  //  Turn the land mask on or off based on the size of the area being displayed.

  if (options.mask && misc.displayed_area.max_x - misc.displayed_area.min_x < 5.0 &&
      misc.displayed_area.max_y - misc.displayed_area.min_y < 5.0)
    {
      map->setLandmask (NVTrue);
    }
  else
    {
      map->setLandmask (NVFalse);
    }


  //  Overlay any raster files that have been requested.  We want these under the coastline.

  overlayData (map, &misc, &options, NVTrue);
}



//  Slot for signal from map class (after drawing coastline and before grid drawing)

void 
areaCheck::slotMidRedraw (NVMAP_DEF l_mapdef __attribute__ ((unused)))
{
  //  Overlay any non-raster files that have been requested.  We want these over the coastline.

  overlayData (map, &misc, &options, NVFalse);
}



//  Local redrawMap call.  We want to get rid of all of the movable objects every time we redraw.

void
areaCheck::redrawMap ()
{
  discardMovableObjects ();
  map->redrawMap (NVTrue);
}



//  Turn coastline on or off.

void
areaCheck::slotCoast ()
{
  if (bCoast->isChecked ())
    {
      options.coast = NVTrue;
      map->setCoasts (NVMAP_AUTO_COAST);
    }
  else
    {
      options.coast = NVFalse;
      map->setCoasts (NVMAP_NO_COAST);
    }


  redrawMap ();
}



//  Turn land mask on or off.

void
areaCheck::slotMask ()
{
  if (bMask->isChecked ())
    {
      options.mask = NVTrue;
    }
  else
    {
      options.mask = NVFalse;
    }

  redrawMap ();
}



//  Using the keys to move around.

void
areaCheck::keyPressEvent (QKeyEvent *e)
{
  if (!misc.drawing)
    {
      switch (e->key ())
        {
        case Qt::Key_Left:
          map->moveMap (NVMAP_LEFT);
          break;

        case Qt::Key_Up:
        case Qt::Key_PageUp:
          map->moveMap (NVMAP_UP);
          break;

        case Qt::Key_Right:
          map->moveMap (NVMAP_RIGHT);
          break;

        case Qt::Key_Down:
        case Qt::Key_PageDown:
          map->moveMap (NVMAP_DOWN);
          break;

        default:
          return;
        }
      e->accept ();
    }
}



//  Somebody pressed the little X in the window decoration.  DOH!

void 
areaCheck::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit (-1);
}



//  Different ways to exit.

void 
areaCheck::slotExitSave ()
{
  slotExit (0);
}



void 
areaCheck::slotExitNoSave ()
{
  slotExit (1);
}



void 
areaCheck::slotExit (int id)
{
  if (!id)
    {
      slotQuit (0);
    }
  else
    {
      slotQuit (-1);
    }
}



void 
areaCheck::slotQuit (NV_INT32 status)
{
  void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);


  //  Save the user's defaults

  envout (&options, &misc, this);


  //  If status is zero we want to save the files that we have created or modified.

  if (!status)
    {
      //  Check for unsaved, modified areas

      for (NV_INT32 k = 0 ; k < MOD_TYPES ; k++)
        {
          for (NV_INT32 i = 0 ; i < misc.num_overlays[k] ; i++)
            {
              if (misc.overlays[k][i].modified)
                {
                  QString string = QFileInfo (misc.overlays[k][i].filename).fileName ();

                  FILE *fp;

                  if ((fp = fopen (misc.overlays[k][i].filename, "w")) != NULL)
                    {
                      switch (k)
                        {
                        case ISS60_AREA:
                          write_ISS60_ARE (fp, misc.overlays[k][i].filename, misc.overlays[k][i].x, misc.overlays[k][i].y, 
                                           misc.overlays[k][i].count);
                          break;

                        case GENERIC_AREA:
                        case GENERIC_ZONE:
                          write_generic (fp, misc.overlays[k][i].x, misc.overlays[k][i].y, misc.overlays[k][i].count);
                          break;

                        case ACE_AREA:
                          write_ace (fp, misc.overlays[k][i].x, misc.overlays[k][i].y, misc.overlays[k][i].count);
                          break;

                        case ISS60_ZONE:
                          write_ISS60_zne (fp, misc.overlays[k][i].filename, misc.overlays[k][i].x, misc.overlays[k][i].y, 
                                           misc.overlays[k][i].count);
                          break;
                        }
                    }
                }
            }
        }
    }


  //  Free the allocated memory.

  for (NV_INT32 k = 0 ; k < MOD_TYPES ; k++)
    {
      for (NV_INT32 i = 0 ; i < misc.num_overlays[k] ; i++)
        {
          free (misc.overlays[k][i].x);
          free (misc.overlays[k][i].y);
        }
      if (misc.num_overlays[k]) free (misc.overlays[k]);
    }


  if (new_overlay.count) 
    {
      free (new_overlay.x);
      free (new_overlay.y);
    }


  //  Get rid of the shared memory (with linked programs).

  if (misc.linked) unregisterABE (misc.abeRegister, misc.abe_register, abe_register_group, active_window_id);


  exit (status);
}



//  Post grid drawing signal from map class.  Mostly just cleanup work.

void 
areaCheck::slotPostRedraw (NVMAP_DEF mapdef __attribute__ ((unused)))
{
  misc.prev_mbr = misc.displayed_area;
  misc.drawing_canceled = NVFalse;


  //  Turn the toolbar buttons (except the Stop button) back on.

  setMainButtons (NVTrue);
  misc.drawing = NVFalse;

  prev_area = cur_area = prev_vertex = cur_vertex = prev_line[0] = cur_line[0] = prev_type = cur_type = -1;

  setFunctionCursor (misc.function);

  QApplication::restoreOverrideCursor ();
}



//  Turn on the map signals and redraw the map.

void 
areaCheck::initializeMaps ()
{
  map->enableSignals ();


  //  Get the size and other goodies

  mapdef = map->getMapdef ();

  redrawMap ();
}



//  Slot to open overlay files.

void 
areaCheck::slotOpenFiles ()
{
  QFileDialog fd (this, tr ("Input files"));
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, options.input_dir);


  //  Possible input file types.

  QStringList filters;
  filters << tr ("SHAPE (*.shp *.SHP)")
          << tr ("TRACK (*.trk)")
          << tr ("AREA (*.ARE *.are *.afs)")
          << tr ("ZONE (*.zne *.ZNE *.tdz)")
          << tr ("GEOTIFF (*.tif)")
          << tr ("CHRTR (*.fin *.ch2)")
          << tr ("BAG (*.bag)")
          << tr ("Binary Feature Data (*.bfd)")
          << tr ("YXZ (*.yxz)")
          << tr ("LLZ (*.llz)")
          << tr ("Optech flight file (*.txt)")
          << tr ("All files (*)");

  fd.setFilters (filters);
  fd.setFileMode (QFileDialog::ExistingFiles);

  fd.selectFilter (options.inputFilter);


  QStringList files;

  if (fd.exec () == QDialog::Accepted) 
    {
      files = fd.selectedFiles ();

      NV_BOOL raster = NVFalse;

      //  Loop through all of the selected files (may be more than one).

      for (QStringList::Iterator it = files.begin () ; it != files.end () ; ++it)
        {
	  //  Check to see if we already have this file loaded.

          NV_BOOL hit = NVFalse;
          for (NV_INT32 k = 0 ; k < NUM_TYPES ; k++)
            {
              for (NV_INT32 i = 0 ; i < misc.num_overlays[k] ; i++)
                {
                  if (!strcmp (misc.overlays[k][i].filename, (*it).toAscii ())) hit = NVTrue;
                }
            }


	  //  If the file isn't already loaded, find out what type it is.  GeoTIFF, CHRTR, and BAG files are
	  //  raster files so we have to do a bit more work for them.

          if (!hit)
            {
              if ((*it).endsWith (".trk"))
                {
                  addOverlay (TRACK, (*it));
                }
              else if ((*it).endsWith (".shp") || (*it).endsWith (".SHP"))
                {
                  addOverlay (SHAPE, (*it));
                }
              else if ((*it).endsWith (".ARE"))
                {
                  addOverlay (ISS60_AREA, (*it));
                }
              else if ((*it).endsWith (".are"))
                {
                  addOverlay (GENERIC_AREA, (*it));
                }
              else if ((*it).endsWith (".afs"))
                {
                  addOverlay (ACE_AREA, (*it));
                }
              else if ((*it).endsWith (".zne") || (*it).endsWith (".ZNE"))
                {
                  addOverlay (ISS60_ZONE, (*it));
                }
              else if ((*it).endsWith (".tdz"))
                {
                  addOverlay (GENERIC_ZONE, (*it));
                }
              else if ((*it).endsWith (".tif"))
                {
                  addOverlay (GEOTIFF, (*it));

		  raster = NVTrue;
                  NV_INT32 k = GEOTIFF;
                  NV_INT32 i = misc.num_overlays[k] - 1;


		  //  We need to get the center of the file in case we are looking at a large area
		  //  and only want to plot an asterisk.

		  GDALDataset *poDataset;
		  NV_FLOAT64 adfGeoTransform[6];

		  GDALAllRegister ();


		  poDataset = (GDALDataset *) GDALOpen (misc.overlays[k][i].filename, GA_ReadOnly);
		  if (poDataset != NULL)
		    {
		      if (poDataset->GetGeoTransform (adfGeoTransform) == CE_None)
			{
			  NV_FLOAT64 GeoTIFF_lon_step = adfGeoTransform[1];
			  NV_FLOAT64 GeoTIFF_lat_step = -adfGeoTransform[5];


			  NV_INT32 width = poDataset->GetRasterXSize ();
			  NV_INT32 height = poDataset->GetRasterYSize ();


			  NV_FLOAT64 GeoTIFF_wlon = adfGeoTransform[0];
			  NV_FLOAT64 GeoTIFF_nlat = adfGeoTransform[3];


			  NV_FLOAT64 GeoTIFF_slat = GeoTIFF_nlat - height * GeoTIFF_lat_step;
			  NV_FLOAT64 GeoTIFF_elon = GeoTIFF_wlon + width * GeoTIFF_lon_step;


			  misc.overlays[k][i].center.x = GeoTIFF_wlon + (GeoTIFF_elon - GeoTIFF_wlon) / 2.0;
			  if (GeoTIFF_wlon > GeoTIFF_elon) misc.overlays[k][i].center.x = GeoTIFF_wlon + ((GeoTIFF_elon + 360.0) - GeoTIFF_wlon) / 2.0;
			  misc.overlays[k][i].center.y = GeoTIFF_slat + (GeoTIFF_nlat - GeoTIFF_slat) / 2.0;

			  delete poDataset;
			}
		      else
			{
			  return;
			}
		    }
		  else
		    {
		      return;
		    }
                }
              else if ((*it).endsWith (".fin") || (*it).endsWith (".ch2"))
                {
                  addOverlay (CHRTR, (*it));
	
                  raster = NVTrue;
                  NV_INT32 k = CHRTR;
                  NV_INT32 i = misc.num_overlays[k] - 1;


		  //  We need to get the center of the file in case we are looking at a large area and only want to plot an asterisk.

                  if ((*it).endsWith (".ch2"))
                    {
                      CHRTR2_HEADER header;
                      NV_INT32 hnd = chrtr2_open_file (misc.overlays[k][i].filename, &header, CHRTR2_READONLY);

                      if (hnd < 0) return;


                      misc.overlays[k][i].x_bin_size_degrees = (NV_FLOAT64) header.lon_grid_size_degrees;
                      misc.overlays[k][i].y_bin_size_degrees = (NV_FLOAT64) header.lat_grid_size_degrees;
                      misc.overlays[k][i].mbr.min_x = (NV_FLOAT64) header.mbr.wlon;
                      misc.overlays[k][i].mbr.min_y = (NV_FLOAT64) header.mbr.slat;
                      misc.overlays[k][i].mbr.max_x = (NV_FLOAT64) header.mbr.elon;
                      misc.overlays[k][i].mbr.max_y = (NV_FLOAT64) header.mbr.nlat;
                      misc.overlays[k][i].bin_width = header.width;
                      misc.overlays[k][i].bin_height = header.height;

                      misc.overlays[k][i].center.x = header.mbr.wlon + (header.mbr.elon - header.mbr.wlon) / 2.0;
                      if (header.mbr.wlon > header.mbr.elon) misc.overlays[k][i].center.x = header.mbr.wlon + ((header.mbr.elon + 360.0) - header.mbr.wlon) / 2.0;
                      misc.overlays[k][i].center.y = header.mbr.slat + (header.mbr.nlat - header.mbr.slat) / 2.0;

                      chrtr2_close_file (hnd);
                    }
                  else
                    {
                      CHRTR_HEADER header;
                      NV_INT32 hnd = open_chrtr (misc.overlays[k][i].filename, &header);

                      if (hnd < 0) return;


                      misc.overlays[k][i].x_bin_size_degrees = (NV_FLOAT64) header.grid_minutes / 60.0;
                      misc.overlays[k][i].y_bin_size_degrees = (NV_FLOAT64) header.grid_minutes / 60.0;
                      misc.overlays[k][i].mbr.min_x = (NV_FLOAT64) header.wlon;
                      misc.overlays[k][i].mbr.min_y = (NV_FLOAT64) header.slat;
                      misc.overlays[k][i].mbr.max_x = (NV_FLOAT64) header.elon;
                      misc.overlays[k][i].mbr.max_y = (NV_FLOAT64) header.nlat;
                      misc.overlays[k][i].bin_width = header.width;
                      misc.overlays[k][i].bin_height = header.height;

                      misc.overlays[k][i].center.x = header.wlon + (header.elon - header.wlon) / 2.0;
                      if (header.wlon > header.elon) misc.overlays[k][i].center.x = header.wlon + ((header.elon + 360.0) - header.wlon) / 2.0;
                      misc.overlays[k][i].center.y = header.slat + (header.nlat - header.slat) / 2.0;

                      close_chrtr (hnd);
                    }
                }
               else if ((*it).endsWith (".bag"))
                {
                  addOverlay (BAG, (*it));

		  raster = NVTrue;
                  NV_INT32 k = BAG;
                  NV_INT32 i = misc.num_overlays[k] - 1;


		  //  We need to get the center of the file in case we are looking at a large area
		  //  and only want to plot an asterisk.

		  bagHandle bagHandle;
		  bagError err;

		  if ((err = bagFileOpen (&bagHandle, BAG_OPEN_READONLY, (u8 *) misc.overlays[k][i].filename)) != BAG_SUCCESS)
		    {
		      u8 *errstr;

		      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS)
			{
			  QString string;
			  string.sprintf (tr ("Error opening BAG file : %s").toAscii (), errstr);
			  QMessageBox::warning (this, tr ("areaCheck Error"), string);
			}
		      return;
		    }


		  SPATIAL_REPRESENTATION_INFO spatial_representation_info;
		  GetSpatialRepresentationInfo (bagGetDataPointer (bagHandle)->metadata, &spatial_representation_info);

		  if (strcmp (spatial_representation_info.dimensionName[0], "row"))
		    {
		      QMessageBox::warning (this, tr ("areaCheck Error"), 
					    tr ("Sorry, I can't handle anything other than row/column data at present."));
		      return;
		    }

		  if (spatial_representation_info.numberOfDimensions != 2)
		    {
		      QMessageBox::warning (this, tr ("areaCheck Error"), tr ("Sorry, I only handle 2D BAGs."));
		      return;
		    }


		  REFERENCE_SYSTEM_INFO reference_system_info;
		  GetReferenceSystemInfo (bagGetDataPointer (bagHandle)->metadata, &reference_system_info);


		  if (strcmp (reference_system_info.projection, "Geodetic"))
		    {
		      QMessageBox::warning (this, tr ("areaCheck Error"), tr ("Sorry, I only handle Geodetic BAGs."));
		      return;
		    }

		  misc.overlays[k][i].bin_width = bagGetDataPointer (bagHandle)->def.ncols;
		  misc.overlays[k][i].bin_height = bagGetDataPointer (bagHandle)->def.nrows;
		  misc.overlays[k][i].x_bin_size_degrees = bagGetDataPointer (bagHandle)->def.nodeSpacingX;
		  misc.overlays[k][i].y_bin_size_degrees = bagGetDataPointer (bagHandle)->def.nodeSpacingY;
		  misc.overlays[k][i].mbr.min_x = bagGetDataPointer (bagHandle)->def.swCornerX;
		  misc.overlays[k][i].mbr.min_y = bagGetDataPointer (bagHandle)->def.swCornerY;
		  misc.overlays[k][i].mbr.max_x = misc.overlays[k][i].mbr.min_x + misc.overlays[k][i].bin_width * misc.overlays[k][i].x_bin_size_degrees;
		  misc.overlays[k][i].mbr.max_y = misc.overlays[k][i].mbr.min_y + misc.overlays[k][i].bin_height * misc.overlays[k][i].y_bin_size_degrees;

		  misc.overlays[k][i].center.x = misc.overlays[k][i].mbr.min_x + (misc.overlays[k][i].mbr.max_x - misc.overlays[k][i].mbr.min_x) / 2.0;
		  if (misc.overlays[k][i].mbr.min_x > misc.overlays[k][i].mbr.max_x) misc.overlays[k][i].center.x = misc.overlays[k][i].mbr.min_x + 
                    ((misc.overlays[k][i].mbr.max_x + 360.0) - misc.overlays[k][i].mbr.min_x) / 2.0;

		  misc.overlays[k][i].center.y = misc.overlays[k][i].mbr.min_y + (misc.overlays[k][i].mbr.max_y - misc.overlays[k][i].mbr.min_y) / 2.0;

		  bagFileClose (bagHandle);
                }
             else if ((*it).endsWith (".bfd"))
                {
                  addOverlay (FEATURE, (*it));
                }
              else if ((*it).endsWith (".yxz"))
                {
                  addOverlay (YXZ, (*it));
                }
              else if ((*it).endsWith (".llz"))
                {
                  addOverlay (LLZ_DATA, (*it));
                }
	      else if ((*it).endsWith (".txt") && (QFileInfo ((*it)).baseName ().mid (2, 2) == "MP" ||
						   QFileInfo ((*it)).baseName ().mid (2, 2) == "MD"))
		{
                  addOverlay (OPTECH_FLT, (*it));
		}
	      else
		{
		  QMessageBox::warning (this, tr ("Unknown file type"), tr ("This file had an unknown type:\n") + *(it));
		}
            }
        }


      //  If we had a raster file we need to redraw everything, otherwise we just overlay the vector data.

      if (raster)
	{
	  redrawMap ();
	}
      else
	{
	  setMainButtons (NVFalse);

          QApplication::setOverrideCursor (Qt::WaitCursor);
          qApp->processEvents ();
	  overlayData (map, &misc, &options, raster);
          QApplication::restoreOverrideCursor ();

          setMainButtons (NVTrue);
	}
    }


  //  Save the directory we were in and the file type filter.

  options.input_dir = fd.directory ().absolutePath ();
  options.inputFilter = fd.selectedFilter ();
}



//  This brings up the dialog to allow you to selectively display files of a given type.

void
areaCheck::slotType (int k)
{
  if (displayFile_dialog[k] == NULL)
    {
      displayFile_dialog[k] = new displayFile (this, &options, &misc, k);

      connect (displayFile_dialog[k], SIGNAL (dataChangedSignal ()), this, SLOT (slotDisplayFileDataChanged ()));
      connect (displayFile_dialog[k], SIGNAL (displayFileDialogClosedSignal (int)), this, SLOT (slotDisplayFileDialogClosed (int)));
    }
  else
    {
      displayFile_dialog[k]->raise ();
    }
}



//  Something changed in the display file dialog.

void 
areaCheck::slotDisplayFileDataChanged ()
{
  redrawMap ();
}



//  We closed the display file dialog.  It deletes on close so we want to set the address to NULL so we'll know
//  to recreate it next time.

void 
areaCheck::slotDisplayFileDialogClosed (int k)
{
  displayFile_dialog[k] = NULL;
}



//  Toggle display of feature info.

void
areaCheck::slotFeatureInfo ()
{
  if (bFeatureInfo->isChecked ())
    {
      options.display_feature_info = NVTrue;
    }
  else
    {
      options.display_feature_info = NVFalse;
    }
  redrawMap ();
}



//  Toggle display of feature children.

void
areaCheck::slotChildren ()
{
  if (bChildren->isChecked ())
    {
      options.display_children = NVTrue;
    }
  else
    {
      options.display_children = NVFalse;
    }
  redrawMap ();
}



//  Toggle display of feature polygons.

void
areaCheck::slotFeaturePoly ()
{
  if (bFeaturePoly->isChecked ())
    {
      options.display_feature_poly = NVTrue;
    }
  else
    {
      options.display_feature_poly = NVFalse;
    }
  redrawMap ();
}



//  Crank up the preferences dialog.

void 
areaCheck::slotPrefs ()
{
  prefs_dialog = new Prefs (this, &options, &misc);

  connect (prefs_dialog, SIGNAL (dataChangedSignal (NV_BOOL)), this, SLOT (slotPrefDataChanged (NV_BOOL)));
}



//  Changed some of the preferences

void 
areaCheck::slotPrefDataChanged (NV_BOOL mapRedrawFlag)
{
  map->setCoastColor (options.coast_color);
  map->setMaskColor (options.mask_color);
  map->setBackgroundColor (options.background_color);

  for (NV_INT32 k = 0 ; k < NUM_TYPES ; k++)
    {
      for (NV_INT32 i = 0 ; i < misc.num_overlays[k] ; i++)
        {
          misc.overlays[k][i].color = options.color[k];
        }
    }


  if (!misc.drawing && mapRedrawFlag) redrawMap ();
}



//  Someone pressed the Stop button so we want to clear and reset everything.

void
areaCheck::slotStop ()
{
  map->stopDrawing ();

  misc.drawing_canceled = NVTrue;

  setMainButtons (NVTrue);


  if (misc.function == DEFINE_RECT_AREA)
    {
      map->discardRubberbandRectangle (&rb_rectangle);
    }
  else
    {
      map->discardRubberbandPolygon (&rb_polygon);
    }

  setFunctionCursor (misc.function);

  setMainButtons (NVTrue);
  misc.drawing = NVFalse;
}



void
areaCheck::slotZoomOut ()
{
  NVMAP_DEF l_mapdef = map->getMapdef ();
  if (l_mapdef.zoom_level)
    {
      discardMovableObjects ();
      map->zoomOut ();

      l_mapdef = map->getMapdef ();
      if (!l_mapdef.zoom_level) bZoomOut->setEnabled (FALSE);
    }
}



void
areaCheck::slotZoomIn ()
{
  misc.function = ZOOM_IN_AREA;
  setFunctionCursor (misc.function);
}



void
areaCheck::about ()
{
  QMessageBox::about (this, VERSION, areaCheckAboutText);
}


void
areaCheck::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}



void
areaCheck::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}




void 
areaCheck::slotDefineRectArea ()
{
  misc.function = DEFINE_RECT_AREA;
  setFunctionCursor (misc.function);
}



void 
areaCheck::slotDefinePolyArea ()
{
  misc.function = DEFINE_POLY_AREA;
  setFunctionCursor (misc.function);
}



//  Set the cursor based on the active function.

void 
areaCheck::setFunctionCursor (NV_INT32 function)
{
  clearArea ();
  clearLine ();
  clearVertex ();

  switch (function)
    {
    case ZOOM_IN_AREA:
      miscLabel->setText (tr ("Left click to start rectangle, left click to close, middle click to abort"));
      modeLabel->setText (tr ("Zoom in"));
      modePalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      modeLabel->setPalette (modePalette);
      map->setCursor (zoomCursor);
      break;

    case DEFINE_RECT_AREA:
      miscLabel->setText (tr ("Left click to start rectangle, left click to close, middle click to abort"));
      modeLabel->setText (tr ("Define rectangular area"));
      map->setCursor (editRectCursor);
      break;

    case DEFINE_POLY_AREA:
      miscLabel->setText (tr ("Left click to define polygon vertices, double click to define last vertex, middle click to abort"));
      modeLabel->setText (tr ("Define polygonal area"));
      modePalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      modeLabel->setPalette (modePalette);
      map->setCursor (editPolyCursor);
      break;

    case MOVE_AREA:
      miscLabel->setText (tr ("Click on highlighted area, move area, left click to place, middle click to abort"));
      modeLabel->setText (tr ("Move area"));
      modePalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      modeLabel->setPalette (modePalette);
      map->setCursor (moveAreaCursor);
      break;

    case COPY_AREA:
      miscLabel->setText (tr ("Click on highlighted area, move area, left click to place, middle click to abort"));
      modeLabel->setText (tr ("Copy area"));
      modePalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      modeLabel->setPalette (modePalette);
      map->setCursor (copyAreaCursor);
      break;

    case MOVE_VERTEX:
      miscLabel->setText (tr ("Click to move highlighted vertex"));
      modeLabel->setText (tr ("Move vertex"));
      modePalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      modeLabel->setPalette (modePalette);
      map->setCursor (moveVertexCursor);
      break;

    case DELETE_VERTEX:
      miscLabel->setText (tr ("Click on highlighted vertex to delete"));
      modeLabel->setText (tr ("Delete vertex"));
      modePalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      modeLabel->setPalette (modePalette);
      map->setCursor (deleteVertexCursor);
      break;

    case EDIT_VERTEX:
      miscLabel->setText (tr ("Click on highlighted vertex to edit"));
      modeLabel->setText (tr ("Edit vertex"));
      modePalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      modeLabel->setPalette (modePalette);
      map->setCursor (editVertexCursor);
      break;

    case ADD_VERTEX:
      miscLabel->setText (tr ("Click to add vertex to highlighted line"));
      modeLabel->setText (tr ("Add vertex"));
      modePalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      modeLabel->setPalette (modePalette);
      map->setCursor (addVertexCursor);
      break;
    }
}



//  Write a generic area file.

void 
areaCheck::write_generic (FILE *fp, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count)
{
  NV_CHAR ltstring[25], lnstring[25], hem;
  NV_FLOAT64 deg, min, sec;


  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      //  Make sure we haven't created any duplicate points

      if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;

      strcpy (ltstring, fixpos (my[i], &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (mx[i], &deg, &min, &sec, &hem, POS_LON, options.position_form));

      fprintf (fp, "%s, %s\n", ltstring, lnstring);
    }
}



//  Write an Army Corps area file.

void 
areaCheck::write_ace (FILE *fp, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count)
{
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      //  Make sure we haven't created any duplicate points

      if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;

      fprintf (fp, "%.9f, %.9f\n", mx[i], my[i]);
    }
}



//  Write an ISS60 area file.

void 
areaCheck::write_ISS60_ARE (FILE *fp, NV_CHAR *fname, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count)
{
  /*
    NAME=liberty
    SCALE=2500.0
    POINT=P-1;30 10 10.00000N;088 45 35.00000W;  ;0.00;00000000;0
    POINT=P-2;30 09 45.00000N;088 45 35.00000W;  ;0.00;00000000;1
    POINT=P-3;30 09 45.00000N;088 44 40.00000W;  ;0.00;00000000;1
    POINT=P-4;30 10 10.00000N;088 44 40.00000W;  ;0.00;00000000;1
  */

  QFileInfo *f = new QFileInfo (fname);
  NV_CHAR name[128];
  strcpy (name, f->baseName ().toAscii ());
  
  fprintf (fp, "NAME=%s\n", name);

  fprintf (fp, "SCALE=0.0\n");

  
  NV_CHAR lthem, lnhem;
  NV_FLOAT64 ltdeg, ltmin, ltsec, lndeg, lnmin, lnsec;


  NV_INT32 j = 0;
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      //  Make sure we haven't created any duplicate points

      if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;

      if (i) j = 1;
      fixpos (my[i], &ltdeg, &ltmin, &ltsec, &lthem, POS_LAT, POS_HDMS);
      fixpos (mx[i], &lndeg, &lnmin, &lnsec, &lnhem, POS_LON, POS_HDMS);

      fprintf (fp, "POINT=P-%d;%02d %02d %08.5f%1c;%03d %02d %08.5f%1c;  ;0.00;00000000;%d\n", i + 1, 
               (NV_INT32) nearbyint (ltdeg), (NV_INT32) nearbyint (ltmin), ltsec, lthem, (NV_INT32) nearbyint (lndeg),
               (NV_INT32) nearbyint (lnmin), lnsec, lnhem, j);
    }
}



//  Write an ISS60 zone file.

void 
areaCheck::write_ISS60_zne (FILE *fp, NV_CHAR *fname, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count)
{
  /*
    NAME=liberty
    TIDE=1
    POINT=P-1;30 10 10.00000N;088 45 35.00000W;  ;0.00;00000000;0
    POINT=P-2;30 09 45.00000N;088 45 35.00000W;  ;0.00;00000000;1
    POINT=P-3;30 09 45.00000N;088 44 40.00000W;  ;0.00;00000000;1
    POINT=P-4;30 10 10.00000N;088 44 40.00000W;  ;0.00;00000000;1
  */

  QFileInfo *f = new QFileInfo (fname);
  NV_CHAR name[128];
  strcpy (name, f->baseName ().toAscii ());

  fprintf (fp, "NAME=%s\n", name);

  fprintf (fp, "TIDE=1\n");

  
  NV_CHAR lthem, lnhem;
  NV_FLOAT64 ltdeg, ltmin, ltsec, lndeg, lnmin, lnsec;


  NV_INT32 j = 0;
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      //  Make sure we haven't created any duplicate points

      if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;

      if (i) j = 1;
      fixpos (my[i], &ltdeg, &ltmin, &ltsec, &lthem, POS_LAT, POS_HDMS);
      fixpos (mx[i], &lndeg, &lnmin, &lnsec, &lnhem, POS_LON, POS_HDMS);

      fprintf (fp, "POINT=P-%d;%02d %02d %08.5f%1c;%03d %02d %08.5f%1c;  ;0.00;00000000;%d\n", i + 1,
               (NV_INT32) nearbyint (ltdeg), (NV_INT32) nearbyint (ltmin), ltsec, lthem, (NV_INT32) nearbyint (lndeg),
               (NV_INT32) nearbyint (lnmin), lnsec, lnhem, j);
    }
}



//  The following functions set the function type and the cursor.

void 
areaCheck::slotMoveArea ()
{
  misc.function = MOVE_AREA;
  setFunctionCursor (misc.function);
}



void 
areaCheck::slotCopyArea ()
{
  misc.function = COPY_AREA;
  setFunctionCursor (misc.function);
}



void 
areaCheck::slotMoveVertex ()
{
  misc.function = MOVE_VERTEX;
  setFunctionCursor (misc.function);
}



void 
areaCheck::slotDeleteVertex ()
{
  misc.function = DELETE_VERTEX;
  setFunctionCursor (misc.function);
}



void 
areaCheck::slotEditVertex ()
{
  misc.function = EDIT_VERTEX;
  setFunctionCursor (misc.function);
}



void 
areaCheck::slotAddVertex ()
{
  misc.function = ADD_VERTEX;
  setFunctionCursor (misc.function);
}



//  Highlight the area nearest to the mouse cursor.

void 
areaCheck::highlight_nearest_area (NV_FLOAT64 lat, NV_FLOAT64 lon)
{
  NV_FLOAT64 min_dist = 999999999.0, dist;

  cur_area = -1;
  cur_vertex = -1;
  cur_type = -1;

  for (NV_INT32 k = 0 ; k < MOD_TYPES ; k++)
    {
      for (NV_INT32 i = 0 ; i < misc.num_overlays[k] ; i++)
        {
          if (misc.overlays[k][i].active)
            {
              for (NV_INT32 j = 0 ; j < misc.overlays[k][i].count ; j++)
                {
                  NV_FLOAT64 ydif = fabs (lat - misc.overlays[k][i].y[j]);
                  NV_FLOAT64 xdif = fabs (lon - misc.overlays[k][i].x[j]);
                  dist = sqrt (ydif * ydif + xdif * xdif);
                  if (dist < min_dist) 
                    {
                      min_dist = dist;
                      cur_area = i;
                      cur_vertex = j;
                      cur_type = k;
                    }
                }
            }
        }
    }


  clearArea ();

  if (cur_area != -1) map->drawPolygon (misc.overlays[cur_type][cur_area].count, misc.overlays[cur_type][cur_area].x,
                                        misc.overlays[cur_type][cur_area].y, options.highlight_color, 2, NVTrue, Qt::SolidLine, NVTrue);


  prev_area = cur_area;
  prev_vertex = cur_vertex;
  prev_type = cur_type;
}



//  Highlight the vertex nearest to the mouse cursor.

void 
areaCheck::highlight_nearest_vertex (NV_FLOAT64 lat, NV_FLOAT64 lon)
{
  NV_FLOAT64 min_dist = 999999999.0, dist;

  cur_area = -1;
  cur_vertex = -1;
  cur_type = -1;

  for (NV_INT32 k = 0 ; k < MOD_TYPES ; k++)
    {
      for (NV_INT32 i = 0 ; i < misc.num_overlays[k] ; i++)
        {
          if (misc.overlays[k][i].active)
            {
              for (NV_INT32 j = 0 ; j < misc.overlays[k][i].count ; j++)
                {
                  NV_FLOAT64 ydif = fabs (lat - misc.overlays[k][i].y[j]);
                  NV_FLOAT64 xdif = fabs (lon - misc.overlays[k][i].x[j]);
                  dist = sqrt (ydif * ydif + xdif * xdif);
                  if (dist < min_dist) 
                    {
                      min_dist = dist;
                      cur_area = i;
                      cur_vertex = j;
                      cur_type = k;
                    }
                }
            }
        }
    }


  clearVertex ();

  if (cur_vertex != -1) map->setMovingPath (&mv_marker, marker, misc.overlays[cur_type][cur_area].x[cur_vertex],
                                            misc.overlays[cur_type][cur_area].y[cur_vertex], 2, Qt::black, marker_brush, NVFalse,
                                            Qt::SolidLine);

  prev_area = cur_area;
  prev_vertex = cur_vertex;
  prev_type = cur_type;
}



//  Highlight the line nearest to the mouse cursor.

void 
areaCheck::highlight_nearest_line (NV_FLOAT64 lat, NV_FLOAT64 lon)
{
  NV_FLOAT64 min_dist = 999999999.0, dist;

  cur_area = -1;
  cur_line[0] = -1;
  cur_type = -1;

  for (NV_INT32 k = 0 ; k < MOD_TYPES ; k++)
    {
      for (NV_INT32 i = 0 ; i < misc.num_overlays[k] ; i++)
        {
          if (misc.overlays[k][i].active)
            {
              for (NV_INT32 j = 0 ; j < misc.overlays[k][i].count ; j++)
                {
                  NV_FLOAT64 ydif = fabs (lat - misc.overlays[k][i].y[j]);
                  NV_FLOAT64 xdif = fabs (lon - misc.overlays[k][i].x[j]);
                  dist = sqrt (ydif * ydif + xdif * xdif);
                  if (dist < min_dist) 
                    {
                      min_dist = dist;
                      cur_area = i;
                      cur_line[0] = j;
                      cur_type = k;
                    }
                }
            }
        }
    }


  clearLine ();


  if (cur_line[0] == (misc.overlays[cur_type][cur_area].count - 1)) cur_line[0] = misc.overlays[cur_type][cur_area].count - 2;

  cur_line[1] = cur_line[0] + 1;

  if (cur_line[0] != -1) map->drawLine (misc.overlays[cur_type][cur_area].x[cur_line[0]], misc.overlays[cur_type][cur_area].y[cur_line[0]], 
                                        misc.overlays[cur_type][cur_area].x[cur_line[0] + 1],
                                        misc.overlays[cur_type][cur_area].y[cur_line[0] + 1], 
                                        options.highlight_color, 2, NVTrue, Qt::SolidLine);

  prev_area = cur_area;
  prev_line[0] = cur_line[0];
  prev_line[1] = cur_line[1];
  prev_type = cur_type;
}



void 
areaCheck::clearLine ()
{
  if (prev_line[0] != -1) 
    {
      QColor clr = options.color[misc.overlays[prev_type][prev_area].type];

      map->drawLine (misc.overlays[prev_type][prev_area].x[prev_line[0]], misc.overlays[prev_type][prev_area].y[prev_line[0]], 
                     misc.overlays[prev_type][prev_area].x[prev_line[1]], misc.overlays[prev_type][prev_area].y[prev_line[1]], 
                     clr, 2, NVTrue, Qt::SolidLine);
    }
}



void 
areaCheck::clearVertex ()
{
  if (prev_vertex != -1) 
    {
      map->closeMovingPath (&mv_marker);

      prev_vertex = -1;
    }
}



void 
areaCheck::clearArea ()
{
  if (prev_area != -1)
    {
      QColor clr = options.color[misc.overlays[prev_type][prev_area].type];

      map->drawPolygon (misc.overlays[prev_type][prev_area].count, misc.overlays[prev_type][prev_area].x,
                        misc.overlays[prev_type][prev_area].y, clr, 2, NVTrue, Qt::SolidLine, NVTrue);
    }
}



void 
areaCheck::slotApplyVertex ()
{
  NV_CHAR lat[50];
  NV_CHAR lon[50];

  strcpy (lat, latEdit->text ().toAscii ());

  posfix (lat, &tmpLat, POS_LAT);

  strcpy (lon, lonEdit->text ().toAscii ());

  posfix (lon, &tmpLon, POS_LON);

  if (tmpLat < 90.0 && tmpLat > -90.0 && tmpLon < 180.0 && tmpLon > -180.0)
    {
      misc.overlays[cur_type][cur_area].x[cur_vertex] = tmpLon;
      misc.overlays[cur_type][cur_area].y[cur_vertex] = tmpLat;
    }
  misc.overlays[cur_type][cur_area].modified = NVTrue;

  editVertexD->close ();

  redrawMap ();
}



void 
areaCheck::slotCancelVertex ()
{
  editVertexD->close ();

  redrawMap ();
}



void 
areaCheck::output_file (NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count)
{
  FILE *fp;
  QFileDialog *fd = NULL;
  QString file;
  NV_CHAR fname[512];
  QStringList files;


  //  If we came in with an empty file name on the command line we want to use that name instead of asking the 
  //  user for a new name.

  if (empty_file)
    {
      file = fileName;
      bRect->setEnabled (FALSE);
      bPoly->setEnabled (FALSE);
    }
  else
    {
      fd = new QFileDialog (this, tr ("areaCheck output area file"));
      fd->setViewMode (QFileDialog::List);


      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
      //  This function is in the nvutility library.

      setSidebarUrls (fd, options.output_dir);


      QStringList filters;
      filters << tr ("ISS60 area file (*.ARE)")
              << tr ("Generic area file (*.are)")
              << tr ("Army Corps area file (*.afs)")
              << tr ("ISS60 zone file (*.zne *.ZNE)")
              << tr ("Generic zone file (*.tdz)");

      fd->setFilters (filters);
      fd->setFileMode (QFileDialog::AnyFile);
      fd->selectFilter (options.outputFilter);

      if (fd->exec () == QDialog::Accepted)
        {
          files = fd->selectedFiles ();

          file = files.at (0);

          options.output_dir = fd->directory ().absolutePath ();
        }
    }


  if (!file.isEmpty())
    {
      //  We don't need to do this if we supplied a name with a recognized extension.

      if (!file.endsWith (".ARE") && !file.endsWith (".are") &&  !file.endsWith (".afs") &&
          !file.endsWith (".zne") && !file.endsWith (".ZNE") &&
          !file.endsWith (".tdz"))
        {
          if (fd->selectedFilter ().contains ("*.ARE"))
            {
              file.append (".ARE");
            }
          else if (fd->selectedFilter ().contains ("*.are"))
            {
              file.append (".are");
            }
          else if (fd->selectedFilter ().contains ("*.afs"))
            {
              file.append (".afs");
            }
          else if (fd->selectedFilter ().contains ("*.zne") || fd->selectedFilter ().contains ("*.ZNE"))
            {
              file.append (".zne");
            }
          else
            {
              file.append (".tdz");
            }
        }

 
      strcpy (fname, file.toAscii ());

      if ((fp = fopen (fname, "w")) != NULL)
        {
          if (file.endsWith (".ARE"))
            {
              addOverlay (ISS60_AREA, file);
              write_ISS60_ARE (fp, fname, mx, my, count);
            }
          else if (file.endsWith (".are"))
            {
              addOverlay (GENERIC_AREA, file);
              write_generic (fp, mx, my, count);
            }
          else if (file.endsWith (".afs"))
            {
              addOverlay (ACE_AREA, file);
              write_ace (fp, mx, my, count);
            }
          else if (file.endsWith (".zne") || file.endsWith (".ZNE"))
            {
              addOverlay (ISS60_ZONE, file);
              write_ISS60_zne (fp, fname, mx, my, count);
            }
          else
            {
              addOverlay (GENERIC_ZONE, file);
              write_generic (fp, mx, my, count);
            }

          fclose (fp);
        }
    }
}
