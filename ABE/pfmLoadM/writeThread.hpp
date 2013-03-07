#ifndef WRITETHREAD_H
#define WRITETHREAD_H


#include "pfmLoadMDef.hpp"


class writeThread:public QThread
{
  Q_OBJECT 


public:

  writeThread (QObject *parent = 0);
  ~writeThread ();

  void write (NV_INT32 a_pass = 0, PFM_LOAD_PARAMETERS *a_load_parms = NULL, PFM_DEFINITION *a_pfm_def = NULL,
              QMutex *pfmMutex = NULL, QSemaphore **a_freeBuffers = NULL, QSemaphore **a_usedBuffers = NULL,
              TRANSFER *a_transfer = NULL, FILE *a_errfp = NULL, NV_BOOL *a_read_done = NULL, THREAD_DATA *a_thd = NULL);

signals:

void complete (NV_INT32 out_count, NV_INT32 out_of_limits, NV_INT32 pass);


protected:


  QMutex                   mutex;

  NV_INT32                 l_pass;
  PFM_LOAD_PARAMETERS      *l_load_parms;
  PFM_DEFINITION           *l_pfm_def;
  QMutex                   *l_pfmMutex;
  QSemaphore               **l_freeBuffers;
  QSemaphore               **l_usedBuffers;
  TRANSFER                 *l_transfer;
  FILE                     *l_errfp;
  NV_BOOL                  *l_read_done;
  THREAD_DATA              *l_thd;

  NV_INT32                 pass;
  PFM_LOAD_PARAMETERS      *load_parms;
  PFM_DEFINITION           *pfm_def;
  QMutex                   *pfmMutex;
  QSemaphore               **freeBuffers;
  QSemaphore               **usedBuffers;
  TRANSFER                 *transfer;
  FILE                     *errfp;
  NV_BOOL                  *read_done;
  THREAD_DATA              *thd;


  NV_INT32                 index[MAX_PFM_FILES];
  NV_INT32                 out_count[MAX_PFM_FILES];
  NV_INT32                 out_of_limits[MAX_PFM_FILES];


  void run ();


protected slots:

private:
};

#endif
