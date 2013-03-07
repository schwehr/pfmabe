#ifndef STARTPAGE_H
#define STARTPAGE_H


#include "trackLineDef.hpp"


class startPage:public QWizardPage
{
  Q_OBJECT 


public:

  startPage (QWidget *parent = 0, OPTIONS *options = NULL);


signals:


protected:

  OPTIONS               *options;

  QLineEdit             *outputFile;


protected slots:

  void slotOutputFileBrowse ();


private:
};

#endif
