/*  lidarMonitor class definitions.  */

#ifndef __LIDARMONITOR_H__
#define __LIDARMONITOR_H__

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <getopt.h>
#include <cmath>

#include "nvutility.h"
#include "pfm.h"
#include "fixpos.h"
#include "pfm_extras.h"
#include "nvmap.hpp"
#include "FileHydroOutput.h"
#include "FileTopoOutput.h"
#include "wlf.h"
#include "hawkeye.h"
#include "czmil.h"
#include "ABE.h"
#include "version.hpp"

#ifdef NVWIN3X
    #include "windows_getuid.h"
#endif

#include <QtCore>
#include <QtGui>


class lidarMonitor:public QMainWindow
{
  Q_OBJECT 


public:

  lidarMonitor (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~lidarMonitor ();


protected:

  NV_INT32            key, pfm_handle[MAX_ABE_PFMS], num_pfms;

  NV_U_INT32          kill_switch;

  QSharedMemory       *abeShare;

  ABE_SHARE           *abe_share;

  PFM_OPEN_ARGS       open_args[MAX_ABE_PFMS];        

  QTextEdit           *listBox;

  NV_INT32            width, height, window_x, window_y;

  QPushButton         *bRestoreDefaults; 

  NV_BOOL             force_redraw, lock_track;

  NV_U_INT32          ac[1];

  NV_INT32            pos_format;

  QButtonGroup        *bGrp;

  QDialog             *prefsD;

  QToolButton         *bQuit, *bPrefs;


  void envin ();
  void envout ();

  void closeEvent (QCloseEvent *);


protected slots:

  void slotResize (QResizeEvent *e);
  
  void trackCursor ();

  void slotHelp ();

  void slotQuit ();

  void slotPrefs ();
  void slotPosClicked (int id);
  void slotClosePrefs ();

  void slotRestoreDefaults ();
  
  void about ();
  void aboutQt ();


 private:

};

#endif
