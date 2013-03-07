
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef __PFMLOADDEF_H__
#define __PFMLOADDEF_H__

#include <stdio.h>
#include <stdlib.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QtCore>
#include <QtGui>

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "pfm_extras.h"

#include "read_srtm_topo.h"
#include "dted.h"
#include "chrtr.h"
#include "chrtr2.h"
#include "binaryFeatureData.h"

#include "pfmWDBSecurity.hpp"


#define GSF_ATTRIBUTES        32
#define HOF_ATTRIBUTES        48
#define TOF_ATTRIBUTES        9
#define WLF_ATTRIBUTES        13
#define CZMIL_ATTRIBUTES      36
#define BAG_ATTRIBUTES        1
#define HAWKEYE_ATTRIBUTES    68


typedef struct
{
  NV_FLOAT32         elev;
  NV_FLOAT32         uncert;
} BAG_RECORD;


typedef struct
{
  QString             name;
  NV_FLOAT32          max_depth;
  NV_FLOAT32          min_depth;
  NV_FLOAT32          precision;
  NV_FLOAT32          mbin_size;
  NV_FLOAT32          gbin_size;
  NV_BOOL             apply_area_filter;
  NV_BOOL             deep_filter_only;
  PFM_OPEN_ARGS       open_args;
  QString             area;
  NV_INT32            polygon_count;
  NV_F64_COORD2       polygon[2000];
  QString             mosaic;
  QString             feature;
  NV_FLOAT32          cellstd;
  NV_FLOAT32          radius;
  NV_INT32            index;
  NV_INT32            hnd;
  NV_BYTE             *add_map;
  NV_BOOL             existing;
  NV_BOOL             dateline;
} PFM_DEFINITION;


typedef struct
{
  NV_INT32           max_files;
  NV_INT32           max_lines;
  NV_INT32           max_pings;
  NV_INT32           max_beams;
  NV_FLOAT64         horizontal_error;
  NV_FLOAT64         vertical_error;
  NV_INT32           iho;
  NV_FLOAT64         capture;
  NV_INT32           queue;
  NV_FLOAT64         horiz;
  NV_FLOAT64         distance;
  NV_FLOAT64         min_context;
  NV_FLOAT64         max_context;
  NV_INT32           std2conf;              //  This is one less than the number needed in navo_pfm_cube
  NV_INT32           disambiguation;
  NV_BOOL            appending;             //  Set if we are appending to one or more pre-existing files
  NV_INT16           attribute_count;
  QString            time_attribute_name;
  NV_INT16           time_attribute_num;
  NV_FLOAT32         time_attribute_def[3];
  QString            gsf_attribute_name[GSF_ATTRIBUTES];
  QString            hof_attribute_name[HOF_ATTRIBUTES];
  QString            tof_attribute_name[TOF_ATTRIBUTES];
  QString            wlf_attribute_name[WLF_ATTRIBUTES];
  QString            czmil_attribute_name[CZMIL_ATTRIBUTES];
  QString            bag_attribute_name[BAG_ATTRIBUTES];
  QString            hawkeye_attribute_name[HAWKEYE_ATTRIBUTES];
  NV_INT16           gsf_attribute_num[GSF_ATTRIBUTES];
  NV_INT16           hof_attribute_num[HOF_ATTRIBUTES];
  NV_INT16           tof_attribute_num[TOF_ATTRIBUTES];
  NV_INT16           wlf_attribute_num[WLF_ATTRIBUTES];
  NV_INT16           czmil_attribute_num[CZMIL_ATTRIBUTES];
  NV_INT16           bag_attribute_num[BAG_ATTRIBUTES];
  NV_INT16           hawkeye_attribute_num[HAWKEYE_ATTRIBUTES];
  NV_FLOAT32         gsf_attribute_def[GSF_ATTRIBUTES][3];
  NV_FLOAT32         hof_attribute_def[HOF_ATTRIBUTES][3];
  NV_FLOAT32         tof_attribute_def[TOF_ATTRIBUTES][3];
  NV_FLOAT32         wlf_attribute_def[WLF_ATTRIBUTES][3];
  NV_FLOAT32         czmil_attribute_def[CZMIL_ATTRIBUTES][3];
  NV_FLOAT32         bag_attribute_def[BAG_ATTRIBUTES][3];
  NV_FLOAT32         hawkeye_attribute_def[HAWKEYE_ATTRIBUTES][3];
  QString            input_dir;
  QString            output_dir;
  QString            area_dir;
  QString            mosaic_dir;
  QString            feature_dir;
  QStringList        input_dirs;         //  List of all directories and filters input via the directory browse method of inputPage
  NV_INT32           cache_mem;
} PFM_GLOBAL;


typedef struct
{
  QString             name;
  NV_BOOL             status;
  NV_INT32            type;
} FILE_DEFINITION;


typedef struct
{
  NV_BOOL             chk;                   //  Preliminary file check
  NV_BOOL             hof;                   //  Load HOF NULL values
  NV_BOOL             sub;                   //  Invert paths from Linux to Windoze
  NV_BOOL             lnd;                   //  Invalidate HOF land data
  NV_BOOL             ref;                   //  Load data as reference
  NV_BOOL             tof;                   //  Load TOF first returns as reference
  NV_BOOL             lid;                   //  Load HOF LIDAR user flags
  NV_BOOL             nom;                   //  Load GSF nominal depth if available
  NV_BOOL             sec;                   //  Invalidate all HOF secondary returns
  NV_BOOL             old;                   //  Load HOF LIDAR data in the old PFM_SHOALS_1K_Data form
  NV_BOOL             srtmb;                 //  Load best resolution SRTM data
  NV_BOOL             srtm1;                 //  Load 1 second resolution SRTM data (US only)
  NV_BOOL             srtm2;                 //  Load 1 second resolution SRTM data (world - DOD restricted)
  NV_BOOL             srtm3;                 //  Load 3 second resolution SRTM data
  NV_BOOL             srtm30;                //  Load 30 second resolution SRTM data
  NV_BOOL             srtmr;                 //  Load SRTM as reference
  NV_BOOL             srtme;                 //  Exclude SRTM2 data (1 second world)
  NV_BOOL             cube;                  //  Run cube at end of load
  NV_BOOL             attr;                  //  Load CUBE attributes
} FLAGS;


typedef struct
{
  QDialog             *gsfD;
  QDialog             *hofD;
  QDialog             *tofD;
  QDialog             *wlfD;
  QDialog             *czmilD;
  QDialog             *bagD;
  QDialog             *hawkeyeD;
  QDialog             *cubeD;
} GLOBAL_DIALOG;


typedef struct
{
  QGroupBox           *fbox;
  QGroupBox           *rbox;
  QProgressBar        *fbar;
  QProgressBar        *rbar;
} RUN_PROGRESS;



#include "load_file.hpp"


#endif
