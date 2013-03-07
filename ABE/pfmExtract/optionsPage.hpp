#ifndef OPTIONSPAGE_H
#define OPTIONSPAGE_H


#include "pfmExtractDef.hpp"


class optionsPage:public QWizardPage
{
  Q_OBJECT 


public:

  optionsPage (QWidget *parent = 0, OPTIONS *op = NULL);


signals:


protected:

  OPTIONS          *options;

  QGroupBox        *utmBox, *landBox, *uncBox;

  QCheckBox        *geoid, *reference, *checked, *flip, *utm, *land, *uncert, *cut;

  QDoubleSpinBox   *cutoff, *datumShift;

  QComboBox        *format, *size;

  QRadioButton     *minFilt, *maxFilt, *avgFilt, *allData;


protected slots:

  void slotSourceClicked (int id);
  void slotFormatChanged (int id);
  void slotCutChanged (int state);
  void slotHelp ();

private:
};

#endif
