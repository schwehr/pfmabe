
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

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmGeotiffWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *sBox = new QGroupBox (tr ("Surface Type"), this);
  QVBoxLayout *sBoxLayout = new QVBoxLayout;
  sBox->setLayout (sBoxLayout);
  sBox->setWhatsThis (surface_typeText);

  QRadioButton *minFilt = new QRadioButton (tr ("Minimum Filtered Surface"));
  QRadioButton *maxFilt = new QRadioButton (tr ("Maximum Filtered Surface"));
  QRadioButton *avgFilt = new QRadioButton (tr ("Average Filtered or MISP Surface"));
  QRadioButton *covData = new QRadioButton (tr ("Coverage by data type"));

  QButtonGroup *surface_type = new QButtonGroup (this);
  surface_type->setExclusive (TRUE);
  connect (surface_type, SIGNAL (buttonClicked (int)), this, SLOT (slotSurfaceTypeClicked (int)));

  surface_type->addButton (minFilt, 0);
  surface_type->addButton (maxFilt, 1);
  surface_type->addButton (avgFilt, 2);
  surface_type->addButton (covData, 3);

  sBoxLayout->addWidget (minFilt);
  sBoxLayout->addWidget (maxFilt);
  sBoxLayout->addWidget (avgFilt);
  sBoxLayout->addWidget (covData);
  
  minFilt->setWhatsThis (min_surfaceText);
  maxFilt->setWhatsThis (max_surfaceText);
  avgFilt->setWhatsThis (avg_surfaceText);
  covData->setWhatsThis (cov_surfaceText);

  surface_type->button (options->surface)->setChecked (TRUE);


  vbox->addWidget (sBox);



  QGroupBox *fBox = new QGroupBox (tr ("Format options"), this);
  QHBoxLayout *fBoxLayout = new QHBoxLayout;
  fBox->setLayout (fBoxLayout);

  QGroupBox *tBox = new QGroupBox (tr ("Transparent Background"), this);
  QHBoxLayout *tBoxLayout = new QHBoxLayout;
  tBox->setLayout (tBoxLayout);
  transparent_check = new QCheckBox (tBox);
  transparent_check->setToolTip (tr ("Toggle creation of transparent background"));
  transparent_check->setWhatsThis (transparentText);
  transparent_check->setChecked (options->transparent);
  tBoxLayout->addWidget (transparent_check);
  fBoxLayout->addWidget (tBox);


  QGroupBox *cBox = new QGroupBox (tr ("Packbits Compression"), this);
  QHBoxLayout *cBoxLayout = new QHBoxLayout;
  cBox->setLayout (cBoxLayout);
  packbits_check = new QCheckBox (cBox);
  packbits_check->setToolTip (tr ("Output in Packbits compressed GeoTIFF format"));
  packbits_check->setWhatsThis (packbitsText);
  packbits_check->setChecked (options->packbits);
  cBoxLayout->addWidget (packbits_check);
  fBoxLayout->addWidget (cBox);


  QGroupBox *gBox = new QGroupBox (tr ("32 bit grey scale GeoTIFF"), this);
  QHBoxLayout *gBoxLayout = new QHBoxLayout;
  gBox->setLayout (gBoxLayout);
  grey_check = new QCheckBox (gBox);
  grey_check->setToolTip (tr ("Output 32 bit grey scale GeoTIFF format"));
  grey_check->setWhatsThis (greyText);
  grey_check->setChecked (options->grey);
  gBoxLayout->addWidget (grey_check);
  fBoxLayout->addWidget (gBox);


  vbox->addWidget (fBox);


  QGroupBox *oBox = new QGroupBox (tr ("Output options"), this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);

  QGroupBox *uBox = new QGroupBox (tr ("Units"), this);
  QHBoxLayout *uBoxLayout = new QHBoxLayout;
  uBox->setLayout (uBoxLayout);
  units = new QComboBox (uBox);
  units->setToolTip (tr ("Units for geoTIFF and (optional) contour files"));
  units->setWhatsThis (unitsText);
  units->setEditable (TRUE);
  units->addItem (tr ("Meters"));
  units->addItem (tr ("Fathoms"));
  units->setCurrentIndex (options->units);
  connect (units, SIGNAL (currentIndexChanged (int)), this, SLOT (slotUnitsChanged (int)));
  uBoxLayout->addWidget (units);
  oBoxLayout->addWidget (uBox);


  QGroupBox *dBox = new QGroupBox (tr ("Convert to 4800 ft/sec"), this);
  QHBoxLayout *dBoxLayout = new QHBoxLayout;
  dBox->setLayout (dBoxLayout);
  dumb_check = new QCheckBox (dBox);
  dumb_check->setToolTip (tr ("Convert data from 1500 m/sec to 4800 ft/sec"));
  dumb_check->setWhatsThis (dumbText);
  dumb_check->setChecked (options->dumb);
  dBoxLayout->addWidget (dumb_check);
  oBoxLayout->addWidget (dBox);
  if (!options->units) dumb_check->setEnabled (FALSE);


  QGroupBox *iBox = new QGroupBox (tr ("Contour interval"), this);
  QHBoxLayout *iBoxLayout = new QHBoxLayout;
  iBox->setLayout (iBoxLayout);
  interval = new QDoubleSpinBox (this);
  interval->setDecimals (2);
  interval->setRange (0.0, 1000.0);
  interval->setSingleStep (20.0);
  interval->setWrapping (FALSE);
  interval->setValue (options->cint);
  interval->setToolTip (tr ("Set the contour interval for an ESRI SHAPE file"));
  interval->setWhatsThis (intervalText);
  iBoxLayout->addWidget (interval);
  oBoxLayout->addWidget (iBox);

  if (options->surface == 3) interval->setEnabled (FALSE);


  QGroupBox *inBox = new QGroupBox (tr ("Interpolated contours"), this);
  QHBoxLayout *inBoxLayout = new QHBoxLayout;
  inBox->setLayout (inBoxLayout);
  in_check = new QCheckBox (inBox);
  in_check->setToolTip (tr ("Include contours over interpolated data"));
  in_check->setWhatsThis (inText);
  in_check->setChecked (options->intrp);
  inBoxLayout->addWidget (in_check);
  oBoxLayout->addWidget (inBox);


  QGroupBox *smBox = new QGroupBox (tr ("Contour smoothing"), this);
  QHBoxLayout *smBoxLayout = new QHBoxLayout;
  smBox->setLayout (smBoxLayout);
  smoothing = new QSpinBox (this);
  smoothing->setRange (0, 10);
  smoothing->setSingleStep (1);
  smoothing->setWrapping (FALSE);
  smoothing->setValue (options->smoothing_factor);
  smoothing->setToolTip (tr ("Set the contour smoothing level (0 = none)"));
  smoothing->setWhatsThis (smoothingText);
  smBoxLayout->addWidget (smoothing);
  oBoxLayout->addWidget (smBox);


  QGroupBox *eBox = new QGroupBox (tr ("Elevation"), this);
  QHBoxLayout *eBoxLayout = new QHBoxLayout;
  eBox->setLayout (eBoxLayout);
  elev_check = new QCheckBox (dBox);
  elev_check->setToolTip (tr ("Output as elevation instead of depth"));
  elev_check->setWhatsThis (elevText);
  elev_check->setChecked (options->elev);
  eBoxLayout->addWidget (elev_check);
  oBoxLayout->addWidget (eBox);


  QGroupBox *nBox = new QGroupBox (tr ("NAVD88"), this);
  QHBoxLayout *nBoxLayout = new QHBoxLayout;
  nBox->setLayout (nBoxLayout);
  navd88 = new QCheckBox (nBox);
  navd88->setToolTip (tr ("Convert data from WGS84 ellipsoid heights to NAVD88 heights"));
  navd88->setWhatsThis (navd88Text);
  navd88->setChecked (options->navd88);
  nBoxLayout->addWidget (navd88);
  oBoxLayout->addWidget (nBox);


  vbox->addWidget (oBox);


  registerField ("transparent_check", transparent_check);
  registerField ("packbits_check", packbits_check);
  registerField ("grey_check", grey_check);
  registerField ("elev_check", elev_check);
  registerField ("dumb_check", dumb_check);
  registerField ("in_check", in_check);
  registerField ("navd88", navd88);
  registerField ("interval", interval, "value");
  registerField ("smoothing", smoothing);
}



void surfacePage::slotSurfaceTypeClicked (int id)
{
  options->surface = id;

  if (options->surface == 3)
    {
      interval->setEnabled (FALSE);
    }
  else
    {
      interval->setEnabled (TRUE);
    }
}




void surfacePage::slotUnitsChanged (int index)
{
  options->units = index;

  if (options->units)
    {
      dumb_check->setEnabled (TRUE);
    }
  else
    {
      dumb_check->setEnabled (FALSE);
    }
}
