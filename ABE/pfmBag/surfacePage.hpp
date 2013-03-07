#ifndef SURFACEPAGE_H
#define SURFACEPAGE_H


#include "pfmBagDef.hpp"


class surfacePage:public QWizardPage
{
  Q_OBJECT 


public:

  surfacePage (QWidget *parent = 0, OPTIONS *op = NULL);

  void setFields (OPTIONS *options);



signals:



protected:

  OPTIONS          *options;

  QComboBox        *surface, *uncertainty;

  QCheckBox        *feature;

  QDoubleSpinBox   *mBinSize, *gBinSize;

  QLineEdit        *title, *individualName, *positionName, *individualName2, *positionName2;

  QTextEdit        *abstract;

  QFormLayout      *formLayout;

  NV_FLOAT64       prev_gbin, prev_mbin, save_mbin;

  NV_BOOL          cube;


protected slots:

  void slotFeatureClicked ();
  void slotSurfaceChanged (int index);
  void slotMBinSizeChanged (double value);
  void slotGBinSizeChanged (double value);



private:
};

#endif
