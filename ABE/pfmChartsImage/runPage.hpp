#ifndef RUNPAGE_H
#define RUNPAGE_H

#include "pfmChartsImageDef.hpp"


class runPage:public QWizardPage
{
  Q_OBJECT 


public:

  runPage (QWidget *parent = 0,  OPTIONS *op = NULL, RUN_PROGRESS *prog = NULL,QListWidget **cList = NULL);

  void setOutFileName (QString name);



signals:


protected:

  OPTIONS          *options;

  RUN_PROGRESS     *progress;

  QListWidget      *checkList;

  QLineEdit        *out_file_edit;

  QCheckBox        *utm_check, *geo_check, *caris_check;

  QButtonGroup     *outGrp;



protected slots:

  void slotOutFileBrowse ();
  void slotOut (int id);
  void slotCarisClicked ();



private:
};

#endif
