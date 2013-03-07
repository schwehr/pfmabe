#ifndef PREFS_H
#define PREFS_H


#include "mosaicViewDef.hpp"


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

  QLineEdit       *featureDiameter, *textSearch;

  QPushButton     *bMarkerColor, *bRectColor, *bFeatureColor, *bFeatureInfoColor, *bFeaturePolyColor, *bCoastColor,
                  *bLandmaskColor, *bHighlightColor, *bRestoreDefaults;

  QPalette        bMarkerPalette, bRectPalette, bFeaturePalette, bFeatureInfoPalette, bFeaturePolyPalette,
                  bCoastPalette, bLandmaskPalette, bHighlightPalette;

  QCheckBox       *sMessage;


  void setFields ();


protected slots:

  void slotMarkerColor ();
  void slotCoastColor ();
  void slotLandmaskColor ();
  void slotRectColor ();
  void slotFeatureColor ();
  void slotFeatureInfoColor ();
  void slotFeaturePolyColor ();
  void slotHighlightColor ();
  void slotRestoreDefaults ();
  void slotPositionClicked (int id);
  void slotApplyPrefs ();
  void slotClosePrefs ();
  void slotHelp ();


private:
};

#endif
