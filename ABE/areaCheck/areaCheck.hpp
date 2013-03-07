/*  areaCheck class definitions.  */

/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


#ifndef AREA_CHECK_H
#define AREA_CHECK_H

#include <cmath>
#include <cerrno>
#include <getopt.h>

#include "areaCheckDef.hpp"
#include "prefs.hpp"
#include "displayFile.hpp"
#include "version.hpp"




using namespace std;


void overlayData (nvMap *map, MISC *misc, OPTIONS *options, NV_BOOL raster);


class areaCheck:public QMainWindow
{
  Q_OBJECT 


public:

  areaCheck (int *argc = 0, char **argv = 0, QWidget *parent = 0);
  ~areaCheck ();

  void redrawMap ();
  void initializeMaps ();


protected:

  Prefs           *prefs_dialog;

  displayFile     *displayFile_dialog[NUM_TYPES];

  QLabel          *geoLabel, *xyLabel, *chrtrLabel, *modeLabel, *miscLabel;

  nvMap           *map;

  OPTIONS         options;

  MISC            misc;

  NV_INT32        edit_area_width, edit_area_column, edit_area_height, edit_area_row;

  NV_INT32        prev_area, cur_area, prev_vertex, cur_vertex, prev_line[2], cur_line[2], prev_type, cur_type;

  NV_INT32        mv_polygon, mv_marker, rb_rectangle, rb_polygon, mv_tracker;

  NV_INT32        active_window_id, abe_register_group;

  QTimer          *trackCursor;

  NV_BOOL         area_moving, vertex_moving, line_moving, area_copying, no_save, input_file, empty_file, any_file, 
                  double_click, need_redraw;

  QStatusBar      *mapStatus;

  QMenu           *popupMenu;

  QAction         *bHelp, *popup0, *popup1, *popup2;

  QMouseEvent     *menu_mouse_event;

  QPalette        modePalette;

  NV_FLOAT64      menu_cursor_lat, menu_cursor_lon;

  OVERLAY         new_overlay;

  NVMAP_DEF       mapdef;

  QToolBar        *tools, *dataTypes;

  QToolButton     *bOpen, *bZoomOut, *bZoomIn, *bPrefs, *bCoast, *bMask, *bFile, *bRect, *bPoly, *bMoveArea, *bMoveVertex,
                  *bDeleteVertex, *bEditVertex, *bAddVertex, *bCopyArea, *bLink;

  QToolButton     *bStop, *bISS60Area, *bGenArea, *bACEArea, *bISS60Zone, *bGenZone, *bGeotiff, *bChrtr, *bBag, *bShape, *bTrack,
                  *bYXZ, *bLLZ, *bFlight, *bFeature, *bChildren, *bFeatureInfo, *bFeaturePoly;

  QButtonGroup    *exitGrp, *typeGrp;

  QToolButton     *bExitSave, *bExitNoSave;

  QDialog         *editVertexD;

  QLineEdit       *latEdit, *lonEdit;

  QString         fileName;

  NV_FLOAT64      tmpLat, tmpLon;

  QCursor         zoomCursor, addVertexCursor, moveVertexCursor, deleteVertexCursor, editVertexCursor, copyAreaCursor,
                  moveAreaCursor, editRectCursor, editPolyCursor;

  QPainterPath    marker;

  QBrush          marker_brush;


  void addOverlay (NV_INT32 k, QString filename);
  void setMainButtons (NV_BOOL enabled);
  void discardMovableObjects ();
  void closeEvent (QCloseEvent *event);
  void setFunctionCursor (NV_INT32 function);
  void write_generic (FILE *fp, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count);
  void write_ace (FILE *fp, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count);
  void write_ISS60_ARE (FILE *fp, NV_CHAR *fname, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count);
  void write_ISS60_zne (FILE *fp, NV_CHAR *fname, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count);
  void highlight_nearest_area (NV_FLOAT64 lat, NV_FLOAT64 lon);
  void highlight_nearest_vertex (NV_FLOAT64 lat, NV_FLOAT64 lon);
  void highlight_nearest_line (NV_FLOAT64 lat, NV_FLOAT64 lon);
  void clearLine ();
  void clearArea ();
  void clearVertex ();
  void output_file (NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count);


protected slots:

  void slotPopupMenu0 ();
  void slotPopupMenu1 ();
  void slotPopupHelp ();

  void slotMouseDoubleClick (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotMousePress (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotMouseRelease (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotMouseMove (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void slotResize (QResizeEvent *e);
  void slotPreRedraw (NVMAP_DEF mapdef);
  void slotMidRedraw (NVMAP_DEF mapdef);
  void slotPostRedraw (NVMAP_DEF mapdef);

  void midMouse (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void leftMouse (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);
  void rightMouse (QMouseEvent *e, NV_FLOAT64 lon, NV_FLOAT64 lat);

  void slotLink ();
  void slotRegisterABEKeySet (NV_INT32 key);
  void slotTrackCursor ();

  void slotExitSave ();
  void slotExitNoSave ();
  void slotExit (int id);
  void slotQuit (NV_INT32 status);

  void slotOpenFiles ();
  void slotZoomIn ();
  void slotZoomOut ();
  void slotCoast ();
  void slotMask ();
  void slotMoveArea ();
  void slotCopyArea ();
  void slotMoveVertex ();
  void slotDeleteVertex ();
  void slotEditVertex ();
  void slotAddVertex ();

  void slotApplyVertex ();
  void slotCancelVertex ();

  void slotStop ();
  void slotType (int k);
  void slotDisplayFileDataChanged ();
  void slotDisplayFileDialogClosed (int k);
  void slotChildren ();
  void slotFeatureInfo ();
  void slotFeaturePoly ();

  void about ();
  void slotAcknowledgements ();
  void aboutQt ();

  void slotPrefs ();
  void slotPrefDataChanged (NV_BOOL mapRedrawFlag);

  void slotDefineRectArea ();
  void slotDefinePolyArea ();

private:

  void keyPressEvent (QKeyEvent *e);

};

#endif
