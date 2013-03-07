#ifndef PFMFEATURE_H
#define PFMFEATURE_H

#include "pfmFeatureDef.hpp"
#include "startPage.hpp"
#include "optionPage.hpp"
#include "runPage.hpp"
#include "selectThread.hpp"


class pfmFeature : public QWizard
{
  Q_OBJECT


public:

  pfmFeature (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~pfmFeature ();


protected:

  void initializePage (int id);
  void cleanupPage (int id);

  void load_arrays (NV_INT32 layer_type, NV_INT32 count, BIN_RECORD bin_record[], NV_FLOAT32 data[]);

  void envin (OPTIONS *options);
  void envout (OPTIONS *options);



  OPTIONS          options;

  RUN_PROGRESS     progress;

  QListWidget      *checkList;

  QString          pfm_file, feature_file, area_file, ex_file;

  optionPage       *oPage;

  NV_INT32         features_count[SELECT_PASSES], min_percent, pfm_handle, l_pfm_handle[SELECT_PASSES];

  NV_BOOL          pass_complete[SELECT_PASSES], complete;

  FEATURE          **features;

  PFM_OPEN_ARGS    open_args;


protected slots:

  void slotHelpClicked ();
  void slotCustomButtonClicked (int id);
  void slotPercentValue (NV_INT32 percent, NV_INT32 pass);
  void slotCompleted (NV_INT32 num_features, NV_INT32 pass);

};

#endif
