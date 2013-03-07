
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

runPage::runPage (QWidget *parent, RUN_PROGRESS *prog):
  QWizardPage (parent)
{
  progress = prog;


  setTitle (tr ("Process Page"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/charts2LASWatermark.png"));

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
