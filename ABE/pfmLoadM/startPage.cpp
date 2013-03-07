
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

startPage::startPage (QWidget *parent, QString parameter_file):
  QWizardPage (parent)
{
  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmLoadMWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("The pfmLoadM program is used to create or append to PFM structures.  Context "
                                  "sensitive help is available by clicking on the Help button and then clicking, with the "
                                  "Question Arrow cursor, on the field of interest.  You can preload a previously saved "
                                  "parameters file (*.prm) by placing it on the command line.  Press <b>Next</b> to go to the "
                                  "first PFM definition page.<br><br>"
                                  "<b><i>IMPORTANT NOTE</i>: You may load up to 30 PFM structures in one run.  This only makes "
                                  "sense if all of the PFM areas contain data from the same set of input files.  "
                                  "Otherwise the program will be checking every data point against every PFM area "
                                  "bounds definition and you will be wasting lots of time.</b><br><br>"
                                  "<b><i>OTHER IMPORTANT NOTE</i>: You may save the parameters entered into this wizard to a "
                                  "new or preexisting parameter file.  There are two types of parameter files.  The normal "
                                  "parameter file (.prm) which can be used on the command line for pfmLoadM or pfmLoadMer, "
                                  "or an <i>update</i> parameter file (.upr) that can only be used as a command line argument "
                                  "to pfmLoadMer.  The <i>update</i> parameter file (.upr) can only be created if you use the "
                                  "<i>Directories->Browse</i> button on the <i>Input Data Files</i> page.  For more information "
                                  "on why you would want to do this, use the <i>What's This</i> help on the <i>Save</i> button "
                                  "on the <i>Build PFM Structure(s)</i> page of this wizard."));

  label->setWordWrap(true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);
  vbox->addStretch (10);


  QGroupBox *prmBox = new QGroupBox (this);
  prmBox->setWhatsThis (tr ("This box will show the name of the parameter file that was placed on the command line.  ") + 
                        tr ("If no command was placed on the command line it will say <b>No command line parameter file</b>"));
  prmBox->setToolTip (tr ("Parameter file from command line (if present)"));
  QHBoxLayout *prmBoxLayout = new QHBoxLayout;
  prmBoxLayout->setSpacing (5);
  prmBox->setLayout (prmBoxLayout);

  QLabel *prmlbl = new QLabel (this);
  prmlbl->setWhatsThis (tr ("This box will show the name of the parameter file that was placed on the command line.  ") + 
                        tr ("If no command was placed on the command line it will say <b>No command line parameter file</b>"));
  prmlbl->setToolTip (tr ("Parameter file from command line (if present)"));
  if (!parameter_file.isEmpty ())
    {
      prmlbl->setText (tr ("Command line parameter file: <b>") + parameter_file + "</b>");
    }
  else
    {
      prmlbl->setText (tr ("No command line parameter file"));
    }
  prmBoxLayout->addWidget (prmlbl);

  vbox->addWidget (prmBox);
}



NV_INT32 startPage::nextId () const
{
  return (1);
}
