#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <proj_api.h>

#include <gdal.h>
#include <cpl_string.h>
#include <ogr_api.h>
#include <ogr_srs_api.h>

#include "FileTopoOutput.h"
#include "FileHydroOutput.h"
#include "FilePOSOutput.h"
#include "FileImage.h"

#include "nvutility.h"

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
  NV_BOOL       remove;
  NV_BOOL       flip;
  NV_BOOL       pos;
  NV_BOOL       exclude;
  NV_BOOL       normalize;
  NV_BOOL       egm;
  NV_BOOL       srtm;
  NV_BOOL       utm;
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
} OPTIONS;
