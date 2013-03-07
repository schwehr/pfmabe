
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "manageOverlays.hpp"
#include "manageOverlaysHelp.hpp"


//!  This is the overlay management dialog.

manageOverlays::manageOverlays (QWidget * parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;


  setWindowTitle ("pfmView Overlays");

  resize (800, 100);

  setSizeGripEnabled (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  browseGrp = new QButtonGroup (this);
  connect (browseGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotBrowse (int)));

  colorGrp = new QButtonGroup (this);
  connect (colorGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotColor (int)));

  displayGrp = new QButtonGroup (this);
  displayGrp->setExclusive (FALSE);
  connect (displayGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotDisplay (int)));

  removeGrp = new QButtonGroup (this);
  connect (removeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotRemove (int)));


  QGroupBox *overlays[NUM_OVERLAYS];
  QHBoxLayout *overlaysLayout[NUM_OVERLAYS];

  for (NV_INT32 i = 0 ; i < NUM_OVERLAYS ; i++)
    {
      overlays[i] = new QGroupBox (this);
      overlays[i]->setWhatsThis (overlaysGridText);
      overlaysLayout[i] = new QHBoxLayout;
      overlays[i]->setLayout (overlaysLayout[i]);
      
      browse[i] = new QPushButton (tr ("Browse"), this); 
      browse[i]->setToolTip (tr ("Select a file to overlay"));
      overlaysLayout[i]->addWidget (browse[i]);
      browseGrp->addButton (browse[i], i);

      file[i] = new QLineEdit (this);
      file[i]->setReadOnly (TRUE);
      file[i]->setToolTip (tr ("Filename (read-only)"));
      if (misc->overlays[i].file_type) file[i]->setText (QString (misc->overlays[i].filename));
      overlaysLayout[i]->addWidget (file[i]);

      color[i] = new QPushButton (tr ("Color"), this);
      color[i]->setToolTip (tr ("Select the overlay color"));
      colorPalette[i] = color[i]->palette ();
      overlaysLayout[i]->addWidget (color[i]);
      colorGrp->addButton (color[i], i);


      NV_INT32 hue, sat, val;
      QColor tmp;
      if (misc->overlays[i].file_type)
        {
          misc->overlays[i].color.getHsv (&hue, &sat, &val);
          tmp = misc->overlays[i].color;
        }
      else
        {
          options->contour_color.getHsv (&hue, &sat, &val);
          tmp = options->contour_color;
        }

      if (val < 128)
        {
          colorPalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
          colorPalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
        }
      else
        {
          colorPalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
          colorPalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
        }
      colorPalette[i].setColor (QPalette::Normal, QPalette::Button, tmp);
      colorPalette[i].setColor (QPalette::Inactive, QPalette::Button, tmp);
      color[i]->setPalette (colorPalette[i]);


      display[i] = new QCheckBox (tr ("Display"), this);
      display[i]->setChecked (misc->overlays[i].display);
      display[i]->setToolTip (tr ("Toggle display of the overlay"));
      overlaysLayout[i]->addWidget (display[i]);
      displayGrp->addButton (display[i], i);


      remove[i] = new QPushButton (tr ("Remove"), this);
      remove[i]->setToolTip (tr ("Remove this file from the overlays list"));
      overlaysLayout[i]->addWidget (remove[i]);
      removeGrp->addButton (remove[i], i);

      vbox->addWidget (overlays[i], 0, 0);
    }


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *applyButton = new QPushButton (tr ("Apply"), this);
  applyButton->setToolTip (tr ("Apply overlay changes"));
  applyButton->setWhatsThis (applyMOText);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApply ()));
  actions->addWidget (applyButton);


  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the overlay dialog"));
  closeButton->setWhatsThis (closeMOText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);


  show ();
}



manageOverlays::~manageOverlays ()
{
}



void
manageOverlays::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
manageOverlays::slotBrowse (int id)
{
  static QString filter = tr ("All files (*)");


  if (!QDir (options->overlay_dir).exists ()) options->overlay_dir = options->input_pfm_dir;


  QFileDialog fd (this, tr ("pfmView Overlay Input files"));
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, options->overlay_dir);


  QStringList filters;
  filters << tr ("TRACK (*.trk)")
          << tr ("AREA (*.ARE *.are *.afs)")
          << tr ("ZONE (*.zne *.ZNE *.tdz)")
          << tr ("SURVEY (*.srv)")
          << tr ("POINT (*.pts *.yxz *.txt)")
          << tr ("SHAPE (*.shp)")
          << tr ("All files (*)");

  fd.setFilters (filters);
  fd.setFileMode (QFileDialog::ExistingFile);
  fd.selectFilter (filter);

  QStringList files;
  QString newfile;
  if (fd.exec () == QDialog::Accepted) 
    {
      files = fd.selectedFiles ();
      newfile = files.at (0);
      

      if (!newfile.isEmpty())
        {
          if (newfile.endsWith (".ARE")) misc->overlays[id].file_type = ISS60_AREA;
          if (newfile.endsWith (".are")) misc->overlays[id].file_type = GENERIC_AREA;
          if (newfile.endsWith (".ZNE") || newfile.endsWith (".zne")) misc->overlays[id].file_type = ISS60_ZONE;
          if (newfile.endsWith (".tdz")) misc->overlays[id].file_type = GENERIC_ZONE;
          if (newfile.endsWith (".srv")) misc->overlays[id].file_type = ISS60_SURVEY;
          if (newfile.endsWith (".trk")) misc->overlays[id].file_type = TRACK;
          if (newfile.endsWith (".pts") || newfile.endsWith (".yxz") || newfile.endsWith (".txt")) misc->overlays[id].file_type = GENERIC_YXZ;
          if (newfile.endsWith (".shp")) misc->overlays[id].file_type = SHAPE;
          if (newfile.endsWith (".afs")) misc->overlays[id].file_type = ACE_AREA;

          strcpy (misc->overlays[id].filename, newfile.toAscii ());

          misc->overlays[id].display = NVTrue;
          display[id]->setChecked (TRUE);
          misc->overlays[id].color = options->contour_color;

          file[id]->setText (QString (misc->overlays[id].filename));


          NV_INT32 hue, sat, val;

          options->contour_color.getHsv (&hue, &sat, &val);
          if (val < 128)
            {
              colorPalette[id].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
              colorPalette[id].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
            }
          else
            {
              colorPalette[id].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
              colorPalette[id].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
            }
          colorPalette[id].setColor (QPalette::Normal, QPalette::Button, options->contour_color);
          colorPalette[id].setColor (QPalette::Inactive, QPalette::Button, options->contour_color);
          color[id]->setPalette (colorPalette[id]);
        }
    }
  filter = fd.selectedFilter ();
  options->overlay_dir = fd.directory ().absolutePath ();
}



void
manageOverlays::slotColor (int id)
{
  if (misc->overlays[id].file_type)
    {
      QColor newcolor = QColorDialog::getColor (misc->overlays[id].color, this);

      if (newcolor.isValid ()) 
        {
          misc->overlays[id].color = newcolor;


          NV_INT32 hue, sat, val;

          misc->overlays[id].color.getHsv (&hue, &sat, &val);
          if (val < 128)
            {
              colorPalette[id].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
              colorPalette[id].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
            }
          else
            {
              colorPalette[id].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
              colorPalette[id].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
            }
          colorPalette[id].setColor (QPalette::Normal, QPalette::Button, misc->overlays[id].color);
          colorPalette[id].setColor (QPalette::Inactive, QPalette::Button, misc->overlays[id].color);
          color[id]->setPalette (colorPalette[id]);
        }
    }
}



void
manageOverlays::slotDisplay (int id)
{
  if (misc->overlays[id].file_type)
    {
      if (display[id]->isChecked ())
        {
          misc->overlays[id].display = NVTrue;
        }
      else
        {
          misc->overlays[id].display = NVFalse;
        }
    }
  else
    {
      display[id]->setChecked (FALSE);
    }
}



void
manageOverlays::slotRemove (int id)
{
  if (misc->overlays[id].file_type)
    {
      misc->overlays[id].file_type = 0;
      misc->overlays[id].display = NVFalse;
      strcpy (misc->overlays[id].filename, "");
      misc->overlays[id].color = options->contour_color;


      file[id]->setText (QString (misc->overlays[id].filename));

      display[id]->setChecked (misc->overlays[id].display);


      NV_INT32 hue, sat, val;

      options->contour_color.getHsv (&hue, &sat, &val);
      if (val < 128)
        {
          colorPalette[id].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
          colorPalette[id].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
        }
      else
        {
          colorPalette[id].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
          colorPalette[id].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
        }
      colorPalette[id].setColor (QPalette::Normal, QPalette::Button, options->contour_color);
      colorPalette[id].setColor (QPalette::Inactive, QPalette::Button, options->contour_color);
      color[id]->setPalette (colorPalette[id]);
    }
}



void
manageOverlays::slotApply ()
{
  emit dataChangedSignal ();
}



void
manageOverlays::slotClose ()
{
  close ();
}
