#ifndef EDITFEATURE_H
#define EDITFEATURE_H

#include "pfm.h"
#include "pfm_extras.h"
#include "mosaicViewDef.hpp"
#include "screenshot.hpp"


class editFeature:public QDialog
{
  Q_OBJECT 


public:

  editFeature (QWidget *pt = 0, OPTIONS *op = NULL, MISC *mi = NULL, NV_INT32 num = 0);
  ~editFeature ();


  signals:

  void dataChangedSignal ();
  void definePolygonSignal ();


protected:

  OPTIONS         *options;

  MISC            *misc;

  QWidget         *parent;

  BFDATA_RECORD   bfd_record;

  BFDATA_POLYGON  bfd_polygon;

  NV_CHAR         image_name[512];

  NV_INT32        current_feature, window_x, window_y, window_width, window_height;

  NV_BOOL         new_feature, assigned_image;

  QDoubleSpinBox  *orientation, *length, *width;

  QSpinBox        *confidence;

  QLineEdit       *remarks, *description;

  QListWidget     *rockBox, *offshoreBox, *lightBox, *lidarBox;

  QDialog         *editFeatureD, *rockD, *offshoreD, *lightD, *lidarD;

  QPushButton     *bView, *bAssign, *bScreen, *bPolygon, *bDelete, *bGroup, *bUngroup;

  QPixmap         helpIcon;

  QTimer          *polyTimer;

  QProcess        *imageProc;

  screenshot      *screen_dialog;


protected slots:

  void slotHelp ();

  void slotSnippetViewerReadyReadStandardError ();
  void slotViewError (QProcess::ProcessError error);
  void slotViewSnippet ();
  void slotAssignSnippet ();
  void slotScreenshotSaved (QString filename);
  void slotScreenshotQuit ();
  void slotScreen ();
  void slotHelpEditFeature ();
  void slotApplyEditFeature ();
  void slotCancelEditFeature ();

  void slotPolygon ();
  void slotPolyTimer ();

  void slotDelete ();
  void slotGroup ();
  void slotUngroup ();

  void slotRocks ();
  void slotRockOKClicked ();
  void slotRockCancelClicked ();

  void slotOffshore ();
  void slotOffshoreOKClicked ();
  void slotOffshoreCancelClicked ();

  void slotLights ();
  void slotLightOKClicked ();
  void slotLightCancelClicked ();

  void slotLidar ();
  void slotLidarOKClicked ();
  void slotLidarCancelClicked ();


 private:
};

#endif
