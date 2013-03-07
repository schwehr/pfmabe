
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



#ifndef _ABE_REGISTER_HPP_
#define _ABE_REGISTER_HPP_

#include "nvtypes.h"
#include "nvdef.h"

#ifdef NVWIN3X
    #include "windows_getuid.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <QtGui>
#include <QSharedMemory>


#define         MAX_ABE_GROUPS              32
#define         MAX_ABE_MEMBERS             32


/*!  Commands - can be issued by any registered process.  The ABE_REGISTER_COMMAND structure can be 
     easily expanded to accomodate new command needs since it is not in shared memory.  */

#define         ZOOM_TO_MBR_COMMAND         7000



typedef struct
{
  NV_INT32    id;                         //!<  Command ID - see above
  NV_INT32    window_id;                  //!<  Process ID of the issuing application
  NV_INT32    number;                     /*!<  Set to a random number by the issuing app (so that other apps will only 
                                                respond to the command once).  If this is zero then no command is
                                                being sent.  */
  NV_F64_XYMBR mbr;                       //!<  Active applications MBR
} ABE_REGISTER_COMMAND;


typedef struct
{
  NV_INT32    active_window_id;           //!<  Process ID of the active window (set by the mouseMove function)
  NV_F64_COORD3 cursor_position;          //!<  Position of the cursor (and possibly depth) in the application
  NV_INT32    polygon_count;              //!<  Number of points in polygon
  NV_FLOAT64  polygon_x[2000];            //!<  Polygon X positions
  NV_FLOAT64  polygon_y[2000];            //!<  Polygon Y positions
} ABE_REGISTER_DATA;


typedef struct
{
  NV_INT32    key;                        //!<  Member's process ID (0 if inactive).
  NV_CHAR     name[30];                   //!<  Process name (e.g. pfmView, areaCheck, possibly CNILE...)
  NV_CHAR     file[512];                  //!<  File name associated with process (if applicable).
} ABE_MEMBER;
					      

/*!
    The ABE_REGISTER shared memory structure is used to connect ABE mapping GUIs with other ABE or CNILE mapping
    GUIs.  When we try to connect two processes we will use this shared memory block to do it.  If this structure
    doesn't exist we will create it.  This structure allows us to track the cursor and share other information
    across applications that were not shelled from each other (like pfmEdit is shelled from pfmView).  The main
    reason for implementing this was to try to get CNILE and ABE talking to each other.  The shared memory key
    for this block will ALWAYS be the user's UID.  
    The ABE_REGISTER_DATA structure is where we'll store the information that will be passed between applications.
*/


typedef struct
{
  NV_BOOL     group[MAX_ABE_GROUPS];                    //!<  Set to NVTrue if active (has members).
  NV_INT32    member_count[MAX_ABE_GROUPS];             //!<  Number of members in this group
  ABE_REGISTER_DATA data[MAX_ABE_GROUPS];               //!<  Shared data
  ABE_MEMBER  member[MAX_ABE_GROUPS][MAX_ABE_MEMBERS];  //!<  Member structures per group
  ABE_REGISTER_COMMAND command[MAX_ABE_GROUPS];         //!<  Command issued by any app in the group.
} ABE_REGISTER;



#endif

