#ifndef AREAINPUTPAGE_H
#define AREAINPUTPAGE_H


#include "chartsLASDef.hpp"


class areaInputPage:public QWizardPage
{
  Q_OBJECT 


public:

  areaInputPage (QWidget *parent = 0, QString *areaFilt = NULL, QTextEdit **areafiles = NULL);


signals:


protected:

  QDir             dir;

  QPushButton      *inputBrowse, *dirBrowse;

  QTextEdit        *inputFiles;

  QString          *inputFilter;

  QComboBox        *fileMask;


protected slots:

  void slotInputBrowseClicked ();
  void slotDirBrowseClicked ();
  void slotFileMaskTextChanged (const QString &text);


private:
};

#endif
