
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef _PFM_VIEW_DEF_H_
#define _PFM_VIEW_DEF_H_


#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "nvutility.h"
#include "nvutility.hpp"

#include "targetlib.h"
#include "MIW.h"

#include "binaryFeatureData.h"

#include "pfmWDBSecurity.hpp"


#ifdef NVWIN3X
    #include "windows_getuid.h"
#endif

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>

#include "pfm.h"
#include "pfm_extras.h"

#include <QtCore>
#include <QtGui>
#include <QSharedMemory>


using namespace std;  // Windoze bullshit - God forbid they should follow a standard


//  Pointer interaction functions.

#define         RECT_EDIT_AREA              0   //!<  Define a rectangular area to edit in 2D
#define         POLY_EDIT_AREA              1   //!<  Define a polygonal area to edit in 2D
#define         RECT_EDIT_AREA_3D           2   //!<  Define a rectangular area to edit in 3D
#define         POLY_EDIT_AREA_3D           3   //!<  Define a polygonal area to edit in 3D
#define         ZOOM_IN_AREA                4   //!<  Define a rectangular area to zoom in to
#define         OUTPUT_POINTS               5   //!<  Select data points to output to an ASCII file
#define         DEFINE_RECT_AREA            6   //!<  Define a rectangular area to save as a generic or Army Corps area file
#define         DEFINE_POLY_AREA            7   //!<  Define a polygonal area to save as a generic or Army Corps area file
#define         SUNANGLE                    8   //!<  Set the sun angle for sun shading
#define         ADD_FEATURE                 9   //!<  Add a feature to the feature file (optionally creates a feature file)
#define         DELETE_FEATURE              10  //!<  Delete a feature from the feature file
#define         EDIT_FEATURE                11  //!<  Edit a feature in the feature file
#define         DRAW_CONTOUR                12  //!<  Draw a contour to be used for redefining the MISP surface
#define         REMISP_AREA                 13  //!<  Define a rectangular area to regrid with MISP
#define         DRAW_CONTOUR_FILTER         14  //!<  Draw a contour to be used for the MISP surface filter
#define         REMISP_FILTER               15  //!<  Define a polygonal area in which to perform the remisp filter
#define         RECT_FILTER_AREA            16  //!<  Define a rectangular area to filter (statistically)
#define         POLY_FILTER_AREA            17  //!<  Define a polygonal area to filter (statistically)
#define         DEFINE_FEATURE_POLY_AREA    18  //!<  Define a polygon to be associated with a feature point
#define         POLY_FILTER_MASK            19  //!<  Define a polygonal area to mask from filtering
#define         RECT_FILTER_MASK            20  //!<  Define a rectangular area to mask from filtering
#define         SELECT_HIGH_POINT           21  //!<  Define a rectangular area in which to mark a high (minimum Z) point
#define         SELECT_LOW_POINT            22  //!<  Define a rectangular area in which to mark a low (maximum Z) point
#define         GRAB_CONTOUR                23  //!<  Define a polygonal area and insert interpolated contours into the bins


//  Coverage area pointer interaction functions.

#define         COV_START_DRAW_RECTANGLE    0   //!<  Start defining a rectangular area
#define         COV_DRAW_RECTANGLE          1   //!<  Define a rectangular area to view
#define         COV_START_DRAG_RECTANGLE    2   //!<  Start the process of dragging the rectangle
#define         COV_DRAG_RECTANGLE          3   //!<  Drag the current rectangular area to a new position


//  Highlight types.

#define         H_NONE                      0   //!<  No highlighting
#define         H_ALL                       1   //!<  Highlight everything (essentially gray shade)
#define         H_CHECKED                   2   //!<  Highlight checked data
#define         H_01                        3   //!<  Highlight bins containing PFM_USER_01 marked data
#define         H_02                        4   //!<  Highlight bins containing PFM_USER_02 marked data
#define         H_03                        5   //!<  Highlight bins containing PFM_USER_03 marked data
#define         H_04                        6   //!<  Highlight bins containing PFM_USER_04 marked data
#define         H_05                        7   //!<  Highlight bins containing PFM_USER_05 marked data
#define         H_INT                       8   //!<  Highlight interpolated (MISP) bins
#define         H_MULT                      9   //!<  Highlight bins with data from multiple lines
#define         H_COUNT                     10  //!<  Highlight bins with more than options.h_count points
#define         H_IHO_S                     11  //!<  Highlight bins containing possible IHO Special Order outliers
#define         H_IHO_1                     12  //!<  Highlight bins containing possible IHO Order 1 outliers
#define         H_IHO_2                     13  //!<  Highlight bins containing possible IHO Order 2 outliers
#define         H_PERCENT                   14  //!<  Highlight bins containing outliers that fall outside a user defined percentage of depth

#define         NUM_HIGHLIGHTS              15


#define         NUMSHADES                   256    //!<  Number of shades to be used for sunshading
#define         NUMHUES                     128    //!<  Number of hues to be used for coloring the surface
#define         NUM_SURFACES                12     //!<  Number of available surface types (including OTF)
#define         NUM_SCALE_LEVELS            16     //!<  Number of color scale boxes
#define         CONTOUR_POINTS              1000   //!<  Max number of points in a drawn contour
#define         DEFAULT_SEGMENT_LENGTH      0.25   //!<  Default segment length for contours in scribe.cpp
#define         NUM_OVERLAYS                10     //!<  Max number of overlays
#define         LINE_WIDTH                  2      //!<  Standard line width
#define         COV_FEATURE_SIZE            2      //!<  Standard coverage feature size
#define         POLYGON_POINTS              NVMAP_POLYGON_POINTS   //!<  Maximum number of points in a polygon
#define         NUM_TOOLBARS                8      //!<  Number of toolbars defined
#define         OTF_OFFSET                  100    //!<  Arbitrary number used to offset the MIN_FILTERED_DEPTH and MAX_FILTERED_DEPTH values
                                                   //!<  in order to indicate that we are doing on-the-fly binning to build the surface
#define         MARKER_W                    15     //!<  Marker (cursor point tracker) width
#define         MARKER_H                    10     //!<  Marker height
#define         MAX_RECENT                  10     //!<  Maximum number of files in the Open Recent... file list
#define         PRE_ATTR                    6      //!<  Number of built in attributes for coloring prior to the PFM bin attributes
#define         NUM_HSV                     NUM_ATTR + PRE_ATTR  //!<  Possible number of scale box HSV settings 


//  Button hotkeys that are editable by the user.  These are used as indexes into the options.buttonAccel, misc.buttonText,
//  misc.button, and misc.buttonIcon arrays.  To add or remove from this list you need to change it here, in set_defaults.cpp
//  and in pfmView.cpp (look for the tool buttons that have one of these defined names attached).

#define         REDRAW_KEY                  0
#define         REDRAW_COVERAGE_KEY         1
#define         TOGGLE_CONTOUR_KEY          2
#define         DISPLAY_AVG_EDITED_KEY      3
#define         DISPLAY_MIN_EDITED_KEY      4
#define         DISPLAY_MAX_EDITED_KEY      5
#define         DISPLAY_AVG_UNEDITED_KEY    6
#define         DISPLAY_MIN_UNEDITED_KEY    7
#define         DISPLAY_MAX_UNEDITED_KEY    8
#define         DISPLAY_MIN_EDITED_OTF_KEY  9
#define         DISPLAY_MAX_EDITED_OTF_KEY  10
#define         TOGGLE_GEOTIFF_KEY          11

#define         HOTKEYS                     12


//  Overlay types

#define         ISS60_AREA                  1
#define         GENERIC_AREA                2
#define         ISS60_ZONE                  3
#define         GENERIC_ZONE                4
#define         ISS60_SURVEY                5
#define         TRACK                       6
#define         GENERIC_YXZ                 7
#define         SHAPE                       8
#define         ACE_AREA                    9


//!  Output data points structure

typedef struct
{
  NV_F64_COORD2   xy;
  NV_FLOAT32      avg;
  NV_FLOAT32      min;
  NV_FLOAT32      max;
} OUTPUT_DATA_POINTS;


//!  The OPTIONS structure contains all those variables that can be saved to the users pfmView QSettings.

typedef struct
{
  NV_BOOL     contour;                    //!<  To contour or not to contour...
  NV_BOOL     coast;                      //!<  Coastline flag
  NV_BOOL     landmask;                   //!<  Landmask flag
  NV_INT32    position_form;              //!<  Position format number
  NV_INT32    contour_width;              //!<  Contour line width/thickness in pixels
  NV_INT32    contour_index;              //!<  Contour index (Nth contour emphasized)
  QColor      contour_color;              //!<  Color to be used for contours
  QColor      contour_highlight_color;    //!<  Color to be used for highlighted contours
  QColor      feature_color;              //!<  Color to be used for features
  QColor      feature_info_color;         //!<  Color to be used for feature information text display
  QColor      feature_poly_color;         //!<  Color to be used for feature polygon display
  QColor      verified_feature_color;     //!<  Color to be used for verified features (confidence = 5)
  QColor      coast_color;                //!<  Color to be used for coast
  QColor      landmask_color;             //!<  Color to be used for landmask
  QColor      background_color;           //!<  Color to be used for background
  QColor      cov_feature_color;          //!<  Color to be used for features on coverage map
  QColor      cov_inv_feature_color;      //!<  Color to be used for invalid features on the coverage map
  QColor      cov_verified_feature_color; //!<  Color to be used for verified features (confidence = 5) on the coverage map
  QColor      poly_filter_mask_color;     //!<  Color to be used for filter masks
  NV_BOOL     stoplight;                  //!<  Set if we're going to display using stoplight coloring
  NV_FLOAT32  stoplight_min_mid;          //!<  Value for crossover from minimum to mid
  NV_FLOAT32  stoplight_max_mid;          //!<  Value for crossover from amximum to mid
  QColor      stoplight_max_color;        //!<  Color to be used for stoplight display max (normally red for depth, green for other).
  QColor      stoplight_mid_color;        //!<  Color to be used for stoplight display mid (normally yellow).
  QColor      stoplight_min_color;        //!<  Color to be used for stoplight display min (normally green for depth, red for other).
  NV_INT32    stoplight_min_index;        //!<  Index into the HSV color arry for the min stoplight color (not saved to qsettings)
  NV_INT32    stoplight_mid_index;        //!<  Index into the HSV color arry for the mid stoplight color (not saved to qsettings)
  NV_INT32    stoplight_max_index;        //!<  Index into the HSV color arry for the max stoplight color (not saved to qsettings)
  NV_U_INT16  min_hsv_color[NUM_HSV];     //!<  HSV color wheel index for color to be used for minimum values in surface (default 0)
  NV_U_INT16  max_hsv_color[NUM_HSV];     //!<  HSV color wheel index for color to be used for maximum values in surface (default 315)
  NV_FLOAT32  min_hsv_value[NUM_HSV];     //!<  Locked minimum data value for minimum HSV color
  NV_FLOAT32  max_hsv_value[NUM_HSV];     //!<  Locked maximum data value for maximum HSV color
  NV_BOOL     min_hsv_locked[NUM_HSV];    //!<  Set if min HSV is locked to a value
  NV_BOOL     max_hsv_locked[NUM_HSV];    //!<  Set if max HSV is locked to a value
  NV_CHAR     edit_name[256];             //!<  Editor name
  NV_CHAR     edit_name_3D[256];          //!<  3D Editor name
  SUN_OPT     sunopts;                    //!<  Sun shading options
  NV_BOOL     display_suspect;            //!<  Flag to show suspect data
  NV_INT32    display_feature;            //!<  0 = no features, 1 = all features, 2 = unverified features, 3 = verified features,
  NV_BOOL     display_children;           //!<  Flag to show grouped feature children locations
  NV_BOOL     display_selected;           //!<  Flag to show selected soundings
  NV_BOOL     display_reference;          //!<  Flag to show reference soundings
  NV_BOOL     display_feature_info;       //!<  Flag to show feature info
  NV_BOOL     display_feature_poly;       //!<  Flag to show feature polygonal area if it exists
  NV_BOOL     display_minmax;             //!<  Flag to show min and max points
  NV_INT32    edit_mode;                  //!<  0 - rectangle, 1 - polygon
  NV_BOOL     auto_redraw;                //!<  Flag for auto redraw after edit
  NV_CHAR     unload_prog[256];           //!<  Unload program name
  NV_CHAR     cube_prog[256];             //!<  CUBE program name
  NV_CHAR     import_prog[256];           //!<  DNC import program name
  NV_INT32    smoothing_factor;           //!<  Contour smoothing factor (0-10)
  NV_FLOAT32  z_factor;                   //!<  Coversion factor for Z values. May be used to convert m to ft...
  NV_FLOAT32  z_offset;                   //!<  Offset value for Z values.
  NV_INT32    highlight;                  //!<  See H_NONE and others above for definitions
  QString     buttonAccel[HOTKEYS];       //!<  Accelerator key sequences for some of the buttons
  NV_FLOAT32  feature_radius;             //!<  Filter exclusion distance for features
  NV_INT32    last_rock_feature_desc;     //!<  Index of last used rock, wreck, obstruction feature description
  NV_INT32    last_offshore_feature_desc; //!<  Index of last used offshore installation feature description
  NV_INT32    last_light_feature_desc;    //!<  Index of last used light, buoy, beacon feature description
  NV_INT32    last_lidar_feature_desc;    //!<  Index of last used lidar feature description
  QString     last_feature_description;   //!<  String containing the last feature descriptor used
  QString     last_feature_remarks;       //!<  String containing the last feature remarks used
  QString     feature_search_string;      //!<  Text to search for when highlighting features
  NV_INT32    misp_weight;                //!<  Weight value for misp_init in remisp
  NV_BOOL     misp_force_original;        //!<  Force original flag for remisp
  NV_BOOL     misp_replace_all;           //!<  Set if you want to replace the surface in bins with original input data
  NV_FLOAT32  filterSTD;                  //!<  Filter standard deviation
  NV_BOOL     deep_filter_only;           //!<  Filter only in the downward direction (assumes positive is depth)
  NV_INT32    overlap_percent;            //!<  Window overlap percentage when moving with arrow keys
  NV_INT32    GeoTIFF_alpha;              //!<  25 - almost invisible, 255 - opaque, range in between - some transparency
  QString     mosaic_prog;                //!<  Mosaic viewer program - from pfmEdit via abe_share or QSettings
  QString     mosaic_hotkey;              //!<  mosaicView hot keys - from pfmEdit via abe_share or QSettings
  QString     mosaic_actkey;              //!<  mosaicView hot keys - from pfmEdit via abe_share or QSettings
  QString     threeD_prog;                //!<  3D viewer program.
  QString     threeD_hotkey;              //!<  3D viewer hot keys.
  QString     threeD_actkey;              //!<  3D viewer hot keys.
  NV_INT32    new_feature;                //!<  Stupid counter so I can force the startup message when I add a new feature
  NV_BOOL     startup_message;            //!<  Display the startup message
  QString     overlay_dir;                //!<  Last directory searched for overlays
  QString     output_area_dir;            //!<  Last directory in which an area file was saved
  QString     output_points_dir;          //!<  Last directory in which an output data points file was saved
  QString     input_pfm_dir;              //!<  Last directory searched for PFM files
  QString     geotiff_dir;                //!<  Last directory searched for GeoTIFF files
  QString     area_dir;                   //!<  Last directory searched for area files
  QString     dnc_dir;                    //!<  Last directory searched for DNC files
  QString     recentFile[MAX_RECENT];     //!<  Saved file names for Open Recent...
  NV_INT32    recent_file_count;          //!<  Number of files in Open Recent... list
  NV_FLOAT32  highlight_percent;          //!<  Percentage of depth for highlight.
  NV_INT32    chart_scale;                //!<  Chart scale used for defining feature circle radius for polygon definition
  NV_INT32    layer_type;                 //!<  Type of bin data/contour to display (same as misc.abe_share->layer_type)
  NV_FLOAT64  otf_bin_size_meters;        //!<  Bin size for on-the-fly gridding
  NV_FLOAT32  cint;                       //!<  Contour interval (same as misc.abe_share->cint)
  NV_INT32    num_levels;                 //!<  Number of contour levels (same as misc.abe_share->num_levels)
  NV_FLOAT32  contour_levels[200];        //!<  User defined contour levels (same as misc.abe_share->contour_levels)
  NV_INT32    screenshot_delay;           //!<  Delay (in seconds) of the screenshot in the updateFeature dialog.
  NV_BOOL     group_features;             //!<  Group feature flag for Define Feature Polygon entry of the Utilities menu
  NV_FLOAT32  contour_filter_envelope;    //!<  Contour filter envelope value
  NV_INT32    main_button_icon_size;      //!<  Main button icon size (16, 24, 32)
  QColor      color_array[2][NUMHUES + 1][NUMSHADES]; //!<  arrays of surface colors
  NV_BOOL     zero_turnover;              //!<  Set if we restart the colors at zero
  NV_INT32    feature_search_type;        //!<  0 = highlight result of search, 1 = only display result of search
  NV_BOOL     feature_search_invert;      //!<  if set, search for features NOT containing search string
  NV_INT32    h_count;                    //!<  Highlight bins with more than this number of points


  //!  CUBE parameters (see CUBE documentation)

  NV_INT32    iho;
  NV_FLOAT64  capture;
  NV_INT32    queue;
  NV_FLOAT64  horiz;
  NV_FLOAT64  distance;
  NV_FLOAT64  min_context;
  NV_FLOAT64  max_context;
  NV_INT32    std2conf;                   //!<  This is one less than the number needed in navo_pfm_cube
  NV_INT32    disambiguation;
} OPTIONS;


//!  Overlay structure.

typedef struct
{
  NV_INT32    file_type;                  //!<  File type (see ISS60_AREA above)
  NV_BOOL     display;                    //!<  Set to NVTrue if it is to be displayed
  QColor      color;                      //!<  Color to draw the overlay
  NV_CHAR     filename[512];              //!<  Overlay file name
} OVERLAY;


//!  Filter mask structure.

typedef struct
{
  NV_FLOAT64  x[POLYGON_POINTS];          //!<  Array of polygon X values
  NV_FLOAT64  y[POLYGON_POINTS];          //!<  Array of polygon Y values
  NV_INT32    count;                      //!<  Number of points in the mask polygon
  NV_BOOL     displayed;                  //!<  Set to NVTrue if this mask has been displayed since last redraw
} FILTER_MASK;


//!  General stuff (miscellaneous).

typedef struct
{
  NV_INT32    function;                   //!<  Active edit or zoom function
  NV_INT32    save_function;              //!<  Save last function
  NV_INT32    last_edit_function;         //!<  Last edit function (so we can always come up in edit mode).
  NV_INT32    cov_function;               //!<  Active coverage map function
  NV_BOOL     coverage_drawing;           //!<  Set if coverage is drawing
  NV_INT32    poly_count;                 //!<  Polygon point count
  NV_FLOAT64  polygon_x[POLYGON_POINTS];  //!<  Polygon X
  NV_FLOAT64  polygon_y[POLYGON_POINTS];  //!<  Polygon Y
  NV_INT32    poly_filter_mask_count;     //!<  Number of polygonal filter mask areas defined
  FILTER_MASK *poly_filter_mask;          //!<  Pointer to array of polygonal filter mask areas.
  NV_INT32    maxd;                       //!<  Maximum contour density per grid cell
  NV_CHAR     GeoTIFF_name[512];          //!<  GeoTIFF file name
  NV_BOOL     GeoTIFF_open;               //!<  Set if GeoTIFF file has been opened.
  NV_BOOL     GeoTIFF_init;               //!<  Set if GeoTIFF is new
  NV_INT32    display_GeoTIFF;            //!<  0 - no display, 1 - display on top, -1 display underneath data
  NV_INT32    color_by_attribute;         //!<  0 for normal colors, otherwise 1 for number, 2 for std, attribute + 3.
  NV_BOOL     drawing;                    //!<  set if we are drawing the surface
  NV_BOOL     drawing_canceled;           //!<  set to cancel drawing
  NV_BOOL     surface_val;                //!<  surface valid or invalid
  NV_BOOL     prev_surface_val;           //!<  used to test for changed surface
  NV_BOOL     clear;                      //!<  clear entire area flag
  NV_BOOL     cov_clear;                  //!<  clear entire coverage area flag
  NV_F64_XYMBR displayed_area[MAX_ABE_PFMS]; //!<  displayed area for each of the PFM(s)
  NV_F64_XYMBR total_displayed_area;      //!<  displayed area for all of the displayed PFMs
  NV_F64_XYMBR prev_total_displayed_area; //!<  previously displayed total area
  NV_INT32    displayed_area_width[MAX_ABE_PFMS]; //!<  displayed area width in columns
  NV_INT32    displayed_area_height[MAX_ABE_PFMS]; //!<  displayed area height in rows
  NV_INT32    displayed_area_column[MAX_ABE_PFMS]; //!<  displayed area start column
  NV_INT32    displayed_area_row[MAX_ABE_PFMS]; //!<  displayed area start row
  NV_FLOAT32  displayed_area_min;         //!<  min Z value for displayed area
  NV_I32_COORD2 displayed_area_min_coord; //!<  PFM coordinates of the min value in the displayed area
  NV_INT32    displayed_area_min_pfm;     //!<  PFM layer of the min value in the displayed area
  NV_FLOAT32  displayed_area_max;         //!<  max Z value for displayed area
  NV_I32_COORD2 displayed_area_max_coord; //!<  PFM coordinates of the max value in the displayed area
  NV_INT32    displayed_area_max_pfm;     //!<  PFM layer of the max value in the displayed area
  NV_FLOAT32  displayed_area_std;         //!<  max STD value for displayed area
  NV_I32_COORD2 displayed_area_std_coord; //!<  PFM coordinates of the max STD in the displayed area
  NV_INT32    displayed_area_std_pfm;     //!<  PFM layer of the max STD in the displayed area
  NV_FLOAT32  displayed_area_range;       //!<  Z range for displayed area
  NV_FLOAT32  displayed_area_attr_min; //!<  Minimum PFM attribute value (if coloring by attribute)
  NV_FLOAT32  displayed_area_attr_max; //!<  Maximum PFM attribute value (if coloring by attribute)
  NV_FLOAT32  displayed_area_attr_range; //!<  PFM attribute range (if coloring by attribute)
  BFDATA_SHORT_FEATURE *feature;          //!<  Feature (target) data array
  NV_INT32    bfd_handle;                 //!<  BFD (target) file handle
  BFDATA_HEADER  bfd_header;              //!<  Header for the current BFD file
  BFDATA_RECORD  new_record;              //!<  Record built when adding a new feature
  NV_BOOL     bfd_open;                   //!<  Set if we have an open bfd file
  NV_INT32    hatchr_start_x;             //!<  start column for hatchr
  NV_INT32    hatchr_end_x;               //!<  end column for hatchr
  NV_INT32    hatchr_start_y;             //!<  start row for hatchr
  NV_INT32    hatchr_end_y;               //!<  end row for hatchr
  NV_FLOAT32  *current_row;               //!<  One D array of data to pass to contour package
  NV_FLOAT32  *current_attr;              //!<  One D array of attribute data to pass to contour package
  NV_U_CHAR   *current_flags;             //!<  One D array of edit flags to pass to render function
  NV_FLOAT32  *next_row;                  //!<  One D array of data for sunshading
  OVERLAY     overlays[NUM_OVERLAYS];     //!<  overlay file information
  OUTPUT_DATA_POINTS *output_points;      //!<  output data points
  NV_INT32    output_point_count;         //!<  number of points for OUTPUT_POINTS
  NV_I32_COORD2 center_point;             //!<  center of drawing area in pixels
  NV_FLOAT32  sin_array[3600];            //!<  Pre-computed sine values at 1/10 degree
  NV_FLOAT32  cos_array[3600];            //!<  Pre-computed cosine values at 1/10 degree
  QIcon       buttonIcon[HOTKEYS];        //!<  Tool button icons for tools having accelerators
  QString     buttonText[HOTKEYS];        //!<  Tooltip text for the buttons that have editable accelerators
  QToolButton *button[HOTKEYS];           //!<  Buttons that have editable accelerators
  QString     html_help_tag;              /*!<  Help tag for the currently running section of code.  This comes from
                                                the PFM_ABE html help documentation.  */
  NV_INT32    nearest_feature;
  NV_INT32    nearest_pfm;
  NV_FLOAT32  draw_contour_level;         //!<  Value inserted when drawing contours.
  QColor      widgetBackgroundColor;      //!<  The normal widget background color.
  NV_INT32    feature_polygon_flag;
  QSharedMemory *abeShare;                //!<  ABE's shared memory pointer.
  QSharedMemory *abeRegister;             //!<  ABE's process register
  ABE_SHARE   *abe_share;                 //!<  Pointer to the ABE_SHARE structure in shared memory.
  ABE_REGISTER *abe_register;             //!<  Pointer to the ABE_REGISTER structure in shared memory.
  NV_BOOL     linked;                     //!<  Set if we have linked to another ABE application.
  NV_BOOL     cube_available;             //!<  NVTrue if the navo_pfm_cube program is available (in the PATH)
  QString     qsettings_org;              //!<  Organization name for QSettings
  QString     qsettings_app;              //!<  Application name for QSettings
  NV_I32_COORD2 add_feature_coord;        //!<  PFM bin coordinates used if we added a feature
  NV_INT32    add_feature_index;          //!<  PFM depth array index in add_feature_coord PFM bin if we added a feature
  QString     help_browser;               //!<  Browser command name
  QWidget     *map_widget;                //!<  Map widget;
  NV_INT32    cov_width;                  //!<  Coverage window width
  NV_INT32    cov_height;                 //!<  Coverage window height
  NV_INT32    width;                      //!<  Main window width
  NV_INT32    height;                     //!<  Main window height
  NV_INT32    window_x;                   //!<  Main window x position
  NV_INT32    window_y;                   //!<  Main window y position
  NV_BOOL     def_feature_poly;           //!<  Set to true when using Utilities->Define Feature Polygon
  NV_FLOAT32  feature_poly_radius;        //!<  Radius used for defining feature polygons based on chart scale
  NV_BOOL     dateline;                   //!<  Set to NVTrue if any of the PFMs cross the dateline.
  NV_F64_COORD3 *filt_contour;            //!<  Contours drawn for filtering a MISP surface
  NV_INT32    filt_contour_count;         //!<  Number of filter contours drawn
  NV_BOOL     otf_surface;                //!<  Whether we're doing an on-the-fly binned surface instead of one of the
                                          //!<  normal surfaces
  OTF_GRID_RECORD *otf_grid;              //!<  Pointer to allocated on-the-fly grid
  QSharedMemory *otfShare;                //!<  otf_grid shared memory pointer
  NV_INT32    process_id;                 //!<  This program's process ID
  QStatusBar  *progStatus;                //!<  Progress (and message) status bar
  QProgressBar *statusProg;               //!<  Generic progress bar in status area
  QLabel      *statusProgLabel;           //!<  Generic progress bar label
  QPalette    statusProgPalette;          //!<  Background color palette for the progress bar label
  QColor      cov_color;                  //!<  Color to be used in the coverage map
  QColor      cov_checked_color;          //!<  Color to be used for checked data in the coverage map
  QColor      cov_verified_color;         //!<  Color to be used for verified data in the coverage map
  NV_BOOL     tposiafps;                  //!<  If set, don't show the famous TPOSIAFPS message again this session.
  scaleBox    *scale[NUM_SCALE_LEVELS];
  NV_FLOAT32   color_min;                 //!<  Minimum value used for color range
  NV_FLOAT32   color_max;                 //!<  Maximum value used for color range
  NV_FLOAT32   color_range;               //!<  Color range


  //  The following concern PFMs as layers.  There are a few things from ABE_SHARE that also need to be 
  //  addressed when playing with layers - open_args, display_pfm, and pfm_count.

  NV_F64_XYMBR total_mbr;                 //!<  MBR of all of the displayed PFMs
  NV_FLOAT64  ss_cell_size_x[MAX_ABE_PFMS]; //!<  Sunshading X cell size
  NV_FLOAT64  ss_cell_size_y[MAX_ABE_PFMS]; //!<  Sunshading Y cell size
  NV_CHAR     attr_format[MAX_ABE_PFMS][NUM_ATTR][20]; //!<  Formats to use for displaying attributes
  NV_INT32    pfm_handle[MAX_ABE_PFMS];   //!<  PFM file handle
  NV_INT32    average_type[MAX_ABE_PFMS]; /*!<  Type of average filtered surface, 0 - normal, 1 - minimum MISP, 
                                                2 - average MISP, 3 - maximum MISP  */
  NV_BOOL     cube_attr_available[MAX_ABE_PFMS]; //!<  NVTrue is CUBE attributes are in the file
  NV_U_BYTE   pfm_alpha[MAX_ABE_PFMS];
  NV_INT32    last_saved_contour_record[MAX_ABE_PFMS]; //!<  Record number of the last record saved from the drawn contour file.
  NV_BOOL     contour_in_pfm[MAX_ABE_PFMS]; //!<  NVTrue if a drawn contour enters the PFM (temporary use)
} MISC;


#endif
