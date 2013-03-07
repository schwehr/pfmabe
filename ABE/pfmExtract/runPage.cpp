#include "runPage.hpp"

runPage::runPage (QWidget *parent, RUN_PROGRESS *prog, QListWidget **eList):
  QWizardPage (parent)
{
  progress = prog;


  setTitle (tr ("Extract Data"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmExtractWatermark.png"));

  setFinalPage (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);



  progress->ebox = new QGroupBox (tr ("Data extraction"), this);
  QVBoxLayout *eboxLayout = new QVBoxLayout;
  progress->ebox->setLayout (eboxLayout);
  eboxLayout->setSpacing (10);


  progress->ebar = new QProgressBar (this);
  progress->ebar->setRange (0, 100);
  progress->ebar->setWhatsThis (tr ("Progress of the data extraction process."));
  eboxLayout->addWidget (progress->ebar);


  vbox->addWidget (progress->ebox);


  QGroupBox *lbox = new QGroupBox (tr ("Process status"), this);
  QVBoxLayout *lboxLayout = new QVBoxLayout;
  lbox->setLayout (lboxLayout);
  lboxLayout->setSpacing (10);


  extractList = new QListWidget (this);
  extractList->setAlternatingRowColors (TRUE);
  lboxLayout->addWidget (extractList);


  vbox->addWidget (lbox);


  *eList = extractList;


  //  Serious cheating here ;-)  I want the finish button to be disabled when you first get to this page
  //  so I set the last progress bar to be a "mandatory" field.  I couldn't disable the damn button in
  //  initializePage in the parent for some unknown reason.

  registerField ("progress_ebar*", progress->ebar, "value");
}
