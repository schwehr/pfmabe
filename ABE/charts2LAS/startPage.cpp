
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



#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (QWidget *parent, NV_BOOL in, NV_BOOL g03, NV_INT32 dtm):
  QWizardPage (parent)
{
  invalid = in;
  geoid03 = g03;
  datum = dtm;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/charts2LASWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("The charts2LAS program is used to convert CHARTS HOF and TOF files to LAS format.  "
                                  "Click the Next button to go to the input file page.  Context sensitive help is available "
                                  "by clicking on the Help button and then clicking, with the Question Arrow cursor, on the "
                                  "field of interest."));

  label->setWordWrap(true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);
  vbox->addStretch (10);



  QGroupBox *oBox = new QGroupBox (this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);
  oBoxLayout->setSpacing (10);


  QGroupBox *iBox = new QGroupBox (tr ("Invalid data"), this);
  QHBoxLayout *iBoxLayout = new QHBoxLayout;
  iBox->setLayout (iBoxLayout);
  iBoxLayout->setSpacing (10);


  oBoxLayout->addWidget (iBox);


  inv = new QCheckBox (this);
  inv->setToolTip (tr ("If checked, include invalid data in output LAS file"));
  inv->setWhatsThis (invText);
  inv->setChecked (invalid);
  iBoxLayout->addWidget (inv);


  QGroupBox *gBox = new QGroupBox (tr ("Orthometric height"), this);
  QHBoxLayout *gBoxLayout = new QHBoxLayout;
  gBox->setLayout (gBoxLayout);
  gBoxLayout->setSpacing (10);


  oBoxLayout->addWidget (gBox);


  geoid = new QCheckBox (this);
  geoid->setToolTip (tr ("If checked, correct from ellipsoidal to orthometric height"));
  geoid->setWhatsThis (geoidText);
  geoid->setChecked (geoid03);
  gBoxLayout->addWidget (geoid);


  QGroupBox *hBox = new QGroupBox (tr ("Horizontal datum"), this);
  QVBoxLayout *hBoxLayout = new QVBoxLayout;
  hBox->setLayout (hBoxLayout);
  hBoxLayout->setSpacing (10);

  hDatum = new QComboBox (this);
  hDatum->setToolTip (tr ("Select the horizontal datum"));
  hDatum->setWhatsThis (hDatumText);
  hDatum->setEditable (FALSE);
  hDatum->addItem ("WGS 84");
  hDatum->addItem ("NAD 83");
  hDatum->setCurrentIndex (datum);
  hBoxLayout->addWidget (hDatum);


  oBoxLayout->addWidget (hBox);


  vbox->addWidget (oBox);


  registerField ("invalid", inv);
  registerField ("geoid", geoid);
  registerField ("hDatum", hDatum);
}
