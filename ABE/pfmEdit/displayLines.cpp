
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



#include "displayLines.hpp"
#include "displayLinesHelp.hpp"


//!  Dialog to allow user to select lines to display.

displayLines::displayLines (QWidget * parent, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  misc = mi;


  setWindowTitle (tr ("pfmEdit View Selected Lines"));
  resize (500, 500);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *sbox = new QGroupBox (tr ("Selected lines"), this);
  QVBoxLayout *sboxLayout = new QVBoxLayout;
  sbox->setLayout (sboxLayout);


  selectBox = new QListWidget (this);
  selectBox->setWhatsThis (DLText);
  selectBox->setSelectionMode (QAbstractItemView::ExtendedSelection);
  //connect (selectBox, SIGNAL (itemClicked (QListWidgetItem *)), this, SLOT (slotItemClicked (QListWidgetItem *)));
  sboxLayout->addWidget (selectBox);

  vbox->addWidget (sbox, 0, 0);


  NV_CHAR temp[512];
  for (NV_INT32 i = 0 ; i < misc->line_count ; i++)
    {
      NV_INT32 pfm = misc->line_number[i] / SHARED_LINE_MULT;
      NV_INT32 line_number = misc->line_number[i] % SHARED_LINE_MULT;

      strcpy (temp, read_line_file (misc->pfm_handle[pfm], line_number));

      if (!strcmp (temp, "UNDEFINED")) sprintf (temp, "%d", line_number);

      QString lineData = QString (temp) + "   :   " + 
	QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm") + "   :   " +
	QString (PFM_data_type[misc->line_type[i]]);

      selectList += lineData;
    }

  selectBox->addItems (selectList);

  for (NV_INT32 i = 0 ; i < misc->line_count ; i++)
    {
      if (misc->num_lines && misc->line_num[i] == misc->line_number[i]) selectBox->item (i)->setSelected (TRUE);
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
  applyButton->setToolTip (tr ("Redraw main window using selected lines"));
  applyButton->setWhatsThis (applyDLText);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApply ()));
  actions->addWidget (applyButton);

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the View Selected Lines dialog"));
  closeButton->setWhatsThis (closeDLText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);


  show ();
}



displayLines::~displayLines ()
{
}



void
displayLines::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
displayLines::slotApply ()
{
  QList<QListWidgetItem *> list = selectBox->selectedItems ();


  //  Clear all the old lines.

  misc->num_lines = 0;


  for (NV_INT32 i = 0 ; i < list.size () ; i++)
    {
      misc->line_num[misc->num_lines] = misc->line_number[selectBox->row (list.at (i))];
      misc->num_lines++;
    }

  emit dataChangedSignal ();
}



void
displayLines::slotClose ()
{
  close ();
}
