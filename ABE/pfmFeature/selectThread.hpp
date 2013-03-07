#ifndef SELECTTHREAD_H
#define SELECTTHREAD_H


#include "pfmFeatureDef.hpp"


class selectThread:public QThread
{
  Q_OBJECT 


public:

  selectThread (QObject *parent = 0);
  ~selectThread ();

  void select (OPTIONS *op = NULL, NV_INT32 ph = -1, PFM_OPEN_ARGS *oa = NULL, FEATURE **f = NULL, NV_INT32 pass = 0, NV_FLOAT64 ss = 0.0);


signals:

  void percentValue (NV_INT32 percent, NV_INT32 pass);
  void completed (NV_INT32 num_features, NV_INT32 pass);


protected:


  QMutex           mutex;

  OPTIONS          *l_options;

  PFM_OPEN_ARGS    l_open_args;

  FEATURE          **l_features;

  NV_INT32         l_pass, l_pfm_handle;

  NV_FLOAT64       l_start_size;

  NV_BOOL          offset;

  void             run ();
  NV_U_BYTE        get_octant (NV_FLOAT64 x0, NV_FLOAT64 y0, NV_FLOAT64 x1, NV_FLOAT64 y1);


protected slots:

private:
};

#endif
