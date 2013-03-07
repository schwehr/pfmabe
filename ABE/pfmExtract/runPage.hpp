#ifndef RUNPAGE_H
#define RUNPAGE_H

#include "pfmExtract.hpp"


class runPage:public QWizardPage
{
  Q_OBJECT 


public:

  runPage (QWidget *parent = 0, RUN_PROGRESS *prog = NULL, QListWidget **eList = NULL);


signals:


protected:

  RUN_PROGRESS     *progress;

  QListWidget      *extractList;


protected slots:



private:
};

#endif
