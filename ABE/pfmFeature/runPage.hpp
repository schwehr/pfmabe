#ifndef RUNPAGE_H
#define RUNPAGE_H

#include "pfmFeature.hpp"


class runPage:public QWizardPage
{
  Q_OBJECT 


public:

  runPage (QWidget *parent = 0, RUN_PROGRESS *prog = NULL);


signals:


protected:

  RUN_PROGRESS     *progress;


protected slots:



private:
};

#endif
