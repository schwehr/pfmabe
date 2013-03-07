
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



#ifndef _EXAMGSF_DEF_H_
#define _EXAMGSF_DEF_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <cmath>

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "gsf.h"
#include "gsf_indx.h"
#include "hmpsflag.h"
#include "check_flag.h"
#include "profile.hpp"


#include <QtCore>
#include <QtGui>


#define         EXAM_X_SIZE                 1200
#define         EXAM_Y_SIZE                 625
#define         BEAM_X_SIZE                 1200
#define         BEAM_Y_SIZE                 400
#define         OTHER_X_SIZE                800
#define         OTHER_Y_SIZE                600
#define         EXTENDED_X_SIZE             500
#define         EXTENDED_Y_SIZE             400
#define         PROFILE_X_SIZE              1200
#define         PROFILE_Y_SIZE              300
#define         SVP_X_SIZE                  200
#define         SVP_Y_SIZE                  600
#define         PAGE_SIZE                   12
#define         GSF_COLUMNS                 15
#define         HALF_PAGE                   (PAGE_SIZE / 2)


//  The OPTIONS structure contains all those variables that can be saved to the
//  users examGSF QSettings.
    
typedef struct
{
  NV_INT32    position_form;              //  Position format number
  NV_INT32    width;                      //  Main window width
  NV_INT32    height;                     //  Main window height
  NV_INT32    window_x;                   //  Main window x position
  NV_INT32    window_y;                   //  Main window y position
  NV_INT32    beam_width;                 //  Beam window width
  NV_INT32    beam_height;                //  Beam window height
  NV_INT32    beam_window_x;              //  Beam window x position
  NV_INT32    beam_window_y;              //  Beam window y position
  NV_INT32    other_width;                //  Other window width
  NV_INT32    other_height;               //  Other window height
  NV_INT32    other_window_x;             //  Other window x position
  NV_INT32    other_window_y;             //  Other window y position
  NV_INT32    extended_width;             //  Extended window width
  NV_INT32    extended_height;            //  Extended window height
  NV_INT32    extended_window_x;          //  Extended window x position
  NV_INT32    extended_window_y;          //  Extended window y position
  NV_INT32    profile_width;              //  Profile window width
  NV_INT32    profile_height;             //  Profile window height
  NV_INT32    profile_window_x;           //  Profile window x position
  NV_INT32    profile_window_y;           //  Profile window y position
  NV_INT32    svp_width;                  //  SVP window width
  NV_INT32    svp_height;                 //  SVP window height
  NV_INT32    svp_window_x;               //  SVP window x position
  NV_INT32    svp_window_y;               //  SVP window y position
  NV_INT32    depth_units;                //  0=meters, 1=feet, 2=fathoms, 3=cubits, 4=willetts
  QString     input_dir;                  //  Last directory searched for GSF files
} OPTIONS;


// General stuff.

typedef struct
{
  QString     depth_string[5];            //  0=(meters), 1=(feet), 2=(fathoms), 3=(cubits), 4=(willetts)
  NV_FLOAT32  depth_factor[5];            //  0=1.00000, 1=3.28084, 2=0.54681, 3=0.44196, 4=16.4042
} MISC;


#endif
