#include "runPage.hpp"
#include "runPageHelp.hpp"

runPage::runPage (QWidget *parent, RUN_PROGRESS *prog, QListWidget **tList):
  QWizardPage (parent)
{
  progress = prog;


  setTitle (tr ("Build trackline file"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/trackLineWatermark.png"));

  setFinalPage (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  progress->fbox = new QGroupBox (tr ("Input file processing progress"), this);
  QVBoxLayout *fboxLayout = new QVBoxLayout;
  progress->fbox->setLayout (fboxLayout);
  fboxLayout->setSpacing (10);


  progress->fbar = new QProgressBar (this);
  progress->fbar->setRange (0, 100);
  progress->fbar->setWhatsThis (tr ("Progress of input file loading"));
  fboxLayout->addWidget (progress->fbar);


  vbox->addWidget (progress->fbox);


  QGroupBox *lbox = new QGroupBox (tr ("Process status"), this);
  QVBoxLayout *lboxLayout = new QVBoxLayout;
  lbox->setLayout (lboxLayout);
  lboxLayout->setSpacing (10);


  trackList = new QListWidget (this);
  trackList->setAlternatingRowColors (TRUE);
  lboxLayout->addWidget (trackList);


  vbox->addWidget (lbox);


  *tList = trackList;


  //  Serious cheating here ;-)  I want the finish button to be disabled when you first get to this page
  //  so I set the last progress bar to be a "mandatory" field.  I couldn't disable the damn button in
  //  initializePage in the parent for some unknown reason.

  registerField ("progress_fbar*", progress->fbar, "value");
}
