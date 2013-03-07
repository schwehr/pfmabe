#include "optionsPage.hpp"
#include "optionsPageHelp.hpp"

optionsPage::optionsPage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("trackLine Options"));


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/trackLineWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);



  QGroupBox *lbox = new QGroupBox (tr ("Limits"), this);
  QHBoxLayout *lboxLayout = new QHBoxLayout;
  lbox->setLayout (lboxLayout);
  QVBoxLayout *lboxLeftLayout = new QVBoxLayout;
  QVBoxLayout *lboxRightLayout = new QVBoxLayout;
  lboxLayout->addLayout (lboxLeftLayout);
  lboxLayout->addLayout (lboxRightLayout);


  QGroupBox *gBox = new QGroupBox (tr ("GSF"), this);
  QHBoxLayout *gBoxLayout = new QHBoxLayout;
  gBox->setLayout (gBoxLayout);
  gBoxLayout->setSpacing (10);

  QGroupBox *ghBox = new QGroupBox (tr ("Heading"), this);
  QHBoxLayout *ghBoxLayout = new QHBoxLayout;
  ghBox->setLayout (ghBoxLayout);
  ghBoxLayout->setSpacing (10);

  headingGSF = new QDoubleSpinBox (this);
  headingGSF->setDecimals (1);
  headingGSF->setRange (1.0, 50.0);
  headingGSF->setSingleStep (10.0);
  headingGSF->setValue (options->heading[TRACKLINE_GSF]);
  headingGSF->setWrapping (TRUE);
  headingGSF->setToolTip (tr ("Set the heading limit for GSF files (degrees)"));
  headingGSF->setWhatsThis (headingGSFText);
  ghBoxLayout->addWidget (headingGSF);
  gBoxLayout->addWidget (ghBox);


  QGroupBox *gsBox = new QGroupBox (tr ("Speed"), this);
  QHBoxLayout *gsBoxLayout = new QHBoxLayout;
  gsBox->setLayout (gsBoxLayout);
  gsBoxLayout->setSpacing (10);

  speedGSF = new QDoubleSpinBox (this);
  speedGSF->setDecimals (1);
  speedGSF->setRange (1.0, 50.0);
  speedGSF->setSingleStep (10.0);
  speedGSF->setValue (options->speed[TRACKLINE_GSF]);
  speedGSF->setWrapping (TRUE);
  speedGSF->setToolTip (tr ("Set the speed limit for GSF files (knots)"));
  speedGSF->setWhatsThis (speedGSFText);
  gsBoxLayout->addWidget (speedGSF);
  gBoxLayout->addWidget (gsBox);


  lboxLeftLayout->addWidget (gBox);


  QGroupBox *pBox = new QGroupBox (tr ("POS/SBET"), this);
  QHBoxLayout *pBoxLayout = new QHBoxLayout;
  pBox->setLayout (pBoxLayout);
  pBoxLayout->setSpacing (10);

  QGroupBox *phBox = new QGroupBox (tr ("Heading"), this);
  QHBoxLayout *phBoxLayout = new QHBoxLayout;
  phBox->setLayout (phBoxLayout);
  phBoxLayout->setSpacing (10);

  headingPOS = new QDoubleSpinBox (this);
  headingPOS->setDecimals (1);
  headingPOS->setRange (1.0, 50.0);
  headingPOS->setSingleStep (10.0);
  headingPOS->setValue (options->heading[TRACKLINE_POS]);
  headingPOS->setWrapping (TRUE);
  headingPOS->setToolTip (tr ("Set the heading limit for POS/SBET files (degrees)"));
  headingPOS->setWhatsThis (headingPOSText);
  phBoxLayout->addWidget (headingPOS);
  pBoxLayout->addWidget (phBox);


  QGroupBox *psBox = new QGroupBox (tr ("Speed"), this);
  QHBoxLayout *psBoxLayout = new QHBoxLayout;
  psBox->setLayout (psBoxLayout);
  psBoxLayout->setSpacing (10);

  speedPOS = new QDoubleSpinBox (this);
  speedPOS->setDecimals (1);
  speedPOS->setRange (50.0, 500.0);
  speedPOS->setSingleStep (10.0);
  speedPOS->setValue (options->speed[TRACKLINE_POS]);
  speedPOS->setWrapping (TRUE);
  speedPOS->setToolTip (tr ("Set the speed limit for POS/SBET files (knots)"));
  speedPOS->setWhatsThis (speedPOSText);
  psBoxLayout->addWidget (speedPOS);
  pBoxLayout->addWidget (psBox);


  lboxLeftLayout->addWidget (pBox);


  QGroupBox *wBox = new QGroupBox (tr ("WLF"), this);
  QHBoxLayout *wBoxLayout = new QHBoxLayout;
  wBox->setLayout (wBoxLayout);
  wBoxLayout->setSpacing (10);

  QGroupBox *whBox = new QGroupBox (tr ("Heading"), this);
  QHBoxLayout *whBoxLayout = new QHBoxLayout;
  whBox->setLayout (whBoxLayout);
  whBoxLayout->setSpacing (10);

  headingWLF = new QDoubleSpinBox (this);
  headingWLF->setDecimals (1);
  headingWLF->setRange (1.0, 50.0);
  headingWLF->setSingleStep (10.0);
  headingWLF->setValue (options->heading[TRACKLINE_WLF]);
  headingWLF->setWrapping (TRUE);
  headingWLF->setToolTip (tr ("Set the heading limit for WLF files (degrees)"));
  headingWLF->setWhatsThis (headingWLFText);
  whBoxLayout->addWidget (headingWLF);
  wBoxLayout->addWidget (whBox);


  QGroupBox *wsBox = new QGroupBox (tr ("Speed"), this);
  QHBoxLayout *wsBoxLayout = new QHBoxLayout;
  wsBox->setLayout (wsBoxLayout);
  wsBoxLayout->setSpacing (10);

  speedWLF = new QDoubleSpinBox (this);
  speedWLF->setDecimals (1);
  speedWLF->setRange (50.0, 500.0);
  speedWLF->setSingleStep (10.0);
  speedWLF->setValue (options->speed[TRACKLINE_WLF]);
  speedWLF->setWrapping (TRUE);
  speedWLF->setToolTip (tr ("Set the speed limit for WLF files (knots)"));
  speedWLF->setWhatsThis (speedWLFText);
  wsBoxLayout->addWidget (speedWLF);
  wBoxLayout->addWidget (wsBox);


  lboxRightLayout->addWidget (wBox);


  QGroupBox *hBox = new QGroupBox (tr ("Hawkeye"), this);
  QHBoxLayout *hBoxLayout = new QHBoxLayout;
  hBox->setLayout (hBoxLayout);
  hBoxLayout->setSpacing (10);

  QGroupBox *hhBox = new QGroupBox (tr ("Heading"), this);
  QHBoxLayout *hhBoxLayout = new QHBoxLayout;
  hhBox->setLayout (hhBoxLayout);
  hhBoxLayout->setSpacing (10);

  headingHWK = new QDoubleSpinBox (this);
  headingHWK->setDecimals (1);
  headingHWK->setRange (1.0, 50.0);
  headingHWK->setSingleStep (10.0);
  headingHWK->setValue (options->heading[TRACKLINE_HWK]);
  headingHWK->setWrapping (TRUE);
  headingHWK->setToolTip (tr ("Set the heading limit for Hawkeye files (degrees)"));
  headingHWK->setWhatsThis (headingHWKText);
  hhBoxLayout->addWidget (headingHWK);
  hBoxLayout->addWidget (hhBox);


  QGroupBox *hsBox = new QGroupBox (tr ("Speed"), this);
  QHBoxLayout *hsBoxLayout = new QHBoxLayout;
  hsBox->setLayout (hsBoxLayout);
  hsBoxLayout->setSpacing (10);

  speedHWK = new QDoubleSpinBox (this);
  speedHWK->setDecimals (1);
  speedHWK->setRange (50.0, 500.0);
  speedHWK->setSingleStep (10.0);
  speedHWK->setValue (options->speed[TRACKLINE_HWK]);
  speedHWK->setWrapping (TRUE);
  speedHWK->setToolTip (tr ("Set the speed limit for Hawkeye files (knots)"));
  speedHWK->setWhatsThis (speedHWKText);
  hsBoxLayout->addWidget (speedHWK);
  hBoxLayout->addWidget (hsBox);


  lboxRightLayout->addWidget (hBox);


  vbox->addWidget (lbox);


  registerField ("headingGSF", headingGSF, "value", "valueChanged");
  registerField ("speedGSF", speedGSF, "value", "valueChanged");
  registerField ("headingPOS", headingPOS, "value", "valueChanged");
  registerField ("speedPOS", speedPOS, "value", "valueChanged");
  registerField ("headingWLF", headingWLF, "value", "valueChanged");
  registerField ("speedWLF", speedWLF, "value", "valueChanged");
  registerField ("headingHWK", headingHWK, "value", "valueChanged");
  registerField ("speedHWK", speedHWK, "value", "valueChanged");
}
