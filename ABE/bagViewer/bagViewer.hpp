
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



/*  bagViewer class definitions.  */

/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


#ifndef _BAGVIEWER_H_
#define _BAGVIEWER_H_

#include <cmath>
#include <cerrno>
#include <getopt.h>

#include "bagViewerDef.hpp"
#include "prefs.hpp"
#include "hotkeyHelp.hpp"
#include "version.hpp"




using namespace std;  // Windoze bullshit - God forbid they should follow a standard


class bagViewer:public QMainWindow
{
  Q_OBJECT 


public:

  bagViewer (int *argc = 0, char **argv = 0, QWidget *parent = 0);
  ~bagViewer ();

  void initializeMaps (NV_BOOL reset);


protected:

  OPTIONS         options;

  MISC            misc;

  NV_FLOAT64      x_bin_size, y_bin_size, menu_cursor_x, menu_cursor_y, menu_cursor_z, map_center_x, map_center_y, map_center_z, mid_y, mid_x, mid_z,
                  sep_x_spacing, sep_y_spacing, sep_sw_corner_x, sep_sw_corner_y;

  NV_F64_XYMBC    mbc;

  NV_CHAR         bag_path[512];

  bagHandle       bagHnd;

  bagHandle_opt   bagHndOpt;

  bagError        bagErr;

  NV_BOOL         sep, got_geotiff, popup_active, double_click, zoom_wait, ready, utm;

  NV_INT32        num_opt_datasets, opt_dataset_entities[10], menu_cursor_mouse_x, menu_cursor_mouse_y, zoom_accum, mv_marker, start_ctrl_x, start_ctrl_y, zone;

  NV_U_INT32      num_correctors, num_sep_rows, num_sep_cols;

  bagVerticalCorrector **vdata;

  projPJ          pj_utm, pj_latlon;

  Prefs           *prefs_dialog;

  NV_I32_COORD2   marker[16];

  QButtonGroup    *surfaceGrp;

  QTimer          *zoomTimer;

  QMouseEvent     *menu_mouse_event;

  nvMapGL         *map;

  QCursor         rotateCursor, zoomCursor;

  NVMAPGL_DEF     mapdef;

  QProcess        *browserProc;

  QMenu           *popupMenu;

  QAction         *bHelp, *popup0, *popup1, *popup2, *popup3, *color[2];

  QToolButton     *bQuit, *bOpen, *bGeoOpen, *bPrefs, *bReset, *bBag, *bGeotiff, *bColor, *bDisplayFeature, *bDisplayFeatureInfo;

  QIcon           colorIcon[2];

  QScrollBar      *exagBar;

  QPalette        exagPalette;



  void setMainButtons (NV_BOOL enabled);
  void discardMovableObjects ();
  void setFunctionCursor (NV_INT32 function);
  void leftMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z);
  void midMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z);
  void rightMouse (NV_INT32 mouse_x, NV_INT32 mouse_y, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z);
  void focusInEvent (QFocusEvent *e);



protected slots:

  void slotMouseDoubleClick (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z);
  void slotMousePress (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z);
  void slotPreliminaryMousePress (QMouseEvent *e);
  void slotMouseRelease (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z);
  void slotWheel (QWheelEvent *e, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z);
  void slotMouseMove (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z, NVMAPGL_DEF l_mapdef);
  void slotResize (QResizeEvent *e);

  void slotExagTriggered (int action);
  void slotExagReleased ();
  void slotExaggerationChanged (NV_FLOAT32 exaggeration, NV_FLOAT32 apparent_exaggeration);

  void slotClose (QCloseEvent *e);

  void slotZoomTimer ();
  void slotZoomWait ();

  void slotPopupMenu0 ();
  void slotPopupMenu1 ();
  void slotPopupMenu2 ();
  void slotPopupHelp ();

  void slotColorMenu (QAction *action);

  void slotDisplayFeature ();
  void slotDisplayFeatureInfo ();

  void slotGeotiff ();
  void slotReset ();

  void slotSurface (int id);

  void slotOpen ();
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
