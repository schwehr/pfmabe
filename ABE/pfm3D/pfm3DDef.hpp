#ifndef _PFM3D_DEF_H_
#define _PFM3D_DEF_H_


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <cerrno>
#include <cmath>


#include "nvutility.h"
#include "nvutility.hpp"


#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>


#include "pfm.h"
#include "pfm_extras.h"

#include "binaryFeatureData.h"

#include <QtCore>
#include <QtGui>
#include <QSharedMemory>


#ifdef NVWIN3X
    #include "windows_getuid.h"

using namespace std;  // Windoze bullshit - God forbid they should follow a standard
#endif


//  Pointer interaction functions.

#define         RECT_EDIT_AREA              0
#define         POLY_EDIT_AREA              1
#define         RECT_EDIT_AREA_3D           2
#define         POLY_EDIT_AREA_3D           3
#define         ROTATE                      4
#define         ZOOM                        5
//#define         DRAG                        6


#define         H_NONE                      0
#define         H_ALL                       1
#define         H_CHECKED                   2
#define         H_01                        3
#define         H_02                        4
#define         H_03                        5
#define         H_04                        6
#define         H_05                        7
#define         H_INT                       8
#define         H_MULT                      9
#define         H_IHO_S                     10
#define         H_IHO_1                     11
#define         H_IHO_2                     12
#define         H_PERCENT                   13  //  Leave this as last highlight always


#define         NUMSHADES                   256
#define         NUMHUES                     128
#define         LINE_WIDTH                  2
#define         POLYGON_POINTS              NVMAP_POLYGON_POINTS
#define         HOTKEYS                     10


#define         MARKER_W                    15
#define         MARKER_H                    10


//  The OPTIONS structure contains all those variables that can be saved to the
//  users pfmView QSettings.


typedef struct
{
  NV_INT32    position_form;              //  Position format number
  QColor      background_color;           //  Color to be used for background
  QColor      highlight_color;            //  Color to be used for highlighting features
  QColor      tracker_color;              //  Color to be used for track cursor
  QColor      edit_color;                 //  Color to be used for edit rectangles and polygons
  QColor      scale_color;                //  Color to be used for the "scale"
  QColor      feature_color;              //  Color to be used for features
  QColor      feature_info_color;         //  Color to be used for feature information text display
  NV_FLOAT32  feature_size;               //  Feature size
  NV_INT32    zoom_percent;               //  Zoom in/out percentage
  NV_FLOAT64  exaggeration;               //  Z exaggeration
  NV_INT32    display_feature;            //  0 = no features, 1 = all features, 2 = unverified features, 3 = verified features,
  NV_BOOL     display_children;           //  Flag to show grouped feature children locations
  NV_BOOL     display_feature_info;       //  Flag to show feature info
  NV_FLOAT32  z_factor;                   //  Coversion factor for Z values. May be used to convert m to ft...
  NV_FLOAT32  z_offset;                   //  Offset value for Z values.
  NV_INT32    highlight;                  //  Type of data to highlight, -1 - all, 0 - none, 6 - checked,
                                          //  7 - multiple coverage, 5 - interpolated bins,
                                          //  1-4 - PFM_USER_01-04
  QString     buttonAccel[12];            //  Accelerator key sequences for some of the buttons
  NV_INT32    new_feature;                //  Stupid counter so I can advertise new features once and then move on
  NV_FLOAT32  highlight_percent;          //  Percentage of depth for highlight.
  NV_INT32    edit_mode;                  //  Saved "misc.function"
  NV_BOOL     draw_scale;                 //  Set this to draw the scale in the display.
} OPTIONS;


//  General stuff.

typedef struct
{
  NV_INT32    function;                   //  Active edit function
  NV_INT32    save_function;              //  Save last edit function
  NV_CHAR     GeoTIFF_name[512];          //  GeoTIFF file name
  NV_BOOL     GeoTIFF_open;               //  Set if GeoTIFF file has been opened.
  NV_BOOL     GeoTIFF_init;               //  Set if GeoTIFF is new
  NV_INT32    display_GeoTIFF;            //  0 - no display, 1 - display as decal, -1 display modulated with depth color
  NV_INT32    color_by_attribute;         //  0 for normal colors, otherwise 1 for number, 2 for std, attribute + 3.
  NV_BOOL     drawing;                    //  set if we are drawing the surface
  NV_BOOL     drawing_canceled;           //  set to cancel drawing
  NV_F64_XYMBR displayed_area;            //  displayed area for the top level PFM (or PFM currently being drawn when actually drawing)
  NV_F64_XYMBR total_displayed_area;      //  displayed area for all of the displayed PFMs
  NV_F64_XYMBR prev_mbr;                  //  previously displayed area
  NV_INT32    displayed_area_width;       //  displayed area width in columns
  NV_INT32    displayed_area_height;      //  displayed area height in rows
  NV_INT32    displayed_area_column;      //  displayed area start column
  NV_INT32    displayed_area_row;         //  displayed area start row
  NV_FLOAT32  displayed_area_min;         //  min Z value for displayed area
  NV_FLOAT32  displayed_area_max;         //  max Z value for displayed area
  NV_FLOAT32  displayed_area_range;       //  Z range for displayed area
  NV_FLOAT32  displayed_area_attr_min;
  NV_FLOAT32  displayed_area_attr_max;
  NV_FLOAT32  displayed_area_attr_range;
  BFDATA_SHORT_FEATURE *feature;          //  Feature (target) data array
  NV_INT32    bfd_handle;                 //  BFD (target) file handle
  BFDATA_HEADER  bfd_header;              //  Header for the current BFD file
  NV_BOOL     bfd_open;                   //  Set if we have an open BFD file
  QColor      color_array[2][NUMHUES + 1][NUMSHADES];
                                          //  arrays of surface colors
  QString     buttonText[12];             //  Tooltip text for the buttons that have editable accelerators
  QToolButton *button[12];                //  Buttons that have editable accelerators
  NV_INT32    nearest_pfm;
  QColor      widgetBackgroundColor;      //  The normal widget background color.
  QString     help_browser;               //  Browser command name
  QString     html_help_tag;              //  Help tag for the currently running section of code.  This comes from
                                          //  the PFM_ABE html help documentation.
  QSharedMemory *abeShare;                //  ABE's shared memory pointer.
  QSharedMemory *abeRegister;             //  ABE's process register
  ABE_SHARE   *abe_share;                 //  Pointer to the ABE_SHARE structure in shared memory.
  ABE_REGISTER *abe_register;             //  Pointer to the ABE_REGISTER structure in shared memory.
  NV_FLOAT64  map_center_x;               //  Center of the map to be used for view reset.
  NV_FLOAT64  map_center_y;               //  Center of the map to be used for view reset.
  NV_FLOAT64  map_center_z;               //  Center of the map to be used for view reset.
  OTF_GRID_RECORD *otf_grid;              //  Pointer to on-the-fly grid (from pfmView).
  QSharedMemory *otfShare;                //  otf_grid shared memory pointer.
  NV_INT32    width;                      //  Main window width
  NV_INT32    height;                     //  Main window height
  NV_INT32    window_x;                   //  Main window x position
  NV_INT32    window_y;                   //  Main window y position


  //  The following concern PFMs as layers.  There are a few things from ABE_SHARE that also need to be 
  //  addressed when playing with layers - open_args, display_pfm, and pfm_count.

  NV_F64_XYMBR total_mbr;                 //  MBR of all of the displayed PFMs
  NV_CHAR     attr_format[MAX_ABE_PFMS][NUM_ATTR][20]; //  Formats to use for displaying attributes
  NV_INT32    pfm_handle[MAX_ABE_PFMS];   //  PFM file handle
  NV_INT32    average_type[MAX_ABE_PFMS]; //  Type of average filtered surface, 0 - normal, 1 - minimum misp, 
                                          //  2 - average misp, 3 - maximum misp
  NV_BOOL     cube_attr_available[MAX_ABE_PFMS]; //  NVTrue is CUBE attributes are in the file
  NV_INT32    layer[MAX_ABE_PFMS];
} MISC;


#endif
