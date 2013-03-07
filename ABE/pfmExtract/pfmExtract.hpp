#ifndef PFMEXTRACT_H
#define PFMEXTRACT_H

#include "pfmExtractDef.hpp"
#include "startPage.hpp"
#include "optionsPage.hpp"
#include "runPage.hpp"
#include "version.hpp"


class pfmExtract : public QWizard
{
  Q_OBJECT


public:

  pfmExtract (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);


protected:

  void initializePage (int id);
  void cleanupPage (int id);


  OPTIONS            options;

  RUN_PROGRESS       progress;

  QListWidget        *extractList;

  projPJ             pj_utm, pj_latlon;
  /*
  NV_INT32 surface;

  NV_FLOAT32 save_cint;

  QButtonGroup *surface_type;

  QRadioButton *min_surface, *max_surface, *avg_surface;

  QLabel *cint_label, *smooth_label, *progress_label[2];

  QProgressBar *progress_bar[2];

  QDoubleSpinBox *cint_spinbox;

  QSpinBox *smooth_spinbox;

  QListWidget *checklist;

  QCheckBox *contour_checkbox;

  QDialog *contoursD;

  QTable *contoursTable;

  QString pfm_file_name, output_file_name, area_file_name;

  QButton *bContourLevels;
  */


protected slots:

  void slotCustomButtonClicked (int id);
  void slotHelpClicked ();
  /*
  void slotPFMFileBrowse ();
  void slotOutputFileEditTextChanged (const QString &file);
  void slotOutputFileName (const QString &st);
  void slotAreaFileBrowse ();
  void slotSurfaceTypeClicked (int id);
  void slotCintValueChanged (int ivalue);
  void slotSmoothValueChanged (int ivalue);
  void slotApplyContours ();
  void slotCloseContours ();
  void slotContourLevelsClicked ();
  void slotContourClicked ();
  void slotStartProcessClicked ();
  */
};

#endif
