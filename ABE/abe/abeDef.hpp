
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



#ifndef _ABE_TOOLBAR_DEF_H_
#define _ABE_TOOLBAR_DEF_H_


#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "nvutility.h"
#include "nvutility.hpp"


#include <QtCore>
#include <QtGui>


using namespace std;  // Windoze bullshit - God forbid they should follow a standard


#define         TRACKLINE                   0
#define         AREACHECK                   1
#define         PFMLOAD                     2
#define         PFMVIEW                     3
#define         GEOSWATH3D                  4
#define         EXAMGSF                     5
#define         PFMFEATURE                  6
#define         PFMEXTRACT                  7
#define         PFMGEOTIFF                  8
#define         PFMCHARTSIMAGE              9
#define         CHARTS2LAS                  10
#define         PFMBAG                      11
#define         BAGGEOTIFF                  12
#define         BAGVIEWER                   13

#define         NUM_PROGS                   14      //  Number of program toolbuttons defined


//  The OPTIONS structure contains all those variables that can be saved to the users abe QSettings.


typedef struct
{
  QString     buttonAccel[NUM_PROGS];     //  Accelerator key sequences for the buttons
  NV_INT32    main_button_icon_size;      //  Main button icon size (16, 24, 32)
  NV_INT32    orientation;                //  Qt::Horizontal or Qt::Vertical
  NV_INT32    window_x;                   //  Main window x position
  NV_INT32    window_y;                   //  Main window y position
} OPTIONS;


//  General stuff.

typedef struct
{
  QString     buttonWhat[NUM_PROGS];      //  WhatsThis text for the buttons
  QIcon       buttonIcon[NUM_PROGS];      //  Button icons
  NV_BOOL     buttonFile[NUM_PROGS];      //  NVTrue if program can accept command line file name
  QString     progName[NUM_PROGS];        //  Actual program names
  QString     qsettings_org;              //  Organization name for QSettings
  QString     qsettings_app;              //  Application name for QSettings
} MISC;


#endif
