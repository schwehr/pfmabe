
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



/*  nvMap class.  */

#include "nvmap.hpp"
#include "read_coast.h"
#include "read_srtm_mask.h"


#define DEG2RAD		0.0174532925199432957692


/***************************************************************************/
/*!

   - Module :        nvMap

   - Programmer :    Jan C. Depner

   - Date :          03/08/07

   - Purpose :       This is a generic map class for use in Qt graphical user
                     interfaces.  At present the projection defaults to 
                     Cylindrical Equidistant (X/Y).  The available map 
                     projections are:

                         - NO_PROJECTION                  -1
                         - CORRECTED_MERCATOR             0
                         - UNDEFINED                      1
                         - CYLINDRICAL_EQUAL_AREA         2
                         - CYLINDRICAL_EQUIDISTANT        3
                         - BRAUN_PERSPECTIVE_CYLINDRICAL  4
                         - GALL_STEREOGRAPHIC             5
                         - MILLER_CYLINDRICAL             6
                         - PSEUDOCYLINDRICAL_EQUAL_AREA   7
                         - SINUSOIDAL                     8
                         - PSEUDOCYLINDRICAL              9
                         - VAN_DER_GRINTEN                10
                         - MERCATOR_FROM_MAILING          11
                         - NARROW                         12
                         - WIDE                           13
                         - STEREOGRAPHIC                  14
                         - GNOMONIC                       15
                         - POSTEL                         16
                         - LAMBERT                        17
                         - ORTHOGRAPHIC                   18
                         - TRANSVERSE_MERCATOR            19

                    I wouldn't bet on any of these working except for the first
                    four or five.  I've only tested CORRECTED_MERCATOR and
                    CYLINDRICAL_EQUIDISTANT.  Other projections have
                    not been tested (why do you want to project on screen?).
                    I have tried to allow the user to work in map space 
                    without having to worry about screen space.  You can still
                    work in screen space though if you need to.

                    If you use NO_PROJECTION X and Y values will not be checked
                    against the normal lat/lon limits.  Also, Y axis inversion 
                    will be handled automatically if you set the minimum Y larger
                    than the maximum Y.


   - Note :         Don't forget that colors can be transparent in Qt4.  You can
                    set the transparency by setting the alpha value to a number
                    between 0 and 255 with 0 being invisible and 255 being opaque.
                    For example - QColor (255, 255, 0, 127) will be a transparent
                    yellow.

    
  - Don't forget to set the defaults that you want in the NVMAP_DEF structure (*init).  A lot
    of these can be set or reset later with the map functions.  Here's an example of a pretty
    standard setup:

  <pre>

  mapdef.projection = CYLINDRICAL_EQUIDISTANT;
  mapdef.draw_width = 900;
  mapdef.draw_height = 600;
  mapdef.frame = NVTrue;

  mapdef.overlap_percent = 5;
  mapdef.grid_inc_x = 10.0;
  mapdef.grid_inc_y = 10.0;
  mapdef.grid_thickness = 2;

  mapdef.coasts = NVMAP_AUTO_COAST;
  mapdef.coast_thickness = 2;
  mapdef.landmask = NVTrue;

  mapdef.border = 5;
  mapdef.coast_color = Qt::black;
  mapdef.landmask_color = Qt::red;
  mapdef.grid_color = Qt::blue;
  mapdef.background_color = Qt::white;

  mapdef.initial_bounds.min_x = -180.0;
  mapdef.initial_bounds.min_y = -90.0;
  mapdef.initial_bounds.max_x = 180.0;
  mapdef.initial_bounds.max_y = 90.0;

  </pre>

  Note that the nvMap functions will deal with inverted Y coordinates if you are using NO_PROJECTION.
  Just set the the initial Y bounds the way you want and it will handle it.  However, if your min_x is 
  greater than your max_x (and you're using one of the normal projections) the nvMap functions assume
  that you will be crossing over the dateline and adjust accordingly.  You can also use the 0-360
  world if you want.

****************************************************************************/

nvMap::nvMap (QWidget *parent, NVMAP_DEF *init):
  QWidget (parent, (Qt::WindowFlags) Qt::WA_StaticContents)
{
  //  Track mouse movement with no button pressed.

  setMouseTracking (TRUE);
  setAttribute (Qt::WA_OpaquePaintEvent);
  setBackgroundRole (QPalette::Base);
  setAutoFillBackground (TRUE);
  setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy (Qt::WheelFocus);


  map = *init;
  proj = (rproj *) NULL;
  stopFlag = NVFalse;
  invertY = NVFalse;
  last_map_called = -1;
  movable_object_count = 0;
  zero_to_360 = NVFalse;
  dummy_buffer = NVFalse;


  for (NV_INT32 i = 0 ; i < MAX_MOVABLE_OBJECTS ; i++)
    {
      movable_object[i].active = NVFalse;
      rubberbandPolyActive[i] = NVFalse;
      rubberbandRectActive[i] = NVFalse;
    }



  //  Make sure we have some decent values if they weren't set.

  if (!map.draw_width) map.draw_width = 600;
  if (!map.draw_height) map.draw_height = 600;
  if (map.border > 50) map.border = 50;
  if (map.zoom_percent < 0 || map.zoom_percent > 100) map.zoom_percent = 10;
  if (map.overlap_percent < 0 || map.overlap_percent > 100) map.overlap_percent = 5;

  map.bounds[0] = map.initial_bounds;
  if (map.projection < NO_PROJECTION || map.projection > TRANSVERSE_MERCATOR) map.projection = CYLINDRICAL_EQUIDISTANT;
  map.zoom_level = 0;


  //  Check for inverted Y axis.

  if (map.initial_bounds.min_y > map.initial_bounds.max_y && map.projection == NO_PROJECTION) invertY = NVTrue;


  //  Initialize the coastline and grid thicknesses if they haven't been set

  if (map.coast_thickness <= 0 || map.coast_thickness > 5) map.coast_thickness = 2;
  if (map.grid_thickness <= 0 || map.grid_thickness > 5) map.grid_thickness = 1;


  //  Dealing with the dateline.

  checkDateline (&map.bounds[0]);


  //  Allow or disallow wrapping in the  moveMap functions.  If the initial X map size is 360 degrees
  //  then no_bounds_limit is set to NVTrue, otherwise it is NVFalse.

  no_bounds_limit = NVFalse;
  if (map.bounds[0].max_x - map.bounds[0].min_x == 360.0) no_bounds_limit = NVTrue;


  signalsEnabled = NVFalse;
  mouseSignalsEnabled = NVFalse;
}



nvMap::~nvMap ()
{
}


//  *************************************************************************************
//
//  Map functions
//
//  *************************************************************************************


/*!  Enable ALL signals emitted by this class.  Signals are disabled by default.  You have
     to enable them to get the class to emit them.  This is handy if you are instantiating
     a map and you don't want it emit resize or redraw signals too soon.  */

void
nvMap::enableSignals ()
{
  signalsEnabled = NVTrue;
  mouseSignalsEnabled = NVTrue;
}



//!  Disable ALL signals emitted by this class.

void
nvMap::disableSignals ()
{
  signalsEnabled = NVFalse;
  mouseSignalsEnabled = NVFalse;
}



/*!  Enable MOUSE signals emitted by this class.  Signals are disabled by default.  You have
     to enable them to get the class to emit them.  This is handy if you are instantiating
     a map and you don't want it emit resize or redraw signals too soon.  */

void
nvMap::enableMouseSignals ()
{
  mouseSignalsEnabled = NVTrue;
}



//!  Disable MOUSE signals emitted by this class.

void
nvMap::disableMouseSignals ()
{
  mouseSignalsEnabled = NVFalse;
}



//!  Set the color with which to draw the land mask.

void
nvMap::setMaskColor (QColor color)
{
  map.landmask_color = color;
}



//!  Set the color with which to draw the coastlines.

void
nvMap::setCoastColor (QColor color)
{
  map.coast_color = color;
}



//!  Set the color with which to draw the grids.

void
nvMap::setGridColor (QColor color)
{
  map.grid_color = color;
}



//!  Set the background color.

void
nvMap::setBackgroundColor (QColor color)
{
  map.background_color = color;
}



//!  Set the grid spacing (in minutes of longitude [x] and latitude [y]).

void 
nvMap::setGridSpacing (NV_FLOAT64 grid_inc_x, NV_FLOAT64 grid_inc_y)
{
  map.grid_inc_x = grid_inc_x;
  map.grid_inc_y = grid_inc_y;
}



//!  Get the current zoom level.

NV_INT32 
nvMap::getZoomLevel ()
{
  return (map.zoom_level);
}



//!  Set the zoom level.  This allows you to go directly to a previous zoom level.

void
nvMap::setZoomLevel (NV_INT32 level)
{
  map.zoom_level = level;


  //  Check for dateline

  checkDateline (&map.bounds[map.zoom_level]);


  redrawMapWithCheck (NVTrue);
}



//!  Set the zoom percentage for percent zoom in or out.

void
nvMap::setZoomPercent (NV_INT32 percent)
{
  map.zoom_percent = percent;
}



//!  Set the overlap percentage for moves in any direction.

void
nvMap::setOverlapPercent (NV_INT32 percent)
{
  map.overlap_percent = percent;
}



//!  Stop drawing land mask.

void
nvMap::stopDrawing ()
{
  stopFlag = NVTrue;
}



/*!  Zoom in to the specified bounds (see nvtypes.h for the NV_F64_XYMBR structure definition).
     This increments the zoom level.  */

void
nvMap::zoomIn (NV_F64_XYMBR bounds)
{
  zoomIn (bounds, NVTrue);
}



/*!  Zoom in to the specified bounds (see nvtypes.h for the NV_F64_XYMBR structure definition).
     If setLevel is NVTrue then a new zoom level is saved otherwise the last zoom level is
     replaced.  If the current zoom level is 0 then a new level is made regardless of the
     setLevel value.  */

void
nvMap::zoomIn (NV_F64_XYMBR bounds, NV_BOOL setLevel)
{
  if (map.zoom_level < NVMAP_ZOOM_LEVELS)
    {
      //  Check for dateline

      checkDateline (&bounds);


      //  Set a new zoom level if setLevel is NVTrue or if we only have the initial zoom level.

      if (!map.zoom_level || setLevel) map.zoom_level++;


      map.bounds[map.zoom_level].min_y = bounds.min_y;
      map.bounds[map.zoom_level].max_y = bounds.max_y;

      map.bounds[map.zoom_level].min_x = bounds.min_x;
      map.bounds[map.zoom_level].max_x = bounds.max_x;

      redrawMapWithCheck (NVTrue);
    }
}



//!  Zoom out one zoom level.  This decrements the zoom level.

void
nvMap::zoomOut ()
{
  if (map.zoom_level)
    {
      map.zoom_level--;


      //  Check for dateline

      checkDateline (&map.bounds[map.zoom_level]);


      redrawMapWithCheck (NVTrue);
    }
}



//!  Zoom in by the map.zoom_percent.  This increments the zoom level.

void 
nvMap::zoomInPercent ()
{
  zoomInPercent (NVTrue);
}



/*!  Zoom in by the map.zoom_percent.  If setLevel is NVTrue then a new zoom level is saved
     otherwise the last zoom level is replaced.  If the current zoom level is 0 then a new level
     is made regardless of the setLevel value.  */


void
nvMap::zoomInPercent (NV_BOOL setLevel)
{
  if (map.zoom_level < NVMAP_ZOOM_LEVELS)
    {
      NV_FLOAT64 percentage = (NV_FLOAT64) map.zoom_percent / 100.0;

      NV_FLOAT64 xsize = map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x;
      NV_FLOAT64 ysize = map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y;


      //  Set a new zoom level if setLevel is NVTrue or if we only have the initial zoom level.

      NV_INT32 old_level = map.zoom_level;
      if (!map.zoom_level || setLevel) map.zoom_level++;


      map.bounds[map.zoom_level].min_x = map.bounds[old_level].min_x + (xsize * percentage) / 2.0;
      map.bounds[map.zoom_level].max_x = map.bounds[old_level].max_x - (xsize * percentage) / 2.0;


      //  Check for dateline

      checkDateline (&map.bounds[map.zoom_level]);


      map.bounds[map.zoom_level].min_y = map.bounds[old_level].min_y + (ysize * percentage) / 2.0;
      map.bounds[map.zoom_level].max_y = map.bounds[old_level].max_y - (ysize * percentage) / 2.0;


      redrawMapWithCheck (NVTrue);
    }
}



//!  Zoom out by map.zoom_percent.  This does not decrement the zoom level.

void
nvMap::zoomOutPercent ()
{
  if (map.zoom_level)
    {
      NV_FLOAT64 percentage = (NV_FLOAT64) map.zoom_percent / 100.0;

      NV_FLOAT64 xsize = map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x;
      NV_FLOAT64 ysize = map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y;


      //  This is a bit weird, if we have crossed the dateline and the original size is 360.0 we don't want to limit
      //  the bounds.  This is a terible kludge and won't work if this is continuously called but I can't think of 
      //  any other way to deal with it.

      NV_BOOL check = NVTrue;
      if (dateline && map.bounds[0].max_x - map.bounds[0].min_x == 360.0 && map.bounds[0].max_x <= 180.0) check = NVFalse;

      map.bounds[map.zoom_level].min_x = map.bounds[map.zoom_level].min_x - (xsize * percentage) / 2.0;
      map.bounds[map.zoom_level].max_x = map.bounds[map.zoom_level].max_x + (xsize * percentage) / 2.0;

      if (check && map.bounds[map.zoom_level].min_x < map.bounds[0].min_x)
        {
          map.bounds[map.zoom_level].min_x = map.bounds[0].min_x;
          map.bounds[map.zoom_level].max_x = map.bounds[map.zoom_level].min_x + xsize * percentage * 2.0;

          if (map.bounds[map.zoom_level].max_x > map.bounds[0].max_x)
            map.bounds[map.zoom_level].max_x = map.bounds[0].max_x;
        }


      if (check && map.bounds[map.zoom_level].max_x > map.bounds[0].max_x)
        {
          map.bounds[map.zoom_level].max_x = map.bounds[0].max_x;
          map.bounds[map.zoom_level].min_x = map.bounds[map.zoom_level].max_x - xsize * percentage * 2.0;

          if (map.bounds[map.zoom_level].min_x < map.bounds[0].min_x)
            map.bounds[map.zoom_level].min_x = map.bounds[0].min_x;
        }


      //  Check for dateline

      checkDateline (&map.bounds[map.zoom_level]);


      map.bounds[map.zoom_level].min_y = map.bounds[map.zoom_level].min_y - (ysize * percentage) / 2.0;
      map.bounds[map.zoom_level].max_y = map.bounds[map.zoom_level].max_y + (ysize * percentage) / 2.0;

      if (map.bounds[map.zoom_level].min_y < map.bounds[0].min_y)
        {
          map.bounds[map.zoom_level].min_y = map.bounds[0].min_y;
          map.bounds[map.zoom_level].max_y = map.bounds[map.zoom_level].min_y + ysize * percentage * 2.0;

          if (map.bounds[map.zoom_level].max_y > map.bounds[0].max_y)
            map.bounds[map.zoom_level].max_y = map.bounds[0].max_y;
        }


      if (map.bounds[map.zoom_level].max_y > map.bounds[0].max_y)
        {
          map.bounds[map.zoom_level].max_y = map.bounds[0].max_y;
          map.bounds[map.zoom_level].min_y = map.bounds[map.zoom_level].max_y - ysize * percentage * 2.0;

          if (map.bounds[map.zoom_level].min_y < map.bounds[0].min_y)
            map.bounds[map.zoom_level].min_y = map.bounds[0].min_y;
        }


      redrawMapWithCheck (NVTrue);
    }
}



//!  Reset zoom level 0 to the specified bounds.

void 
nvMap::resetBounds (NV_F64_XYMBC bounds)
{
  NV_F64_XYMBR nbounds;

  nbounds.min_x = bounds.min_x;
  nbounds.min_y = bounds.min_y;
  nbounds.max_x = bounds.max_x;
  nbounds.max_y = bounds.max_y;

  resetBounds (nbounds);
}



//!  Reset the initial bounds to the specified bounds.

void 
nvMap::resetBounds (NV_F64_XYMBR bounds)
{
  map.bounds[0] = map.initial_bounds = bounds;
  map.zoom_level = 0;


  //  Check for dateline

  checkDateline (&map.bounds[map.zoom_level]);


  //  Check for inverted Y axis.

  if (map.bounds[0].min_y > map.bounds[0].max_y && map.projection == NO_PROJECTION) invertY = NVTrue;
}



//!  Set the projection (see nvMap comments above for available projections).

void 
nvMap::setProjection (NV_INT32 projection)
{
  map.projection = projection;
}



//!  Set coastline drawing in redrawMap on or off.

void 
nvMap::setCoasts (NV_INT32 set)
{
  map.coasts = set;
}



//!  Set landmask filling in redrawMap on or off.

void 
nvMap::setLandmask (NV_BOOL set)
{
  map.landmask = set;
}



//!  Set the thickness (in pixels) of the coastline.

void 
nvMap::setCoastThickness (NV_INT32 thickness)
{
  map.coast_thickness = thickness;
}



//!  Return a snapshot of the complete NVMAP_DEF structure.

NVMAP_DEF 
nvMap::getMapdef ()
{
  return (map);
}



//!  Move map to center "map_x, map_y" in map space.

void
nvMap::moveMap (NV_FLOAT64 map_x, NV_FLOAT64 map_y)
{
  NV_FLOAT64 xsize = (map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x);
  NV_FLOAT64 ysize = (map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y);


  map.bounds[map.zoom_level].min_x = map_x - xsize / 2.0;
  map.bounds[map.zoom_level].max_x = map_x + xsize / 2.0;


  //  Special case, if the initial map was a world map we will allow movement past the boundaries and then adjust
  //  for the dateline

  if ((map.bounds[0].max_x == 180.0 && map.bounds[0].min_x == -180.0) ||
      (map.bounds[0].max_x == 360.0 && map.bounds[0].min_x == 0.0))
    {
      map.bounds[map.zoom_level].min_x = fmod (map.bounds[map.zoom_level].min_x, 360.0);
      map.bounds[map.zoom_level].max_x = fmod (map.bounds[map.zoom_level].max_x, 360.0);

      if (map.bounds[map.zoom_level].min_x < -180.0) map.bounds[map.zoom_level].min_x += 360.0;
      if (map.bounds[map.zoom_level].min_x > map.bounds[map.zoom_level].max_x) map.bounds[map.zoom_level].max_x += 360.0;
      if (map.bounds[map.zoom_level].max_x > 180.0 && !zero_to_360) map.bounds[map.zoom_level].max_x -= 360.0;
    }
  else
    {
      if (map.bounds[map.zoom_level].max_x > map.bounds[0].max_x)
        {
          map.bounds[map.zoom_level].max_x = map.bounds[0].max_x;
          map.bounds[map.zoom_level].min_x = map.bounds[map.zoom_level].max_x - xsize;
        }
      if (map.bounds[map.zoom_level].min_x < map.bounds[0].min_x)
        {
          map.bounds[map.zoom_level].min_x = map.bounds[0].min_x;
          map.bounds[map.zoom_level].max_x = map.bounds[map.zoom_level].min_x + xsize;
        }
    }


  //  Check for dateline

  checkDateline (&map.bounds[map.zoom_level]);


  map.bounds[map.zoom_level].min_y = map_y - ysize / 2.0;
  map.bounds[map.zoom_level].max_y = map_y + ysize / 2.0;

  if (map.bounds[map.zoom_level].max_y > map.bounds[0].max_y)
    {
      map.bounds[map.zoom_level].max_y = map.bounds[0].max_y;
      map.bounds[map.zoom_level].min_y = map.bounds[map.zoom_level].max_y - ysize;
    }
  if (map.bounds[map.zoom_level].min_y < map.bounds[0].min_y)
    {
      map.bounds[map.zoom_level].min_y = map.bounds[0].min_y;
      map.bounds[map.zoom_level].max_y = map.bounds[map.zoom_level].min_y + ysize;
    }

  redrawMapWithCheck (NVTrue);
}



/*!  Move in some direction with overlap (set in setOverlapPercent).  "direction" is NVMAP_LEFT = 0, NVMAP_UP = 1, NVMAP_RIGHT = 2, NVMAP_DOWN = 3,
     NVMAP_UP_LEFT = 4, NVMAP_UP_RIGHT = 5, NVMAP_DOWN_LEFT = 6, NVMAP_DOWN_RIGHT = 7.  */

void
nvMap::moveMap (NV_INT32 direction)
{
  NV_FLOAT64 xsize = (map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x);
  NV_FLOAT64 ysize = (map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y);

  NV_FLOAT64 overlap_mult = 1.0 - ((NV_FLOAT64) map.overlap_percent / 100.0);


  //  Notice the overlap.

  if (direction == NVMAP_LEFT || direction == NVMAP_UP_LEFT || direction == NVMAP_DOWN_LEFT)
    {
      map.bounds[map.zoom_level].min_x = map.bounds[map.zoom_level].min_x - xsize * overlap_mult;
      map.bounds[map.zoom_level].max_x = map.bounds[map.zoom_level].max_x - xsize * overlap_mult;


      //  Special case, if the initial map was a world map we will allow movement past the boundaries and then adjust
      //  for the dateline

      if ((map.bounds[0].max_x == 180.0 && map.bounds[0].min_x == -180.0) ||
          (map.bounds[0].max_x == 360.0 && map.bounds[0].min_x == 0.0))
        {
          map.bounds[map.zoom_level].min_x = fmod (map.bounds[map.zoom_level].min_x, 360.0);
          map.bounds[map.zoom_level].max_x = fmod (map.bounds[map.zoom_level].max_x, 360.0);

          if (map.bounds[map.zoom_level].min_x < -180.0) map.bounds[map.zoom_level].min_x += 360.0;
          if (map.bounds[map.zoom_level].min_x > map.bounds[map.zoom_level].max_x) map.bounds[map.zoom_level].max_x += 360.0;
          if (map.bounds[map.zoom_level].max_x > 180.0 && !zero_to_360) map.bounds[map.zoom_level].max_x -= 360.0;
        }
      else
        {
          if (map.bounds[map.zoom_level].min_x < map.bounds[0].min_x)
            {
              map.bounds[map.zoom_level].min_x = map.bounds[0].min_x;
              map.bounds[map.zoom_level].max_x = map.bounds[map.zoom_level].min_x + xsize;
            }
        }
    }


  if (direction == NVMAP_UP || direction == NVMAP_UP_LEFT || direction == NVMAP_UP_RIGHT)
    {
      map.bounds[map.zoom_level].min_y = map.bounds[map.zoom_level].min_y + ysize * overlap_mult;
      map.bounds[map.zoom_level].max_y = map.bounds[map.zoom_level].max_y + ysize * overlap_mult;

      if (map.bounds[map.zoom_level].max_y > map.bounds[0].max_y)
        {
          map.bounds[map.zoom_level].max_y = map.bounds[0].max_y;
          map.bounds[map.zoom_level].min_y = map.bounds[map.zoom_level].max_y - ysize;
        }
    }


  if (direction == NVMAP_RIGHT || direction == NVMAP_UP_RIGHT || direction == NVMAP_DOWN_RIGHT)
    {
      map.bounds[map.zoom_level].min_x = map.bounds[map.zoom_level].min_x + xsize * overlap_mult;
      map.bounds[map.zoom_level].max_x = map.bounds[map.zoom_level].max_x + xsize * overlap_mult;


      //  Special case, if the initial map was a world map we will allow movement past the boundaries and then adjust
      //  for the dateline

      if ((map.bounds[0].max_x == 180.0 && map.bounds[0].min_x == -180.0) ||
          (map.bounds[0].max_x == 360.0 && map.bounds[0].min_x == 0.0))
        {
          map.bounds[map.zoom_level].min_x = fmod (map.bounds[map.zoom_level].min_x, 360.0);
          map.bounds[map.zoom_level].max_x = fmod (map.bounds[map.zoom_level].max_x, 360.0);

          if (map.bounds[map.zoom_level].min_x < -180.0) map.bounds[map.zoom_level].min_x += 360.0;
          if (map.bounds[map.zoom_level].min_x > map.bounds[map.zoom_level].max_x) map.bounds[map.zoom_level].max_x += 360.0;
          if (map.bounds[map.zoom_level].max_x > 180.0 && !zero_to_360) map.bounds[map.zoom_level].max_x -= 360.0;
        }
      else
        {
          if (map.bounds[map.zoom_level].max_x > map.bounds[0].max_x)
            {
              map.bounds[map.zoom_level].max_x = map.bounds[0].max_x;
              map.bounds[map.zoom_level].min_x = map.bounds[map.zoom_level].max_x - xsize;
            }
        }
    }


  if (direction == NVMAP_DOWN || direction == NVMAP_DOWN_LEFT || direction == NVMAP_DOWN_RIGHT)
    {
      map.bounds[map.zoom_level].min_y = map.bounds[map.zoom_level].min_y - ysize * overlap_mult;
      map.bounds[map.zoom_level].max_y = map.bounds[map.zoom_level].max_y - ysize * overlap_mult;

      if (map.bounds[map.zoom_level].min_y < map.bounds[0].min_y)
        {
          map.bounds[map.zoom_level].min_y = map.bounds[0].min_y;
          map.bounds[map.zoom_level].max_y = map.bounds[map.zoom_level].min_y + ysize;
        }
    }


  //  Check for dateline

  checkDateline (&map.bounds[map.zoom_level]);


  redrawMapWithCheck (NVTrue);
}



/*!  This function will call the redrawmap function that was last called.  The reason for this is that
     the user may have been using redrawMapArea and does a resize or a zoom.  The zoom functions will
     call redrawMapWithCheck instead of redrawMap or redrawMapArea.  */

void 
nvMap::redrawMapWithCheck (NV_BOOL clear)
{
  //  If we haven't called either or we last called redrawMap, call redrawMap.

  if (last_map_called)
    {
      redrawMap (clear);
    }
  else
    {
      redrawMapArea (clear);
    }
}



/*!  This is a special case of redrawMap to allow you to use the map area for 
     some other, non-projected (at least internally) data.  It emits the same signals
     and sets the same variables as the normal call but it doesn't draw coastlines,
     landmask, or grid lines.  */

void
nvMap::redrawMapArea (NV_BOOL clear)
{
  //  Compute the center position

  NV_FLOAT64 range = map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x;
  map.center_x = map.bounds[map.zoom_level].min_x + range / 2.0;
  range = map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y;
  map.center_y = map.bounds[map.zoom_level].min_y + range / 2.0;


  //  This shuts off movable object painting until we are done redrawing the map.

  static NV_INT32 save_movable_object_count = movable_object_count;
  movable_object_count = 0;


  last_map_called = 0;


  map.width = map.draw_width - (map.border * 2);
  map.height = map.draw_height - (map.border * 2);

  if (clear)
    {
      QBrush brush;
      brush.setStyle (Qt::SolidPattern);
      brush.setColor (Qt::white);
      painter.fillRect (0, 0, width (), height (), brush);

      NV_INT32 x0, y0;
      x0 = map.border;
      y0 = map.border;

      brush.setColor (map.background_color);
      painter.fillRect (x0, y0, map.width, map.height, brush);
    }
    

  //  Let the parent do something after clearing (we're going to emit all of the
  //  calls that the regular redrawMap does)

  if (signalsEnabled) emit preRedrawSignal (map);


  //  Let the parent do something after drawing coastlines but prior to drawing grid.

  if (signalsEnabled) emit midRedrawSignal (map);


  //  Draw the grid lines.

  if (clear)
    {
      NV_INT32 segx[2], segy[2];

      NV_INT32 start_y = (NV_INT32) (map.bounds[map.zoom_level].min_y - 1.0);
      NV_INT32 end_y = (NV_INT32) (map.bounds[map.zoom_level].max_y + 1.0);
      NV_INT32 start_x = (NV_INT32) (map.bounds[map.zoom_level].min_x - 1.0);
      NV_INT32 end_x = (NV_INT32) (map.bounds[map.zoom_level].max_x + 1.0);


      if (map.frame)
        {
          drawRectangle (map.bounds[map.zoom_level].min_x, map.bounds[map.zoom_level].min_y, 
                         map.bounds[map.zoom_level].max_x, map.bounds[map.zoom_level].max_y,
                         map.grid_color, map.grid_thickness, Qt::SolidLine, NVFalse);
        }
          
      if (map.grid_inc_x)
        {
          NV_FLOAT64 min_x = map.bounds[map.zoom_level].min_x;
          NV_FLOAT64 max_x = map.bounds[map.zoom_level].max_x;

          for (NV_INT32 x = start_x ; x < end_x ; x++)
            {
              if (!(x % (NV_INT32) map.grid_inc_x) && x > min_x && x < max_x)
                {
                  NV_FLOAT64 map_x = (NV_FLOAT64) x;

                  map_to_screen (1, &map_x, &map.bounds[map.zoom_level].min_y, &dum_map_z[0], &segx[0], &segy[0],
                                 &dum_z[0]);

                  map_to_screen (1, &map_x, &map.bounds[map.zoom_level].max_y, &dum_map_z[0], &segx[1], &segy[1],
                                 &dum_z[0]);

                  drawLine (segx[0], segy[0], segx[1], segy[1], map.grid_color, map.grid_thickness, NVFalse,
                            Qt::SolidLine);
                }
            }
        }


      if (map.grid_inc_y)
        {
          NV_FLOAT64 min_y = map.bounds[map.zoom_level].min_y;
          NV_FLOAT64 max_y = map.bounds[map.zoom_level].max_y;

          for (NV_INT32 y = start_y ; y < end_y ; y++)
            {
              if (!(y % (NV_INT32) map.grid_inc_y) && y > min_y && y < max_y)
                {
                  NV_FLOAT64 map_y = (NV_FLOAT64) y;

                  map_to_screen (1, &map.bounds[map.zoom_level].min_x, &map_y, &dum_map_z[0], &segx[0], &segy[0],
                                 &dum_z[0]);

                  map_to_screen (1, &map.bounds[map.zoom_level].max_x, &map_y, &dum_map_z[0], &segx[1], &segy[1],
                                 &dum_z[0]);

                  drawLine (segx[0], segy[0], segx[1], segy[1], map.grid_color, 2, NVFalse, Qt::SolidLine);
                }
            }
        }
    }

  //  Flush the paint buffer.

  update ();


  //  Let the parent do something after grid has been drawn.

  if (signalsEnabled) emit postRedrawSignal (map);


  update ();


  //  Turn movable object painting back on.

  movable_object_count = save_movable_object_count;
}



/*!  Draw the map.  Emit signals when needed.  The argument 'clear' can be:

     - NVFalse = do not clear screen
     - NVTrue  = clear projected screen area  */

void
nvMap::redrawMap (NV_BOOL clear)
{
  //  Compute the center position

  NV_FLOAT64 range = map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x;
  map.center_x = map.bounds[map.zoom_level].min_x + range / 2.0;
  range = map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y;
  map.center_y = map.bounds[map.zoom_level].min_y + range / 2.0;


  //  This shuts off movable object painting until we are done redrawing the map.

  static NV_INT32 save_movable_object_count = movable_object_count;
  movable_object_count = 0;


  last_map_called = 1;


  static NV_CHAR files[9][12] = {"coasts.dat", "rivers.dat", "bounds.dat", "wvsfull.dat", 
                                 "wvs250k.dat", "wvs1.dat", "wvs3.dat", "wvs12.dat", "wvs43.dat"};

  void wdbplt(NV_CHAR *file, NV_FLOAT64 slatd, NV_FLOAT64 nlatd, NV_FLOAT64 wlond, 
              NV_FLOAT64 elond, nvMap *map, QColor color, NV_INT32 line_width);


  if (map.projection > NO_PROJECTION) init_proj (0, 0, map.draw_width, map.draw_height, map.border, 
                                                 map.bounds[map.zoom_level].min_x, map.bounds[map.zoom_level].min_y, 
                                                 map.bounds[map.zoom_level].max_x, map.bounds[map.zoom_level].max_y);


  NV_INT32 x0, y0, x1, y1;
  map_to_screen (1, &map.bounds[map.zoom_level].min_x, &map.bounds[map.zoom_level].max_y, &dum_map_z[0], &x0, &y0,
                 &dum_z[0]);
  map_to_screen (1, &map.bounds[map.zoom_level].max_x, &map.bounds[map.zoom_level].min_y, &dum_map_z[0], &x1, &y1,
                 &dum_z[0]);
  map.height = y1 - y0;
  map.width = x1 - x0;


  if (clear)
    {
      QBrush brush;
      brush.setStyle (Qt::SolidPattern);
      brush.setColor (Qt::white);
      painter.fillRect (0, 0, width (), height (), brush);

      brush.setColor (map.background_color);
      painter.fillRect (x0, y0, map.width, map.height, brush);
    }
    

  //  Let the parent do something after clearing but prior to drawing coastlines.

  if (signalsEnabled) emit preRedrawSignal (map);


  //  Plot coastlines and landmask if requested.  Use size to determine resolution.

  if (clear)
    {
      if (map.landmask)
        {
          NV_INT32 start_lat = (NV_INT32) map.bounds[map.zoom_level].min_y - 1;
          NV_INT32 start_lon = (NV_INT32) map.bounds[map.zoom_level].min_x - 1;
          NV_INT32 end_lat = (NV_INT32) map.bounds[map.zoom_level].max_y + 1;
          NV_INT32 end_lon = (NV_INT32) map.bounds[map.zoom_level].max_x + 1;
          NV_FLOAT64 lat[2], lon[2];
          NV_U_CHAR *array = NULL;


          NV_INT32 wsize = -1, hsize = 0;


          NV_INT32 total = (end_lat - start_lat) * (end_lon - start_lon);
          QProgressDialog progress (tr ("Painting land mask..."), tr ("Stop painting"), 0, total, this);

          progress.setWindowModality (Qt::WindowModal);
          progress.show ();

          qApp->processEvents();


          for (NV_INT32 j = start_lat ; j < end_lat ; j++)
            {
              for (NV_INT32 k = start_lon ; k < end_lon ; k++)
                {
                  NV_INT32 prog = (j - start_lat) * (end_lon - start_lon) + (k - start_lon);
                  progress.setValue (prog);
                  qApp->processEvents();

                  if (progress.wasCanceled ())
                    {
                      stopFlag = NVTrue;
                      break;
                    }

                  if (map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x < 2.0 && 
                      map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y < 2.0)
                    {
                      wsize = read_srtm_mask_one_degree (j, k, &array, 1);
                    }
                  else if (map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x < 5.0 && 
                           map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y < 5.0)
                    {
                      wsize = read_srtm_mask_one_degree (j, k, &array, 3);
                    }
                  else
                    {
                      wsize = read_srtm_mask_one_degree (j, k, &array, 30);
                    }



                  hsize = wsize;
                  if (wsize == 1800) hsize = 3600;

                  if (wsize > 2)
                    {
                      NV_FLOAT64 winc = 1.0L / (NV_FLOAT64) wsize;
                      NV_FLOAT64 hinc = 1.0L / (NV_FLOAT64) hsize;

                      for (NV_INT32 m = 0 ; m < hsize ; m++)
                        {
                          lat[0] = (NV_FLOAT64) (j + 1) - (NV_FLOAT64) (m + 1) * hinc;
                          lat[1] = lat[0] + hinc;

                          if (lat[0] >= map.bounds[map.zoom_level].min_y && lat[1] <= map.bounds[map.zoom_level].max_y)
                            {
                              for (NV_INT32 n = 0 ; n < wsize ; n++)
                                {
                                  if (array[m * wsize + n])
                                    {
                                      lon[0] = (NV_FLOAT64) k + (NV_FLOAT64) (n + 1) * winc;
                                      lon[1] = lon[0] + winc;

                                      if (lon[0] >= map.bounds[map.zoom_level].min_x && lon[1] <= map.bounds[map.zoom_level].max_x)
                                        {
                                          fillRectangle (lon[0], lat[0], lon[1], lat[1], map.landmask_color, NVFalse);
                                        }
                                    }
                                }
                            }
                        }
                    }
                  else
                    {
                      if (wsize == 1)
                        {
                          lat[0] = (NV_FLOAT64) j;
                          lon[0] = (NV_FLOAT64) k;
                          lat[1] = lat[0] + 1.0L;
                          lon[1] = lon[0] + 1.0L;

                          if (lat[0] < map.bounds[map.zoom_level].min_y) lat[0] = map.bounds[map.zoom_level].min_y;
                          if (lon[0] < map.bounds[map.zoom_level].min_x) lon[0] = map.bounds[map.zoom_level].min_x;
                          if (lat[1] > map.bounds[map.zoom_level].max_y) lat[1] = map.bounds[map.zoom_level].max_y;
                          if (lon[1] > map.bounds[map.zoom_level].max_x) lon[1] = map.bounds[map.zoom_level].max_x;

                          if (lat[0] >= map.bounds[map.zoom_level].min_y && lat[1] <= map.bounds[map.zoom_level].max_y &&
                              lon[0] >= map.bounds[map.zoom_level].min_x && lon[1] <= map.bounds[map.zoom_level].max_x)
                            {
                              fillRectangle (lon[0], lat[0], lon[1], lat[1], map.landmask_color, NVFalse);
                            }
                        }
                    }
                }

              qApp->processEvents ();
              if (stopFlag) break;
            }
          if (wsize > 2) cleanup_srtm_mask ();

          progress.setValue (total);

          repaint ();
        }


      //  Coastlines - note that we use the old wdbplt for the lower resolutions.

      if (map.coasts)
        {
          NV_FLOAT32 xsize = map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x;
          NV_FLOAT32 ysize = map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y;


          //  This is the 1:50,000 scale coastline.  It is not reliable north of 68N or south of 57S.

          if (map.coasts == NVMAP_NGA_50K_COAST || (map.coasts == NVMAP_AUTO_COAST && check_coast (COAST_50K) && xsize < 6.0 && ysize < 6.0 && 
                                                    map.bounds[map.zoom_level].max_y < 68.0 && map.bounds[map.zoom_level].min_y > -57.0))
            {
              map.coasts_used = NVMAP_NGA_50K_COAST;

              NV_INT32 slat = (NV_INT32) (map.bounds[map.zoom_level].min_y + 90.0) - 90;
              NV_INT32 nlat = (NV_INT32) (map.bounds[map.zoom_level].max_y + 90.0) - 89;
              NV_INT32 wlon = (NV_INT32) (map.bounds[map.zoom_level].min_x + 180.0) - 180;
              NV_INT32 elon = (NV_INT32) (map.bounds[map.zoom_level].max_x + 180.0) - 179;

              NV_FLOAT64 *coast_x, *coast_y;
              NV_INT32 segCount;

              for (NV_INT32 i = slat ; i <= nlat ; i++)
                {
                  for (NV_INT32 j = wlon ; j <= elon ; j++)
                    {
                      while ((segCount = read_coast (COAST_50K, j, i, &coast_x, &coast_y)))
                        {
                          NV_BOOL in = NVFalse;
                          for (NV_INT32 k = 0 ; k < segCount ; k++)
                            {
                              if (dateline && coast_x[k] < 0.0) coast_x[k] += 360.0;

                              if (checkBounds (coast_x[k], coast_y[k]))
                                {
                                  if (k && in) drawLine (coast_x[k - 1], coast_y[k - 1], coast_x[k], coast_y[k],
                                                         map.coast_color, map.coast_thickness, NVFalse, Qt::SolidLine);
                                  in = NVTrue;
                                }
                              else
                                {
                                  in = NVFalse;
                                }
                            }

                          free (coast_x);
                          free (coast_y);
                        }
                    }
                }

              update ();
            }
          else if (map.coasts == NVMAP_WVS_FULL_COAST || (map.coasts == NVMAP_AUTO_COAST && check_coast (GSHHS_ALL) && (xsize < 10.0 || ysize < 10.0)))
            {
              map.coasts_used = NVMAP_WVS_FULL_COAST;

              NV_INT32 slat = (NV_INT32) (map.bounds[map.zoom_level].min_y + 90.0) - 90;
              NV_INT32 nlat = (NV_INT32) (map.bounds[map.zoom_level].max_y + 90.0) - 89;
              NV_INT32 wlon = (NV_INT32) (map.bounds[map.zoom_level].min_x + 180.0) - 180;
              NV_INT32 elon = (NV_INT32) (map.bounds[map.zoom_level].max_x + 180.0) - 179;
              NV_FLOAT64 *coast_x, *coast_y;
              NV_INT32 segCount;

              for (NV_INT32 i = slat ; i <= nlat ; i++)
                {
                  for (NV_INT32 j = wlon ; j <= elon ; j++)
                    {
                      while ((segCount = read_coast (GSHHS_ALL, j, i, &coast_x, &coast_y)))
                        {
                          NV_BOOL in = NVFalse;
                          for (NV_INT32 k = 0 ; k < segCount ; k++)
                            {
                              if (dateline && coast_x[k] < 0.0) coast_x[k] += 360.0;

                              if (checkBounds (coast_x[k], coast_y[k]))
                                {
                                  if (k && in) drawLine (coast_x[k - 1], coast_y[k - 1], coast_x[k], coast_y[k],
                                                         map.coast_color, map.coast_thickness, NVFalse, Qt::SolidLine);
                                  in = NVTrue;
                                }
                              else
                                {
                                  in = NVFalse;
                                }
                            }


                          free (coast_x);
                          free (coast_y);
                        }
                    }
                }
              update ();
            }
          else if (map.coasts == NVMAP_WVS_1M_COAST || (map.coasts == NVMAP_AUTO_COAST && (xsize < 20.0 || ysize < 20.0)))
            {
              map.coasts_used = NVMAP_WVS_1M_COAST;

              wdbplt (files[5], map.bounds[map.zoom_level].min_y, map.bounds[map.zoom_level].max_y,
                      map.bounds[map.zoom_level].min_x, map.bounds[map.zoom_level].max_x, this, map.coast_color,
                      map.coast_thickness);
            }
          else if (map.coasts == NVMAP_WVS_3M_COAST || (map.coasts == NVMAP_AUTO_COAST && (xsize < 50.0 || ysize < 50.0)))
            {
              map.coasts_used = NVMAP_WVS_3M_COAST;

              wdbplt (files[6], map.bounds[map.zoom_level].min_y, map.bounds[map.zoom_level].max_y,
                      map.bounds[map.zoom_level].min_x, map.bounds[map.zoom_level].max_x, this, map.coast_color,
                      map.coast_thickness);
            }
          else if (map.coasts == NVMAP_WVS_12M_COAST || (map.coasts == NVMAP_AUTO_COAST && (xsize < 90.0 || ysize < 90.0)))
            {
              map.coasts_used = NVMAP_WVS_12M_COAST;

              wdbplt (files[7], map.bounds[map.zoom_level].min_y, map.bounds[map.zoom_level].max_y,
                      map.bounds[map.zoom_level].min_x, map.bounds[map.zoom_level].max_x, this, map.coast_color,
                      map.coast_thickness);
            }
          else
            {
              map.coasts_used = NVMAP_WVS_43M_COAST;

              wdbplt (files[8], map.bounds[map.zoom_level].min_y, map.bounds[map.zoom_level].max_y,
                      map.bounds[map.zoom_level].min_x, map.bounds[map.zoom_level].max_x, this, map.coast_color,
                      map.coast_thickness);
            }
        }
    }


  //  Reset the stopFlag just in case.

  stopFlag = NVFalse;


  //  Let the parent do something after drawing coastlines but prior to drawing grid.

  if (signalsEnabled) emit midRedrawSignal (map);


  //  Draw the grid lines.

  if (clear)
    {
      NV_INT32 segx[2], segy[2];

      NV_INT32 start_y = (NV_INT32) ((map.bounds[map.zoom_level].min_y - 1.0) * 60.0);
      NV_INT32 end_y = (NV_INT32) ((map.bounds[map.zoom_level].max_y + 1.0) * 60.0);
      NV_INT32 start_x = (NV_INT32) ((map.bounds[map.zoom_level].min_x - 1.0) * 60.0);
      NV_INT32 end_x = (NV_INT32) ((map.bounds[map.zoom_level].max_x + 1.0) * 60.0);


      if (map.frame)
        {
          drawRectangle (map.bounds[map.zoom_level].min_x, map.bounds[map.zoom_level].min_y, 
                         map.bounds[map.zoom_level].max_x, map.bounds[map.zoom_level].max_y,
                         map.grid_color, map.grid_thickness, Qt::SolidLine, NVFalse);
        }
          
      if (map.grid_inc_x)
        {
          NV_FLOAT64 min_x = map.bounds[map.zoom_level].min_x * 60.0;
          NV_FLOAT64 max_x = map.bounds[map.zoom_level].max_x * 60.0;

          for (NV_INT32 x = start_x ; x < end_x ; x++)
            {
              if (!(x % (NV_INT32) map.grid_inc_x) && x > min_x && x < max_x)
                {
                  NV_FLOAT64 map_x = (NV_FLOAT64) x / 60.0;

                  map_to_screen (1, &map_x, &map.bounds[map.zoom_level].min_y, &dum_map_z[0], &segx[0], &segy[0],
                                 &dum_z[0]);

                  map_to_screen (1, &map_x, &map.bounds[map.zoom_level].max_y, &dum_map_z[0], &segx[1], &segy[1],
                                 &dum_z[0]);

                  drawLine (segx[0], segy[0], segx[1], segy[1], map.grid_color, map.grid_thickness, NVFalse,
                            Qt::SolidLine);
                }
            }
        }


      if (map.grid_inc_y)
        {
          NV_FLOAT64 min_y = map.bounds[map.zoom_level].min_y * 60.0;
          NV_FLOAT64 max_y = map.bounds[map.zoom_level].max_y * 60.0;

          for (NV_INT32 y = start_y ; y < end_y ; y++)
            {
              if (!(y % (NV_INT32) map.grid_inc_y) && y > min_y && y < max_y)
                {
                  NV_FLOAT64 map_y = (NV_FLOAT64) y / 60.0;

                  map_to_screen (1, &map.bounds[map.zoom_level].min_x, &map_y, &dum_map_z[0], &segx[0], &segy[0],
                                 &dum_z[0]);

                  map_to_screen (1, &map.bounds[map.zoom_level].max_x, &map_y, &dum_map_z[0], &segx[1], &segy[1],
                                 &dum_z[0]);

                  drawLine (segx[0], segy[0], segx[1], segy[1], map.grid_color, 2, NVFalse, Qt::SolidLine);
                }
            }
        }
    }


  //  Flush the paint buffer.

  update ();


  //  Let the parent do something after grid has been drawn.

  if (signalsEnabled) emit postRedrawSignal (map);


  //  Turn movable object painting back on.

  movable_object_count = save_movable_object_count;
}



/*******************************  IMPORTANT NOTE  ***********************************************/
/*!

    - There can only be MAX_MOVABLE_OBJECTS of the following rubberband or move operations active
      at any given time.  It is best to "discard" or "close" rubberband or moving objects prior
      to starting new ones.  Note that redrawMap and redrawMapArea will automatically discard any
      active rubberband or moving objects.  Also, note that the functions that actually do the 
      drawing do a repaint () instead of an update ().  This forces the paintevent function to
      perform the drawing immediately instead of trying to stack it and wait for later.

    - Create a movable object

************************************************************************************************/

NV_INT32 nvMap::create_movable_object (NVMAP_MOVABLE_OBJECT mo)
{
  for (NV_INT32 i = 0 ; i < MAX_MOVABLE_OBJECTS ; i++)
    {
      if (!movable_object[i].active)
        {
          movable_object[i] = mo;
          ot[movable_object_count] = &movable_object[i];
          movable_object_count++;
          return (i);
        }
    }

  return (-1);
}


//!  Get rid of a movable object

void nvMap::discard_movable_object (NV_INT32 *num)
{
  if (*num < 0 || *num >= MAX_MOVABLE_OBJECTS) return;


  for (NV_INT32 i = 0 ; i < MAX_MOVABLE_OBJECTS ; i++)
    {
      if (ot[i] == &movable_object[*num])
        {
          movable_object[*num].active = NVFalse;

          for (NV_INT32 j = i ; j < movable_object_count - 1 ; j++) ot[j] = ot[j + 1];

          movable_object_count--;

          *num = -1;
        }
    }
}



//  *************************************************************************************
//
//  Rubberband functions
//
//  *************************************************************************************


/*!  Starting a rubberband polygon in map space.  Use for first point only.  If you want to draw
     a polyline instead of a closed polygon, set close to NVFalse.  */

void 
nvMap::anchorRubberbandPolygon (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, QColor color, NV_INT32 line_width, NV_BOOL close,
                                Qt::PenStyle penStyle)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  return (anchorRubberbandPolygon (num, x, y, color, line_width, close, penStyle));
}



/*!  Starting a rubberband polygon in screen space.  Use for first point only.  If you want to draw
     a polyline instead of a closed polygon, set close to NVFalse.  */

void 
nvMap::anchorRubberbandPolygon (NV_INT32 *num, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, NV_BOOL close,
                                Qt::PenStyle penStyle)
{
  NVMAP_MOVABLE_OBJECT mo;


  mo.type = NVMAP_RUBBER_POLYGON;
  mo.color = color;
  mo.line_width = line_width;
  mo.penStyle = penStyle;
  mo.close_polygon = close;
  mo.filled = NVFalse;
  mo.outlined = NVTrue;
  mo.active = NVTrue;


  *num = create_movable_object (mo);


  if (*num >= 0)
    {
      movable_object[*num].polygon_x[0] = rubberbandPolyLastX[*num] = x;
      movable_object[*num].polygon_y[0] = rubberbandPolyLastY[*num] = y;

      movable_object[*num].polygon_count = 1;

      rubberbandPolyActive[*num] = NVTrue;
    }


  return;
}



//!  Dragging a line (Tommy James and the Shondells) from the previous vertex in a rubberband polygon (map space).

void 
nvMap::dragRubberbandPolygon (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  dragRubberbandPolygon (num, x, y);
}



//!  Dragging a line from the previous vertex in a rubberband polygon (screen space).

void 
nvMap::dragRubberbandPolygon (NV_INT32 num, NV_INT32 x, NV_INT32 y)
{
  if (rubberbandPolyActive[num] && (rubberbandPolyLastX[num] != x || rubberbandPolyLastY[num] != y))
    {
      //  Save for previous last point.

      rubberbandPolyLastX[num] = x;
      rubberbandPolyLastY[num] = y;

      repaint ();
    }
}



//!  Return the current polygon line segment anchor position in map coordinates.

void 
nvMap::getRubberbandPolygonCurrentAnchor (NV_INT32 num, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y)
{
  if (num < 0) return;


  screen_to_map (1, map_x, map_y, &dum_map_z[0], 
                 &movable_object[num].polygon_x[movable_object[num].polygon_count - 1], 
                 &movable_object[num].polygon_y[movable_object[num].polygon_count - 1], &dum_z[0]);
}



//!  Add a new vertex to a rubberband polygon in map space.

void 
nvMap::vertexRubberbandPolygon (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  vertexRubberbandPolygon (num, x, y);
}



//!  Add a new vertex to a rubberband polygon in screen space.

void 
nvMap::vertexRubberbandPolygon (NV_INT32 num, NV_INT32 x, NV_INT32 y)
{
  movable_object[num].polygon_x[movable_object[num].polygon_count] = rubberbandPolyLastX[num] = x;
  movable_object[num].polygon_y[movable_object[num].polygon_count] = rubberbandPolyLastY[num] = y;


  movable_object[num].polygon_count++;

  if (movable_object[num].polygon_count >= NVMAP_POLYGON_POINTS)
    {
      fprintf (stderr, "You have exceeded the maximum number of polygon points (%d).", 
               NVMAP_POLYGON_POINTS);
      fflush (stderr);
      discard_movable_object (&num);
      return;
    }
}



//!  Return the points in the current polygon (in screen [px/py] and map [mx/my] space) without closing the polygon.

void 
nvMap::getRubberbandPolygon (NV_INT32 num, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my)
{
  if (num < 0) return;

  *px = movable_object[num].polygon_x;
  *py = movable_object[num].polygon_y;
  *count = movable_object[num].polygon_count;

  screen_to_map (movable_object[num].polygon_count, rubberbandPolyMapX[num], rubberbandPolyMapY[num], dum_map_z, 
                 movable_object[num].polygon_x, movable_object[num].polygon_y, dum_z);

  *mx = rubberbandPolyMapX[num];
  *my = rubberbandPolyMapY[num];
}



/*!  Close a rubberband polygon in map space adding the input point (map_x, map_y).  Returns the
     points in the current polygon (in screen [px/py] and map [mx/my] space).  The arguments
     px, py, mx, and my should be declared in your calling function as NV_INT32 *px and 
     NV_FLOAT64 *mx.  They should be passed to this function as &px.  Remember to "discard"
     the polygon to free the associated allocated memory (returned as px, py, mx, my).  */

void 
nvMap::closeRubberbandPolygon (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 *count, NV_INT32 **px, 
                               NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  closeRubberbandPolygon (num, x, y, count, px, py, mx, my);
}



/*!  Close a rubberband polygon in screen space adding the input point (x, y).  Returns the
     points in the current polygon (in screen [px/py] and map [mx/my] space).  The arguments
     px, py, mx, and my should be declared in your calling function as NV_INT32 *px and 
     NV_FLOAT64 *mx.  They should be passed to this function as &px.  Remember to "discard"
     the polygon to free the associated allocated memory (returned as px, py, mx, my).  */

void 
nvMap::closeRubberbandPolygon (NV_INT32 num, NV_INT32 x, NV_INT32 y, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, 
                               NV_FLOAT64 **mx, NV_FLOAT64 **my)
{
  if (num < 0 || !rubberbandPolyActive[num]) return;


  rubberbandPolyActive[num] = NVFalse;

  vertexRubberbandPolygon (num, x, y);


  getRubberbandPolygon (num, count, px, py, mx, my);
}



//!  Returns whether a rubberband polygon is active (being created) at present.

NV_BOOL
nvMap::rubberbandPolygonIsActive (NV_INT32 num)
{
  if (num < 0) return (NVFalse);

  return (rubberbandPolyActive[num]);
}



//!  Erase the last rubberband polygon and free the associated allocated memory.

void 
nvMap::discardRubberbandPolygon (NV_INT32 *num)
{
  if (*num < 0 || !movable_object[*num].active) return;


  rubberbandPolyActive[*num] = NVFalse;


  discard_movable_object (num);


  repaint ();
}



//!  Anchor a rubberband rectangle in map space.  Starting corner point.

void 
nvMap::anchorRubberbandRectangle (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, QColor color, NV_INT32 line_width,
                                  Qt::PenStyle penStyle)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  return (anchorRubberbandRectangle (num, x, y, color, line_width, penStyle));
}



//!  Anchor a rubberband rectangle in screen space.  Starting corner point.

void 
nvMap::anchorRubberbandRectangle (NV_INT32 *num, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, Qt::PenStyle penStyle)
{
  NVMAP_MOVABLE_OBJECT mo;


  mo.color = color;
  mo.line_width = line_width;
  mo.penStyle = penStyle;
  mo.type = NVMAP_RUBBER_RECTANGLE;
  mo.filled = NVFalse;
  mo.outlined = NVTrue;
  mo.active = NVTrue;

  *num = create_movable_object (mo);

  if (*num >= 0)
    {
      rubberbandRectX[*num][0] = rubberbandRectLastX[*num] = x;
      rubberbandRectY[*num][0] = rubberbandRectLastY[*num] = y;


      rubberbandRectActive[*num] = NVTrue;
    }

  return;
}



//!  Drag a rubberband rectangle in map space.

void 
nvMap::dragRubberbandRectangle (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  dragRubberbandRectangle (num, x, y);
}



//!  Drag a rubberband rectangle in screen space.

void 
nvMap::dragRubberbandRectangle (NV_INT32 num, NV_INT32 x, NV_INT32 y)
{
  if (rubberbandRectActive[num] && (rubberbandRectLastX[num] != x || rubberbandRectLastY[num] != y))
    {
      rubberbandRectLastX[num] = x;
      rubberbandRectLastY[num] = y;

      movable_object[num].rect.setCoords (rubberbandRectX[num][0], rubberbandRectY[num][0], x, y);

      repaint ();
    }
}



//!  Return the points in the current rectangle (in screen [px/py] and map [mx/my] space) without closing the rectangle.

void 
nvMap::getRubberbandRectangle (NV_INT32 num, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my)
{
  if (num < 0) return;

  *px = rubberbandRectX[num];
  *py = rubberbandRectY[num];

  for (NV_INT32 i = 0 ; i < 4 ; i++)
    {
      screen_to_map (1, &rubberbandRectMapX[num][i], &rubberbandRectMapY[num][i], &dum_map_z[i], &rubberbandRectX[num][i],
                     &rubberbandRectY[num][i], &dum_z[i]);
    }

  *mx = rubberbandRectMapX[num];
  *my = rubberbandRectMapY[num];
}



/*!  Close a rubberband rectangle in map space.  The ending corner is specified by the point
     map_x,map_y.  Returns the points in the rectangle as a 4 point polygon in screen space
     (px, py) and map space (mx, my).  The arguments px, py, mx, and my should be declared
     in your calling function as NV_INT32 *px and NV_FLOAT64 *mx.  They should be passed to
     this function as &px.  */

void 
nvMap::closeRubberbandRectangle (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 **px, NV_INT32 **py, 
                                 NV_FLOAT64 **mx, NV_FLOAT64 **my)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  closeRubberbandRectangle (num, x, y, px, py, mx, my);
}



/*!  Close a rubberband rectangle in screen space.  The ending corner is specified by the point
     x,y.  Returns the points in the rectangle as a 4 point polygon in screen space (px, py)
     and map space (mx, my).  The arguments px, py, mx, and my should be declared in your calling
     function as NV_INT32 *px and NV_FLOAT64 *mx.  They should be passed to this function as &px.  */

void 
nvMap::closeRubberbandRectangle (NV_INT32 num, NV_INT32 x, NV_INT32 y, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, 
                                 NV_FLOAT64 **my)
{
  if (num < 0 || !rubberbandRectActive[num]) return;


  rubberbandRectActive[num] = NVFalse;


  rubberbandRectX[num][1] = rubberbandRectX[num][0];
  rubberbandRectY[num][1] = y;
  rubberbandRectX[num][2] = x;
  rubberbandRectY[num][2] = y;
  rubberbandRectX[num][3] = x;
  rubberbandRectY[num][3] = rubberbandRectY[num][0];

  *px = rubberbandRectX[num];
  *py = rubberbandRectY[num];

  screen_to_map (4, rubberbandRectMapX[num], rubberbandRectMapY[num], dum_map_z, rubberbandRectX[num], 
                 rubberbandRectY[num], dum_z);

  *mx = rubberbandRectMapX[num];
  *my = rubberbandRectMapY[num];
}



//!  Return whether a rubberband rectangle is active.

NV_BOOL 
nvMap::rubberbandRectangleIsActive (NV_INT32 num)
{
  if (num < 0) return (NVFalse);

  return (rubberbandRectActive[num]);
}



//!  Return the map anchor position of the current active rectangle.

void 
nvMap::getRubberbandRectangleAnchor (NV_INT32 num, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y)
{
  if (num < 0) return;

  screen_to_map (1, map_x, map_y, &dum_map_z[0], &rubberbandRectX[num][0], &rubberbandRectY[num][0], &dum_z[0]);
}



//!  Erase the rubberband rectangle.

void 
nvMap::discardRubberbandRectangle (NV_INT32 *num)
{
  if (*num < 0 || !movable_object[*num].active) return;


  rubberbandRectActive[*num] = NVFalse;


  discard_movable_object (num);


  repaint ();
}



//!  Anchor a rubberband line in map space.  Start point.

void 
nvMap::anchorRubberbandLine (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, QColor color, NV_INT32 line_width, Qt::PenStyle penStyle)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  return (anchorRubberbandLine (num, x, y, color, line_width, penStyle));
}



//!  Anchor a rubberband line in screen space.  Start point.

void 
nvMap::anchorRubberbandLine (NV_INT32 *num, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, Qt::PenStyle penStyle)
{
  NVMAP_MOVABLE_OBJECT mo;


  mo.color = color;
  mo.line_width = line_width;
  mo.penStyle = penStyle;
  mo.type = NVMAP_RUBBER_LINE;
  mo.filled = NVFalse;
  mo.outlined = NVTrue;
  mo.active = NVTrue;


  *num = create_movable_object (mo);


  if (*num >= 0)
    {
      rubberbandLineAnchorX[*num] = x;
      rubberbandLineAnchorY[*num] = y;

      rubberbandLineLastX[*num] = x;
      rubberbandLineLastY[*num] = y;

      rubberbandLineActive[*num] = NVTrue;
    }


  return;
}



//!  Drag a rubberband line in map space.

void 
nvMap::dragRubberbandLine (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  dragRubberbandLine (num, x, y);
}



//!  Drag a rubberband line in screen space.

void 
nvMap::dragRubberbandLine (NV_INT32 num, NV_INT32 x, NV_INT32 y)
{
  if (rubberbandLineActive[num] && (rubberbandLineLastX[num] != x || rubberbandLineLastY[num] != y))
    {
      movable_object[num].x0 = rubberbandLineAnchorX[num];
      movable_object[num].y0 = rubberbandLineAnchorY[num];
      movable_object[num].x1 = x;
      movable_object[num].y1 = y;

      repaint ();

      rubberbandLineLastX[num] = x;
      rubberbandLineLastY[num] = y;
    }
}



//!  Return whether a rubberband line is active.

NV_BOOL 
nvMap::rubberbandLineIsActive (NV_INT32 num)
{
  return (rubberbandLineActive[num]);
}



//!  Erase the rubberband line.

void 
nvMap::discardRubberbandLine (NV_INT32 *num)
{
  if (*num < 0 || !movable_object[*num].active) return;


  rubberbandLineActive[*num] = NVFalse;


  discard_movable_object (num);


  repaint ();
}



//  *************************************************************************************
//
//  Moving object functions
//
//  *************************************************************************************


/*!  Move a rectangle in map space without wiping out the underlying stuff.  The is defined by the
     two corner points map_x0,map_y0 and map_x1,map_y1.  The rectangle may be filled with the specified
     color by setting the "filled" argument to NVTrue.  */

void
nvMap::setMovingRectangle (NV_INT32 *num, NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_FLOAT64 map_x1, NV_FLOAT64 map_y1, 
                           QColor color, NV_INT32 line_width, NV_BOOL filled, Qt::PenStyle penStyle)
{
  NV_INT32 x0, y0, x1, y1;

  map_to_screen (1, &map_x0, &map_y0, &dum_map_z[0], &x0, &y0, &dum_z[0]);
  map_to_screen (1, &map_x1, &map_y1, &dum_map_z[0], &x1, &y1, &dum_z[0]);

  setMovingRectangle (num, x0, y0, x1, y1, color, line_width, filled, penStyle);
}



/*!  Move a rectangle in screen space without wiping out the underlying stuff.  The is defined by the
     two corner points x0,y0 and x1,y1.  The rectangle may be filled with the specified color by setting
     the "filled" argument to NVTrue.  */

void
nvMap::setMovingRectangle (NV_INT32 *num, NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color,
                           NV_INT32 line_width, NV_BOOL filled, Qt::PenStyle penStyle)
{
  //  If the object number is less than zero it has been discarded and, thus, needs to be created.

  if (*num < 0)
    {
      NVMAP_MOVABLE_OBJECT mo;

      mo.color = color;
      mo.line_width = line_width;
      mo.type = NVMAP_MOVE_RECTANGLE;
      mo.filled = filled;
      mo.penStyle = penStyle;
      mo.outlined = NVTrue;
      mo.active = NVTrue;
      mo.rect.setCoords (x0, y0, x1, y1);

      *num = create_movable_object (mo);
    }
  else
    {
      movable_object[*num].rect.setCoords (x0, y0, x1, y1);
    }


  repaint ();
}



//!  Get the coordinates for the current moving rectangle (in screen [px/py] and map [mx/my] space).

void 
nvMap::getMovingRectangle (NV_INT32 num, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my)
{
  rubberbandRectX[num][0] = movable_object[num].rect.left ();
  rubberbandRectY[num][0] = movable_object[num].rect.bottom ();
  rubberbandRectX[num][1] = movable_object[num].rect.left ();
  rubberbandRectY[num][1] = movable_object[num].rect.top ();
  rubberbandRectX[num][2] = movable_object[num].rect.right ();
  rubberbandRectY[num][2] = movable_object[num].rect.top ();
  rubberbandRectX[num][3] = movable_object[num].rect.right ();
  rubberbandRectY[num][3] = movable_object[num].rect.bottom ();

  *px = rubberbandRectX[num];
  *py = rubberbandRectY[num];

  screen_to_map (4, rubberbandRectMapX[num], rubberbandRectMapY[num], dum_map_z, rubberbandRectX[num], 
                 rubberbandRectY[num], dum_z);

  *mx = rubberbandRectMapX[num];
  *my = rubberbandRectMapY[num];
}



//!  Close a moving rectangle

void 
nvMap::closeMovingRectangle (NV_INT32 *num)
{
  if (*num < 0 || !movable_object[*num].active) return;

  discard_movable_object (num);

  repaint ();
}



/*!  Move a polygon in map space without wiping out the underlying stuff.  This is defined by the arrays
     map_x[] and map_y[].  The polygon may be filled with the specified color by setting the "filled"
     argument to NVTrue.  */

void
nvMap::setMovingPolygon (NV_INT32 *num, NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color,
                         NV_INT32 line_width, NV_BOOL filled, Qt::PenStyle penStyle)
{
  NV_INT32 *x = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_INT32 *z = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_FLOAT64 *dz = (NV_FLOAT64 *) calloc (count + 1, sizeof (NV_FLOAT64));

  map_to_screen (count, map_x, map_y, dz, x, y, z);

  free (dz);
  free (z);

  setMovingPolygon (num, count, x, y, color, line_width, filled, penStyle);

  free (x);
  free (y);
}



/*!  Move a polygon in screen space without wiping out the underlying stuff.  This is defined by the arrays
     x[] and y[].  The polygon may be filled with the specified color by setting the "filled" argument to
     NVTrue.  */

void
nvMap::setMovingPolygon (NV_INT32 *num, NV_INT32 count, NV_INT32 *x, NV_INT32 *y, QColor color, NV_INT32 line_width,
                         NV_BOOL filled, Qt::PenStyle penStyle)
{
  if (count > NVMAP_POLYGON_POINTS)
    {
      fprintf (stderr, "You have exceeded the maximum number of polygon points (%d).\nTerminating\n",
               NVMAP_POLYGON_POINTS);
      fflush (stderr);

      discard_movable_object (num);

      return;
    }


  //  If the object number is less than zero it has been discarded and, thus, needs to be created.

  if (*num < 0)
    {
      NVMAP_MOVABLE_OBJECT mo;

      mo.color = color;
      mo.line_width = line_width;
      mo.type = NVMAP_MOVE_POLYGON;
      mo.filled = filled;
      mo.penStyle = penStyle;
      mo.outlined = NVTrue;
      mo.active = NVTrue;
      for (NV_INT32 i = 0 ; i < count ; i++)
        {
          mo.polygon_x[i] = x[i];
          mo.polygon_y[i] = y[i];
        }
      mo.polygon_count = count;

      *num = create_movable_object (mo);
    }
  else
    {
      for (NV_INT32 i = 0 ; i < count ; i++)
        {
          movable_object[*num].polygon_x[i] = x[i];
          movable_object[*num].polygon_y[i] = y[i];
        }
      movable_object[*num].polygon_count = count;
    }


  repaint ();
}



//!  Return the points in the current moving polygon (in screen [px/py] and map [mx/my] space).

void 
nvMap::getMovingPolygon (NV_INT32 num, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my)
{
  getRubberbandPolygon (num, count, px, py, mx, my);
}



//!  Close a moving polygon.

void 
nvMap::closeMovingPolygon (NV_INT32 *num)
{
  if (*num < 0 || !movable_object[*num].active) return;

  discard_movable_object (num);

  repaint ();
}



//!  Move a polyline in map space without wiping out the underlying stuff.  This is defined by the arrays map_x[] and map_y[].

void
nvMap::setMovingPolyline (NV_INT32 *num, NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color,
                          NV_INT32 line_width, Qt::PenStyle penStyle)
{
  NV_INT32 *x = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_INT32 *z = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_FLOAT64 *dz = (NV_FLOAT64 *) calloc (count + 1, sizeof (NV_FLOAT64));

  map_to_screen (count, map_x, map_y, dz, x, y, z);

  free (dz);
  free (z);

  setMovingPolyline (num, count, x, y, color, line_width, penStyle);

  free (x);
  free (y);
}



//!  Move a polyline in screen space without wiping out the underlying stuff.  This is defined by the arrays x[] and y[].

void
nvMap::setMovingPolyline (NV_INT32 *num, NV_INT32 count, NV_INT32 *x, NV_INT32 *y, QColor color, NV_INT32 line_width,
                          Qt::PenStyle penStyle)
{
  if (count > NVMAP_POLYGON_POINTS)
    {
      fprintf (stderr, "You have exceeded the maximum number of polygon points (%d).\nTerminating\n",
               NVMAP_POLYGON_POINTS);
      fflush (stderr);

      discard_movable_object (num);

      return;
    }


  //  If the object number is less than zero it has been discarded and, thus, needs to be created.

  if (*num < 0)
    {
      NVMAP_MOVABLE_OBJECT mo;

      mo.color = color;
      mo.line_width = line_width;
      mo.penStyle = penStyle;
      mo.type = NVMAP_MOVE_POLYLINE;
      mo.active = NVTrue;
      for (NV_INT32 i = 0 ; i < count ; i++)
        {
          mo.polygon_x[i] = x[i];
          mo.polygon_y[i] = y[i];
        }
      mo.polygon_count = count;

      *num = create_movable_object (mo);
    }
  else
    {
      for (NV_INT32 i = 0 ; i < count ; i++)
        {
          movable_object[*num].polygon_x[i] = x[i];
          movable_object[*num].polygon_y[i] = y[i];
        }
      movable_object[*num].polygon_count = count;
    }


  repaint ();
}



//!  Return the points in the current moving polyline (in screen [px/py] and map [mx/my] space).

void 
nvMap::getMovingPolyline (NV_INT32 num, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my)
{
  getRubberbandPolygon (num, count, px, py, mx, my);
}



//!  Close a moving polyline.

void 
nvMap::closeMovingPolyline (NV_INT32 *num)
{
  if (*num < 0 || !movable_object[*num].active) return;

  discard_movable_object (num);

  repaint ();
}



/*!  Move a circle in map space without wiping out the underlying stuff.  The center of the circle will
     be at map_x,map_y and it will have diameter "d" in pixels.  start_angle and span_angle are in degrees
     but are converted to 16ths of a degree for Qt.  */

void
nvMap::setMovingCircle (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 d, NV_FLOAT32 start_angle, 
                        NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, NV_BOOL filled,
                        Qt::PenStyle penStyle)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  setMovingCircle (num, x, y, d, start_angle, span_angle, line_width, color, filled, penStyle);
}



/*!  Move a circle in map space without wiping out the underlying stuff.  The center of the circle will
     be at map_x,map_y and the diameter of the circle will be twice the distance to end_map_x,end_map_y.
     start_angle and span_angle are in degrees but are converted to 16ths of a degree for Qt.  */

void
nvMap::setMovingCircle (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, 
                        NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color,
                        NV_BOOL filled, Qt::PenStyle penStyle)
{
  NV_INT32 x0, y0, x1, y1, x2, y2, d;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x0, &y0, &dum_z[0]);
  map_to_screen (1, &end_map_x, &end_map_y, &dum_map_z[0], &x1, &y1, &dum_z[0]);

  if (x1 > x0)
    {
      x2 = x1 - x0;
    }
  else
    {
      x2 = x0 - x1;
    }

  if (y1 > y0)
    {
      y2 = y1 - y0;
    }
  else
    {
      y2 = y0 - y1;
    }

  d = NINT (sqrt (NV_FLOAT64 ((y2 * y2) + (x2 * x2))) * 2.0);

  if (d < 1) d = 1;

  setMovingCircle (num, x0, y0, d, start_angle, span_angle, line_width, color, filled, penStyle);
}



/*!  Move a circle in screen space without wiping out the underlying stuff.  The center of the circle will
     be at x,y and it will have diameter "d" in pixels.  start_angle and span_angle are in degrees
     but are converted to 16ths of a degree for Qt.  */

void
nvMap::setMovingCircle (NV_INT32 *num, NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, 
                        NV_INT32 line_width, QColor color, NV_BOOL filled, Qt::PenStyle penStyle)
{
  //  If the object number is less than zero it has been discarded and, thus, needs to be created.

  if (*num < 0)
    {
      NVMAP_MOVABLE_OBJECT mo;

      mo.color = color;
      mo.line_width = line_width;
      mo.type = NVMAP_MOVE_CIRCLE;
      mo.filled = filled;
      mo.penStyle = penStyle;
      mo.outlined = NVTrue;
      mo.x = x;
      mo.y = y;
      mo.width = d;
      mo.height = d;
      mo.start_angle = NINT (start_angle * 16.0);
      mo.span_angle = NINT (span_angle * 16.0);
      mo.active = NVTrue;

      *num = create_movable_object (mo);
    }
  else
    {
      movable_object[*num].x = x;
      movable_object[*num].y = y;
    }


  repaint ();
}



//!  Get the current moving circle (in screen and map coordinates) with diameter "d" in pixels.

void
nvMap::getMovingCircle (NV_INT32 num, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_INT32 *x, NV_INT32 *y, NV_INT32 *d)
{
  *x = movable_object[num].x;
  *y = movable_object[num].y;
  *d = movable_object[num].width;

  screen_to_map (1, map_x, map_y, &dum_map_z[0], x, y, &dum_z[0]);
}



//!  Close a moving circle.

void 
nvMap::closeMovingCircle (NV_INT32 *num)
{
  if (*num < 0 || !movable_object[*num].active) return;

  discard_movable_object (num);

  repaint ();
}



/*!
    Move a painter path in map space.  You can define any type of object using a painter path in
    your calling function.  For example:

    <pre>

    QPainterPath bullseye = QPainterPath ();

    bullseye.moveTo (0, 0);
    bullseye.lineTo (30, 0);
    bullseye.lineTo (30, 20);
    bullseye.lineTo (0, 20);
    bullseye.lineTo (0, 0);

    bullseye.moveTo (0, 10);
    bullseye.lineTo (12, 10);

    bullseye.moveTo (30, 10);
    bullseye.lineTo (18, 10);

    bullseye.moveTo (15, 0);
    bullseye.lineTo (15, 6);

    bullseye.moveTo (15, 20);
    bullseye.lineTo (15, 14);

    This creates a little target kind of thing that looks something like this:

    _______________
    |      |      |
    |      |      |
    |----     ----|
    |      |      |
    |      |      |
    ---------------

    </pre>

    The path will be drawn with its center at map_center_x,map_center_y based on the bounding rectangle of 
    the path.  If you wish to fill the path you'll need to set the "brush" attributes and set the "filled"
    argument to NVTrue.  You can rotate the path by setting the angle value.

*/

void 
nvMap::setMovingPath (NV_INT32 *num, QPainterPath path, NV_FLOAT64 map_center_x, NV_FLOAT64 map_center_y,
                      NV_INT32 line_width, QColor color, QBrush brush, NV_BOOL filled, NV_FLOAT64 angle,
                      Qt::PenStyle penStyle)
{
  NV_INT32 center_x, center_y;

  map_to_screen (1, &map_center_x, &map_center_y, &dum_map_z[0], &center_x, &center_y, &dum_z[0]);

  setMovingPath (num, path, center_x, center_y, line_width, color, brush, filled, angle, penStyle);
}



void 
nvMap::setMovingPath (NV_INT32 *num, QPainterPath path, NV_FLOAT64 map_center_x, NV_FLOAT64 map_center_y,
                      NV_INT32 line_width, QColor color, QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle)
{
  setMovingPath (num, path, map_center_x, map_center_y, line_width, color, brush, filled, 0.0, penStyle);
}



/*!  Move a painter path in screen space.  See above for example.  The path will be drawn with its center
     at center_x,center_y based on the bounding rectangle of the path.  If you wish to fill the path you'll
     need to set the "brush" attributes and set the "filled" argument to NVTrue.  You can rotate the path by setting
     the angle value.  */

void 
nvMap::setMovingPath (NV_INT32 *num, QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width,
                      QColor color, QBrush brush, NV_BOOL filled, NV_FLOAT64 angle, Qt::PenStyle penStyle)
{
  //  If the object number is less than zero it has been discarded and, thus, needs to be created.

  if (*num < 0)
    {
      NVMAP_MOVABLE_OBJECT mo;

      mo.color = color;
      mo.line_width = line_width;
      mo.type = NVMAP_MOVE_PATH;
      mo.path = path;
      mo.filled = filled;
      mo.penStyle = penStyle;
      mo.brush = brush;
      mo.outlined = NVTrue;
      mo.x = center_x;
      mo.y = center_y;
      mo.rotation_angle = angle;
      mo.active = NVTrue;

      *num = create_movable_object (mo);
    }
  else
    {
      movable_object[*num].x = center_x;
      movable_object[*num].y = center_y;
      movable_object[*num].rotation_angle = angle;
    }

  repaint ();
}



void 
nvMap::setMovingPath (NV_INT32 *num, QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width,
                      QColor color, QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle)
{
  setMovingPath (num, path, center_x, center_y, line_width, color, brush, filled, 0.0, penStyle);
}



//!  Close a moving path.

void 
nvMap::closeMovingPath (NV_INT32 *num)
{
  if (*num < 0 || !movable_object[*num].active) return;

  discard_movable_object (num);

  repaint ();
}



//!  Move text in map space.

void 
nvMap::setMovingText (NV_INT32 *num, QString text, NV_FLOAT64 map_center_x, NV_FLOAT64 map_center_y,
                      QColor color, NV_FLOAT64 angle, NV_INT32 size)
{
  NV_INT32 center_x, center_y;

  map_to_screen (1, &map_center_x, &map_center_y, &dum_map_z[0], &center_x, &center_y, &dum_z[0]);

  setMovingText (num, text, center_x, center_y, color, angle, size);
}



//!  Move text in screen space.

void 
nvMap::setMovingText (NV_INT32 *num, QString text, NV_INT32 center_x, NV_INT32 center_y, QColor color, NV_FLOAT64 angle, NV_INT32 size)
{
  //  If the object number is less than zero it has been discarded and, thus, needs to be created.

  if (*num < 0)
    {
      NVMAP_MOVABLE_OBJECT mo;

      mo.color = color;
      mo.height = size;
      mo.type = NVMAP_MOVE_TEXT;
      mo.text = text;
      mo.x = center_x;
      mo.y = center_y;
      mo.rotation_angle = angle;
      mo.active = NVTrue;

      *num = create_movable_object (mo);
    }
  else
    {
      movable_object[*num].x = center_x;
      movable_object[*num].y = center_y;
      movable_object[*num].rotation_angle = angle;
    }

  repaint ();
}



//!  Close moving text.

void 
nvMap::closeMovingText (NV_INT32 *num)
{
  if (*num < 0 || !movable_object[*num].active) return;

  discard_movable_object (num);

  repaint ();
}



//  *************************************************************************************
//
//  Drawing functions
//
//  *************************************************************************************

/*!  - All of the following functions have a "flush" argument.  If it is set to NVTrue then the operation will be
       flushed to the screen immediately.  Otherwise, the application must handle flushing by calling nvMap::flush ()
       after all of the drawing is finished.
     - Draw text in map space.  The text will start with the lower left corner at
       map_x,map_y.  Size is in pixels.  Use an angle of 90.0 for normal horizontal text.
       Clear version blanks the surrounding rectangle to the background color.  */

void
nvMap::drawText (QString string, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 angle, NV_INT32 size, QColor color, NV_BOOL flush)
{
  drawTextClear (string, map_x, map_y, angle, size, color, NVFalse, flush);
}



void
nvMap::drawTextClear (QString string, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 angle, NV_INT32 size, QColor color,
                      NV_BOOL clear, NV_BOOL flush)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  drawTextClear (string, x, y, angle, size, color, clear, flush);
}



/*!  Draw text in screen space.  The text will start with the lower left corner at
     x,y.  Size is in pixels.  Use an angle of 90.0 for normal horizontal text.
     Clear version blanks the surrounding rectangle to the background color.  */

void
nvMap::drawText (QString string, NV_INT32 x, NV_INT32 y, NV_FLOAT64 angle, NV_INT32 size, QColor color, NV_BOOL flush)
{
  drawTextClear (string, x, y, angle, size, color, NVFalse, flush);
}



//!  Draw text in screen space but clear the background.

void
nvMap::drawTextClear (QString string, NV_INT32 x, NV_INT32 y, NV_FLOAT64 angle, NV_INT32 size, QColor color, NV_BOOL clear, NV_BOOL flush)
{
  QFont font ("Charter", size);
  font = QFont (font, painter.device ());
  font.setWeight (QFont::Bold);
  QFontMetrics fontMetrics (font);
  QRect rect = fontMetrics.boundingRect (string);

  painter.save ();
  painter.translate (x, y);
  painter.rotate (angle - 90.0);

  if (clear)
    {
      QBrush brush;
      brush.setStyle (Qt::SolidPattern);
      brush.setColor (map.background_color);
      painter.setPen (map.background_color);
      painter.fillRect (rect, brush);
    }


  painter.setFont(font);

  painter.setPen (color);

  QRect r;
  painter.drawText (rect, Qt::AlignLeft | Qt::AlignBottom, string, &r); 

  painter.restore ();

  if (flush) update (r);
}



//!  Draw text in map space.  Uses a default angle of 90 degrees and height of 12 pixels.

void
nvMap::drawText (QString string, NV_FLOAT64 map_x, NV_FLOAT64 map_y, QColor color, NV_BOOL flush)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  drawText (string, x, y, 90.0, 12, color, flush);
}



//!  Draw text in screen space.  Uses a default angle of 90 degrees and height of 12 pixels.

void
nvMap::drawText (QString string, NV_INT32 x, NV_INT32 y, QColor color, NV_BOOL flush)
{
  drawText (string, x, y, 90.0, 12, color, flush);
}



/*!  Draw a line in map space (optionally flush to screen).  The line is defined by the points
     map_x0,map_y0 and map_x1,map_y1.  Note: if you don't flush to the screen you will have to
     keep track of what needs to be updated or you will have to call flush for the entire
     screen area (i.e. dump the pixmap to the screen).  In practice, flush usually works just
     fine.  */

void
nvMap::drawLine (NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_FLOAT64 map_x1, NV_FLOAT64 map_y1,
                 QColor color, NV_INT32 line_width, NV_BOOL flush, Qt::PenStyle penStyle)
{
  NV_INT32 x0, y0, x1, y1;

  map_to_screen (1, &map_x0, &map_y0, &dum_map_z[0], &x0, &y0, &dum_z[0]);
  map_to_screen (1, &map_x1, &map_y1, &dum_map_z[0], &x1, &y1, &dum_z[0]);

  drawLine (x0, y0, x1, y1, color, line_width, flush, penStyle);
}



/*!  Draw a line in screen space (optionally flush to screen).  The line is defined by the points
     x0,y0 and x1,y1.  Note: if you don't flush to the screen you will have to keep track of what
     needs to be updated or you will have to call flush for the entire screen area (i.e. dump the
     pixmap to the screen).  In practice, flush usually works just fine.  */

void
nvMap::drawLine (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width, NV_BOOL flush,
                 Qt::PenStyle penStyle)
{
  QPen pen;
  pen.setColor (color);
  pen.setWidth (line_width);
  pen.setStyle (penStyle);

  painter.setPen (pen);

  painter.drawLine (x0, y0, x1, y1);

  if (flush)
    {
      QRect r;
      r.setCoords (x0, y0, x1, y1);

      r = r.normalized ();
      r.setLeft (r.left () - 1);
      r.setTop (r.top () - 1);
      r.setRight (r.right () + 1);
      r.setBottom (r.bottom () + 1);

      update (r);
    }
}



/*!  Draw a circle in map space.  The center of the circle will be at map_x,map_y and it will
     have diameter "d" in pixels.  start_angle and span_angle are in degrees
     but are converted to 16ths of a degree for Qt.  */

void
nvMap::drawCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                   NV_INT32 line_width, QColor color, Qt::PenStyle penStyle, NV_BOOL flush)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  drawCircle (x, y, d, start_angle, span_angle, line_width, color, penStyle, flush);
}



/*!  Draw a circle in map space.  The center of the circle will be at map_x,map_y and the diameter
     of the circle will twice the distance to end_map_x,end_map_y.  start_angle and span_angle
     are in degrees but are converted to 16ths of a degree for Qt.  */

void
nvMap::drawCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, NV_FLOAT32 start_angle,
                   NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, Qt::PenStyle penStyle, NV_BOOL flush)
{
  NV_INT32 x0, y0, x1, y1, x2, y2, d;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x0, &y0, &dum_z[0]);
  map_to_screen (1, &end_map_x, &end_map_y, &dum_map_z[0], &x1, &y1, &dum_z[0]);

  if (x1 > x0)
    {
      x2 = x1 - x0;
    }
  else
    {
      x2 = x0 - x1;
    }

  if (y1 > y0)
    {
      y2 = y1 - y0;
    }
  else
    {
      y2 = y0 - y1;
    }

  d = NINT (sqrt (NV_FLOAT64 ((y2 * y2) + (x2 * x2))) * 2.0);

  if (d < 1) d = 1;

  drawCircle (x0, y0, d, start_angle, span_angle, line_width, color, penStyle, flush);
}



/*!  Draw a circle in screen space.  The center of the circle will be at map_x,map_y and it will
     have diameter "d" in pixels.  start_angle and span_angle are in degrees but are converted
     to 16ths of a degree for Qt.  */

void
nvMap::drawCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                   NV_INT32 line_width, QColor color, Qt::PenStyle penStyle, NV_BOOL flush)
{
  //  Center the circle.

  NV_INT32 newx = x - d / 2;
  NV_INT32 newy = y - d / 2;


  QPen pen;
  pen.setColor (color);
  pen.setWidth (line_width);
  pen.setStyle (penStyle);
  painter.setPen (pen);

  painter.drawArc (newx, newy, d, d, NINT (start_angle * 16.0), NINT (span_angle * 16.0));

  if (flush) update (newx, newy, d, d);
}



/*!  Draw a filled circle in map space.  The center of the circle will be at map_x,map_y and it will
     have diameter "d" in pixels.  start_angle and span_angle are in degrees but are converted
     to 16ths of a degree for Qt.  */

void
nvMap::fillCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 d, NV_FLOAT32 start_angle, 
                   NV_FLOAT32 span_angle, QColor color, NV_BOOL flush)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  fillCircle (x, y, d, start_angle, span_angle, color, flush);
}



/*!  Draw a filled circle in map space.  The center of the circle will be at map_x,map_y and the diameter
     of the circle will twice the distance to end_map_x,end_map_y.  start_angle and span_angle
     are in degrees but are converted to 16ths of a degree for Qt.  */

void
nvMap::fillCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, 
                   NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, QColor color, NV_BOOL flush)
{
  NV_INT32 x0, y0, x1, y1, x2, y2, d;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x0, &y0, &dum_z[0]);
  map_to_screen (1, &end_map_x, &end_map_y, &dum_map_z[0], &x1, &y1, &dum_z[0]);

  if (x1 > x0)
    {
      x2 = x1 - x0;
    }
  else
    {
      x2 = x0 - x1;
    }

  if (y1 > y0)
    {
      y2 = y1 - y0;
    }
  else
    {
      y2 = y0 - y1;
    }

  d = NINT (sqrt (NV_FLOAT64 ((y2 * y2) + (x2 * x2))) * 2.0);

  if (d < 1) d = 1;

  fillCircle (x0, y0, d, start_angle, span_angle, color, flush);
}



/*!  Draw a filled circle in screen space.  The center of the circle will be at map_x,map_y and it will
     have diameter "d" in pixels.  start_angle and span_angle are in degrees but are converted to
     16ths of a degree for Qt.  */

void
nvMap::fillCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, QColor color, NV_BOOL flush)
{
  //  Center the circle.

  NV_INT32 newx = x - d / 2;
  NV_INT32 newy = y - d / 2;


  QBrush brush;
  brush.setStyle (Qt::SolidPattern);
  brush.setColor (color);
  painter.setPen (color);


  painter.setBrush (brush);

  painter.drawChord (newx, newy, d, d, NINT (start_angle * 16.0), NINT (span_angle * 16.0));
  //painter.drawPie (newx, newy, d, d, NINT (start_angle * 16.0), NINT (span_angle * 16.0));

  if (flush) update (newx, newy, d, d);
}



/*!  Draw and fill a rectangle in map space (optionally flush to screen).  The upper left corner of the
     rectangle is defined by map_x,map_y and it will be "w" pixels wide by "h" pixels high.  Note: if
     you don't flush to the screen you will have to keep track of what needs to be updated or you will
     have to call flush for the entire screen area (i.e. dump the pixmap to the screen).  */

void 
nvMap::fillRectangle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush)
{
  NV_INT32 x, y;

  map_to_screen (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

  fillRectangle (x, y, w, h, color, flush);
}



/*!  Draw and fill a rectangle in map space (optionally flush to screen).  The upper left corner of the
     rectangle is defined by map_x,map_y and the lower right corner by end_map_x,end_map_y.  Note: if
     you don't flush to the screen you will have to keep track of what needs to be updated or you will
     have to call flush for the entire screen area (i.e. dump the pixmap to the screen).  */

void 
nvMap::fillRectangle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, QColor color,
                    NV_BOOL flush)
{
  NV_INT32 x, y, w, h;

  map_to_screen (1, &map_x, &end_map_y, &dum_map_z[0], &x, &y, &dum_z[0]);
  map_to_screen (1, &end_map_x, &map_y, &dum_map_z[0], &w, &h, &dum_z[0]);
  w = w - x;
  h = h - y;

  if (h < 1) h = 1;
  if (w < 1) w = 1;

  fillRectangle (x, y, w, h, color, flush);
}



/*!  Draw and fill a rectangle in screen space (optionally flush to screen).  The lower left corner of the
     rectangle is defined by x,y and it will be "w" pixels wide by "h" pixels high.  Note: if you don't
     flush to the screen you will have to keep track of what needs to be updated or you will have to call
     flush for the entire screen area (i.e. dump the pixmap to the screen).  */

void
nvMap::fillRectangle (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush)
{
  //  Don't outline the rectangle

  painter.setPen (Qt::NoPen);

  QBrush brush;
  brush.setStyle (Qt::SolidPattern);
  brush.setColor (color);

  painter.setBrush (brush);

  painter.fillRect (x, y, w, h, brush);

  if (flush) update (x, y, w, h);
}



/*!  Draw a rectangle in map space.  The upper left corner of the rectangle will be at map_x0,map_y0
     and the lower right corner will be at map_x1,map_y1.  */

void
nvMap::drawRectangle (NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_FLOAT64 map_x1, NV_FLOAT64 map_y1,
                      QColor color, NV_INT32 line_width, Qt::PenStyle penStyle, NV_BOOL flush)
{
  NV_INT32 x0, y0, x1, y1;

  map_to_screen (1, &map_x0, &map_y0, &dum_map_z[0], &x0, &y0, &dum_z[0]);
  map_to_screen (1, &map_x1, &map_y1, &dum_map_z[0], &x1, &y1, &dum_z[0]);

  drawRectangle (x0, y0, x1, y1, color, line_width, penStyle, flush);
}



/*!  Draw a rectangle in map space.  The upper left corner of the rectangle will be at map_x0,map_y0
     and width and height in pixels will be w and h.  */

void
nvMap::drawRectangle (NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_INT32 w, NV_INT32 h,
                      QColor color, NV_INT32 line_width, Qt::PenStyle penStyle, NV_BOOL flush)
{
  NV_INT32 x0, y0;

  map_to_screen (1, &map_x0, &map_y0, &dum_map_z[0], &x0, &y0, &dum_z[0]);

  drawRectangle (x0, y0, x0 + w, y0 + h, color, line_width, penStyle, flush);
}



/*!  - Draw a rectangle in screen space.  The upper left corner of the rectangle will be at x0,y0 and
       the lower right corner will be at x1,y1.

     - So why am I doing four lines instead of a rectangle.  A stupid X bug
       I think.  It has to do with trying to make a rectangle where the start 
       position is less in X or Y than the end position.  I had the same problems 
       using X11 directly.
*/

void
nvMap::drawRectangle (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width,
                      Qt::PenStyle penStyle, NV_BOOL flush)
{
  drawLine (x0, y0, x0, y1, color, line_width, NVTrue, penStyle);
  drawLine (x0, y1, x1, y1, color, line_width, NVTrue, penStyle);
  drawLine (x1, y1, x1, y0, color, line_width, NVTrue, penStyle);
  drawLine (x1, y0, x0, y0, color, line_width, NVTrue, penStyle);

  QRect r;
  r.setCoords (x0, y0, x1, y1);
  if (flush) update (r.normalized ());
}



//!  Draw and fill a polygon in map space.

void
nvMap::fillPolygon (NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color, NV_BOOL flush)
{
  NV_INT32 *x = (NV_INT32 *) calloc (count, sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) calloc (count, sizeof (NV_INT32));
  NV_INT32 *z = (NV_INT32 *) calloc (count, sizeof (NV_INT32));
  NV_FLOAT64 *dz = (NV_FLOAT64 *) calloc (count, sizeof (NV_FLOAT64));

  map_to_screen (count, map_x, map_y, dz, x, y, z);

  free (dz);
  free (z);

  fillPolygon (count, x, y, color, flush);

  free (x);
  free (y);
}



//!  Draw and fill a polygon in map space using NV_F64_COORD2 array for X and Y.

void
nvMap::fillPolygon (NV_INT32 count, NV_F64_COORD2 map_xy[], QColor color, NV_BOOL flush)
{
  NV_FLOAT64 *x = (NV_FLOAT64 *) calloc (count, sizeof (NV_FLOAT64));
  NV_FLOAT64 *y = (NV_FLOAT64 *) calloc (count, sizeof (NV_FLOAT64));

  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      x[i] = map_xy[i].x;
      y[i] = map_xy[i].y;
    }

  fillPolygon (count, x, y, color, flush);

  free (x);
  free (y);
}



//!  Draw and fill a polygon in screen space.

void
nvMap::fillPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color, NV_BOOL flush)
{
  //  Don't outline the rectangle

  painter.setPen (Qt::NoPen);

  QBrush brush;
  brush.setStyle (Qt::SolidPattern);
  brush.setColor (color);

  painter.setBrush (brush);


  QPolygon poly;
  poly = QPolygon (count);

  for (NV_INT32 i = 0 ; i < count ; i++) poly.setPoint (i, x[i], y[i]);

  painter.drawPolygon (poly);

  if (flush)
    {
      QRect r = poly.boundingRect ();

      r = r.normalized ();
      r.setLeft (r.left () - 1);
      r.setTop (r.top () - 1);
      r.setRight (r.right () + 1);
      r.setBottom (r.bottom () + 1);

      update (r.normalized ());
    }
}



//!  Draw and fill a polygon in screen space using NV_I32_COORD2 array for X and Y.

void
nvMap::fillPolygon (NV_INT32 count, NV_I32_COORD2 xy[], QColor color, NV_BOOL flush)
{
  NV_INT32 *x = (NV_INT32 *) calloc (count, sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) calloc (count, sizeof (NV_INT32));

  for (NV_INT32 i = 0 ; i < count ; i++) 
    {
      x[i] = xy[i].x;
      y[i] = xy[i].y;
    }

  fillPolygon (count, x, y, color, flush);

  free (x);
  free (y);
}



//!  Draw a polygon in map space.  To draw a polyline, set "close" to NVFalse.

void
nvMap::drawPolygon (NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color, NV_INT32 line_width,
                    NV_BOOL close, Qt::PenStyle penStyle, NV_BOOL flush)
{
  NV_INT32 *x = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_INT32 *z = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_FLOAT64 *dz = (NV_FLOAT64 *) calloc (count + 1, sizeof (NV_FLOAT64));

  map_to_screen (count, map_x, map_y, dz, x, y, z);

  free (dz);
  free (z);

  drawPolygon (count, x, y, color, line_width, close, penStyle, flush);

  free (x);
  free (y);
}



//!  Draw a polygon in map space using NV_F64_COORD2 array for X and Y.  To draw a polyline, set "close" to NVFalse.

void
nvMap::drawPolygon (NV_INT32 count, NV_F64_COORD2 map_xy[], QColor color, NV_INT32 line_width,
                    NV_BOOL close, Qt::PenStyle penStyle, NV_BOOL flush)
{
  NV_FLOAT64 *x = (NV_FLOAT64 *) calloc (count, sizeof (NV_FLOAT64));
  NV_FLOAT64 *y = (NV_FLOAT64 *) calloc (count, sizeof (NV_FLOAT64));

  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      x[i] = map_xy[i].x;
      y[i] = map_xy[i].y;
    }

  drawPolygon (count, x, y, color, line_width, close, penStyle, flush);

  free (x);
  free (y);
}



//!  Draw a polygon in screen space.  To draw a polyline, set "close" to NVFalse.

void
nvMap::drawPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color, NV_INT32 line_width, NV_BOOL close, 
                    Qt::PenStyle penStyle, NV_BOOL flush)
{
  //  Make sure we close the polygon.

  NV_INT32 *new_x = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_INT32 *new_y = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));

  for (NV_INT32 i = 0 ; i < count ; i++) 
    {
      new_x[i] = x[i];
      new_y[i] = y[i];
    }
  new_x[count] = x[0];
  new_y[count] = y[0];


  //  Check for closure.

  if (!close) count = count - 1;


  for (NV_INT32 i = 0 ; i < count ; i++) drawLine (new_x[i], new_y[i], new_x[i + 1], new_y[i + 1], color, line_width,
                                                   flush, penStyle);

  free (new_x);
  free (new_y);
}



//!  Draw a polygon in screen space using an NV_I32_COORD2 array for X and Y.  To draw a polyline, set "close" to NVFalse.

void
nvMap::drawPolygon (NV_INT32 count, NV_I32_COORD2 xy[], QColor color, NV_INT32 line_width, NV_BOOL close, 
                    Qt::PenStyle penStyle, NV_BOOL flush)
{
  NV_INT32 *x = (NV_INT32 *) calloc (count, sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) calloc (count, sizeof (NV_INT32));

  for (NV_INT32 i = 0 ; i < count ; i++) 
    {
      x[i] = xy[i].x;
      y[i] = xy[i].y;
    }

  drawPolygon (count, x, y, color, line_width, close, penStyle, flush);

  free (x);
  free (y);
}



/*!
    Draw a painter path in map space.  You can define any type of object using a painter path in
    your calling function.  For example:

    <pre>

    QPainterPath bullseye = QPainterPath ();

    bullseye.moveTo (0, 0);
    bullseye.lineTo (30, 0);
    bullseye.lineTo (30, 20);
    bullseye.lineTo (0, 20);
    bullseye.lineTo (0, 0);

    bullseye.moveTo (0, 10);
    bullseye.lineTo (12, 10);

    bullseye.moveTo (30, 10);
    bullseye.lineTo (18, 10);

    bullseye.moveTo (15, 0);
    bullseye.lineTo (15, 6);

    bullseye.moveTo (15, 20);
    bullseye.lineTo (15, 14);


    This creates a little target kind of thing that looks something like this:

    _______________
    |      |      |
    |      |      |
    |----     ----|
    |      |      |
    |      |      |
    ---------------

    </pre>


    The path will be drawn with its center at map_center_x,map_center_y based on the bounding rectangle of 
    the path.  If you wish to fill the path you'll need to set the "brush" attributes and set the "filled"
    argument to NVTrue.

*/

void 
nvMap::drawPath (QPainterPath path, NV_FLOAT64 map_center_x, NV_FLOAT64 map_center_y, NV_INT32 line_width, QColor color,
                 QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle, NV_BOOL flush)
{
  NV_INT32 center_x, center_y;

  map_to_screen (1, &map_center_x, &map_center_y, &dum_map_z[0], &center_x, &center_y, &dum_z[0]);

  drawPath (path, center_x, center_y, line_width, color, brush, filled, penStyle, flush);
}



/*!  Draw a painter path in screen space.  See above for example.  The path will be drawn with its center
     at center_x,center_y based on the bounding rectangle of the path.  If you wish to fill the path you'll
     need to set the "brush" attributes and set the "filled" argument to NVTrue.  */

void 
nvMap::drawPath (QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width, QColor color,
                 QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle, NV_BOOL flush)
{
  //  Translate the center of the painter path to the provided x and y.

  QRectF r2 = path.boundingRect ();

  NV_INT32 x_offset = center_x - (NV_INT32) (r2.width () / 2.0);
  NV_INT32 y_offset = center_y - (NV_INT32) (r2.height () / 2.0);

  painter.save ();
  painter.translate (x_offset, y_offset);


  if (filled)
    {
      painter.setPen (color);
      painter.setBrush (brush);

      painter.drawPath (path);
    }
  else
    {
      QPen pen;
      pen.setColor (color);
      pen.setWidth (line_width);
      pen.setStyle (penStyle);
      painter.setPen (pen);

      painter.drawPath (path);
    }

  painter.restore ();

  if (flush) update ();
}



/*!  Draw a pixmap at screen location x,y.  The pixmap is "src".  The location src_x,src_y
     is where in the pixmap you're going to get the upper left corner of what you want to
     paint (normally 0,0).  The width will be src_w and the height will be src_h.  */

void 
nvMap::drawPixmap (NV_INT32 x, NV_INT32 y, QPixmap *src, NV_INT32 src_x, NV_INT32 src_y, NV_INT32 src_w, NV_INT32 src_h, NV_BOOL flush)
{
  painter.drawPixmap (x, y, *src, src_x, src_y, src_w, src_h);

  if (flush) update (x, y, src_w, src_h);
}



//!  Grab a pixmap at screen location x,y.  Width is w.  Height is h.

QPixmap 
nvMap::grabPixmap (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h)
{
  return (buffer.copy (x, y, w, h));
}


//!  Grab the entire pixmap.

QPixmap 
nvMap::grabPixmap ()
{
  NV_INT32 start = map.draw_height - map.height - map.border;
  return (buffer.copy (map.border, start, map.width, map.height));
}



//  *************************************************************************************
//
//  Utility functions
//
//  *************************************************************************************


/*!  Convert map coordinates to screen coordinates.  Z is usually ignored.  This version returns
     NV_FLOAT64 pixels so you don't have to convert them going into some other functions.  */

void 
nvMap::map_to_screen (NV_INT32 num_points, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], 
                      NV_FLOAT64 map_z[], NV_FLOAT64 pix_x[], NV_FLOAT64 pix_y[], NV_FLOAT64 pix_z[])
{
  NV_INT32 *x = (NV_INT32 *) malloc (num_points * sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) malloc (num_points * sizeof (NV_INT32));
  NV_INT32 *z = (NV_INT32 *) malloc (num_points * sizeof (NV_INT32));

  map_to_screen (num_points, map_x, map_y, map_z, x, y, z);

  for (NV_INT32 i = 0 ; i < num_points ; i++)
    {
      pix_x[i] = (NV_FLOAT64) x[i];
      pix_y[i] = (NV_FLOAT64) y[i];
      pix_z[i] = (NV_FLOAT64) z[i];
    }

  free (x);
  free (y);
  free (z);
}



/*!  Convert map coordinates to screen coordinates.  Z is usually ignored.  This version returns
     NV_INT32 pixels.  */

void 
nvMap::map_to_screen (NV_INT32 num_points, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], 
                      NV_FLOAT64 map_z[], NV_INT32 pix_x[], NV_INT32 pix_y[], NV_INT32 pix_z[])
{
  NV_FLOAT64 *dx = (NV_FLOAT64 *) calloc (num_points, sizeof (NV_FLOAT64));
  NV_FLOAT64 *dy = (NV_FLOAT64 *) calloc (num_points, sizeof (NV_FLOAT64));
  NV_FLOAT64 *dz = (NV_FLOAT64 *) calloc (num_points, sizeof (NV_FLOAT64));


  //  Unprojected data or last_map_called == 0 (called redrawMapArea).

  if (!last_map_called || map.projection == NO_PROJECTION)
    {
      //  Check for inverted Y axis

      if (invertY)
        {
          for (NV_INT32 i = 0 ; i < num_points ; i++)
            {
              dx[i] = (NV_FLOAT64) map.draw_width * ((map_x[i] - map.bounds[map.zoom_level].min_x) /
                                                     (map.bounds[map.zoom_level].max_x -
                                                      map.bounds[map.zoom_level].min_x)); 
              dy[i] = (NV_FLOAT64) map.draw_height * ((map_y[i] - map.bounds[map.zoom_level].max_y) /
                                                      (map.bounds[map.zoom_level].min_y -
                                                       map.bounds[map.zoom_level].max_y));
            }
        }
      else
        {
          for (NV_INT32 i = 0 ; i < num_points ; i++)
            {
              dx[i] = (NV_FLOAT64) map.draw_width * ((map_x[i] - map.bounds[map.zoom_level].min_x) /
                                                     (map.bounds[map.zoom_level].max_x - 
                                                      map.bounds[map.zoom_level].min_x));
              dy[i] = (NV_FLOAT64) map.draw_height * ((map.bounds[map.zoom_level].max_y - map_y[i]) /
                                                      (map.bounds[map.zoom_level].max_y -
                                                       map.bounds[map.zoom_level].min_y));
            }
        }
    }
  else
    {
      //  We don't want to modify the input points if we're in the zero to 360 world.

      NV_FLOAT64 *mx = (NV_FLOAT64 *) calloc (num_points, sizeof (NV_FLOAT64));

      for (NV_INT32 i = 0 ; i < num_points ; i++)
        {
          mx[i] = map_x[i];

          if (dateline && mx[i] < 0.0) mx[i] += 360.0;
        }

      proj->project (num_points, RPROJ_DEG, mx, map_y, map_z, dx, dy, dz);

      free (mx);
    }

  for (NV_INT32 i = 0 ; i < num_points ; i++)
    {
      pix_x[i] = NINT (dx[i]);
      pix_y[i] = NINT (dy[i]);
      pix_z[i] = NINT (dz[i]);
    }

  free (dx);
  free (dy);
  free (dz);
}



//!  Convert screen coordinates to map coordinates.  Z is usually ignored.

void 
nvMap::screen_to_map (NV_INT32 num_points, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], NV_FLOAT64 map_z[], NV_INT32 pix_x[], NV_INT32 pix_y[], NV_INT32 pix_z[])
{
  NV_FLOAT64 *dx = (NV_FLOAT64 *) calloc (num_points, sizeof (NV_FLOAT64));
  NV_FLOAT64 *dy = (NV_FLOAT64 *) calloc (num_points, sizeof (NV_FLOAT64));
  NV_FLOAT64 *dz = (NV_FLOAT64 *) calloc (num_points, sizeof (NV_FLOAT64));

  for (NV_INT32 i = 0 ; i < num_points ; i++)
    {
      dx[i] = (NV_FLOAT64) pix_x[i];
      dy[i] = (NV_FLOAT64) pix_y[i];
      dz[i] = (NV_FLOAT64) pix_z[i];
    }


  screen_to_map (num_points, map_x, map_y, map_z, dx, dy, dz);


  free (dx);
  free (dy);
  free (dz);
}



void 
nvMap::screen_to_map (NV_INT32 num_points, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], NV_FLOAT64 map_z[], NV_FLOAT64 pix_x[], NV_FLOAT64 pix_y[], NV_FLOAT64 pix_z[])
{
  //  Unprojected data or last_map_called == 0 (called redrawMapArea).

  if (!last_map_called || map.projection == NO_PROJECTION)
    {
      //  Check for inverted Y axis

      if (invertY)
        {
          for (NV_INT32 i = 0 ; i < num_points ; i++)
            {
              map_x[i] = map.bounds[map.zoom_level].min_x +
                (map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x) *
                (pix_x[i] / (NV_FLOAT64) map.draw_width);
              map_y[i] = map.bounds[map.zoom_level].max_y +
                (map.bounds[map.zoom_level].min_y - map.bounds[map.zoom_level].max_y) *
                (pix_y[i] / (NV_FLOAT64) map.draw_height);
            }
        }
      else
        {
          for (NV_INT32 i = 0 ; i < num_points ; i++)
            {
              map_x[i] = map.bounds[map.zoom_level].min_x +
                (map.bounds[map.zoom_level].max_x - map.bounds[map.zoom_level].min_x) *
                (pix_x[i] / (NV_FLOAT64) map.draw_width);
              map_y[i] = map.bounds[map.zoom_level].min_y +
                (map.bounds[map.zoom_level].max_y - map.bounds[map.zoom_level].min_y) *
                (((NV_FLOAT64) map.draw_height - pix_y[i]) / (NV_FLOAT64) map.draw_height);
            }
        }
    }
  else
    {
      proj->unproject (num_points, RPROJ_DEG, map_x, map_y, map_z, pix_x, pix_y, pix_z);


      //  We want to adjust greater than 180 values if we're crossing the dateline unless the user is expecting
      //  zero to 360 values.

      if (dateline && !zero_to_360)
        {
          for (NV_INT32 i = 0 ; i < num_points ; i++)
            {
              if (map_x[i] > 180.0) map_x[i] -= 360.0;
            }
        }
    }
}



/*!  Make sure a map point (map_x,map_y) is in the currently displayed map area.
     Not to be confused with boundsCheck below which modifies the points based on invertY.  */

NV_BOOL 
nvMap::checkBounds (NV_FLOAT64 map_x, NV_FLOAT64 map_y)
{
  NV_FLOAT64 mx = map_x, my = map_y;


  if (dateline && mx < 0.0) mx += 360.0;


  if (mx >= map.bounds[map.zoom_level].min_x && mx <= map.bounds[map.zoom_level].max_x &&
      my >= map.bounds[map.zoom_level].min_y && my <= map.bounds[map.zoom_level].max_y) return (NVTrue);


  return (NVFalse);
}



//!  Returns an MBR adjusted for dateline crossing (and zero to 360 world view) based on arrays of X and Y values.

void 
nvMap::getMBR (NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_INT32 num_points, NV_F64_XYMBR *mbr)
{
  NV_FLOAT64 *mx = (NV_FLOAT64 *) calloc (num_points, sizeof (NV_FLOAT64));

  mbr->min_x = 999999.0;
  mbr->min_y = 999999.0;
  mbr->max_x = -999999.0;
  mbr->max_y = -999999.0;
  for (NV_INT32 i = 0 ; i < num_points ; i++)
    {
      mx[i] = map_x[i];
      if (dateline && mx[i] < 0.0) mx[i] += 360.0;

      mbr->min_x = qMin (mx[i], mbr->min_x);
      mbr->min_y = qMin (map_y[i], mbr->min_y);
      mbr->max_x = qMax (mx[i], mbr->max_x);
      mbr->max_y = qMax (map_y[i], mbr->max_y);
    }

  free (mx);


  if (dateline && !zero_to_360 && mbr->max_x > 180.0) mbr->max_x -= 360.0;


  //  Check for inverted Y axis.

  if (invertY)
    {
      NV_FLOAT64 temp = mbr->min_y;
      mbr->min_y = mbr->max_y;
      mbr->max_y = temp;
    }
}



//!  Set to true if the user wants zero to 360 world coordinates returned

void 
nvMap::setZeroToThreeSixty (NV_BOOL enable)
{
  zero_to_360 = enable;
}



/*!  Set to true if the user wants to allow wrapping in the moveMap functions.  If the initial
     X map size is 360 degrees then this is set to NVTrue by default otherwise it is NVFalse.  */

void 
nvMap::setNoBoundsLimit (NV_BOOL enable)
{
  no_bounds_limit = enable;
}



/*!  Checks the line segment map_x1,map_y1 to map_x2,map_y2 against the current zoom MBR,
     clips the input line segment to MBR, and then returns modified values.  This function
     will handle dateline issues (unlike the external standalone function).  */

NV_INT32 
nvMap::clipLine (NV_FLOAT64 map_x1, NV_FLOAT64 map_y1, NV_FLOAT64 map_x2, NV_FLOAT64 map_y2, 
                 NV_FLOAT64 *ret_x1, NV_FLOAT64 *ret_y1, NV_FLOAT64 *ret_x2, NV_FLOAT64 *ret_y2)
{
  NV_FLOAT64 x[2], y[2], mod_x1 = map_x1, mod_x2 = map_x2, mod_y1 = map_y1, mod_y2 = map_y2;
  NV_INT32 hits, retval;
  NV_F64_XYMBR mbr = map.bounds[map.zoom_level];


  *ret_x1 = mod_x1;
  *ret_y1 = mod_y1;
  *ret_x2 = mod_x2;
  *ret_y2 = mod_y2;


  //  We only have to adjust the points since the MBR will have already been adjusted.

  if (dateline)
    {
      if (mod_x1 < 0.0) mod_x1 += 360.0;
      if (mod_x2 < 0.0) mod_x2 += 360.0;


      //  Arbitrary cutoff - if we're crossing the dateline and one point is negative and the other is >=0
      //  and either point is more than 90 degrees outside of the boundaries we're going to say that this line
      //  is completely outside of the area.  If you want to do really large areas that cross the datelin you
      //  need to use 0-360 world always.

      if ((map_x1 < 0.0 && map_x2 >= 0.0 && ((mbr.min_x - mod_x2) > 90.0 || (mod_x2 - mbr.max_x) > 90.0 ||
                                             (mbr.min_x - mod_x1) > 90.0 || (mod_x1 - mbr.max_x) > 90.0)) ||
          (map_x2 < 0.0 && map_x1 >= 0.0 && ((mbr.min_x - mod_x1) > 90.0 || (mod_x1 - mbr.max_x) > 90.0 ||
                                             (mbr.min_x - mod_x2) > 90.0 || (mod_x2 - mbr.max_x) > 90.0))) return (0);
    }


  /*  No point in doing the math if the whole line is in the rectangle.  */

  if (mod_x1 >= mbr.min_x && mod_x1 <= mbr.max_x && mod_x2 >= mbr.min_x && mod_x2 <= mbr.max_x &&
      mod_y1 >= mbr.min_y && mod_y1 <= mbr.max_y && mod_y2 >= mbr.min_y && mod_y2 <= mbr.max_y) return (1);


  /*  No point in doing the math if no part of the line can be in the rectangle.  */

  if ((mod_x1 > mbr.max_x && mod_x2 > mbr.max_x) || (mod_x1 < mbr.min_x && mod_x2 < mbr.min_x) ||
      (mod_y1 > mbr.max_y && mod_y2 > mbr.max_y) || (mod_y1 < mbr.min_y && mod_y2 < mbr.min_y)) return (0);


  /*  Since this is a rectangle we can only intersect two lines at most.  */

  hits = 0;

  if (lineIntersection (mod_x1, mod_y1, mod_x2, mod_y2, mbr.min_x, mbr.min_y, mbr.min_x, mbr.max_y,
                        &x[hits], &y[hits]) == 2) hits++;


  if (lineIntersection (mod_x1, mod_y1, mod_x2, mod_y2, mbr.min_x, mbr.max_y, mbr.max_x, mbr.max_y,
                        &x[hits], &y[hits]) == 2) hits++;


  if (hits < 2)
    {
      if (lineIntersection (mod_x1, mod_y1, mod_x2, mod_y2, mbr.max_x, mbr.max_y, mbr.max_x, mbr.min_y,
                            &x[hits], &y[hits]) == 2) hits++;

      if (hits < 2)
        {
          if (lineIntersection (mod_x1, mod_y1, mod_x2, mod_y2, mbr.max_x, mbr.min_y, mbr.min_x, mbr.min_y,
                                &x[hits], &y[hits]) == 2) hits++;
        }
    }


  retval = 0;


  /*  If we only intersected one boundary, figure out which end needs to be clipped.  */

  if (hits == 1)
    {
      /*  Start point is inside.  */

      if (mod_x1 >= mbr.min_x && mod_x1 <= mbr.max_x && mod_y1 >= mbr.min_y && mod_y1 <= mbr.max_y)
        {
          *ret_x2 = x[0];
          *ret_y2 = y[0];

          retval = 3;
        }


      /*  End point is inside.  */

      else
        {
          *ret_x1 = x[0];
          *ret_y1 = y[0];

          retval = 2;
        }
    }


  /*  We intersected two sides.  Both start and end points must be outside the mbr.  */

  if (hits == 2)
    {
      *ret_x1 = x[0];
      *ret_y1 = y[0];
      *ret_x2 = x[1];
      *ret_y2 = y[1];

      retval = 4;
    }


  return (retval);
}



//  *************************************************************************************
//
//  Internal functions
//
//  *************************************************************************************


//!  Make sure a map point (map_x,map_y) is in the currently displayed map area.

void 
nvMap::boundsCheck (NV_FLOAT64 *map_x, NV_FLOAT64 *map_y)
{
  NV_FLOAT64 mx = *map_x;


  //  Check for dateline.

  if (dateline && mx < 0.0) mx += 360.0;

  if (mx < map.bounds[map.zoom_level].min_x) *map_x = map.bounds[map.zoom_level].min_x;
  if (mx > map.bounds[map.zoom_level].max_x) *map_x = map.bounds[map.zoom_level].max_x;

  if (dateline && !zero_to_360 && *map_x > 180.0) *map_x -= 360.0;


  //  Check for inverted Y axis.

  if (invertY)
    {
      if (*map_y < map.bounds[map.zoom_level].max_y) *map_y = map.bounds[map.zoom_level].max_y;
      if (*map_y > map.bounds[map.zoom_level].min_y) *map_y = map.bounds[map.zoom_level].min_y;
    }
  else
    {
      if (*map_y < map.bounds[map.zoom_level].min_y) *map_y = map.bounds[map.zoom_level].min_y;
      if (*map_y > map.bounds[map.zoom_level].max_y) *map_y = map.bounds[map.zoom_level].max_y;
    }
}



//!  Check for the dateline, adjust the bounds if needed, and set the flags.

void 
nvMap::checkDateline (NV_F64_XYMBR *mbr)
{
  dateline = NVFalse;


  //  Make sure that we don't check the dateline for unprojected data.

  if (map.projection == NO_PROJECTION) return;


  if (mbr->max_x > 180.0)
    {
      dateline = NVTrue;
    }
  else
    {
      if (mbr->min_x > mbr->max_x)
        {
          dateline = NVTrue;
          mbr->max_x += 360.0;
        }
    }
}



//!  Entered the map area.

void
nvMap::enterEvent (QEvent *e)
{
  if (mouseSignalsEnabled)
    {
      //  Let the parent do something.

      emit enterSignal (e);
    }
}



//!  Left the map area.

void
nvMap::leaveEvent (QEvent *e)
{
  if (mouseSignalsEnabled)
    {
      //  Let the parent do something.

      emit leaveSignal (e);
    }
}



/*!  Double clicked a mouse button.  Convert screen coordinates to map coordinates and
     emit a signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvMap::mouseDoubleClickEvent (QMouseEvent *e)
{
  if (mouseSignalsEnabled)
    {
      NV_FLOAT64 map_x, map_y;
      NV_INT32 x = e->x ();
      NV_INT32 y = e->y ();

      screen_to_map (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

      boundsCheck (&map_x, &map_y);


      //  Let the parent do something.

      emit mouseDoubleClickSignal (e, map_x, map_y);
    }
}



/*!  Pressed a mouse button.  Convert screen coordinates to map coordinates and
     emit a signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvMap::mousePressEvent (QMouseEvent *e)
{
  if (mouseSignalsEnabled)
    {
      NV_FLOAT64 map_x, map_y;
      NV_INT32 x = e->x ();
      NV_INT32 y = e->y ();

      screen_to_map (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);

      boundsCheck (&map_x, &map_y);


      //  Let the parent do something.

      emit mousePressSignal (e, map_x, map_y);
    }
  else
    {
      emit preliminaryMousePressSignal (e);
    }
}



/*!  Released a mouse button.  Convert screen coordinates to map coordinates and
     emit a signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvMap::mouseReleaseEvent (QMouseEvent *e)
{
  if (mouseSignalsEnabled)
    {
      NV_FLOAT64 map_x, map_y;
      NV_INT32 x = e->x ();
      NV_INT32 y = e->y ();

      screen_to_map (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);


      boundsCheck (&map_x, &map_y);


      //  Let the parent do something.

      emit mouseReleaseSignal (e, map_x, map_y);
    }
}



/*!  Moved mouse.  Convert screen coordinates to map coordinates and emit a 
     signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvMap::mouseMoveEvent (QMouseEvent *e)
{
  if (mouseSignalsEnabled)
    {
      NV_FLOAT64 map_x, map_y;
      NV_INT32 x = e->x ();
      NV_INT32 y = e->y ();


      screen_to_map (1, &map_x, &map_y, &dum_map_z[0], &x, &y, &dum_z[0]);


      boundsCheck (&map_x, &map_y);


      //  Let the parent do something.

      emit mouseMoveSignal (e, map_x, map_y);
    }
}



//!  Popped a wheelie!

void
nvMap::wheelEvent (QWheelEvent *e)
{
  if (mouseSignalsEnabled)
    {
      //  Let the parent do something.

      emit mouseWheelSignal (e);
    }
}



/*!  Emit a repaint to force a paint of the off-screen pixmap to the screen.  Use this if you've drawn some lines
     or painted some rectangles without setting the "flush" argument to NVTrue.  */

void 
nvMap::flush ()
{
  repaint ();
}



/*!  Open a pixmap of an arbitrary size in order to get a complete pixmap (not limited by screen size).  Set "start" to NVTrue
     to begin, draw your stuff, do a grabPixmap, then set "start" to NVFalse.  */

void 
nvMap::dummyBuffer (NV_INT32 width, NV_INT32 height, NV_BOOL start)
{
  static NV_INT32 ow = 0, oh = 0;


  dummy_buffer = start;


  if (start)
    {
      //  Save the old width and height so we can recover when we're done (start == NVFalse).

      ow = size ().width ();
      oh = size ().height ();


      //  Kill the old painter to the off-screen pixmap.

      if (painter.isActive ()) painter.end ();


      //  Resize the off-screen pixmap.

      buffer = QPixmap (width, height);
      buffer.fill (this, 0, 0);

      map.draw_width = width - (map.border * 2);
      map.draw_height = height - (map.border * 2);


      //  Start the painter to the off-screen pixmap.

      painter.begin (&buffer);
    }
  else
    {
      //  Kill the old painter to the off-screen pixmap.

      if (painter.isActive ()) painter.end ();


      //  Resize the off-screen pixmap.

      buffer = QPixmap (ow, oh);
      buffer.fill (this, 0, 0);

      map.draw_width = ow - (map.border * 2);
      map.draw_height = oh - (map.border * 2);


      //  Start the painter to the off-screen pixmap.

      painter.begin (&buffer);
    }
}



//!  Duh, resize event.

void
nvMap::resizeEvent (QResizeEvent *e)
{
  static NV_INT32 ow, oh;

  QWidget::resizeEvent (e);

  NV_INT32 w = e->size ().width ();
  NV_INT32 h = e->size ().height ();


  //  Kill the old painter to the off-screen pixmap.

  if (painter.isActive ()) painter.end ();


  //  Resize the off-screen pixmap.

  buffer = QPixmap (size ());
  buffer.fill (this, 0, 0);

  map.draw_width = w - (map.border * 2);
  map.draw_height = h - (map.border * 2);


  //  Start the painter to the off-screen pixmap.

  painter.begin (&buffer);


  //  Tell someone we resized.

  emit resizeSignal (e);


  ow = w;
  oh = h;
}



//!  Someone may try to close the window using the little X button (I hate those things).

void 
nvMap::closeEvent (QCloseEvent *e)
{
  emit closeSignal (e);
}



/*!  These two focus events are here because the paintevent would sometimes go batshit if there was a movable
     object and you lost or gained focus.  I have no idea why.  They'll emit a signal if you need it for anything.  */

void 
nvMap::focusInEvent (QFocusEvent *e)
{
  emit focusInSignal (e);
}



void 
nvMap::focusOutEvent (QFocusEvent *e)
{
  emit focusOutSignal (e);
}



/*!  This is the equivalent of the X expose event.  It's also triggered by calling update () or repaint ().
     Note that 99.999% of this is dealing with moving objects or rubberbands.  For everything else we
     just dump the pixmap that we've been painting on to the screen.  */

void
nvMap::paintEvent (QPaintEvent *e)
{
  if (dummy_buffer) return;


  QPainter screen_painter (this);


  //  Movable objects (moving or rubberband) are only drawn on the screen, not to the pixmap.

  if (movable_object_count)
    {
      //  Originally I was trying to be elegant and restore just the area under the last drawn moving object but
      //  this caused problems when I did some painting from a timer function.  Again, I have no idea why.  At
      //  any rate, it turns out that it takes almost exactly the same amount of time to repaint the entire
      //  pixmap as it does to repaint just a subsection of it so, in the interest of simplicity, that's what I'm
      //  doing.  I'm going to leave in the part that computes the previous rectangle just in case I figure the
      //  damn thing out some day ;-)

      //  Clear the previous object by restoring the entire pixmap.

      screen_painter.drawPixmap (0, 0, buffer, 0, 0, width (), height ());


      //  This is how it should really be done.  

      //  First make sure we don't try to go outside the window bounds.

      //  movable_object.prev_rect.setLeft (qMax (0, movable_object.prev_rect.left ()));
      //  movable_object.prev_rect.setTop (qMax (0, movable_object.prev_rect.top ()));
      //  movable_object.prev_rect.setRight (qMin ((width () - 1), movable_object.prev_rect.right ()));
      //  movable_object.prev_rect.setBottom (qMin ((height () - 1), movable_object.prev_rect.bottom ()));
      //  screen_painter.drawPixmap (movable_object.prev_rect, buffer, movable_object.prev_rect);

      for (NV_INT32 k = 0 ; k < movable_object_count ; k++)
        {
          NV_INT32 inc = ot[k]->line_width / 2 + 1;
          NV_INT32 inc2 = inc * 2;
          NV_INT32 newx, newy;
          QPen pen;
          QRect r;
          QRectF r2;
          QPolygon *poly;
          QBrush brush;
          QFont font;


          switch (ot[k]->type)
            {
            case NVMAP_MOVE_POLYGON:
              if (ot[k]->filled)
                {
                  brush.setStyle (Qt::SolidPattern);
                  brush.setColor (ot[k]->color);
                  screen_painter.setPen (ot[k]->color);

                  screen_painter.setBrush (brush);
                }
              else
                {
                  pen.setColor (ot[k]->color);
                  pen.setWidth (ot[k]->line_width);
                  pen.setStyle (ot[k]->penStyle);

                  screen_painter.setPen (pen);
                }

              poly = new QPolygon (ot[k]->polygon_count);

              for (NV_INT32 i = 0 ; i < ot[k]->polygon_count ; i++) 
                {
                  poly->setPoint (i, ot[k]->polygon_x[i], ot[k]->polygon_y[i]);
                }

              screen_painter.drawPolygon (*poly);

              r = poly->boundingRect ().normalized ();

              delete poly;

              r.setLeft (r.left () - inc);
              r.setTop (r.top () - inc);
              r.setRight (r.right () + inc);
              r.setBottom (r.bottom () + inc);

              ot[k]->prev_rect = r;
              break;


            case NVMAP_MOVE_POLYLINE:
              pen.setColor (ot[k]->color);
              pen.setWidth (ot[k]->line_width);
              pen.setStyle (ot[k]->penStyle);

              screen_painter.setPen (pen);

              poly = new QPolygon (ot[k]->polygon_count);

              for (NV_INT32 i = 0 ; i < ot[k]->polygon_count ; i++) 
                {
                  poly->setPoint (i, ot[k]->polygon_x[i], ot[k]->polygon_y[i]);
                }

              screen_painter.drawPolyline (*poly);

              r = poly->boundingRect ().normalized ();

              delete poly;

              r.setLeft (r.left () - inc);
              r.setTop (r.top () - inc);
              r.setRight (r.right () + inc);
              r.setBottom (r.bottom () + inc);

              ot[k]->prev_rect = r;
              break;


            case NVMAP_MOVE_RECTANGLE:
              r = ot[k]->rect;

              if (ot[k]->filled)
                {
                  //  Don't outline the rectangle

                  screen_painter.setPen (Qt::NoPen);

                  brush.setStyle (Qt::SolidPattern);
                  brush.setColor (ot[k]->color);
                  screen_painter.setPen (ot[k]->color);

                  screen_painter.fillRect (r.normalized (), brush);
                }
              else
                {
                  pen.setColor (ot[k]->color);
                  pen.setWidth (ot[k]->line_width);
                  pen.setStyle (ot[k]->penStyle);

                  screen_painter.setPen (pen);

                  screen_painter.drawRect (r.normalized ());
                }


              ot[k]->prev_rect.setCoords (r.normalized().x() - inc, r.normalized().y() - inc,
                                          r.normalized().x() + r.normalized().width() + inc2,
                                          r.normalized().y() + r.normalized().height() + inc2);

              break;


            case NVMAP_MOVE_CIRCLE:

              //  Center the circle.

              newx = ot[k]->x - ot[k]->width / 2;
              newy = ot[k]->y - ot[k]->height / 2;


              if (ot[k]->filled)
                {
                  brush.setStyle (Qt::SolidPattern);
                  brush.setColor (ot[k]->color);
                  screen_painter.setPen (ot[k]->color);
                  screen_painter.setBrush (brush);

                  screen_painter.drawPie (newx, newy, ot[k]->height, ot[k]->width, 
                                          ot[k]->start_angle, ot[k]->span_angle);
                }
              else
                {
                  pen.setColor (ot[k]->color);
                  pen.setWidth (ot[k]->line_width);
                  pen.setStyle (ot[k]->penStyle);
                  screen_painter.setPen (pen);

                  screen_painter.drawArc (newx, newy, ot[k]->width, ot[k]->height, 
                                          ot[k]->start_angle, ot[k]->span_angle);
                }

              r.setLeft (newx);
              r.setWidth (ot[k]->width);
              r.setTop (newy);
              r.setHeight (ot[k]->height);

              ot[k]->prev_rect.setCoords (r.normalized().x() - inc, r.normalized().y() - inc,
                                          r.normalized().x() + r.normalized().width() + inc2,
                                          r.normalized().y() + r.normalized().height() + inc2);

              break;


            case NVMAP_MOVE_PATH:

              //  Translate the center of the painter path to the provided x and y.

              r2 = ot[k]->path.boundingRect ();

              screen_painter.save ();
              screen_painter.translate (ot[k]->x - (NV_INT32) (r2.width () / 2.0),
                                        ot[k]->y - (NV_INT32) (r2.height () / 2.0));

              screen_painter.rotate (ot[k]->rotation_angle);


              if (ot[k]->filled)
                {
                  screen_painter.setPen (ot[k]->color);
                  screen_painter.setBrush (ot[k]->brush);

                  screen_painter.drawPath (ot[k]->path);
                }
              else
                {
                  pen.setColor (ot[k]->color);
                  pen.setWidth (ot[k]->line_width);
                  pen.setStyle (ot[k]->penStyle);
                  screen_painter.setPen (pen);

                  screen_painter.drawPath (ot[k]->path);
                }

              r2 = ot[k]->path.boundingRect ();

              ot[k]->prev_rect.setCoords ((NV_INT32) r2.normalized().x() - inc, 
                                          (NV_INT32) r2.normalized().y() - inc,
                                          (NV_INT32) r2.normalized().x() +
                                          (NV_INT32) r2.normalized().width() + inc2,
                                          (NV_INT32) r2.normalized().y() +
                                          (NV_INT32) r2.normalized().height() + inc2);

              screen_painter.restore ();

              break;


            case NVMAP_MOVE_TEXT:

              screen_painter.save ();
              screen_painter.translate (ot[k]->x, ot[k]->y);
              screen_painter.rotate (ot[k]->rotation_angle - 90.0);

              font.setFamily ("Charter");
              font.setPointSize (ot[k]->height);
              font = QFont (font, screen_painter.device ());

              screen_painter.setFont(font);

              r = screen_painter.fontMetrics ().boundingRect (ot[k]->text);

              screen_painter.setPen (ot[k]->color);

              screen_painter.drawText (r, Qt::AlignLeft | Qt::AlignBottom, ot[k]->text, &r2);

              ot[k]->prev_rect.setCoords ((NV_INT32) r2.normalized().x() - inc, 
                                          (NV_INT32) r2.normalized().y() - inc,
                                          (NV_INT32) r2.normalized().x() +
                                          (NV_INT32) r2.normalized().width() + inc2,
                                          (NV_INT32) r2.normalized().y() +
                                          (NV_INT32) r2.normalized().height() + inc2);

              screen_painter.restore ();

              break;


            case NVMAP_RUBBER_POLYGON:
              pen.setColor (ot[k]->color);
              pen.setWidth (ot[k]->line_width);
              pen.setStyle (Qt::SolidLine);

              screen_painter.setPen (pen);

              poly = new QPolygon (ot[k]->polygon_count + 1);

              for (NV_INT32 i = 0 ; i < ot[k]->polygon_count ; i++) 
                {
                  poly->setPoint (i, ot[k]->polygon_x[i], ot[k]->polygon_y[i]);
                }
              poly->setPoint (ot[k]->polygon_count, rubberbandPolyLastX[k], rubberbandPolyLastY[k]);

              if (ot[k]->close_polygon)
                {
                  screen_painter.drawPolygon (*poly);
                }
              else
                {
                  screen_painter.drawPolyline (*poly);
                }

              r = poly->boundingRect ().normalized ();

              delete poly;

              r.setLeft (r.left () - inc);
              r.setTop (r.top () - inc);
              r.setRight (r.right () + inc);
              r.setBottom (r.bottom () + inc);

              ot[k]->prev_rect = r;
              break;


            case NVMAP_RUBBER_RECTANGLE:
              pen.setColor (ot[k]->color);
              pen.setWidth (ot[k]->line_width);
              pen.setStyle (Qt::SolidLine);

              screen_painter.setPen (pen);

              screen_painter.drawRect (ot[k]->rect.normalized ());

              ot[k]->prev_rect.setCoords (ot[k]->rect.normalized().x() - inc,
                                          ot[k]->rect.normalized().y() - inc,
                                          ot[k]->rect.normalized().x() + 
                                          ot[k]->rect.normalized().width() + inc2,
                                          ot[k]->rect.normalized().y() +
                                          ot[k]->rect.normalized().height() + inc2);

              break;


            case NVMAP_RUBBER_LINE:
              pen.setColor (ot[k]->color);
              pen.setWidth (ot[k]->line_width);
              pen.setStyle (Qt::SolidLine);

              screen_painter.setPen (pen);

              screen_painter.drawLine (ot[k]->x0, ot[k]->y0, ot[k]->x1, ot[k]->y1);

              r.setCoords (ot[k]->x0, ot[k]->y0, ot[k]->x1, ot[k]->y1);

              ot[k]->prev_rect.setCoords (r.normalized().x() - inc, r.normalized().y() - inc,
                                          r.normalized().x() + r.normalized().width() + inc2,
                                          r.normalized().y() + r.normalized().height() + inc2);

              break;
            }
        }
    }
  else


    //  Paint some portion (or all) of the off-screen pixmap to the screen.

    {
      screen_painter.drawPixmap (e->rect (), buffer, e->rect ());
    }


  screen_painter.end ();
}



//  Initialize the projection for the current view.  Screen coordinates x, y, width, and height in
//  pixels.  Border in pixels.  Map coordinates min_x, min_y, max_x, max_y.

void 
nvMap::init_proj (NV_INT32 x, NV_INT32 y, NV_INT32 width, NV_INT32 height, NV_INT32 border, NV_FLOAT64 min_x, 
                NV_FLOAT64 min_y, NV_FLOAT64 max_x, NV_FLOAT64 max_y)
{
  NV_FLOAT64       dum1, dum2, dum3;

  if (proj) delete (proj);

  proj = new rproj (map.projection, 0.0, 0.0, (NV_FLOAT64) (x + border), (NV_FLOAT64) ((x + width) - border), 
                    (NV_FLOAT64) ((y + height) - border), (NV_FLOAT64) (y + border), min_y, max_y, min_x, max_x, 
                    &dum1, &dum2, &dum3);
}



/************************************************************************/
/*!

     - Module Name:    lineIntersection

     - Programmer:     Jan C. Depner

     - Date Written:   September 2006

     - Purpose:        Determines if two lines intersect.  If so it
                       returns the intersection point.

     - Inputs:
                       - x1            =  starting x coordinate line 1
                       - y1            =  starting y coordinate line 1
                       - x2            =  ending x coordinate line 1
                       - y2            =  ending y coordinate line 1
                       - x3            =  starting x coordinate line 2
                       - y3            =  starting y coordinate line 2
                       - x4            =  ending x coordinate line 2
                       - y4            =  ending y coordinate line 2
                       - x             =  intersection x coordinate
                       - y             =  intersection y coordinate

     - Outputs:
                       - 0 = no intersection
                       - 1 = lines intersect but the segments don't
                       - 2 = segments intersect

     - Referenced from: http://en.wikipedia.org/wiki/Line_segment_intersection


     - Derived from: http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
                     Intersection point of two lines (2 dimensions)
                     Paul Bourke, April 1989



     - This note describes the technique and algorithm for determining the intersection point of two lines
       (or line segments) in 2 dimensions.

     <pre>

                             P3        P2
                              \        /
                        line b \      /
                                \    /
                                 \  /
                                  \/
                                  /\
                                 /  \
                                /    \
                        line a /      \
                              /        \
                             P1        P4


    The equations of the lines are

    Pa = P1 + ua ( P2 - P1 )

    Pb = P3 + ub ( P4 - P3 )

    Solving for the point where Pa = Pb gives the following two equations in two unknowns (ua and ub) 
    x1 + ua (x2 - x1) = x3 + ub (x4 - x3)

    and
    y1 + ua (y2 - y1) = y3 + ub (y4 - y3)

    Solving gives the following expressions for ua and ub

    Substituting either of these into the corresponding equation for the line gives the intersection point.
    For example the intersection point (x,y) is
    x = x1 + ua (x2 - x1)

    y = y1 + ua (y2 - y1)


    Notes:

    * The denominators for the equations for ua and ub are the same.

    * If the denominator for the equations for ua and ub is 0 then the two lines are parallel.

    * If the denominator and numerator for the equations for ua and ub are 0 then the two lines are coincident.

    * The equations apply to lines, if the intersection of line segments is required then it is only necessary to test
      if ua and ub lie between 0 and 1. Whichever one lies within that range then the corresponding line segment
      contains the intersection point. If both lie within the range of 0 to 1 then the intersection point is within
      both line segments. 

    </pre>

 ************************************************************************/


NV_INT32 
nvMap::lineIntersection (NV_FLOAT64 x1, NV_FLOAT64 y1, NV_FLOAT64 x2, NV_FLOAT64 y2,
                         NV_FLOAT64 x3, NV_FLOAT64 y3, NV_FLOAT64 x4, NV_FLOAT64 y4,
                         NV_FLOAT64 *x, NV_FLOAT64 *y)
{
  NV_FLOAT64 denominator, ua, ub;


  denominator = ((y4 -y3) * (x2 - x1) - (x4 -x3) * (y2 - y1));

  if (denominator == 0.0) return (0);


  ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denominator;
  ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denominator;

  *x = x1 + ua * (x2 - x1);
  *y = y1 + ua * (y2 - y1);

  if (ua >= 0.0 && ua <= 1.0 && ub >= 0.0 && ub <= 1.0) return (2);


  return (1);
}
