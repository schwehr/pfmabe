
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "runPage.hpp"
#include "runPageHelp.hpp"

runPage::runPage (QWidget *parent, RUN_PROGRESS *prog, QListWidget **cList):
  QWizardPage (parent)
{
  progress = prog;


  setTitle (tr ("Build PFM Structure(s)"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmLoadMWatermark.png"));

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


  progress->rbox = new QGroupBox (tr ("PFM bin recompute/filter progress"), this);
  QVBoxLayout *rboxLayout = new QVBoxLayout;
  progress->rbox->setLayout (rboxLayout);
  rboxLayout->setSpacing (10);

  progress->rbar = new QProgressBar (this);
  progress->rbar->setRange (0, 100);
  progress->rbar->setWhatsThis (tr ("Progress of PFM bin recomputation or filter process"));
  rboxLayout->addWidget (progress->rbar);


  vbox->addWidget (progress->rbox);


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

  registerField ("progress_rbar*", progress->rbar, "value");
}
