#ifndef HOFWAVEFILTER_H
#define HOFWAVEFILTER_H

#include "hofWaveFilterDef.hpp"
#include "version.hpp"


class hofWaveFilter : QObject
{
  Q_OBJECT


public:

  hofWaveFilter (NV_INT32 argc = 0, NV_CHAR **argv = NULL);
  ~hofWaveFilter ();


protected:

  void usage ();


  MISC            misc;

  WAVE_DATA       *wave_data;


protected slots:


};

#endif
