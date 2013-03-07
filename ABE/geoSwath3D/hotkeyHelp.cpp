#include "hotkeyHelp.hpp"

hotkeyHelp::hotkeyHelp (QWidget * parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;


  setWindowTitle (tr ("Hot Key Information"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QTextEdit *arrowKeys = new QTextEdit (tr ("<br><br>The Graphical User Interface (GUI) can be controlled as follows:<br><br>"
					    "Point of view is controlled by holding down the <b>Ctrl</b> key and clicking and "
					    "dragging with the left mouse button.  It can also be controlled by holding down "
					    "the <b>Ctrl</b> key and using the left, right, up, and down arrow keys.  Zoom is "
					    "controlled by holding down the <b>Ctrl</b> key and using the mouse wheel or holding "
					    "down the <b>Ctrl</b> key, pressing the right mouse button, and moving the cursor up "
					    "or down.  To reposition the center of the view just place the cursor at the desired "
					    "location, hold down the <b>Ctrl</b> key, and click the middle mouse button.  Z "
					    "exaggeration can be changed by pressing <b>Ctrl-Page Up</b> or <b>Ctrl-Page Down</b> "
					    "or by editing the exaggeration value in the Preferences dialog "
					    "<img source=\":/icons/prefs.xpm\"><br><br><br>"
					    "Slicing of the data is initiated by using the mouse wheel, dragging the slider "
					    "in the scroll bar on the right of the display, by clicking the up or down arrow "
					    "keys (without holding down the <b>Ctrl</b> key), or pressing the up and down arrow "
					    "buttons in the slice scroll bar."));
  arrowKeys->setReadOnly (TRUE);



  QTableWidget *mb = new QTableWidget (HOTKEYS, 3, this);
  mb->setAlternatingRowColors (TRUE);

  QTableWidgetItem *mb0Item = new QTableWidgetItem (tr ("Icon"));
  mb->setHorizontalHeaderItem (0, mb0Item);

  QTableWidgetItem *mb1Item = new QTableWidgetItem (tr ("Action"));
  mb->setHorizontalHeaderItem (1, mb1Item);

  QTableWidgetItem *mb2Item = new QTableWidgetItem (tr ("Hot key"));
  mb->setHorizontalHeaderItem (2, mb2Item);


  QTableWidgetItem *bItem[HOTKEYS];


  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      bItem[i] = new QTableWidgetItem (misc->buttonIcon[i], "");
      mb->setItem (i, 0, bItem[i]);

      QTableWidgetItem *act = new QTableWidgetItem (misc->buttonText[i]);
      mb->setItem (i, 1, act);

      QTableWidgetItem *hk = new QTableWidgetItem (options->buttonAccel[i]);
      mb->setItem (i, 2, hk);
    }
  mb->resizeColumnsToContents ();




  QTableWidget *ap = new QTableWidget (NUMPROGS, 6, this);
  ap->setAlternatingRowColors (TRUE);

  QTableWidgetItem *ap0Item = new QTableWidgetItem (tr ("Program name"));
  ap->setHorizontalHeaderItem (0, ap0Item);

  QTableWidgetItem *ap1Item = new QTableWidgetItem (tr ("Description"));
  ap->setHorizontalHeaderItem (1, ap1Item);

  QTableWidgetItem *ap2Item = new QTableWidgetItem (tr ("Hot key"));
  ap->setHorizontalHeaderItem (2, ap2Item);

  QTableWidgetItem *ap3Item = new QTableWidgetItem (tr ("Action keys"));
  ap->setHorizontalHeaderItem (3, ap3Item);

  QTableWidgetItem *ap4Item = new QTableWidgetItem (tr ("Polygon eligible"));
  ap->setHorizontalHeaderItem (4, ap4Item);

  QTableWidgetItem *ap5Item = new QTableWidgetItem (tr ("Polygon only"));
  ap->setHorizontalHeaderItem (5, ap5Item);

  
  QTableWidgetItem *descItem[NUMPROGS];


  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      QTableWidgetItem *pn = new QTableWidgetItem (options->name[i]);
      ap->setItem (i, 0, pn);

      descItem[i] = new QTableWidgetItem (options->description[i]);
      ap->setItem (i, 1, descItem[i]);

      QTableWidgetItem *hk = new QTableWidgetItem (options->hotkey[i]);
      ap->setItem (i, 2, hk);

      QTableWidgetItem *ak = new QTableWidgetItem (options->action[i]);
      ap->setItem (i, 3, ak);


      QString text;

      if (options->hk_poly_eligible[i])
	{
	  text = tr ("Yes");
	}
      else
	{
	  text = tr ("No");
	}
      QTableWidgetItem *hpe = new QTableWidgetItem (text);
      ap->setItem (i, 4, hpe);


      if (options->hk_poly_only[i])
	{
	  text = tr ("Yes");
	}
      else
	{
	  text = tr ("No");
	}
      QTableWidgetItem *hpo = new QTableWidgetItem (text);
      ap->setItem (i, 5, hpo);
    }
  ap->resizeColumnsToContents ();


  hotkeyTabWidget = new QTabWidget;
  hotkeyTabWidget->addTab (arrowKeys, tr ("User view control (Non-modifiable)"));
  hotkeyTabWidget->addTab (mb, tr ("Main Buttons (Preferences)"));
  hotkeyTabWidget->addTab (ap, tr ("Ancillary Programs (Preferences)"));

  vbox->addWidget (hotkeyTabWidget, 1);

  resize (912, 568);

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *applyButton = new QPushButton (tr ("Close"), this);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (applyButton);
}



hotkeyHelp::~hotkeyHelp ()
{
}



void 
hotkeyHelp::slotClose ()
{
  close ();
}
