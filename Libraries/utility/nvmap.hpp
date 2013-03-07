
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



/*  nvMap class definitions.  */

#ifndef __NVMAP_H__
#define __NVMAP_H__

#include <cmath>


#include "nvtypes.h"
#include "nvdef.h"
#include "rproj.hpp"


#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <QtGui>


#include "fixpos.h"


#define NVMAP_LEFT             0           //!<  Move map one block to left with overlap
#define NVMAP_UP               1           //!<  Move map one block up with overlap
#define NVMAP_RIGHT            2           //!<  Move map one block to right with overlap
#define NVMAP_DOWN             3           //!<  Move map one block down with overlap
#define NVMAP_UP_LEFT          4           //!<  Move map one block up and left with overlap
#define NVMAP_UP_RIGHT         5           //!<  Move map one block up and right with overlap
#define NVMAP_DOWN_LEFT        6           //!<  Move map one block down and left with overlap
#define NVMAP_DOWN_RIGHT       7           //!<  Move map one block down and right with overlap


#define NO_PROJECTION          -1          //!<  No map projection defined


#define NVMAP_POLYGON_POINTS   2000
#define NVMAP_ZOOM_LEVELS      100


/*  Coastline types  */

#define NVMAP_NO_COAST         0           //!<  Don't draw coastline
#define NVMAP_AUTO_COAST       1           //!<  Automatically determine coastline density based on size
#define NVMAP_NGA_50K_COAST    2           //!<  Use NGA's restricted 1:50,000 coastline
#define NVMAP_WVS_FULL_COAST   3           //!<  Use full resolution WVS coastline
#define NVMAP_WVS_1M_COAST     4           //!<  Use 1:1,000,000 WVS coastline
#define NVMAP_WVS_3M_COAST     5           //!<  Use 1:3,000,000 WVS coastline
#define NVMAP_WVS_12M_COAST    6           //!<  Use 1:12,000,000 WVS coastline
#define NVMAP_WVS_43M_COAST    7           //!<  Use 1:43,000,000 WVS coastline


/*!  nvmap definition structure.  */

typedef struct
{
  NV_F64_XYMBR initial_bounds;             //!<  Initial x,y boundaries
  NV_F64_XYMBR bounds[NVMAP_ZOOM_LEVELS];  //!<  All zoom bounds
  NV_INT32 projection;                     //!<  Projection number
  NV_INT32 zoom_level;                     //!<  Zoom level
  NV_INT32 zoom_percent;                   //!<  Zoom in/out percentage (0 - 100)
  NV_FLOAT64 center_x;                     //!<  Center x value
  NV_FLOAT64 center_y;                     //!<  Center y value
  NV_INT32 border;                         //!<  Border in pixels
  NV_INT32 overlap_percent;                //!<  Move overlap percentage (0 - 100)
  NV_FLOAT64 grid_inc_y;                   //!<  y grid overlay increment in minutes, 0 for no overlay
  NV_FLOAT64 grid_inc_x;                   //!<  x grid overlay increment in minutes, 0 for no overlay
  NV_INT32 grid_thickness;                 //!<  grid line thickness in pixels
  NV_BOOL frame;                           //!<  Set to draw a surrounding frame
  NV_INT32 draw_width;                     //!<  Drawing area width in pixels
  NV_INT32 draw_height;                    //!<  Drawing area height in pixels
  NV_INT32 width;                          //!<  Map width in pixels
  NV_INT32 height;                         //!<  Map height in pixels
  NV_INT32 coasts;                         //!<  Type of coastline to draw (see Coastline types above)
  NV_INT32 coasts_used;                    //!<  Actual coastline last drawn (if coasts is set to NVMAP_AUTO_COAST this won't be NV_AUTO_COAST)
  NV_BOOL coast_thickness;                 //!<  Coast line thickness
  QColor coast_color;                      //!<  Coastline color
  NV_BOOL landmask;                        //!<  Landmask on or off
  QColor landmask_color;                   //!<  Landmask color
  QColor grid_color;                       //!<  Grid color
  QColor background_color;                 //!<  Background color
} NVMAP_DEF;



//  MOVABLE_OBJECT types

#define NVMAP_MOVE_POLYGON       0           //!<  Moveable polygon
#define NVMAP_MOVE_POLYLINE      1           //!<  Moveable polyline
#define NVMAP_MOVE_RECTANGLE     2           //!<  Moveable rectangle
#define NVMAP_MOVE_CIRCLE        3           //!<  Moveable circle
#define NVMAP_MOVE_PATH          4           //!<  Moveable path
#define NVMAP_MOVE_TEXT          5           //!<  Moveable text
#define NVMAP_RUBBER_POLYGON     6           //!<  Rubberband polygon
#define NVMAP_RUBBER_RECTANGLE   7           //!<  Rubberband rectangle
#define NVMAP_RUBBER_LINE        8           //!<  Rubberband line


#define MAX_MOVABLE_OBJECTS     20           //!<  Number of available movable objects


/*!
    Movable object section.  A little explanation - in order to avoid a ton of if statements in the 
    paint event I've implemented an object table (ot).  The object table is just an array of pointers
    to objects in the movable_object array.  This way we can track the number of the object without
    having to check state for each of MAX_MOVABLE_OBJECTS objects in the paint event.  We will have one
    "for" loop in the paint event that will go from 0 to movable_object_count (which equates to
    "movable_object_count" "if" statements).  That is, we're going to have a count of active objects
    and when we delete an object we'll compress the array.  Otherwise we have to do a "for" loop over
    MAX_MOVABLE_OBJECTS (which equates to MAX_MOVABLE_OBJECTS "if" statements) and an "if" per object.
    This would result in 2 * MAX_MOVABLE_OBJECTS "if" statements every time we had a paint event, even
    if we had NO movable_objects.  Confusing, yes, but much more efficient.  We will be accessing
    ot[object_number] (the pointer to the proper movable_object) when we're in the "for" loop or
    elsewhere in the code.  A pointer of NULL in the object table means the object has been discarded
    and is available for re-use.  I'm sure I could have done this using QGraphicsView but, since I don't
    have the entire scene visible (or in memory) at any given time, it would have been less efficient.
    For most of my stuff I'll only be handling a small number of movable objects.
*/

typedef struct
{
  NV_INT32                      type;        //!<  Type of object
  NV_BOOL                       active;      //!<  Whether the object is active
  NV_BOOL                       filled;      //!<  Whether the object is filled
  NV_BOOL                       outlined;    //!<  Whether the object is outlined
  NV_BOOL                       close_polygon; //!<  Whether to automatically close the polygon
  QBrush                        brush;       //!<  Brush to use to fill object
  Qt::PenStyle                  penStyle;    //!<  Pen style to use
  NV_INT32                      polygon_x[NVMAP_POLYGON_POINTS]; //!<  The X polygon points
  NV_INT32                      polygon_y[NVMAP_POLYGON_POINTS]; //!<  The Y polygon points
  NV_INT32                      polygon_count; //!<  The number of points in a polygon
  NV_INT32                      x;           //!<  X position of object in screen coordinates
  NV_INT32                      y;           //!<  Y position of object in screen coordinates
  NV_INT32                      width;       //!<  Width of object in screen coordinates
  NV_INT32                      height;      //!<  Height of object in screen coordinates
  NV_INT32                      start_angle; //!<  Start angle of object
  NV_INT32                      span_angle;  //!<  Span angle of object
  QColor                        color;       //!<  Color of object
  QRect                         rect;        //!<  Definition of rectangle
  QRect                         prev_rect;   //!<  Definition of previous rectangle (if it moved)
  NV_INT32                      x0;          //!<  Starting X position of a line segment
  NV_INT32                      y0;          //!<  Starting Y position of a line segment
  NV_INT32                      x1;          //!<  Ending X position of a line segment
  NV_INT32                      y1;          //!<  Ending Y position of a line segment
  NV_INT32                      line_width;  //!<  Line width in pixels
  QPainterPath                  path;        //!<  QPainterPath object
  NV_FLOAT64                    rotation_angle; //!< Rotation angle (usually for text)
  QString                       text;        //!<  Text
  NV_BOOL                       clear;       //!<  Clear before drawing (usually for text)
} NVMAP_MOVABLE_OBJECT;


class nvMap:public QWidget
{
  Q_OBJECT 


public:

  nvMap (QWidget *parent = 0, NVMAP_DEF *mapdef = 0);
  ~nvMap ();


  //  Map functions
 
  NV_INT32 getZoomLevel ();
  void setZoomLevel (NV_INT32 level);
  void setZoomPercent (NV_INT32 percent);
  void setOverlapPercent (NV_INT32 percent);
  void stopDrawing ();
  void zoomInPercent ();
  void zoomInPercent (NV_BOOL setLevel);
  void zoomOutPercent ();
  void zoomIn (NV_F64_XYMBR bounds);
  void zoomIn (NV_F64_XYMBR bounds, NV_BOOL setLevel);
  void zoomOut ();
  void setCoastColor (QColor color);
  void setMaskColor (QColor color);
  void setProjection (NV_INT32 projection);
  void setGridColor (QColor color);
  void setBackgroundColor (QColor color);
  void setGridSpacing (NV_FLOAT64 grid_inc_x, NV_FLOAT64 grid_inc_y);
  void resetBounds (NV_F64_XYMBC bounds);
  void resetBounds (NV_F64_XYMBR bounds);
  void setCoasts (NV_INT32 set);
  void setLandmask (NV_BOOL set);
  void setCoastThickness (NV_INT32 thickness);
  NVMAP_DEF getMapdef ();
  void moveMap (NV_INT32 direction);
  void moveMap (NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void redrawMapArea (NV_BOOL clear);
  void redrawMap (NV_BOOL clear);



  //  Rubberband functions

  void anchorRubberbandPolygon (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, QColor color, NV_INT32 line_width, NV_BOOL close,
				Qt::PenStyle penStyle);
  void anchorRubberbandPolygon (NV_INT32 *num, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, NV_BOOL close,
				Qt::PenStyle penStyle);
  void dragRubberbandPolygon (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void dragRubberbandPolygon (NV_INT32 num, NV_INT32 x, NV_INT32 y);
  void getRubberbandPolygonCurrentAnchor (NV_INT32 num, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y);
  void vertexRubberbandPolygon (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void vertexRubberbandPolygon (NV_INT32 num, NV_INT32 x, NV_INT32 y);
  void getRubberbandPolygon (NV_INT32 num, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx,
                             NV_FLOAT64 **my);
  void closeRubberbandPolygon (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 *count, 
                               NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my);
  void closeRubberbandPolygon (NV_INT32 num, NV_INT32 x, NV_INT32 y, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, 
                               NV_FLOAT64 **mx, NV_FLOAT64 **my);
  NV_BOOL rubberbandPolygonIsActive (NV_INT32 num);
  void discardRubberbandPolygon (NV_INT32 *num);

  void anchorRubberbandRectangle (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, QColor color, NV_INT32 line_width,
				  Qt::PenStyle penStyle);
  void anchorRubberbandRectangle (NV_INT32 *num, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, Qt::PenStyle penStyle);
  void dragRubberbandRectangle (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void dragRubberbandRectangle (NV_INT32 num, NV_INT32 x, NV_INT32 y);
  void getRubberbandRectangle (NV_INT32 num, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my);
  void closeRubberbandRectangle (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 **px, 
                                 NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my);
  void closeRubberbandRectangle (NV_INT32 num, NV_INT32 x, NV_INT32 y, NV_INT32 **px, NV_INT32 **py, 
                                 NV_FLOAT64 **mx, NV_FLOAT64 **my);
  NV_BOOL rubberbandRectangleIsActive (NV_INT32 num);
  void discardRubberbandRectangle (NV_INT32 *num);
  void getRubberbandRectangleAnchor (NV_INT32 num, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y);

  void anchorRubberbandLine (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, QColor color, NV_INT32 line_width, 
			     Qt::PenStyle penStyle);
  void anchorRubberbandLine (NV_INT32 *num, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, Qt::PenStyle penStyle);
  void dragRubberbandLine (NV_INT32 num, NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void dragRubberbandLine (NV_INT32 num, NV_INT32 x, NV_INT32 y);
  NV_BOOL rubberbandLineIsActive (NV_INT32 num);
  void discardRubberbandLine (NV_INT32 *num);


  //  Moving object functions

  void setMovingRectangle (NV_INT32 *num, NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_FLOAT64 map_x1, NV_FLOAT64 map_y1, 
                           QColor color, NV_INT32 line_width, NV_BOOL filled, Qt::PenStyle penStyle);
  void setMovingRectangle (NV_INT32 *num, NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color,
                           NV_INT32 line_width, NV_BOOL filled, Qt::PenStyle penStyle);
  void getMovingRectangle (NV_INT32 num, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my);
  void closeMovingRectangle (NV_INT32 *num);
  void setMovingPolygon (NV_INT32 *num, NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color,
                         NV_INT32 line_width, NV_BOOL filled, Qt::PenStyle penStyle);
  void setMovingPolygon (NV_INT32 *num, NV_INT32 count, NV_INT32 *x, NV_INT32 *y, QColor color, NV_INT32 line_width,
                         NV_BOOL filled, Qt::PenStyle penStyle);
  void getMovingPolygon (NV_INT32 num, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my);
  void closeMovingPolygon (NV_INT32 *num);
  void setMovingPolyline (NV_INT32 *num, NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color,
                          NV_INT32 line_width, Qt::PenStyle penStyle);
  void setMovingPolyline (NV_INT32 *num, NV_INT32 count, NV_INT32 *x, NV_INT32 *y, QColor color, NV_INT32 line_width,
                          Qt::PenStyle penStyle);
  void getMovingPolyline (NV_INT32 num, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my);
  void closeMovingPolyline (NV_INT32 *num);
  void setMovingCircle (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 d, NV_FLOAT32 start_angle, 
                        NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, NV_BOOL filled, Qt::PenStyle penStyle);
  void setMovingCircle (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, 
                        NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, NV_BOOL filled,
                        Qt::PenStyle penStyle);
  void setMovingCircle (NV_INT32 *num, NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                        NV_INT32 line_width, QColor color, NV_BOOL filled, Qt::PenStyle penStyle);
  void getMovingCircle (NV_INT32 num, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_INT32 *x, NV_INT32 *y, NV_INT32 *d);
  void closeMovingCircle (NV_INT32 *num);
  void setMovingPath (NV_INT32 *num, QPainterPath path, NV_FLOAT64 map_center_x, NV_FLOAT64 map_center_y,
                      NV_INT32 line_width, QColor color, QBrush brush, NV_BOOL filled, NV_FLOAT64 angle,
                      Qt::PenStyle penStyle);
  void setMovingPath (NV_INT32 *num, QPainterPath path, NV_FLOAT64 map_center_x, NV_FLOAT64 map_center_y,
                      NV_INT32 line_width, QColor color, QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle);
  void setMovingPath (NV_INT32 *num, QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width,
                      QColor color, QBrush brush, NV_BOOL filled, NV_FLOAT64 angle, Qt::PenStyle penStyle);
  void setMovingPath (NV_INT32 *num, QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width,
                      QColor color, QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle);
  void closeMovingPath (NV_INT32 *num);
  void setMovingText (NV_INT32 *num, QString text, NV_FLOAT64 map_center_x, NV_FLOAT64 map_center_y, QColor color, NV_FLOAT64 angle,
                      NV_INT32 size);
  void setMovingText (NV_INT32 *num, QString text, NV_INT32 center_x, NV_INT32 center_y, QColor color, NV_FLOAT64 angle, NV_INT32 size);
  void closeMovingText (NV_INT32 *num);


  //  Drawing functions

  void drawText (QString string, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 angle, NV_INT32 size, QColor color,
                 NV_BOOL flush);
  void drawTextClear (QString string, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 angle, NV_INT32 size, QColor color,
                      NV_BOOL clear, NV_BOOL flush);
  void drawText (QString string, NV_INT32 x, NV_INT32 y, NV_FLOAT64 angle, NV_INT32 size, QColor color, NV_BOOL flush);
  void drawTextClear (QString string, NV_INT32 x, NV_INT32 y, NV_FLOAT64 angle, NV_INT32 size, QColor color, NV_BOOL clear,
                      NV_BOOL flush);
  void drawText (QString string, NV_FLOAT64 map_x, NV_FLOAT64 map_y, QColor color, NV_BOOL flush);
  void drawText (QString string, NV_INT32 x, NV_INT32 y, QColor color, NV_BOOL flush);
  void drawLine (NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_FLOAT64 map_x1, NV_FLOAT64 map_y1, QColor color,
                 NV_INT32 line_width, NV_BOOL flush, Qt::PenStyle penStyle);
  void drawLine (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width, NV_BOOL flush,
                 Qt::PenStyle penStyle);
  void fillCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 d, NV_FLOAT32 start_angle,
                   NV_FLOAT32 span_angle, QColor color, NV_BOOL flush);
  void fillCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, 
                   NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, QColor color, NV_BOOL flush);
  void fillCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                   QColor color, NV_BOOL flush);
  void drawCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 d, NV_FLOAT32 start_angle,
                   NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, Qt::PenStyle penStyle, NV_BOOL flush);
  void drawCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, 
                   NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color,
                   Qt::PenStyle penStyle, NV_BOOL flush);
  void drawCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                   NV_INT32 line_width, QColor color, Qt::PenStyle penStyle, NV_BOOL flush);
  void fillRectangle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush);
  void fillRectangle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, QColor color,
                      NV_BOOL flush);
  void fillRectangle (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush);
  void drawRectangle (NV_FLOAT64 x0, NV_FLOAT64 y0, NV_FLOAT64 x1, NV_FLOAT64 y1, QColor color, NV_INT32 line_width,
                      Qt::PenStyle penStyle, NV_BOOL flush);
  void drawRectangle (NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_INT32 w, NV_INT32 h,
                      QColor color, NV_INT32 line_width, Qt::PenStyle penStyle, NV_BOOL flush);
  void drawRectangle (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width,
                      Qt::PenStyle penStyle, NV_BOOL flush);
  void fillPolygon (NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color, NV_BOOL flush);
  void fillPolygon (NV_INT32 count, NV_F64_COORD2 map_xy[], QColor color, NV_BOOL flush);
  void fillPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color, NV_BOOL flush);
  void fillPolygon (NV_INT32 count, NV_I32_COORD2 xy[], QColor color, NV_BOOL flush);
  void drawPolygon (NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color, NV_INT32 line_width,
                    NV_BOOL close, Qt::PenStyle penStyle, NV_BOOL flush);
  void drawPolygon (NV_INT32 count, NV_F64_COORD2 map_xy[], QColor color, NV_INT32 line_width,
                    NV_BOOL close, Qt::PenStyle penStyle, NV_BOOL flush);
  void drawPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color, NV_INT32 line_width, NV_BOOL close, 
                    Qt::PenStyle penStyle, NV_BOOL flush);
  void drawPolygon (NV_INT32 count, NV_I32_COORD2 xy[], QColor color, NV_INT32 line_width, NV_BOOL close, 
                    Qt::PenStyle penStyle, NV_BOOL flush);
  void drawPath (QPainterPath path, NV_FLOAT64 map_center_x, NV_FLOAT64 map_center_y, NV_INT32 line_width, QColor color,
                 QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle, NV_BOOL flush);
  void drawPath (QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width, QColor color,
                 QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle, NV_BOOL flush);
  void drawPixmap (NV_INT32 x, NV_INT32 y, QPixmap *src, NV_INT32 src_x, NV_INT32 src_y, 
                   NV_INT32 src_w, NV_INT32 src_h, NV_BOOL flush);
  QPixmap grabPixmap (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h);
  QPixmap grabPixmap ();


  //  Utility functions

  void map_to_screen (NV_INT32 num_points, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], 
                      NV_FLOAT64 map_z[], NV_FLOAT64 pix_x[], NV_FLOAT64 pix_y[], NV_FLOAT64 pix_z[]);
  void map_to_screen (NV_INT32 num_points, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], 
                      NV_FLOAT64 map_z[], NV_INT32 pix_x[], NV_INT32 pix_y[], NV_INT32 pix_z[]);
  void screen_to_map (NV_INT32 num_points, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], NV_FLOAT64 map_z[],
                      NV_INT32 pix_x[], NV_INT32 pix_y[], NV_INT32 pix_z[]);
  void screen_to_map (NV_INT32 num_points, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], NV_FLOAT64 map_z[],
                      NV_FLOAT64 pix_x[], NV_FLOAT64 pix_y[], NV_FLOAT64 pix_z[]);
  NV_BOOL checkBounds (NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void getMBR (NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_INT32 num_points, NV_F64_XYMBR *mbr);
  void setZeroToThreeSixty (NV_BOOL enable);
  void setNoBoundsLimit (NV_BOOL enable);
  NV_INT32 clipLine (NV_FLOAT64 map_x1, NV_FLOAT64 map_y1, NV_FLOAT64 map_x2, NV_FLOAT64 map_y2, 
                     NV_FLOAT64 *ret_x1, NV_FLOAT64 *ret_y1, NV_FLOAT64 *ret_x2, NV_FLOAT64 *ret_y2);
  void enableSignals ();
  void disableSignals ();
  void enableMouseSignals ();
  void disableMouseSignals ();
  void flush ();
  void dummyBuffer (NV_INT32 width, NV_INT32 height, NV_BOOL start);


signals:

  void enterSignal (QEvent *e);
  void leaveSignal (QEvent *e);
  void mousePressSignal (QMouseEvent *e, NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void mouseDoubleClickSignal (QMouseEvent *e, NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void preliminaryMousePressSignal (QMouseEvent *e);
  void mouseReleaseSignal (QMouseEvent *e, NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void mouseMoveSignal (QMouseEvent *e, NV_FLOAT64 map_x, NV_FLOAT64 map_y);
  void mouseWheelSignal (QWheelEvent *e);
  void closeSignal (QCloseEvent *e);
  void focusInSignal (QFocusEvent *e);
  void focusOutSignal (QFocusEvent *e);
  void resizeSignal (QResizeEvent *e);
  void preRedrawSignal (NVMAP_DEF map);
  void midRedrawSignal (NVMAP_DEF map);
  void postRedrawSignal (NVMAP_DEF map);

protected:

  void enterEvent (QEvent *e);
  void leaveEvent (QEvent *e);
  void boundsCheck (NV_FLOAT64 *map_x, NV_FLOAT64 *map_y);
  void checkDateline (NV_F64_XYMBR *mbr);
  void mouseDoubleClickEvent (QMouseEvent *e);
  void mousePressEvent (QMouseEvent *e);
  void mouseReleaseEvent (QMouseEvent *e);
  void mouseMoveEvent (QMouseEvent *e);
  void wheelEvent (QWheelEvent *e);
  void resizeEvent (QResizeEvent *e);
  void paintEvent (QPaintEvent *e);
  void closeEvent (QCloseEvent *e);
  void focusInEvent (QFocusEvent *e);
  void focusOutEvent (QFocusEvent *e);
  void redrawMapWithCheck (NV_BOOL clear);

  void init_proj (NV_INT32 x, NV_INT32 y, NV_INT32 width, NV_INT32 height, NV_INT32 border, 
                  NV_FLOAT64 xmin, NV_FLOAT64 ymin, NV_FLOAT64 xmax, NV_FLOAT64 ymax);

  NV_INT32 lineIntersection (NV_FLOAT64 x1, NV_FLOAT64 y1, NV_FLOAT64 x2, NV_FLOAT64 y2,
                             NV_FLOAT64 x3, NV_FLOAT64 y3, NV_FLOAT64 x4, NV_FLOAT64 y4,
                             NV_FLOAT64 *x, NV_FLOAT64 *y);

  NV_INT32 create_movable_object (NVMAP_MOVABLE_OBJECT mo);
  void discard_movable_object (NV_INT32 *num);


  rproj                 *proj;

  NV_INT16              last_map_called;

  NV_BOOL               invertY;

  NV_BOOL               stopFlag;

  QPixmap               buffer;

  QPainter              painter;

  NVMAP_MOVABLE_OBJECT  movable_object[MAX_MOVABLE_OBJECTS];
  NVMAP_MOVABLE_OBJECT  *ot[MAX_MOVABLE_OBJECTS];
  NV_INT32              movable_object_count;

  NV_BOOL               rubberbandLineActive[MAX_MOVABLE_OBJECTS];
  NV_INT32              rubberbandLineAnchorX[MAX_MOVABLE_OBJECTS], rubberbandLineAnchorY[MAX_MOVABLE_OBJECTS],
                        rubberbandLineLastX[MAX_MOVABLE_OBJECTS], rubberbandLineLastY[MAX_MOVABLE_OBJECTS];


  NV_BOOL               rubberbandPolyActive[MAX_MOVABLE_OBJECTS];
  NV_INT32              rubberbandPolyLastX[MAX_MOVABLE_OBJECTS], rubberbandPolyLastY[MAX_MOVABLE_OBJECTS];
  NV_FLOAT64            rubberbandPolyMapX[MAX_MOVABLE_OBJECTS][NVMAP_POLYGON_POINTS], 
                        rubberbandPolyMapY[MAX_MOVABLE_OBJECTS][NVMAP_POLYGON_POINTS];


  NV_BOOL               rubberbandRectActive[MAX_MOVABLE_OBJECTS];
  NV_INT32              rubberbandRectX[MAX_MOVABLE_OBJECTS][4], rubberbandRectY[MAX_MOVABLE_OBJECTS][4],
                        rubberbandRectLastX[MAX_MOVABLE_OBJECTS], rubberbandRectLastY[MAX_MOVABLE_OBJECTS];
  NV_FLOAT64            rubberbandRectMapX[MAX_MOVABLE_OBJECTS][4], rubberbandRectMapY[MAX_MOVABLE_OBJECTS][4];


  NV_BOOL               signalsEnabled, mouseSignalsEnabled, dateline, zero_to_360, no_bounds_limit, dummy_buffer;
  NV_INT32              dum_z[NVMAP_POLYGON_POINTS];
  NV_FLOAT64            dum_map_z[NVMAP_POLYGON_POINTS];

  NVMAP_DEF             map;
};

#endif
