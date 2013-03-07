#ifndef IMAGEPAGE_H
#define IMAGEPAGE_H


#include "pfmChartsImageDef.hpp"


class imagePage:public QWizardPage
{
  Q_OBJECT 


public:

  imagePage (QWidget *parent = 0, OPTIONS *op = NULL);


signals:


protected:


  OPTIONS          *options;

  QDoubleSpinBox   *datumSpin, *demSpin;

  QSpinBox         *intervalSpin, *lowSpin, *highSpin;

  QCheckBox        *casi_check, *opposite_check, *exclude_check, *normalize_check, *flip_check, *pos_check, *srtm_check, *hof_check, *tof_check, *both_check;

  QButtonGroup     *typeGrp;


protected slots:

  void slotExcludeClicked ();
  void slotOppositeClicked ();
  void slotLowSpinValueChanged (int value);
  void slotHighSpinValueChanged (int value);
  void slotSRTMClicked ();
  void slotNormalizeClicked ();
  void slotFlipClicked ();
  void slotPosClicked ();
  void slotEgmClicked ();
  void slotType (int id);


private:
};

#endif
