#ifndef ANCILLARYPROG_H
#define ANCILLARYPROG_H

#include "geoSwath3DDef.hpp"


void set_defaults (MISC *misc, OPTIONS *options, POINT_DATA *data, NV_BOOL restore);


class ancillaryProg:public QProcess
{
  Q_OBJECT 


public:

  ancillaryProg (QWidget *parent = 0, nvMapGL *ma = NULL, POINT_DATA *sw = NULL, OPTIONS *op = NULL,
                 MISC *mi = NULL, NV_INT32 prog = 0, NV_INT32 lock_point = 0);
  ~ancillaryProg ();


signals:

  void redrawSignal ();


protected:

  OPTIONS         *options;

  MISC            *misc;

  POINT_DATA      *data;

  nvMapGL         *map;

  NV_BOOL         command_line, command_line_up;

  QString         cmd;

  QStringList     args;

  QWidget         *pa;

  QDialog         *commandLineD;

  QTextEdit       *commandText;

  QTimer          *appTimer;

  NV_INT32        appTimerCount;


protected slots:

  void slotDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotError (QProcess::ProcessError error);
  void slotReadyReadStandardError ();
  void slotReadyReadStandardOutput ();
  void slotAppTimer ();
  void slotHelp ();
  void slotCloseCommand ();

private:
};

#endif
