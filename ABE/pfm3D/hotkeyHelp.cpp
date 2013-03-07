#include "hotkeyHelp.hpp"

hotkeyHelp::hotkeyHelp (QWidget * parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;


  setWindowTitle (tr ("GUI Control Information"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QTextEdit *arrowKeys = new QTextEdit (tr ("<br><br>The Graphical User Interface (GUI) can be controlled as follows:<br><br>"
                                            "<ul>"
                                            "<li>Dynamic Zoom - Hold down the <b>Ctrl</b> key and rotate the mouse wheel forward/back "
                                            "(<i>continues until the wheel is turned in the opposite direction</i></li><br>"
                                            "<li>Static Zoom - Hold down the <b>Ctrl</b> key, press the right mouse button and "
                                            "move the cursor up/down</li><br>"
                                            "<li>Rotate - Hold down the <b>Ctrl</b> key, press the left mouse button and move "
                                            " the cursor left/right/up/down</li><br>"
                                            "<li>Center - Hold down the <b>Ctrl</b> key, position the cursor on the desired center "
                                            "and press the middle mouse button </li><br>"
                                            "<li>Exaggeration - Hold down the <b>Ctrl</b> key and press <b>PgUp/PgDn</b></li>"
                                            "</ul>"));
  /*
					    "Slicing of the data is initiated by using the mouse wheel, dragging the slider "
					    "in the scroll bar on the right of the display, by clicking the up or down arrow "
					    "keys (without holding down the <b>Ctrl</b> key), or pressing the up and down arrow "
					    "buttons in the slice scroll bar."));
  */
  arrowKeys->setReadOnly (TRUE);


  vbox->addWidget (arrowKeys, 1);

  resize (640, 480);

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
