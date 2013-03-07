#ifndef HOTKEYPOLYGON_H
#define HOTKEYPOLYGON_H

#include "geoSwath3DDef.hpp"
#include "sharedFile.h"


class hotkeyPolygon:public QProcess
{
  Q_OBJECT 


public:

  hotkeyPolygon (QWidget *parent = 0, nvMapGL *ma = NULL, POINT_DATA *da = NULL, OPTIONS *op = NULL,
                 MISC *mi = NULL, NV_INT32 prog = 0, NV_BOOL *failed = NULL);
  ~hotkeyPolygon ();

signals:

  void hotkeyPolygonDone ();


protected:

  OPTIONS         *options;

  MISC            *misc;

  POINT_DATA      *data;

  nvMapGL         *map;

  NV_INT32        shared_count, inc;

  QString         cmd;

  QStringList     args;

  QWidget         *pa;


protected slots:

  void slotDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotError (QProcess::ProcessError error);
  void slotReadyReadStandardError ();
  void slotReadyReadStandardOutput ();


private:
};

#endif
