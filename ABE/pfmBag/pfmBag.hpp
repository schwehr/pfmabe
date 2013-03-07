#ifndef PFMBAG_H
#define PFMBAG_H

#include "pfmBagDef.hpp"
#include "startPage.hpp"
#include "surfacePage.hpp"
#include "datumPage.hpp"
#include "classPage.hpp"
#include "runPage.hpp"


class pfmBag : public QWizard
{
  Q_OBJECT


public:

  pfmBag (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~pfmBag ();


protected:

  void initializePage (int id);
  void cleanupPage (int id);

  void envin (OPTIONS *options);
  void envout (OPTIONS *options);



  OPTIONS          options;

  RUN_PROGRESS     progress;

  projPJ           pj_utm, pj_latlon;

  surfacePage      *sPage;

  QListWidget      *checkList;

  QString          output_file_name, area_file_name, sep_file_name;


protected slots:

  void slotHelpClicked ();
  void slotCustomButtonClicked (int id);

};

#endif
