#ifndef __TRACKLINEDEF_H__
#define __TRACKLINEDEF_H__


#include <cerrno>
#include <unistd.h>
#include <sys/types.h>

#include <QtCore>
#include <QtGui>

#include "nvutility.h"
#include "nvutility.hpp"

#include "hmpsparm.h"

#include "gsf.h"
#include "FilePOSOutput.h"
#include "wlf.h"
#include "hawkeye.h"

#include "version.hpp"


#define MPS_TO_KNOTS 1.943844924


//  Data types.

#define TRACKLINE_GSF        0
#define TRACKLINE_POS        1
#define TRACKLINE_WLF        2
#define TRACKLINE_HWK        3


typedef struct
{
  NV_FLOAT64          x;      //  position longitude (dec deg)
  NV_FLOAT64          y;      //  position latitude (dec deg)
  NV_FLOAT64          h;      //  heading (degrees)
  NV_FLOAT64          s;      //  speed in nautical miles per hour
  NV_FLOAT64          t;      //  time in seconds from 1900
  NV_INT32            f;      //  input file # (starting from 0)
  NV_INT32            p;      //  ping record_number (starting with 1)
} INFO;


typedef struct
{
  QGroupBox           *fbox;
  QProgressBar        *fbar;
} RUN_PROGRESS;


typedef struct
{
  NV_FLOAT64          heading[4];
  NV_FLOAT64          speed[4];
  QString             inputFilter;
  QString             input_dir;
  QString             output_dir;
  NV_INT32            window_x;
  NV_INT32            window_y;
  NV_INT32            width;
  NV_INT32            height;
} OPTIONS;


void envin (OPTIONS *options);
void envout (OPTIONS *options);


#endif
