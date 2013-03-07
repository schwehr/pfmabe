
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



#ifndef _PFM_EDIT_3D_DEF_H_
#define _PFM_EDIT_3D_DEF_H_

#include "nvutility.h"
#include "nvutility.hpp"
#include "nvMapGL.hpp"

#include "pfm.h"
#include "pfm_extras.h"

#include "targetlib.h"
#include "MIW.h"

#include "binaryFeatureData.h"

#include "pfmWDBSecurity.hpp"

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <proj_api.h>

#include "gdal_priv.h"

#include <QtCore>
#include <QtGui>
#include <QSharedMemory>


#define         MARKER_W                    15
#define         MARKER_H                    10
#define         KILL_SWITCH_OFFSET          10000000


//    Pointer interaction functions.

#define         DELETE_RECTANGLE            1
#define         DELETE_POLYGON              2
#define         RESTORE_RECTANGLE           3
#define         RESTORE_POLYGON             4
#define         DELETE_POINT                5
#define         KEEP_POLYGON                6
#define         UNSET_SINGLE                7
#define         SET_MULTIPLE                8
#define         MOVE_FEATURE                9
#define         DELETE_FEATURE              10
#define         EDIT_FEATURE                11
#define         ADD_FEATURE                 12
#define         HOTKEY_POLYGON              13
#define         SET_REFERENCE               14
#define         UNSET_REFERENCE             15
#define         ROTATE                      16
#define         ZOOM                        17
#define         MASK_INSIDE_RECTANGLE       18
#define         MASK_OUTSIDE_RECTANGLE      19
#define         MASK_INSIDE_POLYGON         20
#define         MASK_OUTSIDE_POLYGON        21
#define         MEASURE                     22
#define         POLY_FILTER_MASK            23
#define         RECT_FILTER_MASK            24
#define	        AV_DISTANCE_TOOL            25
#define         HIGHLIGHT_POLYGON           26
#define         CLEAR_POLYGON               27


#define         NUMSHADES                   100
#define         FREEZE_POPUP                5
#define         NUM_SCALE_LEVELS            16
#define         DEFAULT_SEGMENT_LENGTH      0.25
#define         CHRTRNULL                   10000000000000000.0
#define         EDIT_OPTIONS                7
#define         FEATURE_OPTIONS             6
#define         LINE_WIDTH                  2
#define         POLYGON_POINTS              NVMAPGL_POLYGON_POINTS
#define         NUM_TOOLBARS                8
#define         PRE_ATTR                    4
#define         PRE_USER                    6
#define         MAX_SLICE_SIZE              50
#define         MAX_TRANS_VALUE             64
#define         CONTOUR_POINTS              1000
#define         NUM_HSV                     NUM_ATTR + PRE_ATTR  //!<  Possible number of scale box HSV settings 


/*!
  NUMPOPUPS should always be one more than you think you're using in rightMouse because the last one 
  (i.e. [NUMPOPUPS - 1]) is used for turning off slicing.  You also have to modify slotPopupMenu
  because it checks this value for debug purposes.
*/

#define         NUMPOPUPS                   15


/*!
  Button hotkeys that are editable by the user.  These are used as indexes into the options.buttonAccel, misc.buttonText,
  misc.button, and misc.buttonIcon arrays.  To add or remove from this list you need to change it here, in set_defaults.cpp
  and in pfmEdit3D.cpp (look for the tool buttons that have one of these defined names attached).  If the
  name has ACTION in it then it is a QAction from a menu.  In this case the misc.action item is set to point to the 
  QAction and the misc.button item is set to NULL (vice versa for QToolButtons).  This is how we tell them apart in the
  code.
*/

#define         SAVE_EXIT_KEY               0
#define         SAVE_EXIT_MASK_KEY          1
#define         NO_SAVE_EXIT_KEY            2
#define         RESET_KEY                   3
#define         DELETE_POINT_MODE_KEY       4
#define         DELETE_RECTANGLE_MODE_KEY   5
#define         DELETE_POLYGON_MODE_KEY     6
#define         FILTER_KEY                  7
#define         ATTR_FILTER_KEY             8
#define         RECTANGLE_FILTER_MASK_KEY   9
#define         POLYGON_FILTER_MASK_KEY     10
#define         RUN_HOTKEY_POLYGON_MODE_KEY 11
#define         EDIT_FEATURE_MODE_KEY       12
#define         UNDO_KEY                    13
#define         DISPLAY_MULTIPLE_KEY        14
#define         DISPLAY_ALL_KEY             15
#define         CLEAR_HIGHLIGHT_KEY         16
#define         HIGHLIGHT_POLYGON_MODE_KEY  17
#define         CLEAR_POLYGON_MODE_KEY      18
#define         TOGGLE_CONTOUR_KEY          19
#define         COLOR_BY_DEPTH_ACTION_KEY   20
#define         COLOR_BY_LINE_ACTION_KEY    21
#define         AV_DISTANCE_THRESHOLD_KEY   22


#define         HOTKEYS                     23


/*!
  PROGRAM indices into the ancillary program arrays.  If you want to change this (i.e. add or subtract programs
  it needs to be done here, in set_defaults.cpp, and add or subtract buttons in pfmEdit3D.cpp.
*/

#define         EXAMGSF                     0
#define         GSFMONITOR                  1
#define         WAVEFORMMONITOR             2
#define         UNISIPS                     3
#define         LIDARMONITOR                4
#define         HAWKEYEMONITOR              5
#define         CHARTSPIC                   6
#define         MOSAICVIEW                  7
#define         WAVEWATERFALL_APD           8
#define         WAVEWATERFALL_PMT           9
#define         CZMILWAVEMONITOR            10
#define         HOFRETURNKILL               11
#define         HOFRETURNKILL_SWA           12
#define         ATTRIBUTEVIEWER             13
#define         RMSMONITOR                  14
#define         HOFWAVEFILTER               15
#define         INVALIDATE_FEATURES         16
#define         ACCEPT_FILTER_HIGHLIGHTED   17
#define         REJECT_FILTER_HIGHLIGHTED   18
#define         DELETE_SINGLE_POINT         19
#define         FREEZE                      20
#define         FREEZE_ALL                  21

#define         NUMPROGS                    22


//!  IHO order levels for minimum Z window size computation.

#define         IHO_SPECIAL_ORDER           1
#define         IHO_ORDER_1A                2
#define         IHO_ORDER_1B                3
#define         IHO_ORDER_2                 4


/*!
  - enum:	AVInput

  - This enumeration will represent the types of input that can be fed to
    the attribute viewer
*/

enum AVInput
  {
    AV_AOI = 0,
    AV_DIST_THRESH = 1
  };


//!  MAX_STACK_POINTS points nearest to the cursor.  MAX_STACK_POINTS is defined in ABE.h (utility library directory).

typedef struct
{
  NV_INT32      point[MAX_STACK_POINTS];
  NV_FLOAT64    dist[MAX_STACK_POINTS];
  NV_INT32      num_stack_points;
} NEAREST_STACK;


//!  Attribute viewer information

typedef struct
{
  NV_INT32      av_rectangle;
  NV_INT32      avInterfacePixelBuffer;    //!<  represents the amount of pixels to move over
  NV_INT32      maxNumShots;
  AV_SHOT_INFO  *shotArray; 
  NV_I32_COORD2 avInterfaceCenterMousePt;
  NV_F64_COORD3 avInterfaceCenterLatLon;
  NV_FLOAT32    avBoxSize;
  NV_FLOAT32    actualMin;
  NV_FLOAT32    actualRange;
  NV_FLOAT32    attrMin;
  NV_FLOAT32    attrRange;
} AV_BUNDLE;


//!  The OPTIONS structure contains all those variables that can be saved to the users pfmEdit3D QSettings.

typedef struct
{
  NV_INT32    position_form;              //!<  Position format number
  NV_INT32    contour_width;              //!<  Contour line width/thickness in pixels
  QColor      contour_color;              //!<  Color to be used for contours
  QColor      edit_color;                 //!<  Color to be used for edit rectangles and polygons
  QColor      marker_color;               //!<  Color to be used for markers (highlighted points)
  QColor      ref_color[2];               //!<  Color to be used for reference and null data.
  QColor      tracker_color;              //!<  Color to be used for "other ABE window" tracker
  QColor      background_color;           //!<  Color to be used for background
  QColor      scale_color;                //!<  Color to be used for the "scale"
  QColor      waveColor[MAX_STACK_POINTS];//!<  Colors for point markers and waveforms used in CZMILwaveMonitor and waveWaterfall 
  NV_INT32    smoothing_factor;           //!<  Contour smoothing factor (0-10)
  QColor      feature_color;              //!<  Color to be used for features
  QColor      feature_info_color;         //!<  Color to be used for feature information text display
  QColor      feature_poly_color;         //!<  Color to be used for feature polygon display
  QColor      feature_highlight_color;    //!<  Color to be used for highlighted features (for text search from pfmView)
  QColor      verified_feature_color;     //!<  Color to be used for verified features (confidence = 5)
  NV_FLOAT32  feature_size;               //!<  Feature size
  NV_FLOAT32  min_window_size;            //!<  Minimum window size in Z
  NV_INT32    iho_min_window;             //!<  If not 0 compute minimum window size using IHO order level (special=1, 1=2, 2=3)
  NV_INT32    color_index;                /*!<  - Color by:
                                                    - 0 = color by depth
                                                    - 1 = color by line
                                                    - 2 = color by horizontal uncertainty
                                                    - 3 = color by vertical uncertainty
                                                    - 4 to 13 = color by numbered PFM attribute (1 through 10)
                                          */
  NV_INT32    flag_index;                 /*!<  - Flag data points:
                                                    - 0 = no flag
                                                    - 1 = flag suspect
                                                    - 2 = flag selected
                                                    - 3 = flag feature
                                                    - 4 = flag designated
                                                    - 5 = flag invalid
                                                    - 6 to 11 = flag PFM_USER_XX
                                          */
  NV_U_INT16  min_hsv_color[NUM_HSV];     //!<  HSV color wheel index for color to be used for minimum values (default 0)
  NV_U_INT16  max_hsv_color[NUM_HSV];     //!<  HSV color wheel index for color to be used for maximum values (default 315)
  NV_FLOAT32  min_hsv_value[NUM_HSV];     //!<  Locked minimum data value for minimum HSV color
  NV_FLOAT32  max_hsv_value[NUM_HSV];     //!<  Locked maximum data value for maximum HSV color
  NV_BOOL     min_hsv_locked[NUM_HSV];    //!<  Set if min HSV is locked to a value
  NV_BOOL     max_hsv_locked[NUM_HSV];    //!<  Set if max HSV is locked to a value
  NV_BOOL     display_contours;           //!<  Display contours flag
  NV_BOOL     display_man_invalid;        //!<  Display manually invalid data flag
  NV_BOOL     display_flt_invalid;        //!<  Display filter invalid data flag
  NV_BOOL     display_null;               //!<  Display null data flag
  NV_INT32    display_feature;            /*!<  - Display features:
                                                    - 0 = no features
                                                    - 1 = all features
                                                    - 2 = unverified features
                                                    - 3 = verified features
                                          */
  NV_BOOL     display_children;           //!<  Flag to show grouped feature children locations
  NV_BOOL     display_reference;          //!<  Display reference data
  NV_BOOL     display_feature_info;       //!<  Display feature info
  NV_BOOL     display_feature_poly;       //!<  Display feature polygon
  NV_INT32    function;                   //!<  Active edit or zoom function
  NV_INT32    slice_percent;              //!<  Percent of "Z" depth to use for a rotated "slice" view
  NV_INT32    slice_alpha;                //!<  Alpha (transparency) value for data that falls outside of the slice (0-255).
  NV_FLOAT32  z_factor;                   /*!<  Conversion factor for Z values.  May be used to convert 
                                                m to ft...  Passed from pfm_view.  */
  NV_FLOAT32  z_offset;                   //!<  Offset value for Z values.  Passed from pfm_view.
  NV_INT32    point_size;                 //!<  Point size in pixels
  NV_FLOAT32  rotation_increment;         //!<  Degrees to rotate when pressing the left and right arrow keys
  QString     unload_prog;                //!<  Unload program name
  NV_BOOL     auto_unload;                /*!<  Set if we wish to automatically unload edits to input files
                                                at the end of the session  */
  NV_INT32    last_rock_feature_desc;     //!<  Index of last used rock, wreck, obstruction feature description
  NV_INT32    last_offshore_feature_desc; //!<  Index of last used offshore installation feature description
  NV_INT32    last_light_feature_desc;    //!<  Index of last used light, buoy, beacon feature description
  NV_INT32    last_lidar_feature_desc;    //!<  Index of last used lidar feature description
  QString     last_feature_description;   //!<  String containing the last feature descriptor used
  QString     last_feature_remarks;       //!<  String containing the last feature remarks used
  NV_FLOAT32  nearest_value;              //!<  Value of point nearest to the cursor when moving the cursor
  NV_INT32    sparse_limit;               //!<  Number of input points to exceed before we zoom and rotate sparse data
  NV_INT32    zoom_percent;               //!<  Zoom in/out percentage
  NV_FLOAT32  exaggeration;               //!<  Z exaggeration
  NV_INT32    screenshot_delay;           //!<  Delay (in seconds) of the screenshot in the editFeature dialog.
  NV_FLOAT32  zx_rotation;                //!<  Rotation around the ZX axis to be saved for startup.
  NV_FLOAT32  y_rotation;                 //!<  Rotation around the Y axis to be saved for startup.
  QString     feature_dir;                //!<  Directory that contains feature file.
  NV_FLOAT32  filterSTD;                  //!<  Filter standard deviation
  NV_BOOL     deep_filter_only;           //!<  Filter only in the downward direction (assumes positive is depth)
  NV_FLOAT32  feature_radius;             //!<  Filter exclusion distance for features
  NV_INT32    overlap_percent;            //!<  Percentage of overlap to use for Ctrl-arrow key moves
  NV_BOOL     kill_and_respawn;           /*!<  Set this to NVTrue to kill the ancillary programs on exit and 
                                                respawn them on restart.  If set to NVFalse then the programs will remain
                                                running and reconnect to pfmEdit3D when it restarts.  This is a blanket
                                                setting and will be enforced regardless of the value of "state[]".  */
  NV_INT32    main_button_icon_size;      //!<  Main button icon size (16, 24, 32)
  NV_BOOL     draw_scale;                 //!<  Set this to draw the scale in the display.
  NV_INT32    avInterfaceBoxSize;         //!<  Box size of the Attribute Viewer interface can be small, medium or large
  QString     buttonAccel[HOTKEYS];       //!<  Accelerator key sequences for some of the buttons or actions
  NV_INT32    undo_levels;                //!<  Number of undo levels
  NV_BOOL     confirm_filter;             //!<  Whether or not to confirm filter kills
  NV_FLOAT32  attr_filter_range[NUM_ATTR + PRE_ATTR][2]; /*!<  Min and max values for attribute filter.  Note that we save a
                                                               spot for depth and line number but they're not used.  This
                                                               makes the code a bit simpler.  */


  //  These relate to the ancillary programs

  QString     prog[NUMPROGS];             //!<  Ancillary program command strings with keywords
  QString     name[NUMPROGS];             //!<  Ancillary program name
  QString     hotkey[NUMPROGS];           //!<  Ancillary programs hot key (the extra 2 are for the freeze options)
  QString     action[NUMPROGS];           //!<  Ancillary programs action hot keys
  QToolButton *progButton[NUMPROGS];      //!<  If this program is attached to a button, this is the button (otherwise NULL)
  NV_BOOL     data_type[NUMPROGS][PFM_DATA_TYPES]; //!<  The data types associated with an ancillary program
  NV_BOOL     hk_poly_eligible[NUMPROGS]; //!<  Whether the key is eligible to be used in a polygon
  NV_INT16    hk_poly_filter[NUMPROGS];   //!<  Whether the key can only be used for hotkey polygons (1) or external filter (2), otherwise 0
  NV_INT32    state[NUMPROGS];            /*!<  - Ancillary program state:
                                                    - 0 = program does not respawn on restart
                                                    - 1 = program respawns but was not up when pfmEdit3D last closed
                                                    - 2 = program respawns and was up
                                          */
  NV_U_INT32  kill_switch[NUMPROGS];      /*!<  Kill switch value to be used to terminate "kill and respawn" ancillary programs.
                                                This number will always be above 10,000,000.  */
  QString     description[NUMPROGS];      //!<  Ancillary program description
  NV_INT32    drawingMode;                //!<  0: point mode, 2: spheres  (ability to support more is there)
  NV_FLOAT32  objectWidth;
  NV_INT32    objectDivisionals;
  NV_FLOAT32  distThresh;
  NV_BOOL     auto_scale;                 //!<  NVTrue if we're auto-scaling when we mask or do other operations that show or hide data.
  NV_FLOAT64  hofWaveFilter_search_radius;//!<  hofWaveFilter search radius in meters
  NV_INT32    hofWaveFilter_search_width; //!<  Number of points to search before and after selected point on nearby waveforms
  NV_INT32    hofWaveFilter_rise_threshold;//!<  Number of rise points needed as corroborating data on nearby waveforms
  NV_INT32    hofWaveFilter_pmt_ac_zero_offset_required;/*!<  If selected point is less than this amount above the
                                                              PMT AC zero offset the point will be marked invalid  */
  NV_INT32    hofWaveFilter_apd_ac_zero_offset_required;/*!<  If selected point is less than this amount above the
                                                              APD AC zero offset the point will be marked invalid  */
} OPTIONS;


//! The UNDO block structure.

typedef struct
{
  NV_U_INT32  *val;                       //!<  Validity
  NV_U_INT32  *num;                       //!<  Point number in the POINT_CLOUD array
  NV_INT32    count;                      //!<  Number of points in this undo block
} UNDO;


//! General stuff.

typedef struct
{
  NV_BOOL     area_drawn;                 //!<  Area has been displayed flag
  NV_BOOL     busy;                       //!<  Busy drawing or running external process
  NV_INT32    save_function;              //!<  Saved active function when non-edit functions in use
  NV_INT32    save_mode;                  //!<  Saved active edit mode for exit (only delete functions)
  NV_INT32    num_records;                //!<  Number of records per section
  NV_INT32    poly_count;                 //!<  Polygon point count
  NV_FLOAT64  polygon_x[POLYGON_POINTS];  //!<  Polygon X
  NV_FLOAT64  polygon_y[POLYGON_POINTS];  //!<  Polygon Y
  NV_INT32    hotkey_poly_count;          //!<  hotkey polygon count
  NV_INT32    *hotkey_poly_x;             //!<  hotkey polygon X points (allocated)
  NV_INT32    *hotkey_poly_y;             //!<  hotkey polygon Y points (allocated)
  NV_INT32    rotate_angle;               //!<  Rotation of view angle at 1/10 degree
  NV_FLOAT32  ortho_scale;                //!<  X scale of rotated view 
  NV_INT32    ortho_angle;                //!<  Rotation of view angle at 1/10 degree
  NV_INT32    ortho_min;                  //!<  Minimum "Z" value when rotated
  NV_INT32    ortho_max;                  //!<  Maximum "Z" value when rotated
  NV_INT32    ortho_range;                //!<  Max "Z" - min "Z" when rotated
  NV_BOOL     slice;                      //!<  Set if we're "slicing" the data
  NV_INT32    slice_min;                  //!<  Min "Z" value in slice
  NV_INT32    slice_max;                  //!<  Max "Z" value in slice
  NV_INT32    slice_size;                 //!<  Size of slice in "Z" (0 = off)
  NV_FLOAT64  slice_meters;               //!<  Size of slice in meters
  NV_FLOAT32  sin_array[3600];            //!<  Pre-computed sine values at 1/10 degree
  NV_FLOAT32  cos_array[3600];            //!<  Pre-computed cosine values at 1/10 degree
  NV_INT32    line_count;                 //!<  Number of lines in the displayed area
  NV_INT32    line_number[PFM_MAX_FILES]; //!<  Original line numbers of lines
  NV_INT32    line_type[PFM_MAX_FILES];   //!<  Original data types of lines
  NV_INT32    line_interval;              //!<  Interval between line colors
  NV_INT32    num_lines;                  //!<  Number of lines to display, 0-all
  NV_INT32    line_num[PFM_MAX_FILES];    //!<  Line numbers to display after selection
  NV_INT32    highlight_count;            //!<  Number of highlighted points
  NV_INT32    *highlight;                 //!<  Highlighted points
  NV_INT32    maxd;                       //!<  Maximum contour density per grid cell
  NV_BOOL     reference_flag;             //!<  True if there is reference data present
  NV_INT32    nearest_feature;            //!<  Nearest feature to the cursor.
  NV_BOOL     resized;                    //!<  resized flag
  NV_INT32    hotkey;                     //!<  function number if we just pushed a command hotkey (or -1)
  BFDATA_SHORT_FEATURE *feature;          //!<  Feature (target) data array
  NV_INT32    bfd_handle;                 //!<  BFD (target) file handle
  BFDATA_HEADER  bfd_header;              //!<  Header for the current BFD file
  BFDATA_RECORD  new_record;              //!<  Record built when adding a new feature
  NV_BOOL     bfd_open;                   //!<  Set if we have an open bfd file
  NV_INT32    visible_feature_count;      //!<  visible feature count
  NV_BOOL     feature_mod;                //!<  If feature data was modified
  NV_BOOL     drawing_canceled;           //!<  set to cancel drawing
  NV_U_BYTE   color_array[3][NUMSHADES][4];//!<  Colors for points (0 is full color, 1 is semi-transparent);
  QColor      track_color_array[NUMSHADES];//!<  Colors for points in the tracker map
  NV_U_BYTE   line_color_array[2][NUMSHADES * 2][4];/*!<  Colors for points in color by line mode (0 is full color, 1 is semi-transparent, 
                                                          first half is full value, second half is half value - darker)  */
  NV_INT32    draw_area_width;            //!<  Width of map drawing area in pixels
  NV_INT32    draw_area_height;           //!<  Height of map drawing area in pixels
  NV_I32_COORD2 center_point;             //!<  center of drawing area in pixels
  NV_INT32    nearest_point;              //!<  point nearest to the cursor
  QIcon       buttonIcon[HOTKEYS];        //!<  Tool button icons for tools having accelerators
  QString     buttonText[HOTKEYS];        //!<  Tooltip text for the buttons that have editable accelerators
  QToolButton *button[HOTKEYS];           //!<  Buttons that have editable accelerators (or NULL)
  QAction     *action[HOTKEYS];           //!<  QActions that have editable accelerators (or NULL)
  QString     html_help_tag;              /*!<  Help tag for the currently running section of code.  This comes from
                                                the PFM_ABE html help documentation.  */
  QString     shared_file;                //!<  Shared file name for hotkey polygons.
  QColor      widgetBackgroundColor;      //!<  The normal widget background color.
  NV_INT32    feature_polygon_flag;
  QSharedMemory *abeShare;                //!<  ABE's shared memory pointer.
  ABE_SHARE   *abe_share;                 //!<  Pointer to the ABE_SHARE structure in shared memory.
  QSharedMemory *dataShare;               //!<  Point cloud shared memory.
  POINT_CLOUD *data;                      /*!<  Pointer to POINT_CLOUD structure in point cloud shared memory.  To see what is in the 
                                                POINT_CLOUD structure please see the ABE.h file in the nvutility library.  */
  NV_F64_XYMBR displayed_area;            //!<  Currently displayed area
  NEAREST_STACK nearest_stack;            //!<  Nine points nearest to the cursor
  NV_FLOAT64  x_grid_size;                //!<  X grid spacing (degrees) for contours
  NV_FLOAT64  y_grid_size;                //!<  Y grid spacing (degrees) for contours
  NV_FLOAT32  min_z;
  NV_FLOAT32  max_z;
  NV_BOOL     marker_mode;                /*!<  - Marker mode (only used in DELETE_POINT mode):
                                                    - 0 = normal
                                                    - 1 = center marker is frozen but others are normal (when CZMILwaveMonitor is running)
                                                    - 2 = freeze all markers (when CZMILwaveMonitor is running).
                                          */
  NV_INT32    frozen_point;               //!<  Use this instead of nearest point if we freeze the marker
  QString     qsettings_org;              //!<  Organization name for QSettings
  QString     qsettings_app;              //!<  Application name for QSettings
  NV_BOOL     need_sparse;                //!<  Set to NVTrue if we have more than options.sparse_limit points to display.
  NV_BOOL     mask_active;                //!<  Set if we are masking any data.
  NV_INT16    unique_type[PFM_DATA_TYPES];//!<  Unique data types being displayed.
  NV_INT16    unique_count;               //!<  Number of unique data types being displayed.
  NV_INT32    add_feature_index;          //!<  "data" array index if we added or moved a feature
  NV_INT32    nearest_feature_point;      /*!<  "data" array index of the nearest point to a feature that we are moving.
                                                 We use this to try to unset PFM_SELECTED_FEATURE on the point after we
                                                 move the feature.  */
  QString     help_browser;               //!<  Browser command name
  QWidget     *map_widget;                //!<  Map widget
  NV_BOOL     filter_mask;                //!<  Set if we have any filtermasked points.
  NV_INT32    *filter_kill_list;          //!<  List of points that are set to be killed by the filter
  NV_INT32    filter_kill_count;          //!<  Number of points that are set to be killed by the filter
  NV_BOOL     filtered;                   //!<  NVTrue if area was filtered.
  NV_INT32    *av_dist_list;              //!<  List of points that are highlighted when using the AV_DISTANCE_TOOL
  NV_INT32    av_dist_count;              //!<  Number of points that highlight when using the AV_DISTANCE_TOOL
  NV_BOOL     hydro_lidar_present;        //!<  NVTrue if we have any Hydro LIDAR data.
  NV_BOOL     lidar_present;              //!<  NVTrue if we have any Hydro or Topo LIDAR data.
  NV_BOOL     hof_present;                //!<  NVTrue if we have any CHARTS HOF data
  NV_BOOL     gsf_present;                //!<  NVTrue if we have any GSF data.
  NV_INT32    process_id;                 //!<  This program's process ID
  QStatusBar  *progStatus;                //!<  Progress (and message) status bar
  QProgressBar *statusProg;               //!<  Generic progress bar in status area
  QLabel      *statusProgLabel;           //!<  Generic progress bar label
  QPalette    statusProgPalette;          //!<  Background color palette for the progress bar label
  NV_FLOAT64  map_center_x;               //!<  Center of displayed area (lon)
  NV_FLOAT64  map_center_y;               //!<  Center of displayed area (lat)
  NV_FLOAT64  map_center_z;               //!<  Center of displayed area (z)
  UNDO        *undo;                      //!<  The undo block array
  NV_INT32    undo_count;                 //!<  The number of undo blocks
  NV_INT32    time_attr;                  /*!<  If this is set to >= 0 then this is the attribute number for the 
                                                minutes from 01/01/1970 used in PFMWDB and will be translated accordingly  */
  NV_INT32    datum_attr;                 /*!<  If this is set to >= 0 then this is the attribute number for the 
                                                vertical datum number (from gsf.h) used in PFMWDB and will be translated accordingly  */
  NV_INT32    ellipsoid_attr;             /*!<  If this is set to >= 0 then this is the attribute number for the 
                                                ellipsoid offset used in PFMWDB and will be translated accordingly  */
  scaleBox    *scale[NUM_SCALE_LEVELS];
  NV_F64_XYMBR orig_bounds;               //!<  Starting bounds of area to be viewed
  NV_F64_XYMBC bounds;                    //!<  Currently visible bounds
  NV_F64_XYMBC aspect_bounds;             //!<  Bounds that are passed to nvMapGL in order to keep the proper aspect ratio
  NV_FLOAT32   color_min_z;               //!<  Minimum value used for color range
  NV_FLOAT32   color_max_z;               //!<  Maximum value used for color range
  NV_FLOAT32   color_range_z;             //!<  Color range
  NV_FLOAT32   attr_color_min;            //!<  Minimum value used for attribute color range
  NV_FLOAT32   attr_color_max;            //!<  Maximum value used for attribute color range
  NV_FLOAT32   attr_color_range;          //!<  Attribute color range
  NV_FLOAT64   avg_bin_size_meters;       //!<  Average bin size in meters of all opened PFMs
  NV_FLOAT64   avg_x_bin_size_degrees;    //!<  Average X bin size in degrees of all opened PFMs
  NV_FLOAT64   avg_y_bin_size_degrees;    //!<  Average Y bin size in degrees of all opened PFMs
  projPJ       pj_utm;                    //!<  Proj4 UTM projection structure
  projPJ       pj_latlon;                 //!<  Proj4 latlon projection structure
  AV_BUNDLE    avb;                       //!<  attributeViewer data structure
  enum AV_SHARE_ACTION performingAction;  //!<  Need a variable to distinguish between actions that are being performed so slotMouseMove can navigate correctly
  AVInput      avInput;                   //!<  AV AOI or AV Distance Threshold tool
  NV_FLOAT32  attr_min;                   //!<  Attribute minimum value (if coloring by attribute)
  NV_FLOAT32  attr_max;                   //!<  Attribute maximum value (if coloring by attribute)


  //  The following concern PFMs as layers.  There are a few things from ABE_SHARE that also need to be 
  //  addressed when playing with layers - open_args, display_pfm, and pfm_count.

  NV_F64_XYMBC total_mbr;                 //!<  MBR of all of the displayed PFMs
  NV_U_INT16  max_attr;                   //!<  Maximum number of index attributes in any of the PFM files
  NV_I32_COORD2 ll[MAX_ABE_PFMS];         //!<  lower left coordinates for get_buffer and put_buffer
  NV_I32_COORD2 ur[MAX_ABE_PFMS];         //!<  upper right coordinates for get_buffer and put_buffer
  NV_FLOAT32  null_val[MAX_ABE_PFMS];     //!<  Null depth value
  NV_CHAR     attr_format[MAX_ABE_PFMS][NUM_ATTR][20]; //!<  Formats to use for displaying attributes
  NV_INT32    pfm_handle[MAX_ABE_PFMS];   //!<  PFM file handle
} MISC;


NV_INT32 check_bounds (OPTIONS *options, MISC *misc, NV_INT32 index, NV_BOOL ck, NV_BOOL slice);
NV_INT32 check_bounds (OPTIONS *options, MISC *misc, NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT32 z, NV_U_INT32 v, NV_BOOL mask, NV_INT16 pfm,
                       NV_BOOL ck, NV_BOOL slice);
NV_BOOL check_line (MISC *misc, NV_INT32 line);
void set_area (nvMapGL *map, OPTIONS *options, MISC *misc, NV_INT32 *x_bounds, NV_INT32 *y_bounds, NV_INT32 num_vertices, NV_INT32 rect_flag);
void keep_area (nvMapGL *map, OPTIONS *options, MISC *misc, NV_INT32 *x_bounds, NV_INT32 *y_bounds, NV_INT32 num_vertices);
void overlayFlag (nvMapGL *map, OPTIONS *options, MISC *misc);
void overlayData (nvMapGL *map, OPTIONS *options, MISC *misc);
void compute_ortho_values (nvMapGL *map, MISC *misc, OPTIONS *options, QScrollBar *sliceBar, NV_BOOL set_to_min);
void store_undo (MISC *misc, NV_INT32 undo_levels, NV_U_INT32 val, NV_U_INT32 num);
void end_undo_block (MISC *misc);
void undo (MISC *misc);
NV_BOOL resize_undo (MISC *misc, OPTIONS *options, NV_INT32 undo_levels);


#endif
