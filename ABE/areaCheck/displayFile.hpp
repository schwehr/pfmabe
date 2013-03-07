#ifndef DISPLAYFILE_H
#define DISPLAYFILE_H

#include "areaCheckDef.hpp"


class displayFile:public QDialog
{
  Q_OBJECT 


    public:

  displayFile (QWidget *parent = 0, OPTIONS *op = NULL, MISC *mi = NULL, NV_INT32 type = 0);
  ~displayFile ();


 signals:

  void dataChangedSignal ();
  void displayFileDialogClosedSignal (int k);


 protected:

  OPTIONS         *options;

  MISC            *misc;

  QTableWidget    *displayTable;

  QTableWidgetItem **vItem;

  QCheckBox       **displayed;

  QButtonGroup    *displayGrp;

  NV_INT32        count;

  NV_INT32        k;

  NV_BOOL         *origStatus, *newStatus;


  protected slots:

  void slotHelp ();
  void slotValueChanged (int row);
  void slotApply ();
  void slotClose ();

 private:
};

#endif
