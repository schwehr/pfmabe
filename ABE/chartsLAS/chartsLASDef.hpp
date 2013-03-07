#ifndef __CHARTSLASDEF_H__
#define __CHARTSLASDEF_H__

#include <liblas/capi/liblas.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <proj_api.h>

#include <QtCore>
#include <QtGui>

#include "nvutility.h"

#include "FileHydroOutput.h"
#include "FileTopoOutput.h"


#define  GCS_WGS_84           4326
#define  GCS_NAD83            4269

typedef struct
{
  QString             name;
  NV_INT32            type;
} FILE_DEFINITION;


typedef struct 
{
  NV_U_INT16                  key_directory_version;
  NV_U_INT16                  key_revision;
  NV_U_INT16                  minor_revision;
  NV_U_INT16                  number_of_keys;
  NV_U_INT16                  key_id_0;
  NV_U_INT16                  TIFF_tag_location_0;
  NV_U_INT16                  count_0;
  NV_U_INT16                  value_offset_0;
  NV_U_INT16                  key_id_1;
  NV_U_INT16                  TIFF_tag_location_1;
  NV_U_INT16                  count_1;
  NV_U_INT16                  value_offset_1;
  //NV_U_INT16                  key_id_2;
  //NV_U_INT16                  TIFF_tag_location_2;
  //NV_U_INT16                  count_2;
  //NV_U_INT16                  value_offset_2;
} GEOKEYDIRECTORYTAG_RECORD;


typedef struct
{
  QString             name;
  QString             dir_name;
  NV_FLOAT64          polygon_x[50];
  NV_FLOAT64          polygon_y[50];
  NV_INT32            poly_count;
  NV_BOOL             rect_flag;
  NV_F64_XYMBR        mbr;
  NV_INT32            tmp_num_recs[2];
  NV_INT64            start_week;
  NV_F64_COORD3       min;
  NV_F64_COORD3       max;
  LASWriterH          writer;
  LASHeaderH          header;
  NV_BOOL             opened;
  projPJ              pj_utm;
  projPJ              pj_latlon;
  NV_INT32            zone;
  FILE                *ascfp[2];
} AREA_DEFINITION;


typedef struct
{
  QGroupBox           *obox;
  QGroupBox           *fbox;
  QProgressBar        *obar;
  QProgressBar        *fbar;
  QListWidget         *list;
} RUN_PROGRESS;


#endif
