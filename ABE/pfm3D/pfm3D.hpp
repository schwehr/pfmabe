/*  pfm3D class definitions.  */

/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


#ifndef _PFM3D_H_
#define _PFM3D_H_

#include <cmath>
#include <cerrno>
#include <getopt.h>

#include "pfm3DDef.hpp"
#include "prefs.hpp"
#include "hotkeyHelp.hpp"
#include "version.hpp"




using namespace std;  // Windoze bullshit - God forbid they should follow a standard


class pfm3D:public QMainWindow
{
  Q_OBJECT 


public:

  pfm3D (int *argc = 0, char **argv = 0, QWidget *parent = 0);
  ~pfm3D ();

  void initializeMaps (NV_BOOL reset);
  void clean_exit (NV_INT32 ret);


protected:

  OPTIONS         options;

  MISC            misc;

  ABE_SHARE       abe_share;

  Prefs           *prefs_dialog;

  long            zone;

  NV_INT32        start_ctrl_x, start_ctrl_y;

  NV_I32_COORD2   marker[16];

  QTimer          *trackCursor, *zoomTimer;

  QCheckBox       *sMessage;

  QMouseEvent     *menu_mouse_event;

  nvMapGL         *map;

  NV_INT32        menu_cursor_x, menu_cursor_y, zoom_accum;

  QCursor         rotateCursor, zoomCursor, editRectCursor, editPolyCursor;

  NV_U_INT32      ac[3];

  NVMAPGL_DEF     mapdef;

  NV_INT32        prev_poly_x, prev_poly_y, pfmEditMod;

  QProcess        *browserProc;

  QMenu           *popupMenu;

  QAction         *bHelp, *popup0, *popup1, *popup2, *popup3, *displayFeature[4];

  QToolButton     *bQuit, *bGeoOpen, *bPrefs, *bReset, *bEditRect, *bEditPoly, *bEditRect3D, *bEditPoly3D, *bDisplayFeature,
                  *bDisplayChildren, *bDisplayFeatureInfo, *bGeotiff;

  QIcon           displayFeatureIcon[4];

  QScrollBar      *exagBar;

  QPalette        exagPalette;

  NV_INT32        mv_marker, mv_tracker, rb_rectangle, rb_polygon;

  NV_INT32        active_window_id, abe_register_group;

  NV_FLOAT64      menu_cursor_lon, menu_cursor_lat, menu_cursor_z;

  NV_BOOL         got_geotiff, popup_active, double_click, zoom_wait, polygon_flip, pfm_edit_active;

  NV_BOOL         pfmEdit3D_opened, cleared, delayClear;

  NV_FLOAT64      mid_lat, mid_lon, mid_z;



  void setMainButtons (NV_BOOL enabled);
  void discardMovableObjects ();
  void setFunctionCursor (NV_INT32 function);
  void leftMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void midMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void rightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void focusInEvent (QFocusEvent *e);


protected slots:

  void slotMouseDoubleClick (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void slotPreliminaryMousePress (QMouseEvent *e);
  void slotMouseRelease (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void slotWheel (QWheelEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z);
  void slotMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat, NV_FLOAT64 z, NVMAPGL_DEF l_mapdef);
  void slotResize (QResizeEvent *e);

  void slotExagTriggered (int action);
  void slotExagReleased ();
  void slotExaggerationChanged (NV_FLOAT32 exaggeration, NV_FLOAT32 apparent_exaggeration);

  void slotClose (QCloseEvent *e);

  void slotTrackCursor ();
  void slotZoomTimer ();
  void slotZoomWait ();

  void slotPopupMenu0 ();
  void slotPopupMenu1 ();
  void slotPopupMenu2 ();
  void slotPopupHelp ();

  void slotGeotiff ();
  void slotReset ();
  void slotFeatureMenu (QAction *action);
  void slotDisplayChildren ();
  void slotDisplayFeatureInfo ();
  void slotEditMode (int id);

  void slotOpenGeotiff ();
  void slotPrefs ();
  void slotPrefDataChanged ();

  void slotBrowserError (QProcess::ProcessError error);
  void extendedHelp ();

  void slotHotkeyHelp ();
  void about ();
  void slotAcknowledgements ();
  void aboutQt ();

  void slotQuit ();


private:

  void keyPressEvent (QKeyEvent *e);

};

#endif
