
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



//  pfmEdit3D class.

#include "pfmEdit3D.hpp"
#include "pfmEdit3DHelp.hpp"
#include "verticalDatums.hpp"



void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);
NV_INT32 ComputeLineDistanceFilter (NV_F64_COORD2 lineStart, NV_F64_COORD2 lineEnd, MISC *misc, OPTIONS *options);
NV_INT32 GetPixelOffsetByMeter (MISC *misc, nvMap *map);
void LoadShotsWithinAVBox (MISC *misc, OPTIONS *options, nvMap *map, NV_I32_COORD2 targetPt);
NV_INT32 GetFlightlineColorIndex (NV_INT32 flightIndex, NV_INT32 *flightLnArray, NV_INT32 noFlights, NV_INT32 interval);
void DrawAVInterfaceBox (nvMap *map, MISC *misc, NV_F64_COORD3 latlon);
void setFlags (MISC *misc, OPTIONS *options);


/***************************************************************************/
/*!

   - Module :        pfmEdit3D

   - Programmer :    Jan C. Depner

   - Date :          11/18/08

   - Purpose :       3D version of Area-Based Editor point cloud editor.

****************************************************************************/

pfmEdit3D::pfmEdit3D (int *argc, char **argv, QWidget *parent):
  QMainWindow (parent, 0)
{
  NV_BOOL envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);
  void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore);


  extern char *optarg;
  NV_INT32 option_index = 0;
  NV_INT32 shmid = 0;

  options.z_factor = 1.0;
  options.z_offset = 0.0;
  force_auto_unload = NVFalse;

  while (NVTrue) 
    {
      static struct option long_options[] = {{"shared_memory_key", required_argument, 0, 0},
                                             {"force_auto_unload", no_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "n", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              sscanf (optarg, "%d", &shmid);
              break;

            case 1:
              force_auto_unload = NVTrue;
              break;
            }
          break;


          //  This is a place saver.  Not used.

        case 'n':
          break;
        }
    }


  //  We must have the shared memory ID.

  if (!shmid)
    {
      QMessageBox::critical (this, tr ("pfmEdit3D"), tr ("pfmEdit3D can only be run from pfmView using shared memory."), QMessageBox::Close);
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
  setFocus (Qt::ActiveWindowFocusReason);
  

  //  Set a few defaults for startup

  polygon_flip = NVFalse;
  prev_poly_x = -1;
  prev_poly_y = -1;
  popup_active = NVFalse;
  double_click = NVFalse;
  move_feature = 0;
  browserProc = NULL;
  prefs_dialog = NULL;
  mv_tracker = -1;
  mv_2D_tracker = -1;
  mv_measure_anchor = -1;
  mv_measure = -1;
  rb_polygon = -1;
  rb_measure = -1;
  rb_dist = -1;
  rb_rectangle = -1;
  misc.avb.av_rectangle = -1;
  moveWindow = 0;
  start_ctrl_x = -1;
  start_ctrl_y = -1;
  rotate_index = 0;
  lock_point = 0;
  slicing = NVFalse;
  PFMWDB_attr_count = 0;
  filterMessage_slider_count = -1;
  filter_active = NVFalse;
  attr_filter_active = NVFalse;
  hof_filter_active = NVFalse;
  misc.nearest_point = 0;
  misc.performingAction = AVA_NONE;
  misc.avInput = AV_AOI;
  local_num_lines = 0;


  //  Need to set the distLineStart and distLineEnd variables to indicate no prior selection

  distLineStart.x = distLineStart.y = -1.0f;
  distLineEnd.x = distLineEnd.y = -1.0f;


  // 
  //                      Key flags are intialized here.
  //                      multiMode represents a waveform display mode in the CZMILwaveMonitor program 
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

  setWindowIcon (QIcon (":/icons/pfm_abe.xpm"));


  //  Define all of the cursors

  editFeatureCursor = QCursor (QPixmap (":/icons/edit_feature_cursor.xpm"), 11, 26);
  deleteFeatureCursor = QCursor (QPixmap (":/icons/delete_feature_cursor.xpm"), 15, 15);
  addFeatureCursor = QCursor (QPixmap (":/icons/add_feature_cursor.xpm"), 15, 15);
  moveFeatureCursor = QCursor (QPixmap (":/icons/move_feature_cursor.xpm"), 15, 15);
  deleteRectCursor = QCursor (QPixmap (":/icons/delete_rect_cursor.xpm"), 3, 0);
  deletePolyCursor = QCursor (QPixmap (":/icons/delete_poly_cursor.xpm"), 3, 0);
  restoreRectCursor = QCursor (QPixmap (":/icons/restore_rect_cursor.xpm"), 11, 0);
  restorePolyCursor = QCursor (QPixmap (":/icons/restore_poly_cursor.xpm"), 11, 0);
  hotkeyPolyCursor = QCursor (QPixmap (":/icons/hotkey_poly_cursor.xpm"), 0, 19);
  keepPolyCursor = QCursor (QPixmap (":/icons/keep_poly_cursor.xpm"), 5, 0);
  unreferencePolyCursor = QCursor (QPixmap (":/icons/unreference_poly_cursor.xpm"), 1, 1);
  referencePolyCursor = QCursor (QPixmap (":/icons/reference_poly_cursor.xpm"), 1, 1);
  rotateCursor = QCursor (QPixmap (":/icons/rotate_cursor.xpm"), 17, 17);
  zoomCursor = QCursor (QPixmap (":/icons/zoom_cursor.xpm"), 11, 11);
  maskCursor = QCursor (QPixmap (":/icons/mask_cursor.xpm"), 1, 1);
  measureCursor = QCursor (QPixmap (":/icons/measure_cursor.xpm"), 1, 1);
  filterMaskRectCursor = QCursor (QPixmap (":/icons/filter_mask_rect_cursor.xpm"), 1, 1);
  filterMaskPolyCursor = QCursor (QPixmap (":/icons/filter_mask_poly_cursor.xpm"), 1, 1);
  highlightPolyCursor = QCursor (QPixmap (":/icons/highlight_polygon_cursor.xpm"), 1, 1);
  clearPolyCursor = QCursor (QPixmap (":/icons/clear_polygon_cursor.xpm"), 1, 1);


  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++) ancillaryProgram[i] = NULL;


  if (misc.abe_share->polygon_count)
    {
      misc.abe_share->edit_area.min_y = 999999999.0;
      misc.abe_share->edit_area.max_y = -999999999.0;
      misc.abe_share->edit_area.min_x = 999999999.0;
      misc.abe_share->edit_area.max_x = -999999999.0;

      for (NV_INT32 i = 0 ; i < misc.abe_share->polygon_count ; i++)
        {
          misc.abe_share->edit_area.min_y = qMin (misc.abe_share->edit_area.min_y, misc.abe_share->polygon_y[i]);
          misc.abe_share->edit_area.min_x = qMin (misc.abe_share->edit_area.min_x, misc.abe_share->polygon_x[i]);
          misc.abe_share->edit_area.max_y = qMax (misc.abe_share->edit_area.max_y, misc.abe_share->polygon_y[i]);
          misc.abe_share->edit_area.max_x = qMax (misc.abe_share->edit_area.max_x, misc.abe_share->polygon_x[i]);
        }
    }


  //  Open the PFM files and compute the MBR of all of the PFM areas.

  misc.total_mbr.min_y = 999.0;
  misc.total_mbr.min_x = 999.0;
  misc.total_mbr.max_y = -999.0;
  misc.total_mbr.max_x = -999.0;

  for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
    {
      if ((misc.pfm_handle[pfm] = open_existing_pfm_file (&misc.abe_share->open_args[pfm])) < 0)
        {
          QMessageBox::warning (this, tr ("Open PFM Structure"),
                                tr ("The file ") + QDir::toNativeSeparators (QString (misc.abe_share->open_args[pfm].list_path)) + 
                                tr (" is not a PFM handle or list file or there was an error reading the file.") +
                                tr ("  The error message returned was:\n\n") +
                                QString (pfm_error_str (pfm_error)));
        }

      if (misc.abe_share->open_args[pfm].head.proj_data.projection)
        {
          QMessageBox::warning (this, tr ("Open PFM Structure"), tr ("Sorry, pfmEdit3D does not handle projected data."));
          close_pfm_file (misc.pfm_handle[pfm]);
          printf ("Edit return status:0,0\n");
          clean_exit (-1);
        }


      misc.total_mbr.min_y = qMin (misc.total_mbr.min_y, misc.abe_share->open_args[pfm].head.mbr.min_y);
      misc.total_mbr.min_x = qMin (misc.total_mbr.min_x, misc.abe_share->open_args[pfm].head.mbr.min_x);
      misc.total_mbr.max_y = qMax (misc.total_mbr.max_y, misc.abe_share->open_args[pfm].head.mbr.max_y);
      misc.total_mbr.max_x = qMax (misc.total_mbr.max_x, misc.abe_share->open_args[pfm].head.mbr.max_x);


      //  Compute the average bin sizes.

      misc.avg_x_bin_size_degrees += misc.abe_share->open_args[pfm].head.x_bin_size_degrees;
      misc.avg_y_bin_size_degrees += misc.abe_share->open_args[pfm].head.y_bin_size_degrees;
    }

  misc.avg_x_bin_size_degrees /= (NV_FLOAT64) misc.abe_share->pfm_count;
  misc.avg_y_bin_size_degrees /= (NV_FLOAT64) misc.abe_share->pfm_count;


  //  For the average bin size in meters we're really only interested in the Y bin size because we're going to use it for geo_distance.

  if (misc.abe_share->otf_width)
    {
      NV_FLOAT64 az;
      invgp (NV_A0, NV_B0, misc.total_mbr.min_y, misc.total_mbr.min_x, misc.total_mbr.min_y + misc.abe_share->otf_y_bin_size, misc.total_mbr.min_x,
             &misc.avg_bin_size_meters, &az);
    }
  else
    {
      NV_FLOAT64 az;
      invgp (NV_A0, NV_B0, misc.total_mbr.min_y, misc.total_mbr.min_x, misc.total_mbr.min_y + misc.avg_y_bin_size_degrees, misc.total_mbr.min_x,
             &misc.avg_bin_size_meters, &az);
    }

  init_geo_distance (misc.avg_bin_size_meters, misc.total_mbr.min_x, misc.total_mbr.min_y, misc.total_mbr.max_x, misc.total_mbr.max_y);


  //  We may be running the attributeViewer program.  Because of that we'll need to initialize the UTM/Geodetic conversion
  //  software (from PROJ.4).  Get the zone from the center of the total MBR of all of the PFM files.

  NV_FLOAT64 cen_x = misc.total_mbr.min_x + (misc.total_mbr.max_x - misc.total_mbr.min_x) / 2.0;

  NV_CHAR str[60];
  if (misc.total_mbr.max_y < 0.0)
    {
      sprintf (str, "+proj=utm -ellps=WGS84 +datum=WGS84 +south +lon_0=%.9f", cen_x);
    }
  else
    {
      sprintf (str, "+proj=utm -ellps=WGS84 +datum=WGS84 +lon_0=%.9f", cen_x);
    }

  if (!(misc.pj_utm = pj_init_plus (str)))
    {
      QMessageBox::critical (this, tr ("pfmEdit3D"), tr ("Error initializing UTM projection\n"));
    }

  if (!(misc.pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
    {
      QMessageBox::critical (this, tr ("pfmEdit3D"), tr ("Error initializing latlon projection\n"));
    }


  //  Set all of the defaults

  set_defaults (&misc, &options, NVFalse);


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
  toolBar[0]->setObjectName (tr ("pfmEdit3D View tool bar"));

  QButtonGroup *exitGrp = new QButtonGroup (this);
  connect (exitGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotExit (int)));

  bExitSave = new QToolButton (this);
  misc.button[SAVE_EXIT_KEY] = bExitSave;
  bExitSave->setIcon (misc.buttonIcon[SAVE_EXIT_KEY]);
  exitGrp->addButton (bExitSave, 0);
  bExitSave->setWhatsThis (exitSaveText);
  toolBar[0]->addWidget (bExitSave);

  bExitMask = new QToolButton (this);
  misc.button[SAVE_EXIT_MASK_KEY] = bExitMask;
  bExitMask->setIcon (misc.buttonIcon[SAVE_EXIT_MASK_KEY]);
  exitGrp->addButton (bExitMask, 1);
  bExitMask->setWhatsThis (exitMaskText);
  toolBar[0]->addWidget (bExitMask);

  bExitNoSave = new QToolButton (this);
  misc.button[NO_SAVE_EXIT_KEY] = bExitNoSave;
  bExitNoSave->setIcon (misc.buttonIcon[NO_SAVE_EXIT_KEY]);
  exitGrp->addButton (bExitNoSave, 2);
  bExitNoSave->setWhatsThis (exitNoSaveText);
  toolBar[0]->addWidget (bExitNoSave);

  bReset = new QToolButton (this);
  misc.button[RESET_KEY] = bReset;
  bReset->setIcon (misc.buttonIcon[RESET_KEY]);
  bReset->setWhatsThis (resetText);
  connect (bReset, SIGNAL (clicked ()), this, SLOT (slotReset ()));
  toolBar[0]->addWidget (bReset);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  bDisplayManInvalid = new QToolButton (this);
  bDisplayManInvalid->setIcon (QIcon (":/icons/display_manually_invalid.png"));
  bDisplayManInvalid->setToolTip (tr ("Display manually invalidated points in addition to valid points"));
  bDisplayManInvalid->setWhatsThis (displayManInvalidText);
  bDisplayManInvalid->setCheckable (TRUE);
  toolBar[0]->addWidget (bDisplayManInvalid);

  bDisplayFltInvalid = new QToolButton (this);
  bDisplayFltInvalid->setIcon (QIcon (":/icons/display_filter_invalid.png"));
  bDisplayFltInvalid->setToolTip (tr ("Display filter invalidated points in addition to valid points"));
  bDisplayFltInvalid->setWhatsThis (displayFltInvalidText);
  bDisplayFltInvalid->setCheckable (TRUE);
  toolBar[0]->addWidget (bDisplayFltInvalid);

  bDisplayNull = new QToolButton (this);
  bDisplayNull->setIcon (QIcon (":/icons/display_null.png"));
  bDisplayNull->setToolTip (tr ("Display NULL value points in addition to valid points"));
  bDisplayNull->setWhatsThis (displayNullText);
  bDisplayNull->setCheckable (TRUE);
  toolBar[0]->addWidget (bDisplayNull);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  bDisplayAll = new QToolButton (this);
  misc.button[DISPLAY_ALL_KEY] = bDisplayAll;
  bDisplayAll->setIcon (misc.buttonIcon[DISPLAY_ALL_KEY]);
  bDisplayAll->setWhatsThis (displayAllText);
  bDisplayAll->setEnabled (FALSE);
  connect (bDisplayAll, SIGNAL (clicked ()), this, SLOT (slotDisplayAll ()));
  toolBar[0]->addWidget (bDisplayAll);


  bUndisplaySingle = new QToolButton (this);
  bUndisplaySingle->setIcon (QIcon (":/icons/undisplaysingle.xpm"));
  bUndisplaySingle->setToolTip (tr ("Hide data points from a single line"));
  bUndisplaySingle->setWhatsThis (undisplaySingleText);
  connect (bUndisplaySingle, SIGNAL (clicked ()), this, SLOT (slotUndisplaySingle ()));
  toolBar[0]->addWidget (bUndisplaySingle);


  bDisplayMultiple = new QToolButton (this);
  misc.button[DISPLAY_MULTIPLE_KEY] = bDisplayMultiple;
  bDisplayMultiple->setIcon (misc.buttonIcon[DISPLAY_MULTIPLE_KEY]);
  bDisplayMultiple->setWhatsThis (displayMultipleText);
  connect (bDisplayMultiple, SIGNAL (clicked ()), this, SLOT (slotDisplayMultiple ()));
  toolBar[0]->addWidget (bDisplayMultiple);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  //  If you add to the 4 buttons below you must change the value of PRE_ATTR in pfmEdit3DDef.hpp.

  attrIcon[0] = misc.buttonIcon[COLOR_BY_DEPTH_ACTION_KEY];
  attrIcon[1] = misc.buttonIcon[COLOR_BY_LINE_ACTION_KEY];
  attrIcon[2] = QIcon (":/icons/horiz.xpm");
  attrIcon[3] = QIcon (":/icons/vert.xpm");


  attrIcon[PRE_ATTR + 0] = QIcon (":/icons/attr01.xpm");
  attrIcon[PRE_ATTR + 1] = QIcon (":/icons/attr02.xpm");
  attrIcon[PRE_ATTR + 2] = QIcon (":/icons/attr03.xpm");
  attrIcon[PRE_ATTR + 3] = QIcon (":/icons/attr04.xpm");
  attrIcon[PRE_ATTR + 4] = QIcon (":/icons/attr05.xpm");
  attrIcon[PRE_ATTR + 5] = QIcon (":/icons/attr06.xpm");
  attrIcon[PRE_ATTR + 6] = QIcon (":/icons/attr07.xpm");
  attrIcon[PRE_ATTR + 7] = QIcon (":/icons/attr08.xpm");
  attrIcon[PRE_ATTR + 8] = QIcon (":/icons/attr09.xpm");
  attrIcon[PRE_ATTR + 9] = QIcon (":/icons/attr10.xpm");


  QMenu *attrMenu = new QMenu (this);

  attributeGrp = new QActionGroup (this);
  connect (attributeGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotAttrMenu (QAction *)));


  attr[0] = attrMenu->addAction (misc.buttonText[COLOR_BY_DEPTH_ACTION_KEY]);
  misc.action[COLOR_BY_DEPTH_ACTION_KEY] = attr[0];
  attributeGrp->addAction (attr[0]);
  attr[0]->setIcon (attrIcon[0]);

  attr[1] = attrMenu->addAction (misc.buttonText[COLOR_BY_LINE_ACTION_KEY]);
  misc.action[COLOR_BY_LINE_ACTION_KEY] = attr[1];
  attributeGrp->addAction (attr[1]);
  attr[1]->setIcon (attrIcon[1]);

  attr[2] = attrMenu->addAction (tr ("Color by horizontal uncertainty"));
  attributeGrp->addAction (attr[2]);
  attr[2]->setIcon (attrIcon[2]);

  attr[3] = attrMenu->addAction (tr ("Color by vertical uncertainty"));
  attributeGrp->addAction (attr[3]);
  attr[3]->setIcon (attrIcon[3]);

  bAttr = new QToolButton (this);
  bAttr->setToolTip (tr ("Select point coloring option"));
  bAttr->setWhatsThis (attrText);
  bAttr->setPopupMode (QToolButton::InstantPopup);
  toolBar[0]->addWidget (bAttr);



  //  If you add to the 5 buttons below you must change the value of PRE_USER in pfmEdit3DDef.hpp.  These are the
  //  pre-user flag buttons.  NOTE: flag_invalid should always be the last before the USER flags (i.e. PRE_USER - 1).

  flagIcon[0] = QIcon (":/icons/user_flag.xpm");
  flagIcon[1] = QIcon (":/icons/suspect.xpm");
  flagIcon[2] = QIcon (":/icons/selected.xpm");
  flagIcon[3] = QIcon (":/icons/feature.xpm");
  flagIcon[4] = QIcon (":/icons/designated.xpm");
  flagIcon[5] = QIcon (":/icons/flag_invalid.xpm");


  flagIcon[PRE_USER + 0] = QIcon (":/icons/user_flag01.xpm");
  flagIcon[PRE_USER + 1] = QIcon (":/icons/user_flag02.xpm");
  flagIcon[PRE_USER + 2] = QIcon (":/icons/user_flag03.xpm");
  flagIcon[PRE_USER + 3] = QIcon (":/icons/user_flag04.xpm");
  flagIcon[PRE_USER + 4] = QIcon (":/icons/user_flag05.xpm");


  QMenu *flagMenu = new QMenu (this);

  flagGrp = new QActionGroup (this);
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

  flag[2] = flagMenu->addAction (tr ("Mark selected soundings"));
  flagGrp->addAction (flag[2]);
  flag[2]->setIcon (flagIcon[2]);
  flag[2]->setCheckable (TRUE);

  flag[3] = flagMenu->addAction (tr ("Mark selected features"));
  flagGrp->addAction (flag[3]);
  flag[3]->setIcon (flagIcon[3]);
  flag[3]->setCheckable (TRUE);

  flag[4] = flagMenu->addAction (tr ("Mark designated soundings"));
  flagGrp->addAction (flag[4]);
  flag[4]->setIcon (flagIcon[4]);
  flag[4]->setCheckable (TRUE);

  flag[5] = flagMenu->addAction (tr ("Mark invalid data"));
  flagGrp->addAction (flag[5]);
  flag[5]->setIcon (flagIcon[5]);
  flag[5]->setCheckable (TRUE);

  for (NV_INT32 i = 0 ; i < 5 ; i++)
    {
      QString string = tr ("Mark ") + misc.abe_share->open_args[0].head.user_flag_name[i] + tr (" data");
      flag[i + PRE_USER] = flagMenu->addAction (string);
      flagGrp->addAction (flag[i + PRE_USER]);
      flag[i + PRE_USER]->setIcon (flagIcon[i + PRE_USER]);
      flag[i + PRE_USER]->setCheckable (TRUE);
    }

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

  bInvertHighlight = new QToolButton (this);
  bFlag->setIcon (QIcon (":/icons/user_flag.xpm"));
  bInvertHighlight->setIcon (QIcon (":/icons/invert_highlight.png"));
  bInvertHighlight->setToolTip (tr ("Invert highlighted point selection"));
  bInvertHighlight->setWhatsThis (invertHighlightText);
  connect (bInvertHighlight, SIGNAL (clicked ()), this, SLOT (slotInvertHighlight ()));
  toolBar[0]->addWidget (bInvertHighlight);

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


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  bContour = new QToolButton (this);
  misc.button[TOGGLE_CONTOUR_KEY] = bContour;
  bContour->setIcon (misc.buttonIcon[TOGGLE_CONTOUR_KEY]);
  bContour->setWhatsThis (contourText);
  bContour->setCheckable (TRUE);
  toolBar[0]->addWidget (bContour);


  //  Create Feature toolbar

  toolBar[1] = new QToolBar (tr ("Feature tool bar"));
  toolBar[1]->setToolTip (tr ("Feature tool bar"));
  addToolBar (toolBar[1]);
  toolBar[1]->setObjectName (tr ("pfmEdit3D Feature tool bar"));

  QMenu *featureMenu = new QMenu (this);

  QActionGroup *featureGrp = new QActionGroup (this);
  connect (featureGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFeatureMenu (QAction *)));

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
  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);
  bDisplayFeature->setToolTip (tr ("Select type of feature data to display"));
  bDisplayFeature->setWhatsThis (displayFeatureText);
  bDisplayFeature->setPopupMode (QToolButton::InstantPopup);
  bDisplayFeature->setMenu (featureMenu);
  toolBar[1]->addWidget (bDisplayFeature);

  bDisplayChildren = new QToolButton (this);
  bDisplayChildren->setIcon (QIcon (":/icons/displayfeaturechildren.xpm"));
  bDisplayChildren->setToolTip (tr ("Display feature group members"));
  bDisplayChildren->setWhatsThis (displayChildrenText);
  bDisplayChildren->setCheckable (TRUE);
  toolBar[1]->addWidget (bDisplayChildren);

  bDisplayFeatureInfo = new QToolButton (this);
  bDisplayFeatureInfo->setIcon (QIcon (":/icons/displayfeatureinfo.xpm"));
  bDisplayFeatureInfo->setToolTip (tr ("Display feature information"));
  bDisplayFeatureInfo->setWhatsThis (displayFeatureInfoText);
  bDisplayFeatureInfo->setCheckable (TRUE);
  toolBar[1]->addWidget (bDisplayFeatureInfo);

  bAddFeature = new QToolButton (this);
  bAddFeature->setIcon (QIcon (":/icons/addfeature.xpm"));
  modeGrp->addButton (bAddFeature, ADD_FEATURE);
  bAddFeature->setToolTip (tr ("Select add feature mode"));
  bAddFeature->setWhatsThis (addFeatureText);
  bAddFeature->setCheckable (TRUE);
  toolBar[1]->addWidget (bAddFeature);

  bEditFeature = new QToolButton (this);
  misc.button[EDIT_FEATURE_MODE_KEY] = bEditFeature;
  bEditFeature->setIcon (misc.buttonIcon[EDIT_FEATURE_MODE_KEY]);
  modeGrp->addButton (bEditFeature, EDIT_FEATURE);
  bEditFeature->setWhatsThis (editFeatureText);
  bEditFeature->setCheckable (TRUE);
  toolBar[1]->addWidget (bEditFeature);

  bMoveFeature = new QToolButton (this);
  bMoveFeature->setIcon (QIcon (":/icons/movefeature.xpm"));
  modeGrp->addButton (bMoveFeature, MOVE_FEATURE);
  bMoveFeature->setToolTip (tr ("Select move feature mode"));
  bMoveFeature->setWhatsThis (moveFeatureText);
  bMoveFeature->setCheckable (TRUE);
  toolBar[1]->addWidget (bMoveFeature);


  bDeleteFeature = new QToolButton (this);
  bDeleteFeature->setIcon (QIcon (":/icons/deletefeature.xpm"));
  modeGrp->addButton (bDeleteFeature, DELETE_FEATURE);
  bDeleteFeature->setToolTip (tr ("Select invalidate feature mode"));
  bDeleteFeature->setWhatsThis (deleteFeatureText);
  bDeleteFeature->setCheckable (TRUE);
  toolBar[1]->addWidget (bDeleteFeature);


  bVerifyFeatures = new QToolButton (this);
  bVerifyFeatures->setIcon (QIcon (":/icons/verify_features.png"));
  bVerifyFeatures->setToolTip (tr ("Verify all valid features"));
  bVerifyFeatures->setWhatsThis (verifyFeaturesText);
  connect (bVerifyFeatures, SIGNAL (clicked ()), this, SLOT (slotVerifyFeatures ()));
  toolBar[1]->addWidget (bVerifyFeatures);


  //  Create Reference toolbar

  toolBar[2] = new QToolBar (tr ("Reference tool bar"));
  toolBar[2]->setToolTip (tr ("Reference tool bar"));
  addToolBar (toolBar[2]);
  toolBar[2]->setObjectName (tr ("pfmEdit3D Reference tool bar"));

  bDisplayReference = new QToolButton (this);
  bDisplayReference->setIcon (QIcon (":/icons/displayreference.xpm"));
  bDisplayReference->setToolTip (tr ("Mark reference data"));
  bDisplayReference->setWhatsThis (displayReferenceText);
  bDisplayReference->setCheckable (TRUE);
  toolBar[2]->addWidget (bDisplayReference);


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();


  bReferencePoly = new QToolButton (this);
  bReferencePoly->setIcon (QIcon (":/icons/reference_poly.xpm"));
  modeGrp->addButton (bReferencePoly, SET_REFERENCE);
  bReferencePoly->setToolTip (tr ("Select set polygon to reference mode"));
  bReferencePoly->setWhatsThis (referencePolyText);
  bReferencePoly->setCheckable (TRUE);
  toolBar[2]->addWidget (bReferencePoly);

  bUnreferencePoly = new QToolButton (this);
  bUnreferencePoly->setIcon (QIcon (":/icons/unreference_poly.xpm"));
  modeGrp->addButton (bUnreferencePoly, UNSET_REFERENCE);
  bUnreferencePoly->setToolTip (tr ("Select unset polygon to reference mode"));
  bUnreferencePoly->setWhatsThis (unreferencePolyText);
  bUnreferencePoly->setCheckable (TRUE);
  toolBar[2]->addWidget (bUnreferencePoly);


  //  Create Utilities toolbar

  toolBar[3] = new QToolBar (tr ("Utilities tool bar"));
  toolBar[3]->setToolTip (tr ("Utilities tool bar"));
  addToolBar (toolBar[3]);
  toolBar[3]->setObjectName (tr ("pfmEdit3D Utilities tool bar"));


  bUnload = new QToolButton (this);
  bUnload->setIcon (QIcon (":/icons/unload.xpm"));
  bUnload->setToolTip (tr ("Automatically unload changed data on save/exit"));
  bUnload->setWhatsThis (unloadText);
  bUnload->setCheckable (TRUE);
  toolBar[3]->addWidget (bUnload);


  toolBar[3]->addSeparator ();
  toolBar[3]->addSeparator ();


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.xpm"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  toolBar[3]->addWidget (bPrefs);


  toolBar[3]->addSeparator ();
  toolBar[3]->addSeparator ();


  bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  toolBar[3]->addAction (bHelp);


  //  Create Edit toolbar

  toolBar[4] = new QToolBar (tr ("Edit tool bar"));
  toolBar[4]->setToolTip (tr ("Edit tool bar"));
  addToolBar (toolBar[4]);
  toolBar[4]->setObjectName (tr ("pfmEdit3D Edit tool bar"));

  bStop = new QToolButton (this);
  bStop->setIcon (QIcon (":/icons/stop.xpm"));
  bStop->setToolTip (tr ("Stop drawing"));
  bStop->setWhatsThis (stopText);
  bStop->setEnabled (FALSE);
  connect (bStop, SIGNAL (clicked ()), this, SLOT (slotStop ()));
  toolBar[4]->addWidget (bStop);


  toolBar[4]->addSeparator ();
  toolBar[4]->addSeparator ();


  bDeletePoint = new QToolButton (this);
  misc.button[DELETE_POINT_MODE_KEY] = bDeletePoint;
  bDeletePoint->setIcon (misc.buttonIcon[DELETE_POINT_MODE_KEY]);
  modeGrp->addButton (bDeletePoint, DELETE_POINT);
  bDeletePoint->setWhatsThis (deletePointText);
  bDeletePoint->setCheckable (TRUE);
  toolBar[4]->addWidget (bDeletePoint);


  toolBar[4]->addSeparator ();
  toolBar[4]->addSeparator ();


  bDeleteRect = new QToolButton (this);
  misc.button[DELETE_RECTANGLE_MODE_KEY] = bDeleteRect;
  bDeleteRect->setIcon (misc.buttonIcon[DELETE_RECTANGLE_MODE_KEY]);
  modeGrp->addButton (bDeleteRect, DELETE_RECTANGLE);
  bDeleteRect->setWhatsThis (deleteRectText);
  bDeleteRect->setCheckable (TRUE);
  toolBar[4]->addWidget (bDeleteRect);

  bDeletePoly = new QToolButton (this);
  misc.button[DELETE_POLYGON_MODE_KEY] = bDeletePoly;
  bDeletePoly->setIcon (misc.buttonIcon[DELETE_POLYGON_MODE_KEY]);
  modeGrp->addButton (bDeletePoly, DELETE_POLYGON);
  bDeletePoly->setWhatsThis (deletePolyText);
  bDeletePoly->setCheckable (TRUE);
  toolBar[4]->addWidget (bDeletePoly);


  toolBar[4]->addSeparator ();
  toolBar[4]->addSeparator ();


  bKeepPoly = new QToolButton (this);
  bKeepPoly->setIcon (QIcon (":/icons/keep_poly.xpm"));
  modeGrp->addButton (bKeepPoly, KEEP_POLYGON);
  bKeepPoly->setToolTip (tr ("Invalidate data outside polygon"));
  bKeepPoly->setWhatsThis (keepPolyText);
  bKeepPoly->setCheckable (TRUE);
  toolBar[4]->addWidget (bKeepPoly);


  toolBar[4]->addSeparator ();
  toolBar[4]->addSeparator ();


  bRestoreRect = new QToolButton (this);
  bRestoreRect->setIcon (QIcon (":/icons/restore_rect.xpm"));
  modeGrp->addButton (bRestoreRect, RESTORE_RECTANGLE);
  bRestoreRect->setToolTip (tr ("Restore invalidated data inside a rectangular area"));
  bRestoreRect->setWhatsThis (restoreRectText);
  bRestoreRect->setCheckable (TRUE);
  toolBar[4]->addWidget (bRestoreRect);

  bRestorePoly = new QToolButton (this);
  misc.button[10] = bRestorePoly;
  bRestorePoly->setIcon (QIcon (":/icons/restore_poly.xpm"));
  modeGrp->addButton (bRestorePoly, RESTORE_POLYGON);
  bRestorePoly->setToolTip (tr ("Restore invalidated data inside a polygonal area"));
  bRestorePoly->setWhatsThis (restorePolyText);
  bRestorePoly->setCheckable (TRUE);
  toolBar[4]->addWidget (bRestorePoly);


  bUndo = new QToolButton (this);
  misc.button[UNDO_KEY] = bUndo;
  bUndo->setIcon (misc.buttonIcon[UNDO_KEY]);
  bUndo->setToolTip (tr ("Undo last edit operation"));
  bUndo->setWhatsThis (undoText);
  connect (bUndo, SIGNAL (clicked ()), this, SLOT (slotUndo ()));
  toolBar[4]->addWidget (bUndo);


  toolBar[4]->addSeparator ();
  toolBar[4]->addSeparator ();


  bHotkeyPoly = new QToolButton (this);
  misc.button[RUN_HOTKEY_POLYGON_MODE_KEY] = bHotkeyPoly;
  bHotkeyPoly->setIcon (misc.buttonIcon[RUN_HOTKEY_POLYGON_MODE_KEY]);
  modeGrp->addButton (bHotkeyPoly, HOTKEY_POLYGON);
  bHotkeyPoly->setWhatsThis (hotkeyPolyText);
  bHotkeyPoly->setCheckable (TRUE);
  toolBar[4]->addWidget (bHotkeyPoly);


  toolBar[4]->addSeparator ();
  toolBar[4]->addSeparator ();


  bFilter = new QToolButton (this);
  misc.button[FILTER_KEY] = bFilter;
  bFilter->setIcon (misc.buttonIcon[FILTER_KEY]);
  bFilter->setWhatsThis (filterText);
  connect (bFilter, SIGNAL (clicked ()), this, SLOT (slotFilter ()));
  toolBar[4]->addWidget (bFilter);

  bAttrFilter = new QToolButton (this);
  misc.button[ATTR_FILTER_KEY] = bAttrFilter;
  bAttrFilter->setIcon (misc.buttonIcon[ATTR_FILTER_KEY]);
  bAttrFilter->setWhatsThis (attrFilterText);
  connect (bAttrFilter, SIGNAL (clicked ()), this, SLOT (slotAttrFilter ()));
  toolBar[4]->addWidget (bAttrFilter);

  bFilterRectMask = new QToolButton (this);
  misc.button[RECTANGLE_FILTER_MASK_KEY] = bFilterRectMask;
  bFilterRectMask->setIcon (misc.buttonIcon[RECTANGLE_FILTER_MASK_KEY]);
  bFilterRectMask->setWhatsThis (filterRectMaskText);
  modeGrp->addButton (bFilterRectMask, RECT_FILTER_MASK);
  bFilterRectMask->setCheckable (TRUE);
  toolBar[4]->addWidget (bFilterRectMask);

  bFilterPolyMask = new QToolButton (this);
  misc.button[POLYGON_FILTER_MASK_KEY] = bFilterPolyMask;
  bFilterPolyMask->setIcon (misc.buttonIcon[POLYGON_FILTER_MASK_KEY]);
  bFilterPolyMask->setToolTip (tr ("Mask a polygonal area from the filter"));
  bFilterPolyMask->setWhatsThis (filterPolyMaskText);
  modeGrp->addButton (bFilterPolyMask, POLY_FILTER_MASK);
  bFilterPolyMask->setCheckable (TRUE);
  toolBar[4]->addWidget (bFilterPolyMask);

  bClearMasks = new QToolButton (this);
  bClearMasks->setIcon (QIcon (":/icons/clear_filter_masks.xpm"));
  bClearMasks->setToolTip (tr ("Clear filter masks"));
  bClearMasks->setWhatsThis (clearMasksText);
  connect (bClearMasks, SIGNAL (clicked ()), this, SLOT (slotClearFilterMasks ()));
  toolBar[4]->addWidget (bClearMasks);

  toolBar[4]->addSeparator ();
  toolBar[4]->addSeparator ();


  bMeasure = new QToolButton (this);
  bMeasure->setIcon (QIcon (":/icons/measure.xpm"));
  bMeasure->setToolTip (tr ("Measure distance, azimuth, and Z difference"));
  modeGrp->addButton (bMeasure, MEASURE);
  bMeasure->setWhatsThis (measureText);
  bMeasure->setCheckable (TRUE);
  toolBar[4]->addWidget (bMeasure);


  //  Create Mask toolbar

  toolBar[5] = new QToolBar (tr ("Mask tool bar"));
  toolBar[5]->setToolTip (tr ("Mask tool bar"));
  addToolBar (toolBar[5]);
  toolBar[5]->setObjectName (tr ("pfmEdit3D Mask tool bar"));

  bMaskOutsideRect = new QToolButton (this);
  bMaskOutsideRect->setIcon (QIcon (":/icons/mask_outside_rect.xpm"));
  bMaskOutsideRect->setToolTip (tr ("Mask data outside a rectangle"));
  bMaskOutsideRect->setWhatsThis (maskOutsideRectText);
  bMaskOutsideRect->setCheckable (TRUE);
  modeGrp->addButton (bMaskOutsideRect, MASK_OUTSIDE_RECTANGLE);
  toolBar[5]->addWidget (bMaskOutsideRect);

  bMaskOutsidePoly = new QToolButton (this);
  bMaskOutsidePoly->setIcon (QIcon (":/icons/mask_outside_poly.xpm"));
  bMaskOutsidePoly->setToolTip (tr ("Mask data outside a polygon"));
  bMaskOutsidePoly->setWhatsThis (maskOutsidePolyText);
  bMaskOutsidePoly->setCheckable (TRUE);
  modeGrp->addButton (bMaskOutsidePoly, MASK_OUTSIDE_POLYGON);
  toolBar[5]->addWidget (bMaskOutsidePoly);


  toolBar[5]->addSeparator ();
  toolBar[5]->addSeparator ();


  bMaskInsideRect = new QToolButton (this);
  bMaskInsideRect->setIcon (QIcon (":/icons/mask_inside_rect.xpm"));
  bMaskInsideRect->setToolTip (tr ("Mask data inside a rectangle"));
  bMaskInsideRect->setWhatsThis (maskInsideRectText);
  bMaskInsideRect->setCheckable (TRUE);
  modeGrp->addButton (bMaskInsideRect, MASK_INSIDE_RECTANGLE);
  toolBar[5]->addWidget (bMaskInsideRect);

  bMaskInsidePoly = new QToolButton (this);
  bMaskInsidePoly->setIcon (QIcon (":/icons/mask_inside_poly.xpm"));
  bMaskInsidePoly->setToolTip (tr ("Mask data inside a polygon"));
  bMaskInsidePoly->setWhatsThis (maskInsidePolyText);
  bMaskInsidePoly->setCheckable (TRUE);
  modeGrp->addButton (bMaskInsidePoly, MASK_INSIDE_POLYGON);
  toolBar[5]->addWidget (bMaskInsidePoly);


  toolBar[5]->addSeparator ();
  toolBar[5]->addSeparator ();


  //  Create LIDAR toolbar
  //  If you want to add or subtract programs it needs to be done here, in pfmEditDef.hpp, and in set_defaults.cpp

  toolBar[6] = new QToolBar (tr ("LIDAR tool bar"));
  toolBar[6]->setToolTip (tr ("LIDAR tool bar"));
  addToolBar (toolBar[6]);
  toolBar[6]->setObjectName (tr ("pfmEdit LIDAR tool bar"));


  QButtonGroup *progGrp = new QButtonGroup (this);
  connect (progGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotProg (int)));
  progGrp->setExclusive (FALSE);


  //  bAttributeViewer is not added to progGrp because it is handled differently than the other programs in pfmEdit3D.

  bAttributeViewer = new QToolButton (this);
  bAttributeViewer->setIcon (QIcon (":/icons/attributeviewer.png"));
  bAttributeViewer->setWhatsThis (attributeViewerText);
  bAttributeViewer->setCheckable (TRUE);
  QString tip =  options.description[ATTRIBUTEVIEWER] + "  [" + options.hotkey[ATTRIBUTEVIEWER] + "]";
  bAttributeViewer->setToolTip (tip);
  bAttributeViewer->setShortcut (QKeySequence (options.hotkey[ATTRIBUTEVIEWER]));
  options.progButton[ATTRIBUTEVIEWER] = bAttributeViewer;
  connect (bAttributeViewer, SIGNAL (clicked (bool)), this, SLOT (slotAttributeViewer (bool)));
  toolBar[6]->addWidget (bAttributeViewer);


  //  It is noted that the distance threshold tool button is not an external process.  It is merely a tool that relies
  //  on Attribute Viewer for data transmission.  Therefore no hotkey is established for this button.

  bDistanceThresh = new QToolButton (this);
  misc.button[AV_DISTANCE_THRESHOLD_KEY] = bDistanceThresh;
  bDistanceThresh->setIcon (misc.buttonIcon[AV_DISTANCE_THRESHOLD_KEY]);
  bDistanceThresh->setWhatsThis (distanceThreshText);
  bDistanceThresh->setCheckable(TRUE);
  bDistanceThresh->setEnabled (FALSE);
  connect (bDistanceThresh, SIGNAL (toggled (bool)), this, SLOT (slotAVDistanceThresh (bool)));
  toolBar[6]->addWidget (bDistanceThresh);


  bLidarMonitor = new QToolButton (this);
  bLidarMonitor->setIcon (QIcon (":/icons/lidar_monitor.png"));
  bLidarMonitor->setWhatsThis (lidarMonitorText);
  bLidarMonitor->setCheckable (TRUE);
  tip =  options.description[LIDARMONITOR] + "  [" + options.hotkey[LIDARMONITOR] + "]";
  bLidarMonitor->setToolTip (tip);
  bLidarMonitor->setShortcut (QKeySequence (options.hotkey[LIDARMONITOR]));
  options.progButton[LIDARMONITOR] = bLidarMonitor;
  progGrp->addButton (bLidarMonitor, LIDARMONITOR);
  toolBar[6]->addWidget (bLidarMonitor);

  bRMSMonitor = new QToolButton (this);
  bRMSMonitor->setIcon (QIcon (":/icons/rms_monitor.png"));
  bRMSMonitor->setWhatsThis (rmsMonitorText);
  bRMSMonitor->setCheckable (TRUE);
  tip =  options.description[RMSMONITOR] + "  [" + options.hotkey[RMSMONITOR] + "]";
  bRMSMonitor->setToolTip (tip);
  bRMSMonitor->setShortcut (options.hotkey[RMSMONITOR]);
  options.progButton[RMSMONITOR] = bRMSMonitor;
  progGrp->addButton (bRMSMonitor, RMSMONITOR);
  toolBar[6]->addWidget (bRMSMonitor);


  //  bHOFWaveFilter is not added to progGrp because it is handled differently than the other programs in pfmEdit3D.

  bHOFWaveFilter = new QToolButton (this);
  bHOFWaveFilter->setIcon (QIcon (":/icons/hof_filter.png"));
  bHOFWaveFilter->setWhatsThis (HOFWaveFilterText);
  bHOFWaveFilter->setCheckable (FALSE);
  tip = options.description[HOFWAVEFILTER] + "  [" + options.hotkey[HOFWAVEFILTER] + "]";
  bHOFWaveFilter->setToolTip (tip);
  bHOFWaveFilter->setShortcut (QKeySequence (options.hotkey[HOFWAVEFILTER]));
  options.progButton[HOFWAVEFILTER] = bHOFWaveFilter;
  connect (bHOFWaveFilter, SIGNAL (clicked ()), this, SLOT (slotHOFWaveFilter ()));
  toolBar[6]->addWidget (bHOFWaveFilter);


  bChartsPic = new QToolButton (this);
  bChartsPic->setIcon (QIcon (":/icons/charts_pic.png"));
  bChartsPic->setWhatsThis (chartsPicText);
  bChartsPic->setCheckable (TRUE);
  tip =  options.description[CHARTSPIC] + "  [" + options.hotkey[CHARTSPIC] + "]";
  bChartsPic->setToolTip (tip);
  bChartsPic->setShortcut (QKeySequence (options.hotkey[CHARTSPIC]));
  options.progButton[CHARTSPIC] = bChartsPic;
  progGrp->addButton (bChartsPic, CHARTSPIC);
  toolBar[6]->addWidget (bChartsPic);

  bWaveformMonitor = new QToolButton (this);
  bWaveformMonitor->setIcon (QIcon (":/icons/waveform_monitor.xpm"));
  bWaveformMonitor->setWhatsThis (waveformMonitorText);
  bWaveformMonitor->setCheckable (TRUE);
  tip =  options.description[WAVEFORMMONITOR] + "  [" + options.hotkey[WAVEFORMMONITOR] + "]";
  bWaveformMonitor->setToolTip (tip);
  bWaveformMonitor->setShortcut (QKeySequence (options.hotkey[WAVEFORMMONITOR]));
  options.progButton[WAVEFORMMONITOR] = bWaveformMonitor;
  progGrp->addButton (bWaveformMonitor, WAVEFORMMONITOR);
  toolBar[6]->addWidget (bWaveformMonitor);

  bCZMILwaveMonitor = new QToolButton (this);
  bCZMILwaveMonitor->setIcon (QIcon (":/icons/wave_monitor.xpm"));
  bCZMILwaveMonitor->setWhatsThis (CZMILwaveMonitorText);
  bCZMILwaveMonitor->setCheckable (TRUE);
  tip =  options.description[CZMILWAVEMONITOR] + "  [" + options.hotkey[CZMILWAVEMONITOR] + "]";
  bCZMILwaveMonitor->setToolTip (tip);
  bCZMILwaveMonitor->setShortcut (QKeySequence (options.hotkey[CZMILWAVEMONITOR]));
  options.progButton[CZMILWAVEMONITOR] = bCZMILwaveMonitor;
  progGrp->addButton (bCZMILwaveMonitor, CZMILWAVEMONITOR);
  toolBar[6]->addWidget (bCZMILwaveMonitor);

  bWaveWaterfallAPD = new QToolButton (this);
  bWaveWaterfallAPD->setIcon (QIcon (":/icons/wave_waterfall_apd.png"));
  bWaveWaterfallAPD->setWhatsThis (waveWaterfallAPDText);
  bWaveWaterfallAPD->setCheckable (TRUE);
  tip =  options.description[WAVEWATERFALL_APD] + "  [" + options.hotkey[WAVEWATERFALL_APD] + "]";
  bWaveWaterfallAPD->setToolTip (tip);
  bWaveWaterfallAPD->setShortcut (QKeySequence (options.hotkey[WAVEWATERFALL_APD]));
  options.progButton[WAVEWATERFALL_APD] = bWaveWaterfallAPD;
  progGrp->addButton (bWaveWaterfallAPD, WAVEWATERFALL_APD);
  toolBar[6]->addWidget (bWaveWaterfallAPD);

  bWaveWaterfallPMT = new QToolButton (this);
  bWaveWaterfallPMT->setIcon (QIcon (":/icons/wave_waterfall_pmt.png"));
  bWaveWaterfallPMT->setWhatsThis (waveWaterfallPMTText);
  bWaveWaterfallPMT->setCheckable (TRUE);
  tip =  options.description[WAVEWATERFALL_PMT] + "  [" + options.hotkey[WAVEWATERFALL_PMT] + "]";
  bWaveWaterfallPMT->setToolTip (tip);
  bWaveWaterfallPMT->setShortcut (QKeySequence (options.hotkey[WAVEWATERFALL_PMT]));
  options.progButton[WAVEWATERFALL_PMT] = bWaveWaterfallPMT;
  progGrp->addButton (bWaveWaterfallPMT, WAVEWATERFALL_PMT);
  toolBar[6]->addWidget (bWaveWaterfallPMT);

  bHawkeyeMonitor = new QToolButton (this);
  bHawkeyeMonitor->setIcon (QIcon (":/icons/hawkeye_monitor.png"));
  bHawkeyeMonitor->setWhatsThis (hawkeyeMonitorText);
  bHawkeyeMonitor->setCheckable (TRUE);
  tip =  options.description[HAWKEYEMONITOR] + "  [" + options.hotkey[HAWKEYEMONITOR] + "]";
  bHawkeyeMonitor->setToolTip (tip);
  bHawkeyeMonitor->setShortcut (QKeySequence (options.hotkey[HAWKEYEMONITOR]));
  options.progButton[HAWKEYEMONITOR] = bHawkeyeMonitor;
  progGrp->addButton (bHawkeyeMonitor, HAWKEYEMONITOR);
  toolBar[6]->addWidget (bHawkeyeMonitor);


  //  Create GSF toolbar

  toolBar[7] = new QToolBar (tr ("GSF tool bar"));
  toolBar[7]->setToolTip (tr ("GSF tool bar"));
  addToolBar (toolBar[7]);
  toolBar[7]->setObjectName (tr ("pfmEdit GSF tool bar"));

  bGSFMonitor = new QToolButton (this);
  bGSFMonitor->setIcon (QIcon (":/icons/gsf_monitor.png"));
  bGSFMonitor->setWhatsThis (gsfMonitorText);
  bGSFMonitor->setCheckable (TRUE);
  tip =  options.description[GSFMONITOR] + "  [" + options.hotkey[GSFMONITOR] + "]";
  bGSFMonitor->setToolTip (tip);
  bGSFMonitor->setShortcut (options.hotkey[GSFMONITOR]);
  options.progButton[GSFMONITOR] = bGSFMonitor;
  progGrp->addButton (bGSFMonitor, GSFMONITOR);
  toolBar[7]->addWidget (bGSFMonitor);


  QMenu *hideMenu = new QMenu (this);
  connect (hideMenu, SIGNAL (aboutToShow ()), this, SLOT (slotHideMenuShow ()));

  hideGrp = new QActionGroup (this);
  hideGrp->setExclusive (FALSE);
  connect (hideGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotHideMenu (QAction *)));


  for (NV_INT32 i = 0 ; i < 5 ; i++)
    {
      QString user;
      user.sprintf (tr ("Hide PFM_USER_%02d").toAscii (), i);
      hideAct[i] = hideMenu->addAction (user);
      hideAct[i]->setCheckable (TRUE);
      hideAct[i]->setChecked (FALSE);
      hideGrp->addAction (hideAct[i]);
    }

  hideAct[5] = hideMenu->addAction (tr ("Hide highlighted data"));
  hideAct[5]->setCheckable (TRUE);
  hideAct[5]->setChecked (FALSE);
  hideGrp->addAction (hideAct[5]);

  bHide = new QToolButton (this);
  bHide->setToolTip (tr ("Hide data based on types or flags"));
  bHide->setWhatsThis (hideText);
  bHide->setPopupMode (QToolButton::InstantPopup);
  bHide->setIcon (QIcon (":/icons/hide_user_flag.xpm"));
  bHide->setMenu (hideMenu);
  toolBar[5]->addWidget (bHide);


  QMenu *showMenu = new QMenu (this);
  connect (showMenu, SIGNAL (aboutToShow ()), this, SLOT (slotShowMenuShow ()));

  showGrp = new QActionGroup (this);
  showGrp->setExclusive (FALSE);
  connect (showGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotShowMenu (QAction *)));


  for (NV_INT32 i = 0 ; i < 5 ; i++)
    {
      QString user;
      user.sprintf (tr ("Show PFM_USER_%02d").toAscii (), i);
      showAct[i] = showMenu->addAction (user);
      showAct[i]->setCheckable (TRUE);
      showAct[i]->setChecked (FALSE);
      showGrp->addAction (showAct[i]);
    }

  showAct[5] = showMenu->addAction (tr ("Show highlighted data"));
  showAct[5]->setCheckable (TRUE);
  showAct[5]->setChecked (FALSE);
  showGrp->addAction (showAct[5]);


  bShow = new QToolButton (this);
  bShow->setToolTip (tr ("Show data based on types or flags"));
  bShow->setWhatsThis (showText);
  bShow->setPopupMode (QToolButton::InstantPopup);
  bShow->setIcon (QIcon (":/icons/show_user_flag.xpm"));
  bShow->setMenu (showMenu);
  toolBar[5]->addWidget (bShow);


  toolBar[5]->addSeparator ();
  toolBar[5]->addSeparator ();


  bMaskReset = new QToolButton (this);
  bMaskReset->setIcon (QIcon (":/icons/mask_reset.xpm"));
  bMaskReset->setToolTip (tr ("Clear data mask"));
  bMaskReset->setWhatsThis (maskResetText);
  connect (bMaskReset, SIGNAL (clicked ()), this, SLOT (slotMaskReset ()));
  toolBar[5]->addWidget (bMaskReset);


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


  //  Save the hofWaveFilter options to shared memory.

  misc.abe_share->filterShare.search_radius = options.hofWaveFilter_search_radius;
  misc.abe_share->filterShare.search_width = options.hofWaveFilter_search_width;
  misc.abe_share->filterShare.rise_threshold = options.hofWaveFilter_rise_threshold;
  misc.abe_share->filterShare.pmt_ac_zero_offset_required = options.hofWaveFilter_pmt_ac_zero_offset_required;
  misc.abe_share->filterShare.apd_ac_zero_offset_required = options.hofWaveFilter_apd_ac_zero_offset_required;


  //  Allocate the colors.

  options.min_hsv_color[0] = misc.abe_share->min_hsv_color;
  options.max_hsv_color[0] = misc.abe_share->max_hsv_color;
  options.min_hsv_value[0] = misc.abe_share->min_hsv_value;
  options.max_hsv_value[0] = misc.abe_share->max_hsv_value;
  options.min_hsv_locked[0] = misc.abe_share->min_hsv_locked;
  options.max_hsv_locked[0] = misc.abe_share->max_hsv_locked;

  NV_FLOAT32 hue_inc = (NV_FLOAT32) (options.max_hsv_color[0] - options.min_hsv_color[0]) / (NV_FLOAT32) (NUMSHADES + 1);

  for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
    {
      misc.track_color_array[j].setHsv ((NV_INT32) (options.min_hsv_color[0] + j * hue_inc), 255, 255, 255); 
      misc.color_array[0][j][0] = misc.color_array[1][j][0] = misc.track_color_array[j].red ();
      misc.color_array[0][j][1] = misc.color_array[1][j][1] = misc.track_color_array[j].green ();
      misc.color_array[0][j][2] = misc.color_array[1][j][2] = misc.track_color_array[j].blue ();
      misc.color_array[0][j][3] = misc.track_color_array[j].alpha ();

      misc.color_array[1][j][3] = options.slice_alpha;
    }
  options.ref_color[1] = options.ref_color[0];
  options.ref_color[1].setAlpha (options.slice_alpha);


  //  Allocate the colors to be used for color by line.  These are full value and half value colors.
  //  We're going to range the colors from 200 to 0 then wrap around from 360 to 300 which gives us a 
  //  range of 260.  The colors transitions for this don't need to be smooth since the locations are
  //  somewhat random based on the line number.  These colors never change (unlike the color by depth or 
  //  attribute colors) but the alpha value may change.

  hue_inc = 260.0 / (NV_FLOAT32) (NUMSHADES + 1);

  for (NV_INT32 i = 0 ; i < 2 ; i++)
    {
      NV_FLOAT32 start_hue = 200.0;

      for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
        {
          NV_INT32 hue = (NV_INT32) (start_hue - (NV_FLOAT32) j * hue_inc);
          if (hue < 0) hue = 360 + hue;

          NV_INT32 k = NUMSHADES * i + j;


          QColor color;

          if (i)
            {
              color.setHsv (hue, 255, 128, 255);
            }
          else
            {
              color.setHsv (hue, 255, 255, 255);
            }

          misc.line_color_array[0][k][0] = misc.line_color_array[1][k][0] = color.red ();
          misc.line_color_array[0][k][1] = misc.line_color_array[1][k][1] = color.green ();
          misc.line_color_array[0][k][2] = misc.line_color_array[1][k][2] = color.blue ();
          misc.line_color_array[0][k][3] = 255;
          misc.line_color_array[1][k][3] = options.slice_alpha;
        }
    }


  //  Allocate the undo blocks based on the max number of undo levels (user option).

  misc.undo = (UNDO *) calloc (options.undo_levels, sizeof (UNDO));
  if (misc.undo == NULL)
    {
      perror ("Allocating undo block pointers in pfmEdit3D.cpp");
      exit (-1);
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

  mapdef.renderMode = options.drawingMode;
  mapdef.complexObjectWidth = options.objectWidth;
  mapdef.complexDivisionals = options.objectDivisionals;


  //  Save the original bounds for use if we move the area from within pfmEdit3D.

  misc.orig_bounds = misc.abe_share->edit_area;


  //  If pfm3D is up we need to tell it to clear it's memory until we are finished with the 
  //  3D editor.  If we have a huge amount of memory in use it can slow things down too much.
  //  The other end of this (i.e. setting PFMEDIT3D_CLOSED) happens in pfmView.

  misc.abeShare->lock ();
  {
    misc.abe_share->key = PFMEDIT3D_OPENED;
  }
  misc.abeShare->unlock ();


  //  Save the original bounds for use if we move the area from within pfmEdit3D.

  orig_bounds = misc.abe_share->edit_area;


  //  Read in the data so that we can set the bounds correctly.

  get_buffer (&misc, &options);


  //  Set the Attribute viewer defaults.

  if (misc.hydro_lidar_present)
    {
      misc.abeShare->lock ();
      misc.abe_share->avShare.avNewData = NVFalse;
      misc.abe_share->avShare.hitMax = NVFalse;


      // set AV tracking bits to any AV's that are running

      misc.abe_share->avShare.avTracking = misc.abe_share->avShare.avRunning;

      misc.avb.shotArray = (AV_SHOT_INFO *) malloc (sizeof (AV_SHOT_INFO) * MAX_ATTRIBUTE_SHOTS);

      if (misc.avb.shotArray == NULL)
        {
          perror ("Allocating attribute viewer shot memory");
          clean_exit (-1);
        }
 
      switch (options.avInterfaceBoxSize)
        {
        case 0:
          misc.avb.avBoxSize = 4.5f;
          break;

        case 1:
          misc.avb.avBoxSize = 12.5f;
          break;

        case 2:
          misc.avb.avBoxSize = 25.0f;
          break;
        }

      misc.abe_share->avShare.numShots = 0;
      misc.abeShare->unlock ();
    }


  //  Set number of points loaded in the window title

  QString title;
  title.sprintf (tr (" - Number of points - %d").toAscii (), misc.abe_share->point_cloud_count);
  title.prepend (QString (VERSION));
  this->setWindowTitle (title);


  mapdef.initial_bounds.min_x = misc.displayed_area.min_x;
  mapdef.initial_bounds.min_y = misc.displayed_area.min_y;
  mapdef.initial_bounds.max_x = misc.displayed_area.max_x;
  mapdef.initial_bounds.max_y = misc.displayed_area.max_y;
  mapdef.initial_bounds.min_z = 999999999.0;
  mapdef.initial_bounds.max_z = -999999999.0;


  //  Compute the initial Z bounds.

  for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
    {
      if (!check_bounds (&options, &misc, i, NVTrue, NVFalse))
        {
          mapdef.initial_bounds.min_z = qMin (mapdef.initial_bounds.min_z, (NV_FLOAT64) -misc.data[i].z);
          mapdef.initial_bounds.max_z = qMax (mapdef.initial_bounds.max_z, (NV_FLOAT64) -misc.data[i].z);
        }
    }

  mapdef.initial_y_rotation = options.y_rotation;
  mapdef.initial_zx_rotation = options.zx_rotation;


  //  Save the displayed area for other ABE programs

  misc.abe_share->displayed_area = misc.displayed_area;


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Make the map.

  map = new nvMapGL (this, &mapdef, "pfmEdit3D");
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


  NV_INT32 trackBoxWidth = 250, trackBoxHeight = 250;


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

  track_mapdef.initial_bounds.min_x = misc.abe_share->edit_area.min_x;
  track_mapdef.initial_bounds.min_y = misc.abe_share->edit_area.min_y;
  track_mapdef.initial_bounds.max_x = misc.abe_share->edit_area.max_x;
  track_mapdef.initial_bounds.max_y = misc.abe_share->edit_area.max_y;


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


  //  The following sets up the statistical filter accept/reject notebook page.

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


  stdValue = new QLineEdit ("2.4", this);
  stdValue->setAlignment (Qt::AlignCenter);
  QString std;
  std.sprintf ("%0.1f", options.filterSTD);
  stdValue->setText (std);
  stdValue->setToolTip (tr ("Filter standard deviation value"));
  stdValue->setWhatsThis (stdSliderText);
  connect (stdValue, SIGNAL (returnPressed ()), this, SLOT (slotFilterStdReturn ()));
  sboxLayout->addWidget (stdValue);


  filterBoxLayout->addWidget (sbox);
  fboxLayout->addWidget (filterBox);


  QGroupBox *arbox = new QGroupBox (this);
  arbox->setFlat (TRUE);
  QHBoxLayout *arboxLayout = new QHBoxLayout;
  arbox->setLayout (arboxLayout);

  filterAccept = new QPushButton (tr ("Accept"), this);
  filterAccept->setToolTip (tr ("Accept filter results ") + "[" + options.hotkey[ACCEPT_FILTER_HIGHLIGHTED] + "]");
  filterAccept->setWhatsThis (tr ("Accept filter results and invalidate marked points"));
  connect (filterAccept, SIGNAL (clicked ()), this, SLOT (slotFilterAccept ()));
  arboxLayout->addWidget (filterAccept);


  filterReject = new QPushButton (tr ("Reject"), this);
  filterReject->setToolTip (tr ("Reject filter results ") + "[" + options.hotkey[REJECT_FILTER_HIGHLIGHTED] + "]");
  filterReject->setWhatsThis (tr ("Discard filter results"));
  connect (filterReject, SIGNAL (clicked ()), this, SLOT (slotFilterReject ()));
  arboxLayout->addWidget (filterReject);


  fboxLayout->addWidget (arbox);


  notebook->addTab (fbox, tr ("Statistical Filter Results"));
  notebook->setTabEnabled (1, FALSE);


  //  The following sets up the attribute filter accept/reject notebook page.

  QGroupBox *afbox = new QGroupBox (this);
  QVBoxLayout *afboxLayout = new QVBoxLayout;
  afbox->setLayout (afboxLayout);


  attrFilterBox = new QGroupBox (tr ("000000 points selected"), this);
  attrFilterBox->setWhatsThis (attrFilterBoxText);
  QHBoxLayout *attrFilterBoxLayout = new QHBoxLayout;
  attrFilterBox->setLayout (attrFilterBoxLayout);

  
  QGroupBox *minbox = new QGroupBox (tr ("Filter Min"), this);
  QVBoxLayout *minboxLayout = new QVBoxLayout;
  minbox->setLayout (minboxLayout);


  minSlider = new QSlider ();
  minSlider->setTracking (TRUE);
  minSlider->setRange (0, 100);
  minSlider->setSingleStep (1);
  minSlider->setPageStep (1);
  minSlider->setValue (0);
  minSlider->setToolTip (tr ("Attribute filter minimum value"));
  minSlider->setWhatsThis (minSliderText);
  connect (minSlider, SIGNAL (valueChanged (int)), this, SLOT (slotAttrFilterMinChanged (int)));
  connect (minSlider, SIGNAL (sliderReleased ()), this, SLOT (slotAttrFilter ()));
  minboxLayout->addWidget (minSlider, 10);


  minValue = new QLineEdit ("0000.0000", this);
  minValue->setAlignment (Qt::AlignCenter);
  minValue->setText ("0.0");
  minValue->setToolTip (tr ("Attribute filter minimum value"));
  minValue->setWhatsThis (minSliderText);
  connect (minValue, SIGNAL (returnPressed ()), this, SLOT (slotAttrFilterMinReturn ()));
  minboxLayout->addWidget (minValue);


  attrFilterBoxLayout->addWidget (minbox);


  QGroupBox *maxbox = new QGroupBox (tr ("Filter Max"), this);
  QVBoxLayout *maxboxLayout = new QVBoxLayout;
  maxbox->setLayout (maxboxLayout);


  maxSlider = new QSlider ();
  maxSlider->setTracking (TRUE);
  maxSlider->setRange (0, 100);
  maxSlider->setSingleStep (1);
  maxSlider->setPageStep (1);
  maxSlider->setValue (0);
  maxSlider->setToolTip (tr ("Attribute filter maximum value"));
  maxSlider->setWhatsThis (maxSliderText);
  connect (maxSlider, SIGNAL (valueChanged (int)), this, SLOT (slotAttrFilterMaxChanged (int)));
  connect (maxSlider, SIGNAL (sliderReleased ()), this, SLOT (slotAttrFilter ()));
  maxboxLayout->addWidget (maxSlider, 10);


  maxValue = new QLineEdit ("0000.0000", this);
  maxValue->setAlignment (Qt::AlignCenter);
  maxValue->setText ("0.0");
  maxValue->setToolTip (tr ("Attribute filter maximum value"));
  maxValue->setWhatsThis (maxSliderText);
  connect (maxValue, SIGNAL (returnPressed ()), this, SLOT (slotAttrFilterMaxReturn ()));
  maxboxLayout->addWidget (maxValue);


  attrFilterBoxLayout->addWidget (maxbox);
  afboxLayout->addWidget (attrFilterBox);


  QGroupBox *rbox = new QGroupBox (this);
  rbox->setFlat (TRUE);
  QHBoxLayout *rboxLayout = new QHBoxLayout;
  rbox->setLayout (rboxLayout);

  attrFilterAccept = new QPushButton (tr ("Accept"), this);
  attrFilterAccept->setToolTip (tr ("Accept filter results ") + "[" + options.hotkey[ACCEPT_FILTER_HIGHLIGHTED] + "]");
  attrFilterAccept->setWhatsThis (tr ("Accept filter results and invalidate marked points"));
  connect (attrFilterAccept, SIGNAL (clicked ()), this, SLOT (slotFilterAccept ()));
  rboxLayout->addWidget (attrFilterAccept);


  attrFilterReject = new QPushButton (tr ("Reject"), this);
  attrFilterReject->setToolTip (tr ("Reject filter results ") + "[" + options.hotkey[REJECT_FILTER_HIGHLIGHTED] + "]");
  attrFilterReject->setWhatsThis (tr ("Discard filter results"));
  connect (attrFilterReject, SIGNAL (clicked ()), this, SLOT (slotFilterReject ()));
  rboxLayout->addWidget (attrFilterReject);


  afboxLayout->addWidget (rbox);


  notebook->addTab (afbox, tr ("Attribute Filter Results"));
  notebook->setTabEnabled (2, FALSE);


  //  The following sets up the hofWaveFilter accept/reject notebook page (only if there is some HOF data)

  if (misc.hof_present)
    {
      QGroupBox *hfbox = new QGroupBox (this);
      QVBoxLayout *hfboxLayout = new QVBoxLayout;
      hfbox->setLayout (hfboxLayout);


      hofFilterBox = new QGroupBox (tr ("HOF Wave Filter"), this);
      hofFilterBox->setWhatsThis (HOFWaveFilterText);
      QVBoxLayout *hofFilterBoxLayout = new QVBoxLayout;
      hofFilterBox->setLayout (hofFilterBoxLayout);

  
      QHBoxLayout *srboxLayout = new QHBoxLayout;
      hofFilterBoxLayout->addLayout (srboxLayout);

      QLabel *srLabel = new QLabel (tr ("Radius"));
      srboxLayout->addWidget (srLabel);

      srSlider = new QSlider ();
      srSlider->setOrientation (Qt::Horizontal);
      srSlider->setTracking (TRUE);
      srSlider->setRange (1, 100);
      srSlider->setSingleStep (1);
      srSlider->setPageStep (1);
      srSlider->setValue (NINT (misc.abe_share->filterShare.search_radius * 10.0));
      srSlider->setToolTip (tr ("HOF waveform filter search radius"));
      srSlider->setWhatsThis (srSliderText);
      connect (srSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterSrChanged (int)));
      connect (srSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
      srboxLayout->addWidget (srSlider, 100);


      srValue = new QLineEdit (" 00.0 ", this);
      srValue->setAlignment (Qt::AlignLeft);
      srValue->setMaximumWidth (40);
      QString sr;
      sr.sprintf ("%0.1f", misc.abe_share->filterShare.search_radius);
      srValue->setText (sr);
      srValue->setToolTip (tr ("HOF waveform filter search radius"));
      srValue->setWhatsThis (srSliderText);
      connect (srValue, SIGNAL (returnPressed ()), this, SLOT (slotHOFWaveFilterSrReturn ()));
      srboxLayout->addWidget (srValue);


      QHBoxLayout *swboxLayout = new QHBoxLayout;
      hofFilterBoxLayout->addLayout (swboxLayout);

      QLabel *swLabel = new QLabel (tr ("Width"));
      swboxLayout->addWidget (swLabel);


      swSlider = new QSlider ();
      swSlider->setOrientation (Qt::Horizontal);
      swSlider->setTracking (TRUE);
      swSlider->setRange (3, 20);
      swSlider->setSingleStep (1);
      swSlider->setPageStep (1);
      swSlider->setValue (misc.abe_share->filterShare.search_width);
      swSlider->setToolTip (tr ("HOF waveform filter search width"));
      swSlider->setWhatsThis (swSliderText);
      connect (swSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterSwChanged (int)));
      connect (swSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
      swboxLayout->addWidget (swSlider, 100);


      swValue = new QLineEdit (" 00 ", this);
      swValue->setAlignment (Qt::AlignLeft);
      swValue->setMaximumWidth (40);
      QString sw;
      sw.sprintf ("%d", misc.abe_share->filterShare.search_width);
      swValue->setText (sw);
      swValue->setToolTip (tr ("HOF waveform filter search width"));
      swValue->setWhatsThis (swSliderText);
      connect (swValue, SIGNAL (returnPressed ()), this, SLOT (slotHOFWaveFilterSwReturn ()));
      swboxLayout->addWidget (swValue);


      QHBoxLayout *rtboxLayout = new QHBoxLayout;
      hofFilterBoxLayout->addLayout (rtboxLayout);

      QLabel *rtLabel = new QLabel (tr ("Rise"));
      rtboxLayout->addWidget (rtLabel);


      rtSlider = new QSlider ();
      rtSlider->setOrientation (Qt::Horizontal);
      rtSlider->setTracking (TRUE);
      rtSlider->setRange (3, 8);
      rtSlider->setSingleStep (1);
      rtSlider->setPageStep (1);
      rtSlider->setValue (misc.abe_share->filterShare.rise_threshold);
      rtSlider->setToolTip (tr ("HOF waveform filter rise threshold"));
      rtSlider->setWhatsThis (rtSliderText);
      connect (rtSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterRtChanged (int)));
      connect (rtSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
      rtboxLayout->addWidget (rtSlider, 100);


      rtValue = new QLineEdit (" 00 ", this);
      rtValue->setAlignment (Qt::AlignLeft);
      rtValue->setMaximumWidth (40);
      QString rt;
      rt.sprintf ("%d", misc.abe_share->filterShare.rise_threshold);
      rtValue->setText (rt);
      rtValue->setToolTip (tr ("HOF waveform filter rise threshold"));
      rtValue->setWhatsThis (rtSliderText);
      connect (rtValue, SIGNAL (returnPressed ()), this, SLOT (slotHOFWaveFilterRtReturn ()));
      rtboxLayout->addWidget (rtValue);


      QHBoxLayout *pmtboxLayout = new QHBoxLayout;
      hofFilterBoxLayout->addLayout (pmtboxLayout);

      QLabel *pmtLabel = new QLabel (tr ("PMT AC"));
      pmtboxLayout->addWidget (pmtLabel);


      pmtSlider = new QSlider ();
      pmtSlider->setOrientation (Qt::Horizontal);
      pmtSlider->setTracking (TRUE);
      pmtSlider->setRange (0, 30);
      pmtSlider->setSingleStep (1);
      pmtSlider->setPageStep (1);
      pmtSlider->setValue (misc.abe_share->filterShare.pmt_ac_zero_offset_required);
      pmtSlider->setToolTip (tr ("HOF waveform filter PMT AC zero offset threshold"));
      pmtSlider->setWhatsThis (pmtSliderText);
      connect (pmtSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterPMTChanged (int)));
      connect (pmtSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
      pmtboxLayout->addWidget (pmtSlider, 100);


      pmtValue = new QLineEdit (" 00 ", this);
      pmtValue->setAlignment (Qt::AlignLeft);
      pmtValue->setMaximumWidth (40);
      QString pmt;
      pmt.sprintf ("%d", misc.abe_share->filterShare.pmt_ac_zero_offset_required);
      pmtValue->setText (pmt);
      pmtValue->setToolTip (tr ("HOF waveform filter rise threshold"));
      pmtValue->setWhatsThis (pmtSliderText);
      connect (pmtValue, SIGNAL (returnPressed ()), this, SLOT (slotHOFWaveFilterPMTReturn ()));
      pmtboxLayout->addWidget (pmtValue);


      QHBoxLayout *apdboxLayout = new QHBoxLayout;
      hofFilterBoxLayout->addLayout (apdboxLayout);

      QLabel *apdLabel = new QLabel (tr ("APD AC"));
      apdboxLayout->addWidget (apdLabel);


      apdSlider = new QSlider ();
      apdSlider->setOrientation (Qt::Horizontal);
      apdSlider->setTracking (TRUE);
      apdSlider->setRange (0, 30);
      apdSlider->setSingleStep (1);
      apdSlider->setPageStep (1);
      apdSlider->setValue (misc.abe_share->filterShare.apd_ac_zero_offset_required);
      apdSlider->setToolTip (tr ("HOF waveform filter APD AC zero offset threshold"));
      apdSlider->setWhatsThis (apdSliderText);
      connect (apdSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterAPDChanged (int)));
      connect (apdSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
      apdboxLayout->addWidget (apdSlider, 100);


      apdValue = new QLineEdit (" 00 ", this);
      apdValue->setAlignment (Qt::AlignLeft);
      apdValue->setMaximumWidth (40);
      QString apd;
      apd.sprintf ("%d", misc.abe_share->filterShare.apd_ac_zero_offset_required);
      apdValue->setText (apd);
      apdValue->setToolTip (tr ("HOF waveform filter rise threshold"));
      apdValue->setWhatsThis (apdSliderText);
      connect (apdValue, SIGNAL (returnPressed ()), this, SLOT (slotHOFWaveFilterAPDReturn ()));
      apdboxLayout->addWidget (apdValue);


      hfboxLayout->addWidget (hofFilterBox);


      QGroupBox *hfarbox = new QGroupBox (this);
      hfarbox->setFlat (TRUE);
      QHBoxLayout *hfarboxLayout = new QHBoxLayout;
      hfarbox->setLayout (hfarboxLayout);

      hofWaveFilterAccept = new QPushButton (tr ("Accept"), this);
      hofWaveFilterAccept->setToolTip (tr ("Accept filter results ") + "[" + options.hotkey[ACCEPT_FILTER_HIGHLIGHTED] + "]");
      hofWaveFilterAccept->setWhatsThis (tr ("Accept filter results and invalidate marked points"));
      connect (hofWaveFilterAccept, SIGNAL (clicked ()), this, SLOT (slotFilterAccept ()));
      hfarboxLayout->addWidget (hofWaveFilterAccept);


      hofWaveFilterReject = new QPushButton (tr ("Reject"), this);
      hofWaveFilterReject->setWhatsThis (tr ("Discard filter results"));
      hofWaveFilterReject->setToolTip (tr ("Reject filter results ") + "[" + options.hotkey[REJECT_FILTER_HIGHLIGHTED] + "]");
      connect (hofWaveFilterReject, SIGNAL (clicked ()), this, SLOT (slotFilterReject ()));
      hfarboxLayout->addWidget (hofWaveFilterReject);


      hfboxLayout->addWidget (hfarbox);


      notebook->addTab (hfbox, tr ("HOF Waveform Filter Results"));
      notebook->setTabEnabled (3, FALSE);
    }

  notebook->setCurrentIndex (0);

  leftBoxLayout->addWidget (notebook);


  //  Now we set up the information/status portion of the left side box.

  QGroupBox *filBox = new QGroupBox ();
  filBox->setMaximumWidth (trackBoxWidth);
  QVBoxLayout *filBoxLayout = new QVBoxLayout ();
  filBox->setLayout (filBoxLayout);

  pfmLabel = new QLabel (this);
  pfmLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  pfmLabel->setToolTip (tr ("PFM name for current point"));
  pfmLabel->setWhatsThis (tr ("PFM name for current point"));
  filBoxLayout->addWidget (pfmLabel);

  typLabel = new QLabel (this);
  typLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  typLabel->setToolTip (tr ("Data type for current point"));
  typLabel->setWhatsThis (tr ("Data type for current point"));
  filBoxLayout->addWidget (typLabel);


  leftBoxLayout->addWidget (filBox);


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
  connect (misc.scale[0], SIGNAL (mousePressSignal (QMouseEvent *)), this, SLOT (slotMinScalePressed (QMouseEvent *)));
  connect (misc.scale[NUM_SCALE_LEVELS - 1], SIGNAL (mousePressSignal (QMouseEvent *)), this, SLOT (slotMaxScalePressed (QMouseEvent *)));


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

  QLabel *hvName = new QLabel ("H / V:", this);
  hvName->setToolTip (tr ("Horizontal/Vertical uncertainty of current point"));
  hvName->setMinimumSize (hvName->sizeHint ());
  hvName->setMaximumSize (hvName->sizeHint ());
  statBoxLayout->addWidget (hvName, 5, 1, Qt::AlignLeft);

  hvLabel = new QLabel (this);
  hvLabel->setToolTip (tr ("Horizontal/Vertical uncertainty of current point"));
  hvLabel->setAutoFillBackground (TRUE);
  statBoxLayout->addWidget (hvLabel, 5, 2, Qt::AlignLeft);

  QLabel *exagName = new QLabel ("Exag:", this);
  exagName->setToolTip (tr ("Vertical exaggeration"));
  exagName->setMinimumSize (exagName->sizeHint ());
  statBoxLayout->addWidget (exagName, 6, 1, Qt::AlignLeft);

  exagLabel = new QLabel (this);
  exagLabel->setToolTip (tr ("Vertical exaggeration"));
  exagLabel->setAutoFillBackground (TRUE);
  statBoxLayout->addWidget (exagLabel, 6, 2, Qt::AlignLeft);
  QString lbl;
  lbl.sprintf (tr ("%5.3f").toAscii (), options.exaggeration);
  exagLabel->setText (lbl);


  //  If you add above this, change the value of pos to match

  NV_INT32 pos = 7;


  PFMWDB_attr_count = 0;
  for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++)
    {
      //  We want to make the label even though it's not populated.

      if (i > misc.max_attr - 1)
        {
          QLabel *a1 = new QLabel ("");
          statBoxLayout->addWidget (a1, pos, 1, Qt::AlignLeft);
          QLabel *a2 = new QLabel ("");
          statBoxLayout->addWidget (a2, pos, 2, Qt::AlignLeft);
        }
      else
        {
          QLabel *attrName = new QLabel (tr ("Attr %1:").arg (i + 1), this);
          statBoxLayout->addWidget (attrName, pos, 1, Qt::AlignLeft);

          attrLabel[i] = new QLabel ("00000.00000", this);
          attrLabel[i]->setMinimumSize (attrLabel[i]->sizeHint ());
          statBoxLayout->addWidget (attrLabel[i], pos, 2, Qt::AlignLeft);


          //  Special attribute settings for stuff coming from PFMWDB (time may come from a regular PFM as well).

          if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]).contains ("Time (POSIX minutes)"))
            {
              attrLabel[i]->setToolTip (tr ("Date/Time (minutes)"));
              attrLabel[i]->setWhatsThis (tr ("Date/Time (minutes)"));
              misc.time_attr = i;
              PFMWDB_attr_count++;
            }
          else if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]).contains ("Vertical datum"))
            {
              attrLabel[i]->setToolTip (tr ("Vertical datum"));
              QString vd = tr ("<center><h3>Vertical Datums</h3></center><br><br>");
              for (NV_INT32 j = 1 ; j < 15 ; j++) vd += shortVerticalDatum[j] + " = " + verticalDatum[j] + "<br>";
              attrLabel[i]->setWhatsThis (vd);
              misc.datum_attr = i;
              PFMWDB_attr_count++;
            }
          else if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]).contains ("WGS84 Ellipsoid offset"))
            {
              attrLabel[i]->setToolTip (tr ("WGS84 Ellipsoid offset"));
              attrLabel[i]->setWhatsThis (tr ("WGS84 Ellipsoid offset"));
              misc.ellipsoid_attr = i;
              PFMWDB_attr_count++;
            }
          else
            {
              attrLabel[i]->setToolTip (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]));
              attrLabel[i]->setWhatsThis (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]));
            }
        }

      pos++;
    }

  statBoxLayout->setColumnStretch (1, 1);
  statBoxLayout->setRowStretch (NUM_SCALE_LEVELS + 1, 1);


  leftBoxLayout->addWidget (statBox);


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
  hBox->addWidget (exagBar);
  hBox->addWidget (map, 1);
  hBox->addWidget (sliceBox);
  vBox->addLayout (hBox, 1);


  //  Set View toolbar button states based on saved options.  We don't set the shortcut for the first three because they wil be triggered
  //  from the shortcuts in the pulldown menu.

  bDisplayManInvalid->setChecked (options.display_man_invalid);
  connect (bDisplayManInvalid, SIGNAL (clicked ()), this, SLOT (slotDisplayManInvalid ()));

  bDisplayFltInvalid->setChecked (options.display_flt_invalid);
  connect (bDisplayFltInvalid, SIGNAL (clicked ()), this, SLOT (slotDisplayFltInvalid ()));

  bDisplayNull->setChecked (options.display_null);
  connect (bDisplayNull, SIGNAL (clicked ()), this, SLOT (slotDisplayNull ()));

  bContour->setChecked (options.display_contours);
  connect (bContour, SIGNAL (clicked ()), this, SLOT (slotContour ()));

  bClearHighlight->setToolTip (tr ("Clear highlighted points ") + "[" + options.hotkey[REJECT_FILTER_HIGHLIGHTED] + "]");


  //  Set Feature toolbar button states based on saved options

  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);

  bDisplayChildren->setChecked (options.display_children);
  connect (bDisplayChildren, SIGNAL (clicked ()), this, SLOT (slotDisplayChildren ()));

  bDisplayFeatureInfo->setChecked (options.display_feature_info);
  connect (bDisplayFeatureInfo, SIGNAL (clicked ()), this, SLOT (slotDisplayFeatureInfo ()));

  if (options.function == EDIT_FEATURE) bEditFeature->setChecked (TRUE);


  //  Set Reference toolbar button states based on saved options

  bDisplayReference->setChecked (options.display_reference);
  connect (bDisplayReference, SIGNAL (clicked ()), this, SLOT (slotDisplayReference ()));


  //  Set Utilities toolbar button states based on saved options

  if (misc.abe_share->read_only) options.auto_unload = NVFalse;
  bUnload->setChecked (options.auto_unload);
  connect (bUnload, SIGNAL (clicked ()), this, SLOT (slotUnload ()));


  //  Set Edit toolbar button states based on saved options

  if (options.function == DELETE_POINT) bDeletePoint->setChecked (TRUE);

  if (options.function == DELETE_RECTANGLE) bDeleteRect->setChecked (TRUE);

  if (options.function == DELETE_POLYGON) bDeletePoly->setChecked (TRUE);

  if (options.function == KEEP_POLYGON) bKeepPoly->setChecked (TRUE);

  if (options.function == RESTORE_RECTANGLE) bRestoreRect->setChecked (TRUE);

  if (options.function == RESTORE_POLYGON) bRestorePoly->setChecked (TRUE);


  //  Set all of the button hotkey tooltips and shortcuts

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++) slotPrefHotKeyChanged (i);


  //  Check for attributes and, if there, add the requisite buttons.

  for (NV_INT32 i = 0 ; i < misc.max_attr ; i++)
    {
      QString string = tr ("Color by ") + QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]);
      attr[i + PRE_ATTR] = attrMenu->addAction (string);
      attr[i + PRE_ATTR]->setIcon (attrIcon[i + PRE_ATTR]);
      attributeGrp->addAction (attr[i + PRE_ATTR]);
    }
  bAttr->setMenu (attrMenu);


  //  If the saved value of color_index is greater than the maximum attribute number + PRE_ATTR we switch it to the default.
  //  Values after PRE_ATTR are PFM dependent.

  if (options.color_index < 0 || options.color_index >= misc.max_attr + PRE_ATTR) options.color_index = 0;


  //  Set the PFM user flag color index text and tooltips.

  flag[PRE_USER + 0]->setText (tr ("Mark ") + misc.abe_share->open_args[0].head.user_flag_name[0] + " data");
  flag[PRE_USER + 1]->setText (tr ("Mark ") + misc.abe_share->open_args[0].head.user_flag_name[1] + " data");
  flag[PRE_USER + 2]->setText (tr ("Mark ") + misc.abe_share->open_args[0].head.user_flag_name[2] + " data");
  flag[PRE_USER + 3]->setText (tr ("Mark ") + misc.abe_share->open_args[0].head.user_flag_name[3] + " data");
  flag[PRE_USER + 4]->setText (tr ("Mark ") + misc.abe_share->open_args[0].head.user_flag_name[4] + " data");

  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[0], "PFM_USER_01")) flag[PRE_USER + 0]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[1], "PFM_USER_02")) flag[PRE_USER + 1]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[2], "PFM_USER_03")) flag[PRE_USER + 2]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[3], "PFM_USER_04")) flag[PRE_USER + 3]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[4], "PFM_USER_05")) flag[PRE_USER + 4]->setVisible (FALSE);


  //  Set the PFM user flag hide and show text and tooltips.

  hideAct[0]->setText (tr ("Hide ") + misc.abe_share->open_args[0].head.user_flag_name[0] + " data");
  hideAct[1]->setText (tr ("Hide ") + misc.abe_share->open_args[0].head.user_flag_name[1] + " data");
  hideAct[2]->setText (tr ("Hide ") + misc.abe_share->open_args[0].head.user_flag_name[2] + " data");
  hideAct[3]->setText (tr ("Hide ") + misc.abe_share->open_args[0].head.user_flag_name[3] + " data");
  hideAct[4]->setText (tr ("Hide ") + misc.abe_share->open_args[0].head.user_flag_name[4] + " data");

  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[0], "PFM_USER_01")) hideAct[0]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[1], "PFM_USER_02")) hideAct[1]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[2], "PFM_USER_03")) hideAct[2]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[3], "PFM_USER_04")) hideAct[3]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[4], "PFM_USER_05")) hideAct[4]->setVisible (FALSE);


  //  Add data types if there's more than one.

  if (misc.unique_count > 1)
    {
      hideMenu->addSeparator ();
      for (NV_INT32 i = 0 ; i < misc.unique_count ; i++)
        {
          QString user;
          user.sprintf (tr ("Hide %s data").toAscii (), PFM_data_type[misc.unique_type[i]]);
          hideAct[i + 6] = hideMenu->addAction (user);
          hideAct[i + 6]->setCheckable (TRUE);
          hideAct[i + 6]->setChecked (FALSE);
          hideGrp->addAction (hideAct[i + 6]);
        }
    }
      

  showAct[0]->setText (tr ("Show ") + misc.abe_share->open_args[0].head.user_flag_name[0] + " data");
  showAct[1]->setText (tr ("Show ") + misc.abe_share->open_args[0].head.user_flag_name[1] + " data");
  showAct[2]->setText (tr ("Show ") + misc.abe_share->open_args[0].head.user_flag_name[2] + " data");
  showAct[3]->setText (tr ("Show ") + misc.abe_share->open_args[0].head.user_flag_name[3] + " data");
  showAct[4]->setText (tr ("Show ") + misc.abe_share->open_args[0].head.user_flag_name[4] + " data");

  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[0], "PFM_USER_01")) showAct[0]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[1], "PFM_USER_02")) showAct[1]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[2], "PFM_USER_03")) showAct[2]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[3], "PFM_USER_04")) showAct[3]->setVisible (FALSE);
  if (!strcmp (misc.abe_share->open_args[0].head.user_flag_name[4], "PFM_USER_05")) showAct[4]->setVisible (FALSE);


  //  Add data types if there's more than one.

  if (misc.unique_count > 1)
    {
      showMenu->addSeparator ();
      for (NV_INT32 i = 0 ; i < misc.unique_count ; i++)
        {
          QString user;
          user.sprintf (tr ("Show %s data").toAscii (), PFM_data_type[misc.unique_type[i]]);
          showAct[i + 6] = showMenu->addAction (user);
          showAct[i + 6]->setCheckable (TRUE);
          showAct[i + 6]->setChecked (FALSE);
          showGrp->addAction (showAct[i + 6]);
        }
    }


  //  Right click popup menu

  popupMenu = new QMenu (map);
  connect (popupMenu, SIGNAL (triggered (QAction *)), this, SLOT (slotPopupMenu (QAction *)));

  for (NV_INT32 i = 0 ; i < NUMPOPUPS ; i++) popup[i] = popupMenu->addAction (("            "));

  popupMenu->addSeparator ();
  popupHelp = popupMenu->addAction (tr ("Help"));
  connect (popupHelp, SIGNAL (triggered ()), this, SLOT (slotPopupHelp ()));


  //  Setup the file menu (don't set shortcuts for these menu items because they're handled by the keypress event with the associated buttons)

  saveExitAction = new QAction (tr ("Save and Exit"), this);
  saveExitAction->setStatusTip (tr ("Save changes and exit"));
  connect (saveExitAction, SIGNAL (triggered ()), this, SLOT (slotExitSave ()));

  maskExitAction = new QAction (tr ("Save, Exit, and Mask"), this);
  maskExitAction->setStatusTip (tr ("Save changes, exit, and filter mask in pfmView"));
  connect (maskExitAction, SIGNAL (triggered ()), this, SLOT (slotExitMask ()));

  QAction *noSaveExitAction = new QAction (tr ("Exit"), this);
  noSaveExitAction->setStatusTip (tr ("Exit without saving changes"));
  connect (noSaveExitAction, SIGNAL (triggered ()), this, SLOT (slotExitNoSave ()));


  QMenu *fileMenu = menuBar ()->addMenu (tr ("File"));
  fileMenu->addAction (saveExitAction);
  fileMenu->addAction (maskExitAction);
  fileMenu->addAction (noSaveExitAction);


  //  Setup the Edit menu

  QAction *prefsAct = new QAction (tr ("Preferences"), this);
  prefsAct->setStatusTip (tr ("Change program preferences"));
  prefsAct->setWhatsThis (prefsText);
  connect (prefsAct, SIGNAL (triggered ()), this, SLOT (slotPrefs ()));


  QMenu *editMenu = menuBar ()->addMenu (tr ("Edit"));
  editMenu->addAction (prefsAct);


  //  Setup the Layers menu.

  QAction *layerPrefs = new QAction (tr ("Display"), this);
  layerPrefs->setStatusTip (tr ("Turn display of layers on or off"));
  layerPrefs->setWhatsThis (layerPrefsText);
  connect (layerPrefs, SIGNAL (triggered ()), this, SLOT (slotLayers ()));

  layerMenu = menuBar ()->addMenu (tr ("Layers"));
  layerMenu->addAction (layerPrefs);


  //  Setup the help menu.

  QAction *exHelp = new QAction (tr ("User Guide"), this);
  exHelp->setStatusTip (tr ("Open the Area-Based Editor user guide in a browser"));
  connect (exHelp, SIGNAL (triggered ()), this, SLOT (extendedHelp ()));

  QAction *hotHelp = new QAction (tr ("Hot Keys and GUI control"), this);
  hotHelp->setStatusTip (tr ("Help on GUI control and hot keys (fixed and user modifiable)"));
  connect (hotHelp, SIGNAL (triggered ()), this, SLOT (slotHotkeyHelp ()));

  QAction *toolHelp = new QAction (tr ("Tool bars"), this);
  toolHelp->setStatusTip (tr ("Help on tool bars"));
  connect (toolHelp, SIGNAL (triggered ()), this, SLOT (slotToolbarHelp ()));

  QAction *whatsThisAct = QWhatsThis::createAction (this);

  QAction *aboutAct = new QAction (tr ("About"), this);
  aboutAct->setStatusTip (tr ("Information about pfmEdit3D"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgements = new QAction (tr ("Acknowledgements"), this);
  acknowledgements->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgements, SIGNAL (triggered ()), this, SLOT (slotAcknowledgements ()));

  QAction *aboutQtAct = new QAction (tr ("About Qt"), this);
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("Help"));
  helpMenu->addAction (exHelp);
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


  filLabel = new QLabel (" N/A ", this);
  filLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  filLabel->setToolTip (tr ("File name for current point"));
  filLabel->setWhatsThis (tr ("File name for current point"));
  filLabel->setMinimumSize (filLabel->sizeHint ());
  fpBoxLayout->addWidget (filLabel, 0, 0);

  linLabel = new QLabel (" N/A ", this);
  linLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  linLabel->setToolTip (tr ("Line name for current point"));
  linLabel->setWhatsThis (tr ("Line name for current point"));
  linLabel->setAutoFillBackground (TRUE);
  linPalette = linLabel->palette ();
  misc.widgetBackgroundColor = linPalette.color (QPalette::Normal, QPalette::Window);
  linLabel->setMinimumSize (linLabel->sizeHint ());
  fpBoxLayout->addWidget (linLabel, 0, 1);

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


  //  Get the map center for the reset view slot.

  map->getMapCenter (&misc.map_center_x, &misc.map_center_y, &misc.map_center_z);


  redrawMap (NVTrue);


  //  Check for ancillary programs that may have been up when this program last ran

  if (options.kill_and_respawn)
    {
      for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
        {
          //  If it was up..

          if (options.state[i] == 2)
            {
              //  Check for matching data type in displayed data.

              for (NV_INT32 j = 0 ; j < misc.abe_share->point_cloud_count ; j++)
                {
                  //  If we get a match, kick off the program.

                  if (options.data_type[i][misc.data[j].type])
                    {
                      //  If there is a button associated with this program we want to press the button instead of
                      //  just calling ancillaryProgram.  Pressing the button will kick off the program and leave 
                      //  the button in the proper state.

                      if (options.progButton[i] != NULL)
                        {
                          options.progButton[i]->click ();
                        }
                      else
                        {
                          ancillaryProgram[i] = new ancillaryProg (this, map, &options, &misc, i, j);
                          connect (ancillaryProgram[i], SIGNAL (redrawSignal ()), this, SLOT (slotRedrawMap ()));
                        }
                      break;
                    }
                }
            }
        }
    }
  else
    {
      //  If it was running (only monitor programs), set the button state.  The assumption is that it is still up.

      for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
        {
          //  If it was up..

          if (options.state[i] == 2 && options.progButton[i] != NULL) options.progButton[i]->setChecked (TRUE);
        }
    }


  //  If the attributeViewer is up, get the pixel offsets and tell it to start tracking again.

  if (misc.abe_share->avShare.avRunning)
    {
      misc.avb.avInterfacePixelBuffer = GetPixelOffsetByMeter (&misc, trackMap);
      misc.abeShare->lock ();


      // set tracking bits to match running bits

      misc.abe_share->avShare.avTracking = misc.abe_share->avShare.avRunning;

      misc.abeShare->unlock ();

      bDistanceThresh->setEnabled (TRUE);
    }


  //  If we are flagging data we need to highlight it.

  if (options.flag_index) slotFlagMenu (flag[options.flag_index]);
}



pfmEdit3D::~pfmEdit3D ()
{
}



void 
pfmEdit3D::setMainButtons (NV_BOOL enable)
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


  bExitNoSave->setEnabled (enable);
  bReset->setEnabled (enable);


  //  Only enable the layers menu if we have more than one layer.

  if (misc.abe_share->pfm_count > 1)
    {
      layerMenu->setEnabled (enable);
    }
  else
    {
      layerMenu->setEnabled (FALSE);
    }


  //  Only enable UNDO if we saved an undo block.

  if (misc.undo_count)
    {
      bUndo->setEnabled (enable);
    }
  else
    {
      bUndo->setEnabled (FALSE);
    }


  bDisplayFeature->setEnabled (enable);

  if (!enable || (enable && options.display_feature))
    {
      bDisplayChildren->setEnabled (enable);
      bDisplayFeatureInfo->setEnabled (enable);

      bAddFeature->setEnabled (enable);
      if ((!enable || (enable && misc.visible_feature_count)) && !misc.abe_share->read_only)
        {
          bEditFeature->setEnabled (enable);
          bMoveFeature->setEnabled (enable);
          bDeleteFeature->setEnabled (enable);
          bVerifyFeatures->setEnabled (enable);
        }
      else
        {
          bEditFeature->setEnabled (FALSE);
          bMoveFeature->setEnabled (FALSE);
          bDeleteFeature->setEnabled (FALSE);
          bVerifyFeatures->setEnabled (FALSE);
        }
    }


  //  Only enable these if we're not in read only mode.

  if (!misc.abe_share->read_only)
    {
      bExitSave->setEnabled (enable);
      bExitMask->setEnabled (enable);
      saveExitAction->setEnabled (enable);
      maskExitAction->setEnabled (enable);
      bUnload->setEnabled (enable);
    }
  else
    {
      bExitSave->setEnabled (FALSE);
      bExitMask->setEnabled (FALSE);
      saveExitAction->setEnabled (FALSE);
      maskExitAction->setEnabled (FALSE);
      bUnload->setEnabled (FALSE);
    }


  bDisplayReference->setEnabled (enable);

  bDisplayManInvalid->setEnabled (enable);
  bDisplayFltInvalid->setEnabled (enable);
  if (!enable || (enable && (options.display_man_invalid || options.display_flt_invalid))) bDisplayNull->setEnabled (enable);

  if (!enable || (enable && misc.num_lines)) bDisplayAll->setEnabled (enable);

  bUndisplaySingle->setEnabled (enable);
  bDisplayMultiple->setEnabled (enable);

  bContour->setEnabled (enable);

  bPrefs->setEnabled (enable);


  //  Stop is always the inverse.

  bStop->setEnabled (!enable);


  bDeletePoint->setEnabled (enable);
  bDeleteRect->setEnabled (enable);
  bDeletePoly->setEnabled (enable);
  bKeepPoly->setEnabled (enable);
  bHotkeyPoly->setEnabled (enable);
  bRestoreRect->setEnabled (enable);
  bRestorePoly->setEnabled (enable);

  bFilter->setEnabled (enable);


  //  We don't allow filtering by attribute if we're coloring by depth or line (color_index < 2)

  if (options.color_index < 2)
    {
      bAttrFilter->setEnabled (FALSE);
    }
  else
    {
      bAttrFilter->setEnabled (enable);
    }


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

  if (!enable || (enable && options.display_reference)) 
    {
      bReferencePoly->setEnabled (enable);
      bUnreferencePoly->setEnabled (enable);
    }


  bAttr->setIcon (attrIcon[options.color_index]);
  bAttr->setEnabled (enable);

  bFlag->setIcon (flagIcon[options.flag_index]);
  bFlag->setEnabled (enable);


  if (!misc.hydro_lidar_present)
    {
      bAttributeViewer->setEnabled (FALSE);
    }
  else
    {
      bAttributeViewer->setEnabled (enable);
    }


  if (!misc.gsf_present)
    {
      bGSFMonitor->setEnabled (FALSE);
    }
  else
    {
      bGSFMonitor->setEnabled (enable);
    }


  if (!misc.hydro_lidar_present)
    {
      bLidarMonitor->setEnabled (FALSE);
      bWaveformMonitor->setEnabled (FALSE);
      bCZMILwaveMonitor->setEnabled (FALSE);
      bWaveWaterfallAPD->setEnabled (FALSE);
      bWaveWaterfallPMT->setEnabled (FALSE);
      bRMSMonitor->setEnabled (FALSE);
      bHOFWaveFilter->setEnabled (FALSE);
      bHawkeyeMonitor->setEnabled (FALSE);
    }
  else
    {
      bLidarMonitor->setEnabled (enable);
      bWaveformMonitor->setEnabled (enable);
      bCZMILwaveMonitor->setEnabled (enable);
      bWaveWaterfallAPD->setEnabled (enable);
      bWaveWaterfallPMT->setEnabled (enable);
      bRMSMonitor->setEnabled (enable);
      bHawkeyeMonitor->setEnabled (enable);
    }

  if (!misc.hof_present)
    {
      bHOFWaveFilter->setEnabled (FALSE);
    }
  else
    {
      bHOFWaveFilter->setEnabled (enable);
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
      bInvertHighlight->setEnabled (enable);
      bClearHighlight->setEnabled (enable);
      bClearPoly->setEnabled (enable);
    }
  bHighlightPoly->setEnabled (enable);

  bMeasure->setEnabled (enable);
  bMaskInsideRect->setEnabled (enable);
  bMaskInsidePoly->setEnabled (enable);
  bMaskOutsideRect->setEnabled (enable);
  bMaskOutsidePoly->setEnabled (enable);

  bHide->setEnabled (enable);
  bShow->setEnabled (enable);

  if (!enable || misc.mask_active) bMaskReset->setEnabled (enable);


  //  Set the scaleBox ToolTips and WhatsThis (inverted for attributes).

  NV_INT32 k = options.color_index;
  if (k)
    {
      QString str;

      if (options.max_hsv_locked[k])
        {
          str.sprintf (tr ("Click here to unlock the maximum value (%.2f) and/or change the maximum color").toAscii (), options.max_hsv_value[k]);
          misc.scale[0]->setToolTip (str);
          misc.scale[0]->setWhatsThis (str);
        }
      else
        {
          misc.scale[0]->setToolTip (tr ("Click here to lock the maximum value and/or change the maximum color"));
          misc.scale[0]->setWhatsThis (tr ("Click here to lock the maximum value and/or change the maximum color"));
        }

      if (options.min_hsv_locked[k])
        {
          str.sprintf (tr ("Click here to unlock the minimum value (%.2f) and/or change the maximum color").toAscii (), options.min_hsv_value[k]);
          misc.scale[NUM_SCALE_LEVELS - 1]->setToolTip (str);
          misc.scale[NUM_SCALE_LEVELS - 1]->setWhatsThis (str);
        }
      else
        {
          misc.scale[NUM_SCALE_LEVELS - 1]->setToolTip (tr ("Click here to lock the minimum value and/or change the minimum color"));
          misc.scale[NUM_SCALE_LEVELS - 1]->setWhatsThis (tr ("Click here to lock the minimum value and/or change the minimum color"));
        }
    }
  else
    {
      QString str;

      if (options.min_hsv_locked[k])
        {
          str.sprintf (tr ("Click here to unlock the minimum value (%.2f) and/or change the maximum color").toAscii (), options.min_hsv_value[k]);
          misc.scale[0]->setToolTip (str);
          misc.scale[0]->setWhatsThis (str);
        }
      else
        {
          misc.scale[0]->setToolTip (tr ("Click here to lock the minimum value and/or change the minimum color"));
          misc.scale[0]->setWhatsThis (tr ("Click here to lock the minimum value and/or change the minimum color"));
        }

      if (options.max_hsv_locked[k])
        {
          str.sprintf (tr ("Click here to unlock the maximum value (%.2f) and/or change the maximum color").toAscii (), options.max_hsv_value[k]);
          misc.scale[NUM_SCALE_LEVELS - 1]->setToolTip (str);
          misc.scale[NUM_SCALE_LEVELS - 1]->setWhatsThis (str);
        }
      else
        {
          misc.scale[NUM_SCALE_LEVELS - 1]->setToolTip (tr ("Click here to lock the maximum value and/or change the maximum color"));
          misc.scale[NUM_SCALE_LEVELS - 1]->setWhatsThis (tr ("Click here to lock the maximum value and/or change the maximum color"));
        }
    }
}



void 
pfmEdit3D::setFunctionCursor (NV_INT32 function)
{
  //  Default to point mode for the map.  If we want feature mode we'll switch it below.

  map->setMapMode (NVMAPGL_POINT_MODE);


  discardMovableObjects ();


  switch (function)
    {
    case AV_DISTANCE_TOOL:
      misc.statusProgLabel->setText (tr ("AV distance tool"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::SizeAllCursor);
      break;

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

    case SET_REFERENCE:
      misc.statusProgLabel->setText (tr ("Polygon set reference mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (referencePolyCursor);
      bReferencePoly->setChecked (TRUE);
      break;

    case UNSET_REFERENCE:
      misc.statusProgLabel->setText (tr ("Polygon unset reference mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (unreferencePolyCursor);
      bUnreferencePoly->setChecked (TRUE);
      break;

    case MOVE_FEATURE:
      misc.statusProgLabel->setText (tr ("Move feature mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (moveFeatureCursor);
      map->setMapMode (NVMAPGL_FEATURE_MODE);
      bMoveFeature->setChecked (TRUE);


      //  Have to unfreeze if we go to this mode.

      misc.marker_mode = 0;
      break;

    case DELETE_FEATURE:
      misc.statusProgLabel->setText (tr ("invalidate feature mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (deleteFeatureCursor);
      map->setMapMode (NVMAPGL_FEATURE_MODE);
      bDeleteFeature->setChecked (TRUE);


      //  Have to unfreeze if we go to this mode.

      misc.marker_mode = 0;
      break;

    case EDIT_FEATURE:
      misc.statusProgLabel->setText (tr ("Edit feature mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (addFeatureCursor);
      map->setMapMode (NVMAPGL_FEATURE_MODE);
      bEditFeature->setChecked (TRUE);


      //  Have to unfreeze if we go to this mode.

      misc.marker_mode = 0;
      break;

    case ADD_FEATURE:
      misc.statusProgLabel->setText (tr ("Add feature mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (addFeatureCursor);
      bAddFeature->setChecked (TRUE);


      //  Have to unfreeze if we go to this mode.

      misc.marker_mode = 0;


      //  Unlike the other FEATURE modes we don't set NVMAPGL_FEATURE_MODE for ADD_FEATURE because we will be
      //  searching for the nearest point so that we can attach a feature to it.


      break;

    case DELETE_POINT:
      misc.statusProgLabel->setText (tr ("Delete subrecord/record mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::PointingHandCursor);
      bDeletePoint->setChecked (TRUE);
      break;

    case UNSET_SINGLE:
      misc.statusProgLabel->setText (tr ("Select hide single file mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::ArrowCursor);


      //  Have to unfreeze if we go to this mode.

      misc.marker_mode = 0;
      break;

    case SET_MULTIPLE:
      misc.statusProgLabel->setText (tr ("Select multiple files mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::ArrowCursor);


      //  Have to unfreeze if we go to this mode.

      misc.marker_mode = 0;
      break;

    case ROTATE:
      misc.statusProgLabel->setText (tr ("Rotate data"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (rotateCursor);
      if (misc.need_sparse) map->setMapMode (NVMAPGL_SPARSE_MODE);
      break;

    case ZOOM:
      misc.statusProgLabel->setText (tr ("Zoom in/out"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (zoomCursor);
      if (misc.need_sparse) map->setMapMode (NVMAPGL_SPARSE_MODE);
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

      misc.marker_mode = 0;
      break;

    case MASK_OUTSIDE_RECTANGLE:
      misc.statusProgLabel->setText (tr ("Mask outside rectangle mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (maskCursor);

      misc.marker_mode = 0;
      break;

    case MASK_INSIDE_POLYGON:
      misc.statusProgLabel->setText (tr ("Mask inside polygon mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (maskCursor);

      misc.marker_mode = 0;
      break;

    case MASK_OUTSIDE_POLYGON:
      misc.statusProgLabel->setText (tr ("Mask outside polygon mode"));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (maskCursor);

      misc.marker_mode = 0;
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
pfmEdit3D::discardMovableObjects ()
{
  map->closeMovingList (&mv_tracker);
  trackMap->closeMovingPath (&mv_2D_tracker);
  map->closeMovingList (&mv_measure_anchor);
  map->closeMovingList (&mv_measure);
  map->discardRubberbandRectangle (&rb_rectangle);
  map->discardRubberbandPolygon (&rb_polygon);
  map->discardRubberbandLine (&rb_measure);
  trackMap->closeMovingRectangle (&misc.avb.av_rectangle);
  map->discardRubberbandLine (&rb_dist);


  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++) map->closeMovingList (&(multi_marker[i]));
}



void 
pfmEdit3D::leftMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z)
{
  NV_INT32 *px, *py;

  void get_feature_event_time (MISC *misc);


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
    case AV_DISTANCE_TOOL:

      // if the line is active then this must be the second press

      if (map->rubberbandLineIsActive (rb_dist))
        {
          map->discardRubberbandLine (&rb_dist);

          int pointsWithin = ComputeLineDistanceFilter (distLineStart, distLineEnd, &misc, &options);

          // we will set up some variables to let us overlay data seamlessly for the shots
          // that passed the Distance Threshold test

          if (pointsWithin > 0)
            {
              trackMap->redrawMap (NVTrue);
              redrawMap (NVTrue);
            }
        }
		

      // the line wasn't active so this is the first mouse press

      else
        {
          NV_INT32 pix_x, pix_y;
          map->get2DCoords (lon, lat, z, &pix_x, &pix_y);
          map->anchorRubberbandLine (&rb_dist, pix_x, pix_y, Qt::yellow, 2, Qt::SolidLine);			

          NV_FLOAT64 x = lon * NV_DEG_TO_RAD;
          NV_FLOAT64 y = lat * NV_DEG_TO_RAD;

          pj_transform (misc.pj_latlon, misc.pj_utm, 1, 1, &x, &y, NULL);

          distLineStart.x = x;
          distLineStart.y = y;
        }
      break;

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

          set_area (map, &options, &misc, px, py, 4, 1);

          qApp->restoreOverrideCursor ();


          map->discardRubberbandRectangle (&rb_rectangle);


          if (options.function == MASK_INSIDE_RECTANGLE || options.function == MASK_OUTSIDE_RECTANGLE) misc.mask_active = NVTrue;

          if (options.function == RECT_FILTER_MASK)
            {
              misc.filter_mask = NVTrue;


              //  If the filter message box is up then we are filter masking after the filter so we want to discard the current filter points
              //  then filter again.

              if (filter_active) slotFilter ();
              if (attr_filter_active) slotAttrFilter ();
              if (hof_filter_active) slotHOFWaveFilter ();
            }


          redrawMap (NVTrue);


          //  In the event of a deletion that hits any frozen markers on the screen, we will unfreeze the markers.
          //  Otherwise, they will stay frozen and rooted

          if (misc.marker_mode)
            {
              if (misc.data[misc.frozen_point].val & PFM_MANUALLY_INVAL)
                {
                  misc.marker_mode = 0;
                }
              else if (misc.abe_share->mwShare.multiMode == 0 && misc.marker_mode == 2)
                {
                  for (int i = 1; i < MAX_STACK_POINTS; i++)
                    {
                      if (misc.data[misc.abe_share->mwShare.multiLocation[i]].val & PFM_MANUALLY_INVAL)
                        {
                          misc.marker_mode = 0;
                          break;
                        }
                    }
                }						  

              slotMouseMove((QMouseEvent *)NULL, misc.data[misc.frozen_point].x, misc.data[misc.frozen_point].y, misc.data[misc.frozen_point].z, mapdef);
            }


          //  Even though we do this in redrawMap we need to do it again here because otherwise the damn markers disappear.
          //  It's OpenGL, I have no idea why.

          if (options.function == RECT_FILTER_MASK) overlayData (map, &options, &misc);


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
    case SET_REFERENCE:
    case UNSET_REFERENCE:
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
            case SET_REFERENCE:
            case UNSET_REFERENCE:
            case HIGHLIGHT_POLYGON:
            case CLEAR_POLYGON:

              set_area (map, &options, &misc, px, py, count, 0);


              //  In the event of a deletion that hits any frozen markers on the screen, we will unfreeze the markers.
              //  Otherwise, they will stay frozen and rooted

              if (misc.marker_mode)
                {
                  if (misc.data[misc.frozen_point].val & PFM_MANUALLY_INVAL)
                    {
                      misc.marker_mode = 0;
                    }
                  else if (misc.abe_share->mwShare.multiMode == 0 && misc.marker_mode == 2)
                    {
                      for (int i = 1; i < MAX_STACK_POINTS; i++)
                        {
                          if (misc.data[misc.abe_share->mwShare.multiLocation[i]].val & PFM_MANUALLY_INVAL)
                            {
                              misc.marker_mode = 0;
                              break;
                            }
                        }
                    }
                  slotMouseMove ((QMouseEvent *)NULL, misc.data[misc.frozen_point].x, misc.data[misc.frozen_point].y, misc.data[misc.frozen_point].z, mapdef);
                }


              if (options.function == CLEAR_POLYGON || options.function == HIGHLIGHT_POLYGON)
                {
                  //  Since we decided to manually clear or set some points let's make sure that we weren't highlighting by flag.

                  options.flag_index = 0;
                  bFlag->setIcon (flagIcon[options.flag_index]);
                  disconnect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));
                  flag[0]->setChecked (TRUE);
                  connect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));
                }

              break;

            case POLY_FILTER_MASK:
              set_area (map, &options, &misc, px, py, count, 0);
              misc.filter_mask = NVTrue;


              //  If the filter message box is up then we are filter masking after the filter so we want to discard the current filter points
              //  then filter again.

              if (filter_active) slotFilter ();
              if (attr_filter_active) slotAttrFilter ();
              if (hof_filter_active) slotHOFWaveFilter ();
              break;

            case MASK_INSIDE_POLYGON:
            case MASK_OUTSIDE_POLYGON:
              set_area (map, &options, &misc, px, py, count, 0);
              misc.mask_active = NVTrue;
              break;

            case KEEP_POLYGON:
              keep_area (map, &options, &misc, px, py, count);
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

              if (options.function == POLY_FILTER_MASK) overlayData (map, &options, &misc);
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
          map->get2DCoords (misc.data[misc.nearest_point].x, misc.data[misc.nearest_point].y, -misc.data[misc.nearest_point].z, &pix_x, &pix_y);

          map->anchorRubberbandLine (&rb_measure, pix_x, pix_y, options.edit_color, LINE_WIDTH, Qt::SolidLine);
          line_anchor.x = misc.data[misc.nearest_point].x;
          line_anchor.y = misc.data[misc.nearest_point].y;
          line_anchor.z = misc.data[misc.nearest_point].z;
        }
      break;


    case ADD_FEATURE:

      //  Add feature at nearest_point

      NV_F64_COORD3 xyz;
      xyz.x = misc.data[misc.nearest_point].x;
      xyz.y = misc.data[misc.nearest_point].y;
      xyz.z = misc.data[misc.nearest_point].z;

      misc.add_feature_index = misc.nearest_point;


      memset (&misc.new_record, 0, sizeof (BFDATA_RECORD));

      get_feature_event_time (&misc);

      misc.new_record.record_number = misc.bfd_header.number_of_records;
      misc.new_record.length = 0.0;
      misc.new_record.width = 0.0;
      misc.new_record.height = 0.0;
      misc.new_record.confidence_level = 3;
      misc.new_record.depth = (NV_FLOAT32) misc.data[misc.nearest_point].z;
      misc.new_record.horizontal_orientation = 0.0;
      misc.new_record.vertical_orientation = 0.0;
      strcpy (misc.new_record.description, "");
      strcpy (misc.new_record.remarks, "");
      misc.new_record.latitude = misc.data[misc.nearest_point].y;
      misc.new_record.longitude = misc.data[misc.nearest_point].x;
      strcpy (misc.new_record.analyst_activity, "NAVOCEANO BHY");
      misc.new_record.equip_type = 3;
      misc.new_record.nav_system = 1;
      misc.new_record.platform_type = 4;
      misc.new_record.sonar_type = 3;

      misc.new_record.poly_count = 0;

      editFeatureNum (-1);

      misc.feature_mod = NVTrue;

      break;


    case EDIT_FEATURE:
      if (misc.nearest_feature != -1) editFeatureNum (misc.nearest_feature);
      break;


    case DELETE_FEATURE:
      if (misc.nearest_feature != -1)
        {
          //  Invalidate feature at nearest_feature

          BFDATA_RECORD bfd_record;
          if (binaryFeatureData_read_record (misc.bfd_handle, misc.nearest_feature, &bfd_record) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (this, tr ("Invalidate Feature"), tr ("Unable to read feature record\nReason: ") + msg);
              break;
            }


          //  Zero out the confidence value

          bfd_record.confidence_level = misc.feature[misc.nearest_feature].confidence_level = 0;


          if (binaryFeatureData_write_record (misc.bfd_handle, misc.nearest_feature, &bfd_record, NULL, NULL) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (this, tr ("Invalidate Feature"), tr ("Unable to update feature record\nReason: ") + msg);
              break;
            }

          misc.feature_mod = NVTrue;


          options.function = misc.save_function;
          setFunctionCursor (options.function);
          redrawMap (NVFalse);


          //  If the mosaic viewer was running, tell it to redraw.

          misc.abe_share->key = FEATURE_FILE_MODIFIED;
        }
      break;


    case MOVE_FEATURE:
      if (move_feature)
        {
          if (misc.nearest_feature != -1)
            {
	      //  Try to unset the PFM_SELECTED_FEATURE flag on what should have been the last feature data point.

	      if (misc.nearest_feature_point != -1) misc.data[misc.nearest_feature_point].val &= ~PFM_SELECTED_FEATURE;


              //  Move the nearest_feature to the nearest_point;

              BFDATA_RECORD bfd_record;
              BFDATA_POLYGON bfd_polygon;

              binaryFeatureData_read_record (misc.bfd_handle, misc.feature[misc.nearest_feature].record_number, &bfd_record);

              if (bfd_record.poly_count) binaryFeatureData_read_polygon (misc.bfd_handle, misc.feature[misc.nearest_feature].record_number,
                                                           &bfd_polygon);

              bfd_record.latitude = misc.feature[misc.nearest_feature].latitude = misc.data[misc.nearest_point].y;
              bfd_record.longitude = misc.feature[misc.nearest_feature].longitude = misc.data[misc.nearest_point].x;
              bfd_record.depth = misc.feature[misc.nearest_feature].depth = misc.data[misc.nearest_point].z;

              binaryFeatureData_write_record (misc.bfd_handle, misc.feature[misc.nearest_feature].record_number, &bfd_record, &bfd_polygon, NULL);

              misc.feature_mod = NVTrue;


	      //  Set PFM_SELECTED_FEATURE on the nearest point to the feature.

	      misc.data[misc.nearest_point].val |= PFM_SELECTED_FEATURE;
	      misc.add_feature_index = -1;


              options.function = misc.save_function;
              move_feature = 0;
              setFunctionCursor (options.function);
              redrawMap (NVFalse);
            }
        }
      else
        {
          //  Now we need to snap to points, not features.

          map->setMapMode (NVMAPGL_POINT_MODE);

	  misc.nearest_feature_point = misc.nearest_point;
          move_feature++;
        }
      break;


    case UNSET_SINGLE:
      if (misc.nearest_point != -1)
        {
          if (!misc.num_lines)
            {
              misc.num_lines = misc.line_count - 1;

              for (NV_INT32 i = 0, j = 0 ; i < misc.line_count ; i++)
                {
                  if (misc.data[misc.nearest_point].line != misc.line_number[i])
                    {
                      misc.line_num[j] = misc.line_number[i];
                      j++;
                    }
                }
            }
          else
            {
              for (NV_INT32 i = 0, j = 0 ; i < misc.num_lines ; i++)
                {
                  if (misc.data[misc.nearest_point].line != misc.line_num[i])
                    {
                      misc.line_num[j] = misc.line_num[i];
                      j++;
                    }
                }
              misc.num_lines--;
            }

          bDisplayAll->setEnabled (TRUE);

          redrawMap (NVFalse);
        }
      break;


    case SET_MULTIPLE:
      if (misc.nearest_point != -1)
        {
          local_line_num[local_num_lines] = misc.data[misc.nearest_point].line;
          local_num_lines++;

          bDisplayMultiple->setEnabled (FALSE);
          bDisplayAll->setEnabled (TRUE);
        }
      break;
    }
}



void 
pfmEdit3D::midMouse (NV_INT32 mouse_x __attribute__ ((unused)), NV_INT32 mouse_y __attribute__ ((unused)),
                     NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)),
                     NV_FLOAT64 z __attribute__ ((unused)))
{
  switch (options.function)
    {
    case DELETE_POINT:
      if (misc.nearest_point != -1)
        {
          NV_U_INT32 rec = misc.data[misc.nearest_point].rec;

          for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
            {
              if (misc.data[i].rec == rec)
                {
                  store_undo (&misc, options.undo_levels, misc.data[i].val, i);
                  misc.data[i].val |= PFM_MANUALLY_INVAL;
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
    case SET_REFERENCE:
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

    case ADD_FEATURE:
    case EDIT_FEATURE:
    case DELETE_FEATURE:
    case UNSET_SINGLE:
      options.function = misc.save_function;
      break;

    case MOVE_FEATURE:
      move_feature = 0;
      options.function = misc.save_function;
      break;

    case SET_MULTIPLE:
      bDisplayMultiple->setEnabled (TRUE);
      options.function = misc.save_function;
      misc.num_lines = local_num_lines = 0;
      break;
    }
  setFunctionCursor (options.function);
}



void 
pfmEdit3D::slotMouseDoubleClick (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon __attribute__ ((unused)),
                                 NV_FLOAT64 lat __attribute__ ((unused)), NV_FLOAT64 z __attribute__ ((unused)))
{
  if (misc.busy) return;


  //  Lock in nearest point so mouse moves won't change it if we want to run one of the ancillary programs.

  lock_point = misc.nearest_point;


  //  Flip the double_click flag.  The right-click menu sets this to NVTrue so it will flip to NVFalse.
  //  Left-click sets it to NVFalse so it will filp to NVTrue;

  double_click = !double_click;


  switch (options.function)
    {
    case SET_MULTIPLE:

      if (misc.nearest_point != -1)
        {
          local_line_num[local_num_lines] = misc.data[misc.nearest_point].line;
          local_num_lines++;

          bDisplayMultiple->setEnabled (FALSE);
          bDisplayAll->setEnabled (TRUE);
        }

      for (NV_INT32 i = 0 ; i < local_num_lines ; i++) misc.line_num[i] = local_line_num[i];
      misc.num_lines = local_num_lines;
      redrawMap (NVTrue);
      options.function = misc.save_function;
      setFunctionCursor (options.function);
      break;

    default:

      //  Double click is hard-wired to turn off slicing except in a few cases.

      if (misc.slice) slotPopupMenu (popup[NUMPOPUPS - 1]);
      break;
    }
}



//!  Freakin' menus!

void 
pfmEdit3D::rightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z __attribute__ ((unused)))
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
      // provide a mechanism to get out of AV Distance Tool */

    case AV_DISTANCE_TOOL:
      popup[0]->setText (tr ("Change Distance Threshold"));
      popup[1]->setText (tr ("Exit AV Distance Tool"));

      popup_active = NVTrue;
      popupMenu->popup (global_pos);

      break;


    case DELETE_POINT:

      //  If you move the "Delete point" option from popup[0] look through the code for calls to slotPopupMenu with popup[0] as the argument.

      tmp = tr ("Delete point [%1]").arg (options.hotkey[DELETE_SINGLE_POINT]);
      popup[0]->setText (tmp);
      popup[1]->setText (tr ("Delete record (e.g. ping)"));
      popup[2]->setText (tr ("Delete file"));
      popup[2]->setVisible (TRUE);
      popup[3]->setText (tr ("Highlight point"));
      popup[3]->setVisible (TRUE);
      popup[4]->setText (tr ("Highlight record"));
      popup[4]->setVisible (TRUE);


      //  Don't allow point marking if we have filter points or av_dist points.

      if (!misc.filter_kill_count && !misc.av_dist_count)
        {
          popup[3]->setEnabled (TRUE);
          popup[4]->setEnabled (TRUE);
        }
      else
        {
          popup[3]->setEnabled (FALSE);
          popup[4]->setEnabled (FALSE);
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


      //  Check to see if the attribute viewer is running.

      if (misc.abe_share->avShare.avRunning)
        {
          popup[6]->setText (tr ("Change the Attribute Viewer box size"));
          popup[6]->setVisible (TRUE);
          popup[7]->setText (tr ("Attribute Viewer Distance Threshold Tool"));
          popup[7]->setVisible (TRUE);
          popup[7]->setEnabled (TRUE);
        }



      //  Check for the data type of the nearest point and add the possible ancillary programs

      switch (misc.data[misc.nearest_point].type)
        {
        case PFM_GSF_DATA:
          popup[8]->setText (tr ("Run ") + options.name[EXAMGSF] + " [" + options.hotkey[EXAMGSF] + "]");
          popup[8]->setVisible (TRUE);
          popup_prog[8] = EXAMGSF;
          break;

        case PFM_UNISIPS_DEPTH_DATA:
          popup[8]->setText (tr ("Run ") + options.name[UNISIPS] + " [" + options.hotkey[UNISIPS] + "]");
          popup[8]->setVisible (TRUE);
          popup_prog[8] = UNISIPS;
          break;
        }

      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DELETE_RECTANGLE:
      popup[0]->setText (tr ("Close rectangle and delete subrecords (beams)"));
      popup[1]->setText (tr ("Close rectangle and delete records (pings)"));
      popup[2]->setText (tr ("Discard rectangle"));
      popup[2]->setVisible (TRUE);
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
      popup[2]->setVisible (TRUE);
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

    case SET_REFERENCE:
      popup[0]->setText (tr ("Close polygon and set reference data flag"));
      popup[1]->setText (tr ("Discard polygon"));
      popup[2]->setText (tr ("Delete all reference soundings"));
      popup[2]->setVisible (TRUE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case UNSET_REFERENCE:
      popup[0]->setText (tr ("Close polygon and unset reference data flag"));
      popup[1]->setText (tr ("Discard polygon"));
      popup[2]->setText (tr ("Delete all reference soundings"));
      popup[2]->setVisible (TRUE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case HOTKEY_POLYGON:


      //  If we have a defined hotkey polygon, check for the data types in the polygon and activate the entries for
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


          //  We have to convert to NV_FLOAT64 so that the "inside" function will work.

          NV_FLOAT64 *mx = (NV_FLOAT64 *) malloc (misc.hotkey_poly_count * sizeof (NV_FLOAT64));

          if (mx == NULL)
            {
              perror ("Allocating mx array in pfmEdit3D");
              exit (-1);
            }

          NV_FLOAT64 *my = (NV_FLOAT64 *) malloc (misc.hotkey_poly_count * sizeof (NV_FLOAT64));

          if (my == NULL)
            {
              perror ("Allocating my array in pfmEdit3D");
              exit (-1);
            }

          for (NV_INT32 i = 0 ; i < misc.hotkey_poly_count ; i++)
            {
              mx[i] = (NV_FLOAT64) misc.hotkey_poly_x[i];
              my[i] = (NV_FLOAT64) misc.hotkey_poly_y[i];
            }

          NV_INT32 hits = 0, px, py;
          for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
            {
              //  Check for single line display.

              if (!misc.num_lines || check_line (&misc, misc.data[i].line))
                {
                  //  Do not use null points.  Do not use invalid points unless the display_man_invalid, display_flt_invalid, or
                  //  display_null flag is set.  Do not use masked points. Do not check points that are not on the display.

                  if (!check_bounds (&options, &misc, i, NVTrue, misc.slice))
                    {
                      //  Convert the X, Y, and Z value to a projected pixel position

                      map->get2DCoords (misc.data[i].x, misc.data[i].y, -misc.data[i].z, &px, &py);


                      //  Now check the point against the polygon.

                      if (inside_polygon2 (mx, my, misc.hotkey_poly_count, (NV_FLOAT64) px, (NV_FLOAT64) py))
                        {
                          if (misc.data[i].type == PFM_SHOALS_1K_DATA) hits |= 0x01;
                          if (misc.data[i].type == PFM_SHOALS_TOF_DATA) hits |= 0x02;
                          if (misc.data[i].type == PFM_CHARTS_HOF_DATA) hits |= 0x04;
                        }

                      if (hits == 0x07) break;
                    }
                }
            }


          //  Check for features if present

          if (options.display_feature && misc.visible_feature_count)
            {
              for (NV_U_INT32 i = 0 ; i < misc.bfd_header.number_of_records ; i++)
                {
                  if (misc.feature[i].confidence_level)
                    {
                      //  Convert the X, Y, and Z value to a projected pixel position

                      map->get2DCoords (misc.feature[i].longitude, misc.feature[i].latitude, -misc.feature[i].depth, &px, &py);


                      //  Now check the point against the polygon.

                      if (inside_polygon2 (mx, my, misc.hotkey_poly_count, (NV_FLOAT64) px, (NV_FLOAT64) py))
                        {
                          hits |= 0x8;
                          break;
                        }
                    }
                }
            }


          free (mx);
          free (my);


          //  Old CHARTS HOF format (saved for possible re-use)

          if (hits & 0x01)
            {
            }


          //  CHARTS TOF format (saved for possible re-use)

          if (hits & 0x02)
            {
            }


          //  CHARTS HOF format

          if (hits & 0x04)
            {
              popup[10]->setText (tr ("Run ") + options.name[HOFRETURNKILL] + " [" + options.hotkey[HOFRETURNKILL] + "]");
              popup[10]->setVisible (TRUE);
              popup_prog[10] = HOFRETURNKILL;
              popup[11]->setText (tr ("Run ") + options.name[HOFRETURNKILL_SWA] + " [" + options.hotkey[HOFRETURNKILL_SWA] + "]");
              popup[11]->setVisible (TRUE);
              popup_prog[11] = HOFRETURNKILL_SWA;
            }


          //  Feature format

          if (hits & 0x08)
            {
              popup[12]->setText (tr ("Invalidate features"));
              popup[12]->setVisible (TRUE);
              popup_prog[12] = INVALIDATE_FEATURES;
            }
        }
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case UNSET_SINGLE:
      popup[0]->setText (tr ("Hide line"));
      popup[1]->setText (tr ("Leave hide single line mode"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case SET_MULTIPLE:
      popup[0]->setText (tr ("Accept selected lines"));
      popup[1]->setText (tr ("Leave select multiple line mode"));
      popup[2]->setText (tr ("Select multiple lines from list"));
      popup[2]->setVisible (TRUE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case MOVE_FEATURE:
      popup[0]->setText (tr ("Move feature"));
      popup[1]->setText (tr ("Leave move feature mode"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DELETE_FEATURE:
      popup[0]->setText (tr ("invalidate feature"));
      popup[1]->setText (tr ("Leave invalidate feature mode"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case EDIT_FEATURE:
      popup[0]->setText (tr ("Edit feature"));
      popup[1]->setText (tr ("Leave edit feature mode"));
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case ADD_FEATURE:
      popup[0]->setText (tr ("Add feature"));
      popup[1]->setText (tr ("Leave add feature mode"));
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



/*!
  IMPORTANT NOTE:  Never use misc.nearest_point in slotPopupMenu.  Use lock_point instead.  This is set when we right-click,
  double-click, or press a key so that subsequent mouse movement will not affect what we were trying to do when we clicked or pressed.
*/

void 
pfmEdit3D::slotPopupMenu (QAction *ac)
{
  NV_INT32 value, ret;
  QStringList lst, items;
  QString res_str, item, boxSize;
  bool ok;


  void kill_records (nvMapGL *map, OPTIONS *options, MISC *misc, NV_INT32 *rb, NV_INT32 x, NV_INT32 y);


  //  Nice little debug statement for the next time I add a popup slot and forget to change this code.

  if (NUMPOPUPS > 15) fprintf (stderr, "%s %d - Someone tell Jan to fix this function because NUMPOPUPS has changed\n", __FILE__,__LINE__);


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
        case DELETE_POINT:
          if (lock_point != -1)
            {
              //
              //                      if we are frozen, automatically unfreeze
              //

              if (misc.marker_mode) misc.marker_mode = 0;


              //  Single point kills are a real waste of undo ;-)

              store_undo (&misc, options.undo_levels, misc.data[lock_point].val, lock_point);
              misc.data[lock_point].val = PFM_MANUALLY_INVAL;
              end_undo_block (&misc);

              redrawMap (NVTrue);
            }
          break;

        case SET_MULTIPLE:
          slotMouseDoubleClick (NULL, 0.0, 0.0, 0.0);
          break;

        case DELETE_RECTANGLE:
        case RESTORE_RECTANGLE:
        case UNSET_SINGLE:
        case MOVE_FEATURE:
        case DELETE_FEATURE:
        case EDIT_FEATURE:
        case ADD_FEATURE:
        case DELETE_POLYGON:
        case RESTORE_POLYGON:
        case KEEP_POLYGON:
        case SET_REFERENCE:
        case UNSET_REFERENCE:
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

        case AV_DISTANCE_TOOL:
          NV_FLOAT32 savedDistThresh = options.distThresh;


          // to keep the highighted points up during the dialog box

          redrawMap (NVTrue);	


          // we stop the timer because we don't want to flush the map and lose our highlights
          // while the user is deciding on changing the distance threshold.  afterwards, we
          // restart the timer.

          trackCursor->stop ();

          options.distThresh = QInputDialog::getDouble (this, tr ("pfmEdit"), tr ("Distance Threshold"), options.distThresh, 0.5);

          if (options.distThresh != savedDistThresh)
            {
              int pointsWithin = ComputeLineDistanceFilter (distLineStart, distLineEnd, &misc, &options);


              // we will set up some variables to let us overlay data seamlessly for the shots that passed the Distance Threshold test
			
              if (pointsWithin > 0) trackMap->redrawMap (NVTrue);
            }

          redrawMap (NVTrue);

          trackCursor->start (50);

          break;
        }
    }
  else if (ac == popup[1])
    {
      switch (options.function)
        {
        case DELETE_RECTANGLE:
          kill_records (map, &options, &misc, &rb_rectangle, menu_cursor_x, menu_cursor_y);

          redrawMap (NVTrue);

          setFunctionCursor (options.function);
          break;

        case DELETE_POLYGON:
          kill_records (map, &options, &misc, &rb_polygon, menu_cursor_x, menu_cursor_y);

          redrawMap (NVTrue);

          setFunctionCursor (options.function);
          break;

        case AV_DISTANCE_TOOL:
          distLineStart.x = distLineStart.y = -1.0f;
          distLineEnd.x = distLineEnd.y = -1.0f;
          misc.avInput = AV_AOI;
          slotMode (DELETE_POINT);
          bDistanceThresh->setChecked (FALSE);

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
        case DELETE_RECTANGLE:
        case DELETE_POLYGON:
          midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat, menu_cursor_z);
          break;

        case RECT_FILTER_MASK:
        case POLY_FILTER_MASK:
          slotClearFilterMasks ();
          break;


          //  Add files to the delete file queue.

        case DELETE_POINT:
          if (lock_point != -1)
            {
              NV_CHAR fn[512];
              NV_INT16 type;
              read_list_file (misc.pfm_handle[misc.data[lock_point].pfm], misc.data[lock_point].file, fn, &type);

              res_str = tr ("Add file %1 in PFM %2 to the delete file queue?").arg (pfm_basename (fn)).arg 
                (pfm_basename (misc.abe_share->open_args[misc.data[lock_point].pfm].list_path));
              ret = QMessageBox::information (this, tr ("pfmEdit3D Delete File Queue"), res_str, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

              if (ret == QMessageBox::Yes)
                {
                  //  No point in listing a file twice so check for it first.

                  ok = NVTrue;
                  for (NV_INT32 i = 0 ; i < misc.abe_share->delete_file_queue_count ; i++)
                    {
                      if (misc.abe_share->delete_file_queue[i][0] == misc.data[lock_point].pfm &&
                          misc.abe_share->delete_file_queue[i][1] == misc.data[lock_point].file)
                        {
                          ok = NVFalse;
                          break;
                        }
                    }

                  if (ok)
                    {
                      misc.abe_share->delete_file_queue[misc.abe_share->delete_file_queue_count][0] = misc.data[lock_point].pfm;
                      misc.abe_share->delete_file_queue[misc.abe_share->delete_file_queue_count][1] = misc.data[lock_point].file;
                      misc.abe_share->delete_file_queue_count++;
                    }
                }
            }
          break;

        case SET_REFERENCE:
        case UNSET_REFERENCE:
          ret = QMessageBox::information (this, tr ("pfmEdit3D"), tr ("Do you really want to delete all visible reference soundings?"),
                                          QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

          if (ret == QMessageBox::Yes)
            {
              for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
                {
                  //  Check for single line display.

                  if (!misc.num_lines || check_line (&misc, misc.data[i].line))
                    {
                      //  Since you can't access this option without display_reference being set we don't have to
                      //  worry about check_bounds not passing reference points.

                      if (!check_bounds (&options, &misc, i, NVTrue, misc.slice))
                        {
                          store_undo (&misc, options.undo_levels, misc.data[i].val, i);
                          if (misc.data[i].val & PFM_REFERENCE) misc.data[i].val |= PFM_MANUALLY_INVAL;
                        }
                    }
                }
              end_undo_block (&misc);

              redrawMap (NVFalse);
            }
          break;

        case SET_MULTIPLE:
          displayLines_dialog = new displayLines (this, &misc);

          connect (displayLines_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotDisplayLinesDataChanged ()));
          break;
        }
    }
  else if (ac == popup[3])
    {
      switch (options.function)
        {
        case DELETE_POINT:
          misc.highlight = (NV_INT32 *) realloc (misc.highlight, (misc.highlight_count + 1) * sizeof (NV_INT32));
          if (misc.highlight == NULL)
            {
              perror ("Allocating misc.highlight in pfmEdit3D.cpp");
              exit (-1);
            }

          misc.highlight[misc.highlight_count] = lock_point;
          misc.highlight_count++;

          bClearHighlight->setEnabled (TRUE);

          redrawMap (NVFalse);
          break;

        case RECT_FILTER_MASK:
        case POLY_FILTER_MASK:
            value = 0;
            if (!options.deep_filter_only) value = 1;

            lst << tr ("Yes") << tr ("No");

            res_str = QInputDialog::getItem (this, tr ("pfmEdit3D"), tr ("Deep filter only:"), lst, value, FALSE, &ok);

            if (ok)
              {
                if (res_str.contains (tr ("Yes")))
                  {
                    options.deep_filter_only = NVTrue;
                  }
                else
                  {
                    options.deep_filter_only = NVFalse;
                  }
              }
          break;
        }
    }
  else if (ac == popup[4])
    {
      switch (options.function)
        {
        case DELETE_POINT:

          if (misc.highlight_count)
            {
              free (misc.highlight);
              misc.highlight = NULL;
              misc.highlight_count = 0;
            }

          for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
            {
              if (!(misc.data[i].val & (PFM_DELETED | PFM_REFERENCE | PFM_INVAL)))
                {
                  if (!check_bounds (&options, &misc, i, NVTrue, misc.slice))
                    {
                      if (misc.data[i].pfm == misc.data[lock_point].pfm && misc.data[i].file == misc.data[lock_point].file &&
                          misc.data[i].rec == misc.data[lock_point].rec)
                        {
                          misc.highlight = (NV_INT32 *) realloc (misc.highlight, (misc.highlight_count + 1) * sizeof (NV_INT32));
                          if (misc.highlight == NULL)
                            {
                              perror ("Allocating misc.highlight in pfmEdit3D.cpp");
                              exit (-1);
                            }

                          misc.highlight[misc.highlight_count] = i;
                          misc.highlight_count++;
                        }
                    }
                }
            }

          bClearHighlight->setEnabled (TRUE);

          redrawMap (NVFalse);
          break;

        case RECT_FILTER_MASK:
        case POLY_FILTER_MASK:
          break;
        }
    }
  else if (ac == popup[5])
    {
      switch (options.function)
        {
          //  This slot is called when the Freeze/Unfreeze action has been clicked. It will set the proper text for the right
          //  context menu based on whether we are in freeze mode or not.  If we are unfreezing, we'll set the context menu
          //  text to be Freeze Point and close the focus cursor moving path for the overplot as it will be handled in
          //  slotMouseMove.  If we are freezing, we put the Unfreeze Point text on the context menu and indicate that the focus
          //  cursor is present as a legitimate multi-waveform

        case DELETE_POINT:

          //  Save the marker position just in case we are freezing the marker.

          misc.frozen_point = lock_point;


          if (misc.marker_mode)
            {
              //  Just in case CZMILwaveMonitor is running.

              misc.abe_share->mwShare.multiPresent[0] = -1;

              misc.marker_mode = 0;
            }
          else
            {
              //  Just in case CZMILwaveMonitor is running.

              misc.abe_share->mwShare.multiPresent[0] = 0;

              misc.marker_mode = 1;
            }
          break;
        }
    }
  else if (ac == popup[6])
    {
      switch (options.function)
        {
        case DELETE_POINT:
          items << tr ("9m x 9m") << tr ("25m x 25m") << tr ("50m x 50m");

          item = QInputDialog::getItem (this, tr ("pfmEdit3D"), tr("Attribute Viewer box size"), items, options.avInterfaceBoxSize, FALSE, &ok);

          if (ok && !item.isEmpty())
            {
              if (item == tr ("9m x 9m"))
                {
                  options.avInterfaceBoxSize = 0;
                }
              else if (item == tr ("25m x 25m"))
                {
                  options.avInterfaceBoxSize = 1;
                }
              else
                {
                  options.avInterfaceBoxSize = 2;
                }


              //  Only perform if the AV is running

              if (misc.abe_share->avShare.avRunning) slotAVInterfaceChanged (options.avInterfaceBoxSize);
            }
          break;
        }
    }
  else if (ac == popup[7])
    {
      switch (options.function)
        {
        case DELETE_POINT:
          misc.avInput = AV_DIST_THRESH;
          slotMode (AV_DISTANCE_TOOL);
          bDistanceThresh->setChecked (TRUE);
          break;
        }
    }
  else if (ac == popup[8])
    {
      runPopupProg (8);
    }
  else if (ac == popup[9])
    {
      runPopupProg (9);
    }
  else if (ac == popup[10])
    {
      runPopupProg (10);
    }
  else if (ac == popup[11])
    {
      runPopupProg (11);
    }
  else if (ac == popup[12])
    {
      runPopupProg (12);
    }
  else if (ac == popup[13])
    {
      runPopupProg (13);
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
pfmEdit3D::slotPopupHelp ()
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

    case UNSET_SINGLE:
      QWhatsThis::showText (QCursor::pos ( ), undisplaySingleText, map);
      break;

    case SET_MULTIPLE:
      QWhatsThis::showText (QCursor::pos ( ), displayMultipleText, map);
      break;

    case MOVE_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), moveFeatureText, map);
      break;

    case DELETE_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), deleteFeatureText, map);
      break;

    case EDIT_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), editFeatureText, map);
      break;

    case ADD_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), addFeatureText, map);
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

    case SET_REFERENCE:
      QWhatsThis::showText (QCursor::pos ( ), referencePolyText, map);
      break;

    case UNSET_REFERENCE:
      QWhatsThis::showText (QCursor::pos ( ), unreferencePolyText, map);
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
pfmEdit3D::runPopupProg (NV_INT32 prog)
{
  if (options.function == HOTKEY_POLYGON && misc.hotkey_poly_count)
    {
      NV_BOOL failed = NVFalse;
      hkp = new hotkeyPolygon (this, map, &options, &misc, popup_prog[prog], &failed);
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



/*!
  Timer - timeout signal.  Very much like an X workproc.  This tracks the cursor in associated
  programs, not in this window.  This is active whenever the mouse leaves this window.
*/

void
pfmEdit3D::slotTrackCursor ()
{
  NV_CHAR               ltstring[25], lnstring[25];
  QString               string;
  static NV_F64_COORD2  prev_xy;
  static NV_INT32       prevMultiMode = misc.abe_share->mwShare.multiMode;


  if (misc.busy) return;


  //  When Single is toggled to NN or vice versa in Waveform Viewer, remove markers and flush the map so no lingering markers hang around.
  //  This is hard to control due to the XOR logic of the markers and in certain instances, markers would hang around

  if (prevMultiMode != misc.abe_share->mwShare.multiMode)
    {
      prevMultiMode = misc.abe_share->mwShare.multiMode;

      discardMovableObjects ();	  


      //  If we are frozen, the markers whether single or NN should be displayed instantly

      if (misc.marker_mode)
        {		  
          //  Although we are not performing a delete action, we want to make sure that the slotMouseMove method does not return due to
          //  the same mouse position since in effect the cursor could be constantly toggling from the WV.  Therefore, we fake it through
          //  slotMouseMove by pretending we have a delete action even though we do not.

          misc.performingAction = AVA_DELETE;
          slotMouseMove((QMouseEvent *) NULL, misc.data[misc.frozen_point].x, misc.data[misc.frozen_point].y, misc.data[misc.frozen_point].z, mapdef);
          misc.performingAction = AVA_NONE;
        }

      map->flush ();
    }


  if (misc.abe_share->avShare.avRunning) checkForAVAction ();


  //
  //                      if we've closed the CZMILwaveMonitor module and we were not in single waveform mode,
  //                      close the moving paths of all the multiple waveforms and switch to single waveform
  //                      mode so we will only display our lone cursor in pfmEdit3D.
  //

  if (!misc.abe_share->mwShare.waveMonitorRunning && !misc.abe_share->mwShare.multiMode)
    {
      for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++) map->closeMovingList (&(multi_marker[i]));
      misc.abe_share->mwShare.multiMode = 1;
    }


  //  If we have changed the feature file contents in another program, we need to draw the features.

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
                  QMessageBox::warning (this, tr ("pfmEdit3D"), tr ("Unable to read feature records\nReason: ") + msg);
                  binaryFeatureData_close_file (misc.bfd_handle);
                }
              else
                {
                  misc.bfd_open = NVTrue;
                }
            }
        }


      redrawMap (NVFalse);


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

  if (misc.abe_share->active_window_id != misc.process_id && xy.y >= misc.displayed_area.min_y && xy.y <= misc.displayed_area.max_y && 
      xy.x >= misc.displayed_area.min_x && xy.x <= misc.displayed_area.max_x && xy.y != prev_xy.y && xy.x != prev_xy.x)
    {
      //  Find the nearest point to the cursor's location in whatever window has the focus.

      NV_FLOAT64 min_dist = 999999999.0;
      NV_INT32 hit = -1;
      for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
        {
          NV_FLOAT64 dist = sqrt (((xy.y - misc.data[i].y) * (xy.y - misc.data[i].y)) + ((xy.x - misc.data[i].x) * (xy.x - misc.data[i].x)));

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
          map->get2DCoords (misc.data[hit].x, misc.data[hit].y, -misc.data[hit].z, &pix_x, &pix_y);


          //  This particular situation caused a lingering of shot markers so a clearing of markers is called here.  This is a weird situation.

          if (misc.abe_share->active_window_id != misc.process_id - 1) discardMovableObjects ();


          map->setMovingList (&mv_tracker, marker, 16, pix_x, pix_y, 0.0, 2, options.tracker_color);


          //  Then set the 2D tracker.

          QBrush b1;
          trackMap->setMovingPath (&mv_2D_tracker, tracker_2D, misc.data[hit].x, misc.data[hit].y, 2, options.tracker_color,
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



//!  Blink timer slot.

void
pfmEdit3D::slotBlinkTimer ()
{
  static NV_INT32    blink = 0, lblink = 0;


  //  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.  We also want to exit if
  //  pfmView has set the PFMEDIT_KILL switch.

  if (misc.abe_share->key == CHILD_PROCESS_FORCE_EXIT || misc.abe_share->key == PFMEDIT_KILL) slotExit (2);


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


  //  Blink the displayAll button if we're not displaying all lines.

  if (misc.num_lines)
    {
      if (lblink)
        {
          bDisplayAll->setIcon (QIcon (":/icons/displayall.xpm"));
          lblink = 0;
        }
      else
        {
          bDisplayAll->setIcon (QIcon (":/icons/displayall_inverse.xpm"));
          lblink = 1;
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


              //  Special case for attributeViewer because we have to get rid of the tracking box.

              if (options.progButton[i] == bAttributeViewer)
                {
                  trackMap->closeMovingRectangle (&misc.avb.av_rectangle);

                  bDistanceThresh->setChecked (FALSE);
                  bDistanceThresh->setEnabled (FALSE);
                }

              misc.abe_share->killed = 0;
            }
        }
    }


  //  Another silly timer thing.  If the filter slider move counter has exceeded 1 (i.e. 1 to 1.5 seconds since last Shift+wheel move),
  //  we want to trigger the filter and reset the counter to -1.

  if ((filter_active || attr_filter_active || hof_filter_active) && filterMessage_slider_count >= 0) filterMessage_slider_count++;

  if ((filter_active || attr_filter_active || hof_filter_active) && filterMessage_slider_count > 1)
    {
      filterMessage_slider_count = -1;

      if (filter_active) slotFilter ();
      if (attr_filter_active) slotAttrFilter ();
      if (hof_filter_active) slotHOFWaveFilter ();
    }
}



//!  Mouse press signal from the map class.

void 
pfmEdit3D::slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z)
{
  if (misc.busy) return;


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
              if (misc.nearest_point != -1)
                {
                  //  Check for the shift key modifier.  If it's set, we want to delete the point.

                  if (e->modifiers () == Qt::ShiftModifier)
                    {
                      //  If we are frozen, automatically unfreeze

                      if (misc.marker_mode) misc.marker_mode = 0;

                      store_undo (&misc, options.undo_levels, misc.data[misc.nearest_point].val, misc.nearest_point);
                      misc.data[misc.nearest_point].val = PFM_MANUALLY_INVAL;
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

          overlayData (map, &options, &misc);
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



//!  Mouse release signal from the map class.

void 
pfmEdit3D::slotMouseRelease (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)),
                           NV_FLOAT64 lat __attribute__ ((unused)), NV_FLOAT64 z __attribute__ ((unused)))
{
  if (misc.busy) return;


  if (e->button () == Qt::LeftButton)
    {
      if (options.function == ROTATE)
        {
          options.function = misc.save_function;
          setFunctionCursor (options.function);


          //  All we have to do is flush here since setFunctionCursor will set the map mode back to NVMAPGL_POINT_MODE and
          //  we haven't actually changed the point data.

          if (misc.need_sparse) map->flush ();

          overlayData (map, &options, &misc);
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


          //  All we have to do is flush here since setFunctionCursor will set the map mode back to NVMAPGL_POINT_MODE and
          //  we haven't actually changed the point data.

          if (misc.need_sparse) map->flush ();

          overlayData (map, &options, &misc);
        }
    }
}



void 
pfmEdit3D::geo_xyz_label (NV_FLOAT64 lat, NV_FLOAT64 lon, NV_FLOAT64 z)
{
  NV_CHAR ltstring[25], lnstring[25], hem;
  QString val_string;
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
      zPalette.setColor (QPalette::Active, QPalette::Window, misc.widgetBackgroundColor);
      zLabel->setPalette (zPalette);
      zLabel->setToolTip (tr ("Current point Z value"));
      zLabel->setWhatsThis (tr ("Current point Z value"));
    }
  zLabel->setText (val_string);

  prev_z_factor = options.z_factor;
  prev_z_offset = options.z_offset;
}



//!  Mouse wheel signal from the map class.

void
pfmEdit3D::slotWheel (QWheelEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)), NV_FLOAT64 z __attribute__ ((unused)))
{
  if (e->delta () > 0)
    {
      //  Zoom in or out when pressing the Ctrl key and using the mouse wheel.

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->zoomInPercent ();

          overlayData (map, &options, &misc);
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

          overlayData (map, &options, &misc);
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



//!  Mouse move signal from the map class.

void
pfmEdit3D::slotMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z, NVMAPGL_DEF l_mapdef)
{
  NV_I32_COORD2             xy;
  NV_F64_COORD3             hot = {999.0, 999.0, 999.0}, hot_feature = {999.0, 999.0, 999.0};
  static NV_I32_COORD2      prev_xy = {-1, -1};
  static NV_F64_COORD3      prev_hot;
  NV_FLOAT64                dist, min_dist;
  static NV_INT32           prev_nearest_point = -1;
  static NV_INT32           save_nearest_point;


  void get_nearest_kill_point (MISC *misc, NV_FLOAT64 lat, NV_FLOAT64 lon, NV_F64_COORD3 *hot);
  NV_BOOL compare_to_stack (NV_INT32 current_point, NV_FLOAT64 dist, MISC *misc);


  if (misc.marker_mode) save_nearest_point = misc.nearest_point;


  if (misc.busy) return;


  //  Let other ABE programs know which window we're in.

  misc.abe_share->active_window_id = misc.process_id;


  //  Get rid of the tracking cursor from slotTrackCursor ().  But only if it already exists, otherwise we
  //  will be creating a new one (which we don't want to do).

  if (mv_tracker >= 0) map->closeMovingList (&mv_tracker);
  if (mv_2D_tracker >= 0) trackMap->closeMovingPath (&mv_2D_tracker);


  //  I believe because of the XOR logic of the markers, the NN markers were very problematic and not clearing out properly within nvMapGL
  //  in certain situations.  ONLY IN THE CASE OF Nearest Neighbors are we reflushing the map every time.  Until someone further delves into
  //  the proper erasing of these markers, we will refresh the scene to ensure that proper erasing is happening.  I spent hours looking at this and still
  //  can't understand why they do not erase properly.  If you bring up nearest neighbors and that's it, they will work fine.  When you start freezing
  //  and linking with the Attribute Viewer which calls slotMouseMove indirectly with simulated mouse movements, every now and then, markers will be
  //  duplicated and stuck on the screen.  There are a couple of different situations that have to line up just right for this to happen
  //  and I have not pinpointed them just yet.  Therefore, we will be flushing until a better solution is available.  (Gary Morris)

  if (misc.abe_share->mwShare.waveMonitorRunning && !misc.abe_share->mwShare.multiMode) map->flush ();


  //  In case slot invoked manually with null qmouseevent use the x and y of the prev nearest_point

  if (e)
    {
      xy.x = e->x ();
      xy.y = e->y ();
    }
  else
    {
      //  Highlighting uses this too so we will go with the incoming arguments of lon and lat

      map->get2DCoords (lon, lat, -z, &xy.x, &xy.y);
    }


  //  Track the cursor (not the marker) position for other ABE programs.

  misc.abe_share->cursor_position.y = lat;
  misc.abe_share->cursor_position.x = lon;
  misc.abe_share->cursor_position.z = z;


  //  Only process if the pointer position has changed pixels and we haven't frozen all of the markers.

  if (xy.x == prev_xy.x && xy.y == prev_xy.y && misc.performingAction != AVA_DELETE) return;


  //  If we've frozen the central cursor or all of the cursors we want to draw them regardless of the mode we're in (except when we're
  //  using the AV Distance Threshold tool).

  if (misc.marker_mode && options.function != AV_DISTANCE_TOOL)
    {
      //  If we're in multicursor mode, draw all the cursors.

      if (misc.marker_mode == 2)
        {
          if (!misc.abe_share->mwShare.multiMode) DrawMultiCursors ();
        }
      else
        { 
          NV_INT32 pix_x, pix_y;
          map->get2DCoords (misc.data[misc.frozen_point].x, misc.data[misc.frozen_point].y, -misc.data[misc.frozen_point].z, &pix_x, &pix_y);

          map->setMovingList (&(multi_marker[0]), marker, 16, pix_x, pix_y, 0.0, 2, misc.abe_share->mwShare.multiColors[0]);
        }
    }


  QString rec_string = "", sub_string = "", fil_string = "", pfm_string = "", lin_string = "", distance_string = "", hv_string = "";
  NV_FLOAT64 distance[2], azimuth[2];
  NV_CHAR nearest_line[512];


  switch (options.function)
    {
    case ROTATE:
      if (start_ctrl_x != xy.x || start_ctrl_y != xy.y)
	{
	  NV_INT32 diff_x = xy.x - start_ctrl_x;

          //  This will remove all markers from the screen during a rotate

          discardMovableObjects();


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

          //  This will remove all markers from the screen during a zoom

          discardMovableObjects ();


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

    case DELETE_POINT:
    case UNSET_SINGLE:
    case SET_MULTIPLE:
    case ADD_FEATURE:
    case EDIT_FEATURE:
    case MOVE_FEATURE:
    case DELETE_FEATURE:
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
              map->get2DCoords (misc.data[misc.frozen_point].x, misc.data[misc.frozen_point].y, -misc.data[misc.frozen_point].z, &pix_x, &pix_y);

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


          //  Nearest point is already known for a delete action

          if (misc.performingAction != AVA_DELETE) misc.nearest_point = -1;


          if (!move_feature) misc.nearest_feature = -1;
          hot_feature.x = -1;


          //  If we've got filter kill points on screen we want to find the nearest kill point first and lock it in before finding
          //  the nearest stack points.  This allows us to snap to only kill points but then get the surrounding valid points
          //  whether they're kill points or not.  This will set the nearest_point, the hot.x and hot.y positions, and the stack[0]
          //  point and distance (forced to -999.0 so it can't be replaced).

          if (misc.filter_kill_count)
            {
              //  We have to get the nearest marker point (i.e. highlighted) since the mouseMove signal returns the nearest of 
              //  all of the displayed points (unless they're sliced out).
              //  IMPORTANT NOTE: We're replacing the lat, lon, and z values passed from the nvMapGL mouseMoveSignal because we
              //  need for the exteranl programs (e.g. waveWaterfall) to find the nearest geographic points to the filter
              //  highlighted point not the point closest to the cursor.

              map->getMarker3DCoords (e->x (), e->y (), &lon, &lat, &z);

              get_nearest_kill_point (&misc, lat, lon, &hot);
            }


          //  Find the valid point or feature nearest (geographically) to the cursor.

          //  Working with existing features instead of points.

          if ((options.function == EDIT_FEATURE || options.function == DELETE_FEATURE || (options.function == MOVE_FEATURE && !move_feature)) &&
              options.display_feature && misc.bfd_header.number_of_records)
            {
              min_dist = 999999999.0;
              for (NV_U_INT32 i = 0 ; i < misc.bfd_header.number_of_records ; i++)
                {
                  if (options.display_man_invalid || options.display_flt_invalid || misc.feature[i].confidence_level)
                    {
                      if (!check_bounds (&options, &misc, misc.feature[i].longitude, misc.feature[i].latitude, misc.feature[i].depth, PFM_USER,
                                         NVFalse, 0, NVFalse, misc.slice))
                        {
                          if (options.display_children || !misc.feature[i].parent_record)
                            {
                              dist = sqrt ((NV_FLOAT64) ((lat - misc.feature[i].latitude) * (lat - misc.feature[i].latitude)) + 
                                           (NV_FLOAT64) ((lon - misc.feature[i].longitude) * (lon - misc.feature[i].longitude)));

                              if (dist < min_dist)
                                {
                                  misc.nearest_feature = i;
                                  hot_feature.x = misc.feature[i].longitude;
                                  hot_feature.y = misc.feature[i].latitude;
                                  hot_feature.z = misc.feature[i].depth;
                                  min_dist = dist;
                                }
                            }
                        }
                    }
                }
            }


          for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
            {
              //  Check for single line display.

              if (!misc.num_lines || check_line (&misc, misc.data[i].line))
                {
                  //  Do not use null points.  Do not use invalid points unless the display_man_invalid, display_flt_invalid, or
                  //  display_null flag is set.  Do not use masked points. Do not check points that are not on the display.

                  if (!check_bounds (&options, &misc, i, NVTrue, misc.slice))
                    {
                      //  Find the nearest point.

                      dist = sqrt ((NV_FLOAT64) ((lat - misc.data[i].y) * (lat - misc.data[i].y)) +
                                   (NV_FLOAT64) ((lon - misc.data[i].x) * (lon - misc.data[i].x)));


                      //  Check the points against the points in the nearest points stack.

                      if (compare_to_stack (i, dist, &misc))
                        {
                          //  If the return was true then this is the minimum distance so far.

                          //  Only want to set this if we are running through slotMouseMove legitimately if we are performing an
                          //  AVA_DELETE action, we already send over the point we want the cursor to go to.  We know the preceding
                          //  calculations are irrelevant but we are just playing it safe.

                          if (misc.performingAction != AVA_DELETE) misc.nearest_point = misc.nearest_stack.point[0];

                          hot.x = misc.data[i].x;
                          hot.y = misc.data[i].y;
                          hot.z = misc.data[i].z;
                        }
                    }
                }
            }


          //  Only track the nearest position if we haven't frozen the marker.

          if (!misc.marker_mode)
            {
              misc.abe_share->cursor_position.y = misc.data[misc.nearest_point].y;
              misc.abe_share->cursor_position.x = misc.data[misc.nearest_point].x;
              misc.abe_share->cursor_position.z = misc.data[misc.nearest_point].z;
            }


          //  Update the status bars

          // 
          //                      if our point has moved or we have just selected a new overplot we will go
          //                      into this code block.  In the case of deleting a point from AV we want to make sure
          //                      this runs
          //

          if (misc.nearest_point != -1 && (misc.nearest_point != prev_nearest_point || misc.performingAction == AVA_DELETE || options.function == MEASURE))
            {
              //  Show distance and azimuth to last highlighted point in status bar

              if (misc.highlight_count)
                {
                  invgp (NV_A0, NV_B0, misc.data[misc.highlight[misc.highlight_count - 1]].y, misc.data[misc.highlight[misc.highlight_count - 1]].x,
                         misc.data[misc.nearest_point].y, misc.data[misc.nearest_point].x, &distance[0], &azimuth[0]);

                  if (distance[0] < 0.01) azimuth[0] = 0.0;

                  distance_string.sprintf ("Distance : %.2f (m)  Azimuth : %.2f (degrees)", distance[0], azimuth[0]);

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


              //  External programs monitor abe_share->nearest_point to trigger updates of their windows.  They use
              //  abe_share->mwShare.multiRecord[0] for the current record to display.  In this way we can lock the
              //  main screen by not updating abe_share->mwShare.multiRecord[0] and still trigger the external programs
              //  to update (such as for multiple waveforms in CZMILwaveMonitor or waveWaterfall).

              NV_INT16 type;
              read_list_file (misc.pfm_handle[misc.data[misc.nearest_point].pfm], misc.data[misc.nearest_point].file, misc.abe_share->nearest_filename, &type);

              strcpy (nearest_line, read_line_file (misc.pfm_handle[misc.data[misc.nearest_point].pfm], misc.data[misc.nearest_point].line % SHARED_LINE_MULT));


              //  Push the nearest stack points into the ABE shared memory for use by CZMILwaveMonitor, waveWaterfall,
              //  and lidarMonitor.

              for (NV_INT32 j = 0 ; j < MAX_STACK_POINTS ; j++)
                {
                  //  Normal, unfrozen mode.

                  if (!misc.marker_mode)
                    {
                      misc.abe_share->mwShare.multiSubrecord[j] = misc.data[misc.nearest_stack.point[j]].sub;
                      misc.abe_share->mwShare.multiPfm[j] = misc.data[misc.nearest_stack.point[j]].pfm;
                      misc.abe_share->mwShare.multiFile[j] = misc.data[misc.nearest_stack.point[j]].file;
                      misc.abe_share->mwShare.multiLine[j] = misc.data[misc.nearest_stack.point[j]].line % SHARED_LINE_MULT;
                      misc.abe_share->mwShare.multiPoint[j].x = misc.data[misc.nearest_stack.point[j]].x;
                      misc.abe_share->mwShare.multiPoint[j].y = misc.data[misc.nearest_stack.point[j]].y;
                      misc.abe_share->mwShare.multiPoint[j].z = misc.data[misc.nearest_stack.point[j]].z;
                      misc.abe_share->mwShare.multiLocation[j] = misc.nearest_stack.point[j];
                      misc.abe_share->mwShare.multiType[j] = misc.data[misc.nearest_stack.point[j]].type;
                      misc.abe_share->mwShare.multiPresent[j] = misc.data[misc.nearest_stack.point[j]].rec;
                      misc.abe_share->mwShare.multiRecord[j] = misc.data[misc.nearest_stack.point[j]].rec;
                      misc.abe_share->mwShare.multiFlightlineIndex[j] = GetFlightlineColorIndex (misc.abe_share->mwShare.multiLine[j],
                                                                                                 misc.line_number, misc.line_count,
                                                                                                 misc.line_interval);
                    }


                  //  If we're frozen, we don't want to replace the [0] record and we want to push the nearest
                  //  MAX_STACK_POINTS - 1 records up one level.

                  else
                    {
                      if (j)
                        {
                          misc.abe_share->mwShare.multiSubrecord[j] = misc.data[misc.nearest_stack.point[j - 1]].sub;
                          misc.abe_share->mwShare.multiPfm[j] = misc.data[misc.nearest_stack.point[j - 1]].pfm;
                          misc.abe_share->mwShare.multiFile[j] = misc.data[misc.nearest_stack.point[j - 1]].file;
                          misc.abe_share->mwShare.multiLine[j] = misc.data[misc.nearest_stack.point[j - 1]].line % SHARED_LINE_MULT;
                          misc.abe_share->mwShare.multiPoint[j].x = misc.data[misc.nearest_stack.point[j - 1]].x;
                          misc.abe_share->mwShare.multiPoint[j].y = misc.data[misc.nearest_stack.point[j - 1]].y;
                          misc.abe_share->mwShare.multiPoint[j].z = misc.data[misc.nearest_stack.point[j - 1]].z;
                          misc.abe_share->mwShare.multiLocation[j] = misc.nearest_stack.point[j - 1];
                          misc.abe_share->mwShare.multiType[j] = misc.data[misc.nearest_stack.point[j - 1]].type;
                          misc.abe_share->mwShare.multiPresent[j] = misc.data[misc.nearest_stack.point[j - 1]].rec;
                          misc.abe_share->mwShare.multiRecord[j] = misc.data[misc.nearest_stack.point[j - 1]].rec;
                          misc.abe_share->mwShare.multiFlightlineIndex[j] = GetFlightlineColorIndex (misc.abe_share->mwShare.multiLine[j],
                                                                                                     misc.line_number, misc.line_count,
                                                                                                     misc.line_interval);
                        }
                    }
                }
              misc.abe_share->mwShare.multiNum = MAX_STACK_POINTS;


              //  Trigger some of the external applications to update.

              misc.abe_share->nearest_point = misc.nearest_point;


              //  Unlock shared memory.

              misc.abeShare->unlock ();


              //  If we're working with features display the position of the feature instead of the nearest point.

              if ((options.function == EDIT_FEATURE || options.function == DELETE_FEATURE ||
                   (options.function == MOVE_FEATURE && !move_feature)) && options.display_feature && 
                  misc.bfd_header.number_of_records)
                {
                  geo_xyz_label (misc.feature[misc.nearest_feature].latitude,  misc.feature[misc.nearest_feature].longitude,
                                 misc.feature[misc.nearest_feature].depth);
                }
              else
                {
                  geo_xyz_label (misc.data[misc.nearest_point].y,  misc.data[misc.nearest_point].x, misc.data[misc.nearest_point].z);
                }


              //  If we are frozen multiRecord[0] is useless to us, so we must check and use nearest point

              if (!misc.marker_mode)
                {
                  rec_string.sprintf ("%d", misc.abe_share->mwShare.multiRecord[0]);
                  sub_string.sprintf ("%d", misc.abe_share->mwShare.multiSubrecord[0]);

                  pfm_string = QFileInfo (QString (misc.abe_share->open_args[misc.abe_share->mwShare.multiPfm[0]].list_path)).fileName ().remove (".pfm");
                  pfmLabel->setToolTip (QString (misc.abe_share->open_args[misc.abe_share->mwShare.multiPfm[0]].list_path));
                }
              else
                {
                  rec_string.sprintf ("%d", misc.data[misc.nearest_point].rec);
                  sub_string.sprintf ("%d", misc.data[misc.nearest_point].sub);

                  pfm_string = QFileInfo (QString (misc.abe_share->open_args[misc.data[misc.nearest_point].pfm].list_path)).fileName().remove(".pfm");
                  pfmLabel->setToolTip (QString (misc.abe_share->open_args[misc.data[misc.nearest_point].pfm].list_path));
                }

              fil_string.sprintf ("%s", pfm_basename (misc.abe_share->nearest_filename));
              filLabel->setToolTip (QString (misc.abe_share->nearest_filename));
              lin_string.sprintf ("%s", nearest_line);


              recLabel->setText (rec_string);
              subLabel->setText (sub_string);
              filLabel->setText (fil_string);
              pfmLabel->setText (pfm_string);
              linLabel->setText (lin_string);
              typLabel->setText (PFM_data_type[misc.data[misc.nearest_point].type]);

              hv_string.sprintf ("%.2f / %.2f", misc.data[misc.nearest_point].herr, misc.data[misc.nearest_point].verr);
              hvLabel->setText (hv_string);

              NV_INT32 pfm = misc.data[misc.nearest_point].pfm;


              //  Check for the time, datum, and ellipsoid attributes from PFMWDB.  Embed the loop instead of the "if" so we only have
              //  to do one "if" statement instead of misc.max_attr "if" statements every time we move the mouse.

              if (PFMWDB_attr_count)
                {
                  for (NV_INT32 i = 0 ; i < misc.max_attr ; i++)
                    {
                      QString attr_string;

                      if (i == misc.time_attr)
                        {
                          if (misc.data[misc.nearest_point].attr[i] > PFMWDB_NULL_TIME)
                            {
                              NV_INT32 year, jday, hour, minute;
                              NV_FLOAT32 second;
                              time_t tv_sec = NINT (misc.data[misc.nearest_point].attr[i] * 60.0);

                              cvtime (tv_sec, 0, &year, &jday, &hour, &minute, &second);
                              attr_string.sprintf ("%d%03d%02d%02d", year + 1900, jday, hour, minute);
                            }
                          else
                            {
                              attr_string = "N/A";
                            }
                        }
                      else if (i == misc.datum_attr)
                        {
                          attr_string = shortVerticalDatum[NINT (misc.data[misc.nearest_point].attr[i])];
                        }
                      else if (i == misc.ellipsoid_attr)
                        {
                          if (misc.data[misc.nearest_point].attr[i] < PFMWDB_NULL_Z_OFFSET)
                            {
                              attr_string.sprintf (misc.attr_format[pfm][i], misc.data[misc.nearest_point].attr[i]);
                            }
                          else
                            {
                              attr_string = "N/A";
                            }
                        }
                      else
                        {
                          attr_string.sprintf (misc.attr_format[pfm][i], misc.data[misc.nearest_point].attr[i]);
                        }

                      attrLabel[i]->setText (attr_string);
                    }
                }
              else
                {
                  for (NV_INT32 i = 0 ; i < misc.max_attr ; i++)
                    {
                      QString attr_string;

                      attr_string.sprintf (misc.attr_format[pfm][i], misc.data[misc.nearest_point].attr[i]);

                      attrLabel[i]->setText (attr_string);
                    }
                }
            }


          //  If we're working with a feature set the correct hot position

          if (hot_feature.x != -1)
            {
              hot.x = hot_feature.x;
              hot.y = hot_feature.y;
              hot.z = hot_feature.z;
            }


          //  Set the delete, set, unset point marker.

          if (!misc.abe_share->mwShare.multiMode && (options.function == DELETE_POINT))
            {
              DrawMultiCursors ();
            }
          else
            {
              //  If we're frozen, use the frozen point for the marker position.

              if (misc.marker_mode)
                {
                  hot.x = misc.data[misc.frozen_point].x;
                  hot.y = misc.data[misc.frozen_point].y;
                  hot.z = misc.data[misc.frozen_point].z;
                }

              NV_INT32 pix_x, pix_y;
              map->get2DCoords (hot.x, hot.y, -hot.z, &pix_x, &pix_y);

              map->setMovingList (&(multi_marker[0]), marker, 16, pix_x, pix_y, 0.0, 2, misc.abe_share->mwShare.multiColors[0]);
            }


          if (options.function == MEASURE)
            {
              if (map->rubberbandLineIsActive (rb_measure))
                {
                  //  Snap the end of the line to the nearest point

                  NV_INT32 pix_x, pix_y;
                  map->get2DCoords (misc.data[misc.nearest_point].x, misc.data[misc.nearest_point].y, -misc.data[misc.nearest_point].z, &pix_x, &pix_y);

                  map->dragRubberbandLine (rb_measure, pix_x, pix_y);


                  invgp (NV_A0, NV_B0, misc.data[misc.nearest_point].y, misc.data[misc.nearest_point].x, line_anchor.y, line_anchor.x, &distance[0], &azimuth[0]);

                  if (distance[0] < 0.01) azimuth[0] = 0.0;


                  NV_F64_COORD3 coords;
                  map->getFaux3DCoords (line_anchor.x, line_anchor.y, -line_anchor.z, xy.x, xy.y, &coords);
                  invgp (NV_A0, NV_B0, coords.y, coords.x, line_anchor.y, line_anchor.x, &distance[1], &azimuth[1]);

                  if (distance[1] < 0.01) azimuth[1] = 0.0;

                  distance_string.sprintf ("Nearest Point Dist: %.2f  Az: %.2f  deltaZ: %.2f     Cursor Dist: %.2f  Az: %.2f  deltaZ: %.2f",
                                           distance[0], azimuth[0], line_anchor.z - misc.data[misc.nearest_point].z, distance[1], azimuth[1], 
                                           line_anchor.z - (-coords.z));

                  misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
                  misc.statusProgLabel->setPalette (misc.statusProgPalette);
                  misc.statusProgLabel->setText (distance_string);
                  misc.statusProgLabel->setVisible (TRUE);
                }
            }
        }


      //  This is the Attribute Viewer's functionality.
      //  Account for if the av interface box is frozen

      if (misc.abe_share->avShare.avRunning)
        {
          //  Do not draw interface box if in AV Distance Tool

          if (misc.avInput == AV_AOI)
            {
              //  It has to be a normal mouse movement and not simulated

              if (!misc.marker_mode && misc.performingAction == AVA_NONE)
                {
                  DrawAVInterfaceBox (trackMap, &misc, misc.abe_share->cursor_position);
                }
              else
                {
                  DrawAVInterfaceBox (trackMap, &misc, misc.avb.avInterfaceCenterLatLon);
                }
            }

          geo_xyz_label (misc.abe_share->cursor_position.y,  misc.abe_share->cursor_position.x, -misc.abe_share->cursor_position.z);

          if (!misc.marker_mode)
            {
              //  It has to be a normal mouse movement and not simulated.
              //  Always come through this block if deleting because we must load up the new batch of shots to send to the AV.

              if ((misc.nearest_point != prev_nearest_point && misc.performingAction != AVA_HIGHLIGHT) || misc.performingAction == AVA_DELETE)
                {
                  //  Move DrawAVInterfaceBox out so in the event with a new slotPreRedraw call
                  //  with a frozen cursor, the update will occur.

                  if (misc.hydro_lidar_present && misc.abe_share->avShare.avRunning)
                    {
                      if (misc.performingAction == AVA_NONE)
                        {
                          NV_I32_COORD2 hot2;
                          NV_FLOAT64 dz;
                          NV_INT32 xyz_z;

                          trackMap->map_to_screen (1, &misc.abe_share->cursor_position.x, &misc.abe_share->cursor_position.y, &dz, &hot2.x, &hot2.y, &xyz_z);

                          misc.avb.avInterfaceCenterMousePt = hot2;
                          misc.avb.avInterfaceCenterLatLon = misc.abe_share->cursor_position;
                        }

                      if (misc.avInput == AV_AOI) LoadShotsWithinAVBox (&misc, &options, trackMap, misc.avb.avInterfaceCenterMousePt);
                    }
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

      geo_xyz_label (lat, lon, -z);
      filLabel->setText (fil_string);
      pfmLabel->setText (pfm_string);
      recLabel->setText (rec_string);
      subLabel->setText (sub_string);
      linLabel->setText (lin_string);
      typLabel->setText (PFM_data_type[misc.data[misc.nearest_point].type]);
      break;

    case DELETE_POLYGON:
    case RESTORE_POLYGON:
    case KEEP_POLYGON:
    case SET_REFERENCE:
    case UNSET_REFERENCE:
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

      geo_xyz_label (lat, lon, -z);
      filLabel->setText (fil_string);
      pfmLabel->setText (pfm_string);
      recLabel->setText (rec_string);
      subLabel->setText (sub_string);
      linLabel->setText (lin_string);
      typLabel->setText (PFM_data_type[misc.data[misc.nearest_point].type]);

      break;

    case AV_DISTANCE_TOOL:

      if (map->rubberbandLineIsActive (rb_dist))
        {

          NV_INT32 pix_x, pix_y;
          map->get2DCoords (lon, lat, z, &pix_x, &pix_y);

          map->dragRubberbandLine (rb_dist, pix_x, pix_y);

          NV_FLOAT64 x = lon * NV_DEG_TO_RAD;
          NV_FLOAT64 y = lat * NV_DEG_TO_RAD;

          pj_transform (misc.pj_latlon, misc.pj_utm, 1, 1, &x, &y, NULL);

          distLineEnd.x = x;
          distLineEnd.y = y;
        }

      break;
   }


  //  Track the cursor in the 2D tracker box.

  QBrush b1;
  trackMap->setMovingPath (&mv_2D_tracker, tracker_2D, misc.abe_share->cursor_position.x, misc.abe_share->cursor_position.y, 2, options.tracker_color, b1,
                           NVFalse, Qt::SolidLine);


  //  Set the previous cursor.

  prev_xy = xy;

  prev_nearest_point = misc.nearest_point;


  //  If we are frozen, restore the misc.nearest_point

  if (misc.marker_mode) misc.nearest_point = save_nearest_point;
}



/*!
  - DrawMultiCursors

  - This method will loop through the nearest neighbors, and place the multi-cursor on the proper shot
    whether we are in a normal top-down view or a sliced view.
*/

void 
pfmEdit3D::DrawMultiCursors ()
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



/*!
  Resize signal from the map class.  With Qt 4.6.2 this seems to be redundant (and can cause all kinds of problems) so
  this is just a stub at the moment.
*/

void
pfmEdit3D::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
#if QT_VERSION < 0x040602
  redrawMap (NVTrue);
#endif
}



//!  This is where we load all of our data in to OpenGL display lists (in nvMapGL.cpp).

void
pfmEdit3D::redrawMap (NV_BOOL redraw2D)
{
  static NV_BOOL startup = NVTrue;


  void setScale (NV_FLOAT32 actual_min, NV_FLOAT32 actual_max, NV_FLOAT32 attr_min, NV_FLOAT32 attr_max, MISC *misc, OPTIONS *options, NV_BOOL min_lock,
                 NV_BOOL max_lock);
  NV_BOOL compute_bounds (nvMapGL *map, OPTIONS *options, MISC *misc, NV_INT32 *viewable_count, NV_FLOAT32 *attr_min, NV_FLOAT32 *attr_max);
  //void scribe3D (nvMapGL *map, OPTIONS *options, MISC *misc, NV_FLOAT32 *ar, NV_INT32 ncc, NV_INT32 nrr, NV_FLOAT64 xorig, NV_FLOAT64 yorig);


  misc.busy = NVTrue;


  if (startup)
    {
      map->enableSignals ();
      trackMap->enableSignals ();
      startup = NVFalse;
    }


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  discardMovableObjects ();


  //  Erase all text from the map.

  map->clearText ();


  //  Erase all objects (like spheres) from the map.

  map->clearDisplayLists ();


  //  Erase all data points from the map.

  map->clearDataPoints ();


  //  Erase all contour lines from the map.  Contours on 3D maps look a bit weird so I'm commenting this out for now.

  //map->clearLines ();


  //  If we have more than sparse_limit points, erase all sparse points from the map.

  if (misc.need_sparse) map->clearSparsePoints ();


  setMainButtons (NVFalse);


  NVMAPGL_DEF l_mapdef = map->getMapdef ();


  misc.draw_area_height = l_mapdef.draw_height;
  misc.draw_area_width = l_mapdef.draw_width;

  map->setCursor (Qt::WaitCursor);

  misc.drawing_canceled = NVFalse;


  //  Compute the Z range of the data.

  NV_INT32 viewable_count;

  misc.bounds = mapdef.initial_bounds;
  misc.bounds.min_z = CHRTRNULL;
  misc.bounds.max_z = -CHRTRNULL;


  //  Compute the min and max values for defining the 3D space and optionally for coloring.

  NV_BOOL bounds_changed = compute_bounds (map, &options, &misc, &viewable_count, &misc.attr_min, &misc.attr_max);


  //  If we're not coloring by attribute and the min or max hsv lock is set we need to check to see if we want to use the locked value(s).

  misc.color_min_z = misc.bounds.min_z;
  misc.color_max_z = misc.bounds.max_z;
  misc.attr_color_min = misc.attr_min;
  misc.attr_color_max = misc.attr_max;

  NV_BOOL min_lock = NVFalse, max_lock = NVFalse;
  if (!options.color_index)
    {
      if (misc.abe_share->min_hsv_locked)
        {
          if (misc.color_max_z > -misc.abe_share->min_hsv_value)
            {
              misc.color_max_z = -misc.abe_share->min_hsv_value;
              max_lock = NVTrue;
            }
        }

      if (misc.abe_share->max_hsv_locked)
        {
          if (misc.color_min_z < -misc.abe_share->max_hsv_value)
            {
              misc.color_min_z = -misc.abe_share->max_hsv_value;
              min_lock = NVTrue;
            }
        }
    }
  else
    {
      NV_INT32 ndx = options.color_index;
      if (options.min_hsv_locked[ndx])
        {
          if (misc.attr_color_max > options.max_hsv_value[ndx])
            {
              misc.attr_color_max = options.max_hsv_value[ndx];
              max_lock = NVTrue;
            }
        }

      if (options.max_hsv_locked[ndx])
        {
          if (misc.attr_color_max > options.max_hsv_value[ndx])
            {
              misc.attr_color_max = options.max_hsv_value[ndx];
              min_lock = NVTrue;
            }
        }

      misc.attr_color_range = misc.attr_color_max - misc.attr_color_min;
    }

  misc.color_range_z = misc.color_max_z - misc.color_min_z;


  //  Only display if there are viewable points.

  if (viewable_count)
    {
      setScale (misc.bounds.min_z, misc.bounds.max_z, misc.attr_min, misc.attr_max, &misc, &options, min_lock, max_lock);


      /*  Contours on 3D maps look a bit weird so I'm commenting this out for now.

      NV_BOOL small_enough = NVTrue;
      NV_BOOL big_enough = NVTrue;


      NV_INT32 grid_height = NINT ((misc.displayed_area.max_y - misc.displayed_area.min_y) / misc.y_grid_size);
      NV_INT32 grid_width = NINT ((misc.displayed_area.max_x - misc.displayed_area.min_x) / misc.x_grid_size);


      //  Compute the adjusted min y and min x for the grid indexing.  This makes the grid an 
      //  exact number of grid spaces wide and high.

      NV_FLOAT64 adj_rymin = misc.displayed_area.max_y - ((NV_FLOAT32) grid_height * misc.y_grid_size);
      NV_FLOAT64 adj_rxmin = misc.displayed_area.max_x - ((NV_FLOAT32) grid_width * misc.x_grid_size);


      //  If the area to be binned is too big or small, flag it.

      if (grid_width * grid_height > 1000000) small_enough = NVFalse;
      if (grid_width * grid_height <= 0) big_enough = NVFalse;


      NV_FLOAT32 *grid = NULL;
      NV_U_INT16 *count = NULL;
      NV_INT32 length;

      if (options.display_contours && small_enough && big_enough)
        {
          grid = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_height * grid_width);

          if (grid == NULL)
            {
              perror ("Allocating grid memory : ");
              exit (-1);
            }


          count = (NV_U_INT16 *) malloc (sizeof (NV_U_INT16) * grid_height * grid_width);

          if (count == NULL)
            {
              perror ("Allocating count memory : ");
              exit (-1);
            }

          for (NV_INT32 i = 0 ; i < grid_height * grid_width ; i++)
            {
              count[i] = 0;
              grid[i] = CHRTRNULL;
            }
        }
      */


      range_x = misc.bounds.max_x - misc.bounds.min_x;
      range_y = misc.bounds.max_y - misc.bounds.min_y;
      range_z = misc.bounds.max_z - misc.bounds.min_z;


      //  Add 10 percent to the X, Y, and Z bounds.

      misc.bounds.min_x -= (range_x * 0.10);
      misc.bounds.max_x += (range_x * 0.10);

      misc.bounds.min_y -= (range_y * 0.10);
      misc.bounds.max_y += (range_y * 0.10);

      if (range_z == 0.0)
        {
          misc.bounds.min_z -= 1.0;
          misc.bounds.max_z += 1.0;
          range_z = misc.bounds.max_z - misc.bounds.min_z;
        }
      else
        {
          misc.bounds.min_z -= (range_z * 0.10);
          misc.bounds.max_z += (range_z * 0.10);
        }


      if (bounds_changed) map->setBounds (misc.aspect_bounds);


      if (!misc.slice) compute_ortho_values (map, &misc, &options, sliceBar, NVTrue);


      NV_INT32 c_index = -1;
      NV_INT32 prev_xcoord = -1;
      NV_INT32 prev_ycoord = -1;
      NV_INT32 min_z_index = -1;
      NV_INT32 max_z_index = -1;
      NV_FLOAT32 min_z = 999999999.0;
      NV_FLOAT32 max_z = -999999999.0;
      NV_U_BYTE save_min[4], save_max[4];
      NV_BOOL null_point;
      NV_FLOAT32 z_value;
      NV_FLOAT32 numshades = (NV_FLOAT32) (NUMSHADES - 1);


      //  If the scale is changed for an attribute we want to update it

      misc.avb.actualMin = misc.color_min_z;
      misc.avb.actualRange = misc.color_range_z;
      misc.avb.attrMin = misc.attr_color_min;
      misc.avb.attrRange = misc.attr_color_range;


      //  Check for display invalid and flag invalid

      if (options.flag_index == 5 && (options.display_man_invalid || options.display_flt_invalid))
        setFlags (&misc, &options);


      for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
        {
          //  Check for single line display.

          if (!misc.num_lines || check_line (&misc, misc.data[i].line))
            {
              //  Check against the viewing area.

              NV_INT32 trans = 0;
              if ((trans = check_bounds (&options, &misc, i, NVTrue, misc.slice)) < 2)
                {
                  z_value = misc.data[i].z;
                  null_point = NVFalse;


                  //  Check for null point.  We want to place these on the bottom of the displayed cube.

                  if (misc.data[i].z >= misc.null_val[misc.data[i].pfm])
                    {
                      null_point = NVTrue;
                      z_value = -misc.bounds.min_z;
                    }


                  //  Use the edit color for reference and null data.

                  if ((misc.data[i].val & PFM_REFERENCE) || null_point)
                    {
                      c_index = -1;
                    }


                  //  Check for color by attribute

                  else
                    {
		      if (!options.color_index)
			{
                          //  Color by depth

                          c_index = NINT (numshades - ((-misc.data[i].z) - misc.color_min_z) / misc.color_range_z * numshades);
			}
		      else if (options.color_index == 1)
			{
                          //  Color by line

                          for (NV_INT32 j = 0 ; j < misc.line_count ; j++)
                            {
                              if (misc.data[i].line == misc.line_number[j])
                                {
                                  c_index = j * misc.line_interval;
                                  break;
                                }
                            }
			}
		      else if (options.color_index == 2)
			{
                          //  Color by horizontal uncertainty

                          c_index = NINT (numshades - (misc.data[i].herr - misc.attr_color_min) / misc.attr_color_range * numshades);
			}
		      else if (options.color_index == 3)
			{
                          //  Color by vertical uncertainty

                          c_index = NINT (numshades - (misc.data[i].verr - misc.attr_color_min) / misc.attr_color_range * numshades);
			}


		      //  Color by PFM attribute

		      else if (options.color_index >= PRE_ATTR && options.color_index < PRE_ATTR + 10)
			{
                          NV_INT32 ndx = options.color_index - PRE_ATTR;
                          c_index = NINT (numshades - (misc.data[i].attr[ndx] - misc.attr_color_min) / misc.attr_color_range * numshades);
                        }


                      //  Check for out of range data.

                      if (c_index < 0) c_index = 0;

		      if (options.color_index == 1)
                        {
                          if (c_index > (NUMSHADES * 2 - 1)) c_index = NUMSHADES * 2 - 1;
                        }
                      else
                        {
                          if (c_index > NUMSHADES - 1) c_index = NUMSHADES - 1;
                        }
                    }


                  NV_U_BYTE tmp[4];

                  if (c_index < 0)
                    {
                      tmp[0] = options.ref_color[trans].red ();
                      tmp[1] = options.ref_color[trans].green ();
                      tmp[2] = options.ref_color[trans].blue ();
                      tmp[3] = options.ref_color[trans].alpha ();
                    }
                  else
                    {
                      //  Use the line color array for color by line.

		      if (options.color_index == 1)
                        {
                          memcpy (tmp, misc.line_color_array[trans][c_index], 4);
                        }
                      else
                        {
                          memcpy (tmp, misc.color_array[trans][c_index], 4);
                        }
                    }


                  //  If we have more than sparse_limit points we want to load a sparse layer for rotating and zooming.

                  if (misc.need_sparse)
                    {
                      //  If we are slicing and we need sparse data for rotation and zoom, only display sparse points
                      //  outside of the slice.  If we aren't slicing, display them all.

                      if (!misc.slice || !trans) map->setDataPoints (misc.data[i].x, misc.data[i].y, -z_value, tmp, 0, NVFalse);


                      //  Get the min and max displayed Z values for each bin.

                      if (prev_xcoord != -1 && (misc.data[i].xcoord != prev_xcoord || misc.data[i].ycoord != prev_ycoord))
                        {
                          if (min_z_index >= 0)
                            {
                              map->setSparsePoints (misc.data[min_z_index].x, misc.data[min_z_index].y, -misc.data[min_z_index].z, save_min, 0, NVFalse);


                              //  If we're slicing and we need sparse data for zoom and rotate, only load points inside the slice.

                              if (misc.slice && trans) map->setDataPoints (misc.data[min_z_index].x, misc.data[min_z_index].y, -misc.data[min_z_index].z,
                                                                           tmp, 0, NVFalse);
                            }

                          if (max_z_index >= 0 && max_z_index != min_z_index)
                            {
                              map->setSparsePoints (misc.data[max_z_index].x, misc.data[max_z_index].y, -misc.data[max_z_index].z, save_max, 0, NVFalse);


                              //  If we're slicing and we need sparse data for zoom and rotate, only load points inside the slice.

                              if (misc.slice && trans) map->setDataPoints (misc.data[max_z_index].x, misc.data[max_z_index].y, -misc.data[max_z_index].z,
                                                                           tmp, 0, NVFalse);
                            }

                          min_z_index = max_z_index = -1;
                          min_z = 999999999.0;
                          max_z = -999999999.0;
                        }


                      //  Don't use null points to determine min and max values for the  bin.

                      if (!null_point && misc.data[i].z < min_z)
                        {
                          min_z = misc.data[i].z;
                          min_z_index = i;
                          memcpy (save_min, tmp, 4);
                        }

                      if (!null_point && misc.data[i].z > max_z)
                        {
                          max_z = misc.data[i].z;
                          max_z_index = i;
                          memcpy (save_max, tmp, 4);
                        }

                      prev_xcoord = misc.data[i].xcoord;
                      prev_ycoord = misc.data[i].ycoord;
                    }


                  //  We don't need sparse data for rotating or zooming so we set all of the data points.

                  else
                    {
                      map->setDataPoints (misc.data[i].x, misc.data[i].y, -z_value, tmp, 0, NVFalse);
                    }


                  /*  Contours on 3D maps look a bit weird so I'm commenting this out for now.

                  if (big_enough && small_enough && options.display_contours)
                    {
                      //  Load the grid (figure out which bin the point is in).

                      c_index = NINT ((NV_INT32) ((misc.data[i].y - adj_rymin) / misc.y_grid_size) * grid_width + (NV_INT32) ((misc.data[i].x - adj_rxmin) /
                                                                                                                                misc.x_grid_size));


                      //  Make sure there is no memory overrun.

                      if (c_index < grid_width * grid_height)
                        {
                          if (!count[c_index]) grid[c_index] = 0.0;
                          grid[c_index] = (grid[c_index] * (NV_FLOAT32) count[c_index] + (misc.data[i].z * options.z_factor + options.z_offset)) /
                            ((NV_FLOAT32) count[c_index] + 1.0);
                          count[c_index]++;
                        }
                    }
                  */
                }
            }
        }


      //  If we have more than sparse_limit points we want to load a sparse layer for rotating and zooming.

      if (misc.need_sparse)
        {
          if (min_z_index >= 0)
            {
              map->setSparsePoints (misc.data[min_z_index].x, misc.data[min_z_index].y, -misc.data[min_z_index].z, save_min, 0, NVFalse);

              if (misc.slice) map->setDataPoints (misc.data[min_z_index].x, misc.data[min_z_index].y, -misc.data[min_z_index].z, save_min, 0, NVFalse);

            }

          if (max_z_index >= 0 && max_z_index != min_z_index)
            {
              map->setSparsePoints (misc.data[max_z_index].x, misc.data[max_z_index].y, -misc.data[max_z_index].z, save_max, 0, NVFalse);

              if (misc.slice) map->setDataPoints (misc.data[max_z_index].x, misc.data[max_z_index].y, -misc.data[max_z_index].z, save_max, 0, NVFalse);
            }

          map->setSparsePoints (0.0, 0.0, 0.0, save_min, options.point_size, NVTrue);
        }


      map->setDataPoints (0.0, 0.0, 0.0, save_min, options.point_size, NVTrue);


      /*  Contours on 3D maps look a bit weird so I'm commenting this out for now.

      if (big_enough && small_enough && options.display_contours)
        {
          NV_FLOAT32 *spline_array_x_in = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_width);
          NV_FLOAT32 *spline_array_y_in = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_width);
          NV_FLOAT32 *spline_array_x_out = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_width);
          NV_FLOAT32 *spline_array_y_out = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_width);
          if (spline_array_x_in == NULL || spline_array_y_in == NULL ||
              spline_array_x_out == NULL || spline_array_y_out == NULL)
            {
              perror ("Allocating spline memory : ");
              exit (-1);
            }


          //  Fill the spline arrays

          NV_INT32 k;
          for (NV_INT32 i = 0 ; i < grid_height ; i++)
            {
              k = 0;
              for (NV_INT32 j = 0 ; j < grid_width ; j++)
                {
                  if (count[i * grid_width + j])
                    {
                      //  Make sure there is no memory overrun.

                      if (k < grid_width)
                        {
                          spline_array_x_in[k] = (NV_FLOAT32) j;
                          spline_array_y_in[k] = grid[i * grid_width + j];
                          k++;
                        }
                    }
                }


              //  Do not interpolate lines with fewer than 3 points so the slope doesn't go wacko.
                        
              if (k > 2)
                {
                  interpolate (1.0, k, 0.0, grid_width - 1, &length, spline_array_x_in, spline_array_y_in, spline_array_x_out, spline_array_y_out);


                  //  Set all points that do not have a 'real' neighbor within 3 bins to be CHRTRNULL.  
                  //  This way they won't be contoured.

                  NV_BOOL good_flag;
                  for (NV_INT32 j = 0 ; j < length ; j++)
                    {
                      good_flag = NVFalse;
                      for (NV_INT32 m = 0 ; m < k ; m++)
                        {
                          if (ABS (j - spline_array_x_in[m]) < 3)
                            {
                              good_flag = NVTrue;
                              break;
                            }
                        }

                      if (good_flag)
                        {
                          grid[i * grid_width + j] = spline_array_y_out[j];
                        }
                      else
                        {
                          grid[i * grid_width + j] = CHRTRNULL;
                        }
                    }
                }
            }


          //  Contour it

          scribe3D (map, &options, &misc, grid, grid_width, grid_height, adj_rxmin, adj_rymin);


          //  Free all of the memory.

          if (grid != NULL) free (grid);
          if (count != NULL) free (count);
          if (spline_array_x_in != NULL) free (spline_array_x_in);
          if (spline_array_y_in != NULL) free (spline_array_y_in);
          if (spline_array_x_out != NULL) free (spline_array_x_out);
          if (spline_array_y_out != NULL) free (spline_array_y_out);
        }
      */
    }


  //  Set the features.

  if (!misc.drawing_canceled)
    {
      overlayFlag (map, &options, &misc);
      overlayData (map, &options, &misc);
    }


  //  We want to flush the map just in case we didn't have any points to draw.

  map->flush ();


  //  Redraw the 2D tracker unless this redraw was caused by an operation that has no effect on the 2D map.

  if (redraw2D) trackMap->redrawMap (NVTrue);


  //  If we're not auto-scaling we need to turn it off.

  map->setAutoScale (options.auto_scale);


  misc.busy = NVFalse;

  setMainButtons (NVTrue);

  setFunctionCursor (options.function);

  qApp->restoreOverrideCursor ();
}



void 
pfmEdit3D::slotMaskReset ()
{
  //  Clear the "show" and "hide" buttons

  disconnect (showGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotShowMenu (QAction *)));
  for (NV_INT32 i = 0 ; i < 6 ; i++) showAct[i]->setChecked (FALSE);
  if (misc.unique_count > 1) for (NV_INT32 i = 0 ; i < misc.unique_count ; i++) showAct[i + 6]->setChecked (FALSE);
  connect (showGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotShowMenu (QAction *)));

  disconnect (hideGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotHideMenu (QAction *)));
  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      hideAct[i]->setChecked (FALSE);
      hideAct[i]->setEnabled (TRUE);
    }
  if (misc.unique_count > 1)
    {
      for (NV_INT32 i = 0 ; i < misc.unique_count ; i++)
        {
          hideAct[i + 6]->setChecked (FALSE);
          hideAct[i + 6]->setEnabled (TRUE);
        }
    }
  connect (hideGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotHideMenu (QAction *)));

  for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++) misc.data[i].mask = NVFalse;
  misc.mask_active = NVFalse;
  bMaskReset->setIcon (QIcon (":/icons/mask_reset.xpm"));


  //  Turn off slicing if it was on (this will also call redrawMap ()).

  slotPopupMenu (popup[NUMPOPUPS - 1]);


  //  If we are clearing the mask, recalculate the AV ROI box.  Only perform if the AV is running.
  //  Not sure about this one since I don't know if any bounds are being set like in the 2D mode.
  //  Won't hurt regardless

  if (misc.abe_share->avShare.avRunning && misc.avInput == AV_AOI) slotAVInterfaceChanged (options.avInterfaceBoxSize);
}



void 
pfmEdit3D::slotReset ()
{
  misc.poly_count = 0;

  misc.slice = NVFalse;

  misc.slice_min = misc.ortho_min;
  sliceBar->setValue (misc.ortho_min);


  //  Turn on depth testing in the map widget

  map->setDepthTest (NVTrue);


  discardMovableObjects ();


  map->setMapCenter (misc.map_center_x, misc.map_center_y, misc.map_center_z);


  map->resetZoom ();
  map->setYRotation (mapdef.initial_y_rotation);
  map->setZXRotation (mapdef.initial_zx_rotation);


  //  Always turn auto-scaling on whenever we reset.  If we're not auto-scaling it will be turned off again after
  //  we redraw the map.

  map->setAutoScale (NVTrue);


  redrawMap (NVTrue);
}



void 
pfmEdit3D::slotSliceTriggered (int action)
{
  if (!misc.busy)
    {
      // in the event that we came into a slice from the AV Distance Tool, we will change modes back to DELETE_POINT since a
      // distance tool does not make any sense within these types of views

      if (options.function == AV_DISTANCE_TOOL)
        {
          distLineStart.x = distLineStart.y = -1.0f;
          distLineEnd.x = distLineEnd.y = -1.0f;
	  slotMode (DELETE_POINT);	  
	  misc.avInput = AV_AOI;
	  trackMap->setCursor (Qt::ArrowCursor);
        }


      //  Have to compute the ortho values and set the slider to minimum before we slice for the first time.

      if (!misc.slice) compute_ortho_values (map, &misc, &options, sliceBar, NVTrue);


      map->setDepthTest (NVFalse);


      //  Note that we're setting the misc.slice flag to true after we set the slice location.  This is so we can check to
      //  see if this is the first slice.  Thereason being that we don't want to add to the start location if this is the
      //  first slice.

      switch (action)
        {
        case QAbstractSlider::SliderSingleStepAdd:
        case QAbstractSlider::SliderPageStepAdd:
          if (misc.slice) misc.slice_min -= misc.slice_size;
          misc.slice_max = misc.slice_min + misc.slice_size;
          break;

        case QAbstractSlider::SliderSingleStepSub:
        case QAbstractSlider::SliderPageStepSub:
          if (misc.slice) misc.slice_max += misc.slice_size;
          misc.slice_min = misc.slice_max - misc.slice_size;
          break;

        case QAbstractSlider::SliderMove:
          if (misc.slice) misc.slice_min = sliceBar->value ();
          misc.slice_max = misc.slice_min + misc.slice_size;
          break;
        }


      if (misc.slice_min < misc.ortho_min)
        {
          misc.slice_min = misc.ortho_min;
          misc.slice_max = misc.ortho_min + misc.slice_size;
        }


      misc.slice = NVTrue;


      if (action != QAbstractSlider::SliderMove) sliceBar->setValue (misc.slice_min);

      slicing = NVTrue;
      redrawMap (NVFalse);
      slicing = NVFalse;


      //  Even though we do this in redrawMap we need to do it again here because otherwise the damn markers disappear.
      //  It's OpenGL, I have no idea why.

      overlayData (map, &options, &misc);
    }
}



void 
pfmEdit3D::slotSizeTriggered (int action)
{
  if (!misc.busy)
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

      compute_ortho_values (map, &misc, &options, sliceBar, NVFalse);
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

      overlayData (map, &options, &misc);
    }
}



void 
pfmEdit3D::slotTransTriggered (int action)
{
  if (!misc.busy)
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

      NV_FLOAT32 hue_inc = (NV_FLOAT32) (misc.abe_share->max_hsv_color - misc.abe_share->min_hsv_color) / (NV_FLOAT32) (NUMSHADES + 1);

      for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
	{
	  QColor color;
          color.setHsv ((NV_INT32) (misc.abe_share->min_hsv_color + j * hue_inc), 255, 255, 255); 
	  misc.color_array[0][j][0] = misc.color_array[1][j][0] = color.red ();
	  misc.color_array[0][j][1] = misc.color_array[1][j][1] = color.green ();
	  misc.color_array[0][j][2] = misc.color_array[1][j][2] = color.blue ();
	  misc.color_array[0][j][3] = color.alpha ();

	  misc.color_array[1][j][3] = options.slice_alpha;
	}
      options.ref_color[1] = options.ref_color[0];
      options.ref_color[1].setAlpha (options.slice_alpha);


      for (NV_INT32 j = 0 ; j < NUMSHADES * 2 ; j++) misc.line_color_array[1][j][3] = options.slice_alpha;


      QString string;
      string.sprintf ("%02d", options.slice_alpha);
      transLabel->setText (string);


      redrawMap (NVFalse);
    }
}



void 
pfmEdit3D::slotExagTriggered (int action)
{
  if (!misc.busy)
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

      overlayData (map, &options, &misc);
    }
}



void 
pfmEdit3D::slotExagReleased ()
{
  options.exaggeration = (NV_FLOAT32) exagBar->value () / 100.0;
  map->setExaggeration (options.exaggeration);

  redrawMap (NVFalse);
}



void 
pfmEdit3D::slotExaggerationChanged (NV_FLOAT32 exaggeration, NV_FLOAT32 apparent_exaggeration)
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
pfmEdit3D::keyPressEvent (QKeyEvent *e)
{
  void actionKey (pfmEdit3D *parent, nvMapGL *map, MISC *misc, QString key, NV_INT32 lock_point);


  //  Lock in nearest point so mouse moves won't change it.

  lock_point = misc.nearest_point;


  //  If we're drawing and a key is pressed we want to cancel drawing.

  if (misc.busy)
    {
      misc.drawing_canceled = NVTrue;
      return;
    }


  //  First check for the hard-wired keys.

  switch (e->key ())
    {
      //  Arrow keys.
      //
      //  Unmodified up arrow key will start slicing
      //
      //  Ctrl-arrow keys will move in the direction of the arrow with some percentage of overlap.  This implies that
      //  we want to save our edits and we must move the edit area in the viewer to match where we've moved.  If we
      //  moved outside of the area displayed in the viewer we must also move the viewer window.  The amount of 
      //  overlap is specified in the Preferences dialog for each program.

    case Qt::Key_Escape:

      //  Turn off slicing if it was on (this will also call redrawMap ()).

      if (misc.slice) slotPopupMenu (popup[NUMPOPUPS - 1]);

      return;
      break;

    case Qt::Key_Return:
      return;
      break;

    case Qt::Key_Left:
      if (e->modifiers () == Qt::ControlModifier)
        {
          //  Reset the area.

          NV_FLOAT64 xsize = orig_bounds.max_x - orig_bounds.min_x;
          NV_FLOAT64 overlap_mult = (NV_FLOAT64) (100.0 - options.overlap_percent) / 100.0;

          orig_bounds.min_x = orig_bounds.min_x - xsize * overlap_mult;
          orig_bounds.max_x = orig_bounds.max_x - xsize * overlap_mult;

          if (orig_bounds.min_x < misc.total_mbr.min_x)
            {
              orig_bounds.min_x = misc.total_mbr.min_x;
              orig_bounds.max_x = orig_bounds.min_x + xsize;
            }


          //  Set the new shared bounds so that pfmView will know what to display.

          misc.abe_share->edit_area = orig_bounds;


          //  Exit with the special output for pfmView so it will know what to do.

          moveWindow = 100;
          slotExit (0);
        }
      else
        {
          map->rotateY (-options.rotation_increment);
        }
      return;
      break;

    case Qt::Key_PageUp:
      slotExagTriggered (QAbstractSlider::SliderSingleStepSub);
      return;
      break;

    case Qt::Key_Right:
      if (e->modifiers () == Qt::ControlModifier)
        {
          //  Reset the area.

          NV_FLOAT64 xsize = orig_bounds.max_x - orig_bounds.min_x;
          NV_FLOAT64 overlap_mult = (NV_FLOAT64) (100.0 - options.overlap_percent) / 100.0;

          orig_bounds.min_x = orig_bounds.min_x + xsize * overlap_mult;
          orig_bounds.max_x = orig_bounds.max_x + xsize * overlap_mult;

          if (orig_bounds.max_x > misc.total_mbr.max_x)
            {
              orig_bounds.max_x = misc.total_mbr.max_x;
              orig_bounds.min_x = orig_bounds.max_x - xsize;
            }


          //  Set the new shared bounds so that pfmView will know what to display.

          misc.abe_share->edit_area = orig_bounds;


          //  Exit with the special output for pfmView so it will know what to do.

          moveWindow = 100;
          slotExit (0);
        }
      else
        {
          map->rotateY (options.rotation_increment);
        }
      return;
      break;

    case Qt::Key_PageDown:
      slotExagTriggered (QAbstractSlider::SliderSingleStepAdd);
      return;
      break;

    case Qt::Key_Up:

      if (e->modifiers () == Qt::ControlModifier)
        {
          //  Reset the area.

          NV_FLOAT64 ysize = orig_bounds.max_y - orig_bounds.min_y;
          NV_FLOAT64 overlap_mult = (NV_FLOAT64) (100.0 - options.overlap_percent) / 100.0;

          orig_bounds.min_y = orig_bounds.min_y + ysize * overlap_mult;
          orig_bounds.max_y = orig_bounds.max_y + ysize * overlap_mult;

          if (orig_bounds.max_y > misc.total_mbr.max_y)
            {
              orig_bounds.max_y = misc.total_mbr.max_y;
              orig_bounds.min_y = orig_bounds.max_y - ysize;
            }


          //  Set the new shared bounds so that pfmView will know what to display.

          misc.abe_share->edit_area = orig_bounds;


          //  Exit with the special output for pfmView so it will know what to do.

          moveWindow = 100;
          slotExit (0);
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
          //  Reset the area.

          NV_FLOAT64 ysize = orig_bounds.max_y - orig_bounds.min_y;
          NV_FLOAT64 overlap_mult = (NV_FLOAT64) (100.0 - options.overlap_percent) / 100.0;

          orig_bounds.min_y = orig_bounds.min_y - ysize * overlap_mult;
          orig_bounds.max_y = orig_bounds.max_y - ysize * overlap_mult;

          if (orig_bounds.min_y < misc.total_mbr.min_y)
            {
              orig_bounds.min_y = misc.total_mbr.min_y;
              orig_bounds.max_y = orig_bounds.min_y + ysize;
            }


          //  Set the new shared bounds so that pfmView will know what to display.

          misc.abe_share->edit_area = orig_bounds;


          //  Exit with the special output for pfmView so it will know what to do.

          moveWindow = 100;
          slotExit (0);
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

  if (!key.compare (options.hotkey[ACCEPT_FILTER_HIGHLIGHTED], Qt::CaseInsensitive))
    {
      if (filter_active || attr_filter_active || hof_filter_active)
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

              store_undo (&misc, options.undo_levels, misc.data[j].val, j);
              misc.data[j].val |= PFM_MANUALLY_INVAL;
            }
          end_undo_block (&misc);

          free (misc.highlight);
          misc.highlight = NULL;
          misc.highlight_count = 0;

          options.function = misc.save_function;


          //  Just in case we were flagging data and we decided to delete it we want to unset the flag index.

          options.flag_index = 0;
          bFlag->setIcon (flagIcon[options.flag_index]);
          disconnect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));
          flag[0]->setChecked (TRUE);
          connect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));

          redrawMap (NVTrue);
        }

      return;
    }


  if (!key.compare (options.hotkey[REJECT_FILTER_HIGHLIGHTED], Qt::CaseInsensitive))
    {
      if (filter_active || attr_filter_active || hof_filter_active)
        {
          slotFilterReject ();
          return;
        }


      //  If we have highlighted points, check for invalid points and validate them.

      if (misc.highlight_count)
        {
          for (NV_INT32 i = 0 ; i < misc.highlight_count ; i++)
            {
              NV_INT32 j = misc.highlight[i];

              store_undo (&misc, options.undo_levels, misc.data[j].val, j);
              if (misc.data[j].val & PFM_INVAL)
                {
                  misc.data[j].val &= ~PFM_MANUALLY_INVAL;
                  misc.data[j].val &= ~PFM_FILTER_INVAL;
                }
            }
          end_undo_block (&misc);

          free (misc.highlight);
          misc.highlight = NULL;
          misc.highlight_count = 0;

          options.function = misc.save_function;


          //  Just in case we were flagging data and we decided to delete it we want to unset the flag index.

          options.flag_index = 0;
          bFlag->setIcon (flagIcon[options.flag_index]);
          disconnect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));
          flag[0]->setChecked (TRUE);
          connect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));

          redrawMap (NVTrue);
        }

      return;
    }


  //  Delete a single point.  Note that we're calling slotPopupMenu with popup[0].  If you change the right click menu DELETE_POINT
  //  option you have to change this as well.

  if (options.function == DELETE_POINT && !key.compare (options.hotkey[DELETE_SINGLE_POINT], Qt::CaseInsensitive))
    {
      slotPopupMenu (popup[0]);
      return;
    }


  //  Toggle freezing of the main marker and the multi...[0] data that is shared with other programs.

  if ((options.function == DELETE_POINT || options.function == MEASURE) && !key.compare (options.hotkey[FREEZE_ALL], Qt::CaseInsensitive))
    {
      slotPopupMenu (popup[FREEZE_POPUP]);
      return;
    }


  //  Toggle freezing of all of the markers and all of the multi... data that is shared with other programs.

  if ((options.function == DELETE_POINT || options.function == MEASURE) && !key.compare (options.hotkey[FREEZE], Qt::CaseInsensitive))
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

          if (options.function == DELETE_POINT || options.function == ADD_FEATURE || options.function == EDIT_FEATURE ||
              options.function == MEASURE || options.data_type[i][0])
            {
              //  Check type of nearest point against acceptable types for each program.  This will
              //  actually let us use a hot key in more than one place (probably not a good idea
              //  anyway).

              NV_INT16 type;
              NV_CHAR tmp[1024];
              read_list_file (misc.pfm_handle[misc.data[lock_point].pfm], misc.data[lock_point].file, tmp, &type);

              if (!type || options.data_type[i][type] || options.data_type[i][0])
                {
                  NV_INT32 hot_action_type = 0;
                  if (!key.compare (options.hotkey[i], Qt::CaseInsensitive)) hot_action_type = 1;


                  //  Check for an occurrence of any of possible multiple action keys

                  if (!options.action[i].isEmpty ())
                    {
                      for (NV_INT32 j = 0 ; j < 10 ; j++)
                        {
                          QString cut = options.action[i].section (',', j, j);
                          if (cut.isEmpty ()) break;
                          if (!key.compare (cut, Qt::CaseInsensitive)) hot_action_type = 2;
                        }
                    }

                  if (hot_action_type == 1)
                    {
                      //  Don't do anything if this is a hotkey polygon only program.

                      if (options.hk_poly_filter[i] == 1)
                        {
                          QString message;
                          message = tr ("The key <b><i>") + e->text () + tr ("</i></b> is only defined for hot key polygon/area mode. "
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
                      //  Just in case we're going to call chartsPic we want to save the nearest feature snippet file name.
  
                      if (misc.nearest_feature != -1 && options.function == EDIT_FEATURE) 
                        {
                          sprintf (misc.abe_share->snippet_file_name, "%s/%s_%05d.jpg",
                                   pfm_dirname (misc.abe_share->open_args[0].target_path),
                                   pfm_basename (misc.abe_share->open_args[0].target_path),
                                   misc.feature[misc.nearest_feature].record_number);
                        }
                      else
                        {
                          sprintf (misc.abe_share->snippet_file_name, "NONE");
                        }


                      //  Have to check for the mosaicView zoom key so that we can tell pfmView to tell any linked apps
                      //  that we're zooming.

                      if (key == options.action[8].section (',', 0, 0)) misc.abe_share->zoom_requested = NVTrue;


                      actionKey (this, map, &misc, key, lock_point);
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
          if (!key.compare (options.hotkey[i], Qt::CaseInsensitive) && options.hk_poly_eligible[i])
            {
              NV_BOOL failed = NVFalse;
              hkp = new hotkeyPolygon (this, map, &options, &misc, i, &failed);
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
pfmEdit3D::slotRedrawMap ()
{
  redrawMap (NVTrue);
}



void 
pfmEdit3D::slotHotkeyPolygonDone ()
{
  redrawMap (NVTrue);
}



void 
pfmEdit3D::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  if (misc.abe_share->read_only)
    {
      slotExit (2);
    }
  else
    {
      NV_INT32 ret = QMessageBox::Yes;
      ret = QMessageBox::information (this, tr ("pfmEdit3D"), tr ("Do you wish to save any changes you may have made?"),
                                      QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton);

      if (ret == QMessageBox::Yes)
        {
          slotExit (0);
        }
      else
        {
          slotExit (2);
        }
    }
}



//!  A bunch of slots.

void 
pfmEdit3D::slotExitSave ()
{
  slotExit (0);
}



void 
pfmEdit3D::slotExitMask ()
{
  slotExit (1);
}



void 
pfmEdit3D::slotExitNoSave ()
{
  slotExit (2);
}



void 
pfmEdit3D::slotExit (int id)
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
        }
    }


  //  Clear up any highlight memory we had.

  if (misc.highlight_count)
    {
      free (misc.highlight);
      misc.highlight = NULL;
      misc.highlight_count = 0;
    }


  //  Shortcut to make sure we filter mask.

  if (id == 1)
    {
      id = 0;
      misc.filtered = NVTrue;
    }


  //  Make sure the default function is one of the delete functions

  options.function = misc.save_mode;


  //  Check for ancillary programs that can respawn to see if they are up and running.  If they are, set the state to 2.

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      if (options.state[i])
        {
          if (options.state[i] == 2 || (ancillaryProgram[i] && ancillaryProgram[i]->state () == QProcess::Running))
            {
              options.state[i] = 2;
            }
          else
            {
              options.state[i] = 1;
            }
        }
    }


  //  Force persistent running ancillary programs (those with a non-zero state value) to exit cleanly (if
  //  we're doing "kill and respawn").

  if (options.kill_and_respawn) misc.abe_share->key = ANCILLARY_FORCE_EXIT;


  //  Get the last rotation values

  options.zx_rotation = map->getZXRotation ();
  options.y_rotation = map->getYRotation ();


  //  Save the user's defaults

  envout (&options, &misc, this);


  //  Close the BFD file if it was open

  if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);


  //  We output a 0 if no changes have been made, a 1 if changes to the PFM structure have been made or a 2
  //  if just feature changes have been made.  The second value (after the comma) is the filtered flag.  This
  //  tells pfmView that we have filtered and it needs to filter mask the area.

  NV_INT32 put_ret = 0;

  finishing = NVFalse;

  switch (id)
    {
      //  Normal exit and save.

    case 0:
      map->setCursor (Qt::WaitCursor);

      put_ret = put_buffer (&misc);

      if (misc.feature_mod || put_ret || misc.filtered)
        {
          if (put_ret && (options.auto_unload || force_auto_unload)) 
            {
              printf ("Edit return status:%d,%d\n", moveWindow + 1, misc.filtered);
              finishing = NVTrue;
              autoUnload ();
            }
          else
            {
              if (put_ret || misc.filtered)
                {
                  printf ("Edit return status:%d,%d\n", moveWindow + 1, misc.filtered);
                }
              else 
                {
                  printf ("Edit return status:%d,%d\n", moveWindow + 2, misc.filtered);
                }
            }
        }
      else
        {
          printf ("Edit return status:%d,0\n", moveWindow + 0);
        }
      break;


      //  Exit without saving

    case 2:
      if (misc.feature_mod)
        {
          printf ("Edit return status:2,0\n");
        }
      else
        {
          printf ("Edit return status:0,0\n");
        }
      break;
    }


  if (!finishing) 
    {
      for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++) close_pfm_file (misc.pfm_handle[pfm]);

      clean_exit (0);
    }
}



void 
pfmEdit3D::slotPrefs ()
{
  if (prefs_dialog) prefs_dialog->close ();

  prefs_dialog = new Prefs (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  prefs_dialog->move (x () + width () / 2 - prefs_dialog->width () / 2, y () + height () / 2 - prefs_dialog->height () / 2);

  connect (prefs_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotPrefDataChanged ()));
  connect (prefs_dialog, SIGNAL (hotKeyChangedSignal (NV_INT32)), this, SLOT (slotPrefHotKeyChanged (NV_INT32)));
}



//!  Changed some of the preferences

void 
pfmEdit3D::slotPrefDataChanged ()
{
  bContour->setChecked (options.display_contours);

  bDisplayReference->setChecked (options.display_reference);
  bDisplayNull->setChecked (options.display_null);
  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);
  bDisplayChildren->setChecked (options.display_children);
  bDisplayFeatureInfo->setChecked (options.display_feature_info);

  if (options.function == DELETE_POINT) bDeletePoint->setChecked (TRUE);
  if (options.function == DELETE_RECTANGLE) bDeleteRect->setChecked (TRUE);
  if (options.function == DELETE_POLYGON) bDeletePoly->setChecked (TRUE);
  if (options.function == KEEP_POLYGON) bKeepPoly->setChecked (TRUE);


  //  Set the semi-transparent colors for slicing.

  NV_FLOAT32 hue_inc = (NV_FLOAT32) (misc.abe_share->max_hsv_color - misc.abe_share->min_hsv_color) / (NV_FLOAT32) (NUMSHADES + 1);

  for (NV_INT32 j = 0 ; j < NUMSHADES ; j++) 
    {
      QColor color;
      color.setHsv ((NV_INT32) (misc.abe_share->min_hsv_color + j * hue_inc), 255, 255, options.slice_alpha);

      misc.color_array[1][j][0] = color.red ();
      misc.color_array[1][j][0] = color.green ();
      misc.color_array[1][j][0] = color.blue ();
      misc.color_array[1][j][0] = options.slice_alpha;
    }

  options.ref_color[1] = options.ref_color[0];
  options.ref_color[1].setAlpha (options.slice_alpha);


  map->setBackgroundColor (options.background_color);
  map->setScaleColor (options.scale_color);
  map->enableScale (options.draw_scale);
  map->setZoomPercent (options.zoom_percent);
  map->setComplexObjectWidth (options.objectWidth);
  map->setComplexDivisionals (options.objectDivisionals);
  map->setRenderMode (options.drawingMode);

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

  filterAccept->setToolTip (tr ("Accept filter results ") + "[" + options.hotkey[ACCEPT_FILTER_HIGHLIGHTED] + "]");
  filterReject->setToolTip (tr ("Reject filter results ") + "[" + options.hotkey[REJECT_FILTER_HIGHLIGHTED] + "]");
  attrFilterAccept->setToolTip (tr ("Accept filter results ") + "[" + options.hotkey[ACCEPT_FILTER_HIGHLIGHTED] + "]");
  attrFilterReject->setToolTip (tr ("Reject filter results ") + "[" + options.hotkey[REJECT_FILTER_HIGHLIGHTED] + "]");
  if (misc.hof_present)
    {
      hofWaveFilterAccept->setToolTip (tr ("Accept filter results ") + "[" + options.hotkey[ACCEPT_FILTER_HIGHLIGHTED] + "]");
      hofWaveFilterReject->setToolTip (tr ("Reject filter results ") + "[" + options.hotkey[REJECT_FILTER_HIGHLIGHTED] + "]");
    }
  bClearHighlight->setToolTip (tr ("Clear highlighted points ") + "[" + options.hotkey[REJECT_FILTER_HIGHLIGHTED] + "]");

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++) slotPrefHotKeyChanged (i);


  //  Save the user's defaults

  envout (&options, &misc, this);


  //  Redraw the data (which is needed because we may have changed the exaggeration).

  if (!misc.busy) redrawMap (NVTrue);
}



void 
pfmEdit3D::slotPrefHotKeyChanged (NV_INT32 i)
{
  QString tip = misc.buttonText[i] + "  [" + options.buttonAccel[i] + "]";
  if (misc.button[i])
    {
      misc.button[i]->setToolTip (tip);
      misc.button[i]->setShortcut (QKeySequence (options.buttonAccel[i]));
    }
  else if (misc.action[i])
    {
      misc.action[i]->setShortcut (QKeySequence (options.buttonAccel[i]));
    }
}



void
pfmEdit3D::slotFeatureMenu (QAction *action)
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
      bAddFeature->setEnabled (TRUE);
      bDeleteFeature->setEnabled (TRUE);
      bEditFeature->setEnabled (TRUE);
      bVerifyFeatures->setEnabled (TRUE);

      setMainButtons (NVTrue);

      redrawMap (NVFalse);
      map->flush ();
    }
  else
    {
      bDisplayChildren->setEnabled (FALSE);
      bDisplayFeatureInfo->setEnabled (FALSE);
      bAddFeature->setEnabled (FALSE);
      bDeleteFeature->setEnabled (FALSE);
      bEditFeature->setEnabled (FALSE);
      bVerifyFeatures->setEnabled (FALSE);


      //  Make sure we aren't using one of the "feature" editor functions

      if (options.function == ADD_FEATURE || options.function == DELETE_FEATURE ||
          options.function == MOVE_FEATURE || options.function == EDIT_FEATURE)
        {
          options.function = DELETE_POINT;
          setFunctionCursor (options.function);
        }

      redrawMap (NVFalse);
    }
}



void
pfmEdit3D::slotDisplayChildren ()
{
  if (bDisplayChildren->isChecked ())
    {
      options.display_children = NVTrue;

      redrawMap (NVFalse);
    }
  else
    {
      options.display_children = NVFalse;

      redrawMap (NVFalse);

      map->flush ();
    }
}



void
pfmEdit3D::slotDisplayFeatureInfo ()
{
  if (bDisplayFeatureInfo->isChecked ())
    {
      options.display_feature_info = NVTrue;

      redrawMap (NVFalse);
    }
  else
    {
      options.display_feature_info = NVFalse;

      redrawMap (NVFalse);

      map->flush ();
    }
}



void
pfmEdit3D::slotVerifyFeatures ()
{
  BFDATA_RECORD bfd_record;

  for (NV_U_INT32 i = 0 ; i < misc.bfd_header.number_of_records ; i++)
    {
      if (misc.feature[i].confidence_level)
	{
	  if (!check_bounds (&options, &misc, misc.feature[i].longitude, misc.feature[i].latitude,
			     misc.feature[i].depth, PFM_USER, NVFalse, 0, NVFalse, NVFalse))
	    {
	      binaryFeatureData_read_record (misc.bfd_handle, i, &bfd_record);

	      bfd_record.confidence_level = 5;

	      binaryFeatureData_write_record (misc.bfd_handle, i, &bfd_record, NULL, NULL);
	    }
	}
    }


  misc.feature_mod = NVTrue;


  if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);
  misc.bfd_open = NVFalse;

  if ((misc.bfd_handle = binaryFeatureData_open_file (misc.abe_share->open_args[0].target_path, &misc.bfd_header, BFDATA_UPDATE)) >= 0)
    {
      if (binaryFeatureData_read_all_short_features (misc.bfd_handle, &misc.feature) < 0)
	{
	  QString msg = QString (binaryFeatureData_strerror ());
	  QMessageBox::warning (this, tr ("pfmEdit3D"), tr ("Unable to read feature records\nReason: ") + msg);
	  binaryFeatureData_close_file (misc.bfd_handle);
	}
      else
	{
	  misc.bfd_open = NVTrue;
	}
    }


  redrawMap (NVFalse);

  map->flush ();
}



void
pfmEdit3D::slotDisplayReference ()
{
  if (bDisplayReference->isChecked ())
    {
      options.display_reference = NVTrue;
      bReferencePoly->setEnabled (TRUE);
      bUnreferencePoly->setEnabled (TRUE);
    }
  else
    {
      options.display_reference = NVFalse;
      bReferencePoly->setEnabled (FALSE);
      bUnreferencePoly->setEnabled (FALSE);


      //  Make sure we aren't using one of the "reference" editor functions

      if (options.function == SET_REFERENCE || options.function == UNSET_REFERENCE)
        {
          options.function = DELETE_POINT;
          setFunctionCursor (options.function);
        }

    }

  redrawMap (NVFalse);
}



void
pfmEdit3D::slotDisplayManInvalid ()
{
  if (bDisplayManInvalid->isChecked ())
    {
      bDisplayNull->setEnabled (TRUE);

      options.display_man_invalid = NVTrue;
    }
  else
    {
      options.display_man_invalid = NVFalse;

      if (!bDisplayFltInvalid->isChecked ()) bDisplayNull->setEnabled (FALSE);
    }


  //  Always turn auto-scaling on whenever we turn invalid data on or off.  If we're not auto-scaling it will be turned off again after
  //  we redraw the map.

  map->setAutoScale (NVTrue);


  redrawMap (NVTrue);
}



void
pfmEdit3D::slotDisplayFltInvalid ()
{
  if (bDisplayFltInvalid->isChecked ())
    {
      bDisplayNull->setEnabled (TRUE);

      options.display_flt_invalid = NVTrue;
    }
  else
    {
      options.display_flt_invalid = NVFalse;

      if (!bDisplayManInvalid->isChecked ()) bDisplayNull->setEnabled (FALSE);
    }


  //  Always turn auto-scaling on whenever we turn invalid data on or off.  If we're not auto-scaling it will be turned off again after
  //  we redraw the map.

  map->setAutoScale (NVTrue);


  redrawMap (NVTrue);
}



void
pfmEdit3D::slotDisplayNull ()
{
  if (bDisplayNull->isChecked ())
    {
      options.display_null = NVTrue;
    }
  else
    {
      options.display_null = NVFalse;
    }


  //  Always turn auto-scaling on whenever we turn invalid data on or off.  If we're not auto-scaling it will be turned off again after
  //  we redraw the map.

  map->setAutoScale (NVTrue);


  redrawMap (NVTrue);
}



void
pfmEdit3D::slotDisplayAll ()
{
  bUndisplaySingle->setEnabled (TRUE);
  bDisplayMultiple->setEnabled (TRUE);

  bDisplayAll->setEnabled (FALSE);

  bDisplayAll->setIcon (QIcon (":/icons/displayall.xpm"));

  misc.num_lines = local_num_lines = 0;

  options.function = misc.save_function;

  redrawMap (NVTrue);
}



void
pfmEdit3D::slotUndisplaySingle ()
{
  if (options.function != UNSET_SINGLE) misc.save_function = options.function;
  options.function = UNSET_SINGLE;
  setFunctionCursor (options.function);
}



void
pfmEdit3D::slotDisplayMultiple ()
{
  if (options.function != SET_MULTIPLE) misc.save_function = options.function;
  options.function = SET_MULTIPLE;
  setFunctionCursor (options.function);
}



void 
pfmEdit3D::slotInvertHighlight ()
{
  if (misc.highlight_count)
    {
      NV_BOOL *temp_highlight_flag = (NV_BOOL *) calloc (misc.abe_share->point_cloud_count, sizeof (NV_BOOL));
      if (temp_highlight_flag == NULL)
        {
          perror ("Allocating temp_highlight_flag in slotInvertHighlight");
          exit (-1);
        }

      for (NV_INT32 i = 0 ; i < misc.highlight_count ; i++) temp_highlight_flag[misc.highlight[i]] = NVTrue;

      free (misc.highlight);
      misc.highlight = NULL;
      misc.highlight_count = misc.abe_share->point_cloud_count - misc.highlight_count;

      misc.highlight = (NV_INT32 *) calloc (misc.highlight_count, sizeof (NV_INT32));
      if (misc.highlight == NULL)
        {
          perror ("Allocating misc.highlight in slotInvertHighlight");
          exit (-1);
        }

      NV_INT32 new_count = 0;
      for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
        {
          if (!temp_highlight_flag[i])
            {
              misc.highlight[new_count] = i;
              new_count++;
            }
        }

      free (temp_highlight_flag);
    }

  redrawMap (NVFalse);
}



void 
pfmEdit3D::slotClearHighlight ()
{
  if (misc.highlight_count)
    {
      free (misc.highlight);
      misc.highlight = NULL;
      misc.highlight_count = 0;
    }

  bClearHighlight->setEnabled (FALSE);


  //  Since we decided to manually clear all of the highlighted points let's make sure that we weren't highlighting by flag.

  options.flag_index = 0;
  bFlag->setIcon (flagIcon[options.flag_index]);
  disconnect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));
  flag[0]->setChecked (TRUE);
  connect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));


  redrawMap (NVFalse);
}



void
pfmEdit3D::slotContour ()
{
  if (bContour->isChecked ())
    {
      options.display_contours = NVTrue;
    }
  else
    {
      options.display_contours = NVFalse;
    }

  redrawMap (NVTrue);
}



void 
pfmEdit3D::slotDisplayLinesDataChanged ()
{
  if (misc.num_lines) bDisplayAll->setEnabled (TRUE);

  redrawMap (NVTrue);
}



void 
pfmEdit3D::slotLockValueDone (NV_BOOL accepted)
{
  if (accepted)
    {
      //  We have to reset the colors here because we might have changed the min or max color.

      NV_INT32 k = options.color_index;


      NV_FLOAT32 hue_inc = (NV_FLOAT32) (options.max_hsv_color[k] - options.min_hsv_color[k]) / (NV_FLOAT32) (NUMSHADES + 1);

      for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
        {
          misc.track_color_array[j].setHsv ((NV_INT32) (options.min_hsv_color[k] + j * hue_inc), 255, 255, 255); 
          misc.color_array[0][j][0] = misc.color_array[1][j][0] = misc.track_color_array[j].red ();
          misc.color_array[0][j][1] = misc.color_array[1][j][1] = misc.track_color_array[j].green ();
          misc.color_array[0][j][2] = misc.color_array[1][j][2] = misc.track_color_array[j].blue ();
          misc.color_array[0][j][3] = misc.track_color_array[j].alpha ();

          misc.color_array[1][j][3] = options.slice_alpha;
        }
      options.ref_color[1] = options.ref_color[0];
      options.ref_color[1].setAlpha (options.slice_alpha);


      //  If we changed the depth colors, locks, or values (index 0) then we need to set shared memory to inform the editors when they start up.

      if (!k)
        {
          misc.abe_share->min_hsv_color = options.min_hsv_color[0];
          misc.abe_share->max_hsv_color = options.max_hsv_color[0];
          misc.abe_share->min_hsv_value = options.min_hsv_value[0];
          misc.abe_share->max_hsv_value = options.max_hsv_value[0];
          misc.abe_share->min_hsv_locked = options.min_hsv_locked[0];
          misc.abe_share->max_hsv_locked = options.max_hsv_locked[0];


          //  We need to tell pfmView that we changed the settings.

          misc.abe_share->settings_changed = NVTrue;
        }


      redrawMap (NVTrue);
    }
}



void 
pfmEdit3D::slotMinScalePressed (QMouseEvent *e __attribute__ ((unused)))
{
  if (misc.busy) return;

  lockValue *lck = new lockValue (map, &options, &misc, NVTrue, options.color_index);
  connect (lck, SIGNAL (lockValueDone (NV_BOOL)), this, SLOT (slotLockValueDone (NV_BOOL)));


  //  Put the dialog in the middle of the screen.

  lck->move (x () + width () / 2 - lck->width () / 2, y () + height () / 2 - lck->height () / 2);
}



void 
pfmEdit3D::slotMaxScalePressed (QMouseEvent *e __attribute__ ((unused)))
{
  if (misc.busy) return;

  lockValue *lck = new lockValue (map, &options, &misc, NVFalse, options.color_index);
  connect (lck, SIGNAL (lockValueDone (NV_BOOL)), this, SLOT (slotLockValueDone (NV_BOOL)));


  //  Put the dialog in the middle of the screen.

  lck->move (x () + width () / 2 - lck->width () / 2, y () + height () / 2 - lck->height () / 2);
}



void 
pfmEdit3D::slotMode (int id)
{
  //  Get rid of the av_dist points if any were present

  if (misc.av_dist_count)
    {
      free (misc.av_dist_list);
      misc.av_dist_list = NULL;
      misc.av_dist_count = 0;

      redrawMap (NVTrue);
    }


  misc.save_function = options.function;


  //  If we come back to the DELETE_POINT mode and we have the AV up and running, we may be going back to AV_DISTANCE_TOOL mode.

  if (id == DELETE_POINT && misc.avInput == AV_DIST_THRESH)
    {
      id = AV_DISTANCE_TOOL;
      options.function = id;

      if (distLineStart.x > 0.0f && distLineStart.y > 0.0f && distLineEnd.x > 0.0f && distLineEnd.y > 0.0f)
        {
          NV_INT32 pointsWithin = ComputeLineDistanceFilter (distLineStart, distLineEnd, &misc, &options);


          //  We will set up some variables to let us overlay data seamlessly for the shots that passed the Distance Threshold test

          if (pointsWithin > 0)
            {
              trackMap->redrawMap (NVFalse);
              redrawMap (NVTrue);
            }
        }
    }
  else
    { 
      options.function = id;
    }

  setFunctionCursor (options.function);


  switch (options.function)
    {
      //  Save the last edit mode in case we exit

    case DELETE_RECTANGLE:
    case DELETE_POLYGON:
    case DELETE_POINT:
    case KEEP_POLYGON:
    case EDIT_FEATURE:
      misc.save_mode = options.function;
      break;
    }
}



void 
pfmEdit3D::slotUnload ()
{
  if (options.auto_unload)
    {
      options.auto_unload = NVFalse;
    }
  else
    {
      options.auto_unload = NVTrue;
    }
}



void 
pfmEdit3D::slotUnloadError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmEdit3D unload process"), tr ("Unable to start the unload process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmEdit3D unload process"), tr ("The unload process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmEdit3D unload process"), tr ("The unload process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmEdit3D unload process"), tr ("There was a write error to the unload process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmEdit3D unload process"), tr ("There was a read error from the unload process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmEdit3D unload process"), tr ("The unload process died with an unknown error!"));
      break;
    }

  unload_done = NVTrue;
}



//!  This is the return from the unload QProcess (when finished)

void 
pfmEdit3D::slotUnloadDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  unload_prog->setValue (100);

  if (unloadProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("Unload"), tr ("An error occurred during unload, some or all of your edits were not saved!") +
                             tr ("\nPlease run pfm_unload separatedly to see what the error condition was."));
    }


  //  This is cheating ;-)  We're using the finishing flag to force it not to close after unload if we did a Ctrl-Arrow key move
  //  from the keypress event.  Normally we call unload only on exit.  See keyPressEvent above for more info.

  //  Need the extra comparison or an exit will be blocked, test it with pfm of 0, the last guy.  If this is not the last pfm file
  //  we want to set unload_done to true so the loop in autoUnload can progress.  We also close the pfm file as we are done with the unload for it.

  if ((finishing) && (unload_pfm))
    {
      close_pfm_file (misc.pfm_handle[unload_pfm]);

      unload_done = NVTrue;
      return;
    }

  close_pfm_file (misc.pfm_handle[unload_pfm]);


  //  Only exit after all of the PFM files have been unloaded (0 will always be the last unloaded).


  if (unload_pfm)
    {
      unload_done = NVTrue;
      return;
    }


  //  pfmView will read this value and know that we have modified stuff.

  printf ("Edit return status:1,0\n");


  clean_exit (0);
}



/*!
  This is the stderr read return from the unload QProcess.  We're dumping the percentage to
  stderr in the unload program so we can track it here.
*/

void 
pfmEdit3D::slotUnloadReadyReadStandardError ()
{
  NV_INT32 percent;
  static NV_BOOL flip = NVTrue;

  QByteArray response = unloadProc->readAllStandardError ();
  NV_CHAR *res = response.data ();

  if (strchr (res, '%'))
    {
      sscanf (res, "%d%%", &percent);

      if (percent == 100 && flip)
        {
          percent = 0;
          QString progText = tr ("Resetting modified flags for ") +
            QFileInfo (QString (misc.abe_share->open_args[unload_pfm].list_path)).fileName ().remove (".pfm");

          unload_prog->setLabelText (progText);
          flip = NVFalse;
        }

      if (percent == 100 && !flip) flip = NVTrue;

      unload_prog->setValue (percent);
      qApp->processEvents();
    }
}



//!  Kick off the unload QProcess

void
pfmEdit3D::autoUnload ()
{
  QString arg;
  QStringList arguments;


  unload_done = NVTrue;
  unloadProc = NULL;
  unload_prog = NULL;


  //  Unload bottom to top since the 0 level PFM is always displayed.  This way we can always be assured of
  //  doing the exit from the "finished" slot.

  for (NV_INT32 pfm = misc.abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Wait for the "finished" slot to tell us we're done unloading and have closed the file.

      while (!unload_done)
        {
#ifdef NVWIN3X
          Sleep (2000);
#else
          sleep (2);
#endif
        }


      //  Only unload PFM files that were displayed.

      if (misc.abe_share->display_pfm[pfm])
        {
          if (unloadProc != NULL) delete unloadProc;

          unloadProc = new QProcess (this);


          arguments.clear ();
          arguments += misc.abe_share->open_args[pfm].list_path;
          arguments += QString ("-Q");

          arg.sprintf ("-a %.9f,%.9f,%.9f,%.9f", mapdef.initial_bounds.max_y, mapdef.initial_bounds.min_y, 
                       mapdef.initial_bounds.max_x, mapdef.initial_bounds.min_x);
          arguments += arg;


          if (unload_prog != NULL) delete unload_prog;


          QString progText = tr ("Reading PFM data from ") +
            QFileInfo (QString (misc.abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm");

          unload_prog = new QProgressDialog (progText, 0, 0, 100, this);

          unload_prog->setWindowTitle (tr ("Unloading"));
          unload_prog->setWindowModality (Qt::WindowModal);
          unload_prog->show ();

          qApp->processEvents();


          connect (unloadProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, 
                   SLOT (slotUnloadDone (int, QProcess::ExitStatus)));
          connect (unloadProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotUnloadReadyReadStandardError ()));
          connect (unloadProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotUnloadError (QProcess::ProcessError)));


          //  Let the "finished" slot know which PFM we're unloading.

          unload_pfm = pfm;
          unload_done = NVFalse;


          unloadProc->start (options.unload_prog, arguments);
        }
      else
        {
          unload_done = NVTrue;
        }
    }
}



void
pfmEdit3D::slotStop ()
{
  misc.drawing_canceled = NVTrue;
}



void 
pfmEdit3D::slotEditFeatureDataChanged ()
{
  //  If we added a feature add PFM_SELECTED_FEATURE to the validity for the selected point

  if (misc.add_feature_index >= 0)
    {
      misc.data[misc.add_feature_index].val |= PFM_SELECTED_FEATURE;
      misc.add_feature_index = -1;
    }

  misc.feature_mod = NVTrue;
  redrawMap (NVFalse);


  //  If the mosaic viewer was running, tell it to redraw.

  misc.abe_share->key = FEATURE_FILE_MODIFIED;
}



void 
pfmEdit3D::editFeatureNum (NV_INT32 feature_number)
{
  editFeature_dialog = new editFeature (this, &options, &misc, feature_number);
  connect (editFeature_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotEditFeatureDataChanged ()));

  editFeature_dialog->show ();
}



void
pfmEdit3D::slotAttrMenu (QAction *action)
{
  for (NV_INT32 i = 0 ; i < NUM_ATTR + PRE_ATTR ; i++)
    {
      if (action == attr[i])
        {
          options.color_index = i;

          NV_INT32 k = options.color_index;


          NV_FLOAT32 hue_inc = (NV_FLOAT32) (options.max_hsv_color[k] - options.min_hsv_color[k]) / (NV_FLOAT32) (NUMSHADES + 1);

          for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
            {
              misc.track_color_array[j].setHsv ((NV_INT32) (options.min_hsv_color[k] + j * hue_inc), 255, 255, 255); 
              misc.color_array[0][j][0] = misc.color_array[1][j][0] = misc.track_color_array[j].red ();
              misc.color_array[0][j][1] = misc.color_array[1][j][1] = misc.track_color_array[j].green ();
              misc.color_array[0][j][2] = misc.color_array[1][j][2] = misc.track_color_array[j].blue ();
              misc.color_array[0][j][3] = misc.track_color_array[j].alpha ();

              misc.color_array[1][j][3] = options.slice_alpha;
            }
          options.ref_color[1] = options.ref_color[0];
          options.ref_color[1].setAlpha (options.slice_alpha);

          break;
        }
    }


  //  Check to see if we were trying to run the attribute filter.  If so we want to discard it.

  if (attr_filter_active) slotFilterReject ();


  bAttr->setIcon (attrIcon[options.color_index]);

  redrawMap (NVFalse);


  if (misc.abe_share->avShare.avRunning)
    {
      if (misc.avInput == AV_AOI)
        {
          DrawAVInterfaceBox (trackMap, &misc, avb.avInterfaceCenterLatLon);
          LoadShotsWithinAVBox (&misc, &options, trackMap, misc.avb.avInterfaceCenterMousePt);
        }	  
      else if (misc.avInput == AV_DIST_THRESH)
        {
          ComputeLineDistanceFilter (distLineStart, distLineEnd, &misc, &options);
        }
    }
}



void
pfmEdit3D::slotFlagMenu (QAction *action)
{
  for (NV_INT32 i = 0 ; i < PRE_USER + 5 ; i++)
    {
      if (action == flag[i])
        {
          options.flag_index = i;
          break;
        }
    }


  setFlags (&misc, &options);


  bFlag->setIcon (flagIcon[options.flag_index]);
  disconnect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));
  flag[0]->setChecked (TRUE);
  connect (flagGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFlagMenu (QAction *)));

  redrawMap (NVFalse);


  //  Even though we do this in redrawMap we need to do it again here because otherwise the damn markers disappear.
  //  It's OpenGL, I have no idea why.

  overlayData (map, &options, &misc);
}



//!  LIDAR tool bar button program starters.

void
pfmEdit3D::slotAttributeViewer (bool checked)
{
  //  Recalculate the pixel offset needed in the case the attribute viewer box is drawn.

  if (misc.hydro_lidar_present) misc.avb.avInterfacePixelBuffer = GetPixelOffsetByMeter (&misc, trackMap);


  if (checked)
    {
      if (!misc.abe_share->avShare.avRunning)
        {
          misc.abeShare->lock ();


          // only turn on CZMIL AV Tracking bit

          misc.abe_share->avShare.avTracking |= 0x01;

          misc.abeShare->unlock ();

          bDistanceThresh->setEnabled (TRUE);

          runProg (ATTRIBUTEVIEWER);


          //  Give it time to get running.

#ifdef NVWIN3X
          Sleep (1000);
#else
          sleep (1);
#endif


          //  Switch to DELETE_POINT mode.

          bDeletePoint->click ();


          //  If we are walking into a frozen situation we need to update our cursor position that we draw the attribute viewer around

          if (misc.marker_mode)
            {
              misc.avb.avInterfaceCenterLatLon.x = misc.data[misc.frozen_point].x;
              misc.avb.avInterfaceCenterLatLon.y = misc.data[misc.frozen_point].y;

              DrawAVInterfaceBox (trackMap, &misc, misc.avb.avInterfaceCenterLatLon);

              NV_FLOAT64 dz;
              NV_INT32 xyz_z;

              trackMap->map_to_screen (1, &misc.data[misc.frozen_point].x, &misc.data[misc.frozen_point].y, &dz, &misc.avb.avInterfaceCenterMousePt.x,
                                       &misc.avb.avInterfaceCenterMousePt.y, &xyz_z);
              LoadShotsWithinAVBox (&misc, &options, trackMap, misc.avb.avInterfaceCenterMousePt);
            }
        }
    }
  else
    {
      killProg (ATTRIBUTEVIEWER);


      misc.abeShare->lock ();


      // only kill CZMIL AV tracking bit

      misc.abe_share->avShare.avTracking &= 0xfe;


      // avRunning is now treated as a collection of 8 possible attribute viewers.
      //    Bit 0: CZMIL, the rest unused.
      //    Anding 1111 1110 to turn off bit 0

      misc.abe_share->avShare.avRunning &= 0xfe;


      misc.abeShare->unlock ();

      bDistanceThresh->setChecked (FALSE);
      bDistanceThresh->setEnabled (FALSE);


      // must account for the Distance Tool in the event that we close up the attribute viewer

      switch (options.function)
        {
        case DELETE_POINT:
          trackMap->closeMovingRectangle (&misc.avb.av_rectangle);
          break;

        case AV_DISTANCE_TOOL:
          map->discardRubberbandLine (&rb_dist);			  
          distLineStart.x = distLineStart.y = -1.0f;
          distLineEnd.x = distLineEnd.y = -1.0f;
          slotMode (DELETE_POINT);
          misc.avInput = AV_AOI;
          break;
        }
    }
}



void
pfmEdit3D::slotProg (int id)
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
pfmEdit3D::slotHideMenuShow ()
{
  //  We want to disable the "highlighted" button if there are no highlighted points and enable it if there are.

  if (misc.highlight_count)
    {
      hideAct[5]->setEnabled (TRUE);
    }
  else
    {
      hideAct[5]->setEnabled (FALSE);
    }
}



void
pfmEdit3D::slotHideMenu (QAction *action __attribute__ ((unused)))
{
  //  Make sure we're not doing something stupid like trying to hide every possible data type.

  if (misc.unique_count > 1)
    {
      NV_BOOL no_hit = NVTrue;
      for (NV_INT32 i = 0 ; i < misc.unique_count ; i++)
        {
          if (!hideAct[i + 6]->isChecked ())
            {
              no_hit = NVFalse;
            }
        }

      if (no_hit)
        {
          QMessageBox::warning (this, tr ("Hide Data Types"), 
                                tr ("You are trying to hide all possible data types.\nThis is not allowed."));

          slotMaskReset ();

          return;
        }
    }


  //  Clear the "show" buttons that match the "hide" buttons selected.

  disconnect (showGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotShowMenu (QAction *)));
  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      if (hideAct[i]->isChecked ()) showAct[i]->setChecked (FALSE);
    }
  if (misc.unique_count > 1)
    {
      for (NV_INT32 i = 0 ; i < misc.unique_count ; i++)
        {
          if (hideAct[i + 6]->isChecked ()) showAct[i + 6]->setChecked (FALSE);
        }
    }
  connect (showGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotShowMenu (QAction *)));


  //  IMPORTANT NOTE: Once we hide something we can't get it back until we reset or click a show button.
  //  This is so you can combine data type/flag hiding with masking.

  NV_U_INT32 flags[5] = {PFM_USER_01, PFM_USER_02, PFM_USER_03, PFM_USER_04, PFM_USER_05};
  NV_BOOL hide_flag[6];

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      if (hideAct[i]->isChecked ())
	{
	  hide_flag[i] = NVTrue;
          hideAct[i]->setEnabled (FALSE);
	}
      else
	{
	  hide_flag[i] = NVFalse;
	}
    }


  //  Check for hiding data types if we have more than one.

  NV_BOOL data_flag[PFM_DATA_TYPES];

  if (misc.unique_count > 1)
    {
      for (NV_INT32 i = 0 ; i < misc.unique_count ; i++)
        {
          if (hideAct[i + 6]->isChecked ())
            {
              data_flag[i] = NVTrue;
              hideAct[i + 6]->setEnabled (FALSE);
            }
          else
            {
              data_flag[i] = NVFalse;
            }
        }
    }


  //  Hide highlighted

  if (hide_flag[5])
    {
      for (NV_INT32 i = 0 ; i < misc.highlight_count ; i++)
        {
          misc.data[misc.highlight[i]].mask = NVTrue;
        }
    }


  //  Use misc.data[i].mask to hide the data

  for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++) 
    {
      for (NV_INT32 j = 0 ; j < 5 ; j++)
        {
          if (hide_flag[j])
            {
              if (misc.data[i].val & flags[j]) misc.data[i].mask = NVTrue;
            }
        }


      //  Hide by data type

      if (misc.unique_count > 1)
        {
          for (NV_INT32 j = 0 ; j < misc.unique_count ; j++)
            {
              if (data_flag[j])
                {
                  if (misc.data[i].type == misc.unique_type[j])
                    {
                      misc.data[i].mask = NVTrue;
                    }
                }
            }
        }
    }

  misc.mask_active = NVTrue;


  redrawMap (NVFalse);
}



void
pfmEdit3D::slotShowMenuShow ()
{
  //  We want to disable the "highlighted" button if there are no highlighted points and enable it if there are.

  if (misc.highlight_count)
    {
      showAct[5]->setEnabled (TRUE);
    }
  else
    {
      showAct[5]->setEnabled (FALSE);
    }
}



void
pfmEdit3D::slotShowMenu (QAction *action __attribute__ ((unused)))
{
  //  Make sure we're not doing something stupid like trying to not show anything.

  NV_BOOL no_hit = NVTrue;
  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      if (showAct[i]->isChecked ())
        {
          no_hit = NVFalse;
          break;
        }
    }

  if (no_hit && misc.unique_count > 1)
    {
      for (NV_INT32 i = 0 ; i < misc.unique_count ; i++)
        {
          if (showAct[i + 6]->isChecked ())
            {
              no_hit = NVFalse;
            }
        }
    }

  if (no_hit)
    {
      QMessageBox::warning (this, tr ("Show Data Types"), 
                            tr ("You are trying to show none of the possible data types.\nThis is not allowed."));

      slotMaskReset ();

      return;
    }


  //  Clear the "hide" buttons

  disconnect (hideGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotHideMenu (QAction *)));
  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      hideAct[i]->setChecked (FALSE);
      hideAct[i]->setEnabled (TRUE);
    }
  if (misc.unique_count > 1)
    {
      for (NV_INT32 i = 0 ; i < misc.unique_count ; i++)
        {
          hideAct[i + 6]->setChecked (FALSE);
          hideAct[i + 6]->setEnabled (TRUE);
        }
    }
  connect (hideGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotHideMenu (QAction *)));


  NV_U_INT32 flags[5] = {PFM_USER_01, PFM_USER_02, PFM_USER_03, PFM_USER_04, PFM_USER_05};
  NV_BOOL show_flag[6];

  for (NV_INT32 i = 0 ; i < 6 ; i++)
    {
      if (showAct[i]->isChecked ())
	{
	  show_flag[i] = NVTrue;
	}
      else
	{
	  show_flag[i] = NVFalse;
	}
    }


  //  Check for showing data types if we have more than one.

  NV_BOOL data_flag[PFM_DATA_TYPES];

  if (misc.unique_count > 1)
    {
      for (NV_INT32 i = 0 ; i < misc.unique_count ; i++)
        {
          if (showAct[i + 6]->isChecked ())
            {
              data_flag[i] = NVTrue;
            }
          else
            {
              data_flag[i] = NVFalse;
            }
        }
    }


  //  Use misc.data[i].mask to show the data

  for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++) 
    {
      NV_BOOL hit = NVFalse;
      for (NV_INT32 j = 0 ; j < 5 ; j++)
	{
	  if (show_flag[j] && (misc.data[i].val & flags[j]))
	    {
	      hit = NVTrue;
	      break;
	    }
	}


      //  Show highlighted

      if (show_flag[5])
        {
          for (NV_INT32 j = 0 ; j < misc.highlight_count ; j++)
            {
              if (misc.highlight[j] == i)
                {
                  hit = NVTrue;
                  break;
                }
            }
        }


      //  Show by data type

      if (!hit && misc.unique_count > 1)
        {
          for (NV_INT32 j = 0 ; j < misc.unique_count ; j++)
            {
              if (data_flag[j])
                {
                  if (misc.data[i].type == misc.unique_type[j])
                    {
                      hit = NVTrue;
                      break;
                    }
                }
            }
        }

      if (hit)
        {
          misc.data[i].mask = NVFalse;
        }
      else
        {
          misc.data[i].mask = NVTrue;
        }
    }


  misc.mask_active = NVTrue;


  redrawMap (NVFalse);
}



void 
pfmEdit3D::slotLayers ()
{
  manageLayers_dialog = new manageLayers (this, &options, &misc);

  connect (manageLayers_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotManageLayersDataChanged ()));
}



void 
pfmEdit3D::slotManageLayersDataChanged ()
{
  misc.abe_share->settings_changed = NVTrue;

  if (!misc.busy) redrawMap (NVTrue);
}



void 
pfmEdit3D::slotBrowserError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmEdit3D help browser"), tr ("Unable to start the browser process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmEdit3D help browser"), tr ("The browser process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmEdit3D help browser"), tr ("The browser process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmEdit3D help browser"), tr ("There was a write error to the browser process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmEdit3D help browser"), tr ("There was a read error from the browser process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmEdit3D help browser"), tr ("The browser process died with an unknown error!"));
      break;
    }
}



void
pfmEdit3D::extendedHelp ()
{
  if (getenv ("PFM_ABE") == NULL)
    {
      QMessageBox::warning (this, tr ("pfmEdit3D User Guide"), 
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
      QMessageBox::warning (this, tr ("pfmEdit3D User Guide"), 
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
          QMessageBox::warning (this, tr ("pfmEdit3D User Guide"), 
                                tr ("Unable to find the specified browser: ") + misc.help_browser);
          return;
        }

      hbi = QString (find_startup_name (hb));
    }


  if (!hbi.exists () || !hbi.isExecutable ())
    {
      QMessageBox::warning (this, tr ("pfmEdit3D User Guide"), 
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
pfmEdit3D::slotHotkeyHelp ()
{
  hotkeyHelp *hk = new hotkeyHelp (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  hk->move (x () + width () / 2 - hk->width () / 2, y () + height () / 2 - hk->height () / 2);

  hk->show ();
}


void
pfmEdit3D::slotToolbarHelp ()
{
  QWhatsThis::showText (QCursor::pos ( ), toolbarText, map);
}


void 
pfmEdit3D::slotFilter ()
{
  NV_BOOL filterPolyArea (OPTIONS *options, MISC *misc, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count);

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

  mx[0] = misc.abe_share->edit_area.min_x;
  my[0] = misc.abe_share->edit_area.min_y;
  mx[1] = misc.abe_share->edit_area.min_x;
  my[1] = misc.abe_share->edit_area.max_y;
  mx[2] = misc.abe_share->edit_area.max_x;
  my[2] = misc.abe_share->edit_area.max_y;
  mx[3] = misc.abe_share->edit_area.max_x;
  my[3] = misc.abe_share->edit_area.min_y;


  setMainButtons (NVFalse);


  filterPolyArea (&options, &misc, mx, my, 4);


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


  //  Force redrawing of the filter kill points

  overlayData (map, &options, &misc);
}


void 
pfmEdit3D::slotClearFilterMasks ()
{
  //  Clear any filter masked areas.
  
  for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++) misc.data[i].fmask = NVFalse;

  misc.filter_mask = NVFalse;


  //  If the filter message box is up then we are clearing the filter masks after the filter so we want to discard the current filter points
  //  then filter again.

  if (filter_active) slotFilter ();


  redrawMap (NVFalse);
}



void
pfmEdit3D::slotFilterStdChanged (int value)
{
  options.filterSTD = (NV_FLOAT32) value / 10.0;
  QString std;
  std.sprintf ("%0.1f", options.filterSTD);
  stdValue->setText (std);
}



void
pfmEdit3D::slotFilterStdReturn ()
{
  NV_FLOAT32 value = (NV_FLOAT32) stdValue->text ().toDouble ();
  NV_INT32 ivalue = NINT (value * 10.0);

  if (ivalue < stdSlider->minimum () || ivalue > stdSlider->maximum ()) ivalue = stdSlider->value ();

  stdSlider->setValue (ivalue);

  slotFilter ();

  map->setFocus (Qt::OtherFocusReason);
}



void
pfmEdit3D::slotFilterAccept ()
{
  //  Only do this if we're running the HOF wave filter, otherwise the sliders don't exist.

  if (hof_filter_active)
    {
      misc.busy = NVFalse;

      srSlider->setEnabled (TRUE);
      swSlider->setEnabled (TRUE);
      rtSlider->setEnabled (TRUE);
    }


  filter_active = NVFalse;
  attr_filter_active = NVFalse;
  hof_filter_active = NVFalse;


  //  Use PFM_MANUALLY_INVAL flag since the user has to confirm.

  for (NV_INT32 i = 0 ; i < misc.filter_kill_count ; i++)
    {
      store_undo (&misc, options.undo_levels, misc.data[misc.filter_kill_list[i]].val, misc.filter_kill_list[i]);
      misc.data[misc.filter_kill_list[i]].val |= PFM_MANUALLY_INVAL;
    }
  end_undo_block (&misc);
  misc.filtered = NVTrue;


  free (misc.filter_kill_list);
  misc.filter_kill_list = NULL;
  misc.filter_kill_count = 0;

  notebook->setTabEnabled (1, FALSE);
  notebook->setTabEnabled (2, FALSE);
  notebook->setTabEnabled (3, FALSE);

  notebook->setCurrentIndex (0);

  redrawMap (NVTrue);
}



void
pfmEdit3D::slotFilterReject ()
{
  //  Only do this if we're running the HOF wave filter, otherwise the sliders don't exist.

  if (hof_filter_active)
    {
      misc.busy = NVFalse;

      srSlider->setEnabled (TRUE);
      swSlider->setEnabled (TRUE);
      rtSlider->setEnabled (TRUE);
    }


  filter_active = NVFalse;
  attr_filter_active = NVFalse;
  hof_filter_active = NVFalse;

  free (misc.filter_kill_list);
  misc.filter_kill_list = NULL;
  misc.filter_kill_count = 0;


  notebook->setTabEnabled (1, FALSE);
  notebook->setTabEnabled (2, FALSE);
  notebook->setTabEnabled (3, FALSE);

  notebook->setCurrentIndex (0);

  redrawMap (NVTrue);
}



void 
pfmEdit3D::slotAttrFilter ()
{
  NV_INT32 minval, maxval;


  void attrFilter (OPTIONS *options, MISC *misc);


  //  Figure out the min and max values of the selected attribute.  Note that we don't filter on depth or line number.

  attr_filter_scale = 1.0;

  switch (options.color_index)
    {
    case 2:
      attr_filter_scale = misc.abe_share->open_args[0].head.horizontal_error_scale;
      attrFilterBox->setToolTip (tr ("Range filtering using horizontal error"));
      break;

    case 3:
      attr_filter_scale = misc.abe_share->open_args[0].head.vertical_error_scale;
      attrFilterBox->setToolTip (tr ("Range filtering using vertical error"));
      break;

    default:
      NV_INT32 ndx = options.color_index - PRE_ATTR;
      attr_filter_scale = misc.abe_share->open_args[0].head.ndx_attr_scale[ndx];
      attrFilterBox->setToolTip (tr ("Range filtering using ") + misc.abe_share->open_args[0].head.ndx_attr_name[ndx]);
      break;
    }


  minval = NINT (misc.attr_min * attr_filter_scale);
  maxval = NINT (misc.attr_max * attr_filter_scale);


  NV_INT32 range = abs (maxval - minval);
  NV_INT32 step = range / 10;
  if (!step) step = 1;


  //  Disconnect the sliders so setting them won't trigger the slots.

  disconnect (minSlider, SIGNAL (valueChanged (int)), this, SLOT (slotAttrFilterMinChanged (int)));
  disconnect (minSlider, SIGNAL (sliderReleased ()), this, SLOT (slotAttrFilter ()));
  disconnect (maxSlider, SIGNAL (valueChanged (int)), this, SLOT (slotAttrFilterMaxChanged (int)));
  disconnect (maxSlider, SIGNAL (sliderReleased ()), this, SLOT (slotAttrFilter ()));


  minSlider->setRange (minval, maxval);
  minSlider->setSingleStep (step);
  minSlider->setPageStep (step);

  NV_INT32 minvalue = NINT (options.attr_filter_range[options.color_index][0] * attr_filter_scale);

  if (minvalue < minval || minvalue > maxval)
    {
      minvalue = minval;
      options.attr_filter_range[options.color_index][0] = (NV_FLOAT32) minvalue / attr_filter_scale;
    }

  minSlider->setValue (minvalue);

  maxSlider->setRange (minval, maxval);
  maxSlider->setSingleStep (step);
  maxSlider->setPageStep (step);

  NV_INT32 maxvalue = NINT (options.attr_filter_range[options.color_index][1] * attr_filter_scale);
  if (maxvalue < minval || maxvalue > maxval)
    {
      maxvalue = maxval;
      options.attr_filter_range[options.color_index][1] = (NV_FLOAT32) maxvalue / attr_filter_scale;
    }

  maxSlider->setValue (maxvalue);

  QString min, max;
  if (options.color_index < PRE_ATTR)
    {
      min.sprintf ("%0.2f", options.attr_filter_range[options.color_index][0]);
      max.sprintf ("%0.2f", options.attr_filter_range[options.color_index][1]);
    }
  else
    {
      NV_INT32 ndx = options.color_index - PRE_ATTR;
      min.sprintf (misc.attr_format[0][ndx], options.attr_filter_range[options.color_index][0]);
      max.sprintf (misc.attr_format[0][ndx], options.attr_filter_range[options.color_index][1]);
    }
  minValue->setText (min);
  maxValue->setText (max);


  //  Now we can reconnect the signals and slots.

  connect (minSlider, SIGNAL (valueChanged (int)), this, SLOT (slotAttrFilterMinChanged (int)));
  connect (minSlider, SIGNAL (sliderReleased ()), this, SLOT (slotAttrFilter ()));
  connect (maxSlider, SIGNAL (valueChanged (int)), this, SLOT (slotAttrFilterMaxChanged (int)));
  connect (maxSlider, SIGNAL (sliderReleased ()), this, SLOT (slotAttrFilter ()));


  notebook->setTabEnabled (2, TRUE);
  notebook->setCurrentIndex (2);


  attr_filter_active = NVTrue;


  if (misc.filter_kill_list != NULL)
    {
      free (misc.filter_kill_list);
      misc.filter_kill_list = NULL;
      misc.filter_kill_count = 0;
    }


  setMainButtons (NVFalse);


  attrFilter (&options, &misc);


  if (misc.filter_kill_count == 1)
    {
      attrFilterBox->setTitle (tr ("1 point selected"));
    }
  else
    {
      attrFilterBox->setTitle (tr ("%1 points selected").arg (misc.filter_kill_count));
    }


  redrawMap (NVTrue);


  //  Force redrawing of the filter kill points

  overlayData (map, &options, &misc);
}



void
pfmEdit3D::slotAttrFilterMinChanged (int value)
{
  options.attr_filter_range[options.color_index][0] = (NV_FLOAT32) value / attr_filter_scale;

  QString min;

  if (options.color_index < PRE_ATTR)
    {
      min.sprintf ("%0.2f", options.attr_filter_range[options.color_index][0]);
    }
  else
    {
      NV_INT32 ndx = options.color_index - PRE_ATTR;
      min.sprintf (misc.attr_format[0][ndx], options.attr_filter_range[options.color_index][0]);
    }

  minValue->setText (min);
}



void
pfmEdit3D::slotAttrFilterMinReturn ()
{
  NV_FLOAT32 value = (NV_FLOAT32) minValue->text ().toDouble ();
  NV_INT32 ivalue = NINT (value * attr_filter_scale);

  if (ivalue < minSlider->minimum () || ivalue > minSlider->maximum ()) ivalue = minSlider->value ();

  minSlider->setValue (ivalue);

  slotAttrFilter ();

  map->setFocus (Qt::OtherFocusReason);
}



void
pfmEdit3D::slotAttrFilterMaxChanged (int value)
{
  options.attr_filter_range[options.color_index][1] = (NV_FLOAT32) value / attr_filter_scale;

  QString max;

  if (options.color_index < PRE_ATTR)
    {
      max.sprintf ("%0.2f", options.attr_filter_range[options.color_index][1]);
    }
  else
    {
      NV_INT32 ndx = options.color_index - PRE_ATTR;
      max.sprintf (misc.attr_format[0][ndx], options.attr_filter_range[options.color_index][1]);
    }

  maxValue->setText (max);
}



void
pfmEdit3D::slotAttrFilterMaxReturn ()
{
  NV_FLOAT32 value = (NV_FLOAT32) maxValue->text ().toDouble ();
  NV_INT32 ivalue = NINT (value * attr_filter_scale);

  if (ivalue < maxSlider->minimum () || ivalue > maxSlider->maximum ()) ivalue = maxSlider->value ();

  maxSlider->setValue (ivalue);

  slotAttrFilter ();

  map->setFocus (Qt::OtherFocusReason);
}



void 
pfmEdit3D::slotHOFWaveFilter ()
{
  //  Turn off slicing and clear line hiding if we need to.

  if (misc.slice) slotPopupMenu (popup[NUMPOPUPS - 1]);
  if (misc.num_lines) slotDisplayAll ();


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  hofFilterBox->setTitle (tr ("HOF Wave Filter"));


  //  Disconnect the sliders so setting them won't trigger the slots.

  disconnect (srSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterSrChanged (int)));
  disconnect (srSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
  disconnect (swSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterSwChanged (int)));
  disconnect (swSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
  disconnect (rtSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterRtChanged (int)));
  disconnect (rtSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
  disconnect (pmtSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterPMTChanged (int)));
  disconnect (pmtSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
  disconnect (apdSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterAPDChanged (int)));
  disconnect (apdSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));


  srSlider->setValue (NINT (misc.abe_share->filterShare.search_radius * 10.0));
  swSlider->setValue (misc.abe_share->filterShare.search_width);
  rtSlider->setValue (misc.abe_share->filterShare.rise_threshold);
  pmtSlider->setValue (misc.abe_share->filterShare.pmt_ac_zero_offset_required);
  apdSlider->setValue (misc.abe_share->filterShare.apd_ac_zero_offset_required);


  //  Now we can reconnect the signals and slots.

  connect (srSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterSrChanged (int)));
  connect (srSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
  connect (swSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterSwChanged (int)));
  connect (swSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
  connect (rtSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterRtChanged (int)));
  connect (rtSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
  connect (pmtSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterPMTChanged (int)));
  connect (pmtSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));
  connect (apdSlider, SIGNAL (valueChanged (int)), this, SLOT (slotHOFWaveFilterAPDChanged (int)));
  connect (apdSlider, SIGNAL (sliderReleased ()), this, SLOT (slotHOFWaveFilter ()));


  notebook->setTabEnabled (3, TRUE);
  notebook->setCurrentIndex (3);


  hof_filter_active = NVTrue;
  srSlider->setEnabled (FALSE);
  swSlider->setEnabled (FALSE);
  rtSlider->setEnabled (FALSE);


  if (misc.filter_kill_list != NULL)
    {
      free (misc.filter_kill_list);
      misc.filter_kill_list = NULL;
      misc.filter_kill_count = 0;
    }


  setMainButtons (NVFalse);

  misc.busy = NVTrue;

  NV_BOOL failed = NVFalse;
  exf = new externalFilter (this, map, &options, &misc, HOFWAVEFILTER, &failed);
  if (failed)
    {
      qApp->restoreOverrideCursor ();
      misc.busy = NVFalse;
      redrawMap (NVFalse);
    }
  else
    {
      connect (exf, SIGNAL (externalFilterDone ()), this, SLOT (slotExternalFilterDone ()));
    }

  map->setFocus (Qt::OtherFocusReason);
}



void
pfmEdit3D::slotHOFWaveFilterSrChanged (int value)
{
  options.hofWaveFilter_search_radius = misc.abe_share->filterShare.search_radius = (NV_FLOAT64) value / 10.0;

  QString sr;
  sr.sprintf ("%0.2f", options.hofWaveFilter_search_radius);

  srValue->setText (sr);
}



void
pfmEdit3D::slotHOFWaveFilterSrReturn ()
{
  NV_FLOAT32 value = (NV_FLOAT32) srValue->text ().toDouble ();
  NV_INT32 ivalue = NINT (value * 10.0);

  if (ivalue < srSlider->minimum () || ivalue > srSlider->maximum ()) ivalue = srSlider->value ();

  srSlider->setValue (ivalue);

  slotHOFWaveFilter ();

  map->setFocus (Qt::OtherFocusReason);
}



void
pfmEdit3D::slotHOFWaveFilterSwChanged (int value)
{
  options.hofWaveFilter_search_width = misc.abe_share->filterShare.search_width = value;

  QString sw;
  sw.sprintf ("%d", options.hofWaveFilter_search_width);

  swValue->setText (sw);
}



void
pfmEdit3D::slotHOFWaveFilterSwReturn ()
{
  NV_INT32 value = swValue->text ().toInt ();

  if (value < swSlider->minimum () || value > swSlider->maximum ()) value = swSlider->value ();

  swSlider->setValue (value);

  slotHOFWaveFilter ();

  map->setFocus (Qt::OtherFocusReason);
}



void
pfmEdit3D::slotHOFWaveFilterRtChanged (int value)
{
  options.hofWaveFilter_rise_threshold = misc.abe_share->filterShare.rise_threshold = value;

  QString rt;
  rt.sprintf ("%d", options.hofWaveFilter_rise_threshold);

  rtValue->setText (rt);
}



void
pfmEdit3D::slotHOFWaveFilterRtReturn ()
{
  NV_INT32 value = rtValue->text ().toInt ();

  if (value < rtSlider->minimum () || value > rtSlider->maximum ()) value = rtSlider->value ();

  rtSlider->setValue (value);

  slotHOFWaveFilter ();

  map->setFocus (Qt::OtherFocusReason);
}



void
pfmEdit3D::slotHOFWaveFilterPMTChanged (int value)
{
  options.hofWaveFilter_pmt_ac_zero_offset_required = misc.abe_share->filterShare.pmt_ac_zero_offset_required = value;

  QString pmt;
  pmt.sprintf ("%d", options.hofWaveFilter_pmt_ac_zero_offset_required);

  pmtValue->setText (pmt);
}



void
pfmEdit3D::slotHOFWaveFilterPMTReturn ()
{
  NV_INT32 value = pmtValue->text ().toInt ();

  if (value < pmtSlider->minimum () || value > pmtSlider->maximum ()) value = pmtSlider->value ();

  pmtSlider->setValue (value);

  slotHOFWaveFilter ();

  map->setFocus (Qt::OtherFocusReason);
}



void
pfmEdit3D::slotHOFWaveFilterAPDChanged (int value)
{
  options.hofWaveFilter_apd_ac_zero_offset_required = misc.abe_share->filterShare.apd_ac_zero_offset_required = value;

  QString apd;
  apd.sprintf ("%d", options.hofWaveFilter_apd_ac_zero_offset_required);

  apdValue->setText (apd);
}



void
pfmEdit3D::slotHOFWaveFilterAPDReturn ()
{
  NV_INT32 value = apdValue->text ().toInt ();

  if (value < apdSlider->minimum () || value > apdSlider->maximum ()) value = apdSlider->value ();

  apdSlider->setValue (value);

  slotHOFWaveFilter ();

  map->setFocus (Qt::OtherFocusReason);
}



void 
pfmEdit3D::slotExternalFilterDone ()
{
  qApp->restoreOverrideCursor ();
  qApp->processEvents ();

  srSlider->setEnabled (TRUE);
  swSlider->setEnabled (TRUE);
  rtSlider->setEnabled (TRUE);


  if (misc.filter_kill_count == 1)
    {
      hofFilterBox->setTitle (tr ("1 point selected"));
    }
  else
    {
      hofFilterBox->setTitle (tr ("%1 points selected").arg (misc.filter_kill_count));
    }


  redrawMap (NVTrue);
}



//!  Undo the last edit operation.

void 
pfmEdit3D::slotUndo ()
{
  undo (&misc);

  redrawMap (NVTrue);
}



//!  Kill ancillary programs.

NV_BOOL 
pfmEdit3D::killProg (int id)
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



//!  Run ancillary programs (not in hotkey polygon mode).

void 
pfmEdit3D::runProg (int id)
{
  //  We want to toggle external GUI programs that are capable of kill and respawn.

  NV_BOOL killed = killProg (id);


  //  If the program is not a "kill/respawn" GUI or it is and it wasn't killed immediately
  //  prior to this, we want to start the program.

  if (!options.state[id] || (options.state[id] == 1 && !killed))
    {
      ancillaryProgram[id] = new ancillaryProg (this, map, &options, &misc, id, lock_point);
      connect (ancillaryProgram[id], SIGNAL (redrawSignal ()), this, SLOT (slotRedrawMap ()));


      //  Reset our state to 1 for respawn programs.  We're using the 2 to determine if it was
      //  "respawned" when we started.  We'll set it back to 2 on exit if we still have
      //  options.kill_and_respawn set.

      if (options.state[id]) options.state[id] = 1;
    }
}



//!  Mouse press signal from the 2D trackMap map class.

void 
pfmEdit3D::slotTrackMousePress (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)))
{
  //  If we're drawing and a mouse button is pressed we want to quit drawing.

  if (misc.busy) 
    {
      misc.drawing_canceled = NVTrue;
      return;
    }  

  if (e->button () == Qt::LeftButton)
    {
      switch (options.function)
        {
          //  Placeholder
        }
    }

  if (e->button () == Qt::MidButton) {};
  if (e->button () == Qt::RightButton) {};
}



//!  Mouse release signal from the 2D trackMap map class.  Right now we don't really need this.

void 
pfmEdit3D::slotTrackMouseRelease (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton) {};
  if (e->button () == Qt::MidButton) {};
  if (e->button () == Qt::RightButton) {};
}



//!  Mouse move signal from the 2D trackMap map class.

void
pfmEdit3D::slotTrackMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)))
{
  NV_CHAR                   ltstring[25], lnstring[25], hem;
  NV_FLOAT64                deg, min, sec;
  NV_F64_COORD2             xy;
  QString                   y_string, x_string;
  BIN_RECORD                bin;
  NV_FLOAT64                dist;
  static NV_INT32           prev_nearest_point = -1;


  NV_BOOL compare_to_stack (NV_INT32 current_point, NV_FLOAT64 dist, MISC *misc);


  //  If it's still drawing don't do anything

  if (misc.busy) return;


  //  Let other ABE programs know which window we're in.  This is a special case since we have two map windows here.  We're lying about the 
  //  active window ID (process ID) by subtracting 1 from this program's PID.  This makes trackCursor track it locally.

  misc.abe_share->active_window_id = misc.process_id - 1;


  //  Get rid of the main cursor so we don't have multiple cursors on the main screen.

  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++) map->closeMovingList (&(multi_marker[i]));


  //  Track the cursor position for other ABE programs

  misc.abe_share->cursor_position.y = lat;
  misc.abe_share->cursor_position.x = lon;


  //  Try to find the highest layer with a valid value to display in the status bar.

  xy.y = lat;
  xy.x = lon;
  NV_INT32 hit = -1;

  for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
    {
      if (misc.abe_share->display_pfm[pfm] && bin_inside_ptr (&misc.abe_share->open_args[pfm].head, xy))
        {
          read_bin_record_xy (misc.pfm_handle[pfm], xy, &bin);

          hit = pfm;
          break;
        }
    }


  QString rec_string = "", sub_string = "", fil_string = "", pfm_string = "", lin_string = "", distance_string = "", hv_string = "";
  NV_CHAR nearest_line[512];


  switch (options.function)
    {
    case DELETE_POINT:
    case UNSET_SINGLE:
    case SET_MULTIPLE:
    case ADD_FEATURE:
    case EDIT_FEATURE:
    case MOVE_FEATURE:
    case DELETE_FEATURE:
    case MEASURE:

      //  We only change the status PFM values if we were actually over a PFM data area

      if (hit >= 0) pfmLabel->setText (QFileInfo (misc.abe_share->open_args[hit].list_path).fileName ().remove (".pfm"));


      //  Clear the nearest point stack.

      for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
        {
          misc.nearest_stack.point[i] = -1;
          misc.nearest_stack.dist[i] = 9999999999.0;
        }


      misc.nearest_point = -1;


      for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
        {
          //  Check for single line display.

          if (!misc.num_lines || check_line (&misc, misc.data[i].line))
            {
              //  Do not use null points.  Do not use invalid points unless the display_man_invalid, display_flt_invalid, or
              //  display_null flag is set.  Do not use masked points. Do not check points that are not on the display.

              if (!check_bounds (&options, &misc, i, NVTrue, misc.slice))
                {
                  dist = sqrt ((NV_FLOAT64) ((lat - misc.data[i].y) * (lat - misc.data[i].y)) +
                               (NV_FLOAT64) ((lon - misc.data[i].x) * (lon - misc.data[i].x)));


                  //  Check the points against the points in the nearest points stack.

                  if (compare_to_stack (i, dist, &misc))
                    {
                      //  If the return was true then this is the minimum distance so far.

                      misc.nearest_point = misc.nearest_stack.point[0];
                    }
                }
            }
        }


      //  Update the status bars

      // 
      //                      if our point has moved or we have just selected a new overplot we will go
      //                      into this code block.
      //

      if (misc.nearest_point != -1 && misc.nearest_point != prev_nearest_point)
        {
          //  Locking shared memory.

          misc.abeShare->lock ();


          //  External programs monitor abe_share->nearest_point to trigger updates of their windows.  They use
          //  abe_share->mwShare.multiRecord[0] for the current record to display.  In this way we can lock the
          //  main screen by not updating abe_share->mwShare.multiRecord[0] and still trigger the external programs
          //  to update (such as for multiple waveforms in CZMILwaveMonitor or waveWaterfall).

          NV_INT16 type;
          read_list_file (misc.pfm_handle[misc.data[misc.nearest_point].pfm], misc.data[misc.nearest_point].file, misc.abe_share->nearest_filename, &type);

          strcpy (nearest_line, read_line_file (misc.pfm_handle[misc.data[misc.nearest_point].pfm], misc.data[misc.nearest_point].line % SHARED_LINE_MULT));


          //  Push the nearest stack points into the ABE shared memory for use by CZMILwaveMonitor, waveWaterfall,
          //  and lidarMonitor.

          for (NV_INT32 j = 0 ; j < MAX_STACK_POINTS ; j++)
            {
              //  Normal, unfrozen mode.

              if (!misc.marker_mode)
                {
                  misc.abe_share->mwShare.multiSubrecord[j] = misc.data[misc.nearest_stack.point[j]].sub;
                  misc.abe_share->mwShare.multiPfm[j] = misc.data[misc.nearest_stack.point[j]].pfm;
                  misc.abe_share->mwShare.multiFile[j] = misc.data[misc.nearest_stack.point[j]].file;
                  misc.abe_share->mwShare.multiLine[j] = misc.data[misc.nearest_stack.point[j]].line % SHARED_LINE_MULT;
                  misc.abe_share->mwShare.multiPoint[j].x = misc.data[misc.nearest_stack.point[j]].x;
                  misc.abe_share->mwShare.multiPoint[j].y = misc.data[misc.nearest_stack.point[j]].y;
                  misc.abe_share->mwShare.multiPoint[j].z = misc.data[misc.nearest_stack.point[j]].z;
                  misc.abe_share->mwShare.multiLocation[j] = misc.nearest_stack.point[j];
                  misc.abe_share->mwShare.multiType[j] = misc.data[misc.nearest_stack.point[j]].type;
                  misc.abe_share->mwShare.multiPresent[j] = misc.data[misc.nearest_stack.point[j]].rec;
                  misc.abe_share->mwShare.multiRecord[j] = misc.data[misc.nearest_stack.point[j]].rec;
                  misc.abe_share->mwShare.multiFlightlineIndex[j] = GetFlightlineColorIndex (misc.abe_share->mwShare.multiLine[j],
                                                                                             misc.line_number, misc.line_count,
                                                                                             misc.line_interval);
                }


              //  If we're frozen, we don't want to replace the [0] record and we want to push the nearest
              //  MAX_STACK_POINTS - 1 records up one level.

              else
                {
                  if (j)
                    {
                      misc.abe_share->mwShare.multiSubrecord[j] = misc.data[misc.nearest_stack.point[j - 1]].sub;
                      misc.abe_share->mwShare.multiPfm[j] = misc.data[misc.nearest_stack.point[j - 1]].pfm;
                      misc.abe_share->mwShare.multiFile[j] = misc.data[misc.nearest_stack.point[j - 1]].file;
                      misc.abe_share->mwShare.multiLine[j] = misc.data[misc.nearest_stack.point[j - 1]].line % SHARED_LINE_MULT;
                      misc.abe_share->mwShare.multiPoint[j].x = misc.data[misc.nearest_stack.point[j - 1]].x;
                      misc.abe_share->mwShare.multiPoint[j].y = misc.data[misc.nearest_stack.point[j - 1]].y;
                      misc.abe_share->mwShare.multiPoint[j].z = misc.data[misc.nearest_stack.point[j - 1]].z;
                      misc.abe_share->mwShare.multiLocation[j] = misc.nearest_stack.point[j - 1];
                      misc.abe_share->mwShare.multiType[j] = misc.data[misc.nearest_stack.point[j - 1]].type;
                      misc.abe_share->mwShare.multiPresent[j] = misc.data[misc.nearest_stack.point[j - 1]].rec;
                      misc.abe_share->mwShare.multiRecord[j] = misc.data[misc.nearest_stack.point[j - 1]].rec;
                      misc.abe_share->mwShare.multiFlightlineIndex[j] = GetFlightlineColorIndex (misc.abe_share->mwShare.multiLine[j],
                                                                                                 misc.line_number, misc.line_count,
                                                                                                 misc.line_interval);
                    }
                }
            }
          misc.abe_share->mwShare.multiNum = MAX_STACK_POINTS;


          //  Trigger some of the external applications to update.

          misc.abe_share->nearest_point = misc.nearest_point;


          //  Unlock shared memory.

          misc.abeShare->unlock ();


          //  If we're working with features display the position of the feature instead of the nearest point.

          if ((options.function == EDIT_FEATURE || options.function == DELETE_FEATURE ||
               (options.function == MOVE_FEATURE && !move_feature)) && options.display_feature && 
              misc.bfd_header.number_of_records)
            {
              geo_xyz_label (misc.feature[misc.nearest_feature].latitude,  misc.feature[misc.nearest_feature].longitude,
                             misc.feature[misc.nearest_feature].depth);
            }
          else
            {
              geo_xyz_label (misc.data[misc.nearest_point].y,  misc.data[misc.nearest_point].x, misc.data[misc.nearest_point].z);
            }


          //  If we are frozen multiRecord[0] is useless to us, so we must check and use nearest point

          if (!misc.marker_mode)
            {
              rec_string.sprintf ("%d", misc.abe_share->mwShare.multiRecord[0]);
              sub_string.sprintf ("%d", misc.abe_share->mwShare.multiSubrecord[0]);

              pfm_string = QFileInfo (QString (misc.abe_share->open_args[misc.abe_share->mwShare.multiPfm[0]].list_path)).fileName ().remove (".pfm");
              pfmLabel->setToolTip (QString (misc.abe_share->open_args[misc.abe_share->mwShare.multiPfm[0]].list_path));
            }
          else
            {
              rec_string.sprintf ("%d", misc.data[misc.nearest_point].rec);
              sub_string.sprintf ("%d", misc.data[misc.nearest_point].sub);

              pfm_string = QFileInfo (QString (misc.abe_share->open_args[misc.data[misc.nearest_point].pfm].list_path)).fileName().remove(".pfm");
              pfmLabel->setToolTip (QString (misc.abe_share->open_args[misc.data[misc.nearest_point].pfm].list_path));
            }

          fil_string.sprintf ("%s", pfm_basename (misc.abe_share->nearest_filename));
          filLabel->setToolTip (QString (misc.abe_share->nearest_filename));
          lin_string.sprintf ("%s", nearest_line);


          recLabel->setText (rec_string);
          subLabel->setText (sub_string);
          filLabel->setText (fil_string);
          pfmLabel->setText (pfm_string);
          linLabel->setText (lin_string);
          typLabel->setText (PFM_data_type[misc.data[misc.nearest_point].type]);

          hv_string.sprintf ("%.2f / %.2f", misc.data[misc.nearest_point].herr, misc.data[misc.nearest_point].verr);
          hvLabel->setText (hv_string);


          NV_INT32 pfm = misc.data[misc.nearest_point].pfm;


          //  Check for the time, datum, and ellipsoid attributes from PFMWDB.  Embed the loop instead of the "if" so we only have
          //  to do one "if" statement instead of misc.max_attr "if" statements every time we move the mouse.

          if (PFMWDB_attr_count)
            {
              for (NV_INT32 i = 0 ; i < misc.max_attr ; i++)
                {
                  QString attr_string;

                  if (i == misc.time_attr)
                    {
                      if (misc.data[misc.nearest_point].attr[i] > PFMWDB_NULL_TIME)
                        {
                          NV_INT32 year, jday, hour, minute;
                          NV_FLOAT32 second;
                          time_t tv_sec = NINT (misc.data[misc.nearest_point].attr[i] * 60.0);

                          cvtime (tv_sec, 0, &year, &jday, &hour, &minute, &second);
                          attr_string.sprintf ("%d%03d%02d%02d", year + 1900, jday, hour, minute);
                        }
                      else
                        {
                          attr_string = "N/A";
                        }
                    }
                  else if (i == misc.datum_attr)
                    {
                      attr_string = shortVerticalDatum[NINT (misc.data[misc.nearest_point].attr[i])];
                    }
                  else if (i == misc.ellipsoid_attr)
                    {
                      if (misc.data[misc.nearest_point].attr[i] < PFMWDB_NULL_Z_OFFSET)
                        {
                          attr_string.sprintf (misc.attr_format[pfm][i], misc.data[misc.nearest_point].attr[i]);
                        }
                      else
                        {
                          attr_string = "N/A";
                        }
                    }
                  else
                    {
                      attr_string.sprintf (misc.attr_format[pfm][i], misc.data[misc.nearest_point].attr[i]);
                    }

                  attrLabel[i]->setText (attr_string);
                }
            }
          else
            {
              for (NV_INT32 i = 0 ; i < misc.max_attr ; i++)
                {
                  QString attr_string;

                  attr_string.sprintf (misc.attr_format[pfm][i], misc.data[misc.nearest_point].attr[i]);

                  attrLabel[i]->setText (attr_string);
                }
            }
        }


      //  This is the Attribute Viewer's functionality.
      //  Account for if the av interface box is frozen

      if (misc.abe_share->avShare.avRunning)
        {
          //  It has to be a normal mouse movement and not simulated

          if (!misc.marker_mode && misc.performingAction == AVA_NONE)
            {
              DrawAVInterfaceBox (trackMap, &misc, misc.abe_share->cursor_position);
            }
          else
            {
              DrawAVInterfaceBox (trackMap, &misc, misc.avb.avInterfaceCenterLatLon);
            }

          geo_xyz_label (misc.abe_share->cursor_position.y,  misc.abe_share->cursor_position.x, -misc.abe_share->cursor_position.z);

          if (!misc.marker_mode)
            {			
              //  It has to be a normal mouse movement and not simulated

              if (misc.nearest_point != prev_nearest_point && misc.performingAction != AVA_HIGHLIGHT)
                {
                  //  Move DrawAVInterfaceBox out so in the event with a new slotPreRedraw call
                  //  with a frozen cursor, the update will occur.

                  if (misc.hydro_lidar_present && misc.abe_share->avShare.avRunning)
                    {
                      if (misc.performingAction == AVA_NONE)
                        {
                          NV_I32_COORD2 hot2;
                          NV_FLOAT64 dz;
                          NV_INT32 xyz_z;

                          trackMap->map_to_screen (1, &misc.abe_share->cursor_position.x, &misc.abe_share->cursor_position.y, &dz, &hot2.x, &hot2.y, &xyz_z);

                          misc.avb.avInterfaceCenterMousePt = hot2;
                          misc.avb.avInterfaceCenterLatLon = misc.abe_share->cursor_position;
                        }

                      LoadShotsWithinAVBox (&misc, &options, trackMap, misc.avb.avInterfaceCenterMousePt);
                    }
                }
            }
        }
      break;


    case AV_DISTANCE_TOOL:

      // erase what you've started if you move to the track pane.

      if (map->rubberbandLineIsActive (rb_dist)) map->discardRubberbandLine(&rb_dist);

      break;
    }


  prev_nearest_point = misc.nearest_point;


  strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
  strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));

  latLabel->setText (ltstring);
  lonLabel->setText (lnstring);
}



//!  Resize signal from the 2D trackMap map class.

void
pfmEdit3D::slotTrackResize (QResizeEvent *e __attribute__ ((unused)))
{
}



//!  Prior to drawing coastline signal from the 2D trackMap map class.  This is where we do our heavy lifting.

void 
pfmEdit3D::slotTrackPreRedraw (NVMAP_DEF l_mapdef __attribute__ ((unused)))
{
  void scribe (nvMap *map, OPTIONS *options, MISC *misc, NV_FLOAT32 *ar, NV_INT32 ncc, NV_INT32 nrr, NV_FLOAT64 xorig, NV_FLOAT64 yorig);


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  NV_BOOL small_enough = NVTrue;
  NV_BOOL big_enough = NVTrue;


  NV_INT32 grid_height = NINT ((misc.displayed_area.max_y - misc.displayed_area.min_y) / misc.y_grid_size);
  NV_INT32 grid_width = NINT ((misc.displayed_area.max_x - misc.displayed_area.min_x) / misc.x_grid_size);


  //  Compute the adjusted min y and min x for the grid indexing.  This makes the grid an 
  //  exact number of grid spaces wide and high.

  NV_FLOAT64 adj_rymin = misc.displayed_area.max_y - ((NV_FLOAT32) grid_height * misc.y_grid_size);
  NV_FLOAT64 adj_rxmin = misc.displayed_area.max_x - ((NV_FLOAT32) grid_width * misc.x_grid_size);


  //  If the area to be binned is too big or small, flag it.

  if (grid_width * grid_height > 1000000) small_enough = NVFalse;
  if (grid_width * grid_height <= 0) big_enough = NVFalse;


  NV_FLOAT32 *grid = NULL;
  NV_U_INT16 *count = NULL;
  NV_INT32 length;
  NV_FLOAT32 numshades = (NV_FLOAT32) (NUMSHADES - 1);

  if (options.display_contours && small_enough && big_enough)
    {
      grid = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_height * grid_width);

      if (grid == NULL)
        {
          perror ("Allocating grid memory : ");
          exit (-1);
        }


      count = (NV_U_INT16 *) malloc (sizeof (NV_U_INT16) * grid_height * grid_width);

      if (count == NULL)
        {
          perror ("Allocating count memory : ");
          exit (-1);
        }

      for (NV_INT32 i = 0 ; i < grid_height * grid_width ; i++)
        {
          count[i] = 0;
          grid[i] = CHRTRNULL;
        }
    }


  for (NV_INT32 i = 0 ; i < misc.abe_share->point_cloud_count ; i++)
    {
      NV_INT32 xyz_x, xyz_y, xyz_z;
      NV_FLOAT64 dummy;
      if (!(misc.data[i].val & (PFM_DELETED | PFM_INVAL)))
        {
          //  Check for single line display.

          if (!misc.num_lines || check_line (&misc, misc.data[i].line))
            {
              //  Check against the viewing area.

              NV_INT32 trans = 0;
              if ((trans = check_bounds (&options, &misc, misc.data[i].x, misc.data[i].y, misc.data[i].z, misc.data[i].val, NVFalse, misc.data[i].pfm,
                                         NVTrue, NVFalse)) < 2)
                {
                  NV_INT32 c_index = NINT (numshades - ((-misc.data[i].z) - misc.color_min_z) / misc.color_range_z * numshades);


                  //  Check for out of range data.

                  if (c_index < 0) c_index = 0;
                  if (c_index > NUMSHADES - 1) c_index = NUMSHADES - 1;


                  QColor tmp = misc.track_color_array[c_index];

                  trackMap->map_to_screen (1, &misc.data[i].x, &misc.data[i].y, &dummy, &xyz_x, &xyz_y, &xyz_z);
                  trackMap->fillRectangle (xyz_x, xyz_y, 2, 2, tmp, NVFalse);


                  if (big_enough && small_enough && options.display_contours)
                    {
                      //  Load the grid (figure out which bin the point is in).

                      c_index = NINT ((NV_INT32) ((misc.data[i].y - adj_rymin) / misc.y_grid_size) * grid_width + (NV_INT32) ((misc.data[i].x - adj_rxmin) /
                                                                                                                               misc.x_grid_size));


                      //  Make sure there is no memory overrun.

                      if (c_index < grid_width * grid_height)
                        {
                          if (!count[c_index]) grid[c_index] = 0.0;
                          grid[c_index] = (grid[c_index] * (NV_FLOAT32) count[c_index] + (misc.data[i].z * options.z_factor + options.z_offset)) /
                            ((NV_FLOAT32) count[c_index] + 1.0);
                          count[c_index]++;
                        }
                    }
                }
            }
        }
    }


  // overwrite on top of that white dots indicating the shots within the AV Distance threshold

  if (options.function == AV_DISTANCE_TOOL)
    {
      NV_INT32 xyz_x, xyz_y, xyz_z;
      NV_FLOAT64 dummy;

      for (NV_INT32 i = 0; i < misc.av_dist_count; i++)
        {
          trackMap->map_to_screen (1, &misc.data[misc.av_dist_list[i]].x, &misc.data[misc.av_dist_list[i]].y, &dummy, &xyz_x, &xyz_y, &xyz_z);
          trackMap->fillRectangle (xyz_x, xyz_y, 1, 1, Qt::white, NVFalse);
        }
    }


  //  Plot contours on 2D tracker if requested.

  if (big_enough && small_enough && options.display_contours)
    {
      NV_FLOAT32 *spline_array_x_in = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_width);
      NV_FLOAT32 *spline_array_y_in = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_width);
      NV_FLOAT32 *spline_array_x_out = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_width);
      NV_FLOAT32 *spline_array_y_out = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * grid_width);
      if (spline_array_x_in == NULL || spline_array_y_in == NULL ||
          spline_array_x_out == NULL || spline_array_y_out == NULL)
        {
          perror ("Allocating spline memory : ");
          exit (-1);
        }


      //  Fill the spline arrays

      NV_INT32 k;
      for (NV_INT32 i = 0 ; i < grid_height ; i++)
        {
          k = 0;
          for (NV_INT32 j = 0 ; j < grid_width ; j++)
            {
              if (count[i * grid_width + j])
                {
                  //  Make sure there is no memory overrun.

                  if (k < grid_width)
                    {
                      spline_array_x_in[k] = (NV_FLOAT32) j;
                      spline_array_y_in[k] = grid[i * grid_width + j];
                      k++;
                    }
                }
            }


          //  Do not interpolate lines with fewer than 3 points so the slope doesn't go wacko.
                        
          if (k > 2)
            {
              interpolate (1.0, k, 0.0, grid_width - 1, &length, spline_array_x_in, spline_array_y_in, spline_array_x_out, spline_array_y_out);


              //  Set all points that do not have a 'real' neighbor within 3 bins to be CHRTRNULL.  
              //  This way they won't be contoured.

              NV_BOOL good_flag;
              for (NV_INT32 j = 0 ; j < length ; j++)
                {
                  good_flag = NVFalse;
                  for (NV_INT32 m = 0 ; m < k ; m++)
                    {
                      if (ABS (j - spline_array_x_in[m]) < 3)
                        {
                          good_flag = NVTrue;
                          break;
                        }
                    }

                  if (good_flag)
                    {
                      grid[i * grid_width + j] = spline_array_y_out[j];
                    }
                  else
                    {
                      grid[i * grid_width + j] = CHRTRNULL;
                    }
                }
            }
        }


      //  Contour it

      scribe (trackMap, &options, &misc, grid, grid_width, grid_height, adj_rxmin, adj_rymin);


      //  Free all of the memory.

      if (grid != NULL) free (grid);
      if (count != NULL) free (count);
      if (spline_array_x_in != NULL) free (spline_array_x_in);
      if (spline_array_y_in != NULL) free (spline_array_y_in);
      if (spline_array_x_out != NULL) free (spline_array_x_out);
      if (spline_array_y_out != NULL) free (spline_array_y_out);
    }

  trackMap->flush ();
}



//!  Post grid drawing signal from 2D trackMap map class.  Mostly just cleanup work.

void 
pfmEdit3D::slotTrackPostRedraw (NVMAP_DEF mapdef __attribute__ ((unused)))
{
  qApp->restoreOverrideCursor ();
}



//!  Clean exit routine

void 
pfmEdit3D::clean_exit (NV_INT32 ret)
{
  //  Let go of the shared memory.

  misc.dataShare->unlock ();
  misc.dataShare->detach ();
  misc.abeShare->detach ();


  //  Have to close the GL widget or it stays on screen in VirtualBox

  map->close ();


  exit (ret);
}



/*!
  - slot:			slotAVDistanceThresh

  - This slot will be fired when the AV Distance Threshold tool button is toggled
*/

void 
pfmEdit3D::slotAVDistanceThresh (bool checked)
{
  if (checked)
    {
      misc.avInput = AV_DIST_THRESH;
      slotMode (AV_DISTANCE_TOOL);
    }
  else
    {
      distLineStart.x = distLineStart.y = -1.0f;
      distLineEnd.x = distLineEnd.y = -1.0f;
      misc.avInput = AV_AOI;
      slotMode (DELETE_POINT);
    }
}



void 
pfmEdit3D::slotAVInterfaceChanged (NV_INT32 sizeOfBox)
{
  options.avInterfaceBoxSize = sizeOfBox;

  switch (sizeOfBox)
    {
    case 0:
      misc.avb.avBoxSize = 4.5f;
      break;

    case 1:
      misc.avb.avBoxSize = 12.5f;
      break;

    case 2:
      misc.avb.avBoxSize = 25.0f;
      break;
    }

  misc.avb.avInterfacePixelBuffer = GetPixelOffsetByMeter (&misc, trackMap);

  geo_xyz_label (menu_cursor_lat, menu_cursor_lon, -menu_cursor_z);
	     	
  DrawAVInterfaceBox (trackMap, &misc, misc.avb.avInterfaceCenterLatLon);
  LoadShotsWithinAVBox (&misc, &options, trackMap, misc.avb.avInterfaceCenterMousePt);
}



//!  Added function that checks to see if point marked for deletion in attribute viewer

void pfmEdit3D::checkForAVAction ()
{
  static AV_SHARE_ACTION avAction;
  avAction = misc.abe_share->avShare.action;
	
  if (avAction == AVA_NONE) return;

  if (avAction == AVA_HIGHLIGHT)
    {
      performAVActionHighlight ();
    }
  else if (avAction == AVA_DELETE)
    {
      performAVActionDelete ();
    }
  else if (avAction == AVA_COLOR_LINK)
    {
      performAVActionColorLink ();	
    }
}



//!  This function will take the index of the top location of the array and call slotMouseMove accordingly with the lat and lon of that index

void pfmEdit3D::performAVActionHighlight ()
{
  static NV_INT32 prevId = -1;
  static NV_INT32 id;
  static NV_INT32 frozenState;

  misc.abeShare->lock ();

  id = misc.abe_share->avShare.actionIdxs[0];

  if (prevId != id)
    {
      misc.performingAction = AVA_HIGHLIGHT;
      misc.abe_share->avShare.action = AVA_NONE;	


      //  We want to store the frozen state because we want the slotMouseMove traversal to be that of an unfrozen cursor.
      //  However, after the "simulated" mouse move, we restore the frozen state back to what it was

      frozenState = misc.marker_mode;

      if (misc.marker_mode)
        {
          misc.frozen_point = id;
          misc.nearest_point = id;


          //  Make the slotMouseMove traverse through unfrozen

          misc.marker_mode = 0;
        }
    }

  misc.abeShare->unlock ();


  if (prevId != id)
    {
      //  I need to take the path of a regular delete point mode for highlighting the cursor and to properly sync with the waveform monitor

      NV_BOOL wasInAVDistTool = NVFalse;

      if (options.function == AV_DISTANCE_TOOL)
        {
	  wasInAVDistTool = NVTrue;
          options.function = DELETE_POINT;
        }

      slotMouseMove ((QMouseEvent *) NULL, misc.data[id].x, misc.data[id].y, misc.data[id].z, mapdef);


      //  Restore the frozen state

      misc.marker_mode = frozenState;


      //  Minor bug that propagates to the waveform monitor in which if the attribute viewer is up and we are frozen in the editor,
      //  as we move our cursor along the editor, there will be a switch in the waveform viewer of 2 waveforms.

      if (misc.marker_mode)
        {
          misc.abeShare->lock ();
          misc.abe_share->mwShare.multiPresent[0] = 0;
          misc.abeShare->unlock ();
        }

      prevId = id;
      misc.performingAction = AVA_NONE;
    }	
}



/*!
  - method:			performAVActionColorLink

  - This method will look at the index coming in which is the user-defined attribute index
    that was selected in the AV.  The color code by... option matching this index will be
    selected which will cause the AV to change it's color scheme to match.
*/

void pfmEdit3D::performAVActionColorLink ()
{
  NV_INT32 userDefinedIndex;

  misc.abeShare->lock ();

  userDefinedIndex = misc.abe_share->avShare.actionIdxs[0];
  misc.abe_share->avShare.action = AVA_NONE;

  misc.abeShare->unlock ();


  // simulate color code click

  slotAttrMenu (attr[PRE_ATTR + userDefinedIndex]);
}



//!  This handles a delete action coming over from AV 

void pfmEdit3D::performAVActionDelete()
{
  misc.abeShare->lock ();

  NV_INT32 numToDelete = misc.abe_share->avShare.numActionIdxs - 1, *idxs = misc.abe_share->avShare.actionIdxs;

  for (NV_INT32 i = 0 ; i < numToDelete ; i++)
    {
      store_undo (&misc, options.undo_levels, misc.data[idxs[i]].val, idxs[i]);
      misc.data[idxs[i]].val |= PFM_MANUALLY_INVAL;
    }
  end_undo_block (&misc);

  misc.abe_share->avShare.action = AVA_NONE;
  misc.performingAction = AVA_DELETE;


  //  If we are frozen and anything gets deleted we become unfrozen

  NV_INT32 frozenState = misc.marker_mode;

  if (misc.marker_mode) misc.marker_mode = 0;


  //  We want to set the nearest point to a neighboring shot within the AV ROI box.  This neighboring shot is passed over through
  //  shared memory.  If we have deleted the last shot within the ROI box, we set misc.performingAction to AVA_NONE so slotMouseMove
  //  will find the nearest located shot to place the primary cursor upon.

  if (numToDelete < (MAX_ATTRIBUTE_SHOTS -1) && idxs[numToDelete] != -1)
    {
      misc.nearest_point = idxs[numToDelete];
    }
  else
    { 
      misc.performingAction = AVA_NONE;
      misc.av_dist_count = 0;
      misc.abe_share->avShare.numShots = 0;
      misc.abe_share->avShare.avNewData = NVTrue;
    }


  misc.abeShare->unlock ();

  trackMap->redrawMap (NVTrue);
  redrawMap (NVTrue);


  //  We pass in the lat/lon of the next nearest sorted shot from within AV
	
  slotMouseMove ((QMouseEvent *) NULL, misc.data[misc.nearest_point].x, misc.data[misc.nearest_point].y, misc.data[misc.nearest_point].z, mapdef);
		

  if (misc.performingAction == AVA_DELETE) misc.marker_mode = frozenState;
  misc.performingAction = AVA_NONE;
}



void
pfmEdit3D::about ()
{
  QMessageBox::about (this, VERSION, pfmEdit3DAboutText);
}



void
pfmEdit3D::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}



void
pfmEdit3D::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}
