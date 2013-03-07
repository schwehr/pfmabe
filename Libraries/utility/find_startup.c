
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



#include "find_startup.h"

/***************************************************************************/
/*!

  - Module Name:        find_startup

  - Programmer(s):      Jan C. Depner

  - Date Written:       July 1994

  - Purpose:            Search for and open for read the specified file.

  - Arguments:          file_name   -   file name to search for

  - Return Value:       FILE *      -   file pointer opened or NULL if not
                                        found

  - Method:             Searches the current directory first followed by
                       the user's home directory and each directory in the
                       user's path.

****************************************************************************/
                                                                            
FILE *find_startup (const NV_CHAR *file_name)
{
  NV_CHAR          path[4096], file[512], tmp[512];
  FILE             *fp;
  NV_INT32         i, length, start;

    
  strcpy (file, file_name);


  /*  File in working directory.  */
    
  if ((fp = fopen (file, "r")) != NULL) return (fp);

    
  /*  File in HOME directory.  Deal with possibility of MSYS on Windoze.  */
    
  if (getenv ("HOME") != NULL)
    {
      sprintf (file, "%s/%s", getenv ("HOME"), file_name);

      if ((fp = fopen (file, "r")) != NULL) return (fp);
 
      sprintf (file, "%s\\%s", getenv ("HOME"), file_name);

      if ((fp = fopen (file, "r")) != NULL) return (fp);
    }

    
  /*  File in path.   */
    
  if (getenv ("PATH") != NULL)
    {
      strcpy (path, getenv ("PATH"));

      start = 0;
            
      for (i = 0 ; i < (NV_INT32) strlen (path) ; i++)
        {
          if (path[i] == PATH_SEPARATOR || i == ((int) strlen (path) - 1))
            {
              length = i - start;

              if (i == ((int) strlen (path) - 1)) length++;

              strncpy (file, &path[start], length);
              file[length] = 0;
              start = i + 1;
            
              sprintf (tmp, "%s%1c%s", file, SEPARATOR, file_name);

              if ((fp = fopen (tmp, "r")) != NULL) return (fp);
            }
        }
    }

    
  /*  No file found.  */

  return (NULL);
}
