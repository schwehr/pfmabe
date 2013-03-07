
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



#include "surfacePage.hpp"
#include "surfacePageHelp.hpp"

surfacePage::surfacePage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Surface selection"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmMispWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *sBox = new QGroupBox (tr ("Surface Type"), this);
  QVBoxLayout *sBoxLayout = new QVBoxLayout;
  sBox->setLayout (sBoxLayout);
  sBox->setWhatsThis (surface_typeText);

  QRadioButton *minFilt = new QRadioButton (tr ("Minimum Filtered Surface"));
  QRadioButton *maxFilt = new QRadioButton (tr ("Maximum Filtered Surface"));
  QRadioButton *avgFilt = new QRadioButton (tr ("All depths"));

  QButtonGroup *surface_type = new QButtonGroup (this);
  surface_type->setExclusive (TRUE);
  connect (surface_type, SIGNAL (buttonClicked (int)), this, SLOT (slotSurfaceTypeClicked (int)));

  surface_type->addButton (minFilt, 0);
  surface_type->addButton (maxFilt, 1);
  surface_type->addButton (avgFilt, 2);

  sBoxLayout->addWidget (minFilt);
  sBoxLayout->addWidget (maxFilt);
  sBoxLayout->addWidget (avgFilt);
  
  minFilt->setWhatsThis (min_surfaceText);
  maxFilt->setWhatsThis (max_surfaceText);
  avgFilt->setWhatsThis (avg_surfaceText);

  surface_type->button (options->surface)->setChecked (TRUE);


  vbox->addWidget (sBox);


  QGroupBox *oBox = new QGroupBox (tr ("Options"), this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);

  
  QGroupBox *aBox = new QGroupBox (tr ("Replace all bins"), this);
  QHBoxLayout *aBoxLayout = new QHBoxLayout;
  aBox->setLayout (aBoxLayout);

  replaceAll = new QCheckBox (this);
  replaceAll->setToolTip (tr ("Replace all bins, not just empty bins"));
  replaceAll->setWhatsThis (replaceAllText);
  replaceAll->setChecked (options->replace_all);
  aBoxLayout->addWidget (replaceAll);


  oBoxLayout->addWidget (aBox);


  QGroupBox *cBox = new QGroupBox (tr ("Clear land"), this);
  QHBoxLayout *cBoxLayout = new QHBoxLayout;
  cBox->setLayout (cBoxLayout);

  clearLand = new QCheckBox (this);
  clearLand->setToolTip (tr ("Clear all SRTM land areas"));
  clearLand->setWhatsThis (clearLandText);
  clearLand->setChecked (options->clear_land);
  cBoxLayout->addWidget (clearLand);


  oBoxLayout->addWidget (cBox);


  QGroupBox *nBox = new QGroupBox (tr ("Nibble"), this);
  QHBoxLayout *nBoxLayout = new QHBoxLayout;
  nBox->setLayout (nBoxLayout);
  nBoxLayout->setSpacing (10);

  nibble = new QSpinBox (this);
  nibble->setRange (-1, 20);
  nibble->setSingleStep (1);
  nibble->setValue (options->nibble);
  nibble->setWrapping (FALSE);
  nibble->setToolTip (tr ("Set the cell nibbler value"));
  nibble->setWhatsThis (nibbleText);
  nBoxLayout->addWidget (nibble);


  oBoxLayout->addWidget (nBox);


  vbox->addWidget (oBox);


  QGroupBox *mBox = new QGroupBox (tr ("MISP options"), this);
  QHBoxLayout *mBoxLayout = new QHBoxLayout;
  mBox->setLayout (mBoxLayout);


  QGroupBox *fBox = new QGroupBox (tr ("Weight factor"), this);
  QHBoxLayout *fBoxLayout = new QHBoxLayout;
  fBox->setLayout (fBoxLayout);
  fBoxLayout->setSpacing (10);

  factor = new QSpinBox (this);
  factor->setRange (1, 3);
  factor->setSingleStep (1);
  factor->setValue (options->weight);
  factor->setWrapping (TRUE);
  factor->setToolTip (tr ("Set the input data weight factor"));
  factor->setWhatsThis (factorText);
  fBoxLayout->addWidget (factor);


  mBoxLayout->addWidget (fBox);


  QGroupBox *forceBox = new QGroupBox (tr ("Force original input value"), this);
  QHBoxLayout *forceBoxLayout = new QHBoxLayout;
  forceBox->setLayout (forceBoxLayout);
  forceBoxLayout->setSpacing (10);

  force = new QCheckBox (this);
  force->setChecked (options->force_original_value);
  force->setToolTip (tr ("Force final grid value to original input value"));
  force->setWhatsThis (forceText);
  forceBoxLayout->addWidget (force);

  mBoxLayout->addWidget (forceBox);


  vbox->addWidget (mBox);


  registerField ("nibble", nibble);
  registerField ("replaceAll", replaceAll);
  registerField ("clearLand", clearLand);
  registerField ("factor", factor);
  registerField ("force", force);
}



void surfacePage::slotSurfaceTypeClicked (int id)
{
  options->surface = id;
}
