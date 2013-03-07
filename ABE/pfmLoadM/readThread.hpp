#ifndef READTHREAD_H
#define READTHREAD_H


#include "pfmLoadMDef.hpp"


class readThread:public QThread
{
  Q_OBJECT 


public:

  readThread (QObject *parent = 0);
  ~readThread ();

  void read (NV_INT32 a_pass = 0, THREAD_DATA *a_thd = NULL, NV_INT32 a_shared_file_start = 0,
             NV_INT32 a_shared_file_end = 0, FILE_DEFINITION *a_input_file_def = NULL, PFM_LOAD_PARAMETERS *a_load_parms = NULL,
             PFM_DEFINITION *a_pfm_def = NULL, QSemaphore **a_freeBuffers = NULL, QSemaphore **a_usedBuffers = NULL,
             TRANSFER *a_transfer = NULL, FILE *a_errfp = NULL, NV_INT32 *a_out_of_range = NULL, NV_INT32 *a_count = NULL,
             QMutex *a_pfmMutex = NULL, QMutex *a_transMutex = NULL, NV_BOOL *a_read_done = NULL);


signals:

void percentValue (NV_INT32 percent, NV_INT32 file_num, NV_INT32 pass);


protected:


  QMutex                   mutex;

  NV_INT32                 l_pass;
  THREAD_DATA              *l_thd;
  NV_INT32                 l_shared_file_start;
  NV_INT32                 l_shared_file_end;
  FILE_DEFINITION          *l_input_file_def;
  PFM_LOAD_PARAMETERS      *l_load_parms;
  PFM_DEFINITION           *l_pfm_def;
  QSemaphore               **l_freeBuffers;
  QSemaphore               **l_usedBuffers;
  TRANSFER                 *l_transfer;
  FILE                     *l_errfp;
  NV_INT32                 *l_out_of_range;
  NV_INT32                 *l_count;
  QMutex                   *l_pfmMutex;
  QMutex                   *l_transMutex;
  NV_BOOL                  *l_read_done;


  NV_INT32                 pass;
  THREAD_DATA              *thd;
  NV_INT32                 shared_file_start;
  NV_INT32                 shared_file_end;
  FILE_DEFINITION          *input_file_def;
  PFM_LOAD_PARAMETERS      *load_parms;
  PFM_DEFINITION           *pfm_def;
  QSemaphore               **freeBuffers;
  QSemaphore               **usedBuffers;
  TRANSFER                 *transfer;
  FILE                     *errfp;
  NV_INT32                 *out_of_range;
  NV_INT32                 *count;
  QMutex                   *pfmMutex;
  QMutex                   *transMutex;
  NV_BOOL                  *read_done;


  NV_INT16                 file_number[MAX_PFM_FILES];
  NV_INT32                 line_number[MAX_PFM_FILES];
  NV_INT32                 recnum;
  NV_INT32                 pfm_file_count;
  NV_FLOAT32               attr[NUM_ATTR];
  NV_CHAR                  path[1024];
  NV_CHAR                  line[1024];
  NV_CHAR                  line_path[1024];
  NV_CHAR                  native_path[1024];
  QString                  nativePath;



  void run ();

  NV_INT32 getArea (NV_INT32 pfm, NV_FLOAT64 lat, NV_FLOAT64 lon);
  void GSF_PFM_Processing (gsfRecords *gsf_records);
  NV_INT32 load_gsf_file (NV_INT32 pfm_fc);
  NV_INT32 load_hof_file (NV_INT32 pfm_fc);
  NV_INT32 load_tof_file (NV_INT32 pfm_fc);
  NV_INT32 load_wlf_file (NV_INT32 pfm_fc);
  NV_INT32 load_hawkeye_file (NV_INT32 pfm_fc);
  NV_INT32 load_unisips_depth_file (NV_INT32 pfm_fc);
  NV_INT32 load_hypack_xyz_file (NV_INT32 pfm_fc);
  NV_INT32 load_ivs_xyz_file (NV_INT32 pfm_fc);
  NV_INT32 load_llz_file (NV_INT32 pfm_fc);
  NV_INT32 load_czmil_file (NV_INT32 pfm_fc);
  NV_INT32 load_srtm_file (NV_INT32 pfm_fc);
  NV_INT32 load_dted_file (NV_INT32 pfm_fc);
  NV_INT32 load_chrtr_file (NV_INT32 pfm_fc);
  NV_INT32 load_bag_file (NV_INT32 pfm_fc);



protected slots:

private:
};

#endif
