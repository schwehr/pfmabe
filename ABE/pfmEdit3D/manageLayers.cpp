
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



#include "manageLayers.hpp"
#include "manageLayersHelp.hpp"


//!  This is the PFM layer management dialog.

manageLayers::manageLayers (QWidget * parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;


  setWindowTitle ("pfmEdit3D Layer Preferences");


  resize (400, 100);


  setSizeGripEnabled (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  displayGrp = new QButtonGroup (this);
  displayGrp->setExclusive (FALSE);
  connect (displayGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotDisplay (int)));


  QHBoxLayout *layersLayout[MAX_ABE_PFMS];

  for (NV_INT32 i = 0 ; i < misc->abe_share->pfm_count ; i++)
    {
      layers[i] = new QGroupBox (tr ("Layer  "), this);
      layers[i]->setWhatsThis (layersGridText);
      layersLayout[i] = new QHBoxLayout;
      layers[i]->setLayout (layersLayout[i]);

      file[i] = new QLineEdit (layers[i]);
      file[i]->setReadOnly (TRUE);
      file[i]->setToolTip (tr ("Filename (read-only)"));
      layersLayout[i]->addWidget (file[i]);
      file[i]->setText (QFileInfo (QString (misc->abe_share->open_args[i].list_path)).fileName ());


      display[i] = new QCheckBox (tr ("Display"), layers[i]);
      display[i]->setToolTip (tr ("Toggle display of the layer"));
      display[i]->setChecked (misc->abe_share->display_pfm[i]);
      layersLayout[i]->addWidget (display[i]);
      displayGrp->addButton (display[i], i);

      vbox->addWidget (layers[i], 0, 0);
    }


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the layers dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);


  show ();
}



manageLayers::~manageLayers ()
{
}



void
manageLayers::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
manageLayers::slotDisplay (int id)
{
  if (display[id]->isChecked ())
    {
      misc->abe_share->display_pfm[id] = NVTrue;
    }
  else
    {
      misc->abe_share->display_pfm[id] = NVFalse;
    }

  emit dataChangedSignal ();
}



void
manageLayers::slotClose ()
{
  close ();
}
