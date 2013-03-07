
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



#include "startupMessage.hpp"

/*!
  This is where you would add a "New Features" startup message.  If you do want to add one you need to
  change the "new_feature" value in pfmView.cpp and move the old message (from the msgBox->setText
  below) to the new_features.txt file (just so we'll have a bit of history).
*/

startupMessage::startupMessage (QWidget *parent, OPTIONS *op, NV_INT32 nf):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose | Qt::WindowStaysOnTopHint)
{
  options = op;
  new_feature = nf;

  setWindowTitle (tr ("pfmView New Features"));

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QTextEdit *msgBox = new QTextEdit (0);
  msgBox->setAcceptRichText (TRUE);
  msgBox->setMinimumWidth (800);
  msgBox->setMinimumHeight (600);

  msgBox->setText (tr ("<br><center><h3>New Features</h3></center><br>"
                       "There is a new waveform filter for HOF LIDAR data in pfmEdit(3D).  The filter consists of three passes.  The first "
                       "pass invalidates selected points that have a low fore or back slope.  The second pass uses a user defined search "
                       "radius combined with the horizontal uncertainty and vertical uncertainty of the points to eliminate from filtering "
                       "any valid points that have a valid point from another line within the Hockey Puck of Confidence (TM) of the point "
                       "being considered.  The final pass looks for rising waveforms in both PMT and APD (if applicable) in the waveforms "
                       "of valid or invalid adjacent points (within the radius and Z limits).  A user defined search width defines how far "
                       "before and after the selected bin in the adjacent waveforms to search for a rise.  A user defined rise threshold "
                       "defines how many consecutive rise points are considered significant.  The third pass only works well in areas with "
                       "200% coverage.  Depending on the number of points being displayed in pfmEdit(3D) this filter can take quite a bit "
                       "of time due to the amount of processing involved.<br><br>"
                       "If you flag data points in pfmEdit(3D) the flagged points are treated as highlighted points and may be deleted "
                       "by pressing the <b>Delete</b> or <b>Del</b> key.  You may also unhighlight portions of the flagged data and "
                       "highlight additional points using the highlight buttons.  If you have highlighted/flagged invalid data, pressing "
                       "<b>Insert</b> or <b>Ins</b> will restore the highlighted points.<br><br>"
                       "You can now<b>Hide</b> or <b>Show</b> highlighted data points.  Note that if you hide highlighted points then clear "
                       "the highlights the points will remain hidden until you press the <b>Clear data mask</b> button.<br><br>"
                       "You can now modify the scale color and scale value locks for depth in pfmEdit3D.  Previously this could only be "
                       "done in pfmView.<br><br>"
                       "Filter parameters in pfmEdit(3D) can now be manually changed by entering the desired value in the box next to the "
                       "parameter slider and pressing <b><i>Return</i></b> or <b><i>Enter</i></b>.<br><br>"
                       "<b>IMPORTANT NOTE: The default hot key for deleting manually highlighted or filter highlighted points has been "
                       "changed from </b><i>Return</i><b> to </b><i>Delete</i><b> or </b><i>Del</i><b>.  You can now turn off highlighted "
                       "points or reject filter highlighted points by pressing </b><i>Insert</i><b> or </b><i>Ins</i><b>.  This change was "
                       "necessitated by the above mentioned change to allow manual editing of the filter parameters.</b><br><br>"
                       "You can now color On-The_fly grids using the number of soundings, standard deviation, average minus minimum, "
                       "maximum minus average, or maximum minus minimum attributes.<br><br>"));


  vbox->addWidget (msgBox);

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QCheckBox *dontShow = new QCheckBox (tr ("Don't show this again"), 0);
  dontShow->setToolTip (tr ("Check this box to make sure that the new feature message does not display at next start."));
  connect (dontShow, SIGNAL (stateChanged (int)), this, SLOT (slotDontShowStartupMessage (int)));
  actions->addWidget (dontShow);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the new feature dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseStartupMessage ()));
  actions->addWidget (closeButton);

  show ();
}



startupMessage::~startupMessage ()
{
}



void 
startupMessage::slotDontShowStartupMessage (int state)
{
  if (state)
    {
      options->new_feature = new_feature;
      options->startup_message = NVFalse;
    }
  else
    {
      options->startup_message = NVTrue;
    }
}



void 
startupMessage::slotCloseStartupMessage ()
{
  close ();
}
