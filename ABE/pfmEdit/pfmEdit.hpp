
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



#ifndef PFM_EDIT_H
#define PFM_EDIT_H

#include "pfmEditDef.hpp"
#include "prefs.hpp"
#include "hotkeyHelp.hpp"
#include "version.hpp"

#include "editFeature.hpp"
#include "ancillaryProg.hpp"
#include "hotkeyPolygon.hpp"
#include "externalFilter.hpp"
#include "manageLayers.hpp"
#include "displayLines.hpp"
#include "lockValue.hpp"
#include "Vector.hpp"


//  Data type specific includes

#include "FileHydroOutput.h"  //  Hydro LIDAR


  /*! \mainpage pfmEdit

       <br><br>\section disclaimer Disclaimer

       This is a work of the US Government. In accordance with 17 USC 105, copyright
       protection is not available for any work of the US Government.

       Neither the United States Government nor any employees of the United States
       Government, makes any warranty, express or implied, without even the implied
       warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes
       any liability or responsibility for the accuracy, completeness, or usefulness
       of any information, apparatus, product, or process disclosed, or represents
       that its use would not infringe privately-owned rights. Reference herein to
       any specific commercial products, process, or service by trade name, trademark,
       manufacturer, or otherwise, does not necessarily constitute or imply its
       endorsement, recommendation, or favoring by the United States Government. The
       views and opinions of authors expressed herein do not necessarily state or
       reflect those of the United States Government, and shall not be used for
       advertising or product endorsement purposes.


       <br><br>\section intro Introduction

       The pfmEdit program is the 2D editor companion to pfmView which is what most people refer to as the
       Area-Based Editor (ABE) even though it is only the binned surface viewer portion of ABE.  Normally,
       due to ease of use and speed, the users will prefer to use the 3D point cloud editor,
       <a href="../../pfmEdit3D_Documentation/html/index.html"><b>pfmEdit3D</b></a>, although there are a
       very few functions that are available in pfmEdit that are not available in
       <a href="../../pfmEdit3D_Documentation/html/index.html"><b>pfmEdit3D</b></a>.


       <br><br>\section sec1 Searching the Doxygen Documentation

       The easiest way to find documentation for a particular C++ method is to go to the
       <a href="functions_func.html"><b>Data Structures>Data Fields>Functions</b></a> tab.  If, for example,
       you are looking for <b><i>editFeature</i></b> you would then click on the <b><i>e</i></b> tab,
       find <b><i>editFeature</i></b>, and then click on the associated structure (in this case editFeature).
       The entire group of public methods for a particular class are documented via their class documentation
       not via their include files.  You can see documentation for them in the main
       <a href="annotated.html"><b>Data Structures</b></a> tab or by selecting the class in the Data Structures
       section of the .hpp file.  For example, editFeature can be accessed via the overall
       <a href="annotated.html"><b>Data Structures</b></a> tab or from the class defined in the Data Structures
       section of the editFeature.hpp file.


       <br><br>\section nvmap The nvMap Class

       The pfmEdit program is built around the nvMap class from the nvutility library (libnvutility).  Due to
       this there are many calls to nvMap methods throughout the program.  Documentation for the nvMap class
       can be found in the <a href="../../NVUTILITY_API_Documentation/html/index.html"><b>nvutility documentation</b></a>.
       Specifically, look for the nvMap class in the <a href="../../NVUTILITY_API_Documentation/html/annotated.html"><b>Data
       Structures</b></a> tab.<br><br>


       <br><br>\section structures The OPTIONS and MISC Structures

       The pfmEdit program uses two huge structures; OPTIONS and MISC.  The OPTIONS structure contains variables
       that need to be saved when the program exits and restored when it is started.  This is done using QSettings
       in env_in_out.cpp.  The MISC structure contains a bunch of variables that are used throughout the program
       but don't need to be saved.  Both of these structures are initialized in set_defaults.cpp.  I'm sure that
       it's not all that cool to use giant structures to pass things around but at least I'm passing them by
       reference instead of value so it doesn't slow the program down (try passing a big structure by value in a
       tight loop and you'll see what I mean).  The main reason that I'm using these two structures is that
       I didn't want to have to have function calls with fifty million arguments.  Also, this program was ported
       from C/Motif to C++/Qt and that was how I handled it in C.  If you'd like to rewrite it to make it more
       elegant, be my guest.


       <br><br>\section shared Shared Memory in PFM ABE

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
       memory and then dies (or gets stuck waiting for something), all the other programs will be locked up.  When you
       look through the ABE code you'll see that we very rarely lock shared memory, and then only for very short periods
       of time.  This is by design.<br><br>


  */

class pfmEdit:public QMainWindow
{
  Q_OBJECT 


 public:

  pfmEdit (int *argc = 0, char **argv = 0, QWidget *parent = 0);
  ~pfmEdit ();

  void redrawMap (NV_BOOL clearmap);
  void editFeatureNum (NV_INT32 feature_number);
  void enableClearHighlight (NV_BOOL on_off);


 protected:

  OPTIONS         options;

  MISC            misc;

  Prefs           *prefs_dialog;

  editFeature     *editFeature_dialog;

  manageLayers    *manageLayers_dialog;

  displayLines    *displayLines_dialog;

  QMenu           *layerMenu;

  QPainterPath    marker, arrow;

  QTimer          *trackCursor, *blinkTimer;

  ancillaryProg   *ancillaryProgram[NUMPROGS];

  hotkeyPolygon   *hkp;

  externalFilter  *exf;

  QLabel          *latLabel, *lonLabel, *zLabel, *recLabel, *subLabel, *filLabel, *pfmLabel, *linLabel, *typLabel, *miscLabel,
                  *hvLabel, *attrLabel[NUM_ATTR], *sizeLabel, *meterLabel;

  QScrollBar      *sliceBar, *sizeBar;

  QStatusBar      *statusBar[4];

  nvMap           *map;

  QProgressDialog *unload_prog;

  QProcess        *unloadProc, *browserProc;

  NVMAP_DEF       mapdef;

  QGroupBox       *colorScaleBox;

  QToolBar        *toolBar[NUM_TOOLBARS];

  QButtonGroup    *exitGrp;

  QAction         *bHelp, *noUserFlag, *userFlag[5], *popup[NUMPOPUPS], *popupHelp, *saveExitAction, *maskExitAction;

  QToolButton     *bExitSave, *bExitMask, *bExitNoSave, *bReset, *bPlanView, *bEastView, *bSouthView, *bSelectView, *bRedraw, *bAutoRedraw,
                  *bDisplayFeature, *bDisplayChildren, *bDisplayFeatureInfo, *bDisplayFeaturePoly, *bVerifyFeatures, *bDisplayReference, 
                  *bDisplayManInvalid, *bDisplayFltInvalid, *bDisplayNull, *bDisplayAll, *bDisplayMultiple, *bUndisplaySingle, *bUser,
                  *bHighlightPoly, *bClearPoly, *bInvertHighlight, *bClearHighlight, *bContour, *bGrid, *bUnload, *bPrefs, *bStop, *bDeletePoint,
                  *bDeleteRect, *bDeletePoly, *bKeepPoly, *bRestoreRect, *bRestorePoly, *bUndo, *bReferencePoly, *bUnreferencePoly, *bAddFeature,
                  *bEditFeature, *bDeleteFeature, *bMoveFeature, *bHotkeyPoly, *bShiftLine, *bMeasure, *bResetSingle, *bResetShift, *bGeotiff, *bAttr,
                  *bFlag, *bHide, *bShow, *bFilter, *bAttrFilter, *bFilterRectMask, *bFilterPolyMask, *bClearMasks, *bMaskInsideRect, *bMaskOutsideRect,
                  *bMaskInsidePoly, *bMaskOutsidePoly, *bMaskReset, *bAttributeViewer, *bLidarMonitor, *bRMSMonitor, *bHOFWaveFilter, *bChartsPic,
                  *bWaveformMonitor, *bCZMILwaveMonitor, *bWaveWaterfallAPD, *bWaveWaterfallPMT, *bHawkeyeMonitor, *bGSFMonitor, *bDistanceThresh;

  QButtonGroup    *progGrp;

  QActionGroup    *attributeGrp, *flagGrp, *hideGrp, *showGrp;

  QAction         *attr[NUM_ATTR + PRE_ATTR], *flag[5 + PRE_USER], *hideAct[5 + PFM_DATA_TYPES], *showAct[5 + PFM_DATA_TYPES], *displayFeature[4];

  QPushButton     *filterAccept, *filterReject, *attrFilterAccept, *attrFilterReject, *hofWaveFilterAccept, *hofWaveFilterReject;

  QIcon           attrIcon[NUM_ATTR + PRE_ATTR], flagIcon[5 + PRE_USER], displayFeatureIcon[4];

  QCursor         editRectCursor, editPolyCursor, editFeatureCursor, deleteFeatureCursor, addFeatureCursor, moveFeatureCursor,
                  deleteRectCursor, deletePolyCursor, restoreRectCursor, restorePolyCursor, hotkeyPolyCursor, maskCursor,
                  measureCursor, keepPolyCursor, unreferencePolyCursor, referencePolyCursor, filterMaskRectCursor, filterMaskPolyCursor,
                  highlightPolyCursor, clearPolyCursor;

  QTabWidget      *notebook;

  QGroupBox       *filterBox, *attrFilterBox, *hofFilterBox;

  QLineEdit       *stdValue, *minValue, *maxValue, *srValue, *swValue, *rtValue, *pmtValue, *apdValue;

  QSlider         *stdSlider, *minSlider, *maxSlider, *srSlider, *swSlider, *rtSlider, *pmtSlider, *apdSlider;

  QMenu           *popupMenu, *userMenu;

  QPalette        linPalette, zPalette, slicePalette, sizePalette;

  NV_FLOAT32      attr_filter_scale;

  NV_BOOL         polygon_timer, finishing, got_geotiff, need_redraw, PFMWDB_attr_count, unload_done, double_click, filter_active, attr_filter_active,
                  hof_filter_active, force_auto_unload, popup_active;

  NV_INT32        popup_prog[NUMPOPUPS], unload_pfm, menu_cursor_x, menu_cursor_y, mv_tracker, mv_arrow, rb_rectangle, rb_polygon, rb_measure,
                  mv_measure, multi_marker[MAX_STACK_POINTS], move_feature, shift_point, lock_point, active_window_id, moveWindow, prev_icon_size,
                  filterMessage_slider_count, redraw_count, local_num_lines, local_line_num[4096], rb_dist;

  NV_F64_COORD2	  distLineStart, distLineEnd;

  NV_F64_COORD3   line_anchor;

  NV_FLOAT64      prev_poly_lat, prev_poly_lon, menu_cursor_lat, menu_cursor_lon;

  NV_F64_XYMBR    orig_bounds;

  QString         hotkey_message;


  void closeEvent (QCloseEvent *event);
  void setFunctionCursor (NV_INT32 function);
  void setMainButtons (NV_BOOL enable);
  void autoUnload ();
  void geo_label (NV_FLOAT64 lat, NV_FLOAT64 lon, NV_BOOL blank);
  void z_label (NV_INT32 val);

  void discardMovableObjects ();

  void midMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void leftMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void rightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void runPopupProg (NV_INT32 prog);
  void clean_exit (NV_INT32 ret);

  void performAVActionHighlight ();
  void performAVActionDelete ();
  void performAVActionColorLink ();
  void checkForAVAction ();



protected slots:

  void slotPopupMenu (QAction *ac);
  void slotPopupHelp ();

  void slotMouseDoubleClick (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotMouseRelease (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotWheel (QWheelEvent *e);
  void slotMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotResize (QResizeEvent *e);
  void slotPreRedraw (NVMAP_DEF mapdef);

  void slotTrackCursor ();
  void slotBlinkTimer ();

  void slotPostRedraw (NVMAP_DEF mapdef);

  void slotExitSave ();
  void slotExitMask ();
  void slotExitNoSave ();
  void slotExit (int id);

  void slotMaskReset ();
  void slotReset ();

  void slotView (int id);

  void slotSliceTriggered (int action);
  void slotSizeTriggered (int action);

  void slotRedraw ();
  void slotAutoRedraw ();
  void slotFeatureMenu (QAction *action);
  void slotDisplayChildren ();
  void slotDisplayFeatureInfo ();
  void slotDisplayFeaturePoly ();
  void slotVerifyFeatures ();
  void slotDisplayReference ();
  void slotDisplayManInvalid ();
  void slotDisplayFltInvalid ();
  void slotDisplayNull ();

  void slotDisplayAll ();
  void slotUndisplaySingle ();
  void slotDisplayMultiple ();

  void slotAttrMenu (QAction *action);
  void slotFlagMenu (QAction *action);

  void slotAttributeViewer (bool checked);
  void slotProg (int id);

  void slotHideMenuShow ();
  void slotHideMenu (QAction *action);
  void slotShowMenuShow ();
  void slotShowMenu (QAction *action);

  void slotInvertHighlight ();
  void slotClearHighlight ();

  void slotRedrawMap (NV_BOOL clearmap);

  void slotHotkeyPolygonDone ();
  void slotExternalFilterDone ();

  void slotDisplayLinesDataChanged ();

  void slotLockValueDone (NV_BOOL accepted);
  void slotMinScalePressed (QMouseEvent *e);
  void slotMaxScalePressed (QMouseEvent *e);

  void slotMode (int id);

  void slotResetShift ();

  void slotGeotiff ();

  void slotUnload ();

  void slotStop ();

  void slotContour ();
  void slotGrid ();

  void slotLayers ();
  void slotManageLayersDataChanged ();

  void slotBrowserError (QProcess::ProcessError error);

  void slotUnloadError (QProcess::ProcessError error);
  void slotUnloadDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotUnloadReadyReadStandardError ();

  void slotEditFeatureDataChanged ();

  void extendedHelp ();
  void slotHotkeyHelp ();
  void slotToolbarHelp ();

  void slotFilter ();
  void slotClearFilterMasks ();
  void slotFilterStdChanged (int value);
  void slotFilterStdReturn ();
  void slotFilterAccept ();
  void slotFilterReject ();

  void slotAttrFilter ();
  void slotAttrFilterMinChanged (int value);
  void slotAttrFilterMinReturn ();
  void slotAttrFilterMaxChanged (int value);
  void slotAttrFilterMaxReturn ();

  void slotHOFWaveFilter ();
  void slotHOFWaveFilterSrChanged (int value);
  void slotHOFWaveFilterSrReturn ();
  void slotHOFWaveFilterSwChanged (int value);
  void slotHOFWaveFilterSwReturn ();
  void slotHOFWaveFilterRtChanged (int value);
  void slotHOFWaveFilterRtReturn ();
  void slotHOFWaveFilterPMTChanged (int value);
  void slotHOFWaveFilterPMTReturn ();
  void slotHOFWaveFilterAPDChanged (int value);
  void slotHOFWaveFilterAPDReturn ();

  void slotUndo ();

  void slotPrefs ();
  void slotPrefDataChanged ();
  void slotPrefHotKeyChanged (NV_INT32 i);

  void slotAVInterfaceChanged (int sizeOfBox);
  void slotAVDistanceThresh(bool pressed);

  void about ();
  void slotAcknowledgements ();
  void aboutQt ();


 private:

  void keyPressEvent (QKeyEvent *e);
  void DrawMultiCursors (void);
  void runProg (int id);
  NV_BOOL killProg (int id);

};

#endif
