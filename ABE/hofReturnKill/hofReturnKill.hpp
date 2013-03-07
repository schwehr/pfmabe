#ifndef HOFRETURNKILL_H
#define HOFRETURNKILL_H

#include "hofReturnKillDef.hpp"
#include "version.hpp"


class hofReturnKill : QObject
{
  Q_OBJECT


public:

  hofReturnKill (NV_INT32 argc = 0, NV_CHAR **argv = NULL);
  ~hofReturnKill ();


protected:

  void usage ();


  ABE_SHARE       *abe_share;

  QSharedMemory   *abeShare;

  NV_BOOL         killed;


protected slots:


};

#endif
