#ifndef __PFM_CHARTS_IMAGE_HPP__
#define __PFM_CHARTS_IMAGE_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <proj_api.h>

#include <QtCore>
#include <QtGui>

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>


#include "FileTopoOutput.h"
#include "FileHydroOutput.h"
#include "FilePOSOutput.h"
#include "FileImage.h"

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "pfm_extras.h"

#include "misp.h"

#include "read_srtm_topo.h"


#define         EPS    1.0e-3         /* epsilon criteria for finding match */
#define         FILTER 9


typedef struct
{
  NV_BOOL       hit;
  NV_U_INT32    start;
  NV_U_INT32    end;
} LIST_NUM;


typedef struct
{
  NV_INT32      interval;
  NV_INT32      window_x;
  NV_INT32      window_y;
  NV_INT32      window_width;
  NV_INT32      window_height;
  NV_BOOL       casi;
  NV_BOOL       remove;
  NV_BOOL       flip;
  NV_BOOL       pos;
  NV_BOOL       exclude;
  NV_BOOL       opposite;
  NV_BOOL       normalize;
  NV_BOOL       utm;
  NV_BOOL       caris;                //  Stupid option to unblock the output 'cause Caris can't read it.  DOH!
  NV_BOOL       srtm;
  NV_BOOL       srtm_available;
  NV_FLOAT64    roll_bias;
  NV_FLOAT64    pitch_bias;
  NV_FLOAT64    heading_bias;
  NV_FLOAT64    datum_offset;
  NV_FLOAT64    grid_size;
  NV_FLOAT64    time_offset;
  NV_INT32      low_limit;
  NV_INT32      high_limit;
  NV_FLOAT64    cell_size;
  NV_FLOAT64    focal_length;
  NV_FLOAT64    pixel_size;
  NV_FLOAT64    column_offset;
  NV_FLOAT64    row_offset;
  PFM_OPEN_ARGS open_args;
  NV_INT32      pfm_handle;
  NV_F64_XYMBR  mbr;
  NV_INT32      type;
  NV_FLOAT64    level;
  QString       param_dir;
  QString       output_dir;
  QString       input_dir;
  QString       area_dir;
  QString       level_dir;
} OPTIONS;


typedef struct
{
  QGroupBox           *dbox;
  QGroupBox           *ebox;
  QGroupBox           *mbox;
  QProgressBar        *dbar;
  QProgressBar        *ebar;
  QProgressBar        *mbar;
} RUN_PROGRESS;



#endif
