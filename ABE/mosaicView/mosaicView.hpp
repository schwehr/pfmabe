/*  mosaicView class definitions.  */

/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


#ifndef MOSAICVIEW_H
#define MOSAICVIEW_H


#include "mosaicViewDef.hpp"
#include "version.hpp"
#include "prefs.hpp"
#include "editFeature.hpp"



class mosaicView:public QMainWindow
{
  Q_OBJECT 


public:

  mosaicView (NV_INT32 argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~mosaicView ();

  void redrawMap ();
  void editFeatureNum (NV_INT32 feature_number);


protected:

  OPTIONS         options;

  MISC            misc;

  ABE_SHARE       abe_share;

  Prefs           *prefs_dialog;

  editFeature     *editFeature_dialog;

  long            zone;

  NV_BOOL         double_click, need_redraw, popup_active;

  QPainterPath    marker;

  QTimer          *trackCursor;

  QCheckBox       *sMessage;

  QMouseEvent     *menu_mouse_event;

  nvMap           *map;

  NV_INT32        feature_circle_pixels, menu_cursor_x, menu_cursor_y, redraw_count;

  QCursor         zoomCursor, editFeatureCursor, deleteFeatureCursor, addFeatureCursor, pencilCursor;

  NV_U_INT32      ac[3];

  NVMAP_DEF       mapdef;

  QMenu           *popupMenu;

  QAction         *bHelp, *popup0, *popup1, *popup2, *popup3, *popup4;

  QToolButton     *bQuit, *bZoomOut, *bZoomIn, *bCoast, *bMask, *bPrefs, *bDisplayFeature, *bDisplayChildren, *bDisplayFeatureInfo, 
                  *bDisplayFeaturePoly, *bAddFeature, *bDeleteFeature, *bEditFeature, *bLink, *bUnlink, *bOpen;

  NV_INT32        mv_marker, mv_tracker, mv_circle, rb_rectangle, rb_polygon;

  NV_INT32        active_window_id;

  NV_BOOL         shelled;

  NV_FLOAT64      prev_poly_lat, prev_poly_lon, menu_cursor_lon, menu_cursor_lat;

  NV_CHAR         mosaic_file[512], pf[10];

  NV_INT32        new_feature;


  void setMainButtons (NV_BOOL enabled);
  void discardMovableObjects ();
  void setFunctionCursor (NV_INT32 function);


protected slots:

  void slotMouseDoubleClick (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotPreliminaryMousePress (QMouseEvent *e);
  void slotMouseRelease (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotResize (QResizeEvent *e);
  void slotPreRedraw (NVMAP_DEF mapdef);
  void slotPostRedraw (NVMAP_DEF mapdef);

  void slotTrackCursor ();

  void midMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void leftMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void rightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 lon, NV_FLOAT64 lat);

  void slotPopupMenu0 ();
  void slotPopupMenu1 ();
  void slotPopupMenu2 ();
  void slotPopupMenu3 ();
  void slotPopupHelp ();

  void slotEditMode (int id);

  void slotZoomOut ();

  void slotDisplayFeature ();
  void slotDisplayChildren ();
  void slotDisplayFeatureInfo ();
  void slotDisplayFeaturePoly ();
  void slotEditFeatureDataChanged ();
  void slotEditFeatureDefinePolygon ();

  void slotCoast ();

  void slotMask ();

  void slotPrefs ();
  void slotPrefDataChanged ();

  void slotQuit ();

  void about ();
  void slotAcknowledgements ();
  void aboutQt ();


 private:

  void keyPressEvent (QKeyEvent *e);


};

#endif
