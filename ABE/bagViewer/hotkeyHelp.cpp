
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
