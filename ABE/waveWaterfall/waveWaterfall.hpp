/*  waveWaterfall class definitions.  */

#ifndef WAVEWATERFALL_H
#define WAVEWATERFALL_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <getopt.h>
#include <cmath>

#include "FileHydroOutput.h"
#include "FileWave.h"

#include "wlf.h"

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "pfm_extras.h"

#include "version.hpp"


#include <QtCore>
#include <QtGui>


#define WAVE_X_SIZE       294
#define WAVE_Y_SIZE       769
#define SPOT_SIZE         2
#define WAVE_OFFSET       20
#define WAVE_ALPHA        64


typedef struct
{
  NV_INT32            min_x;
  NV_INT32            max_x;
  NV_INT32            min_y;
  NV_INT32            max_y;
  NV_INT32            range_x;
  NV_INT32            range_y;
  NV_INT32            length;
} BOUNDS;


typedef struct
{
  NV_INT32       length;
  NV_BOOL        wlf_record;
  NV_BOOL        lidar;
  NV_U_INT16     data[1000];       /* overkill */
} WAVE;


class waveWaterfall:public QMainWindow
{
  Q_OBJECT 


public:

  waveWaterfall (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~waveWaterfall ();


protected:

  NV_INT32        key, pfm_handle[MAX_ABE_PFMS], num_pfms;

  NV_U_INT32      kill_switch;

  QSharedMemory   *abeShare;

  ABE_SHARE       *abe_share, l_share;

  PFM_OPEN_ARGS   open_args[MAX_ABE_PFMS];

  BOUNDS          wave_bounds[MAX_STACK_POINTS];
  NV_INT32        wave_len[MAX_STACK_POINTS], width, height, apd_width, apd_height, apd_window_x, apd_window_y, pmt_width, pmt_height,
                  pmt_window_x, pmt_window_y, adjusted_width;

  NV_CHAR         path[MAX_STACK_POINTS][512], wave_path[MAX_STACK_POINTS][512], line_name[MAX_STACK_POINTS][128], filename[MAX_STACK_POINTS][512];
  NV_INT32        recnum[MAX_STACK_POINTS];
  HYDRO_OUTPUT_T  hof_record[MAX_STACK_POINTS];
  WLF_RECORD      wlf_record[MAX_STACK_POINTS];
  NV_BOOL         secondary[MAX_STACK_POINTS], wave_line_mode;

  NV_INT32        wave_type;
  WAVE_DATA_T     wave_data;
  WAVE            wave[MAX_STACK_POINTS];

  QMessageBox     *filError;

  QCheckBox       *sMessage;

  QStatusBar      *statusBar[MAX_STACK_POINTS];

  QLabel          *dateLabel[MAX_STACK_POINTS], *lineLabel[MAX_STACK_POINTS], *distLabel[MAX_STACK_POINTS];

  QColor          waveColor[MAX_STACK_POINTS], primaryColor, secondaryColor, backgroundColor, transWaveColor, transWave2Color, 
                  transPrimaryColor, transSecondaryColor;

  QPalette        bPrimaryPalette, bSecondaryPalette, bBackgroundPalette, dateLabelPalette[MAX_STACK_POINTS], lineLabelPalette[MAX_STACK_POINTS],
                  distLabelPalette[MAX_STACK_POINTS];

  QPushButton     *bWave2Color, *bPrimaryColor, *bSecondaryColor, *bBackgroundColor, *bRestoreDefaults; 

  NV_BOOL         force_redraw, lock_track;

  nvMap           *map;

  NVMAP_DEF       mapdef;

  QButtonGroup    *bGrp;
  QDialog         *prefsD;
  QToolButton     *bQuit, *bPrefs, *bMode;

  QString         pos_format, timestamp, record, first, date_time[MAX_STACK_POINTS];


  void envin ();
  void envout ();

  void leftMouse (NV_FLOAT64 x, NV_FLOAT64 y);
  void midMouse (NV_FLOAT64 x, NV_FLOAT64 y);
  void rightMouse (NV_FLOAT64 x, NV_FLOAT64 y);
  void scaleWave (NV_INT32 x, NV_INT32 y, NV_INT32 *new_x, NV_INT32 *new_y, NVMAP_DEF l_mapdef, NV_INT32 wave_num);
  void drawX (NV_INT32 x, NV_INT32 y, NV_INT32 size, NV_INT32 width, QColor color);
  void setFields ();


protected slots:

  void slotMousePress (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y);
  void slotMouseMove (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y);
  void slotResize (QResizeEvent *e);
  void closeEvent (QCloseEvent *event);
  void slotPlotWaves (NVMAP_DEF l_mapdef);

  void trackCursor ();

  void slotHelp ();

  void slotQuit ();

  void slotMode (bool state);

  void slotPrefs ();
  void slotPosClicked (int id);
  void slotClosePrefs ();

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
