#include "runPage.hpp"

runPage::runPage (QWidget *parent, RUN_PROGRESS *prog, QListWidget **cList):
  QWizardPage (parent)
{
  progress = prog;


  setTitle (tr ("Process Page"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmBagWatermark.png"));

  setFinalPage (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  progress->wbox = new QGroupBox (tr ("Computing enhanced navigation surface weights"), this);
  QVBoxLayout *wboxLayout = new QVBoxLayout;
  progress->wbox->setLayout (wboxLayout);
  wboxLayout->setSpacing (10);


  progress->wbar = new QProgressBar (this);
  progress->wbar->setRange (0, 100);
  progress->wbar->setWhatsThis (tr ("Progress of computing enhanced navigation surface weights."));
  wboxLayout->addWidget (progress->wbar);


  vbox->addWidget (progress->wbox);


  progress->mbox = new QGroupBox (tr ("Writing BAG surface data"), this);
  QVBoxLayout *mboxLayout = new QVBoxLayout;
  progress->mbox->setLayout (mboxLayout);
  mboxLayout->setSpacing (10);


  progress->mbar = new QProgressBar (this);
  progress->mbar->setRange (0, 100);
  progress->mbar->setWhatsThis (tr ("Progress of writing BAG surface data."));
  mboxLayout->addWidget (progress->mbar);


  vbox->addWidget (progress->mbox);


  progress->gbox = new QGroupBox (tr ("Writing tracking list"), this);
  QVBoxLayout *gboxLayout = new QVBoxLayout;
  progress->gbox->setLayout (gboxLayout);
  gboxLayout->setSpacing (10);


  progress->gbar = new QProgressBar (this);
  progress->gbar->setRange (0, 100);
  progress->gbar->setWhatsThis (tr ("Progress of writing tracking list."));
  gboxLayout->addWidget (progress->gbar);


  vbox->addWidget (progress->gbox);


  QGroupBox *lbox = new QGroupBox (tr ("Process status"), this);
  QVBoxLayout *lboxLayout = new QVBoxLayout;
  lbox->setLayout (lboxLayout);
  lboxLayout->setSpacing (10);


  checkList = new QListWidget (this);
  checkList->setAlternatingRowColors (TRUE);
  lboxLayout->addWidget (checkList);


  vbox->addWidget (lbox);


  *cList = checkList;


  //  Serious cheating here ;-)  I want the finish button to be disabled when you first get to this page
  //  so I set the last progress bar to be a "mandatory" field.  I couldn't disable the damn button in
  //  initializePage in the parent for some unknown reason.

  registerField ("progress_gbar*", progress->gbar, "value");
}
