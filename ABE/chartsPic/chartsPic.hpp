/*  ChartPic class definitions.  */

#ifndef CHARTSPIC_H
#define CHARTSPIC_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <cerrno>
#include <cmath>

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "pfm_extras.h"
#include "FileHydroOutput.h"
#include "FileTopoOutput.h"
#include "FilePOSOutput.h"
#include "FileImage.h"
#include "wlf.h"
#include "version.hpp"


#include <QtCore>
#include <QtGui>
#include <QSharedMemory>


#include "basename.h"


class chartsPic:public QMainWindow
{
  Q_OBJECT 


public:

  chartsPic (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~chartsPic ();

  void redrawPic ();

protected:

  NV_INT32        menu_cursor_x, menu_cursor_y, key, stickpin_count, active_window_id;

  NV_U_INT32      kill_switch;

  QSharedMemory   *abeShare;

  NV_I32_COORD2   stickpin[100];

  NV_INT64        image_time;

  NV_BOOL         rotation, startup_message;

  ABE_SHARE       *abe_share, l_share;

  NV_CHAR         prev_file[512], tmp_file[512], pf[10], pos_file[512];

  NV_FLOAT64      x_fov[3], y_fov[3], pitch_bias[3], roll_bias[3], a0, b0, rad_to_deg, heading, center_lat, center_lon;

  QPainterPath    marker;

  QMessageBox     *filError;

  QCheckBox       *sMessage;

  QString         save_file_name;

  QMouseEvent     *menu_mouse_event;

  nvPic           *pic;

  QScrollBar      *vBar, *hBar;

  NV_INT32        pos_format;

  QString         save_directory, snippet_file_name, parentName;

  NV_U_INT32      ac[6];

  NV_INT32        window_x, window_y, width, height;

  NV_CHAR         rot_file[512];

  NVPIC_DEF       picdef;

  QButtonGroup    *bGrp;

  QDialog         *prefsD;

  QToolButton     *bQuit, *bSave, *bScaled, *bPrefs;

  QToolButton     *bRotate;

  QCursor         stickpinCursor; 

  QPixmap         *stickPin;

  QTimer          *trackCursor;


  void picSave (NV_BOOL scaled);
  void envin ();
  void envout ();


protected slots:

  void slotMousePress (QMouseEvent *e, NV_INT32 x, NV_INT32 y);
  void slotMouseRelease (QMouseEvent *e, NV_INT32 x, NV_INT32 y);
  void slotMouseMove (QMouseEvent *e, NV_INT32 x, NV_INT32 y);
  void slotPostRedraw (NVPIC_DEF picdef);
  void slotKeyPress (QKeyEvent *e);
  void closeEvent (QCloseEvent *event);

  void slotTrackCursor ();

  void midMouse (QMouseEvent *e, NV_INT32 x, NV_INT32 y);
  void leftMouse (QMouseEvent *e, NV_INT32 x, NV_INT32 y);
  void rightMouse (QMouseEvent *e, NV_INT32 x, NV_INT32 y);

  void slotQuit ();
  void slotSave ();
  void slotScaled ();
  void slotBrighten ();
  void slotDarken ();

  void slotPrefs ();
  void slotPosClicked (int id);
  void slotClosePrefs ();
  void slotHelp ();

  void slotRotate ();

  void about ();
  void slotAcknowledgements ();
  void aboutQt ();


 private:
};

#endif
