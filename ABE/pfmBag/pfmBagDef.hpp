#ifndef PFMBAGDEF_H
#define PFMBAGDEF_H

#include <QtCore>
#include <QtGui>
#include <stdtypes.h>
#include <proj_api.h>

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "hyp.h"
#include "bag.h"
#include "bag_xml_meta.h"

#include "binaryFeatureData.h"

#include "chrtr2.h"


#define MIN_SURFACE  0
#define MAX_SURFACE  1
#define AVG_SURFACE  2
#define CUBE_SURFACE 3

#define NO_UNCERT    0
#define STD_UNCERT   1
#define TPE_UNCERT   2
#define FIN_UNCERT   3


typedef struct
{
  NV_BOOL            active;
  QString            abbrev;
  QString            name;
} DATUM;


typedef struct
{
  QString       pfm_file_name;
  QString       avg_surface_name;
  NV_INT32      window_x;
  NV_INT32      window_y;
  NV_INT32      window_width;
  NV_INT32      window_height;
  NV_BOOL       has_z0;                //  NVTrue if the CHRTR2 file used for sep has a Z0 layer
  NV_BOOL       has_z1;                //  NVTrue if the CHRTR2 file used for sep has a Z1 layer
  NV_INT32      surface;
  NV_FLOAT64    mbin_size;
  NV_FLOAT64    gbin_size;
  NV_INT32      uncertainty;
  NV_BOOL       enhanced;
  NV_INT32      units;                 //  0 - meters, 1 - feet, 2 - fathoms, 3 - cubits, 4 - willetts
  NV_INT32      depth_cor;             //  0 - corrected, 1 - 1550 m/s, 2 - 4800 ft/s, 3 - 800 fm/s, 4 - mixed
  NV_INT32      projection;            //  0 - geodetic, 1 - UTM
  DATUM         v_datums[100];         //  From icons/vertical_datums.txt
  NV_INT32      v_datum_count;
  NV_INT32      v_datum;
  DATUM         h_datums[1000];        //  From icons/horizontal_datums.txt
  NV_INT32      h_datum_count;
  NV_INT32      h_datum;
  QString       group;
  QString       source;
  QString       version;
  QString       comments;
  NV_INT32      classification;        //  0 - Unclassified, 1 - Confidential, 2 - Secret, 3 - Top Secret
  NV_INT32      authority;
  QDate         declassDate;
  QDate         compDate;
  QString       distStatement;
  QString       title;
  QString       pi_name;
  QString       pi_title;
  QString       poc_name;
  QString       abstract;
  QString       input_dir;
  QString       output_dir;
  QString       area_dir;
  QString       sep_dir;
} OPTIONS;



typedef struct
{
  QGroupBox           *wbox;
  QGroupBox           *mbox;
  QGroupBox           *gbox;
  QProgressBar        *wbar;
  QProgressBar        *mbar;
  QProgressBar        *gbar;
} RUN_PROGRESS;



#endif
