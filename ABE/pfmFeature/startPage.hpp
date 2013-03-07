#ifndef STARTPAGE_H
#define STARTPAGE_H

#include "pfmFeatureDef.hpp"


class startPage:public QWizardPage
{
  Q_OBJECT 


public:

  startPage (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0, OPTIONS *op = NULL);


signals:


protected:

  OPTIONS          *options;

  QLineEdit        *pfm_file_edit, *output_file_edit, *area_file_edit, *ex_file_edit;


protected slots:

  void slotPFMFileBrowse ();
  void slotOutputFileBrowse ();
  void slotAreaFileBrowse ();
  void slotExFileBrowse ();


private:
};

#endif
