#ifndef _GEOSWATH3D_DEF_H_
#define _GEOSWATH3D_DEF_H_

#include "nvutility.h"
#include "nvutility.hpp"
#include "nvMapGL.hpp"

#include "pfm.h"
#include "pfm_extras.h"

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "gdal_priv.h"

#include <QtCore>
#include <QtGui>
#include <QSharedMemory>


#define         MARKER_W        15
#define         MARKER_H        10


//    Pointer interaction functions.

#define         NOOP                        0
#define         DELETE_RECTANGLE            1
#define         DELETE_POLYGON              2
#define         RESTORE_RECTANGLE           3
#define         RESTORE_POLYGON             4
#define         DELETE_POINT                5
#define         KEEP_POLYGON                6
#define         HOTKEY_POLYGON              7
#define         ROTATE                      8
#define         ZOOM                        9
#define         MASK_INSIDE_RECTANGLE       10
#define         MASK_OUTSIDE_RECTANGLE      11
#define         MASK_INSIDE_POLYGON         12
#define         MASK_OUTSIDE_POLYGON        13
#define         MEASURE                     14
#define         POLY_FILTER_MASK            15
#define         RECT_FILTER_MASK            16
#define         HIGHLIGHT_POLYGON           17
#define         CLEAR_POLYGON               18


#define         NUMSHADES                   100
#define         FREEZE_POPUP                3
#define         NUM_SCALE_LEVELS            16
#define         DEFAULT_SEGMENT_LENGTH      0.25
#define         CHRTRNULL                   10000000000000000.0
#define         EDIT_OPTIONS                7
#define         LINE_WIDTH                  2
#define         POLYGON_POINTS              NVMAPGL_POLYGON_POINTS
#define         NUM_TOOLBARS                6
#define         MAX_SLICE_SIZE              50
#define         MAX_TRANS_VALUE             64


//  NUMPOPUPS should always be one more than you think you're using in rightMouse because the last one 
//  (i.e. [NUMPOPUPS - 1]) is used for turning off slicing.  You also have to modify slotPopupMenu
//  because it checks this value for debug purposes.

#define         NUMPOPUPS                   8


//  Button hotkeys that are editable by the user.  These are used as indexes into the options.buttonAccel, misc.buttonText,
//  misc.button, and misc.buttonIcon arrays.  To add or remove from this list you need to change it here, in set_defaults.cpp
//  and in geoSwath3D.cpp (look for the tool buttons that have one of these defined names attached).

#define         OPEN_FILE_KEY               0
#define         QUIT_KEY                    1
#define         RESET_KEY                   2
#define         DELETE_POINT_MODE_KEY       3
#define         DELETE_RECTANGLE_MODE_KEY   4
#define         DELETE_POLYGON_MODE_KEY     5
#define         FILTER_KEY                  6
#define         RECTANGLE_FILTER_MASK_KEY   7
#define         POLYGON_FILTER_MASK_KEY     8
#define         RUN_HOTKEY_POLYGON_MODE_KEY 9
#define         UNDO_KEY                    10
#define         CLEAR_HIGHLIGHT_KEY         11
#define         HIGHLIGHT_POLYGON_MODE_KEY  12
#define         CLEAR_POLYGON_MODE_KEY      13

#define         HOTKEYS                     14


//  PROGRAM indices into the ancillary program arrays.  If you want to change this (i.e. add or subtract programs
//  it needs to be done here, in set_defaults.cpp, and add or subtract buttons in geoSwath3D.cpp.

#define         EXAMGSF                     0
#define         GSFMONITOR                  1
#define         WAVEFORMMONITOR             2
#define         LIDARMONITOR                3
#define         CHARTSPIC                   4
#define         WAVEWATERFALL_APD           5
#define         WAVEWATERFALL_PMT           6
#define         WAVEMONITOR                 7
#define         HOFRETURNKILL               8
#define         HOFRETURNKILL_SWA           9
#define         RMSMONITOR                  10
#define         DELETE_FILTER_HIGHLIGHTED   12
#define         DELETE_SINGLE_POINT         13
#define         FREEZE                      14
#define         FREEZE_ALL                  15

#define         NUMPROGS                    16


//  IHO order levels for minimum Z window size computation.

#define         IHO_SPECIAL_ORDER           1
#define         IHO_ORDER_1A                2
#define         IHO_ORDER_1B                3
#define         IHO_ORDER_2                 4


//  MAX_STACK_POINTS points nearest to the cursor.  MAX_STACK_POINTS is defined in ABE.h (utility library directory).

typedef struct
{
  NV_INT32      point[MAX_STACK_POINTS];
  NV_FLOAT64    dist[MAX_STACK_POINTS];
  NV_INT32      num_stack_points;
} NEAREST_STACK;


//  The OPTIONS structure contains all those variables that can be saved to the
//  users geoSwath3D QSettings.


typedef struct
{
  NV_INT32    position_form;              //  Position format number (MISC)
  QColor      edit_color;                 //  Color to be used for marker and edit rectangles and polygons
  QColor      tracker_color;              //  Color to be used for "other ABE window" tracker
  QColor      background_color;           //  Color to be used for background
  QColor      scale_color;                //  Color to be used for the "scale"
  QColor      waveColor[MAX_STACK_POINTS];//  Colors for point markers and waveforms used in waveMonitor and waveWaterfall 
  NV_INT32    flag_index;                 //  0 - no flag, 1 - flag suspect, 2 - flag invalid
  NV_FLOAT32  min_window_size;            //  Minimum window size in Z
  NV_INT32    iho_min_window;             //  If not 0 compute minimum window size using IHO order level (special=1, 1=2, 2=3)
  NV_BOOL     display_invalid;            //  Display invalid data flag
  NV_INT32    function;                   //  Active edit or zoom function
  NV_INT32    slice_percent;              //  Percent of "Z" depth to use for a rotated "slice" view
  NV_INT32    slice_alpha;                //  Alpha (transparency) value for data that falls outside of the slice (0-255).
  NV_FLOAT32  z_factor;                   //  Conversion factor for Z values.  May be used to convert 
                                          //  m to ft...
  NV_FLOAT32  z_offset;                   //  Offset value for Z values.
  NV_INT32    point_size;                 //  Point size in pixels
  NV_FLOAT32  rotation_increment;         //  Degrees to rotate when pressing the left and right arrow keys
  NV_FLOAT32  nearest_value;              //  Value of point nearest to the cursor when moving the cursor
  NV_INT32    point_limit;                //  Number of input points to be displayed per page
  NV_INT32    zoom_percent;               //  Zoom in/out percentage
  NV_FLOAT64  exaggeration;               //  Z exaggeration
  NV_FLOAT32  zx_rotation;                //  Rotation around the ZX axis to be saved for startup.
  NV_FLOAT32  y_rotation;                 //  Rotation around the Y axis to be saved for startup.
  NV_FLOAT32  filterSTD;                  //  Filter standard deviation
  NV_BOOL     deep_filter_only;           //  Filter only in the downward direction (assumes positive is depth)
  NV_BOOL     kill_and_respawn;           //  Set this to NVTrue to kill the ancillary programs on exit and 
                                          //  respawn them on restart.  If set to NVFalse then the programs will remain
                                          //  running and reconnect to geoSwath3D when it restarts.  This is a blanket
                                          //  setting and will be enforced regardless of the value of "state[]".
  NV_INT32    main_button_icon_size;      //  Main button icon size (16, 24, 32)
  NV_BOOL     draw_scale;                 //  Set this to draw the scale in the display.
  QString     buttonAccel[HOTKEYS];       //  Accelerator key sequences for some of the buttons
  QString     inputFilter;                //  Last used input file filter
  NV_INT32    undo_levels;                //  Number of undo levels


  //  These relate to the ancillary programs

  QString     prog[NUMPROGS];             //  Ancillary program command strings with keywords
  QString     name[NUMPROGS];             //  Ancillary program name
  QString     hotkey[NUMPROGS];           //  Ancillary programs hot key (the extra 2 are for the freeze options)
  QString     action[NUMPROGS];           //  Ancillary programs action hot keys
  QToolButton *progButton[NUMPROGS];      //  If this program is attached to a button, this is the button (otherwise NULL)
  NV_BOOL     data_type[NUMPROGS][PFM_DATA_TYPES]; //  The data types associated with an ancillary program
  NV_BOOL     hk_poly_eligible[NUMPROGS]; //  Whether the key is eligible to be used in a polygon
  NV_BOOL     hk_poly_only[NUMPROGS];     //  Whether the key can only be used for hotkey polygons
  NV_INT32    state[NUMPROGS];            //  0 - program does not respawn on restart, 1 - program respawns but was
                                          //  not up when geoSwath3D last closed, 2 - program respawns and was up
  NV_U_INT32  kill_switch[NUMPROGS];      //  Kill switch value to be used to terminate ancillary programs.
                                          //  This number will always be above 10,000,000.
  QString     description[NUMPROGS];      //  Ancillary program description
  QString     input_dir;                  //  Last directory searched for input files
} OPTIONS;


// The POINT_DATA structure contains variables related to points in the PFM file.

typedef struct
{
  NV_FLOAT64  *x;                         //  X position
  NV_FLOAT64  *y;                         //  Y position
  NV_FLOAT32  *z;                         //  Z value
  NV_U_INT32  *val;                       //  Validity
  NV_U_INT32  *oval;                      //  Original validity
  NV_U_INT32  *rec;                       //  Record (e.g. ping) number
  NV_INT32    *sub;                       //  Subrecord (e.g. beam) number
  NV_FLOAT64  *time;                      //  Time array pointer
  NV_BOOL     *mask;                      //  Whether we are hiding this point or not
  NV_BOOL     *fmask;                     //  Whether this point is filter masked or not
  NV_INT32    count;                      //  Total number of points in section
  NV_INT32    num_beams;                  //  Number of beams in swath
  NV_FLOAT64  x_grid_size;
  NV_FLOAT64  y_grid_size;
  NV_F64_XYMBC bounds;
} POINT_DATA;


// The UNDO block structure.

typedef struct
{
  NV_U_INT32  *val;                       //  Validity
  NV_U_INT32  *num;                       //  Point number in the POINT_DATA array
  NV_INT32    count;                      //  Number of points in this undo block
} UNDO;


// General stuff.

typedef struct
{
  QString     file;                       //  Input file name
  NV_BOOL     no_edit;                    //  Do not allow edits
  NV_BOOL     area_drawn;                 //  Area has been displayed flag
  NV_BOOL     drawing;                    //  Area is being drawn flag
  NV_INT32    save_function;              //  Saved active function when non-edit functions in use
  NV_INT32    save_mode;                  //  Saved active edit mode for exit (only delete functions)
  NV_INT32    num_records;                //  Number of records per section
  NV_INT32    num_subrecords;             //  Number of subrecords per record (actual number of points per section
                                          //  (e.g. 250 pings * 128 beams = 32,000 points) is stored in the data structure
                                          //  in data->count)
  NV_INT32    total_records;              //  Total number of records (pings, shots) in the file
  NV_INT32    start_record;               //  First record of the file (for GSF it's 1 for others usually 0)
  NV_INT32    end_record;                 //  Last record of the file
  NV_FLOAT64  start_time;                 //  Start time of the file
  NV_FLOAT64  end_time;                   //  End time of the file
  NV_INT32    data_type;                  //  File data type
  NV_INT32    poly_count;                 //  Polygon point count
  NV_FLOAT64  polygon_x[POLYGON_POINTS];  //  Polygon X
  NV_FLOAT64  polygon_y[POLYGON_POINTS];  //  Polygon Y
  NV_INT32    hotkey_poly_count;          //  hotkey polygon count
  NV_INT32    *hotkey_poly_x;             //  hotkey polygon X points (allocated)
  NV_INT32    *hotkey_poly_y;             //  hotkey polygon Y points (allocated)
  NV_INT32    rotate_angle;               //  Rotation of view angle at 1/10 degree
  NV_FLOAT32  ortho_scale;                //  X scale of rotated view 
  NV_INT32    ortho_angle;                //  Rotation of view angle at 1/10 degree
  NV_INT32    ortho_min;                  //  Minimum "Z" value when rotated
  NV_INT32    ortho_max;                  //  Maximum "Z" value when rotated
  NV_INT32    ortho_range;                //  Max "Z" - min "Z" when rotated
  NV_BOOL     slice;                      //  Set if we're "slicing" the data
  NV_INT32    slice_min;                  //  Min "Z" value in slice
  NV_INT32    slice_max;                  //  Max "Z" value in slice
  NV_INT32    slice_size;                 //  Size of slice in "Z" (0 = off)
  NV_FLOAT64  slice_meters;               //  Size of slice in meters
  NV_FLOAT32  sin_array[3600];            //  Pre-computed sine values at 1/10 degree
  NV_FLOAT32  cos_array[3600];            //  Pre-computed cosine values at 1/10 degree
  NV_INT32    highlight_count;            //  Number of highlighted points
  NV_INT32    *highlight;                 //  Highlighted points
  NV_BOOL     resized;                    //  resized flag
  NV_INT32    hotkey;                     //  function number if we just pushed a command hotkey (or -1)
  NV_BOOL     drawing_canceled;           //  set to cancel drawing
  QColor      track_color_array[NUMSHADES];//  Colors for points in the tracker map
  NV_U_BYTE   color_array[2][NUMSHADES][4];//  Colors for points (0 is full color, 1 is semi-transparent)
  NV_INT32    draw_area_width;            //  Width of map drawing area in pixels
  NV_INT32    draw_area_height;           //  Height of map drawing area in pixels
  NV_I32_COORD2 center_point;             //  center of drawing area in pixels
  NV_INT32    nearest_point;              //  point nearest to the cursor
  QIcon       buttonIcon[HOTKEYS];        //  Tool button icons for tools having accelerators
  QString     buttonText[HOTKEYS];        //  Tooltip text for the buttons that have editable accelerators
  QToolButton *button[HOTKEYS];           //  Buttons that have editable accelerators
  QString     shared_file;                //  Shared file name for hotkey polygons.
  QColor      widgetBackgroundColor;      //  The normal widget background color.
  QSharedMemory *abeShare;                //  ABE's shared memory pointer.
  ABE_SHARE   *abe_share;                 //  Pointer to the ABE_SHARE structure in shared memory.
  NV_F64_XYMBR displayed_area;            //  Currently displayed area
  NEAREST_STACK nearest_stack;            //  Nine points nearest to the cursor
  NV_FLOAT32  min_z;
  NV_FLOAT32  max_z;
  NV_BOOL     marker_mode;                //  Only used in DELETE_POINT mode:  0 - normal, 1 - center marker is frozen
                                          //  but others are normal (when waveMonitor is running), 2 - freeze all 
                                          //  markers (when waveMonitor is running).
  NV_INT32    frozen_point;               //  Use this instead of nearest point if we freeze the marker
  QString     qsettings_org;              //  Organization name for QSettings
  QString     qsettings_app;              //  Application name for QSettings
  NV_BOOL     mask_active;                //  Set if we are masking any data.
  NV_INT16    unique_type[PFM_DATA_TYPES];//  Unique data types being displayed.
  NV_INT16    unique_count;               //  Number of unique data types being displayed.
  QWidget     *map_widget;                //  Map widget
  NV_BOOL     filter_mask;                //  Set if we have any filtermasked points.
  NV_INT32    *filter_kill_list;          //  List of points that are set to be killed by the filter
  NV_INT32    filter_kill_count;          //  Number of points that are set to be killed by the filter
  NV_BOOL     filtered;                   //  NVTrue if area was filtered.
  NV_BOOL     hydro_lidar_present;        //  NVTrue if we have any Hydro LIDAR data.
  NV_BOOL     lidar_present;              //  NVTrue if we have any Hydro or Topo LIDAR data.
  NV_BOOL     gsf_present;                //  NVTrue if we have any GSF data.
  NV_INT32    process_id;                 //  This program's process ID
  QStatusBar  *progStatus;                //  Progress (and message) status bar
  QProgressBar *statusProg;               //  Generic progress bar in status area
  QLabel      *statusProgLabel;           //  Generic progress bar lchrtr2l
  QPalette    statusProgPalette;          //  Background color palette for the progress bar lchrtr2l
  NV_FLOAT64  map_center_x;               //  Center of displayed area (lon)
  NV_FLOAT64  map_center_y;               //  Center of displayed area (lat)
  NV_FLOAT64  map_center_z;               //  Center of displayed area (z)
  UNDO        *undo;                      //  The undo block array
  NV_INT32    undo_count;                 //  The number of undo blocks
  scaleBox    *scale[NUM_SCALE_LEVELS];
  QSharedMemory *abeRegister;             //  ABE's process register
  ABE_REGISTER *abe_register;             //  Pointer to the ABE_REGISTER structure in shared memory.
  NV_BOOL     linked;                     //  Set if we have linked to another ABE application.
} MISC;


NV_INT32 check_bounds (OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 index, NV_BOOL ck, NV_BOOL slice);
NV_INT32 check_bounds (OPTIONS *options, MISC *misc, POINT_DATA *data, NV_FLOAT64 x, NV_FLOAT64 y, NV_U_INT32 v, NV_BOOL mask, NV_BOOL ck, NV_BOOL slice);
void set_area (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 *x_bounds, NV_INT32 *y_bounds,
               NV_INT32 num_vertices, NV_INT32 rect_flag);
void keep_area (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 *x_bounds, NV_INT32 *y_bounds,
                NV_INT32 num_vertices);
void overlayData (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data);
void overlayFlag (nvMapGL *map, OPTIONS *options, MISC *misc, POINT_DATA *data);
void compute_ortho_values (nvMapGL *map, MISC *misc, OPTIONS *options, POINT_DATA *data, QScrollBar *sliceBar, NV_BOOL set_to_min);
void store_undo (MISC *misc, NV_INT32 undo_levels, NV_U_INT32 val, NV_U_INT32 num);
void end_undo_block (MISC *misc);
void undo (MISC *misc, POINT_DATA *data);
NV_BOOL resize_undo (MISC *misc, OPTIONS *options, NV_INT32 undo_levels);


#endif
