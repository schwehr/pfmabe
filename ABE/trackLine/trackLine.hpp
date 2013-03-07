#ifndef TRACKLINE_H
#define TRACKLINE_H

#include "trackLineDef.hpp"
#include "startPage.hpp"
#include "optionsPage.hpp"
#include "inputPage.hpp"
#include "runPage.hpp"
#include "version.hpp"


class trackLine : public QWizard
{
  Q_OBJECT


public:

  trackLine (QWidget *parent = 0);


protected:

  void initializePage (int id);
  void cleanupPage (int id);


  OPTIONS          options;

  QStringList      input_files;

  QTextEdit        *inputFiles;

  QListWidget      *trackList;

  NV_CHAR          *input_filenames[8000];

  NV_FLOAT64       start_heading;

  NV_INT32         type;

  RUN_PROGRESS     progress;



protected slots:

  void slotCustomButtonClicked (int id);
  void slotHelpClicked ();
  NV_INT32 heading_change (INFO *data);
  NV_BOOL changepos (INFO *prev_data, INFO *data);

};

#endif
