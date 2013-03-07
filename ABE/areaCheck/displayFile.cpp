#include "displayFile.hpp"
#include "displayFileHelp.hpp"

displayFile::displayFile (QWidget * parent, OPTIONS *op, MISC *mi, NV_INT32 type):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  k = type;


  QString title = tr ("areaCheck Display ") + misc->type_name[k] + tr (" Files");
  setWindowTitle (title);


  setSizeGripEnabled (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  count = misc->num_overlays[k];


  displayTable = new QTableWidget (count, 2, this);
  displayTable->setWhatsThis (displayTableText);
  displayTable->setAlternatingRowColors (TRUE);
  QTableWidgetItem *dItem = new QTableWidgetItem (tr ("Display"));
  displayTable->setHorizontalHeaderItem (0, dItem);
  dItem->setTextAlignment (Qt::AlignHCenter);
  QTableWidgetItem *fnItem = new QTableWidgetItem (tr ("File name"));
  displayTable->setHorizontalHeaderItem (1, fnItem);
  fnItem->setTextAlignment (Qt::AlignHCenter);


  displayed = (QCheckBox **) calloc (count, sizeof (QCheckBox *));
  vItem = (QTableWidgetItem **) calloc (count, sizeof (QTableWidgetItem *));
  origStatus = (NV_BOOL *) calloc (count, sizeof (NV_BOOL));
  newStatus = (NV_BOOL *) calloc (count, sizeof (NV_BOOL));


  displayGrp = new QButtonGroup (this);
  displayGrp->setExclusive (FALSE);


  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      QString vh;
      vh.sprintf ("%d", i);
      vItem[i] = new QTableWidgetItem (vh);
      displayTable->setVerticalHeaderItem (i, vItem[i]);

      displayed[i] = new QCheckBox (this);
      displayTable->setCellWidget (i, 0, displayed[i]);
      displayGrp->addButton (displayed[i], i);

      QTableWidgetItem *fn = new QTableWidgetItem (QString (misc->overlays[k][i].filename));
      displayTable->setItem (i, 1, fn);
      fn->setFlags (Qt::ItemIsEnabled);

      if (misc->overlays[k][i].active)
        {
          displayed[i]->setCheckState (Qt::Checked);
          origStatus[i] = newStatus[i] = NVTrue;
        }
      else
        {
          displayed[i]->setCheckState (Qt::Unchecked);
          origStatus[i] = newStatus[i] = NVFalse;
        }
    }
  displayTable->resizeColumnsToContents ();
  displayTable->resizeRowsToContents ();


  vbox->addWidget (displayTable, 0, 0);


  //  Connect after populating table so the slot doesn't get called.

  connect (displayGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotValueChanged (int)));


  NV_INT32 width = qMin (800, displayTable->columnWidth (0) + displayTable->columnWidth (1) + 60);
  NV_INT32 height = qMin (600, displayTable->rowHeight (0) * count + 75);
  resize (width, height);


  displayTable->show ();


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *applyButton = new QPushButton (tr ("Apply"), this);
  applyButton->setToolTip (tr ("Apply changes to display flags"));
  applyButton->setWhatsThis (applyDFText);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApply ()));
  actions->addWidget (applyButton);

  QString closeTip = tr ("Close the Display ") + misc->type_name[k] + tr (" File Dialog");
  QString closeText = tr ("This button closes the Display ") + misc->type_name[k] + tr (" File dialog.  Any changes that have not been applied"
											"by pressing the <b>Apply</b> button will be discarded.");

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (closeTip);
  closeButton->setWhatsThis (closeText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);


  show ();
}



displayFile::~displayFile ()
{
}



void
displayFile::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
displayFile::slotValueChanged (int row)
{
  if (displayed[row]->checkState () == Qt::Checked)
    {
      newStatus[row] = NVTrue;
    }
  else
    {
      newStatus[row] = NVFalse;
    }
}



void
displayFile::slotApply ()
{
  NV_BOOL changed = NVFalse;
  

  //  Check for a change in status.

  for (NV_INT16 i = 0 ; i < count ; i++)
    {
      if (origStatus[i] != newStatus[i])
        {
          misc->overlays[k][i].active = newStatus[i];
	  origStatus[i] = newStatus[i];
          changed = NVTrue;
        }
    }



  if (changed) emit dataChangedSignal ();
}



void
displayFile::slotClose ()
{
  free (displayed);
  free (vItem);
  free (origStatus);
  free (newStatus);

  emit displayFileDialogClosedSignal (k);

  close ();
}
