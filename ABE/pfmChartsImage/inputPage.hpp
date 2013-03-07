#ifndef INPUTPAGE_H
#define INPUTPAGE_H


#include "pfmChartsImageDef.hpp"


class inputPage:public QWizardPage
{
  Q_OBJECT 


public:

  inputPage (QWidget *parent = 0);

  void initializeTable (NV_INT32 icount, QString dir, QString txt_file);

  NV_BOOL getCheckBox (NV_INT32 row);


signals:


protected:

  QTableWidget             *imageTable;

  NV_CHAR                  data_dir[512], text_file[512];

  FILE                     *tfp;

  NV_INT32                 count;

  QTableWidgetItem         **vItem;

  QCheckBox                **imaged;

  QStringList              filenames;

  QProcess                 *imageProc;

  NV_BOOL                  killFlag;

  QButtonGroup             *displayGrp, *imageGrp;

  QList<int>               rng;


protected slots:

  void slotDisplay (int row);
  void slotViewError (QProcess::ProcessError error);
  void slotValueChanged (int row);


private:
};

#endif
