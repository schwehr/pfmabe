#ifndef DATUMPAGE_H
#define DATUMPAGE_H


#include "pfmBagDef.hpp"


class datumPage:public QWizardPage
{
  Q_OBJECT 


public:

  datumPage (QWidget *parent = 0, OPTIONS *op = NULL);


signals:


protected:


  OPTIONS          *options;

  QComboBox        *units, *depthCor, *projection, *hDatum, *vDatum;

  QLineEdit        *group, *source, *version;



protected slots:

  void slotHDatumActivated (int index);
  void slotVDatumActivated (int index);


private:
};

#endif
