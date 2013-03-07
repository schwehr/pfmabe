
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



#ifndef PFMGEOTIFFDEF_H
#define PFMGEOTIFFDEF_H

#include <QtCore>
#include <QtGui>

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>


#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"



#define         NUMSHADES           40
#define         NUMHUES             255
#define         SAMPLE_HEIGHT       200
#define         SAMPLE_WIDTH        130


typedef struct
{
  NV_INT32      surface;
  NV_INT32      window_x;
  NV_INT32      window_y;
  NV_INT32      window_width;
  NV_INT32      window_height;
  NV_BOOL       transparent;
  NV_BOOL       packbits;
  NV_BOOL       grey;
  NV_BOOL       restart;
  NV_FLOAT64    azimuth;
  NV_FLOAT64    elevation;
  NV_FLOAT64    exaggeration;
  NV_FLOAT64    saturation;
  NV_FLOAT64    value;
  NV_FLOAT64    start_hsv;
  NV_FLOAT64    end_hsv;
  SUN_OPT       sunopts;
  NV_INT32      units;
  NV_BOOL       dumb;
  NV_BOOL       intrp;
  NV_BOOL       elev;
  NV_BOOL       navd88;
  NV_FLOAT32    cint;
  NV_INT32      smoothing_factor;
  NV_INT32      maxd;
  QColor        color_array[NUMSHADES * (NUMHUES + 1)];
  NV_INT16      sample_data[SAMPLE_HEIGHT][SAMPLE_WIDTH];
  NV_FLOAT32    sample_min, sample_max;
  QPixmap       sample_pixmap;
  QString       input_dir;                  //  Last directory searched for input PFM files
  QString       output_dir;                 //  Last directory searched for output GeoTIFF files
  QString       area_dir;                   //  Last directory searched for area files
} OPTIONS;



typedef struct
{
  QGroupBox           *mbox;
  QGroupBox           *gbox;
  QGroupBox           *cbox;
  QProgressBar        *mbar;
  QProgressBar        *gbar;
  QProgressBar        *cbar;
} RUN_PROGRESS;



NV_FLOAT32 sunshade(NV_FLOAT32 *lower_row, NV_FLOAT32 *upper_row, NV_INT32 col_num, SUN_OPT *sunopts, NV_FLOAT64 x_cell_size, NV_FLOAT64 y_cell_size);


#endif
