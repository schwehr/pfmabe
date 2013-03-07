
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



/* ****
   **** pfm_extras.c
   ****   - This file contains routines brought over from the utility 
   **** library to allow PFM applications to be built independently 
   **** from each other. Note these routines are equivalent to their utility
   **** library cousins but have pfm_ pre-appended to their name.
   ****     - Mark Paton
   ****
*/

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef NVWIN3X 
# if defined (__MINGW32__) || defined (__MINGW64__)
#  include <ctype.h>
# endif
    #define SEPARATOR '\\'
#else
# include <ctype.h>
    #define SEPARATOR '/'
#endif

#include "pfm_extras.h"

/***************************************************************************/
/*!

  - Module Name:        pfm_ngets

  - Programmer(s):      Jan C. Depner

  - Date Written:       May 1999

  - Purpose:            This is an implementation of fgets that strips the
                        carriage return off of the end of the string if
                        present.

  - Arguments:          See fgets

  - Return Value:       See fgets

****************************************************************************/

NV_CHAR *pfm_ngets (NV_CHAR *s, NV_INT32 size, FILE *stream)
{
    if (fgets (s, size, stream) == NULL) return (NULL);

    while( strlen(s)>0 && (s[strlen(s)-1] == '\n' || s[strlen(s)-1] == '\r') )
        s[strlen(s)-1] = '\0';

    if (s[strlen (s) - 1] == '\n') s[strlen (s) - 1] = 0;

    return (s);
}

/***************************************************************************/
/*!

  - Module Name:        pfm_find_startup

  - Programmer(s):      Jan C. Depner

  - Date Written:       July 1994

  - Purpose:            Search for and open for read the specified file.

  - Arguments:
                        - file_name   =   file name to search for

  - Return Value:
                        - FILE *      =   file pointer opened or NULL if not
                                          found

  - Method:             Searches the current directory first followed by
                        the user's home directory and each directory in the
                        user's path.

****************************************************************************/

FILE *pfm_find_startup (char *file_name)
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
          if (path[i] == PATH_SEPARATOR)
            {
              length = i - start;
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


/***************************************************************************/
/*!

   - Module :        pfm_posfix

   - Programmer :    Jan C. Depner

   - Date :          04/25/94

   - Purpose :       This routine converts a string containing the latitude or
                     longitude in any of the following formats to decimal
                     degrees.
                         - Hemisphere Degrees Minutes Seconds.decimal
                         - Hemisphere Degrees Minutes.decimal
                         - Hemisphere Degrees.decimal
                         - SignDegrees Minutes Seconds.decimal
                         - SignDegrees Minutes.decimal
                         - SignDegrees.decimal

   - Arguments:
                     - *string     =   character string
                     - *degs       =   degrees decimal
                     - type        =   "lat" or "lon"
                    
   - Return Value:
                     - void

****************************************************************************/

void pfm_posfix (char *string, double *degs, char *c_type)
{
    int         i, sign;
    double      fdeg, fmin, fsec, f1, f2, f3;
    char        type[5];


    strcpy (type, c_type);

    
    for (i = 0 ; i < strlen (type) ; i++) type[i] = tolower (type[i]);
    
    /*  Check for and clear sign or hemisphere indicators.          */
        
    sign = 0;
    for (i = 0 ; i < strlen (string) ; i++)
    {
        if (!strncmp (type, "lon", 3))
        {
            if (string[i] == 'W' || string[i] == 'w' || string[i] == '-')
            {
                string[i] = ' ';
                sign = 1;
            }
        }
        else
        {
            if (string[i] == 'S' || string[i] == 's' || string[i] == '-')
            {
                string[i] = ' ';
                sign = 1;
            }
        }

        if (string[i] == 'n' || string[i] == 'N' || string[i] == 'e' ||
            string[i] == 'E' || string[i] == '+') string[i] = ' ';
    }

    fdeg = 0.0;
    fmin = 0.0;
    fsec = 0.0;
    f1 = 0.0;
    f2 = 0.0;
    f3 = 0.0;

    /*  Convert the string to degrees, minutes, and seconds.        */
    
    i = sscanf (string, "%lf %lf %lf", &f1, &f2, &f3);

    /*  Based on the number of values scanned, compute the total    */
    /*  degrees.                                                    */
    
    switch (i)
    {
        case 3:
            fsec = f3 / 3600.0;

        case 2:
            fmin = f2 / 60.0;

        case 1:
            fdeg = f1;
    }

    fdeg += fmin + fsec;
    *degs = fdeg;
    if (sign) *degs = - *degs;
}


/***************************************************************************/
/*!

  - Module Name:        pfm_basename

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
                        - NV_CHAR *   =   basename of the file, for example
                            - path           dirname        basename
                            - "/usr/lib"     "/usr"         "lib"
                            - "/usr/"        "/"            "usr"
                            - "usr"          "."            "usr"
                            - "/"            "/"            "/"
                            - "."            "."            "."
                            - ".."           "."            ".."

****************************************************************************/

NV_CHAR *pfm_basename (NV_CHAR *path)
{
  static NV_CHAR  basename[512];
  NV_INT32        i, j, start = 0, len;


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

  if (!start) return (path);

  for (j = start ; j < len ; j++) basename[j - start] = path[j];
  basename[len - start] = 0;

  return (basename);
}


/***************************************************************************/
/*!

  - Module Name:        pfm_dirname

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
                        - NV_CHAR *   =   basename of the file, for example
                            - path           dirname        basename
                            - "/usr/lib"     "/usr"         "lib"
                            - "/usr/"        "/"            "usr"
                            - "usr"          "."            "usr"
                            - "/"            "/"            "/"
                            - "."            "."            "."
                            - ".."           "."            ".."

****************************************************************************/

NV_CHAR *pfm_dirname (NV_CHAR *path)
{
  static NV_CHAR  dirname[512];
  NV_INT32        i, j, end = 0, len;


  len = strlen (path);

  if (path[len - 1] == '/' || path[len - 1] == '\\') len--;
  if (!len) return (path);

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

  - Module Name:        pfm_standard_attr_name

  - Programmer(s):      Jan C. Depner

  - Date Written:       April 2008

  - Purpose:            This function converts a PFM standard attribute
                        name (those composed of ### and a number, e.g. ###3)
                        to a character string that makes sense.  For the
                        lang and country fields I'm using the ones from QT4.
                        So far only the C locale (1, 0) and
                        English/United States (31, 225) are defined.

  - Arguments:
                        - name        =   standard attribute name

  - Return Value:
                        - NV_CHAR *   =   translated attribute name or the
                                          original attribute name if the
                                          lang and country fields aren't
                                          supported or if this wasn't a
                                          standard attribute name (no ###).

****************************************************************************/

NV_CHAR *pfm_standard_attr_name (NV_INT32 lang, NV_INT32 country, NV_CHAR *name)
{
  static NV_CHAR      new_name[60];


  if ((lang == 31 || lang == 1) && (country == 0 || country == 225))
    {
      if (!strncmp (name, "###", 3))
	{
	  NV_INT32 res_num;
	  sscanf (name, "###%d", &res_num);

	  switch (res_num)
	    {
	    case 0:
	      strcpy (new_name, "CUBE number of hypotheses");
	      break;
	    case 1:
	      strcpy (new_name, "CUBE computed vertical uncertainty");
	      break;
	    case 2:
	      strcpy (new_name, "CUBE hypothesis strength");
	      break;
	    case 3:
	      strcpy (new_name, "CUBE number of contributing soundings");
	      break;
	    case 4:
	      strcpy (new_name, "Average of vertical TPE for best hypothesis soundings");
	      break;
	    case 5:
	      strcpy (new_name, "Final uncertainty (MAX of CUBE StdDev and Average TPE)");
	      break;
	    }

	  return (new_name);
	}
    }

  return (name);
}
