#include "extractPage.hpp"

extractPage::extractPage (QWidget *parent, RUN_PROGRESS *prog, QListWidget **cList):
  QWizardPage (parent)
{
  progress = prog;


  setTitle (tr ("DEM/Extract Page"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmChartsImageWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *lbox = new QGroupBox (tr ("Process status"), this);
  QVBoxLayout *lboxLayout = new QVBoxLayout;
  lbox->setLayout (lboxLayout);
  lboxLayout->setSpacing (10);


  checkList = new QListWidget (this);
  checkList->setAlternatingRowColors (TRUE);
  lboxLayout->addWidget (checkList);


  vbox->addWidget (lbox);


  progress->dbox = new QGroupBox (tr ("Generating Digital Elevation Model (DEM)"), this);
  QVBoxLayout *dboxLayout = new QVBoxLayout;
  progress->dbox->setLayout (dboxLayout);
  dboxLayout->setSpacing (10);


  progress->dbar = new QProgressBar (this);
  progress->dbar->setRange (0, 100);
  progress->dbar->setWhatsThis (tr ("Progress of the generation of the DEM."));
  dboxLayout->addWidget (progress->dbar);


  vbox->addWidget (progress->dbox);


  progress->ebox = new QGroupBox (tr ("Extracting images"), this);
  QVBoxLayout *eboxLayout = new QVBoxLayout;
  progress->ebox->setLayout (eboxLayout);
  eboxLayout->setSpacing (10);


  progress->ebar = new QProgressBar (this);
  progress->ebar->setRange (0, 100);
  progress->ebar->setWhatsThis (tr ("Progress of the image extraction process."));
  eboxLayout->addWidget (progress->ebar);


  vbox->addWidget (progress->ebox);


  *cList = checkList;


  //  Serious cheating here ;-)  I want the next button to be disabled when you first get to this page
  //  so I set the last progress bar to be a "mandatory" field.  I couldn't disable the damn button in
  //  initializePage in the parent for some unknown reason.

  registerField ("progress_ebar*", progress->ebar, "value");
}
