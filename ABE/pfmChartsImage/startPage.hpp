#ifndef STARTPAGE_H
#define STARTPAGE_H

#include "pfmChartsImageDef.hpp"


class startPage:public QWizardPage
{
  Q_OBJECT 


public:

  startPage (QWidget *parent = 0, PFM_OPEN_ARGS *oa = NULL, OPTIONS *op = NULL);


signals:


protected:

  PFM_OPEN_ARGS    *open_args;

  OPTIONS          *options;

  QLineEdit        *pfm_file_edit, *area_file_edit, *image_dir_edit, *level_file_edit;

  QCheckBox        *remove_check;

  QDoubleSpinBox   *levelSpin;


protected slots:

  void slotPFMFileBrowse ();
  void slotAreaFileBrowse ();
  void slotLevelFileBrowse ();
  void slotRemoveClicked ();


private:
};

#endif
