//  geoSwath3D class.

/***************************************************************************\

    Module :        geoSwath3D

    Programmer :    Jan C. Depner

    Date :          01/13/10

    Purpose :       3D version of geoSwath

\***************************************************************************/

#include <unistd.h>
    
#include "geoSwath3D.hpp"
#include "geoSwath3DHelp.hpp"


void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);



geoSwath3D::geoSwath3D (int *argc, char **argv, QWidget *parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;


  NV_BOOL envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);
  void set_defaults (MISC *misc, OPTIONS *options, POINT_DATA *data, NV_BOOL restore);


  options.z_factor = 1.0;
  options.z_offset = 0.0;
  misc.no_edit = NVFalse;
  record_num = 0;


  QFileInfo fileInfo;
  NV_INT32 option_index = 0;
  while (NVTrue) 
    {
      static struct option long_options[] = {{"file", required_argument, 0, 0},
                                             {"record", required_argument, 0, 0},
                                             {"z_factor", required_argument, 0, 0},
                                             {"z_offset", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "n", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              misc.file = QString (optarg);
              fileInfo = QFileInfo (misc.file);
              this->setWindowTitle (QString ("geoSwath3D : ") + fileInfo.fileName ());
              break;

            case 1:
              sscanf (optarg, "%d", &record_num);
              break;

            case 2:
              sscanf (optarg, "%f", &options.z_factor);
              break;

            case 3:
              sscanf (optarg, "%f", &options.z_offset);
              break;
            }
          break;

        case 'n':
          misc.no_edit = NVTrue;
          break;
        }
    }


  //  This is so we can drag and drop files on the Desktop icon.

  if (*argc == 2 && misc.file == "")
    {
      misc.file = QString (argv[1]);
      fileInfo = QFileInfo (misc.file);
      this->setWindowTitle (QString ("geoSwath : ") + fileInfo.fileName ());
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


    //  Get the shared memory area.  If it exists, delete it.  We're using the same shared 
    //  memory structure as PFM_ABE so that we can talk to the same ancillary programs.  

  QString skey;
  skey.sprintf ("%d_abe", getpid ());

  misc.abeShare = new QSharedMemory (skey);

  if (!misc.abeShare->create (sizeof (ABE_SHARE), QSharedMemory::ReadWrite)) misc.abeShare->attach (QSharedMemory::ReadWrite);

  misc.abe_share = (ABE_SHARE *) misc.abeShare->data ();


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);
  setFocus (Qt::ActiveWindowFocusReason);
  

  //  Set a few defaults for startup

  polygon_flip = NVFalse;
  prev_poly_x = -1;
  prev_poly_y = -1;
  popup_active = NVFalse;
  double_click = NVFalse;
  prefs_dialog = NULL;
  mv_tracker = -1;
  mv_2D_tracker = -1;
  mv_measure_anchor = -1;
  mv_measure = -1;
  rb_polygon = -1;
  rb_measure = -1;
  rb_rectangle = -1;
  active_window_id = getpid ();
  start_ctrl_x = -1;
  start_ctrl_y = -1;
  rotate_index = 0;
  lock_point = 0;
  slicing = NVFalse;
  filterMessage_slider_count = -1;
  filter_active = NVFalse;
  file_opened = NVFalse;


  // 
  //                      Key flags are intialized here.
  //                      multiMode represents a waveform display mode in the waveMonitor program 
  //                      (0: nearest-neighbor, 1: single waveform).  multiNum refers to the number
  //                      of multiple waveforms at the current time to display.  This variable will
  //                      equal to MAX_STACK_POINTS if we are in nearest-neighbor mode.
  //

  if (!misc.abe_share->mwShare.waveMonitorRunning) misc.abe_share->mwShare.multiMode = 1;
  misc.abe_share->mwShare.multiNum = 0;

  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      multi_marker[i] = -1;
      misc.abe_share->mwShare.multiPresent[i] = -1;
    }


  save_z = NULL;


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


  //  Make the 2D "tracker" cursor painter path.

  tracker_2D = QPainterPath ();

  tracker_2D.moveTo (0, 0);
  tracker_2D.lineTo (30, 0);
  tracker_2D.lineTo (30, 20);
  tracker_2D.lineTo (0, 20);
  tracker_2D.lineTo (0, 0);

  tracker_2D.moveTo (0, 10);
  tracker_2D.lineTo (12, 10);

  tracker_2D.moveTo (30, 10);
  tracker_2D.lineTo (18, 10);

  tracker_2D.moveTo (15, 0);
  tracker_2D.lineTo (15, 6);

  tracker_2D.moveTo (15, 20);
  tracker_2D.lineTo (15, 14);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/geoSwath3D.png"));


  //  Define all of the cursors

  deleteRectCursor = QCursor (QPixmap (":/icons/delete_rect_cursor.xpm"), 3, 0);
  deletePolyCursor = QCursor (QPixmap (":/icons/delete_poly_cursor.xpm"), 3, 0);
  restoreRectCursor = QCursor (QPixmap (":/icons/restore_rect_cursor.xpm"), 11, 0);
  restorePolyCursor = QCursor (QPixmap (":/icons/restore_poly_cursor.xpm"), 11, 0);
  hotkeyPolyCursor = QCursor (QPixmap (":/icons/hotkey_poly_cursor.xpm"), 0, 19);
  keepPolyCursor = QCursor (QPixmap (":/icons/keep_poly_cursor.xpm"), 5, 0);
  rotateCursor = QCursor (QPixmap (":/icons/rotate_cursor.xpm"), 17, 17);
  zoomCursor = QCursor (QPixmap (":/icons/zoom_cursor.xpm"), 11, 11);
  maskCursor = QCursor (QPixmap (":/icons/mask_cursor.xpm"), 1, 1);
  measureCursor = QCursor (QPixmap (":/icons/measure_cursor.xpm"), 1, 1);
  filterMaskRectCursor = QCursor (QPixmap (":/icons/filter_mask_rect_cursor.xpm"), 1, 1);
  filterMaskPolyCursor = QCursor (QPixmap (":/icons/filter_mask_poly_cursor.xpm"), 1, 1);
  highlightPolyCursor = QCursor (QPixmap (":/icons/highlight_polygon_cursor.xpm"), 1, 1);
  clearPolyCursor = QCursor (QPixmap (":/icons/clear_polygon_cursor.xpm"), 1, 1);


  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++) ancillaryProgram[i] = NULL;


  //  Set all of the defaults

  set_defaults (&misc, &options, &data, NVFalse);


  misc.abe_share->ppid = misc.process_id;


  //  We have to get the icon size out of sequence (the rest of the options are read in env_in_out.cpp)
  //  so that we'll have the proper sized icons for the toolbars.  Otherwise, they won't be placed correctly.

  QSettings settings (misc.qsettings_org, misc.qsettings_app);
  settings.beginGroup (misc.qsettings_app);
  options.main_button_icon_size = settings.value (tr ("main button icon size"), options.main_button_icon_size).toInt ();
  settings.endGroup ();


  //  Create View toolbar

  toolBar[0] = new QToolBar (tr ("View tool bar"));
  toolBar[0]->setToolTip (tr ("View tool bar"));
  addToolBar (toolBar[0]);
  toolBar[0]->setObjectName (tr ("geoSwath3D View tool bar"));



  bOpen = new QToolButton (this);
  misc.button[OPEN_FILE_KEY] = bOpen;
  bOpen->setIcon (misc.buttonIcon[OPEN_FILE_KEY]);
  bOpen->setWhatsThis (openText);
  bOpen->setShortcut (options.buttonAccel[0]);
  connect (bOpen, SIGNAL (clicked ()), this, SLOT (slotOpen ()));
  toolBar[0]->addWidget (bOpen);

  bExit = new QToolButton (this);
  misc.button[QUIT_KEY] = bExit;
  bExit->setIcon (misc.buttonIcon[QUIT_KEY]);
  connect (bExit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  bExit->setWhatsThis (exitText);
  toolBar[0]->addWidget (bExit);

  bReset = new QToolButton (this);
  misc.button[RESET_KEY] = bReset;
  bReset->setIcon (misc.buttonIcon[RESET_KEY]);
  bReset->setWhatsThis (resetText);
  connect (bReset, SIGNAL (clicked ()), this, SLOT (slotReset ()));
  toolBar[0]->addWidget (bReset);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  bDisplayInvalid = new QToolButton (this);
  bDisplayInvalid->setIcon (QIcon (":/icons/displayinvalid.xpm"));
  bDisplayInvalid->setToolTip (tr ("Display invalid data"));
  bDisplayInvalid->setWhatsThis (displayInvalidText);
  bDisplayInvalid->setCheckable (TRUE);
  toolBar[0]->addWidget (bDisplayInvalid);

  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  flagIcon[0] = QIcon (":/icons/user_flag.xpm");
  flagIcon[1] = QIcon (":/icons/suspect.xpm");
  flagIcon[2] = QIcon (":/icons/flag_invalid.xpm");


  QMenu *flagMenu = new QMenu (this);

  QActionGroup *flagGrp = new QActionGroup (this);
  connect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));
  flagGrp->setExclusive (TRUE);

  flag[0] = flagMenu->addAction (tr ("Turn off data flags"));
  flagGrp->addAction (flag[0]);
  flag[0]->setIcon (flagIcon[0]);
  flag[0]->setCheckable (TRUE);

  flag[1] = flagMenu->addAction (tr ("Mark suspect data"));
  flagGrp->addAction (flag[1]);
  flag[1]->setIcon (flagIcon[1]);
  flag[1]->setCheckable (TRUE);

  flag[2] = flagMenu->addAction (tr ("Mark invalid data"));
  flagGrp->addAction (flag[2]);
  flag[2]->setIcon (flagIcon[2]);
  flag[2]->setCheckable (TRUE);

  bFlag = new QToolButton (this);
  bFlag->setIcon (QIcon (":/icons/user_flag.xpm"));
  bFlag->setToolTip (tr ("Toggle data flags on/off"));
  bFlag->setWhatsThis (flagText);
  bFlag->setPopupMode (QToolButton::InstantPopup);
  bFlag->setMenu (flagMenu);
  toolBar[0]->addWidget (bFlag);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  QButtonGroup *modeGrp = new QButtonGroup (this);
  connect (modeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotMode (int)));
  modeGrp->setExclusive (TRUE);


  bHighlightPoly = new QToolButton (this);
  misc.button[HIGHLIGHT_POLYGON_MODE_KEY] = bHighlightPoly;
  bHighlightPoly->setIcon (misc.buttonIcon[HIGHLIGHT_POLYGON_MODE_KEY]);
  modeGrp->addButton (bHighlightPoly, HIGHLIGHT_POLYGON);
  bHighlightPoly->setWhatsThis (highlightPolyText);
  bHighlightPoly->setCheckable (TRUE);
  toolBar[0]->addWidget (bHighlightPoly);

  bClearPoly = new QToolButton (this);
  misc.button[CLEAR_POLYGON_MODE_KEY] = bClearPoly;
  bClearPoly->setIcon (misc.buttonIcon[CLEAR_POLYGON_MODE_KEY]);
  modeGrp->addButton (bClearPoly, CLEAR_POLYGON);
  bClearPoly->setWhatsThis (clearPolyText);
  bClearPoly->setCheckable (TRUE);
  toolBar[0]->addWidget (bClearPoly);

  bClearHighlight = new QToolButton (this);
  misc.button[CLEAR_HIGHLIGHT_KEY] = bClearHighlight;
  bClearHighlight->setIcon (misc.buttonIcon[CLEAR_HIGHLIGHT_KEY]);
  bClearHighlight->setWhatsThis (clearHighlightText);
  connect (bClearHighlight, SIGNAL (clicked ()), this, SLOT (slotClearHighlight ()));
  toolBar[0]->addWidget (bClearHighlight);


  //  Create Utilities toolbar

  toolBar[1] = new QToolBar (tr ("Utilities tool bar"));
  toolBar[1]->setToolTip (tr ("Utilities tool bar"));
  addToolBar (toolBar[1]);
  toolBar[1]->setObjectName (tr ("geoSwath3D Utilities tool bar"));


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.xpm"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  toolBar[1]->addWidget (bPrefs);


  bLink = new QToolButton (this);
  bLink->setIcon (QIcon (":/icons/unlink.xpm"));
  bLink->setToolTip (tr ("Link to other ABE applications"));
  bLink->setWhatsThis (linkText);
  connect (bLink, SIGNAL (clicked ()), this, SLOT (slotLink ()));
  toolBar[1]->addWidget (bLink);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  toolBar[1]->addAction (bHelp);


  //  Create Edit toolbar

  toolBar[2] = new QToolBar (tr ("Edit tool bar"));
  toolBar[2]->setToolTip (tr ("Edit tool bar"));
  addToolBar (toolBar[2]);
  toolBar[2]->setObjectName (tr ("geoSwath3D Edit tool bar"));

  bStop = new QToolButton (this);
  bStop->setIcon (QIcon (":/icons/stop.xpm"));
  bStop->setToolTip (tr ("Stop drawing"));
  bStop->setWhatsThis (stopText);
  bStop->setEnabled (FALSE);
  connect (bStop, SIGNAL (clicked ()), this, SLOT (slotStop ()));
  toolBar[2]->addWidget (bStop);


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();


  bDeletePoint = new QToolButton (this);
  misc.button[DELETE_POINT_MODE_KEY] = bDeletePoint;
  bDeletePoint->setIcon (misc.buttonIcon[DELETE_POINT_MODE_KEY]);
  modeGrp->addButton (bDeletePoint, DELETE_POINT);
  bDeletePoint->setWhatsThis (deletePointText);
  bDeletePoint->setCheckable (TRUE);
  toolBar[2]->addWidget (bDeletePoint);


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();


  bDeleteRect = new QToolButton (this);
  misc.button[DELETE_RECTANGLE_MODE_KEY] = bDeleteRect;
  bDeleteRect->setIcon (misc.buttonIcon[DELETE_RECTANGLE_MODE_KEY]);
  modeGrp->addButton (bDeleteRect, DELETE_RECTANGLE);
  bDeleteRect->setWhatsThis (deleteRectText);
  bDeleteRect->setCheckable (TRUE);
  toolBar[2]->addWidget (bDeleteRect);

  bDeletePoly = new QToolButton (this);
  misc.button[DELETE_POLYGON_MODE_KEY] = bDeletePoly;
  bDeletePoly->setIcon (misc.buttonIcon[DELETE_POLYGON_MODE_KEY]);
  modeGrp->addButton (bDeletePoly, DELETE_POLYGON);
  bDeletePoly->setWhatsThis (deletePolyText);
  bDeletePoly->setCheckable (TRUE);
  toolBar[2]->addWidget (bDeletePoly);


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();


  bKeepPoly = new QToolButton (this);
  bKeepPoly->setIcon (QIcon (":/icons/keep_poly.xpm"));
  modeGrp->addButton (bKeepPoly, KEEP_POLYGON);
  bKeepPoly->setToolTip (tr ("Delete data outside polygon"));
  bKeepPoly->setWhatsThis (keepPolyText);
  bKeepPoly->setCheckable (TRUE);
  toolBar[2]->addWidget (bKeepPoly);


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();


  bRestoreRect = new QToolButton (this);
  bRestoreRect->setIcon (QIcon (":/icons/restore_rect.xpm"));
  modeGrp->addButton (bRestoreRect, RESTORE_RECTANGLE);
  bRestoreRect->setToolTip (tr ("Restore invalid data inside rectangle"));
  bRestoreRect->setWhatsThis (restoreRectText);
  bRestoreRect->setCheckable (TRUE);
  toolBar[2]->addWidget (bRestoreRect);

  bRestorePoly = new QToolButton (this);
  bRestorePoly->setIcon (QIcon (":/icons/restore_poly.xpm"));
  modeGrp->addButton (bRestorePoly, RESTORE_POLYGON);
  bRestorePoly->setToolTip (tr ("Restore invalid data inside polygon"));
  bRestorePoly->setWhatsThis (restorePolyText);
  bRestorePoly->setCheckable (TRUE);
  toolBar[2]->addWidget (bRestorePoly);


  bUndo = new QToolButton (this);
  misc.button[UNDO_KEY] = bUndo;
  bUndo->setIcon (misc.buttonIcon[UNDO_KEY]);
  bUndo->setToolTip (tr ("Undo last edit operation"));
  bUndo->setWhatsThis (undoText);
  connect (bUndo, SIGNAL (clicked ()), this, SLOT (slotUndo ()));
  toolBar[2]->addWidget (bUndo);


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();


  bHotkeyPoly = new QToolButton (this);
  misc.button[RUN_HOTKEY_POLYGON_MODE_KEY] = bHotkeyPoly;
  bHotkeyPoly->setIcon (misc.buttonIcon[RUN_HOTKEY_POLYGON_MODE_KEY]);
  modeGrp->addButton (bHotkeyPoly, HOTKEY_POLYGON);
  bHotkeyPoly->setWhatsThis (hotkeyPolyText);
  bHotkeyPoly->setCheckable (TRUE);
  toolBar[2]->addWidget (bHotkeyPoly);


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();


  bFilter = new QToolButton (this);
  misc.button[FILTER_KEY] = bFilter;
  bFilter->setIcon (misc.buttonIcon[FILTER_KEY]);
  bFilter->setWhatsThis (filterText);
  connect (bFilter, SIGNAL (clicked ()), this, SLOT (slotFilter ()));
  toolBar[2]->addWidget (bFilter);

  bFilterRectMask = new QToolButton (this);
  misc.button[RECTANGLE_FILTER_MASK_KEY] = bFilterRectMask;
  bFilterRectMask->setIcon (misc.buttonIcon[RECTANGLE_FILTER_MASK_KEY]);
  bFilterRectMask->setWhatsThis (filterRectMaskText);
  modeGrp->addButton (bFilterRectMask, RECT_FILTER_MASK);
  bFilterRectMask->setCheckable (TRUE);
  toolBar[2]->addWidget (bFilterRectMask);

  bFilterPolyMask = new QToolButton (this);
  misc.button[POLYGON_FILTER_MASK_KEY] = bFilterPolyMask;
  bFilterPolyMask->setIcon (misc.buttonIcon[POLYGON_FILTER_MASK_KEY]);
  bFilterPolyMask->setWhatsThis (filterPolyMaskText);
  modeGrp->addButton (bFilterPolyMask, POLY_FILTER_MASK);
  bFilterPolyMask->setCheckable (TRUE);
  toolBar[2]->addWidget (bFilterPolyMask);

  bClearMasks = new QToolButton (this);
  bClearMasks->setIcon (QIcon (":/icons/clear_filter_masks.xpm"));
  bClearMasks->setToolTip (tr ("Clear filter masks"));
  bClearMasks->setWhatsThis (clearMasksText);
  connect (bClearMasks, SIGNAL (clicked ()), this, SLOT (slotClearFilterMasks ()));
  toolBar[2]->addWidget (bClearMasks);


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();


  bMeasure = new QToolButton (this);
  bMeasure->setIcon (QIcon (":/icons/measure.xpm"));
  bMeasure->setToolTip (tr ("Measure distance, azimuth, and Z difference"));
  modeGrp->addButton (bMeasure, MEASURE);
  bMeasure->setWhatsThis (measureText);
  bMeasure->setCheckable (TRUE);
  toolBar[2]->addWidget (bMeasure);


  //  Create Mask toolbar

  toolBar[3] = new QToolBar (tr ("Mask tool bar"));
  toolBar[3]->setToolTip (tr ("Mask tool bar"));
  addToolBar (toolBar[3]);
  toolBar[3]->setObjectName (tr ("geoSwath3D Mask tool bar"));

  bMaskOutsideRect = new QToolButton (this);
  bMaskOutsideRect->setIcon (QIcon (":/icons/mask_outside_rect.xpm"));
  bMaskOutsideRect->setToolTip (tr ("Mask data outside a rectangle"));
  bMaskOutsideRect->setWhatsThis (maskOutsideRectText);
  bMaskOutsideRect->setCheckable (TRUE);
  modeGrp->addButton (bMaskOutsideRect, MASK_OUTSIDE_RECTANGLE);
  toolBar[3]->addWidget (bMaskOutsideRect);

  bMaskOutsidePoly = new QToolButton (this);
  bMaskOutsidePoly->setIcon (QIcon (":/icons/mask_outside_poly.xpm"));
  bMaskOutsidePoly->setToolTip (tr ("Mask data outside a polygon"));
  bMaskOutsidePoly->setWhatsThis (maskOutsidePolyText);
  bMaskOutsidePoly->setCheckable (TRUE);
  modeGrp->addButton (bMaskOutsidePoly, MASK_OUTSIDE_POLYGON);
  toolBar[3]->addWidget (bMaskOutsidePoly);


  toolBar[3]->addSeparator ();
  toolBar[3]->addSeparator ();


  bMaskInsideRect = new QToolButton (this);
  bMaskInsideRect->setIcon (QIcon (":/icons/mask_inside_rect.xpm"));
  bMaskInsideRect->setToolTip (tr ("Mask data inside a rectangle"));
  bMaskInsideRect->setWhatsThis (maskInsideRectText);
  bMaskInsideRect->setCheckable (TRUE);
  modeGrp->addButton (bMaskInsideRect, MASK_INSIDE_RECTANGLE);
  toolBar[3]->addWidget (bMaskInsideRect);

  bMaskInsidePoly = new QToolButton (this);
  bMaskInsidePoly->setIcon (QIcon (":/icons/mask_inside_poly.xpm"));
  bMaskInsidePoly->setToolTip (tr ("Mask data inside a polygon"));
  bMaskInsidePoly->setWhatsThis (maskInsidePolyText);
  bMaskInsidePoly->setCheckable (TRUE);
  modeGrp->addButton (bMaskInsidePoly, MASK_INSIDE_POLYGON);
  toolBar[3]->addWidget (bMaskInsidePoly);


  toolBar[3]->addSeparator ();
  toolBar[3]->addSeparator ();


  bMaskReset = new QToolButton (this);
  bMaskReset->setIcon (QIcon (":/icons/mask_reset.xpm"));
  bMaskReset->setToolTip (tr ("Clear data mask"));
  bMaskReset->setWhatsThis (maskResetText);
  connect (bMaskReset, SIGNAL (clicked ()), this, SLOT (slotMaskReset ()));
  toolBar[3]->addWidget (bMaskReset);



  //  Create LIDAR toolbar

  toolBar[4] = new QToolBar (tr ("LIDAR tool bar"));
  toolBar[4]->setToolTip (tr ("LIDAR tool bar"));
  addToolBar (toolBar[4]);
  toolBar[4]->setObjectName (tr ("geoSwath LIDAR tool bar"));


  QButtonGroup *progGrp = new QButtonGroup (this);
  connect (progGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotProg (int)));
  progGrp->setExclusive (FALSE);


  bLidarMonitor = new QToolButton (this);
  bLidarMonitor->setIcon (QIcon (":/icons/lidar_monitor.png"));
  bLidarMonitor->setWhatsThis (lidarMonitorText);
  bLidarMonitor->setCheckable (TRUE);
  QString tip = options.description[LIDARMONITOR] + "  [" + options.hotkey[LIDARMONITOR] + "]";
  bLidarMonitor->setToolTip (tip);
  bLidarMonitor->setShortcut (QKeySequence (options.hotkey[LIDARMONITOR]));
  options.progButton[LIDARMONITOR] = bLidarMonitor;
  progGrp->addButton (bLidarMonitor, LIDARMONITOR);
  toolBar[4]->addWidget (bLidarMonitor);

  bRMSMonitor = new QToolButton (this);
  bRMSMonitor->setIcon (QIcon (":/icons/rms_monitor.png"));
  bRMSMonitor->setWhatsThis (rmsMonitorText);
  bRMSMonitor->setCheckable (TRUE);
  tip =  options.description[RMSMONITOR] + "  [" + options.hotkey[RMSMONITOR] + "]";
  bRMSMonitor->setToolTip (tip);
  bRMSMonitor->setShortcut (options.hotkey[RMSMONITOR]);
  options.progButton[RMSMONITOR] = bRMSMonitor;
  progGrp->addButton (bRMSMonitor, RMSMONITOR);
  toolBar[4]->addWidget (bRMSMonitor);

  bChartsPic = new QToolButton (this);
  bChartsPic->setIcon (QIcon (":/icons/charts_pic.png"));
  bChartsPic->setWhatsThis (chartsPicText);
  bChartsPic->setCheckable (TRUE);
  tip =  options.description[CHARTSPIC] + "  [" + options.hotkey[CHARTSPIC] + "]";
  bChartsPic->setToolTip (tip);
  bChartsPic->setShortcut (QKeySequence (options.hotkey[CHARTSPIC]));
  options.progButton[CHARTSPIC] = bChartsPic;
  progGrp->addButton (bChartsPic, CHARTSPIC);
  toolBar[4]->addWidget (bChartsPic);

  bWaveformMonitor = new QToolButton (this);
  bWaveformMonitor->setIcon (QIcon (":/icons/waveform_monitor.xpm"));
  bWaveformMonitor->setWhatsThis (waveformMonitorText);
  bWaveformMonitor->setCheckable (TRUE);
  tip =  options.description[WAVEFORMMONITOR] + "  [" + options.hotkey[WAVEFORMMONITOR] + "]";
  bWaveformMonitor->setToolTip (tip);
  bWaveformMonitor->setShortcut (QKeySequence (options.hotkey[WAVEFORMMONITOR]));
  options.progButton[WAVEFORMMONITOR] = bWaveformMonitor;
  progGrp->addButton (bWaveformMonitor, WAVEFORMMONITOR);
  toolBar[4]->addWidget (bWaveformMonitor);

  bWaveMonitor = new QToolButton (this);
  bWaveMonitor->setIcon (QIcon (":/icons/wave_monitor.xpm"));
  bWaveMonitor->setWhatsThis (waveMonitorText);
  bWaveMonitor->setCheckable (TRUE);
  tip =  options.description[WAVEMONITOR] + "  [" + options.hotkey[WAVEMONITOR] + "]";
  bWaveMonitor->setToolTip (tip);
  bWaveMonitor->setShortcut (QKeySequence (options.hotkey[WAVEMONITOR]));
  options.progButton[WAVEMONITOR] = bWaveMonitor;
  progGrp->addButton (bWaveMonitor, WAVEMONITOR);
  toolBar[4]->addWidget (bWaveMonitor);

  bWaveWaterfallAPD = new QToolButton (this);
  bWaveWaterfallAPD->setIcon (QIcon (":/icons/wave_waterfall_apd.png"));
  bWaveWaterfallAPD->setWhatsThis (waveWaterfallAPDText);
  bWaveWaterfallAPD->setCheckable (TRUE);
  tip =  options.description[WAVEWATERFALL_APD] + "  [" + options.hotkey[WAVEWATERFALL_APD] + "]";
  bWaveWaterfallAPD->setToolTip (tip);
  bWaveWaterfallAPD->setShortcut (QKeySequence (options.hotkey[WAVEWATERFALL_APD]));
  options.progButton[WAVEWATERFALL_APD] = bWaveWaterfallAPD;
  progGrp->addButton (bWaveWaterfallAPD, WAVEWATERFALL_APD);
  toolBar[4]->addWidget (bWaveWaterfallAPD);

  bWaveWaterfallPMT = new QToolButton (this);
  bWaveWaterfallPMT->setIcon (QIcon (":/icons/wave_waterfall_pmt.png"));
  bWaveWaterfallPMT->setWhatsThis (waveWaterfallPMTText);
  bWaveWaterfallPMT->setCheckable (TRUE);
  tip =  options.description[WAVEWATERFALL_PMT] + "  [" + options.hotkey[WAVEWATERFALL_PMT] + "]";
  bWaveWaterfallPMT->setToolTip (tip);
  bWaveWaterfallPMT->setShortcut (QKeySequence (options.hotkey[WAVEWATERFALL_PMT]));
  options.progButton[WAVEWATERFALL_PMT] = bWaveWaterfallPMT;
  progGrp->addButton (bWaveWaterfallPMT, WAVEWATERFALL_PMT);
  toolBar[4]->addWidget (bWaveWaterfallPMT);


  //  Create GSF toolbar

  toolBar[5] = new QToolBar (tr ("GSF tool bar"));
  toolBar[5]->setToolTip (tr ("GSF tool bar"));
  addToolBar (toolBar[5]);
  toolBar[5]->setObjectName (tr ("geoSwath GSF tool bar"));

  bGSFMonitor = new QToolButton (this);
  bGSFMonitor->setIcon (QIcon (":/icons/gsf_monitor.png"));
  bGSFMonitor->setWhatsThis (gsfMonitorText);
  bGSFMonitor->setCheckable (TRUE);
  tip =  options.description[GSFMONITOR] + "  [" + options.hotkey[GSFMONITOR] + "]";
  bGSFMonitor->setToolTip (tip);
  bGSFMonitor->setShortcut (options.hotkey[GSFMONITOR]);
  options.progButton[GSFMONITOR] = bGSFMonitor;
  progGrp->addButton (bGSFMonitor, GSFMONITOR);
  toolBar[5]->addWidget (bGSFMonitor);


  //  Set the icon sizes so that the tool bars will set up properly.

  prev_icon_size = options.main_button_icon_size;

  QSize mainButtonIconSize (options.main_button_icon_size, options.main_button_icon_size);

  for (NV_INT32 i = 0 ; i < NUM_TOOLBARS ; i++) toolBar[i]->setIconSize (mainButtonIconSize);


  //  Get the user's defaults if available

  if (!envin (&options, &misc, this))
    {
      //  Set the geometry from defaults since we didn't get any from the saved settings.

      this->resize (misc.draw_area_width, misc.draw_area_height);
      this->move (0, 0);
    }


  //  Allocate the undo blocks based on the max number of undo levels (user option).

  misc.undo = (UNDO *) calloc (options.undo_levels, sizeof (UNDO));
  if (misc.undo == NULL)
    {
      perror ("Allocating undo block pointers in geoSwath3D.cpp");
      exit (-1);
    }


  //  Allocate the colors.  Doing this here speeds up drawing.

  NV_FLOAT32 hue_inc = 315.0 / (NV_FLOAT32) (NUMSHADES + 1);

  for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
    {
      misc.track_color_array[j].setHsv ((NV_INT32) (j * hue_inc), 255, 255, 255); 
      misc.color_array[0][j][0] = misc.color_array[1][j][0] = misc.track_color_array[j].red ();
      misc.color_array[0][j][1] = misc.color_array[1][j][1] = misc.track_color_array[j].green ();
      misc.color_array[0][j][2] = misc.color_array[1][j][2] = misc.track_color_array[j].blue ();
      misc.color_array[0][j][3] = misc.track_color_array[j].alpha ();

      misc.color_array[1][j][3] = options.slice_alpha;
    }


  //  Use geometry to get the width and height.

  QRect tmp = this->geometry ();
  NV_INT32 width = tmp.width ();
  NV_INT32 height = tmp.height ();


  hotkey_message = "<ul>";

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    hotkey_message += "<li>" + options.hotkey[i] + " = " + options.prog[i] + tr (";  action keys = ") + options.action[i] + "</li>";

  hotkey_message += "</ul><br>" +
    tr ("For more information on data types and other restrictions on the use of hot keys see <b>Ancillary Programs</b> in the "
        "<b>Preferences</b> dialog - <img source=\":/icons/prefs.xpm\">");


  misc.save_mode = options.function;
  misc.save_function = options.function;


  //  Set the map values from the defaults

  mapdef.draw_width = width;
  mapdef.draw_height = height;
  mapdef.exaggeration = options.exaggeration;

  mapdef.min_z_extents = options.min_window_size;
  mapdef.mode = NVMAPGL_POINT_MODE;
  mapdef.auto_scale = NVTrue;
  mapdef.projected = 0;

  mapdef.coasts = NVFalse;
  mapdef.landmask = NVFalse;
  mapdef.light_model = GL_LIGHT_MODEL_AMBIENT;

  mapdef.coast_color = Qt::white;
  mapdef.background_color = options.background_color;
  mapdef.scale_color = options.scale_color;
  mapdef.zoom_percent = options.zoom_percent;
  mapdef.draw_scale = options.draw_scale;


  mapdef.initial_bounds.min_x = -180.0;
  mapdef.initial_bounds.min_y = -90.0;
  mapdef.initial_bounds.max_x = 180.0;
  mapdef.initial_bounds.max_y = 90.0;
  mapdef.initial_bounds.min_z = 999999999.0;
  mapdef.initial_bounds.max_z = -999999999.0;
  mapdef.initial_y_rotation = options.y_rotation;
  mapdef.initial_zx_rotation = options.zx_rotation;

  mapdef.renderMode = NVMAPGL_RENDER_POINT_MODE;
  mapdef.complexObjectWidth = 0.002;
  mapdef.complexDivisionals = 8;


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Make the map.

  map = new nvMapGL (this, &mapdef, "geoSwath3D");
  map->setWhatsThis (mapText);


  misc.map_widget = map;


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


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  //  Set up the VBox for the 2D tracker box and the status information.

  QGroupBox *leftBox = new QGroupBox ();
  QVBoxLayout *leftBoxLayout = new QVBoxLayout ();
  leftBox->setLayout (leftBoxLayout);


  NV_INT32 trackBoxWidth = 275, trackBoxHeight = 275;


  //  Set up the 2D tracker box.

  track_mapdef.projection = CYLINDRICAL_EQUIDISTANT;
  track_mapdef.draw_width = 100;
  track_mapdef.draw_height = 100;
  track_mapdef.overlap_percent = 0;
  track_mapdef.grid_inc_x = 0.0;
  track_mapdef.grid_inc_y = 0.0;

  track_mapdef.coasts = NVFalse;
  track_mapdef.landmask = NVFalse;

  track_mapdef.border = 0;
  track_mapdef.coast_color = Qt::white;
  track_mapdef.grid_color = Qt::white;
  track_mapdef.background_color = Qt::black;

  track_mapdef.initial_bounds.min_x = -180.0;
  track_mapdef.initial_bounds.min_y = -90.0;
  track_mapdef.initial_bounds.max_x = 180.0;
  track_mapdef.initial_bounds.max_y = 90.0;

  trackMap = new nvMap (this, &track_mapdef);
  trackMap->setMinimumSize (trackBoxWidth - 20, trackBoxHeight - 20);
  trackMap->setMaximumSize (trackBoxWidth - 20, trackBoxHeight - 20);


  trackMap->setWhatsThis (trackMapText);


  //  Connect to the signals from the map class.

  connect (trackMap, SIGNAL (mousePressSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this,
           SLOT (slotTrackMousePress (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (trackMap, SIGNAL (mouseReleaseSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotTrackMouseRelease (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (trackMap, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this,
           SLOT (slotTrackMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (trackMap, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotTrackResize (QResizeEvent *)));
  connect (trackMap, SIGNAL (preRedrawSignal (NVMAP_DEF)), this, SLOT (slotTrackPreRedraw (NVMAP_DEF)));
  connect (trackMap, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotTrackPostRedraw (NVMAP_DEF)));


  notebook = new QTabWidget ();
  notebook->setTabPosition (QTabWidget::North);
  notebook->setMinimumSize (trackBoxWidth, trackBoxHeight);
  notebook->setMaximumSize (trackBoxWidth, trackBoxHeight);


  notebook->addTab (trackMap, tr ("2D Tracker"));


  //  The following sets up the filter accept/reject notebook page.

  QGroupBox *fbox = new QGroupBox (this);
  QVBoxLayout *fboxLayout = new QVBoxLayout;
  fbox->setLayout (fboxLayout);


  filterBox = new QGroupBox (tr ("000000 points selected"), this);
  filterBox->setToolTip (tr ("Please use the What's This help button then click here for full documentation"));
  filterBox->setWhatsThis (filterBoxText);
  QHBoxLayout *filterBoxLayout = new QHBoxLayout;
  filterBox->setLayout (filterBoxLayout);

  
  QGroupBox *sbox = new QGroupBox (tr ("Filter STD"), this);
  QHBoxLayout *sboxLayout = new QHBoxLayout;
  sbox->setLayout (sboxLayout);

  
  stdSlider = new QSlider ();
  stdSlider->setTracking (TRUE);
  stdSlider->setRange (3, 40);
  stdSlider->setSingleStep (1);
  stdSlider->setPageStep (1);
  stdSlider->setInvertedControls (TRUE);
  stdSlider->setInvertedAppearance (TRUE);
  stdSlider->setValue (NINT (options.filterSTD * 10.0));
  stdSlider->setMinimumHeight (70);
  stdSlider->setToolTip (tr ("Filter standard deviation value"));
  stdSlider->setWhatsThis (stdSliderText);
  connect (stdSlider, SIGNAL (valueChanged (int)), this, SLOT (slotFilterStdChanged (int)));
  connect (stdSlider, SIGNAL (sliderReleased ()), this, SLOT (slotFilter ()));
  sboxLayout->addWidget (stdSlider, 10);


  stdValue = new QLabel ("2.4", this);
  stdValue->setAlignment (Qt::AlignCenter);
  QString std;
  std.sprintf ("%0.1f", options.filterSTD);
  stdValue->setText (std);
  stdValue->setToolTip (tr ("Filter standard deviation value"));
  stdValue->setWhatsThis (stdSliderText);
  stdValue->setFrameStyle (QFrame::Panel | QFrame::Sunken);
  stdValue->setLineWidth (3);
  sboxLayout->addWidget (stdValue);


  filterBoxLayout->addWidget (sbox);
  fboxLayout->addWidget (filterBox);


  QGroupBox *arbox = new QGroupBox (this);
  QHBoxLayout *arboxLayout = new QHBoxLayout;
  arbox->setLayout (arboxLayout);

  QPushButton *acceptButton = new QPushButton (tr ("Accept"), this);
  acceptButton->setToolTip (tr ("Accept filter results"));
  acceptButton->setWhatsThis (tr ("Accept filter results and invalidate marked points"));
  connect (acceptButton, SIGNAL (clicked ()), this, SLOT (slotFilterAccept ()));
  arboxLayout->addWidget (acceptButton);


  QPushButton *rejectButton = new QPushButton (tr ("Reject"), this);
  rejectButton->setWhatsThis (tr ("Discard filter results"));
  rejectButton->setToolTip (tr ("Discard filter results"));
  connect (rejectButton, SIGNAL (clicked ()), this, SLOT (slotFilterReject ()));
  arboxLayout->addWidget (rejectButton);


  fboxLayout->addWidget (arbox);


  notebook->addTab (fbox, tr ("Filter Results"));
  notebook->setTabEnabled (1, FALSE);
  notebook->setCurrentIndex (0);


  leftBoxLayout->addWidget (notebook);


  //  Now we set up the information/status portion of the left side box.

  QGroupBox *numBox = new QGroupBox ();
  numBox->setMaximumWidth (trackBoxWidth);
  numBox->setMinimumWidth (trackBoxWidth);
  QVBoxLayout *numBoxLayout = new QVBoxLayout ();
  numBox->setLayout (numBoxLayout);

  numLabel = new QLabel (this);
  numLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  numLabel->setToolTip (tr ("Number of points in current view"));
  numLabel->setWhatsThis (tr ("Number of points in current view"));
  numBoxLayout->addWidget (numLabel);


  leftBoxLayout->addWidget (numBox);


  QGroupBox *statBox = new QGroupBox ();
  QGridLayout *statBoxLayout = new QGridLayout;
  statBox->setLayout (statBoxLayout);
  statBox->setMaximumWidth (trackBoxWidth);


  for (NV_INT32 i = 0 ; i < NUM_SCALE_LEVELS ; i++)
    {
      misc.scale[i] = new scaleBox (this);
      misc.scale[i]->setMinimumWidth (80);
      statBoxLayout->addWidget (misc.scale[i], i, 0, Qt::AlignLeft);
    }


  QLabel *latName = new QLabel ("Lat:", this);
  latName->setToolTip (tr ("Latitude of cursor"));
  latName->setMinimumSize (latName->sizeHint ());
  statBoxLayout->addWidget (latName, 0, 1, Qt::AlignLeft);

  latLabel = new QLabel ("N 90 00 00.00", this);
  latLabel->setToolTip (tr ("Latitude of cursor"));
  latLabel->setMinimumSize (latLabel->sizeHint ());
  statBoxLayout->addWidget (latLabel, 0, 2, Qt::AlignLeft);
  
  QLabel *lonName = new QLabel ("Lon:", this);
  lonName->setToolTip (tr ("Longitude of cursor"));
  lonName->setMinimumSize (lonName->sizeHint ());
  statBoxLayout->addWidget (lonName, 1, 1, Qt::AlignLeft);

  lonLabel = new QLabel ("N 90 00 00.00", this);
  lonLabel->setToolTip (tr ("Longitude of cursor"));
  lonLabel->setMinimumSize (lonLabel->sizeHint ());
  statBoxLayout->addWidget (lonLabel, 1, 2, Qt::AlignLeft);

  QLabel *recName = new QLabel ("Rec:", this);
  recName->setToolTip (tr ("Record number of current point"));
  recName->setMinimumSize (recName->sizeHint ());
  recName->setMaximumSize (recName->sizeHint ());
  statBoxLayout->addWidget (recName, 2, 1, Qt::AlignLeft);

  recLabel = new QLabel (this);
  recLabel->setToolTip (tr ("Record number of current point"));
  recLabel->setAutoFillBackground (TRUE);
  statBoxLayout->addWidget (recLabel, 2, 2, Qt::AlignLeft);

  QLabel *subName = new QLabel ("Sub:", this);
  subName->setToolTip (tr ("Subrecord number of current point"));
  subName->setMinimumSize (subName->sizeHint ());
  subName->setMaximumSize (subName->sizeHint ());
  statBoxLayout->addWidget (subName, 3, 1, Qt::AlignLeft);

  subLabel = new QLabel (this);
  subLabel->setToolTip (tr ("Subrecord number of current point"));
  subLabel->setAutoFillBackground (TRUE);
  statBoxLayout->addWidget (subLabel, 3, 2, Qt::AlignLeft);

  QLabel *zName = new QLabel ("Z:", this);
  zName->setToolTip (tr ("Current point Z value"));
  zName->setMinimumSize (zName->sizeHint ());
  statBoxLayout->addWidget (zName, 4, 1, Qt::AlignLeft);

  zLabel = new QLabel ("0000.00", this);
  zLabel->setToolTip (tr ("Current point Z value"));
  zLabel->setMinimumSize (zLabel->sizeHint ());
  zLabel->setAutoFillBackground (TRUE);
  zPalette = zLabel->palette ();
  statBoxLayout->addWidget (zLabel, 4, 2, Qt::AlignLeft);

  QLabel *exagName = new QLabel ("Exag:", this);
  exagName->setToolTip (tr ("Vertical exaggeration"));
  exagName->setMinimumSize (exagName->sizeHint ());
  statBoxLayout->addWidget (exagName, 5, 1, Qt::AlignLeft);

  exagLabel = new QLabel (this);
  exagLabel->setToolTip (tr ("Vertical exaggeration"));
  exagLabel->setAutoFillBackground (TRUE);
  statBoxLayout->addWidget (exagLabel, 5, 2, Qt::AlignLeft);
  QString lbl;
  lbl.sprintf (tr ("%5.3f").toAscii (), options.exaggeration);
  exagLabel->setText (lbl);


  statBoxLayout->setColumnStretch (1, 1);
  statBoxLayout->setRowStretch (NUM_SCALE_LEVELS + 1, 0);


  leftBoxLayout->addWidget (statBox, 1);


  fileBarEnabled = NVFalse;


  QGroupBox *fileBox = new QGroupBox (tr ("File"));
  QHBoxLayout *fileBoxLayout = new QHBoxLayout ();
  fileBox->setLayout (fileBoxLayout);

  fileBar = new QScrollBar (Qt::Vertical);
  fileBar->setTracking (FALSE);
  fileBar->setToolTip (tr ("Move through the file (Page Down or Page Up)"));
  fileBar->setWhatsThis (fileBarText);
  filePalette.setColor (QPalette::Normal, QPalette::Base, options.background_color);
  filePalette.setColor (QPalette::Normal, QPalette::Window, options.background_color);
  filePalette.setColor (QPalette::Inactive, QPalette::Base, options.background_color);
  filePalette.setColor (QPalette::Inactive, QPalette::Window, options.background_color);
  fileBar->setPalette (filePalette);
  connect (fileBar, SIGNAL (valueChanged (int)), this, SLOT (slotFileValueChanged (int)));
  fileBoxLayout->addWidget (fileBar);

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
  connect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
  connect (exagBar, SIGNAL (sliderReleased ()), this, SLOT (slotExagReleased ()));


  QGroupBox *sliceBox = new QGroupBox (tr ("Slice"));
  QVBoxLayout *sliceBoxLayout = new QVBoxLayout ();
  sliceBox->setLayout (sliceBoxLayout);


  QHBoxLayout *barBoxLayout = new QHBoxLayout ();


  sliceBar = new QScrollBar (Qt::Vertical);
  sliceBar->setTracking (TRUE);
  sliceBar->setInvertedAppearance (TRUE);
  sliceBar->setToolTip (tr ("Slice through the data (<b>Up Arrow</b> or <b>Down Arrow</b>)"));
  sliceBar->setWhatsThis (sliceBarText);
  slicePalette.setColor (QPalette::Normal, QPalette::Base, options.background_color);
  slicePalette.setColor (QPalette::Normal, QPalette::Window, options.background_color);
  slicePalette.setColor (QPalette::Inactive, QPalette::Base, options.background_color);
  slicePalette.setColor (QPalette::Inactive, QPalette::Window, options.background_color);
  sliceBar->setPalette (slicePalette);
  connect (sliceBar, SIGNAL (actionTriggered (int)), this, SLOT (slotSliceTriggered (int)));
  barBoxLayout->addWidget (sliceBar);


  QVBoxLayout *ctrlBox = new QVBoxLayout ();

  transBar = new QScrollBar (Qt::Vertical);
  transBar->setTracking (TRUE);
  transBar->setInvertedAppearance (TRUE);
  transBar->setToolTip (tr ("Transparency of data outside of slice"));
  transBar->setWhatsThis (transText);
  transPalette.setColor (QPalette::Normal, QPalette::Base, options.background_color);
  transPalette.setColor (QPalette::Normal, QPalette::Window, options.background_color);
  transPalette.setColor (QPalette::Inactive, QPalette::Base, options.background_color);
  transPalette.setColor (QPalette::Inactive, QPalette::Window, options.background_color);
  transBar->setPalette (transPalette);
  transBar->setMinimum (0);
  transBar->setMaximum (MAX_TRANS_VALUE);
  transBar->setSingleStep (1);
  transBar->setPageStep (8);
  transBar->setValue (options.slice_alpha);
  connect (transBar, SIGNAL (actionTriggered (int)), this, SLOT (slotTransTriggered (int)));
  ctrlBox->addWidget (transBar, 1);


  transLabel = new QLabel ("000", this);
  transLabel->setAlignment (Qt::AlignCenter);
  transLabel->setMinimumSize (transLabel->sizeHint ());
  transLabel->setToolTip (tr ("Transparency value"));
  transLabel->setWhatsThis (transText);
  transLabel->setFrameStyle (QFrame::Panel | QFrame::Sunken);
  transLabel->setLineWidth (2);
  QString string;
  string.sprintf ("%02d", options.slice_alpha);
  transLabel->setText (string);
  ctrlBox->addWidget (transLabel);


  sizeBar = new QScrollBar (Qt::Vertical);
  sizeBar->setTracking (TRUE);
  sizeBar->setInvertedAppearance (TRUE);
  sizeBar->setToolTip (tr ("Size of slice as percentage of area"));
  sizeBar->setWhatsThis (sizeText);
  sizePalette.setColor (QPalette::Normal, QPalette::Base, options.background_color);
  sizePalette.setColor (QPalette::Normal, QPalette::Window, options.background_color);
  sizePalette.setColor (QPalette::Inactive, QPalette::Base, options.background_color);
  sizePalette.setColor (QPalette::Inactive, QPalette::Window, options.background_color);
  sizeBar->setPalette (sizePalette);
  sizeBar->setMinimum (1);
  sizeBar->setMaximum (MAX_SLICE_SIZE);
  sizeBar->setSingleStep (1);
  sizeBar->setPageStep (5);
  sizeBar->setValue (options.slice_percent);
  connect (sizeBar, SIGNAL (actionTriggered (int)), this, SLOT (slotSizeTriggered (int)));
  ctrlBox->addWidget (sizeBar, 1);

  sizeLabel = new QLabel ("00", this);
  sizeLabel->setAlignment (Qt::AlignCenter);
  sizeLabel->setMinimumSize (sizeLabel->sizeHint ());
  sizeLabel->setToolTip (tr ("Slice size as a percentage"));
  sizeLabel->setWhatsThis (sizeText);
  sizeLabel->setFrameStyle (QFrame::Panel | QFrame::Sunken);
  sizeLabel->setLineWidth (2);
  sizeLabel->setNum (options.slice_percent);
  ctrlBox->addWidget (sizeLabel);

  barBoxLayout->addLayout (ctrlBox);

  sliceBoxLayout->addLayout (barBoxLayout);


  meterLabel = new QLabel ("00000.0", this);
  meterLabel->setAlignment (Qt::AlignCenter);
  meterLabel->setMinimumSize (meterLabel->sizeHint ());
  meterLabel->setToolTip (tr ("Slice size in meters"));
  meterLabel->setFrameStyle (QFrame::Panel | QFrame::Sunken);
  meterLabel->setLineWidth (2);
  meterLabel->setText ("");

  sliceBoxLayout->addWidget (meterLabel);


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (leftBox);
  hBox->addWidget (fileBox);
  hBox->addWidget (exagBar);
  hBox->addWidget (map, 1);
  hBox->addWidget (sliceBox);
  vBox->addLayout (hBox, 1);


  //  Set View toolbar button states based on saved options.  We don't set the shortcut for the first three because they wil be triggered
  //  from the shortcuts in the pulldown menu.

  bDisplayInvalid->setChecked (options.display_invalid);
  connect (bDisplayInvalid, SIGNAL (clicked ()), this, SLOT (slotDisplayInvalid ()));


  //  Set Edit toolbar button states based on saved options

  if (options.function == DELETE_POINT) bDeletePoint->setChecked (TRUE);

  if (options.function == DELETE_RECTANGLE) bDeleteRect->setChecked (TRUE);

  if (options.function == DELETE_POLYGON) bDeletePoly->setChecked (TRUE);

  if (options.function == KEEP_POLYGON) bKeepPoly->setChecked (TRUE);

  if (options.function == RESTORE_RECTANGLE) bRestoreRect->setChecked (TRUE);

  if (options.function == RESTORE_POLYGON) bRestorePoly->setChecked (TRUE);


  //  Set all of the button hotkey tooltips and shortcuts

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++) slotPrefHotKeyChanged (i);


  //  Right click popup menu

  popupMenu = new QMenu (map);
  connect (popupMenu, SIGNAL (triggered (QAction *)), this, SLOT (slotPopupMenu (QAction *)));

  for (NV_INT32 i = 0 ; i < NUMPOPUPS ; i++) popup[i] = popupMenu->addAction (("            "));

  popupMenu->addSeparator ();
  popupHelp = popupMenu->addAction (tr ("Help"));
  connect (popupHelp, SIGNAL (triggered ()), this, SLOT (slotPopupHelp ()));


  //  Setup the file menu (don't set shortcuts for these menu items because they're handled by the keypress event with the associated buttons)

  QAction *openAction = new QAction (tr ("Open"), this);
  connect (openAction, SIGNAL (triggered ()), this, SLOT (slotOpen ()));

  QAction *exitAction = new QAction (tr ("Quit"), this);
  connect (exitAction, SIGNAL (triggered ()), this, SLOT (slotQuit ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (openAction);
  fileMenu->addAction (exitAction);


  //  Setup the Edit menu

  QAction *prefsAct = new QAction (tr ("&Preferences"), this);
  prefsAct->setShortcut (QKeySequence (tr ("Ctrl+P")));
  prefsAct->setStatusTip (tr ("Change program preferences"));
  prefsAct->setWhatsThis (prefsText);
  connect (prefsAct, SIGNAL (triggered ()), this, SLOT (slotPrefs ()));


  QMenu *editMenu = menuBar ()->addMenu (tr ("&Edit"));
  editMenu->addAction (prefsAct);


  //  Setup the help menu.

  QAction *hotHelp = new QAction (tr ("&Hot Keys and GUI control"), this);
  hotHelp->setShortcut (QKeySequence (tr ("Ctrl+H")));
  hotHelp->setStatusTip (tr ("Help on GUI control and hot keys (fixed and user modifiable)"));
  connect (hotHelp, SIGNAL (triggered ()), this, SLOT (slotHotkeyHelp ()));

  QAction *toolHelp = new QAction (tr ("&Tool bars"), this);
  toolHelp->setShortcut (QKeySequence (tr ("Ctrl+T")));
  toolHelp->setStatusTip (tr ("Help on tool bars"));
  connect (toolHelp, SIGNAL (triggered ()), this, SLOT (slotToolbarHelp ()));

  QAction *whatsThisAct = QWhatsThis::createAction (this);

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (QKeySequence (tr ("Ctrl+A")));
  aboutAct->setStatusTip (tr ("Information about geoSwath3D"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgements = new QAction (tr ("A&cknowledgements"), this);
  acknowledgements->setShortcut (QKeySequence (tr ("Ctrl+c")));
  acknowledgements->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgements, SIGNAL (triggered ()), this, SLOT (slotAcknowledgements ()));

  QAction *aboutQtAct = new QAction (tr ("About&Qt"), this);
  aboutQtAct->setShortcut (tr ("Ctrl+Q"));
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("&Help"));
  helpMenu->addAction (hotHelp);
  helpMenu->addAction (toolHelp);
  helpMenu->addAction (whatsThisAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgements);
  helpMenu->addAction (aboutQtAct);


  setMainButtons (NVTrue);


  //  Set up the status bars.

  QGroupBox *fpBox = new QGroupBox ();
  QGridLayout *fpBoxLayout = new QGridLayout;
  fpBox->setLayout (fpBoxLayout);


  misc.statusProgLabel = new QLabel (this);
  misc.statusProgLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  misc.statusProgLabel->setToolTip (tr ("Active mode"));
  misc.statusProgLabel->setWhatsThis (miscLabelText);
  misc.statusProgLabel->setAutoFillBackground (TRUE);
  misc.statusProgPalette = misc.statusProgLabel->palette ();
  fpBoxLayout->addWidget (misc.statusProgLabel, 1, 0);


  //  Progress status bar

  misc.progStatus = new QStatusBar ();
  misc.progStatus->setSizeGripEnabled (FALSE);
  misc.progStatus->setToolTip (tr ("Progress bar and label"));
  misc.progStatus->setWhatsThis (progStatusText);
  fpBoxLayout->addWidget (misc.progStatus, 1, 1);


  misc.statusProg = new QProgressBar (this);
  misc.statusProg->setRange (0, 100);
  misc.statusProg->setValue (0);
  misc.statusProg->setTextVisible (FALSE);
  misc.progStatus->addWidget (misc.statusProg, 10);


  vBox->addWidget (fpBox);


  //  Set the tracking timer function.  This will track the cursor in other ABE program map windows.
  //  Also used to limit the number of points in the polygon functions.

  trackCursor = new QTimer (this);
  connect (trackCursor, SIGNAL (timeout ()), this, SLOT (slotTrackCursor ()));
  trackCursor->start (50);


  //  Set the blink timer (half second) function.

  blinkTimer = new QTimer (this);
  connect (blinkTimer, SIGNAL (timeout ()), this, SLOT (slotBlinkTimer ()));
  blinkTimer->start (500);


  setFunctionCursor (options.function);


  show ();


  if (!misc.file.isEmpty ())
    {
      open_file ();
    }
  else
    {
      slotOpen ();
    }
}



geoSwath3D::~geoSwath3D ()
{
}



void 
geoSwath3D::slotOpen ()
{
  QStringList files, filters;

  if (!QDir (options.input_dir).exists ()) options.input_dir = ".";

  io_data_close ();


  QFileDialog *fd = new QFileDialog (this, tr ("geoSwath3D Open Input File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.input_dir);


  filters << tr ("GSF (*.d\?\? *.gsf)")
          << tr ("HOF (*.hof)")
          << tr ("TOF (*.tof)")
          << tr ("WLF (*.wlf *.wtf *.whf)")
          << tr ("HAWKEYE (*.bin)")
          << tr ("All files (*)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (options.inputFilter);

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      misc.file = files.at (0);


      if (misc.file.isEmpty())
        {
          QMessageBox::warning (this, tr ("Open input file"), tr ("A file must be selected!"));
          return;
        }


      QFileInfo fileInfo = QFileInfo (misc.file);
      this->setWindowTitle (QString ("geoSwath3D : ") + fileInfo.fileName ());


      options.input_dir = fd->directory ().absolutePath ();
      options.inputFilter = fd->selectedFilter ();


      open_file ();
    }
}



void 
geoSwath3D::open_file ()
{
  if (io_data_open (&misc, &options))
    {
      QString tmp;
      tmp.sprintf (" - %s", strerror (errno));
      tmp.prepend (misc.file);

      QMessageBox::warning (this, tr ("geoSwath3D: Error opening input file"), tmp);
      return;
    }


  if (misc.linked)
    {
      NV_CHAR file[512];
      strcpy (file, misc.file.toAscii ());
      changeFileRegisterABE (misc.abeRegister, misc.abe_register, abe_register_group, active_window_id, file);

      QString title;
      title.sprintf ("geoSwath3D (ABE group %02d) : %s", abe_register_group, file);
      this->setWindowTitle (title);
    }


  fileBarEnabled = NVFalse;

  fileBar->setMinimum (misc.start_record);
  fileBar->setMaximum (misc.end_record);
  fileBar->setSingleStep (misc.num_records);
  fileBar->setPageStep (misc.num_records);
  fileBar->setValue (misc.start_record);

  fileBarEnabled = NVTrue;

  first_screen = NVTrue;


  map->setCursor (Qt::WaitCursor);
  map->setToolTip ("");
  qApp->processEvents ();


  if (record_num)
    {
      NV_INT32 beg_rec = qMax (record_num - (misc.num_records / 2), misc.start_record);
      fileBar->setValue (beg_rec);
    }
  else
    {
      get_buffer (&data, &misc, misc.start_record);

      QString cntText = tr ("Number of points: %1").arg (data.count);
      numLabel->setText (cntText);
    }


  strcpy (misc.abe_share->nearest_filename, misc.file.toAscii ());


  file_opened = NVTrue;


  map->setBounds (data.bounds);
  trackMap->resetBounds (data.bounds);


  redrawMap (NVTrue);


  //  Get the map center for the reset view slot.

  map->getMapCenter (&misc.map_center_x, &misc.map_center_y, &misc.map_center_z);


  map->enableSignals ();


  setMainButtons (NVTrue);
}



void 
geoSwath3D::slotLink ()
{
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

      this->setWindowTitle ("geoSwath3D : " + misc.file);


      setMainButtons (NVTrue);
    }
  else
    {
      //  Get the ABE_REGISTER shared memory area.

      NV_CHAR file[512];
      strcpy (file, misc.file.toAscii ());
      registerABE *registerABEDialog = new registerABE (this, "geoSwath3D", active_window_id, file, &misc.abeRegister, &misc.abe_register);


      connect (registerABEDialog, SIGNAL (keySetSignal (NV_INT32)), this, SLOT (slotRegisterABEKeySet (NV_INT32)));
    }
}



void 
geoSwath3D::slotRegisterABEKeySet (NV_INT32 key)
{
  if (key >= 0)
    {
      abe_register_group = key;


      QString title;
      title.sprintf ("geoSwath3D (ABE group %02d) : ", abe_register_group);
      title += misc.file;
      this->setWindowTitle (title);


      misc.linked = NVTrue;


      bLink->setToolTip (tr ("Disconnect from other ABE applications"));
      bLink->setWhatsThis (unlinkText);
      bLink->setIcon (QIcon (":/icons/link.xpm"));
    }
}



void 
geoSwath3D::setMainButtons (NV_BOOL enable)
{
  //  If we changed the icon size in the preferences dialog we need to resize all of the buttons.

  if (options.main_button_icon_size != prev_icon_size)
    {
      QSize mainButtonIconSize (options.main_button_icon_size, options.main_button_icon_size);

      for (NV_INT32 i = 0 ; i < NUM_TOOLBARS ; i++)
        {
          toolBar[i]->setIconSize (mainButtonIconSize);
          toolBar[i]->adjustSize ();
        }

      prev_icon_size = options.main_button_icon_size;
    }


  bOpen->setEnabled (enable);
  bExit->setEnabled (enable);
  bReset->setEnabled (enable);


  //  Only enable UNDO if we saved an undo block.

  if (misc.undo_count)
    {
      bUndo->setEnabled (enable);
    }
  else
    {
      bUndo->setEnabled (FALSE);
    }


  bDisplayInvalid->setEnabled (enable);


  bPrefs->setEnabled (enable);


  //  Stop is always the inverse.

  bStop->setEnabled (!enable);


  //  If we're displaying invalid data we want to disable all editing functions so the users won't get
  //  confused (too late!) unless we're also flagging invalid data.  Restores are allowed when displaying
  //  invalid data since that sort of makes sense.

  if (options.display_invalid && options.flag_index != 2)
    {
      bDeletePoint->setEnabled (FALSE);
      bDeleteRect->setEnabled (FALSE);
      bDeletePoly->setEnabled (FALSE);
      bKeepPoly->setEnabled (FALSE);
      bHotkeyPoly->setEnabled (FALSE);
      bFilter->setEnabled (FALSE);
    }
  else
    {
      bDeletePoint->setEnabled (enable);
      bDeleteRect->setEnabled (enable);
      bDeletePoly->setEnabled (enable);
      bKeepPoly->setEnabled (enable);
      bHotkeyPoly->setEnabled (enable);
      bFilter->setEnabled (enable);
    }


  bRestoreRect->setEnabled (enable);
  bRestorePoly->setEnabled (enable);


  //  Not allowed to filter mask if we're slicing.

  if (misc.slice)
    {
      bFilterRectMask->setEnabled (FALSE);
      bFilterPolyMask->setEnabled (FALSE);

      QString string;
      if (misc.slice_meters < 10.0)
        {
          string.sprintf ("%.2f", misc.slice_meters);
        }
      else if (misc.slice_meters < 100.0)
        {
          string.sprintf ("%.1f", misc.slice_meters);
        }
      else
        {
          string.sprintf ("%.0f", misc.slice_meters);
        }
      meterLabel->setText (string);
    }
  else
    {
      bFilterRectMask->setEnabled (enable);
      bFilterPolyMask->setEnabled (enable);
      meterLabel->setText ("");
    }
  bClearMasks->setEnabled (enable);

  bFlag->setIcon (flagIcon[options.flag_index]);
  bFlag->setEnabled (enable);


  if (!misc.hydro_lidar_present)
    {
      bLidarMonitor->setEnabled (FALSE);
      bRMSMonitor->setEnabled (FALSE);
      bWaveformMonitor->setEnabled (FALSE);
      bWaveMonitor->setEnabled (FALSE);
      bWaveWaterfallAPD->setEnabled (FALSE);
      bWaveWaterfallPMT->setEnabled (FALSE);
    }
  else
    {
      bLidarMonitor->setEnabled (enable);
      bRMSMonitor->setEnabled (enable);
      bWaveformMonitor->setEnabled (enable);
      bWaveMonitor->setEnabled (enable);
      bWaveWaterfallAPD->setEnabled (enable);
      bWaveWaterfallPMT->setEnabled (enable);
    }


  if (!misc.gsf_present)
    {
      bGSFMonitor->setEnabled (FALSE);
    }
  else
    {
      bGSFMonitor->setEnabled (enable);
    }


  if (!misc.lidar_present)
    {
      bChartsPic->setEnabled (FALSE);
      bLidarMonitor->setEnabled (FALSE);
    }
  else
    {
      bChartsPic->setEnabled (enable);
      bLidarMonitor->setEnabled (enable);
    }

  if (!enable || (enable && misc.highlight_count))
    {
      bClearHighlight->setEnabled (enable);
      bClearPoly->setEnabled (enable);
    }
  bHighlightPoly->setEnabled (enable);

  bMeasure->setEnabled (enable);
  bMaskInsideRect->setEnabled (enable);
  bMaskInsidePoly->setEnabled (enable);
  bMaskOutsideRect->setEnabled (enable);
  bMaskOutsidePoly->setEnabled (enable);

  if (!enable || misc.mask_active) bMaskReset->setEnabled (enable);
}



void 
geoSwath3D::setFunctionCursor (NV_INT32 function)
{
  //  Default to point mode for the map.

  map->setMapMode (NVMAPGL_POINT_MODE);


  discardMovableObjects ();


  switch (function)
    {
    case DELETE_RECTANGLE:
      misc.statusProgLabel->setText (tr ("Rectangular delete mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (deleteRectCursor);
      bDeleteRect->setChecked (TRUE);
      break;

    case RESTORE_RECTANGLE:
      misc.statusProgLabel->setText (tr ("Rectangular restore mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (restoreRectCursor);
      bRestoreRect->setChecked (TRUE);
      break;

    case DELETE_POLYGON:
      misc.statusProgLabel->setText (tr ("Polygon delete mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (deletePolyCursor);
      bDeletePoly->setChecked (TRUE);
      break;

    case RESTORE_POLYGON:
      misc.statusProgLabel->setText (tr ("Polygon restore mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (restorePolyCursor);
      bRestorePoly->setChecked (TRUE);
      break;

    case KEEP_POLYGON:
      misc.statusProgLabel->setText (tr ("Delete outside polygon mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (keepPolyCursor);
      bKeepPoly->setChecked (TRUE);
      break;

    case HOTKEY_POLYGON:
      if (misc.hotkey_poly_count)
        {
          misc.statusProgLabel->setText (tr ("Press a hot key"));
        }
      else
        {
          misc.statusProgLabel->setText (tr ("Polygon hot key mode"));
        }
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (hotkeyPolyCursor);
      bHotkeyPoly->setChecked (TRUE);
      break;

    case DELETE_POINT:
      misc.statusProgLabel->setText (tr ("Delete subrecord/record mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::PointingHandCursor);
      bDeletePoint->setChecked (TRUE);
      break;

    case NOOP:
      misc.statusProgLabel->setText (tr ("No Op mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::PointingHandCursor);
      break;

    case ROTATE:
      misc.statusProgLabel->setText (tr ("Rotate data"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (rotateCursor);
      break;

    case ZOOM:
      misc.statusProgLabel->setText (tr ("Zoom in/out"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (zoomCursor);
      break;

    case RECT_FILTER_MASK:
      misc.statusProgLabel->setText (tr ("Rectangle filter mask mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterMaskRectCursor);
      bFilterRectMask->setChecked (TRUE);
      break;

    case POLY_FILTER_MASK:
      misc.statusProgLabel->setText (tr ("Polygon filter mask mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterMaskPolyCursor);
      bFilterPolyMask->setChecked (TRUE);
      break;

    case MASK_INSIDE_RECTANGLE:
      misc.statusProgLabel->setText (tr ("Mask inside rectangle mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (maskCursor);
      break;

    case MASK_OUTSIDE_RECTANGLE:
      misc.statusProgLabel->setText (tr ("Mask outside rectangle mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (maskCursor);
      break;

    case MASK_INSIDE_POLYGON:
      misc.statusProgLabel->setText (tr ("Mask inside polygon mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (maskCursor);
      break;

    case MASK_OUTSIDE_POLYGON:
      misc.statusProgLabel->setText (tr ("Mask outside polygon mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (maskCursor);
      break;

    case MEASURE:
      misc.statusProgLabel->setText (tr ("Measuring mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (measureCursor);
      break;

    case HIGHLIGHT_POLYGON:
      misc.statusProgLabel->setText (tr ("Highlight points in polygon mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (highlightPolyCursor);
      bHighlightPoly->setChecked (TRUE);
      break;

    case CLEAR_POLYGON:
      misc.statusProgLabel->setText (tr ("Clear highlights in polygon mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (clearPolyCursor);
      bClearPoly->setChecked (TRUE);
      break;
    }
}



void 
geoSwath3D::discardMovableObjects ()
{
  map->closeMovingList (&mv_tracker);
  map->closeMovingList (&mv_measure_anchor);
  trackMap->closeMovingPath (&mv_2D_tracker);
  map->closeMovingList (&mv_measure);
  map->discardRubberbandRectangle (&rb_rectangle);
  map->discardRubberbandPolygon (&rb_polygon);
  map->discardRubberbandLine (&rb_measure);


  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++) map->closeMovingList (&(multi_marker[i]));
}



void 
geoSwath3D::leftMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon __attribute__ ((unused)),
                      NV_FLOAT64 lat __attribute__ ((unused)), NV_FLOAT64 z __attribute__ ((unused)))
{
  NV_INT32 *px, *py;

  //  If the popup menu is up discard this mouse press


  if (popup_active)
    {
      //  Flush the OpenGL scene so the menu will disperse

      map->flush ();


      popup_active = NVFalse;
      return;
    }


  switch (options.function)
    {
    case NOOP:
    case DELETE_POINT:
      break;
    
    case DELETE_RECTANGLE:
    case RESTORE_RECTANGLE:
    case MASK_INSIDE_RECTANGLE:
    case MASK_OUTSIDE_RECTANGLE:
    case RECT_FILTER_MASK:

      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, mouse_x, mouse_y, &px, &py);


          //  Invalidate, restore, or mask the area.

          qApp->setOverrideCursor (Qt::WaitCursor);
          qApp->processEvents ();

          set_area (map, &options, &misc, &data, px, py, 4, 1);

          qApp->restoreOverrideCursor ();


          map->discardRubberbandRectangle (&rb_rectangle);


          if (options.function == MASK_INSIDE_RECTANGLE || options.function == MASK_OUTSIDE_RECTANGLE) misc.mask_active = NVTrue;

          if (options.function == RECT_FILTER_MASK)
            {
              misc.filter_mask = NVTrue;


              //  If the filter message box is up then we are filter masking after the filter so we want to discard the current filter points
              //  then filter again.

              if (filter_active) slotFilter ();
            }


          redrawMap (NVTrue);


          //  Even though we do this in redrawMap we need to do it again here because otherwise the damn markers disappear.
          //  It's OpenGL, I have no idea why.

          if (options.function == RECT_FILTER_MASK) overlayData (map, &options, &misc, &data);


          setFunctionCursor (options.function);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, mouse_x, mouse_y, options.edit_color, 2, Qt::SolidLine);
        }
      break;

    case DELETE_POLYGON:
    case RESTORE_POLYGON:
    case KEEP_POLYGON:
    case HOTKEY_POLYGON:
    case MASK_INSIDE_POLYGON:
    case MASK_OUTSIDE_POLYGON:
    case POLY_FILTER_MASK:
    case HIGHLIGHT_POLYGON:
    case CLEAR_POLYGON:

      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          NV_INT32 count;
          map->closeRubberbandPolygon (rb_polygon, mouse_x, mouse_y, &count, &px, &py);


          //  Invalidate or restore the area.

          qApp->setOverrideCursor (Qt::WaitCursor);
          qApp->processEvents ();

          switch (options.function)
            {
            case DELETE_POLYGON:
            case RESTORE_POLYGON:
            case HIGHLIGHT_POLYGON:
            case CLEAR_POLYGON:
              set_area (map, &options, &misc, &data, px, py, count, 0);
              break;

            case POLY_FILTER_MASK:
              set_area (map, &options, &misc, &data, px, py, count, 0);
              misc.filter_mask = NVTrue;


              //  If the filter message box is up then we are filter masking after the filter so we want to discard the current filter points
              //  then filter again.

              if (filter_active) slotFilter ();
              break;

            case MASK_INSIDE_POLYGON:
            case MASK_OUTSIDE_POLYGON:
              set_area (map, &options, &misc, &data, px, py, count, 0);
              misc.mask_active = NVTrue;
              break;

            case KEEP_POLYGON:
              keep_area (map, &options, &misc, &data, px, py, count);
              break;

            case HOTKEY_POLYGON:
              misc.hotkey_poly_count = count;


              //  We have to check because we might have freed this memory in ancillaryProg but, on
              //  the other hand, the user may have aborted the operation so we still might need to
              //  free it here.

              if (misc.hotkey_poly_x)
                {
                  free (misc.hotkey_poly_x);
                  free (misc.hotkey_poly_y);
                }

              misc.hotkey_poly_x = (NV_INT32 *) malloc (count * sizeof (NV_INT32));
              misc.hotkey_poly_y = (NV_INT32 *) malloc (count * sizeof (NV_INT32));

              if (misc.hotkey_poly_y == NULL)
                {
                  fprintf (stderr , tr ("Error allocating memory - %s %d\n").toAscii (), __FILE__, __LINE__);
                  clean_exit (-1);
                }

              for (NV_INT32 i = 0 ; i < count ; i++)
                {
                  misc.hotkey_poly_x[i] = px[i];
                  misc.hotkey_poly_y[i] = py[i];
                }

              //  Now we wait for the user to press a hotkey.  Take a look at keyPressEvent and 
              //  ancillaryProg.

              break;
            }

          qApp->restoreOverrideCursor ();

          prev_poly_x = -1;


          //  Don't redraw if we're setting up for a hotkey polygon

          if (options.function != HOTKEY_POLYGON)
            {
              map->discardRubberbandPolygon (&rb_polygon);
              redrawMap (NVTrue);


              //  Even though we do this in redrawMap we need to do it again here because otherwise the damn markers disappear.
              //  It's OpenGL, I have no idea why.

              if (options.function == POLY_FILTER_MASK) overlayData (map, &options, &misc, &data);
            }
        }
      else
        {
          map->anchorRubberbandPolygon (&rb_polygon, mouse_x, mouse_y, options.edit_color, LINE_WIDTH, NVTrue, Qt::SolidLine);
          prev_poly_x = -1;
        }
      break;


    case MEASURE:
      if (map->rubberbandLineIsActive (rb_measure))
        {
          map->discardRubberbandLine (&rb_measure);
          map->closeMovingList (&mv_measure_anchor);
          map->closeMovingList (&mv_measure);
          map->flush ();
        }
      else
        {
          if (mv_measure_anchor < 0)
            {
              map->setMovingList (&mv_measure_anchor, marker, 16, mouse_x, mouse_y, 0.0, 2, options.edit_color);
            }
          else
            {
              map->setMovingList (&mv_measure, marker, 16, mouse_x, mouse_y, 0.0, 2, options.edit_color);
            }


          //  Snap the start of the line to the nearest point

          NV_INT32 pix_x, pix_y;
          map->get2DCoords (data.x[misc.nearest_point], data.y[misc.nearest_point], -data.z[misc.nearest_point], &pix_x, &pix_y);

          map->anchorRubberbandLine (&rb_measure, pix_x, pix_y, options.edit_color, LINE_WIDTH, Qt::SolidLine);
          line_anchor.x = data.x[misc.nearest_point];
          line_anchor.y = data.y[misc.nearest_point];
          line_anchor.z = data.z[misc.nearest_point];
        }
      break;
    }
}



void 
geoSwath3D::midMouse (NV_INT32 mouse_x __attribute__ ((unused)), NV_INT32 mouse_y __attribute__ ((unused)),
                     NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)),
                     NV_FLOAT64 z __attribute__ ((unused)))
{
  switch (options.function)
    {
    case NOOP:
      break;

    case DELETE_POINT:
      if (misc.nearest_point != -1)
        {
          NV_U_INT32 rec = data.rec[misc.nearest_point];

          for (NV_INT32 i = 0 ; i < data.count ; i++)
            {
              if (data.rec[i] == rec)
                {
                  store_undo (&misc, options.undo_levels, data.val[i], i);
                  data.val[i] |= PFM_MANUALLY_INVAL;
                }
            }
          end_undo_block (&misc);


          //
          //                      if we are frozen, automatically unfreeze
          //

          if (misc.marker_mode) misc.marker_mode = 0;
        }

      redrawMap (NVTrue);
      break;

    case DELETE_RECTANGLE:
    case RESTORE_RECTANGLE:
    case MASK_INSIDE_RECTANGLE:
    case MASK_OUTSIDE_RECTANGLE:
    case RECT_FILTER_MASK:
      map->discardRubberbandRectangle (&rb_rectangle);
      break;

    case DELETE_POLYGON:
    case RESTORE_POLYGON:
    case KEEP_POLYGON:
    case HOTKEY_POLYGON:
    case MASK_INSIDE_POLYGON:
    case MASK_OUTSIDE_POLYGON:
    case POLY_FILTER_MASK:
    case HIGHLIGHT_POLYGON:
    case CLEAR_POLYGON:
      map->discardRubberbandPolygon (&rb_polygon);
      prev_poly_x = -1;
      break;

    case MEASURE:
      map->discardRubberbandLine (&rb_measure);
      map->closeMovingList (&mv_measure_anchor);
      map->closeMovingList (&mv_measure);
      break;
    }
  setFunctionCursor (options.function);
}



void 
geoSwath3D::slotMouseDoubleClick (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon __attribute__ ((unused)),
                                 NV_FLOAT64 lat __attribute__ ((unused)), NV_FLOAT64 z __attribute__ ((unused)))
{
  //  Lock in nearest point so mouse moves won't change it if we want to run one of the ancillary programs.

  lock_point = misc.nearest_point;


  //  Flip the double_click flag.  The right-click menu sets this to NVTrue so it will flip to NVFalse.
  //  Left-click sets it to NVFalse so it will filp to NVTrue;

  double_click = !double_click;


  //  Double click is hard-wired to turn off slicing.

  if (misc.slice) slotPopupMenu (popup[NUMPOPUPS - 1]);
}



//  Freakin' menus!

void 
geoSwath3D::rightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z __attribute__ ((unused)))
{
  QString tmp;


  //  If we already have a context menu up, flush the OpenGL scene so the previous one will go away

  if (popup_active) map->flush ();


  menu_cursor_lon = lon;
  menu_cursor_lat = lat;
  menu_cursor_x = mouse_x;
  menu_cursor_y = mouse_y;


  QPoint pos (mouse_x, mouse_y);


  //  Lock in nearest point so mouse moves won't change it if we want to run one of the ancillary programs.

  lock_point = misc.nearest_point;


  //  Popups need global positioning

  QPoint global_pos = map->mapToGlobal (pos);


  //  In most cases popup[2] through popup[NUMPOPUPS-1] are not used so we set them to false.  If we need them for specific buttons
  //  we'll set them to true in the switch.

  popup[0]->setVisible (TRUE);
  popup[1]->setVisible (TRUE);
  for (NV_INT32 i = 2 ; i < NUMPOPUPS ; i++) popup[i]->setVisible (FALSE);


  switch (options.function)
    {
    case NOOP:
      break;

    case DELETE_POINT:

      //  If you move the "Delete point" option from popup[0] look through the code for calls to slotPopupMenu with popup[0] as the argument.

      tmp = tr ("Delete point [%1]").arg (options.hotkey[DELETE_SINGLE_POINT]);
      popup[0]->setText (tmp);
      popup[1]->setText (tr ("Delete record (e.g. ping)"));
      popup[2]->setText (tr ("Highlight point"));
      popup[2]->setVisible (TRUE);


      //  Don't allow point marking if we have filter points or we are flagging some data.

      if (!misc.filter_kill_count && !options.flag_index)
        {
          popup[2]->setEnabled (TRUE);
        }
      else
        {
          popup[2]->setEnabled (FALSE);
        }


      //  If you need to change the freeze/unfreeze popup position you need to change the FREEZE_POPUP value in pfmEdit3DDef.hpp

      if (misc.marker_mode)
        {
          popup[FREEZE_POPUP]->setText (tr ("Unfreeze marker"));
        }
      else
        {
          popup[FREEZE_POPUP]->setText (tr ("Freeze marker"));
        }
      popup[FREEZE_POPUP]->setVisible (TRUE);


      //  Check for the data type of the nearest point and add the possible ancillary programs

      switch (misc.data_type)
        {
        case PFM_GSF_DATA:
          popup[5]->setText (tr ("Run ") + options.name[EXAMGSF] + " [" + options.hotkey[EXAMGSF] + "]");
          popup[5]->setVisible (TRUE);
          popup_prog[5] = EXAMGSF;
          break;
        }

      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DELETE_RECTANGLE:
      popup[0]->setText (tr ("Close rectangle and delete subrecords (beams)"));
      popup[1]->setText (tr ("Close rectangle and delete records (pings)"));
      popup[2]->setText (tr ("Discard rectangle"));
      popup[3]->setVisible (TRUE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case RESTORE_RECTANGLE:
      popup[0]->setText (tr ("Close rectangle and restore data"));
      popup[1]->setText (tr ("Discard rectangle"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DELETE_POLYGON:
      popup[0]->setText (tr ("Close polygon and delete subrecords (beams)"));
      popup[1]->setText (tr ("Close polygon and delete records (pings)"));
      popup[2]->setText (tr ("Discard polygon"));
      popup[3]->setVisible (TRUE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case KEEP_POLYGON:
      popup[0]->setText (tr ("Close polygon and delete subrecords (beams)"));
      popup[1]->setText (tr ("Discard polygon"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case RESTORE_POLYGON:
      popup[0]->setText (tr ("Close polygon and restore data"));
      popup[1]->setText (tr ("Discard polygon"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case HOTKEY_POLYGON:


      //  If we have a defined hotkey polygon, check the data type and activate the entries for
      //  possible ancillary programs.

      if (!misc.hotkey_poly_count)
        {
          popup[0]->setText (tr ("Close polygon (press hot key to run program)"));
          popup[1]->setText (tr ("Discard polygon"));
        }
      else
        {
          popup[0]->setVisible (FALSE);
          popup[1]->setVisible (FALSE);

          switch (misc.data_type)
            {
            case PFM_CHARTS_HOF_DATA:
              popup[5]->setText (tr ("Run ") + options.name[HOFRETURNKILL] + " [" + options.hotkey[HOFRETURNKILL] + "]");
              popup[5]->setVisible (TRUE);
              popup_prog[5] = HOFRETURNKILL;
              popup[6]->setText (tr ("Run ") + options.name[HOFRETURNKILL_SWA] + " [" + options.hotkey[HOFRETURNKILL_SWA] + "]");
              popup[6]->setVisible (TRUE);
              popup_prog[6] = HOFRETURNKILL_SWA;
              break;

            case PFM_SHOALS_TOF_DATA:
              break;
            }
        }

      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case RECT_FILTER_MASK:
    case POLY_FILTER_MASK:
      if (options.function == RECT_FILTER_MASK)
        {
          popup[0]->setText (tr ("Close rectangle and save mask"));
          popup[1]->setText (tr ("Discard rectangle"));
        }
      else
        {
          popup[0]->setText (tr ("Close polygon and save mask"));
          popup[1]->setText (tr ("Discard polygon"));
        }
      popup[2]->setText (tr ("Clear all masks"));
      if (options.deep_filter_only)
        {
          tmp.sprintf (tr ("Set deep filter only (Yes)").toAscii ());
        }
      else
        {
          tmp.sprintf (tr ("Set deep filter only (No)").toAscii ());
        }
      popup[3]->setText (tmp);
      popup[2]->setVisible (TRUE);
      popup[3]->setVisible (TRUE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case MASK_INSIDE_RECTANGLE:
    case MASK_OUTSIDE_RECTANGLE:
      popup[0]->setText (tr ("Close rectangle and mask data"));
      popup[1]->setText (tr ("Discard rectangle"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case MASK_INSIDE_POLYGON:
    case MASK_OUTSIDE_POLYGON:
      popup[0]->setText (tr ("Close polygon and mask data"));
      popup[1]->setText (tr ("Discard polygon"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case MEASURE:
      popup[0]->setVisible (FALSE);
      popup[1]->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case HIGHLIGHT_POLYGON:
      popup[0]->setText (tr ("Close polygon and highlight points"));
      popup[1]->setText (tr ("Discard polygon"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case CLEAR_POLYGON:
      popup[0]->setText (tr ("Close polygon and un-highlight points"));
      popup[1]->setText (tr ("Discard polygon"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;
    }

  if (misc.slice)
    {
      popup[NUMPOPUPS - 1]->setText (tr ("Turn off slicing"));
      popup[NUMPOPUPS - 1]->setVisible (TRUE);
    }
}



//  IMPORTANT NOTE:  Never use misc.nearest_point in slotPopupMenu.  Use lock_point instead.  This is set when we right-click,
//  double-click, or press a key so that subsequent mouse movement will not affect what we were trying to do when we clicked or pressed.

void 
geoSwath3D::slotPopupMenu (QAction *ac)
{
  NV_INT32 value;
  QStringList lst, items;
  QString res, item, boxSize;
  bool ok;


  void kill_records (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 *rb, NV_INT32 x, NV_INT32 y);


  //  Nice little debug statement for the next time I add a popup slot and forget to change this code.

  if (NUMPOPUPS > 8) fprintf (stderr, "%s %d - Someone tell Jan to fix this function because NUMPOPUPS has changed\n", __FILE__,__LINE__);


  popup_active = NVFalse;


  //  Clear the context menu by flushing the OpenGL scene

  map->flush ();


  //  Each of the following popup slots will execute whatever was set up in rightMouse above.  Instead of 
  //  commenting each piece and then having to change it each time I move something I'll let the rightMouse
  //  function serve as documentation for each piece.

  if (ac == popup[0])
    {
      switch (options.function)
        {
        case NOOP:
          break;

        case DELETE_POINT:
          if (lock_point != -1)
            {
              //
              //                      if we are frozen, automatically unfreeze
              //

              if (misc.marker_mode) misc.marker_mode = 0;


              //  Single point kills are a real waste of undo ;-)

              store_undo (&misc, options.undo_levels, data.val[lock_point], lock_point);
              data.val[lock_point] = PFM_MANUALLY_INVAL;
              end_undo_block (&misc);

              redrawMap (NVTrue);
            }

          break;

        case DELETE_RECTANGLE:
        case RESTORE_RECTANGLE:
        case DELETE_POLYGON:
        case RESTORE_POLYGON:
        case KEEP_POLYGON:
        case HOTKEY_POLYGON:
        case MASK_INSIDE_RECTANGLE:
        case MASK_OUTSIDE_RECTANGLE:
        case MASK_INSIDE_POLYGON:
        case MASK_OUTSIDE_POLYGON:
        case RECT_FILTER_MASK:
        case POLY_FILTER_MASK:
        case HIGHLIGHT_POLYGON:
        case CLEAR_POLYGON:
          leftMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat, menu_cursor_z);
          break;
        }
    }
  else if (ac == popup[1])
    {
      switch (options.function)
        {
        case DELETE_RECTANGLE:
          kill_records (map, &options, &misc, &data, &rb_rectangle, menu_cursor_x, menu_cursor_y);

          redrawMap (NVTrue);

          setFunctionCursor (options.function);
          break;

        case DELETE_POLYGON:
          kill_records (map, &options, &misc, &data, &rb_polygon, menu_cursor_x, menu_cursor_y);

          redrawMap (NVTrue);

          setFunctionCursor (options.function);
          break;

        default:
          midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat, menu_cursor_z);
          break;
        }
    }
  else if (ac == popup[2])
    {
      switch (options.function)
        {
        case DELETE_POINT:
          misc.highlight = (NV_INT32 *) realloc (misc.highlight, (misc.highlight_count + 1) * sizeof (NV_INT32));
          if (misc.highlight == NULL)
            {
              perror ("Allocating misc.highlight in geoSwath3D.cpp");
              exit (-1);
            }

          misc.highlight[misc.highlight_count] = lock_point;
          misc.highlight_count++;

          bClearHighlight->setEnabled (TRUE);

          redrawMap (NVFalse);
          break;

        case DELETE_RECTANGLE:
        case DELETE_POLYGON:
          midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat, menu_cursor_z);
          break;

        case RECT_FILTER_MASK:
        case POLY_FILTER_MASK:
          slotClearFilterMasks ();
          break;
        }
    }
  else if (ac == popup[3])
    {
      switch (options.function)
        {
        case RECT_FILTER_MASK:
        case POLY_FILTER_MASK:
          value = 0;
          if (!options.deep_filter_only) value = 1;

          lst << tr ("Yes") << tr ("No");

          res = QInputDialog::getItem (this, tr ("geoSwath3D"), tr ("Deep filter only:"), lst, value, FALSE, &ok);

          if (ok)
            {
              if (res.contains (tr ("Yes")))
                {
                  options.deep_filter_only = NVTrue;
                }
              else
                {
                  options.deep_filter_only = NVFalse;
                }
            }
          break;


          //  This slot is called when the Freeze/Unfreeze action has been clicked. It will set the proper text for the right
          //  context menu based on whether we are in freeze mode or not.  If we are unfreezing, we'll set the context menu
          //  text to be Freeze Point and close the focus cursor moving path for the overplot as it will be handled in
          //  slotMouseMove.  If we are freezing, we put the Unfreeze Point text on the context menu and indicate that the focus
          //  cursor is present as a legitimate multi-waveform

        case DELETE_POINT:
        case DELETE_RECTANGLE:
        case DELETE_POLYGON:

          //  Save the marker position just in case we are freezing the marker.

          misc.frozen_point = lock_point;


          if (misc.marker_mode)
            {
              //  Just in case waveMonitor is running.

              misc.abe_share->mwShare.multiPresent[0] = -1;

              misc.marker_mode = 0;
            }
          else
            {
              //  Just in case waveMonitor is running.

              misc.abe_share->mwShare.multiPresent[0] = 0;

              misc.marker_mode = 1;
            }
          break;
        }
    }
  else if (ac == popup[4])
    {
      switch (options.function)
        {
        case RECT_FILTER_MASK:
        case POLY_FILTER_MASK:
          break;
        }
    }
  else if (ac == popup[5])
    {
      runPopupProg (5);
    }
  else if (ac == popup[6])
    {
      runPopupProg (6);
    }
  else if (ac == popup[NUMPOPUPS - 1])
    {
      //  Turn off slicing if it is enabled.

      misc.slice = NVFalse;

      misc.slice_min = misc.ortho_min;
      sliceBar->setValue (misc.ortho_min);


      //  Turn on depth testing in the map widget

      map->setDepthTest (NVTrue);


      redrawMap (NVFalse);
    }
}



void 
geoSwath3D::slotPopupHelp ()
{
  switch (options.function)
    {
    case DELETE_RECTANGLE:
      QWhatsThis::showText (QCursor::pos ( ), deleteRectText, map);
      break;

    case RESTORE_RECTANGLE:
      QWhatsThis::showText (QCursor::pos ( ), restoreRectText, map);
      break;

    case DELETE_POINT:
      QWhatsThis::showText (QCursor::pos ( ), deletePointText, map);
      break;

    case DELETE_POLYGON:
      QWhatsThis::showText (QCursor::pos ( ), deletePolyText, map);
      break;

    case RESTORE_POLYGON:
      QWhatsThis::showText (QCursor::pos ( ), restorePolyText, map);
      break;

    case KEEP_POLYGON:
      QWhatsThis::showText (QCursor::pos ( ), keepPolyText, map);
      break;

    case HOTKEY_POLYGON:
      QWhatsThis::showText (QCursor::pos ( ), hotkeyPolyText, map);
      break;

    case RECT_FILTER_MASK:
      QWhatsThis::showText (QCursor::pos ( ), filterRectMaskText, map);
      break;

    case POLY_FILTER_MASK:
      QWhatsThis::showText (QCursor::pos ( ), filterPolyMaskText, map);
      break;

    case MASK_INSIDE_RECTANGLE:
      QWhatsThis::showText (QCursor::pos ( ), maskInsideRectText, map);
      break;

    case MASK_OUTSIDE_RECTANGLE:
      QWhatsThis::showText (QCursor::pos ( ), maskOutsideRectText, map);
      break;

    case MASK_INSIDE_POLYGON:
      QWhatsThis::showText (QCursor::pos ( ), maskInsidePolyText, map);
      break;

    case MASK_OUTSIDE_POLYGON:
      QWhatsThis::showText (QCursor::pos ( ), maskOutsidePolyText, map);
      break;

    case MEASURE:
      QWhatsThis::showText (QCursor::pos ( ), measureText, map);
      break;

    case HIGHLIGHT_POLYGON:
      QWhatsThis::showText (QCursor::pos ( ), highlightPolyText, map);
      break;

    case CLEAR_POLYGON:
      QWhatsThis::showText (QCursor::pos ( ), clearPolyText, map);
      break;
    }

  if (options.function != DELETE_POINT) midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat, menu_cursor_z);
}



void 
geoSwath3D::runPopupProg (NV_INT32 prog)
{
  if (options.function == HOTKEY_POLYGON && misc.hotkey_poly_count)
    {
      NV_BOOL failed = NVFalse;
      hkp = new hotkeyPolygon (this, map, &data, &options, &misc, popup_prog[prog], &failed);
      if (failed)
        {
          redrawMap (NVTrue);
        }
      else
        {
          connect (hkp, SIGNAL (hotkeyPolygonDone ()), this, SLOT (slotHotkeyPolygonDone ()));
        }
    }
  else
    {
      runProg (popup_prog[prog]);
    }
}



//  Timer - timeout signal.  Very much like an X workproc.  This tracks the cursor in associated
//  programs, not in this window.  This is active whenever the mouse leaves this window.

void
geoSwath3D::slotTrackCursor ()
{
  NV_CHAR               ltstring[25], lnstring[25];
  QString               string;
  static NV_F64_COORD2  prev_xy;


  if (misc.drawing) return;


  //  If we got a resize while drawing, redraw once we get done drawing.

  if (misc.resized)
    {
      misc.resized = NVFalse;
      redrawMap (NVTrue);
      return;
    }


  //  If we haven't frozen the marker(s) and we're not in this window, erase the marker(s).

  if (!misc.marker_mode && misc.abe_share->active_window_id != active_window_id)
    {
      for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
        {
          if (multi_marker[i] >= 0) map->closeMovingList (&(multi_marker[i]));                 
        }
    }


  //
  //                      if we've closed the waveMonitor module and we were not in single waveform mode,
  //                      close the moving paths of all the multiple waveforms and switch to single waveform
  //                      mode so we will only display our lone cursor in geoSwath3D.
  //

  if (!misc.abe_share->mwShare.waveMonitorRunning && !misc.abe_share->mwShare.multiMode)
    {
      for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++) map->closeMovingList (&(multi_marker[i]));
      misc.abe_share->mwShare.multiMode = 1;
    }


  NV_F64_COORD2 xy;
  xy.y = misc.abe_share->cursor_position.y;
  xy.x = misc.abe_share->cursor_position.x;

  if (misc.abe_share->active_window_id != active_window_id && xy.y >= misc.displayed_area.min_y && xy.y <= misc.displayed_area.max_y && 
      xy.x >= misc.displayed_area.min_x && xy.x <= misc.displayed_area.max_x && xy.y != prev_xy.y && xy.x != prev_xy.x)
    {
      //  Find the nearest point to the cursor's location in whatever window has the focus.

      NV_FLOAT64 min_dist = 999999999.0;
      NV_INT32 hit = -1;
      for (NV_INT32 i = 0 ; i < data.count ; i++)
        {
          NV_FLOAT64 dist = sqrt (((xy.y - data.y[i]) * (xy.y - data.y[i])) + ((xy.x - data.x[i]) * (xy.x - data.x[i])));

          if (dist < min_dist)
            {
              min_dist = dist;
              hit = i;
            }
        }


      if (hit >= 0)
        {
          //  First set the 3D tracker.

          NV_INT32 pix_x, pix_y;
          map->get2DCoords (data.x[hit], data.y[hit], -data.z[hit], &pix_x, &pix_y);
          map->setMovingList (&mv_tracker, marker, 16, pix_x, pix_y, 0.0, 2, options.tracker_color);


          //  Then set the 2D tracker.

          QBrush b1;
          trackMap->setMovingPath (&mv_2D_tracker, tracker_2D, data.x[hit], data.y[hit], 2, options.tracker_color,
                                   b1, NVFalse, Qt::SolidLine);
        }


      NV_FLOAT64 deg, min, sec;
      NV_CHAR    hem;

      strcpy (ltstring, fixpos (xy.y, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (xy.x, &deg, &min, &sec, &hem, POS_LON, options.position_form));

      latLabel->setText (ltstring);
      lonLabel->setText (lnstring);
    }
  prev_xy = xy;
}



//  Blink timer slot.

void
geoSwath3D::slotBlinkTimer ()
{
  static NV_INT32    blink = 0;


  //  We want to blink the mask reset button (every half_second) if we have a mask set so that the user will know.

  if (misc.mask_active)
    {
      if (blink)
        {
          bMaskReset->setIcon (QIcon (":/icons/mask_reset.xpm"));
          blink = 0;
        }
      else
        {
          bMaskReset->setIcon (QIcon (":/icons/mask_reset_red.xpm"));
          blink = 1;
        }

      qApp->processEvents ();
    }


  //  Check to see if a child process (that was started from a button) has been killed externally.

  if (misc.abe_share->killed)
    {
      for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
        {
          if (misc.abe_share->killed == options.kill_switch[i])
            {
              options.progButton[i]->setChecked (FALSE);

              misc.abe_share->killed = 0;
            }
        }
    }


  //  Another silly timer thing.  If the filter slider move counter has exceeded 1 (i.e. 1 to 1.5 seconds since last Shift+wheel move),
  //  we want to trigger the filter and reset the counter to -1.

  if (filter_active && filterMessage_slider_count >= 0) filterMessage_slider_count++;

  if (filter_active && filterMessage_slider_count > 1)
    {
      filterMessage_slider_count = -1;

      slotFilter ();
    }
}



//  Mouse press signal from the map class.

void 
geoSwath3D::slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z)
{
  //  If we're drawing and a mouse button is pressed we want to quit drawing.

  if (misc.drawing) 
    {
      misc.drawing_canceled = NVTrue;
      return;
    }


  if (e->button () == Qt::LeftButton)
    {
      //  Check for the control key modifier.  If it's set, we want to rotate the image.

      if (e->modifiers () == Qt::ControlModifier)
        {
	  //  Get the cursor position at this time.

          start_ctrl_x = e->x ();
          start_ctrl_y = e->y ();

          misc.save_function = options.function;
          options.function = ROTATE;
          setFunctionCursor (options.function);
        }
      else
        {
          if (options.function == DELETE_POINT)
            {
              if (lock_point != -1)
                {
                  //  Check for the shift key modifier.  If it's set, we want to delete the point.

                  if (e->modifiers () == Qt::ShiftModifier)
                    {
                      //  If we are frozen, automatically unfreeze

                      if (misc.marker_mode) misc.marker_mode = 0;

                      store_undo (&misc, options.undo_levels, data.val[misc.nearest_point], misc.nearest_point);
                      data.val[misc.nearest_point] = PFM_MANUALLY_INVAL;
                      end_undo_block (&misc);
                    }
                }
            }

          leftMouse (e->x (), e->y (), lon, lat, z);
        }
    }

  if (e->button () == Qt::MidButton)
    {
      //  Check for the control key modifier.  If it's set, we want to center on the cursor.

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->setMapCenter (lon, lat, z);

          overlayData (map, &options, &misc, &data);
        }
      else
        {
          midMouse (e->x (), e->y (), lon, lat, z);
        }
    }

  if (e->button () == Qt::RightButton)
    {
      //  Check for the control key modifier.  If it's set, we want to zoom based on cursor movement.

      if (e->modifiers () == Qt::ControlModifier)
        {
          start_ctrl_y = e->y ();

          misc.save_function = options.function;
          options.function = ZOOM;
          setFunctionCursor (options.function);
        }
      else
        {
          rightMouse (e->x (), e->y (), lon, lat, z);
        }
    }
}



//  Mouse release signal from the map class.

void 
geoSwath3D::slotMouseRelease (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)),
                           NV_FLOAT64 lat __attribute__ ((unused)), NV_FLOAT64 z __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton)
    {
      if (options.function == ROTATE)
        {
          options.function = misc.save_function;
          setFunctionCursor (options.function);

          overlayData (map, &options, &misc, &data);
        }
      else
        {
          popup_active = NVFalse;
        }
    }

  if (e->button () == Qt::MidButton) popup_active = NVFalse;

  if (e->button () == Qt::RightButton)
    {
      if (options.function == ZOOM)
        {
          options.function = misc.save_function;
          setFunctionCursor (options.function);

          overlayData (map, &options, &misc, &data);
        }
    }
}



void 
geoSwath3D::geo_z_label (NV_FLOAT64 lat, NV_FLOAT64 lon, NV_FLOAT64 z)
{
  NV_CHAR ltstring[25], lnstring[25], hem;
  QString geo_string, val_string;
  NV_FLOAT64 deg, min, sec;
  static NV_FLOAT32 prev_z_factor = 1.0, prev_z_offset = 0.0;


  strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
  strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));

  latLabel->setText (ltstring);
  lonLabel->setText (lnstring);


  val_string.sprintf ("%.2f", z * options.z_factor + options.z_offset);


  //  Set yellow background for scaled or offset data.

  if ((options.z_factor != 1.0 && prev_z_factor == 1.0) || (options.z_offset != 0.0 && prev_z_offset == 0.0))
    {
      zPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      zLabel->setPalette (zPalette);
      zLabel->setToolTip (tr ("Scaled/offset current point Z value"));
      zLabel->setWhatsThis (tr ("Scaled/offset current point Z value"));
    }
  else if ((options.z_factor == 1.0 && prev_z_factor != 1.0) || (options.z_offset == 0.0 && prev_z_offset != 0.0))
    {
      zPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
      zLabel->setPalette (zPalette);
      zLabel->setToolTip (tr ("Current point Z value"));
      zLabel->setWhatsThis (tr ("Current point Z value"));
    }
  zLabel->setText (val_string);

  prev_z_factor = options.z_factor;
  prev_z_offset = options.z_offset;
}



//  Mouse wheel signal from the map class.

void
geoSwath3D::slotWheel (QWheelEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)),
                      NV_FLOAT64 z __attribute__ ((unused)))
{
  if (e->delta () > 0)
    {
      //  Zoom in or out when pressing the Ctrl key and using the mouse wheel.

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->zoomInPercent ();

          overlayData (map, &options, &misc, &data);
        }


      //  Change the filter dialog standard deviation when pressing the Shift key and using the mouse wheel.

      else if (e->modifiers () == Qt::ShiftModifier && filter_active)
        {
          filterMessage_slider_count = 0;
          stdSlider->triggerAction (QAbstractSlider::SliderSingleStepSub);
        }


      //  Otherwise we're slicing.

      else
        {
          slotSliceTriggered (QAbstractSlider::SliderSingleStepSub);
        }
    }
  else
    {
      //  Zoom in or out when pressing the Ctrl key and using the mouse wheel.

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->zoomOutPercent ();

          overlayData (map, &options, &misc, &data);
        }


      //  Change the filter dialog standard deviation when pressing the Shift key and using the mouse wheel.

      else if (e->modifiers () == Qt::ShiftModifier && filter_active)
        {
          filterMessage_slider_count = 0;
          stdSlider->triggerAction (QAbstractSlider::SliderSingleStepAdd);
        }


      //  Otherwise we're slicing.

      else
        {
          slotSliceTriggered (QAbstractSlider::SliderSingleStepAdd);
        }
    }
}



//  Mouse move signal from the map class.

void
geoSwath3D::slotMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z, NVMAPGL_DEF l_mapdef)
{
  NV_I32_COORD2             xy;
  NV_F64_COORD3             hot = {999.0, 999.0, 999.0};
  static NV_I32_COORD2      prev_xy = {-1, -1};
  static NV_F64_COORD3      prev_hot;
  NV_FLOAT64                dist;
  static NV_INT32           prev_nearest_point = -1;


  void get_nearest_kill_point (MISC *misc, POINT_DATA *data, NV_FLOAT64 lat, NV_FLOAT64 lon, NV_F64_COORD3 *hot);
  NV_BOOL compare_to_stack (NV_INT32 current_point, NV_FLOAT64 dist, MISC *misc);


  //  Let other ABE programs know which window we're in.

  misc.abe_share->active_window_id = active_window_id;


  //  If it's still drawing don't do anything

  if (misc.drawing) return;


  //  Get rid of the tracking cursor from slotTrackCursor ().  But only if it already exists, otherwise we
  //  will be creating a new one (which we don't want to do).

  if (mv_tracker >= 0) map->closeMovingList (&mv_tracker);
  if (mv_2D_tracker >= 0) trackMap->closeMovingPath (&mv_2D_tracker);


  xy.x = e->x ();
  xy.y = e->y ();


  //  Track the cursor (not the marker) position for other ABE programs but only if we're in plan view

  misc.abe_share->cursor_position.y = lat;
  misc.abe_share->cursor_position.x = lon;
  misc.abe_share->cursor_position.z = z;


  //  Only process if the pointer position has changed pixels and we haven't frozen all of the markers.

  if (xy.x == prev_xy.x && xy.y == prev_xy.y) return;


  //  If we've frozen the central cursor or all of the cursors we want to draw them regardless of the mode we're in.

  if (misc.marker_mode)
    {
      //  If we're in multicursor mode, draw all the cursors.

      if (misc.marker_mode == 2)
        {
          DrawMultiCursors ();
        }
      else
        { 
          NV_INT32 pix_x, pix_y;
          map->get2DCoords (data.x[misc.frozen_point], data.y[misc.frozen_point], -data.z[misc.frozen_point], &pix_x, &pix_y);

          map->setMovingList (&(multi_marker[0]), marker, 16, pix_x, pix_y, 0.0, 2, misc.abe_share->mwShare.multiColors[0]);
        }
    }


  QString rec_string = "", sub_string = "", distance_string = "", h_string = "", v_string = "";
  NV_FLOAT64 distance, azimuth;
  NV_CHAR nearest_line[512];


  switch (options.function)
    {
    case ROTATE:
      if (start_ctrl_x != xy.x || start_ctrl_y != xy.y)
	{
	  NV_INT32 diff_x = xy.x - start_ctrl_x;


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
		  if ((xy.y < half && ((zxrot > 0.0 && zxrot < 180.0) || (zxrot < -180.0))) ||
		      (xy.y > half && ((zxrot < 0.0 && zxrot > -180.0) || (zxrot > 180.0)))) diff_x = -diff_x;
		}

              map->rotateY ((NV_FLOAT64) diff_x / 5.0);
            }

	  NV_INT32 diff_y = xy.y - start_ctrl_y;

	  if (diff_y) map->rotateZX ((NV_FLOAT64) diff_y / 5.0);

	  start_ctrl_x = xy.x;
	  start_ctrl_y = xy.y;
	}
      break;

    case ZOOM:
      if (start_ctrl_y != xy.y)
	{
	  NV_INT32 diff_y = xy.y - start_ctrl_y;


	  if (diff_y < -10)
            {
              map->zoomInPercent ();
              start_ctrl_y = xy.y;
            }
          else if (diff_y > 10)
            {
              map->zoomOutPercent ();
              start_ctrl_y = xy.y;
            }
	}
      break;

    case NOOP:
    case DELETE_POINT:
    case MEASURE:

      //  Draw the frozen multi_markers.

      if (misc.marker_mode == 2)
        {
          //  If we're in multicursor mode, draw all the cursors.

          if (!misc.abe_share->mwShare.multiMode)
            {
              DrawMultiCursors ();
            }
          else
            {
              NV_INT32 pix_x, pix_y;
              map->get2DCoords (data.x[misc.frozen_point], data.y[misc.frozen_point], -data.z[misc.frozen_point], &pix_x, &pix_y);

              map->setMovingList (&(multi_marker[0]), marker, 16, pix_x, pix_y, 0.0, 2, misc.abe_share->mwShare.multiColors[0]);
            }
        }
      else
        {
          //  Clear the nearest point stack.

          for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
            {
              misc.nearest_stack.point[i] = -1;
              misc.nearest_stack.dist[i] = 9999999999.0;
            }


          misc.nearest_point = -1;


          //  If we've got filter kill points on screen we want to find the nearest kill point first and lock it in before finding
          //  the nearest stack points.  This allows us to snap to only kill points but then get the surrounding valid points
          //  whether they're kill points or not.  This will set the nearest_point, the hot.x and hot.y positions, and the stack[0]
          //  point and distance (forced to -999.0 so it can't be replaced).

          get_nearest_kill_point (&misc, &data, lat, lon, &hot);


          //  Find the valid point nearest (geographically) to the cursor.

          for (NV_INT32 i = 0 ; i < data.count ; i++)
            {
              //  Do not use invalid points unless the display_invalid flag is set.
              //  Do not use masked points. Do not check points that are not on the display.

              if (!check_bounds (&options, &misc, &data, i, NVTrue, misc.slice))
                {
                  dist = sqrt ((NV_FLOAT64) ((lat - data.y[i]) * (lat - data.y[i])) + (NV_FLOAT64) ((lon - data.x[i]) * (lon - data.x[i])));


                  //  Check the points against the points in the nearest points stack.

                  if (compare_to_stack (i, dist, &misc))
                    {
                      //  If the return was true then this is the minimum distance so far.

                      misc.nearest_point = misc.nearest_stack.point[0];

                      hot.x = data.x[i];
                      hot.y = data.y[i];
                      hot.z = data.z[i];
                    }
                }
            }


          //  Only track the nearest position if we haven't frozen the marker.

          if (!misc.marker_mode)
            {
              misc.abe_share->cursor_position.y = data.y[misc.nearest_point];
              misc.abe_share->cursor_position.x = data.x[misc.nearest_point];
            }


          //  Update the status bars

          // 
          //                      if our point has moved or we have just selected a new overplot we will go
          //                      into this code block.
          //

          if (misc.nearest_point != -1 && misc.nearest_point != prev_nearest_point)
            {
              //  Show distance and azimuth to last highlighted point in status bar

              if (misc.highlight_count)
                {
                  invgp (NV_A0, NV_B0, data.y[misc.highlight[misc.highlight_count - 1]], data.x[misc.highlight[misc.highlight_count - 1]],
                         data.y[misc.nearest_point], data.x[misc.nearest_point], &distance, &azimuth);

                  if (distance < 0.01) azimuth = 0.0;

                  distance_string.sprintf ("Distance : %.2f (m)  Azimuth : %.2f (degrees)", distance, azimuth);

                  misc.statusProgLabel->setToolTip (tr ("Distance and azimuth from last highlighted point"));
                  misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
                  misc.statusProgLabel->setPalette (misc.statusProgPalette);
                  misc.statusProgLabel->setText (distance_string);
                }
              else
                {
                  misc.statusProgLabel->setToolTip ("");
                }


              //  Locking shared memory.

              misc.abeShare->lock ();


              //  Only change the nearest filename if we aren't locked.  External programs monitor
              //  abe_share->nearest_point to trigger updates of their windows.  They use
              //  abe_share->mwShare.multiRecord[0] for the current record to display.  In this way
              //  we can lock the main screen by not updating abe_share->mwShare.multiRecord[0] and still
              //  trigger the external programs to update (such as for multiple waveforms in waveMonitor or
              //  waveWaterfall).  When we do this we don't want nearest_filename or nearest_line to change
              //  either since that is where the external programs get the filename to use with 
              //  abe_share->mwShare.multiRecord[0].

              if (!misc.marker_mode) strcpy (nearest_line, misc.file.toAscii ());


              //  Push the nearest stack points into the ABE shared memory for use by waveMonitor, waveWaterfall,
              //  and lidarMonitor.

              for (NV_INT32 j = 0 ; j < MAX_STACK_POINTS ; j++)
                {
                  //  Normal, unfrozen mode.

                  if (!misc.marker_mode)
                    {
                      misc.abe_share->mwShare.multiSubrecord[j] = data.sub[misc.nearest_stack.point[j]];
                      misc.abe_share->mwShare.multiPoint[j].x = data.x[misc.nearest_stack.point[j]];
                      misc.abe_share->mwShare.multiPoint[j].y = data.y[misc.nearest_stack.point[j]];
                      misc.abe_share->mwShare.multiPoint[j].z = data.z[misc.nearest_stack.point[j]];
                      misc.abe_share->mwShare.multiLocation[j] = misc.nearest_stack.point[j];
                      misc.abe_share->mwShare.multiType[j] = misc.data_type;
                      misc.abe_share->mwShare.multiPresent[j] = data.rec[misc.nearest_stack.point[j]];
                      misc.abe_share->mwShare.multiRecord[j] = data.rec[misc.nearest_stack.point[j]];
                    }


                  //  If we're frozen, we don't want to replace the [0] record and we want to push the nearest
                  //  MAX_STACK_POINTS - 1 records up one level.

                  else
                    {
                      if (j)
                        {
                          misc.abe_share->mwShare.multiSubrecord[j] = data.sub[misc.nearest_stack.point[j - 1]];
                          misc.abe_share->mwShare.multiPoint[j].x = data.x[misc.nearest_stack.point[j - 1]];
                          misc.abe_share->mwShare.multiPoint[j].y = data.y[misc.nearest_stack.point[j - 1]];
                          misc.abe_share->mwShare.multiPoint[j].z = data.z[misc.nearest_stack.point[j - 1]];
                          misc.abe_share->mwShare.multiLocation[j] = misc.nearest_stack.point[j - 1];
                          misc.abe_share->mwShare.multiType[j] = misc.data_type;
                          misc.abe_share->mwShare.multiPresent[j] = data.rec[misc.nearest_stack.point[j - 1]];
                          misc.abe_share->mwShare.multiRecord[j] = data.rec[misc.nearest_stack.point[j - 1]];
                        }
                    }
                }
              misc.abe_share->mwShare.multiNum = MAX_STACK_POINTS;


              //  Trigger some of the external applications to update.

              misc.abe_share->nearest_point = misc.nearest_point;


              //  Unlock shared memory.

              misc.abeShare->unlock ();


              geo_z_label (data.y[misc.nearest_point],  data.x[misc.nearest_point], data.z[misc.nearest_point]);


              rec_string.sprintf ("%d", misc.abe_share->mwShare.multiRecord[0]);
              sub_string.sprintf ("%d", misc.abe_share->mwShare.multiSubrecord[0]);

              recLabel->setText (rec_string);
              subLabel->setText (sub_string);
            }


          //  Set the delete, set, unset point marker.

          if (!misc.abe_share->mwShare.multiMode && (options.function == DELETE_POINT || options.function == NOOP))
            {
              DrawMultiCursors ();
            }
          else
            {
              //  If we're frozen, use the frozen point for the marker position.

              if (misc.marker_mode)
                {
                  hot.x = data.x[misc.frozen_point];
                  hot.y = data.y[misc.frozen_point];
                  hot.z = data.z[misc.frozen_point];
                }

              NV_INT32 pix_x, pix_y;
              map->get2DCoords (hot.x, hot.y, -hot.z, &pix_x, &pix_y);

              map->setMovingList (&(multi_marker[0]), marker, 16, pix_x, pix_y, 0.0, 2, 
                                  misc.abe_share->mwShare.multiColors[0]);
            }


          if (options.function == MEASURE)
            {
              if (map->rubberbandLineIsActive (rb_measure))
                {
                  //  Snap the end of the line to the nearest point

                  NV_INT32 pix_x, pix_y;
                  map->get2DCoords (data.x[misc.nearest_point], data.y[misc.nearest_point], -data.z[misc.nearest_point],
                                    &pix_x, &pix_y);

                  map->dragRubberbandLine (rb_measure, pix_x, pix_y);


                  invgp (NV_A0, NV_B0, data.y[misc.nearest_point], data.x[misc.nearest_point],
                         line_anchor.y, line_anchor.x, &distance, &azimuth);

                  if (distance < 0.01) azimuth = 0.0;

                  distance_string.sprintf ("Dist: %.2f (m)   Az: %.2f (degrees)  deltaZ: %.2f", distance, azimuth,
                                           line_anchor.z - data.z[misc.nearest_point]);

                  misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
                  misc.statusProgLabel->setPalette (misc.statusProgPalette);
                  misc.statusProgLabel->setText (distance_string);
                }
            }
        }


      //  Set the previous nearest point positions.
             
      prev_hot = hot;
      break;


    case DELETE_RECTANGLE:
    case RESTORE_RECTANGLE:
    case MASK_INSIDE_RECTANGLE:
    case MASK_OUTSIDE_RECTANGLE:
    case RECT_FILTER_MASK:
      if (map->rubberbandRectangleIsActive (rb_rectangle)) map->dragRubberbandRectangle (rb_rectangle, xy.x, xy.y);

      geo_z_label (lat, lon, z);
      recLabel->setText (rec_string);
      subLabel->setText (sub_string);
      break;

    case DELETE_POLYGON:
    case RESTORE_POLYGON:
    case KEEP_POLYGON:
    case HOTKEY_POLYGON:
    case MASK_INSIDE_POLYGON:
    case MASK_OUTSIDE_POLYGON:
    case POLY_FILTER_MASK:
    case HIGHLIGHT_POLYGON:
    case CLEAR_POLYGON:

      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          if (!polygon_flip)
            {
              polygon_flip = NVTrue;

              if (prev_poly_x > -1)
                {
                  map->vertexRubberbandPolygon (rb_polygon, prev_poly_x, prev_poly_y);
                }
              else
                {
                  map->vertexRubberbandPolygon (rb_polygon, xy.x, xy.y);
                }
            }
          else
            {
              map->dragRubberbandPolygon (rb_polygon, xy.x, xy.y);
              prev_poly_x = xy.x;
              prev_poly_y = xy.y;
              polygon_flip = NVFalse;
            }
        }

      geo_z_label (lat, lon, z);
      recLabel->setText (rec_string);
      subLabel->setText (sub_string);

      break;
    }


  //  Track the cursor in the 2D tracker box.

  QBrush b1;
  trackMap->setMovingPath (&mv_2D_tracker, tracker_2D, lon, lat, 2, options.tracker_color, b1, NVFalse, Qt::SolidLine);


  //  Set the previous cursor.

  prev_xy = xy;

  prev_nearest_point = misc.nearest_point;
}



//
//                      DrawMultiCursors
//
//                      This method will loop through either the valid overplots or the 
//                      nearest neighbors, and place the multi-cursor on the proper shot
//                      whether we are in a normal top-down view or a sliced view.
//

void 
geoSwath3D::DrawMultiCursors ()
{
  NV_BOOL withinSlice;                     //       boolean check to see if shot is within current volumetric slice

           
  for (NV_INT32 i = MAX_STACK_POINTS - 1 ; i >= 0 ; i--) 
    {
      withinSlice = NVTrue;
     
      if ((misc.abe_share->mwShare.multiPoint[i].x >= misc.displayed_area.min_x) && 
          (misc.abe_share->mwShare.multiPoint[i].x <= misc.displayed_area.max_x) && 
          (misc.abe_share->mwShare.multiPoint[i].y >= misc.displayed_area.min_y) && 
          (misc.abe_share->mwShare.multiPoint[i].y <= misc.displayed_area.max_y) &&
          (misc.abe_share->mwShare.multiPresent[i] != -1) && (withinSlice))
        {
          NV_INT32 pix_x, pix_y;
          map->get2DCoords (misc.abe_share->mwShare.multiPoint[i].x, misc.abe_share->mwShare.multiPoint[i].y,
                            -misc.abe_share->mwShare.multiPoint[i].z, &pix_x, &pix_y);

          map->setMovingList (&(multi_marker[i]), marker, 16, pix_x, pix_y, 0.0, 2, 
                              misc.abe_share->mwShare.multiColors[i]);
        }
      else
        {
          map->closeMovingList (&(multi_marker[i]));
        }
    }
}



//  Resize signal from the map class.

void
geoSwath3D::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  //  Set the flag for a delayed redraw (in trackCursor) if we are currently drawing when we get a resize.

  misc.resized = NVTrue;


  if (!misc.drawing) redrawMap (NVTrue);
}



//  This is where we load all of our data in to OpenGL display lists (in nvMapGL.cpp).

void
geoSwath3D::redrawMap (NV_BOOL redraw2D)
{
  void setScale (NV_FLOAT32 actual_min, NV_FLOAT32 actual_max, MISC *misc);
  NV_BOOL compute_bounds (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 *viewable_count, NV_F64_XYMBC *bounds);


  //  If we don't have a file opened we don't want to try to draw anything.

  if (!file_opened) return;


  misc.drawing = NVTrue;


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  discardMovableObjects ();


  //  Erase all text from the map.

  map->clearText ();


  //  Erase all objects (like spheres) from the map.

  map->clearDisplayLists ();


  //  Erase all data points from the map.

  map->clearDataPoints ();


  setMainButtons (NVFalse);


  NVMAPGL_DEF l_mapdef = map->getMapdef ();


  misc.draw_area_height = l_mapdef.draw_height;
  misc.draw_area_width = l_mapdef.draw_width;

  map->setCursor (Qt::WaitCursor);

  misc.drawing_canceled = NVFalse;


  //  Compute the Z range of the data.

  NV_INT32 viewable_count;

  bounds = data.bounds;
  bounds.min_z = CHRTRNULL;
  bounds.max_z = -CHRTRNULL;


  //  Compute the min and max values for defining the 3D space and optionally for coloring.

  NV_BOOL bounds_changed = compute_bounds (map, &options, &misc, &data, &viewable_count, &bounds);


  //  Only display if there are viewable points.

  if (viewable_count)
    {
      setScale (bounds.min_z, bounds.max_z, &misc);


      range_x = bounds.max_x - bounds.min_x;
      range_y = bounds.max_y - bounds.min_y;
      range_z = bounds.max_z - bounds.min_z;


      //  Add 10 percent to the X, Y, and Z ranges.

      bounds.min_x -= (range_x * 0.10);
      bounds.max_x += (range_x * 0.10);

      bounds.min_y -= (range_y * 0.10);
      bounds.max_y += (range_y * 0.10);

      bounds.min_z -= (range_z * 0.10);
      bounds.max_z += (range_z * 0.10);
      range_z = bounds.max_z - bounds.min_z;


      if (bounds_changed)
        {
          map->setBounds (bounds);
          trackMap->resetBounds (bounds);
        }


      if (!misc.slice) compute_ortho_values (map, &misc, &options, &data, sliceBar, NVTrue);


      NV_INT32 c_index = -1;
      NV_U_BYTE save_min[4];
      NV_FLOAT32 z_value;


      for (NV_INT32 i = 0 ; i < data.count ; i++)
        {
          //  Check against the viewing area.

          NV_INT32 trans = 0;
          if ((trans = check_bounds (&options, &misc, &data, i, NVTrue, misc.slice)) < 2)
            {
              z_value = data.z[i];


              //  Color by depth

              c_index = NINT ((NUMSHADES - 1) - ((-data.z[i]) - bounds.min_z) / range_z * (NV_FLOAT32) (NUMSHADES - 1));


              NV_U_BYTE tmp[4];

              if (c_index < 0)
                {
                  DPRINT
                    //tmp = ?????
                }
              else
                {
                  memcpy (tmp, misc.color_array[trans][c_index], 4);
                }

              map->setDataPoints (data.x[i], data.y[i], -z_value, tmp, 0, NVFalse);
            }
        }


      map->setDataPoints (0.0, 0.0, 0.0, save_min, options.point_size, NVTrue);


      if (!misc.drawing_canceled)
        {
          overlayData (map, &options, &misc, &data);
          overlayFlag (map, &options, &misc, &data);
        }
    }


  //  We want to flush the map just in case we didn't have any points to draw.

  map->flush ();


  //  Redraw the 2D tracker unless this redraw was caused by an operation that has no effect on the 2D map.

  trackMap->enableSignals ();
  if (redraw2D) trackMap->redrawMap (NVTrue);


  misc.drawing = NVFalse;

  setMainButtons (NVTrue);

  setFunctionCursor (options.function);

  qApp->restoreOverrideCursor ();
}



void 
geoSwath3D::slotMaskReset ()
{
  for (NV_INT32 i = 0 ; i < data.count ; i++) data.mask[i] = NVFalse;
  misc.mask_active = NVFalse;
  bMaskReset->setIcon (QIcon (":/icons/mask_reset.xpm"));


  //  Turn off slicing if it was on (this will also call redrawMap ()).

  slotPopupMenu (popup[NUMPOPUPS - 1]);
}



void 
geoSwath3D::slotReset ()
{
  misc.poly_count = 0;

  misc.slice = NVFalse;

  misc.slice_min = misc.ortho_min;
  sliceBar->setValue (misc.ortho_min);


  //  Turn on depth testing in the map widget

  map->setDepthTest (NVTrue);


  discardMovableObjects ();


  map->setMapCenter (misc.map_center_x, misc.map_center_y, misc.map_center_z);


  map->resetPOV ();


  redrawMap (NVFalse);
}



void 
geoSwath3D::slotFileValueChanged (int value)
{
  if (fileBarEnabled)
    {
      map->setCursor (Qt::WaitCursor);
      qApp->processEvents ();


      //  Clear out the undo memory since we've moved on to another buffer.

      for (NV_INT32 i = 0 ; i < misc.undo_count ; i++)
        {
          if (misc.undo[i].count)
            {
              free (misc.undo[i].val);
              free (misc.undo[i].num);
              misc.undo[i].val = NULL;
              misc.undo[i].num = NULL;
            }
        }
      misc.undo_count = 0;


      get_buffer (&data, &misc, value);


      QString cntText = tr ("Number of points: %1").arg (data.count);
      numLabel->setText (cntText);


      map->setBounds (data.bounds);
      trackMap->resetBounds (data.bounds);


      //  Turn off slicing.

      misc.slice = NVFalse;

      misc.slice_min = misc.ortho_min;
      sliceBar->setValue (misc.ortho_min);

      redrawMap (NVTrue);


      //  Get the map center for the reset view slot.

      map->getMapCenter (&misc.map_center_x, &misc.map_center_y, &misc.map_center_z);
    }
}



void 
geoSwath3D::slotSliceTriggered (int action)
{
  if (!misc.drawing)
    {
      //  Have to compute the ortho values and set the slider to minimum before we slice for the first time.

      if (!misc.slice) compute_ortho_values (map, &misc, &options, &data, sliceBar, NVTrue);


      misc.slice = NVTrue;
      map->setDepthTest (NVFalse);

      switch (action)
        {
        case QAbstractSlider::SliderSingleStepAdd:
        case QAbstractSlider::SliderPageStepAdd:
          misc.slice_min -= misc.slice_size;
          misc.slice_max = misc.slice_min + misc.slice_size;
          break;

        case QAbstractSlider::SliderSingleStepSub:
        case QAbstractSlider::SliderPageStepSub:
          misc.slice_max += misc.slice_size;
          misc.slice_min = misc.slice_max - misc.slice_size;
          break;

        case QAbstractSlider::SliderMove:
          misc.slice_min = sliceBar->value ();
          misc.slice_max = misc.slice_min + misc.slice_size;
          break;
        }


      if (misc.slice_min < misc.ortho_min)
        {
          misc.slice_min = misc.ortho_min;
          misc.slice_max = misc.ortho_min + misc.slice_size;
        }

      if (action != QAbstractSlider::SliderMove) sliceBar->setValue (misc.slice_min);

      slicing = NVTrue;
      redrawMap (NVFalse);
      slicing = NVFalse;


      //  Even though we do this in redrawMap we need to do it again here because otherwise the damn markers disappear.
      //  It's OpenGL, I have no idea why.

      overlayData (map, &options, &misc, &data);
    }
}



void 
geoSwath3D::slotSizeTriggered (int action)
{
  if (!misc.drawing)
    {
      switch (action)
        {
        case QAbstractSlider::SliderSingleStepAdd:
        case QAbstractSlider::SliderPageStepAdd:
          options.slice_percent--;
          if (options.slice_percent <= 0) options.slice_percent = 1;
          break;

        case QAbstractSlider::SliderSingleStepSub:
        case QAbstractSlider::SliderPageStepSub:
          options.slice_percent++;
          if (options.slice_percent > MAX_SLICE_SIZE) options.slice_percent = MAX_SLICE_SIZE;
          break;

        case QAbstractSlider::SliderMove:
          options.slice_percent = sizeBar->value ();
          break;
        }


      if (action != QAbstractSlider::SliderMove) sizeBar->setValue (options.slice_percent);


      NV_INT32 val = sliceBar->value ();

      sizeLabel->setNum (options.slice_percent);

      compute_ortho_values (map, &misc, &options, &data, sliceBar, NVFalse);
      map->setDepthTest (NVFalse);

      misc.slice_min = val;
      misc.slice_max = misc.slice_min + misc.slice_size;

      if (misc.slice_max > misc.ortho_max)
        {
          misc.slice_min = misc.ortho_max - misc.slice_size;
          misc.slice_max = misc.ortho_max;
        }

      slicing = NVTrue;
      redrawMap (NVFalse);
      slicing = NVFalse;


      //  Even though we do this in redrawMap we need to do it again here because otherwise the damn markers disappear.
      //  It's OpenGL, I have no idea why.

      overlayData (map, &options, &misc, &data);
    }
}



void 
geoSwath3D::slotTransTriggered (int action)
{
  if (!misc.drawing)
    {
      switch (action)
        {
        case QAbstractSlider::SliderSingleStepAdd:
        case QAbstractSlider::SliderPageStepAdd:
          options.slice_alpha--;
          if (options.slice_alpha < 0) options.slice_alpha = 1;
          break;

        case QAbstractSlider::SliderSingleStepSub:
        case QAbstractSlider::SliderPageStepSub:
          options.slice_alpha++;
          if (options.slice_alpha > MAX_TRANS_VALUE) options.slice_alpha = MAX_TRANS_VALUE;
          break;

        case QAbstractSlider::SliderMove:
          options.slice_alpha = transBar->value ();
          break;
        }


      if (action != QAbstractSlider::SliderMove) transBar->setValue (options.slice_alpha);


      //  Set the semi-transparent colors for slicing.

      NV_FLOAT32 hue_inc = 315.0 / (NV_FLOAT32) (NUMSHADES + 1);

      for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
	{
	  QColor color;
	  color.setHsv ((NV_INT32) (j * hue_inc), 255, 255, 255);
	  misc.color_array[0][j][0] = misc.color_array[1][j][0] = color.red ();
	  misc.color_array[0][j][1] = misc.color_array[1][j][1] = color.green ();
	  misc.color_array[0][j][2] = misc.color_array[1][j][2] = color.blue ();
	  misc.color_array[0][j][3] = color.alpha ();

	  misc.color_array[1][j][3] = options.slice_alpha;
	}


      QString string;
      string.sprintf ("%02d", options.slice_alpha);
      transLabel->setText (string);

      redrawMap (NVFalse);
    }
}



void 
geoSwath3D::slotExagTriggered (int action)
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

              redrawMap (NVFalse);
            }
          break;

        case QAbstractSlider::SliderPageStepAdd:
          if (options.exaggeration >= 5.0)
            {
              options.exaggeration -= 5.0;

              if (options.exaggeration < 1.0) options.exaggeration = 1.0;

              map->setExaggeration (options.exaggeration);

              redrawMap (NVFalse);
            }
          break;

        case QAbstractSlider::SliderSingleStepSub:
          options.exaggeration += 1.0;
          if (options.exaggeration > 100.0) options.exaggeration = 100.0;
          map->setExaggeration (options.exaggeration);

          redrawMap (NVFalse);

          break;

        case QAbstractSlider::SliderPageStepSub:
          options.exaggeration += 5.0;
          if (options.exaggeration > 100.0) options.exaggeration = 100.0;
          map->setExaggeration (options.exaggeration);

          redrawMap (NVFalse);

          break;

        case QAbstractSlider::SliderMove:
          QString lbl;
          lbl.sprintf (tr ("%5.3f").toAscii (), (NV_FLOAT32) exagBar->value () / 100.0);
          exagLabel->setText (lbl);
          break;
        }


      //  Even though we do this in redrawMap we need to do it again here because otherwise the damn markers disappear.
      //  It's OpenGL, I have no idea why.

      overlayData (map, &options, &misc, &data);
    }
}



void 
geoSwath3D::slotExagReleased ()
{
  options.exaggeration = (NV_FLOAT32) exagBar->value () / 100.0;
  map->setExaggeration (options.exaggeration);

  redrawMap (NVFalse);
}



void 
geoSwath3D::slotExaggerationChanged (NV_FLOAT32 exaggeration, NV_FLOAT32 apparent_exaggeration)
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
  lbl.sprintf (tr ("%5.3f").toAscii (), apparent_exaggeration);
  exagLabel->setText (lbl);

  options.exaggeration = exaggeration;

  disconnect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
  exagBar->setValue (NINT (options.exaggeration * 100.0));
  connect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
}



void
geoSwath3D::keyPressEvent (QKeyEvent *e)
{
  void actionKey (geoSwath3D *parent, nvMapGL *map, POINT_DATA *data, OPTIONS *options, MISC *misc, QString key, NV_INT32 lock_point);


  //  Lock in nearest point so mouse moves won't change it.

  lock_point = misc.nearest_point;


  //  If we're drawing and a key is pressed we want to cancel drawing.

  if (misc.drawing)
    {
      misc.drawing_canceled = NVTrue;
      return;
    }


  //  First check for the hard-wired keys.

  switch (e->key ())
    {
    case Qt::Key_Escape:

      //  Turn off slicing if it was on (this will also call redrawMap ()).

      if (misc.slice) slotPopupMenu (popup[NUMPOPUPS - 1]);

      return;
      break;

    case Qt::Key_Return:
      return;
      break;

    case Qt::Key_Left:
      map->rotateY (-options.rotation_increment);
      return;
      break;

    case Qt::Key_PageUp:
      if (e->modifiers () == Qt::ControlModifier)
        {
          slotExagTriggered (QAbstractSlider::SliderSingleStepSub);
        }
      else
        {
          fileBar->triggerAction (QAbstractSlider::SliderPageStepSub);
        }

      return;
      break;

    case Qt::Key_Right:
      map->rotateY (options.rotation_increment);
      return;
      break;

    case Qt::Key_PageDown:
      if (e->modifiers () == Qt::ControlModifier)
        {
          slotExagTriggered (QAbstractSlider::SliderSingleStepAdd);
        }
      else
        {
          fileBar->triggerAction (QAbstractSlider::SliderPageStepAdd);
        }

      return;
      break;

    case Qt::Key_Up:

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->rotateZX (-options.rotation_increment);
        }
      else
        {
          slotSliceTriggered (QAbstractSlider::SliderSingleStepSub);
        }
      return;
      break;

    case Qt::Key_Down:

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->rotateZX (options.rotation_increment);
        }
      else
        {
          //  If we're not already slicing, do nothing.

          if (misc.slice) slotSliceTriggered (QAbstractSlider::SliderSingleStepAdd);
        }
      return;
      break;
    }


  //  Looking for ancillary program hot keys and action keys.

  NV_BOOL hit = NVFalse;
  QString key = e->text ();


  //  Get the keystring.  We need this for Del, Ins, Home, or End if we want to use them for ancillary program hot keys.

  QKeySequence keySeq = e->key ();
  QString keyString = keySeq.toString ();
  if (keyString == "Del" || keyString == "Ins" || keyString == "Home" || keyString == "End") key = keyString;


  //  If key is NULL this is a modifier so we don't want to check it against the hot/action keys.

  if (key == "") return;


  //  Concatenate the (single) modifier and the key value.  There's probably a better way to do this but I don't know
  //  what it is at the moment.

  QString modifier = "";
  if (e->modifiers () == Qt::AltModifier) modifier = "Alt+";
  if (e->modifiers () == Qt::ControlModifier) modifier = "Ctrl+";
  if (e->modifiers () == Qt::MetaModifier) modifier = "Meta+";


  if (!modifier.isEmpty ()) key = modifier + keyString;


  //  Next, check for special purpose keys not associated with an actual program

  if (key == options.hotkey[DELETE_FILTER_HIGHLIGHTED])
    {
      if (filter_active)
        {
          slotFilterAccept ();
          return;
        }


      //  If we have highlighted points, kill them.

      if (misc.highlight_count)
        {
          for (NV_INT32 i = 0 ; i < misc.highlight_count ; i++)
            {
              NV_INT32 j = misc.highlight[i];

              store_undo (&misc, options.undo_levels, data.val[j], j);
              data.val[j] |= PFM_MANUALLY_INVAL;
            }
          end_undo_block (&misc);

          free (misc.highlight);
          misc.highlight = NULL;
          misc.highlight_count = 0;

          options.function = misc.save_function;

          redrawMap (NVTrue);
        }

      return;
    }


  //  Delete a single point.  Note that we're calling slotPopupMenu with popup[0].  If you change the right click menu DELETE_POINT
  //  option you have to change this as well.

  if ((options.function == DELETE_POINT || options.function == NOOP) && key == options.hotkey[DELETE_SINGLE_POINT])
    {
      slotPopupMenu (popup[0]);
      return;
    }


  //  Toggle freezing of the main marker and the multi...[0] data that is shared with other programs.

  if ((options.function == DELETE_POINT || options.function == MEASURE || options.function == NOOP) && key == options.hotkey[FREEZE_ALL])
    {
      slotPopupMenu (popup[FREEZE_POPUP]);
      return;
    }


  //  Toggle freezing of all of the markers and all of the multi... data that is shared with other programs.

  if ((options.function == DELETE_POINT || options.function == MEASURE || options.function == NOOP) && key == options.hotkey[FREEZE])
    {
      //  Only do the overall freeze if multiMode is not set to single.

      if (!misc.abe_share->mwShare.multiMode)
        {
          if (misc.marker_mode)
            {
              misc.marker_mode = 0;
            }
          else
            {
              misc.marker_mode = 2;
            }


          //  Save the marker position

          misc.frozen_point = misc.nearest_point;

          redrawMap (NVFalse);
        }


      //  If you only have one cursor, this will freeze it.

      else
        {
          slotPopupMenu (popup[FREEZE_POPUP]);
        }

      return;
    }


  //  Finally check against the ancillary programs.

  if (options.function != HOTKEY_POLYGON)
    {
      for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
        {
          //  If option.data_type[i][0] for the requested program is set (no data type) we want to allow key presses.

          if (options.function == DELETE_POINT || options.function == MEASURE || options.data_type[i][0])
            {

              //  Check type of nearest point against acceptable types for each program.  This will
              //  actually let us use a hot key in more than one place (probably not a good idea
              //  anyway).

              if (options.data_type[i][misc.data_type] || options.data_type[i][0])
                {
                  NV_INT32 hot_action_type = 0;
                  if (key == options.hotkey[i]) hot_action_type = 1;


                  //  Check for an occurrence of any of possible multiple action keys

                  if (!options.action[i].isEmpty ())
                    {
                      for (NV_INT32 j = 0 ; j < 10 ; j++)
                        {
                          QString cut = options.action[i].section (',', j, j);
                          if (cut.isEmpty ()) break;
                          if (key == cut) hot_action_type = 2;
                        }
                    }

                  if (hot_action_type == 1)
                    {
                      //  Don't do anything if this is a polygon only program.

                      if (options.hk_poly_only[i]) 
                        {
                          QString message;
                          message = tr ("The key <b><i>") + e->text () + tr ("</i></b> is only defined for hot key polygon mode. "
                                                                             "You may have been trying to run another program. "
                                                                             "The available programs are:<br><br>");

                          QMessageBox::warning (this, tr ("Delete Point Hot Key"), message + hotkey_message);
                          hit = NVTrue;
                          break;
                        }


                      //  Kick off the program.

                      runProg (i);
                      hit = NVTrue;
                      break;
                    }
                  else if (hot_action_type == 2)
                    {
                      //  Have to check for the mosaicView zoom key so that we can tell pfmView to tell any linked apps
                      //  that we're zooming.

                      if (key == options.action[8].section (',', 0, 0)) misc.abe_share->zoom_requested = NVTrue;


                      actionKey (this, map, &data, &options, &misc, key, lock_point);
                      hit = NVTrue;
                      break;
                    }
                }
            }
        }


      if (!hit)
        {
          QString message;
          message = tr ("The key <b><i>") + e->text () + tr ("</i></b> either is not defined for ancillary program operations or the data"
                                                             "type of the nearest point does not match the approved data types for "
                                                             "this key.  The available programs are:<br><br>");

          QMessageBox::warning (this, tr ("Delete Point Hot Key"), message + hotkey_message);
        }
    }
  else
    {
      for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
        {
          if (key == options.hotkey[i] && options.hk_poly_eligible[i])
            {
              NV_BOOL failed = NVFalse;
              hkp = new hotkeyPolygon (this, map, &data, &options, &misc, i, &failed);
              if (failed)
                {
                  redrawMap (NVFalse);
                }
              else
                {
                  connect (hkp, SIGNAL (hotkeyPolygonDone ()), this, SLOT (slotHotkeyPolygonDone ()));
                }

              hit = NVTrue;
              break;
            }
        }


      if (!hit) 
        {
          QString message;
          message = tr ("The key <b><i>") + e->text () + tr ("</i></b> is not defined for hot key polygon operations. "
                                                             "You may have been trying to use another program. "
                                                             "The available programs are:<br><br>");

          QMessageBox::warning (this, tr ("Polygon Hot Key"), message + hotkey_message);
        }
    }

  e->accept ();
}



void 
geoSwath3D::slotRedrawMap ()
{
  redrawMap (NVTrue);
}



void 
geoSwath3D::slotHotkeyPolygonDone ()
{
  redrawMap (NVTrue);
}



void 
geoSwath3D::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



//  A bunch of slots.

void 
geoSwath3D::slotQuit ()
{
  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  //  Clear up any undo memory we've used.

  for (NV_INT32 i = 0 ; i < misc.undo_count ; i++)
    {
      if (misc.undo[i].count)
        {
          free (misc.undo[i].val);
          free (misc.undo[i].num);
          misc.undo[i].val = NULL;
          misc.undo[i].num = NULL;
        }
    }
  misc.undo_count = 0;


  //  Clear up any highlight memory we had.

  if (misc.highlight_count)
    {
      free (misc.highlight);
      misc.highlight = NULL;
      misc.highlight_count = 0;
    }


  //  Flush all modified points in the last buffer.

  misc.statusProg->setRange (0, data.count);
  misc.statusProgLabel->setText (geoSwath3D::tr ("Writing..."));
  misc.statusProgLabel->setVisible (TRUE);
  misc.statusProg->setRange (0, data.count);
  misc.statusProg->setTextVisible (TRUE);
  qApp->processEvents();

  for (NV_INT32 i = 0 ; i < data.count ; i++)
    {
      if (data.oval[i] != data.val[i])
        {
          misc.statusProg->setValue (i);

          io_data_write (data.val[i], data.rec[i], data.sub[i], data.time[i]);
	}
    }

  misc.statusProg->reset ();
  misc.statusProg->setRange (0, 100);
  misc.statusProg->setValue (0);
  misc.statusProgLabel->setVisible (FALSE);
  misc.statusProg->setTextVisible (FALSE);
  qApp->processEvents ();



  //  Make sure the default function is one of the delete functions

  options.function = misc.save_mode;


  //  Get the last rotation values

  options.zx_rotation = map->getZXRotation ();
  options.y_rotation = map->getYRotation ();


  //  Save the user's defaults

  envout (&options, &misc, this);


  io_data_close ();


  //  Kill any running ancillary programs since they may be using process ID keyed shared memory.

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      if (ancillaryProgram[i] && ancillaryProgram[i]->state () == QProcess::Running)
        {
          ancillaryProgram[i]->kill ();
          delete (ancillaryProgram[i]);
        }
    }


  //  Get rid of the shared memory.

  misc.abeShare->detach ();

  if (misc.linked) unregisterABE (misc.abeRegister, misc.abe_register, abe_register_group, active_window_id);


  exit (0);
}



void 
geoSwath3D::slotPrefs ()
{
  if (prefs_dialog) prefs_dialog->close ();

  prefs_dialog = new Prefs (this, &options, &misc, &data);


  //  Put the dialog in the middle of the screen.

  prefs_dialog->move (x () + width () / 2 - prefs_dialog->width () / 2, y () + height () / 2 - prefs_dialog->height () / 2);

  connect (prefs_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotPrefDataChanged ()));
  connect (prefs_dialog, SIGNAL (hotKeyChangedSignal (NV_INT32)), this, SLOT (slotPrefHotKeyChanged (NV_INT32)));
}



//  Changed some of the preferences

void 
geoSwath3D::slotPrefDataChanged ()
{
  if (options.function == DELETE_POINT) bDeletePoint->setChecked (TRUE);
  if (options.function == DELETE_RECTANGLE) bDeleteRect->setChecked (TRUE);
  if (options.function == DELETE_POLYGON) bDeletePoly->setChecked (TRUE);
  if (options.function == KEEP_POLYGON) bKeepPoly->setChecked (TRUE);


  //  Set the semi-transparent colors for slicing.

  NV_FLOAT32 hue_inc = 315.0 / (NV_FLOAT32) (NUMSHADES + 1);

  for (NV_INT32 j = 0 ; j < NUMSHADES ; j++) 
    {
      QColor color;
      color.setHsv ((NV_INT32) (j * hue_inc), 255, 255, options.slice_alpha);

      misc.color_array[1][j][0] = color.red ();
      misc.color_array[1][j][0] = color.green ();
      misc.color_array[1][j][0] = color.blue ();
      misc.color_array[1][j][0] = options.slice_alpha;
    }


  map->setBackgroundColor (options.background_color);
  map->setScaleColor (options.scale_color);
  map->enableScale (options.draw_scale);
  map->setZoomPercent (options.zoom_percent);

  slicePalette.setColor (QPalette::Normal, QPalette::Base, options.background_color);
  slicePalette.setColor (QPalette::Normal, QPalette::Window, options.background_color);
  slicePalette.setColor (QPalette::Inactive, QPalette::Base, options.background_color);
  slicePalette.setColor (QPalette::Inactive, QPalette::Window, options.background_color);
  sliceBar->setPalette (slicePalette);


  //  Check for changes to ancillary programs that have associated buttons.

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      if (options.progButton[i])
        {
          QString tip =  options.description[i] + "  [" + options.hotkey[i] + "]";
          options.progButton[i]->setToolTip (tip);
          options.progButton[i]->setShortcut (QKeySequence (options.hotkey[i]));
        }
    }


  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++) slotPrefHotKeyChanged (i);


  //  Save the user's defaults

  envout (&options, &misc, this);


  //  Redraw the data (which is needed because we may have changed the exaggeration or the point_limit).

  if (!misc.drawing) redrawMap (NVFalse);
}



void 
geoSwath3D::slotPrefHotKeyChanged (NV_INT32 i)
{
  QString tip = misc.buttonText[i] + "  [" + options.buttonAccel[i] + "]";
  misc.button[i]->setToolTip (tip);
  misc.button[i]->setShortcut (QKeySequence (options.buttonAccel[i]));
}



void
geoSwath3D::slotDisplayInvalid ()
{
  static NV_BOOL noop_set;
  static NV_INT32 noop_function = 0;


  if (bDisplayInvalid->isChecked ())
    {
      noop_set = NVFalse;

      options.display_invalid = NVTrue;


      //  Don't allow editing when displaying invalid data.

      if (options.function == DELETE_POINT || options.function == DELETE_RECTANGLE || options.function == DELETE_POLYGON ||
          options.function == KEEP_POLYGON || options.function == HOTKEY_POLYGON)
        {
          noop_function = options.function;
          options.function = NOOP;
          noop_set = NVTrue;
        }
    }
  else
    {
      options.display_invalid = NVFalse;


      //  Restore the function if we were in an edit mode when we turned display_invalid on.

      if (noop_set) options.function = noop_function;
    }

  redrawMap (NVTrue);
}



void 
geoSwath3D::slotClearHighlight ()
{
  if (misc.highlight_count)
    {
      free (misc.highlight);
      misc.highlight = NULL;
      misc.highlight_count = 0;
    }

  bClearHighlight->setEnabled (FALSE);
  redrawMap (NVFalse);
}



void 
geoSwath3D::slotMode (int id)
{
  misc.save_function = options.function;
  options.function = id;
  setFunctionCursor (options.function);


  switch (options.function)
    {
      //  Kill any flags that may be on so we won't sit there wondering why we're not highlighting stuff.  DOH!!!

    case HIGHLIGHT_POLYGON:
      slotFlagMenu (flag[0]);
      break;


      //  Save the last edit mode in case we exit

    case DELETE_RECTANGLE:
    case DELETE_POLYGON:
    case DELETE_POINT:
    case KEEP_POLYGON:
      misc.save_mode = options.function;
      break;
    }
}



void
geoSwath3D::slotStop ()
{
  misc.drawing_canceled = NVTrue;
}



void
geoSwath3D::slotFlagMenu (QAction *action)
{
  for (NV_INT32 i = 0 ; i < 5 ; i++)
    {
      if (action == flag[i])
        {
          options.flag_index = i;
          break;
        }
    }


  bFlag->setIcon (flagIcon[options.flag_index]);

  redrawMap (NVFalse);
}



void
geoSwath3D::slotProg (int id)
{
  if (options.progButton[id]->isChecked ())
    {
      runProg (id);


      //  Switch to DELETE_POINT mode.

      bDeletePoint->click ();
    }
  else
    {
      killProg (id);
    }
}



void
geoSwath3D::slotHotkeyHelp ()
{
  hotkeyHelp *hk = new hotkeyHelp (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  hk->move (x () + width () / 2 - hk->width () / 2, y () + height () / 2 - hk->height () / 2);

  hk->show ();
}


void
geoSwath3D::slotToolbarHelp ()
{
  QWhatsThis::showText (QCursor::pos ( ), toolbarText, map);
}


void 
geoSwath3D::slotFilter ()
{
  NV_BOOL filterPolyArea (OPTIONS *options, MISC *misc, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count, POINT_DATA *data);

  notebook->setTabEnabled (1, TRUE);
  notebook->setCurrentIndex (1);

  filter_active = NVTrue;

  if (misc.filter_kill_list != NULL)
    {
      free (misc.filter_kill_list);
      misc.filter_kill_list = NULL;
      misc.filter_kill_count = 0;
    }


  NV_FLOAT64 mx[4], my[4];

  mx[0] = data.bounds.min_x;
  my[0] = data.bounds.min_y;
  mx[1] = data.bounds.min_x;
  my[1] = data.bounds.max_y;
  mx[2] = data.bounds.max_x;
  my[2] = data.bounds.max_y;
  mx[3] = data.bounds.max_x;
  my[3] = data.bounds.min_y;


  setMainButtons (NVFalse);

  filterPolyArea (&options, &misc, mx, my, 4, &data);


  if (misc.filter_kill_count == 1)
    {
      filterBox->setTitle (tr ("1 point selected"));
    }
  else
    {
      filterBox->setTitle (tr ("%1 points selected").arg (misc.filter_kill_count));
    }


  QString std;
  std.sprintf ("%0.1f", options.filterSTD);
  stdValue->setText (std);


  redrawMap (NVTrue);
}


void 
geoSwath3D::slotClearFilterMasks ()
{
  //  Clear any filter masked areas.
  
  for (NV_INT32 i = 0 ; i < data.count ; i++) data.fmask[i] = NVFalse;

  misc.filter_mask = NVFalse;


  //  If the filter message box is up then we are clearing the filter masks after the filter so we want to discard the current filter points
  //  then filter again.

  if (filter_active) slotFilter ();


  redrawMap (NVFalse);
}



void
geoSwath3D::slotFilterStdChanged (int value)
{
  options.filterSTD = (NV_FLOAT32) value / 10.0;
  QString std;
  std.sprintf ("%0.1f", options.filterSTD);
  stdValue->setText (std);
}



void
geoSwath3D::slotFilterAccept ()
{
  filter_active = NVFalse;


  //  Use PFM_MANUALLY_INVAL flag since the user has to confirm.

  for (NV_INT32 i = 0 ; i < misc.filter_kill_count ; i++)
    {
      store_undo (&misc, options.undo_levels, data.val[misc.filter_kill_list[i]], misc.filter_kill_list[i]);
      data.val[misc.filter_kill_list[i]] |= PFM_MANUALLY_INVAL;
    }
  end_undo_block (&misc);
  misc.filtered = NVTrue;


  free (misc.filter_kill_list);
  misc.filter_kill_list = NULL;
  misc.filter_kill_count = 0;

  notebook->setTabEnabled (1, FALSE);
  notebook->setCurrentIndex (0);

  redrawMap (NVTrue);
}



void
geoSwath3D::slotFilterReject ()
{
  filter_active = NVFalse;

  free (misc.filter_kill_list);
  misc.filter_kill_list = NULL;
  misc.filter_kill_count = 0;

  notebook->setTabEnabled (1, FALSE);
  notebook->setCurrentIndex (0);

  redrawMap (NVTrue);
}



//  Undo the last edit operation.

void 
geoSwath3D::slotUndo ()
{
  undo (&misc, &data);

  redrawMap (NVTrue);
}



//  Kill ancillary programs.

NV_BOOL 
geoSwath3D::killProg (int id)
{
  if (ancillaryProgram[id] && ancillaryProgram[id]->state () == QProcess::Running)
    {
      if (options.state[id])
        {
          misc.abeShare->lock ();
          misc.abe_share->key = options.kill_switch[id];
          misc.abeShare->unlock ();


          //  Give the ancillary process time enough to see the kill flag.

#ifdef NVWIN3X
          Sleep (1000);
#else
          sleep (1);
#endif


          misc.abe_share->key = 0;
          options.state[id] = 1;
        }


      //  Try to kill it even if we kill switched it.

      ancillaryProgram[id]->kill ();

      delete (ancillaryProgram[id]);
      ancillaryProgram[id] = NULL;

      return (NVTrue);
    }


  //  Check for the state set to 2.  This would mean that we started the editor and the program was already up and running.

  if (options.state[id] == 2)
    {
      misc.abeShare->lock ();
      misc.abe_share->key = options.kill_switch[id];
      misc.abeShare->unlock ();


      //  Give the ancillary process time enough to see the kill flag.

#ifdef NVWIN3X
      Sleep (1000);
#else
      sleep (1);
#endif


      misc.abe_share->key = 0;
      options.state[id] = 1;
    }

  return (NVFalse);
}



//  Run ancillary programs (not in hotkey polygon mode).

void 
geoSwath3D::runProg (int id)
{
  //  We want to toggle external GUI programs that are capable of kill and respawn.

  NV_BOOL killed = killProg (id);


  //  If the program is not a "kill/respawn" GUI or it is and it wasn't killed immediately
  //  prior to this, we want to start the program.

  if (!options.state[id] || (options.state[id] == 1 && !killed))
    {
      ancillaryProgram[id] = new ancillaryProg (this, map, &data, &options, &misc, id, lock_point);
      connect (ancillaryProgram[id], SIGNAL (redrawSignal ()), this, SLOT (slotRedrawMap ()));
      options.state[id] = 1;
    }
}



//  2D track map slots

//  Mouse press signal from the map class.

void 
geoSwath3D::slotTrackMousePress (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton);
  if (e->button () == Qt::MidButton);
  if (e->button () == Qt::RightButton);
}



//  Mouse release signal from the map class.  Right now we don't really need this.

void 
geoSwath3D::slotTrackMouseRelease (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton);
  if (e->button () == Qt::MidButton);
  if (e->button () == Qt::RightButton);
}



//  Mouse move signal from the map class.

void
geoSwath3D::slotTrackMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  NV_CHAR                   ltstring[25], lnstring[25], hem;
  NV_FLOAT64                deg, min, sec;
  QString                   y_string, x_string;
  NV_FLOAT64                dist;
  static NV_INT32           prev_nearest_point = -1;


  NV_BOOL compare_to_stack (NV_INT32 current_point, NV_FLOAT64 dist, MISC *misc);


  //  Track the cursor position for other ABE programs

  misc.abe_share->cursor_position.y = lat;
  misc.abe_share->cursor_position.x = lon;


  //  Clear the nearest point stack.

  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      misc.nearest_stack.point[i] = -1;
      misc.nearest_stack.dist[i] = 9999999999.0;
    }


  misc.nearest_point = -1;


  for (NV_INT32 i = 0 ; i < data.count ; i++)
    {
      //  Do not use null points.  Do not use invalid points unless the display_man_invalid, display_flt_invalid, or
      //  display_null flag is set.  Do not use masked points. Do not check points that are not on the display.

      NV_INT32 trans = 0;
      if ((trans = check_bounds (&options, &misc, &data, i, NVTrue, misc.slice)) < 2)
        {
          dist = sqrt ((NV_FLOAT64) ((lat - data.y[i]) * (lat - data.y[i])) + (NV_FLOAT64) ((lon - data.x[i]) * (lon - data.x[i])));


          //  Check the points against the points in the nearest points stack.

          if (compare_to_stack (i, dist, &misc))
            {
              //  If the return was true then this is the minimum distance so far.

              misc.nearest_point = misc.nearest_stack.point[0];
            }
        }
    }


  QString rec_string = "", sub_string = "", distance_string = "";


  //  Update the status bars

  // 
  //                      if our point has moved or we have just selected a new overplot we will go
  //                      into this code block.
  //

  if (misc.nearest_point != -1 && misc.nearest_point != prev_nearest_point)
    {
      //  Locking shared memory.

      misc.abeShare->lock ();


      //  Only change the nearest filename if we aren't locked.  External programs monitor
      //  abe_share->nearest_point to trigger updates of their windows.  They use
      //  abe_share->mwShare.multiRecord[0] for the current record to display.  In this way
      //  we can lock the main screen by not updating abe_share->mwShare.multiRecord[0] and still
      //  trigger the external programs to update (such as for multiple waveforms in waveMonitor or
      //  waveWaterfall).  When we do this we don't want nearest_filename or nearest_line to change
      //  either since that is where the external programs get the filename to use with 
      //  abe_share->mwShare.multiRecord[0].


      //  Push the nearest stack points into the ABE shared memory for use by waveMonitor, waveWaterfall,
      //  and lidarMonitor.

      for (NV_INT32 j = 0 ; j < MAX_STACK_POINTS ; j++)
        {
          //  Normal, unfrozen mode.

          if (!misc.marker_mode)
            {
              misc.abe_share->mwShare.multiSubrecord[j] = data.sub[misc.nearest_stack.point[j]];
              misc.abe_share->mwShare.multiPoint[j].x = data.x[misc.nearest_stack.point[j]];
              misc.abe_share->mwShare.multiPoint[j].y = data.y[misc.nearest_stack.point[j]];
              misc.abe_share->mwShare.multiPoint[j].z = data.z[misc.nearest_stack.point[j]];
              misc.abe_share->mwShare.multiLocation[j] = misc.nearest_stack.point[j];
              misc.abe_share->mwShare.multiType[j] = misc.data_type;
              misc.abe_share->mwShare.multiPresent[j] = data.rec[misc.nearest_stack.point[j]];
              misc.abe_share->mwShare.multiRecord[j] = data.rec[misc.nearest_stack.point[j]];
            }


          //  If we're frozen, we don't want to replace the [0] record and we want to push the nearest
          //  MAX_STACK_POINTS - 1 records up one level.

          else
            {
              if (j)
                {
                  misc.abe_share->mwShare.multiSubrecord[j] = data.sub[misc.nearest_stack.point[j - 1]];
                  misc.abe_share->mwShare.multiPoint[j].x = data.x[misc.nearest_stack.point[j - 1]];
                  misc.abe_share->mwShare.multiPoint[j].y = data.y[misc.nearest_stack.point[j - 1]];
                  misc.abe_share->mwShare.multiPoint[j].z = data.z[misc.nearest_stack.point[j - 1]];
                  misc.abe_share->mwShare.multiLocation[j] = misc.nearest_stack.point[j - 1];
                  misc.abe_share->mwShare.multiType[j] = misc.data_type;
                  misc.abe_share->mwShare.multiPresent[j] = data.rec[misc.nearest_stack.point[j - 1]];
                  misc.abe_share->mwShare.multiRecord[j] = data.rec[misc.nearest_stack.point[j - 1]];
                }
            }
        }
      misc.abe_share->mwShare.multiNum = MAX_STACK_POINTS;


      //  Trigger some of the external applications to update.

      misc.abe_share->nearest_point = misc.nearest_point;


      //  Unlock shared memory.

      misc.abeShare->unlock ();


      rec_string.sprintf ("%d", misc.abe_share->mwShare.multiRecord[0]);
      sub_string.sprintf ("%d", misc.abe_share->mwShare.multiSubrecord[0]);


      recLabel->setText (rec_string);
      subLabel->setText (sub_string);
    }


  prev_nearest_point = misc.nearest_point;


  //  First set the 3D tracker.

  NV_INT32 pix_x, pix_y;
  map->get2DCoords (data.x[misc.nearest_point], data.y[misc.nearest_point], -data.z[misc.nearest_point], &pix_x, &pix_y);


  //  This particular situation caused a lingering of shot markers so a clearing of markers is called here.  This is a weird situation.

  discardMovableObjects ();


  map->setMovingList (&mv_tracker, marker, 16, pix_x, pix_y, 0.0, 2, options.tracker_color);


  //  Then set the 2D tracker.

  QBrush b1;
  trackMap->setMovingPath (&mv_2D_tracker, tracker_2D, data.x[misc.nearest_point], data.y[misc.nearest_point], 2, options.tracker_color, b1,
                           NVFalse, Qt::SolidLine);


  strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
  strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));

  latLabel->setText (ltstring);
  lonLabel->setText (lnstring);
}



//  Resize signal from the map class.

void
geoSwath3D::slotTrackResize (QResizeEvent *e __attribute__ ((unused)))
{
}



//  Prior to drawing coastline signal from the map class.  This is where we do our heavy lifting.

void 
geoSwath3D::slotTrackPreRedraw (NVMAP_DEF l_mapdef __attribute__ ((unused)))
{
  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();

  for (NV_INT32 i = 0 ; i < data.count ;  i++)
    {
      NV_INT32 xyz_x, xyz_y, xyz_z;
      NV_FLOAT64 dummy;
      if (!(data.val[i] & (PFM_DELETED | PFM_INVAL)))
        {
          //  Check against the viewing area.

          NV_INT32 trans = 0;
          if ((trans = check_bounds (&options, &misc, &data, i, NVFalse, NVFalse)) < 2)
            {
              NV_INT32 c_index = NINT ((NUMSHADES - 1) - ((-data.z[i]) - bounds.min_z) / range_z * (NV_FLOAT32) (NUMSHADES - 1));


              //  Check for out of range data.

              if (c_index < 0) c_index = 0;
              if (c_index > NUMSHADES - 1) c_index = NUMSHADES - 1;


              QColor tmp = misc.track_color_array[c_index];

              trackMap->map_to_screen (1, &data.x[i], &data.y[i], &dummy, &xyz_x, &xyz_y, &xyz_z);
              trackMap->fillRectangle (xyz_x, xyz_y, 2, 2, tmp, NVFalse);
            }
        }
    }

  trackMap->flush ();
}



//  Post grid drawing signal from map class.  Mostly just cleanup work.

void 
geoSwath3D::slotTrackPostRedraw (NVMAP_DEF mapdef __attribute__ ((unused)))
{
  qApp->restoreOverrideCursor ();
}



//  Clean exit routine

void 
geoSwath3D::clean_exit (NV_INT32 ret)
{
  //  Have to close the GL widget or it stays on screen in VirtualBox

  map->close ();

  exit (ret);
}



void
geoSwath3D::about ()
{
  QMessageBox::about (this, VERSION, geoSwath3DAboutText);
}



void
geoSwath3D::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}



void
geoSwath3D::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}
