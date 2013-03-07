#ifndef PFMFEATUREDEF_H
#define PFMFEATUREDEF_H

#include <QtCore>
#include <QtGui>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


#include "nvutility.h"
#include "nvutility.hpp"
#include "pfm.h"
#include "binaryFeatureData.h"


/*  Define this to do memory check debugging.  */

#undef __PFM_FEATURE_DEBUG__


#define SELECT_PASSES    4



typedef struct
{
  NV_INT32      order;
  NV_BOOL       zero;
  NV_BOOL       hpc;
  NV_FLOAT32    offset;
  QString       description;
  NV_F64_XYMBR  mbr;
  NV_F64_XYMBR  ex_mbr;
  NV_INT32      polygon_count;
  NV_FLOAT64    polygon_x[2000];
  NV_FLOAT64    polygon_y[2000];
  NV_INT32      ex_polygon_count;
  NV_FLOAT64    ex_polygon_x[2000];
  NV_FLOAT64    ex_polygon_y[2000];
  NV_INT32      window_x;
  NV_INT32      window_y;
  NV_INT32      window_width;
  NV_INT32      window_height;
  NV_BOOL       replace;
  NV_BOOL       output_polygons;
  QString       input_dir;
  QString       output_dir;
  QString       area_dir;
  QString       exclude_dir;
} OPTIONS;


typedef struct
{
  QGroupBox           *sbox;
  QGroupBox           *dbox;
  QGroupBox           *wbox;
  QProgressBar        *sbar;
  QProgressBar        *dbar;
  QProgressBar        *wbar;
  QListWidget         *list;
} RUN_PROGRESS;


//  Place to save X, Y, Z, line number, horizontal uncertainty, vertical uncertainty, and anything else we may need in the future.

typedef struct
{
  NV_F64_COORD3       xyz;          //  X, Y, Z
  NV_U_INT32          l;            //  line number for Hockey Puck of Confidence (TM)
  NV_FLOAT32          h;            //  horizontal uncertainty
  NV_FLOAT32          v;            //  vertical uncertainty
} POINT_DATA;


//  This feature structure is overkill but, if I can come up with a better method of determining the
//  shape of the feature, it may be useful in the future.

typedef struct
{
  NV_FLOAT64          x;
  NV_FLOAT64          y;
  NV_FLOAT64          z;
  NV_FLOAT32          h;            //  horizontal uncertainty
  NV_FLOAT32          v;            //  vertical uncertainty
  NV_U_BYTE           confidence;
  NV_BOOL             oct_hit[8];   //  Whether there was a point in this octant
  NV_F64_COORD3       oct_pos[8];   //  Position and depth of the point
} FEATURE;


#endif
