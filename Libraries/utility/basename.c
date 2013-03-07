
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
#include "basename.h"

/***************************************************************************/
/*!
  - Module Name:        gen_basename

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 2004

  - Purpose:            Generic replacement for POSIX basename.  One
                        advantage to this routine over the POSIX one is
                        that it doesn't destroy the input path.  This
                        works on Windoze even when using MSYS (both types
                        of specifiers).

  - Arguments:
                        - path        =   path to parse

  - Return Value:
                        - basename of the file, for example:
                            - path           dirname        basename
                            - "/usr/lib"     "/usr"         "lib"
                            - "/usr/"        "/"            "usr"
                            - "usr"          "."            "usr"
                            - "/"            "/"            "/"
                            - "."            "."            "."
                            - ".."           "."            ".."

****************************************************************************/
                                                                            
NV_CHAR *gen_basename (const NV_CHAR *path)
{
  static NV_CHAR  basename[512];
  NV_INT32        i, j, start = 0, len;


  strcpy (basename, path);

  len = strlen (path);

  if (path[len - 1] == '/' || path[len - 1] == '\\') len--;

  start = 0;
  for (i = len - 1 ; i >= 0 ; i--)
    {
      if (path[i] == '/' || path[i] == '\\')
        {
          start = i + 1;
          break;
        }
    }

  if (!start) return (basename);

  for (j = start ; j < len ; j++) basename[j - start] = path[j];
  basename[len - start] = 0;

  return (basename);
}


/***************************************************************************/
/*!

  - Module Name:        gen_dirname

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 2004

  - Purpose:            Generic replacement for POSIX dirname.  One
                        advantage to this routine over the POSIX one is
                        that it doesn't destroy the input path.  This
                        works on Windoze even when using MSYS (both types
                        of specifiers).

  - Arguments:
                        - path        =   path to parse

  - Return Value:
                        - basename of the file, for example:
                            - path           dirname        basename
                            - "/usr/lib"     "/usr"         "lib"
                            - "/usr/"        "/"            "usr"
                            - "usr"          "."            "usr"
                            - "/"            "/"            "/"
                            - "."            "."            "."
                            - ".."           "."            ".."

****************************************************************************/
                                                                            
NV_CHAR *gen_dirname (const NV_CHAR *path)
{
  static NV_CHAR  dirname[512];
  NV_INT32        i, j, end = 0, len;


  strcpy (dirname, path);

  len = strlen (path);

  if (path[len - 1] == '/' || path[len - 1] == '\\') len--;
  if (!len) return (dirname);

  end = 0;
  for (i = len - 1 ; i >= 0 ; i--)
    {
      if (path[i] == '/' || path[i] == '\\')
        {
          end = i;
          break;
        }
    }

  if (!end) return (".");

  for (j = 0 ; j < end ; j++) dirname[j] = path[j];
  dirname[end] = 0;

  return (dirname);
}



/***************************************************************************/
/*!

  - Module Name:        gen_strrstr

  - Programmer(s):      Unknown

  - Date Written:       Unknown

  - Purpose:            One of the many implementations of strrstr (which
                        doesn't exist in C) to be found on the internet.
                        There are more efficient ways to do this but I
                        always appreciate simplicity.

  - Arguments:
                        - str         =   path to search
                        - strSearch   =   string to be searched for

  - Return Value:
                        - pointer to search string in str

****************************************************************************/
                                                                            
NV_CHAR *gen_strrstr (const NV_CHAR *str, const NV_CHAR *strSearch)
{
  NV_CHAR *ptr, *last = NULL;

  ptr = (NV_CHAR *) str;

  while ((ptr = strstr (ptr, strSearch))) last = ptr++;

  return last;
}
