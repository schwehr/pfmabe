/*  ChartPic class definitions.  */

#ifndef FEATUREPIC_H
#define FEATUREPIC_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <cmath>

#include "nvutility.h"
#include "nvutility.hpp"

#include "binaryFeatureData.h"

#include "version.hpp"

#include <QtCore>
#include <QtGui>


class featurePic:public QMainWindow
{
  Q_OBJECT 


public:

  featurePic (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~featurePic ();

  void redrawPic ();


protected:

  nvPic           *pic;

  NV_INT32        window_x, window_y, width, height;

  NVPIC_DEF       picdef;

  QToolButton     *bQuit;


  void envin ();
  void envout ();


protected slots:

  void slotQuit ();
  void slotBrighten ();
  void slotDarken ();

  void about ();
  void slotAcknowledgements ();
  void aboutQt ();

 private:
};

#endif
