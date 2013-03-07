#ifndef EXTRACTPAGE_H
#define EXTRACTPAGE_H

#include "pfmChartsImage.hpp"


class extractPage:public QWizardPage
{
  Q_OBJECT 


public:

  extractPage (QWidget *parent = 0, RUN_PROGRESS *prog = NULL, QListWidget **cList = NULL);


signals:


protected:

  RUN_PROGRESS     *progress;

  QListWidget      *checkList;


protected slots:



private:
};

#endif
