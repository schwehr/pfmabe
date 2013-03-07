
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef NVWIN3X
  #include <windows.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <io.h>
  #include <direct.h>
#else
  #include <unistd.h>
#endif

#include "large_io.h"


#define NUM_LARGE_FILES         384
#define LARGE_NAME_SIZE         512

static FILE                     *file_pnt[NUM_LARGE_FILES];
static NV_CHAR                  file_mode[NUM_LARGE_FILES][10];
static NV_CHAR                  file_path[NUM_LARGE_FILES][LARGE_NAME_SIZE];
static NV_INT32                 file_handle[NUM_LARGE_FILES] = {
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                -1, -1, -1, -1};
static NV_CHAR                  file_context[NUM_LARGE_FILES] = {
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/***************************************************************************/
/*!

  - Module Name:        lfseek

  - Programmer(s):      Jan C. Depner

  - Date Written:       June 2009

  - Purpose:            Positions to offset within the large file.

  - Arguments:
                        - handle          =   large file handle
                        - offset          =   See fseek (same except long long)
                        - whence          =   See fseek

  - Return Value:
                        - 0               =   success
                        - -1              =   failure, see errno for reason

****************************************************************************/

NV_INT32 lfseek (NV_INT32 handle, NV_INT64 offset, NV_INT32 whence)
{
  /*  Check if a seek is required or not.  */
  /*  NOTE:  SEEK_CUR is used in the case that a seek is really required!  do NOT short circuit SEEK_CUR.  */

  if (whence == SEEK_SET && ftello64 (file_pnt[handle]) == offset) return 0;


  /*  SEEK_CUR is essentially undefined for Windoze when using large files (even in MinGW where SEEK_CUR is 
      defined to be 1).  */

#ifdef NVWIN3X
  if (whence == SEEK_CUR)
    {
      offset += ftello64 (file_pnt[handle]);
      whence = SEEK_SET;
    }
#endif


  return (fseeko64 (file_pnt[handle], offset, whence));
}




/***************************************************************************/
/*!

  - Module Name:        lfopen

  - Programmer(s):      Jan C. Depner

  - Date Written:       June 2009

  - Purpose:            Opens a large file

  - Arguments:
                        - path            =   See fopen
                        - mode            =   See fopen

  - Return Value:
                        - file handle (0 or higher)
                        - -1 on failure

****************************************************************************/

NV_INT32 lfopen (NV_CHAR *path, NV_CHAR *mode)
{
  NV_INT32                    i, handle = -1;

  /*  Check for an available handle.  */

  handle = -1;
  for (i = 0 ; i < NUM_LARGE_FILES ; i++)
    {
      if (file_handle[i] == -1)
        {
          handle = i;
          file_handle[handle] = i;
          break;
        }
    }


  /*  We may be out of handles.  */

  if (handle == -1) return (-1);


  /*  Save the path name.  */

  strcpy (file_path[handle], path);


  /*  Save the mode.   */
    
  strcpy (file_mode[handle], mode);


  /*  Open the file.  */

  if ((file_pnt[handle] = fopen64 (file_path[handle], mode)) == NULL) 
    {
      file_handle[handle] = -1;
      return (-1);
    }


  /*  Return the file handle.    */
    
  return (handle);
}




/***************************************************************************/
/*!

  - Module Name:        lfread

  - Programmer(s):      Jan C. Depner

  - Date Written:       June 2009

  - Purpose:            Reads data from the large file pointed to by handle.

  - Arguments:
                        - ptr             =   See fread
                        - size            =   See fread
                        - nmemb           =   See fread
                        - handle          =   handle of large file

  - Return Value:
                        - Number of items read
                        - 0 on failure

****************************************************************************/

size_t lfread (void *ptr, size_t size, size_t nmemb, NV_INT32 handle)
{
  /*  Note if the file context switched or not.  If it did we need to flush the buffer.  */

  if (file_context[handle] != 'r')
    {
      file_context[handle] = 'r';
      fflush (file_pnt[handle]);
    }


  /*  Read the items and return the number of items read.    */
    
  return (fread (ptr, size, nmemb, file_pnt[handle]));
}




/***************************************************************************/
/*!

  - Module Name:        lfwrite

  - Programmer(s):      Jan C. Depner

  - Date Written:       June 2009

  - Purpose:            Writes data to the large file pointed to by handle.

  - Arguments:
                        - ptr             =   See fwrite
                        - size            =   See fwrite
                        - nmemb           =   See fwrite
                        - handle          =   handle of large file

  - Return Value:
                        - Number of items written
                        - 0 on failure

****************************************************************************/

size_t lfwrite (void *ptr, size_t size, size_t nmemb, NV_INT32 handle)
{
  /*  Note if the file context switched or not.  If it did we need to flush the buffer.  */

  if (file_context[handle] != 'w')
    {
      file_context[handle] = 'w';
      fflush (file_pnt[handle]);
    }


    /*  Write the items and return the number of items written.    */
    
  return (fwrite (ptr, size, nmemb, file_pnt[handle]));
}




/***************************************************************************/
/*!

  - Module Name:        lftell

  - Programmer(s):      Jan C. Depner

  - Date Written:       June 2009

  - Purpose:            Get the current position within a large file.

  - Arguments:
                        - handle          =   handle of large file

  - Return Value:
                        - Current position within the large file

****************************************************************************/

NV_INT64 lftell (NV_INT32 handle)
{
  return (ftello64 (file_pnt[handle]));
}




/***************************************************************************/
/*!

  - Module Name:        lftruncate

  - Programmer(s):      Jan C. Depner

  - Date Written:       June 2009

  - Purpose:            Truncates the file to "length" bytes.

  - Arguments:
                        - handle          =   handle of large file
                        - length          =   length

  - Return Value:
                        - 0               =   success
                        - -1              =   failure, see errno for reason

****************************************************************************/

NV_INT32 lftruncate (NV_INT32 handle, NV_INT64 length)
{
  NV_INT32                    opened, fd, stat;


  /*  If this file has been previously opened, close it.   */

  opened = NVFalse;
  if (file_pnt[handle] != NULL)
    {
      fclose (file_pnt[handle]);
      file_pnt[handle] = NULL;
      opened = NVTrue;
    }


  /*  Truncate the file.  */

#ifdef NVWIN3X
  fd = open (file_path[handle], O_RDWR);

  if (fd < 0) return (fd);

  stat = _chsize (fd, length);
#else
  fd = open (file_path[handle], O_RDWR | O_LARGEFILE);

  if (fd < 0) return (fd);


  stat = ftruncate64 (fd, length);
#endif

  if (stat) return (stat);

  close (fd);


  /*  If the file had been opened, re-open it.  */

  if (opened)
    {
      if ((file_pnt[handle] = fopen64 (file_path[handle], file_mode[handle])) == NULL) 
        {
          file_handle[handle] = -1;
          return (-1);
        }
    }


  return (0);
}




/***************************************************************************/
/*!

  - Module Name:        lfclose

  - Programmer(s):      Jan C. Depner

  - Date Written:       June 2009

  - Purpose:            Closes a large file

  - Arguments:
                        - handle          =   handle of large file

  - Return Value:
                        - 0               =   success
                        - EOF             =   failure

****************************************************************************/

NV_INT32 lfclose (NV_INT32 handle)
{
  NV_INT32            status;


  status = fclose (file_pnt[handle]);


  /*  Make this handle available again.  */

  file_handle[handle] = -1;


  /*  Return the status.  */
    
  return (status);
}



/***************************************************************************/
/*!

  - Module Name:        lrewind

  - Programmer(s):      Jan C. Depner

  - Date Written:       June 2009

  - Purpose:            Rewinds a large file

  - Arguments:
                        - handle          =   handle of large file

  - Return Value:
                        - void

****************************************************************************/

void lrewind (NV_INT32 handle)
{
  fseeko64 (file_pnt[handle], 0LL, SEEK_SET);
}



/***************************************************************************/
/*!

  - Module Name:        lfflush

  - Programmer(s):      Jan C. Depner

  - Date Written:       June 2009

  - Purpose:            Flushes the buffer for the opened file.

  - Arguments:
                        - handle          =   handle of large file

  - Return Value:
                        - void

****************************************************************************/

void lfflush (NV_INT32 handle)
{
  fflush (file_pnt[handle]);
}
