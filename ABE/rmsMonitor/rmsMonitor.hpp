/*  rmsMonitor class definitions.  */

#ifndef __RMSMONITOR_H__
#define __RMSMONITOR_H__

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
#include "FileRMSOutput.h"
#include "ABE.h"
#include "version.hpp"

#ifdef NVWIN3X
    #include "windows_getuid.h"
#endif

#include <QtCore>
#include <QtGui>


class rmsMonitor:public QMainWindow
{
  Q_OBJECT 


public:

  rmsMonitor (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~rmsMonitor ();


protected:

  NV_INT32            key, pfm_handle[MAX_ABE_PFMS], num_pfms;

  NV_U_INT32          kill_switch;

  QSharedMemory       *abeShare;

  ABE_SHARE           *abe_share;

  PFM_OPEN_ARGS       open_args[MAX_ABE_PFMS];        

  QTextEdit           *listBox;

  QPushButton         *bStoplightLowColor, *bStoplightMidColor, *bStoplightHighColor, *bBackgroundColor, *bRestoreDefaults; 

  NV_BOOL             force_redraw, lock_track;

  NV_U_INT32          ac[1];

  NV_FLOAT64          stop_light[3][2];

  QColor              stop_color [3], bg_color;

  QPalette            listBoxPalette, stoplightLowPalette, stoplightMidPalette, stoplightHighPalette, backgroundPalette;

  QButtonGroup        *bGrp;

  QDialog             *prefsD;

  QToolButton         *bQuit, *bPrefs;


  NV_BOOL envin ();
  void envout ();

  void closeEvent (QCloseEvent *);


protected slots:

  void slotResize (QResizeEvent *e);
  
  void trackCursor ();

  void slotHelp ();

  void slotQuit ();

  void slotPrefs ();
  void setFields ();
  void slotClosePrefs ();

  void slotStoplightLowColor ();
  void slotStoplightMidColor ();
  void slotStoplightHighColor ();
  void slotBackgroundColor ();

  void slotRestoreDefaults ();
  
  void about ();
  void aboutQt ();


 private:

};

#endif
