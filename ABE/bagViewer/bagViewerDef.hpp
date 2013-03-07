
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



#ifndef _BAGVIEWER_DEF_H_
#define _BAGVIEWER_DEF_H_


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <cerrno>
#include <cmath>
#include <proj_api.h>


#include "nvutility.h"
#include "nvutility.hpp"

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>

#include "bag.h"
#include "bag_xml_meta.h"

#include <QtCore>
#include <QtGui>
#include <QSharedMemory>


#ifdef NVWIN3X
    #include "windows_getuid.h"

using namespace std;  // Windoze bullshit - God forbid they should follow a standard
#endif


//  Pointer interaction functions.

#define         NOOP                        0
#define         ROTATE                      1
#define         ZOOM                        2


#define         MARKER_W                    15
#define         MARKER_H                    10


typedef struct
{
  NV_U_INT16       list_series;
  NV_FLOAT64       y;
  NV_FLOAT64       x;
  NV_FLOAT32       depth;
  NV_CHAR          description[200];
  NV_CHAR          dateTime[21];
} TRACKING_LIST_ITEM;


//  The OPTIONS structure contains all those variables that can be saved to the
//  users pfmView QSettings.


typedef struct
{
  NV_INT32    position_form;              //  Position format number
  QColor      background_color;           //  Color to be used for background
  QColor      tracker_color;              //  Color to be used for track cursor
  QColor      scale_color;                //  Color to be used for scale
  QColor      feature_color;              //  Color to be used for tracking list points
  QColor      feature_info_color;         //  Color to be used for tracking list feature information text display
  NV_FLOAT64  exaggeration;               //  Z exaggeration
  NV_FLOAT32  z_factor;                   //  Coversion factor for Z values. May be used to convert m to ft...
  NV_FLOAT32  z_offset;                   //  Offset value for Z values.
  NV_INT32    new_feature;                //  Stupid counter so I can advertise new features once and then move on
  NV_BOOL     draw_scale;                 //  Set this to draw the scale in the display.
  NV_INT32    surface;                    //  Elevation, TBD
  NV_BOOL     color_option;               //  NVFalse - color by depth, NVTrue - color by uncertainty
  NV_BOOL     display_tracking_list;      //  Set to display tracking list points
  NV_BOOL     display_tracking_info;      //  Set to display tracking list verbiage
  NV_F64_XYMBR mbr;                       //  Displayed area
  NV_FLOAT32  feature_size;               //  Size of the tracking list markers
  QString     input_dir;
  QString     image_dir;
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
  NV_BOOL     drawing;                    //  set if we are drawing the surface
  NV_BOOL     drawing_canceled;           //  set to cancel drawing
  QColor      widgetBackgroundColor;      //  The normal widget background color.
  QString     help_browser;               //  Browser command name
  QString     html_help_tag;              //  Help tag for the currently running section of code.  This comes from
                                          //  the PFM_ABE html help documentation.
  NV_INT32    width;                      //  Main window width
  NV_INT32    height;                     //  Main window height
  NV_INT32    window_x;                   //  Main window x position
  NV_INT32    window_y;                   //  Main window y position
  NV_U_INT32  tracking_list_len;          //  Number of tracking list points
  TRACKING_LIST_ITEM *tracking_list;      //  Array of tracking list items
} MISC;


#endif
