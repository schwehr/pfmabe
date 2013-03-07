#ifndef STARTPAGE_H
#define STARTPAGE_H

#include "chartsLASDef.hpp"


class startPage:public QWizardPage
{
  Q_OBJECT 


public:

  startPage (QWidget *parent = 0, NV_BOOL g03 = NVFalse, NV_INT32 dtm = 0, QString fpre = "");


signals:


protected:

  QCheckBox        *geoid;

  QButtonGroup     *bGrp;

  QString          outputDir, filePrefix;

  QLineEdit        *out_edit, *pre_edit;

  QPushButton      *outBrowse;

  QComboBox        *hDatum;

  NV_BOOL          geoid03;

  NV_INT32         datum;


protected slots:

  void slotOutBrowseClicked ();


private:
};

#endif
