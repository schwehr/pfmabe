#ifndef PREFS_H
#define PREFS_H


#include "pfm3DDef.hpp"


class Prefs:public QDialog
{
  Q_OBJECT 


public:

  Prefs (QWidget *parent = 0, OPTIONS *op = NULL, MISC *mi = NULL);
  ~Prefs ();


 signals:

  void dataChangedSignal ();


protected:

  OPTIONS         *options, mod_options;

  MISC            *misc, mod_misc;

  QButtonGroup    *bGrp, *cGrp;

  NV_BOOL         dataChanged;

  QSpinBox        *zoomPercent;

  QDoubleSpinBox  *exag, *fSize;

  QCheckBox       *scale;

  QPushButton     *bBackgroundColor, *bFeatureColor, *bFeatureInfoColor, *bHighlightColor, *bTrackerColor, *bScaleColor, *bRestoreDefaults;

  QPalette        bBackgroundPalette, bFeaturePalette, bFeatureInfoPalette, bHighlightPalette, bTrackerPalette, bScalePalette;


  void setFields ();


protected slots:

  void slotBackgroundColor ();
  void slotFeatureColor ();
  void slotFeatureInfoColor ();
  void slotHighlightColor ();
  void slotTrackerColor ();
  void slotScaleColor ();
  void slotRestoreDefaults ();
  void slotPositionClicked (int id);
  void slotApplyPrefs ();
  void slotClosePrefs ();
  void slotHelp ();


private:
};

#endif
