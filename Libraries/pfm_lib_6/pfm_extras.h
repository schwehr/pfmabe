
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

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/





#ifndef PFM_EXTRAS_H
#define PFM_EXTRAS_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "pfm_nvtypes.h"


FILE *pfm_find_startup (char *file_name);
NV_CHAR *pfm_ngets (NV_CHAR *s, NV_INT32 size, FILE *stream);
void pfm_posfix (char *string, double *degs, char *c_type);
NV_CHAR *pfm_basename (NV_CHAR *path);
NV_CHAR *pfm_dirname (NV_CHAR *path);
NV_CHAR *pfm_standard_attr_name (NV_INT32 lang, NV_INT32 country, NV_CHAR *name);

#ifdef  __cplusplus
}
#endif

#endif
