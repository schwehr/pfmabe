#ifndef FILEINPUTPAGE_H
#define FILEINPUTPAGE_H


#include "chartsLASDef.hpp"


class fileInputPage:public QWizardPage
{
  Q_OBJECT 


public:

  fileInputPage (QWidget *parent = 0, QString *inFilt = NULL, QTextEdit **infiles = NULL);


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
