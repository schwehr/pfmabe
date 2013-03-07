/*  hawkeyeMonitor class definitions.  */

#ifndef HAWKEYEMONITOR_H
#define HAWKEYEMONITOR_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <getopt.h>
#include <cmath>

#include "hawkeye.h"

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "pfm_extras.h"
#include "ABE.h"

#include "version.hpp"


#include <QtCore>
#include <QtGui>


typedef struct
{
  NV_INT32            min_x;
  NV_INT32            max_x;
  NV_INT32            min_y;
  NV_INT32            max_y;
  NV_INT32            range_x;
  NV_INT32            range_y;
  NV_INT32            pulse_length;
  NV_INT32            reflex_length;
} BOUNDS;


typedef struct
{
  NV_INT32       length;
  NV_U_BYTE      pulse[500];
  NV_U_BYTE      reflex[500];
} WAVE;


class hawkeyeMonitor:public QMainWindow
{
  Q_OBJECT 


public:

  hawkeyeMonitor (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~hawkeyeMonitor ();


protected:

  QSharedMemory   *abeShare;

  ABE_SHARE       *abe_share, l_share;

  NV_U_INT32      kill_switch, ac[7];

  BOUNDS          bounds[hydroChannelID_nrOfChannels];

  WAVE            wave_data[hydroChannelID_nrOfChannels], save_wave[hydroChannelID_nrOfChannels];

  HAWKEYE_RECORD  hawkeye_record, save_hawkeye;

  HAWKEYE_CONTENTS_HEADER *contents_header;

  HAWKEYE_RECORD_AVAILABILITY available, save_available;

  NV_INT32        width, height, window_x, window_y, recnum, return_type, key, pos_format, shift[hydroChannelID_nrOfChannels];

  NV_FLOAT64      scaleFactor[hydroChannelID_nrOfChannels], polarity[hydroChannelID_nrOfChannels], offset[hydroChannelID_nrOfChannels];

  NV_CHAR         filename[512], save_filename[512];

  NV_BOOL         force_redraw, lock_track, wave_read, displayIR[2], displayShallow[6], displayDeep[6];

  NVMAP_DEF       mapdef[3];

  QMessageBox     *filError;

  QColor          IRColor[2], shallowColor[6], deepColor[6], backgroundColor;

  QPalette        bIRDisplayPalette[2], bShallowDisplayPalette[6], bDeepDisplayPalette[6], bIRPalette[2], bShallowPalette[6], bDeepPalette[6], bBackgroundPalette;

  QLabel          *fileName;

  QTextEdit       *listBox;

  QCheckBox       *bIRDisplay[2], *bShallowDisplay[6], *bDeepDisplay[6];

  QPushButton     *bIRColor[2], *bShallowColor[6], *bDeepColor[6], *bBackgroundColor, *bRestoreDefaults; 

  nvMap           *map[3];

  QButtonGroup    *irGrp, *shallowGrp, *deepGrp, *bGrp, *bIRGrp, *bShallowGrp, *bDeepGrp;

  QDialog         *prefsD;

  QToolButton     *bQuit, *bPrefs;

  QString         parentName;


  void envin ();
  void envout ();

  void leftMouse (NV_FLOAT64 x, NV_FLOAT64 y);
  void midMouse (NV_FLOAT64 x, NV_FLOAT64 y);
  void rightMouse (NV_FLOAT64 x, NV_FLOAT64 y);
  void scaleWave (NV_INT32 x, NV_INT32 y, NV_INT32 *new_x, NV_INT32 *new_y, NV_INT32 type, NVMAP_DEF l_mapdef);
  void setFields ();


protected slots:

  void slotMousePress (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y);
  void slotMouseMove (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y);
  void slotResize (QResizeEvent *e);
  void closeEvent (QCloseEvent *event);
  void slotPlotWaves (NVMAP_DEF l_mapdef);

  void trackCursor ();

  void slotKeyPress (QKeyEvent *e);

  void slotHelp ();

  void slotQuit ();

  void slotPrefs ();
  void slotPosClicked (int id);
  void slotClosePrefs ();

  void slotIRDisplay (int id);
  void slotShallowDisplay (int id);
  void slotDeepDisplay (int id);

  void slotIRColor (int id);
  void slotShallowColor (int id);
  void slotDeepColor (int id);
  void slotBackgroundColor ();
  void slotRestoreDefaults ();

  void about ();
  void slotAcknowledgements ();
  void aboutQt ();


 private:
};

#endif
