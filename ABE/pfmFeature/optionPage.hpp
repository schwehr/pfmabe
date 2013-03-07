#ifndef OPTIONPAGE_H
#define OPTIONPAGE_H


#include "pfmFeatureDef.hpp"


class optionPage:public QWizardPage
{
  Q_OBJECT 


public:

  optionPage (QWidget *parent = 0, OPTIONS *op = NULL);
  void setFiles (QString pfm_file, QString area_file);


signals:



protected:


  OPTIONS          *options;

  QComboBox        *order;

  QCheckBox        *zero, *hpc;

  QLineEdit        *description;

  QListWidget      *rockBox, *offshoreBox, *lightBox;

  QDialog          *rockD, *offshoreD, *lightD;

  QString          pfmFile, areaFile;

  QDoubleSpinBox   *offset;



protected slots:

  void slotRocks ();
  void slotRockOKClicked ();
  void slotRockCancelClicked ();

  void slotOffshore ();
  void slotOffshoreOKClicked ();
  void slotOffshoreCancelClicked ();

  void slotLights ();
  void slotLightOKClicked ();
  void slotLightCancelClicked ();

  void slotEgmClicked ();

  void slotHelp ();


private:
};

#endif
