#ifndef __SCREENSHOT_H__
#define __SCREENSHOT_H__

#include "mosaicViewDef.hpp"


class screenshot:public QDialog
{
  Q_OBJECT 


public:

  screenshot (QWidget *parent = 0, OPTIONS *op = NULL, MISC *mi = NULL, NV_INT32 ct = 0);
  ~screenshot ();



signals:

  void screenshotSavedSignal (QString filename);
  void screenshotQuit ();


protected:

  OPTIONS         *options;

  MISC            *misc;

  NV_INT32        current_feature;

  QPixmap         screenPixmap;

  QLabel          *screenshotLabel;

  QSpinBox        *timeDelay;


  void resizeEvent (QResizeEvent *event);



protected slots:

  void slotNewScreenshot ();
  void slotSaveScreenshot ();
  void grab ();
  void slotQuit ();
  void slotHelp ();



private:
};

#endif
