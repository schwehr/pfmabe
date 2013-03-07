
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



/*  gsfMonitor class definitions.  */

#ifndef GSFMONITOR_H
#define GSFMONITOR_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <getopt.h>
#include <cmath>

#include "nvutility.h"
#include "pfm.h"
#include "fixpos.h"
#include "pfm_extras.h"
#include "nvmap.hpp"
#include "ABE.h"
#include "gsf.h"
#include "hmpsflag.h"
#include "check_flag.h"
#include "version.hpp"

#ifdef NVWIN3X
    #include "windows_getuid.h"
#endif

#include <QtCore>
#include <QtGui>


class gsfMonitor:public QMainWindow
{
  Q_OBJECT 


public:

  gsfMonitor (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~gsfMonitor ();


protected:

  NV_INT32            key;

  NV_U_INT32          kill_switch;

  QSharedMemory       *abeShare;

  ABE_SHARE           *abe_share;

  QTextEdit           *listBox;

  NV_INT32            width, height, window_x, window_y;

  QPushButton         *bRestoreDefaults; 

  NV_BOOL             force_redraw, lock_track;

  NV_U_INT32          ac[1];

  NV_INT32            pos_format;

  QButtonGroup        *bGrp;

  QDialog             *prefsD;

  QToolButton         *bQuit, *bPrefs;



  void envin ();
  void envout ();

  void closeEvent (QCloseEvent *);


protected slots:

  void slotResize (QResizeEvent *e);
  
  void trackCursor ();

  void slotHelp ();

  void slotQuit ();

  void slotPrefs ();
  void slotPosClicked (int id);
  void slotClosePrefs ();

  void slotRestoreDefaults ();
  
  void about ();
  void aboutQt ();


 private:

};

#endif
