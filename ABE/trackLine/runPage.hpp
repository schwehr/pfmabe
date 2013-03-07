#ifndef RUNPAGE_H
#define RUNPAGE_H

#include "trackLine.hpp"


class runPage:public QWizardPage
{
  Q_OBJECT 


public:

  runPage (QWidget *parent = 0, RUN_PROGRESS *prog = NULL, QListWidget **cList = NULL);


signals:


protected:

  RUN_PROGRESS     *progress;

  QListWidget      *trackList;


protected slots:



private:
};

#endif
