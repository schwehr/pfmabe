
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



/*  nvPic class definitions.  */

#ifndef __NVPIC_H__
#define __NVPIC_H__

#include <cmath>


#include "nvtypes.h"
#include "nvdef.h"


#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <QtGui>

#include "fixpos.h"


#define PIC_X_SIZE             800
#define PIC_Y_SIZE             675
#define NVPIC_LEFT             0
#define NVPIC_UP               1
#define NVPIC_RIGHT            2
#define NVPIC_DOWN             3


#define NVPIC_POLYGON_POINTS   1000
#define NVPIC_ZOOM_LEVELS      100


typedef struct
{
  NV_INT32 min_y;
  NV_INT32 min_x;
  NV_INT32 max_y;
  NV_INT32 max_x;
} NV_I32_XYMBR;


typedef struct
{
  NV_I32_XYMBR initial_bounds;             //!<  Initial x,y boundaries
  NV_I32_XYMBR bounds[NVPIC_ZOOM_LEVELS];  //!<  All zoom bounds
  NV_INT32 zoom_level;                     //!<  Zoom level
  NV_FLOAT32 zoom_percent;                 //!<  Zoom in/out percentage (0.0 - 1.0)
  NV_INT32 draw_width;                     //!<  Drawing area width in pixels
  NV_INT32 draw_height;                    //!<  Drawing area height in pixels
  NV_INT32 width;                          //!<  Pic width in pixels
  NV_INT32 height;                         //!<  Pic height in pixels
  Qt::TransformationMode transMode;        //!<  Transformation mode for rotation and scaling (smooth or fast)
} NVPIC_DEF;



//  FLOATING_OBJECT types

#define NVPIC_MOVE_POLYGON       0
#define NVPIC_MOVE_RECTANGLE     1
#define NVPIC_MOVE_CIRCLE        2
#define NVPIC_MOVE_PATH          3
#define NVPIC_RUBBER_POLYGON     4
#define NVPIC_RUBBER_RECTANGLE   5
#define NVPIC_RUBBER_LINE        6

//  FLOATING_OBJECT states

#define NVPIC_OBJECT_NOT_PRESENT 0
#define NVPIC_OBJECT_ACTIVE      1
#define NVPIC_OBJECT_CLEAR       2

typedef struct
{
  NV_INT32 type;                 // See above
  NV_INT32 state;                // See above
  NV_BOOL filled;
  NV_BOOL outlined;
  NV_BOOL close_polygon;
  QBrush brush;
  NV_INT32 polygon_x[NVPIC_POLYGON_POINTS], polygon_y[NVPIC_POLYGON_POINTS], polygon_count;
  NV_INT32 x, y, width, height, start_angle, span_angle;
  QColor color;
  QRect rect, prev_rect;
  NV_INT32 x0, y0, x1, y1;
  NV_INT32 line_width;
  QPainterPath path;
  NV_FLOAT64 rotation_angle;
} NVPIC_FLOATING_OBJECT;


class nvPic:public QWidget
{
  Q_OBJECT 


public:

  nvPic (QWidget *parent = 0, NVPIC_DEF *picdef = 0);
  ~nvPic ();

  void setTransformationMode (Qt::TransformationMode mode);
  void setZoomLevel (NV_INT32 level);
  void setZoomPercent (NV_FLOAT32 percent);
  void zoomInPercent ();
  void zoomOutPercent ();
  void zoomIn (NV_I32_XYMBR bounds);
  void zoomOut ();
  void resetBounds (NV_I32_XYMBR bounds);
  NVPIC_DEF getPicdef ();
  void redrawPic ();

  void brighten (NV_INT32 percentage);
  NV_BOOL OpenFile (NV_CHAR *filename);
  NV_BOOL OpenData (NV_U_CHAR *data, NV_U_INT32 len);
  void SaveFile (QString filename, NV_BOOL scaled);
  void RotatePixmap (NV_FLOAT64 angle);

  void imagePaint (NV_INT32 x, NV_INT32 y, QImage *src, NV_INT32 src_x, NV_INT32 src_y, NV_INT32 src_w, NV_INT32 src_h);
  void pixmapPaint (NV_INT32 x, NV_INT32 y, QPixmap *src, NV_INT32 src_x, NV_INT32 src_y, NV_INT32 src_w, NV_INT32 src_h);
  QPixmap grabPixmap (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h);

  void anchorRubberbandPolygon (NV_INT32 pic_x, NV_INT32 pic_y, QColor color, NV_INT32 line_width, NV_BOOL close);
  void anchorRubberbandPolygon (NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, NV_BOOL close, NV_BYTE q);
  void dragRubberbandPolygon (NV_INT32 pic_x, NV_INT32 pic_y);
  void dragRubberbandPolygon (NV_INT32 x, NV_INT32 y, NV_BYTE q);
  void getRubberbandPolygonCurrentAnchor (NV_INT32 *pic_x, NV_INT32 *pic_y);
  void vertexRubberbandPolygon (NV_INT32 pic_x, NV_INT32 pic_y);
  void vertexRubberbandPolygon (NV_INT32 x, NV_INT32 y, NV_BYTE q);
  void getRubberbandPolygon (NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my);
  void closeRubberbandPolygon (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 *count, 
                               NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my);
  void closeRubberbandPolygon (NV_INT32 x, NV_INT32 y, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, 
                               NV_INT32 **mx, NV_INT32 **my, NV_BYTE q);
  NV_BOOL rubberbandPolygonIsActive ();
  void discardRubberbandPolygon ();

  void anchorRubberbandRectangle (NV_INT32 pic_x, NV_INT32 pic_y, QColor color, NV_INT32 line_width);
  void anchorRubberbandRectangle (NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, NV_BYTE q);
  void dragRubberbandRectangle (NV_INT32 pic_x, NV_INT32 pic_y);
  void dragRubberbandRectangle (NV_INT32 x, NV_INT32 y, NV_BYTE q);
  void getRubberbandRectangle (NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my);
  void closeRubberbandRectangle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 **px, 
                                 NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my);
  void closeRubberbandRectangle (NV_INT32 x, NV_INT32 y, NV_INT32 **px, NV_INT32 **py, 
                                 NV_INT32 **mx, NV_INT32 **my, NV_BYTE q);
  NV_BOOL rubberbandRectangleIsActive ();
  NV_BOOL rubberbandRectangleIsPresent ();
  void discardRubberbandRectangle ();
  void getRubberbandRectangleAnchor (NV_INT32 *pic_x, NV_INT32 *pic_y);

  void anchorRubberbandLine (NV_INT32 pic_x, NV_INT32 pic_y, QColor color, NV_INT32 line_width);
  void anchorRubberbandLine (NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, NV_BYTE q);
  void dragRubberbandLine (NV_INT32 pic_x, NV_INT32 pic_y);
  void dragRubberbandLine (NV_INT32 x, NV_INT32 y, NV_BYTE q);
  NV_BOOL rubberbandLineIsActive ();
  void discardRubberbandLine ();


  void setMovingRectangle (NV_INT32 pic_x0, NV_INT32 pic_y0, NV_INT32 pic_x1, NV_INT32 pic_y1, 
                           QColor color, NV_INT32 line_width, NV_BOOL filled);
  void setMovingRectangle (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width,
                           NV_BOOL filled, NV_BYTE q);
  void getMovingRectangle (NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my);
  void closeMovingRectangle ();
  void setMovingPolygon (NV_INT32 count, NV_INT32 pic_x[], NV_INT32 pic_y[], QColor color, NV_INT32 line_width,
                         NV_BOOL filled);
  void setMovingPolygon (NV_INT32 count, NV_INT32 *x, NV_INT32 *y, QColor color, NV_INT32 line_width, NV_BOOL filled,
                         NV_BYTE q);
  void getMovingPolygon (NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my);
  void closeMovingPolygon ();
  void setMovingCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 d, NV_FLOAT32 start_angle, 
                        NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, NV_BOOL filled);
  void setMovingCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 end_pic_x, NV_INT32 end_pic_y, 
                        NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, NV_BOOL filled);
  void setMovingCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                        NV_INT32 line_width, QColor color, NV_BOOL filled, NV_BYTE q);
  void getMovingCircle (NV_INT32 *pic_x, NV_INT32 *pic_y, NV_INT32 *x, NV_INT32 *y, NV_INT32 *d);
  void closeMovingCircle ();
  void setMovingPath (QPainterPath path, NV_INT32 pic_center_x, NV_INT32 pic_center_y, NV_INT32 line_width,
                      QColor color, QBrush brush, NV_BOOL filled, NV_FLOAT64 angle);
  void setMovingPath (QPainterPath path, NV_INT32 pic_center_x, NV_INT32 pic_center_y, NV_INT32 line_width,
                      QColor color, QBrush brush, NV_BOOL filled);
  void setMovingPath (QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width, QColor color,
                      QBrush brush, NV_BOOL filled, NV_FLOAT64 angle, NV_BYTE q);
  void closeMovingPath ();


  void drawText (QString string, NV_INT32 pic_x, NV_INT32 pic_y, NV_FLOAT64 angle, NV_INT32 size, QColor color);
  void drawText (QString string, NV_INT32 x, NV_INT32 y, NV_FLOAT64 angle, NV_INT32 size, QColor color, NV_BYTE q);
  void drawText (QString string, NV_INT32 pic_x, NV_INT32 pic_y, QColor color);
  void drawText (QString string, NV_INT32 x, NV_INT32 y, QColor color, NV_BYTE q);
  void drawLine (NV_INT32 pic_x0, NV_INT32 pic_y0, NV_INT32 pic_x1, NV_INT32 pic_y1, QColor color,
                 NV_INT32 line_width, NV_BOOL flush);
  void drawLine (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width, NV_BOOL flush,
                 NV_BYTE q);
  void fillCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 d, NV_FLOAT32 start_angle,
                   NV_FLOAT32 span_angle, QColor color);
  void fillCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 end_pic_x, NV_INT32 end_pic_y, 
                   NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, QColor color);
  void fillCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                   QColor color, NV_BYTE q);
  void drawCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 d, NV_FLOAT32 start_angle,
                   NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color);
  void drawCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 end_pic_x, NV_INT32 end_pic_y, 
                   NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color);
  void drawCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                   NV_INT32 line_width, QColor color, NV_BYTE q);
  void fillRectangle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush);
  void fillRectangle (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush, NV_BYTE q);
  void drawRectangle (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width);
  void drawRectangle (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width, NV_BYTE q);
  void fillPolygon (NV_INT32 count, NV_INT32 pic_x[], NV_INT32 pic_y[], QColor color);
  void fillPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color, NV_BYTE q);
  void drawPolygon (NV_INT32 count, NV_INT32 pic_x[], NV_INT32 pic_y[], QColor color, NV_INT32 line_width);
  void drawPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color, NV_INT32 line_width, NV_BYTE q);
  void drawPath (QPainterPath path, NV_INT32 pic_center_x, NV_INT32 pic_center_y, NV_INT32 line_width, QColor color,
                 QBrush brush, NV_BOOL filled);
  void drawPath (QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width, QColor color,
                 QBrush brush, NV_BOOL filled, NV_BYTE q);

  void pic_to_screen (NV_INT32 num_points, NV_INT32 pic_x[], NV_INT32 pic_y[], NV_INT32 pix_x[], NV_INT32 pix_y[]);
  void screen_to_pic (NV_INT32 num_points, NV_INT32 pic_x[], NV_INT32 pic_y[], NV_INT32 pix_x[], NV_INT32 pix_y[]);
  void enableSignals ();
  void disableSignals ();
  void flush ();


  signals:

  void mousePressSignal (QMouseEvent *e, NV_INT32 pic_x, NV_INT32 pic_y);
  void mouseDoubleClickSignal (QMouseEvent *e, NV_INT32 pic_x, NV_INT32 pic_y);
  void preliminaryMousePressSignal (QMouseEvent *e);
  void mouseReleaseSignal (QMouseEvent *e, NV_INT32 pic_x, NV_INT32 pic_y);
  void mouseMoveSignal (QMouseEvent *e, NV_INT32 pic_x, NV_INT32 pic_y);
  void mouseWheelSignal (QWheelEvent *e);
  void closeSignal (QCloseEvent *e);
  void focusInSignal (QFocusEvent *e);
  void focusOutSignal (QFocusEvent *e);
  void resizeSignal (QResizeEvent *e);
  void keyPressSignal (QKeyEvent *e);
  void postRedrawSignal (NVPIC_DEF pic);

protected:

  void boundsCheck (NV_INT32 *pic_x, NV_INT32 *pic_y);
  void mouseDoubleClickEvent (QMouseEvent *e);
  void mousePressEvent (QMouseEvent *e);
  void mouseReleaseEvent (QMouseEvent *e);
  void mouseMoveEvent (QMouseEvent *e);
  void wheelEvent (QWheelEvent *e);
  void keyPressEvent (QKeyEvent *e);
  void resizeEvent (QResizeEvent *e);
  void paintEvent (QPaintEvent *e);
  void closeEvent (QCloseEvent *e);
  void focusInEvent (QFocusEvent *e);
  void focusOutEvent (QFocusEvent *e);


  QMatrix               matrix, trueMatrix, invertedMatrix;

  NV_FLOAT64            rotation;
  NV_BOOL               rotate, box_cleared, pic_loaded;
  NV_FLOAT32            xrange, yrange, scale_min_x_scale, scale_min_y_scale;

  QPixmap               buffer, *orig_pixmap, *scaled_pixmap;
  QPainter              painter;
  NVPIC_FLOATING_OBJECT floating_object;

  NV_BOOL               rubberbandLineActive;
  NV_INT32              rubberbandLineAnchorX, rubberbandLineAnchorY, rubberbandLineLastX, rubberbandLineLastY;


  NV_BOOL               rubberbandPolyActive;
  NV_INT32              rubberbandPolyLastX, rubberbandPolyLastY;
  NV_INT32              rubberbandPolyPicX[NVPIC_POLYGON_POINTS], rubberbandPolyPicY[NVPIC_POLYGON_POINTS];


  NV_BOOL               rubberbandRectActive, rubberbandRectPresent;
  NV_INT32              rubberbandRectCount, rubberbandRectX[4], rubberbandRectY[4], rubberbandRectLastX, 
                        rubberbandRectLastY;
  NV_INT32              rubberbandRectPicX[4], rubberbandRectPicY[4];


  NV_BOOL               signalsEnabled;

  NVPIC_DEF             pic;
};

#endif
