/*  geoSwath3D class definitions.  */

/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


#ifndef PFM_EDIT_H
#define PFM_EDIT_H

#include "geoSwath3DDef.hpp"
#include "prefs.hpp"
#include "hotkeyHelp.hpp"
#include "version.hpp"

#include "ancillaryProg.hpp"
#include "hotkeyPolygon.hpp"
#include "io_data.hpp"


void get_buffer (POINT_DATA *data, MISC *misc, NV_INT32 record);


class geoSwath3D:public QMainWindow
{
  Q_OBJECT 


 public:

  geoSwath3D (int *argc = 0, char **argv = 0, QWidget *parent = 0);
  ~geoSwath3D ();

  void redrawMap (NV_BOOL reset_orientation);


 protected:

  POINT_DATA      data;

  OPTIONS         options;

  MISC            misc;

  NV_FLOAT32      *save_z;

  Prefs           *prefs_dialog;

  NV_I32_COORD2   marker[16];

  QPainterPath    tracker_2D;

  QTimer          *trackCursor, *blinkTimer;

  ancillaryProg   *ancillaryProgram[NUMPROGS];

  hotkeyPolygon   *hkp;

  QLabel          *latLabel, *lonLabel, *zLabel, *recLabel, *subLabel, *numLabel, *exagLabel, *countLabel, *miscLabel, *transLabel, *sizeLabel, *meterLabel;

  QScrollBar      *fileBar, *sliceBar, *transBar, *sizeBar, *exagBar;

  QStatusBar      *statusBar[3];

  nvMapGL         *map;

  nvMap           *trackMap;

  NVMAPGL_DEF     mapdef;

  NVMAP_DEF       track_mapdef;

  QToolBar        *toolBar[NUM_TOOLBARS];

  QButtonGroup    *exitGrp, *progGrp;

  QAction         *bHelp, *popup[NUMPOPUPS], *popupHelp, *flag[5];

  QToolButton     *bOpen, *bExit, *bReset, *bMeasure, *bDisplayInvalid, *bPrefs, *bLink, *bStop, *bDeletePoint, *bDeleteRect, *bDeletePoly,
                  *bKeepPoly, *bRestoreRect, *bRestorePoly, *bUndo, *bHotkeyPoly, *bFlag, *bHighlightPoly, *bClearPoly, *bClearHighlight, *bFilter,
                  *bFilterRectMask, *bFilterPolyMask, *bClearMasks, *bMaskInsideRect, *bMaskOutsideRect, *bMaskInsidePoly, *bMaskOutsidePoly,
                  *bMaskReset, *bLidarMonitor, *bRMSMonitor, *bChartsPic, *bWaveformMonitor, *bWaveMonitor, *bWaveWaterfallAPD, *bWaveWaterfallPMT,
                  *bGSFMonitor;

  QActionGroup    *flagGrp;

  QIcon           flagIcon[5];

  QCursor         rotateCursor, zoomCursor, maskCursor, deleteRectCursor, deletePolyCursor, restoreRectCursor, restorePolyCursor, hotkeyPolyCursor,
                  measureCursor, keepPolyCursor, filterMaskRectCursor, filterMaskPolyCursor, highlightPolyCursor, clearPolyCursor;

  QTabWidget      *notebook;

  QGroupBox       *filterBox;

  QLabel          *stdValue;

  QSlider         *stdSlider;

  QMenu           *popupMenu;

  QPalette        zPalette, filePalette, slicePalette, transPalette, sizePalette, exagPalette;

  QMessageBox     *filt;

  NV_BOOL         polygon_flip, finishing, slicing, fileBarEnabled, first_screen, double_click, redraw2D, filter_active, popup_active, file_opened;

  NV_INT32        popup_prog[NUMPOPUPS], filterMessage_slider_count, menu_cursor_x, menu_cursor_y, record_num, rb_rectangle, rb_polygon, rb_measure,
                  mv_measure, mv_measure_anchor, multi_marker[MAX_STACK_POINTS], prev_poly_x, prev_poly_y, active_window_id, abe_register_group,
                  lock_point, prev_icon_size, start_ctrl_x, start_ctrl_y, rotate_index, mv_tracker, mv_2D_tracker;

  NV_FLOAT64      menu_cursor_lat, menu_cursor_lon, menu_cursor_z, range_x, range_y, range_z;

  NV_F64_COORD3   line_anchor;

  NV_F64_XYMBC    bounds;

  NV_FLOAT32      prev_z_factor, prev_z_offset;

  QString         hotkey_message;


  void open_file ();
  void closeEvent (QCloseEvent *event);
  void setFunctionCursor (NV_INT32 function);
  void setMainButtons (NV_BOOL enable);
  void geo_z_label (NV_FLOAT64 lat, NV_FLOAT64 lon, NV_FLOAT64 z);

  void discardMovableObjects ();

  void midMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void leftMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void rightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void runPopupProg (NV_INT32 prog);
  void clean_exit (NV_INT32 ret);


protected slots:

  void slotOpen ();

  void slotPopupMenu (QAction *ac);
  void slotPopupHelp ();

  void slotMouseDoubleClick (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void slotMouseRelease (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void slotWheel (QWheelEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void slotMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z, NVMAPGL_DEF l_mapdef);
  void slotResize (QResizeEvent *e);

  void slotTrackMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotTrackMouseRelease (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotTrackMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotTrackResize (QResizeEvent *e);
  void slotTrackPreRedraw (NVMAP_DEF mapdef);
  void slotTrackPostRedraw (NVMAP_DEF mapdef);

  void slotLink ();
  void slotRegisterABEKeySet (NV_INT32 key);
  void slotTrackCursor ();
  void slotBlinkTimer ();

  void slotQuit ();

  void slotMaskReset ();
  void slotReset ();

  void slotFileValueChanged (int value);

  void slotSliceTriggered (int action);
  void slotSizeTriggered (int action);
  void slotTransTriggered (int action);

  void slotExagTriggered (int action);
  void slotExagReleased ();
  void slotExaggerationChanged (NV_FLOAT32 exaggeration, NV_FLOAT32 apparent_exaggeration);

  void slotDisplayInvalid ();

  void slotClearHighlight ();

  void slotFlagMenu (QAction *action);

  void slotProg (int id);

  void slotRedrawMap ();

  void slotHotkeyPolygonDone ();

  void slotMode (int id);

  void slotStop ();

  void slotHotkeyHelp ();
  void slotToolbarHelp ();

  void slotFilter ();
  void slotClearFilterMasks ();
  void slotFilterStdChanged (int value);
  void slotFilterAccept ();
  void slotFilterReject ();

  void slotUndo ();

  void slotPrefs ();
  void slotPrefDataChanged ();
  void slotPrefHotKeyChanged (NV_INT32 i);

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
