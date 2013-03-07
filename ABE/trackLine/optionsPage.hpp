#ifndef OPTIONSPAGE_H
#define OPTIONSPAGE_H


#include "trackLineDef.hpp"


class optionsPage:public QWizardPage
{
  Q_OBJECT 


public:

  optionsPage (QWidget *parent = 0, OPTIONS *op = NULL);


signals:


protected:

  OPTIONS          *options;

  QDoubleSpinBox   *headingGSF, *speedGSF, *headingPOS, *speedPOS, *headingWLF, *speedWLF, *headingHWK, *speedHWK;


protected slots:


private:
};

#endif
