
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



#ifndef __CHARTS2LASDEF_H__
#define __CHARTS2LASDEF_H__

#include <liblas/capi/liblas.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <proj_api.h>

#include <QtCore>
#include <QtGui>

#include "nvutility.h"

#include "FileHydroOutput.h"
#include "FileTopoOutput.h"


#define  GCS_WGS_84           4326
#define  GCS_NAD83            4269

typedef struct 
{
  NV_U_INT16                  key_directory_version;
  NV_U_INT16                  key_revision;
  NV_U_INT16                  minor_revision;
  NV_U_INT16                  number_of_keys;
  NV_U_INT16                  key_id_0;
  NV_U_INT16                  TIFF_tag_location_0;
  NV_U_INT16                  count_0;
  NV_U_INT16                  value_offset_0;
  NV_U_INT16                  key_id_1;
  NV_U_INT16                  TIFF_tag_location_1;
  NV_U_INT16                  count_1;
  NV_U_INT16                  value_offset_1;
  //NV_U_INT16                  key_id_2;
  //NV_U_INT16                  TIFF_tag_location_2;
  //NV_U_INT16                  count_2;
  //NV_U_INT16                  value_offset_2;
} GEOKEYDIRECTORYTAG_RECORD;


typedef struct
{
  QGroupBox           *obox;
  QGroupBox           *fbox;
  QProgressBar        *obar;
  QProgressBar        *fbar;
  QListWidget         *list;
} RUN_PROGRESS;


#endif
