#ifndef MOSAICPAGE_H
#define MOSAICPAGE_H


#include "pfmChartsImageDef.hpp"


class mosaicPage:public QWizardPage
{
  Q_OBJECT 


public:

  mosaicPage (QWidget *parent = 0, OPTIONS *op = NULL);


signals:


protected:


  OPTIONS          *options;

  QDoubleSpinBox   *cellSpin, *timeSpin, *rollSpin, *pitchSpin, *headingSpin, *levelSpin, *focalSpin, *pixelSpin, *colSpin, *rowSpin;

  QLineEdit        *txt_file_edit;



protected slots:

  void slotTxtFileBrowse ();



private:
};

#endif
