
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



/*  nvPic class.  */

/***************************************************************************/
/*!

   - Module :        nvPic

   - Programmer :    Jan C. Depner

   - Date :          03/08/07

   - Purpose :       This is a generic pic class for use in Qt graphical user
                     interfaces.  I have tried to allow the user to work in pic
                     space without having to worry about screen space.  You can
                     still work in screen space though if you need to.  The screen
                     space calls all have a useless, one-byte variable appended to
                     the end so we can overload them since pic and screen space are
                     both defined by integer coordinate systems.


   - Note :          Don't forget that colors can be transparent in Qt4.  You can
                     set the transparency by setting the alpha value to a number
                     between 0 and 255 with 0 being invisible and 255 being opaque.
                     For example - QColor (255, 255, 0, 127) will be a transparent
                     yellow.

\***************************************************************************/
    
#include "nvpic.hpp"

nvPic::nvPic (QWidget * parent, NVPIC_DEF *init):
  QWidget (parent, (Qt::WindowFlags) Qt::WA_StaticContents)
{
  //  Track mouse movement with no button pressed.

  setMouseTracking (TRUE);
  setAttribute (Qt::WA_OpaquePaintEvent);
  setBackgroundRole (QPalette::Base);
  setAutoFillBackground (TRUE);
  setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy (Qt::WheelFocus);


  pic = *init;
  pic_loaded = NVFalse;
  orig_pixmap = new QPixmap ();
  scaled_pixmap = new QPixmap ();


  if (!pic.draw_width) pic.draw_width = PIC_X_SIZE;
  if (!pic.draw_height) pic.draw_height = PIC_Y_SIZE;
  if (pic.zoom_percent < 0.0 || pic.zoom_percent > 1.0) pic.zoom_percent = .1;
  if (pic.transMode < 0 || pic.transMode > 1) pic.transMode = Qt::FastTransformation;


  pic.bounds[0] = pic.initial_bounds;
  pic.zoom_level = 0;


  signalsEnabled = NVFalse;


  rubberbandPolyActive = NVFalse;
  rubberbandRectActive = NVFalse;
  rubberbandRectPresent = NVFalse;

  floating_object.polygon_count = 0;
  floating_object.state = NVPIC_OBJECT_NOT_PRESENT;


#ifndef QT_NO_CURSOR
  setCursor (Qt::WaitCursor);
#endif
}



nvPic::~nvPic ()
{
  if (pic_loaded)
    {
      delete orig_pixmap;
      delete scaled_pixmap;
    }
}



/*!  Enable all signals emitted by this class.  Signals are disabled by default.  You have
     to enable them to get the class to emit them.  This is handy if you are instantiating
     a pic and you don't want it emit resize or redraw signals too soon.  */

void
nvPic::enableSignals ()
{
  signalsEnabled = NVTrue;
}



//!  Disable all signals emitted by this class.

void
nvPic::disableSignals ()
{
  signalsEnabled = NVFalse;
}



/*!  Set the transformation mode (smooth or fast) for rotation and scaling.  This is only for viewing.
     If you save a scaled/rotated image it will be smoothly transformed first.  */

void 
nvPic::setTransformationMode (Qt::TransformationMode mode)
{
  pic.transMode = mode;
}



//!  Set the zoom level.  This allows you to go directly to a previous zoom level.

void
nvPic::setZoomLevel (NV_INT32 level)
{
  pic.zoom_level = level;

  redrawPic ();
}



//!  Set the zoom percentage for percent zoom in or out.

void
nvPic::setZoomPercent (NV_FLOAT32 percent)
{
  pic.zoom_percent = percent;
}



/*!  Zoom in to the specified bounds (see nvtypes.h for the NV_I32_XYMBR structure definition).
     This increments the zoom level.  */

void
nvPic::zoomIn (NV_I32_XYMBR bounds)
{
  if (pic.zoom_level < NVPIC_ZOOM_LEVELS)
    {
      pic.zoom_level++;

      pic.bounds[pic.zoom_level].min_y = bounds.min_y;
      pic.bounds[pic.zoom_level].max_y = bounds.max_y;

      pic.bounds[pic.zoom_level].min_x = bounds.min_x;
      pic.bounds[pic.zoom_level].max_x = bounds.max_x;

      redrawPic ();
    }
}



//!  Zoom out one zoom level.  This decrements the zoom level.

void
nvPic::zoomOut ()
{
  if (pic.zoom_level)
    {
      pic.zoom_level--;

      redrawPic ();
    }
}



//!  Zoom in by the pic.zoom_percent.  This increments the zoom level.

void
nvPic::zoomInPercent ()
{
  if (pic.zoom_level < NVPIC_ZOOM_LEVELS)
    {
      NV_INT32 xsize = pic.bounds[pic.zoom_level].max_x - pic.bounds[pic.zoom_level].min_x;
      NV_INT32 ysize = pic.bounds[pic.zoom_level].max_y - pic.bounds[pic.zoom_level].min_y;


      pic.bounds[pic.zoom_level + 1].min_x = pic.bounds[pic.zoom_level].min_x + NINT (((NV_FLOAT32) xsize * 
                                                                                       pic.zoom_percent) / 2.0);
      pic.bounds[pic.zoom_level + 1].max_x = pic.bounds[pic.zoom_level].max_x - NINT (((NV_FLOAT32) xsize *
                                                                                       pic.zoom_percent) / 2.0);


      pic.bounds[pic.zoom_level + 1].min_y = pic.bounds[pic.zoom_level].min_y + NINT (((NV_FLOAT32) ysize *
                                                                                       pic.zoom_percent) / 2.0);
      pic.bounds[pic.zoom_level + 1].max_y = pic.bounds[pic.zoom_level].max_y - NINT (((NV_FLOAT32) ysize *
                                                                                       pic.zoom_percent) / 2.0);


      //  Increment the zoom level.

      pic.zoom_level++;


      redrawPic ();
    }
}



//!  Zoom out by pic.zoom_percent.  This decrements the zoom level.

void
nvPic::zoomOutPercent ()
{
  if (pic.zoom_level == 1)
    {
      pic.zoom_level = 0;
      return;
    }


  if (pic.zoom_level)
    {
      NV_INT32 xsize = pic.bounds[pic.zoom_level].max_x - pic.bounds[pic.zoom_level].min_x;
      NV_INT32 ysize = pic.bounds[pic.zoom_level].max_y - pic.bounds[pic.zoom_level].min_y;


      //  Increment the zoom level and compute the new bounds.

      pic.zoom_level--;


      pic.bounds[pic.zoom_level].min_x = pic.bounds[pic.zoom_level + 1].min_x - NINT (((NV_FLOAT32) xsize *
                                                                                       pic.zoom_percent) / 2.0);
      pic.bounds[pic.zoom_level].max_x = pic.bounds[pic.zoom_level + 1].max_x + NINT (((NV_FLOAT32) xsize *
                                                                                       pic.zoom_percent) / 2.0);

      if (pic.bounds[pic.zoom_level].min_x < pic.bounds[0].min_x)
        {
          pic.bounds[pic.zoom_level].min_x = pic.bounds[0].min_x;
          pic.bounds[pic.zoom_level].max_x = pic.bounds[pic.zoom_level].min_x + NINT ((NV_FLOAT32) xsize *
                                                                                      pic.zoom_percent * 2.0);

          if (pic.bounds[pic.zoom_level].max_x > pic.bounds[0].max_x)
            pic.bounds[pic.zoom_level].max_x = pic.bounds[0].max_x;
        }


      if (pic.bounds[pic.zoom_level].max_x > pic.bounds[0].max_x)
        {
          pic.bounds[pic.zoom_level].max_x = pic.bounds[0].max_x;
          pic.bounds[pic.zoom_level].min_x = pic.bounds[pic.zoom_level].max_x - NINT ((NV_FLOAT32) xsize *
                                                                                      pic.zoom_percent * 2.0);

          if (pic.bounds[pic.zoom_level].min_x < pic.bounds[0].min_x)
            pic.bounds[pic.zoom_level].min_x = pic.bounds[0].min_x;
        }


      pic.bounds[pic.zoom_level].min_y = pic.bounds[pic.zoom_level + 1].min_y - NINT (((NV_FLOAT32) ysize *
                                                                                       pic.zoom_percent) / 2.0);
      pic.bounds[pic.zoom_level].max_y = pic.bounds[pic.zoom_level + 1].max_y + NINT (((NV_FLOAT32) ysize *
                                                                                       pic.zoom_percent) / 2.0);

      if (pic.bounds[pic.zoom_level].min_y < pic.bounds[0].min_y)
        {
          pic.bounds[pic.zoom_level].min_y = pic.bounds[0].min_y;
          pic.bounds[pic.zoom_level].max_y = pic.bounds[pic.zoom_level].min_y + NINT ((NV_FLOAT32) ysize *
                                                                                      pic.zoom_percent * 2.0);

          if (pic.bounds[pic.zoom_level].max_y > pic.bounds[0].max_y)
            pic.bounds[pic.zoom_level].max_y = pic.bounds[0].max_y;
        }


      if (pic.bounds[pic.zoom_level].max_y > pic.bounds[0].max_y)
        {
          pic.bounds[pic.zoom_level].max_y = pic.bounds[0].max_y;
          pic.bounds[pic.zoom_level].min_y = pic.bounds[pic.zoom_level].max_y - NINT ((NV_FLOAT32) ysize *
                                                                                      pic.zoom_percent * 2.0);

          if (pic.bounds[pic.zoom_level].min_y < pic.bounds[0].min_y)
            pic.bounds[pic.zoom_level].min_y = pic.bounds[0].min_y;
        }

      redrawPic ();
    }
}



//!  Reset zoom level 0 to the specified bounds.

void 
nvPic::resetBounds (NV_I32_XYMBR bounds)
{
  pic.bounds[0] = bounds;
  pic.zoom_level = 0;
}



//!  Return a snapshot of the complete NVPIC_DEF structure.

NVPIC_DEF 
nvPic::getPicdef ()
{
  return (pic);
}



/*******************************  IMPORTANT NOTE  ***********************************************

    There can only be one instance of any of the following rubberband or move operations active
    at any given time.  It is best to "discard" or "close" the rubberband or moving object prior
    to starting a new one.  Note that redrawPic will automatically discard any
    active rubberband or moving object.  Also, note that the functions that actually do the 
    drawing do a repaint () instead of an update ().  This forces the paintevent function to
    perform the drawing immediately instead of trying to stack it and wait for later.

************************************************************************************************/


/*!  Starting a rubberband polygon in pic space.  Use for first point only.  If you want to draw
     a polyline instead of a closed polygon, set close to NVFalse.  */


void 
nvPic::anchorRubberbandPolygon (NV_INT32 pic_x, NV_INT32 pic_y, QColor color, NV_INT32 line_width, NV_BOOL close)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  anchorRubberbandPolygon (x, y, color, line_width, close, 0);
}



/*!  Starting a rubberband polygon in screen space.  Use for first point only.  If you want to draw
     a polyline instead of a closed polygon, set close to NVFalse.  */

void 
nvPic::anchorRubberbandPolygon (NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, NV_BOOL close, 
                                NV_BYTE q __attribute__ ((unused)))
{
  //  If there's a polygon active we need to get rid of it.

  if (rubberbandPolyActive) discardRubberbandPolygon ();


  floating_object.type = NVPIC_RUBBER_POLYGON;
  floating_object.color = color;
  floating_object.line_width = line_width;
  floating_object.close_polygon = close;
  floating_object.filled = NVFalse;
  floating_object.outlined = NVTrue;
  floating_object.state = NVPIC_OBJECT_ACTIVE;


  floating_object.polygon_x[0] = rubberbandPolyLastX = x;
  floating_object.polygon_y[0] = rubberbandPolyLastY = y;

  floating_object.polygon_count = 1;
  rubberbandPolyActive = NVTrue;
}



//!  Dragging a line (Tommy James and the Shondells) from the previous vertex in a rubberband polygon (pic space).

void 
nvPic::dragRubberbandPolygon (NV_INT32 pic_x, NV_INT32 pic_y)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  dragRubberbandPolygon (x, y, 0);
}



//!  Dragging a line from the previous vertex in a rubberband polygon (screen space).

void 
nvPic::dragRubberbandPolygon (NV_INT32 x, NV_INT32 y, NV_BYTE q __attribute__ ((unused)))
{
  if (rubberbandPolyActive && (rubberbandPolyLastX != x || rubberbandPolyLastY != y))
    {
      //  Save for previous last point.

      rubberbandPolyLastX = x;
      rubberbandPolyLastY = y;

      repaint ();
    }
}



//!  Return the current polygon line segment anchor position in pic coordinates.

void 
nvPic::getRubberbandPolygonCurrentAnchor (NV_INT32 *pic_x, NV_INT32 *pic_y)
{
  screen_to_pic (1, pic_x, pic_y, &floating_object.polygon_x[floating_object.polygon_count - 1], 
                 &floating_object.polygon_y[floating_object.polygon_count - 1]);
}



//!  Add a new vertex to a rubberband polygon in pic space.

void 
nvPic::vertexRubberbandPolygon (NV_INT32 pic_x, NV_INT32 pic_y)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  vertexRubberbandPolygon (x, y, 0);
}



//!  Add a new vertex to a rubberband polygon in screen space.

void 
nvPic::vertexRubberbandPolygon (NV_INT32 x, NV_INT32 y, NV_BYTE q __attribute__ ((unused)))
{
  floating_object.polygon_x[floating_object.polygon_count] = rubberbandPolyLastX = x;
  floating_object.polygon_y[floating_object.polygon_count] = rubberbandPolyLastY = y;

  floating_object.polygon_count++;

  if (floating_object.polygon_count >= NVPIC_POLYGON_POINTS)
    {
      fprintf (stderr, "You have exceeded the maximum number of polygon points (%d).", 
               NVPIC_POLYGON_POINTS);
      fflush (stderr);
      floating_object.polygon_count = 0;
      floating_object.state = NVPIC_OBJECT_CLEAR;
      return;
    }
}



//!  Return the points in the current polygon (in screen [px/py] and pic [mx/my] space) without closing the polygon.

void 
nvPic::getRubberbandPolygon (NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my)
{
  *px = floating_object.polygon_x;
  *py = floating_object.polygon_y;
  *count = floating_object.polygon_count;

  screen_to_pic (floating_object.polygon_count, rubberbandPolyPicX, rubberbandPolyPicY, floating_object.polygon_x,
                 floating_object.polygon_y);

  *mx = rubberbandPolyPicX;
  *my = rubberbandPolyPicY;
}



/*!  Close a rubberband polygon in pic space adding the input point (pic_x, pic_y).  Returns the
     points in the current polygon (in screen [px/py] and pic [mx/my] space).  The arguments
     px, py, mx, and my should be declared in your calling function as NV_INT32 *px and 
     NV_INT32 *mx.  They should be passed to this function as &px.  Remember to "discard"
     the polygon to free the associated allocated memory (returned as px, py, mx, my).  */

void 
nvPic::closeRubberbandPolygon (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 *count, NV_INT32 **px, 
                               NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  closeRubberbandPolygon (x, y, count, px, py, mx, my, 0);
}



/*!  Close a rubberband polygon in screen space adding the input point (x, y).  Returns the
     points in the current polygon (in screen [px/py] and pic [mx/my] space).  The arguments
     px, py, mx, and my should be declared in your calling function as NV_INT32 *px and 
     NV_INT32 *mx.  They should be passed to this function as &px.  Remember to "discard"
     the polygon to free the associated allocated memory (returned as px, py, mx, my).  */

void 
nvPic::closeRubberbandPolygon (NV_INT32 x, NV_INT32 y, NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, 
                               NV_INT32 **mx, NV_INT32 **my, NV_BYTE q __attribute__ ((unused)))
{
  rubberbandPolyActive = NVFalse;

  vertexRubberbandPolygon (x, y);


  getRubberbandPolygon (count, px, py, mx, my);
}



//  Returns whether a rubberband polygon is active (being created) at present.

NV_BOOL
nvPic::rubberbandPolygonIsActive ()
{
  return (rubberbandPolyActive);
}



//  Erase the last rubberband polygon and free the associated allocated memory.

void 
nvPic::discardRubberbandPolygon ()
{
  if (!floating_object.state) return;


  //  Just in case someone tries to discard before defining.

  if (!floating_object.polygon_count) return;


  rubberbandPolyActive = NVFalse;


  floating_object.polygon_count = 0;
  floating_object.state = NVPIC_OBJECT_CLEAR;

  repaint ();
}



//!  Anchor a rubberband rectangle in pic space.  Starting corner point.

void 
nvPic::anchorRubberbandRectangle (NV_INT32 pic_x, NV_INT32 pic_y, QColor color, NV_INT32 line_width)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  anchorRubberbandRectangle (x, y, color, line_width, 0);

  rubberbandRectPresent = NVTrue;
}



//!  Anchor a rubberband rectangle in screen space.  Starting corner point.

void 
nvPic::anchorRubberbandRectangle (NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, 
                                  NV_BYTE q __attribute__ ((unused)))
{
  if (rubberbandRectActive) discardRubberbandRectangle ();


  rubberbandRectX[0] = rubberbandRectLastX = x;
  rubberbandRectY[0] = rubberbandRectLastY = y;


  floating_object.color = color;
  floating_object.line_width = line_width;


  rubberbandRectPresent = NVTrue;
  rubberbandRectActive = NVTrue;
}



//!  Drag a rubberband rectangle in pic space.

void 
nvPic::dragRubberbandRectangle (NV_INT32 pic_x, NV_INT32 pic_y)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  dragRubberbandRectangle (x, y, 0);
}



//!  Drag a rubberband rectangle in screen space.

void 
nvPic::dragRubberbandRectangle (NV_INT32 x, NV_INT32 y, NV_BYTE q __attribute__ ((unused)))
{
  if (rubberbandRectActive && (rubberbandRectLastX != x || rubberbandRectLastY != y))
    {
      rubberbandRectLastX = x;
      rubberbandRectLastY = y;

      floating_object.type = NVPIC_RUBBER_RECTANGLE;
      floating_object.filled = NVFalse;
      floating_object.outlined = NVTrue;
      floating_object.rect.setCoords (rubberbandRectX[0], rubberbandRectY[0], x, y);
      floating_object.state = NVPIC_OBJECT_ACTIVE;

      repaint ();
    }
}



//!  Return the points in the current rectangle (in screen [px/py] and pic [mx/my] space) without closing the rectangle.

void 
nvPic::getRubberbandRectangle (NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my)
{
  *px = rubberbandRectX;
  *py = rubberbandRectY;

  for (NV_INT32 i = 0 ; i < 4 ; i++)
    {
      screen_to_pic (1, &rubberbandRectPicX[i], &rubberbandRectPicY[i], &rubberbandRectX[i],
                     &rubberbandRectY[i]);
    }

  *mx = rubberbandRectPicX;
  *my = rubberbandRectPicY;
}



/*!  Close a rubberband rectangle in pic space.  The ending corner is specified by the point
     pic_x,pic_y.  Returns the points in the rectangle as a 4 point polygon in screen space
     (px, py) and pic space (mx, my).  The arguments px, py, mx, and my should be declared
     in your calling function as NV_INT32 *px and NV_INT32 *mx.  They should be passed to
     this function as &px.  */

void 
nvPic::closeRubberbandRectangle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 **px, NV_INT32 **py, 
                                 NV_INT32 **mx, NV_INT32 **my)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  closeRubberbandRectangle (x, y, px, py, mx, my, 0);
}



/*!  Close a rubberband rectangle in screen space.  The ending corner is specified by the point
     x,y.  Returns the points in the rectangle as a 4 point polygon in screen space (px, py)
     and pic space (mx, my).  The arguments px, py, mx, and my should be declared in your calling
     function as NV_INT32 *px and NV_INT32 *mx.  They should be passed to this function as &px.  */

void 
nvPic::closeRubberbandRectangle (NV_INT32 x, NV_INT32 y, NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, 
                                 NV_INT32 **my, NV_BYTE q __attribute__ ((unused)))
{
  rubberbandRectActive = NVFalse;

  rubberbandRectX[1] = rubberbandRectX[0];
  rubberbandRectY[1] = y;
  rubberbandRectX[2] = x;
  rubberbandRectY[2] = y;
  rubberbandRectX[3] = x;
  rubberbandRectY[3] = rubberbandRectY[0];

  *px = rubberbandRectX;
  *py = rubberbandRectY;

  screen_to_pic (4, rubberbandRectPicX, rubberbandRectPicY, rubberbandRectX, rubberbandRectY);

  *mx = rubberbandRectPicX;
  *my = rubberbandRectPicY;
}



//!  Return whether a rubberband rectangle is active.

NV_BOOL 
nvPic::rubberbandRectangleIsActive ()
{
  return (rubberbandRectActive);
}



//!  Return whether a rubberband rectangle is present (may not be active).

NV_BOOL
nvPic::rubberbandRectangleIsPresent ()
{
  return (rubberbandRectPresent);
}



//!  Return the pic anchor position of the current active rectangle.

void 
nvPic::getRubberbandRectangleAnchor (NV_INT32 *pic_x, NV_INT32 *pic_y)
{
  screen_to_pic (1, pic_x, pic_y, &rubberbandRectX[0], &rubberbandRectY[0]);
}



//!  Erase the rubberband rectangle.

void 
nvPic::discardRubberbandRectangle ()
{
  if (!floating_object.state) return;

  floating_object.state = NVPIC_OBJECT_CLEAR;

  repaint ();

  rubberbandRectActive = NVFalse;
  rubberbandRectPresent = NVFalse;
}



//!  Anchor a rubberband line in pic space.  Start point.

void 
nvPic::anchorRubberbandLine (NV_INT32 pic_x, NV_INT32 pic_y, QColor color, NV_INT32 line_width)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  anchorRubberbandLine (x, y, color, line_width, 0);
}



//!  Anchor a rubberband line in screen space.  Start point.

void 
nvPic::anchorRubberbandLine (NV_INT32 x, NV_INT32 y, QColor color, NV_INT32 line_width, NV_BYTE q __attribute__ ((unused)))
{
  if (rubberbandLineActive) discardRubberbandLine ();

  floating_object.color = color;
  floating_object.line_width = line_width;

  rubberbandLineAnchorX = x;
  rubberbandLineAnchorY = y;

  rubberbandLineLastX = x;
  rubberbandLineLastY = y;

  rubberbandLineActive = NVTrue;
}



//!  Drag a rubberband line in pic space.

void 
nvPic::dragRubberbandLine (NV_INT32 pic_x, NV_INT32 pic_y)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  dragRubberbandLine (x, y, 0);
}



//!  Drag a rubberband line in screen space.

void 
nvPic::dragRubberbandLine (NV_INT32 x, NV_INT32 y, NV_BYTE q __attribute__ ((unused)))
{
  if (rubberbandLineActive && (rubberbandLineLastX != x || rubberbandLineLastY != y))
    {
      floating_object.type = NVPIC_RUBBER_LINE;
      floating_object.filled = NVFalse;
      floating_object.outlined = NVTrue;
      floating_object.x0 = rubberbandLineAnchorX;
      floating_object.y0 = rubberbandLineAnchorY;
      floating_object.x1 = x;
      floating_object.y1 = y;
      floating_object.state = NVPIC_OBJECT_ACTIVE;

      repaint ();

      rubberbandLineLastX = x;
      rubberbandLineLastY = y;
    }
}



//!  Return whether a rubberband line is active.

NV_BOOL 
nvPic::rubberbandLineIsActive ()
{
  return (rubberbandLineActive);
}



//!  Erase the rubberband line.

void 
nvPic::discardRubberbandLine ()
{
  if (!floating_object.state) return;

  floating_object.state = NVPIC_OBJECT_CLEAR;

  repaint ();

  rubberbandLineActive = NVFalse;
}



/*!  Move a rectangle in pic space without wiping out the underlying stuff.  The is defined by the
     two corner points pic_x0,pic_y0 and pic_x1,pic_y1.  The rectangle may be filled with the specified
     color by setting the "filled" argument to NVTrue.  */

void
nvPic::setMovingRectangle (NV_INT32 pic_x0, NV_INT32 pic_y0, NV_INT32 pic_x1, NV_INT32 pic_y1, 
                           QColor color, NV_INT32 line_width, NV_BOOL filled)
{
  NV_INT32 x0, y0, x1, y1;

  pic_to_screen (1, &pic_x0, &pic_y0, &x0, &y0);
  pic_to_screen (1, &pic_x1, &pic_y1, &x1, &y1);

  setMovingRectangle (x0, y0, x1, y1, color, line_width, filled, 0);
}



/*!  Move a rectangle in screen space without wiping out the underlying stuff.  The is defined by the
     two corner points x0,y0 and x1,y1.  The rectangle may be filled with the specified color by setting
     the "filled" argument to NVTrue.  */

void
nvPic::setMovingRectangle (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width, 
                           NV_BOOL filled, NV_BYTE q __attribute__ ((unused)))
{
  floating_object.color = color;
  floating_object.line_width = line_width;
  floating_object.type = NVPIC_MOVE_RECTANGLE;
  floating_object.filled = filled;
  floating_object.outlined = NVTrue;
  floating_object.rect.setCoords (x0, y0, x1, y1);
  floating_object.state = NVPIC_OBJECT_ACTIVE;

  repaint ();
}



//!  Get the coordinates for the current moving rectangle (in screen [px/py] and pic [mx/my] space).

void 
nvPic::getMovingRectangle (NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my)
{
  rubberbandRectX[0] = floating_object.rect.left ();
  rubberbandRectY[0] = floating_object.rect.bottom ();
  rubberbandRectX[1] = floating_object.rect.left ();
  rubberbandRectY[1] = floating_object.rect.top ();
  rubberbandRectX[2] = floating_object.rect.right ();
  rubberbandRectY[2] = floating_object.rect.top ();
  rubberbandRectX[3] = floating_object.rect.right ();
  rubberbandRectY[3] = floating_object.rect.bottom ();

  *px = rubberbandRectX;
  *py = rubberbandRectY;

  screen_to_pic (4, rubberbandRectPicX, rubberbandRectPicY, rubberbandRectX, rubberbandRectY);

  *mx = rubberbandRectPicX;
  *my = rubberbandRectPicY;
}



//!  Close a moving rectangle

void 
nvPic::closeMovingRectangle ()
{
  if (!floating_object.state) return;

  floating_object.state = NVPIC_OBJECT_CLEAR;

  repaint ();
}



/*!  Move a polygon in pic space without wiping out the underlying stuff.  This is defined by the arrays
     pic_x[] and pic_y[].  The polygon may be filled with the specified color by setting the "filled"
     argument to NVTrue.  */

void
nvPic::setMovingPolygon (NV_INT32 count, NV_INT32 pic_x[], NV_INT32 pic_y[], QColor color, NV_INT32 line_width,
                         NV_BOOL filled)
{
  NV_INT32 *x = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));

  pic_to_screen (count, pic_x, pic_y, x, y);

  setMovingPolygon (count, x, y, color, line_width, filled, 0);

  free (x);
  free (y);
}



/*!  Move a polygon in screen space without wiping out the underlying stuff.  This is defined by the arrays
     x[] and y[].  The polygon may be filled with the specified color by setting the "filled" argument to
     NVTrue.  */

void
nvPic::setMovingPolygon (NV_INT32 count, NV_INT32 *x, NV_INT32 *y, QColor color, NV_INT32 line_width, NV_BOOL filled,
                         NV_BYTE q __attribute__ ((unused)))
{
  if (count > NVPIC_POLYGON_POINTS)
    {
      fprintf (stderr, "You have exceeded the maximum number of polygon points (%d).\nTerminating\n",
               NVPIC_POLYGON_POINTS);
      fflush (stderr);
      floating_object.polygon_count = 0;
      floating_object.state = NVPIC_OBJECT_CLEAR;
      return;
    }

  floating_object.color = color;
  floating_object.line_width = line_width;
  floating_object.type = NVPIC_MOVE_POLYGON;
  floating_object.filled = filled;
  floating_object.outlined = NVTrue;
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      floating_object.polygon_x[i] = x[i];
      floating_object.polygon_y[i] = y[i];
    }
  floating_object.polygon_count = count;
  floating_object.state = NVPIC_OBJECT_ACTIVE;

  repaint ();
}



//!  Return the points in the current moving polygon (in screen [px/py] and pic [mx/my] space).

void 
nvPic::getMovingPolygon (NV_INT32 *count, NV_INT32 **px, NV_INT32 **py, NV_INT32 **mx, NV_INT32 **my)
{
  getRubberbandPolygon (count, px, py, mx, my);
}



//!  Close a moving polygon.

void 
nvPic::closeMovingPolygon ()
{
  if (!floating_object.state) return;

  floating_object.state = NVPIC_OBJECT_CLEAR;

  repaint ();
}



/*!  Move a circle in pic space without wiping out the underlying stuff.  The center of the circle will
     be at pic_x,pic_y and it will have diameter "d" in pixels.  start_angle and span_angle are in degrees
     but are converted to 16ths of a degree for Qt.  */

void
nvPic::setMovingCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 d, NV_FLOAT32 start_angle, 
                        NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, NV_BOOL filled)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  setMovingCircle (x, y, d, start_angle, span_angle, line_width, color, filled, 0);
}



/*!  Move a circle in pic space without wiping out the underlying stuff.  The center of the circle will
     be at pic_x,pic_y and the diameter of the circle will be twice the distance to end_pic_x,end_pic_y.
     start_angle and span_angle are in degrees but are converted to 16ths of a degree for Qt.  */

void
nvPic::setMovingCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 end_pic_x, NV_INT32 end_pic_y, 
                      NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color, NV_BOOL filled)
{
  NV_INT32 x0, y0, x1, y1, x2, y2, d;

  pic_to_screen (1, &pic_x, &pic_y, &x0, &y0);
  pic_to_screen (1, &end_pic_x, &end_pic_y, &x1, &y1);

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

  setMovingCircle (x0, y0, d, start_angle, span_angle, line_width, color, filled, 0);
}



/*!  Move a circle in screen space without wiping out the underlying stuff.  The center of the circle will
     be at x,y and it will have diameter "d" in pixels.  start_angle and span_angle are in degrees
     but are converted to 16ths of a degree for Qt.  */

void
nvPic::setMovingCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, 
                        NV_INT32 line_width, QColor color, NV_BOOL filled, NV_BYTE q __attribute__ ((unused)))
{
  floating_object.color = color;
  floating_object.line_width = line_width;
  floating_object.type = NVPIC_MOVE_CIRCLE;
  floating_object.filled = filled;
  floating_object.outlined = NVTrue;
  floating_object.x = x;
  floating_object.y = y;
  floating_object.width = d;
  floating_object.height = d;
  floating_object.start_angle = NINT (start_angle * 16.0);
  floating_object.span_angle = NINT (span_angle * 16.0);
  floating_object.state = NVPIC_OBJECT_ACTIVE;

  repaint ();
}



//!  Get the current moving circle (in screen and pic coordinates) with diameter "d" in pixels.

void
nvPic::getMovingCircle (NV_INT32 *pic_x, NV_INT32 *pic_y, NV_INT32 *x, NV_INT32 *y, NV_INT32 *d)
{
  *x = floating_object.x;
  *y = floating_object.y;
  *d = floating_object.width;

  screen_to_pic (1, pic_x, pic_y, x, y);
}



//!  Close a moving circle.

void 
nvPic::closeMovingCircle ()
{
  if (!floating_object.state) return;

  floating_object.state = NVPIC_OBJECT_CLEAR;

  repaint ();
}



/*!
    Move a painter path in pic space.  You can define any type of object using a painter path in
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

    The path will be drawn with its center at pic_center_x,pic_center_y based on the bounding rectangle of 
    the path.  If you wish to fill the path you'll need to set the "brush" attributes and set the "filled"
    argument to NVTrue.  You can rotate the path by setting the angle value.

*/

void 
nvPic::setMovingPath (QPainterPath path, NV_INT32 pic_center_x, NV_INT32 pic_center_y, NV_INT32 line_width,
                      QColor color, QBrush brush, NV_BOOL filled, NV_FLOAT64 angle)
{
  NV_INT32 center_x, center_y;

  pic_to_screen (1, &pic_center_x, &pic_center_y, &center_x, &center_y);

  setMovingPath (path, center_x, center_y, line_width, color, brush, filled, angle, 0);
}



void 
nvPic::setMovingPath (QPainterPath path, NV_INT32 pic_center_x, NV_INT32 pic_center_y, NV_INT32 line_width,
                      QColor color, QBrush brush, NV_BOOL filled)
{
  setMovingPath (path, pic_center_x, pic_center_y, line_width, color, brush, filled, 0.0, 0);
}



/*!  Move a painter path in screen space.  See above for example.  The path will be drawn with its center
     at center_x,center_y based on the bounding rectangle of the path.  If you wish to fill the path you'll
     need to set the "brush" attributes and set the "filled" argument to NVTrue.  You can rotate the path by setting
     the angle value.  */

void 
nvPic::setMovingPath (QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width, QColor color,
                      QBrush brush, NV_BOOL filled, NV_FLOAT64 angle, NV_BYTE q __attribute__ ((unused)))
{
  floating_object.color = color;
  floating_object.line_width = line_width;
  floating_object.type = NVPIC_MOVE_PATH;
  floating_object.path = path;
  floating_object.filled = filled;
  floating_object.brush = brush;
  floating_object.outlined = NVTrue;
  floating_object.x = center_x;
  floating_object.y = center_y;
  floating_object.rotation_angle = angle;
  floating_object.state = NVPIC_OBJECT_ACTIVE;

  repaint ();
}



//!  Close a moving path.

void 
nvPic::closeMovingPath ()
{
  if (!floating_object.state) return;

  floating_object.state = NVPIC_OBJECT_CLEAR;

  repaint ();
}



/***********************************************************************************************/


/*!  Draw text in pic space.  The text will start with the lower left corner at
     pic_x,pic_y.  Size is in pixels.  Angle doesn't work 'cause I can't figure out how to get
     the correct bounding rectangle after rotation.  */

void
nvPic::drawText (QString string, NV_INT32 pic_x, NV_INT32 pic_y, NV_FLOAT64 angle, NV_INT32 size, QColor color)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  drawText (string, x, y, angle, size, color, 0);
}



/*!  Draw text in screen space.  The text will start with the lower left corner at
     x,y.  Size is in pixels.  Use an angle of 90.0 for normal horizontal text.  */

void
nvPic::drawText (QString string, NV_INT32 x, NV_INT32 y, NV_FLOAT64 angle, NV_INT32 size, QColor color, 
                 NV_BYTE q __attribute__ ((unused)))
{
  painter.save ();
  painter.translate (x, y);
  painter.rotate (angle - 90.0);

  QFont font ("Charter", size);
  font = QFont (font, painter.device ());
  QFontMetrics fontMetrics (font);
  QRect rect = fontMetrics.boundingRect (string);

  rect.setLeft (rect.left () + x);
  rect.setRight (rect.right () + x);
  rect.setBottom (rect.bottom () + y);
  rect.setTop (rect.top () + y);

  painter.setFont(font);

  painter.setPen (color);

  QRect r;
  painter.drawText (rect, Qt::AlignLeft | Qt::AlignBottom, string, &r); 

  painter.restore ();

  update (r);
}



//!  Draw text in pic space.  Uses a default angle of 90 degrees and height of 12 pixels.

void
nvPic::drawText (QString string, NV_INT32 pic_x, NV_INT32 pic_y, QColor color)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  drawText (string, x, y, color, 0);
}



//!  Draw text in screen space.  Uses a default angle of 90 degrees and height of 12 pixels.

void
nvPic::drawText (QString string, NV_INT32 x, NV_INT32 y, QColor color, NV_BYTE q __attribute__ ((unused)))
{
  drawText (string, x, y, 90.0, 12, color, q);
}



/*!  Draw a line in pic space (optionally flush to screen).  The line is defined by the points
     pic_x0,pic_y0 and pic_x1,pic_y1.  Note: if you don't flush to the screen you will have to
     keep track of what needs to be updated or you will have to call flush for the entire
     screen area (i.e. dump the pixmap to the screen).  In practice, flush usually works just
     fine.  */

void
nvPic::drawLine (NV_INT32 pic_x0, NV_INT32 pic_y0, NV_INT32 pic_x1, NV_INT32 pic_y1,
                 QColor color, NV_INT32 line_width, NV_BOOL flush)
{
  NV_INT32 x0, y0, x1, y1;

  pic_to_screen (1, &pic_x0, &pic_y0, &x0, &y0);
  pic_to_screen (1, &pic_x1, &pic_y1, &x1, &y1);

  drawLine (x0, y0, x1, y1, color, line_width, flush, 0);
}



/*!  Draw a line in screen space (optionally flush to screen).  The line is defined by the points
     x0,y0 and x1,y1.  Note: if you don't flush to the screen you will have to keep track of what
     needs to be updated or you will have to call flush for the entire screen area (i.e. dump the
     pixmap to the screen).  In practice, flush usually works just fine.  */

void
nvPic::drawLine (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width, NV_BOOL flush,
                 NV_BYTE q __attribute__ ((unused)))
{
  QPen pen;
  pen.setColor (color);
  pen.setWidth (line_width);
  pen.setStyle (Qt::SolidLine);

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



/*!  Draw a circle in pic space.  The center of the circle will be at pic_x,pic_y and it will
     have diameter "d" in pixels.  start_angle and span_angle are in degrees
     but are converted to 16ths of a degree for Qt.  */

void
nvPic::drawCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 d, NV_FLOAT32 start_angle, 
                   NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  drawCircle (x, y, d, start_angle, span_angle, line_width, color, 0);
}



/*!  Draw a circle in pic space.  The center of the circle will be at pic_x,pic_y and the diameter
     of the circle will twice the distance to end_pic_x,end_pic_y.  start_angle and span_angle
     are in degrees but are converted to 16ths of a degree for Qt.  */

void
nvPic::drawCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 end_pic_x, NV_INT32 end_pic_y, 
                   NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, NV_INT32 line_width, QColor color)
{
  NV_INT32 x0, y0, x1, y1, x2, y2, d;

  pic_to_screen (1, &pic_x, &pic_y, &x0, &y0);
  pic_to_screen (1, &end_pic_x, &end_pic_y, &x1, &y1);

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

  drawCircle (x0, y0, d, start_angle, span_angle, line_width, color, 0);
}



/*!  Draw a circle in screen space.  The center of the circle will be at pic_x,pic_y and it will
     have diameter "d" in pixels.  start_angle and span_angle are in degrees but are converted
     to 16ths of a degree for Qt.  */

void
nvPic::drawCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                   NV_INT32 line_width, QColor color, NV_BYTE q __attribute__ ((unused)))
{
  //  Center the circle.

  NV_INT32 newx = x - d / 2;
  NV_INT32 newy = y - d / 2;


  QPen pen;
  pen.setColor (color);
  pen.setWidth (line_width);
  pen.setStyle (Qt::SolidLine);
  painter.setPen (pen);

  painter.drawArc (newx, newy, d, d, NINT (start_angle * 16.0), NINT (span_angle * 16.0));

  update (newx, newy, d, d);
}



/*!  Draw a filled circle in pic space.  The center of the circle will be at pic_x,pic_y and it will
     have diameter "d" in pixels.  start_angle and span_angle are in degrees but are converted
     to 16ths of a degree for Qt.  */

void
nvPic::fillCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 d, NV_FLOAT32 start_angle, 
                   NV_FLOAT32 span_angle, QColor color)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  fillCircle (x, y, d, start_angle, span_angle, color, 0);
}



/*!  Draw a filled circle in pic space.  The center of the circle will be at pic_x,pic_y and the diameter
     of the circle will twice the distance to end_pic_x,end_pic_y.  start_angle and span_angle
     are in degrees but are converted to 16ths of a degree for Qt.  */

void
nvPic::fillCircle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 end_pic_x, NV_INT32 end_pic_y, 
                   NV_FLOAT32 start_angle, NV_FLOAT32 span_angle, QColor color)
{
  NV_INT32 x0, y0, x1, y1, x2, y2, d;

  pic_to_screen (1, &pic_x, &pic_y, &x0, &y0);
  pic_to_screen (1, &end_pic_x, &end_pic_y, &x1, &y1);

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

  fillCircle (x0, y0, d, start_angle, span_angle, color, 0);
}



/*!  Draw a filled circle in screen space.  The center of the circle will be at pic_x,pic_y and it will
     have diameter "d" in pixels.  start_angle and span_angle are in degrees but are converted to
     16ths of a degree for Qt.  */

void
nvPic::fillCircle (NV_INT32 x, NV_INT32 y, NV_INT32 d, NV_FLOAT32 start_angle, NV_FLOAT32 span_angle,
                   QColor color, NV_BYTE q __attribute__ ((unused)))
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

  update (newx, newy, d, d);
}



/*!  Draw and fill a rectangle in pic space (optionally flush to screen).  The upper left corner of the
     rectangle is defined by pic_x,pic_y and it will be "w" pixels wide by "h" pixels high.  Note: if
     you don't flush to the screen you will have to keep track of what needs to be updated or you will
     have to call flush for the entire screen area (i.e. dump the pixmap to the screen).  */

void 
nvPic::fillRectangle (NV_INT32 pic_x, NV_INT32 pic_y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush)
{
  NV_INT32 x, y;

  pic_to_screen (1, &pic_x, &pic_y, &x, &y);

  fillRectangle (x, y, w, h, color, flush, 0);
}



/*!  Draw and fill a rectangle in screen space (optionally flush to screen).  The lower left corner of the
     rectangle is defined by x,y and it will be "w" pixels wide by "h" pixels high.  Note: if you don't
     flush to the screen you will have to keep track of what needs to be updated or you will have to call
     flush for the entire screen area (i.e. dump the pixmap to the screen).  */

void
nvPic::fillRectangle (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h, QColor color, NV_BOOL flush, 
                      NV_BYTE q __attribute__ ((unused)))
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



/*!  Draw a rectangle in pic space.  The upper left corner of the rectangle will be at pic_x0,pic_y0
     and the lower right corner will be at pic_x1,pic_y1.  */

void
nvPic::drawRectangle (NV_INT32 pic_x0, NV_INT32 pic_y0, NV_INT32 pic_x1, NV_INT32 pic_y1,
                    QColor color, NV_INT32 line_width)
{
  NV_INT32 x0, y0, x1, y1;

  pic_to_screen (1, &pic_x0, &pic_y0, &x0, &y0);
  pic_to_screen (1, &pic_x1, &pic_y1, &x1, &y1);

  drawRectangle (x0, y0, x1, y1, color, line_width, 0);
}



/*!  Draw a rectangle in screen space.  The upper left corner of the rectangle will be at x0,y0 and
     the lower right corner will be at x1,y1.  */

void
nvPic::drawRectangle (NV_INT32 x0, NV_INT32 y0, NV_INT32 x1, NV_INT32 y1, QColor color, NV_INT32 line_width, 
                      NV_BYTE q __attribute__ ((unused)))
{
  //  So why am I doing four lines instead of a rectangle.  A stupid X bug
  //  I think.  It has to do with trying to make a rectangle where the start 
  //  position is less in X or Y than the end position.  I had the same problems 
  //  using X11 directly.

  drawLine (x0, y0, x0, y1, color, line_width, NVTrue);
  drawLine (x0, y1, x1, y1, color, line_width, NVTrue);
  drawLine (x1, y1, x1, y0, color, line_width, NVTrue);
  drawLine (x1, y0, x0, y0, color, line_width, NVTrue);

  QRect r;
  r.setCoords (x0, y0, x1, y1);
  update (r.normalized ());
}



//!  Draw and fill a polygon in pic space.

void
nvPic::fillPolygon (NV_INT32 count, NV_INT32 pic_x[], NV_INT32 pic_y[], QColor color)
{
  NV_INT32 *x = (NV_INT32 *) calloc (count, sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) calloc (count, sizeof (NV_INT32));

  pic_to_screen (count, pic_x, pic_y, x, y);

  fillPolygon (count, x, y, color, 0);

  free (x);
  free (y);
}



//!  Draw and fill a polygon in screen space.

void
nvPic::fillPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color, NV_BYTE q __attribute__ ((unused)))
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

  painter.drawPolyline (poly);

  QRect r = poly.boundingRect ();

  r = r.normalized ();
  r.setLeft (r.left () - 1);
  r.setTop (r.top () - 1);
  r.setRight (r.right () + 1);
  r.setBottom (r.bottom () + 1);

  update (r.normalized ());
}



//!  Draw a polygon in pic space.

void
nvPic::drawPolygon (NV_INT32 count, NV_INT32 pic_x[], NV_INT32 pic_y[], QColor color, NV_INT32 line_width)
{
  NV_INT32 *x = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));
  NV_INT32 *y = (NV_INT32 *) calloc (count + 1, sizeof (NV_INT32));

  pic_to_screen (count, pic_x, pic_y, x, y);

  drawPolygon (count, x, y, color, line_width, 0);

  free (x);
  free (y);
}



//!  Draw a polygon in screen space.

void
nvPic::drawPolygon (NV_INT32 count, NV_INT32 x[], NV_INT32 y[], QColor color, NV_INT32 line_width, 
                    NV_BYTE q __attribute__ ((unused)))
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

  for (NV_INT32 i = 0 ; i < count ; i++) drawLine (new_x[i], new_y[i], new_x[i + 1], new_y[i + 1], color, line_width,
                                                   NVTrue);

  free (new_x);
  free (new_y);
}



/*!
    Draw a painter path in pic space.  You can define any type of object using a painter path in
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

    The path will be drawn with its center at pic_center_x,pic_center_y based on the bounding rectangle of 
    the path.  If you wish to fill the path you'll need to set the "brush" attributes and set the "filled"
    argument to NVTrue.

*/

void 
nvPic::drawPath (QPainterPath path, NV_INT32 pic_center_x, NV_INT32 pic_center_y, NV_INT32 line_width, QColor color,
                 QBrush brush, NV_BOOL filled)
{
  NV_INT32 center_x, center_y;

  pic_to_screen (1, &pic_center_x, &pic_center_y, &center_x, &center_y);

  drawPath (path, center_x, center_y, line_width, color, brush, filled, 0);
}



/*!  Draw a painter path in screen space.  See above for example.  The path will be drawn with its center
     at center_x,center_y based on the bounding rectangle of the path.  If you wish to fill the path you'll
     need to set the "brush" attributes and set the "filled" argument to NVTrue.  */

void 
nvPic::drawPath (QPainterPath path, NV_INT32 center_x, NV_INT32 center_y, NV_INT32 line_width, QColor color,
                 QBrush brush, NV_BOOL filled, NV_BYTE q __attribute__ ((unused)))
{
  //  Translate the center of the painter path to the provided x and y.

  QRectF r2 = path.boundingRect ();

  NV_INT32 x_offset = center_x - (NV_INT32) (r2.width () / 2.0);
  NV_INT32 y_offset = center_y - (NV_INT32) (r2.height () / 2.0);

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
      pen.setStyle (Qt::SolidLine);
      painter.setPen (pen);

      painter.drawPath (path);
    }

  painter.translate (-x_offset, -y_offset);

  update ();
}



/*!  Paint a pixmap at screen location x,y.  The pixmap is "src".  The location src_x,src_y
     is where in the pixmap you're going to get the upper left corner of what you want to
     paint (normally 0,0).  The width will be src_w and the height will be src_h.  */

void 
nvPic::pixmapPaint (NV_INT32 x, NV_INT32 y, QPixmap *src, NV_INT32 src_x, NV_INT32 src_y, NV_INT32 src_w, NV_INT32 src_h)
{
  painter.drawPixmap (x, y, *src, src_x, src_y, src_w, src_h);

  update (x, y, src_w, src_h);
}



//!  Grab a pixmap at screen location x,y.  Width is w.  Height is h.

QPixmap 
nvPic::grabPixmap (NV_INT32 x, NV_INT32 y, NV_INT32 w, NV_INT32 h)
{
  return (buffer.copy (x, y, w, h));
}



//!  Make sure a pic point (pic_x,pic_y) is in the currently displayed pic area.

void 
nvPic::boundsCheck (NV_INT32 *pic_x, NV_INT32 *pic_y)
{
  if (*pic_x < pic.bounds[pic.zoom_level].min_x) *pic_x = pic.bounds[pic.zoom_level].min_x;
  if (*pic_x > pic.bounds[pic.zoom_level].max_x) *pic_x = pic.bounds[pic.zoom_level].max_x;
  if (*pic_y < pic.bounds[pic.zoom_level].min_y) *pic_y = pic.bounds[pic.zoom_level].min_y;
  if (*pic_y > pic.bounds[pic.zoom_level].max_y) *pic_y = pic.bounds[pic.zoom_level].max_y;
}



/*!  Double clicked a mouse button.  Convert screen coordinates to pic coordinates and
     emit a signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvPic::mouseDoubleClickEvent (QMouseEvent *e)
{
  if (signalsEnabled)
    {
      NV_INT32 pic_x, pic_y;
      NV_INT32 x = e->x ();
      NV_INT32 y = e->y ();

      screen_to_pic (1, &pic_x, &pic_y, &x, &y);

      boundsCheck (&pic_x, &pic_y);


      //  Let the parent do something.

      emit mouseDoubleClickSignal (e, pic_x, pic_y);
    }
}



/*!  Pressed a mouse button.  Convert screen coordinates to pic coordinates and
     emit a signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvPic::mousePressEvent (QMouseEvent *e)
{
  if (signalsEnabled)
    {
      NV_INT32 pic_x, pic_y;
      NV_INT32 x = e->x ();
      NV_INT32 y = e->y ();

      screen_to_pic (1, &pic_x, &pic_y, &x, &y);

      boundsCheck (&pic_x, &pic_y);


      //  Let the parent do something.

      emit mousePressSignal (e, pic_x, pic_y);
    }
  else
    {
      emit preliminaryMousePressSignal (e);
    }
}



/*!  Released a mouse button.  Convert screen coordinates to pic coordinates and
     emit a signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvPic::mouseReleaseEvent (QMouseEvent *e)
{
  if (signalsEnabled)
    {
      NV_INT32 pic_x, pic_y;
      NV_INT32 x = e->x ();
      NV_INT32 y = e->y ();

      screen_to_pic (1, &pic_x, &pic_y, &x, &y);


      boundsCheck (&pic_x, &pic_y);


      //  Let the parent do something.

      emit mouseReleaseSignal (e, pic_x, pic_y);
    }
}



/*!  Moved mouse.  Convert screen coordinates to pic coordinates and emit a 
     signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvPic::mouseMoveEvent (QMouseEvent *e)
{
  if (signalsEnabled)
    {
      NV_INT32 pic_x, pic_y;
      NV_INT32 x = e->x ();
      NV_INT32 y = e->y ();


      screen_to_pic (1, &pic_x, &pic_y, &x, &y);


      boundsCheck (&pic_x, &pic_y);


      //  Let the parent do something.

      emit mouseMoveSignal (e, pic_x, pic_y);
    }
}



//!  Popped a wheelie!

void
nvPic::wheelEvent (QWheelEvent *e)
{
  if (signalsEnabled)
    {
      //  Let the parent do something.

      emit mouseWheelSignal (e);
    }
}



//!  Pressed a key.

void
nvPic::keyPressEvent (QKeyEvent * e)
{
  //  Let the parent do something.

  emit keyPressSignal (e);
}



/*!  Emit an update to paint the off-screen pixmap to the screen.  Use this if you've drawn some lines
     or painted some rectangles without setting the "flush" argument to NVTrue.  */

void 
nvPic::flush ()
{
  update ();
}



//!  Duh, resize event.

void
nvPic::resizeEvent (QResizeEvent *e)
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


  pic.draw_width = w;
  pic.draw_height = h;


  //  Start the painter to the off-screen pixmap.

  painter.begin (&buffer);


  //  Only redraw if we changed size and signals are enabled.

  if (signalsEnabled && (ow != w || oh != h)) redrawPic ();


  //  Tell someone we resized.

  emit resizeSignal (e);


  ow = w;
  oh = h;
}



void 
nvPic::closeEvent (QCloseEvent *e)
{
  emit closeSignal (e);
}



/*!  These two focus events are here because the paintevent would sometimes go batshit if there was a floating
     object and you lost or gained focus.  I have no idea why.  They'll emit a signal if you need it for anything.  */

void 
nvPic::focusInEvent (QFocusEvent *e)
{
  emit focusInSignal (e);
}



void 
nvPic::focusOutEvent (QFocusEvent *e)
{
  emit focusOutSignal (e);
}



//!  This is the equivalent of the X expose event.  It's also triggered by calling update () or repaint ().

void
nvPic::paintEvent (QPaintEvent *e)
{
  QPainter screen_painter (this);


  //  Floating objects (moving or rubberband) are only drawn on the screen, not to the pixmap.

  if (floating_object.state)
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

      //  floating_object.prev_rect.setLeft (qMax (0, floating_object.prev_rect.left ()));
      //  floating_object.prev_rect.setTop (qMax (0, floating_object.prev_rect.top ()));
      //  floating_object.prev_rect.setRight (qMin ((width () - 1), floating_object.prev_rect.right ()));
      //  floating_object.prev_rect.setBottom (qMin ((height () - 1), floating_object.prev_rect.bottom ()));
      //  screen_painter.drawPixmap (floating_object.prev_rect, buffer, floating_object.prev_rect);

      if (floating_object.state == NVPIC_OBJECT_ACTIVE)
        {
          NV_INT32 inc = floating_object.line_width / 2 + 1;
          NV_INT32 inc2 = inc * 2;
          NV_INT32 newx, newy;
          QPen pen;
          QRect r;
          QRectF r2;
          QPolygon poly;
          QBrush brush;

          switch (floating_object.type)
            {
            case NVPIC_MOVE_POLYGON:
              if (floating_object.filled)
                {
                  brush.setStyle (Qt::SolidPattern);
                  brush.setColor (floating_object.color);
                  screen_painter.setPen (floating_object.color);

                  screen_painter.setBrush (brush);
                }
              else
                {
                  pen.setColor (floating_object.color);
                  pen.setWidth (floating_object.line_width);
                  pen.setStyle (Qt::SolidLine);

                  screen_painter.setPen (pen);
                }

              poly = QPolygon (floating_object.polygon_count);

              for (NV_INT32 i = 0 ; i < floating_object.polygon_count ; i++) 
                {
                  poly.setPoint (i, floating_object.polygon_x[i], floating_object.polygon_y[i]);
                }

              screen_painter.drawPolygon (poly);

              r = poly.boundingRect ().normalized ();

              r.setLeft (r.left () - inc);
              r.setTop (r.top () - inc);
              r.setRight (r.right () + inc);
              r.setBottom (r.bottom () + inc);

              floating_object.prev_rect = r;
              break;


            case NVPIC_MOVE_RECTANGLE:
              r = floating_object.rect;

              if (floating_object.filled)
                {
                  //  Don't outline the rectangle

                  screen_painter.setPen (Qt::NoPen);

                  brush.setStyle (Qt::SolidPattern);
                  brush.setColor (floating_object.color);
                  screen_painter.setPen (floating_object.color);

                  screen_painter.fillRect (r.normalized (), brush);
                }
              else
                {
                  pen.setColor (floating_object.color);
                  pen.setWidth (floating_object.line_width);
                  pen.setStyle (Qt::SolidLine);

                  screen_painter.setPen (pen);

                  screen_painter.drawRect (r.normalized ());
                }


              floating_object.prev_rect.setCoords (r.normalized().x() - inc, r.normalized().y() - inc,
                                                   r.normalized().x() + r.normalized().width() + inc2,
                                                   r.normalized().y() + r.normalized().height() + inc2);

              break;


            case NVPIC_MOVE_CIRCLE:

              //  Center the circle.

              newx = floating_object.x - floating_object.width / 2;
              newy = floating_object.y - floating_object.height / 2;


              if (floating_object.filled)
                {
                  brush.setStyle (Qt::SolidPattern);
                  brush.setColor (floating_object.color);
                  screen_painter.setPen (floating_object.color);
                  screen_painter.setBrush (brush);

                  screen_painter.drawChord (newx, newy, floating_object.height, floating_object.width, 
                                            floating_object.start_angle, floating_object.span_angle);
                }
              else
                {
                  pen.setColor (floating_object.color);
                  pen.setWidth (floating_object.line_width);
                  pen.setStyle (Qt::SolidLine);
                  screen_painter.setPen (pen);

                  screen_painter.drawArc (newx, newy, floating_object.width, floating_object.height, 
                                          floating_object.start_angle, floating_object.span_angle);
                }

              r.setLeft (newx);
              r.setWidth (floating_object.width);
              r.setTop (newy);
              r.setHeight (floating_object.height);

              floating_object.prev_rect.setCoords (r.normalized().x() - inc, r.normalized().y() - inc,
                                                   r.normalized().x() + r.normalized().width() + inc2,
                                                   r.normalized().y() + r.normalized().height() + inc2);

              break;


            case NVPIC_MOVE_PATH:

              //  Translate the center of the painter path to the provided x and y.

              r2 = floating_object.path.boundingRect ();

              screen_painter.translate (floating_object.x - (NV_INT32) (r2.width () / 2.0),
                                        floating_object.y - (NV_INT32) (r2.height () / 2.0));

              screen_painter.rotate (floating_object.rotation_angle);


              if (floating_object.filled)
                {
                  screen_painter.setPen (floating_object.color);
                  screen_painter.setBrush (floating_object.brush);

                  screen_painter.drawPath (floating_object.path);
                }
              else
                {
                  pen.setColor (floating_object.color);
                  pen.setWidth (floating_object.line_width);
                  pen.setStyle (Qt::SolidLine);
                  screen_painter.setPen (pen);

                  screen_painter.drawPath (floating_object.path);
                }

              r2 = floating_object.path.boundingRect ();

              floating_object.prev_rect.setCoords ((NV_INT32) r2.normalized().x() - inc, 
                                                   (NV_INT32) r2.normalized().y() - inc,
                                                   (NV_INT32) r2.normalized().x() +
                                                   (NV_INT32) r2.normalized().width() + inc2,
                                                   (NV_INT32) r2.normalized().y() +
                                                   (NV_INT32) r2.normalized().height() + inc2);


              screen_painter.rotate (-floating_object.rotation_angle);

              floating_object.prev_rect.translate (floating_object.x - (NV_INT32) (r2.width () / 2.0), 
                                                   floating_object.y - (NV_INT32) (r2.height () / 2.0));

              break;


            case NVPIC_RUBBER_POLYGON:
              pen.setColor (floating_object.color);
              pen.setWidth (floating_object.line_width);
              pen.setStyle (Qt::SolidLine);

              screen_painter.setPen (pen);

              poly = QPolygon (floating_object.polygon_count + 1);

              for (NV_INT32 i = 0 ; i < floating_object.polygon_count ; i++) 
                {
                  poly.setPoint (i, floating_object.polygon_x[i], floating_object.polygon_y[i]);
                }
              poly.setPoint (floating_object.polygon_count, rubberbandPolyLastX, rubberbandPolyLastY);

              if (floating_object.close_polygon)
                {
                  screen_painter.drawPolygon (poly);
                }
              else
                {
                  screen_painter.drawPolyline (poly);
                }

              r = poly.boundingRect ().normalized ();

              r.setLeft (r.left () - inc);
              r.setTop (r.top () - inc);
              r.setRight (r.right () + inc);
              r.setBottom (r.bottom () + inc);

              floating_object.prev_rect = r;
              break;


            case NVPIC_RUBBER_RECTANGLE:
              pen.setColor (floating_object.color);
              pen.setWidth (floating_object.line_width);
              pen.setStyle (Qt::SolidLine);

              screen_painter.setPen (pen);

              screen_painter.drawRect (floating_object.rect.normalized ());

              floating_object.prev_rect.setCoords (floating_object.rect.normalized().x() - inc,
                                                   floating_object.rect.normalized().y() - inc,
                                                   floating_object.rect.normalized().x() + 
                                                   floating_object.rect.normalized().width() + inc2,
                                                   floating_object.rect.normalized().y() +
                                                   floating_object.rect.normalized().height() + inc2);

              break;


            case NVPIC_RUBBER_LINE:
              pen.setColor (floating_object.color);
              pen.setWidth (floating_object.line_width);
              pen.setStyle (Qt::SolidLine);

              screen_painter.setPen (pen);

              screen_painter.drawLine (floating_object.x0, floating_object.y0, floating_object.x1, floating_object.y1);

              r.setCoords (floating_object.x0, floating_object.y0, floating_object.x1, floating_object.y1);

              floating_object.prev_rect.setCoords (r.normalized().x() - inc, r.normalized().y() - inc,
                                                   r.normalized().x() + r.normalized().width() + inc2,
                                                   r.normalized().y() + r.normalized().height() + inc2);

              break;
            }
        }
      else
        {
          floating_object.state = NVPIC_OBJECT_NOT_PRESENT;
        }
    }
  else


    //  Paint some portion (or all) of the off-screen pixmap to the screen.

    {
      screen_painter.drawPixmap (e->rect (), buffer, e->rect ());
    }


  screen_painter.end ();
}



//!  Convert pic coordinates to screen coordinates.

void 
nvPic::pic_to_screen (NV_INT32 num_points, NV_INT32 pic_x[], NV_INT32 pic_y[], NV_INT32 pix_x[], NV_INT32 pix_y[])
{
  for (NV_INT32 i = 0 ; i < num_points ; i++)
    {
      if (rotate)
        {
          NV_INT32 new_x, new_y;

          trueMatrix.map (pic_x[i], pic_y[i], &new_x, &new_y);

          pix_x[i] = NINT ((((NV_FLOAT32) (new_x - pic.bounds[pic.zoom_level].min_x) / xrange) * 
                            (NV_FLOAT32) pic.draw_width) * scale_min_x_scale);
          pix_y[i] = NINT ((((NV_FLOAT32) (new_y - pic.bounds[pic.zoom_level].min_y) / yrange) * 
                            (NV_FLOAT32) pic.draw_height) * scale_min_y_scale);
        }
      else
        {
          pix_x[i] = NINT (((NV_FLOAT32) (pic_x[i] - pic.bounds[pic.zoom_level].min_x) / xrange) * 
                           (NV_FLOAT32) pic.draw_width);
          pix_y[i] = NINT (((NV_FLOAT32) (pic_y[i] - pic.bounds[pic.zoom_level].min_y) / yrange) * 
                           (NV_FLOAT32) pic.draw_height);
        }
    }
}



//!  Convert screen coordinates to pic coordinates.

void 
nvPic::screen_to_pic (NV_INT32 num_points, NV_INT32 pic_x[], NV_INT32 pic_y[], NV_INT32 pix_x[], NV_INT32 pix_y[])
{
  for (NV_INT32 i = 0 ; i < num_points ; i++)
    {
      if (rotate)
        {
          NV_INT32 new_x, new_y;

          new_x = NINT ((pic.bounds[pic.zoom_level].min_x + ((NV_FLOAT32) pix_x[i] / (NV_FLOAT32) pic.draw_width) *
                         xrange) / scale_min_x_scale);
          new_y = NINT ((pic.bounds[pic.zoom_level].min_y + ((NV_FLOAT32) pix_y[i] / (NV_FLOAT32) pic.draw_height) * 
                         yrange) / scale_min_y_scale);

          invertedMatrix.map (new_x, new_y, &pic_x[i], &pic_y[i]);
        }
      else
        {
          pic_x[i] = NINT (pic.bounds[pic.zoom_level].min_x + 
                           ((NV_FLOAT32) pix_x[i] / (NV_FLOAT32) pic.draw_width) * xrange);
          pic_y[i] = NINT (pic.bounds[pic.zoom_level].min_y + 
                           ((NV_FLOAT32) pix_y[i] / (NV_FLOAT32) pic.draw_height) * yrange);
        }
    }
}



//!  Draw the pic.  Emit signals when needed.

void
nvPic::redrawPic ()
{
  floating_object.state = NVPIC_OBJECT_NOT_PRESENT;

  NV_INT32 w = pic.width;
  NV_INT32 h = pic.height;


  if (!pic_loaded) return;


  QPixmap *tmp = new QPixmap (orig_pixmap->copy (pic.bounds[pic.zoom_level].min_x, pic.bounds[pic.zoom_level].min_y, w, h));


  //  Make sure we're not playing with an empty pixmap.

  if (!tmp->width () || !tmp->height ()) return;


  delete scaled_pixmap;

  if (rotate)
    {
      matrix.reset ();
      matrix.rotate (rotation);


      QPixmap *ptmp = new QPixmap (tmp->transformed (matrix, pic.transMode));


      trueMatrix = ptmp->trueMatrix (matrix, tmp->width (), tmp->height ());

      invertedMatrix = trueMatrix.inverted ();


      //  X and Y ranges are different for rotated images (larger).

      xrange = (NV_FLOAT32) ptmp->width ();
      yrange = (NV_FLOAT32) ptmp->height ();


      scaled_pixmap = new QPixmap (ptmp->scaled (buffer.width (), buffer.height (), Qt::KeepAspectRatio, pic.transMode));


      delete ptmp;


      //  We have to figure out the Qt::KeepAspectRatio scale since it won't be the same as the 
      //  Qt::IgnoreAspectRatio scale we use for non-rotated images.

      scale_min_x_scale = (NV_FLOAT32) scaled_pixmap->width () / (NV_FLOAT32) pic.draw_width;
      scale_min_y_scale = (NV_FLOAT32) scaled_pixmap->height () / (NV_FLOAT32) pic.draw_height;
    }
  else
    {
      xrange = (NV_FLOAT32) pic.width;
      yrange = (NV_FLOAT32) pic.height;


      scaled_pixmap = new QPixmap (tmp->scaled (buffer.width (), buffer.height (), Qt::IgnoreAspectRatio, pic.transMode));
    }

  delete tmp;

  if (rotate) painter.eraseRect (0, 0, buffer.width (), buffer.height ());
  update ();
  pixmapPaint (0, 0, scaled_pixmap, 0, 0, scaled_pixmap->width (), scaled_pixmap->height ());


  //  Tell the parent it's time to draw.

  if (signalsEnabled) emit postRedrawSignal (pic);
}



//!  Brighten or darken the picture.

void
nvPic::brighten (NV_INT32 percentage)
{
  if (!pic_loaded) return;


  QImage *tmp = new QImage (orig_pixmap->toImage ());

  NV_INT32 total = tmp->width () * tmp->height ();
  NV_U_INT32 *pixels = (NV_U_INT32 *) tmp->scanLine (0);


  NV_FLOAT64 mult = (NV_FLOAT64) percentage * 0.01;

  if (mult < 0) mult = -mult;


  NV_U_CHAR lookup[256];

  if (percentage >= 0)
    {
      for (NV_INT32 i = 0 ; i < 256 ; i++)
        {
          NV_INT32 temp = qMin (255, (NV_INT32) (i * mult));
          lookup[i] = temp;
        }
    }
  else
    {
      for (NV_INT32 i = 0 ; i < 256 ; i++)
        {
          NV_INT32 temp = qMax (0, (NV_INT32) (i * mult));
          lookup[i] = temp;
        }
    }

  if (percentage >= 0)
    {
      for (NV_INT32 i = 0 ; i < total ; i++)
        {
          NV_INT32 r = qRed (pixels[i]);
          NV_INT32 g = qGreen (pixels[i]);
          NV_INT32 b = qBlue (pixels[i]);
          r = qMin (255, r + lookup[r]);
          g = qMin (255, g + lookup[g]);
          b = qMin (255, b + lookup[b]);
          pixels[i] = qRgb (r, g, b);
        }
    }
  else
    {
      for (NV_INT32 i = 0 ; i < total ; i++)
        {
          NV_INT32 r = qRed (pixels[i]);
          NV_INT32 g = qGreen (pixels[i]);
          NV_INT32 b = qBlue (pixels[i]);
          r = qMax (0, r - lookup[r]);
          g = qMax (0, g - lookup[g]);
          b = qMax (0, b - lookup[b]);
          pixels[i] = qRgb (r, g, b);
        }
    }

  delete orig_pixmap;
  orig_pixmap = new QPixmap (QPixmap::fromImage (*tmp));


  delete tmp;


  redrawPic ();
}


NV_BOOL
nvPic::OpenFile (NV_CHAR *filename)
{
  if (pic_loaded)
    {
      delete orig_pixmap;
      orig_pixmap = new QPixmap ();
    }

  if (!orig_pixmap->load (QString (filename))) return (NVFalse);

  pic.zoom_level = 0;
  pic.bounds[pic.zoom_level].min_x = pic.initial_bounds.min_x = 0;
  pic.bounds[pic.zoom_level].max_x = pic.initial_bounds.max_x = orig_pixmap->width ();
  pic.bounds[pic.zoom_level].min_y = pic.initial_bounds.min_y = 0;
  pic.bounds[pic.zoom_level].max_y = pic.initial_bounds.max_y = orig_pixmap->height ();
  pic.width = orig_pixmap->width ();
  pic.height = orig_pixmap->height ();

  pic_loaded = NVTrue;

  redrawPic ();

  return (NVTrue);
}




NV_BOOL 
nvPic::OpenData (NV_U_CHAR *data, NV_U_INT32 len)
{
  if (pic_loaded)
    {
      delete orig_pixmap;
      orig_pixmap = new QPixmap ();
    }

  if (!orig_pixmap->loadFromData (data, len)) return (NVFalse);

  pic.zoom_level = 0;
  pic.bounds[pic.zoom_level].min_x = pic.initial_bounds.min_x = 0;
  pic.bounds[pic.zoom_level].max_x = pic.initial_bounds.max_x = orig_pixmap->width ();
  pic.bounds[pic.zoom_level].min_y = pic.initial_bounds.min_y = 0;
  pic.bounds[pic.zoom_level].max_y = pic.initial_bounds.max_y = orig_pixmap->height ();
  pic.width = orig_pixmap->width ();
  pic.height = orig_pixmap->height ();

  pic_loaded = NVTrue;

  redrawPic ();

  return (NVTrue);
}




void
nvPic::SaveFile (QString filename, NV_BOOL scaled)
{
  if (!pic_loaded) return;


  if (scaled)
    {
      //  If we were in Qt::FastTransformation mode let's do a smooth transformation before saving.

      if (pic.transMode == Qt::FastTransformation)
        {
          pic.transMode = Qt::SmoothTransformation;
          redrawPic ();
          pic.transMode = Qt::FastTransformation;
        }


      buffer.save (filename, "JPEG");
    }
  else
    {
      orig_pixmap->save (filename, "JPEG");
    }
}




void 
nvPic::RotatePixmap (NV_FLOAT64 angle)
{
  rotation = angle;
  if (rotation != 0.0)
    {
      rotate = NVTrue;
    }
  else
    {
      rotate = NVFalse;
    }
}
