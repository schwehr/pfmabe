#ifndef CLASSPAGE_H
#define CLASSPAGE_H


#include "pfmBagDef.hpp"


class classPage:public QWizardPage
{
  Q_OBJECT 


public:

  classPage (QWidget *parent = 0, OPTIONS *op = NULL);


signals:


protected:

  OPTIONS          *options;

  QComboBox        *classification, *authority, *distBox;

  QDateEdit        *declassDate, *compDate;

  QTextEdit        *distStatement;



protected slots:

  void slotDistBoxActivated (int index);


private:
};

#endif
