
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



/*  nvMapGL class definitions.  */

#ifndef __NVMAPGL_H__
#define __NVMAPGL_H__

#include <cmath>


#include "nvutility.h"
#include "nvutility.hpp"


#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <QtGui>
#include <QGLWidget>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "squat.hpp"


#define NVMAPGL_LEFT             0
#define NVMAPGL_UP               1
#define NVMAPGL_RIGHT            2
#define NVMAPGL_DOWN             3
#define NVMAPGL_UP_LEFT          4
#define NVMAPGL_UP_RIGHT         5
#define NVMAPGL_DOWN_LEFT        6
#define NVMAPGL_DOWN_RIGHT       7

#define NVMAPGL_POLYGON_POINTS   2000
#define NVMAPGL_ZOOM_LEVELS      100


#define DATA_COLORS 128


typedef struct
{
  NV_INT32 projected;                      //!<  0 - not projected (lat/lon), 1 - UTM (meters)
  NV_F64_XYMBC initial_bounds;             //!<  Initial x,y,z boundaries
  NV_FLOAT32 initial_zx_rotation;          //!<  Initial ZX rotation setting
  NV_FLOAT32 initial_y_rotation;           //!<  Initial Y rotation setting
  NV_INT32 zoom_percent;                   //!<  Zoom in/out percentage (0 - 100)
  NV_FLOAT32 exaggeration;                 //!<  Z exaggeration value (which is inverted to get the exag_scale)
  NV_BOOL auto_scale;                      //!<  Set if we are auto scaling (usually not set for NVMAPGL_BIN_MODE)
  NV_FLOAT32 center_x;                     //!<  Center x value
  NV_BOOL draw_scale;                      //!<  Set to draw a scale
  NV_INT32 draw_width;                     //!<  Drawing area width in pixels
  NV_INT32 draw_height;                    //!<  Drawing area height in pixels
  NV_INT32 width;                          //!<  Map width in pixels
  NV_INT32 height;                         //!<  Map height in pixels
  NV_BOOL coasts;                          //!<  Coastlines on or off
  NV_BOOL coast_thickness;                 //!<  Coast line thickness
  QColor coast_color;                      //!<  Coastline color
  QColor scale_color;
  NV_BOOL landmask;                        //!<  Landmask on or off
  QColor landmask_color;                   //!<  Landmask color
  QColor background_color;                 //!<  Background color
  NV_INT32 mode;                           /*!<  0 - layer mode, 1 - point mode, 2 - feature mode.  These define
                                                 what information will be handed back via signal.  Either
                                                 the nearest bin vertex, data point, or feature respectively.  */
  NV_FLOAT32 zoom_factor;                  //!<  Amount of current zoom
  NV_FLOAT32 min_z_extents;                //!<  Minimum Z extents for Z auto scaling
  GLenum light_model;                      //!<  Either GL_LIGHT_MODEL_AMBIENT or GL_LIGHT_MODEL_DIFFUSE;
  NV_INT32 renderMode;                     //!<  Either NVMAPGL_RENDER_POINT_MODE or NVMAPGL_RENDER_SPHERE_MODE)
  NV_FLOAT32 complexObjectWidth;           //!<  Width of the object to be used for the point/sparse data
  NV_INT32 complexDivisionals;
} NVMAPGL_DEF;



//  MOVABLE_OBJECT types

#define NVMAPGL_MOVE_POLYGON       0
#define NVMAPGL_MOVE_POLYLINE      1
#define NVMAPGL_MOVE_RECTANGLE     2
#define NVMAPGL_MOVE_CIRCLE        3
#define NVMAPGL_MOVE_LIST          4
#define NVMAPGL_RUBBER_POLYGON     5
#define NVMAPGL_RUBBER_RECTANGLE   6
#define NVMAPGL_RUBBER_LINE        7


//  Modes.  Used for finding the nearest point and to determine whether to display the full point cloud
//  or the decimated point cloud.

#define NVMAPGL_BIN_MODE           0   //!<  For layers (PFM bins)
#define NVMAPGL_POINT_MODE         1   //!<  For full point cloud.
#define NVMAPGL_SPARSE_MODE        2   //!<  For decimated point cloud
#define NVMAPGL_FEATURE_MODE       3   //!<  For features


//  Number of available movable objects

#define MAX_GL_MOVABLE_OBJECTS     20


//  Modes for rendering point cloud

#define NVMAPGL_RENDER_POINT_MODE  0
#define NVMAPGL_RENDER_SPHERE_MODE 2


/*!
    Movable object section.  A little explanation - in order to avoid a ton of if statements in the 
    paint event I've implemented a movable object table (mot).  The object table is just an array of pointers
    to objects in the movable_object array.  This way we can track the number of the object without
    having to check state for each of MAX_GL_MOVABLE_OBJECTS objects in the paint event.  We will have one
    "for" loop in the paint event that will go from 0 to movable_object_count (which equates to
    "movable_object_count" "if" statements).  That is, we're going to have a count of active objects
    and when we delete an object we'll compress the array.  Otherwise we have to do a "for" loop over
    MAX_GL_MOVABLE_OBJECTS (which equates to MAX_GL_MOVABLE_OBJECTS "if" statements) and an "if" per object.
    This would result in 2 * MAX_GL_MOVABLE_OBJECTS "if" statements every time we had a paint event, even
    if we had NO movable_objects.  Confusing, yes, but much more efficient.  We will be accessing
    mot[object_number] (the pointer to the proper movable_object) when we're in the "for" loop or
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
  GLushort                      stipple;
  NV_FLOAT64                    polygon[NVMAPGL_POLYGON_POINTS][3];  //!<  Not used currently.
  NV_INT32                      polygon_x[NVMAPGL_POLYGON_POINTS], polygon_y[NVMAPGL_POLYGON_POINTS], polygon_count;
  NV_INT32                      prev_polygon_x[NVMAPGL_POLYGON_POINTS], prev_polygon_y[NVMAPGL_POLYGON_POINTS], prev_polygon_count;
  NV_INT32                      width;       //!<  Width of object in screen coordinates
  NV_INT32                      height;      //!<  Height of object in screen coordinates
  NV_INT32                      start_angle; //!<  Start angle of object
  NV_INT32                      span_angle;  //!<  Span angle of object
  GLubyte                       color[4];    //!<  Color of object
  QRect                         rect;        //!<  Definition of rectangle
  QRect                         prev_rect;   //!<  Definition of previous rectangle (if it moved)
  NV_INT32                      x0;          //!<  Starting X position of a line segment
  NV_INT32                      y0;          //!<  Starting Y position of a line segment
  NV_INT32                      x1;          //!<  Ending X position of a line segment
  NV_INT32                      y1;          //!<  Ending Y position of a line segment
  NV_INT32                      prev_x0, prev_y0, prev_x1, prev_y1;
  NV_INT32                      line_width;  //!<  Line width in pixels
  NV_FLOAT32                    rotation_angle;
  NV_FLOAT32                    prev_rotation_angle;
  QString                       string;
} NVMAPGL_MOVABLE_OBJECT;


//  Number of available text objects

#define MAX_GL_TEXT_OBJECTS     8000


typedef struct
{
  QString                       string;
  NV_FLOAT32                    x, y, z;
  NV_FLOAT32                    color[4];
  NV_FLOAT32                    angle;
  QFont                         font;
  NV_INT32                      size;
} NVMAPGL_TEXT_OBJECT;


typedef struct
{
  NV_FLOAT32                    v[3];      //!<  X, Y, Z
  NV_U_BYTE                     color[4];  //!<  Color
  NV_FLOAT32                    size;      //!<  Size
  NV_INT32                      slices;
  NV_INT32                      stacks;
} NVMAPGL_POINT_OBJECT;


typedef struct
{
  NV_FLOAT32                    v[3];      //!<  X, Y, Z
  NV_U_BYTE                     color[4];  //!<  Color
  NV_FLOAT32                    size;      //!<  Size
} NVMAPGL_MARKER_OBJECT;


typedef struct
{
  NV_F32_COORD3                 *v;
  NV_U_BYTE                     color[4];  //!<  Color
  NV_INT32                      width;      //!<  Size
  NV_INT32                      count;
  GLushort                      stipple;
} NVMAPGL_LINE_OBJECT;


typedef struct
{
  NV_INT32                      x0;
  NV_INT32                      x1;
  NV_INT32                      y0;
  NV_INT32                      y1;
  NV_U_BYTE                     color[4];  //!<  Color
  NV_INT32                      width;
  GLushort                      stipple;
} NVMAPGL_2DLINE_OBJECT;


class nvMapGL:public QGLWidget
{
  Q_OBJECT 


public:

  nvMapGL (QWidget *parent = 0, NVMAPGL_DEF *mapdef = 0, QString pn = NULL);
  ~nvMapGL ();


  void clearData (NV_BOOL clear_texture);


  //  Map functions

  void get2DCoords (NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z, NV_INT32 *px, NV_INT32 *py);
  void get2DCoords (NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT32 z, NV_INT32 *px, NV_INT32 *py);
  void setExaggeration (NV_FLOAT32 value);
  void setMinZExtents (NV_FLOAT32 value);
  void setZoomPercent (NV_INT32 percent);
  //void stopDrawing ();
  void zoomInPercent (NV_INT32 percent);
  void zoomInPercent ();
  void zoomOutPercent (NV_INT32 percent);
  void zoomOutPercent ();
  void resetZoom ();
  void resetPOV ();
  void setBackgroundColor (QColor color);
  void setScaleColor (QColor color);
  void enableScale (NV_BOOL enable);
  //void resetBounds (NV_F64_XYMBR bounds);
  void setTextureType (NV_INT32 type);
  void enableTexture (NV_BOOL enable);
  void setAutoScale (NV_BOOL auto_scale);
  void setBounds (NV_F64_XYMBC total_mbc);
  void setProjected (NV_INT32 projected);

  void clearGeotiffTexture ();
  void setGeotiffTexture (QImage *subImage, NV_F64_XYMBR mbr, NV_INT32 type);

  void clearDataLayer (NV_INT32 layer);
  NV_BOOL setData2Layer (NV_INT32 layer, NV_FLOAT32 **data, NV_FLOAT32 **color, NV_FLOAT32 color_min, NV_FLOAT32 color_max, 
                         NV_INT32 rows, NV_INT32 cols, NV_FLOAT64 y_grid, NV_FLOAT64 x_grid, NV_F64_XYMBR mbr, NV_INT32 layer_type);
  NV_BOOL setDataLayer (NV_INT32 layer, NV_FLOAT32 **data, NV_FLOAT32 **color, NV_FLOAT32 color_min, NV_FLOAT32 color_max, 
                        NV_INT32 rows, NV_INT32 cols, NV_FLOAT64 x_grid, NV_FLOAT64 y_grid, NV_FLOAT32 null_val, NV_F64_XYMBR mbr,
                        NV_INT32 layer_type);

  void clearDataPoints ();
  NV_BOOL setDataPoints (NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z, NV_U_BYTE *point_color, NV_INT32 point_size, NV_BOOL done);

  void clearSparsePoints ();
  NV_BOOL setSparsePoints (NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z, NV_U_BYTE *point_color, NV_INT32 point_size, NV_BOOL done);

  void clearFeaturePoints ();
  NV_BOOL setFeaturePoints (NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z, QColor feature_color, NV_FLOAT32 feature_size, NV_INT32 slices,
                            NV_INT32 stacks, NV_BOOL done);

  void clearMarkerPoints ();
  NV_BOOL setMarkerPoints (NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z, QColor marker_color, NV_FLOAT32 marker_size, NV_BOOL done);

  void clearLines ();
  NV_BOOL setLines (NV_FLOAT64 x, NV_FLOAT64 y, NV_FLOAT64 z, QColor line_color, NV_INT32 line_width, Qt::PenStyle penStyle, NV_INT32 point_num,
                    NV_BOOL done);
  NV_BOOL setLines (NV_FLOAT32 x, NV_FLOAT32 y, NV_FLOAT32 z, QColor line_color, NV_INT32 line_width, Qt::PenStyle penStyle, NV_INT32 point_num, NV_BOOL done);

  void setRenderMode (NV_INT32 mode); 
  void setRenderModeAmbience (NV_INT32 mode);
  void setComplexObjectWidth (NV_FLOAT32 objWidth);
  void setComplexDivisionals (NV_INT32 divisionals);

  void setMapMode (NV_INT32 mode);
  void enableWindozePaintEvent (NV_BOOL enable);

  void rotateX (NV_FLOAT32 angle);
  void rotateY (NV_FLOAT32 angle);
  void rotateZ (NV_FLOAT32 angle);
  void rotateZX (NV_FLOAT32 angle);
  void setXRotation (NV_FLOAT32 angle);
  void setYRotation (NV_FLOAT32 angle);
  void setZRotation (NV_FLOAT32 angle);
  void setZXRotation (NV_FLOAT32 angle);
  NV_FLOAT32 getXRotation ();
  NV_FLOAT32 getYRotation ();
  NV_FLOAT32 getZRotation ();
  NV_FLOAT32 getZXRotation ();
  void setMapCenter (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT32 map_z);
  void setMapCenter (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z);
  void setCenter (NV_FLOAT32 x, NV_FLOAT32 y, NV_FLOAT32 z);
  void getMapCenter (NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_FLOAT64 *map_z);



  //  Rubberband functions

  void anchorRubberbandPolygon (NV_INT32 *num, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, NV_BOOL close,
                                  Qt::PenStyle penStyle);
  void dragRubberbandPolygon (NV_INT32 num, NV_INT32 x, NV_INT32 y);
  void getRubberbandPolygonCurrentAnchor (NV_INT32 num, NV_INT32 *x, NV_INT32 *y);
  void vertexRubberbandPolygon (NV_INT32 num, NV_INT32 x, NV_INT32 y);
  void getRubberbandPolygon (NV_INT32 num, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py);
  void closeRubberbandPolygon (NV_INT32 num, NV_INT32 x, NV_INT32 y, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py);
  NV_BOOL rubberbandPolygonIsActive (NV_INT32 num);
  void discardRubberbandPolygon (NV_INT32 *num);

  void anchorRubberbandRectangle (NV_INT32 *num, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, Qt::PenStyle penStyle);
  void dragRubberbandRectangle (NV_INT32 num, NV_INT32 x, NV_INT32 y);
  void getRubberbandRectangle (NV_INT32 num, NV_INT32 **px, NV_INT32 **py);
  void closeRubberbandRectangle (NV_INT32 num, NV_INT32 x, NV_INT32 y, NV_INT32 **px, NV_INT32 **py);
  NV_BOOL rubberbandRectangleIsActive (NV_INT32 num);
  void discardRubberbandRectangle (NV_INT32 *num);
  void getRubberbandRectangleAnchor (NV_INT32 num, NV_INT32 *px, NV_INT32 *py);

  void anchorRubberbandLine (NV_INT32 *num, NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, Qt::PenStyle penStyle);
  void dragRubberbandLine (NV_INT32 num, NV_INT32 x, NV_INT32 y);
  NV_BOOL rubberbandLineIsActive (NV_INT32 num);
  void discardRubberbandLine (NV_INT32 *num);


  //  Moving object functions

  //void setMovingRectangle (NV_INT32 *num, NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_FLOAT64 map_x1, NV_FLOAT64 map_y1, 
  //                       QColor color, NV_INT32 line_width, NV_BOOL filled, Qt::PenStyle penStyle);
  //void setMovingRectangle (NV_INT32 *num, NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color,
  //                       NV_INT32 line_width, NV_BOOL filled, Qt::PenStyle penStyle);
  //void getMovingRectangle (NV_INT32 num, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my);
  //void closeMovingRectangle (NV_INT32 *num);
  //void setMovingPolygon (NV_INT32 *num, NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color,
  //                     NV_INT32 line_width, NV_BOOL filled, Qt::PenStyle penStyle);
  //void setMovingPolygon (NV_INT32 *num, NV_INT32 count, NV_INT32 *x, NV_INT32 *y, QColor color, NV_INT32 line_width,
  //                       NV_BOOL filled, Qt::PenStyle penStyle);
  //void getMovingPolygon (NV_INT32 num, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my);
  //void closeMovingPolygon (NV_INT32 *num);
  //void setMovingPolyline (NV_INT32 *num, NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color,
  //                        NV_INT32 line_width, Qt::PenStyle penStyle);
  //void setMovingPolyline (NV_INT32 *num, NV_INT32 count, NV_INT32 *x, NV_INT32 *y, QColor color, NV_INT32 line_width,
  //                        Qt::PenStyle penStyle);
  //void getMovingPolyline (NV_INT32 num, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_FLOAT64 **mx, NV_FLOAT64 **my);
  //void closeMovingPolyline (NV_INT32 *num);
  //void setMovingCircle (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 d, NV_FLOAT32 start_angle, 
  //                      NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, NV_BOOL filled, Qt::PenStyle penStyle);
  //void setMovingCircle (NV_INT32 *num, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, 
  //                      NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, NV_BOOL filled,
  //                      Qt::PenStyle penStyle);
  //void setMovingCircle (NV_INT32 *num, NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
  //                      NV_INT32 line_width, QColor color, NV_BOOL filled, Qt::PenStyle penStyle);
  //void getMovingCircle (NV_INT32 num, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_INT32 *x, NV_INT32 *y, NV_INT32 *d);
  //void closeMovingCircle (NV_INT32 *num);
  void setMovingList (NV_INT32 *num, NV_I32_COORD2 *list_data, NV_INT32 line_count, NV_INT32 x, NV_INT32 y, NV_FLOAT64 angle,
                      NV_INT32 line_width, NV_C_RGBA color);
  void setMovingList (NV_INT32 *num, NV_I32_COORD2 *list_data, NV_INT32 line_count, NV_INT32 x, NV_INT32 y, NV_FLOAT64 angle,
                      NV_INT32 line_width, QColor color);
  void closeMovingList (NV_INT32 *num);


  //  Drawing functions

  void clearDisplayLists ();
  void drawSphere (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z, NV_FLOAT64 radius, NV_INT32 slices,
                   NV_INT32 stacks, QColor color, NV_BOOL flush);
  void clearText ();
  void drawText (QString string, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z, NV_FLOAT32 angle, NV_INT32 size,
                 QColor color, NV_BOOL flush);
  void drawText (QString string, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z, NV_FLOAT32 angle, QString font,
                 NV_INT32 size, QColor color, NV_BOOL flush);
  void drawLine (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width, Qt::PenStyle penStyle, NV_BOOL flush);
  //void drawLine (NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_FLOAT64 map_x1, NV_FLOAT64 map_y1, QColor color,
  //               NV_INT32 line_width, NV_BOOL flush, Qt::PenStyle penStyle);
  //void drawLine (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width, NV_BOOL flush,
  //               Qt::PenStyle penStyle);
  //void fillCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 d, NV_FLOAT32 start_angle,
  //                 NV_FLOAT32 span_angle, QColor color);
  //void fillCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, 
  //                 NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, QColor color);
  //void fillCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
  //                 QColor color);
  //void drawCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 d, NV_FLOAT32 start_angle,
  //                 NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, Qt::PenStyle penStyle);
  //void drawCircle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, 
  //                 NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color,
  //                 Qt::PenStyle penStyle);
  //void drawCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
  //                 NV_INT32 line_width, QColor color, Qt::PenStyle penStyle);
  //void fillRectangle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush);
  //void fillRectangle (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 end_map_x, NV_FLOAT64 end_map_y, QColor color,
  //                    NV_BOOL flush);
  //void fillRectangle (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush);
  //void drawRectangle (NV_FLOAT64 x0, NV_FLOAT64 y0, NV_FLOAT64 x1, NV_FLOAT64 y1, QColor color, NV_INT32 line_width,
  //                    Qt::PenStyle penStyle);
  //void drawRectangle (NV_FLOAT64 map_x0, NV_FLOAT64 map_y0, NV_INT32 w, NV_INT32 h,
  //                    QColor color, NV_INT32 line_width, Qt::PenStyle penStyle);
  //void drawRectangle (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width,
  //                    Qt::PenStyle penStyle);
  //void fillPolygon (NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color);
  //void fillPolygon (NV_INT32 count, NV_F64_COORD2 map_xy[], QColor color);
  //void fillPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color);
  //void fillPolygon (NV_INT32 count, NV_I32_COORD2 xy[], QColor color);
  //void drawPolygon (NV_INT32 count, NV_FLOAT64 map_x[], NV_FLOAT64 map_y[], QColor color, NV_INT32 line_width,
  //                  NV_BOOL close, Qt::PenStyle penStyle);
  //void drawPolygon (NV_INT32 count, NV_F64_COORD2 map_xy[], QColor color, NV_INT32 line_width,
  //                  NV_BOOL close, Qt::PenStyle penStyle);
  //void drawPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color, NV_INT32 line_width, NV_BOOL close, 
  //                  Qt::PenStyle penStyle);
  //void drawPolygon (NV_INT32 count, NV_I32_COORD2 xy[], QColor color, NV_INT32 line_width, NV_BOOL close, 
  //                  Qt::PenStyle penStyle);
  //void drawPath (QPainterPath path, NV_FLOAT64 map_center_x, NV_FLOAT64 map_center_y, NV_INT32 line_width, QColor color,
  //               QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle);
  //void drawPath (QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width, QColor color,
  //               QBrush brush, NV_BOOL filled, Qt::PenStyle penStyle);
  //void drawPixmap (NV_INT32 x, NV_INT32 y, QPixmap *src, NV_INT32 src_x, NV_INT32 src_y, NV_INT32 src_w, NV_INT32 src_h, NV_BOOL flush);
  //QPixmap grabPixmap (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h);
  //QPixmap grabPixmap ();


  //  Utility functions

  void get3DCoords (NV_INT32 x, NV_INT32 y, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_FLOAT64 *map_z);
  void getMarker3DCoords (NV_INT32 x, NV_INT32 y, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_FLOAT64 *map_z);
  void getFaux3DCoords (NV_FLOAT64 anchor_x, NV_FLOAT64 anchor_y, NV_FLOAT64 anchor_z, NV_INT32 pixel_x, NV_INT32 pixel_y, NV_F64_COORD3 *coords);
  void map_to_screen (NV_INT32 count, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_FLOAT64 *map_z, NV_FLOAT32 *vertex_x, NV_FLOAT32 *vertex_y, NV_FLOAT32 *vertex_z);
  void map_to_screen (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z, NV_FLOAT32 *vertex_x, NV_FLOAT32 *vertex_y,
                      NV_FLOAT32 *vertex_z);
  void map_to_screen (NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT32 map_z, NV_FLOAT32 *vertex_x, NV_FLOAT32 *vertex_y,
                      NV_FLOAT32 *vertex_z);
  void z_to_y_triangle2 (NV_FLOAT32 **data, NV_INT32 layer, NV_INT32 row, NV_INT32 col);
  void z_to_y_triangle (NV_FLOAT32 **data, NV_INT32 layer, NV_INT32 row, NV_INT32 col);
  void screen_to_map (NV_INT32 count, NV_FLOAT32 *vertex_x, NV_FLOAT32 *vertex_y, NV_FLOAT32 *vertex_z, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y, NV_FLOAT64 *map_z);
  void screen_to_map (NV_FLOAT32 vertex_x, NV_FLOAT32 vertex_y, NV_FLOAT32 vertex_z, NV_FLOAT64 *map_x, NV_FLOAT64 *map_y,
                      NV_FLOAT64 *map_z);
  void displayLayer (NV_INT32 layer, NV_BOOL display);
  void resetMap ();
  void enableSignals ();
  void disableSignals ();
  void force_full_res ();
  void flush ();
  void setDepthTest (NV_BOOL on);
  NVMAPGL_DEF getMapdef ();


signals:

  void enterSignal (QEvent *e);
  void leaveSignal (QEvent *e);
  void mousePressSignal (QMouseEvent *e, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z);
  void mouseDoubleClickSignal (QMouseEvent *e, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z);
  void preliminaryMousePressSignal (QMouseEvent *e);
  void mouseReleaseSignal (QMouseEvent *e, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z);
  void mouseMoveSignal (QMouseEvent *e, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z, NVMAPGL_DEF map);
  void wheelSignal (QWheelEvent *e, NV_FLOAT64 map_x, NV_FLOAT64 map_y, NV_FLOAT64 map_z);
  void closeSignal (QCloseEvent *e);
  void focusInSignal (QFocusEvent *e);
  void focusOutSignal (QFocusEvent *e);
  void resizeSignal (QResizeEvent *e);
  void exaggerationChanged (NV_FLOAT32 exaggeration, NV_FLOAT32 apparent_exaggeration);


protected:

  void drawScale ();
  void enterEvent (QEvent *e);
  void leaveEvent (QEvent *e);
  void boundsCheck (NV_FLOAT64 *map_x, NV_FLOAT64 *map_y);
  void mouseDoubleClickEvent (QMouseEvent *e);
  void mousePressEvent (QMouseEvent *e);
  void mouseReleaseEvent (QMouseEvent *e);
  void mouseMoveEvent (QMouseEvent *e);
  void wheelEvent (QWheelEvent *e);
  void closeEvent (QCloseEvent *e);
  void focusInEvent (QFocusEvent *e);
  void focusOutEvent (QFocusEvent *e);
  NV_BOOL getNearestLayerVertex (NV_FLOAT32 nwx, NV_FLOAT32 nwy, NV_FLOAT32 nwz, NV_FLOAT32 fwx, NV_FLOAT32 fwy, NV_FLOAT32 fwz,
                                 GLuint *data_list, GLfloat ****data_vertices, NV_INT32 *data_rows,
                                 NV_INT32 *data_cols, NV_INT32 *pfm, NV_INT32 *row, NV_INT32 *col);
  NV_BOOL getNearestPoint (NV_FLOAT32 nwx, NV_FLOAT32 nwy, NV_FLOAT32 nwz, NV_FLOAT32 fwx, NV_FLOAT32 fwy, NV_FLOAT32 fwz,
                           NVMAPGL_POINT_OBJECT *data, NV_INT32 count, NV_INT32 *index);
  NV_BOOL getNearestPoint (NV_FLOAT32 nwx, NV_FLOAT32 nwy, NV_FLOAT32 nwz, NV_FLOAT32 fwx, NV_FLOAT32 fwy, NV_FLOAT32 fwz,
                           GLfloat *data, NV_INT32 count, NV_INT32 *index);
  NV_BOOL getNearestPoint (NV_FLOAT32 nwx, NV_FLOAT32 nwy, NV_FLOAT32 nwz, NV_FLOAT32 fwx, NV_FLOAT32 fwy, NV_FLOAT32 fwz,
                           GLfloat *data, GLubyte *color, NV_INT32 count, NV_INT32 *index);
  NV_BOOL getNearestPoint (NV_FLOAT32 nwx, NV_FLOAT32 nwy, NV_FLOAT32 nwz, NV_FLOAT32 fwx, NV_FLOAT32 fwy, NV_FLOAT32 fwz,
                           NVMAPGL_MARKER_OBJECT *data, NV_INT32 count, NV_INT32 *index);

  void normalizedcross (GLfloat* u, GLfloat* v, GLfloat* n);
  void initializeGL();
  void resizeGL(int w, int h);
  void paintEvent (QPaintEvent *e);
  void paintGL ();

  void generateDataPointsDL (NV_INT32 count);
  void generateSparsePointsDL (NV_INT32 count);

  NV_INT32 create_movable_object (NVMAPGL_MOVABLE_OBJECT mo);
  void discard_movable_object (NV_INT32 *num);
  void get3DCoords (NV_INT32 x, NV_INT32 y, NV_F32_COORD3 *coords);
  GLushort penStyle_to_stipple (Qt::PenStyle penStyle);


  QString                 parentName;
  NVMAPGL_MOVABLE_OBJECT  movable_object[MAX_GL_MOVABLE_OBJECTS];
  NVMAPGL_MOVABLE_OBJECT  *mot[MAX_GL_MOVABLE_OBJECTS];
  NV_INT32                movable_object_count;
  NVMAPGL_TEXT_OBJECT     text_object[MAX_GL_TEXT_OBJECTS];
  NV_U_INT32              text_object_count;
  NVMAPGL_DEF             map;


  NV_BOOL                 glReady;

  GLfloat                 ****data_normals;
  GLfloat                 ****data_vertices;
  GLuint                  data_list[MAX_ABE_PFMS];              //  Display list for PFM layer
  NV_INT32                data_cols[MAX_ABE_PFMS];              //  Number of columns in PFM layer
  NV_INT32                data_rows[MAX_ABE_PFMS];              //  Number of rows in PFM layer
  NV_BOOL                 data_display[MAX_ABE_PFMS];           //  NVTrue to display layer
  NV_FLOAT64              data_null_val[MAX_ABE_PFMS];          //  Null value for PFM layer

  GLfloat                 ****data2_normals;
  GLfloat                 ****data2_vertices;
  GLuint                  data2_list[MAX_ABE_PFMS];             //  Display list for PFM layer
  NV_INT32                data2_cols[MAX_ABE_PFMS];             //  Number of columns in PFM layer
  NV_INT32                data2_rows[MAX_ABE_PFMS];             //  Number of rows in PFM layer
  NV_INT32                data2_div[MAX_ABE_PFMS];              //  Whether the display needs a subsampled layer

  GLuint                  point_list;
  GLuint                  sparse_list;
  GLuint                  scale_list;
  GLuint                  *display_list;                        //  Array of display lists for primitives
  NV_U_INT32              list_count;                           //  Number of display lists used
  GLuint                  tiffTex;                              //  Texture made from GeoTIFF
  QImage                  *texImage;                            //  Texture image
  GLfloat                 *point_data;                          //  Pointer to the point data structure
  NV_INT32                point_count;                          //  Number of points loaded in the point data structure
  NV_FLOAT32              point_data_size;
  GLubyte                 *point_color_data;
  GLfloat                 *sparse_data;                         //  Pointer to the sparse data point structure
  NV_INT32                sparse_count;                         //  Number of points loaded in the sparse point data structure
  NV_FLOAT32              sparse_data_size;
  GLubyte                 *sparse_color_data;
  NVMAPGL_POINT_OBJECT    *feature_data;                        //  Pointer to the feature data structure
  NV_INT32                feature_count;                        //  Number of features loaded
  NVMAPGL_MARKER_OBJECT   *marker_data;                         //  Pointer to the marker data structure
  NV_INT32                marker_count;                         //  Number of markers loaded
  NVMAPGL_LINE_OBJECT     *line_data;                           //  Pointer to the line data structure
  NV_INT32                line_count;                           //  Number of lines loaded
  NV_FLOAT64              z_scale;                              //  Scale value used to get Z to relate to X and Y as 1 to 1
  NV_FLOAT32              aspect_ratio;                         //  Aspect ratio of current view
  NV_FLOAT32              near_plane;                           //  Distance to near Z plane of frustum
  NV_FLOAT32              far_plane;                            //  Distance to far Z plane of frustum
  NV_FLOAT32              exag_scale;                           //  Exaggeration scale (1.0 / exaggeration)
  NV_FLOAT32              fovy;                                 //  gluPerspective field of view value
  NV_BOOL                 texture;                              //  True if displaying a GeoTIFF texture overlay
  NV_INT32                texture_type;                         //  -1 for GL_MODULATE, 1 for GL_DECAL
  NV_BOOL                 depth_test;                           //  Whether we're going to mask for depth (used for slicing);
                                                                //  This is only modifiable with setDepthTest.
  NV_FLOAT32              apparent_exaggeration;                //  When Z range exceeds Y and/or X we need to play with the apparent exaggeration
  GLfloat                 data_colors[DATA_COLORS + 3][4];
  GLubyte                 scale_color[4];                       //  Scale color
  NV_F64_COORD3           scale_tic[20];                        //  Coordinates for the scale numbers and lines
  NV_INT32                scale_tic_count;                      //  Number of tics (will always be less than 20
  NV_FLOAT32              x_rotation, y_rotation, z_rotation, zx_rotation;
  NV_FLOAT32              eyeX, eyeY, eyeZ;
  NV_FLOAT32              centerX, centerY, centerZ, prev_centerX, prev_centerY, prev_centerZ;
  NV_FLOAT64              map_center_x, map_center_y, map_center_z;
  NV_F32_COORD3           cursor_pos;
  NV_BOOL                 signalsEnabled;
  GLint                   viewport[4];
  GLdouble                mvmatrix[16], projmatrix[16];
  GLfloat                 x_matrix[16], y_matrix[16], z_matrix[16], zx_matrix[16];
  NV_F64_XYMBC            bounds;
  NV_FLOAT64              range_x, range_y, range_z;
  NV_FLOAT64              tex_ratio_x, tex_ratio_y;
  NV_F64_XYMBR            geotiff_mbr;
  NV_INT32                geotiff_width, geotiff_height;
  SQuat                   *rotation;
  NV_INT32                update_type;                          /*  This will be 0 for XOR update (when we are drawing 
                                                                    movable objects), 1 for normal update, 2 to erase
                                                                    and redraw both front and back buffers, or -1 to
                                                                    just erase the previous, XORed movable object).  
                                                                    Add 256 to plot decimated surface.  */
  NV_BOOL                 windoze_paint_event_enabled;


  NV_BOOL                 rubberbandLineActive[MAX_GL_MOVABLE_OBJECTS];
  NV_INT32                rubberbandLineAnchorX[MAX_GL_MOVABLE_OBJECTS], rubberbandLineAnchorY[MAX_GL_MOVABLE_OBJECTS],
                          rubberbandLineLastX[MAX_GL_MOVABLE_OBJECTS], rubberbandLineLastY[MAX_GL_MOVABLE_OBJECTS];


  NV_BOOL                 rubberbandPolyActive[MAX_GL_MOVABLE_OBJECTS];
  NV_INT32                rubberbandPolyLastX[MAX_GL_MOVABLE_OBJECTS], rubberbandPolyLastY[MAX_GL_MOVABLE_OBJECTS];
  NV_FLOAT64              rubberbandPolyMapX[MAX_GL_MOVABLE_OBJECTS][NVMAP_POLYGON_POINTS], 
                          rubberbandPolyMapY[MAX_GL_MOVABLE_OBJECTS][NVMAP_POLYGON_POINTS];


  NV_BOOL                 rubberbandRectActive[MAX_GL_MOVABLE_OBJECTS];
  NV_INT32                rubberbandRectX[MAX_GL_MOVABLE_OBJECTS][4], rubberbandRectY[MAX_GL_MOVABLE_OBJECTS][4],
                          rubberbandRectLastX[MAX_GL_MOVABLE_OBJECTS], rubberbandRectLastY[MAX_GL_MOVABLE_OBJECTS];
  NV_FLOAT64              rubberbandRectMapX[MAX_GL_MOVABLE_OBJECTS][4], rubberbandRectMapY[MAX_GL_MOVABLE_OBJECTS][4];


protected slots:


private:

};

#endif
