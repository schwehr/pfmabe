
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



//  pfmView class.

/***************************************************************************/
/*!

   - Module :        pfmView

   - Programmer :    Jan C. Depner

   - Date :          01/31/05

   - Purpose :       C++/Qt replacement for C/Motif Area-Based Editor viewer.
                     This program is the PFM surface viewer for the Area-Based
                     Editor.  It is the driver for the actual point editors,
                     pfmEdit and pfmEdit3D.  It is also the driver for the
                     3D PFM surface viewer, pfm3D, and the mosaic viewer,
                     mosaicView.

\***************************************************************************/
    
#include "pfmView.hpp"
#include "pfmViewHelp.hpp"

#include <getopt.h>

pfmView::pfmView (int *argc, char **argv, QWidget *parent):
  QMainWindow (parent, 0)
{
  NV_BOOL envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);
  void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore);
  void open_pfm_files (MISC *misc, NV_CHAR *file, NV_F64_XYMBR *bounds);


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set a few defaults for startup

  pfm_edit_active = NVFalse;
  threeD_edit = NVFalse;
  cov = NULL;
  browserProc = NULL;
  mosaicProc = NULL;
  threeDProc = NULL;
  prefs_dialog = NULL;
  prev_poly_lat = -91.0;
  prev_poly_lon = -181.0;
  popup_active = NVFalse;
  cov_popup_active = NVFalse;
  double_click = NVFalse;
  pfmEditMod = 0;
  pfmEditFilt = 0;
  mv_marker = -1;
  mv_tracker = -1;
  mv_arrow = -1;
  mv_rectangle = -1;
  pfm3D_polygon = -1;
  rb_rectangle = -1;
  rb_polygon = -1;
  cov_rb_rectangle = -1;
  cov_mv_rectangle = -1;
  force_3d_edit = NVFalse;
  need_redraw = NVFalse;
  unloadProc = NULL;
  unloadMsg = "";
  unload_count = 0;
  unload_type = 1;
  pfmEdit_stopped = NVFalse;
  pfmWDBView = NVFalse;
  pfmViewMod = NVFalse;
  logo_first = NVTrue;
  recent_file_flag = 0;
  command_file_flag = NVFalse;
  process_running = NVFalse;
  redraw_count = 0;
  command_line_mbr.max_x = 999.0;


  //  Check to see if we have the (SAIC proprietary) cube program available.

#ifdef NVWIN3X
  strcpy (cube_name, "navo_pfm_cube.exe");
#else
  strcpy (cube_name, "navo_pfm_cube");
#endif


  misc.cube_available = NVTrue;
  if (find_startup_name (cube_name) == NULL) misc.cube_available = NVFalse;

  for (NV_INT32 pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++)
    {
      misc.average_type[pfm] = 0;
      misc.cube_attr_available[pfm] = NVFalse;
      misc.last_saved_contour_record[pfm] = 0;
      misc.pfm_alpha[pfm] = 255;
    }


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


  //  Make the "arrow" painter path.

  arrow = QPainterPath ();

  arrow.moveTo (0, 0);
  arrow.moveTo (5, 25);
  arrow.lineTo (5, -15);
  arrow.lineTo (10, -15);
  arrow.lineTo (0, -25);
  arrow.lineTo (-10, -15);
  arrow.lineTo (-5, -15);
  arrow.lineTo (-5, 25);
  arrow.lineTo (5, 25);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfm_abe.xpm"));


  //  Set up the cursors.

  zoomCursor = QCursor (QPixmap (":/icons/zoom_cursor.xpm"), 11, 11);
  editFeatureCursor = QCursor (QPixmap (":/icons/edit_feature_cursor.xpm"), 15, 15);
  deleteFeatureCursor = QCursor (QPixmap (":/icons/delete_feature_cursor.xpm"), 15, 15);
  addFeatureCursor = QCursor (QPixmap (":/icons/add_feature_cursor.xpm"), 15, 15);
  editRectCursor = QCursor (QPixmap (":/icons/edit_rect_cursor.xpm"), 1, 1);
  editPolyCursor = QCursor (QPixmap (":/icons/edit_poly_cursor.xpm"), 1, 1);
  filterRectCursor = QCursor (QPixmap (":/icons/filter_rect_cursor.xpm"), 1, 1);
  filterPolyCursor = QCursor (QPixmap (":/icons/filter_poly_cursor.xpm"), 1, 1);
  filterMaskRectCursor = QCursor (QPixmap (":/icons/filter_mask_rect_cursor.xpm"), 1, 1);
  filterMaskPolyCursor = QCursor (QPixmap (":/icons/filter_mask_poly_cursor.xpm"), 1, 1);
  remispCursor = QCursor (QPixmap (":/icons/remisp_cursor.xpm"), 1, 1);
  drawContourCursor = QCursor (QPixmap (":/icons/draw_contour_cursor.xpm"), 1, 1);
  pencilCursor = QCursor (QPixmap (":/icons/pencil_cursor.xpm"), 10, 26);
  highCursor = QCursor (QPixmap (":/icons/high_cursor.xpm"), 1, 1);
  lowCursor = QCursor (QPixmap (":/icons/low_cursor.xpm"), 1, 1);
  grabContourCursor = QCursor (QPixmap (":/icons/grab_contour_cursor.xpm"), 5, 1);



  //  Set all of the defaults

  set_defaults (&misc, &options, NVFalse);


  //  We have to get the icon size out of sequence (the rest of the options are read in env_in_out.cpp)
  //  so that we'll have the proper sized icons for the toolbars.  Otherwise, they won't be placed correctly.

  QSettings settings (misc.qsettings_org, misc.qsettings_app);
  settings.beginGroup (misc.qsettings_app);
  options.main_button_icon_size = settings.value (tr ("main button icon size"), options.main_button_icon_size).toInt ();
  settings.endGroup ();


  //  Create the Edit toolbar.  Hopefully, all of this toolbar setup stuff is intuitively obvious to the most
  //  casual observer ;-)

  toolBar[0] = new QToolBar (tr ("Edit tool bar"));
  toolBar[0]->setToolTip (tr ("Edit tool bar"));
  addToolBar (toolBar[0]);
  toolBar[0]->setObjectName (tr ("pfmView Edit tool bar"));

  bOpen = new QToolButton (this);
  bOpen->setIcon (QIcon (":/icons/fileopen.xpm"));
  bOpen->setToolTip (tr ("Open PFM file"));
  bOpen->setWhatsThis (openText);
  connect (bOpen, SIGNAL (clicked ()), this, SLOT (slotOpen ()));
  toolBar[0]->addWidget (bOpen);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  QButtonGroup *editModeGrp = new QButtonGroup (this);
  connect (editModeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotEditMode (int)));
  editModeGrp->setExclusive (TRUE);

  bEditRect = new QToolButton (this);
  bEditRect->setIcon (QIcon (":/icons/edit_rect.xpm"));
  bEditRect->setToolTip (tr ("Set rectangle edit mode"));
  bEditRect->setWhatsThis (editRectText);
  editModeGrp->addButton (bEditRect, RECT_EDIT_AREA);
  bEditRect->setCheckable (TRUE);
  toolBar[0]->addWidget (bEditRect);

  bEditPoly = new QToolButton (this);
  bEditPoly->setIcon (QIcon (":/icons/edit_poly.xpm"));
  bEditPoly->setToolTip (tr ("Set polygon edit mode"));
  bEditPoly->setWhatsThis (editPolyText);
  editModeGrp->addButton (bEditPoly, POLY_EDIT_AREA);
  bEditPoly->setCheckable (TRUE);
  toolBar[0]->addWidget (bEditPoly);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  bEditRect3D = new QToolButton (this);
  bEditRect3D->setIcon (QIcon (":/icons/edit_rect3D.xpm"));
  bEditRect3D->setToolTip (tr ("Set 3D rectangle edit mode"));
  bEditRect3D->setWhatsThis (editRectText);
  editModeGrp->addButton (bEditRect3D, RECT_EDIT_AREA_3D);
  bEditRect3D->setCheckable (TRUE);
  toolBar[0]->addWidget (bEditRect3D);

  bEditPoly3D = new QToolButton (this);
  bEditPoly3D->setIcon (QIcon (":/icons/edit_poly3D.xpm"));
  bEditPoly3D->setToolTip (tr ("Set 3D polygon edit mode"));
  bEditPoly3D->setWhatsThis (editPolyText);
  editModeGrp->addButton (bEditPoly3D, POLY_EDIT_AREA_3D);
  bEditPoly3D->setCheckable (TRUE);
  toolBar[0]->addWidget (bEditPoly3D);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  bFilterRect = new QToolButton (this);
  bFilterRect->setIcon (QIcon (":/icons/filter_rect.xpm"));
  bFilterRect->setToolTip (tr ("Filter a rectangular area"));
  bFilterRect->setWhatsThis (filterRectText);
  editModeGrp->addButton (bFilterRect, RECT_FILTER_AREA);
  bFilterRect->setCheckable (TRUE);
  toolBar[0]->addWidget (bFilterRect);

  bFilterPoly = new QToolButton (this);
  bFilterPoly->setIcon (QIcon (":/icons/filter_poly.xpm"));
  bFilterPoly->setToolTip (tr ("Filter a polygonal area"));
  bFilterPoly->setWhatsThis (filterPolyText);
  editModeGrp->addButton (bFilterPoly, POLY_FILTER_AREA);
  bFilterPoly->setCheckable (TRUE);
  toolBar[0]->addWidget (bFilterPoly);

  bFilterRectMask = new QToolButton (this);
  bFilterRectMask->setIcon (QIcon (":/icons/filter_mask_rect.xpm"));
  bFilterRectMask->setToolTip (tr ("Mask a rectangular area from the filter"));
  bFilterRectMask->setWhatsThis (filterRectMaskText);
  editModeGrp->addButton (bFilterRectMask, RECT_FILTER_MASK);
  bFilterRectMask->setCheckable (TRUE);
  toolBar[0]->addWidget (bFilterRectMask);

  bFilterPolyMask = new QToolButton (this);
  bFilterPolyMask->setIcon (QIcon (":/icons/filter_mask_poly.xpm"));
  bFilterPolyMask->setToolTip (tr ("Mask a polygonal area from the filter"));
  bFilterPolyMask->setWhatsThis (filterPolyMaskText);
  editModeGrp->addButton (bFilterPolyMask, POLY_FILTER_MASK);
  bFilterPolyMask->setCheckable (TRUE);
  toolBar[0]->addWidget (bFilterPolyMask);


  bClearMasks = new QToolButton (this);
  bClearMasks->setIcon (QIcon (":/icons/clear_filter_masks.xpm"));
  bClearMasks->setToolTip (tr ("Clear filter masks"));
  bClearMasks->setWhatsThis (clearMasksText);
  connect (bClearMasks, SIGNAL (clicked ()), this, SLOT (slotClearFilterMasks ()));
  toolBar[0]->addWidget (bClearMasks);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  QMenu *checkMenu = new QMenu (this);

  QActionGroup *checkGrp = new QActionGroup (this);
  connect (checkGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotCheckMenu (QAction *)));

  setChecked = checkMenu->addAction (tr ("Set displayed area checked"));
  setUnchecked = checkMenu->addAction (tr ("Set displayed area unchecked"));
  setVerified = checkMenu->addAction (tr ("Set displayed area verified"));
  setUnverified = checkMenu->addAction (tr ("Set displayed area unverified"));
  checkGrp->addAction (setChecked);
  checkGrp->addAction (setUnchecked);
  checkGrp->addAction (setVerified);
  checkGrp->addAction (setUnverified);

  bSetChecked = new QToolButton (this);
  bSetChecked->setIcon (QIcon (":/icons/setchecked.xpm"));
  bSetChecked->setToolTip (tr ("Set displayed area (un)checked/verified"));
  bSetChecked->setWhatsThis (setCheckedText);
  bSetChecked->setPopupMode (QToolButton::InstantPopup);
  bSetChecked->setMenu (checkMenu);
  toolBar[0]->addWidget (bSetChecked);

  QMenu *unloadMenu = new QMenu (this);

  QActionGroup *unloadGrp = new QActionGroup (this);
  connect (unloadGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotUnloadMenu (QAction *)));

  unloadDisplayed = unloadMenu->addAction (tr ("Unload edits for displayed area"));
  unloadFile = unloadMenu->addAction (tr ("Unload edits for entire PFM"));
  unloadGrp->addAction (unloadDisplayed);
  unloadGrp->addAction (unloadFile);

  bUnload = new QToolButton (this);
  bUnload->setIcon (QIcon (":/icons/unload.xpm"));
  bUnload->setToolTip (tr ("Unload edits for displayed area or entire PFM"));
  bUnload->setWhatsThis (unloadText);
  bUnload->setPopupMode (QToolButton::InstantPopup);
  bUnload->setMenu (unloadMenu);
  toolBar[0]->addWidget (bUnload);


  //  Create the View toolbar

  toolBar[1] = new QToolBar (tr ("View tool bar"));
  toolBar[1]->setToolTip (tr ("View tool bar"));
  addToolBar (toolBar[1]);
  toolBar[1]->setObjectName (tr ("pfmView View tool bar"));

  bStop = new QToolButton (this);
  bStop->setIcon (QIcon (":/icons/stop.xpm"));
  bStop->setToolTip (tr ("Stop drawing"));
  bStop->setWhatsThis (stopText);
  bStop->setEnabled (FALSE);
  connect (bStop, SIGNAL (clicked ()), this, SLOT (slotStop ()));
  toolBar[1]->addWidget (bStop);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bZoomIn = new QToolButton (this);
  bZoomIn->setIcon (QIcon (":/icons/icon_zoomin.xpm"));
  bZoomIn->setToolTip (tr ("Zoom in"));
  bZoomIn->setWhatsThis (zoomInText);
  connect (bZoomIn, SIGNAL (clicked ()), this, SLOT (slotZoomIn ()));
  toolBar[1]->addWidget (bZoomIn);

  bZoomOut = new QToolButton (this);
  bZoomOut->setIcon (QIcon (":/icons/icon_zoomout.xpm"));
  bZoomOut->setToolTip (tr ("Zoom out"));
  bZoomOut->setWhatsThis (zoomOutText);
  connect (bZoomOut, SIGNAL (clicked ()), this, SLOT (slotZoomOut ()));
  toolBar[1]->addWidget (bZoomOut);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bRedraw = new QToolButton (this);
  misc.button[REDRAW_KEY] = bRedraw;
  bRedraw->setIcon (misc.buttonIcon[REDRAW_KEY]);
  bRedraw->setWhatsThis (redrawText);
  connect (bRedraw, SIGNAL (clicked ()), this, SLOT (slotRedraw ()));
  toolBar[1]->addWidget (bRedraw);

  bRedrawCoverage = new QToolButton (this);
  misc.button[REDRAW_COVERAGE_KEY] = bRedrawCoverage;
  bRedrawCoverage->setIcon (misc.buttonIcon[REDRAW_COVERAGE_KEY]);
  bRedrawCoverage->setWhatsThis (redrawCoverageText);
  connect (bRedrawCoverage, SIGNAL (clicked ()), this, SLOT (slotRedrawCoverage ()));
  toolBar[1]->addWidget (bRedrawCoverage);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bContour = new QToolButton (this);
  misc.button[TOGGLE_CONTOUR_KEY] = bContour;
  bContour->setIcon (misc.buttonIcon[TOGGLE_CONTOUR_KEY]);
  bContour->setWhatsThis (contourText);
  bContour->setCheckable (TRUE);
  bContour->setEnabled (FALSE);
  toolBar[1]->addWidget (bContour);

  bCoast = new QToolButton (this);
  bCoast->setIcon (QIcon (":/icons/coast.xpm"));
  bCoast->setToolTip (tr ("Toggle coastline display"));
  bCoast->setWhatsThis (coastText);
  bCoast->setCheckable (TRUE);
  bCoast->setEnabled (FALSE);
  toolBar[1]->addWidget (bCoast);

  bMask = new QToolButton (this);
  bMask->setIcon (QIcon (":/icons/landmask.xpm"));
  bMask->setToolTip (tr ("Toggle land mask display"));
  bMask->setWhatsThis (maskText);
  bMask->setCheckable (TRUE);
  bMask->setEnabled (FALSE);
  toolBar[1]->addWidget (bMask);

  bSunangle = new QToolButton (this);
  bSunangle->setIcon (QIcon (":/icons/sunangle.xpm"));
  bSunangle->setToolTip (tr ("Select sun angle"));
  bSunangle->setWhatsThis (sunangleText);
  bSunangle->setEnabled (FALSE);
  connect (bSunangle, SIGNAL (clicked ()), this, SLOT (slotSunangle ()));
  toolBar[1]->addWidget (bSunangle);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bGeotiff = new QToolButton (this);
  misc.button[TOGGLE_GEOTIFF_KEY] = bGeotiff;
  bGeotiff->setIcon (misc.buttonIcon[TOGGLE_GEOTIFF_KEY]);
  bGeotiff->setWhatsThis (geoText);
  connect (bGeotiff, SIGNAL (clicked ()), this, SLOT (slotGeotiff ()));
  toolBar[1]->addWidget (bGeotiff);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bAutoRedraw = new QToolButton (this);
  bAutoRedraw->setIcon (QIcon (":/icons/autoredraw.xpm"));
  bAutoRedraw->setToolTip (tr ("Auto redraw after edit"));
  bAutoRedraw->setWhatsThis (autoRedrawText);
  bAutoRedraw->setCheckable (TRUE);
  toolBar[1]->addWidget (bAutoRedraw);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bDisplayMinMax = new QToolButton (this);
  bDisplayMinMax->setIcon (QIcon (":/icons/display_min_max.xpm"));
  bDisplayMinMax->setToolTip (tr ("Flag minimum and maximum bins"));
  bDisplayMinMax->setWhatsThis (displayMinMaxText);
  bDisplayMinMax->setCheckable (TRUE);
  toolBar[1]->addWidget (bDisplayMinMax);


  QMenu *highlightMenu = new QMenu (this);

  QActionGroup *highlightGrp = new QActionGroup (this);
  connect (highlightGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotHighlightMenu (QAction *)));

  for (NV_INT32 i = 0 ; i < NUM_HIGHLIGHTS ; i++)
    {
      switch (i)
        {
        case H_NONE:
          highlightIcon[i] = QIcon (":/icons/highlight_none.xpm");
          break;

        case H_ALL:
          highlightIcon[i] = QIcon (":/icons/highlight_all.xpm");
          break;

        case H_CHECKED:
          highlightIcon[i] = QIcon (":/icons/highlight_checked.xpm");
          break;

        case H_01:
          highlightIcon[i] = QIcon (":/icons/highlight01.xpm");
          break;

        case H_02:
          highlightIcon[i] = QIcon (":/icons/highlight02.xpm");
          break;

        case H_03:
          highlightIcon[i] = QIcon (":/icons/highlight03.xpm");
          break;

        case H_04:
          highlightIcon[i] = QIcon (":/icons/highlight04.xpm");
          break;

        case H_05:
          highlightIcon[i] = QIcon (":/icons/highlight05.xpm");
          break;

        case H_INT:
          highlightIcon[i] = QIcon (":/icons/highlight_int.xpm");
          break;

        case H_MULT:
          highlightIcon[i] = QIcon (":/icons/highlight_mult.xpm");
          break;

        case H_COUNT:
          highlightIcon[i] = QIcon (":/icons/highlight_count.xpm");
          break;

        case H_IHO_S:
          highlightIcon[i] = QIcon (":/icons/highlight_IHO_S.xpm");
          break;

        case H_IHO_1:
          highlightIcon[i] = QIcon (":/icons/highlight_IHO_1.xpm");
          break;

        case H_IHO_2:
          highlightIcon[i] = QIcon (":/icons/highlight_IHO_2.xpm");
          break;

        case H_PERCENT:
          highlightIcon[i] = QIcon (":/icons/highlight_percent.xpm");
          break;
        }
    }

  highlightNone = highlightMenu->addAction (tr ("No highlighting"));
  highlightNone->setIcon (highlightIcon[H_NONE]);
  highlightAll = highlightMenu->addAction (tr ("Highlight all"));
  highlightAll->setIcon (highlightIcon[H_ALL]);
  highlightChecked = highlightMenu->addAction (tr ("Highlight checked/verified data"));
  highlightChecked->setIcon (highlightIcon[H_CHECKED]);
  highlightUser01 = highlightMenu->addAction (tr ("Highlight PFM_USER_01 data"));
  highlightUser01->setIcon (highlightIcon[H_01]);
  highlightUser02 = highlightMenu->addAction (tr ("Highlight PFM_USER_02 data"));
  highlightUser02->setIcon (highlightIcon[H_02]);
  highlightUser03 = highlightMenu->addAction (tr ("Highlight PFM_USER_03 data"));
  highlightUser03->setIcon (highlightIcon[H_03]);
  highlightUser04 = highlightMenu->addAction (tr ("Highlight PFM_USER_04 data"));
  highlightUser04->setIcon (highlightIcon[H_04]);
  highlightUser05 = highlightMenu->addAction (tr ("Highlight PFM_USER_05 data"));
  highlightUser05->setIcon (highlightIcon[H_05]);
  highlightInt = highlightMenu->addAction (tr ("Highlight interpolated (MISP) data"));
  highlightInt->setIcon (highlightIcon[H_INT]);
  highlightMenu->addSeparator ();
  highlightMult = highlightMenu->addAction (tr ("Highlight multiple coverage (>= 200%) areas"));
  highlightMult->setIcon (highlightIcon[H_MULT]);
  highlightCount = highlightMenu->addAction (tr ("Highlight valid bin count exceeding 00"));
  highlightCount->setIcon (highlightIcon[H_COUNT]);
  highlightIHOS = highlightMenu->addAction (tr ("Highlight IHO Special Order"));
  highlightIHOS->setIcon (highlightIcon[H_IHO_S]);
  highlightIHO1 = highlightMenu->addAction (tr ("Highlight IHO Order 1"));
  highlightIHO1->setIcon (highlightIcon[H_IHO_1]);
  highlightIHO2 = highlightMenu->addAction (tr ("Highlight IHO Order 2"));
  highlightIHO2->setIcon (highlightIcon[H_IHO_2]);
  highlightPercent = highlightMenu->addAction (tr ("Highlight 10 percent of depth"));
  highlightPercent->setIcon (highlightIcon[H_PERCENT]);
  highlightGrp->addAction (highlightNone);
  highlightGrp->addAction (highlightAll);
  highlightGrp->addAction (highlightChecked);
  highlightGrp->addAction (highlightUser01);
  highlightGrp->addAction (highlightUser02);
  highlightGrp->addAction (highlightUser03);
  highlightGrp->addAction (highlightUser04);
  highlightGrp->addAction (highlightUser05);
  highlightGrp->addAction (highlightInt);
  highlightGrp->addAction (highlightMult);
  highlightGrp->addAction (highlightCount);
  highlightGrp->addAction (highlightIHOS);
  highlightGrp->addAction (highlightIHO1);
  highlightGrp->addAction (highlightIHO2);
  highlightGrp->addAction (highlightPercent);

  bHighlight = new QToolButton (this);
  bHighlight->setIcon (highlightIcon[H_NONE]);
  bHighlight->setToolTip (tr ("Toggle highlight options"));
  bHighlight->setWhatsThis (highlightText);
  bHighlight->setPopupMode (QToolButton::InstantPopup);
  bHighlight->setMenu (highlightMenu);
  toolBar[1]->addWidget (bHighlight);


  //  Create the Color toolbar

  toolBar[2] = new QToolBar (tr ("Color tool bar"));
  toolBar[2]->setToolTip (tr ("Color tool bar"));
  addToolBar (toolBar[2]);
  toolBar[2]->setObjectName (tr ("pfmView Color tool bar"));

  colorGrp = new QButtonGroup (this);
  colorGrp->setExclusive (TRUE);


  for (NV_INT32 i = 0 ; i < NUM_ATTR + PRE_ATTR ; i++)
    {
      switch (i)
        {
        case 0:
          colorIcon[i] = QIcon (":/icons/color_by_depth.xpm");
          colorText[i] = tr ("Color by depth");
          break;

        case 1:
          colorIcon[i] = QIcon (":/icons/number.xpm");
          colorText[i] = tr ("Color by number of soundings");
          break;

        case 2:
          colorIcon[i] = QIcon (":/icons/stddev.xpm");
          colorText[i] = tr ("Color by standard deviation");
          break;

        case 3:
          colorIcon[i] = QIcon (":/icons/avg_min_diff.xpm");
          colorText[i] = tr ("Color by average minus minimum");
          break;

        case 4:
          colorIcon[i] = QIcon (":/icons/max_avg_diff.xpm");
          colorText[i] = tr ("Color by maximum minus average");
          break;

        case 5:
          colorIcon[i] = QIcon (":/icons/max_min_diff.xpm");
          colorText[i] = tr ("Color by maximum minus minimum");
          break;

        case PRE_ATTR + 0:
          colorIcon[i] = QIcon (":/icons/attr01.xpm");
          colorText[i] = tr ("Color by attribute 1");
          break;

        case PRE_ATTR + 1:
          colorIcon[i] = QIcon (":/icons/attr02.xpm");
          colorText[i] = tr ("Color by attribute 2");
          break;
      
        case PRE_ATTR + 2:
          colorIcon[i] = QIcon (":/icons/attr03.xpm");
          colorText[i] = tr ("Color by attribute 3");
          break;

        case PRE_ATTR + 3:
          colorIcon[i] = QIcon (":/icons/attr04.xpm");
          colorText[i] = tr ("Color by attribute 4");
          break;

        case PRE_ATTR + 4:
          colorIcon[i] = QIcon (":/icons/attr05.xpm");
          colorText[i] = tr ("Color by attribute 5");
          break;

        case PRE_ATTR + 5:
          colorIcon[i] = QIcon (":/icons/attr06.xpm");
          colorText[i] = tr ("Color by attribute 6");
          break;

        case PRE_ATTR + 6:
          colorIcon[i] = QIcon (":/icons/attr07.xpm");
          colorText[i] = tr ("Color by attribute 7");
          break;

        case PRE_ATTR + 7:
          colorIcon[i] = QIcon (":/icons/attr08.xpm");
          colorText[i] = tr ("Color by attribute 8");
          break;

        case PRE_ATTR + 8:
          colorIcon[i] = QIcon (":/icons/attr09.xpm");
          colorText[i] = tr ("Color by attribute 9");
          break;

        case PRE_ATTR + 9:
          colorIcon[i] = QIcon (":/icons/attr10.xpm");
          colorText[i] = tr ("Color by attribute 10");
          break;
        }


      bColor[i] = new QToolButton (this);
      bColor[i]->setIcon (colorIcon[i]);
      bColor[i]->setToolTip (colorText[i]);
      colorGrp->addButton (bColor[i], i);
      bColor[i]->setCheckable (TRUE);
      bColorAction[i] = toolBar[2]->addWidget (bColor[i]);

      if (i >= PRE_ATTR) bColorAction[i]->setVisible (FALSE);
    }


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();

  bStoplight = new QToolButton (this);
  bStoplight->setIcon (QIcon (":/icons/stoplight.png"));
  bStoplight->setWhatsThis (stoplightText);
  bStoplight->setCheckable (TRUE);
  bStoplight->setEnabled (FALSE);
  toolBar[2]->addWidget (bStoplight);


  //  Create the Surface toolbar

  toolBar[3] = new QToolBar (tr ("Surface tool bar"));
  toolBar[3]->setToolTip (tr ("Surface tool bar"));
  addToolBar (toolBar[3]);
  toolBar[3]->setObjectName (tr ("pfmView Surface tool bar"));


  surfaceGrp = new QButtonGroup (this);
  surfaceGrp->setExclusive (TRUE);
  connect (surfaceGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotSurface (int)));


  bSurface[0] = new QToolButton (this);
  misc.button[DISPLAY_AVG_EDITED_KEY] = bSurface[0];
  bSurface[0]->setIcon (misc.buttonIcon[DISPLAY_AVG_EDITED_KEY]);
  bSurface[0]->setWhatsThis (avgFiltText);
  surfaceGrp->addButton (bSurface[0], AVERAGE_FILTERED_DEPTH);
  bSurface[0]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[0]);

  bSurface[1] = new QToolButton (this);
  misc.button[DISPLAY_MIN_EDITED_KEY] = bSurface[1];
  bSurface[1]->setIcon (misc.buttonIcon[DISPLAY_MIN_EDITED_KEY]);
  bSurface[1]->setWhatsThis (minFiltText);
  surfaceGrp->addButton (bSurface[1], MIN_FILTERED_DEPTH);
  bSurface[1]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[1]);

  bSurface[2] = new QToolButton (this);
  misc.button[DISPLAY_MAX_EDITED_KEY] = bSurface[2];
  bSurface[2]->setIcon (misc.buttonIcon[DISPLAY_MAX_EDITED_KEY]);
  bSurface[2]->setWhatsThis (maxFiltText);
  surfaceGrp->addButton (bSurface[2], MAX_FILTERED_DEPTH);
  bSurface[2]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[2]);

  bSurface[3] = new QToolButton (this);
  misc.button[DISPLAY_AVG_UNEDITED_KEY] = bSurface[3];
  bSurface[3]->setIcon (misc.buttonIcon[DISPLAY_AVG_UNEDITED_KEY]);
  bSurface[3]->setWhatsThis (avgText);
  surfaceGrp->addButton (bSurface[3], AVERAGE_DEPTH);
  bSurface[3]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[3]);

  bSurface[4] = new QToolButton (this);
  misc.button[DISPLAY_MIN_UNEDITED_KEY] = bSurface[4];
  bSurface[4]->setIcon (misc.buttonIcon[DISPLAY_MIN_UNEDITED_KEY]);
  bSurface[4]->setWhatsThis (minText);
  surfaceGrp->addButton (bSurface[4], MIN_DEPTH);
  bSurface[4]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[4]);

  bSurface[5] = new QToolButton (this);
  misc.button[DISPLAY_MAX_UNEDITED_KEY] = bSurface[5];
  bSurface[5]->setIcon (misc.buttonIcon[DISPLAY_MAX_UNEDITED_KEY]);
  bSurface[5]->setWhatsThis (maxText);
  surfaceGrp->addButton (bSurface[5], MAX_DEPTH);
  bSurface[5]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[5]);


  toolBar[3]->addSeparator ();
  toolBar[3]->addSeparator ();


  bSurface[6] = new QToolButton (this);
  bSurface[6]->setIcon (QIcon (":/icons/avgfilt_otf.xpm"));
  bSurface[6]->setToolTip (tr ("Display the average edited depth surface in on-the-fly grid"));
  bSurface[6]->setWhatsThis (avgFiltOtfText);
  surfaceGrp->addButton (bSurface[6], AVERAGE_FILTERED_DEPTH + OTF_OFFSET);
  bSurface[6]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[6]);

  bSurface[7] = new QToolButton (this);
  misc.button[DISPLAY_MIN_EDITED_OTF_KEY] = bSurface[7];
  bSurface[7]->setIcon (misc.buttonIcon[DISPLAY_MIN_EDITED_OTF_KEY]);
  bSurface[7]->setWhatsThis (minFiltOtfText);
  surfaceGrp->addButton (bSurface[7], MIN_FILTERED_DEPTH + OTF_OFFSET);
  bSurface[7]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[7]);

  bSurface[8] = new QToolButton (this);
  misc.button[DISPLAY_MAX_EDITED_OTF_KEY] = bSurface[8];
  bSurface[8]->setIcon (misc.buttonIcon[DISPLAY_MAX_EDITED_OTF_KEY]);
  bSurface[8]->setWhatsThis (maxFiltOtfText);
  surfaceGrp->addButton (bSurface[8], MAX_FILTERED_DEPTH + OTF_OFFSET);
  bSurface[8]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[8]);

  bSurface[9] = new QToolButton (this);
  bSurface[9]->setIcon (QIcon (":/icons/avg_otf.xpm"));
  bSurface[9]->setToolTip (tr ("Display the average unedited depth surface in on-the-fly grid"));
  bSurface[9]->setWhatsThis (avgOtfText);
  surfaceGrp->addButton (bSurface[9], AVERAGE_DEPTH + OTF_OFFSET);
  bSurface[9]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[9]);

  bSurface[10] = new QToolButton (this);
  bSurface[10]->setIcon (QIcon (":/icons/min_otf.xpm"));
  bSurface[10]->setToolTip (tr ("Display the minimum unedited depth surface in on-the-fly grid"));
  bSurface[10]->setWhatsThis (minOtfText);
  surfaceGrp->addButton (bSurface[10], MIN_DEPTH + OTF_OFFSET);
  bSurface[10]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[10]);

  bSurface[11] = new QToolButton (this);
  bSurface[11]->setIcon (QIcon (":/icons/max_otf.xpm"));
  bSurface[11]->setToolTip (tr ("Display the maximum unedited depth surface in on-the-fly grid"));
  bSurface[11]->setWhatsThis (maxOtfText);
  surfaceGrp->addButton (bSurface[11], MAX_DEPTH + OTF_OFFSET);
  bSurface[11]->setCheckable (TRUE);
  toolBar[3]->addWidget (bSurface[11]);

  bSetOtfBin = new QToolButton (this);
  bSetOtfBin->setIcon (QIcon (":/icons/set_otf_size.xpm"));
  bSetOtfBin->setWhatsThis (setOtfBinText);
  connect (bSetOtfBin, SIGNAL (clicked ()), this, SLOT (slotSetOtfBin ()));
  toolBar[3]->addWidget (bSetOtfBin);


  //  Create the Flag Data toolbar

  toolBar[4] = new QToolBar (tr ("Flag Data tool bar"));
  toolBar[4]->setToolTip (tr ("Flag Data tool bar"));
  addToolBar (toolBar[4]);
  toolBar[4]->setObjectName (tr ("pfmView Flag Data tool bar"));

  bDisplaySuspect = new QToolButton (this);
  bDisplaySuspect->setIcon (QIcon (":/icons/displaysuspect.xpm"));
  bDisplaySuspect->setToolTip (tr ("Flag suspect data"));
  bDisplaySuspect->setWhatsThis (displaySuspectText);
  bDisplaySuspect->setCheckable (TRUE);
  toolBar[4]->addWidget (bDisplaySuspect);

  bDisplaySelected = new QToolButton (this);
  bDisplaySelected->setIcon (QIcon (":/icons/displayselected.xpm"));
  bDisplaySelected->setToolTip (tr ("Flag selected data"));
  bDisplaySelected->setWhatsThis (displaySelectedText);
  bDisplaySelected->setCheckable (TRUE);
  toolBar[4]->addWidget (bDisplaySelected);

  bDisplayReference = new QToolButton (this);
  bDisplayReference->setIcon (QIcon (":/icons/displayreference.xpm"));
  bDisplayReference->setToolTip (tr ("Flag reference data"));
  bDisplayReference->setWhatsThis (displayReferenceText);
  bDisplayReference->setCheckable (TRUE);
  toolBar[4]->addWidget (bDisplayReference);


  //  Create the Feature toolbar

  toolBar[5] = new QToolBar (tr ("Feature tool bar"));
  toolBar[5]->setToolTip (tr ("Feature tool bar"));
  addToolBar (toolBar[5]);
  toolBar[5]->setObjectName (tr ("pfmView Feature tool bar"));

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
  bDisplayFeature->setToolTip (tr ("Select type of feature data to display"));
  bDisplayFeature->setWhatsThis (displayFeatureText);
  bDisplayFeature->setPopupMode (QToolButton::InstantPopup);
  bDisplayFeature->setMenu (featureMenu);
  toolBar[5]->addWidget (bDisplayFeature);

  bDisplayChildren = new QToolButton (this);
  bDisplayChildren->setIcon (QIcon (":/icons/displayfeaturechildren.xpm"));
  bDisplayChildren->setToolTip (tr ("Display feature group members"));
  bDisplayChildren->setWhatsThis (displayChildrenText);
  bDisplayChildren->setCheckable (TRUE);
  toolBar[5]->addWidget (bDisplayChildren);

  bDisplayFeatureInfo = new QToolButton (this);
  bDisplayFeatureInfo->setIcon (QIcon (":/icons/displayfeatureinfo.xpm"));
  bDisplayFeatureInfo->setToolTip (tr ("Display feature information"));
  bDisplayFeatureInfo->setWhatsThis (displayFeatureInfoText);
  bDisplayFeatureInfo->setCheckable (TRUE);
  toolBar[5]->addWidget (bDisplayFeatureInfo);

  bDisplayFeaturePoly = new QToolButton (this);
  bDisplayFeaturePoly->setIcon (QIcon (":/icons/displayfeaturepoly.xpm"));
  bDisplayFeaturePoly->setToolTip (tr ("Display feature polygonal area"));
  bDisplayFeaturePoly->setWhatsThis (displayFeaturePolyText);
  bDisplayFeaturePoly->setCheckable (TRUE);
  toolBar[5]->addWidget (bDisplayFeaturePoly);

  bAddFeature = new QToolButton (this);
  bAddFeature->setIcon (QIcon (":/icons/addfeature.xpm"));
  bAddFeature->setToolTip (tr ("Add a feature"));
  bAddFeature->setWhatsThis (addFeatureText);
  editModeGrp->addButton (bAddFeature, ADD_FEATURE);
  bAddFeature->setCheckable (TRUE);
  toolBar[5]->addWidget (bAddFeature);

  bDeleteFeature = new QToolButton (this);
  bDeleteFeature->setIcon (QIcon (":/icons/deletefeature.xpm"));
  bDeleteFeature->setToolTip (tr ("Delete a feature"));
  bDeleteFeature->setWhatsThis (deleteFeatureText);
  editModeGrp->addButton (bDeleteFeature, DELETE_FEATURE);
  bDeleteFeature->setCheckable (TRUE);
  toolBar[5]->addWidget (bDeleteFeature);

  bEditFeature = new QToolButton (this);
  bEditFeature->setIcon (QIcon (":/icons/editfeature.xpm"));
  bEditFeature->setToolTip (tr ("Edit a feature"));
  bEditFeature->setWhatsThis (editFeatureText);
  editModeGrp->addButton (bEditFeature, EDIT_FEATURE);
  bEditFeature->setCheckable (TRUE);
  toolBar[5]->addWidget (bEditFeature);

  bVerifyFeatures = new QToolButton (this);
  bVerifyFeatures->setIcon (QIcon (":/icons/verify_features.png"));
  bVerifyFeatures->setToolTip (tr ("Verify all visible valid features"));
  bVerifyFeatures->setWhatsThis (verifyFeaturesText);
  connect (bVerifyFeatures, SIGNAL (clicked ()), this, SLOT (slotVerifyFeatures ()));
  toolBar[5]->addWidget (bVerifyFeatures);


  //  Create the MISP/CUBE toolbar

  toolBar[6] = new QToolBar (tr ("MISP/CUBE tool bar"));
  toolBar[6]->setToolTip (tr ("MISP/CUBE tool bar"));
  addToolBar (toolBar[6]);
  toolBar[6]->setObjectName (tr ("pfmView MISP/CUBE tool bar"));

  bDrawContour = new QToolButton (this);
  bDrawContour->setIcon (QIcon (":/icons/draw_contour.xpm"));
  bDrawContour->setToolTip (tr ("Draw and insert a contour"));
  bDrawContour->setWhatsThis (drawContourText);
  editModeGrp->addButton (bDrawContour, DRAW_CONTOUR);
  bDrawContour->setCheckable (TRUE);
  bDrawContour->setEnabled (FALSE);
  toolBar[6]->addWidget (bDrawContour);

  bGrabContour = new QToolButton (this);
  bGrabContour->setIcon (QIcon (":/icons/grab_contour.xpm"));
  bGrabContour->setToolTip (tr ("Capture and insert interpolated contours from a polygon"));
  bGrabContour->setWhatsThis (grabContourText);
  editModeGrp->addButton (bGrabContour, GRAB_CONTOUR);
  bGrabContour->setCheckable (TRUE);
  bGrabContour->setEnabled (FALSE);
  toolBar[6]->addWidget (bGrabContour);

  bRemisp = new QToolButton (this);
  bRemisp->setIcon (QIcon (":/icons/remisp_icon.xpm"));
  bRemisp->setToolTip (tr ("Regrid a rectangular area"));
  bRemisp->setWhatsThis (remispText);
  editModeGrp->addButton (bRemisp, REMISP_AREA);
  bRemisp->setCheckable (TRUE);
  bRemisp->setEnabled (FALSE);
  toolBar[6]->addWidget (bRemisp);


  toolBar[6]->addSeparator ();
  toolBar[6]->addSeparator ();


  bDrawContourFilter = new QToolButton (this);
  bDrawContourFilter->setIcon (QIcon (":/icons/draw_contour_filter.xpm"));
  bDrawContourFilter->setToolTip (tr ("Draw and insert a contour for filtering the surface"));
  bDrawContourFilter->setWhatsThis (drawContourFilterText);
  editModeGrp->addButton (bDrawContourFilter, DRAW_CONTOUR_FILTER);
  bDrawContourFilter->setCheckable (TRUE);
  bDrawContourFilter->setEnabled (FALSE);
  toolBar[6]->addWidget (bDrawContourFilter);

  bRemispFilter = new QToolButton (this);
  bRemispFilter->setIcon (QIcon (":/icons/remisp_filter_icon.xpm"));
  bRemispFilter->setToolTip (tr ("Regrid and filter a polygonal area"));
  bRemispFilter->setWhatsThis (remispFilterText);
  editModeGrp->addButton (bRemispFilter, REMISP_FILTER);
  bRemispFilter->setCheckable (TRUE);
  bRemispFilter->setEnabled (FALSE);
  toolBar[6]->addWidget (bRemispFilter);


  bClearFilterContours = new QToolButton (this);
  bClearFilterContours->setIcon (QIcon (":/icons/clear_filter_contours.xpm"));
  bClearFilterContours->setToolTip (tr ("Clear filter contours"));
  bClearFilterContours->setWhatsThis (clearFilterContoursText);
  connect (bClearFilterContours, SIGNAL (clicked ()), this, SLOT (slotClearFilterContours ()));
  toolBar[6]->addWidget (bClearFilterContours);


  toolBar[6]->addSeparator ();
  toolBar[6]->addSeparator ();


  bHigh = new QToolButton (this);
  bHigh->setIcon (QIcon (":/icons/high.xpm"));
  bHigh->setToolTip (tr ("Select the highest point within a rectangle"));
  bHigh->setWhatsThis (highText);
  editModeGrp->addButton (bHigh, SELECT_HIGH_POINT);
  bHigh->setCheckable (TRUE);
  bHigh->setEnabled (FALSE);
  toolBar[6]->addWidget (bHigh);

  bLow = new QToolButton (this);
  bLow->setIcon (QIcon (":/icons/low.xpm"));
  bLow->setToolTip (tr ("Select the lowest point within a rectangle"));
  bLow->setWhatsThis (lowText);
  editModeGrp->addButton (bLow, SELECT_LOW_POINT);
  bLow->setCheckable (TRUE);
  bLow->setEnabled (FALSE);
  toolBar[6]->addWidget (bLow);

  bCube = new QToolButton (this);
  bCube->setIcon (QIcon (":/icons/cube.xpm"));
  bCube->setToolTip (tr ("Re-CUBE the displayed area"));
  bCube->setWhatsThis (cubeText);
  connect (bCube, SIGNAL (clicked ()), this, SLOT (slotCube ()));
  bCube->setEnabled (FALSE);
  toolBar[6]->addWidget (bCube);


  //  Create the Utilities toolbar
 
  toolBar[7] = new QToolBar (tr ("Utilities tool bar"));
  toolBar[7]->setToolTip (tr ("Utilities tool bar"));
  addToolBar (toolBar[7]);
  toolBar[7]->setObjectName (tr ("pfmView Utilities tool bar"));

  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.xpm"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  toolBar[7]->addWidget (bPrefs);


  toolBar[7]->addSeparator ();
  toolBar[7]->addSeparator ();


  bLink = new QToolButton (this);
  bLink->setIcon (QIcon (":/icons/unlink.xpm"));
  bLink->setToolTip (tr ("Connect to other ABE applications"));
  bLink->setWhatsThis (linkText);
  connect (bLink, SIGNAL (clicked ()), this, SLOT (slotLink ()));
  toolBar[7]->addWidget (bLink);


  toolBar[7]->addSeparator ();
  toolBar[7]->addSeparator ();


  bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  toolBar[7]->addAction (bHelp);


  //  Set the icon sizes so that the tool bars will set up properly.

  prev_icon_size = options.main_button_icon_size;

  QSize mainButtonIconSize (options.main_button_icon_size, options.main_button_icon_size);

  for (NV_INT32 i = 0 ; i < NUM_TOOLBARS ; i++) toolBar[i]->setIconSize (mainButtonIconSize);


  //  Get the user's defaults if available

  if (!envin (&options, &misc, this))
    {
      //  Set the geometry from defaults since we didn't get any from the saved settings.

      this->resize (misc.width, misc.height);
      this->move (misc.window_x, misc.window_y);
    }


  //  Get the command line arguments.  We might be specifying the HSV min and max color values on the command line
  //  and we want those to override what's in the QSettings (from envin).

  extern char *optarg;
  extern int optind;
  NV_INT32 option_index = 0;

  while (NVTrue) 
    {
      static struct option long_options[] = {{"min_hsv_color", required_argument, 0, 0},
                                             {"max_hsv_color", required_argument, 0, 0},
                                             {"min_hsv_value", required_argument, 0, 0},
                                             {"max_hsv_value", required_argument, 0, 0},
                                             {"area_file", required_argument, 0, 0},
                                             {"nsew", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              sscanf (optarg, "%hd", &options.min_hsv_color[0]);
              break;

            case 1:
              sscanf (optarg, "%hd", &options.max_hsv_color[0]);
              break;

            case 2:
              sscanf (optarg, "%f", &options.min_hsv_value[0]);
              options.min_hsv_locked[0] = NVTrue;
              break;

            case 3:
              sscanf (optarg, "%f", &options.max_hsv_value[0]);
              options.max_hsv_locked[0] = NVTrue;
              break;

            case 4:
              NV_INT32 polygon_count;
              NV_FLOAT64 polygon_x[200], polygon_y[200];
              get_area_mbr (optarg, &polygon_count, polygon_x, polygon_y, &command_line_mbr);
              break;

            case 5:
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
            }
          break;
        }
    }


  //  Check the min and max colors and flip them if needed.

  if (options.max_hsv_color[0] > 315) options.max_hsv_color[0] = 315;
  if (options.min_hsv_color[0] > 315) options.min_hsv_color[0] = 0;

  if (options.max_hsv_color[0] < options.min_hsv_color[0])
    {
      NV_U_INT16 tmpu16 = options.max_hsv_color[0];
      options.max_hsv_color[0] = options.min_hsv_color[0];
      options.min_hsv_color[0] = tmpu16;
    }


  //  Allocate the surface colors.

  NV_FLOAT32 hue_inc = (NV_FLOAT32) (options.max_hsv_color[0] - options.min_hsv_color[0]) / (NV_FLOAT32) (NUMHUES + 1);
  for (NV_INT32 m = 0 ; m < 2 ; m++)
    {
      NV_INT32 sat = 255;
      if (m) sat = 25;

      for (NV_INT32 i = 0 ; i < NUMHUES ; i++)
        {
          for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
            {
              options.color_array[m][i][j].setHsv ((NV_INT32) (((NUMHUES + 1) - (i - options.min_hsv_color[0])) * hue_inc), sat, j, 255);
            }
        }
    }


  //  Set the sun shading value based on the defaults

  options.sunopts.sun = sun_unv (options.sunopts.azimuth, options.sunopts.elevation);


  //  For stoplight colors we are looking for the nearest color in the color array so we can use the already
  //  defined shades to do sunshading.

  for (NV_INT32 j = 0 ; j < 3 ; j++)
    {
      QColor tmp;

      switch (j)
        {
        case 0:
          tmp = options.stoplight_min_color;
          break;

        case 1:
          tmp = options.stoplight_mid_color;
          break;

        case 2:
          tmp = options.stoplight_max_color;
          break;
        }

      NV_INT32 min_hue_diff = 99999;
      NV_INT32 closest = -1;

      for (NV_INT32 i = 0 ; i < NUMHUES ; i++)
        {
          NV_INT32 hue_diff = abs (tmp.hue () - options.color_array[0][i][255].hue ());

          if (hue_diff < min_hue_diff)
            {
              min_hue_diff = hue_diff;
              closest = i;
            }
        }

      switch (j)
        {
        case 0:
          options.stoplight_min_color = options.color_array[0][closest][255];
          options.stoplight_min_index = closest;
          break;

        case 1:
          options.stoplight_mid_color = options.color_array[0][closest][255];
          options.stoplight_mid_index = closest;
          break;

        case 2:
          options.stoplight_max_color = options.color_array[0][closest][255];
          options.stoplight_max_index = closest;
          break;
        }
    }


  //  Set the map and cov map values from the defaults

  mapdef.projection = CYLINDRICAL_EQUIDISTANT;
  mapdef.draw_width = misc.width;
  mapdef.draw_height = misc.height;
  mapdef.overlap_percent = options.overlap_percent;
  mapdef.grid_inc_x = 0.0;
  mapdef.grid_inc_y = 0.0;

  mapdef.coasts = options.coast;
  mapdef.landmask = options.landmask;

  mapdef.border = 5;
  mapdef.coast_color = options.coast_color;
  mapdef.landmask_color = options.landmask_color;
  mapdef.grid_color = options.coast_color;
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


  misc.map_widget = map;


  //  Connect to the signals from the map class.

  connect (map, SIGNAL (mousePressSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, SLOT (slotMousePress (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (mouseDoubleClickSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this,
           SLOT (slotMouseDoubleClick (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (preliminaryMousePressSignal (QMouseEvent *)), this, SLOT (slotPreliminaryMousePress (QMouseEvent *)));
  connect (map, SIGNAL (mouseReleaseSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, SLOT (slotMouseRelease (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, SLOT (slotMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (map, SIGNAL (closeSignal (QCloseEvent *)), this, SLOT (slotClose (QCloseEvent *)));
  connect (map, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (map, SIGNAL (preRedrawSignal (NVMAP_DEF)), this, SLOT (slotPreRedraw (NVMAP_DEF)));
  connect (map, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotPostRedraw (NVMAP_DEF)));


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  QGroupBox *leftBox = new QGroupBox ();
  QVBoxLayout *leftBoxLayout = new QVBoxLayout ();
  leftBox->setLayout (leftBoxLayout);


  QGroupBox *covBox = new QGroupBox (tr ("Coverage"));
  QVBoxLayout *covBoxLayout = new QVBoxLayout ();
  covBox->setLayout (covBoxLayout);


  //  Make the coverage map.

  covdef.projection = CYLINDRICAL_EQUIDISTANT;
  covdef.draw_width = misc.cov_width;
  covdef.draw_height = misc.cov_height;

  covdef.coasts = NVMAP_NO_COAST;
  covdef.landmask = NVFalse;

  covdef.coast_color = options.coast_color;
  covdef.landmask_color = options.landmask_color;

  covdef.grid_inc_x = 0.0;
  covdef.grid_inc_y = 0.0;
  covdef.border = 5;
  covdef.background_color = QColor (255, 255, 255);


  covdef.initial_bounds.min_x = -180.0;
  covdef.initial_bounds.min_y = 180.0;
  covdef.initial_bounds.max_x = 90.0;
  covdef.initial_bounds.max_y = -90.0;


  cov = new nvMap (this, &covdef);
  cov->setMinimumSize (misc.cov_width, misc.cov_height);
  cov->setMaximumSize (misc.cov_width, misc.cov_height);


  cov->setWhatsThis (covText);


  //  Connect to the signals from the map class.
    
  connect (cov, SIGNAL (mousePressSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, SLOT (slotCovMousePress (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (cov, SIGNAL (mouseReleaseSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, SLOT (slotCovMouseRelease (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (cov, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, SLOT (slotCovMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (cov, SIGNAL (closeSignal (QCloseEvent *)), this, SLOT (slotClose (QCloseEvent *)));
  connect (cov, SIGNAL (preRedrawSignal (NVMAP_DEF)), this, SLOT (slotCovPreRedraw (NVMAP_DEF)));
  connect (cov, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotCovPostRedraw (NVMAP_DEF)));


  //  Right click popup menu

  covPopupMenu = new QMenu (cov);

  covPopup0 = covPopupMenu->addAction (tr ("Display area"));
  connect (covPopup0, SIGNAL (triggered ()), this, SLOT (slotCovPopupMenu0 ()));
  covPopup1 = covPopupMenu->addAction (tr ("Discard rectangle"));
  connect (covPopup1, SIGNAL (triggered ()), this, SLOT (slotCovPopupMenu1 ()));
  covPopup2 = covPopupMenu->addAction (tr ("Drag rectangle"));
  connect (covPopup2, SIGNAL (triggered ()), this, SLOT (slotCovPopupMenu2 ()));
  covPopupMenu->addSeparator ();
  covPopup3 = covPopupMenu->addAction (tr ("Help"));
  connect (covPopup3, SIGNAL (triggered ()), this, SLOT (slotCovPopupHelp ()));


  cov->setCursor (Qt::WaitCursor);
  cov_area_defined = NVFalse;

  covBoxLayout->addWidget (cov);

  leftBoxLayout->addWidget (covBox);


  
  QGroupBox *pfmBox = new QGroupBox ();
  QVBoxLayout *pfmBoxLayout = new QVBoxLayout ();
  pfmBox->setLayout (pfmBoxLayout);


  pfmLabel = new QLabel (this);
  pfmLabel->setMaximumWidth (misc.cov_width);
  pfmLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  pfmLabel->setToolTip (tr ("PFM layer name"));
  pfmLabel->setWhatsThis (pfmLabelText);
  pfmBoxLayout->addWidget (pfmLabel);
  leftBoxLayout->addWidget (pfmBox);


  //  Set up the status and PFM layer tabbed notebook

  QGroupBox *noteBox = new QGroupBox ();
  QVBoxLayout *noteBoxLayout = new QVBoxLayout ();
  noteBox->setLayout (noteBoxLayout);


  QGroupBox *statBox = new QGroupBox ();
  QGridLayout *statBoxLayout = new QGridLayout;
  statBox->setLayout (statBoxLayout);
  statBox->setMaximumWidth (misc.cov_width);


  for (NV_INT32 i = 0 ; i < NUM_SCALE_LEVELS ; i++)
    {
      misc.scale[i] = new scaleBox (this);
      misc.scale[i]->setMinimumWidth (80);
      misc.scale[i]->setMaximumWidth (80);
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

  QLabel *numName = new QLabel ("#:", this);
  numName->setToolTip (tr ("Number of points in bin"));
  numName->setMinimumSize (numName->sizeHint ());
  statBoxLayout->addWidget (numName, 2, 1, Qt::AlignLeft);

  numLabel = new QLabel ("00000", this);
  numLabel->setToolTip (tr ("Number of points in bin"));
  numLabel->setMinimumSize (numLabel->sizeHint ());
  statBoxLayout->addWidget (numLabel, 2, 2, Qt::AlignLeft);

  QLabel *stdName = new QLabel ("STD:", this);
  stdName->setToolTip (tr ("Standard deviation of points in bin"));
  stdName->setMinimumSize (stdName->sizeHint ());
  statBoxLayout->addWidget (stdName, 3, 1, Qt::AlignLeft);

  stdLabel = new QLabel ("00.0000", this);
  stdLabel->setToolTip (tr ("Standard deviation of points in bin"));
  stdLabel->setMinimumSize (stdLabel->sizeHint ());
  statBoxLayout->addWidget (stdLabel, 3, 2, Qt::AlignLeft);

  QLabel *minName = new QLabel ("Min Z:", this);
  minName->setToolTip (tr ("Minimum Z value for bin"));
  minName->setMinimumSize (minName->sizeHint ());
  statBoxLayout->addWidget (minName, 4, 1, Qt::AlignLeft);

  minLabel = new QLabel ("0000.00", this);
  minLabel->setToolTip (tr ("Minimum Z value for bin"));
  minLabel->setMinimumSize (minLabel->sizeHint ());
  minLabel->setAutoFillBackground (TRUE);
  minPalette = minLabel->palette ();
  misc.widgetBackgroundColor = minPalette.color (QPalette::Normal, QPalette::Window);
  statBoxLayout->addWidget (minLabel, 4, 2, Qt::AlignLeft);

  QLabel *avgName = new QLabel ("Avg Z:", this);
  avgName->setToolTip (tr ("Average Z value for bin"));
  avgName->setMinimumSize (avgName->sizeHint ());
  statBoxLayout->addWidget (avgName, 5, 1, Qt::AlignLeft);

  avgLabel = new QLabel ("0000.00", this);
  avgLabel->setToolTip (tr ("Average Z value for bin"));
  avgLabel->setMinimumSize (avgLabel->sizeHint ());
  avgLabel->setAutoFillBackground (TRUE);
  avgPalette = avgLabel->palette ();
  statBoxLayout->addWidget (avgLabel, 5, 2, Qt::AlignLeft);

  QLabel *maxName = new QLabel ("Max Z:", this);
  maxName->setToolTip (tr ("Maximum Z value for bin"));
  maxName->setMinimumSize (maxName->sizeHint ());
  statBoxLayout->addWidget (maxName, 6, 1, Qt::AlignLeft);

  maxLabel = new QLabel ("0000.00", this);
  maxLabel->setToolTip (tr ("Maximum Z value for bin"));
  maxLabel->setMinimumSize (maxLabel->sizeHint ());
  maxLabel->setAutoFillBackground (TRUE);
  maxPalette = maxLabel->palette ();
  statBoxLayout->addWidget (maxLabel, 6, 2, Qt::AlignLeft);

  sizeName = new QLabel ("Bin:", this);
  sizeName->setToolTip (tr ("Bin size in meters"));
  sizeName->setMinimumSize (sizeName->sizeHint ());
  statBoxLayout->addWidget (sizeName, 7, 1, Qt::AlignLeft);

  sizeLabel = new QLabel (this);
  sizeLabel->setToolTip (tr ("Bin size in meters"));
  sizeLabel->setAutoFillBackground (TRUE);
  statBoxLayout->addWidget (sizeLabel, 7, 2, Qt::AlignLeft);


  //  If you add above this, change the value of pos to match

  NV_INT32 pos = 8;
  for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++)
    {
      attrName[i] = new QLabel (tr ("Attr %1:").arg (i + 1), this);
      statBoxLayout->addWidget (attrName[i], pos, 1, Qt::AlignLeft);

      attrLabel[i] = new QLabel ("00000.00000", this);
      attrLabel[i]->setMinimumSize (attrLabel[i]->sizeHint ());
      statBoxLayout->addWidget (attrLabel[i], pos, 2, Qt::AlignLeft);
      attrName[i]->setText ("");
      attrLabel[i]->setText ("");

      pos++;
    }


  statBoxLayout->setColumnStretch (1, 1);
  statBoxLayout->setRowStretch (NUM_SCALE_LEVELS + 1, 1);


  notebook = new QTabWidget ();
  notebook->setTabPosition (QTabWidget::North);


  notebook->addTab (statBox, tr ("Status"));


  QGroupBox *layerBox = new QGroupBox ();
  QGridLayout *layerBoxLayout = new QGridLayout;
  layerBox->setLayout (layerBoxLayout);
  layerBox->setMaximumWidth (misc.cov_width);
  layerBoxLayout->setColumnStretch (1, 1);


  QButtonGroup *layerCheckGrp = new QButtonGroup (this);
  layerCheckGrp->setExclusive (FALSE);
  connect (layerCheckGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotLayerClicked (int)));


  for (NV_INT32 i = 0 ; i < MAX_ABE_PFMS ; i++)
    {
      layerCheck[i] = new QCheckBox (this);
      layerCheckGrp->addButton (layerCheck[i], i);
      layerBoxLayout->addWidget (layerCheck[i], i, 0, Qt::AlignLeft);

      layerName[i] = new QLabel (this);
      layerBoxLayout->addWidget (layerName[i], i, 1, Qt::AlignLeft);
      layerCheck[i]->hide ();
      layerName[i]->hide ();
    }
 
  layerBoxLayout->setRowStretch (MAX_ABE_PFMS - 1, 1);


  notebook->addTab (layerBox, tr ("Layers"));


  leftBoxLayout->addWidget (notebook, 1);


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (leftBox);
  hBox->addWidget (map, 1);
  vBox->addLayout (hBox);


  QGroupBox *progBox = new QGroupBox ();
  QHBoxLayout *progBoxLayout = new QHBoxLayout;
  progBox->setLayout (progBoxLayout);


  misc.statusProgLabel = new QLabel (this);
  misc.statusProgLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  misc.statusProgLabel->setToolTip (tr ("Active mode"));
  misc.statusProgLabel->setWhatsThis (miscLabelText);
  misc.statusProgLabel->setAutoFillBackground (TRUE);
  misc.statusProgPalette = misc.statusProgLabel->palette ();
  progBoxLayout->addWidget (misc.statusProgLabel);


  //  Progress status bar

  misc.progStatus = new QStatusBar ();
  misc.progStatus->setSizeGripEnabled (FALSE);
  misc.progStatus->setToolTip (tr ("Progress bar and label"));
  misc.progStatus->setWhatsThis (progStatusText);
  progBoxLayout->addWidget (misc.progStatus);


  misc.statusProg = new QProgressBar (this);
  misc.statusProg->setRange (0, 100);
  misc.statusProg->setValue (0);
  misc.statusProg->setTextVisible (FALSE);
  misc.progStatus->addWidget (misc.statusProg, 10);


  vBox->addWidget (progBox);


  //  Set View toolbar button states based on saved options

  QString hString;
  hString.sprintf (tr ("Highlight %0.2f percent of depth").toAscii (), options.highlight_percent);
  highlightPercent->setText (hString);
  highlightCount->setText (tr ("Highlight valid bin count exceeding %1").arg (options.h_count));

  bHighlight->setIcon (highlightIcon[options.highlight]);


  QString tip = tr ("Toggle stoplight display [crossovers at %1 and %2]").arg (options.stoplight_min_mid, 2, 'f', 1).arg (options.stoplight_max_mid, 2, 'f', 1);
  bStoplight->setToolTip (tip);
  bStoplight->setChecked (options.stoplight);
  connect (bStoplight, SIGNAL (clicked ()), this, SLOT (slotStoplight ()));

  bContour->setChecked (options.contour);
  connect (bContour, SIGNAL (clicked ()), this, SLOT (slotContour ()));

  bCoast->setChecked (options.coast);
  connect (bCoast, SIGNAL (clicked ()), this, SLOT (slotCoast ()));

  bMask->setChecked (options.landmask);
  connect (bMask, SIGNAL (clicked ()), this, SLOT (slotMask ()));

  bAutoRedraw->setChecked (options.auto_redraw);
  connect (bAutoRedraw, SIGNAL (clicked ()), this, SLOT (slotAutoRedraw ()));


  //  Set Color toolbar button states based on saved options

  bColor[misc.color_by_attribute]->setChecked (TRUE);
  connect (colorGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotColor (int)));


  //  Set Surface toolbar button states based on saved options

  tip = tr ("Set the on-the-fly gridding bin size in meters (%1)").arg (options.otf_bin_size_meters, 3, 'f', 1);
  bSetOtfBin->setToolTip (tip);


  //  Set the correct surface button from the defaults.

  bSurface[options.layer_type - 2]->setChecked (TRUE);

  setSurfaceValidity (options.layer_type);

  misc.prev_surface_val = misc.surface_val;


  //  Set Flag Data toolbar button states based on saved options

  bDisplaySuspect->setChecked (options.display_suspect);
  connect (bDisplaySuspect, SIGNAL (clicked ()), this, SLOT (slotDisplaySuspect ()));

  bDisplaySelected->setChecked (options.display_selected);
  connect (bDisplaySelected, SIGNAL (clicked ()), this, SLOT (slotDisplaySelected ()));

  bDisplayReference->setChecked (options.display_reference);
  connect (bDisplayReference, SIGNAL (clicked ()), this, SLOT (slotDisplayReference ()));

  bDisplayMinMax->setChecked (options.display_minmax);
  connect (bDisplayMinMax, SIGNAL (clicked ()), this, SLOT (slotDisplayMinMax ()));


  //  Set Feature toolbar button states based on saved options

  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);

  bDisplayChildren->setChecked (options.display_children);
  connect (bDisplayChildren, SIGNAL (clicked ()), this, SLOT (slotDisplayChildren ()));

  bDisplayFeatureInfo->setChecked (options.display_feature_info);
  connect (bDisplayFeatureInfo, SIGNAL (clicked ()), this, SLOT (slotDisplayFeatureInfo ()));

  bDisplayFeaturePoly->setChecked (options.display_feature_poly);
  connect (bDisplayFeaturePoly, SIGNAL (clicked ()), this, SLOT (slotDisplayFeaturePoly ()));

  bAddFeature->setEnabled (options.display_feature);

  bDeleteFeature->setEnabled (options.display_feature);

  bEditFeature->setEnabled (options.display_feature);


  //  Set all of the button hotkey tooltips and shortcuts

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++) slotPrefHotKeyChanged (i);


  //  Set the edit function from the defaults

  misc.function = options.edit_mode;
  misc.save_function = misc.last_edit_function = misc.function;


  //  Right click popup menu

  popupMenu = new QMenu (map);

  popup0 = popupMenu->addAction (tr ("Close polygon and filter area"));
  connect (popup0, SIGNAL (triggered ()), this, SLOT (slotPopupMenu0 ()));
  popup1 = popupMenu->addAction (tr ("Set filter standard deviation (2.4)"));
  connect (popup1, SIGNAL (triggered ()), this, SLOT (slotPopupMenu1 ()));
  popup2 = popupMenu->addAction (tr ("Set deep filter only (Yes)"));
  connect (popup2, SIGNAL (triggered ()), this, SLOT (slotPopupMenu2 ()));
  popup3 = popupMenu->addAction (tr ("Discard polygon"));
  connect (popup3, SIGNAL (triggered ()), this, SLOT (slotPopupMenu3 ()));
  popup4 = popupMenu->addAction (tr ("Clear all filter masks"));
  connect (popup4, SIGNAL (triggered ()), this, SLOT (slotPopupMenu4 ()));
  popupMenu->addSeparator ();
  popupHelp = popupMenu->addAction (tr ("Help"));
  connect (popupHelp, SIGNAL (triggered ()), this, SLOT (slotPopupHelp ()));


  //  The following menus are in the menu bar.

  //  Setup the file menu.

  QAction *fileOpenAction = new QAction (tr ("Open"), this);
  fileOpenAction->setStatusTip (tr ("Open PFM file"));
  fileOpenAction->setWhatsThis (openText);
  connect (fileOpenAction, SIGNAL (triggered ()), this, SLOT (slotOpen ()));


  recentMenu = new QMenu (tr ("Open Recent..."));

  recentGrp = new QActionGroup (this);
  connect (recentGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotOpenRecent (QAction *)));

  for (NV_INT32 i = 0 ; i < MAX_RECENT ; i++)
    {
      recentFileAction[i] = recentMenu->addAction (options.recentFile[i]);
      recentGrp->addAction (recentFileAction[i]);
      if (i < options.recent_file_count)
        {
          recentFileAction[i]->setVisible (TRUE);
        }
      else
        {
          recentFileAction[i]->setVisible (FALSE);
        }
    }


  fileCloseAction = new QAction (tr ("Close PFM file"), this);
  fileCloseAction->setStatusTip (tr ("Close top level/layer PFM file"));
  fileCloseAction->setWhatsThis (closePFMText);
  fileCloseAction->setEnabled (FALSE);
  connect (fileCloseAction, SIGNAL (triggered ()), this, SLOT (slotClosePFM ()));

  fileImportAction = new QAction (tr ("Import DNC Data"), this);
  fileImportAction->setStatusTip (tr ("Import DNC data to feature file"));
  fileImportAction->setWhatsThis (importText);
  fileImportAction->setEnabled (FALSE);
  connect (fileImportAction, SIGNAL (triggered ()), this, SLOT (slotImport ()));

  geotiffOpenAction = new QAction (tr ("Open GeoTIFF file"), this);
  geotiffOpenAction->setStatusTip (tr ("Open a GeoTIFF file to be displayed with the PFM data"));
  geotiffOpenAction->setWhatsThis (geotiffText);
  geotiffOpenAction->setEnabled (FALSE);
  connect (geotiffOpenAction, SIGNAL (triggered ()), this, SLOT (slotOpenGeotiff ()));


  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), this, SLOT (slotQuit ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("File"));
  fileMenu->addAction (fileOpenAction);
  fileMenu->addMenu (recentMenu);
  fileMenu->addAction (fileCloseAction);
  fileMenu->addSeparator ();
  fileMenu->addAction (fileImportAction);
  fileMenu->addAction (geotiffOpenAction);
  fileMenu->addSeparator ();
  fileMenu->addAction (fileQuitAction);


  //  Setup the edit menu.

  QActionGroup *editGrp = new QActionGroup (this);
  connect (editGrp, SIGNAL (triggered (QAction*)), this, SLOT (slotCheckMenu (QAction *)));


  fileChecked = new QAction (tr ("Set File Checked"), this);
  fileChecked->setStatusTip (tr ("Set file checked"));
  fileChecked->setWhatsThis (fileCheckedText);
  editGrp->addAction (fileChecked);


  fileUnchecked = new QAction (tr ("Set File Unchecked"), this);
  fileUnchecked->setStatusTip (tr ("Set file unchecked"));
  fileUnchecked->setWhatsThis (fileUncheckedText);
  editGrp->addAction (fileUnchecked);


  fileVerified = new QAction (tr ("Set File Verified"), this);
  fileVerified->setStatusTip (tr ("Set file verified"));
  fileVerified->setWhatsThis (fileVerifiedText);
  editGrp->addAction (fileVerified);


  fileUnverified = new QAction (tr ("Set File Unverified"), this);
  fileUnverified->setStatusTip (tr ("Set file unverified"));
  fileUnverified->setWhatsThis (fileUnverifiedText);
  editGrp->addAction (fileUnverified);


  deleteRestore = new QAction (tr ("Delete/Restore Files"), this);
  deleteRestore->setStatusTip (tr ("Delete/restore input files"));
  deleteRestore->setWhatsThis (restoreDeleteText);
  connect (deleteRestore, SIGNAL (triggered ()), this, SLOT (slotDeleteFile ()));


  deleteFileQueue = new QAction (tr ("Delete File Queue"), this);
  deleteFileQueue->setStatusTip (tr ("Delete input files in the delete file queue"));
  deleteFileQueue->setWhatsThis (deleteQueueText);
  connect (deleteFileQueue, SIGNAL (triggered ()), this, SLOT (slotDeleteQueue ()));


  changePathAct = new QAction (tr ("Change File Paths"), this);
  changePathAct->setStatusTip (tr ("Change input file paths"));
  changePathAct->setWhatsThis (changePathText);
  connect (changePathAct, SIGNAL (triggered ()), this, SLOT (slotChangePath ()));


  QAction *prefsAct = new QAction (tr ("Preferences"), this);
  prefsAct->setStatusTip (tr ("Change program preferences"));
  prefsAct->setWhatsThis (prefsText);
  connect (prefsAct, SIGNAL (triggered ()), this, SLOT (slotPrefs ()));


  editMenu = menuBar ()->addMenu (tr ("Edit"));
  editMenu->addAction (fileChecked);
  editMenu->addAction (fileUnchecked);
  editMenu->addAction (fileVerified);
  editMenu->addAction (fileUnverified);
  editMenu->addSeparator ();
  editMenu->addAction (deleteRestore);
  editMenu->addAction (deleteFileQueue);
  editMenu->addSeparator ();
  editMenu->addAction (changePathAct);
  editMenu->addSeparator ();
  editMenu->addAction (prefsAct);


  //  Setup the Utilities menu.

  QAction *overlay = new QAction (tr ("Select Overlays"), this);
  overlay->setStatusTip (tr ("Select overlays"));
  overlay->setWhatsThis (selectOverlaysText);
  connect (overlay, SIGNAL (triggered ()), this, SLOT (slotOverlays ()));


  findFeatures = new QAction (tr ("Find Feature"), this);
  findFeatures->setStatusTip (tr ("Find a feature"));
  findFeatures->setWhatsThis (findFeaturesText);
  connect (findFeatures, SIGNAL (triggered ()), this, SLOT (slotFindFeature ()));


  definePolygons = new QAction (tr ("Define Feature Polygon"), this);
  definePolygons->setStatusTip (tr ("Define a polygon to associate with a feature"));
  definePolygons->setWhatsThis (definePolygonText);
  definePolygons->setEnabled (FALSE);
  connect (definePolygons, SIGNAL (triggered ()), this, SLOT (slotDefinePolygon ()));


  changeFeature = new QAction (tr ("Change feature file"), this);
  changeFeature->setStatusTip (tr ("Select or change the feature file for this PFM structure"));
  changeFeature->setWhatsThis (changeFeatureText);
  connect (changeFeature, SIGNAL (triggered ()), this, SLOT (slotChangeFeature ()));


  changeMosaic = new QAction (tr ("Change mosaic file"), this);
  changeMosaic->setStatusTip (tr ("Select or change the mosaic file for this PFM structure"));
  changeMosaic->setWhatsThis (changeMosaicText);
  connect (changeMosaic, SIGNAL (triggered ()), this, SLOT (slotChangeMosaic ()));


  startMosaic = new QAction (tr ("Start mosaic file viewer"), this);
  startMosaic->setShortcut (options.mosaic_hotkey);
  startMosaic->setStatusTip (tr ("Start the mosaic file viewer"));
  startMosaic->setWhatsThis (startMosaicText);
  connect (startMosaic, SIGNAL (triggered ()), this, SLOT (slotStartMosaicViewer ()));


  start3D = new QAction (tr ("Start &3D viewer"), this);
  start3D->setShortcut (tr ("3"));
  start3D->setStatusTip (tr ("Start the 3D viewer"));
  start3D->setWhatsThis (start3DText);
  connect (start3D, SIGNAL (triggered ()), this, SLOT (slotStart3DViewer ()));


  exportImage = new QAction (tr ("Export Image"), this);
  exportImage->setStatusTip (tr ("Export an image file of the displayed data"));
  exportImage->setWhatsThis (exportImageText);
  connect (exportImage, SIGNAL (triggered ()), this, SLOT (slotExportImage ()));


  QAction *displayPFMHeader = new QAction (tr ("Display PFM Header"), this);
  displayPFMHeader->setStatusTip (tr ("Display the PFM bin file header"));
  displayPFMHeader->setWhatsThis (displayPFMHeaderText);
  connect (displayPFMHeader, SIGNAL (triggered ()), this, SLOT (slotDisplayHeader ()));


  QAction *outputDataPoints = new QAction (tr ("Output Data Points File"), this);
  outputDataPoints->setStatusTip (tr ("Output a data points file"));
  outputDataPoints->setWhatsThis (outputDataPointsText);
  connect (outputDataPoints, SIGNAL (triggered ()), this, SLOT (slotOutputDataPoints ()));


  QAction *zoomToArea = new QAction (tr ("Zoom To Area File"), this);
  zoomToArea->setStatusTip (tr ("Zoom in view to selected area file"));
  zoomToArea->setWhatsThis (zoomToAreaText);
  connect (zoomToArea, SIGNAL (triggered ()), this, SLOT (slotZoomToArea ()));


  QAction *defineRectArea = new QAction (tr ("Define Rectangular Area File"), this);
  defineRectArea->setStatusTip (tr ("Define a rectangular area and output to a file"));
  defineRectArea->setWhatsThis (defineRectAreaText);
  connect (defineRectArea, SIGNAL (triggered ()), this, SLOT (slotDefineRectArea ()));


  QAction *definePolyArea = new QAction (tr ("Define Polygonal Area File"), this);
  definePolyArea->setStatusTip (tr ("Define a polygonal area and output to a file"));
  definePolyArea->setWhatsThis (definePolyAreaText);
  connect (definePolyArea, SIGNAL (triggered ()), this, SLOT (slotDefinePolyArea ()));


  utilitiesMenu = menuBar ()->addMenu (tr ("Utilities"));
  utilitiesMenu->addAction (findFeatures);
  utilitiesMenu->addAction (definePolygons);
  utilitiesMenu->addSeparator ();
  utilitiesMenu->addAction (changeFeature);
  utilitiesMenu->addAction (changeMosaic);
  utilitiesMenu->addSeparator ();
  utilitiesMenu->addAction (startMosaic);
  utilitiesMenu->addSeparator ();
  utilitiesMenu->addAction (start3D);
  utilitiesMenu->addSeparator ();
  utilitiesMenu->addAction (overlay);
  utilitiesMenu->addSeparator ();
  utilitiesMenu->addAction (displayPFMHeader);
  utilitiesMenu->addSeparator ();
  utilitiesMenu->addAction (exportImage);
  utilitiesMenu->addAction (outputDataPoints);
  utilitiesMenu->addSeparator ();
  utilitiesMenu->addAction (zoomToArea);
  utilitiesMenu->addSeparator ();
  utilitiesMenu->addAction (defineRectArea);
  utilitiesMenu->addAction (definePolyArea);


  //  Setup the Layers menu.

  QActionGroup *layerGrp = new QActionGroup (this);
  connect (layerGrp, SIGNAL (triggered (QAction*)), this, SLOT (slotLayerGrpTriggered (QAction *)));

  for (NV_INT32 i = 0 ; i < MAX_ABE_PFMS ; i++)
    {
      QString layerString;
      layerString.sprintf (tr ("Layer %d").toAscii (), i + 1);
      layer[i] = new QAction (layerString, this);
      layerString.sprintf (tr ("Move layer %d to the top level").toAscii (), i);
      layer[i]->setStatusTip (layerString);
      layer[i]->setWhatsThis (layerText);
      layerGrp->addAction (layer[i]);
      layer[i]->setVisible (FALSE);
    }


  QAction *layerPrefs = new QAction (tr ("Manage Layers"), this);
  layerPrefs->setStatusTip (tr ("Modify the preferences for each layer"));
  layerPrefs->setWhatsThis (layerPrefsText);
  connect (layerPrefs, SIGNAL (triggered ()), this, SLOT (slotLayers ()));


  layerMenu = menuBar ()->addMenu (tr ("Layers"));
  for (NV_INT32 i = 0 ; i < MAX_ABE_PFMS ; i++) layerMenu->addAction (layer[i]);
  layerMenu->addSeparator ();
  layerMenu->addAction (layerPrefs);


  //  Setup the help menu.

  QAction *exHelp = new QAction (tr ("User Guide"), this);
  exHelp->setStatusTip (tr ("Open the Area-Based Editor user guide in a browser"));
  connect (exHelp, SIGNAL (triggered ()), this, SLOT (extendedHelp ()));

  QAction *toolHelp = new QAction (tr ("Tool bars"), this);
  toolHelp->setStatusTip (tr ("Help on tool bars"));
  connect (toolHelp, SIGNAL (triggered ()), this, SLOT (slotToolbarHelp ()));

  QAction *whatsThisAct = QWhatsThis::createAction (this);

  QAction *aboutAct = new QAction (tr ("About"), this);
  aboutAct->setStatusTip (tr ("Information about pfmView"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgements = new QAction (tr ("Acknowledgements"), this);
  acknowledgements->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgements, SIGNAL (triggered ()), this, SLOT (slotAcknowledgements ()));

  QAction *aboutQtAct = new QAction (tr ("About Qt"), this);
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("Help"));
  helpMenu->addAction (exHelp);
  helpMenu->addAction (toolHelp);
  helpMenu->addSeparator ();
  helpMenu->addAction (whatsThisAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgements);
  helpMenu->addAction (aboutQtAct);


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


  //  Get the ABE shared memory area.  If it exists, use it.  The key is the process ID plus "_abe".  

  QString skey;
  skey.sprintf ("%d_abe", misc.process_id);

  misc.abeShare = new QSharedMemory (skey);

  if (!misc.abeShare->create (sizeof (ABE_SHARE), QSharedMemory::ReadWrite)) misc.abeShare->attach (QSharedMemory::ReadWrite);

  misc.abe_share = (ABE_SHARE *) misc.abeShare->data ();


  //  Clear the shared memory area and set the ppid key.

  memset (misc.abe_share, 0, sizeof (ABE_SHARE));
  misc.abe_share->ppid = misc.process_id;

  misc.abe_share->pfm_count = 0;


  //  Set a couple of things that pfmEdit(3D) will need to know.

  misc.abe_share->settings_changed = NVFalse;
  misc.abe_share->zoom_requested = NVFalse;
  misc.abe_share->position_form = options.position_form;
  misc.abe_share->smoothing_factor = options.smoothing_factor;
  misc.abe_share->z_factor = options.z_factor;
  misc.abe_share->z_offset = options.z_offset;
  misc.abe_share->min_hsv_color = options.min_hsv_color[0];
  misc.abe_share->max_hsv_color = options.max_hsv_color[0];
  misc.abe_share->min_hsv_value = options.min_hsv_value[0];
  misc.abe_share->max_hsv_value = options.max_hsv_value[0];
  misc.abe_share->min_hsv_locked = options.min_hsv_locked[0];
  misc.abe_share->max_hsv_locked = options.max_hsv_locked[0];


  //  Before the first pfmEdit(3D), attribute viewer running is guaranteed to be false

  misc.abe_share->avShare.avRunning = NVFalse;


  //  Move stuff from options to share.

  misc.abe_share->layer_type = options.layer_type;
  misc.abe_share->cint = options.cint;
  misc.abe_share->num_levels = options.num_levels;
  for (NV_INT32 i = 0 ; i < options.num_levels ; i++) misc.abe_share->contour_levels[i] = options.contour_levels[i];
  strcpy (misc.abe_share->feature_search_string, options.feature_search_string.toAscii ());


  //  Set the toolbar buttons and the cursor.

  setMainButtons (NVFalse);
  setFunctionCursor (misc.function);



  //  If there was a filename on the command line...

  if (*argc > 1)
    {
      //  Special case.  pfmWDBView may hand the editor a list file with multiple PFMs to open.  It will also contain the
      //  rectangular area to be displayed.

      if (strstr (argv[optind], "pfmWDBView"))
        {
          open_pfm_files (&misc, argv[optind], &pfmWDBViewBounds);

          for (NV_INT32 pfm = misc.abe_share->pfm_count ; pfm > 0 ; pfm--)
            {
              QString layerString;
              layerString.sprintf ("%d - ", pfm + 1);
              layerString += QFileInfo (misc.abe_share->open_args[pfm].list_path).fileName ();
              layer[pfm]->setText (layerString);
              layer[pfm]->setVisible (TRUE);
            }

          misc.GeoTIFF_open = NVFalse;
          misc.display_GeoTIFF = NVFalse;
          bGeotiff->setEnabled (FALSE);
          misc.GeoTIFF_init = NVTrue;


          QString tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[0]) + tr (" data");
          highlightUser01->setText (tmp);
          tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[1]) + tr (" data");
          highlightUser02->setText (tmp);
          tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[2]) + tr (" data");
          highlightUser03->setText (tmp);
          tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[3]) + tr (" data");
          highlightUser04->setText (tmp);
          tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[4]) + tr (" data");
          highlightUser05->setText (tmp);


          //  Determine the attribute formats, if any

          misc.cube_attr_available[0] = NVFalse;
          if (misc.abe_share->open_args[0].head.num_bin_attr)
            {
              for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++)
                {
                  if (i < misc.abe_share->open_args[0].head.num_bin_attr)
                    {
                      NV_INT32 decimals = (NV_INT32) (log10 (misc.abe_share->open_args[0].head.bin_attr_scale[i]) + 0.05);
                      sprintf (misc.attr_format[0][i], "%%.%df", decimals);
                      attrLabel[i]->setToolTip (QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[i])));
                      attrName[i]->setText (tr ("Attr %1:").arg (i + 1));


                      //  Check to see if this PFM has CUBE attributes.

                      if (!strcmp (misc.abe_share->open_args[0].head.bin_attr_name[i], "###0")) misc.cube_attr_available[0] = NVTrue;


                      QString tmp = tr ("Color by ") + QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[i]));
                      bColor[i + PRE_ATTR]->setToolTip (tmp);
                      bColorAction[i + PRE_ATTR]->setVisible (TRUE);
                    }
                  else
                    {
                      attrName[i]->setText ("");
                      attrLabel[i]->setText ("");
                      bColorAction[i + PRE_ATTR]->setVisible (FALSE);
                    }
                }


              //  If we don't have the cube executable or the attributes we don't want to offer the recube button.

              if (misc.cube_available && misc.cube_attr_available[0])
                {
                  bCube->setEnabled (TRUE);
                }
              else
                {
                  bCube->setEnabled (FALSE);
                }
            }
          else
            {
              for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) bColorAction[i + PRE_ATTR]->setVisible (FALSE);
              bCube->setEnabled (FALSE);
            }


          //  Check for attributes and the color_by_attribute flag.

          if (misc.abe_share->open_args[0].head.num_bin_attr < misc.color_by_attribute) misc.color_by_attribute = 0;


          misc.clear = NVTrue;

          pfmWDBView = NVTrue;


          //  Check to see if the user has editing rights for the PFMWDB.

          if (PFMWDB_is_read_only () || PFMWDB_is_locked ()) misc.abe_share->read_only = NVTrue;
        }


      //  Just a normal file name (save it so we can call slotOpen in commandLineFileCheck);

      else
        {
          commandFile = QFileInfo (QString (argv[optind])).absoluteFilePath ();
          command_file_flag = NVTrue;
        }
    }


  //  Set the tracking timer function.  This is used to track the cursor when it is in other ABE program windows.
  //  Since we may be passing tracking info from child to external apps and vice versa I've halved the normal
  //  time from 50 milliseconds to 25.

  trackCursor = new QTimer (this);
  connect (trackCursor, SIGNAL (timeout ()), this, SLOT (slotTrackCursor ()));
  trackCursor->start (25);


  //  New feature message.  Increment the number when you change the message and append the old message (in startupMessage.cpp) to new_features.txt

  new_feature = 15;
  startup = NVTrue;

  if (options.new_feature < new_feature) options.startup_message = NVTrue;


  //  Set the program start timer function.  This is used when starting and stopping ancillary programs (like mosaicView).

  programTimer = new QTimer (this);
  connect (programTimer, SIGNAL (timeout()), this, SLOT (slotProgramTimer ()));
}



pfmView::~pfmView ()
{
}



//!  This is a special case to handle the possibility that we were started with a file from PFMWDBView on the command line.

void 
pfmView::commandLineFileCheck ()
{
  if (pfmWDBView) 
    {
      initializeMaps ();


      //  We need to zoom to the area.

      misc.clear = NVTrue;
      cov_area_defined = NVTrue;

      zoomIn (pfmWDBViewBounds, NVFalse);

      misc.cov_function = COV_START_DRAW_RECTANGLE;

      redrawCovBounds ();

      setFunctionCursor (misc.function);
    }
  else if (command_file_flag)
    {
      slotOpen ();


      //  Check to see if we requested a specific area on the command line (--nsew or --area_file).

      if (command_line_mbr.max_x < 360.0)
        {
          //  Adjust the command line arguments to the bounds of the just opened PFM.

          command_line_mbr.max_x = qMin (command_line_mbr.max_x, misc.abe_share->open_args[0].head.mbr.max_x);
          command_line_mbr.max_y = qMin (command_line_mbr.max_y, misc.abe_share->open_args[0].head.mbr.max_y);
          command_line_mbr.min_x = qMax (command_line_mbr.min_x, misc.abe_share->open_args[0].head.mbr.min_x);
          command_line_mbr.min_y = qMax (command_line_mbr.min_y, misc.abe_share->open_args[0].head.mbr.min_y);


          initializeMaps ();


          //  We need to zoom to the area.

          misc.clear = NVTrue;
          cov_area_defined = NVTrue;

          zoomIn (command_line_mbr, NVFalse);

          misc.cov_function = COV_START_DRAW_RECTANGLE;

          redrawCovBounds ();

          setFunctionCursor (misc.function);
        }
    }
}



//!  This is where we link to or unlink from other (non-parent or child) ABE programs.

void 
pfmView::slotLink ()
{
  //  If we're linked, unlink.

  if (misc.linked)
    {
      //  Unregister this application.

      unregisterABE (misc.abeRegister, misc.abe_register, abe_register_group, misc.process_id);


      abe_register_group = -1;


      bLink->setToolTip (tr ("Connect to other ABE applications"));
      bLink->setWhatsThis (linkText);
      bLink->setIcon (QIcon (":/icons/unlink.xpm"));


      //  Unset the link flag.

      misc.linked = NVFalse;


      //  Remove the group id from the title

      QString title;
      title.sprintf ("pfmView : %s", misc.abe_share->open_args[0].list_path);
      this->setWindowTitle (title);


      setMainButtons (NVTrue);
    }
  else
    {
      //  Get the ABE_REGISTER shared memory area.

      registerABE *registerABEDialog = new registerABE (this, "pfmView", misc.process_id, misc.abe_share->open_args[0].list_path,
                                                        &misc.abeRegister, &misc.abe_register);


      //  Link to a group (or create a new group).

      connect (registerABEDialog, SIGNAL (keySetSignal (NV_INT32)), this, SLOT (slotRegisterABEKeySet (NV_INT32)));
    }
}



//! Callback from the link/unlink slot.

void 
pfmView::slotRegisterABEKeySet (NV_INT32 key)
{
  if (key >= 0)
    {
      abe_register_group = key;


      misc.linked = NVTrue;


      QString title;
      title.sprintf ("pfmView (ABE group %02d) : %s", abe_register_group, misc.abe_share->open_args[0].list_path);
      this->setWindowTitle (title);


      bLink->setToolTip (tr ("Disconnect from other ABE applications"));
      bLink->setWhatsThis (unlinkText);
      bLink->setIcon (QIcon (":/icons/link.xpm"));
    }
}



//!  Enable or disable all of the main buttons.

void 
pfmView::setMainButtons (NV_BOOL enabled)
{
  //  When we disable the main buttons we want to make sure that mouse presses don't work.

  if (enabled)
    {
      process_running = NVFalse;
    }
  else
    {
      process_running = NVTrue;
    }


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


  //  The stop button is only enabled during drawing

  if (cov_area_defined) bStop->setEnabled (!enabled);


  //  Only enable the layers menu if we have more than one layer.

  if (misc.abe_share->pfm_count > 1)
    {
      layerMenu->setEnabled (enabled);
    }
  else
    {
      layerMenu->setEnabled (FALSE);
    }

  if (misc.abe_share->pfm_count)
    {
      editMenu->setEnabled (enabled);
      utilitiesMenu->setEnabled (enabled);
    }
  else
    {
      editMenu->setEnabled (FALSE);
      utilitiesMenu->setEnabled (FALSE);
    }


  bStoplight->setEnabled (enabled);
  bContour->setEnabled (enabled);
  bCoast->setEnabled (enabled);
  bMask->setEnabled (enabled);
  fileCloseAction->setEnabled (enabled);
  geotiffOpenAction->setEnabled (enabled);


  //  Only enable the GeoTIFF display button if we have opened a GeoTIFF file

  if (enabled && misc.GeoTIFF_open)
    {
      bGeotiff->setEnabled (enabled);
    }
  else
    {
      bGeotiff->setEnabled (FALSE);
    }


  bSunangle->setEnabled (enabled);


  //  Only enable the zoom out button if we've zoomed in.

  mapdef = map->getMapdef ();
  if (enabled && mapdef.zoom_level > 0)
    {
      bZoomOut->setEnabled (enabled);
    }
  else
    {
      bZoomOut->setEnabled (FALSE);
    }

  bZoomIn->setEnabled (enabled);
  bPrefs->setEnabled (enabled);
  bEditRect->setEnabled (enabled);
  bEditPoly->setEnabled (enabled);
  bEditRect3D->setEnabled (enabled);
  bEditPoly3D->setEnabled (enabled);
  bDisplayMinMax->setEnabled (enabled);
  bRedraw->setEnabled (enabled);
  bAutoRedraw->setEnabled (enabled);
  bRedrawCoverage->setEnabled (enabled);
  bDisplaySuspect->setEnabled (enabled);
  bDisplayFeature->setEnabled (enabled);
  bLink->setEnabled (enabled);


  //  Don't enable these if we're in read_only mode.

  if (misc.abe_share->read_only)
    {
      bFilterRect->setEnabled (FALSE);
      bFilterPoly->setEnabled (FALSE);
      bFilterRectMask->setEnabled (FALSE);
      bFilterPolyMask->setEnabled (FALSE);
      bSetChecked->setEnabled (FALSE);
      bUnload->setEnabled (FALSE);
      fileChecked->setEnabled (FALSE);
      fileUnchecked->setEnabled (FALSE);
      fileVerified->setEnabled (FALSE);
      fileUnverified->setEnabled (FALSE);
      deleteRestore->setEnabled (FALSE);
      deleteFileQueue->setEnabled (FALSE);
      changePathAct->setEnabled (FALSE);
    }
  else
    {
      bFilterRect->setEnabled (enabled);
      bFilterPoly->setEnabled (enabled);
      bFilterRectMask->setEnabled (enabled);
      bFilterPolyMask->setEnabled (enabled);
      bSetChecked->setEnabled (enabled);
      bUnload->setEnabled (enabled);
      fileChecked->setEnabled (enabled);
      fileUnchecked->setEnabled (enabled);
      fileVerified->setEnabled (enabled);
      fileUnverified->setEnabled (enabled);
      deleteRestore->setEnabled (enabled);

      if (!misc.abe_share->delete_file_queue_count)
        {
          deleteFileQueue->setEnabled (FALSE);
        }
      else
        {
          deleteFileQueue->setEnabled (enabled);
        }

      changePathAct->setEnabled (enabled);
    }


  //  Only enable the clear filter mask button if we have active filter masks.

  if (!misc.poly_filter_mask_count)
    {
      bClearMasks->setEnabled (FALSE);
    }
  else
    {
      bClearMasks->setEnabled (enabled);
    }


  //  Only enable the clear filter contour button if we have active filter contours.

  if (!misc.filt_contour_count)
    {
      bClearFilterContours->setEnabled (FALSE);
    }
  else
    {
      bClearFilterContours->setEnabled (enabled);
    }


  //  Only enable the feature related buttons if we have an open feature file.

  if (misc.bfd_open)
    {
      findFeatures->setEnabled (enabled);
      definePolygons->setEnabled (enabled);
    }
  else
    {
      findFeatures->setEnabled (FALSE);
      definePolygons->setEnabled (FALSE);
    }


  for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
    {
      //  Turn on the remisp buttons if we have any MISP average surface types.  Note that we break out of the 
      //  loop if this is the case.

      if (enabled && misc.average_type[pfm] && !misc.abe_share->read_only)
        {
          //  Turn on the clear contours button if we have drawn contours.

          if (misc.filt_contour_count)
            {
              bClearFilterContours->setEnabled (enabled);
            }
          else
            {
              bClearFilterContours->setEnabled (FALSE);
            }

          bDrawContour->setEnabled (enabled);
          bGrabContour->setEnabled (enabled);
          bDrawContourFilter->setEnabled (enabled);
          bRemisp->setEnabled (enabled);
          bRemispFilter->setEnabled (enabled);
          break;
        }
      else
        {
          bRemisp->setEnabled (FALSE);
          bRemispFilter->setEnabled (FALSE);
          bDrawContour->setEnabled (FALSE);
          bGrabContour->setEnabled (FALSE);
          bDrawContourFilter->setEnabled (FALSE);
          bClearFilterContours->setEnabled (FALSE);
        }
    }


  bHigh->setEnabled (enabled);
  bLow->setEnabled (enabled);


  //  Enable the CUBE button if we have any PFM files with CUBE attributes.

  for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
    {
      if (misc.cube_available && misc.cube_attr_available[pfm] && !misc.abe_share->read_only)
        {
          bCube->setEnabled (enabled);
          break;
        }
      else
        {
          bCube->setEnabled (FALSE);
        }
    }


  //  No point in having feature info buttons if we're not displaying features

  if (enabled && options.display_feature && !misc.abe_share->read_only)
    {
      bDisplayChildren->setEnabled (enabled);
      bDisplayFeatureInfo->setEnabled (enabled);
      bDisplayFeaturePoly->setEnabled (enabled);
      bAddFeature->setEnabled (enabled);
      bDeleteFeature->setEnabled (enabled);
      bEditFeature->setEnabled (enabled);
      bVerifyFeatures->setEnabled (enabled);
      fileImportAction->setEnabled (enabled);
    }
  else
    {
      bDisplayChildren->setEnabled (FALSE);
      bDisplayFeatureInfo->setEnabled (FALSE);
      bDisplayFeaturePoly->setEnabled (FALSE);
      bAddFeature->setEnabled (FALSE);
      bDeleteFeature->setEnabled (FALSE);
      bEditFeature->setEnabled (FALSE);
      bVerifyFeatures->setEnabled (FALSE);
      fileImportAction->setEnabled (FALSE);
    }


  bDisplaySelected->setEnabled (enabled);
  bDisplayReference->setEnabled (enabled);


  //  Don't allow highlighting if we are doing on-the-fly gridding.

  if (misc.otf_surface)
    {
      bHighlight->setEnabled (FALSE);
    }
  else
    {
      bHighlight->setEnabled (enabled);
    }


  for (NV_INT32 i = 0 ; i < NUM_SURFACES ; i++) bSurface[i]->setEnabled (enabled);
  bSetOtfBin->setEnabled (enabled);


  for (NV_INT32 i = 0 ; i < PRE_ATTR + NUM_ATTR ; i++) bColor[i]->setEnabled (enabled);


  //  Set the scaleBox ToolTips and WhatsThis (inverted for attributes).

  NV_INT32 k = misc.color_by_attribute;
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


  //  If we were called from pfmWDBView we don't want to allow deletion of files, changing of file paths, or unloading of data.

  if (pfmWDBView)
    {
      bUnload->setEnabled (FALSE);
      deleteRestore->setEnabled (FALSE);
      deleteFileQueue->setEnabled (FALSE);
      changePathAct->setEnabled (FALSE);
    }


  //  Set the button states based on the active function.

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

    case RECT_FILTER_AREA:
      bFilterRect->setChecked (TRUE);
      break;

    case POLY_FILTER_AREA:
      bFilterPoly->setChecked (TRUE);
      break;

    case RECT_FILTER_MASK:
      bFilterRectMask->setChecked (TRUE);
      break;

    case POLY_FILTER_MASK:
      bFilterPolyMask->setChecked (TRUE);
      break;

    case ADD_FEATURE:
      bAddFeature->setChecked (TRUE);
      break;

    case EDIT_FEATURE:
      bEditFeature->setChecked (TRUE);
      break;

    case DELETE_FEATURE:
      bDeleteFeature->setChecked (TRUE);
      break;

    case DRAW_CONTOUR:
      bDrawContour->setChecked (TRUE);
      break;

    case GRAB_CONTOUR:
      bGrabContour->setChecked (TRUE);
      break;

    case REMISP_AREA:
      bRemisp->setChecked (TRUE);
      break;

    case DRAW_CONTOUR_FILTER:
      bDrawContourFilter->setChecked (TRUE);
      break;

    case REMISP_FILTER:
      bRemispFilter->setChecked (TRUE);
      break;

    case SELECT_HIGH_POINT:
      bHigh->setChecked (TRUE);
      break;

    case SELECT_LOW_POINT:
      bLow->setChecked (TRUE);
      break;
    }
}



//!  Clear all movable objects.

void 
pfmView::discardMovableObjects ()
{
  map->closeMovingPath (&mv_arrow);
  map->closeMovingPath (&mv_marker);
  map->closeMovingPath (&mv_tracker);
  map->closeMovingPolygon (&pfm3D_polygon);
  map->discardRubberbandRectangle (&rb_rectangle);
  map->discardRubberbandPolygon (&rb_polygon);
}



//!  Left mouse press.  Called from map mouse press callback.

void 
pfmView::leftMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  void get_feature_event_time (NV_INT32 pfm_handle, DEPTH_RECORD depth, time_t *tv_sec, long *tv_nsec);
  void writeContour (MISC *misc, NV_FLOAT32 z_factor, NV_FLOAT32 z_offset, NV_INT32 count, NV_FLOAT64 *cur_x, NV_FLOAT64 *cur_y);
  void filterPolyArea (OPTIONS *options, MISC *misc, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count);
  NV_BOOL setHighLow (NV_FLOAT64 *mx, NV_FLOAT64 *my, MISC *misc, OPTIONS *options, nvMap *map);


  BIN_RECORD bin;


  NV_INT32 count, *px, *py, ndx;
  NV_FLOAT64 *mx, *my, *cur_x, *cur_y;
  NV_F64_XYMBR bounds;
  QFileDialog *fd;
  QString file, string;
  FILE *fp;
  NV_CHAR fname[512], ltstring[25], lnstring[25], hem;
  NV_FLOAT64 deg, min, sec;
  NV_BOOL hit;
  NV_FLOAT64 temp_angle;


  //  If the popup menu is up discard this mouse press

  if (popup_active)
    {
      popup_active = NVFalse;
      return;
    }


  //  Actions based on the active function.  For the most part, if a rubberband rectangle or polygon is active then
  //  this is the second mouse press and we want to perform the operation.  If one isn't present then it's the
  //  first mouse click and we want to start (anchor) a rectangle, line, or polygon.

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
              if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

              if (mx[i] < bounds.min_x) bounds.min_x = mx[i];
              if (my[i] < bounds.min_y) bounds.min_y = my[i];
              if (mx[i] > bounds.max_x) bounds.max_x = mx[i];
              if (my[i] > bounds.max_y) bounds.max_y = my[i];
            }

          misc.clear = NVTrue;

          misc.GeoTIFF_init = NVTrue;

          zoomIn (bounds, NVTrue);


          misc.cov_function = COV_START_DRAW_RECTANGLE;

          redrawCovBounds ();

          map->setToolTip ("");      
          misc.function = misc.save_function;

          setFunctionCursor (misc.function);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine); 
       }
      break;

    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);
          editCommand (mx, my, 4);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;

    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);

          if (setHighLow (mx, my, &misc, &options, map)) cov->redrawMap (NVTrue);

          map->discardRubberbandRectangle (&rb_rectangle);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;


    case RECT_FILTER_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);


          //  Using a 4 point polygon to avoid writing new filter code.

          filterPolyArea (&options, &misc, mx, my, 4);
          redrawMap (NVTrue, NVTrue);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;

    case RECT_FILTER_MASK:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);


          //  Increment the filter mask count and reallocate the memory.

          misc.poly_filter_mask = (FILTER_MASK *) realloc (misc.poly_filter_mask, sizeof (FILTER_MASK) * (misc.poly_filter_mask_count + 1));
          if (misc.poly_filter_mask == NULL)
            {
              perror ("Allocating poly_filter_mask memory");
              exit (-1);
            }


          //  Using a 4 point polygon to avoid writing new masking code.

          misc.poly_filter_mask[misc.poly_filter_mask_count].count = 4;

          for (NV_INT32 i = 0 ; i < 4 ; i++)
            {
              misc.poly_filter_mask[misc.poly_filter_mask_count].x[i] = mx[i];
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[i] = my[i];
            }


          map->discardRubberbandRectangle (&rb_rectangle);


          ndx = misc.poly_filter_mask_count;
          if (options.poly_filter_mask_color.alpha () < 255)
            {
              map->fillPolygon (misc.poly_filter_mask[ndx].count, misc.poly_filter_mask[ndx].x, misc.poly_filter_mask[ndx].y, 
                                options.poly_filter_mask_color, NVTrue);
            }
          else
            {
              //  We don't have to worry about clipping this because moving the area discards the mask areas.

              map->drawPolygon (misc.poly_filter_mask[ndx].count, misc.poly_filter_mask[ndx].x, misc.poly_filter_mask[ndx].y,
                                options.poly_filter_mask_color, 2, NVTrue, Qt::SolidLine, NVTrue);
            }

          misc.poly_filter_mask[ndx].displayed = NVTrue;

          misc.poly_filter_mask_count++;
          bClearMasks->setEnabled (TRUE);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;

    case REMISP_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);
          count = 4;

          bounds.min_x = 999999.0;
          bounds.min_y = 999999.0;
          bounds.max_x = -999999.0;
          bounds.max_y = -999999.0;
          for (NV_INT32 i = 0 ; i < count ; i++)
            {
              if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

              bounds.min_x = qMin (bounds.min_x, mx[i]);
              bounds.min_y = qMin (bounds.min_y, my[i]);
              bounds.max_x = qMax (bounds.max_x, mx[i]);
              bounds.max_y = qMax (bounds.max_y, my[i]);
            }

          remisp (&misc, &options, &bounds);

          map->discardRubberbandRectangle (&rb_rectangle);


          misc.function = misc.save_function;
          setFunctionCursor (misc.function);


          //  If filter contours were used, remove them.

          if (misc.filt_contour_count)
            {
              slotClearFilterContours ();
            }
          else
            {
              redrawMap (NVTrue, NVTrue);
            }
       }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;


    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:

      //  Second left click

      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          NV_INT32 count;
          map->getRubberbandPolygon (rb_polygon, &count, &px, &py, &mx, &my);

          cur_x = (NV_FLOAT64 *) malloc ((count) * sizeof (NV_FLOAT64));
          cur_y = (NV_FLOAT64 *) malloc ((count) * sizeof (NV_FLOAT64));
          NV_FLOAT64 *new_x = (NV_FLOAT64 *) malloc ((count) * sizeof (NV_FLOAT64));
          NV_FLOAT64 *new_y = (NV_FLOAT64 *) malloc ((count) * sizeof (NV_FLOAT64));

          if (new_y == NULL)
            {
              fprintf (stderr , tr ("Error allocating memory - %s %d\n").toAscii (), __FILE__, __LINE__);
              exit (-1);
            }


          for (NV_INT32 i = 0 ; i < count ; i++)
            {
	      if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;
              cur_x[i] = mx[i];
              cur_y[i] = my[i];
            }
          map->discardRubberbandPolygon (&rb_polygon);


          //  We want to plot the line after we have gotten rid of its moveable image.
          //  We also check to find out which PFM layers the line passes through.

          NV_INT32 new_count = 0;
          NV_FLOAT64 prev_x = -181.0, prev_y = -91.0;
          for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++) misc.contour_in_pfm[pfm] = NVFalse;

          for (NV_INT32 i = 0 ; i < count ; i++)
            {
              NV_F64_COORD2 nxy = {cur_x[i], cur_y[i]};


              //  Check against any of the PFM layers.  Don't save points outside of the PFM areas.

              NV_BOOL hit = NVFalse;
              for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
                {
                  if (bin_inside_ptr (&misc.abe_share->open_args[pfm].head, nxy))
                    {
                      if (prev_x > -181.0) map->drawLine (prev_x, prev_y, cur_x[i], cur_y[i], options.contour_highlight_color,
                                                          LINE_WIDTH, NVTrue, Qt::SolidLine);

                      prev_x = cur_x[i];
                      prev_y = cur_y[i];
                      new_x[new_count] = cur_x[i];
                      new_y[new_count] = cur_y[i];
                      new_count++;
                      misc.contour_in_pfm[pfm] = hit = NVTrue;
                      break;
                    }
                }

              if (!hit) prev_x = -181.0;
            }


          //  Force the map to update.

          map->flush ();


          free (cur_x);
          free (cur_y);


          //  If the contour passed through any of the PFM layers we want to insert the contour values into the PFM file (in the case of DRAW_CONTOUR).

          if (new_count)
            {
              qApp->setOverrideCursor (Qt::WaitCursor);
              qApp->processEvents ();


              if (misc.function == DRAW_CONTOUR)
                {
                  //  Write the points to the PFM file.

                  writeContour (&misc, options.z_factor, options.z_offset, new_count, new_x, new_y);
                }
              else if (misc.function == DRAW_CONTOUR_FILTER)
                {
                  //  Save the data for MISP surface generation and filtering.

                  misc.filt_contour = (NV_F64_COORD3 *) realloc (misc.filt_contour, sizeof (NV_F64_COORD3) * (misc.filt_contour_count + new_count + 1));
                  if (misc.filt_contour == NULL)
                    {
                      perror ("Allocating filt_contour in pfmView.cpp");
                      exit (-1);
                    }

                  for (NV_INT32 i = 0 ; i < new_count ; i++)
                    {
                      misc.filt_contour[misc.filt_contour_count].x = new_x[i];
                      misc.filt_contour[misc.filt_contour_count].y = new_y[i];
                      misc.filt_contour[misc.filt_contour_count].z = (misc.draw_contour_level - options.z_offset) / options.z_factor;
                      misc.filt_contour_count++;
                    }


                  //  Set the end sentinel.

                  misc.filt_contour[misc.filt_contour_count].x = -181.0;
                  misc.filt_contour[misc.filt_contour_count].y = -91.0;
                  misc.filt_contour[misc.filt_contour_count].z = -999999.0;
                  misc.filt_contour_count++;

                  bClearFilterContours->setEnabled (TRUE);
                }


              qApp->restoreOverrideCursor ();
            }
          else
            {
              QMessageBox::warning (this, tr ("pfmView Draw contour"), tr ("No input points defined in cells without valid data"));
            }


          free (new_x);
          free (new_y);

          prev_poly_lon = -181.0;
        }


      //  First left click

      else
        {
          QString msc;
          msc.sprintf (tr (" Draw contour - %.2f ").toAscii (), misc.draw_contour_level);
          misc.statusProgLabel->setText (msc);
          misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
          misc.statusProgLabel->setPalette (misc.statusProgPalette);

          map->anchorRubberbandPolygon (&rb_polygon, lon, lat, options.contour_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
          prev_poly_lon = -181.0;
        }
      break;


    case DEFINE_RECT_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);
          count = 4;

          bounds.min_x = 999999.0;
          bounds.min_y = 999999.0;
          bounds.max_x = -999999.0;
          bounds.max_y = -999999.0;
          for (NV_INT32 i = 0 ; i < count ; i++)
            {
              if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

              bounds.min_x = qMin (bounds.min_x, mx[i]);
              bounds.min_y = qMin (bounds.min_y, my[i]);
              bounds.max_x = qMax (bounds.max_x, mx[i]);
              bounds.max_y = qMax (bounds.max_y, my[i]);
            }


          if (!QDir (options.output_area_dir).exists ()) options.output_area_dir = options.input_pfm_dir;


          fd = new QFileDialog (this, tr ("pfmView Output Area File"));
          fd->setViewMode (QFileDialog::List);


          //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
          //  This function is in the nvutility library.

          setSidebarUrls (fd, options.output_area_dir);


          QStringList filters;
          filters << tr ("Generic area file (*.are)")
                  << tr ("Army Corps area file (*.afs)");

          fd->setFilters (filters);
          fd->setFileMode (QFileDialog::AnyFile);
          fd->selectFilter (tr ("Generic area file (*.are)"));

          hit = NVFalse;

          QStringList files;
          QString file;
          if (fd->exec () == QDialog::Accepted)
            {
              options.output_area_dir = fd->directory ().absolutePath ();

              files = fd->selectedFiles ();

              file = files.at (0);


              if (!file.isEmpty())
                {
                  //  Add extension to filename if not there.

                  if (fd->selectedFilter ().contains ("*.are"))
                    {
                      if (!file.endsWith (".are")) file.append (".are");
                    }
                  else if (fd->selectedFilter ().contains ("*.afs"))
                    {
                      if (!file.endsWith (".afs")) file.append (".afs");
                    }


                  strcpy (fname, file.toAscii ());


                  //  Write the file.

                  if ((fp = fopen (fname, "w")) != NULL)
                    {
                      if (file.endsWith (".are"))
                        {
                          for (NV_INT32 i = 0 ; i < count ; i++)
                            {
                              strcpy (ltstring, fixpos (my[i], &deg, &min, &sec, &hem, POS_LAT, options.position_form));
                              strcpy (lnstring, fixpos (mx[i], &deg, &min, &sec, &hem, POS_LON, options.position_form));

                              fprintf (fp, "%s, %s\n", ltstring, lnstring);
                            }
                        }
                      else
                        {
                          for (NV_INT32 i = 0 ; i < count ; i++)
                            {
                              //  Make sure we haven't created any duplicate points

                              if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;

                              fprintf (fp, "%.9f, %.9f\n", mx[i], my[i]);
                            }
                        }

                      fclose (fp);


                      //  Add the new file to the first available overlay slot.

                      for (NV_INT32 i = 0 ; i < NUM_OVERLAYS ; i++)
                        {
                          if (!misc.overlays[i].file_type)
                            {
                              if (file.endsWith (".are"))
                                {
                                  misc.overlays[i].file_type = GENERIC_AREA;
                                }
                              else
                                {
                                  misc.overlays[i].file_type = ACE_AREA;
                                }
                              strcpy (misc.overlays[i].filename, file.toAscii ());
                              misc.overlays[i].display = NVTrue;
                              misc.overlays[i].color = options.contour_color;

                              hit = NVTrue;

                              break;
                            }
                        }
                    }
                }
            }

          map->discardRubberbandRectangle (&rb_rectangle);

          map->setToolTip ("");      
          misc.function = misc.save_function;

          setFunctionCursor (misc.function);

          if (hit) redrawMap (NVTrue, NVFalse);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;


    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case DEFINE_POLY_AREA:
    case DEFINE_FEATURE_POLY_AREA:
    case REMISP_FILTER:
    case GRAB_CONTOUR:
      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          map->vertexRubberbandPolygon (rb_polygon, lon, lat);
        }
      else
        {
          map->anchorRubberbandPolygon (&rb_polygon, lon, lat, options.contour_color, LINE_WIDTH, NVTrue,
                                        Qt::SolidLine);
        }
      break;


    case OUTPUT_POINTS:
      misc.output_points = (OUTPUT_DATA_POINTS *) realloc (misc.output_points, (misc.output_point_count + 1) * 
                                                           sizeof (OUTPUT_DATA_POINTS));
      misc.output_points[misc.output_point_count].xy.x = lon;
      misc.output_points[misc.output_point_count].xy.y = lat;

      read_bin_record_xy (misc.pfm_handle[0], misc.output_points[misc.output_point_count].xy, &bin);
      misc.output_points[misc.output_point_count].avg = bin.avg_filtered_depth;
      misc.output_points[misc.output_point_count].min = bin.min_filtered_depth;
      misc.output_points[misc.output_point_count].max = bin.max_filtered_depth;

      map->fillCircle (lon, lat, 10, 0.0, 360.0, options.contour_color, NVTrue);

      string.sprintf (tr (" Point #%d").toAscii (), misc.output_point_count);

      map->drawText (string, lon, lat, options.contour_color, NVTrue);

      misc.output_point_count++;
      break;


    case SUNANGLE:
      map->closeMovingPath (&mv_arrow);

      temp_angle = (atan2 ((NV_FLOAT64) (misc.center_point.y - mouse_y), 
                           (NV_FLOAT64) (misc.center_point.x - mouse_x))) / NV_DEG_TO_RAD;
      options.sunopts.azimuth = fmod (270.0 + temp_angle, 360.0);

      options.sunopts.sun = sun_unv (options.sunopts.azimuth, options.sunopts.elevation);

      misc.function = misc.save_function;

      redrawMap (NVTrue, NVFalse);
      break;


    case EDIT_FEATURE:
      if (misc.nearest_feature != -1) editFeatureNum (misc.nearest_feature);
      break;


    case ADD_FEATURE:
      NV_F64_XYMBR mbr;
      NV_FLOAT64 tmplat, tmplon;


      //  Search within 20 meters for a valid point.

      newgp (lat, lon, 0.0, 20.0, &mbr.max_y, &tmplon);
      newgp (lat, lon, 90.0, 20.0, &tmplat, &mbr.max_x);
      newgp (lat, lon, 180.0, 20.0, &mbr.min_y, &tmplon);
      newgp (lat, lon, 270.0, 20.0, &tmplat, &mbr.min_x);


      //  Adjust bounds to nearest grid point

      mbr.min_y = misc.abe_share->open_args[0].head.mbr.min_y + 
        (NINT ((mbr.min_y - misc.abe_share->open_args[0].head.mbr.min_y) / 
               misc.abe_share->open_args[0].head.y_bin_size_degrees)) * misc.abe_share->open_args[0].head.y_bin_size_degrees;

      mbr.max_y = misc.abe_share->open_args[0].head.mbr.min_y + 
        (NINT ((mbr.max_y - misc.abe_share->open_args[0].head.mbr.min_y) /
               misc.abe_share->open_args[0].head.y_bin_size_degrees)) * misc.abe_share->open_args[0].head.y_bin_size_degrees;

      mbr.min_x = misc.abe_share->open_args[0].head.mbr.min_x + 
        (NINT ((mbr.min_x - misc.abe_share->open_args[0].head.mbr.min_x) /
               misc.abe_share->open_args[0].head.x_bin_size_degrees)) * misc.abe_share->open_args[0].head.x_bin_size_degrees;

      mbr.max_x = misc.abe_share->open_args[0].head.mbr.min_x +
        (NINT ((mbr.max_x - misc.abe_share->open_args[0].head.mbr.min_x) /
               misc.abe_share->open_args[0].head.x_bin_size_degrees)) * misc.abe_share->open_args[0].head.x_bin_size_degrees;


      NV_FLOAT64 x, y, min_val;
      NV_F64_COORD2 min_bin, mid;
      BIN_RECORD bin;
      DEPTH_RECORD *dep;
      NV_INT32 numrecs;

      misc.add_feature_index = -1;

      min_bin.x = -999.0;
      min_bin.y = -999.0;
      min_val = 9999999.0;


      //  Search the 0 layer PFM for the minimum depth.

      for (y = mbr.min_y ; y < mbr.max_y ; y += misc.abe_share->open_args[0].head.y_bin_size_degrees)
        {
          mid.y = y + misc.abe_share->open_args[0].head.y_bin_size_degrees / 2.0;

          for (x = mbr.min_x ; x < mbr.max_x ; x += misc.abe_share->open_args[0].head.x_bin_size_degrees)
            {
              mid.x = x + misc.abe_share->open_args[0].head.x_bin_size_degrees / 2.0;

              read_bin_record_xy (misc.pfm_handle[0], mid, &bin);

              if (bin.validity & PFM_DATA)
                {
                  if (bin.min_filtered_depth < min_val)
                    {
                      min_val = (NV_FLOAT64) bin.min_filtered_depth;
                      min_bin = mid;
                    }
                }
            }
        }


      //  If we found a valid bin value, create the feature and add it to the feature file.

      if (min_bin.x > -999.0)
        {
          read_depth_array_xy (misc.pfm_handle[0], min_bin, &dep, &numrecs);

          for (NV_INT32 i = 0 ; i < numrecs ; i++)
            {
              if (!(dep[i].validity & (PFM_INVAL | PFM_DELETED)) && fabs (dep[i].xyz.z - min_val) < 0.0005)
                {
                  misc.add_feature_coord = dep[i].coord;
                  misc.add_feature_index = i;

                  memset (&misc.new_record, 0, sizeof (BFDATA_RECORD));

                  get_feature_event_time (misc.pfm_handle[0], dep[i], &misc.new_record.event_tv_sec, &misc.new_record.event_tv_nsec);

                  misc.new_record.record_number = misc.bfd_header.number_of_records;
                  misc.new_record.length = 0.0;
                  misc.new_record.width = 0.0;
                  misc.new_record.height = 0.0;
                  misc.new_record.confidence_level = 3;
                  misc.new_record.depth = (NV_FLOAT32) dep[i].xyz.z;
                  misc.new_record.horizontal_orientation = 0.0;
                  misc.new_record.vertical_orientation = 0.0;
                  strcpy (misc.new_record.description, "");
                  strcpy (misc.new_record.remarks, "");
                  misc.new_record.latitude = dep[i].xyz.y;
                  misc.new_record.longitude = dep[i].xyz.x;
                  strcpy (misc.new_record.analyst_activity, "NAVOCEANO BHY");
                  misc.new_record.equip_type = 3;
                  misc.new_record.nav_system = 1;
                  misc.new_record.platform_type = 4;
                  misc.new_record.sonar_type = 3;

                  misc.new_record.poly_count = 0;

                  editFeatureNum (-1);

                  break;
                }
            }

          free (dep);
        }

      break;


    case DELETE_FEATURE:
      if (misc.nearest_feature != -1)
        {
          //  Delete feature at nearest_feature

          BFDATA_RECORD bfd_record;
          if (binaryFeatureData_read_record (misc.bfd_handle, misc.nearest_feature, &bfd_record) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (this, tr ("pfmView Delete Feature"), tr ("Unable to read feature record\nReason: ") + msg);
              break;
            }


          //  Zero out the confidence value

          bfd_record.confidence_level = misc.feature[misc.nearest_feature].confidence_level = 0;


          if (binaryFeatureData_write_record (misc.bfd_handle, misc.nearest_feature, &bfd_record, NULL, NULL) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (this, tr ("pfmView Delete Feature"), tr ("Unable to update feature record\nReason: ") + msg);
              break;
            }


          redrawMap (NVTrue, NVFalse);


          misc.cov_clear = NVTrue;
          cov->redrawMap (NVTrue);


          //  If the mosaic viewer or 3D viewer was running, tell it to redraw.

          misc.abe_share->key = FEATURE_FILE_MODIFIED;
        }
      break;
    }
}



//!  Middle mouse button press.  Called from the map mouse press callback.  This is usually a discard operation.

void 
pfmView::midMouse (NV_INT32 mouse_x __attribute__ ((unused)), NV_INT32 mouse_y __attribute__ ((unused)),
                   NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  void highlight_contour (MISC *misc, OPTIONS *options, NV_FLOAT64 lat, NV_FLOAT64 lon, nvMap *map);


  //  Actions based on the active function

  switch (misc.function)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
    case ZOOM_IN_AREA:
    case REMISP_AREA:
    case RECT_FILTER_AREA:
    case RECT_FILTER_MASK:
    case DEFINE_RECT_AREA:
      map->discardRubberbandRectangle (&rb_rectangle);
      break;

    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
    case REMISP_FILTER:
    case GRAB_CONTOUR:
      map->discardRubberbandPolygon (&rb_polygon);
      prev_poly_lon = -181.0;
      break;

    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case DEFINE_POLY_AREA:
    case DEFINE_FEATURE_POLY_AREA:
      map->discardRubberbandPolygon (&rb_polygon);
      prev_poly_lon = -181.0;
      misc.feature_polygon_flag = -1;
      break;

    case SUNANGLE:
      map->closeMovingPath (&mv_arrow);
      break;

    case OUTPUT_POINTS:
      free (misc.output_points);
      misc.output_points = (OUTPUT_DATA_POINTS *) NULL;
      misc.output_point_count = 0;
      redrawMap (NVTrue, NVFalse);
      break;


      //  For these we want to load the nearest contour value as the drawn contour value.

    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          map->discardRubberbandPolygon (&rb_polygon);
        }
      else
        {
          //  Grab the nearest cell and highlight the contours.

          highlight_contour (&misc, &options, lat, lon, map);


          QString msc;
          msc.sprintf (tr (" Draw contour - %.2f ").toAscii (), misc.draw_contour_level);
          misc.statusProgLabel->setText (msc);
          misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
          misc.statusProgLabel->setPalette (misc.statusProgPalette);
        }
      prev_poly_lon = -181.0;
    }

  if (misc.function != ADD_FEATURE && misc.function != EDIT_FEATURE && misc.function != DELETE_FEATURE)
    {
      misc.function = misc.save_function;

      setMainButtons (NVTrue);
      setFunctionCursor (misc.function);

      map->setToolTip ("");      
    }
}



//!  If we errored out of the pfmEdit(3D) process...

void 
pfmView::slotEditError (QProcess::ProcessError error)
{
  //  If we manually stopped it we don't want a message.

  if (pfmEdit_stopped)
    {
      pfmEdit_stopped = NVFalse;
      return;
    }


  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("Unable to start the edit process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("The edit process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("The edit process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("There was a write error to the edit process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("There was a read error from the edit process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("The edit process died with an unknown error!"));
      break;
    }
}



//!  This is the return from the edit QProcess (when finished normally)

void 
pfmView::slotEditDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  NV_BOOL force = NVTrue;


  //  If pfm3D is up we need to tell it to reload it's memory now that we are finished with the 
  //  3D editor.  The other end of this (i.e. setting PFMEDIT3D_OPENED) happens in pfmEdit3D.

  if (threeD_edit)
    {
      //  I put brackets inside the lock/unlock for clarity.

      misc.abeShare->lock ();
      {
        misc.abe_share->key = PFMEDIT3D_CLOSED;
      }
      misc.abeShare->unlock ();

      force = NVFalse;
      threeD_edit = NVFalse;
    }


  //  If we canceled drawing, bail out.

  if (misc.drawing_canceled)
    {
      misc.drawing_canceled = NVFalse;
      return;
    }


  pfm_edit_active = NVFalse;


  NV_BOOL areaMoved = NVFalse;


  //  If the return from pfmEdit is greater than or equal to 100 we asked for a move in pfmEdit.  In that case we
  //  will always redraw all of the display and the coverage map.

  if (pfmEditMod >= 100)
    {
      pfmEditMod -= 100;
      areaMoved = NVTrue;
    }


  NV_BOOL feature_change = NVFalse;


  //  If anything changed, get the features (if they exist).

  if (pfmEditMod)
    {
      //  Set the pfmViewMod flag just in case we were called from pfmWDBView.

      pfmViewMod = NVTrue;


      //  Get the feature file name in case we created one in the editor.

      get_target_file (misc.pfm_handle[0], misc.abe_share->open_args[0].list_path, misc.abe_share->open_args[0].target_path);


      if (strcmp (misc.abe_share->open_args[0].target_path, "NONE"))
        {
          if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);
          misc.bfd_open = NVFalse;

          if ((misc.bfd_handle = bfd_check_file (&misc, misc.abe_share->open_args[0].target_path, &misc.bfd_header, BFDATA_UPDATE)) >= 0)
            {
              if (binaryFeatureData_read_all_short_features (misc.bfd_handle, &misc.feature) < 0)
                {
                  QString msg = QString (binaryFeatureData_strerror ());
                  QMessageBox::warning (this, tr ("pfmView"), tr ("Unable to read feature records\nReason: ") + msg);
                  binaryFeatureData_close_file (misc.bfd_handle);
                }
              else
                {
                  misc.bfd_open = NVTrue;
                  feature_change = NVTrue;
                }
            }
        }
    }


  //  If we changed the PFM structure in the edit and the average filtered surface type is a misp surface we need to 
  //  remisp the edited area.

  NV_F64_XYMBR mbr = misc.abe_share->edit_area;

  if (pfmEditMod == 1) remisp (&misc, &options, &mbr);


  //  If we requested a move from within the editor...

  if (areaMoved)
    {
      //  Save the bounds that were passed back from pfmEdit since redrawMap or moveMap will reset these.

      NV_F64_XYMBR orig_bounds = misc.abe_share->edit_area;


      //  If the settings changed, grab the new ones.

      if (misc.abe_share->settings_changed)
        {
          misc.abe_share->settings_changed = NVFalse;
          options.smoothing_factor = misc.abe_share->smoothing_factor;
          options.z_factor = misc.abe_share->z_factor;
          options.z_offset = misc.abe_share->z_offset;
          options.position_form = misc.abe_share->position_form;


          options.mosaic_prog = QString (misc.abe_share->mosaic_prog);
          options.mosaic_hotkey = QString (misc.abe_share->mosaic_hotkey);
          options.mosaic_actkey = QString (misc.abe_share->mosaic_actkey);
          startMosaic->setShortcut (options.mosaic_hotkey);


          if (options.z_factor != 1.0 || options.z_offset != 0.0)
            {
              minPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
              avgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
              maxPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);

              minLabel->setToolTip (tr ("Scaled/offset minimum Z value for bin"));
              avgLabel->setToolTip (tr ("Scaled/offset average Z value for bin"));
              maxLabel->setToolTip (tr ("Scaled/offset maximum Z value for bin"));
            }
          else if (options.z_factor == 1.0 && options.z_offset == 0.0)
            {
              minPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
              avgPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
              maxPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);

              minLabel->setToolTip (tr ("Minimum Z value for bin"));
              avgLabel->setToolTip (tr ("Average Z value for bin"));
              maxLabel->setToolTip (tr ("Maximum Z value for bin"));
            }
          minLabel->setPalette (minPalette);
          avgLabel->setPalette (avgPalette);
          maxLabel->setPalette (maxPalette);
        }


      //  Only redraw if we've moved outside the displayed area or if we actually changed something.

      NV_BOOL changed = NVFalse;
      if (orig_bounds.min_x < misc.total_displayed_area.min_x)
        {
          moveMap (NVMAP_LEFT);
          changed = NVTrue;
        }
      else if (orig_bounds.max_x > misc.total_displayed_area.max_x)
        {
          moveMap (NVMAP_RIGHT);
          changed = NVTrue;
        }
      else if (orig_bounds.min_y < misc.total_displayed_area.min_y)
        {
          moveMap (NVMAP_DOWN);
          changed = NVTrue;
        }
      else if (orig_bounds.max_y > misc.total_displayed_area.max_y)
        {
          moveMap (NVMAP_UP);
          changed = NVTrue;
        }
      else
        {
          if (options.auto_redraw)
            {
              discardMovableObjects ();

              if (pfmEditMod)
                {
                  redrawMap (NVTrue, NVTrue);
                  changed = NVTrue;
                }
            }
          else
            {
              NV_INT32 count, *px, *py;
              NV_FLOAT64 *mx, *my;

              if (!force_3d_edit && (misc.function == RECT_EDIT_AREA || misc.function == RECT_EDIT_AREA_3D))
                {
                  //  If the rubberband rectangle isn't present (ID is -1) we must have done a pfmEdit move prior to this
                  //  in which case we are using a moving rectangle instead.

                  if (rb_rectangle >= 0)
                    {
                      map->getRubberbandRectangle (rb_rectangle, &px, &py, &mx, &my);
                      map->drawRectangle (px[0], py[0], px[2], py[2], options.contour_color, LINE_WIDTH, Qt::SolidLine, NVTrue);
                      map->discardRubberbandRectangle (&rb_rectangle);
                    }
                  else
                    {
                      map->getMovingRectangle (mv_rectangle, &px, &py, &mx, &my);
                      map->drawRectangle (px[0], py[0], px[2], py[2], options.contour_color, LINE_WIDTH, Qt::SolidLine, NVTrue);
                      map->closeMovingRectangle (&mv_rectangle);
                    }
                }
              else
                {
                  map->getRubberbandPolygon (rb_polygon, &count, &px, &py, &mx, &my);
                  map->drawPolygon (count, px, py, options.contour_color, LINE_WIDTH, NVTrue, Qt::SolidLine, NVTrue);
                  map->discardRubberbandPolygon (&rb_polygon);
                }
            }
        }


      if (changed)
        {
          cov->redrawMap (NVTrue);
          redrawCovBounds ();
        }


      map->setMovingRectangle (&mv_rectangle, orig_bounds.min_x, orig_bounds.min_y, orig_bounds.max_x, orig_bounds.max_y,
                               options.contour_color, LINE_WIDTH, NVFalse, Qt::SolidLine);


      NV_FLOAT64 mx[4], my[4];
      mx[0] = orig_bounds.min_x;
      my[0] = orig_bounds.min_y;
      mx[1] = mx[0];
      my[1] = orig_bounds.max_y;
      mx[2] = orig_bounds.max_x;
      my[2] = my[1];
      mx[3] = mx[2];
      my[3] = my[0];


      // Set the function and the buttons to rectangle edit mode.

      setMainButtons (NVFalse);


      //  Kick the editor off again with the new area.

      editCommand (mx, my, 4);

      return;
    }
  else
    {
      //  If we filtered the area in the editor we need to "automagically" filter mask the area.

      if (pfmEditFilt)
        {
          misc.poly_filter_mask = (FILTER_MASK *) realloc (misc.poly_filter_mask, sizeof (FILTER_MASK) * (misc.poly_filter_mask_count + 1));
          if (misc.poly_filter_mask == NULL)
            {
              perror ("Allocating poly_filter_mask memory");
              exit (-1);
            }


          //  Polygon.

          if (misc.abe_share->polygon_count)
            {
              misc.poly_filter_mask[misc.poly_filter_mask_count].count = misc.abe_share->polygon_count;

              for (NV_INT32 i = 0 ; i < misc.abe_share->polygon_count ; i++)
                {
                  misc.poly_filter_mask[misc.poly_filter_mask_count].x[i] = misc.abe_share->polygon_x[i];
                  misc.poly_filter_mask[misc.poly_filter_mask_count].y[i] = misc.abe_share->polygon_y[i];
                }
            }


          //  Rectangle.

          else
            {
              misc.poly_filter_mask[misc.poly_filter_mask_count].count = 4;

              misc.poly_filter_mask[misc.poly_filter_mask_count].x[0] = misc.abe_share->edit_area.min_x;
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[0] = misc.abe_share->edit_area.min_y;
              misc.poly_filter_mask[misc.poly_filter_mask_count].x[1] = misc.abe_share->edit_area.min_x;
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[1] = misc.abe_share->edit_area.max_y;
              misc.poly_filter_mask[misc.poly_filter_mask_count].x[2] = misc.abe_share->edit_area.max_x;
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[2] = misc.abe_share->edit_area.max_y;
              misc.poly_filter_mask[misc.poly_filter_mask_count].x[3] = misc.abe_share->edit_area.max_x;
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[3] = misc.abe_share->edit_area.min_y;
            }
          misc.poly_filter_mask[misc.poly_filter_mask_count].displayed = NVFalse;

          misc.poly_filter_mask_count++;
        }


      //  Auto redraw.

      if (options.auto_redraw)
        {
          //  Just in case we did a pfmEdit move prior to this we'll clear the moving rectangle that we used to display the
          //  moved area.

          map->closeMovingRectangle (&mv_rectangle);


          if (!force_3d_edit && (misc.function == RECT_EDIT_AREA || misc.function == RECT_EDIT_AREA_3D))
            {
              map->discardRubberbandRectangle (&rb_rectangle);
            }
          else
            {
              map->closeMovingPolygon (&pfm3D_polygon);
              map->discardRubberbandPolygon (&rb_polygon);
            }

          if (pfmEditMod)
            {
              NV_INT32 old_count = misc.bfd_header.number_of_records;


              //  If we didn't have any features and suddenly we have some we need to redraw the entire
              //  map and coverage since the user may have done an add_feature and used an existing
              //  feature file.  We also want to redraw the entire thing if we're displaying contours since it's
              //  almost impossible to merge the contours back in correctly.

              if ((!old_count && misc.bfd_header.number_of_records) || options.contour)
                {
                  feature_change = NVFalse;

                  redrawMap (NVTrue, NVTrue);

                  misc.cov_clear = NVTrue;
                  cov->redrawMap (NVTrue);
                }
              else
                {
                  //  We want to just redraw the area that was edited.  This is why we don't just 
                  //  call map->redrawMap ().

                  misc.clear = NVFalse;
                  slotPreRedraw (map->getMapdef ());


                  //  Make sure we turn mouse signals back on.  This is normally done in the postRedraw slot.

                  map->enableMouseSignals ();


                  misc.clear = NVTrue;


                  misc.cov_clear = NVFalse;
                  cov->redrawMap (NVFalse);


                  //  Make sure we kick pfm3D if it's up.

                  if (threeDProc)
                    {
                      if (threeDProc->state () == QProcess::Running && force) misc.abe_share->key = PFM3D_FORCE_RELOAD;
                    }
                }
            }
        }


      //  No auto redraw.

      else
        {
          NV_INT32 count, *px, *py;
          NV_FLOAT64 *mx, *my;

          if (!force_3d_edit && (misc.function == RECT_EDIT_AREA || misc.function == RECT_EDIT_AREA_3D))
            {
              //  If the rubberband rectangle isn't present (ID is -1) we must have done a pfmEdit move prior to this
              //  in which case we are using a moving rectangle instead.

              if (rb_rectangle >= 0)
                {
                  map->getRubberbandRectangle (rb_rectangle, &px, &py, &mx, &my);
                  map->drawRectangle (px[0], py[0], px[2], py[2], options.contour_color, LINE_WIDTH, Qt::SolidLine, NVTrue);
                  map->discardRubberbandRectangle (&rb_rectangle);
                }
              else
                {
                  map->getMovingRectangle (mv_rectangle, &px, &py, &mx, &my);
                  map->drawRectangle (px[0], py[0], px[2], py[2], options.contour_color, LINE_WIDTH, Qt::SolidLine, NVTrue);
                  map->closeMovingRectangle (&mv_rectangle);
                }
            }
          else
            {
              map->getRubberbandPolygon (rb_polygon, &count, &px, &py, &mx, &my);
              map->drawPolygon (count, px, py, options.contour_color, LINE_WIDTH, NVTrue, Qt::SolidLine, NVTrue);
              map->discardRubberbandPolygon (&rb_polygon);
            }
        }


      //  If we filtered but aren't in auto redraw mode we must still paint the new filter masked area (in semi transparent mode).

      if (pfmEditFilt && !misc.abe_share->settings_changed)
        {
          NV_INT32 fm = misc.poly_filter_mask_count - 1;
          QColor fmc = options.poly_filter_mask_color;

          if (fmc.alpha () == 255) fmc.setAlpha (96);

          map->fillPolygon (misc.poly_filter_mask[fm].count, misc.poly_filter_mask[fm].x, misc.poly_filter_mask[fm].y, fmc, NVTrue);
          misc.poly_filter_mask[fm].displayed = NVTrue;
        }
    }


  //  Make sure we reset the important things in case we did a partial redraw.

  setFunctionCursor (misc.function);


  //  If the user changed settings in pfmEdit let's redraw.

  if (misc.abe_share->settings_changed)
    {
      misc.abe_share->settings_changed = NVFalse;
      options.smoothing_factor = misc.abe_share->smoothing_factor;
      options.z_factor = misc.abe_share->z_factor;
      options.z_offset = misc.abe_share->z_offset;
      options.position_form = misc.abe_share->position_form;
      options.min_hsv_color[0] = misc.abe_share->min_hsv_color;
      options.max_hsv_color[0] = misc.abe_share->max_hsv_color;
      options.min_hsv_value[0] = misc.abe_share->min_hsv_value;
      options.max_hsv_value[0] = misc.abe_share->max_hsv_value;
      options.min_hsv_locked[0] = misc.abe_share->min_hsv_locked;
      options.max_hsv_locked[0] = misc.abe_share->max_hsv_locked;

      options.mosaic_prog = QString (misc.abe_share->mosaic_prog);
      options.mosaic_hotkey = QString (misc.abe_share->mosaic_hotkey);
      options.mosaic_actkey = QString (misc.abe_share->mosaic_actkey);
      startMosaic->setShortcut (options.mosaic_hotkey);


      if (options.z_factor != 1.0 || options.z_offset != 0.0)
        {
          minPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
          avgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
          maxPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);

          minLabel->setToolTip (tr ("Scaled/offset minimum Z value for bin"));
          avgLabel->setToolTip (tr ("Scaled/offset average Z value for bin"));
          maxLabel->setToolTip (tr ("Scaled/offset maximum Z value for bin"));
        }
      else if (options.z_factor == 1.0 && options.z_offset == 0.0)
        {
          minPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
          avgPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
          maxPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);

          minLabel->setToolTip (tr ("Minimum Z value for bin"));
          avgLabel->setToolTip (tr ("Average Z value for bin"));
          maxLabel->setToolTip (tr ("Maximum Z value for bin"));
        }
      minLabel->setPalette (minPalette);
      avgLabel->setPalette (avgPalette);
      maxLabel->setPalette (maxPalette);


      feature_change = NVFalse;

      redrawMap (NVTrue, NVFalse);
    }


  if (options.display_feature && feature_change)
    {
      overlayFlag (map, &options, &misc, NVTrue, NVTrue, NVTrue);
      misc.cov_clear = NVTrue;
      cov->redrawMap (NVTrue);
    }


  setMainButtons (NVTrue);


  //  No matter what, we need to set the displayed bounds back to the pfmView displayed bounds
  //  when we exit pfmEdit.

  misc.abe_share->displayed_area = misc.abe_share->viewer_displayed_area = misc.total_displayed_area;


  misc.drawing = NVFalse;
}



//!  This is the stderr read return from the edit QProcess.  Hopefully you won't see anything here.

void 
pfmView::slotEditReadyReadStandardError ()
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



/*!
  This is the stdout read return from the edit QProcess
  We need to look at the value returned on exit to decide if we need to redraw the map.
  pfmEdit(3D) will print out a 0 for no changes, a 1 for PFM structure changes, or a 2 for feature changes.
  The second number is the filter mask flag.  If it's set we want to filter mask the area.
*/

void 
pfmView::slotEditReadyReadStandardOutput ()
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



//!  Kick off the edit QProcess

void 
pfmView::editCommand (NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count)
{
  //  Only do the following if we don't already have an edit window opened

  if (!pfm_edit_active)
    {
      pfmEditMod = 0;
      pfmEditFilt = 0;
      pfm_edit_active = NVTrue;


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


      //  Adjust to displayed area bounds

      if (min_y < misc.total_displayed_area.min_y) min_y = misc.total_displayed_area.min_y;
      if (max_y > misc.total_displayed_area.max_y) max_y = misc.total_displayed_area.max_y;
      if (min_x < misc.total_displayed_area.min_x) min_x = misc.total_displayed_area.min_x;
      if (max_x > misc.total_displayed_area.max_x) max_x = misc.total_displayed_area.max_x;


      misc.abe_share->edit_area.min_x = min_x;
      misc.abe_share->edit_area.max_x = max_x;
      misc.abe_share->edit_area.min_y = min_y;
      misc.abe_share->edit_area.max_y = max_y;


      map->setCursor (Qt::WaitCursor);


      //  For rectangles we pass a count of zero to tell pfmEdit that it's a rectangle.  All forced edits from pfm3D are
      //  polygons.

      if (!force_3d_edit && (misc.function == RECT_EDIT_AREA || misc.function == RECT_EDIT_AREA_3D))
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


      //  If we're looking at the GeoTIFF surface...

      if (misc.display_GeoTIFF && misc.GeoTIFF_open) 
        {
          arg.sprintf ("-G%s", misc.GeoTIFF_name);
          arguments += arg;
        }


      //  If we are doing a Z value conversion...

      misc.abe_share->z_factor = options.z_factor;


      //  If we are doing a Z value offset...

      misc.abe_share->z_offset = options.z_offset;


      //  Always add the shared memory ID (the process ID).

      arg.sprintf ("--shared_memory_key=%d", misc.abe_share->ppid);
      arguments += arg;


      connect (editProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotEditDone (int, QProcess::ExitStatus)));
      connect (editProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotEditReadyReadStandardError ()));
      connect (editProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotEditReadyReadStandardOutput ()));
      connect (editProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotEditError (QProcess::ProcessError)));

      setMainButtons (NVFalse);
      qApp->processEvents ();


      if (force_3d_edit || misc.function == RECT_EDIT_AREA_3D || misc.function == POLY_EDIT_AREA_3D)
        {
          threeD_edit = NVTrue;

          editProc->start (QString (options.edit_name_3D), arguments);
        }
      else
        {
          editProc->start (QString (options.edit_name), arguments);
        }
    }
}



//!  Mouse double click.  Called from the map double click callback.  This is usually a completion operation.

void 
pfmView::slotMouseDoubleClick (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  BIN_RECORD bin;


  NV_INT32 count, *px, *py, ndx;
  NV_FLOAT64 *mx, *my;
  QFileDialog *fd;
  QString file, string;
  FILE *fp;
  NV_CHAR fname[512], ltstring[25], lnstring[25], hem;
  NV_FLOAT64 deg, min, sec;
  QStringList filters;
  QStringList files;
  static QDir dir = QDir (".");
  NV_BOOL hit = NVFalse;


  void filterPolyArea (OPTIONS *options, MISC *misc, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count);


  //  Flip the double_click flag.  The right-click menu sets this to NVTrue so it will flip to NVFalse.
  //  Left-click sets it to NVFalse so it will flip to NVTrue;

  double_click = !double_click;


  hit = NVFalse;


  //  Actions based on the active function

  switch (misc.function)
    {
    case DEFINE_POLY_AREA:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      if (double_click) count--;


      if (!QDir (options.output_area_dir).exists ()) options.output_area_dir = options.input_pfm_dir;


      fd = new QFileDialog (this, tr ("pfmView Output Area File"));
      fd->setViewMode (QFileDialog::List);


      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
      //  This function is in the nvutility library.

      setSidebarUrls (fd, options.output_area_dir);


      filters << tr ("Generic area file (*.are)")
              << tr ("Army Corps area file (*.afs)");

      fd->setFilters (filters);
      fd->setFileMode (QFileDialog::AnyFile);
      fd->selectFilter (tr ("Generic area file (*.are)"));


      if (fd->exec () == QDialog::Accepted)
        {
          files = fd->selectedFiles ();

          file = files.at (0);


          if (!file.isEmpty())
            {

              //  Add extension to filename if not there.
            
              if (fd->selectedFilter ().contains ("*.are"))
                {
                  if (!file.endsWith (".are")) file.append (".are");
                }
              else if (fd->selectedFilter ().contains ("*.afs"))
                {
                  if (!file.endsWith (".afs")) file.append (".afs");
                }

 
              strcpy (fname, file.toAscii ());


              if ((fp = fopen (fname, "w")) != NULL)
                {
                  if (file.endsWith (".are"))
                    {
                      for (NV_INT32 i = 0 ; i < count ; i++)
                        {
                          strcpy (ltstring, fixpos (my[i], &deg, &min, &sec, &hem, POS_LAT, options.position_form));
                          strcpy (lnstring, fixpos (mx[i], &deg, &min, &sec, &hem, POS_LON, options.position_form));

                          fprintf (fp, "%s, %s\n", ltstring, lnstring);
                        }
                    }
                  else
                    {
                      for (NV_INT32 i = 0 ; i < count ; i++)
                        {
                          //  Make sure we haven't created any duplicate points

                          if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;

                          fprintf (fp, "%.9f, %.9f\n", mx[i], my[i]);
                        }
                    }

                  fclose (fp);


                  //  Add the new file to the first available overlay slot.

                  for (NV_INT32 i = 0 ; i < NUM_OVERLAYS ; i++)
                    {
                      if (!misc.overlays[i].file_type)
                        {
                          if (file.endsWith (".are"))
                            {
                              misc.overlays[i].file_type = GENERIC_AREA;
                            }
                          else
                            {
                              misc.overlays[i].file_type = ACE_AREA;
                            }
                          strcpy (misc.overlays[i].filename, file.toAscii ());
                          misc.overlays[i].display = NVTrue;
                          misc.overlays[i].color = options.contour_color;

                          hit = NVTrue;

                          break;
                        }
                    }
                }
            }
          options.output_area_dir = fd->directory ().absolutePath ();
        }

      map->discardRubberbandPolygon (&rb_polygon);

      map->setToolTip ("");      
      misc.function = misc.save_function;

      setFunctionCursor (misc.function);

      if (hit) redrawMap (NVTrue, NVFalse);
      break;


    case DEFINE_FEATURE_POLY_AREA:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      for (NV_INT32 i = 0 ; i < count ; i++)
        {
          misc.polygon_x[i] = mx[i];
          misc.polygon_y[i] = my[i];
        }
      misc.poly_count = count;

      map->discardRubberbandPolygon (&rb_polygon);

      map->setToolTip ("");
      misc.function = misc.save_function;

      setFunctionCursor (misc.function);

      misc.feature_polygon_flag = 1;
      redrawMap (NVTrue, NVFalse);
      break;


    case POLY_FILTER_AREA:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      filterPolyArea (&options, &misc, mx, my, count);
      redrawMap (NVTrue, NVTrue);

      break;


    case POLY_FILTER_MASK:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      misc.poly_filter_mask = (FILTER_MASK *) realloc (misc.poly_filter_mask, sizeof (FILTER_MASK) * (misc.poly_filter_mask_count + 1));
      if (misc.poly_filter_mask == NULL)
        {
          perror ("Allocating poly_filter_mask memory");
          exit (-1);
        }

      misc.poly_filter_mask[misc.poly_filter_mask_count].count = count;

      for (NV_INT32 i = 0 ; i < count ; i++)
        {
          misc.poly_filter_mask[misc.poly_filter_mask_count].x[i] = mx[i];
          misc.poly_filter_mask[misc.poly_filter_mask_count].y[i] = my[i];
        }


      map->discardRubberbandPolygon (&rb_polygon);


      ndx = misc.poly_filter_mask_count;
      if (options.poly_filter_mask_color.alpha () < 255)
        {
          map->fillPolygon (misc.poly_filter_mask[ndx].count, misc.poly_filter_mask[ndx].x, misc.poly_filter_mask[ndx].y, 
                            options.poly_filter_mask_color, NVTrue);
        }
      else
        {
          //  We don't have to worry about clipping this because moving the area discards the mask areas.

          map->drawPolygon (misc.poly_filter_mask[ndx].count, misc.poly_filter_mask[ndx].x, misc.poly_filter_mask[ndx].y,
                            options.poly_filter_mask_color, 2, NVTrue, Qt::SolidLine, NVTrue);
        }
      misc.poly_filter_mask[ndx].displayed = NVTrue;

      misc.poly_filter_mask_count++;
      bClearMasks->setEnabled (TRUE);
      break;


    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);
      editCommand (mx, my, count);
      break;


    case GRAB_CONTOUR:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      for (NV_INT32 i = 0 ; i < count ; i++)
        {
          misc.polygon_x[i] = mx[i];
          misc.polygon_y[i] = my[i];
        }
      misc.poly_count = count;

      misc.function = misc.save_function;

      setFunctionCursor (misc.function);

      redrawMap (NVTrue, NVTrue);

      misc.poly_count = 0;
      break;


    case REMISP_FILTER:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      remispFilter (&misc, &options, mx, my, count);

      redrawMap (NVTrue, NVTrue);

      break;


      //  Output manually selected points to a file.

    case OUTPUT_POINTS:
      misc.output_points = (OUTPUT_DATA_POINTS *) realloc (misc.output_points, (misc.output_point_count + 1) * 
                                                           sizeof (OUTPUT_DATA_POINTS));
      misc.output_points[misc.output_point_count].xy.x = lon;
      misc.output_points[misc.output_point_count].xy.y = lat;

      read_bin_record_xy (misc.pfm_handle[0], misc.output_points[misc.output_point_count].xy, &bin);
      misc.output_points[misc.output_point_count].avg = bin.avg_filtered_depth;
      misc.output_points[misc.output_point_count].min = bin.min_filtered_depth;
      misc.output_points[misc.output_point_count].max = bin.max_filtered_depth;

      map->fillCircle (lon, lat, 10, 0.0, 360.0, options.contour_color, NVTrue);

      string.sprintf (tr (" Point #%d").toAscii (), misc.output_point_count);

      map->drawText (string, lon, lat, options.contour_color, NVTrue);

      misc.output_point_count++;


      if (!QDir (options.output_points_dir).exists ()) options.output_points_dir = options.input_pfm_dir;


      fd = new QFileDialog (this, tr ("pfmView Output Points File"));
      fd->setViewMode (QFileDialog::List);


      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
      //  This function is in the nvutility library.

      setSidebarUrls (fd, options.output_points_dir);


      filters << tr ("Points (*.pts)");

      fd->setFilters (filters);
      fd->setFileMode (QFileDialog::AnyFile);
      fd->selectFilter (tr ("Points (*.pts)"));

      if (fd->exec () == QDialog::Accepted)
        {
          files = fd->selectedFiles ();

          file = files.at (0);


          if (!file.isEmpty())
            {
              //  Add .pts to filename if not there.

              if (!file.endsWith (".pts")) file.append (".pts");
 
              strcpy (fname, file.toAscii ());


              if ((fp = fopen (fname, "w")) != NULL)
                {
                  for (NV_INT32 i = 0 ; i < misc.output_point_count ; i++)
                    fprintf (fp, tr ("%.9f, %.9f, Point #%d %f %f %f\n").toAscii (), misc.output_points[i].xy.y, 
                             misc.output_points[i].xy.x, i, misc.output_points[i].avg, 
                             misc.output_points[i].min, misc.output_points[i].max);

                  fclose (fp);


                  //  Add the new file to the first available overlay slot.

                  for (NV_INT32 i = 0 ; i < NUM_OVERLAYS ; i++)
                    {
                      if (!misc.overlays[i].file_type)
                        {
                          misc.overlays[i].file_type = GENERIC_YXZ;
                          strcpy (misc.overlays[i].filename, file.toAscii ());
                          misc.overlays[i].display = NVTrue;
                          misc.overlays[i].color = options.contour_color;

                          hit = NVTrue;

                          break;
                        }
                    }
                }
            }
          options.output_points_dir = fd->directory ().absolutePath ();
        }
      if (e != NULL) midMouse (e->x (), e->y (), lon, lat);

      map->setToolTip ("");      
      misc.function = misc.save_function;

      setFunctionCursor (misc.function);

      if (hit) redrawMap (NVTrue, NVFalse);

      break;
    }
}



//!  Right mouse press.  Called from the map mouse press callback.  This pops up the right mouse click menu.

void 
pfmView::rightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  QString tmp;

  menu_cursor_lon = lon;
  menu_cursor_lat = lat;
  menu_cursor_x = mouse_x;
  menu_cursor_y = mouse_y;


  QPoint pos (mouse_x, mouse_y);


  //  Popups need global positioning

  QPoint global_pos = map->mapToGlobal (pos);


  popup4->setVisible (FALSE);


  //  Define the menu entrys based on the active function.

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
      popup0->setText (tr ("Close rectangle and zoom in"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DEFINE_RECT_AREA:
      popup0->setText (tr ("Close rectangle and save to file"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      popup0->setText (tr ("Close rectangle and edit area"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case SELECT_HIGH_POINT:
      popup0->setText (tr ("Close rectangle and select highest point"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case SELECT_LOW_POINT:
      popup0->setText (tr ("Close rectangle and select lowest point"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case REMISP_AREA:
      popup0->setText (tr ("Close rectangle and regrid area"));
      tmp.sprintf (tr ("Set MISP weight factor (%d)").toAscii (), options.misp_weight);
      popup1->setText (tmp);
      if (options.misp_force_original)
        {
          tmp.sprintf (tr ("Set MISP force original input (Yes)").toAscii ());
        }
      else
        {
          tmp.sprintf (tr ("Set MISP force original input (No)").toAscii ());
        }
      popup2->setText (tmp);
      popup2->setVisible (TRUE);
      popup3->setText (tr ("Discard rectangle"));
      popup3->setVisible (TRUE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
      popup0->setText (tr ("End contour"));
      tmp.sprintf (tr ("Set depth for contour (%f)").toAscii (), misc.draw_contour_level);
      popup1->setText (tmp);
      popup2->setText (tr ("Grab nearest contour depth for contour"));
      popup3->setText (tr ("Discard contour"));

      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          popup0->setVisible (TRUE);
          popup1->setVisible (FALSE);
          popup2->setVisible (FALSE);
          popup3->setVisible (TRUE);
        }
      else
        {
          popup0->setVisible (FALSE);
          popup1->setVisible (TRUE);
          popup2->setVisible (TRUE);
          popup3->setVisible (FALSE);
        }
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DEFINE_POLY_AREA:
      popup0->setText (tr ("Close polygon and save to file"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DEFINE_FEATURE_POLY_AREA:
      popup0->setText (tr ("Close polygon and save to feature file"));
      popup1->setText (tr ("Discard polygon"));
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

    case GRAB_CONTOUR:
      popup0->setText (tr ("Close polygon and capture contours"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case REMISP_FILTER:
      popup0->setText (tr ("Close polygon, filter, and remisp the area"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case RECT_FILTER_AREA:
      popup0->setText (tr ("Close rectangle and filter area"));
      tmp.sprintf (tr ("Set filter standard deviation (%.1f)").toAscii (), options.filterSTD);
      popup1->setText (tmp);
      if (options.deep_filter_only)
        {
          tmp.sprintf (tr ("Set deep filter only (Yes)").toAscii ());
        }
      else
        {
          tmp.sprintf (tr ("Set deep filter only (No)").toAscii ());
        }
      popup2->setText (tmp);
      popup3->setText (tr ("Discard rectangle"));
      popup2->setVisible (TRUE);
      popup3->setVisible (TRUE);
      popup4->setVisible (TRUE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case POLY_FILTER_AREA:
      popup0->setText (tr ("Close polygon and filter area"));
      tmp.sprintf (tr ("Set filter standard deviation (%.1f)").toAscii (), options.filterSTD);
      popup1->setText (tmp);
      if (options.deep_filter_only)
        {
          tmp.sprintf (tr ("Set deep filter only (Yes)").toAscii ());
        }
      else
        {
          tmp.sprintf (tr ("Set deep filter only (No)").toAscii ());
        }
      popup2->setText (tmp);
      popup3->setText (tr ("Discard polygon"));
      popup2->setVisible (TRUE);
      popup3->setVisible (TRUE);
      popup4->setVisible (TRUE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case RECT_FILTER_MASK:
      popup0->setText (tr ("Close rectangle and save mask"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setText (tr ("Clear all masks"));
      popup2->setVisible (TRUE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case POLY_FILTER_MASK:
      popup0->setText (tr ("Close polygon and save mask"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setText (tr ("Clear all masks"));
      popup2->setVisible (TRUE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case OUTPUT_POINTS:
      popup0->setText (tr ("Save points to file"));
      popup1->setText (tr ("Discard points"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DELETE_FEATURE:
      popup0->setText (tr ("Delete feature"));
      popup1->setText (tr ("Cancel"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case EDIT_FEATURE:
      popup0->setText (tr ("Edit feature"));
      popup1->setText (tr ("Highlight features (text search)"));
      popup2->setText (tr ("Cancel"));
      popup2->setVisible (TRUE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case ADD_FEATURE:
      popup0->setText (tr ("Add feature"));
      popup1->setText (tr ("Cancel"));
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case SUNANGLE:
      popup0->setText (tr ("Set sun angle"));
      popup1->setText (tr ("Leave set sun angle mode"));
      popup2->setVisible (FALSE);
      popup3->setVisible (FALSE);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;
    }
}



//!  Right click popup menu first entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu0 ()
{
  popup_active = NVFalse;

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
    case DEFINE_RECT_AREA:
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
    case RECT_FILTER_AREA:
    case RECT_FILTER_MASK:
    case DELETE_FEATURE:
    case ADD_FEATURE:
    case EDIT_FEATURE:
    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
    case REMISP_AREA:
    case SUNANGLE:
      leftMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;

    case DEFINE_POLY_AREA:
    case DEFINE_FEATURE_POLY_AREA:
    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case OUTPUT_POINTS:
    case REMISP_FILTER:
    case GRAB_CONTOUR:


      //  Fake out slotMouseDoubleClick so that it won't decrement the point count.

      double_click = NVTrue;

      slotMouseDoubleClick (NULL, menu_cursor_lon, menu_cursor_lat);
      break;
    }

}



//!  Right click popup menu second entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu1 ()
{
  popup_active = NVFalse;

  bool ok;
  NV_FLOAT64 res;
  NV_INT32 ires;
  QString text;

  switch (misc.function)
    {
    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
      NV_FLOAT64 ret;
      ret = QInputDialog::getDouble (this, tr ("pfmView"), tr ("Enter contour level:"), misc.draw_contour_level, -20000.0, 20000.0, 1, &ok);

      if (ok)
        {
          if (ret > misc.abe_share->open_args[0].max_depth || ret < -misc.abe_share->open_args[0].offset)
            {
              QMessageBox::warning (this, tr ("pfmView value out of range"), 
                                    tr ("Contour value entered is out of range for this PFM structure."));
            }
          else
            {
              misc.draw_contour_level = (NV_FLOAT32) ret;
            }
        }
      break;

    case ADD_FEATURE:
      break;

    case POLY_FILTER_AREA:
    case RECT_FILTER_AREA:
      res = QInputDialog::getDouble (this, tr ("pfmView"), tr ("Enter filter standard deviation (1.0 - 3.0):"), options.filterSTD, 1.0, 4.0, 1, &ok);
      if (ok) options.filterSTD = (NV_FLOAT32) res;
      break;

    case POLY_FILTER_MASK:
    case RECT_FILTER_MASK:
    case REMISP_FILTER:
      midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;

    case EDIT_FEATURE:
      text = QInputDialog::getText (this, tr ("pfmView"), tr ("Enter text to search for:"), QLineEdit::Normal, QString::null, &ok);
      if (ok && !text.isEmpty ())
        {
          options.feature_search_string = text;
        }
      else
        {
          options.feature_search_string = "";
        }
      redrawMap (NVTrue, NVFalse);

      strcpy (misc.abe_share->feature_search_string, options.feature_search_string.toAscii ());
      misc.abe_share->feature_search_type = options.feature_search_type;
      misc.abe_share->feature_search_invert = options.feature_search_invert;

      if (threeDProc)
        {
          if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
        }
      break;

    case REMISP_AREA:
      ires = QInputDialog::getInteger (this, tr ("pfmView"), tr ("Enter MISP weight:"), options.misp_weight, 1, 3, 1, &ok);

      if (ok) options.misp_weight = ires;
      break;

    case OUTPUT_POINTS:
      midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;
    }
}



//!  Right click popup menu third entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu2 ()
{
  QStringList lst, items;
  QString res, item;
  NV_INT32 value, current;
  bool ok;

  popup_active = NVFalse;

  switch (misc.function)
    {
    case RECT_FILTER_AREA:
    case POLY_FILTER_AREA:
      value = 0;
      if (!options.deep_filter_only) value = 1;

      lst << tr ("Yes") << tr ("No");

      res = QInputDialog::getItem (this, tr ("pfmView"), tr ("Deep filter only:"), lst, value, FALSE, &ok);

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

    case REMISP_AREA:
      items << tr ("Yes") << tr ("No");

      current = 1;
      if (options.misp_force_original) current = 0;

      item = QInputDialog::getItem (this, tr ("pfmView"),
                                    tr ("MISP force original input value:"), items, current, false, &ok);
      if (ok && !item.isEmpty ())
        {
          if (item == tr ("Yes"))
            {
              options.misp_force_original = NVTrue;
            }
          else
            {
              options.misp_force_original = NVFalse;
            }
        }
      break;

    case RECT_FILTER_MASK:
    case POLY_FILTER_MASK:
      clearFilterMasks ();

      redrawMap (NVTrue, NVFalse);
      break;

    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
    case ADD_FEATURE:
    case EDIT_FEATURE:
      midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;
    }
}



//!  Right click popup menu fourth entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu3 ()
{
  popup_active = NVFalse;

  switch (misc.function)
    {
    case POLY_FILTER_AREA:
    case RECT_FILTER_AREA:
    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
    case REMISP_AREA:
      midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;
    }
}



//!  Right click popup menu fifth entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu4 ()
{
  popup_active = NVFalse;

  if (misc.function == POLY_FILTER_AREA || misc.function == RECT_FILTER_AREA)
    {
      clearFilterMasks ();
      redrawMap (NVTrue, NVFalse);
    }
}



//!  Right click popup menu help entry has been selected.  Display help based on the active function.

void 
pfmView::slotPopupHelp ()
{
  switch (misc.function)
    {
    case ZOOM_IN_AREA:
      QWhatsThis::showText (QCursor::pos ( ), zoomInText, map);
      break;

    case DEFINE_RECT_AREA:
      QWhatsThis::showText (QCursor::pos ( ), defineRectAreaText, map);
      break;

    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      QWhatsThis::showText (QCursor::pos ( ), editRectText, map);
      break;

    case SELECT_HIGH_POINT:
      QWhatsThis::showText (QCursor::pos ( ), highText, map);
      break;

    case SELECT_LOW_POINT:
      QWhatsThis::showText (QCursor::pos ( ), lowText, map);
      break;

    case REMISP_AREA:
      QWhatsThis::showText (QCursor::pos ( ), remispText, map);
      break;

    case REMISP_FILTER:
      QWhatsThis::showText (QCursor::pos ( ), remispFilterText, map);
      break;

    case DEFINE_POLY_AREA:
      QWhatsThis::showText (QCursor::pos ( ), definePolyAreaText, map);
      break;

    case DEFINE_FEATURE_POLY_AREA:
      QWhatsThis::showText (QCursor::pos ( ), defineFeaturePolyAreaText, map);
      break;

    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      QWhatsThis::showText (QCursor::pos ( ), editPolyText, map);
      break;

    case RECT_FILTER_AREA:
      QWhatsThis::showText (QCursor::pos ( ), filterRectText, map);
      break;

    case POLY_FILTER_AREA:
      QWhatsThis::showText (QCursor::pos ( ), filterPolyText, map);
      break;

    case RECT_FILTER_MASK:
      QWhatsThis::showText (QCursor::pos ( ), filterRectMaskText, map);
      break;

    case POLY_FILTER_MASK:
      QWhatsThis::showText (QCursor::pos ( ), filterPolyMaskText, map);
      break;

    case OUTPUT_POINTS:
      QWhatsThis::showText (QCursor::pos ( ), outputDataPointsText, map);
      break;

    case DELETE_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), deleteFeatureText, map);
      break;

    case ADD_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), addFeatureText, map);
      break;

    case EDIT_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), editFeatureText, map);
      break;

    case SUNANGLE:
      QWhatsThis::showText (QCursor::pos ( ), sunangleText, map);
      break;

    case DRAW_CONTOUR:
      QWhatsThis::showText (QCursor::pos ( ), drawContourText, map);
      break;

    case GRAB_CONTOUR:
      QWhatsThis::showText (QCursor::pos ( ), grabContourText, map);
      break;

    case DRAW_CONTOUR_FILTER:
      QWhatsThis::showText (QCursor::pos ( ), drawContourFilterText, map);
      break;
    }


  //  Discard the operation.

  midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
}



/*!
  Timer - timeout signal.  Very much like an X workproc.  This tracks the cursor in associated (linked or child)
  programs not in this window.  This is active whenever the mouse leaves this window.
*/

void
pfmView::slotTrackCursor ()
{
  NV_CHAR            ltstring[25], lnstring[25];
  QString            string, x_string, y_string;
  static NV_FLOAT64  prev_child_x = -999.0, prev_child_y = -999.0, prev_linked_x = -999.0, prev_linked_y = -999.0;
  static NV_INT32    prev_linked_command_num = 0;


  if (misc.drawing || !cov_area_defined) return;


  //  pfm3D (the 3D bin surface viewer) may issue a request to edit so we're going to implement it here.

  if (misc.abe_share->key == PFMVIEW_FORCE_EDIT || misc.abe_share->key == PFMVIEW_FORCE_EDIT_3D)
    {
      map->setMovingPolygon (&pfm3D_polygon, misc.abe_share->polygon_count, misc.abe_share->polygon_x, misc.abe_share->polygon_y, 
                             options.contour_color, LINE_WIDTH, NVFalse, Qt::SolidLine);

      qApp->processEvents ();


      if (misc.abe_share->key == PFMVIEW_FORCE_EDIT_3D) force_3d_edit = NVTrue;

      editCommand (misc.abe_share->polygon_x, misc.abe_share->polygon_y, misc.abe_share->polygon_count);

      force_3d_edit = NVFalse;

      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED;

      return;
    }


  //  If we received a bunch of resize signals (see slotResize) and we're not currently drawing
  //  the map, we want to make sure we haven't received a resize for about two seconds.  If that's the
  //  case we will redraw.

  if (need_redraw)
    {
      redraw_count++;
      if (redraw_count > 80)
        {
          redrawMap (NVTrue, NVFalse);
          need_redraw = NVFalse;
          redraw_count = 0;
        }
    }


  //  If we have changed the feature file contents in one of our children, we need to reload the features.

  if (misc.abe_share->key == FEATURE_FILE_MODIFIED)
    {
      readFeature (this, &misc);


      //redrawMap (NVTrue, NVFalse);


      //  Wait 2 seconds before resetting key so that all associated programs will see the modified flag.

#ifdef NVWIN3X
      Sleep (2000);
#else
      sleep (2);
#endif

      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED; 
    }


  //  Checking for pfmEdit(3D) requesting a zoom in mosaicView

  if (misc.linked && misc.abe_share->zoom_requested)
    { 
      ABE_REGISTER_COMMAND command;
      command.id = ZOOM_TO_MBR_COMMAND;
      command.window_id = misc.process_id;
      command.mbr = misc.abe_share->displayed_area;
      command.number = rand ();

      misc.abe_register->command[abe_register_group] = command;
      
      misc.abe_share->zoom_requested = NVFalse;
    }


  //  Boy, is this weird!  We might be getting position information from one of two sources.  Either from a child process
  //  or from a linked application.  If it's a child and we're linked we need to inform other linked processes about the
  //  cursor location.  If it's a linked app we need to inform the child processes about the location.  In order to figure
  //  out which one it's from we have to check for change in either the abe_register memory or the abe_share memory.


  NV_BOOL hit = NVFalse;
  NV_FLOAT64 lat = 0.0;
  NV_FLOAT64 lon = 0.0;


  //  This is the case where one of our children has focus.

  if (misc.abe_share->active_window_id != misc.process_id && (prev_child_x != misc.abe_share->cursor_position.x ||
                                                              prev_child_y != misc.abe_share->cursor_position.y))
    {
      lat = misc.abe_share->cursor_position.y;
      lon = misc.abe_share->cursor_position.x;

      prev_child_x = lon;
      prev_child_y = lat;

      if (lat >= misc.total_displayed_area.min_y && lat <= misc.total_displayed_area.max_y && 
          lon >= misc.total_displayed_area.min_x && lon <= misc.total_displayed_area.max_x) hit = NVTrue;


      //  Tell other apps what our child is doing.

      if (misc.linked)
        {
          misc.abe_register->data[abe_register_group].active_window_id = misc.abe_share->active_window_id;
          misc.abe_register->data[abe_register_group].cursor_position.x = lon;
          misc.abe_register->data[abe_register_group].cursor_position.y = lat;
        }
    }


  //  This is the case where some other app has focus.

  else if (misc.linked && misc.abe_register->data[abe_register_group].active_window_id != misc.process_id &&
           (prev_linked_x != misc.abe_register->data[abe_register_group].cursor_position.x ||
            prev_linked_y != misc.abe_register->data[abe_register_group].cursor_position.y))
    {

      //  Check for a command, make sure it's a new command (we haven't already done it), and make sure
      //  it's not one that we issued.

      if (misc.abe_register->command[abe_register_group].number != prev_linked_command_num && 
          misc.abe_register->command[abe_register_group].window_id != misc.process_id)
        {
          prev_linked_command_num = misc.abe_register->command[abe_register_group].number;

          switch (misc.abe_register->command[abe_register_group].id)
            {
            case ZOOM_TO_MBR_COMMAND:
              misc.clear = NVTrue;

              misc.GeoTIFF_init = NVTrue;

              zoomIn (misc.abe_register->command[abe_register_group].mbr, NVTrue);


              misc.cov_function = COV_START_DRAW_RECTANGLE;

              redrawCovBounds ();

              map->setToolTip ("");      
              misc.function = misc.save_function;

              setFunctionCursor (misc.function);

              break;
            }
        }


      //  Get the geographic location of the cursor from the other application.

      lat = misc.abe_register->data[abe_register_group].cursor_position.y;
      lon = misc.abe_register->data[abe_register_group].cursor_position.x;

      prev_linked_x = lon;
      prev_linked_y = lat;

      if (lat >= misc.total_displayed_area.min_y && lat <= misc.total_displayed_area.max_y && 
          lon >= misc.total_displayed_area.min_x && lon <= misc.total_displayed_area.max_x) hit = NVTrue;


      //  Tell our children what the other apps are doing.

      misc.abe_share->active_window_id = misc.abe_register->data[abe_register_group].active_window_id;
      misc.abe_share->cursor_position.x = lon;
      misc.abe_share->cursor_position.y = lat;
    }


  //  If the geographic position of the cursor in the other application is in our displayed area set the marker and the
  //  status bar information.

  if (hit)
    {
      NV_FLOAT64 deg, min, sec;
      NV_CHAR    hem;


      strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));

      latLabel->setText (ltstring);
      lonLabel->setText (lnstring);


      QBrush b1;
      map->setMovingPath (&mv_tracker, marker, lon, lat, 2, options.contour_color, b1, NVFalse, Qt::SolidLine);
    }
}



//!  Mouse press signal from the map class.

void 
pfmView::slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  if (!misc.drawing && !pfm_edit_active && !process_running)
    {
      //  Check for dateline.

      if (misc.dateline && lon < 0.0) lon += 360.0;


      if (e->button () == Qt::LeftButton)
        {
          //  Bring the nearest PFM layer to the top level if Ctrl-left-click.

          if (e->modifiers () == Qt::ControlModifier && misc.nearest_pfm > 0)
            {
              slotLayerGrpTriggered (layer[misc.nearest_pfm]);
            }
          else
            {
              leftMouse (e->x (), e->y (), lon, lat);
            }
        }
      else if (e->button () == Qt::MidButton)
        {
          midMouse (e->x (), e->y (), lon, lat);
        }
      else if (e->button () == Qt::RightButton)
        {
          rightMouse (e->x (), e->y (), lon, lat);
        }
    }
}



/*!
  Mouse press signal prior to signals being enabled from the map class.  This is just used to pop up the help verbiage
  if you haven't opened a PFM file yet.
*/

void 
pfmView::slotPreliminaryMousePress (QMouseEvent *e)
{
  QPoint pos = QPoint (e->x (), e->y ());

  QWhatsThis::showText (pos, mapText, map);
}



//!  Mouse release signal from the map class.

void 
pfmView::slotMouseRelease (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)))
{
  if (!misc.drawing && !pfm_edit_active)
    {
      if (e->button () == Qt::LeftButton) popup_active = NVFalse;
      if (e->button () == Qt::MidButton) popup_active = NVFalse;
      if (e->button () == Qt::RightButton) {};
    }
}



//!  Mouse move signal from the map class.

void
pfmView::slotMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  NV_CHAR              ltstring[25], lnstring[25], hem;
  QString              string, x_string, y_string, num_string, stddev_string, min_string, avg_string, max_string;
  NV_FLOAT64           deg, min, sec;
  BIN_RECORD           bin;
  NV_F64_COORD2        xy;
  NV_I32_COORD2        hot = {-1, -1}, ixy, prev_xy = {-1, -1};
  static NV_I32_COORD2 prev_hot;
  static NV_FLOAT32    prev_z_factor = 1.0;
  static NV_FLOAT32    prev_z_offset = 0.0;



  //  Don't worry about the mouse if we're drawing

  if (misc.drawing) return;


  //  Check for dateline.

  if (misc.dateline && lon < 0.0) lon += 360.0;


  //  Get rid of the tracking cursor from slotTrackCursor ()since, if we got a move signal, we're obviously in
  //  this window.  We only want to kill it if it already exists, otherwise we will be creating a new one (which
  //  we don't want to do).

  if (mv_tracker >= 0) map->closeMovingPath (&mv_tracker);


  //  Let other ABE programs know which window we're in.

  misc.abe_share->active_window_id = misc.process_id;


  //  Track the cursor position for other ABE programs

  misc.abe_share->cursor_position.y = lat;
  misc.abe_share->cursor_position.x = lon;


  //  If we're linked to other apps (non-child) we need to let them know as well.

  if (misc.linked)
    {
      misc.abe_register->data[abe_register_group].active_window_id = misc.process_id;
      misc.abe_register->data[abe_register_group].cursor_position.y = lat;
      misc.abe_register->data[abe_register_group].cursor_position.x = lon;
    }


  //  Save the cursor posiyion in screen space (pixels).

  ixy.x = e->x ();
  ixy.y = e->y ();

  misc.nearest_feature = -1;


  //  If the cursor position has changed we want to set the status display and markers (also other goodies).

  if (lat >= misc.total_displayed_area.min_y && lat <= misc.total_displayed_area.max_y && 
      lon >= misc.total_displayed_area.min_x && lon <= misc.total_displayed_area.max_x)
    {
      strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));


      //  Try to find the highest layer with a valid value to display in the status bar.

      xy.y = lat;
      xy.x = lon;
      NV_INT32 hit = -1;

      for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
        {
          if (misc.abe_share->display_pfm[pfm] && bin_inside_ptr (&misc.abe_share->open_args[pfm].head, xy))
            {
              read_bin_record_xy (misc.pfm_handle[pfm], xy, &bin);


              //  Check the validity.

              if ((bin.validity & PFM_DATA) || ((bin.validity & PFM_INTERPOLATED) && misc.abe_share->layer_type == AVERAGE_FILTERED_DEPTH))
                {
                  hit = pfm;
                  break;
                }
            }
        }


      //  We only change the status bar PFM values if we were actually over a PFM data area

      if (hit >= 0)
        {
          misc.nearest_pfm = hit;


          //  If misc.abe_share->open_args.head.x_bin_size_degrees = misc.abe_share->open_args.head.y_bin_size_degrees
          //  we're using lat/lon bin size in minutes.

          QString tmp;
          if (misc.otf_surface)
            {
              tmp += QString ("%1").arg (options.otf_bin_size_meters, 0, 'f', 2);

              sizeName->setToolTip (tr ("Bin size in meters"));

              switch (options.layer_type)
                {
                case MIN_FILTERED_DEPTH:
                  pfmLabel->setText (tr ("Min OTF grid"));
                  break;

                case MAX_FILTERED_DEPTH:
                  pfmLabel->setText (tr ("Max OTF grid"));
                  break;

                case AVERAGE_FILTERED_DEPTH:
                  pfmLabel->setText (tr ("Average OTF grid"));
                  break;
                }
            }
          else if (misc.abe_share->open_args[hit].head.x_bin_size_degrees != misc.abe_share->open_args[hit].head.y_bin_size_degrees)
            {
              tmp += QString ("%1").arg (misc.abe_share->open_args[hit].head.bin_size_xy, 0, 'f', 2);

              sizeName->setToolTip (tr ("Bin size in meters"));
              pfmLabel->setText (QFileInfo (misc.abe_share->open_args[hit].list_path).fileName ().remove (".pfm"));
            }
          else
            {
              tmp += QString ("%1").arg (misc.abe_share->open_args[hit].head.y_bin_size_degrees * 60.0, 0, 'f', 2);

              sizeName->setToolTip (tr ("Bin size in minutes"));
              pfmLabel->setText (QFileInfo (misc.abe_share->open_args[hit].list_path).fileName ().remove (".pfm"));
            }

          sizeLabel->setText (tmp);


          num_string.sprintf ("%d", bin.num_soundings);
          stddev_string.sprintf ("%.4f", bin.standard_dev);
          if (misc.surface_val)
            {
              min_string.sprintf ("%.2f", bin.min_filtered_depth * options.z_factor + options.z_offset);
              avg_string.sprintf ("%.2f", bin.avg_filtered_depth * options.z_factor + options.z_offset);
              max_string.sprintf ("%.2f", bin.max_filtered_depth * options.z_factor + options.z_offset);
            }
          else
            {
              min_string.sprintf ("%.2f", bin.min_depth * options.z_factor + options.z_offset);
              avg_string.sprintf ("%.2f", bin.avg_depth * options.z_factor + options.z_offset);
              max_string.sprintf ("%.2f", bin.max_depth * options.z_factor + options.z_offset);
            }


          //  If we're scaling or offsetting the values, color the status background yellow.

          if ((options.z_factor != 1.0 && prev_z_factor == 1.0) || (options.z_offset != 0.0 && prev_z_offset == 0.0))
            {
              minPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
              avgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
              maxPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);

              minLabel->setToolTip (tr ("Scaled/offset minimum Z value for bin"));
              avgLabel->setToolTip (tr ("Scaled/offset average Z value for bin"));
              maxLabel->setToolTip (tr ("Scaled/offset maximum Z value for bin"));
            }
          else if ((options.z_factor == 1.0 && prev_z_factor != 1.0) || (options.z_offset == 0.0 && prev_z_offset != 0.0))
            {
              minPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
              avgPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);
              maxPalette.setColor (QPalette::Normal, QPalette::Window, misc.widgetBackgroundColor);

              minLabel->setToolTip (tr ("Minimum Z value for bin"));
              avgLabel->setToolTip (tr ("Average Z value for bin"));
              maxLabel->setToolTip (tr ("Maximum Z value for bin"));
            }
          minLabel->setPalette (minPalette);
          avgLabel->setPalette (avgPalette);
          maxLabel->setPalette (maxPalette);
          prev_z_factor = options.z_factor;
          prev_z_offset = options.z_offset;

          numLabel->setText (num_string);
          stdLabel->setText (stddev_string);
          minLabel->setText (min_string);
          avgLabel->setText (avg_string);
          maxLabel->setText (max_string);


          //  Populate the attribute value status bar.

          for (NV_INT32 i = 0 ; i < misc.abe_share->open_args[hit].head.num_bin_attr ; i++)
            {
              QString attr_string;

              attr_string.sprintf (misc.attr_format[hit][i], bin.attr[i]);

              attrLabel[i]->setText (attr_string);
            }
        }

      latLabel->setText (ltstring);
      lonLabel->setText (lnstring);
    }


  NV_FLOAT64 anc_lat, anc_lon, az, dist, min_dist, dz, temp_angle;
  NV_INT32 xyz_x, xyz_y, xyz_z;
  QBrush b1;
  QColor c1;


  //  Actions based on the active function.  Here we're usually draggin a rubberband rectangle, line, or polygon line based
  //  on the active function.

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
    case RECT_FILTER_AREA:
    case RECT_FILTER_MASK:
    case REMISP_AREA:
    case DEFINE_RECT_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->dragRubberbandRectangle (rb_rectangle, lon, lat);

          map->getRubberbandRectangleAnchor (rb_rectangle, &anc_lon, &anc_lat);

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, lon, &dist, &az);


          string.sprintf (tr (" Distance from anchor in meters: %.2f ").toAscii (), dist);
          misc.statusProgLabel->setText (string);
        }
      break;

    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case DEFINE_POLY_AREA:
    case DEFINE_FEATURE_POLY_AREA:
    case REMISP_FILTER:
    case GRAB_CONTOUR:
      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          map->dragRubberbandPolygon (rb_polygon, lon, lat);

          map->getRubberbandPolygonCurrentAnchor (rb_polygon, &anc_lon, &anc_lat);

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, lon, &dist, &az);

 
          string.sprintf (tr (" Distance from anchor in meters: %.2f ").toAscii (), dist);
          misc.statusProgLabel->setText (string);
        }
      break;

    case OUTPUT_POINTS:
      string.sprintf (tr (" Point #: %d ").toAscii (), misc.output_point_count);
      misc.statusProgLabel->setText (string);
      break;

    case SUNANGLE:
      temp_angle = (atan2 ((NV_FLOAT64) (misc.center_point.y - ixy.y), 
                           (NV_FLOAT64) (misc.center_point.x - ixy.x))) / NV_DEG_TO_RAD;
      temp_angle = 360.0 - fmod (270.0 - temp_angle, 360.0);

      b1.setStyle (Qt::SolidPattern);
      c1 = QColor (options.contour_color);
      c1.setAlpha (128);
      b1.setColor (c1);
      map->setMovingPath (&mv_arrow, arrow, misc.center_point.x, misc.center_point.y, LINE_WIDTH,
                          options.contour_color, b1, NVTrue, temp_angle, Qt::SolidLine);

      temp_angle = fmod (temp_angle + 180.0, 360.0);
      string.sprintf (tr (" Sun Angle: %.1f ").toAscii (), temp_angle);
      misc.statusProgLabel->setText (string);
      break;

    case EDIT_FEATURE:
    case DELETE_FEATURE:
      min_dist = 999999999.0;

      for (NV_U_INT32 i = 0 ; i < misc.bfd_header.number_of_records ; i++)
        {
          if (misc.feature[i].confidence_level || !misc.surface_val)
            {
              if (misc.feature[i].latitude >= misc.total_displayed_area.min_y &&
                  misc.feature[i].latitude <= misc.total_displayed_area.max_y && 
                  misc.feature[i].longitude >= misc.total_displayed_area.min_x &&
                  misc.feature[i].longitude <= misc.total_displayed_area.max_x)
                {
                  if (options.display_children || !misc.feature[i].parent_record)
                    {
                      map->map_to_screen (1, &misc.feature[i].longitude, &misc.feature[i].latitude, &dz, &xyz_x, &xyz_y, &xyz_z);

                      dist = sqrt ((NV_FLOAT64) ((ixy.y - xyz_y) * (ixy.y - xyz_y)) + 
                                   (NV_FLOAT64) ((ixy.x - xyz_x) * (ixy.x - xyz_x)));

                      if (dist < min_dist)
                        {
                          misc.nearest_feature = i;
                          hot.x = xyz_x;
                          hot.y = xyz_y;
                          min_dist = dist;
                        }
                    }
                }
            }
        }

      map->setMovingPath (&mv_marker, marker, hot.x, hot.y, LINE_WIDTH, options.contour_color, b1, NVFalse,
                          Qt::SolidLine);

      prev_hot = hot;

      break;

    case ADD_FEATURE:
      break;

    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:

      //  Only process if the pointer position has changed pixels.

      if ((ixy.x != prev_xy.x || ixy.y != prev_xy.y) && map->rubberbandPolygonIsActive (rb_polygon))
        {
          xy.y = lat;
          xy.x = lon;


          if (prev_poly_lon > -180.5) map->vertexRubberbandPolygon (rb_polygon, prev_poly_lon, prev_poly_lat);
          map->dragRubberbandPolygon (rb_polygon, lon, lat);
          prev_poly_lat = lat;
          prev_poly_lon = lon;

          QString msc;
          msc.sprintf (tr (" Draw contour - %.2f ").toAscii (), misc.draw_contour_level);
          misc.statusProgLabel->setText (msc);
          misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
          misc.statusProgLabel->setPalette (misc.statusProgPalette);
        }
      break;
    }


  //  Set the previous cursor.
             
  prev_xy = ixy;
}



//!  Resize signal from the map class.

void
pfmView::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  //  The first time we get a resize (or if we have closed all of the open PFM files) we want to display the 
  //  fabulously elegant PFM splash screen ;-)

  if (logo_first)
    {
      QPixmap *logo = new QPixmap (":/icons/PFMLogo.png");

      NV_INT32 w = map->width () - 3 * mapdef.border;
      NV_INT32 h = map->height () - 3 * mapdef.border;

      QPixmap *logo_scaled = new QPixmap (logo->scaled (w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

      delete logo;

      map->drawPixmap (mapdef.border, mapdef.border, logo_scaled, 0, 0, w, h, NVTrue);

      delete logo_scaled;

      map->setCursor (Qt::WhatsThisCursor);


      //  Display the new features (startup) message if needed.

      if (startup && options.startup_message)
        {
          startupMessage *sm = new startupMessage (this, &options, new_feature);


          //  Put the dialog in the middle of the screen.

          sm->move (x () + width () / 2 - sm->width () / 2, y () + height () / 2 - sm->height () / 2);

          startup = NVFalse;
        }
    }
  else
    {
      if (misc.drawing) return;


      //  If the windowing system is doing "Display content in resizing windows" we'll get about a million
      //  resize callbacks all stacked up.  What we want to do is use the trackCursor function to wait for
      //  2 seconds of inactivity (i.e. no resizes) and then redraw the map.  We use the redraw_count to 
      //  time it.  I thought this was easier than starting and killing a one-shot timer every time we
      //  came through this section of code.

      need_redraw = NVTrue;
      redraw_count = 0;
      misc.GeoTIFF_init = NVTrue;

      map->setCursor (Qt::WaitCursor);
      qApp->processEvents ();
    }
}



//!  Prior to drawing coastline signal from the map class.  This is where we do our heavy lifting (actually in paint_surface).

void 
pfmView::slotPreRedraw (NVMAP_DEF l_mapdef)
{
  void paint_surface (MISC *misc, OPTIONS *options, nvMap *map, NVMAP_DEF *mapdef, NV_BOOL cov_area_defined);
  void paint_otf_surface (MISC *misc, OPTIONS *options, nvMap *map, NVMAP_DEF *mapdef, NV_BOOL cov_area_defined);


  //  Make sure we know whether we're using a valid of invalid surface.

  setSurfaceValidity (misc.abe_share->layer_type);


  //  Set the drawing flag and turn the map mouse signals off until we are through painting.

  misc.drawing = NVTrue;
  map->disableMouseSignals ();


  setMainButtons (NVFalse);

  map->setCursor (Qt::WaitCursor);
  cov->setCursor (Qt::WaitCursor);

  qApp->processEvents ();

  mapdef = l_mapdef;


  if (misc.otf_surface)
    {
      paint_otf_surface (&misc, &options, map, &mapdef, cov_area_defined);
    }
  else
    {
      paint_surface (&misc, &options, map, &mapdef, cov_area_defined);
    }
}



/*!
  Move the display left, right, up, or down.  This is called from the keyPressEvent or when a move
  is requested in pfmEdit(3D).
*/

void 
pfmView::moveMap (NV_INT32 direction)
{
  discardMovableObjects ();


  clearFilterMasks ();


  map->moveMap (direction);


  //  We want to redraw the pfm3D view if it's up.

  if (threeDProc)
    {
      if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
    }
}



//!  Zoom in to the specified bounds.

void
pfmView::zoomIn (NV_F64_XYMBR bounds, NV_BOOL pfm3D)
{
  discardMovableObjects ();

  clearFilterMasks ();

  map->zoomIn (bounds);

  bZoomOut->setEnabled (TRUE);


  //  We want to redraw the pfm3D view if it's up.

  if (pfm3D)
    {
      if (threeDProc)
        {
          if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
        }
    }
}



//!  Zoom out one level.

void
pfmView::zoomOut ()
{
  discardMovableObjects ();


  clearFilterMasks ();


  map->zoomOut ();


  //  We want to redraw the pfm3D view if it's up.

  if (threeDProc)
    {
      if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
    }
}



//!  Redraw the entire map.

void
pfmView::redrawMap (NV_BOOL clear, NV_BOOL pfm3D)
{
  misc.clear = NVTrue;

  discardMovableObjects ();


  //  Clear the filter mask displayed flags.

  for (NV_INT32 i = 0 ; i < misc.poly_filter_mask_count ; i++) misc.poly_filter_mask[i].displayed = NVFalse;


  map->redrawMap (clear);


  //  If we changed the surface we want to redraw the pfm3D view if it's up.

  if (pfm3D)
    {
      if (threeDProc)
        {
          if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
        }
    }
}



//!  Slot to call the redrawMap function.

void
pfmView::slotRedraw ()
{
  redrawMap (NVTrue, NVFalse);
}



//!  Slot to toggle auto redraw after edit.

void
pfmView::slotAutoRedraw ()
{
  if (bAutoRedraw->isChecked ())
    {
      options.auto_redraw = NVTrue;
    }
  else
    {
      options.auto_redraw = NVFalse;
    }
}



//!  Redraw the coverage map.

void
pfmView::slotRedrawCoverage ()
{
  misc.cov_clear = NVTrue;

  cov->redrawMap (NVTrue);
}



//!  Using the keys to move around.  We also may be starting the GeoTIFF viewer (mosaicView) or pfm3D.

void
pfmView::keyPressEvent (QKeyEvent *e)
{
  //  Discard key presses if we're drawing or if one of the editors is running.

  if (!misc.drawing && !pfm_edit_active)
    {
      QString qkey = e->text ();


      if (qkey == options.mosaic_hotkey)
        {
          //  Mosaic (GeoTIFF) viewer.

          slotStartMosaicViewer ();
        }
      else if (qkey == options.threeD_hotkey)
        {
          //  3D surface viewer.

          slotStart3DViewer ();
        }


      //  Check for one of three mosaic viewer action keys.

      else if (qkey == options.mosaic_actkey.section (',', 0, 0) ||
               qkey == options.mosaic_actkey.section (',', 1, 1) ||
               qkey == options.mosaic_actkey.section (',', 2, 2))
        {
          misc.abe_share->modcode = 0;
          NV_CHAR key[20];
          strcpy (key, e->text ().toAscii ());
          misc.abe_share->key = (NV_U_INT32) key[0];


          //  Tell other registered ABE applications to zoom to MBR (the first mosaicView action key is zoom to MBR).

          if (misc.linked && qkey == options.mosaic_actkey.section (',', 0, 0))
            {
              ABE_REGISTER_COMMAND command;
              command.id = ZOOM_TO_MBR_COMMAND;
              command.window_id = misc.process_id;
              command.mbr = misc.total_displayed_area;
              command.number = rand ();

              misc.abe_register->command[abe_register_group] = command;
            }
        }


      //  Move the viewing area up, down, left, right, up/right, down/right, up/left, and down/left based on the arrow (or keypad) key pressed.

      else
        {
          misc.clear = NVTrue;
          switch (e->key ())
            {
            case Qt::Key_Left:
              moveMap (NVMAP_LEFT);
              break;

            case Qt::Key_Up:
              moveMap (NVMAP_UP);
              break;

            case Qt::Key_Right:
              moveMap (NVMAP_RIGHT);
              break;

            case Qt::Key_Down:
              moveMap (NVMAP_DOWN);
              break;

            case Qt::Key_PageUp:
              moveMap (NVMAP_UP_RIGHT);
              break;

            case Qt::Key_PageDown:
              moveMap (NVMAP_DOWN_RIGHT);
              break;

            case Qt::Key_Home:
              moveMap (NVMAP_UP_LEFT);
              break;

            case Qt::Key_End:
              moveMap (NVMAP_DOWN_LEFT);
              break;

            default:
              return;
            }

          redrawCovBounds ();
        }
    }
}



//!  Somebody pressed the little X in the window decoration.  DOH!

void 
pfmView::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



void 
pfmView::slotClose (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



//!  Slot called on exit.

void 
pfmView::slotQuit ()
{
  void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);


  //  Check for files in the delete file queue.  If there are any there offer the user the chance to get rid of them.

  if (misc.abe_share->delete_file_queue_count)
    {
      NV_INT32 ret = QMessageBox::Yes;
      ret = QMessageBox::information (this, tr ("pfmView"), tr ("There are files in the delete file queue.  Do you really want to exit?"),
                                      QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

      if (ret == QMessageBox::No) return;
    }


  //  No matter what program started them (i.e. this or pfmEdit(3D)), if we're leaving the viewer we want to nicely terminate
  //  all child processes (or at least try).

  misc.abe_share->key = CHILD_PROCESS_FORCE_EXIT;


  //  Make sure we don't save GeoTIFF or an attribute as the default layer to display.

  if (misc.abe_share->layer_type >= ATTRIBUTE0) misc.abe_share->layer_type = MIN_FILTERED_DEPTH;


  //  Move stuff from share to options before we save the options.

  options.layer_type = misc.abe_share->layer_type;
  options.cint = misc.abe_share->cint;
  options.num_levels = misc.abe_share->num_levels;
  for (NV_INT32 i = 0 ; i < misc.abe_share->num_levels ; i++) options.contour_levels[i] = misc.abe_share->contour_levels[i];


  options.edit_mode = misc.last_edit_function;


  //  Save the user's defaults

  envout (&options, &misc, this);


  //  Close the feature file if it's open

  if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);


  //  Get rid of the shared memory.

  misc.abeShare->detach ();


  //  If we're linked to another program(s), unregister the link.

  if (misc.linked) unregisterABE (misc.abeRegister, misc.abe_register, abe_register_group, misc.process_id);


  //  If we were called from pfmWDBView, output the mod status.

  if (pfmWDBView)
    {
      if (pfmViewMod)
        {
          fprintf (stdout, "View return status:1\n");
        }
      else
        {
          fprintf (stdout, "View return status:0\n");
        }
    }

  exit (0);
}



//!  Post grid drawing signal from map class.  Mostly just cleanup work.

void 
pfmView::slotPostRedraw (NVMAP_DEF mapdef __attribute__ ((unused)))
{
  //  Turn the map mouse signals back on.

  map->enableMouseSignals ();


  logo_first = NVFalse;


  if (cov_area_defined)
    {
      //  Redraw the coverage map if needed.

      if (misc.prev_total_displayed_area.min_x != 999.0 &&
          (misc.prev_total_displayed_area.min_x != misc.total_displayed_area.min_x || 
           misc.prev_total_displayed_area.min_y != misc.total_displayed_area.min_y ||
           misc.prev_total_displayed_area.max_x != misc.total_displayed_area.max_x || 
           misc.prev_total_displayed_area.max_y != misc.total_displayed_area.max_y))
        {
          misc.cov_clear = NVTrue;
          cov->redrawMap (NVFalse);
        }

      setMainButtons (NVTrue);
    }
  misc.prev_total_displayed_area = misc.total_displayed_area;
  misc.drawing_canceled = NVFalse;

  setFunctionCursor (misc.function);

  cov->setCursor (editRectCursor);

  misc.drawing = NVFalse;
}



//!  Open PFM file.  Called when someone uses "open" in the menu or presses the "open" button.

void 
pfmView::slotOpen ()
{
  QStringList files, filters;
  QString file, dir;
  PFM_OPEN_ARGS open_args;
  NV_INT32 pfm_handle = -1;


  NV_BOOL check_recent (QWidget *, OPTIONS *options, MISC *misc, QString file, QActionGroup *recentGrp, QMenu *recentMenu, QAction **recentFileAction);
  void setSidebarUrls (QFileDialog *fd, QString dir);


  NV_BOOL accepted = NVFalse;

  if (recent_file_flag)
    {
      files << options.recentFile[recent_file_flag - 1];
      dir = QFileInfo (options.recentFile[recent_file_flag - 1]).dir ().absolutePath ();

      recent_file_flag = 0;

      accepted = NVTrue;
    }
  else if (command_file_flag)
    {
      files << commandFile;
      dir = QFileInfo (commandFile).dir ().absolutePath ();

      command_file_flag = NVFalse;

      accepted = NVTrue;
    }
  else
    {
      if (!QDir (options.input_pfm_dir).exists ()) options.input_pfm_dir = ".";


      QFileDialog *fd = new QFileDialog (this, tr ("pfmView Open PFM Structure"));
      fd->setViewMode (QFileDialog::List);


      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
      //  This function is in the nvutility library.

      setSidebarUrls (fd, options.input_pfm_dir);


      filters << tr ("PFM (*.pfm)");

      fd->setFilters (filters);
      fd->setFileMode (QFileDialog::ExistingFiles);
      fd->selectFilter (tr ("PFM (*.pfm)"));


      if (fd->exec () == QDialog::Accepted) accepted = NVTrue;

      files = fd->selectedFiles ();
      dir = fd->directory ().absolutePath ();
    }


  if (accepted)
    {
      if ((misc.abe_share->pfm_count + files.size ()) >= MAX_ABE_PFMS)
        {
          QMessageBox::warning (this, tr ("pfmView Open PFM Structure"), tr ("Too many open PFM structures.\n") +
                                tr ("Please close some before trying to open new ones."));
          return;
        }


      for (NV_INT32 i = 0 ; i < files.size () ; i++)
        {
          file = files.at (i);


          if (!file.isEmpty())
            {
              //  Open the file and make sure it is a valid PFM file.

              strcpy (open_args.list_path, file.toAscii ());

              open_args.checkpoint = 0;
              pfm_handle = open_existing_pfm_file (&open_args);

              if (pfm_handle < 0)
                {
                  QMessageBox::warning (this, tr ("pfmView Open PFM Structure"), tr ("The file ") + 
                                        QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                        tr (" is not a PFM structure or there was an error reading the file.") +
                                        tr ("  The error message returned was:\n\n") + QString (pfm_error_str (pfm_error)));

                  if (pfm_error == CHECKPOINT_FILE_EXISTS_ERROR)
                    {
                      fprintf (stderr, "\n\n%s\n", pfm_error_str (pfm_error));
                      slotQuit ();
                    }

                  return;
                }


              //  Check the file to see if we already have it opened.  If we do then we'll just move it to the top level.

              NV_BOOL opened = NVFalse;
              for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
                {
                  if (!strcmp (misc.abe_share->open_args[pfm].list_path, open_args.list_path))
                    {
                      //  No point in moving the top level layer to the top level.

                      if (pfm) slotLayerGrpTriggered (layer[pfm]);

                      opened = NVTrue;
                    }
                }


              if (!opened)
                {
                  //  We don't do projected ;-)

                  if (open_args.head.proj_data.projection)
                    {
                      QMessageBox::warning (this, tr ("pfmView Open PFM Structure"), tr ("Sorry, pfmView only handles geographic PFM structures."));
                      close_pfm_file (pfm_handle);
                      return;
                    }
                  else
                    {
                      //  Check to see if this is one of the recently opened files.  If not, add it to the list.

                      if (!check_recent (this, &options, &misc, file, recentGrp, recentMenu, recentFileAction)) return;


                      for (NV_INT32 pfm = misc.abe_share->pfm_count ; pfm > 0 ; pfm--)
                        {
                          misc.abe_share->open_args[pfm] = misc.abe_share->open_args[pfm - 1];
                          misc.ss_cell_size_x[pfm] = misc.ss_cell_size_x[pfm - 1];
                          misc.ss_cell_size_y[pfm] = misc.ss_cell_size_y[pfm - 1];
                          for (NV_INT32 i = 0 ; i < misc.abe_share->open_args[pfm - 1].head.num_bin_attr ; i++)
                            strcpy (misc.attr_format[pfm][i], misc.attr_format[pfm - 1][i]);
                          misc.pfm_handle[pfm] = misc.pfm_handle[pfm - 1];
                          misc.average_type[pfm] = misc.average_type[pfm - 1];
                          misc.cube_attr_available[pfm] = misc.cube_attr_available[pfm - 1];
                          misc.abe_share->display_pfm[pfm] = misc.abe_share->display_pfm[pfm - 1];
                          misc.pfm_alpha[pfm] = misc.pfm_alpha[pfm - 1];
                          misc.last_saved_contour_record[pfm] = misc.last_saved_contour_record[pfm - 1];


                          QString layerString;
                          layerString.sprintf ("%d - ", pfm + 1);
                          layerString += QFileInfo (misc.abe_share->open_args[pfm].list_path).fileName ();
                          layer[pfm]->setText (layerString);
                          layer[pfm]->setVisible (TRUE);
                        }

                      misc.pfm_handle[0] = pfm_handle;
                      misc.abe_share->open_args[0] = open_args;

                      misc.GeoTIFF_open = NVFalse;
                      misc.display_GeoTIFF = NVFalse;
                      bGeotiff->setEnabled (FALSE);
                      misc.GeoTIFF_init = NVTrue;


                      QString tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[0]) + tr (" data");
                      highlightUser01->setText (tmp);
                      tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[1]) + tr (" data");
                      highlightUser02->setText (tmp);
                      tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[2]) + tr (" data");
                      highlightUser03->setText (tmp);
                      tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[3]) + tr (" data");
                      highlightUser04->setText (tmp);
                      tmp = tr ("Highlight ") + QString (misc.abe_share->open_args[0].head.user_flag_name[4]) + tr (" data");
                      highlightUser05->setText (tmp);


                      //  Default average surface button setup.  We'll check for CUBE right after this.

                      bCube->setEnabled (FALSE);

                      if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "MISP"))
                        {
                          bSurface[0]->setIcon (QIcon  (":/icons/misp_surface.png"));
                          QString tip = tr ("Display MISP surface  [") + options.buttonAccel[DISPLAY_AVG_EDITED_KEY] + "]";
                          misc.button[DISPLAY_AVG_EDITED_KEY]->setToolTip (tip);
                        }
                      else
                        {
                          bSurface[0]->setIcon (misc.buttonIcon[DISPLAY_AVG_EDITED_KEY]);
                          QString tip = misc.buttonText[DISPLAY_AVG_EDITED_KEY] + "  [" +
                            options.buttonAccel[DISPLAY_AVG_EDITED_KEY] + "]";
                          misc.button[DISPLAY_AVG_EDITED_KEY]->setToolTip (tip);
                        }


                      //  Determine the attribute formats, if any

                      misc.cube_attr_available[0] = NVFalse;
                      if (misc.abe_share->open_args[0].head.num_bin_attr)
                        {
                          for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++)
                            {
                              if (i < misc.abe_share->open_args[0].head.num_bin_attr)
                                {
                                  NV_INT32 decimals = (NV_INT32) (log10 (misc.abe_share->open_args[0].head.bin_attr_scale[i]) + 0.05);
                                  sprintf (misc.attr_format[0][i], "%%.%df", decimals);
                                  attrLabel[i]->setToolTip (QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[i])));
                                  attrName[i]->setText (tr ("Attr %1:").arg (i + 1));


                                  //  Check to see if this PFM has CUBE attributes.

                                  if (!strcmp (misc.abe_share->open_args[0].head.bin_attr_name[i], "###0")) misc.cube_attr_available[0] = NVTrue;


                                  QString tmp = tr ("Color by ") + QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[i]));
                                  bColor[i + PRE_ATTR]->setToolTip (tmp);
                                  bColorAction[i + PRE_ATTR]->setVisible (TRUE);
                                }
                              else
                                {
                                  attrName[i]->setText ("");
                                  attrLabel[i]->setText ("");
                                  bColorAction[i + PRE_ATTR]->setVisible (FALSE);
                                }
                            }


                          //  If we have the cube executable and the cube attributes we want to offer the recube button and
                          //  set the average surface button accordingly.

                          if (misc.cube_available && misc.cube_attr_available[0])
                            {
                              bCube->setEnabled (TRUE);

                              if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "CUBE"))
                                {
                                  bSurface[0]->setIcon (QIcon  (":/icons/cube_surface.png"));
                                  QString tip = tr ("Display CUBE surface  [") + options.buttonAccel[DISPLAY_AVG_EDITED_KEY] + "]";
                                  misc.button[DISPLAY_AVG_EDITED_KEY]->setToolTip (tip);
                                }
                            }
                        }
                      else
                        {
                          for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++) bColorAction[i + PRE_ATTR]->setVisible (FALSE);
                          bCube->setEnabled (FALSE);
                        }


                      //  Check for attributes and the color_by_attribute flag.

                      if (misc.abe_share->open_args[0].head.num_bin_attr < misc.color_by_attribute) misc.color_by_attribute = 0;


                      //  Compute cell sizes for sunshading.

                      misc.ss_cell_size_x[0] = misc.abe_share->open_args[0].head.bin_size_xy;
                      misc.ss_cell_size_y[0] = misc.abe_share->open_args[0].head.bin_size_xy;


                      QString layerString = "1 - " + QFileInfo (misc.abe_share->open_args[0].list_path).fileName ();
                      layer[0]->setText (layerString);
                      layer[0]->setVisible (TRUE);
                    }
                }


              //  If the mosaic viewer and/or 3D viewer was running, tell them to redraw.

              if (strcmp (misc.abe_share->open_args[0].target_path, "NONE")) misc.abe_share->key = FEATURE_FILE_MODIFIED;


              misc.abe_share->display_pfm[0] = NVTrue;
              misc.pfm_alpha[0] = 255;

              misc.average_type[0] = 0;


              //  Check to see if we're using something other than the standard AVERAGE EDITED DEPTH (MISP interpolated surface).

              if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "MINIMUM MISP"))
                {
                  misc.average_type[0] = 1;
                }
              else if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "AVERAGE MISP"))
                {
                  misc.average_type[0] = 2;
                }
              else if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "MAXIMUM MISP"))
                {
                  misc.average_type[0] = 3;
                }

              misc.abe_share->display_pfm[0] = NVTrue;


              misc.abe_share->pfm_count++;


              //  If we are editing a new PFM, switch the working directories to be the PFM directory.  Otherwise, we
              //  leave them where they were.

              if (options.input_pfm_dir != dir)
                {
                  options.overlay_dir = dir;
                  options.output_area_dir = dir;
                  options.output_points_dir = dir;
                  options.geotiff_dir = dir;
                  options.area_dir = dir;
                  options.dnc_dir = dir;
                }
              options.input_pfm_dir = dir;
            }
        }


      //  Initialize the coverage and main maps

      initializeMaps ();


      //  Check to see if we have a GeoTIFF file associated with this PFM.

      if (!strcmp (misc.abe_share->open_args[0].image_path, "NONE"))
        {
          //  If the mosaic viewer was running, kill it.

          if (mosaicProc)
            {
              if (mosaicProc->state () == QProcess::Running)
                {
                  disconnect (mosaicProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotMosaicDone (int, QProcess::ExitStatus)));
                  disconnect (mosaicProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotMosaicError (QProcess::ProcessError)));
                  disconnect (mosaicProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotMosaicReadyReadStandardError ()));

                  mosaicProc->kill ();
                }

              delete mosaicProc;
              mosaicProc = NULL;
            }

          startMosaic->setEnabled (FALSE);
        }
      else
        {
          //  If the mosaic viewer was running, tell it to switch files.

          misc.abe_share->key = MOSAICVIEW_FORCE_RELOAD;

          startMosaic->setEnabled (TRUE);
        }
    }
}



//!  Opens a recently used file based on the qaction that was assigned to that file.

void 
pfmView::slotOpenRecent (QAction *action)
{
  for (NV_INT32 i = 0 ; i < MAX_RECENT ; i++)
    {
      if (recentFileAction[i] == action)
        {
          recent_file_flag = i + 1;
          break;
        }
    }
  slotOpen ();
}



//!  Close one of the PFM layers.

void 
pfmView::slotClosePFM ()
{
  if (QMessageBox::information (this, tr ("pfmView Close PFM confirmation"), 
                                tr ("Do you really want to close\n") + 
                                QDir::toNativeSeparators (QString (misc.abe_share->open_args[0].list_path)),
                                QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;


  remove_layer (&misc, 0);

  if (!misc.abe_share->pfm_count) cov_area_defined = NVFalse;

  slotManageLayersDataChanged ();
}



//!  This is where we define the bounds and other things associated with the open PFM(s).

void 
pfmView::initializeMaps ()
{
  //  Set up the PFM layers notebook page filenames

  NV_INT32 num_displayed = 0;
  notebook->setTabEnabled (1, FALSE);
  for (NV_INT32 pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++)
    {
      if (pfm < misc.abe_share->pfm_count)
        {
          QString name = QFileInfo (QString (misc.abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm");

          layerName[pfm]->setText (name);

          if (pfm) notebook->setTabEnabled (1, TRUE);

          if (misc.abe_share->display_pfm[pfm])
            {
              num_displayed++;
              layerCheck[pfm]->setChecked (TRUE);
            }
          else
            {
              layerCheck[pfm]->setChecked (FALSE);
            }
          layerName[pfm]->show ();
          layerCheck[pfm]->show ();
        }
      else
        {
          layerName[pfm]->hide ();
          layerCheck[pfm]->hide ();
        }
    }


  //  Check to see if we have a MISP surface in the top PFM layer.

  if (misc.average_type[0])
    {
      bRemisp->setEnabled (TRUE);
      bRemispFilter->setEnabled (TRUE);
      bDrawContour->setEnabled (TRUE);
      bDrawContourFilter->setEnabled (TRUE);
      bClearFilterContours->setEnabled (TRUE);
    }
  else
    {
      bRemisp->setEnabled (FALSE);
      bRemispFilter->setEnabled (FALSE);
      bDrawContour->setEnabled (FALSE);
      bDrawContourFilter->setEnabled (FALSE);
      bClearFilterContours->setEnabled (FALSE);
    }


  if (misc.linked)
    {
      changeFileRegisterABE (misc.abeRegister, misc.abe_register, abe_register_group, misc.process_id,
                             misc.abe_share->open_args[0].list_path);
      QString title;
      title.sprintf ("pfmView (ABE group %02d) : %s", abe_register_group, misc.abe_share->open_args[0].list_path);
      this->setWindowTitle (title);
    }


  //  Compute the MBR of all of the PFM areas.

  compute_total_mbr (&misc);


  misc.cov_clear = NVTrue;


  //  Try to open the feature file and read the features into memory.

  readFeature (this, &misc);


  NV_F64_XYMBR mbr = mapdef.bounds[mapdef.zoom_level];


  //  Disable the map signals, reset the bounds and then enable the map signals.

  map->disableSignals ();

  clearFilterMasks ();

  map->resetBounds (misc.total_mbr);
  map->enableSignals ();


  //  Don't want to draw coasts or grids when opening the file, but we need to open it to get a size for the coverage map.

  map->setCoasts (NVMAP_NO_COAST);
  map->setLandmask (NVFalse);
  map->setGridSpacing (0.0, 0.0);

  if (cov_area_defined)
    {
      zoomIn (mbr, NVFalse);
    }
  else
    {
      redrawMap (NVTrue, NVFalse);
    }

  map->setCoasts (options.coast);
  map->setLandmask (options.landmask);


  //  Get the size and other goodies

  mapdef = map->getMapdef ();


  //  Redefine the coverage map bounds.

  covdef.initial_bounds.min_x = misc.total_mbr.min_x;
  covdef.initial_bounds.min_y = misc.total_mbr.min_y;
  covdef.initial_bounds.max_x = misc.total_mbr.max_x;
  covdef.initial_bounds.max_y = misc.total_mbr.max_y;


  cov->resetBounds (covdef.initial_bounds);

  cov->enableSignals ();

  covdef = cov->getMapdef ();

  slotRedrawCoverage ();
}



//!  Open a GeoTIFF (image) file.

void 
pfmView::slotOpenGeotiff ()
{
  QStringList files, filters;
  QString file;

  QFileDialog *fd = new QFileDialog (this, tr ("pfmView Open GeoTIFF"));
  fd->setViewMode (QFileDialog::List);

  if (!QDir (options.geotiff_dir).exists ()) options.geotiff_dir = options.input_pfm_dir;


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.geotiff_dir);


  filters << tr ("GeoTIFF (*.tif)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("GeoTIFF (*.tif)"));

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


                          if (GeoTIFF_nlat < misc.total_displayed_area.min_y || 
                              GeoTIFF_slat > misc.total_displayed_area.max_y ||
                              GeoTIFF_elon < misc.total_displayed_area.min_x || 
                              GeoTIFF_wlon > misc.total_displayed_area.max_x)
                            {
                              QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                                    tr ("No part of this GeoTIFF file falls within the displayed area."));
                              return;
                            }
                        }
                      else
                        {
                          QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                                tr ("This program only handles unprojected GeoTIFF files."));
                          return;
                        }
                    }
                  else
                    {
                      QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                            tr ("This program only handles geographic GeoTIFF files."));
                      return;
                    }
                }
              else
                {
                  QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                        tr ("This file has no datum/projection information."));
                  return;
                }
            }
          else
            {
              QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                    tr ("Unable to open file."));
              return;
            }

          delete poDataset;

          misc.GeoTIFF_open = NVTrue;
          bGeotiff->setEnabled (TRUE);

          strcpy (misc.GeoTIFF_name, file.toAscii ());
          misc.GeoTIFF_init = NVTrue;

          bGeotiff->setEnabled (TRUE);
          bGeotiff->setChecked (TRUE);
          misc.display_GeoTIFF = NVTrue;
          redrawMap (NVTrue, NVFalse);

          options.geotiff_dir = fd->directory ().absolutePath ();
        }
    }
}



//!  Kick off the preferences dialog.

void 
pfmView::slotPrefs ()
{
  if (prefs_dialog) prefs_dialog->close ();
  prefs_dialog = new Prefs (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  prefs_dialog->move (x () + width () / 2 - prefs_dialog->width () / 2, y () + height () / 2 - prefs_dialog->height () / 2);

  connect (prefs_dialog, SIGNAL (dataChangedSignal (NV_BOOL)), this, SLOT (slotPrefDataChanged (NV_BOOL)));
  connect (prefs_dialog, SIGNAL (hotKeyChangedSignal (NV_INT32)), this, SLOT (slotPrefHotKeyChanged (NV_INT32)));
}



//!  Changed some of the preferences in the preferences dialog.

void 
pfmView::slotPrefDataChanged (NV_BOOL feature_search_changed)
{
  misc.GeoTIFF_init = NVTrue;


  bContour->setChecked (options.contour);


  bAutoRedraw->setChecked (options.auto_redraw);
  bDisplaySuspect->setChecked (options.display_suspect);
  bDisplaySelected->setChecked (options.display_selected);
  bDisplayReference->setChecked (options.display_reference);
  bDisplayMinMax->setChecked (options.display_minmax);
  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);
  bDisplayChildren->setChecked (options.display_children);
  bDisplayFeatureInfo->setChecked (options.display_feature_info);
  bDisplayFeaturePoly->setChecked (options.display_feature_poly);


  for (NV_INT32 i = 0 ; i < NUM_SURFACES ; i++) bSurface[i]->setChecked (FALSE);

  bSurface[misc.abe_share->layer_type - 2]->setChecked (TRUE);

 
 //  Set the tooltip for the bin size button just in case it changed.

  QString tip = tr ("Set the on-the-fly gridding bin size in meters (%1)").arg (options.otf_bin_size_meters, 3, 'f', 1);
  bSetOtfBin->setToolTip (tip);


  QString percentString;
  percentString.sprintf (tr ("Highlight %0.2f percent of depth").toAscii (), options.highlight_percent);
  highlightPercent->setText (percentString);
  highlightCount->setText (tr ("Highlight valid bin count exceeding %1").arg (options.h_count));


  map->setCoastColor (options.coast_color);
  map->setBackgroundColor (options.background_color);
  map->setOverlapPercent (options.overlap_percent);


  slotRedrawCoverage ();
  if (!misc.drawing) redrawMap (NVTrue, NVFalse);


  //  Set a couple of things that pfmEdit(3D) will need to know.

  misc.abe_share->position_form = options.position_form;
  misc.abe_share->smoothing_factor = options.smoothing_factor;


  //  Check to see if we changed z_factor or z_offset.

  if (misc.abe_share->z_factor != options.z_factor || misc.abe_share->z_offset != options.z_offset) setStoplight ();


  misc.abe_share->z_factor = options.z_factor;
  misc.abe_share->z_offset = options.z_offset;


  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++) slotPrefHotKeyChanged (i);


  //  If the feature search string changed we need to tell pfm3D (if it's running).

  if (feature_search_changed)
    {
      strcpy (misc.abe_share->feature_search_string, options.feature_search_string.toAscii ());
      misc.abe_share->feature_search_type = options.feature_search_type;
      misc.abe_share->feature_search_invert = options.feature_search_invert;

      if (threeDProc)
        {
          if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
        }
    }
}



//!  Changed some of the hot keys in the preferences dialog.

void 
pfmView::slotPrefHotKeyChanged (NV_INT32 i)
{
  QString tip = misc.buttonText[i] + "  [" + options.buttonAccel[i] + "]";
  misc.button[i]->setToolTip (tip);
  misc.button[i]->setShortcut (QKeySequence (options.buttonAccel[i]));
}



//! Slot called when the user pressed one of the "highlight" buttons.

void
pfmView::slotHighlightMenu (QAction *action)
{
  setMainButtons (NVFalse);


  if (action == highlightNone) options.highlight = H_NONE;
  if (action == highlightAll) options.highlight = H_ALL;
  if (action == highlightChecked) options.highlight = H_CHECKED;
  if (action == highlightUser01) options.highlight = H_01;
  if (action == highlightUser02) options.highlight = H_02;
  if (action == highlightUser03) options.highlight = H_03;
  if (action == highlightUser04) options.highlight = H_04;
  if (action == highlightUser05) options.highlight = H_05;
  if (action == highlightInt) options.highlight = H_INT;
  if (action == highlightMult) options.highlight = H_MULT;
  if (action == highlightCount) options.highlight = H_COUNT;
  if (action == highlightIHOS) options.highlight = H_IHO_S;
  if (action == highlightIHO1) options.highlight = H_IHO_1;
  if (action == highlightIHO2) options.highlight = H_IHO_2;
  if (action == highlightPercent) options.highlight = H_PERCENT;

  bHighlight->setIcon (highlightIcon[options.highlight]);


  redrawMap (NVTrue, NVFalse);
}



//!  Display suspect toggle function.

void
pfmView::slotDisplaySuspect ()
{
  if (bDisplaySuspect->isChecked ())
    {
      options.display_suspect = NVTrue;
    }
  else
    {
      options.display_suspect = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display features toggle function.

void
pfmView::slotFeatureMenu (QAction *action)
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
      bDisplayFeaturePoly->setEnabled (TRUE);
      bAddFeature->setEnabled (TRUE);
      bDeleteFeature->setEnabled (TRUE);
      bEditFeature->setEnabled (TRUE);
      bVerifyFeatures->setEnabled (TRUE);
    }
  else
    {
      bDisplayChildren->setEnabled (FALSE);
      bDisplayFeatureInfo->setEnabled (FALSE);
      bDisplayFeaturePoly->setEnabled (FALSE);
      bAddFeature->setEnabled (FALSE);
      bDeleteFeature->setEnabled (FALSE);
      bEditFeature->setEnabled (FALSE);
      bVerifyFeatures->setEnabled (FALSE);
    }

  redrawMap (NVTrue, NVFalse);
  misc.cov_clear = NVTrue;
  cov->redrawMap (NVTrue);
  redrawCovBounds ();
}



//!  Display feature children toggle function.

void
pfmView::slotDisplayChildren ()
{
  if (bDisplayChildren->isChecked ())
    {
      options.display_children = NVTrue;
    }
  else
    {
      options.display_children = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display feature text information toggle function.

void
pfmView::slotDisplayFeatureInfo ()
{
  if (bDisplayFeatureInfo->isChecked ())
    {
      options.display_feature_info = NVTrue;
    }
  else
    {
      options.display_feature_info = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display feature polygon toggle function.

void
pfmView::slotDisplayFeaturePoly ()
{
  if (bDisplayFeaturePoly->isChecked ())
    {
      options.display_feature_poly = NVTrue;
    }
  else
    {
      options.display_feature_poly = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display selected soundings toggle function.

void
pfmView::slotDisplaySelected ()
{
  if (bDisplaySelected->isChecked ())
    {
      options.display_selected = NVTrue;
    }
  else
    {
      options.display_selected = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display reference points toggle function.

void
pfmView::slotDisplayReference ()
{
  if (bDisplayReference->isChecked ())
    {
      options.display_reference = NVTrue;
    }
  else
    {
      options.display_reference = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display minimum, maximum, and maximum standard deviation toggle function.

void
pfmView::slotDisplayMinMax ()
{
  if (bDisplayMinMax->isChecked ())
    {
      options.display_minmax = NVTrue;
    }
  else
    {
      options.display_minmax = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Error callback from the unload process.

void 
pfmView::slotUnloadError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("Unable to start the unload process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("The unload process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("The unload process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("There was a write error to the unload process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("There was a read error from the unload process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("The unload process died with an unknown error!"));
      break;
    }


  //  Kick the unload count up by one and start the next PFM layer unload (if there is a next layer).

  if (misc.abe_share->pfm_count > 1)
    {
      unload_count++;
      slotUnloadMenu (NULL);
    }
  else
    {
      unload_count = 0;
    }


  setMainButtons (NVTrue);

  qApp->restoreOverrideCursor ();
  qApp->processEvents ();
}



//!  This is the return from the unload QProcess (when finished)

void 
pfmView::slotUnloadDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  unloadProc->closeReadChannel (QProcess::StandardError);


  misc.statusProg->setRange (0, 100);
  misc.statusProg->setValue (0);
  misc.statusProgLabel->setText (tr ("Unload complete"));
  misc.statusProgLabel->setVisible (TRUE);
  misc.statusProg->setTextVisible (FALSE);
  qApp->processEvents();


  if (unloadProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("pfmView Unload"), tr ("An error occurred during unload, some or all of your edits were not saved!") +
                             tr ("\nPlease run pfm_unload separatedly to see what the error condition was."));
    }
  else
    {
      //  Display the unload information gathered in slotUnloadReadyReadStandardError.

      QMessageBox *msgBox = new QMessageBox (this);
      msgBox->setText (unloadMsg);
      msgBox->setWindowTitle (tr ("pfmView unload"));
      msgBox->setWindowModality (Qt::WindowModal);
      msgBox->exec ();

      unloadMsg = "";
    }


  //  Kick the unload count up by one and start the next PFM layer unload (if there is a next layer).

  if (misc.abe_share->pfm_count > 1)
    {
      unload_count++;
      slotUnloadMenu (NULL);
    }
  else
    {
      unload_count = 0;
    }


  setMainButtons (NVTrue);

  setFunctionCursor (misc.function);

  qApp->restoreOverrideCursor ();
  qApp->processEvents ();
}



/*!
  This is the stderr read return from the unload QProcess.  We're dumping the percentage to
  stderr in the unload program so we can track it here.
*/

void 
pfmView::slotUnloadReadyReadStandardError ()
{
  NV_INT32 percent = 0;
  static QString resp_string = "";


  QByteArray response = unloadProc->readAll ();


  //  Parse the return response for carriage returns and line feeds

  for (NV_INT32 i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n' || response.at (i) == '\r')
        {
          NV_CHAR resp[512];
          strcpy (resp, resp_string.toAscii ());

          if (resp_string.contains ("%"))
            {
              sscanf (resp, "%d%%", &percent);

              if (percent == 100)
                {
                  percent = 0;

                  if (!unload_stat_count)
                    {
                      misc.statusProgLabel->setText (tr (" Updating input file records "));
                      unload_stat_count++;
                    }
                  else if (unload_stat_count == 1)
                    {
                      misc.statusProgLabel->setText (tr (" Resetting modified flags in PFM bin file "));
                      unload_stat_count++;
                    }
                }

              misc.statusProg->setValue (percent);
              qApp->processEvents();
            }
          else
            {
              if (!resp_string.contains ("Reading") && !resp_string.contains ("Updating") && !resp_string.contains ("Resetting"))
                unloadMsg += resp_string + "\n";
            }

          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



//!  Kick off the unload QProcess

void
pfmView::slotUnloadMenu (QAction *action)
{
  QString name = QFileInfo (misc.abe_share->open_args[unload_count].list_path).baseName ();


  //  If the unload_count isn't zero we're on the second or lower layer PFM and we've already confirmed and gotten unload_type.

  if (!unload_count)
    {
      if (action == unloadDisplayed)
        {
          //  Confirm the unload for the displayed area

          if (QMessageBox::information (this, tr ("pfmView Unload data confirmation"), 
                                        tr ("Do you really want to unload all edits for the displayed area?"),
                                        QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;

          unload_type = 1;
        }
      else
        {
          //  Confirm the unload for the PFM area

          if (QMessageBox::information (this, tr ("pfmView Unload data confirmation"), 
                                        tr ("Do you really want to unload all edits for the entire PFM?"),
                                        QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;

          unload_type = 0;
        }
    }


  //  Do the unload for the next PFM.

  if (unload_count < misc.abe_share->pfm_count)
    {
      setMainButtons (NVFalse);

      qApp->setOverrideCursor (Qt::WaitCursor);
      qApp->processEvents ();


      if (unloadProc != NULL) delete unloadProc;

      unloadProc = new QProcess (this);
      unloadProc->setReadChannel (QProcess::StandardError);

      QStringList arguments;
      QString arg;


      unloadMsg = tr ("Unload status for %1").arg (QFileInfo (misc.abe_share->open_args[unload_count].list_path).baseName ()) + "\n\n";


      arguments += "-Q";

      if (unload_type)
        {
          arg.sprintf ("-a %.9f,%.9f,%.9f,%.9f", misc.total_displayed_area.max_y, misc.total_displayed_area.min_y, misc.total_displayed_area.max_x,
                       misc.total_displayed_area.min_x);
          arguments += arg;
        }


      //  Make sure that the file name is last on the command line.  For some reason getopt on Windows doesn't like it otherwise.

      NV_CHAR native_path[1024];
      QString nativePath;


      //  Convert to native separators so that we get nice file names on Windows.

      nativePath = QDir::toNativeSeparators (QString (misc.abe_share->open_args[unload_count].list_path));
      strcpy (native_path, nativePath.toAscii ());

      arg.sprintf ("""%s""", native_path);
      arguments += arg;


      //  Zero the counter for the output status from pfm_unload.  Set the status progress bar label and range.

      unload_stat_count = 0;

      misc.statusProg->setRange (0, 100);
      misc.statusProgLabel->setText (tr (" Reading PFM bin and index files"));
      misc.statusProgLabel->setVisible (TRUE);
      misc.statusProg->setTextVisible (TRUE);
      qApp->processEvents();


      connect (unloadProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotUnloadDone (int, QProcess::ExitStatus)));
      connect (unloadProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotUnloadReadyReadStandardError ()));
      connect (unloadProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotUnloadError (QProcess::ProcessError)));


      unloadProc->start (QString (options.unload_prog), arguments);
    }
  else
    {
      unload_count = 0;
    }
}



//!  This is the stderr read return from the CUBE QProcess.

void 
pfmView::slotCubeReadyReadStandardError ()
{
  static QString resp_string = "";


  QByteArray response = cubeProc->readAllStandardError ();


  //  Parse the return response for carriage returns and line feeds

  for (NV_INT32 i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n' || response.at (i) == '\r')
        {
          NV_CHAR resp[512];
          strcpy (resp, resp_string.toAscii ());
          fprintf (stderr, "%s %d %s\n", __FILE__, __LINE__, resp);

          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



//!  This is the stdout read return from the CUBE QProcess.

void 
pfmView::slotCubeReadyReadStandardOutput ()
{
  static QString resp_string = "";


  QByteArray response = cubeProc->readAllStandardOutput ();


  //  Parse the return response for carriage returns and line feeds

  for (NV_INT32 i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n' || response.at (i) == '\r')
        {
          if (resp_string.contains ("%"))
            {
              NV_CHAR resp[512];
              strcpy (resp, resp_string.toAscii ());

              NV_INT32 value;
              sscanf (resp, "%d", &value);
              misc.statusProg->setValue (value);
            }

          if (resp_string.contains ("CUBE_AREA_XY:"))
            {
              misc.statusProgLabel->setText (tr (" PFM CUBE processing (pass 1 of 2) "));
              misc.statusProg->reset ();
            }

          if (resp_string.contains ("Extract Best Hypothesis"))
            {
              misc.statusProgLabel->setText (tr (" Extracting best CUBE hypotheses (pass 2 of 2) "));
              misc.statusProg->reset ();
            }

          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



//!  Error return from the CUBE process.

void 
pfmView::slotCubeError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("Unable to start the CUBE process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("The CUBE process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("The CUBE process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("There was a write error from the CUBE process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("There was a read error from the CUBE process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("The CUBE process died with an unknown error!"));
      break;
    }
}



//!  This is the return from the CUBE QProcess (when finished normally)

void 
pfmView::slotCubeDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  misc.statusProg->reset ();
  misc.statusProgLabel->setVisible (FALSE);
  misc.statusProg->setTextVisible (FALSE);
  qApp->processEvents();

  if (cubeProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("An error occurred during CUBE processing!") +
                             tr ("\nPlease run navo_pfm_cube separatedly to see what the error condition was."));
    }
  if (misc.color_by_attribute) redrawMap (NVTrue, NVFalse);
}



//!  Kick off the CUBE QProcess

void
pfmView::slotCube ()
{
  //  Confirm the cube

  if (QMessageBox::information (this, tr ("pfmView CUBE data confirmation"), 
                                tr ("Do you really want to CUBE process the displayed area?"),
                                QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;


  //  Only start it if we have the (SAIC proprietary) CUBE program available and we have CUBE attributes in the PFM

  if (misc.cube_available && misc.cube_attr_available[0])
    {
      cubeProc = new QProcess (this);

      QStringList arguments;
      QString arg;


      arg.sprintf ("""%s""", misc.abe_share->open_args[0].list_path);
      arguments += arg;

      arguments += "0";

      arg.sprintf ("%.9f", misc.total_displayed_area.min_x);
      arguments += arg;

      arg.sprintf ("%.9f", misc.total_displayed_area.min_y);
      arguments += arg;

      arg.sprintf ("%.9f", misc.total_displayed_area.max_x);
      arguments += arg;

      arg.sprintf ("%.9f", misc.total_displayed_area.max_y);
      arguments += arg;

      arguments += arg.setNum (options.iho);
      arguments += arg.setNum (options.capture);
      arguments += arg.setNum (options.queue);
      arguments += arg.setNum (options.horiz);
      arguments += arg.setNum (options.distance);
      arguments += arg.setNum (options.min_context);
      arguments += arg.setNum (options.max_context);
      arguments += arg.setNum (options.std2conf + 1);
      arguments += arg.setNum (options.disambiguation);



      misc.statusProg->setRange (0, 100);
      misc.statusProgLabel->setText (tr (" CUBE processing "));
      misc.statusProgLabel->setVisible (TRUE);
      misc.statusProg->setTextVisible (TRUE);
      qApp->processEvents();


      connect (cubeProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotCubeDone (int, QProcess::ExitStatus)));
      connect (cubeProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotCubeReadyReadStandardError ()));
      connect (cubeProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotCubeReadyReadStandardOutput ()));
      connect (cubeProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotCubeError (QProcess::ProcessError)));

      cubeProc->start (QString (options.cube_prog), arguments);
    }
}



//!  This function determines if we are viewing a valid or invalid surface.

void 
pfmView::setSurfaceValidity (NV_INT32 layer)
{
  if (cov_area_defined)
    {
      //  Determine if the surface layer that we are displaying is an edited or unedited surface.

      if (layer == AVERAGE_FILTERED_DEPTH || layer == MIN_FILTERED_DEPTH || layer == MAX_FILTERED_DEPTH || layer == NUM_LAYERS) 
        {
          misc.surface_val = NVTrue;
        }
      else
        {
          misc.surface_val = NVFalse;
        }


      if (misc.surface_val != misc.prev_surface_val)
        {
          misc.prev_surface_val = misc.surface_val;
          if (cov) slotRedrawCoverage ();
        }
    }
}



//!  Slot called when we have finished setting a lock value for the min or max color scale.

void 
pfmView::slotLockValueDone (NV_BOOL accepted)
{
  if (accepted)
    {
      //  We have to reset the colors here because we might have changed the min or max color.

      NV_INT32 k = misc.color_by_attribute;

      NV_FLOAT32 hue_inc = (NV_FLOAT32) (options.max_hsv_color[k] - options.min_hsv_color[k]) / (NV_FLOAT32) (NUMHUES + 1);
      for (NV_INT32 m = 0 ; m < 2 ; m++)
        {
          NV_INT32 sat = 255;
          if (m) sat = 25;

          for (NV_INT32 i = 0 ; i < NUMHUES ; i++)
            {
              for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
                {
                  options.color_array[m][i][j].setHsv ((NV_INT32) (((NUMHUES + 1) - (i - options.min_hsv_color[k])) * hue_inc), sat, j, 255);
                }
            }
        }


      //  If we changed the depth colors, locks, or values (index 0) then we need to set shared memory to inform the editors when they start up.

      if (!k)
        {
          misc.abe_share->min_hsv_color = options.min_hsv_color[0];
          misc.abe_share->max_hsv_color = options.max_hsv_color[0];
          misc.abe_share->min_hsv_value = options.min_hsv_value[0];
          misc.abe_share->max_hsv_value = options.max_hsv_value[0];
          misc.abe_share->min_hsv_locked = options.min_hsv_locked[0];
          misc.abe_share->max_hsv_locked = options.max_hsv_locked[0];
        }


      redrawMap (NVTrue, NVTrue);
      slotRedrawCoverage ();
    }
}



//!  Slot called if we pressed the minimum color scale "button".

void 
pfmView::slotMinScalePressed (QMouseEvent *e __attribute__ ((unused)))
{
  if (misc.drawing) return;

  lockValue *lck = new lockValue (map, &options, &misc, NVTrue, misc.color_by_attribute);
  connect (lck, SIGNAL (lockValueDone (NV_BOOL)), this, SLOT (slotLockValueDone (NV_BOOL)));


  //  Put the dialog in the middle of the screen.

  lck->move (x () + width () / 2 - lck->width () / 2, y () + height () / 2 - lck->height () / 2);
}



//!  Slot called if we pressed the maximum color scale "button".

void 
pfmView::slotMaxScalePressed (QMouseEvent *e __attribute__ ((unused)))
{
  if (misc.drawing) return;

  lockValue *lck = new lockValue (map, &options, &misc, NVFalse, misc.color_by_attribute);
  connect (lck, SIGNAL (lockValueDone (NV_BOOL)), this, SLOT (slotLockValueDone (NV_BOOL)));


  //  Put the dialog in the middle of the screen.

  lck->move (x () + width () / 2 - lck->width () / 2, y () + height () / 2 - lck->height () / 2);
}



//!  Slot called to toggle display of a PFM layer.

void 
pfmView::slotLayerClicked (int id)
{
  if (layerCheck[id]->checkState () == Qt::Checked)
    {
      misc.abe_share->display_pfm[id] = NVTrue;
    }
  else
    {
      misc.abe_share->display_pfm[id] = NVFalse;
    }

  redrawMap (NVTrue, NVTrue);
  slotRedrawCoverage ();
}



//!  Toggle display of stoplight colors.

void
pfmView::slotStoplight ()
{
  if (bStoplight->isChecked ())
    {
      options.stoplight = NVTrue;


      //  See if we need to reset the stoplight crossover colors.

      if (options.stoplight_min_mid == options.stoplight_max_mid) setStoplight ();
    }
  else
    {
      options.stoplight = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Toggle display of contours.

void
pfmView::slotContour ()
{
  if (bContour->isChecked ())
    {
      options.contour = NVTrue;
    }
  else
    {
      options.contour = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Toggle display of coastlines.

void
pfmView::slotCoast ()
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

  redrawMap (NVTrue, NVFalse);
}



//!  Toggle display of SRTM landmask.

void
pfmView::slotMask ()
{
  if (bMask->isChecked ())
    {
      options.landmask = NVTrue;
    }
  else
    {
      options.landmask = NVFalse;
    }

  map->setLandmask (options.landmask);
  redrawMap (NVTrue, NVFalse);
}



//!  Toggle display of GeoTIFF.

void
pfmView::slotGeotiff ()
{
  if (!misc.display_GeoTIFF)
    {
      misc.display_GeoTIFF = 1;
      bGeotiff->setIcon (QIcon (":/icons/geo.xpm"));
    }
  else if (misc.display_GeoTIFF > 0)
    {
      misc.display_GeoTIFF = -1;
      bGeotiff->setIcon (QIcon (":/icons/geo_under.xpm"));
    }
  else
    {
      misc.display_GeoTIFF = 0;
      bGeotiff->setIcon (QIcon (":/icons/geo_off.xpm"));
    }
  qApp->processEvents ();

  misc.clear = NVTrue;
  redrawMap (NVTrue, NVFalse);
}



//!  Slot called when we want to modify the sun angle from the GUI.

void
pfmView::slotSunangle ()
{
  misc.function = SUNANGLE;
  setFunctionCursor (misc.function);
}



//!  Stop drawing and kill pfmEdit(3D) if it is running.

void
pfmView::slotStop ()
{
  misc.clear = NVTrue;
  misc.drawing_canceled = NVTrue;

  if (pfm_edit_active) 
    {
      //  Set the PFMEDIT_KILL flag.

      misc.abe_share->key = PFMEDIT_KILL;


      //  Wait 2 seconds so that pfmEdit(3D) will see the flag.

#ifdef NVWIN3X
      Sleep (2000);
#else
      sleep (2);
#endif


      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED; 


      pfmEdit_stopped = NVTrue;


      //  Make sure it's really dead.

      editProc->kill ();

      setMainButtons (NVTrue);

      pfm_edit_active = NVFalse;


      //  Make sure we reset the important things in case we did a partial redraw.

      setFunctionCursor (misc.function);
      if (misc.function == RECT_EDIT_AREA || misc.function == RECT_EDIT_AREA_3D)
        {
          map->discardRubberbandRectangle (&rb_rectangle);
        }
      else
        {
          map->discardRubberbandPolygon (&rb_polygon);
        }

      setMainButtons (NVTrue);
      misc.drawing = NVFalse;
    }
}



//!  We changed a feature in the feature file (BFD).  This is usally done in pfmEdit(3D).

void 
pfmView::slotEditFeatureDataChanged ()
{
  //  Try to open the feature file and read the features into memory.

  readFeature (this, &misc);


  //  If we added a feature add PFM_SELECTED_FEATURE to the validity for the selected point

  if (misc.add_feature_index >= 0)
    {
      DEPTH_RECORD *dep = NULL;
      NV_INT32 numrecs = 0;

      read_depth_array_index (misc.pfm_handle[0], misc.add_feature_coord, &dep, &numrecs);

      dep[misc.add_feature_index].validity |= PFM_SELECTED_FEATURE;

      update_depth_record_index (misc.pfm_handle[0], &dep[misc.add_feature_index]);

      free (dep);

      misc.add_feature_index = -1;
    }


  redrawMap (NVTrue, NVTrue);


  misc.cov_clear = NVTrue;
  cov->redrawMap (NVTrue);


  //  If the mosaic viewer or 3D viewer was running, tell it to redraw.

  misc.abe_share->key = FEATURE_FILE_MODIFIED;
}



//!  Slot called when the user wants to define a feature polygon from the feature edit dialog.

void 
pfmView::slotEditFeatureDefinePolygon ()
{
  map->setToolTip (tr ("Left click to define polygon vertices, double click to define last vertex, middle click to abort"));
  misc.save_function = misc.function;
  misc.function = DEFINE_FEATURE_POLY_AREA;
  setFunctionCursor (misc.function);
}



//!  Bring up the feature edit dialog for feature record number "feature_number".

void 
pfmView::editFeatureNum (NV_INT32 feature_number)
{
  misc.def_feature_poly = NVTrue;


  if (options.chart_scale)
    {
      misc.feature_poly_radius = (0.005 * options.chart_scale) / 2.0;
    }
  else
    {
      //  Default to 1:5000

      misc.feature_poly_radius = (0.005 * 5000.0) / 2.0;
    }


  redrawMap (NVTrue, NVTrue);
  qApp->processEvents ();


  editFeature_dialog = new editFeature (this, &options, &misc, feature_number);
  connect (editFeature_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotEditFeatureDataChanged ()));
  connect (editFeature_dialog, SIGNAL (definePolygonSignal ()), this, SLOT (slotEditFeatureDefinePolygon ()));
  connect (editFeature_dialog, SIGNAL (chartScaleChangedSignal ()), this, SLOT (slotDefinePolygonChartScaleChanged ()));

  editFeature_dialog->setModal (TRUE);

  editFeature_dialog->show ();
}



//!  We pushed one of the buttons to display one of the surfaces (min, max, avg, OTF min, ...).

void
pfmView::slotSurface (int id)
{
  if (id != NUM_LAYERS)
    {
      misc.clear = NVTrue;


      //  I added OTF_OFFSET to the MIN_FILTERED_DEPTH, MAX_FILTERED_DEPTH, AVERAGE_FILTERED_DEPTH, MIN_DEPTH, MAX_DEPTH, and
      //  AVERAGE_DEPTH values to indicate that I want to do an on-the-fly binned surface.

      if (id > OTF_OFFSET)
        {
          misc.otf_surface = NVTrue;
          id -= OTF_OFFSET;
        }
      else
        {
          misc.otf_surface = NVFalse;

          if (misc.otf_grid != NULL)
            {
              misc.otfShare->detach ();
              misc.otf_grid = NULL;
              misc.abe_share->otf_width = misc.abe_share->otf_height = 0;
            }
        }


      misc.abe_share->layer_type = options.layer_type = id;
    }

  redrawMap (NVTrue, NVTrue);
}



//!  Slot called when the user wants to redefine the OTF bin size (they pushed the OTF bin size button).

void 
pfmView::slotSetOtfBin ()
{
  NV_FLOAT64 res;
  bool ok;

  res = QInputDialog::getDouble (this, tr ("pfmView"), tr ("Enter the on-the-fly gridding bin size in meters:"), options.otf_bin_size_meters, 0.10, 1852.0, 1, &ok);
  if (ok)
    {
      options.otf_bin_size_meters = (NV_FLOAT32) res;
      QString tip = tr ("Set the on-the-fly gridding bin size in meters (%1)").arg (options.otf_bin_size_meters, 3, 'f', 1);
      bSetOtfBin->setToolTip (tip);
    }

  if (misc.otf_surface) redrawMap (NVTrue, NVTrue);
}



//!  Mark all displayed features as valid (button slot).

void
pfmView::slotVerifyFeatures ()
{
  NV_INT32 ret = QMessageBox::Yes;
  ret = QMessageBox::information (this, tr ("pfmView"), tr ("Are you sure you want to verify all displayed features?"),
                                  QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (ret == QMessageBox::No) return;


  setMainButtons (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  misc.statusProg->setRange (0, misc.bfd_header.number_of_records);
  misc.statusProgLabel->setText (tr (" Verifying features "));
  misc.statusProgLabel->setVisible (TRUE);
  misc.statusProg->setTextVisible (TRUE);
  qApp->processEvents();


  BFDATA_RECORD bfd_record;

  for (NV_U_INT32 i = 0 ; i < misc.bfd_header.number_of_records ; i++)
    {
      misc.statusProg->setValue (i);
      if (misc.feature[i].confidence_level)
	{
	  if (misc.feature[i].latitude >= misc.total_displayed_area.min_y && misc.feature[i].latitude <= misc.total_displayed_area.max_y &&
              misc.feature[i].longitude >= misc.total_displayed_area.min_x && misc.feature[i].longitude <= misc.total_displayed_area.max_x)
	    {
	      binaryFeatureData_read_record (misc.bfd_handle, i, &bfd_record);

	      bfd_record.confidence_level = 5;

	      binaryFeatureData_write_record (misc.bfd_handle, i, &bfd_record, NULL, NULL);
	    }
	}
    }


  if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);
  misc.bfd_open = NVFalse;

  if ((misc.bfd_handle = binaryFeatureData_open_file (misc.abe_share->open_args[0].target_path, &misc.bfd_header, BFDATA_UPDATE)) >= 0)
    {
      if (binaryFeatureData_read_all_short_features (misc.bfd_handle, &misc.feature) < 0)
	{
	  QString msg = QString (binaryFeatureData_strerror ());
	  QMessageBox::warning (this, tr ("pfmView"), tr ("Unable to read feature records\nReason: ") + msg);
	  binaryFeatureData_close_file (misc.bfd_handle);
	}
      else
	{
	  misc.bfd_open = NVTrue;
	}
    }


  misc.statusProg->reset ();
  misc.statusProgLabel->setVisible (FALSE);
  misc.statusProg->setTextVisible (FALSE);

  qApp->restoreOverrideCursor ();
  qApp->processEvents ();

  redrawMap (NVTrue, NVTrue);

  if (options.display_feature)
    {
      misc.cov_clear = NVTrue;
      cov->redrawMap (NVTrue);
    }
}



/*!
  Clear all contours drawn for filtering purposes.  This is one of the PFM "grid" functions so it should be
  deleted eventually.
*/

void
pfmView::slotClearFilterContours ()
{
  free (misc.filt_contour);
  misc.filt_contour = NULL;
  misc.filt_contour_count = 0;

  redrawMap (NVTrue, NVTrue);
}



//!  Color by attribute menu entry was pressed.

void 
pfmView::slotColor (int id)
{
  misc.color_by_attribute = id;


  //  We have to reset the colors here in case the "color by" attribute has a different color range than the last 
  //  option.

  NV_FLOAT32 hue_inc = (NV_FLOAT32) (options.max_hsv_color[id] - options.min_hsv_color[id]) / (NV_FLOAT32) (NUMHUES + 1);
  for (NV_INT32 m = 0 ; m < 2 ; m++)
    {
      NV_INT32 sat = 255;
      if (m) sat = 25;

      for (NV_INT32 i = 0 ; i < NUMHUES ; i++)
        {
          for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
            {
              options.color_array[m][i][j].setHsv ((NV_INT32) (((NUMHUES + 1) - (i - options.min_hsv_color[id])) * hue_inc), sat, j, 255);
            }
        }
    }

  setStoplight ();

  redrawMap (NVTrue, NVFalse);
}



//!  Slot called when zoom out button is pressed.

void
pfmView::slotZoomOut ()
{
  mapdef = map->getMapdef ();
  if (mapdef.zoom_level)
    {
      misc.clear = NVTrue;
      misc.GeoTIFF_init = NVTrue;

      zoomOut ();

      redrawCovBounds ();

      mapdef = map->getMapdef ();
      if (mapdef.zoom_level == 0) bZoomOut->setEnabled (FALSE);

      misc.cov_function = COV_START_DRAW_RECTANGLE;
    }
}



//!  Slot called when zoom in button is pressed.  Puts us into ZOOM_IN_AREA mode.

void
pfmView::slotZoomIn ()
{
  map->setToolTip (tr ("Left click to start rectangle, middle click to abort, right click for menu"));
  misc.function = ZOOM_IN_AREA;
  setFunctionCursor (misc.function);
}



//!  Clear all filter masks.

void
pfmView::slotClearFilterMasks ()
{
  clearFilterMasks ();
  redrawMap (NVTrue, NVTrue);
}



//!  This determines the active function.  The id is set by the editMode button group.

void
pfmView::slotEditMode (int id)
{
  QString msc;


  switch (id)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
    case GRAB_CONTOUR:
      misc.function = misc.last_edit_function = id;
      misc.save_function = misc.function;
      break;

    case RECT_FILTER_AREA:
    case RECT_FILTER_MASK:
    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case DELETE_FEATURE:
    case EDIT_FEATURE:
    case REMISP_AREA:
    case REMISP_FILTER:
      misc.function = id;
      misc.save_function = misc.function;
      break;

    case DRAW_CONTOUR:
      misc.function = id;
      misc.save_function = misc.function;
      break;

    case DRAW_CONTOUR_FILTER:
      misc.function = id;
      misc.save_function = misc.function;


      //  If contours already existed, remove them.

      if (misc.filt_contour_count) slotClearFilterContours ();
      break;

    case ADD_FEATURE:
      misc.save_function = misc.function;
      misc.function = id;
      break;
    }
  setFunctionCursor (misc.function);
}



/*!
  Setting the displayed area or the entire file to checked, unchecked, verified, or unverified.  The displayed area
  set comes from the toolbar button.  The entire file set comes from the "Edit" pulldown menu.
*/

void 
pfmView::slotCheckMenu (QAction *action)
{
  void setAreaChecked (MISC *misc, NV_INT32 type, NV_BOOL area);


  setMainButtons (NVFalse);


  NV_INT32 type = 0;


  //  Determine what action to take and whether we need to perform it for an area or the entire file.

  NV_BOOL area = NVTrue;
  if (action == setChecked || action == fileChecked) type = 0;
  if (action == setUnchecked || action == fileUnchecked) type = 1;
  if (action == setVerified || action == fileVerified) type = 2;
  if (action == setUnverified || action == fileUnverified) type = 3;

  if (action == fileChecked || action == fileUnchecked || action == fileVerified || action == fileUnverified) area = NVFalse;


  pfmViewMod = NVTrue;


  setMainButtons (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  setAreaChecked (&misc, type, area);


  //  Why are we closing and opening the file?  This should force a cache flush on SMB or NFS file systems.
  //  We have seen a problem when two people are accessing the same file - when one of them sets an area
  //  to checked/unchecked/verified/unverified the other won't see it when they redraw their coverage map.
  //  This is caused by the remote client using it's SMB or NFS cache instead of actually reading the file.
  //  Close and open is supposed to inform remote clients that they need to flush their cache and reread.
  //  I wouldn't bet on it though ;-)

  close_pfm_file (misc.pfm_handle[0]);
  misc.abe_share->open_args[0].checkpoint = 0;
  misc.pfm_handle[0] = open_existing_pfm_file (&misc.abe_share->open_args[0]);


  qApp->restoreOverrideCursor ();
  qApp->processEvents ();


  redrawMap (NVTrue, NVFalse);

  cov->redrawMap (NVFalse);
}



//!  Move the selected layer to the top of the PFM layers.
  
void 
pfmView::slotLayerGrpTriggered (QAction *action)
{
  NV_INT32 hit = -1;


  //  Find out which layer we selected

  for (NV_INT32 pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
    {
      if (action == layer[pfm])
        {
          hit = pfm;
          break;
        }
    }


  //  If we selected a layer other than 0, move the layer to the top.

  if (hit > 0)
    {
      move_layer_to_top (&misc, hit);

      slotManageLayersDataChanged ();
    }
}



//!  Kick off the layer management dialog.

void 
pfmView::slotLayers ()
{
  manageLayers_dialog = new manageLayers (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  manageLayers_dialog->move (x () + width () / 2 - manageLayers_dialog->width () / 2, y () + height () / 2 - manageLayers_dialog->height () / 2);

  connect (manageLayers_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotManageLayersDataChanged ()));
}



//!  We've changed something about the layers (multiple PFMs) in the manage layer dialog so we have to do something.

void 
pfmView::slotManageLayersDataChanged ()
{
  if (!misc.drawing)
    {
      //  Set up the layers stuff

      NV_INT32 num_displayed = 0;
      notebook->setTabEnabled (1, FALSE);

      for (NV_INT32 pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++)
        {
          if (pfm < misc.abe_share->pfm_count)
            {
              QString layerString;
              layerString.sprintf ("%d - ", pfm + 1);
              layerString += QFileInfo (misc.abe_share->open_args[pfm].list_path).fileName ();
              layer[pfm]->setText (layerString);
              layer[pfm]->setVisible (TRUE);


              //  This is for the PFM layers notebook page.

              QString name = QFileInfo (QString (misc.abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm");

              layerName[pfm]->setText (name);

              if (pfm) notebook->setTabEnabled (1, TRUE);

              if (misc.abe_share->display_pfm[pfm])
                {
                  num_displayed++;
                  layerCheck[pfm]->setChecked (TRUE);
                }
              else
                {
                  layerCheck[pfm]->setChecked (FALSE);
                }
              layerName[pfm]->show ();
              layerCheck[pfm]->show ();
            }
          else
            {
              layer[pfm]->setVisible (FALSE);

              layerName[pfm]->hide ();
              layerCheck[pfm]->hide ();
            }
        }


      for (NV_INT32 i = 0 ; i < NUM_ATTR ; i++)
        {
          if (i < misc.abe_share->open_args[0].head.num_bin_attr)
            {
              attrLabel[i]->setToolTip (QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[i])));
              attrName[i]->setText (tr ("Attr %1:").arg (i + 1));


              QString tmp = tr ("Color by ") + 
                QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[i]));
              bColor[i + PRE_ATTR]->setToolTip (tmp);
              bColorAction[i + PRE_ATTR]->setVisible (TRUE);
            }
          else
            {
              attrName[i]->setText ("");
              attrLabel[i]->setText ("");
              bColorAction[i + PRE_ATTR]->setVisible (FALSE);
            }
        }


      if (misc.abe_share->open_args[0].head.num_bin_attr)
        {
          //  If we don't have the cube executable or the attributes we don't want to offer the recube button.

          if (misc.cube_available && misc.cube_attr_available[0])
            {
              bCube->setEnabled (TRUE);
            }
          else
            {
              bCube->setEnabled (FALSE);
            }
        }
    }


  //  Try to open the feature file and read the features into memory.

  readFeature (this, &misc);


  if (!misc.abe_share->pfm_count)
    {
      discardMovableObjects ();

      cov_area_defined = NVFalse;

      misc.clear = NVTrue;
      misc.cov_clear = NVTrue;

      clearFilterMasks ();

      map->setZoomLevel (0);

      slotRedrawCoverage ();

      logo_first = NVTrue;
      slotResize (NULL);
    }
  else
    {
      misc.cov_clear = NVTrue;
      misc.clear = NVTrue;


      //  Compute the MBR of all of the PFM areas.

      compute_total_mbr (&misc);


      map->disableSignals ();

      clearFilterMasks ();

      map->resetBounds (misc.total_mbr);

      map->enableSignals ();


      covdef.initial_bounds.min_x = misc.total_mbr.min_x;
      covdef.initial_bounds.min_y = misc.total_mbr.min_y;
      covdef.initial_bounds.max_x = misc.total_mbr.max_x;
      covdef.initial_bounds.max_y = misc.total_mbr.max_y;

      cov->resetBounds (covdef.initial_bounds);

      slotRedrawCoverage ();


      if (cov_area_defined) zoomIn (misc.total_displayed_area, NVTrue);
    }


  //  Let other programs know that we have changed PFM layers

  misc.abe_share->key = PFM_LAYERS_CHANGED;


  //  Wait 2 seconds so that all associated programs will see the flag.

#ifdef NVWIN3X
  Sleep (2000);
#else
  sleep (2);
#endif


  misc.abe_share->key = 0;
  misc.abe_share->modcode = NO_ACTION_REQUIRED; 
}



//!  Error callback from the help browser process.

void 
pfmView::slotBrowserError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView help browser"), tr ("Unable to start the browser process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView help browser"), tr ("The browser process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView help browser"), tr ("The browser process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView help browser"), tr ("There was a write error to the browser process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView help browser"), tr ("There was a read error from the browser process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView help browser"), tr ("The browser process died with an unknown error!"));
      break;
    }
}



//!  Kick off the browser and point it to the help HTML file.

void
pfmView::extendedHelp ()
{
  if (getenv ("PFM_ABE") == NULL)
    {
      QMessageBox::warning (this, tr ("pfmView User Guide"), 
                            tr ("The PFM_ABE environment variable is not set.\n") +
                            tr ("This must point to the folder that contains the doc folder containing the PFM_ABE documentation."));
      return;
    }



  QString html_help_file;
  QString dir (getenv ("PFM_ABE"));
  html_help_file = dir + SEPARATOR + "doc" + SEPARATOR + "PFM" + SEPARATOR + "Area_Based_Editor.html";


  QFileInfo hf (html_help_file);

  if (!hf.exists () || !hf.isReadable ())
    {
      QMessageBox::warning (this, tr ("pfmView User Guide"), 
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
          QMessageBox::warning (this, tr ("pfmView extended help"), 
                                tr ("Unable to find the specified browser: ") + misc.help_browser);
          return;
        }

      hbi = QString (find_startup_name (hb));
    }


  if (!hbi.exists () || !hbi.isExecutable ())
    {
      QMessageBox::warning (this, tr ("pfmView User Guide"), 
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



//!  Help for the toolbars so that the user can figure out how to set them up.

void
pfmView::slotToolbarHelp ()
{
  QWhatsThis::showText (QCursor::pos ( ), toolbarText, map);
}



//!  The "About" help menu item was selected.

void
pfmView::about ()
{
  QMessageBox::about (this, VERSION, pfmViewAboutText);
}



//!  The "Acknowledgements" help menu item was selected.

void
pfmView::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}



//!  The "About Qt" help menu item was selected.

void
pfmView::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



/*!
************************************************************************************************

   - Coverage map functions.

   - These work the same way as the main map functions (after all, they use the same type of
     map widget).  The coverage map just has fewer possible actions.

   -  This function redraws the coverage bounds (the area we're viewing).

************************************************************************************************/

void 
pfmView::redrawCovBounds ()
{
  if (cov_area_defined)
    {
      mapdef = map->getMapdef ();

      if (cov->rubberbandRectangleIsActive (cov_rb_rectangle)) cov->discardRubberbandRectangle (&cov_rb_rectangle);

      cov->drawRectangle (mapdef.bounds[mapdef.zoom_level].min_x,
                          mapdef.bounds[mapdef.zoom_level].min_y, mapdef.bounds[mapdef.zoom_level].max_x,
                          mapdef.bounds[mapdef.zoom_level].max_y, Qt::blue/*options.contour_highlight_color*/,
                          LINE_WIDTH, Qt::SolidLine, NVTrue);
    }
}



//!  The left mouse button was pressed in the coverage map.

void 
pfmView::covLeftMouse (NV_INT32 mouse_x __attribute__ ((unused)), NV_INT32 mouse_y __attribute__ ((unused)), NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  //  If the popup menu is up discard this mouse press

  if (cov_popup_active)
    {
      cov_popup_active = NVFalse;
      return;
    }


  NV_INT32 *px, *py;
  NV_FLOAT64 *mx, *my;
  NV_F64_XYMBR bounds;


  switch (misc.cov_function)
    {
    case COV_START_DRAW_RECTANGLE:
      cov->discardRubberbandRectangle (&cov_rb_rectangle);

      misc.cov_function = COV_DRAW_RECTANGLE;
      cov->anchorRubberbandRectangle (&cov_rb_rectangle, lon, lat, Qt::blue, LINE_WIDTH, Qt::SolidLine);
      break;

    case COV_DRAW_RECTANGLE:

      //  If we have a rubberband rectangle active (moving) we close it and do the zoom in the main map.

      if (cov->rubberbandRectangleIsActive (cov_rb_rectangle))
        {
          cov->closeRubberbandRectangle (cov_rb_rectangle, lon, lat, &px, &py, &mx, &my);


          bounds.min_x = 999999.0;
          bounds.min_y = 999999.0;
          bounds.max_x = -999999.0;
          bounds.max_y = -999999.0;

          for (NV_INT32 i = 0 ; i < 4 ; i++)
            {
              if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

              if (mx[i] < bounds.min_x) bounds.min_x = mx[i];
              if (my[i] < bounds.min_y) bounds.min_y = my[i];
              if (mx[i] > bounds.max_x) bounds.max_x = mx[i];
              if (my[i] > bounds.max_y) bounds.max_y = my[i];
            }


          //  The first time in we want to enable all of the main buttons after an area
          //  has been defined.

          if (!cov_area_defined) setMainButtons (NVTrue);

          cov_area_defined = NVTrue;

          misc.cov_function = COV_START_DRAW_RECTANGLE;

          zoomIn (bounds, NVTrue);
        }
      break;


    case COV_START_DRAG_RECTANGLE:

      cov_drag_bounds = mapdef.bounds[mapdef.zoom_level];

      cov->setMovingRectangle (&cov_mv_rectangle, cov_drag_bounds.min_x, cov_drag_bounds.min_y, cov_drag_bounds.max_x, cov_drag_bounds.max_y,
                               Qt::blue, LINE_WIDTH, NVFalse, Qt::SolidLine);

      cov_start_drag_lat = lat;
      cov_start_drag_lon = lon;

      cov->setCursor (Qt::ClosedHandCursor);
      misc.cov_function = COV_DRAG_RECTANGLE;
      break;


    case COV_DRAG_RECTANGLE:

      cov->getMovingRectangle (cov_mv_rectangle, &px, &py, &mx, &my);

      cov_drag_bounds.min_x = 999999.0;
      cov_drag_bounds.min_y = 999999.0;
      cov_drag_bounds.max_x = -999999.0;
      cov_drag_bounds.max_y = -999999.0;

      for (NV_INT32 i = 0 ; i < 4 ; i++)
        {
          if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

          if (mx[i] < cov_drag_bounds.min_x) cov_drag_bounds.min_x = mx[i];
          if (my[i] < cov_drag_bounds.min_y) cov_drag_bounds.min_y = my[i];
          if (mx[i] > cov_drag_bounds.max_x) cov_drag_bounds.max_x = mx[i];
          if (my[i] > cov_drag_bounds.max_y) cov_drag_bounds.max_y = my[i];
        }

      cov->closeMovingRectangle (&cov_mv_rectangle);

      cov->setCursor (editRectCursor);
      misc.cov_function = COV_START_DRAW_RECTANGLE;
      zoomIn (cov_drag_bounds, NVTrue);
      break;
    }
}



//!  The middle mouse button was pressed in the coverage map.

void 
pfmView::covMidMouse (NV_INT32 mouse_x __attribute__ ((unused)), NV_INT32 mouse_y __attribute__ ((unused)),
                      NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)))
{
  misc.cov_function = COV_START_DRAW_RECTANGLE;
  cov->setCursor (editRectCursor);

  cov->discardRubberbandRectangle (&cov_rb_rectangle);
  cov->closeMovingRectangle (&cov_mv_rectangle);

  redrawCovBounds ();
}



//!  The right mouse button was pressed in the coverage map.

void 
pfmView::covRightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  cov_menu_cursor_lon = lon;
  cov_menu_cursor_lat = lat;
  cov_menu_cursor_x = mouse_x;
  cov_menu_cursor_y = mouse_y;


  cov_popup_active = NVTrue;

  covPopupMenu->popup (QCursor::pos ( ));
}



//!  The first popup menu item was selected in the coverage map.

void 
pfmView::slotCovPopupMenu0 ()
{
  cov_popup_active = NVFalse;

  covLeftMouse (cov_menu_cursor_x, cov_menu_cursor_y, cov_menu_cursor_lon, cov_menu_cursor_lat);
}



//!  The second popup menu item was selected in the coverage map.

void 
pfmView::slotCovPopupMenu1 ()
{
  cov_popup_active = NVFalse;

  covMidMouse (cov_menu_cursor_x, cov_menu_cursor_y, cov_menu_cursor_lon, cov_menu_cursor_lat);
}



//!  The third popup menu item was selected in the coverage map.

void 
pfmView::slotCovPopupMenu2 ()
{
  cov_popup_active = NVFalse;

  if (cov_area_defined)
    {
      cov->setCursor (Qt::OpenHandCursor);

      misc.cov_function = COV_START_DRAG_RECTANGLE;
    }
}



//!  The "help" popup menu item was selected in the coverage map.

void 
pfmView::slotCovPopupHelp ()
{
  QWhatsThis::showText (QCursor::pos ( ), covMenuText, cov);
  covMidMouse (cov_menu_cursor_x, cov_menu_cursor_y, cov_menu_cursor_lon, cov_menu_cursor_lat);
}



//!  This slot is called whenever a mouse button (any of them) is pressed in the coverage map.

void 
pfmView::slotCovMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  if (!misc.drawing && !pfm_edit_active)
    {
      if (e->button () == Qt::LeftButton)
        {
          //  Check for the control key modifier.  If it's set, we want to drag the rectangle.

          if (e->modifiers () == Qt::ControlModifier) misc.cov_function = COV_START_DRAG_RECTANGLE;

          covLeftMouse (e->x (), e->y (), lon, lat);
        }
      if (e->button () == Qt::MidButton) covMidMouse (e->x (), e->y (), lon, lat);
      if (e->button () == Qt::RightButton) covRightMouse (e->x (), e->y (), lon, lat);
    }
}



//!  This isn't used yet but maybe later

void 
pfmView::slotCovMouseRelease (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton) {};
  if (e->button () == Qt::MidButton) {};
  if (e->button () == Qt::RightButton) {};
}



//!  The mouse was moved in the coverage map.

void
pfmView::slotCovMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 lon, NV_FLOAT64 lat)
{
  NV_CHAR ltstring[25], lnstring[25], hem;
  NV_FLOAT64 deg, min, sec;
  NV_F64_COORD2 xy;
  QString y_string, x_string;
  BIN_RECORD bin;


  //  Let other ABE programs know which window we're in.  This is a special case since we have two map windows here.  We're lying about the 
  //  active window ID (process ID) by subtracting 1 from this program's PID.  This makes trackCursor track it locally.

  misc.abe_share->active_window_id = misc.process_id - 1;


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


  //  We only change the status PFM values if we were actually over a PFM data area

  if (hit >= 0) pfmLabel->setText (QFileInfo (misc.abe_share->open_args[hit].list_path).fileName ().remove (".pfm"));


  strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
  strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));

  latLabel->setText (ltstring);
  lonLabel->setText (lnstring);


  NV_FLOAT64 diff_lat, diff_lon;

  switch (misc.cov_function)
    {
    case COV_DRAW_RECTANGLE:
      cov->dragRubberbandRectangle (cov_rb_rectangle, lon, lat);
      break;

    case COV_DRAG_RECTANGLE:
      diff_lat = lat - cov_start_drag_lat;
      diff_lon = lon - cov_start_drag_lon;

      for (NV_INT32 i = 0 ; i < 4 ; i++)
        {
          cov_drag_bounds.min_x = mapdef.bounds[mapdef.zoom_level].min_x + diff_lon;
          cov_drag_bounds.max_x = mapdef.bounds[mapdef.zoom_level].max_x + diff_lon;
          cov_drag_bounds.min_y = mapdef.bounds[mapdef.zoom_level].min_y + diff_lat;
          cov_drag_bounds.max_y = mapdef.bounds[mapdef.zoom_level].max_y + diff_lat;
        }

      if (cov_drag_bounds.min_x < mapdef.bounds[0].min_x)
        {
          cov_drag_bounds.min_x = mapdef.bounds[0].min_x;
          cov_drag_bounds.max_x = mapdef.bounds[0].min_x + (mapdef.bounds[mapdef.zoom_level].max_x - mapdef.bounds[mapdef.zoom_level].min_x);
        }

      if (cov_drag_bounds.max_x > mapdef.bounds[0].max_x)
        {
          cov_drag_bounds.max_x = mapdef.bounds[0].max_x;
          cov_drag_bounds.min_x = mapdef.bounds[0].max_x - (mapdef.bounds[mapdef.zoom_level].max_x - mapdef.bounds[mapdef.zoom_level].min_x);
        }

      if (cov_drag_bounds.min_y < mapdef.bounds[0].min_y)
        {
          cov_drag_bounds.min_y = mapdef.bounds[0].min_y;
          cov_drag_bounds.max_y = mapdef.bounds[0].min_y + (mapdef.bounds[mapdef.zoom_level].max_y - mapdef.bounds[mapdef.zoom_level].min_y);
        }

      if (cov_drag_bounds.max_y > mapdef.bounds[0].max_y)
        {
          cov_drag_bounds.max_y = mapdef.bounds[0].max_y;
          cov_drag_bounds.min_y = mapdef.bounds[0].max_y - (mapdef.bounds[mapdef.zoom_level].max_y - mapdef.bounds[mapdef.zoom_level].min_y);
        }

      cov->setMovingRectangle (&cov_mv_rectangle, cov_drag_bounds.min_x, cov_drag_bounds.min_y, cov_drag_bounds.max_x, cov_drag_bounds.max_y,
                               Qt::blue, LINE_WIDTH, NVFalse, Qt::SolidLine);

      break;
    }
}



//!  We don't do anything pre-coastline draw in the coverage map

void 
pfmView::slotCovPreRedraw (NVMAP_DEF map_covdef __attribute__ ((unused)))
{
}



//!  Slot called after all of the background stuff has been drawn in the coverage map.  This is where we draw the coverage.

void 
pfmView::slotCovPostRedraw (NVMAP_DEF covdef)
{
  NV_FLOAT64 z;
  NV_INT32 pix_z, cov_area_x, cov_area_y, cov_area_width, cov_area_height, cov_area_bin_x, cov_area_bin_y;


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  //  Get the actual map area corner pixels.

  cov->map_to_screen (1, &covdef.bounds[0].min_x, &covdef.bounds[0].max_y, &z, &cov_area_x, &cov_area_y, &pix_z);
  cov->map_to_screen (1, &covdef.bounds[0].max_x, &covdef.bounds[0].min_y, &z, &cov_area_width, &cov_area_height, &pix_z);

  cov_area_width = cov_area_width - cov_area_x;
  cov_area_height = cov_area_height - cov_area_x;


  NV_U_BYTE *coverage = NULL;
  QColor c1, prev_c1;
  NV_INT32 cov_start_row, cov_start_col, cov_start_height, cov_start_width;


  //  Remove the box outline prior to drawing.

  cov->discardRubberbandRectangle (&cov_rb_rectangle);


  c1.setRgb (255, 255, 255);

  if (misc.cov_clear)
    {
      cov->fillRectangle (misc.total_mbr.min_x, misc.total_mbr.min_y, misc.total_mbr.max_x, misc.total_mbr.max_y, c1, NVTrue);
    }
  else
    {
      cov->fillRectangle (misc.total_displayed_area.min_x, misc.total_displayed_area.min_y, misc.total_displayed_area.max_x, 
                          misc.total_displayed_area.max_y, c1, NVTrue);
    }


  //  Loop through the open (and displayed) PFMs and paint the coverage for each.

  for (NV_INT32 pfm = misc.abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Only draw coverage if we're displaying the layer.

      if (misc.abe_share->display_pfm[pfm])
        {
          if (misc.cov_clear)
            {
              cov_start_row = 0;
              cov_start_col = 0;
              cov_start_height = misc.abe_share->open_args[pfm].head.bin_height;
              cov_start_width = misc.abe_share->open_args[pfm].head.bin_width;
            }
          else
            {
              NV_INT32 displayed_area_width = (NINT ((misc.total_displayed_area.max_x - misc.total_displayed_area.min_x) / 
                                                     misc.abe_share->open_args[pfm].head.x_bin_size_degrees));
              NV_INT32 displayed_area_height = (NINT ((misc.total_displayed_area.max_y - misc.total_displayed_area.min_y) / 
                                                      misc.abe_share->open_args[pfm].head.y_bin_size_degrees));
              NV_INT32 displayed_area_row = NINT ((misc.total_displayed_area.min_y - misc.abe_share->open_args[pfm].head.mbr.min_y) / 
                                                  misc.abe_share->open_args[pfm].head.y_bin_size_degrees);
              NV_INT32 displayed_area_column = NINT ((misc.total_displayed_area.min_x - misc.abe_share->open_args[pfm].head.mbr.min_x) / 
                                                     misc.abe_share->open_args[pfm].head.x_bin_size_degrees);

              if (displayed_area_column + displayed_area_width > misc.abe_share->open_args[pfm].head.bin_width - 1)
                displayed_area_width = misc.abe_share->open_args[pfm].head.bin_width - displayed_area_column - 1;

              if (displayed_area_row + displayed_area_height > misc.abe_share->open_args[pfm].head.bin_height - 1)
                displayed_area_height = misc.abe_share->open_args[pfm].head.bin_height - displayed_area_row - 1;


              cov_start_row = displayed_area_row;
              cov_start_col = displayed_area_column;
              cov_start_height = displayed_area_height;
              cov_start_width = displayed_area_width;
            }


          //  Determine how many bins cover a single pixel???

          cov_area_bin_x = misc.abe_share->open_args[pfm].head.bin_width / cov_area_width;
          if (misc.abe_share->open_args[pfm].head.bin_width % cov_area_width) cov_area_bin_x++;

          cov_area_bin_y = misc.abe_share->open_args[pfm].head.bin_height / cov_area_height;
          if (misc.abe_share->open_args[pfm].head.bin_height % cov_area_height) cov_area_bin_y++;


          NV_INT32 checked_sum, verified_sum, cov_col, cov_width;
          NV_I32_COORD2 coord;
          NV_BOOL has_data;

          NV_INT32 cov_row = cov_start_row / cov_area_bin_y;

          NV_INT32 cov_height = cov_start_height / cov_area_bin_y;

          NV_INT32 size = (cov_area_bin_y + 1) * ((cov_start_width) + cov_area_bin_x + 1);


          //  If size is less than or equal to 0 we have asked for an area outside of the PFM's MBR so we don't want to do anything.

          if (size > 0)
            {
              if ((coverage = (NV_U_CHAR *) calloc (size, sizeof (NV_U_CHAR))) == NULL)
                {
                  fprintf (stderr, tr ("Unable to allocate coverage map memory.\n").toAscii ());
                  fprintf (stderr, tr ("Requesting %d bytes.\n").toAscii (), size);
                  exit (-1);
                }

              for (NV_INT32 i = cov_row ; i <= cov_row + cov_height ; i++)
                {
                  //  Drawing one row at a time.

                  NV_FLOAT64 start_lat = misc.abe_share->open_args[pfm].head.mbr.min_y + (i * cov_area_bin_y) * 
                    misc.abe_share->open_args[pfm].head.y_bin_size_degrees;
                  NV_FLOAT64 lat = misc.abe_share->open_args[pfm].head.mbr.min_y + ((i + 1) * cov_area_bin_y) * 
                    misc.abe_share->open_args[pfm].head.y_bin_size_degrees;


                  memset (coverage, 0, size);

                  for (NV_INT32 k = 0 ; k <= cov_area_bin_y ; k++)
                    {
                      coord.y = (i * cov_area_bin_y) + k;
                      for (NV_INT32 m = 0 ; m <= cov_start_width ; m++)
                        {
                          coord.x = m + cov_start_col;

                          read_cov_map_index (misc.pfm_handle[pfm], coord, (coverage + k * cov_start_width + m));
                        }
                    }


                  cov_col = cov_start_col / cov_area_bin_x;
                  cov_width = cov_start_width / cov_area_bin_x + cov_col;


                  //  We're only going to paint once the color has changed.  That is, we're making a long skinny rectangle
                  //  until the color needs to change and then we're painting the rectangle.

                  prev_c1.setRgb (255, 255, 255);
                  prev_c1.setAlpha (0);


                  NV_FLOAT64 start_lon = misc.abe_share->open_args[pfm].head.mbr.min_x + ((cov_col - 1) * cov_area_bin_x) * 
                    misc.abe_share->open_args[pfm].head.x_bin_size_degrees;
                  NV_FLOAT64 lon = 0.0;

                  for (NV_INT32 j = cov_col ; j < cov_width ; j++)
                    {
                      lon = misc.abe_share->open_args[pfm].head.mbr.min_x + (j * cov_area_bin_x) *
                        misc.abe_share->open_args[pfm].head.x_bin_size_degrees;

                      has_data = NVFalse;
                      checked_sum = 0;
                      verified_sum = 0;

                      for (NV_INT32 k = 0 ; k < cov_area_bin_y ; k++)
                        {
                          for (NV_INT32 m = 0 ; m < cov_area_bin_x ; m++)
                            {
                              if ((*(coverage + k * cov_start_width + (j - cov_col) * cov_area_bin_x + m)) & COV_DATA) 
                                has_data = NVTrue;
                              if ((*(coverage + k * cov_start_width + (j - cov_col) * cov_area_bin_x + m)) & COV_CHECKED) 
                                checked_sum++;
                              if ((*(coverage + k * cov_start_width + (j - cov_col) * cov_area_bin_x + m)) & COV_VERIFIED) 
                                verified_sum++;
                            }
                        }

                      if (has_data)
                        {
                          c1 = misc.cov_color;

                          if (checked_sum == cov_area_bin_x * cov_area_bin_y) c1 = misc.cov_checked_color;
                          if (verified_sum == cov_area_bin_x * cov_area_bin_y) c1 = misc.cov_verified_color;

                          c1.setAlpha (255);
                        }
                      else
                        {
                          c1.setRgb (255, 255, 255);


                          //  If it's gonna be white we want it to be clear so it doesn't write over any 
                          //  underlaying PFM coverage layers.

                          c1.setAlpha (0);
                        }


                      if (c1 != prev_c1 && j != cov_col)
                        {
                          cov->fillRectangle (start_lon, start_lat, lon, lat, prev_c1, NVTrue);

                          start_lon = lon;

                          prev_c1 = c1;
                        }
                    }

                  cov->fillRectangle (start_lon, start_lat, lon, lat, prev_c1, NVTrue);
                }


              free (coverage);
            }
        }
    }


  //  Figure out which is the highest active (displayed) layer.

  NV_INT32 hit = 0;
  for (NV_INT32 pfm = misc.abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      if (misc.abe_share->display_pfm[pfm]) hit = pfm;
    }


  //  Draw a semi-transparent outline around each PFM.

  for (NV_INT32 pfm = misc.abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Only draw coverage if we're displaying the layer.

      if (misc.abe_share->display_pfm[pfm])
        {
          if (pfm == hit)
            {
              c1 = Qt::green;
            }
          else
            {
              c1 = Qt::red;
            }

          c1.setAlpha (128);
          cov->drawPolygon (misc.abe_share->open_args[pfm].head.polygon_count, misc.abe_share->open_args[pfm].head.polygon, 
                            c1, LINE_WIDTH, NVTrue, Qt::SolidLine, NVTrue);
        }
    }


  if (options.display_feature) drawCovFeatures ();


  //  Restore the box outline after drawing.

  redrawCovBounds ();


  cov->setCursor (editRectCursor);



  //  Draw an outline around the entire area.

  cov->drawRectangle (covdef.initial_bounds.min_x, covdef.initial_bounds.min_y, covdef.initial_bounds.max_x,
                      covdef.initial_bounds.max_y, Qt::blue/*options.contour_highlight_color*/, LINE_WIDTH, Qt::SolidLine, NVTrue);

  qApp->restoreOverrideCursor ();
}



//!  Draw the features on the coverage map

void 
pfmView::drawCovFeatures ()
{
  for (NV_U_INT32 j = 0 ; j < misc.bfd_header.number_of_records ; j++)
    {
      if ((misc.feature[j].confidence_level || misc.abe_share->layer_type == AVERAGE_DEPTH || misc.abe_share->layer_type == MIN_DEPTH || 
           misc.abe_share->layer_type == MAX_DEPTH) && (!misc.feature[j].parent_record || options.display_children))
        {
          if (misc.feature[j].latitude >= misc.total_mbr.min_y &&
              misc.feature[j].latitude <= misc.total_mbr.max_y &&
              misc.feature[j].longitude >= misc.total_mbr.min_x &&
              misc.feature[j].longitude <= misc.total_mbr.max_x)
            {
              //  Check for the type of feature display

              if (options.display_feature == 1 || (options.display_feature == 2 && misc.feature[j].confidence_level != 5) ||
                  (options.display_feature == 3 && misc.feature[j].confidence_level == 5))
                {
                  //  Check the feature for the feature search string and highlighting.

                  QString feature_info;
                  NV_BOOL highlight;
                  if (checkFeature (&misc, &options, j, &highlight, &feature_info))
                    {
                      if (misc.feature[j].confidence_level)
                        {
                          if (misc.feature[j].confidence_level == 5)
                            {
                              cov->fillRectangle (misc.feature[j].longitude, misc.feature[j].latitude, COV_FEATURE_SIZE, 
                                                  COV_FEATURE_SIZE, options.cov_verified_feature_color, NVTrue);
                            }
                          else
                            {
                              cov->fillRectangle (misc.feature[j].longitude, misc.feature[j].latitude, COV_FEATURE_SIZE, 
                                                  COV_FEATURE_SIZE, options.cov_feature_color, NVTrue);
                            }
                        }
                      else
                        {
                          if (!misc.surface_val)
                            {
                              cov->fillRectangle (misc.feature[j].longitude, misc.feature[j].latitude, COV_FEATURE_SIZE,
                                                  COV_FEATURE_SIZE, options.cov_inv_feature_color, NVTrue);
                            }
                        }
                    }
                }
            }
        }
    }
}



//!  Kick off the overlays dialog.

void 
pfmView::slotOverlays ()
{
  manageOverlays_dialog = new manageOverlays (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  manageOverlays_dialog->move (x () + width () / 2 - manageOverlays_dialog->width () / 2, y () + height () / 2 - manageOverlays_dialog->height () / 2);

  connect (manageOverlays_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotManageOverlaysDataChanged ()));
}



//!  This slot is called if the overlays data has been changed in the overlays dialog.

void 
pfmView::slotManageOverlaysDataChanged ()
{
  if (!misc.drawing) redrawMap (NVTrue, NVFalse);
}



//!  Kick off the delete/restore file dialog.

void 
pfmView::slotDeleteFile ()
{
  deleteFile_dialog = new deleteFile (this, &options, &misc);

  connect (deleteFile_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotDeleteFileDataChanged ()));
}



//!  This slot is called if one or more input file statuses was changed in the delete/restore file dialog.

void 
pfmView::slotDeleteFileDataChanged ()
{
  slotRedrawCoverage ();
  if (!misc.drawing) redrawMap (NVTrue, NVTrue);
}



//!  Kick off the delete file queue dialog.

void 
pfmView::slotDeleteQueue ()
{
  deleteQueue_dialog = new deleteQueue (this, &options, &misc);

  connect (deleteQueue_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotDeleteFileDataChanged ()));
}



//!  Kick off the change path dialog.

void 
pfmView::slotChangePath ()
{
  changePath_dialog = new changePath (this, &options, &misc);

  connect (changePath_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotChangePathDataChanged ()));
}



//!  This slot is called if one or more input file data paths was changed in the change path dialog.

void 
pfmView::slotChangePathDataChanged ()
{
  //  We've changed the file names in the .ctl file so we have to reread them.

  close_pfm_file (misc.pfm_handle[0]);
  misc.abe_share->open_args[0].checkpoint = 0;
  misc.pfm_handle[0] = open_existing_pfm_file (&misc.abe_share->open_args[0]);
}



//!  Kick off the find feature dialog.

void 
pfmView::slotFindFeature ()
{
  if (misc.bfd_open)
    {
      findFeature_dialog = new findFeature (this, cov, &options, &misc);


      //  Put the dialog in the middle of the screen.

      findFeature_dialog->move (x () + width () / 2 - findFeature_dialog->width () / 2, y () + height () / 2 - findFeature_dialog->height () / 2);
    }
  else
    {
      QMessageBox::warning (this, tr ("pfmView Find Feature"), tr ("No feature file available."));
    }
}



//!  Change chart scale (almost never used anymore).

void 
pfmView::slotDefinePolygonChartScaleChanged ()
{
  redrawMap (NVTrue, NVTrue);
  qApp->processEvents ();
}



//!  Slot called when the user wants to define a feature polygon.

void 
pfmView::slotDefinePolygon ()
{
  if (misc.bfd_open)
    {
      if (options.chart_scale)
        {
          misc.feature_poly_radius = (0.005 * options.chart_scale) / 2.0;
        }
      else
        {
          //  Default to 1:5000

          misc.feature_poly_radius = (0.005 * 5000.0) / 2.0;
        }

      misc.def_feature_poly = NVTrue;


      redrawMap (NVTrue, NVTrue);
      qApp->processEvents ();


      definePolygon_dialog = new definePolygon (this, &options, &misc);
      definePolygon_dialog->setModal (TRUE);

      connect (definePolygon_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotEditFeatureDataChanged ()));
      connect (definePolygon_dialog, SIGNAL (definePolygonSignal ()), this, SLOT (slotEditFeatureDefinePolygon ()));
      connect (definePolygon_dialog, SIGNAL (chartScaleChangedSignal ()), this, SLOT (slotDefinePolygonChartScaleChanged ()));

      definePolygon_dialog->show ();
    }
  else
    {
      QMessageBox::warning (this, tr ("pfmView Define Feature Polygon"), tr ("No feature file available."));
    }
}



//!  Display the 0 layer PFM header.

void 
pfmView::slotDisplayHeader ()
{
  displayHeader_dialog = new displayHeader (this, &misc);


  //  Put the dialog in the middle of the screen.

  displayHeader_dialog->move (x () + width () / 2 - displayHeader_dialog->width () / 2, y () + height () / 2 - displayHeader_dialog->height () / 2);
}



//!  Slot caled when the user wants to output a file of ASCII data point positions and Z values.

void 
pfmView::slotOutputDataPoints ()
{
  map->setToolTip (tr ("Left click to define points, double click to define last point, middle click to abort"));
  misc.save_function = misc.function;
  misc.function = OUTPUT_POINTS;
  setFunctionCursor (misc.function);
}



//!  Zooms to a specific area based on an input area file.

void 
pfmView::slotZoomToArea ()
{
  static QString filter = tr ("AREA (*.ARE *.are *.afs)");


  if (!QDir (options.area_dir).exists ()) options.area_dir = options.input_pfm_dir;


  QFileDialog fd (this, tr ("pfmView Zoom to area file")); 
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, options.area_dir);


  QStringList filters;
  filters << tr ("AREA (*.ARE *.are *.afs)");

  fd.setFilters (filters);
  fd.setFileMode (QFileDialog::ExistingFile);
  fd.selectFilter (filter);

  QStringList files;
  QString newfile;
  if (fd.exec () == QDialog::Accepted) 
    {
      files = fd.selectedFiles ();
      newfile = files.at (0);


      if (!newfile.isEmpty())
        {
          NV_INT32 polygon_count;
          NV_FLOAT64 polygon_x[200], polygon_y[200];
          NV_CHAR path[512];
          NV_F64_XYMBR mbr;

          strcpy (path, newfile.toAscii ());

          if (get_area_mbr (path, &polygon_count, polygon_x, polygon_y, &mbr))
            {
              cov_area_defined = NVTrue;

              zoomIn (mbr, NVTrue);


              misc.cov_function = COV_START_DRAW_RECTANGLE;

              redrawCovBounds ();
            }

          options.area_dir = fd.directory ().absolutePath ();
        }
    }
}



//!  Slot called when the user wants to define a rectangular area file.

void 
pfmView::slotDefineRectArea ()
{
  map->setToolTip (tr ("Left click to start rectangle, middle click to abort, right click for menu"));
  misc.save_function = misc.function;
  misc.function = DEFINE_RECT_AREA;
  setFunctionCursor (misc.function);
}



//!  Slot called when the user wants to define a polygonal area file.

void 
pfmView::slotDefinePolyArea ()
{
  map->setToolTip (tr ("Left click to define polygon vertices, double click to define last vertex, middle click to abort"));
  misc.save_function = misc.function;
  misc.function = DEFINE_POLY_AREA;
  setFunctionCursor (misc.function);
}



//!  Define the cursor based on the active function.  Also adds a background color and function message to the status bar.

void 
pfmView::setFunctionCursor (NV_INT32 function)
{
  QString msc;


  discardMovableObjects ();

  switch (function)
    {
    case RECT_EDIT_AREA:
    case RECT_EDIT_AREA_3D:
      misc.statusProgLabel->setText (tr (" Rectangular edit mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (editRectCursor);
      break;

    case SELECT_HIGH_POINT:
      misc.statusProgLabel->setText (tr (" Select high point mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (highCursor);
      break;

    case SELECT_LOW_POINT:
      misc.statusProgLabel->setText (tr (" Select low point mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (lowCursor);
      break;

    case POLY_EDIT_AREA:
    case POLY_EDIT_AREA_3D:
      misc.statusProgLabel->setText (tr (" Polygon edit mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (editPolyCursor);
      break;

    case GRAB_CONTOUR:
      misc.statusProgLabel->setText (tr (" Capture contour mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (grabContourCursor);
      break;

    case RECT_FILTER_AREA:
      misc.statusProgLabel->setText (tr (" Rectangle filter mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterRectCursor);
      break;

    case POLY_FILTER_AREA:
      misc.statusProgLabel->setText (tr (" Polygon filter mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterPolyCursor);
      break;

    case RECT_FILTER_MASK:
      misc.statusProgLabel->setText (tr (" Rectangle filter mask mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterMaskRectCursor);
      break;

    case POLY_FILTER_MASK:
      misc.statusProgLabel->setText (tr (" Polygon filter mask mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterMaskPolyCursor);
      break;

    case ZOOM_IN_AREA:
      misc.statusProgLabel->setText (tr (" Zoom in rectangle mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (zoomCursor);
      break;

    case OUTPUT_POINTS:
      misc.statusProgLabel->setText (tr (" Output data points mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::CrossCursor);
      break;

    case DEFINE_RECT_AREA:
      misc.statusProgLabel->setText (tr (" Define rectangular area file mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::SizeFDiagCursor);
      break;

    case DEFINE_POLY_AREA:
      misc.statusProgLabel->setText (tr (" Define polygonal area file mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::ArrowCursor);
      break;

    case DEFINE_FEATURE_POLY_AREA:
      misc.statusProgLabel->setText (tr (" Define polygonal feature area mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (pencilCursor);
      break;

    case SUNANGLE:
      misc.statusProgLabel->setText (tr (" Define sun angle mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::CrossCursor);
      break;

    case DELETE_FEATURE:
      misc.statusProgLabel->setText (tr (" Delete feature mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (deleteFeatureCursor);
      break;

    case EDIT_FEATURE:
      misc.statusProgLabel->setText (tr (" Edit feature mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (editFeatureCursor);
      break;

    case ADD_FEATURE:
      misc.statusProgLabel->setText (tr (" Add feature mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (addFeatureCursor);
      break;

    case DRAW_CONTOUR:
      msc.sprintf (tr (" Draw contour - %.2f ").toAscii (), misc.draw_contour_level);
      misc.statusProgLabel->setText (msc);
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (drawContourCursor);
      break;

    case REMISP_AREA:
      misc.statusProgLabel->setText (tr (" Rectangular regrid mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (remispCursor);
      break;

    case DRAW_CONTOUR_FILTER:
      msc.sprintf (tr (" Draw contour for filter - %.2f ").toAscii (), misc.draw_contour_level);
      misc.statusProgLabel->setText (msc);
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (drawContourCursor);
      break;

    case REMISP_FILTER:
      misc.statusProgLabel->setText (tr (" Remisp filter mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterPolyCursor);
      break;
    }
}



//!  This is the stderr read return from the DNC import QProcess.  Hopefully you won't see anything here.

void 
pfmView::slotImportReadyReadStandardError ()
{
  QByteArray response = importProc->readAllStandardError ();
  NV_CHAR *res = response.data ();

  fprintf (stderr, "%s %d %s\n", __FILE__, __LINE__, res);
  fflush (stderr);
}



//!  Error return from the DNC import process.

void 
pfmView::slotImportError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("Unable to start the import process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("The import process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("The import process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("There was a write error to the import process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("There was a read error from the import process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("The import process died with an unknown error!"));
      break;
    }
}



//!  This is the return from the DNC import QProcess (when finished normally)

void 
pfmView::slotImportDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  if (importProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("pfmView Import"), tr ("An error occurred during import"));
    }
  else
    {
      //  First we have to remove the "dump_dnc" files that we aren't going to use.

      QString name;
      name = importTempFile + "_coast.trk";
      QFile file (name);
      file.remove ();

      name = importTempFile + ".trk";
      file.setFileName (name);
      file.remove ();

      name = importTempFile + ".llz";
      file.setFileName (name);
      file.remove ();


      if (!misc.bfd_open)
        {
          QDir featureDir;
          name = QString (misc.abe_share->open_args[0].list_path);
          name.replace (".pfm", ".bfd");

          strcpy (misc.abe_share->open_args[0].target_path, name.toAscii ());

          name = importTempFile + ".bfd";
          file.setFileName (name);
          file.rename (QString (misc.abe_share->open_args[0].target_path));

          update_target_file (misc.pfm_handle[0], misc.abe_share->open_args[0].list_path, misc.abe_share->open_args[0].target_path);
        }
      else
        {
          NV_INT32 bfdh;
          BFDATA_HEADER bfd_header;
          BFDATA_RECORD bfd_record;
          BFDATA_POLYGON bfd_polygon;

          name = importTempFile + ".bfd";
          file.setFileName (name);
          NV_CHAR filename[512];
          strcpy (filename, name.toAscii ());

          if ((bfdh = binaryFeatureData_open_file (filename, &bfd_header, BFDATA_READONLY)) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (this, tr ("pfmView"), tr ("Unable to open feature file\nReason: ") + msg);
              binaryFeatureData_close_file (misc.bfd_handle);
              return;
            }


          //  Read the new feature file and append it to the existing feature file.

          for (NV_U_INT32 i = 0 ; i < bfd_header.number_of_records ; i++)
            {
              if (binaryFeatureData_read_record (bfdh, i, &bfd_record) < 0)
                {
                  QString msg = QString (binaryFeatureData_strerror ());
                  QMessageBox::warning (this, tr ("pfmView"), tr ("Unable to read feature record\nReason: ") + msg);
                  binaryFeatureData_close_file (misc.bfd_handle);
                  binaryFeatureData_close_file (bfdh);
                  return;
                }


              if (bfd_record.poly_count)
                {
                  if (binaryFeatureData_read_polygon (bfdh, i, &bfd_polygon) < 0)
                    {
                      QString msg = QString (binaryFeatureData_strerror ());
                      QMessageBox::warning (this, tr ("pfmView"), tr ("Unable to read feature polygon\nReason: ") + msg);
                      binaryFeatureData_close_file (misc.bfd_handle);
                      binaryFeatureData_close_file (bfdh);
                      return;
                    }
                }


              if (binaryFeatureData_write_record (misc.bfd_handle, BFDATA_NEXT_RECORD, &bfd_record, &bfd_polygon, NULL) < 0)
                {
                  QString msg = QString (binaryFeatureData_strerror ());
                  QMessageBox::warning (this, tr ("pfmView"), tr ("Unable to write feature record\nReason: ") + msg);
                  binaryFeatureData_close_file (misc.bfd_handle);
                  binaryFeatureData_close_file (bfdh);
                  return;
                }
            }


          binaryFeatureData_close_file (bfdh);

          file.remove ();
        }

      slotEditFeatureDataChanged ();
    }
}



//!  Import DNC feature information.

void 
pfmView::slotImport ()
{
  void importDNC (QString dncName, MISC *misc);


  QStringList files;
  QString file;


  if (!QDir (options.dnc_dir).exists ()) options.dnc_dir = options.input_pfm_dir;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmView Import DNC data"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.output_area_dir);


  fd->setFileMode (QFileDialog::DirectoryOnly);

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      if (!file.isEmpty())
        {
          options.dnc_dir = fd->directory ().absolutePath ();

          importProc = new QProcess (this);

          QStringList arguments;


          arguments += file;
          importTempFile.sprintf ("./pfmView_import_%05d", misc.process_id);
          arguments += importTempFile;

          connect (importProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, 
                   SLOT (slotImportDone (int, QProcess::ExitStatus)));
          connect (importProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotImportReadyReadStandardError ()));
          connect (importProc, SIGNAL (error (QProcess::ProcessError)), this, 
                   SLOT (slotImportError (QProcess::ProcessError)));


          importProc->start (QString (options.import_prog), arguments);
        }
    }
}



//!  Change the feature file associated with the top (0) layer PFM.

void 
pfmView::slotChangeFeature ()
{
  QStringList files, filters;
  QString file;
  QFileInfo fi = QFileInfo (QString (misc.abe_share->open_args[0].target_path));
  QDir dir = fi.dir ();


  QFileDialog *fd = new QFileDialog (this, tr ("pfmView Change feature file"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.input_pfm_dir);


  filters << tr ("Binary Feature Data file (*.bfd)")
          << tr ("NAVO standard target file (*.xml)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("Binary Feature Data file (*.bfd)"));
  fd->selectFile (QString (misc.abe_share->open_args[0].target_path));


  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);

      if (!file.isEmpty())
        {
          strcpy (misc.abe_share->open_args[0].target_path, file.toAscii ());

          update_target_file (misc.pfm_handle[0], misc.abe_share->open_args[0].list_path, misc.abe_share->open_args[0].target_path);

          slotEditFeatureDataChanged ();
        }
    }
}



//!  Change the mosiac (GeoTIFF) file associated with the top (0) layer PFM.

void 
pfmView::slotChangeMosaic ()
{
  QStringList files, filters;
  QString file;

  QFileInfo fi = QFileInfo (QString (misc.abe_share->open_args[0].image_path));
  QDir dir = fi.dir ();


  QFileDialog *fd = new QFileDialog (this, tr ("pfmView Change mosaic file"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, dir.absolutePath ());


  filters << tr ("GeoTIFF mosaic file (*.tif *.TIF)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("GeoTIFF mosaic file (*.tif *.TIF)"));
  fd->selectFile (QString (misc.abe_share->open_args[0].image_path));


  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);

      if (!file.isEmpty())
        {
          strcpy (misc.abe_share->open_args[0].image_path, file.toAscii ());

          update_mosaic_file (misc.pfm_handle[0], misc.abe_share->open_args[0].list_path, misc.abe_share->open_args[0].image_path);


          startMosaic->setEnabled (TRUE);


          //  If the mosaic viewer was running, tell it to switch files.

          misc.abe_share->key = MOSAICVIEW_FORCE_RELOAD;
        }
    }
}



//!  This is the stderr read return from the mosaic viewer QProcess.  Hopefully you won't see anything here.

void 
pfmView::slotMosaicReadyReadStandardError ()
{
  QByteArray response = mosaicProc->readAllStandardError ();
  NV_CHAR *res = response.data ();

  fprintf (stderr, "%s %d %s\n", __FILE__, __LINE__, res);
  fflush (stderr);
}



//!  Error return from the mosaic viewer process.

void 
pfmView::slotMosaicError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("Unable to start the mosaic viewer process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("The mosaic viewer process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("The mosaic viewer process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("There was a write error to the mosaic viewer process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("There was a read error from the mosaic viewer process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("The mosaic viewer process died with an unknown error!"));
      break;
    }

  programTimerCount = 0;
  programTimer->stop ();
  qApp->restoreOverrideCursor ();
}



//!  This is the return from the mosaic viewer QProcess (when finished normally)

void 
pfmView::slotMosaicDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  if (mosaicProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("pfmView Mosaic"), tr ("An error occurred while running the mosaic viewer"));
    }
}



//!  This starts the mosaic viewer.  Most people will use the pfm3D viewer and drape the mosaic on it instead of using this.

void 
pfmView::slotStartMosaicViewer ()
{
  if (!strcmp (misc.abe_share->open_args[0].image_path, "NONE")) return;


  if (mosaicProc)
    {
      if (mosaicProc->state () == QProcess::Running)
        {
          disconnect (mosaicProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, 
                      SLOT (slotMosaicDone (int, QProcess::ExitStatus)));
          disconnect (mosaicProc, SIGNAL (error (QProcess::ProcessError)), this,
                      SLOT (slotMosaicError (QProcess::ProcessError)));
          disconnect (mosaicProc, SIGNAL (readyReadStandardError ()), this,
                      SLOT (slotMosaicReadyReadStandardError ()));

          mosaicProc->kill ();
        }
      delete mosaicProc;
    }


  program = options.mosaic_prog.section (' ', 0, 0);

  mosaicProc = new QProcess (this);

  QStringList arguments;
  arguments.clear ();

  arguments += QString (misc.abe_share->open_args[0].image_path);
  arguments += "--actionkey00=" + options.mosaic_actkey.section (',', 0, 0);
  arguments += "--actionkey01=" + options.mosaic_actkey.section (',', 1, 1);
  arguments += "--actionkey02=" + options.mosaic_actkey.section (',', 2, 2);
  arguments += QString ("--shared_memory_key=%1").arg (misc.abe_share->ppid);


  QString command;

  command = options.mosaic_prog;
  command.remove (" [MOSAIC_FILE]");


  connect (mosaicProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, 
           SLOT (slotMosaicDone (int, QProcess::ExitStatus)));
  connect (mosaicProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotMosaicError (QProcess::ProcessError)));
  connect (mosaicProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotMosaicReadyReadStandardError ()));


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();
  misc.abe_share->key = WAIT_FOR_START;


  mosaicProc->start (command, arguments);


  //  Starting a half second timer so that we can set the wait cursor.  If we use the started signal from QProcess
  //  the return is instant even though the mosaic viewer isn't visible.  This is just to give the user a warm and
  //  fuzzy.  If the WAIT_FOR_START flag isn't reset in 5 seconds we release anyway.

  programTimerCount = 0;
  programTimer->start (500);
}



//!  This is just to let you know that the ancillary program you tried to start is trying to start ;-)

void 
pfmView::slotProgramTimer ()
{
  if (programTimerCount > 10 || misc.abe_share->key != WAIT_FOR_START)
    {
      programTimer->stop ();

      if (programTimerCount > 10)
        {
          QMessageBox::information (this, tr ("pfmView mosaic viewer"), tr ("The program ") + 
                                    program + tr (" has been started but may take some time to appear.\n") +
                                    tr ("Please be patient, it will appear eventually (or die trying ;-)\n\n") +
                                    tr ("IMPORTANT NOTE: Due to processing overhead the system may ") +
                                    tr ("freeze for a few moments when ") + program + 
                                    tr (" is almost ready to appear.  Again, please be patient."));
          qApp->processEvents ();
        }

      qApp->restoreOverrideCursor ();
      programTimerCount = 0;
    }
  else
    {
      programTimerCount++;
    }
}



//!  This is the stderr read return from the 3D viewer QProcess.  Hopefully you won't see anything here.

void 
pfmView::slot3DReadyReadStandardError ()
{
  QByteArray response = threeDProc->readAllStandardError ();
  NV_CHAR *res = response.data ();

  fprintf (stderr, "%s %d %s\n", __FILE__, __LINE__, res);
  fflush (stderr);
}



//!  This is the error return from the 3D viewer process.

void 
pfmView::slot3DError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("Unable to start the 3D viewer process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("The 3D viewer process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("The 3D viewer process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("There was a write error to the 3D viewer process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("There was a read error from the 3D viewer process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("The 3D viewer process died with an unknown error!"));
      break;
    }

  programTimerCount = 0;
  programTimer->stop ();
  qApp->restoreOverrideCursor ();
}



//!  This is the return from the 3D viewer QProcess (when finished normally)

void 
pfmView::slot3DDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  if (threeDProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("pfmView 3D Viewer"), tr ("An error occurred while running the 3D viewer"));
    }
}



//!  This starts the 3D surface viewer (with or without draped GeoTIFF data).

void 
pfmView::slotStart3DViewer ()
{
  if (threeDProc)
    {
      if (threeDProc->state () == QProcess::Running)
        {
          disconnect (threeDProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slot3DDone (int, QProcess::ExitStatus)));
          disconnect (threeDProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slot3DError (QProcess::ProcessError)));
          disconnect (threeDProc, SIGNAL (readyReadStandardError ()), this, SLOT (slot3DReadyReadStandardError ()));

          threeDProc->kill ();
        }
      delete threeDProc;
    }


  program = options.threeD_prog.section (' ', 0, 0);

  threeDProc = new QProcess (this);

  QStringList arguments;
  arguments.clear ();

  arguments += QString (misc.abe_share->open_args[0].image_path);
  arguments += QString ("--shared_memory_key=%1").arg (misc.abe_share->ppid);


  QString command;

  command = options.threeD_prog;


  connect (threeDProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slot3DDone (int, QProcess::ExitStatus)));
  connect (threeDProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slot3DError (QProcess::ProcessError)));
  connect (threeDProc, SIGNAL (readyReadStandardError ()), this, SLOT (slot3DReadyReadStandardError ()));


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();
  misc.abe_share->key = WAIT_FOR_START;


  threeDProc->start (command, arguments);


  //  Starting a half second timer so that we can set the wait cursor.  If we use the started signal from QProcess
  //  the return is instant even though the 3D viewer isn't visible.  This is just to give the user a warm and
  //  fuzzy.  If the WAIT_FOR_START flag isn't reset in 5 seconds we release anyway.

  programTimerCount = 0;
  programTimer->start (500);
}



//!  Export the displayed view to a graphics file (jpg, GeoTIFF, etc.).

void 
pfmView::slotExportImage ()
{
  void export_image (QWidget *parent, MISC *misc, OPTIONS *options, nvMap *map);


  QStringList lst;
  bool ok;

  NV_INT32 w = misc.displayed_area_width[0];
  NV_INT32 h = misc.displayed_area_height[0];

  QString pfm_str = QString ("%1x%2").arg (w).arg (h) + tr (" (PFM resolution)");
  QString screen_str = QString ("%1x%2").arg (mapdef.draw_width).arg (mapdef.draw_height) + tr (" (Screen resolution)");

  lst << pfm_str << screen_str << tr ("1280x1024") << tr ("2560x2048") << tr ("5120x4096") << tr ("10240x8192");


  QString res = QInputDialog::getItem (this, tr ("pfmView"), tr ("Image resolution"), lst, 0, FALSE, &ok);

  if (ok && !res.isEmpty ())
    {
      w = res.section ('x', 0, 0).toInt ();


      //  Make sure we don't have a problem with the parentheses on the first two options.

      if (res.contains ('('))
        {
          h = res.section ('x', 1, 1).section (' ', 0, 0).toInt ();
        }
      else
        {
          h = res.section ('x', 1, 1).toInt ();
        }

      map->dummyBuffer (w, h, NVTrue);

      qApp->setOverrideCursor (Qt::WaitCursor);
      qApp->processEvents ();

      redrawMap (NVTrue, NVTrue);

      export_image (this, &misc, &options, map);


      qApp->restoreOverrideCursor ();


      map->dummyBuffer (0, 0, NVFalse);
    }
}



//!  Clear the filter masks.

void 
pfmView::clearFilterMasks ()
{
  //  Clear any filter masked areas.
  
  if (misc.poly_filter_mask_count)
    {
      if (misc.poly_filter_mask)
        {
          free (misc.poly_filter_mask);
          misc.poly_filter_mask = NULL;
          misc.poly_filter_mask_count = 0;
        }
    }
}



//!  Set the stoplight coloring boundaries.

void 
pfmView::setStoplight ()
{
  //  Check for stoplight display

  if (options.stoplight)
    {
      NV_FLOAT64 res;
      bool ok;

      res = QInputDialog::getDouble (this, tr ("pfmView"), tr ("Enter the minimum to mid crossover value:"), options.stoplight_min_mid,
                                     -9500.0, 11500.0, 1, &ok);
      if (ok) options.stoplight_min_mid = (NV_FLOAT32) res;


      res = QInputDialog::getDouble (this, tr ("pfmView"), tr ("Enter the maximum to mid crossover value:"), options.stoplight_max_mid,
                                     -9500.0, 11500.0, 1, &ok);
      if (ok) options.stoplight_max_mid = (NV_FLOAT32) res;


      if (options.stoplight_max_mid < options.stoplight_min_mid)
        {
          NV_FLOAT32 tmp = options.stoplight_max_mid;
          options.stoplight_max_mid = options.stoplight_min_mid;
          options.stoplight_min_mid = tmp;
        }
    }
  else
    {
      //  Make sure that the next time someone turns on stoplight it asks for new crossover values.

      options.stoplight_min_mid = options.stoplight_max_mid = 0.0;
    }


  QString tip = tr ("Toggle stoplight display [crossovers at %1 and %2]").arg (options.stoplight_min_mid, 2, 'f', 1).arg (options.stoplight_max_mid, 2, 'f', 1);
  bStoplight->setToolTip (tip);
}
