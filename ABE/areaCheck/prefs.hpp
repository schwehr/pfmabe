#ifndef PREFS_H
#define PREFS_H

#include "pfm.h"
#include "areaCheckDef.hpp"


class Prefs:public QDialog
{
  Q_OBJECT 


    public:

  Prefs (QWidget *parent = 0, OPTIONS *op = NULL, MISC *mi = NULL);
  ~Prefs ();


 signals:

  void dataChangedSignal (NV_BOOL mapRedrawFlag);


 protected:

  OPTIONS         *options, mod_options;

  MISC            *misc;

  QButtonGroup    *bGrp, *colorGrp;

  QPalette        bCoastPalette, bMaskPalette, bBackgroundPalette, bHighlightPalette, c2Palette[NUM_TYPES];

  QPushButton     *bCoastColor, *bMaskColor, *bBackgroundColor, *bHighlightColor, *bRestoreDefaults, *c2[NUM_TYPES];

  QDoubleSpinBox  *sunEx, *sunAz, *sunEl;

  QCheckBox       *stopLight;

  QLineEdit       *minStop, *maxStop;

  NV_BOOL         dataChanged;


  void setFields ();


  protected slots:

  void slotCoastColor ();
  void slotMaskColor ();
  void slotBackgroundColor ();
  void slotColor (int id);
  void slotHighlightColor ();
  void slotHelp ();
  void slotPositionClicked (int id);
  void slotStopLightClicked ();
  void slotRestoreDefaults ();
  void slotApplyPrefs ();
  void slotClosePrefs ();

 private:
};

#endif
