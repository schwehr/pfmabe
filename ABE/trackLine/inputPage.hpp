#ifndef INPUTPAGE_H
#define INPUTPAGE_H


#include "trackLineDef.hpp"


class inputPage:public QWizardPage
{
  Q_OBJECT 


public:

  inputPage (QWidget *parent = 0, OPTIONS *op = NULL, QTextEdit **infiles = NULL);


signals:


protected:

  OPTIONS          *options;

  QDir             dir;

  QPushButton      *inputBrowse, *dirBrowse;

  QTextEdit        *inputFiles;

  QComboBox        *fileMask;


protected slots:

  void slotInputBrowseClicked ();
  void slotDirBrowseClicked ();
  void slotFileMaskTextChanged (const QString &text);


private:
};

#endif
