
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

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef _ABE_H_
#define _ABE_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include "nvtypes.h"
#include "nvdef.h"
#include "pfm.h"


#define         MAX_CONTOUR_LEVELS          200  /*!<  Maximum number of contour levels                                */
#define         MAX_STACK_POINTS            9    /*!<  Maximum stack points (for waveWaterfall from pfmEdit)           */
#define         MAX_ABE_PFMS                16   /*!<  Maximum number of PFM layers for pfmView and pfmEdit            */
#define         AV_NUM_PARAMETERS           4    /*!<  Attribute Viewer number of parameters (LIDAR specific).         */
#define         MAX_ATTRIBUTE_SHOTS         2000 /*!<  Maximum number of shots for the attribute viewer.               */
#define         MAX_DELETE_FILE_QUEUE       100  /*!<  Maximum number of files allowed on the delete file queue.       */
#define         OTF_GRID_MAX                2147483648



  /*  Command codes for ABE programs.  */

#define         STUPID_WINDOZE_ID           6666 /*!<  Used in Windows if there is no user ID available.  The ID
                                                       of the beast ;-)                                                */
#define         CHARTSPIC_SAVED_PICTURE     6969 /*!<  The chartsPic program sets this flag when it has saved a 
                                                       JPG for pfmEdit                                                 */
#define         PFMEDIT_FORCE_SHARE         6970 /*!<  Returned from tofWaterKill when changes have been made          */
#define         WAVEMONITOR_FORCE_REDRAW    6971 /*!<  Force waveMonitor to reread and redraw its display              */
#define         WAIT_FOR_START              6972 /*!<  chartsPic, mosaicView, and pfm3D set this when they have        */
                                                 /*!<  finished initializing                                           */
#define         MOSAICVIEW_FORCE_RELOAD     6973 /*!<  Force mosaicView to reload and redraw its display (changed
                                                       mosaic files)                                                   */
#define         FEATURE_FILE_MODIFIED       6974 /*!<  Let ancillary and related programs know that we have changed
                                                       feature files                                                   */
#define         CHILD_PROCESS_FORCE_EXIT    6975 /*!<  Force all children to exit (this must be checked without
                                                       attempting to lock shared memory)                               */
#define         PFM_LAYERS_CHANGED          6978 /*!<  Set whenever pfmView or pfmEdit change PFM layers, display of
                                                       PFM layers, or order of PFM layers (for multiple PFM display
                                                       in pfmView or pfmEdit)                                          */
#define         PFM3D_FORCE_RELOAD          6979 /*!<  Force pfm3D to reload and redraw its display (changed display   */
                                                 /*!<  in pfmView).                                                    */
#define         ANCILLARY_FORCE_EXIT        6980 /*!<  Force clean exit in pfmEdit ancillary programs                  */
#define         PFMVIEW_FORCE_EDIT          6981 /*!<  Force pfmView to edit the area defined in pfm3D                 */
#define         PFMVIEW_FORCE_EDIT_3D       6982 /*!<  Force pfmView to edit the area defined in pfm3D                 */
#define         PFMEDIT3D_OPENED            6983 /*!<  Set when pfmEdit3D starts.  Used to clear pfm3D memory.         */
#define         PFMEDIT3D_CLOSED            6984 /*!<  Set when pfmEdit3D exits.  Used to reload pfm3D memory.         */
#define         PFMEDIT_KILL                6985 /*!<  Set to gracefully kill pfmEdit(3D).                             */
#define         NO_ACTION_REQUIRED          9999 /*!<  No action required, duh                                         */




  /*!  Attribute viewer shot info (LIDAR specific).  */

  typedef struct
  {
    NV_INT32      pfmHandle;
    NV_INT16      fileNo;
    NV_U_INT32    recordNumber;
    NV_INT32      subRecordNumber;
    NV_INT32      masterIdx;
    NV_INT32      colorH;
    NV_INT32      colorS;
    NV_INT32      colorV;
    NV_INT16      type;
  } AV_SHOT_INFO;


  /*!  Structure containing all Attribute Viewer "need-to-knows" that pfmEdit and the Attribute Viewer
       AV will communicate with.  */

  enum AV_SHARE_ACTION {AVA_NONE, AVA_HIGHLIGHT, AVA_DELETE, AVA_COLOR_LINK} ;

  typedef struct
  {
    NV_BOOL       avRunning;
    NV_BOOL       avNewData;
    NV_BOOL       avTracking;
    NV_BOOL       hitMax;
    AV_SHOT_INFO  shots[MAX_ATTRIBUTE_SHOTS];
    NV_INT32      numShots;
    NV_FLOAT64    highlightLatitude;
    NV_FLOAT64    highlightLongitude;
    enum          AV_SHARE_ACTION action;
    NV_INT32      actionIdxs[MAX_ATTRIBUTE_SHOTS + 1];
    NV_INT32      numActionIdxs;
  } AV_SHARE;


  /*! 
     This structure contains all of the information needed to process the multiwaveform data being collaborated 
     by pfmEdit and waveMonitor.  It is also used by geoSwath, chartsSwap, and chartsPic so be careful if you
     change things here.  The arrays (with MAX_STACK_POINTS items) contain data that is associated with the nearest
     point to the cursor in pfmEdit (or geoSwath).  The [0] point is usually the closest to the cursor of the
     MAX_STACK_POINTS points.
  */


  typedef struct 
  {
    NV_INT32      multiNum;                            /*!<  currently not used                                        */
    NV_F64_COORD3 multiPoint[MAX_STACK_POINTS];        /*!<  3D coordinate                                             */
    NV_INT32      multiFile[MAX_STACK_POINTS];         /*!<  file descriptor                                           */
    NV_U_INT32    multiRecord[MAX_STACK_POINTS];       /*!<  record descriptor                                         */
    NV_INT32      multiSubrecord[MAX_STACK_POINTS];    /*!<  subrecord descriptor                                      */
    NV_INT32      multiPfm[MAX_STACK_POINTS];          /*!<  PFM descriptor                                            */
    NV_INT32      multiLine[MAX_STACK_POINTS];         /*!<  line descriptor                                           */
    NV_INT32      multiFlightlineIndex[MAX_STACK_POINTS];/*!<  index number of the flightline so a proper color can be
                                                               used for waveforms in color by flightline mode          */
    NV_INT32      multiPresent[MAX_STACK_POINTS];      /*!<  is waveform present?                                      */
    NV_INT32      multiLocation[MAX_STACK_POINTS];     /*!<  location within swath                                     */
    NV_INT16      multiType[MAX_STACK_POINTS];         /*!<  data type of point                                        */
    NV_C_RGBA     multiColors[MAX_STACK_POINTS];       /*!<  color struct for multi-waveforms                          */
    NV_U_INT32    multiNearRecord;                     /*!<  currently not used                                        */
    NV_INT32      multiMode;                           /*!<  multiple waveform mode (0 - nearest neighbor, 1 - single) */
    NV_BOOL       multiSwitch;                         /*!<  have we changed multi-modes?                              */
    NV_BOOL       waveMonitorRunning;                  /*!<  Set if waveDisplay program is running                     */
  } MULTIWAVE_SHARE;



  typedef struct
  {
    NV_FLOAT64    search_radius;                       /*!<  Search radius in meters                                   */
    NV_INT32      search_width;                        /*!<  Number of points to search before and after selected 
                                                             point on nearby waveforms                                 */
    NV_INT32      rise_threshold;                      /*!<  Number of rise points needed as corroborating data on 
                                                             nearby waveforms                                          */
    NV_INT32      pmt_ac_zero_offset_required;         /*!<  If selected point is less than this amount above the
                                                             PMT AC zero offset the point will be marked invalid       */
    NV_INT32      apd_ac_zero_offset_required;         /*!<  If selected point is less than this amount above the
                                                             APD AC zero offset the point will be marked invalid       */
  } FILTER_SHARE;


  /*  This is the On-The-Fly (OTF) grid record structure used in pfmView and pfm3D.  */

  typedef struct
  {
    NV_U_INT32    cnt;                       /*!<  Number of VALID soundings in the bin (set to greater than
                                                   OTF_GRID_MAX to force a recompute while loading).                   */
    NV_FLOAT32    min;                       /*!<  Minimum Z.                                                          */
    NV_FLOAT32    max;                       /*!<  Maximum Z.                                                          */
    NV_FLOAT32    avg;                       /*!<  Average Z (used as sum while loading).                              */
    NV_FLOAT32    std;                       /*!<  Standard deviation (used as sum of squares while loading).          */
  } OTF_GRID_RECORD;


  /* The POINT_CLOUD structure defines the values stored in the point cloud in pfmEdit or pfmEdit3D.  The actual     */
  /* data is stored in a separate shared memory structure that is named parent process ID (ppid in the ABE_SHARE     */
  /* structure) combined with "_abe_pfmEdit" (e.g.35467_abe_pfmEdit).                                                */

  typedef struct
  {
    NV_FLOAT64    x;                         /*!<  X position                                                            */
    NV_FLOAT64    y;                         /*!<  Y position                                                            */
    NV_FLOAT32    z;                         /*!<  Z value                                                               */
    NV_FLOAT32    herr;                      /*!<  Horizontal error                                                      */
    NV_FLOAT32    verr;                      /*!<  Vertical error                                                        */
    NV_U_INT32    val;                       /*!<  Validity                                                              */
    NV_U_INT32    oval;                      /*!<  Original validity                                                     */
    NV_INT16      pfm;                       /*!<  PFM number                                                            */
    NV_INT16      file;                      /*!<  File number                                                           */
    NV_INT32      line;                      /*!<  Line number (uses 32 bits because we're kludging it with the PFM 
                                                   number to make it unique)                                             */
    NV_U_INT32    rec;                       /*!<  Record (e.g. ping) number                                             */
    NV_INT32      sub;                       /*!<  Subrecord (e.g. beam) number                                          */
    NV_INT16      type;                      /*!<  Data type                                                             */
    NV_INT64      addr;                      /*!<  Depth record block address                                            */
    NV_U_BYTE     pos;                       /*!<  Depth record address position                                         */
    NV_INT32      xcoord;                    /*!<  X coordinate of the bin cell                                          */
    NV_INT32      ycoord;                    /*!<  Y coordinate of the bin cell                                          */
    NV_BOOL       exflag;                    /*!<  Externally flagged point                                              */
    NV_BOOL       mask;                      /*!<  Whether we are hiding this point or not                               */
    NV_BOOL       fmask;                     /*!<  Whether this point is filter masked or not                            */
    NV_FLOAT32    attr[NUM_ATTR];            /*!<  Optional attributes                                                   */
  } POINT_CLOUD;




  /*********************************************************************************************************************/
  /*  This structure (ABE_SHARE) is shared between ABE programs for inter-process communication (IPC).  */
  /*********************************************************************************************************************/

  typedef struct
  {
    NV_BOOL       settings_changed;           /*!<  Set if pfmEdit has changed any of the shared settings                */
    NV_BOOL       zoom_requested;             /*!<  Set if pfmEdit has requested a mosaicView zoom (so pfmView can 
                                                    inform any linked apps of the zoom request).                         */
    NV_INT32      position_form;              /*!<  Position format number (position formats are described in MISC)      */
    NV_INT32      smoothing_factor;           /*!<  Contour smoothing factor (0-10)                                      */
    NV_FLOAT32    z_factor;                   /*!<  Z scaling factor                                                     */
    NV_FLOAT32    z_offset;                   /*!<  Z offset value                                                       */
    NV_CHAR       snippet_file_name[512];     /*!<  Snippet file name that chartsPic will use to store a feature image
                                                    from HOF or TOF files (jpg from the .img file, see 
                                                    CHARTSPIC_SAVED_PICTURE above)                                       */
    NV_FLOAT32    heading;                    /*!<  Heading from chartsPic for use in displaying the snippet.            */
    NV_INT32      active_window_id;           /*!<  Process ID of the active window (set by the mouseMove function)      */
    NV_F64_COORD3 cursor_position;            /*!<  Position of the cursor (and possibly depth) in the application       */
    NV_CHAR       mosaic_hotkey[10];          /*!<  Hotkey to start the mosaic viewer program (can be modified in 
                                                    pfmEdit)                                                             */
    NV_CHAR       mosaic_prog[100];           /*!<  Mosaic viewer program name                                           */
    NV_CHAR       mosaic_actkey[10];          /*!<  Action keys for the mosaic viewer program                            */
    NV_INT32      polygon_count;              /*!<  Number of points in polygon                                          */
    NV_FLOAT64    polygon_x[2000];            /*!<  Polygon X positions                                                  */
    NV_FLOAT64    polygon_y[2000];            /*!<  Polygon Y positions                                                  */
    NV_CHAR       feature_search_string[128]; /*!<  Feature search string from pfmView                                   */
    NV_INT32      feature_search_type;        /*!<  0 = highlight result of search, 1 = only display result of search    */
    NV_BOOL       feature_search_invert;      /*!<  if set, search for features NOT containing search string             */


    /*  The following block of variables pertains to the color scale boxes on the left of pfmView, pfmEdit, and
        pfmEdit3D.  This is only applicable to color-by-depth mode in any of the three programs.  */

    NV_U_INT16    min_hsv_color;              /*!<  HSV color wheel index for color to be used for minimum values
                                                    (default 0)  */
    NV_U_INT16    max_hsv_color;              /*!<  HSV color wheel index for color to be used for maximum values
                                                    (default 315)  */
    NV_FLOAT32    min_hsv_value;              /*!<  Locked minimum data value for minimum HSV color  */
    NV_FLOAT32    max_hsv_value;              /*!<  Locked maximum data value for maximum HSV color  */
    NV_BOOL       min_hsv_locked;             /*!<  Set if min HSV is locked to a value  */
    NV_BOOL       max_hsv_locked;             /*!<  Set if max HSV is locked to a value  */


    /*  This part was lifted almost verbatim from pfm.h.  I've stopped using pfm.h because it is almost impossible
	to change the shared part without impacting everybody else who is using it.  */

    NV_F64_XYMBR  edit_area;                  /*!<  Total rectangular area to be edited                                  */
    NV_INT32      ppid;                       /*!<  Parent process ID (pfmView)                                          */
    NV_FLOAT32    cint;                       /*!<  Current contour interval (0 for user defined)                        */
    NV_FLOAT32    contour_levels[MAX_CONTOUR_LEVELS]; /*!<  User defined contour levels                                  */
    NV_INT32      num_levels;                 /*!<  Number of user defined contour levels                                */
    NV_INT32      layer_type;                 /*!<  Type of bin data/contour to display (AVERAGE_FILTERED_DEPTH,
                                                    MIN_FILTERED_DEPTH, MAX_DEPTH, etc. from pfm.h                       */
    NV_F64_XYMBR  displayed_area;             /*!<  Displayed area in the ndx editor                                     */
    NV_F64_XYMBR  viewer_displayed_area;      /*!<  Displayed area in the bin viewer                                     */
    PFM_OPEN_ARGS open_args[MAX_ABE_PFMS];    /*!<  Opening arguments for open_pfm_file                                  */
    NV_BOOL       display_pfm[MAX_ABE_PFMS];  /*!<  Whether the PFM is visible (if not, don't use it).                   */
    NV_INT32      pfm_count;                  /*!<  Number of currently open PFM files (layers).                         */
    NV_INT32      point_cloud_count;          /*!<  Total number of points in use in the point cloud.                    */


    /*  The following section is used for On-The-Fly (OTF) gridding in pfmView and pfm3D.  */

    NV_INT32      otf_width;                  /*!<  Width of otf grid (if this is set to 0 we're not using OTF).         */
    NV_INT32      otf_height;                 /*!<  Height of otf grid.                                                  */
    NV_FLOAT32    otf_min_z;                  /*!<  Minimum Z value of the otf grid.                                     */
    NV_FLOAT32    otf_max_z;                  /*!<  Maximum Z value of the otf grid.                                     */
    NV_FLOAT32    otf_null_value;             /*!<  Null value for otf grid.                                             */
    NV_FLOAT64    otf_x_bin_size;             /*!<  X otf bin size in degrees.                                           */
    NV_FLOAT64    otf_y_bin_size;             /*!<  Y otf bin size in degrees.                                           */


    /*  This is the delete file queue that will be populated by pfmEdit or pfmEdit3D (right click menu in DELETE_POINT
        mode) and acted on in pfmView.  The 2D array contains the PFM number in [0] and the file number in [1].        */

    NV_INT32      delete_file_queue[MAX_DELETE_FILE_QUEUE][2];
    NV_INT32      delete_file_queue_count;


    MULTIWAVE_SHARE  mwShare;                 /*!<  CHL: structure for multi-waveform data with waveMonitor              */
    AV_SHARE      avShare;                    /*!<  structure containing Attribute Viewer related stuff                  */


    FILTER_SHARE  filterShare;                /*!<  Values to be used in external filter programs                        */


    /*!  Flag to tell pfmView, pfmEdit, and pfmEdit3D that they are not allowed to edit the PFMs they are viewing.
         This is only used when working with the PFM World Data Base (PFMWDB) if the user is not allowed to edit
         the PFMWDB.  */

    NV_BOOL       read_only;


    /*!  Only one of these lest the overhead kill us.                                                                   */

    NV_CHAR       nearest_filename[512];


    /*!  Nearest point to the cursor in pfmEdit.  This is the trigger for waveMonitor and waveWaterfall.                */

    NV_INT32      nearest_point;



    NV_U_INT32    key;                        /*!<  Key or command code.                                                 */


    NV_U_INT32    killed;                     /*!<  Kill switch number for a process that is killed in the process.      
                                                    This is used to check on the death of processes that are started
                                                    from buttons (that need their state set).  These type of programs
                                                    are monitor type programs like chartsMonitor or chartsPic.  These
                                                    programs have their "state" set to non-zero in pfmEdit and/or
                                                    pfmEdit3D.  That is, they are respawnable.                            */


    /*!  Modified record information from ancillary programs.  This info will be read by pfmEdit and pfmEdit3D to
         effect the changes to the PFM.                                                                                  */

    NV_CHAR       modified_file[512];
    NV_CHAR       modified_line[512];
    NV_INT32      modified_file_num;
    NV_INT32      modified_line_num;
    NV_INT16      modified_type;
    NV_U_INT32    modified_record;
    NV_INT32      modified_subrecord;
    NV_INT32      modified_point;
    NV_FLOAT64    modified_value;
    NV_U_INT32    modified_validity;
    NV_U_INT32    modcode;                    /*!<  Set to something other than zero to indicate a change in the shared
                                                    data                                                                 */
  } ABE_SHARE;


#ifdef  __cplusplus
}
#endif

#endif
