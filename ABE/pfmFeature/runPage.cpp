#include "runPage.hpp"

runPage::runPage (QWidget *parent, RUN_PROGRESS *prog):
  QWizardPage (parent)
{
  progress = prog;


  setTitle (tr ("Process Page"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmFeatureWatermark.png"));

  setFinalPage (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  progress->sbox = new QGroupBox (tr ("Feature selection"), this);
  QVBoxLayout *sboxLayout = new QVBoxLayout;
  progress->sbox->setLayout (sboxLayout);
  sboxLayout->setSpacing (10);


  progress->sbar = new QProgressBar (this);
  progress->sbar->setRange (0, 100);
  progress->sbar->setWhatsThis (tr ("Progress of the feature selection process."));
  sboxLayout->addWidget (progress->sbar);


  vbox->addWidget (progress->sbox);


  progress->dbox = new QGroupBox (tr ("Feature deconfliction"), this);
  QVBoxLayout *dboxLayout = new QVBoxLayout;
  progress->dbox->setLayout (dboxLayout);
  dboxLayout->setSpacing (10);


  progress->dbar = new QProgressBar (this);
  progress->dbar->setRange (0, 100);
  progress->dbar->setWhatsThis (tr ("Progress of the feature deconfliction process."));
  dboxLayout->addWidget (progress->dbar);


  vbox->addWidget (progress->dbox);


  progress->wbox = new QGroupBox (tr ("Writing features"), this);
  QVBoxLayout *wboxLayout = new QVBoxLayout;
  progress->wbox->setLayout (wboxLayout);
  wboxLayout->setSpacing (10);


  progress->wbar = new QProgressBar (this);
  progress->wbar->setRange (0, 100);
  progress->wbar->setWhatsThis (tr ("Progress of the feature writing process."));
  wboxLayout->addWidget (progress->wbar);


  vbox->addWidget (progress->wbox);


  QGroupBox *lbox = new QGroupBox (tr ("Process status"), this);
  QVBoxLayout *lboxLayout = new QVBoxLayout;
  lbox->setLayout (lboxLayout);
  lboxLayout->setSpacing (10);


  progress->list = new QListWidget (this);
  progress->list->setAlternatingRowColors (TRUE);
  progress->list->setMaximumHeight (90);
  lboxLayout->addWidget (progress->list);


  vbox->addWidget (lbox);


  //  Serious cheating here ;-)  I want the finish button to be disabled when you first get to this page
  //  so I set the last progress bar to be a "mandatory" field.  I couldn't disable the damn button in
  //  initializePage in the parent for some unknown reason.

  registerField ("progress_wbar*", progress->wbar, "value");
}
