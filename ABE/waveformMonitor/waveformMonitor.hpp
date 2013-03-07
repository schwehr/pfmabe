/*  waveformMonitor class definitions.  */

#ifndef WAVEFORMMONITOR_H
#define WAVEFORMMONITOR_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <getopt.h>
#include <cmath>

#include "FileHydroOutput.h"
#include "FileWave.h"

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "pfm_extras.h"
#include "ABE.h"

#include "wlf.h"

#include "version.hpp"


#include <QtCore>
#include <QtGui>


#define WAVE_X_SIZE       270
#define WAVE_Y_SIZE       600
#define SPOT_SIZE         2


typedef struct
{
  NV_INT32            min_x;
  NV_INT32            max_x;
  NV_INT32            min_y;
  NV_INT32            max_y;
  NV_INT32            range_x;
  NV_INT32            range_y;
  NV_INT32            length;
  NV_U_INT16          ac_zero_offset;
} BOUNDS;


typedef struct
{
  NV_INT32       length;
  NV_BOOL        wlf_record;
  NV_U_INT16     data[4][1000];       /* overkill */
} WAVE;


class waveformMonitor:public QMainWindow
{
  Q_OBJECT 


public:

  waveformMonitor (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~waveformMonitor ();


protected:

  NV_INT32        key;

  QSharedMemory   *abeShare;

  ABE_SHARE       *abe_share, l_share;

  NV_U_INT32      kill_switch;

  WAVE_HEADER_T   wave_header;
  BOUNDS          bounds[4];
  NV_INT32        width, height, window_x, window_y;

  NV_CHAR         wave_path[512], filename[512];
  NV_INT32        flightline, recnum;
  HYDRO_OUTPUT_T  hof_record;
  WLF_HEADER      wlf_header;
  WLF_RECORD      wlf_record;
  NV_BOOL         secondary, wave_line_mode, startup_message, lock_track;
  NV_FLOAT32      tide;

  WAVE_DATA_T     wave_data;
  NV_INT32        wave_read;
  WAVE            wave;

  QMessageBox     *filError;

  QCheckBox       *sMessage;

  QStatusBar      *statusBar[5];

  QLabel          *dateLabel, *timestampLabel, *recordLabel, *correctDepth, *tideLabel, 
                  *dataType, *abdcLabel, *fullConfLabel, *nameLabel, 
                  *reportedDepth, *secDepth, *waveHeight, *sabdcLabel, *botBinLabel,
                  *secBotBinLabel, *sfcBinLabel, *fullConf, *secFullConf, *bfomThresh, 
                  *secBfomThresh, *sigStrength, *secSigStrength;

  QColor          waveColor[4], acZeroColor[4], surfaceColor, primaryColor, secondaryColor, backgroundColor, ;

  QPalette        bWavePalette[4], bSurfacePalette, bPrimaryPalette, bSecondaryPalette, bBackgroundPalette;

  QPushButton     *bWaveColor[4], *bSurfaceColor, *bPrimaryColor, *bSecondaryColor, *bBackgroundColor, *bRestoreDefaults; 

  NV_BOOL         force_redraw, display[4];

  nvMap           *map;

  NVMAP_DEF       mapdef;

  NV_U_INT32      ac[7];

  QButtonGroup    *bGrp;
  QDialog         *prefsD;
  QToolButton     *bQuit, *bPrefs, *bMode;

  QString         pos_format, timestamp, record, first, level, water_level, sabdc,
                  pos_conf, flat, flon, sec_depth, slat, slon, ssig, s_full_conf,
                  f_full_conf, sfc_fom_a, sfc_fom_i, sfc_fom_r, fsig, bot_bin, haps_v,
                  sec_bot_bin, nadir_ang, scanner_az, mode, date_time, db_name, rep_depth,
                  wave_height, sfc_bin, sec_sfc_bin, calc_bfom, sec_calc_bfom, sug_stat, 
                  sus_stat, tide_stat, top, alt, warn, warn2, warn3, calc0, calc1, pri_run,
                  pri_slope, pri_backslope, sec_run, sec_slope, sec_backslope, parentName;

  NV_INT32        abdc;


  void envin ();
  void envout ();

  void leftMouse (NV_FLOAT64 x, NV_FLOAT64 y);
  void midMouse (NV_FLOAT64 x, NV_FLOAT64 y);
  void rightMouse (NV_FLOAT64 x, NV_FLOAT64 y);
  void scaleWave (NV_INT32 x, NV_INT32 y, NV_INT32 *new_x, NV_INT32 *new_y, NV_INT32 type, NVMAP_DEF l_mapdef);
  void drawX (NV_INT32 x, NV_INT32 y, NV_INT32 size, NV_INT32 width, QColor color);
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

  void slotMode (bool state);

  void slotPrefs ();
  void slotPosClicked (int id);
  void slotClosePrefs ();

  void slotWaveColor (int id);
  void slotSurfaceColor ();
  void slotPrimaryColor ();
  void slotSecondaryColor ();
  void slotBackgroundColor ();
  void slotRestoreDefaults ();

  void about ();
  void slotAcknowledgements ();
  void aboutQt ();


 private:
};

#endif
