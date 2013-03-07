#ifndef __PFMEXTRACTDEF_H__
#define __PFMEXTRACTDEF_H__


#include <cerrno>

#include <QtCore>
#include <QtGui>

#include <proj_api.h>

#include "libshp/shapefil.h"

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "llz.h"


typedef struct
{
  NV_BYTE     source;                     //  0 - min filtered, 1 - max filtered, 2 - avg/MISP, 3 - all data.
  NV_BYTE     format;                     //  0 - ASCII, 1 - LLZ, 2 - RDP, 3 - SHP
  NV_BOOL     ref;                        //  Include reference data
  NV_BOOL     geoid03;                    //  Orthometric corrections applied
  NV_BOOL     chk;                        //  Only include checked/verified bins
  NV_BOOL     flp;                        //  Invert sign of Z values
  NV_BOOL     utm;                        //  Convert position to UTM
  NV_BOOL     unc;                        //  Include uncertainty data in ASCII output
  NV_BOOL     lnd;                        //  Exclude land (negative values)
  NV_BOOL     cut;                        //  Apply the depth cutoff
  NV_FLOAT64  cutoff;                     //  Depth cutoff value
  NV_FLOAT64  datum_shift;                //  Datum shift to be added
  NV_BYTE     size;                       //  0 - no limit, 1 - 1GB, 2 - 2GB, 3 - 4GB
  QString     input_dir;
  QString     area_dir;
  NV_INT32    window_x;
  NV_INT32    window_y;
  NV_INT32    window_width;
  NV_INT32    window_height;
} OPTIONS;


typedef struct
{
  QGroupBox           *ebox;
  QProgressBar        *ebar;
} RUN_PROGRESS;


#endif
