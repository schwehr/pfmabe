
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



#ifndef SCALEBOX_H
#define SCALEBOX_H


#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "nvutility.h"
#include "nvutility.hpp"


#include <QtCore>
#include <QtGui>


class scaleBox:public QWidget
{
  Q_OBJECT 


public:

  scaleBox (QWidget *parent = 0, QColor clr = Qt::white, NV_FLOAT32 val = 99999.99);
  ~scaleBox ();
  void setContents (QColor clr, NV_FLOAT32 val);
  void setContents (QColor clr, NV_FLOAT32 val, NV_INT32 lckd);


signals:

  void mouseDoubleClickSignal (QMouseEvent *e);
  void mousePressSignal (QMouseEvent *e);
  void mouseReleaseSignal (QMouseEvent *e);


protected:

  QColor          color;

  NV_FLOAT32      value;

  NV_INT32        locked;

  QPixmap         lockPixmap, unlockPixmap;


protected slots:


private:

  void paintEvent (QPaintEvent *e);
  void mouseDoubleClickEvent (QMouseEvent *e);
  void mousePressEvent (QMouseEvent *e);
  void mouseReleaseEvent (QMouseEvent *e);

};

#endif
