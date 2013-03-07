#ifndef _MOSAICVIEW_DEF_H_
#define _MOSAICVIEW_DEF_H_


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <cerrno>
#include <cmath>


#include "gdal_priv.h"


#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"

#include "binaryFeatureData.h"

#include <QtCore>
#include <QtGui>
#include <QSharedMemory>



using namespace std;  // Windoze bullshit - God forbid they should follow a standard



//  Pointer interaction functions.

#define         ZOOM_IN_AREA                1
#define         ADD_FEATURE                 2
#define         DELETE_FEATURE              3
#define         EDIT_FEATURE                4
#define         DEFINE_FEATURE_POLY_AREA    5


#define         NUM_OVERLAYS                10
#define         LINE_WIDTH                  2
#define         POLYGON_POINTS              NVMAP_POLYGON_POINTS


//  The OPTIONS structure contains all those variables that can be saved to the
//  users mosaicView QSettings.


typedef struct
{
  NV_BOOL     coast;                      //  Coastline flag
  NV_BOOL     landmask;                   //  Landmask flag
  NV_INT32    position_form;              //  Position format number
  QColor      marker_color;               //  Color to be used for the marker
  QColor      coast_color;                //  Color to be used for coast
  QColor      landmask_color;             //  Color to be used for landmask
  QColor      feature_color;              //  Color to be used for features
  QColor      feature_info_color;         //  Color to be used for feature information text display
  QColor      feature_poly_color;         //  Color to be used for feature polygon display
  QColor      rect_color;                 //  Color to be used for rectangles
  QColor      highlight_color;            //  Color to be used for highlighted objects
  NV_BOOL     display_feature;            //  Flag to show feature location
  NV_BOOL     display_children;           //  Flag to show grouped feature children locations
  NV_BOOL     display_feature_info;       //  Flag to show feature info
  NV_BOOL     display_feature_poly;       //  Flag to show feature polygonal area if it exists
  NV_FLOAT32  feature_diameter;           //  Add feature circle diameter (meters)
  NV_INT32    last_rock_feature_desc;     //  Index of last used rock, wreck, obstruction feature description
  NV_INT32    last_offshore_feature_desc; //  Index of last used offshore installation feature description
  NV_INT32    last_light_feature_desc;    //  Index of last used light, buoy, beacon feature description
  NV_INT32    last_lidar_feature_desc;    //  Index of last used lidar feature description
  QString     last_feature_description;   //  String containing the last feature descriptor used
  QString     last_feature_remarks;       //  String containing the last feature remarks used
  NV_INT32    zoom_percent;               //  Zoom in/out percentage
  NV_INT32    width;                      //  Main window width
  NV_INT32    height;                     //  Main window height
  NV_INT32    window_x;                   //  Main window x position
  NV_INT32    window_y;                   //  Main window y position
  NV_BOOL     startup_message;            //  Whether to display the startup message
  NV_INT32    new_feature;                //  Stupid counter so I can advertise new features once and then move on
  NV_INT32    screenshot_delay;           //  Delay (in seconds) of the screenshot in the updateFeature dialog.
} OPTIONS;


//  General stuff.

typedef struct
{
  NV_INT32    function;                   //  Active edit or zoom function
  NV_INT32    save_function;              //  Save last edit function
  NV_INT32    poly_count;                 //  Polygon point count
  NV_FLOAT64  polygon_x[POLYGON_POINTS];  //  Polygon X
  NV_FLOAT64  polygon_y[POLYGON_POINTS];  //  Polygon Y
  NV_F64_XYMBR displayed_area;            //  displayed area
  NV_BOOL     drawing;                    //  set if we are drawing the surface
  BFDATA_SHORT_FEATURE *feature;          //  Feature (target) data array
  NV_INT32    bfd_handle;                 //  BFD (target) file handle
  BFDATA_HEADER  bfd_header;              //  Header for the current BFD file
  BFDATA_RECORD  new_record;              //  Record built when adding a new feature
  NV_BOOL     bfd_open;                   //  Set if we have an open bfd file
  NV_INT32    nearest_feature;
  QString     feature_search_string;      //  Text to search for when highlighting features
  NV_INT32    feature_polygon_flag;
  NV_FLOAT64  lat_step;
  NV_FLOAT64  lon_step;
  NV_F64_XYMBR geotiff_area;              //  Bounds of the GeoTIFF file
  NV_INT32    mosaic_width;
  NV_INT32    mosaic_height;
  QImage      *full_res_image;
  QSharedMemory *abeShare;                //  ABE's shared memory pointer.
  ABE_SHARE   *abe_share;                 //  Pointer to the ABE_SHARE structure in shared memory.
} MISC;


#endif
