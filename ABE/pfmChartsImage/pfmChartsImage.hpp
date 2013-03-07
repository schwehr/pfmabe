#ifndef PFMCHARTSIMAGE_H
#define PFMCHARTSIMAGE_H

#include "pfmChartsImageDef.hpp"
#include "startPage.hpp"
#include "imagePage.hpp"
#include "mosaicPage.hpp"
#include "extractPage.hpp"
#include "inputPage.hpp"
#include "runPage.hpp"


class pfmChartsImage : public QWizard
{
  Q_OBJECT


public:

  pfmChartsImage (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~pfmChartsImage ();


protected:

  void initializePage (int id);
  void cleanupPage (int id);
  bool validateCurrentPage ();

  NV_INT32 get_images (NV_CHAR *path, NV_INT32 type, char *dir, NV_FLOAT64 *polygon_x, NV_FLOAT64 *polygon_y, NV_INT32 polygon_count, NV_INT32 start,
                       NV_INT32 end, FILE *txt_fp, NV_CHAR *areafile);

  NV_BOOL misp (NV_FLOAT64 bin_size_meters, NV_CHAR *dem_file, NV_CHAR *casi_file, NV_INT32 pfm_start_x, NV_INT32 pfm_start_y, NV_INT32 pfm_width,
                NV_INT32 pfm_height, NV_FLOAT64 *level_x, NV_FLOAT64 *level_y, NV_INT32 level_count, NV_FLOAT64 level, LIST_NUM *list,
                NV_FLOAT32 datum_offset, NV_BOOL flip, NV_BOOL casi);

  void envin (OPTIONS *options);
  void envout (OPTIONS *options);



  OPTIONS          options;

  RUN_PROGRESS     progress;

  PFM_OPEN_ARGS    open_args;

  inputPage        *iPage;

  runPage          *rPage;

  QString          pfm_file_name, area_file_name, image_dir_name, level_file_name, out_file_name, tmp_file_name;

  QProcess         *mosaicProc;

  QListWidget      *eCheckList;

  NV_INT32         zone;

  projPJ           pj_utm, pj_latlon;

  NV_FLOAT64       min_ne_x, min_ne_y, max_ne_x, max_ne_y;

  NV_INT32         n_blocks, icount, run_counter;

  NV_CHAR          txt_file[512], pars_file[512], dir[512], areafile[512];

  NV_INT32         polygon_count;

  NV_FLOAT64       polygon_x[200], polygon_y[200];

  NV_BOOL          restricted, writer_stat;


protected slots:

  void slotHelpClicked ();
  void slotCustomButtonClicked (int id);

  void slotMosaicReadyReadStandardError ();
  void slotMosaicReadyReadStandardOutput ();
  void slotMosaicError (QProcess::ProcessError error);
  void slotMosaicDone (int exitCode, QProcess::ExitStatus exitStatus);

};

#endif
