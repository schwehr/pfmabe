#ifndef _AREA_CHECK_DEF_H_
#define _AREA_CHECK_DEF_H_


#include "nvutility.h"
#include "nvutility.hpp"
#include "pfm.h"

#include "llz.h"
#include "chrtr2.h"

#include "bag.h"
#include "bag_xml_meta.h"

#include "binaryFeatureData.h"

#include "gdal_priv.h"


#include <QtCore>
#include <QtGui>


using namespace std;  // Windoze bullshit - God forbid they should follow a standard


//  Pointer interaction functions.

#define         EDIT_AREA                   0
#define         ZOOM_IN_AREA                1
#define         DEFINE_RECT_AREA            2
#define         DEFINE_POLY_AREA            3
#define         MOVE_AREA                   4
#define         ADD_VERTEX                  5
#define         DELETE_VERTEX               6
#define         MOVE_VERTEX                 7
#define         EDIT_VERTEX                 8
#define         COPY_AREA                   9

#define         VERTEX_SIZE                 8
#define         NUMSHADES                   256
#define         NUMHUES                     128
#define         POLYGON_POINTS              NVMAP_POLYGON_POINTS


//  Data types

//  Modifiable

#define         ISS60_AREA                  0
#define         GENERIC_AREA                1
#define         ACE_AREA                    2
#define         ISS60_ZONE                  3
#define         GENERIC_ZONE                4

#define         MOD_TYPES                   5


//  Non-modifiable

#define         SHAPE                       5
#define         TRACK                       6
#define         FEATURE                     7
#define         YXZ                         8
#define         LLZ_DATA                    9
#define         GEOTIFF                     10
#define         CHRTR                       11
#define         BAG                         12
#define         OPTECH_FLT                  13


#define         NUM_TYPES                   14


//  The OPTIONS structure contains all those variables that can be saved to the
//  users areaCheck QSettings.


typedef struct
{
  NV_BOOL     coast;                      //  Coastline flag
  NV_BOOL     mask;                       //  Landmask flag
  NV_INT32    position_form;              //  Position format number
  QColor      coast_color;                //  Color to be used for coast
  QColor      mask_color;                 //  Color to be used for landmask
  QColor      background_color;           //  Color to be used for background
  QColor      color[NUM_TYPES];           //  Colors to be used for different data types
  QColor      highlight_color;            //  Color to be used for highlight
  QString     inputFilter;
  QString     outputFilter;
  NV_BOOL     stoplight;                  //  Set if displaying MINMAX .trk files in stop light colors
  NV_FLOAT32  minstop;                    //  Value under which MINMAX .trk data is displayed as green
  NV_FLOAT32  maxstop;                    //  Value over which MINMAX .trk data is displayed as red
  NV_BOOL     display_children;           //  Set to display sub-features
  NV_BOOL     display_feature_info;       //  Set to display feature description and remarks
  NV_BOOL     display_feature_poly;       //  Set to display feature polygons if present
  SUN_OPT     sunopts;                    //  Sun shading options
  QString     input_dir;
  QString     output_dir;
} OPTIONS;


typedef struct
{
  NV_INT32    type;
  NV_BOOL     chrtr2;
  NV_BOOL     modified;
  QColor      color;
  NV_CHAR     filename[512];
  NV_FLOAT64  *x;
  NV_FLOAT64  *y;
  NV_INT32    count;
  NV_F64_COORD2 center;
  NV_FLOAT64  x_bin_size_degrees;
  NV_FLOAT64  y_bin_size_degrees;
  NV_FLOAT64  x_bin_size_meters;
  NV_FLOAT64  y_bin_size_meters;
  NV_F64_XYMBR mbr;
  NV_INT32    bin_width;
  NV_INT32    bin_height;
  NV_BOOL     active;
} OVERLAY;


//  General stuff.

typedef struct
{
  NV_INT32    function;                   //  Active edit or zoom function
  NV_INT32    poly_count;                 //  Polygon point count
  NV_FLOAT64  polygon_x[POLYGON_POINTS];  //  Polygon X
  NV_FLOAT64  polygon_y[POLYGON_POINTS];  //  Polygon Y
  NV_BOOL     drawing;                    //  set if we are drawing the surface
  NV_BOOL     drawing_canceled;           //  set to cancel drawing
  NV_F64_XYMBR displayed_area;            //  displayed area
  NV_F64_XYMBR prev_mbr;                  //  previously displayed area
  NV_INT32    displayed_area_width;       //  displayed area width in columns
  NV_INT32    displayed_area_height;      //  displayed area height in rows
  OVERLAY     *overlays[NUM_TYPES];       //  data file overlay information (see data types above for type values)
  NV_INT32    num_overlays[NUM_TYPES];    //  number of data overlays
  QString     type_name[NUM_TYPES];       //  Data type names
  QToolButton *overlayButton[NUM_TYPES];  //  Button associated with each type;
  QColor      color_array[NUMHUES][NUMSHADES]; //  Colors for minmax, CHRTR, and BAG
  QSharedMemory *abeRegister;             //  ABE's process register
  ABE_REGISTER *abe_register;             //  Pointer to the ABE_REGISTER structure in shared memory.
  NV_BOOL     linked;                     //  Set if we have linked to another ABE application.
  NV_INT32    width;                      //  Main window width
  NV_INT32    height;                     //  Main window height
  NV_INT32    window_x;                   //  Main window x position
  NV_INT32    window_y;                   //  Main window y position
} MISC;


#endif
