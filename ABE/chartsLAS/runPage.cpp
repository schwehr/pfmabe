#include "runPage.hpp"

runPage::runPage (QWidget *parent, RUN_PROGRESS *prog):
  QWizardPage (parent)
{
  progress = prog;


  setTitle (tr ("Process Page"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/chartsLASWatermark.png"));

  setFinalPage (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  progress->fbox = new QGroupBox (tr ("Current file conversion progress"), this);
  QVBoxLayout *fboxLayout = new QVBoxLayout;
  progress->fbox->setLayout (fboxLayout);
  fboxLayout->setSpacing (10);


  progress->fbar = new QProgressBar (this);
  progress->fbar->setRange (0, 100);
  progress->fbar->setWhatsThis (tr ("Progress of the conversion of the current input data file."));
  fboxLayout->addWidget (progress->fbar);


  vbox->addWidget (progress->fbox);


  progress->obox = new QGroupBox (tr ("Total file conversion progress"), this);
  QVBoxLayout *oboxLayout = new QVBoxLayout;
  progress->obox->setLayout (oboxLayout);
  oboxLayout->setSpacing (10);


  progress->obar = new QProgressBar (this);
  progress->obar->setRange (0, 100);
  progress->obar->setWhatsThis (tr ("Progress of the conversion of all of the input data files."));
  oboxLayout->addWidget (progress->obar);


  vbox->addWidget (progress->obox);


  QGroupBox *listBox = new QGroupBox (tr ("Status information"), this);
  QHBoxLayout *listBoxLayout = new QHBoxLayout;
  listBox->setLayout (listBoxLayout);
  listBoxLayout->setSpacing (10);

  progress->list = new QListWidget (this);
  progress->list->setAlternatingRowColors (TRUE);

  listBoxLayout->addWidget (progress->list);

  vbox->addWidget (listBox);


  //  Serious cheating here ;-)  I want the finish button to be disabled when you first get to this page
  //  so I set the last progress bar to be a "mandatory" field.  I couldn't disable the damn button in
  //  initializePage in the parent for some unknown reason.

  registerField ("progress_obar*", progress->obar, "value");
}
