
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



#ifndef HUGE_IO
#define HUGE_IO

#include "pfm_nvtypes.h"


#ifdef  __cplusplus
extern "C" {
#endif


#define MAX_SUB_FILE_SIZE       (NV_INT32_MAX - 65536.0)


NV_INT32 hfseek (NV_INT32 handle, NV_INT64 offset, NV_INT32 whence);
NV_INT32 hfopen (NV_CHAR *path, NV_CHAR *mode);
size_t hfread (void *ptr, size_t size, size_t nmemb, NV_INT32 handle);
size_t hfwrite (void *ptr, size_t size, size_t nmemb, NV_INT32 handle);
NV_INT32 hftruncate (NV_INT32 handle, NV_INT64 length);
NV_INT64 hftell (NV_INT32 handle);
NV_INT32 hfclose (NV_INT32 handle);
void hrewind (NV_INT32 handle);
void hfflush (NV_INT32 handle);


#ifdef  __cplusplus
}
#endif


#endif
